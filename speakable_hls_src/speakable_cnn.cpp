#include "speakable_cnn.h"

// ----------------------------------------------------------------------
// 구조 개요
//
//  필요한 데이터를 먼저 온칩 버퍼(BRAM/LUTRAM)에 올려 두고 계산함.
//
//   1) 레이어 시작 시 bias, weight 전체를 온칩에 적재 (최대 36KB)
//   2) 출력 행 TH개 단위(band)로 입력 TH+2행 × 전체 입력 채널을 온칩에 적재
//   3) 출력 채널 TOC개 묶음마다 TIC × TOC = 64 MAC/cycle, II=1로 계산
//   4) 결과를 풀링(또는 그대로 복사)해서 64비트 워드로 묶어 DDR에 기록
//
//  conv 중간 결과는 온칩에만 두고 DDR에는 최종 결과만 기록함.
// ----------------------------------------------------------------------

#define W_BUF_DEPTH   (OC_TILES * IC_GROUPS * KK)            // 576
#define IN_BUF_DEPTH  (IC_GROUPS * BAND_ROWS * ROW_WORDS)     // 640
#define OUT_BUF_DEPTH ((TH / 2) * ROW_WORDS)                  // 32

static pixel_t get_byte(word_t v, int b) {
    #pragma HLS INLINE
    return (pixel_t)v.range(8 * b + 7, 8 * b);
}

// ----------------------------------------------------------------------
// bias 적재: bias_buf[oc % TOC][oc / TOC]
// ----------------------------------------------------------------------
static void load_bias(const acc_t *bias, acc_t bias_buf[TOC][OC_TILES], ap_uint<8> out_ch) {
LOAD_BIAS:
    for (int oc = 0; oc < out_ch; oc++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_OUT_CH avg=32
        bias_buf[oc % TOC][oc / TOC] = bias[oc];
    }
}

// ----------------------------------------------------------------------
// weight 적재
//   DDR: weight[oc][ic][kh][kw] (출력 채널 하나 = 576바이트 = 72워드, 8바이트 정렬)
//   온칩: w_buf[oc % TOC][ic % TIC][((oc / TOC) * IC_GROUPS + ic / TIC) * KK + kh * 3 + kw]
//   → 계산 루프에서 64개 뱅크를 같은 주소로 한 번에 읽을 수 있는 배치
//
//   출력 채널마다 72워드를 버스트로 읽어 임시 버퍼에 두고, 사이클당 1바이트씩 배치함.
//   (한 사이클에 8바이트를 임의 뱅크로 쓰면 64뱅크 × 8 쓰기 포트 디멀티플렉서가 생겨
//    로직이 폭증하므로, 레이어당 한 번뿐인 적재는 1바이트/사이클로 단순하게 처리)
// ----------------------------------------------------------------------
static void load_weights(const word_t *weight, weight_t w_buf[TOC][TIC][W_BUF_DEPTH],
                         ap_uint<8> in_ch, ap_uint<8> out_ch) {
    const int n_bytes = in_ch * KK;
    const int n_words = (n_bytes + WORD_BYTES - 1) / WORD_BYTES;

    word_t oc_words[OC_WEIGHT_WORDS];

LOAD_W_OC:
    for (int oc = 0; oc < out_ch; oc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_OUT_CH avg=32
    LOAD_W_WORD:
        for (int wd = 0; wd < n_words; wd++) {
            #pragma HLS PIPELINE II=1
            #pragma HLS LOOP_TRIPCOUNT min=2 max=OC_WEIGHT_WORDS avg=36
            oc_words[wd] = weight[oc * OC_WEIGHT_WORDS + wd];
        }

        // 바이트 j → (ic, k) 를 나눗셈 없이 카운터로 추적
        const int base = (oc / TOC) * IC_GROUPS * KK;
        int ic = 0, k = 0;
    LOAD_W_BYTE:
        for (int j = 0; j < n_bytes; j++) {
            #pragma HLS PIPELINE II=1
            #pragma HLS LOOP_TRIPCOUNT min=9 max=MAX_IN_CH*KK avg=288
            w_buf[oc % TOC][ic % TIC][base + (ic / TIC) * KK + k] = get_byte(oc_words[j >> 3], j & 7);
            if (k == KK - 1) { k = 0; ic++; }
            else             { k++; }
        }
    }
}

