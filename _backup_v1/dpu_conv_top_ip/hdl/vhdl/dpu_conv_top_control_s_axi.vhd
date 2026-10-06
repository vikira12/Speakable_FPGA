-- ==============================================================
-- Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
-- Tool Version Limit: 2025.11
-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
-- 
-- ==============================================================
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity dpu_conv_top_control_s_axi is
generic (
    C_S_AXI_ADDR_WIDTH    : INTEGER := 8;
    C_S_AXI_DATA_WIDTH    : INTEGER := 32);
port (
    ACLK                  :in   STD_LOGIC;
    ARESET                :in   STD_LOGIC;
    ACLK_EN               :in   STD_LOGIC;
    AWADDR                :in   STD_LOGIC_VECTOR(C_S_AXI_ADDR_WIDTH-1 downto 0);
    AWVALID               :in   STD_LOGIC;
    AWREADY               :out  STD_LOGIC;
    WDATA                 :in   STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH-1 downto 0);
    WSTRB                 :in   STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH/8-1 downto 0);
    WVALID                :in   STD_LOGIC;
    WREADY                :out  STD_LOGIC;
    BRESP                 :out  STD_LOGIC_VECTOR(1 downto 0);
    BVALID                :out  STD_LOGIC;
    BREADY                :in   STD_LOGIC;
    ARADDR                :in   STD_LOGIC_VECTOR(C_S_AXI_ADDR_WIDTH-1 downto 0);
    ARVALID               :in   STD_LOGIC;
    ARREADY               :out  STD_LOGIC;
    RDATA                 :out  STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH-1 downto 0);
    RRESP                 :out  STD_LOGIC_VECTOR(1 downto 0);
    RVALID                :out  STD_LOGIC;
    RREADY                :in   STD_LOGIC;
    ifmap_0               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_1               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_2               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_3               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_4               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_5               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_6               :out  STD_LOGIC_VECTOR(63 downto 0);
    ifmap_7               :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_0              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_1              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_2              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_3              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_4              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_5              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_6              :out  STD_LOGIC_VECTOR(63 downto 0);
    weight_7              :out  STD_LOGIC_VECTOR(63 downto 0);
    bias                  :out  STD_LOGIC_VECTOR(63 downto 0);
    conv_scratch          :out  STD_LOGIC_VECTOR(63 downto 0);
    ofmap                 :out  STD_LOGIC_VECTOR(63 downto 0)
);
end entity dpu_conv_top_control_s_axi;

