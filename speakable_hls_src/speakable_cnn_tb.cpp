#include "speakable_cnn.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

// 실제 모델 가중치와 테스트 벡터 (학습 산출물 → speakable_arm_ref/tools/export_c.py로 생성)
#include "../speakable_arm_ref/generated/speakable_weights.h"
#include "../speakable_arm_ref/generated/speakable_testvec.h"

// ----------------------------------------------------------------------
// 테스트벤치
//  1) 레이어 종류·stride·크기를 바꿔 가며 하드웨어 함수 결과를 골든 모델과 바이트 단위 비교
//     (유효 영역 밖을 덮어쓰지 않는지, 이전 레이어 잔여값이 새지 않는지도 확인)
//  2) 실제 모델 L1~L9를 하드웨어 함수로 연결 실행해 학습 쪽 정수 추론 출력(테스트 벡터)과 비교
// ----------------------------------------------------------------------

static const int8_t SENTINEL = 0x5A;

// PS(ARM)와 같은 DDR 배치
static int8_t  ifmap_b[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W];
static int8_t  weight_b[MAX_OUT_CH][MAX_IN_CH * KK];      // 출력 채널마다 576바이트 칸
static int32_t bias_b[MAX_OUT_CH], mult_b[MAX_OUT_CH], shift_b[MAX_OUT_CH];
static int8_t  ofmap_b[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];
static int8_t  ref_b[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];

// 레이어 종류별 원래 가중치 (골든 모델용)
static int8_t  w_conv[MAX_OUT_CH][MAX_IN_CH][KK];
static int8_t  w_dw[MAX_OUT_CH][KK];
static int8_t  w_pw[MAX_OUT_CH][MAX_IN_CH];

static word_t ifmap_w[FMAP_WORDS], ofmap_w[FMAP_WORDS], weight_w[WEIGHT_WORDS];
static acc_t  bias_w[MAX_OUT_CH], mult_w[MAX_OUT_CH], shift_w[MAX_OUT_CH];

static uint32_t rng_state = 12345;
static uint32_t rng() { rng_state = rng_state * 1664525u + 1013904223u; return rng_state >> 8; }

static void pack_bytes(const int8_t *src, word_t *dst, int n_words) {
    for (int i = 0; i < n_words; i++) {
        word_t v = 0;
        for (int b = 0; b < WORD_BYTES; b++) v.range(8 * b + 7, 8 * b) = (uint8_t)src[i * WORD_BYTES + b];
        dst[i] = v;
    }
}
static void unpack_bytes(const word_t *src, int8_t *dst, int n_words) {
    for (int i = 0; i < n_words; i++)
        for (int b = 0; b < WORD_BYTES; b++) dst[i * WORD_BYTES + b] = (int8_t)(uint8_t)src[i].range(8 * b + 7, 8 * b).to_uint();
}

// ----------------------------------------------------------------------
// 골든 모델 (ARM 기준 구현 speakable_model.c와 같은 정수 규약)
// ----------------------------------------------------------------------
static int8_t requant_ref(int64_t acc, int32_t mult, int shift, bool relu) {
    const int64_t prod = acc * (int64_t)mult;
    const int64_t half = shift > 0 ? ((int64_t)1 << (shift - 1)) : 0;
    int64_t v = prod >= 0 ? ((prod + half) >> shift) : -((-prod + half) >> shift);
    if (v > 127) v = 127;
    if (v < -128) v = -128;
    if (relu && v < 0) v = 0;
    return (int8_t)v;
}

static int64_t acc_ref(int type, int oc, int oh, int ow, int H, int W, int IC, int s) {
    int64_t acc = bias_b[oc];
    if (type == LAYER_PW1X1) {
        for (int ic = 0; ic < IC; ic++) acc += ifmap_b[ic][oh * s][ow * s] * w_pw[oc][ic];
        return acc;
    }
    for (int kh = 0; kh < 3; kh++)
        for (int kw = 0; kw < 3; kw++) {
            const int ih = oh * s + kh - 1, iw = ow * s + kw - 1;
            if (ih < 0 || ih >= H || iw < 0 || iw >= W) continue;
            if (type == LAYER_DW3X3) acc += ifmap_b[oc][ih][iw] * w_dw[oc][kh * 3 + kw];
            else for (int ic = 0; ic < IC; ic++) acc += ifmap_b[ic][ih][iw] * w_conv[oc][ic][kh * 3 + kw];
        }
    return acc;
}

