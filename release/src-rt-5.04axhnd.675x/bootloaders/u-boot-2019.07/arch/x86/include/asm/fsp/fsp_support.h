/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_SUPPORT_H__
#define __FSP_SUPPORT_H__

#include "fsp_types.h"
#include "fsp_fv.h"
#include "fsp_ffs.h"
#include "fsp_api.h"
#include "fsp_hob.h"
#include "fsp_infoheader.h"
#include "fsp_bootmode.h"
#include "fsp_azalia.h"
#include <asm/arch/fsp/fsp_vpd.h>
#include <asm/arch/fsp/fsp_configs.h>

#define FSP_LOWMEM_BASE		0x100000UL
#define FSP_HIGHMEM_BASE	0x100000000ULL
#define UPD_TERMINATOR		0x55AA


/**
 * FSP Continuation assembly helper routine
 *
 * This routine jumps to the C version of FSP continuation function
 */
void asm_continuation(void);

/**
 * FSP initialization complete
 *
 * This is the function that indicates FSP initialization is complete and jumps
 * back to the bootloader with HOB list pointer as the parameter.
 *
 * @hob_list:    HOB list pointer
 */
void fsp_init_done(void *hob_list);

/**
 * FSP Continuation function
 *
 * @status:      Always 0
 * @hob_list:    HOB list pointer
 *
 * @retval:      Never returns
 */
void fsp_continue(u32 status, void *hob_list);

/**
 * Find FSP header offset in FSP image
 *
 * @retval: the offset of FSP header. If signature is invalid, returns 0.
 */
struct fsp_header *find_fsp_header(void);

/**
 * FSP initialization wrapper function.
 *
 * @stack_top: bootloader stack top address
 * @boot_mode: boot mode defined in fsp_bootmode.h
 * @nvs_buf:   Non-volatile memory buffer pointer
 */
void fsp_init(u32 stack_top, u32 boot_mode, void *nvs_buf);

/**
 * FSP notification wrapper function
 *
 * @fsp_hdr: Pointer to FSP information header
 * @phase:   FSP initialization phase defined in enum fsp_phase
 *
 * @retval:  compatible status code with EFI_STATUS defined in PI spec
 */
u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase);

/**
 * This function retrieves the top of usable low memory.
 *
 * @hob_list: A HOB list pointer.
 *
 * @retval:   Usable low memory top.
 */
u32 fsp_get_usable_lowmem_top(const void *hob_list);

/**
 * This function retrieves the top of usable high memory.
 *
 * @hob_list: A HOB list pointer.
 *
 * @retval:   Usable high memory top.
 */
u64 fsp_get_usable_highmem_top(const void *hob_list);

/**
 * This function retrieves a special reserved memory region.
 *
 * @hob_list: A HOB list pointer.
 * @len:      A pointer to the GUID HOB data buffer length.
 *            If the GUID HOB is located, the length will be updated.
 * @guid:     A pointer to the owner guild.
 *
 * @retval:   Reserved region start address.
 *            0 if this region does not exist.
 */
u64 fsp_get_reserved_mem_from_guid(const void *hob_list,
				   u64 *len, struct efi_guid *guid);

/**
 * This function retrieves the FSP reserved normal memory.
 *
 * @hob_list: A HOB list pointer.
 * @len:      A pointer to the FSP reserved memory length buffer.
 *            If the GUID HOB is located, the length will be updated.
 * @retval:   FSP reserved memory base
 *            0 if this region does not exist.
 */
u32 fsp_get_fsp_reserved_mem(const void *hob_list, u32 *len);

/**
 * This function retrieves the TSEG reserved normal memory.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the TSEG reserved memory length buffer.
 *                 If the GUID HOB is located, the length will be updated.
 *
 * @retval NULL:   Failed to find the TSEG reserved memory.
 * @retval others: TSEG reserved memory base.
 */
u32 fsp_get_tseg_reserved_mem(const void *hob_list, u32 *len);

/**
 * Returns the next instance of a HOB type from the starting HOB.
 *
 * @type:     HOB type to search
 * @hob_list: A pointer to the HOB list
 *
 * @retval:   A HOB object with matching type; Otherwise NULL.
 */
const struct hob_header *fsp_get_next_hob(uint type, const void *hob_list);

/**
 * Returns the next instance of the matched GUID HOB from the starting HOB.
 *
 * @guid:     GUID to search
 * @hob_list: A pointer to the HOB list
 *
 * @retval:   A HOB object with matching GUID; Otherwise NULL.
 */
const struct hob_header *fsp_get_next_guid_hob(const struct efi_guid *guid,
					       const void *hob_list);

/**
 * This function retrieves a GUID HOB data buffer and size.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the GUID HOB data buffer length.
 *                 If the GUID HOB is located, the length will be updated.
 * @guid           A pointer to HOB GUID.
 *
 * @retval NULL:   Failed to find the GUID HOB.
 * @retval others: GUID HOB data buffer pointer.
 */
void *fsp_get_guid_hob_data(const void *hob_list, u32 *len,
			    struct efi_guid *guid);

/**
 * This function retrieves FSP Non-volatile Storage HOB buffer and size.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the NVS data buffer length.
 *                 If the HOB is located, the length will be updated.
 *
 * @retval NULL:   Failed to find the NVS HOB.
 * @retval others: FSP NVS data buffer pointer.
 */
void *fsp_get_nvs_data(const void *hob_list, u32 *len);

/**
 * This function retrieves Bootloader temporary stack buffer and size.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the bootloader temporary stack length.
 *                 If the HOB is located, the length will be updated.
 *
 * @retval NULL:   Failed to find the bootloader temporary stack HOB.
 * @retval others: Bootloader temporary stackbuffer pointer.
 */
void *fsp_get_bootloader_tmp_mem(const void *hob_list, u32 *len);

/**
 * This function retrieves graphics information.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the graphics info HOB length.
 *                 If the HOB is located, the length will be updated.
 *
 * @retval NULL:   Failed to find the graphics info HOB.
 * @retval others: A pointer to struct hob_graphics_info.
 */
void *fsp_get_graphics_info(const void *hob_list, u32 *len);

/**
 * This function overrides the default configurations of FSP.
 *
 * @config:  A pointer to the FSP configuration data structure
 * @rt_buf:  A pointer to the FSP runtime buffer data structure
 *
 * @return:  None
 */
void update_fsp_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf);

/**
 * fsp_init_phase_pci() - Tell the FSP that we have completed PCI init
 *
 * @return 0 if OK, -EPERM if the FSP gave an error.
 */
int fsp_init_phase_pci(void);

#endif