// ----------------------------------------------------------------------
// 입력 band 적재: 출력 행 oh0 ~ oh0+TH-1 계산에 필요한 입력 행 oh0-1 ~ oh0+TH
//   in_buf[c % TIC][col % 8][((c / TIC) * BAND_ROWS + r) * ROW_WORDS + col / 8]
//   이미지 바깥 행(위/아래 패딩)은 0으로 채움
// ----------------------------------------------------------------------
static void load_band(const word_t *ifmap, pixel_t in_buf[TIC][WORD_BYTES][IN_BUF_DEPTH],
                      int oh0, ap_uint<8> in_h, ap_uint<8> in_w, ap_uint<8> in_ch) {
    const int n_words = (in_w + WORD_BYTES - 1) / WORD_BYTES;

LOAD_IN_CH:
    for (int c = 0; c < in_ch; c++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_IN_CH avg=32
    LOAD_IN_ROW:
        for (int r = 0; r < BAND_ROWS; r++) {
            int ih   = oh0 - 1 + r;
            int base = ((c / TIC) * BAND_ROWS + r) * ROW_WORDS;
            if (ih >= 0 && ih < in_h) {
            LOAD_IN_WORD:
                for (int wd = 0; wd < n_words; wd++) {
                    #pragma HLS PIPELINE II=1
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=ROW_WORDS avg=4
                    word_t v = ifmap[(c * MAX_IMG_H + ih) * ROW_WORDS + wd];
                    for (int b = 0; b < WORD_BYTES; b++) {
                        #pragma HLS UNROLL
                        in_buf[c % TIC][b][base + wd] = get_byte(v, b);
                    }
                }
            } else {
            ZERO_IN_WORD:
                for (int wd = 0; wd < n_words; wd++) {
                    #pragma HLS PIPELINE II=1
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=ROW_WORDS avg=4
                    for (int b = 0; b < WORD_BYTES; b++) {
                        #pragma HLS UNROLL
                        in_buf[c % TIC][b][base + wd] = 0;
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
// 출력 채널 타일 ot(TOC개)에 대해 band 안의 rows × in_w 픽셀 계산
//   (r, ow, g, kh, kw) 5중 루프를 하나로 펼쳐서 II=1 파이프라인 한 개로 돌림
//   → 픽셀마다 파이프라인을 다시 채우는 오버헤드가 없음
//   매 사이클: 입력 TIC개 읽기 × 가중치 TOC×TIC개 → 64 MAC, TOC개 누산기 갱신
// ----------------------------------------------------------------------
static void compute_tile(pixel_t in_buf[TIC][WORD_BYTES][IN_BUF_DEPTH],
                         weight_t w_buf[TOC][TIC][W_BUF_DEPTH],
                         acc_t bias_buf[TOC][OC_TILES],
                         pixel_t out_buf[TOC][2][WORD_BYTES][OUT_BUF_DEPTH],
                         int ot, int rows, ap_uint<8> in_w, ap_uint<8> in_ch,
                         ap_uint<1> do_relu, ap_uint<32> &iter_count) {
    // 자동 식 재배열을 끄고 아래의 명시적 덧셈 트리를 그대로 쓰게 함.
    // (켜 두면 HLS가 acc를 트리 맨 앞에 붙여 누산 루프 경로가 9.3ns로 길어짐)
    #pragma HLS EXPRESSION_BALANCE off
    static_assert(TIC == 8, "아래 덧셈 트리는 TIC == 8 기준으로 작성됨");

    const int n_grp = (in_ch + TIC - 1) / TIC;
    const int total = rows * in_w * n_grp * KK;

    acc_t acc[TOC];
    #pragma HLS ARRAY_PARTITION variable=acc complete

    int r = 0, ow = 0, g = 0, kh = 0, kw = 0;

COMPUTE:
    for (int it = 0; it < total; it++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=9 max=36864 avg=9216
        const bool first = (g == 0 && kh == 0 && kw == 0);
        const bool last  = (g == n_grp - 1 && kh == KSIZE - 1 && kw == KSIZE - 1);

        // 좌우 패딩: 이미지 바깥 열은 0 (위아래는 load_band에서 0으로 채워 둠)
        const int  col    = ow + kw - 1;
        const bool col_ok = (col >= 0) && (col < in_w);
        const int  cc     = col_ok ? col : 0;
        const int  addr   = (g * BAND_ROWS + r + kh) * ROW_WORDS + (cc >> 3);

        pixel_t x[TIC];
        #pragma HLS ARRAY_PARTITION variable=x complete
        for (int tic = 0; tic < TIC; tic++) {
            #pragma HLS UNROLL
            // in_ch가 TIC의 배수가 아니면 마지막 묶음의 남는 채널은 0으로 (이전 레이어 잔여값 차단)
            const bool ch_ok = (g * TIC + tic) < in_ch;
            x[tic] = (col_ok && ch_ok) ? in_buf[tic][cc & 7][addr] : (pixel_t)0;
        }

        const int widx = (ot * IC_GROUPS + g) * KK + kh * KSIZE + kw;
        for (int toc = 0; toc < TOC; toc++) {
            #pragma HLS UNROLL
            // 8개 곱(16비트)을 3단 덧셈 트리로 합산 (최대 |8 × 128 × 128| < 2^18 → 20비트면 충분)
            // 트리는 누산 루프 밖에서 파이프라인되고, 루프를 도는 경로는 마지막 덧셈 하나뿐
            ap_int<20> p[TIC];
            #pragma HLS ARRAY_PARTITION variable=p complete
            for (int tic = 0; tic < TIC; tic++) {
                #pragma HLS UNROLL
                p[tic] = x[tic] * w_buf[toc][tic][widx];
            }
            const ap_int<20> s01 = p[0] + p[1], s23 = p[2] + p[3];
            const ap_int<20> s45 = p[4] + p[5], s67 = p[6] + p[7];
            const ap_int<20> s03 = s01 + s23,  s47 = s45 + s67;
            const ap_int<20> sum = s03 + s47;

            const acc_t next = (first ? bias_buf[toc][ot] : acc[toc]) + sum;
            acc[toc] = next;

            if (last) {
                // 재양자화: 32비트 누산값을 8비트로 축소 (스케일은 실제 양자화 파라미터에 맞춰 조정 필요)
                acc_t shifted = next >> 8;
                if (do_relu && shifted < 0) shifted = 0;
                if (shifted > 127)  shifted = 127;
                if (shifted < -128) shifted = -128;
                out_buf[toc][r & 1][ow & 7][(r >> 1) * ROW_WORDS + (ow >> 3)] = (pixel_t)shifted;
            }
        }

        // 펼친 루프 인덱스 갱신 (kw → kh → g → ow → r 순)
        if (kw == KSIZE - 1) {
            kw = 0;
            if (kh == KSIZE - 1) {
                kh = 0;
                if (g == n_grp - 1) {
                    g = 0;
                    if (ow == in_w - 1) { ow = 0; r++; }
                    else                { ow++; }
                } else {
                    g++;
                }
            } else {
                kh++;
            }
        } else {
            kw++;
        }
    }

    iter_count += total;
}

// ----------------------------------------------------------------------
// 2x2 MaxPooling(stride 2) 또는 그대로 복사해서 DDR ofmap에 기록
//   한 워드(8픽셀)씩 묶어서 쓰고, 유효 폭을 넘는 바이트는 0으로 채움
//   (행 패딩 영역 안에서만 쓰므로 다른 행/채널을 덮어쓰지 않음)
// ----------------------------------------------------------------------
static void store_tile(pixel_t out_buf[TOC][2][WORD_BYTES][OUT_BUF_DEPTH], word_t *ofmap,
                       int ot, int oh0, int rows, ap_uint<8> in_w, ap_uint<8> out_ch,
                       ap_uint<1> do_pool) {
    const int n_oc = (out_ch - ot * TOC < TOC) ? (int)(out_ch - ot * TOC) : TOC;

    if (do_pool) {
        // (toc, pr, px) 3중 루프를 펼쳐서 사이클당 출력 1픽셀 처리.
        // 2x2 창의 4개 값은 행 홀짝 × 열 하위 비트로 서로 다른 뱅크에 있어 한 사이클에 읽힘.
        // 8픽셀이 모이면 64비트 워드 하나로 기록
        const int pw      = in_w / 2;
        const int p_rows  = rows / 2;
        const int px_n    = ((pw + WORD_BYTES - 1) / WORD_BYTES) * WORD_BYTES;
        const int total   = n_oc * p_rows * px_n;

        int toc = 0, pr = 0, px = 0;
        word_t v = 0;
    POOL:
        for (int it = 0; it < total; it++) {
            #pragma HLS PIPELINE II=1
            #pragma HLS LOOP_TRIPCOUNT min=0 max=TOC*(TH/2)*(MAX_IMG_W/2) avg=512
            const int c0   = 2 * px;                         // 짝수 열 → c0, c0+1은 같은 워드
            const int addr = pr * ROW_WORDS + (c0 >> 3);
            pixel_t p00 = out_buf[toc][0][c0 & 7][addr];
            pixel_t p01 = out_buf[toc][0][(c0 + 1) & 7][addr];
            pixel_t p10 = out_buf[toc][1][c0 & 7][addr];
            pixel_t p11 = out_buf[toc][1][(c0 + 1) & 7][addr];
            pixel_t m0 = (p00 > p01) ? p00 : p01;
            pixel_t m1 = (p10 > p11) ? p10 : p11;
            pixel_t m  = (m0 > m1) ? m0 : m1;

            // 리틀엔디언: 먼저 들어온 픽셀이 하위 바이트로 가도록 오른쪽으로 밀어 넣음
            v = v >> 8;
            v.range(63, 56) = (px < pw) ? m : (pixel_t)0;
            if ((px & 7) == 7)
                ofmap[((ot * TOC + toc) * MAX_IMG_H + oh0 / 2 + pr) * ROW_WORDS + (px >> 3)] = v;

            if (px == px_n - 1) {
                px = 0;
                if (pr == p_rows - 1) { pr = 0; toc++; }
                else                  { pr++; }
            } else {
                px++;
            }
        }
    } else {
        const int n_words = (in_w + WORD_BYTES - 1) / WORD_BYTES;
    COPY_OC:
        for (int toc = 0; toc < n_oc; toc++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=TOC avg=8
        COPY_H:
            for (int r = 0; r < rows; r++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=TH avg=8
            COPY_W:
                for (int wd = 0; wd < n_words; wd++) {
                    #pragma HLS PIPELINE II=1
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=ROW_WORDS avg=4
                    word_t v = 0;
                    for (int b = 0; b < WORD_BYTES; b++) {
                        #pragma HLS UNROLL
                        const int px = wd * WORD_BYTES + b;
                        pixel_t p = out_buf[toc][r & 1][b][(r >> 1) * ROW_WORDS + wd];
                        v.range(8 * b + 7, 8 * b) = (px < in_w) ? p : (pixel_t)0;
                    }
                    ofmap[((ot * TOC + toc) * MAX_IMG_H + oh0 + r) * ROW_WORDS + wd] = v;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
// 최상위 함수 (Vitis HLS가 IP로 합성하는 대상)
// ----------------------------------------------------------------------
void speakable_cnn_top(
    const word_t *ifmap,
    const word_t *weight,
    const acc_t  *bias,
    word_t       *ofmap,
    ap_uint<8>  in_h,
    ap_uint<8>  in_w,
    ap_uint<8>  in_ch,
    ap_uint<8>  out_ch,
    ap_uint<1>  do_relu,
    ap_uint<1>  do_pool,
    perf_counters_t &perf
) {
    // ---- AXI 인터페이스 지정 ----
    // 특징맵은 gmem0, 파라미터(weight/bias)는 gmem1. 둘 다 HP 포트에 붙일 64비트 AXI4 Master
    // (포트 인자에 ARRAY_PARTITION을 걸면 마스터가 여러 개로 쪼개지므로 걸지 않음)
    #pragma HLS INTERFACE m_axi port=ifmap  offset=slave bundle=gmem0 depth=FMAP_WORDS   max_read_burst_length=16
    #pragma HLS INTERFACE m_axi port=ofmap  offset=slave bundle=gmem0 depth=FMAP_WORDS   max_write_burst_length=16
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem1 depth=WEIGHT_WORDS max_read_burst_length=16
    #pragma HLS INTERFACE m_axi port=bias   offset=slave bundle=gmem1 depth=MAX_OUT_CH   max_read_burst_length=16

    // 버퍼 주소, 제어 레지스터, 스칼라 파라미터를 AXI4-Lite 하나(CTRL)로 통합
    #pragma HLS INTERFACE s_axilite port=ifmap   bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=weight  bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=bias    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=ofmap   bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=in_h    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=in_w    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=in_ch   bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=out_ch  bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=do_relu bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=do_pool bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=perf    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return  bundle=CTRL   // start/done/idle 핸드셰이크

    // ---- 온칩 버퍼 ----
    // 가중치: 64뱅크(TOC×TIC) × 576바이트 → 한 사이클에 64개 동시 읽기
    static weight_t w_buf[TOC][TIC][W_BUF_DEPTH];
    #pragma HLS ARRAY_PARTITION variable=w_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=w_buf dim=2 complete

    // 입력 band: 채널(TIC) × 열 하위 3비트(8) = 64뱅크 × 640바이트
    // 열 방향으로도 나눠서 DDR 워드 하나(8바이트)를 한 사이클에 적재할 수 있게 함
    static pixel_t in_buf[TIC][WORD_BYTES][IN_BUF_DEPTH];
    #pragma HLS ARRAY_PARTITION variable=in_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=in_buf dim=2 complete

    // conv 결과 (출력 채널 TOC개 × TH행): 풀링/복사 전 임시 저장
    // [toc][행 홀짝][열 % 8][(행 / 2) * 8 + 열 / 8] → 128뱅크 × 32바이트, LUTRAM으로 고정
    // (레지스터로 풀리면 가변 인덱스 멀티플렉서 때문에 LUT/FF가 폭증함)
    static pixel_t out_buf[TOC][2][WORD_BYTES][OUT_BUF_DEPTH];
    #pragma HLS ARRAY_PARTITION variable=out_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=out_buf dim=2 complete
    #pragma HLS ARRAY_PARTITION variable=out_buf dim=3 complete
    #pragma HLS BIND_STORAGE variable=out_buf type=ram_2p impl=lutram

    static acc_t bias_buf[TOC][OC_TILES];
    #pragma HLS ARRAY_PARTITION variable=bias_buf dim=1 complete

    ap_uint<32> iter_count = 0;

    load_bias(bias, bias_buf, out_ch);
    load_weights(weight, w_buf, in_ch, out_ch);

    const int n_ot = (out_ch + TOC - 1) / TOC;

BAND_LOOP:
    for (int oh0 = 0; oh0 < in_h; oh0 += TH) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_IMG_H/TH avg=4
        const int rows = (in_h - oh0 < TH) ? (int)(in_h - oh0) : TH;

        load_band(ifmap, in_buf, oh0, in_h, in_w, in_ch);

    OC_TILE_LOOP:
        for (int ot = 0; ot < n_ot; ot++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=OC_TILES avg=4
            compute_tile(in_buf, w_buf, bias_buf, out_buf, ot, rows, in_w, in_ch, do_relu, iter_count);
            store_tile(out_buf, ofmap, ot, oh0, rows, in_w, out_ch, do_pool);
        }
    }

    // 성능 카운터를 결과 구조체에 담아 PS로 전달
    // mac_count: 3x3/padding=1에서 한 축의 유효 탭 수 합 = 3N-2 이므로 닫힌 식으로 계산
    // invoke_count 누적은 하드웨어가 아니라 PS(소프트웨어)에서 처리
    ap_uint<32> taps_h = 3 * (ap_uint<32>)in_h - 2;
    ap_uint<32> taps_w = 3 * (ap_uint<32>)in_w - 2;
    perf.cycle_count  = iter_count;
    perf.mac_count    = (ap_uint<32>)in_ch * out_ch * taps_h * taps_w;
    perf.invoke_count = 1;
}
