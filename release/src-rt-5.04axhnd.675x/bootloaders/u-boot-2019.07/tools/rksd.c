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

static void rksd_set_header(void *buf,  struct stat *sbuf,  int ifd,
			    struct image_tool_params *params)
{
	unsigned int size;
	int ret;

	/*
	 * We need to calculate this using 'RK_SPL_HDR_START' and not using
	 * 'tparams->header_size', as the additional byte inserted when
	 * 'is_boot0' is true counts towards the payload (and not towards the
	 * header).
	 */
	size = params->file_size - RK_SPL_HDR_START;
	ret = rkcommon_set_header(buf, size, params);
	if (ret) {
		/* TODO(sjg@chromium.org): This method should return an error */
		printf("Warning: SPL image is too large (size %#x) and will "
		       "not boot\n", size);
	}
}

static int rksd_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKSD)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int rksd_vrec_header(struct image_tool_params *params,
			    struct image_type_params *tparams)
{
	/*
	 * Pad to a 2KB alignment, as required for init_size by the ROM
	 * (see https://lists.denx.de/pipermail/u-boot/2017-May/293268.html)
	 */
	return rkcommon_vrec_header(params, tparams, RK_INIT_SIZE_ALIGN);
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
	rksd_set_header,
	NULL,
	rksd_check_image_type,
	NULL,
	rksd_vrec_header
);
