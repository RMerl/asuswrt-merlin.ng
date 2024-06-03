/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

#ifdef CONFIG_SYS_MX5_HCLK
#define MXC_HCLK	CONFIG_SYS_MX5_HCLK
#else
#define MXC_HCLK	24000000
#endif

#ifdef CONFIG_SYS_MX5_CLK32
#define MXC_CLK32	CONFIG_SYS_MX5_CLK32
#else
#define MXC_CLK32	32768
#endif

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_IPG_PERCLK,
	MXC_UART_CLK,
	MXC_CSPI_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
	MXC_ESDHC4_CLK,
	MXC_FEC_CLK,
	MXC_SATA_CLK,
	MXC_DDR_CLK,
	MXC_NFC_CLK,
	MXC_PERIPH_CLK,
	MXC_I2C_CLK,
	MXC_LDB_CLK,
};

u32 imx_get_uartclk(void);
u32 imx_get_fecclk(void);
unsigned int mxc_get_clock(enum mxc_clock clk);
int mxc_set_clock(u32 ref, u32 freq, u32 clk_type);
void set_usb_phy_clk(void);
void enable_usb_phy1_clk(bool enable);
void enable_usb_phy2_clk(bool enable);
void set_usboh3_clk(void);
void enable_usboh3_clk(bool enable);
void mxc_set_sata_internal_clock(void);
int enable_i2c_clk(unsigned char enable, unsigned i2c_num);
void enable_nfc_clk(unsigned char enable);
void enable_efuse_prog_supply(bool enable);

#endif /* __ASM_ARCH_CLOCK_H */
