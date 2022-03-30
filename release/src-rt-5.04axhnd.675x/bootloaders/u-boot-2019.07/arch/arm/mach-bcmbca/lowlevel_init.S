/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 *
 */
#include <linux/linkage.h>
#include <asm/macro.h>

#ifndef CONFIG_TPL_BUILD	
.align 8
.global boot_params
boot_params:
	.word 0x0

.global save_boot_params
save_boot_params:

#ifdef CONFIG_ARM64
	ldr	x10, =0xd0deed
	cmp	x9, x10
	bne	1f
	adr	x8, boot_params
	ldr	w9, [x8]
	orr	w9, w9, #0x1
	str	w9, [x8]
#else
	ldr	r10,=0xd0deed
	cmp	r9, r10
	bne	1f
	ldr	r9, boot_params
	orr	r9, r9, #0x1
	str	r9, boot_params
#endif

1:
	/* Returns */
	b	save_boot_params_ret
#endif
	
#if defined(CONFIG_TPL_ATF) && defined(CONFIG_ARM64)
/*
 * Called by armv8_switch_to_el2 to switch from el2 to el1
 * aarch32. When ATF support this switch, we can remove
 * this function
 */
ENTRY(armv8_el2_to_aarch32)
	armv8_switch_to_el1_m x4, x5, x6	
ENDPROC(armv8_el2_to_aarch32)	
#endif	
