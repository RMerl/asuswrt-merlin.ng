// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Linaro LTD, www.linaro.org
 * Author: John Rigby <john.rigby@linaro.org>
 * Based on TI's signGP.c
 *
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include "imagetool.h"
#include <compiler.h>
#include <image.h>
#include "gpheader.h"
#include "omapimage.h"

#define DIV_ROUND_UP(n, d)     (((n) + (d) - 1) / (d))

/* Header size is CH header rounded up to 512 bytes plus GP header */
#define OMAP_CH_HDR_SIZE 512
#define OMAP_FILE_HDR_SIZE (OMAP_CH_HDR_SIZE + GPIMAGE_HDR_SIZE)

static int do_swap32 = 0;

static uint8_t omapimage_header[OMAP_FILE_HDR_SIZE];

static int omapimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_OMAPIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int omapimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct ch_toc *toc = (struct ch_toc *)ptr;
	struct gp_header *gph = (struct gp_header *)(ptr+OMAP_CH_HDR_SIZE);
	uint32_t offset, size;

	while (toc->section_offset != 0xffffffff
			&& toc->section_size != 0xffffffff) {
		if (do_swap32) {
			offset = cpu_to_be32(toc->section_offset);
			size = cpu_to_be32(toc->section_size);
		} else {
			offset = toc->section_offset;
			size = toc->section_size;
		}
		if (!offset || !size)
			return -1;
		if (offset >= OMAP_CH_HDR_SIZE ||
		    offset+size >= OMAP_CH_HDR_SIZE)
			return -1;
		toc++;
	}

	return gph_verify_header(gph, do_swap32);
}

static void omapimage_print_section(struct ch_settings *chs)
{
	const char *section_name;

	if (chs->section_key)
		section_name = "CHSETTINGS";
	else
		section_name = "UNKNOWNKEY";

	printf("%s (%x) "
		"valid:%x "
		"version:%x "
		"reserved:%x "
		"flags:%x\n",
		section_name,
		chs->section_key,
		chs->valid,
		chs->version,
		chs->reserved,
		chs->flags);
}

static void omapimage_print_header(const void *ptr)
{
	const struct ch_toc *toc = (struct ch_toc *)ptr;
	const struct gp_header *gph =
			(struct gp_header *)(ptr+OMAP_CH_HDR_SIZE);
	uint32_t offset, size;

	while (toc->section_offset != 0xffffffff
			&& toc->section_size != 0xffffffff) {
		if (do_swap32) {
			offset = cpu_to_be32(toc->section_offset);
			size = cpu_to_be32(toc->section_size);
		} else {
			offset = toc->section_offset;
			size = toc->section_size;
		}

		if (offset >= OMAP_CH_HDR_SIZE ||
		    offset+size >= OMAP_CH_HDR_SIZE)
			exit(EXIT_FAILURE);

		printf("Section %s offset %x length %x\n",
			toc->section_name,
			toc->section_offset,
			toc->section_size);

		omapimage_print_section((struct ch_settings *)(ptr+offset));
		toc++;
	}

	gph_print_header(gph, do_swap32);
}

static int toc_offset(void *hdr, void *member)
{
	return member - hdr;
}

static void omapimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct ch_toc *toc = (struct ch_toc *)ptr;
	struct ch_settings *chs = (struct ch_settings *)
					(ptr + 2 * sizeof(*toc));
	struct gp_header *gph = (struct gp_header *)(ptr + OMAP_CH_HDR_SIZE);

	toc->section_offset = toc_offset(ptr, chs);
	toc->section_size = sizeof(struct ch_settings);
	strcpy((char *)toc->section_name, "CHSETTINGS");

	chs->section_key = KEY_CHSETTINGS;
	chs->valid = 0;
	chs->version = 1;
	chs->reserved = 0;
	chs->flags = 0;

	toc++;
	memset(toc, 0xff, sizeof(*toc));

	gph_set_header(gph, sbuf->st_size - OMAP_CH_HDR_SIZE,
		       params->addr, 0);

	if (strncmp(params->imagename, "byteswap", 8) == 0) {
		do_swap32 = 1;
		int swapped = 0;
		uint32_t *data = (uint32_t *)ptr;
		const off_t size_in_words =
			DIV_ROUND_UP(sbuf->st_size, sizeof(uint32_t));

		while (swapped < size_in_words) {
			*data = cpu_to_be32(*data);
			swapped++;
			data++;
		}
	}
}

/*
 * omapimage parameters
 */
U_BOOT_IMAGE_TYPE(
	omapimage,
	"TI OMAP CH/GP Boot Image support",
	OMAP_FILE_HDR_SIZE,
	(void *)&omapimage_header,
	gpimage_check_params,
	omapimage_verify_header,
	omapimage_print_header,
	omapimage_set_header,
	NULL,
	omapimage_check_image_types,
	NULL,
	NULL
);
