/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_SUPPORT_H__
#define __FSP_SUPPORT_H__

#include <signatures.h>

#include <asm/fsp/fsp_bootmode.h>
#include <asm/fsp/fsp_fv.h>
#include <asm/fsp/fsp_hob.h>
#include <asm/fsp/fsp_infoheader.h>
#include <asm/fsp_arch.h>
#include <asm/fsp/fsp_azalia.h>

#define FSP_LOWMEM_BASE		0x100000UL
#define FSP_HIGHMEM_BASE	0x100000000ULL
#define UPD_TERMINATOR		0x55AA

/**
 * fsp_find_header() - Find FSP header offset in FSP image
 *
 * Return: the offset of FSP header. If signature is invalid, returns 0.
 */
struct fsp_header *fsp_find_header(void);

/**
 * fsp_notify() - FSP notification wrapper function
 *
 * @fsp_hdr: Pointer to FSP information header
 * @phase:   FSP initialization phase defined in enum fsp_phase
 *
 * Return: compatible status code with EFI_STATUS defined in PI spec
 */
u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase);

/**
 * fsp_get_usable_lowmem_top() - retrieves the top of usable low memory
 *
 * @hob_list: A HOB list pointer.
 *
 * Return: Usable low memory top.
 */
u32 fsp_get_usable_lowmem_top(const void *hob_list);

/**
 * fsp_get_usable_highmem_top() - retrieves the top of usable high memory
 *
 * @hob_list: A HOB list pointer.
 *
 * Return: Usable high memory top.
 */
u64 fsp_get_usable_highmem_top(const void *hob_list);

/**
 * fsp_get_reserved_mem_from_guid() - retrieves a special reserved memory region
 *
 * @hob_list: A HOB list pointer.
 * @len:      A pointer to the GUID HOB data buffer length.
 *            If the GUID HOB is located, the length will be updated.
 * @guid:     A pointer to the owner guild.
 *
 * Return: Reserved region start address.
 *            0 if this region does not exist.
 */
u64 fsp_get_reserved_mem_from_guid(const void *hob_list,
				   u64 *len, const efi_guid_t *guid);

/**
 * fsp_get_fsp_reserved_mem() - retrieves the FSP reserved normal memory
 *
 * @hob_list: A HOB list pointer.
 * @len:      A pointer to the FSP reserved memory length buffer.
 *            If the GUID HOB is located, the length will be updated.
 * Return: FSP reserved memory base
 *            0 if this region does not exist.
 */
u32 fsp_get_fsp_reserved_mem(const void *hob_list, u32 *len);

/**
 * fsp_get_tseg_reserved_mem() - retrieves the TSEG reserved normal memory
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the TSEG reserved memory length buffer.
 *                 If the GUID HOB is located, the length will be updated.
 *
 * Return: NULL:   Failed to find the TSEG reserved memory.
 * Return: others: TSEG reserved memory base.
 */
u32 fsp_get_tseg_reserved_mem(const void *hob_list, u32 *len);

/**
 * fsp_get_nvs_data() - retrieves FSP Non-volatile Storage HOB buffer and size
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the NVS data buffer length.
 *                 If the HOB is located, the length will be updated.
 *
 * Return: NULL:   Failed to find the NVS HOB.
 * Return: others: FSP NVS data buffer pointer.
 */
void *fsp_get_nvs_data(const void *hob_list, u32 *len);

/**
 * fsp_get_var_nvs_data() - get FSP variable Non-volatile Storage HOB buffer
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the NVS data buffer length.
 *                 If the HOB is located, the length will be updated.
 *
 * Return: NULL:   Failed to find the NVS HOB.
 * Return: others: FSP NVS data buffer pointer.
 */
void *fsp_get_var_nvs_data(const void *hob_list, u32 *len);

/**
 * fsp_get_graphics_info() - retrieves graphics information.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the graphics info HOB length.
 *                 If the HOB is located, the length will be updated.
 *
 * Return: NULL:   Failed to find the graphics info HOB.
 * Return: others: A pointer to struct hob_graphics_info.
 */
void *fsp_get_graphics_info(const void *hob_list, u32 *len);

/**
 * fsp_init_phase_pci() - Tell the FSP that we have completed PCI init
 *
 * Return: 0 if OK, -EPERM if the FSP gave an error.
 */
int fsp_init_phase_pci(void);

/**
 * fsp_scan_for_ram_size() - Scan the HOB list to find the RAM size
 *
 * This sets gd->ram_size based on what it finds.
 *
 * Return: 0 if OK, -ve on error
 */
int fsp_scan_for_ram_size(void);

/**
 * fsp_notify() - FSP notification wrapper function
 *
 * @fsp_hdr: Pointer to FSP information header
 * @phase:   FSP initialization phase defined in enum fsp_phase
 *
 * Return: compatible status code with EFI_STATUS defined in PI spec
 */
u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase);

#endif
