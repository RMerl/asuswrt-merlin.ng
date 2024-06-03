/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <config.h>
#include <asm/psci.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
#ifndef CONFIG_CMDLINE
	/DISCARD/ : { *(.u_boot_list_2_cmd_*) }
#endif
#if defined(CONFIG_ARMV7_SECURE_BASE) && defined(CONFIG_ARMV7_NONSEC)
	/*
	 * If CONFIG_ARMV7_SECURE_BASE is true, secure code will not
	 * bundle with u-boot, and code offsets are fixed. Secure zone
	 * only needs to be copied from the loading address to
	 * CONFIG_ARMV7_SECURE_BASE, which is the linking and running
	 * address for secure code.
	 *
	 * If CONFIG_ARMV7_SECURE_BASE is undefined, the secure zone will
	 * be included in u-boot address space, and some absolute address
	 * were used in secure code. The absolute addresses of the secure
	 * code also needs to be relocated along with the accompanying u-boot
	 * code.
	 *
	 * So DISCARD is only for CONFIG_ARMV7_SECURE_BASE.
	 */
	/DISCARD/ : { *(.rel._secure*) }
#endif
	. = 0x00000000;

	. = ALIGN(4);
	.text :
	{
		*(.__image_copy_start)
		*(.vectors)
		CPUDIR/start.o (.text*)
	}

	/* This needs to come before *(.text*) */
	.__efi_runtime_start : {
		*(.__efi_runtime_start)
	}

	.efi_runtime : {
		*(.text.efi_runtime*)
		*(.rodata.efi_runtime*)
		*(.data.efi_runtime*)
	}

	.__efi_runtime_stop : {
		*(.__efi_runtime_stop)
	}

	.text_rest :
	{
		*(.text*)
	}

#ifdef CONFIG_ARMV7_NONSEC

	/* Align the secure section only if we're going to use it in situ */
	.__secure_start
#ifndef CONFIG_ARMV7_SECURE_BASE
		ALIGN(CONSTANT(COMMONPAGESIZE))
#endif
	: {
		KEEP(*(.__secure_start))
	}

#ifndef CONFIG_ARMV7_SECURE_BASE
#define CONFIG_ARMV7_SECURE_BASE
#define __ARMV7_PSCI_STACK_IN_RAM
#endif

	.secure_text CONFIG_ARMV7_SECURE_BASE :
		AT(ADDR(.__secure_start) + SIZEOF(.__secure_start))
	{
		*(._secure.text)
	}

	.secure_data : AT(LOADADDR(.secure_text) + SIZEOF(.secure_text))
	{
		*(._secure.data)
	}

#ifdef CONFIG_ARMV7_PSCI
	.secure_stack ALIGN(ADDR(.secure_data) + SIZEOF(.secure_data),
			    CONSTANT(COMMONPAGESIZE)) (NOLOAD) :
#ifdef __ARMV7_PSCI_STACK_IN_RAM
		AT(ADDR(.secure_stack))
#else
		AT(LOADADDR(.secure_data) + SIZEOF(.secure_data))
#endif
	{
		KEEP(*(.__secure_stack_start))

		/* Skip addreses for stack */
		. = . + CONFIG_ARMV7_PSCI_NR_CPUS * ARM_PSCI_STACK_SIZE;

		/* Align end of stack section to page boundary */
		. = ALIGN(CONSTANT(COMMONPAGESIZE));

		KEEP(*(.__secure_stack_end))

#ifdef CONFIG_ARMV7_SECURE_MAX_SIZE
		/*
		 * We are not checking (__secure_end - __secure_start) here,
		 * as these are the load addresses, and do not include the
		 * stack section. Instead, use the end of the stack section
		 * and the start of the text section.
		 */
		ASSERT((. - ADDR(.secure_text)) <= CONFIG_ARMV7_SECURE_MAX_SIZE,
		       "Error: secure section exceeds secure memory size");
#endif
	}

#ifndef __ARMV7_PSCI_STACK_IN_RAM
	/* Reset VMA but don't allocate space if we have secure SRAM */
	. = LOADADDR(.secure_stack);
#endif

#endif

	.__secure_end : AT(ADDR(.__secure_end)) {
		*(.__secure_end)
		LONG(0x1d1071c);	/* Must output something to reset LMA */
	}
#endif

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);

	. = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

	.efi_runtime_rel_start :
	{
		*(.__efi_runtime_rel_start)
	}

	.efi_runtime_rel : {
		*(.rel*.efi_runtime)
		*(.rel*.efi_runtime.*)
	}

	.efi_runtime_rel_stop :
	{
		*(.__efi_runtime_rel_stop)
	}

	. = ALIGN(4);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rel.dyn : {
		*(.rel*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	.end :
	{
		*(.__end)
	}

	_image_binary_end = .;

	/*
	 * Deprecated: this MMU section is used by pxa at present but
	 * should not be used by new boards/CPUs.
	 */
	. = ALIGN(4096);
	.mmutable : {
		*(.mmutable)
	}

/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}

	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_limit = .;
	}

	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}

	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu.hash : { *(.gnu.hash) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
	.gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }
}
