// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2022 Advanced Micro Devices, Inc
 * Michal Simek <michal.simek@amd.com>
 */

#include <elf.h>
#include <log.h>
#include <linux/types.h>

#define R_MICROBLAZE_NONE	0
#define R_MICROBLAZE_32		1
#define R_MICROBLAZE_REL	16
#define R_MICROBLAZE_GLOB_DAT	18

/**
 * mb_fix_rela - update relocation to new address
 * @reloc_addr: new relocation address
 * @verbose: enable version messages
 * @rela_start: rela section start
 * @rela_end: rela section end
 * @dyn_start: dynamic section start
 * @origin_addr: address where u-boot starts(doesn't need to be CONFIG_TEXT_BASE)
 */
void mb_fix_rela(u32 reloc_addr, u32 verbose, u32 rela_start,
		 u32 rela_end, u32 dyn_start, u32 origin_addr)
{
	u32 num, type, mask, i, reloc_off;

	/*
	 * Return in case u-boot.elf is used directly.
	 * Skip it when u-boot.bin is loaded to different address than
	 * CONFIG_TEXT_BASE. In this case relocation is necessary to run.
	 */
	if (reloc_addr == CONFIG_TEXT_BASE) {
		debug_cond(verbose,
			   "Relocation address is the same - skip relocation\n");
		return;
	}

	reloc_off = reloc_addr - origin_addr;

	debug_cond(verbose, "Relocation address:\t0x%08x\n", reloc_addr);
	debug_cond(verbose, "Relocation offset:\t0x%08x\n", reloc_off);
	debug_cond(verbose, "Origin address:\t0x%08x\n", origin_addr);
	debug_cond(verbose, "Rela start:\t0x%08x\n", rela_start);
	debug_cond(verbose, "Rela end:\t0x%08x\n", rela_end);
	debug_cond(verbose, "Dynsym start:\t0x%08x\n", dyn_start);

	num = (rela_end - rela_start) / sizeof(Elf32_Rela);

	debug_cond(verbose, "Number of entries:\t%u\n", num);

	for (i = 0; i < num; i++) {
		Elf32_Rela *rela;
		u32 temp;

		rela = (Elf32_Rela *)(rela_start + sizeof(Elf32_Rela) * i);

		mask = 0xffULL; /* would be different on 32-bit */
		type = rela->r_info & mask;

		debug_cond(verbose, "\nRela possition:\t%d/0x%x\n",
			   i, (u32)rela);

		switch (type) {
		case R_MICROBLAZE_REL:
			temp = *(u32 *)rela->r_offset;

			debug_cond(verbose, "Type:\tREL\n");
			debug_cond(verbose, "Rela r_offset:\t\t0x%x\n", rela->r_offset);
			debug_cond(verbose, "Rela r_info:\t\t0x%x\n", rela->r_info);
			debug_cond(verbose, "Rela r_addend:\t\t0x%x\n", rela->r_addend);
			debug_cond(verbose, "Value at r_offset:\t0x%x\n", temp);

			rela->r_offset += reloc_off;
			rela->r_addend += reloc_off;

			temp = *(u32 *)rela->r_offset;
			temp += reloc_off;
			*(u32 *)rela->r_offset = temp;

			debug_cond(verbose, "New:Rela r_offset:\t0x%x\n", rela->r_offset);
			debug_cond(verbose, "New:Rela r_addend:\t0x%x\n", rela->r_addend);
			debug_cond(verbose, "New:Value at r_offset:\t0x%x\n", temp);
			break;
		case R_MICROBLAZE_32:
		case R_MICROBLAZE_GLOB_DAT:
			debug_cond(verbose, "Type:\t(32/GLOB) %u\n", type);
			debug_cond(verbose, "Rela r_offset:\t\t0x%x\n", rela->r_offset);
			debug_cond(verbose, "Rela r_info:\t\t0x%x\n", rela->r_info);
			debug_cond(verbose, "Rela r_addend:\t\t0x%x\n", rela->r_addend);
			debug_cond(verbose, "Value at r_offset:\t0x%x\n", temp);

			rela->r_offset += reloc_off;

			temp = *(u32 *)rela->r_offset;
			temp += reloc_off;
			*(u32 *)rela->r_offset = temp;

			debug_cond(verbose, "New:Rela r_offset:\t0x%x\n", rela->r_offset);
			debug_cond(verbose, "New:Value at r_offset:\t0x%x\n", temp);
			break;
		case R_MICROBLAZE_NONE:
			debug_cond(verbose, "R_MICROBLAZE_NONE - skip\n");
			break;
		default:
			debug_cond(verbose, "warning: unsupported relocation type %d at %x\n",
				   type, rela->r_offset);
		}
	}
}
