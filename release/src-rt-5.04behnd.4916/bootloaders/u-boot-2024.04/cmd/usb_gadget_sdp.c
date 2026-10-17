// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_sdp.c -- sdp command
 *
 * Copyright (C) 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#include <common.h>
#include <command.h>
#include <g_dnl.h>
#include <sdp.h>
#include <usb.h>
#include <linux/printk.h>

static int do_sdp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int controller_index;
	struct udevice *udc;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	controller_index = simple_strtoul(argv[1], NULL, 0);
	ret = udc_device_get_by_index(controller_index, &udc);
	if (ret)
		return ret;

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	if (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		goto exit_register;
	}

	ret = sdp_init(udc);
	if (ret) {
		pr_err("SDP init failed: %d\n", ret);
		goto exit;
	}

	/* This command typically does not return but jumps to an image */
	sdp_handle(udc);
	pr_err("SDP ended\n");

exit:
	g_dnl_unregister();
exit_register:
	udc_device_put(udc);

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(sdp, 2, 1, do_sdp,
	"Serial Downloader Protocol",
	"<USB_controller>\n"
	"  - serial downloader protocol via <USB_controller>\n"
);
