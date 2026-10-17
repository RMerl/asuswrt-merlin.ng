/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier SG (SoC Glue) block registers
 *
 * Copyright (C) 2011-2015 Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef UNIPHIER_SG_REGS_H
#define UNIPHIER_SG_REGS_H

#include <linux/bitops.h>

#ifndef __ASSEMBLY__
#include <linux/compiler.h>
#ifdef CONFIG_ARCH_UNIPHIER_V8_MULTI
extern void __iomem *sg_base;
#else
#define sg_base			((void __iomem *)SG_BASE)
#endif
#endif /* __ASSEMBLY__ */

/* Base Address */
#define SG_BASE			0x5f800000

/* Revision */
#define SG_REVISION		0x0000
#define   SG_REVISION_TYPE_MASK		GENMASK(23, 16)
#define   SG_REVISION_MODEL_MASK	GENMASK(10, 8)
#define   SG_REVISION_REV_MASK		GENMASK(4, 0)

/* Memory Configuration */
#define SG_MEMCONF		0x0400

#define SG_MEMCONF_CH0_SZ_MASK		((0x1 << 10) | (0x03 << 0))
#define SG_MEMCONF_CH0_SZ_64M		((0x0 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_SZ_128M		((0x0 << 10) | (0x02 << 0))
#define SG_MEMCONF_CH0_SZ_256M		((0x0 << 10) | (0x03 << 0))
#define SG_MEMCONF_CH0_SZ_512M		((0x1 << 10) | (0x00 << 0))
#define SG_MEMCONF_CH0_SZ_1G		((0x1 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_NUM_MASK		(0x1 << 8)
#define SG_MEMCONF_CH0_NUM_1		(0x1 << 8)
#define SG_MEMCONF_CH0_NUM_2		(0x0 << 8)

#define SG_MEMCONF_CH1_SZ_MASK		((0x1 << 11) | (0x03 << 2))
#define SG_MEMCONF_CH1_SZ_64M		((0x0 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_SZ_128M		((0x0 << 11) | (0x02 << 2))
#define SG_MEMCONF_CH1_SZ_256M		((0x0 << 11) | (0x03 << 2))
#define SG_MEMCONF_CH1_SZ_512M		((0x1 << 11) | (0x00 << 2))
#define SG_MEMCONF_CH1_SZ_1G		((0x1 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_NUM_MASK		(0x1 << 9)
#define SG_MEMCONF_CH1_NUM_1		(0x1 << 9)
#define SG_MEMCONF_CH1_NUM_2		(0x0 << 9)

#define SG_MEMCONF_CH2_SZ_MASK		((0x1 << 26) | (0x03 << 16))
#define SG_MEMCONF_CH2_SZ_64M		((0x0 << 26) | (0x01 << 16))
#define SG_MEMCONF_CH2_SZ_128M		((0x0 << 26) | (0x02 << 16))
#define SG_MEMCONF_CH2_SZ_256M		((0x0 << 26) | (0x03 << 16))
#define SG_MEMCONF_CH2_SZ_512M		((0x1 << 26) | (0x00 << 16))
#define SG_MEMCONF_CH2_SZ_1G		((0x1 << 26) | (0x01 << 16))
#define SG_MEMCONF_CH2_NUM_MASK		(0x1 << 24)
#define SG_MEMCONF_CH2_NUM_1		(0x1 << 24)
#define SG_MEMCONF_CH2_NUM_2		(0x0 << 24)
/* PH1-LD6b, ProXstream2, PH1-LD20 only */
#define SG_MEMCONF_CH2_DISABLE		(0x1 << 21)

#define SG_MEMCONF_SPARSEMEM		(0x1 << 4)

#define SG_USBPHYCTRL		0x0500
#define SG_ETPHYPSHUT		0x0554
#define SG_ETPHYCNT		0x0550

/* Pin Control */
#define SG_PINCTRL_BASE		0x1000

/* PH1-Pro4, PH1-Pro5 */
#define SG_LOADPINCTRL		0x1700

/* Input Enable */
#define SG_IECTRL		0x1d00

/* Pin Monitor */
#define SG_PINMON0		0x00100100
#define SG_PINMON2		0x00100108

#define SG_PINMON0_CLK_MODE_UPLLSRC_MASK	(0x3 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_DEFAULT	(0x0 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_VPLL27A	(0x2 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_VPLL27B	(0x3 << 19)

#define SG_PINMON0_CLK_MODE_AXOSEL_MASK		(0x3 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_24576KHZ	(0x0 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ	(0x1 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_6144KHZ	(0x2 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_6250KHZ	(0x3 << 16)

#define SG_PINMON0_CLK_MODE_AXOSEL_DEFAULT	(0x0 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_U	(0x1 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_20480KHZ	(0x2 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_A	(0x3 << 16)

#endif /* UNIPHIER_SG_REGS_H */
