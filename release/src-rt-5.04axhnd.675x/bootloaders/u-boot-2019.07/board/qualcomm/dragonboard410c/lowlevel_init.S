/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Cédric Schieli <cschieli@gmail.com>
 */

#include <config.h>

.align 8
.global fw_dtb
fw_dtb:
	.dword 0x0

/*
 * Routine: save_boot_params (called after reset from start.S)
 * Description: save ATAG/FDT address provided by the firmware at boot time
 */

.global save_boot_params
save_boot_params:

	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
	adr	x8, fw_dtb
	str	x0, [x8]

	/* Returns */
	b	save_boot_params_ret
