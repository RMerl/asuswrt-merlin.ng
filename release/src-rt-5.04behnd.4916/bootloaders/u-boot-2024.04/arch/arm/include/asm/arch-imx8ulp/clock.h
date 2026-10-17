/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef _ASM_ARCH_IMX8ULP_CLOCK_H
#define _ASM_ARCH_IMX8ULP_CLOCK_H

#include <asm/arch/pcc.h>
#include <asm/arch/cgc.h>

#define MHZ(X)	((X) * 1000000UL)

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
	MXC_ESDHC3_CLK,
	MXC_I2C_CLK,
};

u32 mxc_get_clock(enum mxc_clock clk);
u32 get_lpuart_clk(void);
#ifdef CONFIG_SYS_I2C_IMX_LPI2C
int enable_i2c_clk(unsigned char enable, unsigned int i2c_num);
u32 imx_get_i2cclk(unsigned int i2c_num);
#endif
void enable_usboh3_clk(unsigned char enable);
int enable_usb_pll(ulong usb_phy_base);
#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable);
#endif
void init_clk_usdhc(u32 index);
void init_clk_fspi(int index);
void init_clk_ddr(void);
int set_ddr_clk(u32 phy_freq_mhz);
void clock_init_early(void);
void clock_init_late(void);
void cgc1_enet_stamp_sel(u32 clk_src);
void mxs_set_lcdclk(u32 base_addr, u32 freq_in_khz);
void reset_lcdclk(void);
void enable_mipi_dsi_clk(unsigned char enable);
void enable_adc1_clk(bool enable);
#endif
