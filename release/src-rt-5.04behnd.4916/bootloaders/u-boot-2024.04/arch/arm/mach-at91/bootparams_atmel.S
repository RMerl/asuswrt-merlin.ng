/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Atmel SAMA5Dx boot parameter handling
 *
 * Copyright (c) 2016 Marek Vasut <marex@denx.de>
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/system.h>
#include <linux/linkage.h>

ENTRY(save_boot_params)
	ldr	r0, =bootrom_stash
	str	r4, [r0, #0]
	b	save_boot_params_ret
ENDPROC(save_boot_params)
