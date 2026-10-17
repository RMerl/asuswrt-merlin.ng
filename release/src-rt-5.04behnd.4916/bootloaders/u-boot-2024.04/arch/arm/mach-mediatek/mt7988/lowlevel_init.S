/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

/*
 * Switch from AArch64 EL2 to AArch32 EL2
 * @param inputs:
 * x0: argument, zero
 * x1: machine nr
 * x2: fdt address
 * x3: input argument
 * x4: kernel entry point
 * @param outputs for secure firmware:
 * x0: function id
 * x1: kernel entry point
 * x2: machine nr
 * x3: fdt address
*/

.global armv8_el2_to_aarch32
armv8_el2_to_aarch32:
	mov     x3, x2
	mov     x2, x1
	mov     x1, x4
	mov	x4, #0
	ldr x0, =0x82000200
	SMC #0
	ret
