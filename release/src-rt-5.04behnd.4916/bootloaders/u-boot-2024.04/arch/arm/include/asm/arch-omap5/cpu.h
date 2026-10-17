/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2010
 * Texas Instruments, <www.ti.com>
 *
 *	Aneesh V <aneesh@ti.com>
 */

#ifndef _CPU_H
#define _CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#include <asm/arch/hardware.h>

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
#include <asm/ti-common/omap_wdt.h>

struct gptimer {
	u32 tidr;		/* 0x00 r */
	u8 res1[0xc];
	u32 tiocp_cfg;		/* 0x10 rw */
	u8 res2[0x10];
	u32 tisr_raw;		/* 0x24 r */
	u32 tisr;		/* 0x28 rw */
	u32 tier;		/* 0x2c rw */
	u32 ticr;		/* 0x30 rw */
	u32 twer;		/* 0x34 rw */
	u32 tclr;		/* 0x38 rw */
	u32 tcrr;		/* 0x3c rw */
	u32 tldr;		/* 0x40 rw */
	u32 ttgr;		/* 0x44 rw */
	u32 twpc;		/* 0x48 r */
	u32 tmar;		/* 0x4c rw */
	u32 tcar1;		/* 0x50 r */
	u32 tcicr;		/* 0x54 rw */
	u32 tcar2;		/* 0x58 r */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* enable sys_clk NO-prescale /1 */
#define GPT_EN			((0x0 << 2) | (0x1 << 1) | (0x1 << 0))

#define WDT_BASE                (OMAP54XX_L4_WKUP_BASE + 0x14000)
/* Watchdog */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct watchdog {
	u8 res1[0x34];
	u32 wwps;		/* 0x34 r */
	u8 res2[0x10];
	u32 wspr;		/* 0x48 rw */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* I2C base */
#define I2C_BASE1		(OMAP54XX_L4_PER_BASE + 0x70000)
#define I2C_BASE2		(OMAP54XX_L4_PER_BASE + 0x72000)
#define I2C_BASE3		(OMAP54XX_L4_PER_BASE + 0x60000)
#define I2C_BASE4		(OMAP54XX_L4_PER_BASE + 0x7A000)
#define I2C_BASE5		(OMAP54XX_L4_PER_BASE + 0x7C000)

/* MUSB base */
#define MUSB_BASE		(OMAP54XX_L4_CORE_BASE + 0xAB000)

/* OMAP4 GPIO registers */
#define OMAP_GPIO_REVISION		0x0000
#define OMAP_GPIO_SYSCONFIG		0x0010
#define OMAP_GPIO_SYSSTATUS		0x0114
#define OMAP_GPIO_IRQSTATUS1		0x0118
#define OMAP_GPIO_IRQSTATUS2		0x0128
#define OMAP_GPIO_IRQENABLE2		0x012c
#define OMAP_GPIO_IRQENABLE1		0x011c
#define OMAP_GPIO_WAKE_EN		0x0120
#define OMAP_GPIO_CTRL			0x0130
#define OMAP_GPIO_OE			0x0134
#define OMAP_GPIO_DATAIN		0x0138
#define OMAP_GPIO_DATAOUT		0x013c
#define OMAP_GPIO_LEVELDETECT0		0x0140
#define OMAP_GPIO_LEVELDETECT1		0x0144
#define OMAP_GPIO_RISINGDETECT		0x0148
#define OMAP_GPIO_FALLINGDETECT		0x014c
#define OMAP_GPIO_DEBOUNCE_EN		0x0150
#define OMAP_GPIO_DEBOUNCE_VAL		0x0154
#define OMAP_GPIO_CLEARIRQENABLE1	0x0160
#define OMAP_GPIO_SETIRQENABLE1		0x0164
#define OMAP_GPIO_CLEARWKUENA		0x0180
#define OMAP_GPIO_SETWKUENA		0x0184
#define OMAP_GPIO_CLEARDATAOUT		0x0190
#define OMAP_GPIO_SETDATAOUT		0x0194

/*
 * PRCM
 */

/* PRM */
#define PRM_BASE		0x4AE06000
#define PRM_DEVICE_BASE		(PRM_BASE + 0x1B00)

#define PRM_RSTCTRL		PRM_DEVICE_BASE
#define PRM_RSTCTRL_RESET	0x01
#define PRM_RSTST		(PRM_DEVICE_BASE + 0x4)
#define PRM_RSTST_WARM_RESET_MASK	0x7FEA

/* DRA7XX CPSW Config space */
#define CPSW_BASE			0x48484000
#define CPSW_MDIO_BASE			0x48485000

/* gmii_sel register defines */
#define GMII1_SEL_MII		0x0
#define GMII1_SEL_RMII		0x1
#define GMII1_SEL_RGMII		0x2
#define GMII2_SEL_MII		(GMII1_SEL_MII << 4)
#define GMII2_SEL_RMII		(GMII1_SEL_RMII << 4)
#define GMII2_SEL_RGMII		(GMII1_SEL_RGMII << 4)

#define MII_MODE_ENABLE		(GMII1_SEL_MII | GMII2_SEL_MII)
#define RMII_MODE_ENABLE        (GMII1_SEL_RMII | GMII2_SEL_RMII)
#define RGMII_MODE_ENABLE	(GMII1_SEL_RGMII | GMII2_SEL_RGMII)

#endif /* _CPU_H */
