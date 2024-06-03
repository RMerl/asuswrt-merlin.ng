/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MIPS Coherence Manager (CM) Register Definitions
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 */
#ifndef __MIPS_ASM_CM_H__
#define __MIPS_ASM_CM_H__

/* Global Control Register (GCR) offsets */
#define GCR_BASE			0x0008
#define GCR_BASE_UPPER			0x000c
#define GCR_REV				0x0030
#define GCR_L2_CONFIG			0x0130
#define GCR_L2_TAG_ADDR			0x0600
#define GCR_L2_TAG_ADDR_UPPER		0x0604
#define GCR_L2_TAG_STATE		0x0608
#define GCR_L2_TAG_STATE_UPPER		0x060c
#define GCR_L2_DATA			0x0610
#define GCR_L2_DATA_UPPER		0x0614
#define GCR_Cx_COHERENCE		0x2008

/* GCR_REV CM versions */
#define GCR_REV_CM3			0x0800

/* GCR_L2_CONFIG fields */
#define GCR_L2_CONFIG_ASSOC_SHIFT	0
#define GCR_L2_CONFIG_ASSOC_BITS	8
#define GCR_L2_CONFIG_LINESZ_SHIFT	8
#define GCR_L2_CONFIG_LINESZ_BITS	4
#define GCR_L2_CONFIG_SETSZ_SHIFT	12
#define GCR_L2_CONFIG_SETSZ_BITS	4
#define GCR_L2_CONFIG_BYPASS		(1 << 20)

/* GCR_Cx_COHERENCE */
#define GCR_Cx_COHERENCE_DOM_EN		(0xff << 0)
#define GCR_Cx_COHERENCE_EN		(0x1 << 0)

#ifndef __ASSEMBLY__

#include <asm/io.h>

static inline void *mips_cm_base(void)
{
	return (void *)CKSEG1ADDR(CONFIG_MIPS_CM_BASE);
}

static inline unsigned long mips_cm_l2_line_size(void)
{
	unsigned long l2conf, line_sz;

	l2conf = __raw_readl(mips_cm_base() + GCR_L2_CONFIG);

	line_sz = l2conf >> GCR_L2_CONFIG_LINESZ_SHIFT;
	line_sz &= GENMASK(GCR_L2_CONFIG_LINESZ_BITS - 1, 0);
	return line_sz ? (2 << line_sz) : 0;
}

#endif /* !__ASSEMBLY__ */

#endif /* __MIPS_ASM_CM_H__ */
