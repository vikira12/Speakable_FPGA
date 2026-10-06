// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xdpu_conv_top.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XDpu_conv_top_CfgInitialize(XDpu_conv_top *InstancePtr, XDpu_conv_top_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XDpu_conv_top_Start(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL) & 0x80;
    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XDpu_conv_top_IsDone(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XDpu_conv_top_IsIdle(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XDpu_conv_top_IsReady(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XDpu_conv_top_EnableAutoRestart(XDpu_conv_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL, 0x80);
}

void XDpu_conv_top_DisableAutoRestart(XDpu_conv_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_AP_CTRL, 0);
}

void XDpu_conv_top_Set_ifmap_0(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_0_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_0_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_0(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_0_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_0_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_1(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_1_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_1_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_1(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_1_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_1_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_2(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_2_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_2_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_2(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_2_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_2_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_3(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_3_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_3_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_3(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_3_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_3_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_4(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_4_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_4_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_4(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_4_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_4_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_5(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_5_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_5_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_5(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_5_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_5_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_6(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_6_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_6_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_6(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_6_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_6_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ifmap_7(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_7_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_7_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ifmap_7(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_7_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_IFMAP_7_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_0(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_0_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_0_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_0(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_0_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_0_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_1(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_1_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_1_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_1(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_1_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_1_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_2(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_2_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_2_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_2(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_2_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_2_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_3(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_3_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_3_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_3(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_3_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_3_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_4(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_4_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_4_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_4(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_4_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_4_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_5(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_5_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_5_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_5(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_5_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_5_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_6(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_6_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_6_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_6(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_6_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_6_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_weight_7(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_7_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_7_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_weight_7(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_7_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_WEIGHT_7_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_bias(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_BIAS_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_BIAS_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_bias(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_BIAS_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_BIAS_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_conv_scratch(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_CONV_SCRATCH_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_CONV_SCRATCH_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_conv_scratch(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_CONV_SCRATCH_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_CONV_SCRATCH_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_ofmap(XDpu_conv_top *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_OFMAP_DATA, (u32)(Data));
    XDpu_conv_top_WriteReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_OFMAP_DATA + 4, (u32)(Data >> 32));
}

u64 XDpu_conv_top_Get_ofmap(XDpu_conv_top *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_OFMAP_DATA);
    Data += (u64)XDpu_conv_top_ReadReg(InstancePtr->Control_BaseAddress, XDPU_CONV_TOP_CONTROL_ADDR_OFMAP_DATA + 4) << 32;
    return Data;
}

void XDpu_conv_top_Set_in_h(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_H_DATA, Data);
}

u32 XDpu_conv_top_Get_in_h(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_H_DATA);
    return Data;
}

void XDpu_conv_top_Set_in_w(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_W_DATA, Data);
}

u32 XDpu_conv_top_Get_in_w(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_W_DATA);
    return Data;
}

void XDpu_conv_top_Set_in_ch(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_CH_DATA, Data);
}

u32 XDpu_conv_top_Get_in_ch(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IN_CH_DATA);
    return Data;
}

void XDpu_conv_top_Set_out_ch(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_OUT_CH_DATA, Data);
}

u32 XDpu_conv_top_Get_out_ch(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_OUT_CH_DATA);
    return Data;
}

void XDpu_conv_top_Set_do_relu(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_DO_RELU_DATA, Data);
}

u32 XDpu_conv_top_Get_do_relu(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_DO_RELU_DATA);
    return Data;
}

void XDpu_conv_top_Set_do_pool(XDpu_conv_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_DO_POOL_DATA, Data);
}

u32 XDpu_conv_top_Get_do_pool(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_DO_POOL_DATA);
    return Data;
}

XDpu_conv_top_Perf XDpu_conv_top_Get_perf(XDpu_conv_top *InstancePtr) {
    XDpu_conv_top_Perf Data;

    Data.word_0 = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_PERF_DATA + 0);
    Data.word_1 = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_PERF_DATA + 4);
    Data.word_2 = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_PERF_DATA + 8);
    return Data;
}

u32 XDpu_conv_top_Get_perf_vld(XDpu_conv_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_PERF_CTRL);
    return Data & 0x1;
}

void XDpu_conv_top_InterruptGlobalEnable(XDpu_conv_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_GIE, 1);
}

void XDpu_conv_top_InterruptGlobalDisable(XDpu_conv_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_GIE, 0);
}

void XDpu_conv_top_InterruptEnable(XDpu_conv_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IER);
    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IER, Register | Mask);
}

void XDpu_conv_top_InterruptDisable(XDpu_conv_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IER);
    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IER, Register & (~Mask));
}

void XDpu_conv_top_InterruptClear(XDpu_conv_top *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XDpu_conv_top_WriteReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_ISR, Mask);
}

u32 XDpu_conv_top_InterruptGetEnabled(XDpu_conv_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_IER);
}

u32 XDpu_conv_top_InterruptGetStatus(XDpu_conv_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XDpu_conv_top_ReadReg(InstancePtr->Ctrl_BaseAddress, XDPU_CONV_TOP_CTRL_ADDR_ISR);
}

