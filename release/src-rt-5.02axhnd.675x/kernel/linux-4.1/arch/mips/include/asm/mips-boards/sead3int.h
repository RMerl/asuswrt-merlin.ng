/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000,2012 MIPS Technologies, Inc.  All rights reserved.
 *	Douglas Leung <douglas@mips.com>
 *	Steven J. Hill <sjhill@mips.com>
 */
#ifndef _MIPS_SEAD3INT_H
#define _MIPS_SEAD3INT_H

#include <linux/irqchip/mips-gic.h>

/* SEAD-3 GIC address space definitions. */
#define GIC_BASE_ADDR		0x1b1c0000
#define GIC_ADDRSPACE_SZ	(128 * 1024)

/* CPU interrupt offsets */
#define CPU_INT_GIC		2
#define CPU_INT_EHCI		2
#define CPU_INT_UART0		4
#define CPU_INT_UART1		4
#define CPU_INT_NET		6

/* GIC interrupt offsets */
#define GIC_INT_NET		GIC_SHARED_TO_HWIRQ(0)
#define GIC_INT_UART1		GIC_SHARED_TO_HWIRQ(2)
#define GIC_INT_UART0		GIC_SHARED_TO_HWIRQ(3)
#define GIC_INT_EHCI		GIC_SHARED_TO_HWIRQ(5)

#endif /* !(_MIPS_SEAD3INT_H) */
