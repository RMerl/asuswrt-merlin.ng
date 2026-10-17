// SPDX-License-Identifier: GPL-2.0+
/*
 * J7200 specific clock platform data
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

static const char * const wkup_fref_clksel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
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

static const char * const mcuusart_clk_sel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"postdiv2_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_gpio0_clksel_out0_parents[] = {
	"k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk",
	"k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk",
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk",
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
};

static const char * const wkup_i2c0_mcupll_bypass_clksel_out0_parents[] = {
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

static const char * const main_pll_hfosc_sel_out14_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out2_parents[] = {
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

static const char * const wkup_obsclk_mux_out0_parents[] = {
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
	NULL,
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
	"hsdiv4_16fft_mcu_1_hsdivout1_clk",
	"hsdiv4_16fft_mcu_1_hsdivout2_clk",
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk",
	"hsdiv4_16fft_mcu_2_hsdivout1_clk",
	"hsdiv4_16fft_mcu_2_hsdivout2_clk",
	"hsdiv4_16fft_mcu_2_hsdivout3_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
	"gluelogic_hfosc0_clkout",
	"board_0_wkup_lf_clkin_out",
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

static const char * const main_pll8_sel_extwave_out0_parents[] = {
	"pllfracf_ssmod_16fft_main_8_foutvcop_clk",
	"hsdiv0_16fft_main_8_hsdivout0_clk",
};

static const char * const mcu_obsclk_outmux_out0_parents[] = {
	"mcu_obsclk_div_out0",
	"gluelogic_hfosc0_clkout",
};

static const char * const clkout_mux_out0_parents[] = {
	"hsdiv4_16fft_main_3_hsdivout0_clk",
	"hsdiv4_16fft_main_3_hsdivout0_clk",
};

static const char * const emmcsd_refclk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const emmcsd_refclk_sel_out1_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const gtc_clk_mux_out0_parents[] = {
	"hsdiv4_16fft_main_3_hsdivout1_clk",
	"postdiv2_16fft_main_0_hsdivout6_clk",
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

static const char * const obsclk1_mux_out0_parents[] = {
	NULL,
	"hsdiv0_16fft_main_8_hsdivout0_clk",
	NULL,
	NULL,
};

static const char * const gpmc_fclk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout3_clk",
	"hsdiv4_16fft_main_2_hsdivout1_clk",
	"hsdiv4_16fft_main_2_hsdivout1_clk",
	"k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk",
};

static const char * const audio_refclko_mux_out0_parents[] = {
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
	"hsdiv2_16fft_main_4_hsdivout2_clk",
	NULL,
	NULL,
	NULL,
};

static const char * const audio_refclko_mux_out1_parents[] = {
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
	"hsdiv2_16fft_main_4_hsdivout2_clk",
	NULL,
	NULL,
	NULL,
};

static const char * const obsclk0_mux_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout0_clk",
	"hsdiv4_16fft_main_1_hsdivout0_clk",
	"hsdiv4_16fft_main_2_hsdivout0_clk",
	"hsdiv4_16fft_main_3_hsdivout0_clk",
	"hsdiv2_16fft_main_4_hsdivout0_clk",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv0_16fft_main_12_hsdivout0_clk",
	"obsclk1_mux_out0",
	"hsdiv1_16fft_main_14_hsdivout0_clk",
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
	"j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
	"board_0_wkup_lf_clkin_out",
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
	CLK_FIXED_RATE("board_0_wkup_i2c0_scl_out", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_ospi_0_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk", 12500000, 0),
	CLK_FIXED_RATE("j7vc_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk", 32550, 0),
	CLK_MUX("mcu_ospi0_iclk_sel_out0", mcu_ospi0_iclk_sel_out0_parents, 2, 0x40f08030, 4, 1, 0),
	CLK_FIXED_RATE("mshsi2c_wkup_0_porscl", 0, 0),
	CLK_MUX("wkup_fref_clksel_out0", wkup_fref_clksel_out0_parents, 2, 0x43008050, 8, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out1", main_pll_hfosc_sel_out1_parents, 2, 0x43008084, 0, 1, 0),
	CLK_PLL_DEFFREQ("pllfracf_ssmod_16fft_main_1_foutvcop_clk", "main_pll_hfosc_sel_out1", 0x681000, 0, 1920000000),
	CLK_DIV("pllfracf_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf_ssmod_16fft_main_1_foutpostdiv_clk", "pllfracf_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", 0x681038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf_ssmod_16fft_mcu_0_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d00000, 0),
	CLK_PLL_DEFFREQ("pllfracf_ssmod_16fft_mcu_1_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d01000, 0, 2400000000),
	CLK_PLL_DEFFREQ("pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d02000, 0, 2000000000),
	CLK_DIV("postdiv2_16fft_main_1_hsdivout5_clk", "pllfracf_ssmod_16fft_main_1_foutpostdiv_clk", 0x681094, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_mcu_0_hsdivout0_clk", "pllfracf_ssmod_16fft_mcu_0_foutvcop_clk", 0x40d00080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout3_clk", "pllfracf_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d0108c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout4_clk", "pllfracf_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_mcu_2_hsdivout4_clk", "pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02090, 0, 7, 0, 0, 166666666),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents, 2, 0x42010000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x42010118, 0, 5, 0, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out0", mcu_ospi_ref_clk_sel_out0_parents, 2, 0x40f08030, 0, 1, 0),
	CLK_MUX("mcuusart_clk_sel_out0", mcuusart_clk_sel_out0_parents, 2, 0x40f081c0, 0, 1, 0),
	CLK_MUX("wkup_gpio0_clksel_out0", wkup_gpio0_clksel_out0_parents, 4, 0x43008070, 0, 2, 0),
	CLK_MUX("wkup_i2c0_mcupll_bypass_clksel_out0", wkup_i2c0_mcupll_bypass_clksel_out0_parents, 2, 0x43008060, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out0", main_pll_hfosc_sel_out0_parents, 2, 0x43008080, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out12", main_pll_hfosc_sel_out12_parents, 2, 0x430080b0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out14", main_pll_hfosc_sel_out14_parents, 2, 0x430080b8, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out2", main_pll_hfosc_sel_out2_parents, 2, 0x43008088, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out3", main_pll_hfosc_sel_out3_parents, 2, 0x4300808c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out4", main_pll_hfosc_sel_out4_parents, 2, 0x43008090, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out7", main_pll_hfosc_sel_out7_parents, 2, 0x4300809c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out8", main_pll_hfosc_sel_out8_parents, 2, 0x430080a0, 0, 1, 0),
	CLK_MUX("usb0_refclk_sel_out0", usb0_refclk_sel_out0_parents, 2, 0x1080e0, 0, 1, 0),
	CLK_FIXED_RATE("board_0_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_i2c0_scl_out", 0, 0),
	CLK_FIXED_RATE("board_0_wkup_lf_clkin_out", 0, 0),
	CLK_FIXED_RATE("emmcsd4ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_main_1_hsdivout0_clk", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681080, 0, 7, 0, 0, 192000000),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout2_clk", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout1_clk", "pllfracf_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout2_clk", "pllfracf_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout0_clk", "pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout1_clk", "pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout2_clk", "pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout3_clk", "pllfracf_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d0208c, 0, 7, 0, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_0_foutvcop_clk", "main_pll_hfosc_sel_out0", 0x680000, 0),
	CLK_DIV("pllfracf_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", 0x680038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf_ssmod_16fft_main_12_foutvcop_clk", "main_pll_hfosc_sel_out12", 0x68c000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_14_foutvcop_clk", "main_pll_hfosc_sel_out14", 0x68e000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_2_foutvcop_clk", "main_pll_hfosc_sel_out2", 0x682000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_3_foutvcop_clk", "main_pll_hfosc_sel_out3", 0x683000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_7_foutvcop_clk", "main_pll_hfosc_sel_out7", 0x687000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_8_foutvcop_clk", "main_pll_hfosc_sel_out8", 0x688000, 0),
	CLK_DIV("postdiv2_16fft_main_0_hsdivout6_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", 0x680098, 0, 7, 0, 0),
	CLK_DIV("postdiv2_16fft_main_1_hsdivout7_clk", "pllfracf_ssmod_16fft_main_1_foutpostdiv_clk", 0x68109c, 0, 7, 0, 0),
	CLK_MUX("wkup_obsclk_mux_out0", wkup_obsclk_mux_out0_parents, 16, 0x43008000, 0, 4, 0),
	CLK_MUX("main_pll4_xref_sel_out0", main_pll4_xref_sel_out0_parents, 2, 0x43008090, 4, 1, 0),
	CLK_MUX("mcu_clkout_mux_out0", mcu_clkout_mux_out0_parents, 2, 0x40f08010, 0, 1, 0),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out0", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c0, 0, 2, 0, 0, 48000000),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out1", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c4, 0, 2, 0, 0, 48000000),
	CLK_DIV("hsdiv0_16fft_main_12_hsdivout0_clk", "pllfracf_ssmod_16fft_main_12_foutvcop_clk", 0x68c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_7_hsdivout0_clk", "pllfracf_ssmod_16fft_main_7_foutvcop_clk", 0x687080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_8_hsdivout0_clk", "pllfracf_ssmod_16fft_main_8_foutvcop_clk", 0x688080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_14_hsdivout0_clk", "pllfracf_ssmod_16fft_main_14_foutvcop_clk", 0x68e080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout0_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout1_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout2_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout3_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x68008c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout4_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680090, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout0_clk", "pllfracf_ssmod_16fft_main_2_foutvcop_clk", 0x682080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout1_clk", "pllfracf_ssmod_16fft_main_2_foutvcop_clk", 0x682084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout0_clk", "pllfracf_ssmod_16fft_main_3_foutvcop_clk", 0x683080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout1_clk", "pllfracf_ssmod_16fft_main_3_foutvcop_clk", 0x683084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout2_clk", "pllfracf_ssmod_16fft_main_3_foutvcop_clk", 0x683088, 0, 7, 0, 0),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_main_0_sysclkout_clk", k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents, 2, 0x410000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x410118, 0, 5, 0, 0),
	CLK_MUX("main_pll8_sel_extwave_out0", main_pll8_sel_extwave_out0_parents, 2, 0x688040, 0, 1, 0),
	CLK_DIV("mcu_obsclk_div_out0", "wkup_obsclk_mux_out0", 0x43008000, 8, 4, 0, 0),
	CLK_MUX("mcu_obsclk_outmux_out0", mcu_obsclk_outmux_out0_parents, 2, 0x43008000, 24, 1, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_4_foutvcop_clk", "main_pll4_xref_sel_out0", 0x684000, 0),
	CLK_MUX("clkout_mux_out0", clkout_mux_out0_parents, 2, 0x108010, 0, 1, 0),
	CLK_MUX("emmcsd_refclk_sel_out0", emmcsd_refclk_sel_out0_parents, 4, 0x1080b0, 0, 2, 0),
	CLK_MUX("emmcsd_refclk_sel_out1", emmcsd_refclk_sel_out1_parents, 4, 0x1080b4, 0, 2, 0),
	CLK_MUX("gtc_clk_mux_out0", gtc_clk_mux_out0_parents, 16, 0x108030, 0, 4, 0),
	CLK_MUX("obsclk1_mux_out0", obsclk1_mux_out0_parents, 4, 0x108004, 0, 2, 0),
	CLK_MUX("gpmc_fclk_sel_out0", gpmc_fclk_sel_out0_parents, 4, 0x1080d0, 0, 2, 0),
	CLK_DIV("hsdiv2_16fft_main_4_hsdivout0_clk", "pllfracf_ssmod_16fft_main_4_foutvcop_clk", 0x684080, 0, 7, 0, 0),
	CLK_DIV("hsdiv2_16fft_main_4_hsdivout2_clk", "pllfracf_ssmod_16fft_main_4_foutvcop_clk", 0x684088, 0, 7, 0, 0),
	CLK_MUX("audio_refclko_mux_out0", audio_refclko_mux_out0_parents, 32, 0x1082e0, 0, 5, 0),
	CLK_MUX("audio_refclko_mux_out1", audio_refclko_mux_out1_parents, 32, 0x1082e4, 0, 5, 0),
	CLK_MUX("obsclk0_mux_out0", obsclk0_mux_out0_parents, 32, 0x108000, 0, 5, 0),
	CLK_DIV("osbclk0_div_out0", "obsclk0_mux_out0", 0x108000, 8, 8, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x41011c, 0, 5, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x4201011c, 0, 5, 0, 0),
};

static const struct dev_clk soc_dev_clk_data[] = {
	DEV_CLK(4, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(4, 1, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(4, 2, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(8, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(8, 5, "hsdiv0_16fft_main_12_hsdivout0_clk"),
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
	DEV_CLK(30, 12, "j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(61, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(61, 1, "gtc_clk_mux_out0"),
	DEV_CLK(61, 2, "hsdiv4_16fft_main_3_hsdivout1_clk"),
	DEV_CLK(61, 3, "postdiv2_16fft_main_0_hsdivout6_clk"),
	DEV_CLK(61, 4, "board_0_mcu_cpts0_rft_clk_out"),
	DEV_CLK(61, 5, "board_0_cpts0_rft_clk_out"),
	DEV_CLK(61, 6, "board_0_mcu_ext_refclk0_out"),
	DEV_CLK(61, 7, "board_0_ext_refclk1_out"),
	DEV_CLK(61, 16, "hsdiv4_16fft_mcu_2_hsdivout1_clk"),
	DEV_CLK(61, 17, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(91, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(91, 3, "emmcsd_refclk_sel_out0"),
	DEV_CLK(91, 4, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(91, 5, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(91, 6, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(91, 7, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(92, 0, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(92, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(92, 2, "emmcsd_refclk_sel_out1"),
	DEV_CLK(92, 3, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(92, 4, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(92, 5, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(92, 6, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(102, 1, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 4, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(102, 5, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(102, 7, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(103, 0, "mcu_ospi_ref_clk_sel_out0"),
	DEV_CLK(103, 1, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(103, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(103, 3, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(103, 4, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(103, 5, "mcu_ospi0_iclk_sel_out0"),
	DEV_CLK(103, 6, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(103, 7, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(103, 8, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(104, 0, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(104, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(104, 7, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(113, 0, "wkup_gpio0_clksel_out0"),
	DEV_CLK(113, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(113, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(113, 3, "j7vc_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk"),
	DEV_CLK(113, 4, "j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(133, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(133, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(138, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(138, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(146, 2, "usart_programmable_clock_divider_out0"),
	DEV_CLK(146, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(149, 2, "mcuusart_clk_sel_out0"),
	DEV_CLK(149, 3, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(149, 4, "postdiv2_16fft_main_1_hsdivout5_clk"),
	DEV_CLK(149, 5, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(154, 0, "j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(154, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(154, 2, "gluelogic_hfosc0_clkout"),
	DEV_CLK(157, 5, "osbclk0_div_out0"),
	DEV_CLK(157, 7, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n"),
	DEV_CLK(157, 14, "mcu_obsclk_outmux_out0"),
	DEV_CLK(157, 15, "mcu_obsclk_div_out0"),
	DEV_CLK(157, 16, "gluelogic_hfosc0_clkout"),
	DEV_CLK(157, 35, "clkout_mux_out0"),
	DEV_CLK(157, 36, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 37, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 38, "osbclk0_div_out0"),
	DEV_CLK(157, 57, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p"),
	DEV_CLK(157, 65, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 69, "mcu_clkout_mux_out0"),
	DEV_CLK(157, 70, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 71, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 77, "audio_refclko_mux_out1"),
	DEV_CLK(157, 106, "hsdiv2_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 110, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 114, "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk"),
	DEV_CLK(157, 123, "mshsi2c_wkup_0_porscl"),
	DEV_CLK(157, 131, "audio_refclko_mux_out0"),
	DEV_CLK(157, 160, "hsdiv2_16fft_main_4_hsdivout2_clk"),
	DEV_CLK(157, 169, "board_0_mcu_i2c0_scl_out"),
	DEV_CLK(157, 177, "k3_pll_ctrl_wrap_main_0_sysclkout_clk"),
	DEV_CLK(157, 184, "gpmc_fclk_sel_out0"),
	DEV_CLK(157, 187, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 192, "osbclk0_div_out0"),
	DEV_CLK(157, 193, "hsdiv4_16fft_main_0_hsdivout0_clk"),
	DEV_CLK(157, 194, "hsdiv4_16fft_main_1_hsdivout0_clk"),
	DEV_CLK(157, 195, "hsdiv4_16fft_main_2_hsdivout0_clk"),
	DEV_CLK(157, 196, "hsdiv4_16fft_main_3_hsdivout0_clk"),
	DEV_CLK(157, 197, "hsdiv2_16fft_main_4_hsdivout0_clk"),
	DEV_CLK(157, 205, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(157, 206, "obsclk1_mux_out0"),
	DEV_CLK(157, 207, "hsdiv1_16fft_main_14_hsdivout0_clk"),
	DEV_CLK(157, 220, "j7vc_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(157, 221, "board_0_wkup_lf_clkin_out"),
	DEV_CLK(157, 222, "hsdiv4_16fft_main_0_hsdivout0_clk"),
	DEV_CLK(157, 223, "board_0_hfosc1_clk_out"),
	DEV_CLK(157, 224, "gluelogic_hfosc0_clkout"),
	DEV_CLK(197, 0, "board_0_wkup_i2c0_scl_out"),
	DEV_CLK(197, 1, "wkup_i2c0_mcupll_bypass_clksel_out0"),
	DEV_CLK(197, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(202, 2, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(203, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(278, 2, "usart_programmable_clock_divider_out1"),
	DEV_CLK(278, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 3, "postdiv2_16fft_main_1_hsdivout7_clk"),
	DEV_CLK(288, 4, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 6, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 12, "usb0_refclk_sel_out0"),
	DEV_CLK(288, 13, "gluelogic_hfosc0_clkout"),
	DEV_CLK(288, 14, "board_0_hfosc1_clk_out"),
	DEV_CLK(288, 15, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(288, 17, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(323, 0, "main_pll8_sel_extwave_out0"),
	DEV_CLK(323, 1, "pllfracf_ssmod_16fft_main_8_foutvcop_clk"),
	DEV_CLK(323, 2, "hsdiv0_16fft_main_8_hsdivout0_clk"),
};

const struct ti_k3_clk_platdata j7200_clk_platdata = {
	.clk_list = clk_list,
	.clk_list_cnt = ARRAY_SIZE(clk_list),
	.soc_dev_clk_data = soc_dev_clk_data,
	.soc_dev_clk_data_cnt = ARRAY_SIZE(soc_dev_clk_data),
};
