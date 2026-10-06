// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XDPU_CONV_TOP_H
#define XDPU_CONV_TOP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xdpu_conv_top_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
    u64 Ctrl_BaseAddress;
} XDpu_conv_top_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XDpu_conv_top;

typedef u32 word_type;

typedef struct {
    u32 word_0;
    u32 word_1;
    u32 word_2;
} XDpu_conv_top_Perf;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XDpu_conv_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XDpu_conv_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XDpu_conv_top_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XDpu_conv_top_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XDpu_conv_top_Initialize(XDpu_conv_top *InstancePtr, UINTPTR BaseAddress);
XDpu_conv_top_Config* XDpu_conv_top_LookupConfig(UINTPTR BaseAddress);
#else
int XDpu_conv_top_Initialize(XDpu_conv_top *InstancePtr, u16 DeviceId);
XDpu_conv_top_Config* XDpu_conv_top_LookupConfig(u16 DeviceId);
#endif
int XDpu_conv_top_CfgInitialize(XDpu_conv_top *InstancePtr, XDpu_conv_top_Config *ConfigPtr);
#else
int XDpu_conv_top_Initialize(XDpu_conv_top *InstancePtr, const char* InstanceName);
int XDpu_conv_top_Release(XDpu_conv_top *InstancePtr);
#endif

void XDpu_conv_top_Start(XDpu_conv_top *InstancePtr);
u32 XDpu_conv_top_IsDone(XDpu_conv_top *InstancePtr);
u32 XDpu_conv_top_IsIdle(XDpu_conv_top *InstancePtr);
u32 XDpu_conv_top_IsReady(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_EnableAutoRestart(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_DisableAutoRestart(XDpu_conv_top *InstancePtr);

void XDpu_conv_top_Set_ifmap_0(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_0(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_1(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_1(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_2(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_2(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_3(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_3(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_4(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_4(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_5(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_5(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_6(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_6(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ifmap_7(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ifmap_7(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_0(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_0(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_1(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_1(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_2(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_2(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_3(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_3(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_4(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_4(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_5(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_5(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_6(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_6(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_weight_7(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_weight_7(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_bias(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_bias(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_conv_scratch(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_conv_scratch(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_ofmap(XDpu_conv_top *InstancePtr, u64 Data);
u64 XDpu_conv_top_Get_ofmap(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_in_h(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_in_h(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_in_w(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_in_w(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_in_ch(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_in_ch(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_out_ch(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_out_ch(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_do_relu(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_do_relu(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_Set_do_pool(XDpu_conv_top *InstancePtr, u32 Data);
u32 XDpu_conv_top_Get_do_pool(XDpu_conv_top *InstancePtr);
XDpu_conv_top_Perf XDpu_conv_top_Get_perf(XDpu_conv_top *InstancePtr);
u32 XDpu_conv_top_Get_perf_vld(XDpu_conv_top *InstancePtr);

void XDpu_conv_top_InterruptGlobalEnable(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_InterruptGlobalDisable(XDpu_conv_top *InstancePtr);
void XDpu_conv_top_InterruptEnable(XDpu_conv_top *InstancePtr, u32 Mask);
void XDpu_conv_top_InterruptDisable(XDpu_conv_top *InstancePtr, u32 Mask);
void XDpu_conv_top_InterruptClear(XDpu_conv_top *InstancePtr, u32 Mask);
u32 XDpu_conv_top_InterruptGetEnabled(XDpu_conv_top *InstancePtr);
u32 XDpu_conv_top_InterruptGetStatus(XDpu_conv_top *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
