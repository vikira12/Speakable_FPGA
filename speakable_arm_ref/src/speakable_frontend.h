#ifndef SPEAKABLE_FRONTEND_H
#define SPEAKABLE_FRONTEND_H

#include <stdint.h>

// ----------------------------------------------------------------------
// 스피커블 음성 전처리: PCM16 mono 16kHz → CNN 입력 INT8 [1][40][64]
//
//  1) /32768 → 고역 통과 필터 (y[n] = x[n] − x[n−1] + a·y[n−1], 발화마다 초기화)
//  2) 발화 전체 RMS를 −20 dBFS로 (±30 dB 제한, 최대 진폭 0.95 이하가 되도록 전체 이득 축소)
//  3) 400샘플 Hann 창, hop 160, FFT 512 → 파워 스펙트럼 → Mel 40 (center 없음, 마지막 불완전 프레임 버림)
//  4) 시간 축을 면적 가중 평균으로 64프레임에 맞춤 → 자연로그 (하한 1e−6)
//  5) Mel 대역별 평균·표준편차 정규화 → float32 → 입력 scale로 양자화 (0에서 먼 방향 반올림, −128~127)
//
// 모든 계산은 double (AI 팀 numpy 구현과 같은 정밀도). 파라미터는 generated/speakable_frontend_params.h
// ----------------------------------------------------------------------

#define SPK_FE_MIN_SAMPLES   8000     // 0.5초
#define SPK_FE_MAX_SAMPLES   48000    // 3초

typedef enum {
    SPK_FE_OK = 0,
    SPK_FE_TOO_SHORT,
    SPK_FE_TOO_LONG
} spk_fe_status_t;

// 단계별 중간값을 받고 싶을 때 (검증용, 필요 없으면 NULL)
typedef struct {
    double log_mel[40 * 64];   // [Mel][시간]
} spk_fe_debug_t;

spk_fe_status_t spk_frontend_run(const int16_t *pcm, int n_samples, int8_t *input, spk_fe_debug_t *dbg);

#endif
