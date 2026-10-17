// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>

#define CRU_RESET_OFFSET	0x1803F184

void reset_cpu(void)
{
	/* Reset the cpu by setting software reset request bit */
	writel(0x1, CRU_RESET_OFFSET);

	while (1)
		;	/* loop forever till reset */
}
