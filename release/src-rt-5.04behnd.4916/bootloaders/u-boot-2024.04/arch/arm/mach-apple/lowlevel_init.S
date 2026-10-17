/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 */

.align 8
.global fw_dtb_pointer
fw_dtb_pointer:
	.quad	0

.global save_boot_params
save_boot_params:
	/* Stash DTB pointer passed by m1n1 */
	adr	x1, fw_dtb_pointer
	str	x0, [x1]

	b	save_boot_params_ret
