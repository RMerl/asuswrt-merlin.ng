/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP1_API_H__
#define __FSP1_API_H__

#include <linux/linkage.h>
#include <asm/fsp/fsp_api.h>
/*
 * FSP common configuration structure.
 * This needs to be included in the platform-specific struct fsp_config_data.
 */
struct fsp_cfg_common {
	struct fsp_header	*fsp_hdr;
	u32			stack_top;
	u32			boot_mode;
};

/*
 * FspInit continuation function prototype.
 * Control will be returned to this callback function after FspInit API call.
 */
typedef void (*fsp_continuation_f)(u32 status, void *hob_list);

struct fsp_init_params {
	/* Non-volatile storage buffer pointer */
	void			*nvs_buf;
	/* Runtime buffer pointer */
	void			*rt_buf;
	/* Continuation function address */
	fsp_continuation_f	continuation;
};

struct common_buf {
	/*
	 * Stack top pointer used by the bootloader. The new stack frame will be
	 * set up at this location after FspInit API call.
	 */
	u32	stack_top;
	u32	boot_mode;	/* Current system boot mode */
	void	*upd_data;	/* User platform configuraiton data region */
	u32	tolum_size;	/* Top of low usable memory size (FSP 1.1) */
	u32	reserved[6];	/* Reserved */
};

/* FspInit API function prototype */
typedef asmlinkage u32 (*fsp_init_f)(struct fsp_init_params *params);

#endif
