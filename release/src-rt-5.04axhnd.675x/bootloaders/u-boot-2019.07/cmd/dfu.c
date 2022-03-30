// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <watchdog.h>
#include <dfu.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <net.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	if (argc < 4)
		return CMD_RET_USAGE;

#ifdef CONFIG_DFU_OVER_USB
	char *usb_controller = argv[1];
#endif
#if defined(CONFIG_DFU_OVER_USB) || defined(CONFIG_DFU_OVER_TFTP)
	char *interface = argv[2];
	char *devstring = argv[3];
#endif

	int ret = 0;
#ifdef CONFIG_DFU_OVER_TFTP
	unsigned long addr = 0;
	if (!strcmp(argv[1], "tftp")) {
		if (argc == 5)
			addr = simple_strtoul(argv[4], NULL, 0);

		return update_tftp(addr, interface, devstring);
	}
#endif
#ifdef CONFIG_DFU_OVER_USB
	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	ret = CMD_RET_SUCCESS;
	if (argc > 4 && strcmp(argv[4], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

	int controller_index = simple_strtoul(usb_controller, NULL, 0);

	run_usb_dnl_gadget(controller_index, "usb_dnl_dfu");

done:
	dfu_free_entities();
#endif
	return ret;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	""
#ifdef CONFIG_DFU_OVER_USB
	"<USB_controller> <interface> <dev> [list]\n"
	"  - device firmware upgrade via <USB_controller>\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [list] - list available alt settings\n"
#endif
#ifdef CONFIG_DFU_OVER_TFTP
#ifdef CONFIG_DFU_OVER_USB
	"dfu "
#endif
	"tftp <interface> <dev> [<addr>]\n"
	"  - device firmware upgrade via TFTP\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [<addr>] - address where FIT image has been stored\n"
#endif
);