static void reference(int type, int H, int W, int IC, int OC, int s, bool relu, bool pool) {
    const int OH = (H - 1) / s + 1, OW = (W - 1) / s + 1;
    static int8_t tmp[MAX_IMG_H][MAX_IMG_W];
    for (int oc = 0; oc < OC; oc++) {
        for (int oh = 0; oh < OH; oh++)
            for (int ow = 0; ow < OW; ow++)
                tmp[oh][ow] = requant_ref(acc_ref(type, oc, oh, ow, H, W, IC, s), mult_b[oc], shift_b[oc], relu);
        for (int oh = 0; oh < (pool ? OH / 2 : OH); oh++)
            for (int ow = 0; ow < (pool ? OW / 2 : OW); ow++) {
                if (!pool) { ref_b[oc][oh][ow] = tmp[oh][ow]; continue; }
                int8_t m = -128;
                for (int d = 0; d < 4; d++) { const int8_t v = tmp[2 * oh + d / 2][2 * ow + d % 2]; if (v > m) m = v; }
                ref_b[oc][oh][ow] = m;
            }
    }
}

// 레이어 종류별 가중치를 DDR 칸(출력 채널마다 576바이트)에 배치
static void pack_weights(int type, int IC, int OC) {
    memset(weight_b, 0, sizeof(weight_b));
    for (int oc = 0; oc < OC; oc++) {
        if (type == LAYER_CONV3X3) for (int ic = 0; ic < IC; ic++) for (int k = 0; k < KK; k++) weight_b[oc][ic * KK + k] = w_conv[oc][ic][k];
        if (type == LAYER_DW3X3)   for (int k = 0; k < KK; k++) weight_b[oc][k] = w_dw[oc][k];
        if (type == LAYER_PW1X1)   for (int ic = 0; ic < IC; ic++) weight_b[oc][ic] = w_pw[oc][ic];
    }
    pack_bytes(&weight_b[0][0], weight_w, WEIGHT_WORDS);
    for (int i = 0; i < MAX_OUT_CH; i++) { bias_w[i] = bias_b[i]; mult_w[i] = mult_b[i]; shift_w[i] = shift_b[i]; }
}

static const char *type_name(int t) { return t == LAYER_CONV3X3 ? "CONV" : (t == LAYER_DW3X3 ? "DW  " : "PW  "); }

// ----------------------------------------------------------------------
// 1) 무작위 레이어 테스트
// ----------------------------------------------------------------------
struct case_t { int type, h, w, in_ch, out_ch, stride; bool relu, pool, ties; };

