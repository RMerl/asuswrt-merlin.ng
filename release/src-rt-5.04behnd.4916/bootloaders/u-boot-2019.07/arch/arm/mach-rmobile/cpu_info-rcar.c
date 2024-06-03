// SPDX-License-Identifier: GPL-2.0
/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-rcar.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */
#include <common.h>
#include <asm/io.h>

#define PRR_MASK		0x7fff
#define R8A7796_REV_1_0		0x5200
#define R8A7796_REV_1_1		0x5210

static u32 rmobile_get_prr(void)
{
#ifdef CONFIG_RCAR_GEN3
	return readl(0xFFF00044);
#else
	return readl(0xFF000044);
#endif
}

u32 rmobile_get_cpu_type(void)
{
	return (rmobile_get_prr() & 0x00007F00) >> 8;
}

u32 rmobile_get_cpu_rev_integer(void)
{
	const u32 prr = rmobile_get_prr();

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return ((prr & 0x000000F0) >> 4) + 1;
}

u32 rmobile_get_cpu_rev_fraction(void)
{
	const u32 prr = rmobile_get_prr();

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return prr & 0x0000000F;
}
