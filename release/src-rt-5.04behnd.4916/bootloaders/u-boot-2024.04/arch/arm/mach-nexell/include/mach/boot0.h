/* SPDX-License-Identifier: GPL-2.0+
 *
 * NSIH (Nexell System Information Header) for FriendlyArm nanopi2 board
 *
 * The NSIH (first 512 Bytes of u-boot.bin) is necessary for the
 * 2nd-Bootloader to get information like load address of U-Boot.
 *
 * 0x400 must be added to CONFIG_TEXT_BASE to have the actual load and
 * start address because 2nd-Bootloader loads with an offset of 0x400
 * (NSIH + 0x200 bytes are not loaded into RAM).
 *
 * It has been tested / is working with the following 2nd-Bootloader:
 * "BL1 by Nexell V1.0.0-gd551e13 [Built on 2018-01-25 16:58:29]"
 *
 * (C) Copyright 2020 Stefan Bosch <stefan_b@posteo.net>
 */

#ifndef __BOOT0_H
#define __BOOT0_H

	ARM_VECTORS
	.space	0x30
	.word	(_end - _start) + 20 * 1024	/* 0x50: load size
						 *       (bin + 20k for DTB) */
	.space	0x4
	.word	CONFIG_TEXT_BASE + 0x400	/* 0x58: load address */
	.word	0x00000000
	.word	CONFIG_TEXT_BASE + 0x400	/* 0x60: start address */
	.space	0x198
	.byte	'N'				/* 0x1FC: "NSIH" signature */
	.byte	'S'
	.byte	'I'
	.byte	'H'

	/* The NSIH + 0x200 bytes are omitted by the 2nd-Bootloader */
	.space	0x200
_start:
	ARM_VECTORS

#endif /* __BOOT0_H */
