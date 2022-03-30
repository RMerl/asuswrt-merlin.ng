/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

OUTPUT_FORMAT("elf32-nds32", "elf32-nds32", "elf32-nds32")
OUTPUT_ARCH(nds32)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text :
	{
		*(.text)
	}

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);

	.got : {
		__got_start = .;
		*(.got)
		__got_end = .;
	}

	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss) }
	__bss_end = .;

	. = ALIGN(4);
	.rela.text : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }

	_end = .;
}
