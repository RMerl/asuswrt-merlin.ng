/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <linux/linkage.h>

.extern	preloader_param

ENTRY(save_boot_params)
	ldr	r6, =preloader_param
	str	r4, [r6]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(lowlevel_init)
	/* enable SMP bit */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1
	mov	pc, lr
ENDPROC(lowlevel_init)
