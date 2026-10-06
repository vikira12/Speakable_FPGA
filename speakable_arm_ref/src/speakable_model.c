#include "speakable_model.h"
#include "speakable_weights.h"
#include <math.h>

// 레이어 사이 특징맵 버퍼 (가장 큰 활성값 32×20×32 = 20480바이트)
#define SPK_BUF_BYTES (32 * 20 * 32)
static int8_t buf_a[SPK_BUF_BYTES];
static int8_t buf_b[SPK_BUF_BYTES];

const spk_layer_t spk_layers[SPK_NUM_LAYERS] = {
    // type         in_c in_h in_w  out_c out_h out_w  s  relu  weight  bias   mult      shift
    { SPK_CONV3X3,   1,  40,  64,   16,   20,   32,   2,  1,   L1_w,   L1_b,  L1_mult,  L1_shift },
    { SPK_DW3X3,    16,  20,  32,   16,   20,   32,   1,  1,   L2_w,   L2_b,  L2_mult,  L2_shift },
    { SPK_PW1X1,    16,  20,  32,   32,   20,   32,   1,  1,   L3_w,   L3_b,  L3_mult,  L3_shift },
    { SPK_DW3X3,    32,  20,  32,   32,   10,   16,   2,  1,   L4_w,   L4_b,  L4_mult,  L4_shift },
    { SPK_PW1X1,    32,  10,  16,   64,   10,   16,   1,  1,   L5_w,   L5_b,  L5_mult,  L5_shift },
    { SPK_DW3X3,    64,  10,  16,   64,   10,   16,   1,  1,   L6_w,   L6_b,  L6_mult,  L6_shift },
    { SPK_PW1X1,    64,  10,  16,   64,   10,   16,   1,  1,   L7_w,   L7_b,  L7_mult,  L7_shift },
    { SPK_DW3X3,    64,  10,  16,   64,    5,    8,   2,  1,   L8_w,   L8_b,  L8_mult,  L8_shift },
    { SPK_PW1X1,    64,   5,   8,   64,    5,    8,   1,  1,   L9_w,   L9_b,  L9_mult,  L9_shift },
    { SPK_GAP,      64,   5,   8,   64,    1,    1,   1,  0,   0,      0,     0,        0        },
    { SPK_FC,       64,   1,   1,   64,    1,    1,   1,  0,   L11_w,  L11_b, L11_mult, L11_shift },
};

// ----------------------------------------------------------------------
// 정수 규약
// ----------------------------------------------------------------------

// 오른쪽 시프트 + 반올림(정확히 절반이면 0에서 먼 방향)
static int64_t rshift_round_away(int64_t x, int shift) {
    if (shift <= 0) return x;
    const int64_t half = (int64_t)1 << (shift - 1);
    return (x >= 0) ? ((x + half) >> shift) : -((-x + half) >> shift);
}

static int8_t sat8(int64_t v) {
    if (v > 127)  return 127;
    if (v < -128) return -128;
    return (int8_t)v;
}

// TODO(AI 팀 확정): multiplier·shift의 정확한 정의 (Q31 여부, shift 범위)가 오면 이 함수만 맞추면 됨
// 현재 가정: out = sat8( round_away( acc × mult / 2^shift ) )
int8_t spk_requant(int32_t acc, int32_t mult, int shift) {
    return sat8(rshift_round_away((int64_t)acc * (int64_t)mult, shift));
}

static int8_t finish(int32_t acc, const spk_layer_t *L, int oc) {
    int8_t v = spk_requant(acc, L->mult[oc], L->shift[oc]);
    return (L->relu && v < 0) ? 0 : v;
}

// ----------------------------------------------------------------------
// 레이어 연산
// ----------------------------------------------------------------------

