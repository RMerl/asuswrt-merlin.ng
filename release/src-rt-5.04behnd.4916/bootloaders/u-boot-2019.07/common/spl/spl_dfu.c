// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#include <common.h>
#include <spl.h>
#include <linux/compiler.h>
#include <errno.h>
#include <watchdog.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <dfu.h>
#include <environment.h>

static int run_dfu(int usb_index, char *interface, char *devstring)
{
	int ret;

	ret = dfu_init_env_entities(interface, devstring);
	if (ret) {
		dfu_free_entities();
		goto exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	return ret;
}

int spl_dfu_cmd(int usbctrl, char *dfu_alt_info, char *interface, char *devstr)
{
	char *str_env;
	int ret;

	/* set default environment */
	set_default_env(NULL, 0);
	str_env = env_get(dfu_alt_info);
	if (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		return -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	if (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		return -EINVAL;
	}

	/* invoke dfu command */
	return run_dfu(usbctrl, interface, devstr);
}
