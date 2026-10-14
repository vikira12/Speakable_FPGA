#include <stdio.h>
#include "speakable_model.h"
#include "speakable_fpga.h"
#include "speakable_testvec.h"
#include "xparameters.h"
#include "xiltimer.h"
#include "xil_io.h"

// ----------------------------------------------------------------------
// 보드 검증: FPGA 가속 추론(L1~L9 FPGA + GAP/FC ARM)
//  1) 레이어마다 출력이 테스트 벡터와 비트 단위로 같은지
//  2) 같은 입력에서 ARM 단독(spk_cnn_run)과 FPGA 가속(spk_fpga_cnn_run) 시간 비교
// ----------------------------------------------------------------------

static double elapsed_us(XTime a, XTime b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }

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
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);   // JTAG 실행 시 꺼져 있는 글로벌 타이머 켜기

    if (spk_fpga_init(XPAR_SPEAKABLE_CNN_0_BASEADDR) != 0) {
        printf("CNN IP init failed\r\n");
        return -1;
    }

    int n_fail = 0;
    int8_t emb_arm[SPK_EMB_DIM], emb_fpga[SPK_EMB_DIM];
    double sum_arm = 0.0, sum_fpga = 0.0;

    for (int tv = 0; tv < SPK_NUM_TV; tv++) {
        check_ctx_t ctx = { tv, 0, 0 };
        spk_fpga_cnn_run(spk_tv_input[tv], emb_fpga, check_layer, &ctx);

        XTime t0, t1, t2;
        XTime_GetTime(&t0);
        spk_cnn_run(spk_tv_input[tv], emb_arm, 0, 0);
        XTime_GetTime(&t1);
        spk_fpga_cnn_run(spk_tv_input[tv], emb_fpga, 0, 0);
        XTime_GetTime(&t2);

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
