/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 *  Routines to access the system control register
 *
 *  Copyright (c) 2018 Heinrich Schuchardt
 */

#include <linux/linkage.h>

/*
 * void allow_unaligned(void) - allow unaligned access
 *
 * This routine clears the aligned flag in the system control register.
 * After calling this routine unaligned access does no longer lead to a
 * data abort but is handled by the CPU.
 */
ENTRY(allow_unaligned)
	mrc	p15, 0, r0, c1, c0, 0	@ load system control register
	bic	r0, r0, #2		@ clear aligned flag
	mcr	p15, 0, r0, c1, c0, 0	@ write system control register
	bx	lr			@ return
ENDPROC(allow_unaligned)
