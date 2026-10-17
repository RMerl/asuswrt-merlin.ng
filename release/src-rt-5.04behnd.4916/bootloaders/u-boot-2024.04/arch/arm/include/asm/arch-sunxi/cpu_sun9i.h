/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 */

#ifndef _SUNXI_CPU_SUN9I_H
#define _SUNXI_CPU_SUN9I_H

#define REGS_AHB0_BASE			0x01C00000
#define REGS_AHB1_BASE			0x00800000
#define REGS_AHB2_BASE			0x03000000
#define REGS_APB0_BASE			0x06000000
#define REGS_APB1_BASE			0x07000000
#define REGS_RCPUS_BASE			0x08000000

#define SUNXI_SRAM_D_BASE		0x08100000

/* AHB0 Module */
#define SUNXI_NFC_BASE			(REGS_AHB0_BASE + 0x3000)

#define SUNXI_GTBUS_BASE		(REGS_AHB0_BASE + 0x9000)
/* SID address space starts at 0x01ce000, but e-fuse is at offset 0x200 */
#define SUNXI_SID_BASE			(REGS_AHB0_BASE + 0xe200)

#define SUNXI_MMC0_BASE			(REGS_AHB0_BASE + 0x0f000)
#define SUNXI_MMC1_BASE			(REGS_AHB0_BASE + 0x10000)
#define SUNXI_MMC2_BASE			(REGS_AHB0_BASE + 0x11000)
#define SUNXI_MMC3_BASE			(REGS_AHB0_BASE + 0x12000)
#define SUNXI_MMC_COMMON_BASE		(REGS_AHB0_BASE + 0x13000)

#define SUNXI_GIC400_BASE		(REGS_AHB0_BASE + 0x40000)

#define SUNXI_DRAM_COM_BASE		(REGS_AHB0_BASE + 0x62000)
#define SUNXI_DRAM_CTL0_BASE		(REGS_AHB0_BASE + 0x63000)
#define SUNXI_DRAM_CTL1_BASE		(REGS_AHB0_BASE + 0x64000)
#define SUNXI_DRAM_PHY0_BASE		(REGS_AHB0_BASE + 0x65000)
#define SUNXI_DRAM_PHY1_BASE		(REGS_AHB0_BASE + 0x66000)

#define SUNXI_DE_FE0_BASE		(REGS_AHB2_BASE + 0x100000)
#define SUNXI_DE_BE0_BASE		(REGS_AHB2_BASE + 0x200000)
#define SUNXI_LCD0_BASE			(REGS_AHB2_BASE + 0xC00000)
#define SUNXI_LCD1_BASE			(REGS_AHB2_BASE + 0xC10000)
#define SUNXI_LCD2_BASE			(REGS_AHB2_BASE + 0xC20000)
#define SUNXI_HDMI_BASE			(REGS_AHB2_BASE + 0xD00000)

/* APB0 Module */
#define SUNXI_CCM_BASE			(REGS_APB0_BASE + 0x0000)
#define SUNXI_TIMER_BASE		(REGS_APB0_BASE + 0x0C00)
#define SUNXI_PWM_BASE			(REGS_APB0_BASE + 0x1400)

/* APB1 Module */
#define SUNXI_TWI0_BASE			(REGS_APB1_BASE + 0x2800)
#define SUNXI_TWI1_BASE			(REGS_APB1_BASE + 0x2C00)

/* RCPUS Module */
#define SUNXI_PRCM_BASE			(REGS_RCPUS_BASE + 0x1400)
#define SUNXI_RSB_BASE			(REGS_RCPUS_BASE + 0x3400)

#ifndef __ASSEMBLY__
void sunxi_board_init(void);
void sunxi_reset(void);
int sunxi_get_sid(unsigned int *sid);
#endif

#endif /* _SUNXI_CPU_SUN9I_H */
