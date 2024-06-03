// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#include <common.h>
#include <spl.h>
#include <usb.h>
#include <g_dnl.h>
#include <sdp.h>

static int spl_sdp_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int ret;
	const int controller_index = 0;

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	if (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		return ret;
	}

	ret = sdp_init(controller_index);
	if (ret) {
		pr_err("SDP init failed: %d\n", ret);
		return -ENODEV;
	}

	/*
	 * This command either loads a legacy image, jumps and never returns,
	 * or it loads a FIT image and returns it to be handled by the SPL
	 * code.
	 */
	ret = spl_sdp_handle(controller_index, spl_image);
	debug("SDP ended\n");

	return ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);
