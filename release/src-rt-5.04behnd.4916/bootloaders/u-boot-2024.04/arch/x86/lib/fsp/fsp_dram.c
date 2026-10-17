// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <handoff.h>
#include <init.h>
#include <log.h>
#include <asm/fsp/fsp_support.h>
#include <asm/e820.h>
#include <asm/global_data.h>
#include <asm/mrccache.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

int fsp_scan_for_ram_size(void)
{
	phys_size_t ram_size = 0;
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;

	hdr = gd->arch.hob_list;
	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			if (res_desc->type == RES_SYS_MEM ||
			    res_desc->type == RES_MEM_RESERVED)
				ram_size += res_desc->len;
		}
		hdr = get_next_hob(hdr);
	}

	gd->ram_size = ram_size;
	post_code(POST_DRAM);

	return 0;
};

int dram_init_banksize(void)
{
	efi_guid_t fsp = FSP_HOB_RESOURCE_OWNER_FSP_GUID;
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;
	phys_addr_t mtrr_top;
	phys_addr_t low_end;
	uint bank;
	bool update_mtrr;

	/*
	 * For FSP1, the system memory and reserved memory used by FSP are
	 * already programmed in the MTRR by FSP. Also it is observed that
	 * FSP on Intel Queensbay platform reports the TSEG memory range
	 * that has the same RES_MEM_RESERVED resource type whose address
	 * is programmed by FSP to be near the top of 4 GiB space, which is
	 * not what we want for DRAM.
	 *
	 * However it seems FSP2's behavior is different. We need to add the
	 * DRAM range in MTRR otherwise the boot process goes very slowly,
	 * which was observed on Chromebook Coral with FSP2.
	 */
	update_mtrr = CONFIG_IS_ENABLED(FSP_VERSION2);

	if (!ll_boot_init()) {
		gd->bd->bi_dram[0].start = 0;
		gd->bd->bi_dram[0].size = gd->ram_size;

		if (update_mtrr)
			mtrr_add_request(MTRR_TYPE_WRBACK, 0, gd->ram_size);
		return 0;
	}

	low_end = 0;	/* top of low memory usable by U-Boot */
	mtrr_top = 0;	/* top of low memory (even if reserved) */
	for (bank = 1, hdr = gd->arch.hob_list;
	     bank < CONFIG_NR_DRAM_BANKS && !end_of_hob(hdr);
	     hdr = get_next_hob(hdr)) {
		if (hdr->type != HOB_TYPE_RES_DESC)
			continue;
		res_desc = (struct hob_res_desc *)hdr;
		if (!guidcmp(&res_desc->owner, &fsp))
			low_end = res_desc->phys_start;
		if (res_desc->type != RES_SYS_MEM &&
		    res_desc->type != RES_MEM_RESERVED)
			continue;
		if (res_desc->phys_start < (1ULL << 32)) {
			mtrr_top = max(mtrr_top,
				       res_desc->phys_start + res_desc->len);
		} else {
			gd->bd->bi_dram[bank].start = res_desc->phys_start;
			gd->bd->bi_dram[bank].size = res_desc->len;
			if (update_mtrr)
				mtrr_add_request(MTRR_TYPE_WRBACK,
						 res_desc->phys_start,
						 res_desc->len);
			log_debug("ram %llx %llx\n",
				  gd->bd->bi_dram[bank].start,
				  gd->bd->bi_dram[bank].size);
		}
	}

	/* Add the memory below 4GB */
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = low_end;

	/*
	 * Set up an MTRR to the top of low, reserved memory. This is necessary
	 * for graphics to run at full speed in U-Boot.
	 */
	if (update_mtrr)
		mtrr_add_request(MTRR_TYPE_WRBACK, 0, mtrr_top);

	return 0;
}

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	unsigned int num_entries = 0;
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;
	const fdt64_t *prop;
	int size;

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

	if (IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)) {
		ulong stack_size;

		stack_size = CONFIG_IS_ENABLED(HAVE_ACPI_RESUME,
					       (CONFIG_STACK_SIZE_RESUME), (0));
		/*
		 * Everything between U-Boot's stack and ram top needs to be
		 * reserved in order for ACPI S3 resume to work.
		 */
		entries[num_entries].addr = gd->start_addr_sp - stack_size;
		entries[num_entries].size = gd->ram_top - gd->start_addr_sp +
			stack_size;
		entries[num_entries].type = E820_RESERVED;
		num_entries++;
	}

	prop = ofnode_read_chosen_prop("e820-entries", &size);
	if (prop) {
		int count = size / (sizeof(u64) * 3);
		int i;

		if (num_entries + count >= max_entries)
			return -ENOSPC;
		for (i = 0; i < count; i++, num_entries++, prop += 3) {
			entries[num_entries].addr = fdt64_to_cpu(prop[0]);
			entries[num_entries].size = fdt64_to_cpu(prop[1]);
			entries[num_entries].type = fdt64_to_cpu(prop[2]);
		}
	}

	return num_entries;
}

#if CONFIG_IS_ENABLED(HANDOFF) && IS_ENABLED(CONFIG_USE_HOB)
int handoff_arch_save(struct spl_handoff *ho)
{
	ho->arch.usable_ram_top = gd->bd->bi_dram[0].size;
	ho->arch.hob_list = gd->arch.hob_list;

	return 0;
}
#endif