static bool run_case(const case_t &t) {
    const int IC = t.in_ch, OC = (t.type == LAYER_DW3X3) ? t.in_ch : t.out_ch;
    for (int c = 0; c < MAX_IN_CH; c++) for (int h = 0; h < MAX_IMG_H; h++) for (int w = 0; w < MAX_IMG_W; w++)
        ifmap_b[c][h][w] = t.ties ? (int8_t)((int)(rng() % 9) - 4) : (int8_t)(rng() & 0xFF);
    for (int oc = 0; oc < MAX_OUT_CH; oc++) {
        bias_b[oc] = t.ties ? (int32_t)(rng() % 9) - 4 : (int32_t)(rng() % 4001) - 2000;
        for (int k = 0; k < KK; k++) w_dw[oc][k] = (int8_t)(rng() & 0xFF);
        for (int ic = 0; ic < MAX_IN_CH; ic++) {
            w_pw[oc][ic] = t.ties ? (int8_t)((int)(rng() % 5) - 2) : (int8_t)(rng() & 0xFF);
            for (int k = 0; k < KK; k++) w_conv[oc][ic][k] = t.ties ? (int8_t)((int)(rng() % 5) - 2) : (int8_t)(rng() & 0xFF);
        }
    }
    if (t.ties) {
        // 작은 누산값 + 작은 shift → 정확히 절반·음수가 자주 나와 반올림 규칙을 직접 시험
        for (int oc = 0; oc < MAX_OUT_CH; oc++) { mult_b[oc] = 1 + (int32_t)(rng() % 3); shift_b[oc] = 1 + (int32_t)(rng() % 3); }
    } else {
        // 채널별 최대 누산값이 ±100 근처가 되도록 multiplier·shift 보정 (실제 모델과 같은 Q31 형태)
        const int OH = (t.h - 1) / t.stride + 1, OW = (t.w - 1) / t.stride + 1;
        for (int oc = 0; oc < OC; oc++) {
            int64_t mx = 1;
            for (int oh = 0; oh < OH; oh++) for (int ow = 0; ow < OW; ow++) {
                int64_t a = acc_ref(t.type, oc, oh, ow, t.h, t.w, IC, t.stride);
                if (a < 0) a = -a;
                if (a > mx) mx = a;
            }
            mult_b[oc] = (int32_t)(0x40000000u + (rng() % 0x3FFFFFFFu));
            int sh = 31;
            while (sh < 62 && ((mx * (int64_t)mult_b[oc]) >> sh) > 120) sh++;
            shift_b[oc] = sh;
        }
    }

    pack_bytes(&ifmap_b[0][0][0], ifmap_w, FMAP_WORDS);
    pack_weights(t.type, IC, OC);
    memset(ofmap_b, SENTINEL, sizeof(ofmap_b));
    pack_bytes(&ofmap_b[0][0][0], ofmap_w, FMAP_WORDS);

    perf_counters_t perf = {0, 0, 0};
    speakable_cnn_top(ifmap_w, weight_w, bias_w, mult_w, shift_w, ofmap_w,
                      t.h, t.w, IC, OC, t.type, t.stride, t.relu ? 1 : 0, t.pool ? 1 : 0, perf);
    unpack_bytes(ofmap_w, &ofmap_b[0][0][0], FMAP_WORDS);
    reference(t.type, t.h, t.w, IC, OC, t.stride, t.relu, t.pool);

    const int OH = (t.h - 1) / t.stride + 1, OW = (t.w - 1) / t.stride + 1;
    const int oh_n = t.pool ? OH / 2 : OH, ow_n = t.pool ? OW / 2 : OW;
    const int ow_written = ((ow_n + WORD_BYTES - 1) / WORD_BYTES) * WORD_BYTES;
    int mismatch = 0, clobbered = 0, nonzero = 0, ties_seen = 0;
    for (int oc = 0; oc < MAX_OUT_CH; oc++) for (int h = 0; h < MAX_IMG_H; h++) for (int w = 0; w < MAX_IMG_W; w++) {
        const int8_t got = ofmap_b[oc][h][w];
        const bool valid = oc < OC && h < oh_n && w < ow_n;
        const bool pad   = oc < OC && h < oh_n && w >= ow_n && w < ow_written;
        if (valid) {
            if (got != ref_b[oc][h][w]) {
                if (mismatch < 5) std::cout << "    MISMATCH [" << oc << "][" << h << "][" << w << "] hw=" << (int)got << " ref=" << (int)ref_b[oc][h][w] << std::endl;
                mismatch++;
            }
            if (got != 0) nonzero++;
        } else if (pad) { if (got != 0) clobbered++; }
        else if (got != SENTINEL) clobbered++;
    }
    if (t.ties) {   // 반올림 경계가 실제로 나왔는지 확인 (안 나오면 이 테스트는 의미가 없음)
        const int OHt = OH, OWt = OW;
        for (int oc = 0; oc < OC; oc++) for (int oh = 0; oh < OHt; oh++) for (int ow = 0; ow < OWt; ow++) {
            const int64_t p = acc_ref(t.type, oc, oh, ow, t.h, t.w, IC, t.stride) * mult_b[oc];
            const int64_t m = p < 0 ? -p : p;
            if (p < 0 && (m & ((1LL << shift_b[oc]) - 1)) == (1LL << (shift_b[oc] - 1))) ties_seen++;
        }
    }
    const bool ok = mismatch == 0 && clobbered == 0 && nonzero > 0 && (!t.ties || ties_seen > 0);
    std::cout << (ok ? "  [PASS] " : "  [FAIL] ") << type_name(t.type) << " s" << t.stride
              << " " << t.h << "x" << t.w << " " << IC << "->" << OC << (t.relu ? " relu" : "") << (t.pool ? " pool" : "")
              << (t.ties ? " (반올림 경계 " + std::to_string(ties_seen) + "개)" : "")
              << " | mismatch=" << mismatch << " clobbered=" << clobbered << " cycles=" << perf.cycle_count << std::endl;
    return ok;
}

