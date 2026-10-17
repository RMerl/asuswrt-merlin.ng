// SPDX-License-Identifier: GPL-2.0+
/*
 * J721E specific clock platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Dave Gerlach <d-gerlach@ti.com>.
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/clk-provider.h>
#include "k3-clk.h"

static const char * const gluelogic_hfosc0_clkout_parents[] = {
	"osc_19_2_mhz",
	"osc_20_mhz",
	"osc_24_mhz",
	"osc_25_mhz",
	"osc_26_mhz",
	"osc_27_mhz",
};

static const char * const mcu_ospi0_iclk_sel_out0_parents[] = {
	"board_0_mcu_ospi0_dqs_out",
	"fss_mcu_0_ospi_0_ospi_oclk_clk",
};

static const char * const mcu_ospi1_iclk_sel_out0_parents[] = {
	"board_0_mcu_ospi1_dqs_out",
	"fss_mcu_0_ospi_1_ospi_oclk_clk",
};

static const char * const wkup_fref_clksel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
};

static const char * const main_pll_hfosc_sel_out1_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents[] = {
	"wkup_fref_clksel_out0",
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
};

static const char * const mcu_ospi_ref_clk_sel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
};

static const char * const mcu_ospi_ref_clk_sel_out1_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
};

static const char * const mcuusart_clk_sel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"postdiv3_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_i2c0_mcupll_bypass_clksel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"gluelogic_hfosc0_clkout",
};

static const char * const main_pll25_hfosc_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out12_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out13_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out14_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out15_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out16_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out17_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out18_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out19_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out2_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out23_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out3_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out4_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out5_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out6_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out7_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out8_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const usb0_refclk_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const usb1_refclk_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const wkup_obsclk_mux_out0_parents[] = {
	"j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
	NULL,
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
	"hsdiv4_16fft_mcu_1_hsdivout1_clk",
	"hsdiv4_16fft_mcu_1_hsdivout2_clk",
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
	"j7_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk",
	"hsdiv4_16fft_mcu_2_hsdivout1_clk",
	"hsdiv4_16fft_mcu_2_hsdivout2_clk",
	"hsdiv4_16fft_mcu_2_hsdivout3_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
	"gluelogic_hfosc0_clkout",
	"gluelogic_lpxosc_clkout",
};

static const char * const main_pll15_xref_sel_out0_parents[] = {
	"main_pll_hfosc_sel_out15",
	"board_0_ext_refclk1_out",
};

static const char * const main_pll24_hfosc_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_mlb0_mlbcp_out",
};

static const char * const main_pll4_xref_sel_out0_parents[] = {
	"main_pll_hfosc_sel_out4",
	"board_0_ext_refclk1_out",
};

static const char * const mcu_clkout_mux_out0_parents[] = {
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
};

static const char * const k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents[] = {
	"main_pll_hfosc_sel_out0",
	"hsdiv4_16fft_main_0_hsdivout0_clk",
};

static const char * const mcu_obsclk_outmux_out0_parents[] = {
	"mcu_obsclk_div_out0",
	"gluelogic_hfosc0_clkout",
};

static const char * const obsclk1_mux_out0_parents[] = {
	"hsdiv0_16fft_main_7_hsdivout0_clk",
	"hsdiv0_16fft_main_8_hsdivout0_clk",
	"hsdiv3_16fft_main_13_hsdivout0_clk",
	NULL,
};

static const char * const clkout_mux_out0_parents[] = {
	"hsdiv4_16fft_main_3_hsdivout0_clk",
	"hsdiv4_16fft_main_3_hsdivout0_clk",
};

static const char * const emmcsd_refclk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const emmcsd_refclk_sel_out1_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const gtc_clk_mux_out0_parents[] = {
	"hsdiv4_16fft_main_3_hsdivout1_clk",
	"postdiv3_16fft_main_0_hsdivout6_clk",
	"board_0_mcu_cpts0_rft_clk_out",
	"board_0_cpts0_rft_clk_out",
	"board_0_mcu_ext_refclk0_out",
	"board_0_ext_refclk1_out",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv4_16fft_mcu_2_hsdivout1_clk",
	"k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk",
};

static const char * const gpmc_fclk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout3_clk",
	"hsdiv4_16fft_main_2_hsdivout1_clk",
	"hsdiv4_16fft_main_2_hsdivout1_clk",
	"k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk",
};

static const char * const mcasp_ahclko_mux_out0_parents[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv3_16fft_main_4_hsdivout2_clk",
	"hsdiv3_16fft_main_15_hsdivout2_clk",
	NULL,
	NULL,
	"board_0_audio_ext_refclk0_out",
};

static const char * const mcasp_ahclko_mux_out1_parents[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv3_16fft_main_4_hsdivout2_clk",
	"hsdiv3_16fft_main_15_hsdivout2_clk",
	NULL,
	NULL,
	"board_0_audio_ext_refclk1_out",
};

static const char * const mcasp_ahclko_mux_out2_parents[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv3_16fft_main_4_hsdivout2_clk",
	"hsdiv3_16fft_main_15_hsdivout2_clk",
	NULL,
	NULL,
	"board_0_audio_ext_refclk2_out",
};

static const char * const mcasp_ahclko_mux_out3_parents[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv3_16fft_main_4_hsdivout2_clk",
	"hsdiv3_16fft_main_15_hsdivout2_clk",
	NULL,
	NULL,
	"board_0_audio_ext_refclk3_out",
};

static const char * const obsclk0_mux_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout0_clk",
	"hsdiv4_16fft_main_1_hsdivout0_clk",
	"hsdiv4_16fft_main_2_hsdivout0_clk",
	"hsdiv4_16fft_main_3_hsdivout0_clk",
	"hsdiv3_16fft_main_4_hsdivout0_clk",
	"hsdiv3_16fft_main_5_hsdivout0_clk",
	"hsdiv0_16fft_main_6_hsdivout0_clk",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv0_16fft_main_12_hsdivout0_clk",
	"obsclk1_mux_out0",
	"hsdiv1_16fft_main_14_hsdivout0_clk",
	"hsdiv3_16fft_main_15_hsdivout0_clk",
	"hsdiv1_16fft_main_16_hsdivout0_clk",
	"hsdiv1_16fft_main_17_hsdivout0_clk",
	"hsdiv1_16fft_main_18_hsdivout0_clk",
	"hsdiv1_16fft_main_19_hsdivout0_clk",
	NULL,
	NULL,
	NULL,
	"hsdiv1_16fft_main_23_hsdivout0_clk",
	"hsdiv0_16fft_main_24_hsdivout0_clk",
	"hsdiv1_16fft_main_25_hsdivout0_clk",
	NULL,
	"j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
	"gluelogic_lpxosc_clkout",
	"hsdiv4_16fft_main_0_hsdivout0_clk",
	"board_0_hfosc1_clk_out",
	"gluelogic_hfosc0_clkout",
};

static const struct clk_data clk_list[] = {
	CLK_FIXED_RATE("osc_27_mhz", 27000000, 0),
	CLK_FIXED_RATE("osc_26_mhz", 26000000, 0),
	CLK_FIXED_RATE("osc_25_mhz", 25000000, 0),
	CLK_FIXED_RATE("osc_24_mhz", 24000000, 0),
	CLK_FIXED_RATE("osc_20_mhz", 20000000, 0),
	CLK_FIXED_RATE("osc_19_2_mhz", 19200000, 0),
	CLK_MUX("gluelogic_hfosc0_clkout", gluelogic_hfosc0_clkout_parents, 6, 0x43000030, 0, 3, 0),
	CLK_FIXED_RATE("board_0_hfosc1_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ospi0_dqs_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ospi1_dqs_out", 0, 0),
	CLK_FIXED_RATE("board_0_wkup_i2c0_scl_out", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_ospi_0_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_ospi_1_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk", 12500000, 0),
	CLK_MUX("mcu_ospi0_iclk_sel_out0", mcu_ospi0_iclk_sel_out0_parents, 2, 0x40f08030, 4, 1, 0),
	CLK_MUX("mcu_ospi1_iclk_sel_out0", mcu_ospi1_iclk_sel_out0_parents, 2, 0x40f08034, 4, 1, 0),
	CLK_FIXED_RATE("mshsi2c_wkup_0_porscl", 0, 0),
	CLK_MUX("wkup_fref_clksel_out0", wkup_fref_clksel_out0_parents, 2, 0x43008050, 8, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out1", main_pll_hfosc_sel_out1_parents, 2, 0x43008084, 0, 1, 0),
	CLK_PLL_DEFFREQ("pllfrac2_ssmod_16fft_main_1_foutvcop_clk", "main_pll_hfosc_sel_out1", 0x681000, 0, 1920000000),
	CLK_DIV("pllfrac2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", "pllfrac2_ssmod_16fft_main_1_foutvcop_clk", 0x681038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfrac2_ssmod_16fft_main_1_foutpostdiv_clk", "pllfrac2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", 0x681038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfrac2_ssmod_16fft_mcu_0_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d00000, 0),
	CLK_PLL_DEFFREQ("pllfrac2_ssmod_16fft_mcu_1_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d01000, 0, 2400000000),
	CLK_PLL_DEFFREQ("pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d02000, 0, 2000000000),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout5_clk", "pllfrac2_ssmod_16fft_main_1_foutpostdiv_clk", 0x681094, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_mcu_0_hsdivout0_clk", "pllfrac2_ssmod_16fft_mcu_0_foutvcop_clk", 0x40d00080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout3_clk", "pllfrac2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d0108c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout4_clk", "pllfrac2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_mcu_2_hsdivout4_clk", "pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02090, 0, 7, 0, 0, 166666666),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents, 2, 0x42010000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x42010118, 0, 5, 0, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out0", mcu_ospi_ref_clk_sel_out0_parents, 2, 0x40f08030, 0, 1, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out1", mcu_ospi_ref_clk_sel_out1_parents, 2, 0x40f08034, 0, 1, 0),
	CLK_MUX("mcuusart_clk_sel_out0", mcuusart_clk_sel_out0_parents, 2, 0x40f081c0, 0, 1, 0),
	CLK_MUX("wkup_i2c0_mcupll_bypass_clksel_out0", wkup_i2c0_mcupll_bypass_clksel_out0_parents, 2, 0x43008060, 0, 1, 0),
	CLK_FIXED_RATE("gluelogic_lpxosc_clkout", 32768, 0),
	CLK_MUX("main_pll25_hfosc_sel_out0", main_pll25_hfosc_sel_out0_parents, 2, 0x430080e4, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out0", main_pll_hfosc_sel_out0_parents, 2, 0x43008080, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out12", main_pll_hfosc_sel_out12_parents, 2, 0x430080b0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out13", main_pll_hfosc_sel_out13_parents, 2, 0x430080b4, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out14", main_pll_hfosc_sel_out14_parents, 2, 0x430080b8, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out15", main_pll_hfosc_sel_out15_parents, 2, 0x430080bc, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out16", main_pll_hfosc_sel_out16_parents, 2, 0x430080c0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out17", main_pll_hfosc_sel_out17_parents, 2, 0x430080c4, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out18", main_pll_hfosc_sel_out18_parents, 2, 0x430080c8, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out19", main_pll_hfosc_sel_out19_parents, 2, 0x430080cc, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out2", main_pll_hfosc_sel_out2_parents, 2, 0x43008088, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out23", main_pll_hfosc_sel_out23_parents, 2, 0x430080dc, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out3", main_pll_hfosc_sel_out3_parents, 2, 0x4300808c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out4", main_pll_hfosc_sel_out4_parents, 2, 0x43008090, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out5", main_pll_hfosc_sel_out5_parents, 2, 0x43008094, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out6", main_pll_hfosc_sel_out6_parents, 2, 0x43008098, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out7", main_pll_hfosc_sel_out7_parents, 2, 0x4300809c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out8", main_pll_hfosc_sel_out8_parents, 2, 0x430080a0, 0, 1, 0),
	CLK_MUX("usb0_refclk_sel_out0", usb0_refclk_sel_out0_parents, 2, 0x1080e0, 0, 1, 0),
	CLK_MUX("usb1_refclk_sel_out0", usb1_refclk_sel_out0_parents, 2, 0x1080e4, 0, 1, 0),
	CLK_FIXED_RATE("board_0_audio_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_audio_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_audio_ext_refclk2_out", 0, 0),
	CLK_FIXED_RATE("board_0_audio_ext_refclk3_out", 0, 0),
	CLK_FIXED_RATE("board_0_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_mlb0_mlbcp_out", 0, 0),
	CLK_FIXED_RATE("ddr32ss_16ffc_ew_dv_wrap_main_0_ddrss_io_ck", 0, 0),
	CLK_FIXED_RATE("ddr32ss_16ffc_ew_dv_wrap_main_0_ddrss_io_ck_n", 0, 0),
	CLK_FIXED_RATE("emmc8ss_16ffc_main_0_emmcss_io_clk", 0, 0),
	CLK_FIXED_RATE("emmcsd4ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_main_1_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_1_foutvcop_clk", 0x681080, 0, 7, 0, 0, 192000000),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_1_foutvcop_clk", 0x681088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout1_clk", "pllfrac2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout2_clk", "pllfrac2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout0_clk", "pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout1_clk", "pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout2_clk", "pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout3_clk", "pllfrac2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d0208c, 0, 7, 0, 0),
	CLK_FIXED_RATE("j7_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk", 32000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_0_foutvcop_clk", "main_pll_hfosc_sel_out0", 0x680000, 0),
	CLK_DIV("pllfrac2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x680038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfrac2_ssmod_16fft_main_0_foutpostdiv_clk", "pllfrac2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", 0x680038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfrac2_ssmod_16fft_main_13_foutvcop_clk", "main_pll_hfosc_sel_out13", 0x68d000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_14_foutvcop_clk", "main_pll_hfosc_sel_out14", 0x68e000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_16_foutvcop_clk", "main_pll_hfosc_sel_out16", 0x690000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_17_foutvcop_clk", "main_pll_hfosc_sel_out17", 0x691000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_18_foutvcop_clk", "main_pll_hfosc_sel_out18", 0x692000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_19_foutvcop_clk", "main_pll_hfosc_sel_out19", 0x693000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_2_foutvcop_clk", "main_pll_hfosc_sel_out2", 0x682000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_23_foutvcop_clk", "main_pll_hfosc_sel_out23", 0x697000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_25_foutvcop_clk", "main_pll25_hfosc_sel_out0", 0x699000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_3_foutvcop_clk", "main_pll_hfosc_sel_out3", 0x683000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_5_foutvcop_clk", "main_pll_hfosc_sel_out5", 0x685000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_6_foutvcop_clk", "main_pll_hfosc_sel_out6", 0x686000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_7_foutvcop_clk", "main_pll_hfosc_sel_out7", 0x687000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_8_foutvcop_clk", "main_pll_hfosc_sel_out8", 0x688000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_12_foutvcop_clk", "main_pll_hfosc_sel_out12", 0x68c000, 0),
	CLK_DIV("postdiv3_16fft_main_0_hsdivout6_clk", "pllfrac2_ssmod_16fft_main_0_foutpostdiv_clk", 0x680098, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout7_clk", "pllfrac2_ssmod_16fft_main_1_foutpostdiv_clk", 0x68109c, 0, 7, 0, 0),
	CLK_MUX("wkup_obsclk_mux_out0", wkup_obsclk_mux_out0_parents, 16, 0x43008000, 0, 4, 0),
	CLK_MUX("main_pll15_xref_sel_out0", main_pll15_xref_sel_out0_parents, 2, 0x430080bc, 4, 1, 0),
	CLK_MUX("main_pll24_hfosc_sel_out0", main_pll24_hfosc_sel_out0_parents, 2, 0x430080e0, 0, 1, 0),
	CLK_MUX("main_pll4_xref_sel_out0", main_pll4_xref_sel_out0_parents, 2, 0x43008090, 4, 1, 0),
	CLK_MUX("mcu_clkout_mux_out0", mcu_clkout_mux_out0_parents, 2, 0x40f08010, 0, 1, 0),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out0", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c0, 0, 2, 0, 0, 48000000),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out2", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c8, 0, 2, 0, 0, 48000000),
	CLK_DIV("hsdiv0_16fft_main_12_hsdivout0_clk", "pllfracf_ssmod_16fft_main_12_foutvcop_clk", 0x68c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_6_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_6_foutvcop_clk", 0x686080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_7_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_7_foutvcop_clk", 0x687080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_8_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_8_foutvcop_clk", 0x688080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_14_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_14_foutvcop_clk", 0x68e080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_16_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_16_foutvcop_clk", 0x690080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_17_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_17_foutvcop_clk", 0x691080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_18_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_18_foutvcop_clk", 0x692080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_19_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_19_foutvcop_clk", 0x693080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_23_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_23_foutvcop_clk", 0x697080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_25_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_25_foutvcop_clk", 0x699080, 0, 7, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_13_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_13_foutvcop_clk", 0x68d080, 0, 7, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_5_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_5_foutvcop_clk", 0x685080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x680080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout1_clk", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x680084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x680088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout3_clk", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x68008c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout4_clk", "pllfrac2_ssmod_16fft_main_0_foutvcop_clk", 0x680090, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_2_foutvcop_clk", 0x682080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout1_clk", "pllfrac2_ssmod_16fft_main_2_foutvcop_clk", 0x682084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_2_foutvcop_clk", 0x682088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_3_foutvcop_clk", 0x683080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout1_clk", "pllfrac2_ssmod_16fft_main_3_foutvcop_clk", 0x683084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_3_foutvcop_clk", 0x683088, 0, 7, 0, 0),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_main_0_sysclkout_clk", k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents, 2, 0x410000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x410118, 0, 5, 0, 0),
	CLK_DIV("mcu_obsclk_div_out0", "wkup_obsclk_mux_out0", 0x43008000, 8, 4, 0, 0),
	CLK_MUX("mcu_obsclk_outmux_out0", mcu_obsclk_outmux_out0_parents, 2, 0x43008000, 24, 1, 0),
	CLK_MUX("obsclk1_mux_out0", obsclk1_mux_out0_parents, 4, 0x108004, 0, 2, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_15_foutvcop_clk", "main_pll15_xref_sel_out0", 0x68f000, 0),
	CLK_PLL("pllfrac2_ssmod_16fft_main_4_foutvcop_clk", "main_pll4_xref_sel_out0", 0x684000, 0),
	CLK_MUX("clkout_mux_out0", clkout_mux_out0_parents, 2, 0x108010, 0, 1, 0),
	CLK_MUX("emmcsd_refclk_sel_out0", emmcsd_refclk_sel_out0_parents, 4, 0x1080b0, 0, 2, 0),
	CLK_MUX("emmcsd_refclk_sel_out1", emmcsd_refclk_sel_out1_parents, 4, 0x1080b4, 0, 2, 0),
	CLK_MUX("gtc_clk_mux_out0", gtc_clk_mux_out0_parents, 16, 0x108030, 0, 4, 0),
	CLK_MUX("gpmc_fclk_sel_out0", gpmc_fclk_sel_out0_parents, 4, 0x1080d0, 0, 2, 0),
	CLK_DIV("hsdiv0_16fft_main_24_hsdivout0_clk", "plldeskew_16fft_main_24_foutp_clk", 0x698080, 0, 0, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_15_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_15_foutvcop_clk", 0x68f080, 0, 7, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_15_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_15_foutvcop_clk", 0x68f088, 0, 7, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_4_hsdivout0_clk", "pllfrac2_ssmod_16fft_main_4_foutvcop_clk", 0x684080, 0, 7, 0, 0),
	CLK_DIV("hsdiv3_16fft_main_4_hsdivout2_clk", "pllfrac2_ssmod_16fft_main_4_foutvcop_clk", 0x684088, 0, 7, 0, 0),
	CLK_MUX("mcasp_ahclko_mux_out0", mcasp_ahclko_mux_out0_parents, 33, 0x1082e0, 0, 5, 0),
	CLK_MUX("mcasp_ahclko_mux_out1", mcasp_ahclko_mux_out1_parents, 33, 0x1082e4, 0, 5, 0),
	CLK_MUX("mcasp_ahclko_mux_out2", mcasp_ahclko_mux_out2_parents, 33, 0x1082e8, 0, 5, 0),
	CLK_MUX("mcasp_ahclko_mux_out3", mcasp_ahclko_mux_out3_parents, 33, 0x1082ec, 0, 5, 0),
	CLK_MUX("obsclk0_mux_out0", obsclk0_mux_out0_parents, 32, 0x108000, 0, 5, 0),
	CLK_DIV("osbclk0_div_out0", "obsclk0_mux_out0", 0x108000, 8, 8, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x41011c, 0, 5, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x4201011c, 0, 5, 0, 0),
};

static const struct dev_clk soc_dev_clk_data[] = {
	DEV_CLK(4, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(4, 1, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(4, 2, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(30, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(30, 1, "board_0_hfosc1_clk_out"),
	DEV_CLK(30, 2, "hsdiv4_16fft_main_0_hsdivout3_clk"),
	DEV_CLK(30, 4, "hsdiv4_16fft_main_0_hsdivout1_clk"),
	DEV_CLK(30, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(30, 6, "gluelogic_hfosc0_clkout"),
	DEV_CLK(30, 7, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(30, 8, "hsdiv4_16fft_main_0_hsdivout4_clk"),
	DEV_CLK(30, 9, "gluelogic_hfosc0_clkout"),
	DEV_CLK(30, 10, "board_0_hfosc1_clk_out"),
	DEV_CLK(30, 11, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(30, 12, "j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(47, 0, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(47, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(47, 2, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(47, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(61, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(61, 1, "gtc_clk_mux_out0"),
	DEV_CLK(61, 2, "hsdiv4_16fft_main_3_hsdivout1_clk"),
	DEV_CLK(61, 3, "postdiv3_16fft_main_0_hsdivout6_clk"),
	DEV_CLK(61, 4, "board_0_mcu_cpts0_rft_clk_out"),
	DEV_CLK(61, 5, "board_0_cpts0_rft_clk_out"),
	DEV_CLK(61, 6, "board_0_mcu_ext_refclk0_out"),
	DEV_CLK(61, 7, "board_0_ext_refclk1_out"),
	DEV_CLK(61, 16, "hsdiv4_16fft_mcu_2_hsdivout1_clk"),
	DEV_CLK(61, 17, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(91, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(91, 1, "emmcsd_refclk_sel_out0"),
	DEV_CLK(91, 2, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(91, 3, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(91, 4, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(91, 5, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(92, 0, "emmcsd_refclk_sel_out1"),
	DEV_CLK(92, 1, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(92, 2, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(92, 3, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(92, 4, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(92, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(92, 6, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(102, 0, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 1, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(102, 3, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 4, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(103, 0, "mcu_ospi_ref_clk_sel_out0"),
	DEV_CLK(103, 1, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(103, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(103, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(103, 4, "mcu_ospi0_iclk_sel_out0"),
	DEV_CLK(103, 5, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(103, 6, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(103, 7, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(103, 8, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(104, 0, "mcu_ospi_ref_clk_sel_out1"),
	DEV_CLK(104, 1, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(104, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(104, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(104, 4, "mcu_ospi1_iclk_sel_out0"),
	DEV_CLK(104, 5, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(104, 6, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(104, 7, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(104, 8, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(113, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(133, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(133, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(138, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(138, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(146, 0, "usart_programmable_clock_divider_out0"),
	DEV_CLK(146, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(149, 0, "mcuusart_clk_sel_out0"),
	DEV_CLK(149, 1, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(149, 2, "postdiv3_16fft_main_1_hsdivout5_clk"),
	DEV_CLK(149, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(154, 0, "j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(154, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(154, 2, "gluelogic_hfosc0_clkout"),
	DEV_CLK(157, 18, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 19, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 21, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(157, 22, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(157, 42, "mshsi2c_wkup_0_porscl"),
	DEV_CLK(157, 50, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p"),
	DEV_CLK(157, 51, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n"),
	DEV_CLK(157, 91, "ddr32ss_16ffc_ew_dv_wrap_main_0_ddrss_io_ck"),
	DEV_CLK(157, 92, "ddr32ss_16ffc_ew_dv_wrap_main_0_ddrss_io_ck_n"),
	DEV_CLK(157, 99, "emmc8ss_16ffc_main_0_emmcss_io_clk"),
	DEV_CLK(157, 100, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 104, "gpmc_fclk_sel_out0"),
	DEV_CLK(157, 109, "hsdiv1_16fft_main_19_hsdivout0_clk"),
	DEV_CLK(157, 111, "hsdiv1_16fft_main_23_hsdivout0_clk"),
	DEV_CLK(157, 113, "osbclk0_div_out0"),
	DEV_CLK(157, 114, "hsdiv4_16fft_main_0_hsdivout0_clk"),
	DEV_CLK(157, 115, "hsdiv4_16fft_main_1_hsdivout0_clk"),
	DEV_CLK(157, 116, "hsdiv4_16fft_main_2_hsdivout0_clk"),
	DEV_CLK(157, 117, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 118, "hsdiv3_16fft_main_4_hsdivout0_clk"),
	DEV_CLK(157, 119, "hsdiv3_16fft_main_5_hsdivout0_clk"),
	DEV_CLK(157, 120, "hsdiv0_16fft_main_6_hsdivout0_clk"),
	DEV_CLK(157, 126, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(157, 127, "obsclk1_mux_out0"),
	DEV_CLK(157, 128, "hsdiv1_16fft_main_14_hsdivout0_clk"),
	DEV_CLK(157, 129, "hsdiv3_16fft_main_15_hsdivout0_clk"),
	DEV_CLK(157, 130, "hsdiv1_16fft_main_16_hsdivout0_clk"),
	DEV_CLK(157, 131, "hsdiv1_16fft_main_17_hsdivout0_clk"),
	DEV_CLK(157, 132, "hsdiv1_16fft_main_18_hsdivout0_clk"),
	DEV_CLK(157, 133, "hsdiv1_16fft_main_19_hsdivout0_clk"),
	DEV_CLK(157, 137, "hsdiv1_16fft_main_23_hsdivout0_clk"),
	DEV_CLK(157, 138, "hsdiv0_16fft_main_24_hsdivout0_clk"),
	DEV_CLK(157, 139, "hsdiv1_16fft_main_25_hsdivout0_clk"),
	DEV_CLK(157, 141, "j7_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(157, 142, "gluelogic_lpxosc_clkout"),
	DEV_CLK(157, 143, "hsdiv4_16fft_main_0_hsdivout0_clk"),
	DEV_CLK(157, 144, "board_0_hfosc1_clk_out"),
	DEV_CLK(157, 145, "gluelogic_hfosc0_clkout"),
	DEV_CLK(157, 146, "obsclk1_mux_out0"),
	DEV_CLK(157, 147, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(157, 148, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(157, 149, "hsdiv3_16fft_main_13_hsdivout0_clk"),
	DEV_CLK(157, 152, "mcu_obsclk_outmux_out0"),
	DEV_CLK(157, 153, "mcu_obsclk_div_out0"),
	DEV_CLK(157, 154, "gluelogic_hfosc0_clkout"),
	DEV_CLK(157, 169, "k3_pll_ctrl_wrap_main_0_sysclkout_clk"),
	DEV_CLK(157, 170, "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk"),
	DEV_CLK(157, 172, "clkout_mux_out0"),
	DEV_CLK(157, 173, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 174, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 175, "mcu_clkout_mux_out0"),
	DEV_CLK(157, 176, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 177, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 301, "mcasp_ahclko_mux_out0"),
	DEV_CLK(157, 330, "hsdiv3_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 331, "hsdiv3_16fft_main_15_hsdivout2_clk"),
	DEV_CLK(157, 334, "board_0_audio_ext_refclk0_out"),
	DEV_CLK(157, 336, "mcasp_ahclko_mux_out1"),
	DEV_CLK(157, 365, "hsdiv3_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 366, "hsdiv3_16fft_main_15_hsdivout2_clk"),
	DEV_CLK(157, 369, "board_0_audio_ext_refclk1_out"),
	DEV_CLK(157, 371, "mcasp_ahclko_mux_out2"),
	DEV_CLK(157, 400, "hsdiv3_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 401, "hsdiv3_16fft_main_15_hsdivout2_clk"),
	DEV_CLK(157, 404, "board_0_audio_ext_refclk2_out"),
	DEV_CLK(157, 406, "mcasp_ahclko_mux_out3"),
	DEV_CLK(157, 435, "hsdiv3_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 436, "hsdiv3_16fft_main_15_hsdivout2_clk"),
	DEV_CLK(157, 439, "board_0_audio_ext_refclk3_out"),
	DEV_CLK(197, 0, "wkup_i2c0_mcupll_bypass_clksel_out0"),
	DEV_CLK(197, 1, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(197, 2, "gluelogic_hfosc0_clkout"),
	DEV_CLK(197, 3, "board_0_wkup_i2c0_scl_out"),
	DEV_CLK(197, 4, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(202, 2, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(203, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(279, 0, "usart_programmable_clock_divider_out2"),
	DEV_CLK(279, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 3, "postdiv3_16fft_main_1_hsdivout7_clk"),
	DEV_CLK(288, 4, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 15, "usb0_refclk_sel_out0"),
	DEV_CLK(288, 16, "gluelogic_hfosc0_clkout"),
	DEV_CLK(288, 17, "board_0_hfosc1_clk_out"),
	DEV_CLK(288, 18, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 19, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(289, 3, "postdiv3_16fft_main_1_hsdivout7_clk"),
	DEV_CLK(289, 4, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(289, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(289, 15, "usb1_refclk_sel_out0"),
	DEV_CLK(289, 16, "gluelogic_hfosc0_clkout"),
	DEV_CLK(289, 17, "board_0_hfosc1_clk_out"),
	DEV_CLK(289, 18, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(289, 19, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
};

const struct ti_k3_clk_platdata j721e_clk_platdata = {
	.clk_list = clk_list,
	.clk_list_cnt = 157,
	.soc_dev_clk_data = soc_dev_clk_data,
	.soc_dev_clk_data_cnt = 173,
};
