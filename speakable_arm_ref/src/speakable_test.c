#include <stdio.h>
#include "speakable_model.h"
#include "speakable_testvec.h"

// ----------------------------------------------------------------------
// 테스트 벡터 검증: 레이어마다 출력이 참조값(PyTorch/Python 정수 추론)과 비트 단위로 같은지 확인
//   PC: gcc로 빌드해서 실행
//   보드: Vitis 앱에 src/, generated/ 파일을 넣고 SPK_ON_BOARD를 정의하면 추론 시간도 측정
// ----------------------------------------------------------------------

#ifdef SPK_ON_BOARD
#include "xiltimer.h"
#include "xil_io.h"
#include "xparameters.h"
static double elapsed_us(XTime a, XTime b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }
#endif

typedef struct {
    int tv;
    int mismatch[SPK_NUM_LAYERS + 1];
    int first_idx[SPK_NUM_LAYERS + 1];
    int got[SPK_NUM_LAYERS + 1];
    int exp[SPK_NUM_LAYERS + 1];
} check_ctx_t;

static void check_layer(int layer_idx, const int8_t *out, int size, void *p) {
    check_ctx_t *c = (check_ctx_t *)p;
    const int8_t *ref = spk_tv_expect[c->tv][layer_idx - 1];
    c->mismatch[layer_idx] = 0;
    for (int i = 0; i < size; i++)
        if (out[i] != ref[i]) {
            if (c->mismatch[layer_idx] == 0) {
                c->first_idx[layer_idx] = i;
                c->got[layer_idx] = out[i];
                c->exp[layer_idx] = ref[i];
            }
            c->mismatch[layer_idx]++;
        }
}

int main(void) {
#ifdef SPK_ON_BOARD
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);   // JTAG 실행 시 꺼져 있는 글로벌 타이머 켜기
#endif
    int n_fail = 0;
    int8_t emb[SPK_EMB_DIM];

    // 재양자화 경계값 (정확히 절반·음수·포화): 실제 테스트 벡터에선 거의 나오지 않아 따로 검사
    int rq_fail = 0;
    for (int i = 0; i < SPK_NUM_RQ_CASES; i++) {
        const int8_t got = spk_requant(spk_rq_cases[i].acc, spk_rq_cases[i].mult, spk_rq_cases[i].shift);
        if (got != spk_rq_cases[i].expect) {
            if (rq_fail < 5)
                printf("  requant(%ld, %ld, %d) = %d, 기대값 %d\r\n", (long)spk_rq_cases[i].acc,
                       (long)spk_rq_cases[i].mult, spk_rq_cases[i].shift, got, spk_rq_cases[i].expect);
            rq_fail++;
        }
    }
    printf("%s 재양자화 경계값 %d개\r\n", rq_fail ? "[FAIL]" : "[PASS]", SPK_NUM_RQ_CASES);
    n_fail += rq_fail ? 1 : 0;

    for (int tv = 0; tv < SPK_NUM_TV; tv++) {
        check_ctx_t ctx = { .tv = tv };
        spk_cnn_run(spk_tv_input[tv], emb, check_layer, &ctx);

        int tv_fail = 0;
        for (int l = 1; l <= SPK_NUM_LAYERS; l++) {
            if (ctx.mismatch[l]) {
                printf("  tv%d L%-2d MISMATCH %d개 (첫 위치 %d: got=%d exp=%d)\r\n",
                       tv, l, ctx.mismatch[l], ctx.first_idx[l], ctx.got[l], ctx.exp[l]);
                tv_fail = 1;
            }
        }

#ifdef SPK_ON_BOARD
        XTime t0, t1;
        XTime_GetTime(&t0);
        spk_cnn_run(spk_tv_input[tv], emb, 0, 0);
        XTime_GetTime(&t1);
        printf("%s tv%d | %s | ARM 추론 %.1f us\r\n",
               tv_fail ? "[FAIL]" : "[PASS]", tv, tv_fail ? "불일치 레이어 있음" : "모든 레이어 일치",
               elapsed_us(t0, t1));
#else
        printf("%s tv%d | %s\r\n",
               tv_fail ? "[FAIL]" : "[PASS]", tv, tv_fail ? "불일치 레이어 있음" : "모든 레이어 일치");
#endif
        n_fail += tv_fail;
    }

    printf(n_fail ? "SOME FAILED\r\n" : "ALL PASSED\r\n");
    return n_fail ? 1 : 0;
}
