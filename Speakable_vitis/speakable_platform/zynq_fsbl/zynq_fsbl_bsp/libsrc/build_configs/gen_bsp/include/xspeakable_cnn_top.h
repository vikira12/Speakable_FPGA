// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XSPEAKABLE_CNN_TOP_H
#define XSPEAKABLE_CNN_TOP_H

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
#include "xspeakable_cnn_top_hw.h"

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
    u64 Ctrl_BaseAddress;
} XSpeakable_cnn_top_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XSpeakable_cnn_top;

typedef u32 word_type;

typedef struct {
    u32 word_0;
    u32 word_1;
    u32 word_2;
} XSpeakable_cnn_top_Perf;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XSpeakable_cnn_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSpeakable_cnn_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XSpeakable_cnn_top_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XSpeakable_cnn_top_ReadReg(BaseAddress, RegOffset) \
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
int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, UINTPTR BaseAddress);
XSpeakable_cnn_top_Config* XSpeakable_cnn_top_LookupConfig(UINTPTR BaseAddress);
#else
int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, u16 DeviceId);
XSpeakable_cnn_top_Config* XSpeakable_cnn_top_LookupConfig(u16 DeviceId);
#endif
int XSpeakable_cnn_top_CfgInitialize(XSpeakable_cnn_top *InstancePtr, XSpeakable_cnn_top_Config *ConfigPtr);
#else
int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, const char* InstanceName);
int XSpeakable_cnn_top_Release(XSpeakable_cnn_top *InstancePtr);
#endif

void XSpeakable_cnn_top_Start(XSpeakable_cnn_top *InstancePtr);
u32 XSpeakable_cnn_top_IsDone(XSpeakable_cnn_top *InstancePtr);
u32 XSpeakable_cnn_top_IsIdle(XSpeakable_cnn_top *InstancePtr);
u32 XSpeakable_cnn_top_IsReady(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_EnableAutoRestart(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_DisableAutoRestart(XSpeakable_cnn_top *InstancePtr);

void XSpeakable_cnn_top_Set_ifmap(XSpeakable_cnn_top *InstancePtr, u64 Data);
u64 XSpeakable_cnn_top_Get_ifmap(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_weight(XSpeakable_cnn_top *InstancePtr, u64 Data);
u64 XSpeakable_cnn_top_Get_weight(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_bias(XSpeakable_cnn_top *InstancePtr, u64 Data);
u64 XSpeakable_cnn_top_Get_bias(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_ofmap(XSpeakable_cnn_top *InstancePtr, u64 Data);
u64 XSpeakable_cnn_top_Get_ofmap(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_in_h(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_in_h(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_in_w(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_in_w(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_in_ch(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_in_ch(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_out_ch(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_out_ch(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_do_relu(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_do_relu(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_Set_do_pool(XSpeakable_cnn_top *InstancePtr, u32 Data);
u32 XSpeakable_cnn_top_Get_do_pool(XSpeakable_cnn_top *InstancePtr);
XSpeakable_cnn_top_Perf XSpeakable_cnn_top_Get_perf(XSpeakable_cnn_top *InstancePtr);
u32 XSpeakable_cnn_top_Get_perf_vld(XSpeakable_cnn_top *InstancePtr);

void XSpeakable_cnn_top_InterruptGlobalEnable(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_InterruptGlobalDisable(XSpeakable_cnn_top *InstancePtr);
void XSpeakable_cnn_top_InterruptEnable(XSpeakable_cnn_top *InstancePtr, u32 Mask);
void XSpeakable_cnn_top_InterruptDisable(XSpeakable_cnn_top *InstancePtr, u32 Mask);
void XSpeakable_cnn_top_InterruptClear(XSpeakable_cnn_top *InstancePtr, u32 Mask);
u32 XSpeakable_cnn_top_InterruptGetEnabled(XSpeakable_cnn_top *InstancePtr);
u32 XSpeakable_cnn_top_InterruptGetStatus(XSpeakable_cnn_top *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
