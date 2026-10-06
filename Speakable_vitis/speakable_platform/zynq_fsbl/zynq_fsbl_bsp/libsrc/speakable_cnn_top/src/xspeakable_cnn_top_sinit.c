// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xspeakable_cnn_top.h"

extern XSpeakable_cnn_top_Config XSpeakable_cnn_top_ConfigTable[];

#ifdef SDT
XSpeakable_cnn_top_Config *XSpeakable_cnn_top_LookupConfig(UINTPTR BaseAddress) {
	XSpeakable_cnn_top_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XSpeakable_cnn_top_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XSpeakable_cnn_top_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) {
			ConfigPtr = &XSpeakable_cnn_top_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, UINTPTR BaseAddress) {
	XSpeakable_cnn_top_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSpeakable_cnn_top_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSpeakable_cnn_top_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XSpeakable_cnn_top_Config *XSpeakable_cnn_top_LookupConfig(u16 DeviceId) {
	XSpeakable_cnn_top_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSPEAKABLE_CNN_TOP_NUM_INSTANCES; Index++) {
		if (XSpeakable_cnn_top_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSpeakable_cnn_top_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSpeakable_cnn_top_Initialize(XSpeakable_cnn_top *InstancePtr, u16 DeviceId) {
	XSpeakable_cnn_top_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSpeakable_cnn_top_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSpeakable_cnn_top_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

