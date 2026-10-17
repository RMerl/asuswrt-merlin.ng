// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>

#define CRMU_MAIL_BOX1		0x03024028
#define CRMU_SOFT_RESET_CMD	0xFFFFFFFF

void reset_cpu(void)
{
	/* Send soft reset command via Mailbox. */
	writel(CRMU_SOFT_RESET_CMD, CRMU_MAIL_BOX1);

	while (1)
		;	/* loop forever till reset */
}
