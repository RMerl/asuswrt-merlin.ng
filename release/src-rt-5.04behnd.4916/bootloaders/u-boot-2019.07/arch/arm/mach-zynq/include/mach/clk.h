/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Xilinx Inc.
 */

#ifndef _ZYNQ_CLK_H_
#define _ZYNQ_CLK_H_

enum zynq_clk {
	armpll_clk, ddrpll_clk, iopll_clk,
	cpu_6or4x_clk, cpu_3or2x_clk, cpu_2x_clk, cpu_1x_clk,
	ddr2x_clk, ddr3x_clk, dci_clk,
	lqspi_clk, smc_clk, pcap_clk, gem0_clk, gem1_clk,
	fclk0_clk, fclk1_clk, fclk2_clk, fclk3_clk, can0_clk, can1_clk,
	sdio0_clk, sdio1_clk, uart0_clk, uart1_clk, spi0_clk, spi1_clk, dma_clk,
	usb0_aper_clk, usb1_aper_clk, gem0_aper_clk, gem1_aper_clk,
	sdio0_aper_clk, sdio1_aper_clk, spi0_aper_clk, spi1_aper_clk,
	can0_aper_clk, can1_aper_clk, i2c0_aper_clk, i2c1_aper_clk,
	uart0_aper_clk, uart1_aper_clk, gpio_aper_clk, lqspi_aper_clk,
	smc_aper_clk, swdt_clk, dbg_trc_clk, dbg_apb_clk, clk_max};

#endif
