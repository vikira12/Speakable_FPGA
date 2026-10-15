#include <stdio.h>
#include "speakable_model.h"
#include "speakable_fpga.h"
#include "speakable_testvec.h"

// ----------------------------------------------------------------------
// 보드 검증: FPGA 가속 추론(L1~L9 FPGA + GAP/FC ARM)
//  1) 레이어마다 출력이 테스트 벡터와 비트 단위로 같은지
//  2) 같은 입력에서 ARM 단독(spk_cnn_run)과 FPGA 가속(spk_fpga_cnn_run) 시간 비교
// 베어메탈(Vitis 앱)과 Linux(PetaLinux, UIO) 양쪽에서 빌드된다
// ----------------------------------------------------------------------

#ifdef __linux__
#include <time.h>
typedef unsigned long long spk_time_t;
static spk_time_t now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (spk_time_t)t.tv_sec * 1000000000ULL + (spk_time_t)t.tv_nsec;
}
static double elapsed_us(spk_time_t a, spk_time_t b) { return (double)(b - a) / 1e3; }
#define SPK_CNN_BASE 0                                   // Linux는 UIO 이름으로 찾음
#else
#include "xparameters.h"
#include "xiltimer.h"
#include "xil_io.h"
typedef XTime spk_time_t;
static spk_time_t now(void) {
    XTime t;
    XTime_GetTime(&t);
    return t;
}
static double elapsed_us(spk_time_t a, spk_time_t b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }
#define SPK_CNN_BASE XPAR_SPEAKABLE_CNN_0_BASEADDR
#endif

typedef struct { int tv; int bad_layers; int first_bad; } check_ctx_t;

static void check_layer(int layer_idx, const int8_t *out, int size, void *p) {
    check_ctx_t *c = (check_ctx_t *)p;
    const int8_t *ref = spk_tv_expect[c->tv][layer_idx - 1];
    for (int i = 0; i < size; i++)
        if (out[i] != ref[i]) {
            if (c->bad_layers == 0) c->first_bad = layer_idx;
            c->bad_layers++;
            return;
        }
}

int main(void) {
#ifndef __linux__
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);   // JTAG 실행 시 꺼져 있는 글로벌 타이머 켜기
#endif
    const int err = spk_fpga_init(SPK_CNN_BASE);
    if (err != 0) {
        printf("CNN IP init failed (%d)\r\n", err);
        return -1;
    }

    int n_fail = 0;
    int8_t emb_arm[SPK_EMB_DIM], emb_fpga[SPK_EMB_DIM];
    double sum_arm = 0.0, sum_fpga = 0.0;

    for (int tv = 0; tv < SPK_NUM_TV; tv++) {
        check_ctx_t ctx = { tv, 0, 0 };
        spk_fpga_cnn_run(spk_tv_input[tv], emb_fpga, check_layer, &ctx);

        const spk_time_t t0 = now();
        spk_cnn_run(spk_tv_input[tv], emb_arm, 0, 0);
        const spk_time_t t1 = now();
        spk_fpga_cnn_run(spk_tv_input[tv], emb_fpga, 0, 0);
        const spk_time_t t2 = now();

        int same = 1;
        for (int i = 0; i < SPK_EMB_DIM; i++) if (emb_arm[i] != emb_fpga[i]) same = 0;

        const double ta = elapsed_us(t0, t1), tf = elapsed_us(t1, t2);
        sum_arm += ta;
        sum_fpga += tf;
        const int ok = ctx.bad_layers == 0 && same;
        if (ctx.bad_layers)
            printf("  tv%d: 불일치 레이어 %d개 (첫 레이어 L%d)\r\n", tv, ctx.bad_layers, ctx.first_bad);
        printf("%s tv%d | 11개 레이어 %s, 임베딩 ARM=FPGA %s | ARM %.2f ms, FPGA %.2f ms (x%.1f)\r\n",
               ok ? "[PASS]" : "[FAIL]", tv, ctx.bad_layers ? "불일치" : "일치", same ? "일치" : "불일치",
               ta / 1000.0, tf / 1000.0, ta / tf);
        n_fail += ok ? 0 : 1;
    }

    printf("평균 | ARM %.2f ms, FPGA %.2f ms (x%.1f)\r\n",
           sum_arm / SPK_NUM_TV / 1000.0, sum_fpga / SPK_NUM_TV / 1000.0, sum_arm / sum_fpga);
    printf(n_fail ? "SOME FAILED\r\n" : "ALL PASSED\r\n");
    return n_fail ? 1 : 0;
}