// ----------------------------------------------------------------------
// 2) 실제 모델 L1~L9 연결 실행 (DDR 버퍼를 그대로 다음 레이어 입력으로 사용)
// ----------------------------------------------------------------------
struct model_layer_t {
    int type, in_c, in_h, in_w, out_c, stride;
    const int8_t *w; const int32_t *b; const int32_t *mult; const int8_t *shift;
};
static const model_layer_t MODEL[9] = {
    { LAYER_CONV3X3,  1, 40, 64, 16, 2, L1_w, L1_b, L1_mult, L1_shift },
    { LAYER_DW3X3,   16, 20, 32, 16, 1, L2_w, L2_b, L2_mult, L2_shift },
    { LAYER_PW1X1,   16, 20, 32, 32, 1, L3_w, L3_b, L3_mult, L3_shift },
    { LAYER_DW3X3,   32, 20, 32, 32, 2, L4_w, L4_b, L4_mult, L4_shift },
    { LAYER_PW1X1,   32, 10, 16, 64, 1, L5_w, L5_b, L5_mult, L5_shift },
    { LAYER_DW3X3,   64, 10, 16, 64, 1, L6_w, L6_b, L6_mult, L6_shift },
    { LAYER_PW1X1,   64, 10, 16, 64, 1, L7_w, L7_b, L7_mult, L7_shift },
    { LAYER_DW3X3,   64, 10, 16, 64, 2, L8_w, L8_b, L8_mult, L8_shift },
    { LAYER_PW1X1,   64,  5,  8, 64, 1, L9_w, L9_b, L9_mult, L9_shift },
};
static word_t fm_a[FMAP_WORDS], fm_b[FMAP_WORDS];

