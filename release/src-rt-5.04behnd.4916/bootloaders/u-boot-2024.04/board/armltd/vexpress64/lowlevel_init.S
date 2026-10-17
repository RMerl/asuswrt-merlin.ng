/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2021 Arm Limited
 */

.global save_boot_params
save_boot_params:

	adr	x8, prior_stage_fdt_address
	stp	x0, x1, [x8]

	b	save_boot_params_ret
