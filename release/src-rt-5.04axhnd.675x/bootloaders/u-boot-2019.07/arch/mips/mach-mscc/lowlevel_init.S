/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <asm/asm.h>
#include <asm/regdef.h>

    .set noreorder
    .extern     vcoreiii_tlb_init
#ifdef CONFIG_SOC_LUTON
    .extern     pll_init
#endif

LEAF(lowlevel_init)
	/*
	 * As we have no stack yet, we can assume the restricted
	 * luxury of the sX-registers without saving them
	 */
	move	s0,ra

	jal	vcoreiii_tlb_init
	 nop
#ifdef CONFIG_SOC_LUTON
	jal	pll_init
	 nop
#endif
	jr	s0
	 nop
	END(lowlevel_init)
