// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/fsp/fsp_support.h>
#include <asm/post.h>

/**
 * Compares two GUIDs
 *
 * If the GUIDs are identical then true is returned.
 * If there are any bit differences in the two GUIDs, then false is returned.
 *
 * @guid1:        A pointer to a 128 bit GUID.
 * @guid2:        A pointer to a 128 bit GUID.
 *
 * @retval true:  guid1 and guid2 are identical.
 * @retval false: guid1 and guid2 are not identical.
 */
static bool compare_guid(const struct efi_guid *guid1,
			 const struct efi_guid *guid2)
{
	if (memcmp(guid1, guid2, sizeof(struct efi_guid)) == 0)
		return true;
	else
		return false;
}

struct fsp_header *__attribute__((optimize("O0"))) find_fsp_header(void)
{
	/*
	 * This function may be called before the a stack is established,
	 * so special care must be taken. First, it cannot declare any local
	 * variable using stack. Only register variable can be used here.
	 * Secondly, some compiler version will add prolog or epilog code
	 * for the C function. If so the function call may not work before
	 * stack is ready.
	 *
	 * GCC 4.8.1 has been verified to be working for the following codes.
	 */
	volatile register u8 *fsp asm("eax");

	/* Initalize the FSP base */
	fsp = (u8 *)CONFIG_FSP_ADDR;

	/* Check the FV signature, _FVH */
	if (((struct fv_header *)fsp)->sign == EFI_FVH_SIGNATURE) {
		/* Go to the end of the FV header and align the address */
		fsp += ((struct fv_header *)fsp)->ext_hdr_off;
		fsp += ((struct fv_ext_header *)fsp)->ext_hdr_size;
		fsp  = (u8 *)(((u32)fsp + 7) & 0xFFFFFFF8);
	} else {
		fsp  = 0;
	}

	/* Check the FFS GUID */
	if (fsp &&
	    ((struct ffs_file_header *)fsp)->name.data1 == FSP_GUID_DATA1 &&
	    ((struct ffs_file_header *)fsp)->name.data2 == FSP_GUID_DATA2 &&
	    ((struct ffs_file_header *)fsp)->name.data3 == FSP_GUID_DATA3 &&
	    ((struct ffs_file_header *)fsp)->name.data4[0] == FSP_GUID_DATA4_0 &&
	    ((struct ffs_file_header *)fsp)->name.data4[1] == FSP_GUID_DATA4_1 &&
	    ((struct ffs_file_header *)fsp)->name.data4[2] == FSP_GUID_DATA4_2 &&
	    ((struct ffs_file_header *)fsp)->name.data4[3] == FSP_GUID_DATA4_3 &&
	    ((struct ffs_file_header *)fsp)->name.data4[4] == FSP_GUID_DATA4_4 &&
	    ((struct ffs_file_header *)fsp)->name.data4[5] == FSP_GUID_DATA4_5 &&
	    ((struct ffs_file_header *)fsp)->name.data4[6] == FSP_GUID_DATA4_6 &&
	    ((struct ffs_file_header *)fsp)->name.data4[7] == FSP_GUID_DATA4_7) {
		/* Add the FFS header size to find the raw section header */
		fsp += sizeof(struct ffs_file_header);
	} else {
		fsp = 0;
	}

	if (fsp &&
	    ((struct raw_section *)fsp)->type == EFI_SECTION_RAW) {
		/* Add the raw section header size to find the FSP header */
		fsp += sizeof(struct raw_section);
	} else {
		fsp = 0;
	}

	return (struct fsp_header *)fsp;
}

void fsp_continue(u32 status, void *hob_list)
{
	post_code(POST_MRC);

	assert(status == 0);

	/* The boot loader main function entry */
	fsp_init_done(hob_list);
}

