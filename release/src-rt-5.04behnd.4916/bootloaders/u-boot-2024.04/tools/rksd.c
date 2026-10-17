// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * See README.rockchip for details of the rksd format
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

static int rksd_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKSD)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/*
 * rk_sd parameters
 */
U_BOOT_IMAGE_TYPE(
	rksd,
	"Rockchip SD Boot Image support",
	0,
	NULL,
	rkcommon_check_params,
	rkcommon_verify_header,
	rkcommon_print_header,
	rkcommon_set_header,
	NULL,
	rksd_check_image_type,
	NULL,
	rkcommon_vrec_header
);
