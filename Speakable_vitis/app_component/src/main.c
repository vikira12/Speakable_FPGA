#include <stdio.h>
#include <stdint.h>
#include "xparameters.h"
#include "xspeakable_cnn_top.h"
#include "xil_cache.h"
#include "xiltimer.h"
#include "xil_io.h"

#define MAX_C 64
#define MAX_H 64
#define MAX_W 64

/* DDR 배치는 항상 MAX 크기(패딩 포함). 64바이트 정렬 → 캐시 라인과 맞춤 */
static int8_t  ifmap [MAX_C][MAX_H][MAX_W]   __attribute__((aligned(64)));
static int8_t  weight[MAX_C][MAX_C][3][3]    __attribute__((aligned(64)));
static int32_t bias  [MAX_C]                 __attribute__((aligned(64)));
static int8_t  ofmap [MAX_C][MAX_H][MAX_W]   __attribute__((aligned(64)));
static int8_t  ref   [MAX_C][MAX_H][MAX_W];

static uint32_t seed = 12345;
static uint32_t rng(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }

static int8_t conv_px(int oc, int oh, int ow, int H, int W, int IC, int relu) {
    int32_t acc = bias[oc];
    for (int ic = 0; ic < IC; ic++)
        for (int kh = 0; kh < 3; kh++)
            for (int kw = 0; kw < 3; kw++) {
                int ih = oh + kh - 1, iw = ow + kw - 1;
                if (ih < 0 || ih >= H || iw < 0 || iw >= W) continue;
                acc += ifmap[ic][ih][iw] * weight[oc][ic][kh][kw];
            }
    int32_t s = acc >> 8;
    if (relu && s < 0) s = 0;
    if (s > 127) s = 127;
    if (s < -128) s = -128;
    return (int8_t)s;
}

static void arm_reference(int H, int W, int IC, int OC, int relu, int pool) {
    for (int oc = 0; oc < OC; oc++) {
        if (pool) {
            for (int ph = 0; ph < H / 2; ph++)
                for (int pw = 0; pw < W / 2; pw++) {
                    int8_t m = -128;
                    for (int dy = 0; dy < 2; dy++)
                        for (int dx = 0; dx < 2; dx++) {
                            int8_t v = conv_px(oc, 2*ph+dy, 2*pw+dx, H, W, IC, relu);
                            if (v > m) m = v;
                        }
                    ref[oc][ph][pw] = m;
                }
        } else {
            for (int oh = 0; oh < H; oh++)
                for (int ow = 0; ow < W; ow++)
                    ref[oc][oh][ow] = conv_px(oc, oh, ow, H, W, IC, relu);
        }
    }
}

/* BSP의 COUNTS_PER_SECOND는 괄호 없이 "CPU_FREQ/2"로 정의되어 있어 반드시 괄호로 감싸서 사용 */
static double us(XTime a, XTime b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }

