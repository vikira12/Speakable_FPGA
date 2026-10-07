#include <stdio.h>
#include <math.h>
#include "speakable_frontend.h"
#include "speakable_model.h"
#include "speakable_frontend_testvec.h"

// ----------------------------------------------------------------------
// 전처리 검증: audio.wav(PCM) → 입력 INT8이 AI 팀 결과와 같은지, 이어서 임베딩까지 실행
//   PC: gcc로 빌드해서 실행 / 보드: SPK_ON_BOARD 정의 시 단계별 시간도 측정
// ----------------------------------------------------------------------

#ifdef SPK_ON_BOARD
#include "xiltimer.h"
#include "xil_io.h"
#include "xparameters.h"
static double elapsed_us(XTime a, XTime b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }
#endif

static spk_fe_debug_t dbg;

int main(void) {
#ifdef SPK_ON_BOARD
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);   // JTAG 실행 시 꺼져 있는 글로벌 타이머 켜기
#endif
    int n_fail = 0;
    int8_t input[40 * 64], emb[SPK_EMB_DIM];

    for (int k = 0; k < SPK_FE_NUM_TV; k++) {
#ifdef SPK_ON_BOARD
        XTime t0, t1, t2;
        XTime_GetTime(&t0);
#endif
        const spk_fe_status_t st = spk_frontend_run(spk_fe_tv_pcm[k], spk_fe_tv_len[k], input, &dbg);
#ifdef SPK_ON_BOARD
        XTime_GetTime(&t1);
#endif
        spk_cnn_run(input, emb, 0, 0);
#ifdef SPK_ON_BOARD
        XTime_GetTime(&t2);
#endif
        if (st != SPK_FE_OK) {
            printf("[FAIL] tv%d 전처리 상태 %d\r\n", k, (int)st);
            n_fail++;
            continue;
        }

        int bad = 0;
        double max_err = 0.0;
        for (int i = 0; i < 40 * 64; i++) {
            if (input[i] != spk_fe_tv_input[k][i]) bad++;
            const double e = fabs(dbg.log_mel[i] - spk_fe_tv_log_mel[k][i]);
            if (e > max_err) max_err = e;
        }
#ifdef SPK_ON_BOARD
        printf("%s tv%d | %d샘플 | 입력 INT8 불일치 %d/2560 | log_mel 최대 오차 %.2e | 전처리 %.1f ms, CNN %.1f ms\r\n",
               bad ? "[FAIL]" : "[PASS]", k, spk_fe_tv_len[k], bad, max_err,
               elapsed_us(t0, t1) / 1000.0, elapsed_us(t1, t2) / 1000.0);
#else
        printf("%s tv%d | %d샘플 | 입력 INT8 불일치 %d/2560 | log_mel 최대 오차 %.2e\r\n",
               bad ? "[FAIL]" : "[PASS]", k, spk_fe_tv_len[k], bad, max_err);
#endif
        n_fail += bad ? 1 : 0;
    }

    printf(n_fail ? "SOME FAILED\r\n" : "ALL PASSED\r\n");
    return n_fail ? 1 : 0;
}