static bool run_model() {
    bool all_ok = true;
    for (int tv = 0; tv < SPK_NUM_TV; tv++) {
        memset(ifmap_b, 0, sizeof(ifmap_b));
        for (int h = 0; h < 40; h++) for (int w = 0; w < 64; w++) ifmap_b[0][h][w] = spk_tv_input[tv][h * 64 + w];
        pack_bytes(&ifmap_b[0][0][0], fm_a, FMAP_WORDS);
        word_t *in = fm_a, *out = fm_b;
        int bad_layers = 0;
        long long cycles = 0;

        for (int l = 0; l < 9; l++) {
            const model_layer_t &L = MODEL[l];
            memset(weight_b, 0, sizeof(weight_b));
            const int per_oc = (L.type == LAYER_CONV3X3) ? L.in_c * KK : (L.type == LAYER_DW3X3 ? KK : L.in_c);
            for (int oc = 0; oc < L.out_c; oc++) {
                memcpy(weight_b[oc], L.w + oc * per_oc, per_oc);
                bias_b[oc] = L.b[oc]; mult_b[oc] = L.mult[oc]; shift_b[oc] = L.shift[oc];
            }
            pack_bytes(&weight_b[0][0], weight_w, WEIGHT_WORDS);
            for (int i = 0; i < MAX_OUT_CH; i++) { bias_w[i] = bias_b[i]; mult_w[i] = mult_b[i]; shift_w[i] = shift_b[i]; }

            perf_counters_t perf = {0, 0, 0};
            speakable_cnn_top(in, weight_w, bias_w, mult_w, shift_w, out,
                              L.in_h, L.in_w, L.in_c, L.out_c, L.type, L.stride, 1, 0, perf);
            cycles += perf.cycle_count.to_uint();

            unpack_bytes(out, &ofmap_b[0][0][0], FMAP_WORDS);
            const int OH = (L.in_h - 1) / L.stride + 1, OW = (L.in_w - 1) / L.stride + 1;
            const int8_t *exp = spk_tv_expect[tv][l];
            int bad = 0;
            for (int c = 0; c < L.out_c; c++) for (int h = 0; h < OH; h++) for (int w = 0; w < OW; w++)
                if (ofmap_b[c][h][w] != exp[(c * OH + h) * OW + w]) bad++;
            if (bad) { std::cout << "    tv" << tv << " L" << l + 1 << " MISMATCH " << bad << "개" << std::endl; bad_layers++; }

            word_t *t = in; in = out; out = t;
        }
        std::cout << (bad_layers ? "  [FAIL] " : "  [PASS] ") << "모델 tv" << tv
                  << " | L1~L9 하드웨어 출력이 학습 쪽 정수 추론 출력과 " << (bad_layers ? "불일치" : "모두 일치")
                  << " | 연산 사이클 합 " << cycles << std::endl;
        all_ok = all_ok && bad_layers == 0;
    }
    return all_ok;
}

int main() {
    // 큰 레이어 → 작은 레이어 순서로 섞어서 이전 레이어의 온칩 잔여값이 새어 들어오는지도 검사
    const case_t cases[] = {
        { LAYER_CONV3X3, 40, 64, 64, 64, 1, true,  true,  false },   // 최대 채널, 풀링
        { LAYER_CONV3X3, 16, 16,  8,  4, 1, true,  true,  false },
        { LAYER_CONV3X3, 13, 11,  5, 10, 1, false, false, false },   // 8의 배수가 아닌 크기
        { LAYER_CONV3X3, 40, 64,  1, 16, 2, true,  false, false },   // L1 형태 (stride 2, 입력 1채널)
        { LAYER_CONV3X3, 17, 19, 13,  9, 2, false, false, false },   // 홀수 크기 stride 2
        { LAYER_DW3X3,   20, 32, 16,  0, 1, true,  false, false },   // L2 형태
        { LAYER_DW3X3,   20, 32, 32,  0, 2, true,  false, false },   // L4 형태
        { LAYER_DW3X3,    9, 15, 13,  0, 2, false, false, false },   // 채널 수 8의 배수 아님
        { LAYER_PW1X1,   10, 16, 64, 64, 1, true,  false, false },   // L7 형태
        { LAYER_PW1X1,   20, 32, 16, 32, 1, true,  false, false },   // L3 형태
        { LAYER_PW1X1,    7,  9,  5, 11, 1, false, false, false },
        { LAYER_DW3X3,    1,  1,  3,  0, 1, false, false, false },   // 1x1 이미지
        { LAYER_CONV3X3, 12, 10, 11,  7, 1, false, false, true  },   // 반올림 경계 (음수 절반)
    };

    int n_fail = 0, n_case = 0;
    std::cout << "== 레이어 단위 테스트" << std::endl;
    for (const case_t &t : cases) { n_case++; if (!run_case(t)) n_fail++; }
    std::cout << "== 실제 모델 연결 실행 (L1~L9)" << std::endl;
    n_case++;
    if (!run_model()) n_fail++;

    std::cout << (n_fail == 0 ? "TEST PASSED" : "TEST FAILED") << " (" << n_case - n_fail << "/" << n_case << ")" << std::endl;
    return n_fail == 0 ? 0 : 1;
}
