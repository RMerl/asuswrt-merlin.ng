/*
 * (C) Copyright 2017 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CPU_SUN50I_H6_H
#define _SUNXI_CPU_SUN50I_H6_H

#define SUNXI_SRAMC_BASE		0x03000000
#define SUNXI_CCM_BASE			0x03001000
/* SID address space starts at 0x03006000, but e-fuse is at offset 0x200 */
#define SUNXI_SIDC_BASE			0x03006000
#define SUNXI_SID_BASE			0x03006200
#define SUNXI_TIMER_BASE		0x03009000

#define SUNXI_GIC400_BASE		0x03020000

#ifdef CONFIG_MACH_SUN50I_H6
#define SUNXI_DRAM_COM_BASE		0x04002000
#define SUNXI_DRAM_CTL0_BASE		0x04003000
#define SUNXI_DRAM_PHY0_BASE		0x04005000
#endif
#define SUNXI_NFC_BASE			0x04011000
#define SUNXI_MMC0_BASE			0x04020000
#define SUNXI_MMC1_BASE			0x04021000
#define SUNXI_MMC2_BASE			0x04022000
#ifdef CONFIG_MACH_SUN50I_H616
#define SUNXI_DRAM_COM_BASE		0x047FA000
#define SUNXI_DRAM_CTL0_BASE		0x047FB000
#define SUNXI_DRAM_PHY0_BASE		0x04800000
#endif

#define SUNXI_TWI0_BASE			0x05002000
#define SUNXI_TWI1_BASE			0x05002400
#define SUNXI_TWI2_BASE			0x05002800
#define SUNXI_TWI3_BASE			0x05002C00

#define SUNXI_HDMI_BASE			0x06000000

#define SUNXI_RTC_BASE			0x07000000
#define SUNXI_R_CPUCFG_BASE		0x07000400
#define SUNXI_PRCM_BASE			0x07010000
#define SUNXI_R_WDOG_BASE		0x07020400
#define SUNXI_R_TWI_BASE		0x07081400

#ifndef __ASSEMBLY__
void sunxi_board_init(void);
void sunxi_reset(void);
int sunxi_get_sid(unsigned int *sid);
#endif

#endif /* _SUNXI_CPU_SUN9I_H */
