// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * (C) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * Helper functions for Rockchip images
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))

enum {
	RK_SIGNATURE		= 0x0ff0aa55,
};

/**
 * struct header0_info - header block for boot ROM
 *
 * This is stored at SD card block 64 (where each block is 512 bytes, or at
 * the start of SPI flash. It is encoded with RC4.
 *
 * @signature:		Signature (must be RKSD_SIGNATURE)
 * @disable_rc4:	0 to use rc4 for boot image,  1 to use plain binary
 * @init_offset:	Offset in blocks of the SPL code from this header
 *			block. E.g. 4 means 2KB after the start of this header.
 * Other fields are not used by U-Boot
 */
struct header0_info {
	uint32_t signature;
	uint8_t reserved[4];
	uint32_t disable_rc4;
	uint16_t init_offset;
	uint8_t reserved1[492];
	uint16_t init_size;
	uint16_t init_boot_size;
	uint8_t reserved2[2];
};

/**
 * struct header1_info
 */
struct header1_info {
	uint32_t magic;
};

/**
 * struct spl_info - spl info for each chip
 *
 * @imagename:		Image name(passed by "mkimage -n")
 * @spl_hdr:		Boot ROM requires a 4-bytes spl header
 * @spl_size:		Spl size(include extra 4-bytes spl header)
 * @spl_rc4:		RC4 encode the SPL binary (same key as header)
 */

struct spl_info {
	const char *imagename;
	const char *spl_hdr;
	const uint32_t spl_size;
	const bool spl_rc4;
};

static struct spl_info spl_infos[] = {
	{ "rk3036", "RK30", 0x1000, false },
	{ "rk3128", "RK31", 0x1800, false },
	{ "rk3188", "RK31", 0x8000 - 0x800, true },
	{ "rk322x", "RK32", 0x8000 - 0x1000, false },
	{ "rk3288", "RK32", 0x8000, false },
	{ "rk3328", "RK32", 0x8000 - 0x1000, false },
	{ "rk3368", "RK33", 0x8000 - 0x1000, false },
	{ "rk3399", "RK33", 0x30000 - 0x2000, false },
	{ "rv1108", "RK11", 0x1800, false },
};

static unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7,
	45, 44, 123, 56, 23, 13, 23, 17
};

static struct spl_info *rkcommon_get_spl_info(char *imagename)
{
	int i;

	if (!imagename)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		if (!strncmp(imagename, spl_infos[i].imagename, 6))
			return spl_infos + i;

	return NULL;
}

int rkcommon_check_params(struct image_tool_params *params)
{
	int i;

	if (rkcommon_get_spl_info(params->imagename) != NULL)
		return EXIT_SUCCESS;

	/*
	 * If this is a operation (list or extract), the don't require
	 * imagename to be set.
	 */
	if (params->lflag || params->iflag)
		return EXIT_SUCCESS;

	fprintf(stderr, "ERROR: imagename (%s) is not supported!\n",
		params->imagename ? params->imagename : "NULL");

	fprintf(stderr, "Available imagename:");
	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		fprintf(stderr, "\t%s", spl_infos[i].imagename);
	fprintf(stderr, "\n");

	return EXIT_FAILURE;
}

const char *rkcommon_get_spl_hdr(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_hdr;
}


int rkcommon_get_spl_size(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_size;
}

bool rkcommon_need_rc4_spl(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_rc4;
}

static void rkcommon_set_header0(void *buf, uint file_size,
				 struct image_tool_params *params)
{
	struct header0_info *hdr = buf;

	memset(buf, '\0', RK_INIT_OFFSET * RK_BLK_SIZE);
	hdr->signature = RK_SIGNATURE;
	hdr->disable_rc4 = !rkcommon_need_rc4_spl(params);
	hdr->init_offset = RK_INIT_OFFSET;

	hdr->init_size = DIV_ROUND_UP(file_size, RK_BLK_SIZE);
	/*
	 * The init_size has to be a multiple of 4 blocks (i.e. of 2K)
	 * or the BootROM will not boot the image.
	 *
	 * Note: To verify that this is not a legacy constraint, we
	 *       rechecked this against the RK3399 BootROM.
	 */
	hdr->init_size = ROUND(hdr->init_size, 4);
	/*
	 * init_boot_size needs to be set, as it is read by the BootROM
	 * to determine the size of the next-stage bootloader (e.g. U-Boot
	 * proper), when used with the back-to-bootrom functionality.
	 *
	 * see https://lists.denx.de/pipermail/u-boot/2017-May/293267.html
	 * for a more detailed explanation by Andy Yan
	 */
	hdr->init_boot_size = hdr->init_size + RK_MAX_BOOT_SIZE / RK_BLK_SIZE;

	rc4_encode(buf, RK_BLK_SIZE, rc4_key);
}

int rkcommon_set_header(void *buf, uint file_size,
			struct image_tool_params *params)
{
	struct header1_info *hdr = buf + RK_SPL_HDR_START;

	if (file_size > rkcommon_get_spl_size(params))
		return -ENOSPC;

	rkcommon_set_header0(buf, file_size, params);

	/* Set up the SPL name (i.e. copy spl_hdr over) */
	memcpy(&hdr->magic, rkcommon_get_spl_hdr(params), RK_SPL_HDR_SIZE);

	if (rkcommon_need_rc4_spl(params))
		rkcommon_rc4_encode_spl(buf, RK_SPL_HDR_START,
					params->file_size - RK_SPL_HDR_START);

	return 0;
}

