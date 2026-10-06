// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
`timescale 1ns/1ps
(* DowngradeIPIdentifiedWarnings="yes" *) module dpu_conv_top_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 8,
    C_S_AXI_DATA_WIDTH = 32
)(
    input  wire                          ACLK,
    input  wire                          ARESET,
    input  wire                          ACLK_EN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] AWADDR,
    input  wire                          AWVALID,
    output wire                          AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0] WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] WSTRB,
    input  wire                          WVALID,
    output wire                          WREADY,
    output wire [1:0]                    BRESP,
    output wire                          BVALID,
    input  wire                          BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] ARADDR,
    input  wire                          ARVALID,
    output wire                          ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0] RDATA,
    output wire [1:0]                    RRESP,
    output wire                          RVALID,
    input  wire                          RREADY,
    output wire [63:0]                   ifmap_0,
    output wire [63:0]                   ifmap_1,
    output wire [63:0]                   ifmap_2,
    output wire [63:0]                   ifmap_3,
    output wire [63:0]                   ifmap_4,
    output wire [63:0]                   ifmap_5,
    output wire [63:0]                   ifmap_6,
    output wire [63:0]                   ifmap_7,
    output wire [63:0]                   weight_0,
    output wire [63:0]                   weight_1,
    output wire [63:0]                   weight_2,
    output wire [63:0]                   weight_3,
    output wire [63:0]                   weight_4,
    output wire [63:0]                   weight_5,
    output wire [63:0]                   weight_6,
    output wire [63:0]                   weight_7,
    output wire [63:0]                   bias,
    output wire [63:0]                   conv_scratch,
    output wire [63:0]                   ofmap
);
//------------------------Address Info-------------------
// Protocol Used: ap_ctrl_none
//
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : Data signal of ifmap_0
//        bit 31~0 - ifmap_0[31:0] (Read/Write)
// 0x14 : Data signal of ifmap_0
//        bit 31~0 - ifmap_0[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of ifmap_1
//        bit 31~0 - ifmap_1[31:0] (Read/Write)
// 0x20 : Data signal of ifmap_1
//        bit 31~0 - ifmap_1[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of ifmap_2
//        bit 31~0 - ifmap_2[31:0] (Read/Write)
// 0x2c : Data signal of ifmap_2
//        bit 31~0 - ifmap_2[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of ifmap_3
//        bit 31~0 - ifmap_3[31:0] (Read/Write)
// 0x38 : Data signal of ifmap_3
//        bit 31~0 - ifmap_3[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of ifmap_4
//        bit 31~0 - ifmap_4[31:0] (Read/Write)
// 0x44 : Data signal of ifmap_4
//        bit 31~0 - ifmap_4[63:32] (Read/Write)
// 0x48 : reserved
// 0x4c : Data signal of ifmap_5
//        bit 31~0 - ifmap_5[31:0] (Read/Write)
// 0x50 : Data signal of ifmap_5
//        bit 31~0 - ifmap_5[63:32] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of ifmap_6
//        bit 31~0 - ifmap_6[31:0] (Read/Write)
// 0x5c : Data signal of ifmap_6
//        bit 31~0 - ifmap_6[63:32] (Read/Write)
// 0x60 : reserved
// 0x64 : Data signal of ifmap_7
//        bit 31~0 - ifmap_7[31:0] (Read/Write)
// 0x68 : Data signal of ifmap_7
//        bit 31~0 - ifmap_7[63:32] (Read/Write)
// 0x6c : reserved
// 0x70 : Data signal of weight_0
//        bit 31~0 - weight_0[31:0] (Read/Write)
// 0x74 : Data signal of weight_0
//        bit 31~0 - weight_0[63:32] (Read/Write)
// 0x78 : reserved
// 0x7c : Data signal of weight_1
//        bit 31~0 - weight_1[31:0] (Read/Write)
// 0x80 : Data signal of weight_1
//        bit 31~0 - weight_1[63:32] (Read/Write)
// 0x84 : reserved
// 0x88 : Data signal of weight_2
//        bit 31~0 - weight_2[31:0] (Read/Write)
// 0x8c : Data signal of weight_2
//        bit 31~0 - weight_2[63:32] (Read/Write)
// 0x90 : reserved
// 0x94 : Data signal of weight_3
//        bit 31~0 - weight_3[31:0] (Read/Write)
// 0x98 : Data signal of weight_3
//        bit 31~0 - weight_3[63:32] (Read/Write)
// 0x9c : reserved
// 0xa0 : Data signal of weight_4
//        bit 31~0 - weight_4[31:0] (Read/Write)
// 0xa4 : Data signal of weight_4
//        bit 31~0 - weight_4[63:32] (Read/Write)
// 0xa8 : reserved
// 0xac : Data signal of weight_5
//        bit 31~0 - weight_5[31:0] (Read/Write)
// 0xb0 : Data signal of weight_5
//        bit 31~0 - weight_5[63:32] (Read/Write)
// 0xb4 : reserved
// 0xb8 : Data signal of weight_6
//        bit 31~0 - weight_6[31:0] (Read/Write)
// 0xbc : Data signal of weight_6
//        bit 31~0 - weight_6[63:32] (Read/Write)
// 0xc0 : reserved
// 0xc4 : Data signal of weight_7
//        bit 31~0 - weight_7[31:0] (Read/Write)
// 0xc8 : Data signal of weight_7
//        bit 31~0 - weight_7[63:32] (Read/Write)
// 0xcc : reserved
// 0xd0 : Data signal of bias
//        bit 31~0 - bias[31:0] (Read/Write)
// 0xd4 : Data signal of bias
//        bit 31~0 - bias[63:32] (Read/Write)
// 0xd8 : reserved
// 0xdc : Data signal of conv_scratch
//        bit 31~0 - conv_scratch[31:0] (Read/Write)
// 0xe0 : Data signal of conv_scratch
//        bit 31~0 - conv_scratch[63:32] (Read/Write)
// 0xe4 : reserved
// 0xe8 : Data signal of ofmap
//        bit 31~0 - ofmap[31:0] (Read/Write)
// 0xec : Data signal of ofmap
//        bit 31~0 - ofmap[63:32] (Read/Write)
// 0xf0 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

//------------------------Parameter----------------------
localparam
    ADDR_IFMAP_0_DATA_0      = 8'h10,
    ADDR_IFMAP_0_DATA_1      = 8'h14,
    ADDR_IFMAP_0_CTRL        = 8'h18,
    ADDR_IFMAP_1_DATA_0      = 8'h1c,
    ADDR_IFMAP_1_DATA_1      = 8'h20,
    ADDR_IFMAP_1_CTRL        = 8'h24,
    ADDR_IFMAP_2_DATA_0      = 8'h28,
    ADDR_IFMAP_2_DATA_1      = 8'h2c,
    ADDR_IFMAP_2_CTRL        = 8'h30,
    ADDR_IFMAP_3_DATA_0      = 8'h34,
    ADDR_IFMAP_3_DATA_1      = 8'h38,
    ADDR_IFMAP_3_CTRL        = 8'h3c,
    ADDR_IFMAP_4_DATA_0      = 8'h40,
    ADDR_IFMAP_4_DATA_1      = 8'h44,
    ADDR_IFMAP_4_CTRL        = 8'h48,
    ADDR_IFMAP_5_DATA_0      = 8'h4c,
    ADDR_IFMAP_5_DATA_1      = 8'h50,
    ADDR_IFMAP_5_CTRL        = 8'h54,
    ADDR_IFMAP_6_DATA_0      = 8'h58,
    ADDR_IFMAP_6_DATA_1      = 8'h5c,
    ADDR_IFMAP_6_CTRL        = 8'h60,
    ADDR_IFMAP_7_DATA_0      = 8'h64,
    ADDR_IFMAP_7_DATA_1      = 8'h68,
    ADDR_IFMAP_7_CTRL        = 8'h6c,
    ADDR_WEIGHT_0_DATA_0     = 8'h70,
    ADDR_WEIGHT_0_DATA_1     = 8'h74,
    ADDR_WEIGHT_0_CTRL       = 8'h78,
    ADDR_WEIGHT_1_DATA_0     = 8'h7c,
    ADDR_WEIGHT_1_DATA_1     = 8'h80,
    ADDR_WEIGHT_1_CTRL       = 8'h84,
    ADDR_WEIGHT_2_DATA_0     = 8'h88,
    ADDR_WEIGHT_2_DATA_1     = 8'h8c,
    ADDR_WEIGHT_2_CTRL       = 8'h90,
    ADDR_WEIGHT_3_DATA_0     = 8'h94,
    ADDR_WEIGHT_3_DATA_1     = 8'h98,
    ADDR_WEIGHT_3_CTRL       = 8'h9c,
    ADDR_WEIGHT_4_DATA_0     = 8'ha0,
    ADDR_WEIGHT_4_DATA_1     = 8'ha4,
    ADDR_WEIGHT_4_CTRL       = 8'ha8,
    ADDR_WEIGHT_5_DATA_0     = 8'hac,
    ADDR_WEIGHT_5_DATA_1     = 8'hb0,
    ADDR_WEIGHT_5_CTRL       = 8'hb4,
    ADDR_WEIGHT_6_DATA_0     = 8'hb8,
    ADDR_WEIGHT_6_DATA_1     = 8'hbc,
    ADDR_WEIGHT_6_CTRL       = 8'hc0,
    ADDR_WEIGHT_7_DATA_0     = 8'hc4,
    ADDR_WEIGHT_7_DATA_1     = 8'hc8,
    ADDR_WEIGHT_7_CTRL       = 8'hcc,
    ADDR_BIAS_DATA_0         = 8'hd0,
    ADDR_BIAS_DATA_1         = 8'hd4,
    ADDR_BIAS_CTRL           = 8'hd8,
    ADDR_CONV_SCRATCH_DATA_0 = 8'hdc,
    ADDR_CONV_SCRATCH_DATA_1 = 8'he0,
    ADDR_CONV_SCRATCH_CTRL   = 8'he4,
    ADDR_OFMAP_DATA_0        = 8'he8,
    ADDR_OFMAP_DATA_1        = 8'hec,
    ADDR_OFMAP_CTRL          = 8'hf0,
    WRIDLE                   = 2'd0,
    WRDATA                   = 2'd1,
    WRRESP                   = 2'd2,
    WRRESET                  = 2'd3,
    RDIDLE                   = 2'd0,
    RDDATA                   = 2'd1,
    RDRESET                  = 2'd2,
    ADDR_BITS                = 8;

//------------------------Local signal-------------------
    reg  [1:0]                    wstate = WRRESET;
    reg  [1:0]                    wnext;
    reg  [ADDR_BITS-1:0]          waddr;
    wire [C_S_AXI_DATA_WIDTH-1:0] wmask;
    wire                          aw_hs;
    wire                          w_hs;
    reg  [1:0]                    rstate = RDRESET;
    reg  [1:0]                    rnext;
    reg  [C_S_AXI_DATA_WIDTH-1:0] rdata;
    wire                          ar_hs;
    wire [ADDR_BITS-1:0]          raddr;
    // internal registers
    reg  [63:0]                   int_ifmap_0 = 'b0;
    reg  [63:0]                   int_ifmap_1 = 'b0;
    reg  [63:0]                   int_ifmap_2 = 'b0;
    reg  [63:0]                   int_ifmap_3 = 'b0;
    reg  [63:0]                   int_ifmap_4 = 'b0;
    reg  [63:0]                   int_ifmap_5 = 'b0;
    reg  [63:0]                   int_ifmap_6 = 'b0;
    reg  [63:0]                   int_ifmap_7 = 'b0;
    reg  [63:0]                   int_weight_0 = 'b0;
    reg  [63:0]                   int_weight_1 = 'b0;
    reg  [63:0]                   int_weight_2 = 'b0;
    reg  [63:0]                   int_weight_3 = 'b0;
    reg  [63:0]                   int_weight_4 = 'b0;
    reg  [63:0]                   int_weight_5 = 'b0;
    reg  [63:0]                   int_weight_6 = 'b0;
    reg  [63:0]                   int_weight_7 = 'b0;
    reg  [63:0]                   int_bias = 'b0;
    reg  [63:0]                   int_conv_scratch = 'b0;
    reg  [63:0]                   int_ofmap = 'b0;

//------------------------Instantiation------------------


//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA);
assign BVALID  = (wstate == WRRESP);
assign BRESP   = 2'b00;  // OKAY
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always @(*) begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (WVALID)
                wnext = WRRESP;
            else
                wnext = WRDATA;
        WRRESP:
            if (BREADY & BVALID)
                wnext = WRIDLE;
            else
                wnext = WRRESP;
        default:
            wnext = WRIDLE;
    endcase
end

// waddr
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= {AWADDR[ADDR_BITS-1:2], {2{1'b0}}};
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA);
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always @(*) begin
    case (rstate)
        RDIDLE:
            if (ARVALID)
                rnext = RDDATA;
            else
                rnext = RDIDLE;
        RDDATA:
            if (RREADY & RVALID)
                rnext = RDIDLE;
            else
                rnext = RDDATA;
        default:
            rnext = RDIDLE;
    endcase
end

// rdata
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 'b0;
            case (raddr)
                ADDR_IFMAP_0_DATA_0: begin
                    rdata <= int_ifmap_0[31:0];
                end
                ADDR_IFMAP_0_DATA_1: begin
                    rdata <= int_ifmap_0[63:32];
                end
                ADDR_IFMAP_1_DATA_0: begin
                    rdata <= int_ifmap_1[31:0];
                end
                ADDR_IFMAP_1_DATA_1: begin
                    rdata <= int_ifmap_1[63:32];
                end
                ADDR_IFMAP_2_DATA_0: begin
                    rdata <= int_ifmap_2[31:0];
                end
                ADDR_IFMAP_2_DATA_1: begin
                    rdata <= int_ifmap_2[63:32];
                end
                ADDR_IFMAP_3_DATA_0: begin
                    rdata <= int_ifmap_3[31:0];
                end
                ADDR_IFMAP_3_DATA_1: begin
                    rdata <= int_ifmap_3[63:32];
                end
                ADDR_IFMAP_4_DATA_0: begin
                    rdata <= int_ifmap_4[31:0];
                end
                ADDR_IFMAP_4_DATA_1: begin
                    rdata <= int_ifmap_4[63:32];
                end
                ADDR_IFMAP_5_DATA_0: begin
                    rdata <= int_ifmap_5[31:0];
                end
                ADDR_IFMAP_5_DATA_1: begin
                    rdata <= int_ifmap_5[63:32];
                end
                ADDR_IFMAP_6_DATA_0: begin
                    rdata <= int_ifmap_6[31:0];
                end
                ADDR_IFMAP_6_DATA_1: begin
                    rdata <= int_ifmap_6[63:32];
                end
                ADDR_IFMAP_7_DATA_0: begin
                    rdata <= int_ifmap_7[31:0];
                end
                ADDR_IFMAP_7_DATA_1: begin
                    rdata <= int_ifmap_7[63:32];
                end
                ADDR_WEIGHT_0_DATA_0: begin
                    rdata <= int_weight_0[31:0];
                end
                ADDR_WEIGHT_0_DATA_1: begin
                    rdata <= int_weight_0[63:32];
                end
                ADDR_WEIGHT_1_DATA_0: begin
                    rdata <= int_weight_1[31:0];
                end
                ADDR_WEIGHT_1_DATA_1: begin
                    rdata <= int_weight_1[63:32];
                end
                ADDR_WEIGHT_2_DATA_0: begin
                    rdata <= int_weight_2[31:0];
                end
                ADDR_WEIGHT_2_DATA_1: begin
                    rdata <= int_weight_2[63:32];
                end
                ADDR_WEIGHT_3_DATA_0: begin
                    rdata <= int_weight_3[31:0];
                end
                ADDR_WEIGHT_3_DATA_1: begin
                    rdata <= int_weight_3[63:32];
                end
                ADDR_WEIGHT_4_DATA_0: begin
                    rdata <= int_weight_4[31:0];
                end
                ADDR_WEIGHT_4_DATA_1: begin
                    rdata <= int_weight_4[63:32];
                end
                ADDR_WEIGHT_5_DATA_0: begin
                    rdata <= int_weight_5[31:0];
                end
                ADDR_WEIGHT_5_DATA_1: begin
                    rdata <= int_weight_5[63:32];
                end
                ADDR_WEIGHT_6_DATA_0: begin
                    rdata <= int_weight_6[31:0];
                end
                ADDR_WEIGHT_6_DATA_1: begin
                    rdata <= int_weight_6[63:32];
                end
                ADDR_WEIGHT_7_DATA_0: begin
                    rdata <= int_weight_7[31:0];
                end
                ADDR_WEIGHT_7_DATA_1: begin
                    rdata <= int_weight_7[63:32];
                end
                ADDR_BIAS_DATA_0: begin
                    rdata <= int_bias[31:0];
                end
                ADDR_BIAS_DATA_1: begin
                    rdata <= int_bias[63:32];
                end
                ADDR_CONV_SCRATCH_DATA_0: begin
                    rdata <= int_conv_scratch[31:0];
                end
                ADDR_CONV_SCRATCH_DATA_1: begin
                    rdata <= int_conv_scratch[63:32];
                end
                ADDR_OFMAP_DATA_0: begin
                    rdata <= int_ofmap[31:0];
                end
                ADDR_OFMAP_DATA_1: begin
                    rdata <= int_ofmap[63:32];
                end
            endcase
        end
    end
end


//------------------------Register logic-----------------
assign ifmap_0      = int_ifmap_0;
assign ifmap_1      = int_ifmap_1;
assign ifmap_2      = int_ifmap_2;
assign ifmap_3      = int_ifmap_3;
assign ifmap_4      = int_ifmap_4;
assign ifmap_5      = int_ifmap_5;
assign ifmap_6      = int_ifmap_6;
assign ifmap_7      = int_ifmap_7;
assign weight_0     = int_weight_0;
assign weight_1     = int_weight_1;
assign weight_2     = int_weight_2;
assign weight_3     = int_weight_3;
assign weight_4     = int_weight_4;
assign weight_5     = int_weight_5;
assign weight_6     = int_weight_6;
assign weight_7     = int_weight_7;
assign bias         = int_bias;
assign conv_scratch = int_conv_scratch;
assign ofmap        = int_ofmap;
// int_ifmap_0[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_0[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_0_DATA_0)
            int_ifmap_0[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_0[31:0] & ~wmask);
    end
end

// int_ifmap_0[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_0[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_0_DATA_1)
            int_ifmap_0[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_0[63:32] & ~wmask);
    end
end

// int_ifmap_1[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_1[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_1_DATA_0)
            int_ifmap_1[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_1[31:0] & ~wmask);
    end
end

// int_ifmap_1[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_1[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_1_DATA_1)
            int_ifmap_1[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_1[63:32] & ~wmask);
    end
end

// int_ifmap_2[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_2[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_2_DATA_0)
            int_ifmap_2[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_2[31:0] & ~wmask);
    end
end

// int_ifmap_2[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_2[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_2_DATA_1)
            int_ifmap_2[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_2[63:32] & ~wmask);
    end
end

// int_ifmap_3[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_3[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_3_DATA_0)
            int_ifmap_3[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_3[31:0] & ~wmask);
    end
end

// int_ifmap_3[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_3[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_3_DATA_1)
            int_ifmap_3[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_3[63:32] & ~wmask);
    end
end

// int_ifmap_4[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_4[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_4_DATA_0)
            int_ifmap_4[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_4[31:0] & ~wmask);
    end
end

// int_ifmap_4[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_4[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_4_DATA_1)
            int_ifmap_4[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_4[63:32] & ~wmask);
    end
end

// int_ifmap_5[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_5[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_5_DATA_0)
            int_ifmap_5[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_5[31:0] & ~wmask);
    end
end

// int_ifmap_5[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_5[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_5_DATA_1)
            int_ifmap_5[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_5[63:32] & ~wmask);
    end
end

// int_ifmap_6[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_6[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_6_DATA_0)
            int_ifmap_6[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_6[31:0] & ~wmask);
    end
end

// int_ifmap_6[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_6[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_6_DATA_1)
            int_ifmap_6[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_6[63:32] & ~wmask);
    end
end

// int_ifmap_7[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_7[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_7_DATA_0)
            int_ifmap_7[31:0] <= (WDATA[31:0] & wmask) | (int_ifmap_7[31:0] & ~wmask);
    end
end

// int_ifmap_7[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ifmap_7[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IFMAP_7_DATA_1)
            int_ifmap_7[63:32] <= (WDATA[31:0] & wmask) | (int_ifmap_7[63:32] & ~wmask);
    end
end

// int_weight_0[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_0[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_0_DATA_0)
            int_weight_0[31:0] <= (WDATA[31:0] & wmask) | (int_weight_0[31:0] & ~wmask);
    end
end

// int_weight_0[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_0[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_0_DATA_1)
            int_weight_0[63:32] <= (WDATA[31:0] & wmask) | (int_weight_0[63:32] & ~wmask);
    end
end

// int_weight_1[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_1[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_1_DATA_0)
            int_weight_1[31:0] <= (WDATA[31:0] & wmask) | (int_weight_1[31:0] & ~wmask);
    end
end

// int_weight_1[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_1[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_1_DATA_1)
            int_weight_1[63:32] <= (WDATA[31:0] & wmask) | (int_weight_1[63:32] & ~wmask);
    end
end

// int_weight_2[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_2[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_2_DATA_0)
            int_weight_2[31:0] <= (WDATA[31:0] & wmask) | (int_weight_2[31:0] & ~wmask);
    end
end

// int_weight_2[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_2[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_2_DATA_1)
            int_weight_2[63:32] <= (WDATA[31:0] & wmask) | (int_weight_2[63:32] & ~wmask);
    end
end

// int_weight_3[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_3[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_3_DATA_0)
            int_weight_3[31:0] <= (WDATA[31:0] & wmask) | (int_weight_3[31:0] & ~wmask);
    end
end

// int_weight_3[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_3[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_3_DATA_1)
            int_weight_3[63:32] <= (WDATA[31:0] & wmask) | (int_weight_3[63:32] & ~wmask);
    end
end

// int_weight_4[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_4[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_4_DATA_0)
            int_weight_4[31:0] <= (WDATA[31:0] & wmask) | (int_weight_4[31:0] & ~wmask);
    end
end

// int_weight_4[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_4[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_4_DATA_1)
            int_weight_4[63:32] <= (WDATA[31:0] & wmask) | (int_weight_4[63:32] & ~wmask);
    end
end

// int_weight_5[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_5[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_5_DATA_0)
            int_weight_5[31:0] <= (WDATA[31:0] & wmask) | (int_weight_5[31:0] & ~wmask);
    end
end

// int_weight_5[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_5[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_5_DATA_1)
            int_weight_5[63:32] <= (WDATA[31:0] & wmask) | (int_weight_5[63:32] & ~wmask);
    end
end

// int_weight_6[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_6[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_6_DATA_0)
            int_weight_6[31:0] <= (WDATA[31:0] & wmask) | (int_weight_6[31:0] & ~wmask);
    end
end

// int_weight_6[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_6[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_6_DATA_1)
            int_weight_6[63:32] <= (WDATA[31:0] & wmask) | (int_weight_6[63:32] & ~wmask);
    end
end

// int_weight_7[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_7[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_7_DATA_0)
            int_weight_7[31:0] <= (WDATA[31:0] & wmask) | (int_weight_7[31:0] & ~wmask);
    end
end

// int_weight_7[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_weight_7[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_WEIGHT_7_DATA_1)
            int_weight_7[63:32] <= (WDATA[31:0] & wmask) | (int_weight_7[63:32] & ~wmask);
    end
end

// int_bias[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_bias[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_BIAS_DATA_0)
            int_bias[31:0] <= (WDATA[31:0] & wmask) | (int_bias[31:0] & ~wmask);
    end
end

// int_bias[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_bias[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_BIAS_DATA_1)
            int_bias[63:32] <= (WDATA[31:0] & wmask) | (int_bias[63:32] & ~wmask);
    end
end

// int_conv_scratch[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_conv_scratch[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_CONV_SCRATCH_DATA_0)
            int_conv_scratch[31:0] <= (WDATA[31:0] & wmask) | (int_conv_scratch[31:0] & ~wmask);
    end
end

// int_conv_scratch[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_conv_scratch[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_CONV_SCRATCH_DATA_1)
            int_conv_scratch[63:32] <= (WDATA[31:0] & wmask) | (int_conv_scratch[63:32] & ~wmask);
    end
end

// int_ofmap[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_ofmap[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_OFMAP_DATA_0)
            int_ofmap[31:0] <= (WDATA[31:0] & wmask) | (int_ofmap[31:0] & ~wmask);
    end
end

// int_ofmap[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_ofmap[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_OFMAP_DATA_1)
            int_ofmap[63:32] <= (WDATA[31:0] & wmask) | (int_ofmap[63:32] & ~wmask);
    end
end


//------------------------Memory logic-------------------

endmodule
