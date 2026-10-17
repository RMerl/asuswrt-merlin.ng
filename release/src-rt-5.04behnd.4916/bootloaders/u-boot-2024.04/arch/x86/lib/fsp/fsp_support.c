// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <asm/fsp/fsp_support.h>
#include <asm/post.h>

u32 fsp_get_usable_lowmem_top(const void *hob_list)
{
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;
	phys_addr_t phys_start;
	u32 top;
#ifdef CONFIG_FSP_BROKEN_HOB
	struct hob_mem_alloc *res_mem;
	phys_addr_t mem_base = 0;
#endif

	/* Get the HOB list for processing */
	hdr = hob_list;

	/* * Collect memory ranges */
	top = FSP_LOWMEM_BASE;
	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			if (res_desc->type == RES_SYS_MEM) {
				phys_start = res_desc->phys_start;
				/* Need memory above 1MB to be collected here */
				if (phys_start >= FSP_LOWMEM_BASE &&
				    phys_start < (phys_addr_t)FSP_HIGHMEM_BASE)
					top += (u32)(res_desc->len);
			}
		}

#ifdef CONFIG_FSP_BROKEN_HOB
		/*
		 * Find out the lowest memory base address allocated by FSP
		 * for the boot service data
		 */
		if (hdr->type == HOB_TYPE_MEM_ALLOC) {
			res_mem = (struct hob_mem_alloc *)hdr;
			if (!mem_base)
				mem_base = res_mem->mem_base;
			if (res_mem->mem_base < mem_base)
				mem_base = res_mem->mem_base;
		}
#endif

		hdr = get_next_hob(hdr);
	}

#ifdef CONFIG_FSP_BROKEN_HOB
	/*
	 * Check whether the memory top address is below the FSP HOB list.
	 * If not, use the lowest memory base address allocated by FSP as
	 * the memory top address. This is to prevent U-Boot relocation
	 * overwrites the important boot service data which is used by FSP,
	 * otherwise the subsequent call to fsp_notify() will fail.
	 */
	if (top > (u32)hob_list) {
		debug("Adjust memory top address due to a buggy FSP\n");
		top = (u32)mem_base;
	}
#endif

	return top;
}

u64 fsp_get_usable_highmem_top(const void *hob_list)
{
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;
	phys_addr_t phys_start;
	u64 top;

	/* Get the HOB list for processing */
	hdr = hob_list;

	/* Collect memory ranges */
	top = FSP_HIGHMEM_BASE;
	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			if (res_desc->type == RES_SYS_MEM) {
				phys_start = res_desc->phys_start;
				/* Need memory above 4GB to be collected here */
				if (phys_start >= (phys_addr_t)FSP_HIGHMEM_BASE)
					top += (u32)(res_desc->len);
			}
		}
		hdr = get_next_hob(hdr);
	}

	return top;
}

u64 fsp_get_reserved_mem_from_guid(const void *hob_list, u64 *len,
				   const efi_guid_t *guid)
{
	const struct hob_header *hdr;
	struct hob_res_desc *res_desc;

	/* Get the HOB list for processing */
	hdr = hob_list;

	/* Collect memory ranges */
	while (!end_of_hob(hdr)) {
		if (hdr->type == HOB_TYPE_RES_DESC) {
			res_desc = (struct hob_res_desc *)hdr;
			if (res_desc->type == RES_MEM_RESERVED) {
				if (!guidcmp(&res_desc->owner, guid)) {
					if (len)
						*len = (u32)(res_desc->len);

					return (u64)(res_desc->phys_start);
				}
			}
		}
		hdr = get_next_hob(hdr);
	}

	return 0;
}

u32 fsp_get_fsp_reserved_mem(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_HOB_RESOURCE_OWNER_FSP_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, &guid);
	if (len && base)
		*len = (u32)length;

	return base;
}

u32 fsp_get_tseg_reserved_mem(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_HOB_RESOURCE_OWNER_TSEG_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, &guid);
	if (len && base)
		*len = (u32)length;

	return base;
}

void *fsp_get_nvs_data(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;

	return hob_get_guid_hob_data(hob_list, len, &guid);
}

void *fsp_get_var_nvs_data(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_VARIABLE_NV_DATA_HOB_GUID;

	return hob_get_guid_hob_data(hob_list, len, &guid);
}

void *fsp_get_bootloader_tmp_mem(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_BOOTLOADER_TEMP_MEM_HOB_GUID;

	return hob_get_guid_hob_data(hob_list, len, &guid);
}

void *fsp_get_graphics_info(const void *hob_list, u32 *len)
{
	const efi_guid_t guid = FSP_GRAPHICS_INFO_HOB_GUID;

	return hob_get_guid_hob_data(hob_list, len, &guid);
}
