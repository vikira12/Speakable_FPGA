// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of ifmap
//        bit 31~0 - ifmap[31:0] (Read/Write)
// 0x14 : Data signal of ifmap
//        bit 31~0 - ifmap[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of weight
//        bit 31~0 - weight[31:0] (Read/Write)
// 0x20 : Data signal of weight
//        bit 31~0 - weight[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of bias
//        bit 31~0 - bias[31:0] (Read/Write)
// 0x2c : Data signal of bias
//        bit 31~0 - bias[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of ofmap
//        bit 31~0 - ofmap[31:0] (Read/Write)
// 0x38 : Data signal of ofmap
//        bit 31~0 - ofmap[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of in_h
//        bit 7~0 - in_h[7:0] (Read/Write)
//        others  - reserved
// 0x44 : reserved
// 0x48 : Data signal of in_w
//        bit 7~0 - in_w[7:0] (Read/Write)
//        others  - reserved
// 0x4c : reserved
// 0x50 : Data signal of in_ch
//        bit 7~0 - in_ch[7:0] (Read/Write)
//        others  - reserved
// 0x54 : reserved
// 0x58 : Data signal of out_ch
//        bit 7~0 - out_ch[7:0] (Read/Write)
//        others  - reserved
// 0x5c : reserved
// 0x60 : Data signal of do_relu
//        bit 0  - do_relu[0] (Read/Write)
//        others - reserved
// 0x64 : reserved
// 0x68 : Data signal of do_pool
//        bit 0  - do_pool[0] (Read/Write)
//        others - reserved
// 0x6c : reserved
// 0x70 : Data signal of perf
//        bit 31~0 - perf[31:0] (Read)
// 0x74 : Data signal of perf
//        bit 31~0 - perf[63:32] (Read)
// 0x78 : Data signal of perf
//        bit 31~0 - perf[95:64] (Read)
// 0x7c : Control signal of perf
//        bit 0  - perf_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_AP_CTRL      0x00
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_GIE          0x04
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_IER          0x08
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_ISR          0x0c
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_IFMAP_DATA   0x10
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_IFMAP_DATA   64
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_WEIGHT_DATA  0x1c
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_WEIGHT_DATA  64
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_BIAS_DATA    0x28
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_BIAS_DATA    64
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_OFMAP_DATA   0x34
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_OFMAP_DATA   64
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_H_DATA    0x40
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_IN_H_DATA    8
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_W_DATA    0x48
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_IN_W_DATA    8
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_IN_CH_DATA   0x50
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_IN_CH_DATA   8
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_OUT_CH_DATA  0x58
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_OUT_CH_DATA  8
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_RELU_DATA 0x60
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_DO_RELU_DATA 1
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_DO_POOL_DATA 0x68
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_DO_POOL_DATA 1
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_DATA    0x70
#define XSPEAKABLE_CNN_TOP_CTRL_BITS_PERF_DATA    96
#define XSPEAKABLE_CNN_TOP_CTRL_ADDR_PERF_CTRL    0x7c

