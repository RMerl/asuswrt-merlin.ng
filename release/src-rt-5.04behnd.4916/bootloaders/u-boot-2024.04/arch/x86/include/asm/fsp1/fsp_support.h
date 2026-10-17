/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP1_SUPPORT_H__
#define __FSP1_SUPPORT_H__

#include <asm/fsp/fsp_support.h>
#include "fsp_ffs.h"

/**
 * fsp_asm_continuation() - FSP Continuation assembly helper routine
 *
 * This routine jumps to the C version of FSP continuation function
 */
void fsp_asm_continuation(void);

/**
 * fsp_init_done() - FSP initialization complete
 *
 * This is the function that indicates FSP initialization is complete and jumps
 * back to the bootloader with HOB list pointer as the parameter.
 *
 * @hob_list:    HOB list pointer
 */
void fsp_init_done(void *hob_list);

/**
 * fsp_continue() - FSP Continuation function
 *
 * @status:      Always 0
 * @hob_list:    HOB list pointer
 *
 * Return: Never returns
 */
void fsp_continue(u32 status, void *hob_list);

/**
 * fsp_init() - FSP initialization wrapper function
 *
 * @stack_top: bootloader stack top address
 * @boot_mode: boot mode defined in fsp_bootmode.h
 * @nvs_buf:   Non-volatile memory buffer pointer
 */
void fsp_init(u32 stack_top, u32 boot_mode, void *nvs_buf);

/**
 * fsp_get_bootloader_tmp_mem() - retrieves temporary stack buffer and size
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the bootloader temporary stack length.
 *                 If the HOB is located, the length will be updated.
 *
 * Return: NULL:   Failed to find the bootloader temporary stack HOB.
 * Return: others: Bootloader temporary stackbuffer pointer.
 */
void *fsp_get_bootloader_tmp_mem(const void *hob_list, u32 *len);

/**
 * fsp_update_configs() - overrides the default configurations of FSP
 *
 * @config:  A pointer to the FSP configuration data structure
 * @rt_buf:  A pointer to the FSP runtime buffer data structure
 *
 * Return: None
 */
void fsp_update_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf);

#endif
