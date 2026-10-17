/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#include <config.h>

.align 8
.global rom_pointer
rom_pointer:
	.space 256

/*
 * Routine: save_boot_params (called after reset from start.S)
 */

.global save_boot_params
save_boot_params:
#ifndef CONFIG_SPL_BUILD
	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
	adr	x0, rom_pointer
	stp	x1, x2, [x0], #16
	stp	x3, x4, [x0], #16
#endif
	/* Returns */
	b	save_boot_params_ret
