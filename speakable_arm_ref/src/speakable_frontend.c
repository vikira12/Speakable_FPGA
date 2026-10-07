#include "speakable_frontend.h"
#include "speakable_frontend_params.h"
#include <math.h>

#define FRAME_LEN   400
#define HOP         160
#define NFFT        512
#define NBINS       (NFFT / 2 + 1)
#define NMELS       40
#define OUT_T       64
#define MAX_FRAMES  (1 + (SPK_FE_MAX_SAMPLES - FRAME_LEN) / HOP)

static double sig[SPK_FE_MAX_SAMPLES];
static double mel_raw[NMELS][MAX_FRAMES];

// ----------------------------------------------------------------------
// 512점 복소 FFT (radix-2, 제자리 계산)
// ----------------------------------------------------------------------
static double tw_re[NFFT / 2], tw_im[NFFT / 2];
static int    bitrev[NFFT];
static int    fft_ready = 0;

static void fft_init(void) {
    const double pi = 3.14159265358979323846;
    for (int k = 0; k < NFFT / 2; k++) {
        tw_re[k] = cos(-2.0 * pi * k / NFFT);
        tw_im[k] = sin(-2.0 * pi * k / NFFT);
    }
    for (int i = 0; i < NFFT; i++) {
        int r = 0;
        for (int b = 0; b < 9; b++) r |= ((i >> b) & 1) << (8 - b);
        bitrev[i] = r;
    }
    fft_ready = 1;
}

static void fft512(double *re, double *im) {
    for (int i = 0; i < NFFT; i++) {
        const int j = bitrev[i];
        if (j > i) {
            double t = re[i]; re[i] = re[j]; re[j] = t;
            t = im[i]; im[i] = im[j]; im[j] = t;
        }
    }
    for (int len = 2; len <= NFFT; len <<= 1) {
        const int half = len >> 1, step = NFFT / len;
        for (int s = 0; s < NFFT; s += len)
            for (int k = 0; k < half; k++) {
                const double wr = tw_re[k * step], wi = tw_im[k * step];
                const int a = s + k, b = a + half;
                const double xr = re[b] * wr - im[b] * wi;
                const double xi = re[b] * wi + im[b] * wr;
                re[b] = re[a] - xr;  im[b] = im[a] - xi;
                re[a] += xr;         im[a] += xi;
            }
    }
}

// ----------------------------------------------------------------------
// 단계별 처리
// ----------------------------------------------------------------------
static void hpf_and_gain(const int16_t *pcm, int n) {
    double prev_x = 0.0, prev_y = 0.0, sumsq = 0.0, peak = 0.0;
    for (int i = 0; i < n; i++) {
        const double x = (double)pcm[i] / SPK_FE_PCM_SCALE;
        prev_y = x - prev_x + SPK_FE_HPF_A * prev_y;
        prev_x = x;
        sig[i] = prev_y;
        sumsq += prev_y * prev_y;
        if (fabs(prev_y) > peak) peak = fabs(prev_y);
    }

    double rms = sqrt(sumsq / n);
    if (rms < SPK_FE_GAIN_SILENCE_FLOOR) rms = SPK_FE_GAIN_SILENCE_FLOOR;
    double g_db = SPK_FE_GAIN_TARGET_DBFS - 20.0 * log10(rms);
    if (g_db < SPK_FE_GAIN_MIN_DB) g_db = SPK_FE_GAIN_MIN_DB;
    if (g_db > SPK_FE_GAIN_MAX_DB) g_db = SPK_FE_GAIN_MAX_DB;
    double g = pow(10.0, g_db / 20.0);
    if (peak * g > SPK_FE_GAIN_PEAK_CEIL) g *= SPK_FE_GAIN_PEAK_CEIL / (peak * g);

    for (int i = 0; i < n; i++) sig[i] *= g;
}

static int stft_mel(int n) {
    const int n_frames = 1 + (n - FRAME_LEN) / HOP;
    // 약 10KB라 스택에 두지 않음 (Vitis 베어메탈 기본 스택 8KB)
    static double re[NFFT], im[NFFT], power[NBINS];

    for (int t = 0; t < n_frames; t++) {
        for (int i = 0; i < NFFT; i++) {
            re[i] = (i < FRAME_LEN) ? sig[t * HOP + i] * spk_fe_hann[i] : 0.0;
            im[i] = 0.0;
        }
        fft512(re, im);
        for (int k = 0; k < NBINS; k++) power[k] = re[k] * re[k] + im[k] * im[k];

        for (int m = 0; m < NMELS; m++) {
            const double *w = &spk_fe_mel_w[spk_fe_mel_off[m]];
            double acc = 0.0;
            for (int k = 0; k < spk_fe_mel_len[m]; k++) acc += w[k] * power[spk_fe_mel_start[m] + k];
            mel_raw[m][t] = acc;
        }
    }
    return n_frames;
}

// 출력 프레임 j = 입력 구간 [j·T/64, (j+1)·T/64)와 겹친 넓이로 가중 평균
static void area_resize(int t_in, double out[NMELS][OUT_T]) {
    for (int j = 0; j < OUT_T; j++) {
        const double a = (double)(j * t_in) / OUT_T;
        const double b = (double)((j + 1) * t_in) / OUT_T;
        double acc[NMELS] = { 0 };
        for (int i = (int)floor(a); i < (int)ceil(b); i++) {
            const double w = ((b < i + 1) ? b : i + 1) - ((a > i) ? a : i);
            if (w > 0)
                for (int m = 0; m < NMELS; m++) acc[m] += w * mel_raw[m][i];
        }
        for (int m = 0; m < NMELS; m++) out[m][j] = acc[m] / (b - a);
    }
}

static int8_t quantize(double v) {
    // AI 팀 파이프라인과 같게 정규화 값을 float32로 한 번 거친 뒤 양자화
    const double q = (double)(float)v / SPK_FE_INPUT_SCALE;
    const double r = (q >= 0) ? floor(q + 0.5) : -floor(-q + 0.5);
    if (r > 127)  return 127;
    if (r < -128) return -128;
    return (int8_t)r;
}

spk_fe_status_t spk_frontend_run(const int16_t *pcm, int n_samples, int8_t *input, spk_fe_debug_t *dbg) {
    if (n_samples < SPK_FE_MIN_SAMPLES) return SPK_FE_TOO_SHORT;
    if (n_samples > SPK_FE_MAX_SAMPLES) return SPK_FE_TOO_LONG;
    if (!fft_ready) fft_init();

    hpf_and_gain(pcm, n_samples);
    const int t_in = stft_mel(n_samples);

    static double mel64[NMELS][OUT_T];
    area_resize(t_in, mel64);

    for (int m = 0; m < NMELS; m++) {
        const double sd = (spk_fe_std[m] > SPK_FE_STD_FLOOR) ? spk_fe_std[m] : SPK_FE_STD_FLOOR;
        for (int j = 0; j < OUT_T; j++) {
            const double lm = log((mel64[m][j] > SPK_FE_LOG_FLOOR) ? mel64[m][j] : SPK_FE_LOG_FLOOR);
            if (dbg) dbg->log_mel[m * OUT_T + j] = lm;
            input[m * OUT_T + j] = quantize((lm - spk_fe_mean[m]) / sd);
        }
    }
    return SPK_FE_OK;
}
