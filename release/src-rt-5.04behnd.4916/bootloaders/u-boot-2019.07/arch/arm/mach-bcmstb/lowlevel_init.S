/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#include <linux/linkage.h>

ENTRY(save_boot_params)
	ldr	r6, =bcmstb_boot_parameters
	str	r0, [r6, #0]
	str	r1, [r6, #4]
	str	r2, [r6, #8]
	str	r3, [r6, #12]
	str	sp, [r6, #16]
	str	lr, [r6, #20]
	ldr	r6, =prior_stage_fdt_address
	str	r2, [r6]
	b	save_boot_params_ret
ENDPROC(save_boot_params)