static inline unsigned rkcommon_offset_to_spi(unsigned offset)
{
	/*
	 * While SD/MMC images use a flat addressing, SPI images are padded
	 * to use the first 2K of every 4K sector only.
	 */
	return ((offset & ~0x7ff) << 1) + (offset & 0x7ff);
}

static int rkcommon_parse_header(const void *buf, struct header0_info *header0,
				 struct spl_info **spl_info)
{
	unsigned hdr1_offset;
	struct header1_info *hdr1_sdmmc, *hdr1_spi;
	int i;

	if (spl_info)
		*spl_info = NULL;

	/*
	 * The first header (hdr0) is always RC4 encoded, so try to decrypt
	 * with the well-known key.
	 */
	memcpy((void *)header0, buf, sizeof(struct header0_info));
	rc4_encode((void *)header0, sizeof(struct header0_info), rc4_key);

	if (header0->signature != RK_SIGNATURE)
		return -EPROTO;

	/* We don't support RC4 encoded image payloads here, yet... */
	if (header0->disable_rc4 == 0)
		return -ENOSYS;

	hdr1_offset = header0->init_offset * RK_BLK_SIZE;
	hdr1_sdmmc = (struct header1_info *)(buf + hdr1_offset);
	hdr1_spi = (struct header1_info *)(buf +
					   rkcommon_offset_to_spi(hdr1_offset));

	for (i = 0; i < ARRAY_SIZE(spl_infos); i++) {
		if (!memcmp(&hdr1_sdmmc->magic, spl_infos[i].spl_hdr, 4)) {
			if (spl_info)
				*spl_info = &spl_infos[i];
			return IH_TYPE_RKSD;
		} else if (!memcmp(&hdr1_spi->magic, spl_infos[i].spl_hdr, 4)) {
			if (spl_info)
				*spl_info = &spl_infos[i];
			return IH_TYPE_RKSPI;
		}
	}

	return -1;
}

int rkcommon_verify_header(unsigned char *buf, int size,
			   struct image_tool_params *params)
{
	struct header0_info header0;
	struct spl_info *img_spl_info, *spl_info;
	int ret;

	ret = rkcommon_parse_header(buf, &header0, &img_spl_info);

	/* If this is the (unimplemented) RC4 case, then rewrite the result */
	if (ret == -ENOSYS)
		return 0;

	if (ret < 0)
		return ret;

	/*
	 * If no 'imagename' is specified via the commandline (e.g. if this is
	 * 'dumpimage -l' w/o any further constraints), we accept any spl_info.
	 */
	if (params->imagename == NULL)
		return 0;

	/* Match the 'imagename' against the 'spl_hdr' found */
	spl_info = rkcommon_get_spl_info(params->imagename);
	if (spl_info && img_spl_info)
		return strcmp(spl_info->spl_hdr, img_spl_info->spl_hdr);

	return -ENOENT;
}

void rkcommon_print_header(const void *buf)
{
	struct header0_info header0;
	struct spl_info *spl_info;
	uint8_t image_type;
	int ret;

	ret = rkcommon_parse_header(buf, &header0, &spl_info);

	/* If this is the (unimplemented) RC4 case, then fail silently */
	if (ret == -ENOSYS)
		return;

	if (ret < 0) {
		fprintf(stderr, "Error: image verification failed\n");
		return;
	}

	image_type = ret;

	printf("Image Type:   Rockchip %s (%s) boot image\n",
	       spl_info->spl_hdr,
	       (image_type == IH_TYPE_RKSD) ? "SD/MMC" : "SPI");
	printf("Data Size:    %d bytes\n", header0.init_size * RK_BLK_SIZE);
}

void rkcommon_rc4_encode_spl(void *buf, unsigned int offset, unsigned int size)
{
	unsigned int remaining = size;

	while (remaining > 0) {
		int step = (remaining > RK_BLK_SIZE) ? RK_BLK_SIZE : remaining;

		rc4_encode(buf + offset, step, rc4_key);
		offset += RK_BLK_SIZE;
		remaining -= step;
	}
}

int rkcommon_vrec_header(struct image_tool_params *params,
			 struct image_type_params *tparams,
			 unsigned int alignment)
{
	unsigned int  unpadded_size;
	unsigned int  padded_size;

	/*
	 * The SPL image looks as follows:
	 *
	 * 0x0    header0 (see rkcommon.c)
	 * 0x800  spl_name ('RK30', ..., 'RK33')
	 *        (start of the payload for AArch64 payloads: we expect the
	 *        first 4 bytes to be available for overwriting with our
	 *        spl_name)
	 * 0x804  first instruction to be executed
	 *        (start of the image/payload for 32bit payloads)
	 *
	 * For AArch64 (ARMv8) payloads, natural alignment (8-bytes) is
	 * required for its sections (so the image we receive needs to
	 * have the first 4 bytes reserved for the spl_name).  Reserving
	 * these 4 bytes is done using the BOOT0_HOOK infrastructure.
	 *
	 * The header is always at 0x800 (as we now use a payload
	 * prepadded using the boot0 hook for all targets): the first
	 * 4 bytes of these images can safely be overwritten using the
	 * boot magic.
	 */
	tparams->header_size = RK_SPL_HDR_START;

	/* Allocate, clear and install the header */
	tparams->hdr = malloc(tparams->header_size);
	if (!tparams->hdr)
		return -ENOMEM;
	memset(tparams->hdr, 0, tparams->header_size);

	/*
	 * If someone passed in 0 for the alignment, we'd better handle
	 * it correctly...
	 */
	if (!alignment)
		alignment = 1;

	unpadded_size = tparams->header_size + params->file_size;
	padded_size = ROUND(unpadded_size, alignment);

	return padded_size - unpadded_size;
}
