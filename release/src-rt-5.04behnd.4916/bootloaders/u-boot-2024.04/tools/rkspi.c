// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * See README.rockchip for details of the rkspi format
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

enum {
	RKSPI_SECT_LEN		= RK_BLK_SIZE * 4,
};

static void rkspi_set_header(void *buf, struct stat *sbuf, int ifd,
			     struct image_tool_params *params)
{
	int sector;
	unsigned int size;

	size = params->orig_file_size;

	rkcommon_set_header(buf, sbuf, ifd, params);

	/*
	 * Spread the image out so we only use the first 2KB of each 4KB
	 * region. This is a feature of the SPI format required by the Rockchip
	 * boot ROM. Its rationale is unknown.
	 */
	if (params->vflag)
		fprintf(stderr, "Spreading spi image from %u to %u\n",
			size, params->file_size);

	for (sector = size / RKSPI_SECT_LEN - 1; sector >= 0; sector--) {
		debug("sector %u\n", sector);
		memmove(buf + sector * RKSPI_SECT_LEN * 2,
			buf + sector * RKSPI_SECT_LEN,
			RKSPI_SECT_LEN);
		memset(buf + sector * RKSPI_SECT_LEN * 2 + RKSPI_SECT_LEN,
		       '\0', RKSPI_SECT_LEN);
	}
}

static int rkspi_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKSPI)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/*
 * The SPI payload needs to make space for odd half-sector layout used in flash
 * (i.e. only the first 2K of each 4K sector is used).
 */
static int rkspi_vrec_header(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	rkcommon_vrec_header(params, tparams);

	/*
	 * Converting to the SPI format (i.e. splitting each 4K page into two
	 * 2K subpages and then padding these 2K pages up to take a complete
	 * 4K sector again) which will double the image size.
	 */
	params->file_size = ROUND(params->file_size, RKSPI_SECT_LEN) << 1;

	/* Ignoring pad len, since we are using our own copy_image() */
	return 0;
}

/*
 * rk_spi parameters
 */
U_BOOT_IMAGE_TYPE(
	rkspi,
	"Rockchip SPI Boot Image support",
	0,
	NULL,
	rkcommon_check_params,
	rkcommon_verify_header,
	rkcommon_print_header,
	rkspi_set_header,
	NULL,
	rkspi_check_image_type,
	NULL,
	rkspi_vrec_header
);
