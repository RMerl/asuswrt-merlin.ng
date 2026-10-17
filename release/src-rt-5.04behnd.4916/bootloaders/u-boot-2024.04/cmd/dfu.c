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
#include <command.h>
#include <watchdog.h>
#include <dfu.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <net.h>

static int do_dfu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{

	if (argc < 2)
		return CMD_RET_USAGE;

#ifdef CONFIG_DFU_OVER_USB
	char *usb_controller = argv[1];
#endif
	char *interface = NULL;
	char *devstring = NULL;
#if defined(CONFIG_DFU_TIMEOUT) || defined(CONFIG_DFU_OVER_TFTP)
	unsigned long value = 0;
#endif
	if (argc >= 4) {
		interface = argv[2];
		devstring = argv[3];
	}

#if defined(CONFIG_DFU_TIMEOUT) || defined(CONFIG_DFU_OVER_TFTP)
	if (argc == 5 || argc == 3)
		value = simple_strtoul(argv[argc - 1], NULL, 0);
#endif

	int ret = 0;
#ifdef CONFIG_DFU_OVER_TFTP
	if (!strcmp(argv[1], "tftp"))
		return update_tftp(value, interface, devstring);
#endif
	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

#ifdef CONFIG_DFU_TIMEOUT
	dfu_set_timeout(value * 1000);
#endif

	ret = CMD_RET_SUCCESS;
	if (strcmp(argv[argc - 1], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

#ifdef CONFIG_DFU_OVER_USB
	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	bool retry = false;
	do {
		ret = run_usb_dnl_gadget(controller_index, "usb_dnl_dfu");

		if (dfu_reinit_needed) {
			dfu_free_entities();
			ret = dfu_init_env_entities(interface, devstring);
			if (ret)
				goto done;
			retry = true;
		}
	} while (retry);

#endif
done:
	dfu_free_entities();
	return ret;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	""
#ifdef CONFIG_DFU_OVER_USB
#ifdef CONFIG_DFU_TIMEOUT
	"<USB_controller> [<interface> <dev>] [<timeout>] [list]\n"
#else
	"<USB_controller> [<interface> <dev>] [list]\n"
#endif
	"  - device firmware upgrade via <USB_controller>\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
#ifdef CONFIG_DFU_TIMEOUT
	"    [<timeout>] - specify inactivity timeout in seconds\n"
#endif
#endif
	"    [list] - list available alt settings\n"
#ifdef CONFIG_DFU_OVER_TFTP
#ifdef CONFIG_DFU_OVER_USB
	"dfu "
#endif
	"tftp [<interface> <dev>] [<addr>]\n"
	"  - device firmware upgrade via TFTP\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [<addr>] - address where FIT image has been stored\n"
#endif
);
