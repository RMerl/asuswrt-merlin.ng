// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_sdp.c -- sdp command
 *
 * Copyright (C) 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#include <common.h>
#include <g_dnl.h>
#include <sdp.h>
#include <usb.h>

static int do_sdp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	char *usb_controller = argv[1];
	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	usb_gadget_initialize(controller_index);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	if (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		goto exit_register;
	}

	ret = sdp_init(controller_index);
	if (ret) {
		pr_err("SDP init failed: %d\n", ret);
		goto exit;
	}

	/* This command typically does not return but jumps to an image */
	sdp_handle(controller_index);
	pr_err("SDP ended\n");

exit:
	g_dnl_unregister();
exit_register:
	usb_gadget_release(controller_index);

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(sdp, 2, 1, do_sdp,
	"Serial Downloader Protocol",
	"<USB_controller>\n"
	"  - serial downloader protocol via <USB_controller>\n"
);
