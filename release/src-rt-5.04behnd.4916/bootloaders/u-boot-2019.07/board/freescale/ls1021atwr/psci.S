/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 NXP Semiconductor.
 * Author: Wang Dongsheng <dongsheng.wang@freescale.com>
 */

#include <config.h>
#include <linux/linkage.h>

#include <asm/armv7.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

	.arch_extension sec

	.align	5

.globl	psci_system_off
psci_system_off:
1:	wfi
	b	1b

	.popsection
