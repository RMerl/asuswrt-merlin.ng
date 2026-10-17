/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _TPL_PARAMS_H
#define _TPL_PARAMS_H

#include "boot_blob.h"

#define EF_JTAG_LOAD 0x1

typedef struct tpl_params_s {
	void *environment;
	uint32_t early_flags;
	uint32_t ddr_size;  /* in MB */
	uint8_t boot_device; 
} tpl_params;

extern uint32_t boot_params;
#if defined(CONFIG_TPL_BUILD)
extern tpl_params* tplparams;
#endif

#define TPL_PARAMS_ADDR ((CONFIG_TPL_TEXT_BASE - BOOT_BLOB_MAX_ENV_SIZE - sizeof(tpl_params) \
	- 0x100)&~0xff)

#define IS_JTAG_LOADED(params) ((params) & EF_JTAG_LOAD)

#endif
