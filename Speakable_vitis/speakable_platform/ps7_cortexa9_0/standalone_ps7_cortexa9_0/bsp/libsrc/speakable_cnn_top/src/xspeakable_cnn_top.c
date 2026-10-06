// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xspeakable_cnn_top.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSpeakable_cnn_top_CfgInitialize(XSpeakable_cnn_top *InstancePtr, XSpeakable_cnn_top_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSpeakable_cnn_top_Start(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL) & 0x80;
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSpeakable_cnn_top_IsDone(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSpeakable_cnn_top_IsIdle(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSpeakable_cnn_top_IsReady(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSpeakable_cnn_top_EnableAutoRestart(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL, 0x80);
}

void XSpeakable_cnn_top_DisableAutoRestart(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL, 0);
}

void XSpeakable_cnn_top_Set_ifmap(XSpeakable_cnn_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IFMAP_DATA, (u32)(Data));
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IFMAP_DATA + 4, (u32)(Data >> 32));
}

u64 XSpeakable_cnn_top_Get_ifmap(XSpeakable_cnn_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IFMAP_DATA);
    Data += (u64)XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IFMAP_DATA + 4) << 32;
    return Data;
}

void XSpeakable_cnn_top_Set_weight(XSpeakable_cnn_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_WEIGHT_DATA, (u32)(Data));
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_WEIGHT_DATA + 4, (u32)(Data >> 32));
}

u64 XSpeakable_cnn_top_Get_weight(XSpeakable_cnn_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_WEIGHT_DATA);
    Data += (u64)XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_WEIGHT_DATA + 4) << 32;
    return Data;
}

void XSpeakable_cnn_top_Set_bias(XSpeakable_cnn_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_BIAS_DATA, (u32)(Data));
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_BIAS_DATA + 4, (u32)(Data >> 32));
}

u64 XSpeakable_cnn_top_Get_bias(XSpeakable_cnn_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_BIAS_DATA);
    Data += (u64)XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_BIAS_DATA + 4) << 32;
    return Data;
}

void XSpeakable_cnn_top_Set_ofmap(XSpeakable_cnn_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OFMAP_DATA, (u32)(Data));
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OFMAP_DATA + 4, (u32)(Data >> 32));
}

u64 XSpeakable_cnn_top_Get_ofmap(XSpeakable_cnn_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OFMAP_DATA);
    Data += (u64)XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OFMAP_DATA + 4) << 32;
    return Data;
}

void XSpeakable_cnn_top_Set_in_h(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_H_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_in_h(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_H_DATA);
    return Data;
}

void XSpeakable_cnn_top_Set_in_w(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_W_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_in_w(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_W_DATA);
    return Data;
}

void XSpeakable_cnn_top_Set_in_ch(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_CH_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_in_ch(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_CH_DATA);
    return Data;
}

void XSpeakable_cnn_top_Set_out_ch(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OUT_CH_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_out_ch(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_OUT_CH_DATA);
    return Data;
}

void XSpeakable_cnn_top_Set_do_relu(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_RELU_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_do_relu(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_RELU_DATA);
    return Data;
}

void XSpeakable_cnn_top_Set_do_pool(XSpeakable_cnn_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_POOL_DATA, Data);
}

u32 XSpeakable_cnn_top_Get_do_pool(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_POOL_DATA);
    return Data;
}

XSpeakable_cnn_top_Perf XSpeakable_cnn_top_Get_perf(XSpeakable_cnn_top *InstancePtr) {
    XSpeakable_cnn_top_Perf Data;

    Data.word_0 = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_DATA + 0);
    Data.word_1 = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_DATA + 4);
    Data.word_2 = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_DATA + 8);
    return Data;
}

u32 XSpeakable_cnn_top_Get_perf_vld(XSpeakable_cnn_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_CTRL);
    return Data & 0x1;
}

void XSpeakable_cnn_top_InterruptGlobalEnable(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_GIE, 1);
}

void XSpeakable_cnn_top_InterruptGlobalDisable(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_GIE, 0);
}

void XSpeakable_cnn_top_InterruptEnable(XSpeakable_cnn_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER);
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER, Register | Mask);
}

void XSpeakable_cnn_top_InterruptDisable(XSpeakable_cnn_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER);
    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER, Register & (~Mask));
}

void XSpeakable_cnn_top_InterruptClear(XSpeakable_cnn_top *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSpeakable_cnn_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_ISR, Mask);
}

u32 XSpeakable_cnn_top_InterruptGetEnabled(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER);
}

u32 XSpeakable_cnn_top_InterruptGetStatus(XSpeakable_cnn_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSpeakable_cnn_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XSPEAKABLE_CNN_TOP_CTRL_ADDR_ISR);
}

