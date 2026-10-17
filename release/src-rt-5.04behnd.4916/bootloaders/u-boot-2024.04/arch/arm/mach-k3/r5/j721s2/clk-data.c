// SPDX-License-Identifier: GPL-2.0+
/*
 * J721S2 specific clock platform data
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
	NULL,
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

static const char * const mcu_usart_clksel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"postdiv3_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_i2c_mcupll_bypass_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"gluelogic_hfosc0_clkout",
};

static const char * const main_pll_hfosc_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out12_parents[] = {
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

static const char * const main_pll_hfosc_sel_out26_0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out3_parents[] = {
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

static const char * const emmcsd1_lb_clksel_out0_parents[] = {
	"board_0_mmc1_clklb_out",
	"board_0_mmc1_clk_out",
};

static const char * const mcu_clkout_mux_out0_parents[] = {
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
};

static const char * const k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents[] = {
	"main_pll_hfosc_sel_out0",
	"hsdiv4_16fft_main_0_hsdivout0_clk",
};

static const char * const dpi0_ext_clksel_out0_parents[] = {
	"hsdiv1_16fft_main_19_hsdivout0_clk",
	"board_0_vout0_extpclkin_out",
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
	CLK_MUX("mcu_ospi0_iclk_sel_out0", mcu_ospi0_iclk_sel_out0_parents, 2, 0x40f08030, 4, 1, 0),
	CLK_MUX("mcu_ospi1_iclk_sel_out0", mcu_ospi1_iclk_sel_out0_parents, 2, 0x40f08034, 4, 1, 0),
	CLK_FIXED_RATE("mshsi2c_wkup_0_porscl", 0, 0),
	CLK_MUX("wkup_fref_clksel_out0", wkup_fref_clksel_out0_parents, 2, 0x43008050, 8, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out1", main_pll_hfosc_sel_out1_parents, 2, 0x43008084, 0, 1, 0),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_main_1_foutvcop_clk", "main_pll_hfosc_sel_out1", 0x681000, 0, 1920000000),
	CLK_DIV("pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", 0x681038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf2_ssmod_16fft_mcu_0_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d00000, 0),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d01000, 0, 2400000000),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d02000, 0, 2000000000),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout5_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", 0x681094, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_mcu_0_hsdivout0_clk", "pllfracf2_ssmod_16fft_mcu_0_foutvcop_clk", 0x40d00080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout3_clk", "pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d0108c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout4_clk", "pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_mcu_2_hsdivout4_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02090, 0, 7, 0, 0, 166666666),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents, 2, 0x42010000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x42010118, 0, 5, 0, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out0", mcu_ospi_ref_clk_sel_out0_parents, 2, 0x40f08030, 0, 1, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out1", mcu_ospi_ref_clk_sel_out1_parents, 2, 0x40f08034, 0, 1, 0),
	CLK_MUX("mcu_usart_clksel_out0", mcu_usart_clksel_out0_parents, 2, 0x40f081c0, 0, 1, 0),
	CLK_MUX("wkup_i2c_mcupll_bypass_out0", wkup_i2c_mcupll_bypass_out0_parents, 2, 0x43008060, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out0", main_pll_hfosc_sel_out0_parents, 2, 0x43008080, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out12", main_pll_hfosc_sel_out12_parents, 2, 0x430080b0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out19", main_pll_hfosc_sel_out19_parents, 2, 0x430080cc, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out2", main_pll_hfosc_sel_out2_parents, 2, 0x43008088, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out26_0", main_pll_hfosc_sel_out26_0_parents, 2, 0x430080e8, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out3", main_pll_hfosc_sel_out3_parents, 2, 0x4300808c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out7", main_pll_hfosc_sel_out7_parents, 2, 0x4300809c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out8", main_pll_hfosc_sel_out8_parents, 2, 0x430080a0, 0, 1, 0),
	CLK_MUX("usb0_refclk_sel_out0", usb0_refclk_sel_out0_parents, 2, 0x1080e0, 0, 1, 0),
	CLK_FIXED_RATE("board_0_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ddr0_ckn_out", 0, 0),
	CLK_FIXED_RATE("board_0_ddr0_ckp_out", 0, 0),
	CLK_FIXED_RATE("board_0_ddr1_ckn_out", 0, 0),
	CLK_FIXED_RATE("board_0_ddr1_ckp_out", 0, 0),
	CLK_FIXED_RATE("board_0_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clklb_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_vout0_extpclkin_out", 0, 0),
	CLK_FIXED_RATE("emmc8ss_16ffc_main_0_emmcss_io_clk", 0, 0),
	CLK_FIXED_RATE("emmcsd4ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_main_1_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681080, 0, 7, 0, 0, 192000000),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout0_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout1_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02084, 0, 7, 0, 0),
	CLK_FIXED_RATE("j7am_ddr_ew_wrap_dv_wrap_main_0_ddrss_io_ck", 0, 0),
	CLK_FIXED_RATE("j7am_ddr_ew_wrap_dv_wrap_main_0_ddrss_io_ck_n", 0, 0),
	CLK_FIXED_RATE("j7am_ddr_ew_wrap_dv_wrap_main_1_ddrss_io_ck", 0, 0),
	CLK_FIXED_RATE("j7am_ddr_ew_wrap_dv_wrap_main_1_ddrss_io_ck_n", 0, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_0_foutvcop_clk", "main_pll_hfosc_sel_out0", 0x680000, 0),
	CLK_DIV("pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", 0x680038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf2_ssmod_16fft_main_12_foutvcop_clk", "main_pll_hfosc_sel_out12", 0x68c000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_19_foutvcop_clk", "main_pll_hfosc_sel_out19", 0x693000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_2_foutvcop_clk", "main_pll_hfosc_sel_out2", 0x682000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_26_foutvcop_clk", "main_pll_hfosc_sel_out26_0", 0x69a000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_3_foutvcop_clk", "main_pll_hfosc_sel_out3", 0x683000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_7_foutvcop_clk", "main_pll_hfosc_sel_out7", 0x687000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_8_foutvcop_clk", "main_pll_hfosc_sel_out8", 0x688000, 0),
	CLK_DIV("postdiv3_16fft_main_0_hsdivout6_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", 0x680098, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_0_hsdivout8_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", 0x6800a0, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout7_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", 0x68109c, 0, 7, 0, 0),
	CLK_MUX("emmcsd1_lb_clksel_out0", emmcsd1_lb_clksel_out0_parents, 2, 0x1080b4, 16, 1, 0),
	CLK_MUX("mcu_clkout_mux_out0", mcu_clkout_mux_out0_parents, 2, 0x40f08010, 0, 1, 0),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out0", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c0, 0, 2, 0, 0, 48000000),
	CLK_DIV("usart_programmable_clock_divider_out5", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081d4, 0, 2, 0, 0),
	CLK_DIV("usart_programmable_clock_divider_out8", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081e0, 0, 2, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_12_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_12_foutvcop_clk", 0x68c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_26_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_26_foutvcop_clk", 0x69a080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_7_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_7_foutvcop_clk", 0x687080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_8_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_8_foutvcop_clk", 0x688080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_19_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_19_foutvcop_clk", 0x693080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout3_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x68008c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout4_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680090, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_2_foutvcop_clk", 0x682088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout1_clk", "pllfracf2_ssmod_16fft_main_3_foutvcop_clk", 0x683084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_3_foutvcop_clk", 0x683088, 0, 7, 0, 0),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_main_0_sysclkout_clk", k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents, 2, 0x410000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x410118, 0, 5, 0, 0),
	CLK_MUX("dpi0_ext_clksel_out0", dpi0_ext_clksel_out0_parents, 2, 0x108300, 0, 1, 0),
	CLK_MUX("emmcsd_refclk_sel_out0", emmcsd_refclk_sel_out0_parents, 4, 0x1080b0, 0, 2, 0),
	CLK_MUX("emmcsd_refclk_sel_out1", emmcsd_refclk_sel_out1_parents, 4, 0x1080b4, 0, 2, 0),
	CLK_MUX("gtc_clk_mux_out0", gtc_clk_mux_out0_parents, 16, 0x108030, 0, 4, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x41011c, 0, 5, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x4201011c, 0, 5, 0, 0),
};

static const struct dev_clk soc_dev_clk_data[] = {
	DEV_CLK(4, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(4, 1, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(4, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(43, 0, "postdiv3_16fft_main_0_hsdivout8_clk"),
	DEV_CLK(43, 1, "hsdiv4_16fft_main_0_hsdivout3_clk"),
	DEV_CLK(43, 2, "gluelogic_hfosc0_clkout"),
	DEV_CLK(43, 3, "board_0_hfosc1_clk_out"),
	DEV_CLK(43, 5, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(43, 6, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(43, 7, "board_0_hfosc1_clk_out"),
	DEV_CLK(43, 9, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(43, 10, "hsdiv4_16fft_main_0_hsdivout4_clk"),
	DEV_CLK(43, 11, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(43, 12, "gluelogic_hfosc0_clkout"),
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
	DEV_CLK(98, 1, "emmcsd_refclk_sel_out0"),
	DEV_CLK(98, 2, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(98, 3, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(98, 4, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(98, 5, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(98, 6, "emmcsd1_lb_clksel_out0"),
	DEV_CLK(98, 7, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(99, 1, "emmcsd_refclk_sel_out1"),
	DEV_CLK(99, 2, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(99, 3, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(99, 4, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(99, 5, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(99, 7, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(99, 8, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(108, 1, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(108, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(108, 3, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(108, 6, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(108, 11, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(109, 0, "mcu_ospi0_iclk_sel_out0"),
	DEV_CLK(109, 1, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(109, 2, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(109, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(109, 5, "mcu_ospi_ref_clk_sel_out0"),
	DEV_CLK(109, 6, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(109, 7, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(109, 8, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(109, 9, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(110, 0, "mcu_ospi1_iclk_sel_out0"),
	DEV_CLK(110, 1, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(110, 2, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(110, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(110, 5, "mcu_ospi_ref_clk_sel_out1"),
	DEV_CLK(110, 6, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(110, 7, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(110, 8, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(110, 9, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(115, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(126, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(126, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(138, 0, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(138, 1, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(138, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(138, 3, "board_0_ddr0_ckn_out"),
	DEV_CLK(138, 5, "board_0_ddr0_ckp_out"),
	DEV_CLK(138, 7, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(139, 0, "hsdiv0_16fft_main_26_hsdivout0_clk"),
	DEV_CLK(139, 1, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(139, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(139, 3, "board_0_ddr1_ckn_out"),
	DEV_CLK(139, 5, "board_0_ddr1_ckp_out"),
	DEV_CLK(139, 7, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(143, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(143, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(146, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(146, 3, "usart_programmable_clock_divider_out0"),
	DEV_CLK(149, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(149, 3, "mcu_usart_clksel_out0"),
	DEV_CLK(149, 4, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(149, 5, "postdiv3_16fft_main_1_hsdivout5_clk"),
	DEV_CLK(157, 9, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 103, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 104, "j7am_ddr_ew_wrap_dv_wrap_main_0_ddrss_io_ck"),
	DEV_CLK(157, 111, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(157, 174, "j7am_ddr_ew_wrap_dv_wrap_main_1_ddrss_io_ck"),
	DEV_CLK(157, 177, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(157, 179, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 182, "mshsi2c_wkup_0_porscl"),
	DEV_CLK(157, 187, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(157, 194, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 197, "j7am_ddr_ew_wrap_dv_wrap_main_0_ddrss_io_ck_n"),
	DEV_CLK(157, 208, "j7am_ddr_ew_wrap_dv_wrap_main_1_ddrss_io_ck_n"),
	DEV_CLK(157, 214, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p"),
	DEV_CLK(157, 221, "mcu_clkout_mux_out0"),
	DEV_CLK(157, 222, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 223, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 225, "emmc8ss_16ffc_main_0_emmcss_io_clk"),
	DEV_CLK(157, 231, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n"),
	DEV_CLK(157, 352, "dpi0_ext_clksel_out0"),
	DEV_CLK(180, 0, "gluelogic_hfosc0_clkout"),
	DEV_CLK(180, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(202, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(203, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(223, 1, "wkup_i2c_mcupll_bypass_out0"),
	DEV_CLK(223, 2, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(223, 3, "gluelogic_hfosc0_clkout"),
	DEV_CLK(223, 4, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(223, 5, "board_0_wkup_i2c0_scl_out"),
	DEV_CLK(354, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(354, 3, "usart_programmable_clock_divider_out5"),
	DEV_CLK(357, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(357, 3, "usart_programmable_clock_divider_out8"),
	DEV_CLK(360, 4, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(360, 13, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(360, 15, "postdiv3_16fft_main_1_hsdivout7_clk"),
	DEV_CLK(360, 16, "usb0_refclk_sel_out0"),
	DEV_CLK(360, 17, "gluelogic_hfosc0_clkout"),
	DEV_CLK(360, 18, "board_0_hfosc1_clk_out"),
	DEV_CLK(360, 22, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(360, 23, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
};

const struct ti_k3_clk_platdata j721s2_clk_platdata = {
	.clk_list = clk_list,
	.clk_list_cnt = 105,
	.soc_dev_clk_data = soc_dev_clk_data,
	.soc_dev_clk_data_cnt = 124,
};
