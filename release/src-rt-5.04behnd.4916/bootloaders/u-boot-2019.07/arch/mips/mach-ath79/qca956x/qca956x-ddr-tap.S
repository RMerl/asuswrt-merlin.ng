// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Rosy Song <rosysong@rosinson.com>
 *
 * Based on QSDK
 */

#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <mach/ar71xx_regs.h>

    .set noreorder

LEAF(ddr_tap_tuning)
	li	a0, 0xbd001f00
	sw	zero, 0x0(a0)			/* Place where the tap values are saved and used for SWEEP */
	sw	zero, 0x4(a0)			/* Place where the number of passing taps are saved. */
	sw	zero, 0x14(a0)		/* Place where the last pass tap value is stored */
	li	a1, 0xaa55aa55		/* Indicates that the First pass tap value is not found */
	sw	a1, 0x10(a0)		/* Place where the First pass tap value is stored */
	 nop

	li	a0, CKSEG1ADDR(AR71XX_RESET_BASE)		/* RESET_BASE_ADDRESS */
	lw	a1, 0x1c(a0)		/* Reading the RST_RESET_ADDRESS */
	li	a2, 0x08000000		/* Setting the RST_RESET_RTC_RESET */
	or	a1, a1, a2
	sw	a1, 0x1c(a0)

	li	a3, 0xffffffff
	xor	a2, a2, a3
	and	a1, a1, a2
	sw	a1, 0x1c(a0)		/* Taking the RTC out of RESET */
	 nop

	li	a0, CKSEG1ADDR(QCA956X_RTC_BASE)		/* RTC_BASE_ADDRESS */
	li	a1, 0x1
	sw	a1, 0x0040(a0)		/* RTC_SYNC_RESET_ADDRESS */

	li	a2, 0x2

_poll_for_RTC_ON:
	lw	a1, 0x0044(a0)		/* RTC_SYNC_STATUS_ADDRESS */
	and	a1, a2, a1
	bne	a1, a2, _poll_for_RTC_ON
	  nop

_CHANGE_TAPS:
	li	t0, 0xbd001f00		/* Read the current value of the TAP for programming */
	lw	t1, 0x0(t0)
	li	t2, 0x00000000
	or	t3, t1, t2

	li	t0, 0xb8000000		/* DDR_BASE_ADDRESS */
	sw	t3, 0x1c(t0)		/* TAP_CONTROL_0_ADDRESS */
	sw	t3, 0x20(t0)		/* TAP_CONTROL_1_ADDRESS */
	sw	t3, 0x24(t0)		/* TAP_CONTROL_2_ADDRESS */
	sw	t3, 0x28(t0)		/* TAP_CONTROL_3_ADDRESS */

	li	t1, 0x00000010		/* Running the test 8 times */
	sw	t1, 0x0068(t0)		/* PERF_COMP_ADDR_1_ADDRESS */

	li	t1, 0xfa5de83f		/* 4 Row Address Bits, 4 Column Address Bits, 2 BA bits */
	sw	t1, 0x002c(t0)		/* PERF_MASK_ADDR_0_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0070(t0)		/* PERF_COMP_AHB_GE0_1_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0040(t0)		/* PERF_COMP_AHB_GE1_0_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0078(t0)		/* PERF_COMP_AHB_GE1_1_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0034(t0)		/* PERF_MASK_AHB_GE0_0_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x006c(t0)		/* PERF_MASK_AHB_GE0_1_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x003c(t0)		/* PERF_MASK_AHB_GE1_0_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0074(t0)		/* PERF_MASK_AHB_GE1_1_ADDRESS */

	li	t1, 0x0000ffff
	sw	t1, 0x0038(t0)		/* PERF_COMP_AHB_GE0_0_ADDRESS */

	li	t1, 0x00000001
	sw	t1, 0x011c(t0)		/* DDR_BIST_ADDRESS */

	li	t2, 0x1

_bist_done_poll:
	lw	t1, 0x0120(t0)		/* DDR_BIST_STATUS_ADDRESS */
	and	t1, t1, t2
	bne	t1, t2, _bist_done_poll
	 nop

	lw	t1, 0x0120(t0)		/* DDR_BIST_STATUS_ADDRESS */
	li	t4, 0x000001fe
	and	t2, t1, t4
	srl	t2, t2, 0x1		/* no. of Pass Runs */

	li	t5, 0x00000000
	sw	t5, 0x011c(t0)		/* DDR_BIST_ADDRESS	- Stop the DDR BIST test */

	li	t5, 0x0001fe00
	and	t5, t5, t1
	bnez	t5, _iterate_tap		/* This is a redundant compare but nevertheless - Comparing the FAILS */
	 nop

	lw	t1, 0x0068(t0)		/* PERF_COMP_ADDR_1_ADDRESS */
	li	t3, 0x000001fe
	and	t3, t3, t1
	srl	t3, t3, 0x1		/* No. of runs in the config register. */
	bne	t3, t2, _iterate_tap
	 nop

pass_tap:
	li	t0, 0xbd001f00
	lw	t1, 0x4(t0)
	addiu	t1, t1, 0x1
	sw	t1, 0x4(t0)

	li	t0, 0xbd001f10
	lw	t1, 0x0(t0)
	li	t2, 0xaa55aa55
	beq	t1, t2, _first_pass
	 nop

	li	t0, 0xbd001f00
	lw	t1, 0x0(t0)
	li	t0, 0xbd001f10
	sw	t1, 0x4(t0)
	 nop
	b	_iterate_tap
	 nop

_first_pass:
	li	t0, 0xbd001f00
	lw	t1, 0x0(t0)
	li	t0, 0xbd001f10
	sw	t1, 0x0(t0)
	sw	t1, 0x4(t0)
	 nop

_iterate_tap:
	li	t0, 0xbd001f00
	lw	t1, 0x0(t0)
	li	t2, 0x3f
	beq	t1, t2, _STOP_TEST
	 nop

	addiu	t1, t1, 0x1
	sw	t1, 0x0(t0)
	 nop
	b	_CHANGE_TAPS
	 nop

_STOP_TEST:
	li	t0, 0xbd001f00
	lw	t1, 0x4(t0)
	bnez	t1, _load_center_tap
	 nop

	li	t3, 0x8			/* Default Tap to be used */
	b	_load_tap_into_reg
	 nop

_load_center_tap:
	li	t0, 0xbd001f10
	lw	t1, 0x0(t0)
	lw	t2, 0x4(t0)
	add	t3, t1, t2
	srl	t3, t3, 0x1
	li	t4, 0x3f
	and	t3, t3, t4

_load_tap_into_reg:
	li	t0, 0xb8000000
	sw	t3, 0x1c(t0)		/* TAP_CONTROL_0_ADDRESS */
	sw	t3, 0x20(t0)		/* TAP_CONTROL_1_ADDRESS */
	sw	t3, 0x24(t0)		/* TAP_CONTROL_2_ADDRESS */
	sw	t3, 0x28(t0)		/* TAP_CONTROL_3_ADDRESS */

	 nop
	jr ra
	 nop
    END(ddr_tap_tuning)
