// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Eddie Cai <eddie.cai.linux@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <asm/arch-rockchip/f_rockusb.h>

static int do_rockusb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int controller_index, dev_index;
	char *usb_controller;
	char *devtype;
	char *devnum;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	usb_controller = argv[1];
	controller_index = simple_strtoul(usb_controller, NULL, 0);

	if (argc >= 4) {
		devtype = argv[2];
		devnum  = argv[3];
	} else {
		return CMD_RET_USAGE;
	}
	dev_index = simple_strtoul(devnum, NULL, 0);
	rockusb_dev_init(devtype, dev_index);

	ret = usb_gadget_initialize(controller_index);
	if (ret) {
		printf("USB init failed: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_rockusb");
	if (ret)
		return CMD_RET_FAILURE;

	if (!g_dnl_board_usb_cable_connected()) {
		puts("\rUSB cable not detected, Command exit.\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	while (1) {
		if (g_dnl_detach())
			break;
		if (ctrlc())
			break;
		usb_gadget_handle_interrupts(controller_index);
	}
	ret = CMD_RET_SUCCESS;

exit:
	g_dnl_unregister();
	g_dnl_clear_detach();
	usb_gadget_release(controller_index);

	return ret;
}

U_BOOT_CMD(rockusb, 4, 1, do_rockusb,
	   "use the rockusb protocol",
	   "<USB_controller> <devtype> <dev[:part]>  e.g. rockusb 0 mmc 0\n"
);
