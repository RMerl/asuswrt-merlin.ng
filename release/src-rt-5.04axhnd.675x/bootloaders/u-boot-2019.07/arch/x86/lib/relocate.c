// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <relocate.h>
#include <asm/u-boot-x86.h>
#include <asm/sections.h>
#include <elf.h>

DECLARE_GLOBAL_DATA_PTR;

int copy_uboot_to_ram(void)
{
	size_t len = (uintptr_t)&__data_end - (uintptr_t)&__text_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	memcpy((void *)gd->relocaddr, (void *)&__text_start, len);

	return 0;
}

int clear_bss(void)
{
	ulong dst_addr = (ulong)&__bss_start + gd->reloc_off;
	size_t len = (uintptr_t)&__bss_end - (uintptr_t)&__bss_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	memset((void *)dst_addr, 0x00, len);

	return 0;
}

#if CONFIG_IS_ENABLED(X86_64)
static void do_elf_reloc_fixups64(unsigned int text_base, uintptr_t size,
				  Elf64_Rela *re_src, Elf64_Rela *re_end)
{
	Elf64_Addr *offset_ptr_rom, *last_offset = NULL;
	Elf64_Addr *offset_ptr_ram;

	do {
		unsigned long long type = ELF64_R_TYPE(re_src->r_info);

		if (type != R_X86_64_RELATIVE) {
			printf("%s: unsupported relocation type 0x%llx "
			       "at %p, ", __func__, type, re_src);
			printf("offset = 0x%llx\n", re_src->r_offset);
			continue;
		}

		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf64_Addr *)(uintptr_t)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf64_Addr *)(uintptr_t)text_base &&
		    offset_ptr_rom > last_offset) {
			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf64_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

			/* Check that the target points into .text */
			if (*offset_ptr_ram >= text_base &&
			    *offset_ptr_ram <= text_base + size) {
				*offset_ptr_ram = gd->reloc_off +
							re_src->r_addend;
			} else {
				debug("   %p: %lx: rom reloc %lx, ram %p, value %lx, limit %lX\n",
				      re_src, (ulong)re_src->r_info,
				      (ulong)re_src->r_offset, offset_ptr_ram,
				      (ulong)*offset_ptr_ram, text_base + size);
			}
		} else {
			debug("   %p: %lx: rom reloc %lx, last %p\n", re_src,
			      (ulong)re_src->r_info, (ulong)re_src->r_offset,
			      last_offset);
		}
		last_offset = offset_ptr_rom;

	} while (++re_src < re_end);
}
#else
static void do_elf_reloc_fixups32(unsigned int text_base, uintptr_t size,
				  Elf32_Rel *re_src, Elf32_Rel *re_end)
{
	Elf32_Addr *offset_ptr_rom, *last_offset = NULL;
	Elf32_Addr *offset_ptr_ram;

	do {
		unsigned int type = ELF32_R_TYPE(re_src->r_info);

		if (type != R_386_RELATIVE) {
			printf("%s: unsupported relocation type 0x%x "
			       "at %p, ", __func__, type, re_src);
			printf("offset = 0x%x\n", re_src->r_offset);
			continue;
		}

		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf32_Addr *)(uintptr_t)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf32_Addr *)(uintptr_t)text_base &&
		    offset_ptr_rom > last_offset) {

			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf32_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

			/* Check that the target points into .text */
			if (*offset_ptr_ram >= text_base &&
			    *offset_ptr_ram <= text_base + size) {
				*offset_ptr_ram += gd->reloc_off;
			} else {
				debug("   %p: rom reloc %x, ram %p, value %x, limit %lX\n",
				      re_src, re_src->r_offset, offset_ptr_ram,
				      *offset_ptr_ram, text_base + size);
			}
		} else {
			debug("   %p: rom reloc %x, last %p\n", re_src,
			       re_src->r_offset, last_offset);
		}
		last_offset = offset_ptr_rom;

	} while (++re_src < re_end);
}
#endif

/*
 * This function has more error checking than you might expect. Please see
 * this commit message for more information:
 *    62f7970a x86: Add error checking to x86 relocation code
 */
int do_elf_reloc_fixups(void)
{
	void *re_src = (void *)(&__rel_dyn_start);
	void *re_end = (void *)(&__rel_dyn_end);
	uint text_base;

	/* The size of the region of u-boot that runs out of RAM. */
	uintptr_t size = (uintptr_t)&__bss_end - (uintptr_t)&__text_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	if (re_src == re_end)
		panic("No relocation data");

#ifdef CONFIG_SYS_TEXT_BASE
	text_base = CONFIG_SYS_TEXT_BASE;
#else
	panic("No CONFIG_SYS_TEXT_BASE");
#endif
#if CONFIG_IS_ENABLED(X86_64)
	do_elf_reloc_fixups64(text_base, size, re_src, re_end);
#else
	do_elf_reloc_fixups32(text_base, size, re_src, re_end);
#endif

	return 0;
}
