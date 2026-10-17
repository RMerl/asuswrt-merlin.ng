// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <asm/fsp1/fsp_support.h>
#include <asm/post.h>

struct fsp_header *__attribute__((optimize("O0"))) fsp_find_header(void)
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
	    ((struct ffs_file_header *)fsp)->name.b[0] == FSP_GUID_BYTE0 &&
	    ((struct ffs_file_header *)fsp)->name.b[1] == FSP_GUID_BYTE1 &&
	    ((struct ffs_file_header *)fsp)->name.b[2] == FSP_GUID_BYTE2 &&
	    ((struct ffs_file_header *)fsp)->name.b[3] == FSP_GUID_BYTE3 &&
	    ((struct ffs_file_header *)fsp)->name.b[4] == FSP_GUID_BYTE4 &&
	    ((struct ffs_file_header *)fsp)->name.b[5] == FSP_GUID_BYTE5 &&
	    ((struct ffs_file_header *)fsp)->name.b[6] == FSP_GUID_BYTE6 &&
	    ((struct ffs_file_header *)fsp)->name.b[7] == FSP_GUID_BYTE7 &&
	    ((struct ffs_file_header *)fsp)->name.b[8] == FSP_GUID_BYTE8 &&
	    ((struct ffs_file_header *)fsp)->name.b[9] == FSP_GUID_BYTE9 &&
	    ((struct ffs_file_header *)fsp)->name.b[10] == FSP_GUID_BYTE10 &&
	    ((struct ffs_file_header *)fsp)->name.b[11] == FSP_GUID_BYTE11 &&
	    ((struct ffs_file_header *)fsp)->name.b[12] == FSP_GUID_BYTE12 &&
	    ((struct ffs_file_header *)fsp)->name.b[13] == FSP_GUID_BYTE13 &&
	    ((struct ffs_file_header *)fsp)->name.b[14] == FSP_GUID_BYTE14 &&
	    ((struct ffs_file_header *)fsp)->name.b[15] == FSP_GUID_BYTE15) {
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

	fsp_hdr = fsp_find_header();
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
	fsp_update_configs(&config_data, &rt_buf);

	memset(&params, 0, sizeof(struct fsp_init_params));
	params.nvs_buf = nvs_buf;
	params.rt_buf = (struct fspinit_rtbuf *)&rt_buf;
	params.continuation = (fsp_continuation_f)fsp_asm_continuation;

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
		".global fsp_asm_continuation;"
		"fsp_asm_continuation:;"
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
		fsp_hdr = (struct fsp_header *)fsp_find_header();

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
