/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * iPAQ h2200 header
 *
 * Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
 */

	.word 0xea0003fe /* b 0x1000 */

	.org 0x40
	.ascii "ECEC"

	.org 0x1000 - 1
	.byte 0x0
