// SPDX-License-Identifier: GPL-2.0+
/*
 * GXP driver
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 */

#include <cpu_func.h>
#include <asm/io.h>

#define GXP_CCR	0xc0000000

/* empty to satisfy current lowlevel_init, can be removed any time */
void lowlevel_init(void)
{
}

void reset_cpu(void)
{
	writel(1, GXP_CCR);

	while (1)
		;	/* loop forever till reset */
}
