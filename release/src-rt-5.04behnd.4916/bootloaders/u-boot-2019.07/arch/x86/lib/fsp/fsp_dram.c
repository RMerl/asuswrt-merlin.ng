// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/fsp/fsp_support.h>
#include <asm/e820.h>
#include <asm/mrccache.h>
#include <asm/post.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	phys_size_t ram_size = 0;
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;

	hdr = gd->arch.hob_list;
	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			if (res_desc->type == RES_SYS_MEM ||
			    res_desc->type == RES_MEM_RESERVED) {
				ram_size += res_desc->len;
			}
		}
		hdr = get_next_hob(hdr);
	}

	gd->ram_size = ram_size;
	post_code(POST_DRAM);

#ifdef CONFIG_ENABLE_MRC_CACHE
	gd->arch.mrc_output = fsp_get_nvs_data(gd->arch.hob_list,
					       &gd->arch.mrc_output_len);
#endif

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return fsp_get_usable_lowmem_top(gd->arch.hob_list);
}

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	unsigned int num_entries = 0;
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;

	hdr = gd->arch.hob_list;

	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			entries[num_entries].addr = res_desc->phys_start;
			entries[num_entries].size = res_desc->len;

			if (res_desc->type == RES_SYS_MEM)
				entries[num_entries].type = E820_RAM;
			else if (res_desc->type == RES_MEM_RESERVED)
				entries[num_entries].type = E820_RESERVED;

			num_entries++;
		}
		hdr = get_next_hob(hdr);
	}

	/* Mark PCIe ECAM address range as reserved */
	entries[num_entries].addr = CONFIG_PCIE_ECAM_BASE;
	entries[num_entries].size = CONFIG_PCIE_ECAM_SIZE;
	entries[num_entries].type = E820_RESERVED;
	num_entries++;

#ifdef CONFIG_HAVE_ACPI_RESUME
	/*
	 * Everything between U-Boot's stack and ram top needs to be
	 * reserved in order for ACPI S3 resume to work.
	 */
	entries[num_entries].addr = gd->start_addr_sp - CONFIG_STACK_SIZE;
	entries[num_entries].size = gd->ram_top - gd->start_addr_sp + \
		CONFIG_STACK_SIZE;
	entries[num_entries].type = E820_RESERVED;
	num_entries++;
#endif

	return num_entries;
}
