#include "speakable_cnn.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

// ----------------------------------------------------------------------
// 테스트벤치: 하드웨어 함수 결과를 순수 C++ 참조 구현(골든 모델)과 바이트 단위로 비교
//
//  - 입력/가중치/바이어스는 고정 시드 난수 (INT8 전 범위 사용)
//  - 여러 레이어 크기를 연달아 실행해서
//      · 8의 배수가 아닌 크기 (채널/폭/높이)
//      · 홀수 높이·폭에서의 풀링
//      · 큰 레이어 뒤에 작은 레이어 실행 시 온칩 버퍼 잔여값 영향
//    까지 확인함
//  - 유효 영역 밖(패딩 영역)을 침범해서 쓰지 않는지도 확인
// ----------------------------------------------------------------------

// PS(ARM)와 같은 DDR 배치: int8 배열을 64비트 워드 배열로 그대로 해석
static int8_t  ifmap_b[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W];
static int8_t  weight_b[MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE];
static int32_t bias_b[MAX_OUT_CH];
static int8_t  ofmap_b[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];
static int8_t  ref_b[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];

static word_t ifmap_w[FMAP_WORDS];
static word_t weight_w[WEIGHT_WORDS];
static acc_t  bias_w[MAX_OUT_CH];
static word_t ofmap_w[FMAP_WORDS];

static const int8_t SENTINEL = 0x5A;   // 출력 버퍼 초기값. 쓰면 안 되는 영역이 그대로인지 확인용

static uint32_t rng_state = 12345;
static uint32_t rng() {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state >> 8;
}

static void pack_bytes(const int8_t *src, word_t *dst, int n_words) {
    for (int i = 0; i < n_words; i++) {
        word_t v = 0;
        for (int b = 0; b < WORD_BYTES; b++)
            v.range(8 * b + 7, 8 * b) = (uint8_t)src[i * WORD_BYTES + b];
        dst[i] = v;
    }
}

static void unpack_bytes(const word_t *src, int8_t *dst, int n_words) {
    for (int i = 0; i < n_words; i++)
        for (int b = 0; b < WORD_BYTES; b++)
            dst[i * WORD_BYTES + b] = (int8_t)(uint8_t)src[i].range(8 * b + 7, 8 * b).to_uint();
}

// ----------------------------------------------------------------------
// 골든 모델: conv3x3(pad=1) + bias → >>8 → [ReLU] → clamp → [maxpool 2x2]
// ----------------------------------------------------------------------
static int8_t conv_ref(int oc, int oh, int ow, int H, int W, int IC, bool relu) {
    int64_t acc = bias_b[oc];
    for (int ic = 0; ic < IC; ic++)
        for (int kh = 0; kh < KSIZE; kh++)
            for (int kw = 0; kw < KSIZE; kw++) {
                int ih = oh + kh - 1, iw = ow + kw - 1;
                if (ih < 0 || ih >= H || iw < 0 || iw >= W) continue;
                acc += (int)ifmap_b[ic][ih][iw] * (int)weight_b[oc][ic][kh][kw];
            }
    int64_t s = acc >> 8;             // 산술 시프트 (하드웨어 acc_t >> 8과 동일)
    if (relu && s < 0) s = 0;
    if (s > 127)  s = 127;
    if (s < -128) s = -128;
    return (int8_t)s;
}

static void reference(int H, int W, int IC, int OC, bool relu, bool pool) {
    for (int oc = 0; oc < OC; oc++) {
        if (pool) {
            for (int ph = 0; ph < H / 2; ph++)
                for (int pw = 0; pw < W / 2; pw++) {
                    int8_t m = -128;
                    for (int dy = 0; dy < 2; dy++)
                        for (int dx = 0; dx < 2; dx++) {
                            int8_t v = conv_ref(oc, 2 * ph + dy, 2 * pw + dx, H, W, IC, relu);
                            if (v > m) m = v;
                        }
                    ref_b[oc][ph][pw] = m;
                }
        } else {
            for (int oh = 0; oh < H; oh++)
                for (int ow = 0; ow < W; ow++)
                    ref_b[oc][oh][ow] = conv_ref(oc, oh, ow, H, W, IC, relu);
        }
    }
}

struct test_case_t {
    int  h, w, in_ch, out_ch;
    bool relu, pool;
};

