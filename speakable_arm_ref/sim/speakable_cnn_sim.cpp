// PC 시뮬레이션용 드라이버 구현: 레지스터 값을 모았다가 Start에서 HLS C 모델을 호출
// 빌드: speakable_hls_src/speakable_cnn.cpp와 함께 g++ (Vitis HLS의 ap_int 헤더 필요). 사용법은 README
#include "xspeakable_cnn_top.h"
#include "speakable_cnn.h"

static struct {
    u64 ifmap, weight, bias, mult, shift, ofmap;
    u32 in_h, in_w, in_ch, out_ch, layer_type, stride, do_relu, do_pool;
    int done;
} reg;

static perf_counters_t perf;

extern "C" {

int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *p, UINTPTR base) {
    p->Ctrl_BaseAddress = base;
    p->IsReady = 1;
    return XST_SUCCESS;
}

void XSpeakable_cnn_top_Start(XSpeakable_cnn_top *) {
    speakable_cnn_top((const word_t *)(UINTPTR)reg.ifmap, (const word_t *)(UINTPTR)reg.weight,
                      (const acc_t *)(UINTPTR)reg.bias, (const acc_t *)(UINTPTR)reg.mult,
                      (const acc_t *)(UINTPTR)reg.shift, (word_t *)(UINTPTR)reg.ofmap,
                      reg.in_h, reg.in_w, reg.in_ch, reg.out_ch, reg.layer_type, reg.stride,
                      reg.do_relu, reg.do_pool, perf);
    reg.done = 1;
}

u32 XSpeakable_cnn_top_IsDone(XSpeakable_cnn_top *) {
    const u32 d = reg.done;                              // 실제 IP처럼 읽으면 지워짐
    reg.done = 0;
    return d;
}

void XSpeakable_cnn_top_Set_ifmap(XSpeakable_cnn_top *, u64 d) { reg.ifmap = d; }
void XSpeakable_cnn_top_Set_weight(XSpeakable_cnn_top *, u64 d) { reg.weight = d; }
void XSpeakable_cnn_top_Set_bias(XSpeakable_cnn_top *, u64 d) { reg.bias = d; }
void XSpeakable_cnn_top_Set_mult(XSpeakable_cnn_top *, u64 d) { reg.mult = d; }
void XSpeakable_cnn_top_Set_shift(XSpeakable_cnn_top *, u64 d) { reg.shift = d; }
void XSpeakable_cnn_top_Set_ofmap(XSpeakable_cnn_top *, u64 d) { reg.ofmap = d; }
void XSpeakable_cnn_top_Set_in_h(XSpeakable_cnn_top *, u32 d) { reg.in_h = d; }
void XSpeakable_cnn_top_Set_in_w(XSpeakable_cnn_top *, u32 d) { reg.in_w = d; }
void XSpeakable_cnn_top_Set_in_ch(XSpeakable_cnn_top *, u32 d) { reg.in_ch = d; }
void XSpeakable_cnn_top_Set_out_ch(XSpeakable_cnn_top *, u32 d) { reg.out_ch = d; }
void XSpeakable_cnn_top_Set_layer_type(XSpeakable_cnn_top *, u32 d) { reg.layer_type = d; }
void XSpeakable_cnn_top_Set_stride(XSpeakable_cnn_top *, u32 d) { reg.stride = d; }
void XSpeakable_cnn_top_Set_do_relu(XSpeakable_cnn_top *, u32 d) { reg.do_relu = d; }
void XSpeakable_cnn_top_Set_do_pool(XSpeakable_cnn_top *, u32 d) { reg.do_pool = d; }

}
