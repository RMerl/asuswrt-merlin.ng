/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Author: Hongbo Zhang <hongbo.zhang@nxp.com>
 * This file implements LS102X platform PSCI SYSTEM-SUSPEND function
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

.globl	psci_version
psci_version:
	ldr	w0, =0x00010000		/* PSCI v1.0 */
	ret

	.popsection
