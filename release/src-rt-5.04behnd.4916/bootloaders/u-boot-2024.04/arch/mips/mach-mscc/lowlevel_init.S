/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <asm/asm.h>
#include <asm/regdef.h>

    .set noreorder
    .extern     vcoreiii_tlb_init
    .extern     vcoreiii_ddr_init
#ifdef CONFIG_SOC_LUTON
    .extern     pll_init
#endif

LEAF(lowlevel_init)
	/*
	 * As we have no stack yet, we can assume the restricted
	 * luxury of the sX-registers without saving them
	 */

	/* Modify ra/s0 such we return to physical NOR location */
	li	t0, 0x0fffffff
	li	t1, CONFIG_TEXT_BASE
	and	s0, ra, t0
	add	s0, s0, t1

	jal	vcoreiii_tlb_init
	 nop

#ifdef CONFIG_SOC_LUTON
	jal	pll_init
	 nop
#endif

	/* Initialize DDR controller to enable stack/gd/heap */
0:
	jal	vcoreiii_ddr_init
	 nop
	bnez	v0, 0b		/* Retry on error */
	 nop

	jr	s0
	 nop
	END(lowlevel_init)
