// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <elf.h>
#include <asm-generic/sections.h>

extern ulong __image_copy_start;
extern ulong __ivt_start;
extern ulong __ivt_end;
extern ulong __text_end;

DECLARE_GLOBAL_DATA_PTR;

int copy_uboot_to_ram(void)
{
	size_t len = (size_t)&__image_copy_end - (size_t)&__image_copy_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;

	memcpy((void *)gd->relocaddr, (void *)&__image_copy_start, len);

	return 0;
}

int clear_bss(void)
{
	ulong dst_addr = (ulong)&__bss_start + gd->reloc_off;
	size_t len = (size_t)&__bss_end - (size_t)&__bss_start;

	memset((void *)dst_addr, 0x00, len);

	return 0;
}

/*
 * Base functionality is taken from x86 version with added ARC-specifics
 */
int do_elf_reloc_fixups(void)
{
	Elf32_Rela *re_src = (Elf32_Rela *)(&__rel_dyn_start);
	Elf32_Rela *re_end = (Elf32_Rela *)(&__rel_dyn_end);

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;

	debug("Section .rela.dyn is located at %08x-%08x\n",
	      (unsigned int)re_src, (unsigned int)re_end);

	Elf32_Addr *offset_ptr_rom;
	Elf32_Addr *offset_ptr_ram;

	do {
		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf32_Addr *)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf32_Addr *)&__image_copy_start &&
		    offset_ptr_rom < (Elf32_Addr *)&__image_copy_end) {
			unsigned int val, do_swap = 0;
			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf32_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

#ifdef __LITTLE_ENDIAN__
			/* If location in ".text" section swap value */
			if (((u32)offset_ptr_rom >= (u32)&__text_start &&
			     (u32)offset_ptr_rom <= (u32)&__text_end)
#if defined(__ARC700__) || defined(__ARC600__)
			    || ((u32)offset_ptr_rom >= (u32)&__ivt_start &&
				(u32)offset_ptr_rom <= (u32)&__ivt_end)
#endif
			   )
				do_swap = 1;
#endif

			debug("Patching value @ %08x (relocated to %08x)%s\n",
			      (unsigned int)offset_ptr_rom,
			      (unsigned int)offset_ptr_ram,
			      do_swap ? ", middle-endian encoded" : "");

			/*
			 * Use "memcpy" because target location might be
			 * 16-bit aligned on ARC so we may need to read
			 * byte-by-byte. On attempt to read entire word by
			 * CPU throws an exception
			 */
			memcpy(&val, offset_ptr_ram, sizeof(int));

			if (do_swap)
				val = (val << 16) | (val >> 16);

			/* Check that the target points into executable */
			if (val < (unsigned int)&__image_copy_start ||
			    val > (unsigned int)&__image_copy_end) {
				/* TODO: Use panic() instead of debug()
				 *
				 * For some reason GCC might generate
				 * fake relocation even for LD/SC of constant
				 * inderectly. See an example below:
				 * ----------------------->8--------------------
				 * static int setup_mon_len(void)
				 * {
				 *         gd->mon_len = (ulong)&__bss_end - CONFIG_SYS_MONITOR_BASE;
				 *         return 0;
				 * }
				 * ----------------------->8--------------------
				 *
				 * And that's what we get in the binary:
				 * ----------------------->8--------------------
				 * 10005cb4 <setup_mon_len>:
				 * 10005cb4:       193c 3f80 0003 2f80     st      0x32f80,[r25,60]
				 *                         10005cb8: R_ARC_RELATIVE        *ABS*-0x10000000
				 * 10005cbc:       7fe0                    j_s.d   [blink]
				 * 10005cbe:       700c                    mov_s   r0,0
				 * ----------------------->8--------------------
				 */
				debug("Relocation target %08x points outside of image\n",
				      val);
			}

			val += gd->reloc_off;

			if (do_swap)
				val = (val << 16) | (val >> 16);

			memcpy(offset_ptr_ram, &val, sizeof(int));
		}
	} while (++re_src < re_end);

	return 0;
}
