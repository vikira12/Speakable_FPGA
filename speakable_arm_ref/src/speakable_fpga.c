#include "speakable_fpga.h"
#include "xspeakable_cnn_top.h"
#include "xil_cache.h"
#include <string.h>

// IP의 DDR 배치 (speakable_hls_src/speakable_types.h와 같아야 함)
#define MAX_C       64
#define MAX_H       64
#define MAX_W       64
#define OC_SLOT     (MAX_C * 9)          // 출력 채널 하나의 가중치 칸 (576바이트)
#define LAYER_CONV  0
#define LAYER_DW    1
#define LAYER_PW    2

static XSpeakable_cnn_top cnn;

static int8_t  fm[2][MAX_C * MAX_H * MAX_W]     __attribute__((aligned(64)));
static int8_t  wpack[SPK_FPGA_LAYERS][MAX_C * OC_SLOT] __attribute__((aligned(64)));
static int32_t bias[SPK_FPGA_LAYERS][MAX_C]    __attribute__((aligned(64)));
static int32_t mult[SPK_FPGA_LAYERS][MAX_C]    __attribute__((aligned(64)));
static int32_t shift[SPK_FPGA_LAYERS][MAX_C]   __attribute__((aligned(64)));
static int8_t  dense[MAX_C * MAX_H * MAX_W];    // 패딩 배치 → 실제 크기 배치 변환용

static int ip_type(spk_layer_type_t t) {
    return t == SPK_DW3X3 ? LAYER_DW : (t == SPK_PW1X1 ? LAYER_PW : LAYER_CONV);
}

int spk_fpga_init(uintptr_t base_addr) {
    if (XSpeakable_cnn_top_Initialize(&cnn, base_addr) != XST_SUCCESS) return -1;

    memset(wpack, 0, sizeof(wpack));
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        const spk_layer_t *L = &spk_layers[l];
        const int per_oc = L->type == SPK_CONV3X3 ? L->in_c * 9 : (L->type == SPK_DW3X3 ? 9 : L->in_c);
        for (int oc = 0; oc < L->out_c; oc++) {
            memcpy(&wpack[l][oc * OC_SLOT], L->weight + oc * per_oc, per_oc);
            bias[l][oc]  = L->bias[oc];
            mult[l][oc]  = L->mult[oc];
            shift[l][oc] = L->shift[oc];
        }
    }
    memset(fm, 0, sizeof(fm));

    // 가속기는 캐시를 거치지 않고 DDR을 직접 읽으므로 CPU가 쓴 내용을 DDR로 내보냄
    Xil_DCacheFlushRange((UINTPTR)wpack, sizeof(wpack));
    Xil_DCacheFlushRange((UINTPTR)bias, sizeof(bias));
    Xil_DCacheFlushRange((UINTPTR)mult, sizeof(mult));
    Xil_DCacheFlushRange((UINTPTR)shift, sizeof(shift));
    Xil_DCacheFlushRange((UINTPTR)fm, sizeof(fm));
    return 0;
}

// 패딩 배치 [MAX_C][MAX_H][MAX_W] → 실제 크기 [C][H][W]
static void unpad(const int8_t *src, int c, int h, int w, int8_t *dst) {
    for (int i = 0; i < c; i++)
        for (int y = 0; y < h; y++)
            memcpy(&dst[(i * h + y) * w], &src[(i * MAX_H + y) * MAX_W], w);
}

static void run_layer(int l, const int8_t *in, int8_t *out) {
    const spk_layer_t *L = &spk_layers[l];
    XSpeakable_cnn_top_Set_ifmap(&cnn, (u64)(UINTPTR)in);
    XSpeakable_cnn_top_Set_ofmap(&cnn, (u64)(UINTPTR)out);
    XSpeakable_cnn_top_Set_weight(&cnn, (u64)(UINTPTR)wpack[l]);
    XSpeakable_cnn_top_Set_bias(&cnn, (u64)(UINTPTR)bias[l]);
    XSpeakable_cnn_top_Set_mult(&cnn, (u64)(UINTPTR)mult[l]);
    XSpeakable_cnn_top_Set_shift(&cnn, (u64)(UINTPTR)shift[l]);
    XSpeakable_cnn_top_Set_in_h(&cnn, L->in_h);
    XSpeakable_cnn_top_Set_in_w(&cnn, L->in_w);
    XSpeakable_cnn_top_Set_in_ch(&cnn, L->in_c);
    XSpeakable_cnn_top_Set_out_ch(&cnn, L->out_c);
    XSpeakable_cnn_top_Set_layer_type(&cnn, ip_type(L->type));
    XSpeakable_cnn_top_Set_stride(&cnn, L->stride);
    XSpeakable_cnn_top_Set_do_relu(&cnn, L->relu);
    XSpeakable_cnn_top_Set_do_pool(&cnn, 0);
    XSpeakable_cnn_top_Start(&cnn);
    while (!XSpeakable_cnn_top_IsDone(&cnn)) ;
}

void spk_fpga_cnn_run(const int8_t *input, int8_t *embedding, spk_layer_hook_t hook, void *ctx) {
    // 입력 1×40×64를 패딩 배치로 (행 폭이 64라 행 단위 복사)
    for (int y = 0; y < SPK_IN_H; y++) memcpy(&fm[0][y * MAX_W], &input[y * SPK_IN_W], SPK_IN_W);
    Xil_DCacheFlushRange((UINTPTR)fm[0], SPK_IN_H * MAX_W);

    int cur = 0;
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        run_layer(l, fm[cur], fm[1 - cur]);
        cur = 1 - cur;
        if (hook) {
            const spk_layer_t *L = &spk_layers[l];
            Xil_DCacheInvalidateRange((UINTPTR)fm[cur], L->out_c * MAX_H * MAX_W);
            unpad(fm[cur], L->out_c, L->out_h, L->out_w, dense);
            hook(l + 1, dense, L->out_c * L->out_h * L->out_w, ctx);
        }
    }

    // L9 출력(64×5×8) → ARM에서 GAP, FC
    const spk_layer_t *L9 = &spk_layers[SPK_FPGA_LAYERS - 1];
    Xil_DCacheInvalidateRange((UINTPTR)fm[cur], L9->out_c * MAX_H * MAX_W);
    unpad(fm[cur], L9->out_c, L9->out_h, L9->out_w, dense);
    int8_t gap_out[SPK_EMB_DIM];
    spk_run_layer(10, dense, gap_out);
    if (hook) hook(10, gap_out, SPK_EMB_DIM, ctx);
    spk_run_layer(11, gap_out, embedding);
    if (hook) hook(11, embedding, SPK_EMB_DIM, ctx);
}
