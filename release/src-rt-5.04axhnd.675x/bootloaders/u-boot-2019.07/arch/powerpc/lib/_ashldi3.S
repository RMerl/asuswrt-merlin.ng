/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * This code was copied from arch/powerpc/kernel/misc_32.S in the Linux
 * kernel sources (commit 85e2efbb1db9a18d218006706d6e4fbeb0216213, also
 * known as 2.6.38-rc5).  The source file copyrights are as follows:
 *
 * (C) Copyright 1995-1996 Gary Thomas (gdt@linuxppc.org)
 *
 * Largely rewritten by Cort Dougan (cort@cs.nmt.edu)
 * and Paul Mackerras.
 */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <config.h>

/*
 * Extended precision shifts.
 *
 * Updated to be valid for shift counts from 0 to 63 inclusive.
 * -- Gabriel
 *
 * R3/R4 has 64 bit value
 * R5    has shift count
 * result in R3/R4
 *
 *  ashrdi3: arithmetic right shift (sign propagation)
 *  lshrdi3: logical right shift
 *  ashldi3: left shift
 */
	.globl __ashldi3
__ashldi3:
	subfic	r6,r5,32
	slw	r3,r3,r5	# MSW = count > 31 ? 0 : MSW << count
	addi	r7,r5,32	# could be xori, or addi with -32
	srw	r6,r4,r6	# t1 = count > 31 ? 0 : LSW >> (32-count)
	slw	r7,r4,r7	# t2 = count < 32 ? 0 : LSW << (count-32)
	or	r3,r3,r6	# MSW |= t1
	slw	r4,r4,r5	# LSW = LSW << count
	or	r3,r3,r7	# MSW |= t2
	blr
