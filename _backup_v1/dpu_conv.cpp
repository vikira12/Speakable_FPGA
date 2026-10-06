#include "dpu_conv.h"

// ----------------------------------------------------------------------
// 내부 함수: 3x3 컨볼루션 + (선택) ReLU
// 결과는 conv_buf에 INT8로 저장 (요구 정밀도에 맞춰 shift로 재양자화)
// ----------------------------------------------------------------------
static void conv3x3_relu(
    pixel_t   ifmap[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W],
    weight_t  weight[MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE],
    acc_t     bias[MAX_OUT_CH],
    pixel_t   conv_buf[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    ap_uint<8> in_h, ap_uint<8> in_w, ap_uint<8> in_ch, ap_uint<8> out_ch,
    ap_uint<1> do_relu,
    ap_uint<32> &mac_count,
    ap_uint<32> &cycle_count
) {
    // 입력 채널을 여러 뱅크로 쪼개서 UNROLL_FACTOR만큼 동시에 읽을 수 있게 함
    // (완전 분할하면 BRAM 포트가 부족해지므로 cyclic 분할 사용)
    #pragma HLS ARRAY_PARTITION variable=ifmap  dim=1 cyclic factor=UNROLL_FACTOR
    #pragma HLS ARRAY_PARTITION variable=weight dim=2 cyclic factor=UNROLL_FACTOR

OC_LOOP:
    for (int oc = 0; oc < out_ch; oc++) {
        // out_ch는 런타임 값이라 HLS가 실제 반복 횟수를 모름.
        // 리포트가 현실적인 숫자를 보여주도록 예상 범위 힌트를 줌 (기능에는 무영향)
        #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_OUT_CH avg=4
    OH_LOOP:
        for (int oh = 0; oh < in_h; oh++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_IMG_H avg=16
        OW_LOOP:
            for (int ow = 0; ow < in_w; ow++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=MAX_IMG_W avg=16
                acc_t acc = bias[oc];

            IC_LOOP:
                for (int ic = 0; ic < MAX_IN_CH; ic += UNROLL_FACTOR) {
                    // 채널 UNROLL_FACTOR개 묶음 하나 = 1사이클 (II=1).
                    // OW_LOOP이 아니라 여기에 걸어서, 한 사이클에 동시에 더해야
                    // 하는 곱셈 개수를 UNROLL_FACTOR × KSIZE² = 72개로 제한함
                    #pragma HLS PIPELINE II=1

                    // UNROLL_FACTOR개의 입력 채널을 같은 사이클에 병렬로 누산
                    for (int u = 0; u < UNROLL_FACTOR; u++) {
                        #pragma HLS UNROLL
                        int cur_ic = ic + u;
                        if (cur_ic >= in_ch) continue;

                        for (int kh = 0; kh < KSIZE; kh++) {
                            for (int kw = 0; kw < KSIZE; kw++) {
                                #pragma HLS UNROLL
                                int ih = oh + kh - 1;   // 3x3, padding=1
                                int iw = ow + kw - 1;

                                if (ih >= 0 && ih < in_h && iw >= 0 && iw < in_w) {
                                    acc += ifmap[cur_ic][ih][iw] * weight[oc][cur_ic][kh][kw];
                                    mac_count++;
                                }
                            }
                        }
                    }
                    cycle_count++;  // IC_LOOP 한 바퀴 = 1사이클 (II=1)이므로, 이 반복 횟수가 곧 사이클 수
                }

                // 재양자화: 32비트 누산값을 8비트로 축소 (스케일은 실제 양자화 파라미터에 맞춰 조정 필요)
                acc_t shifted = acc >> 8;
                if (do_relu && shifted < 0) shifted = 0;
                if (shifted > 127)  shifted = 127;
                if (shifted < -128) shifted = -128;

                conv_buf[oc][oh][ow] = (pixel_t)shifted;
            }
        }
    }
}

// ----------------------------------------------------------------------
// 내부 함수: 2x2 MaxPooling, stride 2
// ----------------------------------------------------------------------
static void maxpool2x2(
    pixel_t conv_buf[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    pixel_t ofmap[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    ap_uint<8> in_h, ap_uint<8> in_w, ap_uint<8> out_ch
) {
POOL_OC:
    for (int oc = 0; oc < out_ch; oc++) {
    POOL_H:
        for (int oh = 0; oh < in_h / 2; oh++) {
        POOL_W:
            for (int ow = 0; ow < in_w / 2; ow++) {
                #pragma HLS PIPELINE II=1
                pixel_t p00 = conv_buf[oc][2*oh][2*ow];
                pixel_t p01 = conv_buf[oc][2*oh][2*ow+1];
                pixel_t p10 = conv_buf[oc][2*oh+1][2*ow];
                pixel_t p11 = conv_buf[oc][2*oh+1][2*ow+1];

                pixel_t m0 = (p00 > p01) ? p00 : p01;
                pixel_t m1 = (p10 > p11) ? p10 : p11;
                ofmap[oc][oh][ow] = (m0 > m1) ? m0 : m1;
            }
        }
    }
}

// ----------------------------------------------------------------------
// 내부 함수: 풀링을 쓰지 않을 때 conv_buf를 ofmap으로 그대로 복사
// (DATAFLOW 영역 안에는 함수 호출만 있어야 하므로, if-else 안에 직접
//  루프를 쓰지 않고 이렇게 별도 함수로 분리함)
// ----------------------------------------------------------------------
static void copy_passthrough(
    pixel_t conv_buf[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    pixel_t ofmap[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    ap_uint<8> in_h, ap_uint<8> in_w, ap_uint<8> out_ch
) {
COPY_OC:
    for (int oc = 0; oc < out_ch; oc++)
    COPY_H:
        for (int oh = 0; oh < in_h; oh++)
        COPY_W:
            for (int ow = 0; ow < in_w; ow++) {
                #pragma HLS PIPELINE II=1
                ofmap[oc][oh][ow] = conv_buf[oc][oh][ow];
            }
}

// ----------------------------------------------------------------------
// 최상위 함수 (Vitis HLS가 IP로 합성하는 대상)
// ----------------------------------------------------------------------
void dpu_conv_top(
    pixel_t   ifmap[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W],
    weight_t  weight[MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE],
    acc_t     bias[MAX_OUT_CH],
    pixel_t   conv_scratch[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],  // conv 중간 결과용 DDR 스크래치 공간
    pixel_t   ofmap[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    ap_uint<8>  in_h,
    ap_uint<8>  in_w,
    ap_uint<8>  in_ch,
    ap_uint<8>  out_ch,
    ap_uint<1>  do_relu,
    ap_uint<1>  do_pool,
    perf_counters_t &perf
) {
    // ---- AXI 인터페이스 지정 ----
    // 대용량 데이터(특징맵/가중치/바이어스/출력)는 DDR을 직접 가리키는 AXI4 Master로
    // conv_scratch도 온칩이 아니라 DDR에 두므로, 크기 걱정 없이 MAX 크기 그대로 잡아도 됨
    #pragma HLS INTERFACE m_axi port=ifmap        offset=slave bundle=gmem0 depth=262144
    #pragma HLS INTERFACE m_axi port=weight       offset=slave bundle=gmem1 depth=36864
    #pragma HLS INTERFACE m_axi port=bias         offset=slave bundle=gmem1 depth=64
    #pragma HLS INTERFACE m_axi port=conv_scratch offset=slave bundle=gmem2 depth=262144
    #pragma HLS INTERFACE m_axi port=ofmap        offset=slave bundle=gmem0 depth=262144

    // 제어 레지스터 및 스칼라 파라미터는 AXI4-Lite로 (PS에서 직접 읽고 씀)
    #pragma HLS INTERFACE s_axilite port=in_h    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=in_w    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=in_ch   bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=out_ch  bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=do_relu bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=do_pool bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=perf    bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return  bundle=CTRL   // start/done/idle 핸드셰이크

    // 온칩 conv_buf와 DATAFLOW를 제거함 (레이어가 클 때 온칩 버퍼 자체가 BRAM 예산을 넘어섰기 때문).
    // 이제 conv는 DDR의 conv_scratch에 쓰고, pool/copy는 거기서 다시 읽어 ofmap에 씀 —
    // conv와 pool이 겹쳐 실행되진 않지만(순차 실행), 온칩 메모리는 거의 안 씀

    ap_uint<32> mac_count = 0;
    ap_uint<32> cycle_count = 0;

    conv3x3_relu(ifmap, weight, bias, conv_scratch,
                 in_h, in_w, in_ch, out_ch, do_relu,
                 mac_count, cycle_count);

    if (do_pool) {
        maxpool2x2(conv_scratch, ofmap, in_h, in_w, out_ch);
    } else {
        copy_passthrough(conv_scratch, ofmap, in_h, in_w, out_ch);
    }

    // 성능 카운터를 결과 구조체에 담아 PS로 전달
    // invoke_count 누적은 하드웨어가 아니라 PS(소프트웨어)에서 처리 (아래 설명 참고)
    perf.cycle_count  = cycle_count;
    perf.mac_count    = mac_count;
    perf.invoke_count = 1;
}
