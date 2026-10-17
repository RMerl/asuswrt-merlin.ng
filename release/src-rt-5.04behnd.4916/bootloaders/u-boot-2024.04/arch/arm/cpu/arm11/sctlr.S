/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 *  Routines to access the system control register
 *
 *  Copyright (c) 2019 Heinrich Schuchardt
 */

#include <linux/linkage.h>

/*
 * void arm11_arch_cp15_allow_unaligned(void) - allow unaligned access
 *
 * This routine sets the enable unaligned data support flag and clears the
 * aligned flag in the system control register.
 * After calling this routine unaligned access does no longer leads to a
 * data abort or undefined behavior but is handled by the CPU.
 * For details see the "ARM Architecture Reference Manual" for ARMv6.
 */
ENTRY(arm11_arch_cp15_allow_unaligned)
	mrc	p15, 0, r0, c1, c0, 0	@ load system control register
	orr	r0, r0, #1 << 22	@ set unaligned data support flag
	bic	r0, r0, #2		@ clear aligned flag
	mcr	p15, 0, r0, c1, c0, 0	@ write system control register
	bx	lr			@ return
ENDPROC(arm11_arch_cp15_allow_unaligned)
