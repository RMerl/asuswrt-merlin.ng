/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * Copyright 2021 Broadcom Ltd.
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text :
	{
	__image_copy_start = .;
#if defined(CONFIG_MMC) && defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD) 
	/* 63138 eMMC requires use of the bootrom, which requires the entry point 
	to be at the very begining of the image binary (image starts executing from 
        the address where the bootrom has placed it). We therefore make sure that 
        the _bcm_boot (.vectors) code is at the start of the image */
	        *(.vectors)
	        *(.text*)
	        *(.glue*)
#else 	
	/* For 63138, the NAND image is laid out in such a way that only
	the _bcm_boot (.vectors) code  is aligned to the 0xYYY1_0000 Flash XIP reset
	vector. This is done to utilize the NAND XIP functionality, and allows
	the rest of the SPL code to also occupy the first 64K of NAND flash. */
		*(EXCLUDE_FILE (common/built-in.o) .text*)
		*(.glue*)
		
		. = 0x00010000;
		/* 
		 * Install the boot entry at SOC XIP address at 64K offset and extra
		 * code after boot entry
		 */
		*(.vectors)
		common/built-in.o(.text*)
#endif
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	.binman_sym_table : {
		__binman_sym_start = .;
		KEEP(*(SORT(.binman_sym*)));
		__binman_sym_end = .;
	}

	. = ALIGN(4);

	__image_copy_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	.end :
	{
		*(.__end)
	}

	_image_binary_end = .;

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.bss*)
		 . = ALIGN(4);
		__bss_end = .;
	}
	__bss_size = __bss_end - __bss_start;
	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.hash : { *(.hash*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
}

#if defined(IMAGE_MAX_SIZE)
ASSERT(__image_copy_end - __image_copy_start <= (IMAGE_MAX_SIZE), \
	"SPL image too big");
#endif

#if defined(CONFIG_SPL_BSS_MAX_SIZE)
ASSERT(__bss_end - __bss_start <= (CONFIG_SPL_BSS_MAX_SIZE), \
	"SPL image BSS too big");
#endif

#if defined(CONFIG_SPL_MAX_FOOTPRINT)
ASSERT(__bss_end - _start <= (CONFIG_SPL_MAX_FOOTPRINT), \
	"SPL image plus BSS too big");
#endif
