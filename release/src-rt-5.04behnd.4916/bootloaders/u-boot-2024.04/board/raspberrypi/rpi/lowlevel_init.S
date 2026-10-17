/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Cédric Schieli <cschieli@gmail.com>
 */

#include <config.h>

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
	ldr	r8, =fw_dtb_pointer
	str	r2, [r8]
#endif

	/* Returns */
	b	save_boot_params_ret
