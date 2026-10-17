/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am35x_def.h - TI's AM35x specific definitions.
 *
 * Based on arch/arm/include/asm/arch-omap3/cpu.h
 *
 * Author: Ajay Kumar Gupta <ajay.gupta@ti.com>
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 */

#ifndef _AM35X_DEF_H_
#define _AM35X_DEF_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__

/* LVL_INTR_CLEAR bits */
#define USBOTGSS_INT_CLR	(1 << 4)

/* IP_SW_RESET bits */
#define USBOTGSS_SW_RST		(1 << 0)	/* reset USBOTG */
#define CPGMACSS_SW_RST		(1 << 1)	/* reset CPGMAC */

/* DEVCONF2 bits */
#define CONF2_PHY_GPIOMODE	(1 << 23)
#define CONF2_OTGMODE		(3 << 14)
#define CONF2_NO_OVERRIDE	(0 << 14)
#define CONF2_FORCE_HOST	(1 << 14)
#define CONF2_FORCE_DEVICE	(2 << 14)
#define CONF2_FORCE_HOST_VBUS_LOW (3 << 14)
#define CONF2_SESENDEN		(1 << 13)
#define CONF2_VBDTCTEN		(1 << 12)
#define CONF2_REFFREQ_24MHZ	(2 << 8)
#define CONF2_REFFREQ_26MHZ	(7 << 8)
#define CONF2_REFFREQ_13MHZ	(6 << 8)
#define CONF2_REFFREQ		(0xf << 8)
#define CONF2_PHYCLKGD		(1 << 7)
#define CONF2_VBUSSENSE		(1 << 6)
#define CONF2_PHY_PLLON		(1 << 5)
#define CONF2_RESET		(1 << 4)
#define CONF2_PHYPWRDN		(1 << 3)
#define CONF2_OTGPWRDN		(1 << 2)
#define CONF2_DATPOL		(1 << 1)

/* General register mappings of system control module */
#define AM35X_SCM_GEN_BASE	0x48002270
struct am35x_scm_general {
	u32 res1[0xC4];		/* 0x000 - 0x30C */
	u32 devconf2;		/* 0x310 */
	u32 devconf3;		/* 0x314 */
	u32 res2[0x2];		/* 0x318 - 0x31C */
	u32 cba_priority;	/* 0x320 */
	u32 lvl_intr_clr;	/* 0x324 */
	u32 ip_sw_reset;	/* 0x328 */
	u32 ipss_clk_ctrl;	/* 0x32C */
};
#define am35x_scm_general_regs ((struct am35x_scm_general *)AM35X_SCM_GEN_BASE)

#define AM35XX_IPSS_USBOTGSS_BASE	0x5C040000

#endif /*__ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#endif /* _AM35X_DEF_H_ */
