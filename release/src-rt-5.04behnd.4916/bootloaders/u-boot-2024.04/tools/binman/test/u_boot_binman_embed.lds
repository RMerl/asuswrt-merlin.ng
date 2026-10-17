/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x00000000;
	_start = .;

	. = ALIGN(4);
	.text :
	{
		*(.text*)
	}

	. = ALIGN(4);
	.data : {
		dtb_embed_begin = .;
		KEEP(*(.mydtb));
		dtb_embed_end = .;
	}
	.interp : { *(.interp*) }

}