static bool run_case(const test_case_t &t) {
    // 매 케이스마다 입력 전체(패딩 영역 포함)를 새 난수로 채움
    // → 하드웨어가 유효 영역 밖 데이터를 읽어서 쓰면 결과가 틀어져서 잡힘
    for (int c = 0; c < MAX_IN_CH; c++)
        for (int h = 0; h < MAX_IMG_H; h++)
            for (int w = 0; w < MAX_IMG_W; w++)
                ifmap_b[c][h][w] = (int8_t)(rng() & 0xFF);
    for (int oc = 0; oc < MAX_OUT_CH; oc++) {
        bias_b[oc] = (int32_t)(rng() % 4001) - 2000;
        for (int ic = 0; ic < MAX_IN_CH; ic++)
            for (int kh = 0; kh < KSIZE; kh++)
                for (int kw = 0; kw < KSIZE; kw++)
                    weight_b[oc][ic][kh][kw] = (int8_t)(rng() & 0xFF);
    }
    std::memset(ofmap_b, SENTINEL, sizeof(ofmap_b));

    pack_bytes(&ifmap_b[0][0][0], ifmap_w, FMAP_WORDS);
    pack_bytes(&weight_b[0][0][0][0], weight_w, WEIGHT_WORDS);
    pack_bytes(&ofmap_b[0][0][0], ofmap_w, FMAP_WORDS);
    for (int i = 0; i < MAX_OUT_CH; i++) bias_w[i] = bias_b[i];

    perf_counters_t perf = {0, 0, 0};
    speakable_cnn_top(ifmap_w, weight_w, bias_w, ofmap_w,
                 t.h, t.w, t.in_ch, t.out_ch,
                 t.relu ? 1 : 0, t.pool ? 1 : 0,
                 perf);

    unpack_bytes(ofmap_w, &ofmap_b[0][0][0], FMAP_WORDS);
    reference(t.h, t.w, t.in_ch, t.out_ch, t.relu, t.pool);

    const int oh_n = t.pool ? t.h / 2 : t.h;
    const int ow_n = t.pool ? t.w / 2 : t.w;
    // 하드웨어는 행을 8바이트 워드 단위로 쓰므로, 유효 폭 이후 워드 끝까지는 0이 써짐
    const int ow_written = ((ow_n + WORD_BYTES - 1) / WORD_BYTES) * WORD_BYTES;

    int mismatches = 0, clobbered = 0;
    for (int oc = 0; oc < MAX_OUT_CH; oc++)
        for (int h = 0; h < MAX_IMG_H; h++)
            for (int w = 0; w < MAX_IMG_W; w++) {
                const int8_t got = ofmap_b[oc][h][w];
                const bool in_valid = oc < t.out_ch && h < oh_n && w < ow_n;
                const bool in_word_pad = oc < t.out_ch && h < oh_n && w >= ow_n && w < ow_written;
                if (in_valid) {
                    if (got != ref_b[oc][h][w]) {
                        if (mismatches < 5)
                            std::cout << "    MISMATCH ofmap[" << oc << "][" << h << "][" << w << "] hw="
                                      << (int)got << " ref=" << (int)ref_b[oc][h][w] << std::endl;
                        mismatches++;
                    }
                } else if (in_word_pad) {
                    if (got != 0) clobbered++;
                } else if (got != SENTINEL) {
                    clobbered++;
                }
            }

    const uint32_t mac_ref = (uint32_t)t.in_ch * t.out_ch * (3 * t.h - 2) * (3 * t.w - 2);
    const uint32_t iter_ref = (uint32_t)t.h * t.w * ((t.in_ch + TIC - 1) / TIC) * KK * ((t.out_ch + TOC - 1) / TOC);
    const bool mac_ok  = perf.mac_count.to_uint() == mac_ref;
    const bool iter_ok = perf.cycle_count.to_uint() == iter_ref;

    const bool pass = mismatches == 0 && clobbered == 0 && mac_ok && iter_ok;
    std::cout << (pass ? "  [PASS] " : "  [FAIL] ")
              << "H=" << t.h << " W=" << t.w << " IC=" << t.in_ch << " OC=" << t.out_ch
              << " relu=" << t.relu << " pool=" << t.pool
              << " | mismatch=" << mismatches << " clobbered=" << clobbered
              << " | cycle_count=" << perf.cycle_count << (iter_ok ? "" : " (expected " + std::to_string(iter_ref) + ")")
              << " mac_count=" << perf.mac_count << (mac_ok ? "" : " (expected " + std::to_string(mac_ref) + ")")
              << std::endl;
    return pass;
}

int main() {
    // 큰 채널 수 → 작은 채널 수 순서로 섞어서, 이전 레이어의 온칩 잔여값이 새어 들어오는지도 검사
    const test_case_t cases[] = {
        {16, 16,  8,  4, true,  true },   // 기본 크기
        {13, 11,  5, 10, false, false},   // 8의 배수가 아닌 크기, 풀링 없음
        {32, 40, 16, 24, true,  false},   // 여러 band / 여러 출력 채널 타일
        {17, 19, 13,  9, true,  true },   // 홀수 크기 풀링 (마지막 행/열 버림)
        { 1,  1,  3,  2, false, false},   // 최소 크기 (1x1 이미지)
        {40, 64, 64, 64, true,  true },   // 최대 채널/폭
        { 9,  7,  1, 17, false, true },   // 입력 채널 1개 (첫 레이어 형태)
    };

    int n_fail = 0;
    for (const test_case_t &t : cases)
        if (!run_case(t)) n_fail++;

    std::cout << (n_fail == 0 ? "TEST PASSED" : "TEST FAILED")
              << " (" << (int)(sizeof(cases) / sizeof(cases[0])) - n_fail << "/"
              << (int)(sizeof(cases) / sizeof(cases[0])) << " cases)" << std::endl;
    return n_fail == 0 ? 0 : 1;
}
