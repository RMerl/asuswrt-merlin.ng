/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#ifndef _RELOCATE_H_
#define _RELOCATE_H_

#ifndef USE_HOSTCC
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;
#endif

/**
 * copy_uboot_to_ram() - Copy U-Boot to its new relocated position
 *
 * Return: 0 if OK, -ve on error
 */
int copy_uboot_to_ram(void);

/**
 * clear_bss() - Clear the BSS (Blocked Start by Symbol) segment
 *
 * This clears the memory used by global variables
 *
 * Return: 0 if OK, -ve on error
 */
int clear_bss(void);

/**
 * do_elf_reloc_fixups() - Fix up ELF relocations in the relocated code
 *
 * This processes the relocation tables to ensure that the code can run in its
 * new location.
 *
 * Return: 0 if OK, -ve on error
 */
int do_elf_reloc_fixups(void);

#endif	/* _RELOCATE_H_ */
