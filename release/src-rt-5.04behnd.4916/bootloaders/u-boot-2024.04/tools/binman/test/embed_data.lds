/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	_start = .;
	__data_start = .;
	.data :
	{
		. = ALIGN(32);
		embed_start = .;
		*(.embed*)
		embed_end = .;
		region_size = 0;
		. = ALIGN(32);
		*(.data*)
	}
}
