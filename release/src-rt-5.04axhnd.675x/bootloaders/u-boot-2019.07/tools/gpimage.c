// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated
 * Add gpimage format for keystone devices to format spl image. This is
 * Based on omapimage.c
 *
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

static uint8_t gpimage_header[GPIMAGE_HDR_SIZE];

/* to be in keystone gpimage */
static int gpimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_GPIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int gpimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct gp_header *gph = (struct gp_header *)ptr;

	return gph_verify_header(gph, 1);
}

static void gpimage_print_header(const void *ptr)
{
	const struct gp_header *gph = (struct gp_header *)ptr;

	gph_print_header(gph, 1);
}

static void gpimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct gp_header *gph = (struct gp_header *)ptr;

	gph_set_header(gph, sbuf->st_size - GPIMAGE_HDR_SIZE, params->addr, 1);
}

/*
 * gpimage parameters
 */
U_BOOT_IMAGE_TYPE(
	gpimage,
	"TI KeyStone GP Image support",
	GPIMAGE_HDR_SIZE,
	(void *)&gpimage_header,
	gpimage_check_params,
	gpimage_verify_header,
	gpimage_print_header,
	gpimage_set_header,
	NULL,
	gpimage_check_image_types,
	NULL,
	NULL
);
