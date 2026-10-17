/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

.align 8
.global fdt_base_addr
fdt_base_addr:
	.dword 0x0

.global save_boot_params
save_boot_params:
	/* Read FDT base from x1 register passed by ATF */
	adr	x21, fdt_base_addr
	str	x1, [x21]

	/* Returns */
	b	save_boot_params_ret

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

	/* any lowlevel init should go here */

	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)
