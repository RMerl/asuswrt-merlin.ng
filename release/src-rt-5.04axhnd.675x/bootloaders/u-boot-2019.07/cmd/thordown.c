// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_thordown.c -- USB TIZEN "THOR" Downloader gadget
 *
 * Copyright (C) 2013 Lukasz Majewski <l.majewski@samsung.com>
 * All rights reserved.
 */

#include <common.h>
#include <thor.h>
#include <dfu.h>
#include <g_dnl.h>
#include <usb.h>

int do_thor_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 4)
		return CMD_RET_USAGE;

	char *usb_controller = argv[1];
	char *interface = argv[2];
	char *devstring = argv[3];

	int ret;

	puts("TIZEN \"THOR\" Downloader\n");

	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	ret = usb_gadget_initialize(controller_index);
	if (ret) {
		pr_err("USB init failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	g_dnl_register("usb_dnl_thor");

	ret = thor_init();
	if (ret) {
		pr_err("THOR DOWNLOAD failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	ret = thor_handle();
	if (ret) {
		pr_err("THOR failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

exit:
	g_dnl_unregister();
	usb_gadget_release(controller_index);
done:
	dfu_free_entities();

	return ret;
}

U_BOOT_CMD(thordown, CONFIG_SYS_MAXARGS, 1, do_thor_down,
	   "TIZEN \"THOR\" downloader",
	   "<USB_controller> <interface> <dev>\n"
	   "  - device software upgrade via LTHOR TIZEN download\n"
	   "    program via <USB_controller> on device <dev>,\n"
	   "	attached to interface <interface>\n"
);
