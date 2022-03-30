// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 */
#include <common.h>
#include <asm/arch/cpu.h>

#define FEROCEON_EXTRA_FEATURE_L2C_EN (1<<22)

void l2_cache_disable()
{
	u32 ctrl;

	ctrl = readfr_extra_feature_reg();
	ctrl &= ~FEROCEON_EXTRA_FEATURE_L2C_EN;
	writefr_extra_feature_reg(ctrl);
}
