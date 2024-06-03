/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Cédric Schieli <cschieli@gmail.com>
 */

#include <config.h>

.align 8
.global fw_dtb_pointer
fw_dtb_pointer:
#ifdef CONFIG_ARM64
	.dword 0x0
#else
	.word 0x0
#endif

/*
 * Routine: save_boot_params (called after reset from start.S)
 * Description: save ATAG/FDT address provided by the firmware at boot time
 */

.global save_boot_params
save_boot_params:

	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
#ifdef CONFIG_ARM64
	adr	x8, fw_dtb_pointer
	str	x0, [x8]
#else
	str	r2, fw_dtb_pointer
#endif

	/* Returns */
	b	save_boot_params_ret
