// SPDX-License-Identifier: GPL-2.0+
/*
 * AM62AX specific clock platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Bryan Brattlof <bb@ti.com>.
 *
 * Copyright (C) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/clk-provider.h>
#include "k3-clk.h"

static const char * const gluelogic_hfosc0_clkout_parents[] = {
	NULL,
	NULL,
	"osc_24_mhz",
	"osc_25_mhz",
	"osc_26_mhz",
	NULL,
};

static const char * const clk_32k_rc_sel_out0_parents[] = {
	"gluelogic_rcosc_clk_1p0v_97p65k",
	"gluelogic_hfosc0_clkout",
	"gluelogic_rcosc_clk_1p0v_97p65k",
	"gluelogic_lfosc0_clkout",
};

static const char * const main_emmcsd0_io_clklb_sel_out0_parents[] = {
	"board_0_mmc0_clklb_out",
	"board_0_mmc0_clk_out",
};

static const char * const main_emmcsd1_io_clklb_sel_out0_parents[] = {
	"board_0_mmc1_clklb_out",
	"board_0_mmc1_clk_out",
};

static const char * const main_ospi_loopback_clk_sel_out0_parents[] = {
	"board_0_ospi0_dqs_out",
	"board_0_ospi0_lbclko_out",
};

static const char * const main_usb0_refclk_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"postdiv4_16ff_main_0_hsdivout8_clk",
};

static const char * const main_usb1_refclk_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"postdiv4_16ff_main_0_hsdivout8_clk",
};

static const char * const sam62_pll_ctrl_wrap_main_0_sysclkout_clk_parents[] = {
	"gluelogic_hfosc0_clkout",
	"hsdiv4_16fft_main_0_hsdivout0_clk",
};

static const char * const sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk_parents[] = {
	"gluelogic_hfosc0_clkout",
	"hsdiv4_16fft_mcu_0_hsdivout0_clk",
};

static const char * const clkout0_ctrl_out0_parents[] = {
	"hsdiv4_16fft_main_2_hsdivout1_clk",
	"hsdiv4_16fft_main_2_hsdivout1_clk",
};

static const char * const main_emmcsd0_refclk_sel_out0_parents[] = {
	"postdiv4_16ff_main_0_hsdivout5_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
};

static const char * const main_emmcsd1_refclk_sel_out0_parents[] = {
	"postdiv4_16ff_main_0_hsdivout5_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
};

static const char * const main_gtcclk_sel_out0_parents[] = {
	"postdiv4_16ff_main_2_hsdivout5_clk",
	"postdiv4_16ff_main_0_hsdivout6_clk",
	"board_0_cp_gemac_cpts0_rft_clk_out",
	NULL,
	"board_0_mcu_ext_refclk0_out",
	"board_0_ext_refclk1_out",
	"sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk",
	"sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk",
};

static const char * const main_ospi_ref_clk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout1_clk",
	"postdiv1_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_clkout_sel_out0_parents[] = {
	NULL,
	"gluelogic_lfosc0_clkout",
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"postdiv4_16ff_main_2_hsdivout9_clk",
	"clk_32k_rc_sel_out0",
	"gluelogic_rcosc_clkout",
	"gluelogic_hfosc0_clkout",
};

static const char * const wkup_clkout_sel_io_out0_parents[] = {
	"wkup_clkout_sel_out0",
	"gluelogic_hfosc0_clkout",
};

static const char * const wkup_clksel_out0_parents[] = {
	"hsdiv2_16fft_main_15_hsdivout0_clk",
	"hsdiv4_16fft_mcu_0_hsdivout0_clk",
};

static const char * const main_usart0_fclk_sel_out0_parents[] = {
	"usart_programmable_clock_divider_out0",
	"hsdiv4_16fft_main_1_hsdivout1_clk",
};

static const struct clk_data clk_list[] = {
	CLK_FIXED_RATE("osc_26_mhz", 26000000, 0),
	CLK_FIXED_RATE("osc_25_mhz", 25000000, 0),
	CLK_FIXED_RATE("osc_24_mhz", 24000000, 0),
	CLK_MUX("gluelogic_hfosc0_clkout", gluelogic_hfosc0_clkout_parents, 6, 0x43000030, 0, 3, 0),
	CLK_FIXED_RATE("gluelogic_rcosc_clkout", 12500000, 0),
	CLK_FIXED_RATE("gluelogic_rcosc_clk_1p0v_97p65k", 97656, 0),
	CLK_FIXED_RATE("board_0_cp_gemac_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ddr0_ck0_out", 0, 0),
	CLK_FIXED_RATE("board_0_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_i2c0_scl_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc0_clklb_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc0_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clklb_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ospi0_dqs_out", 0, 0),
	CLK_FIXED_RATE("board_0_ospi0_lbclko_out", 0, 0),
	CLK_FIXED_RATE("board_0_tck_out", 0, 0),
	CLK_FIXED_RATE("emmcsd4ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_FIXED_RATE("emmcsd8ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_FIXED_RATE("fss_ul_main_0_ospi_0_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("mshsi2c_main_0_porscl", 0, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_0_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x680000, 0),
	CLK_DIV("pllfracf_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", 0x680038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf_ssmod_16fft_main_1_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x681000, 0),
	CLK_DIV("pllfracf_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf_ssmod_16fft_main_1_foutpostdiv_clk", "pllfracf_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", 0x681038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf_ssmod_16fft_main_12_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x68c000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_15_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x68f000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_main_2_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x682000, 0),
	CLK_DIV("pllfracf_ssmod_16fft_main_2_foutpostdiv_clk_subdiv", "pllfracf_ssmod_16fft_main_2_foutvcop_clk", 0x682038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf_ssmod_16fft_main_2_foutpostdiv_clk", "pllfracf_ssmod_16fft_main_2_foutpostdiv_clk_subdiv", 0x682038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf_ssmod_16fft_main_8_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x688000, 0),
	CLK_PLL("pllfracf_ssmod_16fft_mcu_0_foutvcop_clk", "gluelogic_hfosc0_clkout", 0x4040000, 0),
	CLK_DIV("postdiv1_16fft_main_1_hsdivout5_clk", "pllfracf_ssmod_16fft_main_1_foutpostdiv_clk", 0x681094, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_0_hsdivout5_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", 0x680094, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_0_hsdivout6_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", 0x680098, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_0_hsdivout8_clk", "pllfracf_ssmod_16fft_main_0_foutpostdiv_clk", 0x6800a0, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_2_hsdivout5_clk", "pllfracf_ssmod_16fft_main_2_foutpostdiv_clk", 0x682094, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_2_hsdivout8_clk", "pllfracf_ssmod_16fft_main_2_foutpostdiv_clk", 0x6820a0, 0, 7, 0, 0),
	CLK_DIV("postdiv4_16ff_main_2_hsdivout9_clk", "pllfracf_ssmod_16fft_main_2_foutpostdiv_clk", 0x6820a4, 0, 7, 0, 0),
	CLK_MUX("clk_32k_rc_sel_out0", clk_32k_rc_sel_out0_parents, 4, 0x4508058, 0, 2, 0),
	CLK_MUX("main_emmcsd0_io_clklb_sel_out0", main_emmcsd0_io_clklb_sel_out0_parents, 2, 0x108160, 16, 1, 0),
	CLK_MUX("main_emmcsd1_io_clklb_sel_out0", main_emmcsd1_io_clklb_sel_out0_parents, 2, 0x108168, 16, 1, 0),
	CLK_MUX("main_ospi_loopback_clk_sel_out0", main_ospi_loopback_clk_sel_out0_parents, 2, 0x108500, 4, 1, 0),
	CLK_MUX("main_usb0_refclk_sel_out0", main_usb0_refclk_sel_out0_parents, 2, 0x43008190, 0, 1, 0),
	CLK_MUX("main_usb1_refclk_sel_out0", main_usb1_refclk_sel_out0_parents, 2, 0x43008194, 0, 1, 0),
	CLK_DIV("hsdiv0_16fft_main_12_hsdivout0_clk", "pllfracf_ssmod_16fft_main_12_foutvcop_clk", 0x68c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_8_hsdivout0_clk", "pllfracf_ssmod_16fft_main_8_foutvcop_clk", 0x688080, 0, 7, 0, 0),
	CLK_DIV("hsdiv2_16fft_main_15_hsdivout0_clk", "pllfracf_ssmod_16fft_main_15_foutvcop_clk", 0x68f080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout0_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout1_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout2_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout3_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x68008c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout4_clk", "pllfracf_ssmod_16fft_main_0_foutvcop_clk", 0x680090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_main_1_hsdivout0_clk", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681080, 0, 7, 0, 0, 192000000),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout1_clk", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout2_clk", "pllfracf_ssmod_16fft_main_1_foutvcop_clk", 0x681088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout1_clk", "pllfracf_ssmod_16fft_main_2_foutvcop_clk", 0x682084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout2_clk", "pllfracf_ssmod_16fft_main_2_foutvcop_clk", 0x682088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_0_hsdivout0_clk", "pllfracf_ssmod_16fft_mcu_0_foutvcop_clk", 0x4040080, 0, 7, 0, 0),
	CLK_MUX_PLLCTRL("sam62_pll_ctrl_wrap_main_0_sysclkout_clk", sam62_pll_ctrl_wrap_main_0_sysclkout_clk_parents, 2, 0x410000, 0),
	CLK_DIV("sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk", "sam62_pll_ctrl_wrap_main_0_sysclkout_clk", 0x410118, 0, 5, 0, 0),
	CLK_MUX_PLLCTRL("sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk", sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk_parents, 2, 0x4020000, 0),
	CLK_DIV("sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk", "sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk", 0x4020118, 0, 5, 0, 0),
	CLK_MUX("clkout0_ctrl_out0", clkout0_ctrl_out0_parents, 2, 0x108010, 0, 1, 0),
	CLK_MUX("main_emmcsd0_refclk_sel_out0", main_emmcsd0_refclk_sel_out0_parents, 2, 0x108160, 0, 1, 0),
	CLK_MUX("main_emmcsd1_refclk_sel_out0", main_emmcsd1_refclk_sel_out0_parents, 2, 0x108168, 0, 1, 0),
	CLK_MUX("main_gtcclk_sel_out0", main_gtcclk_sel_out0_parents, 8, 0x43008030, 0, 3, 0),
	CLK_MUX("main_ospi_ref_clk_sel_out0", main_ospi_ref_clk_sel_out0_parents, 2, 0x108500, 0, 1, 0),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out0", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x108240, 0, 2, 0, 0, 48000000),
	CLK_MUX("wkup_clkout_sel_out0", wkup_clkout_sel_out0_parents, 8, 0x43008020, 0, 3, 0),
	CLK_MUX("wkup_clkout_sel_io_out0", wkup_clkout_sel_io_out0_parents, 2, 0x43008020, 24, 1, 0),
	CLK_MUX("wkup_clksel_out0", wkup_clksel_out0_parents, 2, 0x43008010, 0, 1, 0),
	CLK_MUX("main_usart0_fclk_sel_out0", main_usart0_fclk_sel_out0_parents, 2, 0x108280, 0, 1, 0),
	CLK_DIV("hsdiv4_16fft_mcu_0_hsdivout1_clk", "pllfracf_ssmod_16fft_mcu_0_foutvcop_clk", 0x4040084, 0, 7, 0, 0),
	CLK_FIXED_RATE("mshsi2c_wkup_0_porscl", 0, 0),
	CLK_DIV("sam62_pll_ctrl_wrap_main_0_chip_div24_clk_clk", "sam62_pll_ctrl_wrap_main_0_sysclkout_clk", 0x41011c, 0, 5, 0, 0),
	CLK_DIV("sam62_pll_ctrl_wrap_mcu_0_chip_div24_clk_clk", "sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk", 0x402011c, 0, 5, 0, 0),
};

static const struct dev_clk soc_dev_clk_data[] = {
	DEV_CLK(16, 0, "hsdiv4_16fft_main_0_hsdivout1_clk"),
	DEV_CLK(16, 1, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(16, 2, "hsdiv4_16fft_main_0_hsdivout3_clk"),
	DEV_CLK(16, 3, "hsdiv4_16fft_main_0_hsdivout4_clk"),
	DEV_CLK(16, 4, "gluelogic_hfosc0_clkout"),
	DEV_CLK(16, 5, "board_0_ext_refclk1_out"),
	DEV_CLK(16, 6, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(16, 7, "postdiv4_16ff_main_2_hsdivout8_clk"),
	DEV_CLK(16, 8, "gluelogic_hfosc0_clkout"),
	DEV_CLK(16, 9, "board_0_ext_refclk1_out"),
	DEV_CLK(16, 10, "gluelogic_rcosc_clkout"),
	DEV_CLK(16, 11, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(16, 12, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(57, 0, "main_emmcsd0_io_clklb_sel_out0"),
	DEV_CLK(57, 1, "board_0_mmc0_clklb_out"),
	DEV_CLK(57, 2, "board_0_mmc0_clk_out"),
	DEV_CLK(57, 5, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(57, 6, "main_emmcsd0_refclk_sel_out0"),
	DEV_CLK(57, 7, "postdiv4_16ff_main_0_hsdivout5_clk"),
	DEV_CLK(57, 8, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(58, 0, "main_emmcsd1_io_clklb_sel_out0"),
	DEV_CLK(58, 1, "board_0_mmc1_clklb_out"),
	DEV_CLK(58, 2, "board_0_mmc1_clk_out"),
	DEV_CLK(58, 5, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(58, 6, "main_emmcsd1_refclk_sel_out0"),
	DEV_CLK(58, 7, "postdiv4_16ff_main_0_hsdivout5_clk"),
	DEV_CLK(58, 8, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(61, 0, "main_gtcclk_sel_out0"),
	DEV_CLK(61, 1, "postdiv4_16ff_main_2_hsdivout5_clk"),
	DEV_CLK(61, 2, "postdiv4_16ff_main_0_hsdivout6_clk"),
	DEV_CLK(61, 3, "board_0_cp_gemac_cpts0_rft_clk_out"),
	DEV_CLK(61, 5, "board_0_mcu_ext_refclk0_out"),
	DEV_CLK(61, 6, "board_0_ext_refclk1_out"),
	DEV_CLK(61, 7, "sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk"),
	DEV_CLK(61, 8, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(61, 9, "wkup_clksel_out0"),
	DEV_CLK(61, 10, "hsdiv2_16fft_main_15_hsdivout0_clk"),
	DEV_CLK(61, 11, "hsdiv4_16fft_mcu_0_hsdivout0_clk"),
	DEV_CLK(75, 0, "board_0_ospi0_dqs_out"),
	DEV_CLK(75, 1, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(75, 2, "main_ospi_loopback_clk_sel_out0"),
	DEV_CLK(75, 3, "board_0_ospi0_dqs_out"),
	DEV_CLK(75, 4, "board_0_ospi0_lbclko_out"),
	DEV_CLK(75, 6, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(75, 7, "main_ospi_ref_clk_sel_out0"),
	DEV_CLK(75, 8, "hsdiv4_16fft_main_0_hsdivout1_clk"),
	DEV_CLK(75, 9, "postdiv1_16fft_main_1_hsdivout5_clk"),
	DEV_CLK(77, 0, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(95, 0, "gluelogic_rcosc_clkout"),
	DEV_CLK(95, 1, "gluelogic_hfosc0_clkout"),
	DEV_CLK(95, 2, "wkup_clksel_out0"),
	DEV_CLK(95, 3, "hsdiv2_16fft_main_15_hsdivout0_clk"),
	DEV_CLK(95, 4, "hsdiv4_16fft_mcu_0_hsdivout0_clk"),
	DEV_CLK(102, 0, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(102, 1, "board_0_i2c0_scl_out"),
	DEV_CLK(102, 2, "hsdiv4_16fft_main_1_hsdivout0_clk"),
	DEV_CLK(107, 0, "wkup_clksel_out0"),
	DEV_CLK(107, 1, "hsdiv2_16fft_main_15_hsdivout0_clk"),
	DEV_CLK(107, 2, "hsdiv4_16fft_mcu_0_hsdivout0_clk"),
	DEV_CLK(107, 4, "hsdiv4_16fft_mcu_0_hsdivout1_clk"),
	DEV_CLK(135, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(140, 0, "sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk"),
	DEV_CLK(140, 1, "sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk"),
	DEV_CLK(146, 0, "main_usart0_fclk_sel_out0"),
	DEV_CLK(146, 1, "usart_programmable_clock_divider_out0"),
	DEV_CLK(146, 2, "hsdiv4_16fft_main_1_hsdivout1_clk"),
	DEV_CLK(146, 5, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(157, 20, "clkout0_ctrl_out0"),
	DEV_CLK(157, 21, "hsdiv4_16fft_main_2_hsdivout1_clk"),
	DEV_CLK(157, 22, "hsdiv4_16fft_main_2_hsdivout1_clk"),
	DEV_CLK(157, 24, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(157, 25, "board_0_ddr0_ck0_out"),
	DEV_CLK(157, 40, "mshsi2c_main_0_porscl"),
	DEV_CLK(157, 77, "sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk"),
	DEV_CLK(157, 83, "emmcsd8ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 85, "emmcsd8ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 87, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 89, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 130, "fss_ul_main_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 146, "sam62_pll_ctrl_wrap_main_0_sysclkout_clk"),
	DEV_CLK(157, 159, "wkup_clkout_sel_io_out0"),
	DEV_CLK(157, 160, "wkup_clkout_sel_out0"),
	DEV_CLK(157, 161, "gluelogic_hfosc0_clkout"),
	DEV_CLK(161, 0, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(161, 1, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(161, 2, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(161, 3, "main_usb0_refclk_sel_out0"),
	DEV_CLK(161, 4, "gluelogic_hfosc0_clkout"),
	DEV_CLK(161, 5, "postdiv4_16ff_main_0_hsdivout8_clk"),
	DEV_CLK(161, 10, "board_0_tck_out"),
	DEV_CLK(162, 0, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(162, 1, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(162, 2, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(162, 3, "main_usb1_refclk_sel_out0"),
	DEV_CLK(162, 4, "gluelogic_hfosc0_clkout"),
	DEV_CLK(162, 5, "postdiv4_16ff_main_0_hsdivout8_clk"),
	DEV_CLK(162, 10, "board_0_tck_out"),
	DEV_CLK(166, 3, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(166, 5, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(169, 0, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(169, 1, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(170, 1, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(170, 2, "board_0_tck_out"),
	DEV_CLK(170, 3, "sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
};

const struct ti_k3_clk_platdata am62ax_clk_platdata = {
	.clk_list = clk_list,
	.clk_list_cnt = 80,
	.soc_dev_clk_data = soc_dev_clk_data,
	.soc_dev_clk_data_cnt = 104,
};
