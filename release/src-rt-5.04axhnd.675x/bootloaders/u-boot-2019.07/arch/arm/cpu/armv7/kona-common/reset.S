/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Broadcom Corporation.
 */

.globl reset_cpu
reset_cpu:
	ldr	r1, =0x35001f00
	ldr	r2, [r1]
	ldr	r4, =0x80000000
	and	r4, r2, r4
	ldr	r3, =0xA5A500
	orr	r4, r4, r3
	orr	r4, r4, #0x1

	str	r4, [r1]

	ldr	r1, =0x35001f04
	ldr	r2, [r1]
	ldr	r4, =0x80000000
	and	r4, r2, r4
	str	r4, [r1]

_loop_forever:
	b	_loop_forever