-- ------------------------Address Info-------------------
-- Protocol Used: ap_ctrl_none
--
-- 0x00 : reserved
-- 0x04 : reserved
-- 0x08 : reserved
-- 0x0c : reserved
-- 0x10 : Data signal of ifmap_0
--        bit 31~0 - ifmap_0[31:0] (Read/Write)
-- 0x14 : Data signal of ifmap_0
--        bit 31~0 - ifmap_0[63:32] (Read/Write)
-- 0x18 : reserved
-- 0x1c : Data signal of ifmap_1
--        bit 31~0 - ifmap_1[31:0] (Read/Write)
-- 0x20 : Data signal of ifmap_1
--        bit 31~0 - ifmap_1[63:32] (Read/Write)
-- 0x24 : reserved
-- 0x28 : Data signal of ifmap_2
--        bit 31~0 - ifmap_2[31:0] (Read/Write)
-- 0x2c : Data signal of ifmap_2
--        bit 31~0 - ifmap_2[63:32] (Read/Write)
-- 0x30 : reserved
-- 0x34 : Data signal of ifmap_3
--        bit 31~0 - ifmap_3[31:0] (Read/Write)
-- 0x38 : Data signal of ifmap_3
--        bit 31~0 - ifmap_3[63:32] (Read/Write)
-- 0x3c : reserved
-- 0x40 : Data signal of ifmap_4
--        bit 31~0 - ifmap_4[31:0] (Read/Write)
-- 0x44 : Data signal of ifmap_4
--        bit 31~0 - ifmap_4[63:32] (Read/Write)
-- 0x48 : reserved
-- 0x4c : Data signal of ifmap_5
--        bit 31~0 - ifmap_5[31:0] (Read/Write)
-- 0x50 : Data signal of ifmap_5
--        bit 31~0 - ifmap_5[63:32] (Read/Write)
-- 0x54 : reserved
-- 0x58 : Data signal of ifmap_6
--        bit 31~0 - ifmap_6[31:0] (Read/Write)
-- 0x5c : Data signal of ifmap_6
--        bit 31~0 - ifmap_6[63:32] (Read/Write)
-- 0x60 : reserved
-- 0x64 : Data signal of ifmap_7
--        bit 31~0 - ifmap_7[31:0] (Read/Write)
-- 0x68 : Data signal of ifmap_7
--        bit 31~0 - ifmap_7[63:32] (Read/Write)
-- 0x6c : reserved
-- 0x70 : Data signal of weight_0
--        bit 31~0 - weight_0[31:0] (Read/Write)
-- 0x74 : Data signal of weight_0
--        bit 31~0 - weight_0[63:32] (Read/Write)
-- 0x78 : reserved
-- 0x7c : Data signal of weight_1
--        bit 31~0 - weight_1[31:0] (Read/Write)
-- 0x80 : Data signal of weight_1
--        bit 31~0 - weight_1[63:32] (Read/Write)
-- 0x84 : reserved
-- 0x88 : Data signal of weight_2
--        bit 31~0 - weight_2[31:0] (Read/Write)
-- 0x8c : Data signal of weight_2
--        bit 31~0 - weight_2[63:32] (Read/Write)
-- 0x90 : reserved
-- 0x94 : Data signal of weight_3
--        bit 31~0 - weight_3[31:0] (Read/Write)
-- 0x98 : Data signal of weight_3
--        bit 31~0 - weight_3[63:32] (Read/Write)
-- 0x9c : reserved
-- 0xa0 : Data signal of weight_4
--        bit 31~0 - weight_4[31:0] (Read/Write)
-- 0xa4 : Data signal of weight_4
--        bit 31~0 - weight_4[63:32] (Read/Write)
-- 0xa8 : reserved
-- 0xac : Data signal of weight_5
--        bit 31~0 - weight_5[31:0] (Read/Write)
-- 0xb0 : Data signal of weight_5
--        bit 31~0 - weight_5[63:32] (Read/Write)
-- 0xb4 : reserved
-- 0xb8 : Data signal of weight_6
--        bit 31~0 - weight_6[31:0] (Read/Write)
-- 0xbc : Data signal of weight_6
--        bit 31~0 - weight_6[63:32] (Read/Write)
-- 0xc0 : reserved
-- 0xc4 : Data signal of weight_7
--        bit 31~0 - weight_7[31:0] (Read/Write)
-- 0xc8 : Data signal of weight_7
--        bit 31~0 - weight_7[63:32] (Read/Write)
-- 0xcc : reserved
-- 0xd0 : Data signal of bias
--        bit 31~0 - bias[31:0] (Read/Write)
-- 0xd4 : Data signal of bias
--        bit 31~0 - bias[63:32] (Read/Write)
-- 0xd8 : reserved
-- 0xdc : Data signal of conv_scratch
--        bit 31~0 - conv_scratch[31:0] (Read/Write)
-- 0xe0 : Data signal of conv_scratch
--        bit 31~0 - conv_scratch[63:32] (Read/Write)
-- 0xe4 : reserved
-- 0xe8 : Data signal of ofmap
--        bit 31~0 - ofmap[31:0] (Read/Write)
-- 0xec : Data signal of ofmap
--        bit 31~0 - ofmap[63:32] (Read/Write)
-- 0xf0 : reserved
-- (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

architecture behave of dpu_conv_top_control_s_axi is
attribute DowngradeIPIdentifiedWarnings : STRING;
attribute DowngradeIPIdentifiedWarnings of behave : architecture is "yes";
    type states is (wridle, wrdata, wrresp, wrreset, rdidle, rddata, rdreset);  -- read and write fsm states
    signal wstate  : states := wrreset;
    signal rstate  : states := rdreset;
    signal wnext, rnext: states;
    constant ADDR_IFMAP_0_DATA_0      : INTEGER := 16#10#;
    constant ADDR_IFMAP_0_DATA_1      : INTEGER := 16#14#;
    constant ADDR_IFMAP_0_CTRL        : INTEGER := 16#18#;
    constant ADDR_IFMAP_1_DATA_0      : INTEGER := 16#1c#;
    constant ADDR_IFMAP_1_DATA_1      : INTEGER := 16#20#;
    constant ADDR_IFMAP_1_CTRL        : INTEGER := 16#24#;
    constant ADDR_IFMAP_2_DATA_0      : INTEGER := 16#28#;
    constant ADDR_IFMAP_2_DATA_1      : INTEGER := 16#2c#;
    constant ADDR_IFMAP_2_CTRL        : INTEGER := 16#30#;
    constant ADDR_IFMAP_3_DATA_0      : INTEGER := 16#34#;
    constant ADDR_IFMAP_3_DATA_1      : INTEGER := 16#38#;
    constant ADDR_IFMAP_3_CTRL        : INTEGER := 16#3c#;
    constant ADDR_IFMAP_4_DATA_0      : INTEGER := 16#40#;
    constant ADDR_IFMAP_4_DATA_1      : INTEGER := 16#44#;
    constant ADDR_IFMAP_4_CTRL        : INTEGER := 16#48#;
    constant ADDR_IFMAP_5_DATA_0      : INTEGER := 16#4c#;
    constant ADDR_IFMAP_5_DATA_1      : INTEGER := 16#50#;
    constant ADDR_IFMAP_5_CTRL        : INTEGER := 16#54#;
    constant ADDR_IFMAP_6_DATA_0      : INTEGER := 16#58#;
    constant ADDR_IFMAP_6_DATA_1      : INTEGER := 16#5c#;
    constant ADDR_IFMAP_6_CTRL        : INTEGER := 16#60#;
    constant ADDR_IFMAP_7_DATA_0      : INTEGER := 16#64#;
    constant ADDR_IFMAP_7_DATA_1      : INTEGER := 16#68#;
    constant ADDR_IFMAP_7_CTRL        : INTEGER := 16#6c#;
    constant ADDR_WEIGHT_0_DATA_0     : INTEGER := 16#70#;
    constant ADDR_WEIGHT_0_DATA_1     : INTEGER := 16#74#;
    constant ADDR_WEIGHT_0_CTRL       : INTEGER := 16#78#;
    constant ADDR_WEIGHT_1_DATA_0     : INTEGER := 16#7c#;
    constant ADDR_WEIGHT_1_DATA_1     : INTEGER := 16#80#;
    constant ADDR_WEIGHT_1_CTRL       : INTEGER := 16#84#;
    constant ADDR_WEIGHT_2_DATA_0     : INTEGER := 16#88#;
    constant ADDR_WEIGHT_2_DATA_1     : INTEGER := 16#8c#;
    constant ADDR_WEIGHT_2_CTRL       : INTEGER := 16#90#;
    constant ADDR_WEIGHT_3_DATA_0     : INTEGER := 16#94#;
    constant ADDR_WEIGHT_3_DATA_1     : INTEGER := 16#98#;
    constant ADDR_WEIGHT_3_CTRL       : INTEGER := 16#9c#;
    constant ADDR_WEIGHT_4_DATA_0     : INTEGER := 16#a0#;
    constant ADDR_WEIGHT_4_DATA_1     : INTEGER := 16#a4#;
    constant ADDR_WEIGHT_4_CTRL       : INTEGER := 16#a8#;
    constant ADDR_WEIGHT_5_DATA_0     : INTEGER := 16#ac#;
    constant ADDR_WEIGHT_5_DATA_1     : INTEGER := 16#b0#;
    constant ADDR_WEIGHT_5_CTRL       : INTEGER := 16#b4#;
    constant ADDR_WEIGHT_6_DATA_0     : INTEGER := 16#b8#;
    constant ADDR_WEIGHT_6_DATA_1     : INTEGER := 16#bc#;
    constant ADDR_WEIGHT_6_CTRL       : INTEGER := 16#c0#;
    constant ADDR_WEIGHT_7_DATA_0     : INTEGER := 16#c4#;
    constant ADDR_WEIGHT_7_DATA_1     : INTEGER := 16#c8#;
    constant ADDR_WEIGHT_7_CTRL       : INTEGER := 16#cc#;
    constant ADDR_BIAS_DATA_0         : INTEGER := 16#d0#;
    constant ADDR_BIAS_DATA_1         : INTEGER := 16#d4#;
    constant ADDR_BIAS_CTRL           : INTEGER := 16#d8#;
    constant ADDR_CONV_SCRATCH_DATA_0 : INTEGER := 16#dc#;
    constant ADDR_CONV_SCRATCH_DATA_1 : INTEGER := 16#e0#;
    constant ADDR_CONV_SCRATCH_CTRL   : INTEGER := 16#e4#;
    constant ADDR_OFMAP_DATA_0        : INTEGER := 16#e8#;
    constant ADDR_OFMAP_DATA_1        : INTEGER := 16#ec#;
    constant ADDR_OFMAP_CTRL          : INTEGER := 16#f0#;
    constant ADDR_BITS         : INTEGER := 8;

    signal AWREADY_t           : STD_LOGIC;
    signal WREADY_t            : STD_LOGIC;
    signal ARREADY_t           : STD_LOGIC;
    signal RVALID_t            : STD_LOGIC;
    signal BVALID_t            : STD_LOGIC;
    signal waddr               : UNSIGNED(ADDR_BITS-1 downto 0);
    signal wmask               : UNSIGNED(C_S_AXI_DATA_WIDTH-1 downto 0);
    signal aw_hs               : STD_LOGIC;
    signal w_hs                : STD_LOGIC;
    signal rdata_data          : UNSIGNED(C_S_AXI_DATA_WIDTH-1 downto 0);
    signal ar_hs               : STD_LOGIC;
    signal raddr               : UNSIGNED(ADDR_BITS-1 downto 0);
    -- internal registers
    signal int_ifmap_0         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_1         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_2         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_3         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_4         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_5         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_6         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ifmap_7         : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_0        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_1        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_2        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_3        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_4        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_5        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_6        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_weight_7        : UNSIGNED(63 downto 0) := (others => '0');
    signal int_bias            : UNSIGNED(63 downto 0) := (others => '0');
    signal int_conv_scratch    : UNSIGNED(63 downto 0) := (others => '0');
    signal int_ofmap           : UNSIGNED(63 downto 0) := (others => '0');


begin
-- ----------------------- Instantiation------------------


-- ----------------------- AXI WRITE ---------------------
    AWREADY_t <=  '1' when wstate = wridle else '0';
    AWREADY   <=  AWREADY_t;
    WREADY_t  <=  '1' when wstate = wrdata else '0';
    WREADY    <=  WREADY_t;
    BVALID_t  <=  '1' when wstate = wrresp else '0';
    BVALID    <=  BVALID_t;
    BRESP     <=  "00";  -- OKAY
    wmask     <=  (31 downto 24 => WSTRB(3), 23 downto 16 => WSTRB(2), 15 downto 8 => WSTRB(1), 7 downto 0 => WSTRB(0));
    aw_hs     <=  AWVALID and AWREADY_t;
    w_hs      <=  WVALID and WREADY_t;

    -- write FSM
    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                wstate <= wrreset;
            elsif (ACLK_EN = '1') then
                wstate <= wnext;
            end if;
        end if;
    end process;

    process (wstate, AWVALID, WVALID, BREADY, BVALID_t)
    begin
        case (wstate) is
        when wridle =>
            if (AWVALID = '1') then
                wnext <= wrdata;
            else
                wnext <= wridle;
            end if;
        when wrdata =>
            if (WVALID = '1') then
                wnext <= wrresp;
            else
                wnext <= wrdata;
            end if;
        when wrresp =>
            if (BREADY = '1' and BVALID_t = '1') then
                wnext <= wridle;
            else
                wnext <= wrresp;
            end if;
        when others =>
            wnext <= wridle;
        end case;
    end process;

    waddr_proc : process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (aw_hs = '1') then
                    waddr <= UNSIGNED(AWADDR(ADDR_BITS-1 downto 2) & (1 downto 0 => '0'));
                end if;
            end if;
        end if;
    end process;

-- ----------------------- AXI READ ----------------------
    ARREADY_t <= '1' when (rstate = rdidle) else '0';
    ARREADY <= ARREADY_t;
    RDATA   <= STD_LOGIC_VECTOR(rdata_data);
    RRESP   <= "00";  -- OKAY
    RVALID_t  <= '1' when (rstate = rddata) else '0';
    RVALID    <= RVALID_t;
    ar_hs   <= ARVALID and ARREADY_t;
    raddr   <= UNSIGNED(ARADDR(ADDR_BITS-1 downto 0));

    -- read FSM
    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                rstate <= rdreset;
            elsif (ACLK_EN = '1') then
                rstate <= rnext;
            end if;
        end if;
    end process;

    process (rstate, ARVALID, RREADY, RVALID_t)
    begin
        case (rstate) is
        when rdidle =>
            if (ARVALID = '1') then
                rnext <= rddata;
            else
                rnext <= rdidle;
            end if;
        when rddata =>
            if (RREADY = '1' and RVALID_t = '1') then
                rnext <= rdidle;
            else
                rnext <= rddata;
            end if;
        when others =>
            rnext <= rdidle;
        end case;
    end process;

    rdata_proc : process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (ar_hs = '1') then
                    rdata_data <= (others => '0');
                    case (TO_INTEGER(raddr)) is
                    when ADDR_IFMAP_0_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_0(31 downto 0), 32);
                    when ADDR_IFMAP_0_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_0(63 downto 32), 32);
                    when ADDR_IFMAP_1_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_1(31 downto 0), 32);
                    when ADDR_IFMAP_1_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_1(63 downto 32), 32);
                    when ADDR_IFMAP_2_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_2(31 downto 0), 32);
                    when ADDR_IFMAP_2_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_2(63 downto 32), 32);
                    when ADDR_IFMAP_3_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_3(31 downto 0), 32);
                    when ADDR_IFMAP_3_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_3(63 downto 32), 32);
                    when ADDR_IFMAP_4_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_4(31 downto 0), 32);
                    when ADDR_IFMAP_4_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_4(63 downto 32), 32);
                    when ADDR_IFMAP_5_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_5(31 downto 0), 32);
                    when ADDR_IFMAP_5_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_5(63 downto 32), 32);
                    when ADDR_IFMAP_6_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_6(31 downto 0), 32);
                    when ADDR_IFMAP_6_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_6(63 downto 32), 32);
                    when ADDR_IFMAP_7_DATA_0 =>
                        rdata_data <= RESIZE(int_ifmap_7(31 downto 0), 32);
                    when ADDR_IFMAP_7_DATA_1 =>
                        rdata_data <= RESIZE(int_ifmap_7(63 downto 32), 32);
                    when ADDR_WEIGHT_0_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_0(31 downto 0), 32);
                    when ADDR_WEIGHT_0_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_0(63 downto 32), 32);
                    when ADDR_WEIGHT_1_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_1(31 downto 0), 32);
                    when ADDR_WEIGHT_1_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_1(63 downto 32), 32);
                    when ADDR_WEIGHT_2_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_2(31 downto 0), 32);
                    when ADDR_WEIGHT_2_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_2(63 downto 32), 32);
                    when ADDR_WEIGHT_3_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_3(31 downto 0), 32);
                    when ADDR_WEIGHT_3_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_3(63 downto 32), 32);
                    when ADDR_WEIGHT_4_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_4(31 downto 0), 32);
                    when ADDR_WEIGHT_4_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_4(63 downto 32), 32);
                    when ADDR_WEIGHT_5_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_5(31 downto 0), 32);
                    when ADDR_WEIGHT_5_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_5(63 downto 32), 32);
                    when ADDR_WEIGHT_6_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_6(31 downto 0), 32);
                    when ADDR_WEIGHT_6_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_6(63 downto 32), 32);
                    when ADDR_WEIGHT_7_DATA_0 =>
                        rdata_data <= RESIZE(int_weight_7(31 downto 0), 32);
                    when ADDR_WEIGHT_7_DATA_1 =>
                        rdata_data <= RESIZE(int_weight_7(63 downto 32), 32);
                    when ADDR_BIAS_DATA_0 =>
                        rdata_data <= RESIZE(int_bias(31 downto 0), 32);
                    when ADDR_BIAS_DATA_1 =>
                        rdata_data <= RESIZE(int_bias(63 downto 32), 32);
                    when ADDR_CONV_SCRATCH_DATA_0 =>
                        rdata_data <= RESIZE(int_conv_scratch(31 downto 0), 32);
                    when ADDR_CONV_SCRATCH_DATA_1 =>
                        rdata_data <= RESIZE(int_conv_scratch(63 downto 32), 32);
                    when ADDR_OFMAP_DATA_0 =>
                        rdata_data <= RESIZE(int_ofmap(31 downto 0), 32);
                    when ADDR_OFMAP_DATA_1 =>
                        rdata_data <= RESIZE(int_ofmap(63 downto 32), 32);
                    when others =>
                        NULL;
                    end case;
                end if;
            end if;
        end if;
    end process;

