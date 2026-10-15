#include <stdio.h>
#include <string.h>
#include <math.h>
#include "speakable_frontend.h"
#include "speakable_model.h"
#include "speakable_frontend_testvec.h"

// ----------------------------------------------------------------------
// 전처리 검증: audio.wav(PCM) → 입력 INT8이 참조 전처리 결과와 같은지, 이어서 임베딩까지 실행
//   PC: gcc로 빌드해서 실행
//   보드: SPK_ON_BOARD 정의 시 FPGA 가속 CNN도 실행해 임베딩이 ARM 단독과 같은지 확인하고,
//         단계별 시간(전처리, CNN ARM·FPGA, 기기 경로 = 전처리 + FPGA CNN)을 측정
// ----------------------------------------------------------------------

#ifdef SPK_ON_BOARD
#include "speakable_fpga.h"
#include "xiltimer.h"
#include "xil_io.h"
#include "xparameters.h"
static double elapsed_us(XTime a, XTime b) { return (double)(b - a) * 1e6 / (double)(COUNTS_PER_SECOND); }
#endif

static spk_fe_debug_t dbg;

int main(void) {
#ifdef SPK_ON_BOARD
    Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08U, 0x1U);   // JTAG 실행 시 꺼져 있는 글로벌 타이머 켜기
    const int err = spk_fpga_init(XPAR_SPEAKABLE_CNN_0_BASEADDR);
    if (err != 0) {
        printf("CNN IP init failed (%d)\r\n", err);
        return -1;
    }
    int8_t emb_fpga[SPK_EMB_DIM];
    double sum_fe = 0.0, sum_fpga = 0.0, sum_sec = 0.0;
#endif
    int n_fail = 0;
    int8_t input[40 * 64], emb[SPK_EMB_DIM];

    for (int k = 0; k < SPK_FE_NUM_TV; k++) {
#ifdef SPK_ON_BOARD
        XTime t0, t1, t2, t3;
        XTime_GetTime(&t0);
#endif
        const spk_fe_status_t st = spk_frontend_run(spk_fe_tv_pcm[k], spk_fe_tv_len[k], input, &dbg);
#ifdef SPK_ON_BOARD
        XTime_GetTime(&t1);
#endif
        spk_cnn_run(input, emb, 0, 0);
#ifdef SPK_ON_BOARD
        XTime_GetTime(&t2);
        spk_fpga_cnn_run(input, emb_fpga, 0, 0);
        XTime_GetTime(&t3);
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
        const int emb_same = memcmp(emb, emb_fpga, SPK_EMB_DIM) == 0;
        const double t_fe = elapsed_us(t0, t1), t_arm = elapsed_us(t1, t2), t_fpga = elapsed_us(t2, t3);
        sum_fe += t_fe;
        sum_fpga += t_fpga;
        sum_sec += spk_fe_tv_len[k] / 16000.0;
        printf("%s tv%d | %d샘플(%.2f s) | 입력 INT8 불일치 %d/2560 | log_mel 최대 오차 %.2e | 임베딩 ARM=FPGA %s\r\n"
               "      전처리 %.1f ms, CNN ARM %.1f ms / FPGA %.1f ms, 기기 경로(전처리+FPGA) %.1f ms\r\n",
               (bad || !emb_same) ? "[FAIL]" : "[PASS]", k, spk_fe_tv_len[k], spk_fe_tv_len[k] / 16000.0, bad, max_err,
               emb_same ? "일치" : "불일치", t_fe / 1000.0, t_arm / 1000.0, t_fpga / 1000.0, (t_fe + t_fpga) / 1000.0);
        n_fail += (bad || !emb_same) ? 1 : 0;
#else
        printf("%s tv%d | %d샘플 | 입력 INT8 불일치 %d/2560 | log_mel 최대 오차 %.2e\r\n",
               bad ? "[FAIL]" : "[PASS]", k, spk_fe_tv_len[k], bad, max_err);
        n_fail += bad ? 1 : 0;
#endif
    }

#ifdef SPK_ON_BOARD
    printf("평균 | 전처리 %.1f ms (음성 1초당 %.1f ms), FPGA CNN %.1f ms, 기기 경로 %.1f ms\r\n",
           sum_fe / SPK_FE_NUM_TV / 1000.0, sum_fe / sum_sec / 1000.0, sum_fpga / SPK_FE_NUM_TV / 1000.0,
           (sum_fe + sum_fpga) / SPK_FE_NUM_TV / 1000.0);
#endif
    printf(n_fail ? "SOME FAILED\r\n" : "ALL PASSED\r\n");
    return n_fail ? 1 : 0;
}