static int run_case(XSpeakable_cnn_top *cnn, int H, int W, int IC, int OC, int relu, int pool) {
    for (int c = 0; c < MAX_C; c++) for (int h = 0; h < MAX_H; h++) for (int w = 0; w < MAX_W; w++)
        ifmap[c][h][w] = (int8_t)(rng() & 0xFF);
    for (int oc = 0; oc < MAX_C; oc++) {
        bias[oc] = (int32_t)(rng() % 4001) - 2000;
        for (int ic = 0; ic < MAX_C; ic++) for (int k = 0; k < 9; k++)
            weight[oc][ic][k / 3][k % 3] = (int8_t)(rng() & 0xFF);
    }

    /* PS가 쓴 데이터를 DDR로 내보냄 (가속기는 캐시를 거치지 않음) */
    Xil_DCacheFlushRange((UINTPTR)ifmap,  sizeof(ifmap));
    Xil_DCacheFlushRange((UINTPTR)weight, sizeof(weight));
    Xil_DCacheFlushRange((UINTPTR)bias,   sizeof(bias));
    Xil_DCacheFlushRange((UINTPTR)ofmap,  sizeof(ofmap));

    XSpeakable_cnn_top_Set_ifmap (cnn, (u64)(UINTPTR)ifmap);
    XSpeakable_cnn_top_Set_weight(cnn, (u64)(UINTPTR)weight);
    XSpeakable_cnn_top_Set_bias  (cnn, (u64)(UINTPTR)bias);
    XSpeakable_cnn_top_Set_ofmap (cnn, (u64)(UINTPTR)ofmap);
    XSpeakable_cnn_top_Set_in_h  (cnn, H);
    XSpeakable_cnn_top_Set_in_w  (cnn, W);
    XSpeakable_cnn_top_Set_in_ch (cnn, IC);
    XSpeakable_cnn_top_Set_out_ch(cnn, OC);
    XSpeakable_cnn_top_Set_do_relu(cnn, relu);
    XSpeakable_cnn_top_Set_do_pool(cnn, pool);

    XTime t0, t1, t2, t3;
    XTime_GetTime(&t0);
    XSpeakable_cnn_top_Start(cnn);
    while (!XSpeakable_cnn_top_IsDone(cnn)) ;
    XTime_GetTime(&t1);

    /* 가속기가 쓴 결과를 캐시 대신 DDR에서 읽도록 */
    Xil_DCacheInvalidateRange((UINTPTR)ofmap, sizeof(ofmap));

    XTime_GetTime(&t2);
    arm_reference(H, W, IC, OC, relu, pool);
    XTime_GetTime(&t3);

    int oh_n = pool ? H / 2 : H, ow_n = pool ? W / 2 : W, err = 0;
    for (int oc = 0; oc < OC; oc++) for (int h = 0; h < oh_n; h++) for (int w = 0; w < ow_n; w++)
        if (ofmap[oc][h][w] != ref[oc][h][w]) {
            if (err < 5) printf("  MISMATCH [%d][%d][%d] hw=%d ref=%d\r\n", oc, h, w, ofmap[oc][h][w], ref[oc][h][w]);
            err++;
        }

    XSpeakable_cnn_top_Perf perf = XSpeakable_cnn_top_Get_perf(cnn);
    printf("%s H=%d W=%d IC=%d OC=%d relu=%d pool=%d | FPGA %.1f us, ARM %.1f us (x%.1f) | cycles=%lu macs=%lu | err=%d\r\n",
           err ? "[FAIL]" : "[PASS]", H, W, IC, OC, relu, pool,
           us(t0, t1), us(t2, t3), us(t2, t3) / us(t0, t1),
           (unsigned long)perf.word_0, (unsigned long)perf.word_1, err);
    return err == 0;
}

int main(void) {
    XSpeakable_cnn_top cnn;

    /* JTAG로 실행하면 FSBL을 거치지 않아 Cortex-A9 글로벌 타이머가 꺼져 있음.
       XTime_GetTime이 이 카운터를 읽으므로 측정 전에 직접 켬 (CTRL 레지스터 bit0) */
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);
    if (XSpeakable_cnn_top_Initialize(&cnn, XPAR_SPEAKABLE_CNN_0_BASEADDR) != XST_SUCCESS) {
        printf("CNN init failed\r\n");
        return -1;
    }
    int ok = 1;
    ok &= run_case(&cnn, 16, 16,  8,  4, 1, 1);
    ok &= run_case(&cnn, 13, 11,  5, 10, 0, 0);
    ok &= run_case(&cnn, 17, 19, 13,  9, 1, 1);
    ok &= run_case(&cnn, 40, 64, 64, 64, 1, 1);
    ok &= run_case(&cnn, 64, 64, 64, 64, 1, 0);
    printf(ok ? "ALL PASSED\r\n" : "SOME FAILED\r\n");
    return 0;
}