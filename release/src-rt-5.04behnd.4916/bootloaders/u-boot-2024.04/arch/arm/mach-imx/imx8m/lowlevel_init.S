/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#include <config.h>

.align 4
.global rom_pointer
rom_pointer:
	.space 256

/*
 * Routine: save_boot_params (called after reset from start.S)
 */

.global save_boot_params
save_boot_params:
	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
	adr	x0, rom_pointer
	stp	x1, x2, [x0], #16
	stp	x3, x4, [x0], #16
	stp	x5, x6, [x0], #16
	stp	x7, x8, [x0], #16
	stp	x9, x10, [x0], #16
	stp	x11, x12, [x0], #16
	stp	x13, x14, [x0], #16
	stp	x15, x16, [x0], #16
	stp	x17, x18, [x0], #16
	stp	x19, x20, [x0], #16
	stp	x21, x22, [x0], #16
	stp	x23, x24, [x0], #16
	stp	x25, x26, [x0], #16
	stp	x27, x28, [x0], #16
	stp	x29, x30, [x0], #16
	mov	x30, sp
	str	x30, [x0], #8

	/* Returns */
	b	save_boot_params_ret

.global restore_boot_params
restore_boot_params:
	adr	x0, rom_pointer
	ldp	x1, x2, [x0], #16
	ldp	x3, x4, [x0], #16
	ldp	x5, x6, [x0], #16
	ldp	x7, x8, [x0], #16
	ldp	x9, x10, [x0], #16
	ldp	x11, x12, [x0], #16
	ldp	x13, x14, [x0], #16
	ldp	x15, x16, [x0], #16
	ldp	x17, x18, [x0], #16
	ldp	x19, x20, [x0], #16
	ldp	x21, x22, [x0], #16
	ldp	x23, x24, [x0], #16
	ldp	x25, x26, [x0], #16
	ldp	x27, x28, [x0], #16
	ldp	x29, x30, [x0], #16
	ldr	x0, [x0]
	mov	sp, x0
	ret

.global armv8_el2_to_aarch32
armv8_el2_to_aarch32:
	cmp     x0, #0
	bne     0f
	mov     x3, x2
	mov     x2, x1
	mov     x1, x4
	ldr x0, =0xc20000fd
0:
	smc #0
	ret
