/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_ARCH_CLOCK_H
#define _ASM_ARCH_CLOCK_H

#include <asm/arch/pcc.h>
#include <asm/arch/scg.h>

/* Mainly for compatible to imx common code. */
enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_UART_CLK,
	MXC_CSPI_CLK,
	MXC_AXI_CLK,
	MXC_DDR_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_I2C_CLK,
};

u32 mxc_get_clock(enum mxc_clock clk);
u32 get_lpuart_clk(void);
#ifdef CONFIG_SYS_I2C_IMX_LPI2C
int enable_i2c_clk(unsigned char enable, unsigned i2c_num);
u32 imx_get_i2cclk(unsigned i2c_num);
#endif
#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable);
#endif
#ifdef CONFIG_USB_EHCI_HCD
void enable_usboh3_clk(unsigned char enable);
#endif
void init_clk_usdhc(u32 index);
void clock_init(void);
void hab_caam_clock_enable(unsigned char enable);
#endif