static void conv3x3(const spk_layer_t *L, const int8_t *in, int8_t *out) {
    for (int oc = 0; oc < L->out_c; oc++)
        for (int oh = 0; oh < L->out_h; oh++)
            for (int ow = 0; ow < L->out_w; ow++) {
                int32_t acc = L->bias[oc];
                for (int ic = 0; ic < L->in_c; ic++)
                    for (int kh = 0; kh < 3; kh++) {
                        const int ih = oh * L->stride + kh - 1;
                        if (ih < 0 || ih >= L->in_h) continue;
                        for (int kw = 0; kw < 3; kw++) {
                            const int iw = ow * L->stride + kw - 1;
                            if (iw < 0 || iw >= L->in_w) continue;
                            acc += in[(ic * L->in_h + ih) * L->in_w + iw] *
                                   L->weight[((oc * L->in_c + ic) * 3 + kh) * 3 + kw];
                        }
                    }
                out[(oc * L->out_h + oh) * L->out_w + ow] = finish(acc, L, oc);
            }
}

static void dw3x3(const spk_layer_t *L, const int8_t *in, int8_t *out) {
    for (int c = 0; c < L->out_c; c++)
        for (int oh = 0; oh < L->out_h; oh++)
            for (int ow = 0; ow < L->out_w; ow++) {
                int32_t acc = L->bias[c];
                for (int kh = 0; kh < 3; kh++) {
                    const int ih = oh * L->stride + kh - 1;
                    if (ih < 0 || ih >= L->in_h) continue;
                    for (int kw = 0; kw < 3; kw++) {
                        const int iw = ow * L->stride + kw - 1;
                        if (iw < 0 || iw >= L->in_w) continue;
                        acc += in[(c * L->in_h + ih) * L->in_w + iw] * L->weight[(c * 3 + kh) * 3 + kw];
                    }
                }
                out[(c * L->out_h + oh) * L->out_w + ow] = finish(acc, L, c);
            }
}

// PW1×1과 FC는 같은 연산 (FC는 H = W = 1)
static void pw1x1(const spk_layer_t *L, const int8_t *in, int8_t *out) {
    const int hw = L->in_h * L->in_w;
    for (int oc = 0; oc < L->out_c; oc++)
        for (int p = 0; p < hw; p++) {
            int32_t acc = L->bias[oc];
            for (int ic = 0; ic < L->in_c; ic++)
                acc += in[ic * hw + p] * L->weight[oc * L->in_c + ic];
            out[oc * hw + p] = finish(acc, L, oc);
        }
}

// TODO(AI 팀 확정): GAP의 정수 나눗셈 반올림 규칙. 현재 가정: 합 / (H×W), 0에서 먼 방향 반올림
static void gap(const spk_layer_t *L, const int8_t *in, int8_t *out) {
    const int n = L->in_h * L->in_w;
    for (int c = 0; c < L->in_c; c++) {
        int32_t sum = 0;
        for (int p = 0; p < n; p++) sum += in[c * n + p];
        const int32_t mag = (sum >= 0 ? sum : -sum);
        const int32_t q = (2 * mag + n) / (2 * n);
        out[c] = sat8(sum >= 0 ? q : -q);
    }
}

// ----------------------------------------------------------------------
// 전체 추론
// ----------------------------------------------------------------------
void spk_cnn_run(const int8_t *input, int8_t *embedding, spk_layer_hook_t hook, void *ctx) {
    const int8_t *in = input;
    int8_t *out = buf_a;

    for (int i = 0; i < SPK_NUM_LAYERS; i++) {
        const spk_layer_t *L = &spk_layers[i];
        if (i == SPK_NUM_LAYERS - 1) out = embedding;

        switch (L->type) {
        case SPK_CONV3X3: conv3x3(L, in, out); break;
        case SPK_DW3X3:   dw3x3(L, in, out);   break;
        case SPK_PW1X1:
        case SPK_FC:      pw1x1(L, in, out);   break;
        case SPK_GAP:     gap(L, in, out);     break;
        }

        if (hook) hook(i + 1, out, L->out_c * L->out_h * L->out_w, ctx);

        in  = out;
        out = (out == buf_a) ? buf_b : buf_a;
    }
}

void spk_l2_normalize(const int8_t *embedding, float *out) {
    float norm = 0.0f;
    for (int i = 0; i < SPK_EMB_DIM; i++) norm += (float)embedding[i] * (float)embedding[i];
    norm = sqrtf(norm);
    for (int i = 0; i < SPK_EMB_DIM; i++) out[i] = (norm > 0.0f) ? (float)embedding[i] / norm : 0.0f;
}