-- ----------------------- Register logic ----------------
    ifmap_0              <= STD_LOGIC_VECTOR(int_ifmap_0);
    ifmap_1              <= STD_LOGIC_VECTOR(int_ifmap_1);
    ifmap_2              <= STD_LOGIC_VECTOR(int_ifmap_2);
    ifmap_3              <= STD_LOGIC_VECTOR(int_ifmap_3);
    ifmap_4              <= STD_LOGIC_VECTOR(int_ifmap_4);
    ifmap_5              <= STD_LOGIC_VECTOR(int_ifmap_5);
    ifmap_6              <= STD_LOGIC_VECTOR(int_ifmap_6);
    ifmap_7              <= STD_LOGIC_VECTOR(int_ifmap_7);
    weight_0             <= STD_LOGIC_VECTOR(int_weight_0);
    weight_1             <= STD_LOGIC_VECTOR(int_weight_1);
    weight_2             <= STD_LOGIC_VECTOR(int_weight_2);
    weight_3             <= STD_LOGIC_VECTOR(int_weight_3);
    weight_4             <= STD_LOGIC_VECTOR(int_weight_4);
    weight_5             <= STD_LOGIC_VECTOR(int_weight_5);
    weight_6             <= STD_LOGIC_VECTOR(int_weight_6);
    weight_7             <= STD_LOGIC_VECTOR(int_weight_7);
    bias                 <= STD_LOGIC_VECTOR(int_bias);
    conv_scratch         <= STD_LOGIC_VECTOR(int_conv_scratch);
    ofmap                <= STD_LOGIC_VECTOR(int_ofmap);

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_0(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_0_DATA_0) then
                    int_ifmap_0(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_0(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_0(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_0_DATA_1) then
                    int_ifmap_0(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_0(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_1(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_1_DATA_0) then
                    int_ifmap_1(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_1(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_1(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_1_DATA_1) then
                    int_ifmap_1(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_1(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_2(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_2_DATA_0) then
                    int_ifmap_2(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_2(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_2(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_2_DATA_1) then
                    int_ifmap_2(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_2(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_3(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_3_DATA_0) then
                    int_ifmap_3(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_3(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_3(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_3_DATA_1) then
                    int_ifmap_3(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_3(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_4(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_4_DATA_0) then
                    int_ifmap_4(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_4(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_4(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_4_DATA_1) then
                    int_ifmap_4(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_4(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_5(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_5_DATA_0) then
                    int_ifmap_5(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_5(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_5(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_5_DATA_1) then
                    int_ifmap_5(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_5(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_6(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_6_DATA_0) then
                    int_ifmap_6(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_6(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_6(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_6_DATA_1) then
                    int_ifmap_6(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_6(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_7(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_7_DATA_0) then
                    int_ifmap_7(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_7(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ifmap_7(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IFMAP_7_DATA_1) then
                    int_ifmap_7(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ifmap_7(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_0(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_0_DATA_0) then
                    int_weight_0(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_0(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_0(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_0_DATA_1) then
                    int_weight_0(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_0(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_1(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_1_DATA_0) then
                    int_weight_1(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_1(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_1(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_1_DATA_1) then
                    int_weight_1(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_1(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_2(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_2_DATA_0) then
                    int_weight_2(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_2(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_2(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_2_DATA_1) then
                    int_weight_2(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_2(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_3(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_3_DATA_0) then
                    int_weight_3(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_3(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_3(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_3_DATA_1) then
                    int_weight_3(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_3(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_4(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_4_DATA_0) then
                    int_weight_4(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_4(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_4(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_4_DATA_1) then
                    int_weight_4(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_4(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_5(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_5_DATA_0) then
                    int_weight_5(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_5(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_5(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_5_DATA_1) then
                    int_weight_5(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_5(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_6(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_6_DATA_0) then
                    int_weight_6(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_6(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_6(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_6_DATA_1) then
                    int_weight_6(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_6(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_7(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_7_DATA_0) then
                    int_weight_7(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_7(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_weight_7(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_WEIGHT_7_DATA_1) then
                    int_weight_7(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_weight_7(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bias(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_BIAS_DATA_0) then
                    int_bias(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_bias(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bias(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_BIAS_DATA_1) then
                    int_bias(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_bias(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_conv_scratch(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_CONV_SCRATCH_DATA_0) then
                    int_conv_scratch(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_conv_scratch(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_conv_scratch(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_CONV_SCRATCH_DATA_1) then
                    int_conv_scratch(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_conv_scratch(63 downto 32));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ofmap(31 downto 0) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_OFMAP_DATA_0) then
                    int_ofmap(31 downto 0) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ofmap(31 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ofmap(63 downto 32) <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_OFMAP_DATA_1) then
                    int_ofmap(63 downto 32) <= (UNSIGNED(WDATA(31 downto 0)) and wmask(31 downto 0)) or ((not wmask(31 downto 0)) and int_ofmap(63 downto 32));
                end if;
            end if;
        end if;
    end process;


-- ----------------------- Memory logic ------------------

end architecture behave;
