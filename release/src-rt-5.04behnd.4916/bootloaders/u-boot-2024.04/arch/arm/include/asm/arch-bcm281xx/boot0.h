/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Broadcom Corporation.
 */

/* BOOT0 header information */
_start:
	ARM_VECTORS
	.word	0xbabeface
	.word	_end - _start
