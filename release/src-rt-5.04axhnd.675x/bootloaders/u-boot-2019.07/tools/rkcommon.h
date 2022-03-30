/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _RKCOMMON_H
#define _RKCOMMON_H

enum {
	RK_BLK_SIZE		= 512,
	RK_INIT_SIZE_ALIGN      = 2048,
	RK_INIT_OFFSET		= 4,
	RK_MAX_BOOT_SIZE	= 512 << 10,
	RK_SPL_HDR_START	= RK_INIT_OFFSET * RK_BLK_SIZE,
	RK_SPL_HDR_SIZE		= 4,
	RK_SPL_START		= RK_SPL_HDR_START + RK_SPL_HDR_SIZE,
	RK_IMAGE_HEADER_LEN	= RK_SPL_START,
};

/**
 * rkcommon_check_params() - check params
 *
 * @return 0 if OK, -1 if ERROR.
 */
int rkcommon_check_params(struct image_tool_params *params);

/**
 * rkcommon_get_spl_hdr() - get 4-bytes spl hdr for a Rockchip boot image
 *
 * Rockchip's bootrom requires the spl loader to start with a 4-bytes
 * header. The content of this header depends on the chip type.
 */
const char *rkcommon_get_spl_hdr(struct image_tool_params *params);

/**
 * rkcommon_get_spl_size() - get spl size for a Rockchip boot image
 *
 * Different chip may have different sram size. And if we want to jump
 * back to the bootrom after spl, we may need to reserve some sram space
 * for the bootrom.
 * The spl loader size should be sram size minus reserved size(if needed)
 */
int rkcommon_get_spl_size(struct image_tool_params *params);

/**
 * rkcommon_set_header() - set up the header for a Rockchip boot image
 *
 * This sets up a 2KB header which can be interpreted by the Rockchip boot ROM.
 *
 * @buf:	Pointer to header place (must be at least 2KB in size)
 * @file_size:	Size of the file we want the boot ROM to load, in bytes
 * @return 0 if OK, -ENOSPC if too large
 */
int rkcommon_set_header(void *buf, uint file_size,
			struct image_tool_params *params);

/**
 * rkcommon_verify_header() - verify the header for a Rockchip boot image
 *
 * @buf:	Pointer to the image file
 * @file_size:	Size of entire bootable image file (incl. all padding)
 * @return 0 if OK
 */
int rkcommon_verify_header(unsigned char *buf, int size,
			   struct image_tool_params *params);

/**
 * rkcommon_print_header() - print the header for a Rockchip boot image
 *
 * This prints the header, spl_name and whether this is a SD/MMC or SPI image.
 *
 * @buf:	Pointer to the image (can be a read-only file-mapping)
 */
void rkcommon_print_header(const void *buf);

/**
 * rkcommon_need_rc4_spl() - check if rc4 encoded spl is required
 *
 * Some socs cannot disable the rc4-encryption of the spl binary.
 * rc4 encryption is disabled normally except on socs that cannot
 * handle unencrypted binaries.
 * @return true or false depending on rc4 being required.
 */
bool rkcommon_need_rc4_spl(struct image_tool_params *params);

/**
 * rkcommon_rc4_encode_spl() - encode the spl binary
 *
 * Encrypts the SPL binary using the generic rc4 key as required
 * by some socs.
 *
 * @buf:	Pointer to the SPL data (header and SPL binary)
 * @offset:	offset inside buf to start at
 * @size:	number of bytes to encode
 */
void rkcommon_rc4_encode_spl(void *buf, unsigned int offset, unsigned int size);

/**
 * rkcommon_vrec_header() - allocate memory for the header
 *
 * @params:     Pointer to the tool params structure
 * @tparams:    Pointer tot the image type structure (for setting
 *              the header and header_size)
 * @alignment:  Alignment (a power of two) that the image should be
 *              padded to (e.g. 512 if we want to align with SD/MMC
 *              blocksizes or 2048 for the SPI format)
 *
 * @return bytes of padding required/added (does not include the header_size)
 */
int rkcommon_vrec_header(struct image_tool_params *params,
			 struct image_type_params *tparams,
			 unsigned int alignment);

#endif
