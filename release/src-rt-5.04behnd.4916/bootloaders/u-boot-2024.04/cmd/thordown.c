// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_thordown.c -- USB TIZEN "THOR" Downloader gadget
 *
 * Copyright (C) 2013 Lukasz Majewski <l.majewski@samsung.com>
 * All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <thor.h>
#include <dfu.h>
#include <g_dnl.h>
#include <usb.h>
#include <linux/printk.h>

int do_thor_down(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *interface, *devstring;
	int controller_index;
	struct udevice *udc;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	puts("TIZEN \"THOR\" Downloader\n");

	interface = argv[2];
	devstring = argv[3];

	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	controller_index = simple_strtoul(argv[1], NULL, 0);
	ret = udc_device_get_by_index(controller_index, &udc);
	if (ret) {
		pr_err("USB init failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	ret = g_dnl_register("usb_dnl_thor");
	if (ret) {
		pr_err("g_dnl_register failed %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	ret = thor_init(udc);
	if (ret) {
		pr_err("THOR DOWNLOAD failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	do {
		ret = thor_handle(udc);
		if (ret == THOR_DFU_REINIT_NEEDED) {
			dfu_free_entities();
			ret = dfu_init_env_entities(interface, devstring);
		}
		if (ret) {
			pr_err("THOR failed: %d\n", ret);
			ret = CMD_RET_FAILURE;
			goto exit;
		}
	} while (ret == 0);
exit:
	g_dnl_unregister();
	udc_device_put(udc);
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
