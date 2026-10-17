// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018, 2019 Marvell International Ltd.
 */

#include <asm/global_data.h>
#include <mach/clock.h>

DECLARE_GLOBAL_DATA_PTR;

ulong notrace get_tbclk(void)
{
	return gd->cpu_clk;
}