void fsp_init(u32 stack_top, u32 boot_mode, void *nvs_buf)
{
	struct fsp_config_data config_data;
	fsp_init_f init;
	struct fsp_init_params params;
	struct fspinit_rtbuf rt_buf;
	struct fsp_header *fsp_hdr;
	struct fsp_init_params *params_ptr;
#ifdef CONFIG_FSP_USE_UPD
	struct vpd_region *fsp_vpd;
	struct upd_region *fsp_upd;
#endif

	fsp_hdr = find_fsp_header();
	if (fsp_hdr == NULL) {
		/* No valid FSP info header was found */
		panic("Invalid FSP header");
	}

	config_data.common.fsp_hdr = fsp_hdr;
	config_data.common.stack_top = stack_top;
	config_data.common.boot_mode = boot_mode;

#ifdef CONFIG_FSP_USE_UPD
	/* Get VPD region start */
	fsp_vpd = (struct vpd_region *)(fsp_hdr->img_base +
			fsp_hdr->cfg_region_off);

	/* Verify the VPD data region is valid */
	assert(fsp_vpd->sign == VPD_IMAGE_ID);

	fsp_upd = &config_data.fsp_upd;

	/* Copy default data from Flash */
	memcpy(fsp_upd, (void *)(fsp_hdr->img_base + fsp_vpd->upd_offset),
	       sizeof(struct upd_region));

	/* Verify the UPD data region is valid */
	assert(fsp_upd->terminator == UPD_TERMINATOR);
#endif

	memset(&rt_buf, 0, sizeof(struct fspinit_rtbuf));

	/* Override any configuration if required */
	update_fsp_configs(&config_data, &rt_buf);

	memset(&params, 0, sizeof(struct fsp_init_params));
	params.nvs_buf = nvs_buf;
	params.rt_buf = (struct fspinit_rtbuf *)&rt_buf;
	params.continuation = (fsp_continuation_f)asm_continuation;

	init = (fsp_init_f)(fsp_hdr->img_base + fsp_hdr->fsp_init);
	params_ptr = &params;

	post_code(POST_PRE_MRC);

	/* Load GDT for FSP */
	setup_fsp_gdt();

	/*
	 * Use ASM code to ensure the register value in EAX & EDX
	 * will be passed into fsp_continue
	 */
	asm volatile (
		"pushl	%0;"
		"call	*%%eax;"
		".global asm_continuation;"
		"asm_continuation:;"
		"movl	4(%%esp), %%eax;"	/* status */
		"movl	8(%%esp), %%edx;"	/* hob_list */
		"jmp	fsp_continue;"
		: : "m"(params_ptr), "a"(init)
	);

	/*
	 * Should never get here.
	 * Control will continue from fsp_continue.
	 * This line below is to prevent the compiler from optimizing
	 * structure intialization.
	 *
	 * DO NOT REMOVE!
	 */
	init(&params);
}

u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase)
{
	fsp_notify_f notify;
	struct fsp_notify_params params;
	struct fsp_notify_params *params_ptr;
	u32 status;

	if (!fsp_hdr)
		fsp_hdr = (struct fsp_header *)find_fsp_header();

	if (fsp_hdr == NULL) {
		/* No valid FSP info header */
		panic("Invalid FSP header");
	}

	notify = (fsp_notify_f)(fsp_hdr->img_base + fsp_hdr->fsp_notify);
	params.phase = phase;
	params_ptr = &params;

	/*
	 * Use ASM code to ensure correct parameter is on the stack for
	 * FspNotify as U-Boot is using different ABI from FSP
	 */
	asm volatile (
		"pushl	%1;"		/* push notify phase */
		"call	*%%eax;"	/* call FspNotify */
		"addl	$4, %%esp;"	/* clean up the stack */
		: "=a"(status) : "m"(params_ptr), "a"(notify), "m"(*params_ptr)
	);

	return status;
}

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
				   struct efi_guid *guid)
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
				if (compare_guid(&res_desc->owner, guid)) {
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
	const struct efi_guid guid = FSP_HOB_RESOURCE_OWNER_FSP_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

u32 fsp_get_tseg_reserved_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_HOB_RESOURCE_OWNER_TSEG_GUID;
	u64 length;
	u32 base;

	base = (u32)fsp_get_reserved_mem_from_guid(hob_list,
			&length, (struct efi_guid *)&guid);
	if ((len != 0) && (base != 0))
		*len = (u32)length;

	return base;
}

const struct hob_header *fsp_get_next_hob(uint type, const void *hob_list)
{
	const struct hob_header *hdr;

	hdr = hob_list;

	/* Parse the HOB list until end of list or matching type is found */
	while (!end_of_hob(hdr)) {
		if (hdr->type == type)
			return hdr;

		hdr = get_next_hob(hdr);
	}

	return NULL;
}

const struct hob_header *fsp_get_next_guid_hob(const struct efi_guid *guid,
					       const void *hob_list)
{
	const struct hob_header *hdr;
	struct hob_guid *guid_hob;

	hdr = hob_list;
	while ((hdr = fsp_get_next_hob(HOB_TYPE_GUID_EXT,
			hdr)) != NULL) {
		guid_hob = (struct hob_guid *)hdr;
		if (compare_guid(guid, &(guid_hob->name)))
			break;
		hdr = get_next_hob(hdr);
	}

	return hdr;
}

void *fsp_get_guid_hob_data(const void *hob_list, u32 *len,
			    struct efi_guid *guid)
{
	const struct hob_header *guid_hob;

	guid_hob = fsp_get_next_guid_hob(guid, hob_list);
	if (guid_hob == NULL) {
		return NULL;
	} else {
		if (len)
			*len = get_guid_hob_data_size(guid_hob);

		return get_guid_hob_data(guid_hob);
	}
}

void *fsp_get_nvs_data(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;

	return fsp_get_guid_hob_data(hob_list, len, (struct efi_guid *)&guid);
}

void *fsp_get_bootloader_tmp_mem(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_BOOTLOADER_TEMP_MEM_HOB_GUID;

	return fsp_get_guid_hob_data(hob_list, len, (struct efi_guid *)&guid);
}

void *fsp_get_graphics_info(const void *hob_list, u32 *len)
{
	const struct efi_guid guid = FSP_GRAPHICS_INFO_HOB_GUID;

	return fsp_get_guid_hob_data(hob_list, len, (struct efi_guid *)&guid);
}
