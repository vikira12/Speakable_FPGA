#ifndef XSPEAKABLE_CNN_TOP_H
#define XSPEAKABLE_CNN_TOP_H

// PC 시뮬레이션용 드라이버 (실제 드라이버와 같은 API). Start가 HLS C 모델 speakable_cnn_top()을 바로 호출한다
// 버퍼 주소는 PC 포인터 그대로 (베어메탈처럼 CPU 주소 = 가속기 주소)

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XSpeakable_cnn_top;

int  XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, UINTPTR BaseAddress);
void XSpeakable_cnn_top_Start(XSpeakable_cnn_top *InstancePtr);
u32  XSpeakable_cnn_top_IsDone(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_ifmap(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_weight(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_bias(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_mult(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_shift(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_ofmap(XSpeakable_cnn_top *InstancePtr, u64 Data);
void XSpeakable_cnn_top_Set_in_h(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_in_w(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_in_ch(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_out_ch(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_layer_type(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_stride(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_do_relu(XSpeakable_cnn_top *InstancePtr, u32 Data);
void XSpeakable_cnn_top_Set_do_pool(XSpeakable_cnn_top *InstancePtr, u32 Data);

#ifdef __cplusplus
}
#endif

#endif
