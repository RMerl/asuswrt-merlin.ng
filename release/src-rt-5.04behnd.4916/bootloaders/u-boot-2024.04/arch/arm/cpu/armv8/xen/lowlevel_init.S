/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) 2017 NXP
 * (C) 2020 EPAM Systems Inc.
 */

#include <config.h>

.align 8
.global rom_pointer
rom_pointer:
	.space 32

/*
 * Routine: save_boot_params (called after reset from start.S)
 */

.global save_boot_params
save_boot_params:
	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
	adr	x1, rom_pointer
	stp	x0, x2, [x1], #16
	stp	x3, x4, [x1], #16

	/* Returns */
	b	save_boot_params_ret

.global restore_boot_params
restore_boot_params:
	adr	x1, rom_pointer
	ldp	x0, x2, [x1], #16
	ldp	x3, x4, [x1], #16
	ret
