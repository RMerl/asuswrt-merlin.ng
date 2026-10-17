/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <config.h>
#include <asm-offsets.h>
#include <asm/regdef.h>
#include <asm/asm.h>

	.set noreorder

NESTED(lowlevel_init, 0, ra)
	/* Save ra and do real lowlevel initialization */
	move	s0, ra

	/*
	* Use SRAM from 802.11n MAC/BBP, 16KiB (0x10184000 ~ 0x10187fff)
	* NOTE: non-word operations may fail in this SRAM.
	* Use it as stack only for CPU/DRAM init which only has word operations.
	*/
	PTR_LI	sp, 0xb0187f00

	/* We still need a temporary gd for udelay */
	PTR_SUBU \
		sp, sp, GD_SIZE		# reserve space for gd
	li	t0, -16
	and	sp, sp, t0		# force 16 byte alignment
	move	k0, sp			# save gd pointer

	move	fp, sp

	/* Clear gd */
	move	t0, k0
1:
	PTR_S	zero, 0(t0)
	PTR_ADDIU t0, PTRSIZE
	blt	t0, t1, 1b
	 nop

	/* Do CPU & DRAM initialization */
	PTR_LA	t9, mt7620_init
	jalr	t9
	 nop

	/* Restore ra */
	move	ra, s0

	jr	ra
	 nop
	END(lowlevel_init)
