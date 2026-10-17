// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <g_dnl.h>
#include <fastboot.h>
#include <net.h>
#include <usb.h>
#include <watchdog.h>
#include <linux/printk.h>
#include <linux/stringify.h>

static int do_fastboot_udp(int argc, char *const argv[],
			   uintptr_t buf_addr, size_t buf_size)
{
	int err;

	if (!IS_ENABLED(CONFIG_UDP_FUNCTION_FASTBOOT)) {
		pr_err("Fastboot UDP not enabled\n");
		return CMD_RET_FAILURE;
	}

	err = net_loop(FASTBOOT_UDP);

	if (err < 0) {
		printf("fastboot udp error: %d\n", err);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_fastboot_tcp(int argc, char *const argv[],
			   uintptr_t buf_addr, size_t buf_size)
{
	int err;

	if (!IS_ENABLED(CONFIG_TCP_FUNCTION_FASTBOOT)) {
		pr_err("Fastboot TCP not enabled\n");
		return CMD_RET_FAILURE;
	}

	err = net_loop(FASTBOOT_TCP);

	if (err < 0) {
		printf("fastboot tcp error: %d\n", err);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_fastboot_usb(int argc, char *const argv[],
			   uintptr_t buf_addr, size_t buf_size)
{
	int controller_index;
	char *usb_controller;
	struct udevice *udc;
	char *endp;
	int ret;

	if (!IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT)) {
		pr_err("Fastboot USB not enabled\n");
		return CMD_RET_FAILURE;
	}

	if (argc < 2)
		return CMD_RET_USAGE;

	usb_controller = argv[1];
	controller_index = simple_strtoul(usb_controller, &endp, 0);
	if (*endp != '\0') {
		pr_err("Error: Wrong USB controller index format\n");
		return CMD_RET_FAILURE;
	}

	ret = udc_device_get_by_index(controller_index, &udc);
	if (ret) {
		pr_err("USB init failed: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_fastboot");
	if (ret)
		return ret;

	if (!g_dnl_board_usb_cable_connected()) {
		puts("\rUSB cable not detected.\n" \
		     "Command exit.\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	while (1) {
		if (g_dnl_detach())
			break;
		if (ctrlc())
			break;
		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}

	ret = CMD_RET_SUCCESS;

exit:
	udc_device_put(udc);
	g_dnl_unregister();
	g_dnl_clear_detach();

	return ret;
}

static int do_fastboot(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	uintptr_t buf_addr = (uintptr_t)NULL;
	size_t buf_size = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'l':
				if (--argc <= 0)
					return CMD_RET_USAGE;
				buf_addr = hextoul(*++argv, NULL);
				goto NXTARG;

			case 's':
				if (--argc <= 0)
					return CMD_RET_USAGE;
				buf_size = hextoul(*++argv, NULL);
				goto NXTARG;

			default:
				return CMD_RET_USAGE;
			}
		}
NXTARG:
		;
	}

	/* Handle case when USB controller param is just '-' */
	if (argc == 1) {
		pr_err("Error: Incorrect USB controller index\n");
		return CMD_RET_USAGE;
	}

	fastboot_init((void *)buf_addr, buf_size);

	if (!strcmp(argv[1], "udp"))
		return do_fastboot_udp(argc, argv, buf_addr, buf_size);
	if (!strcmp(argv[1], "tcp"))
		return do_fastboot_tcp(argc, argv, buf_addr, buf_size);
	if (!strcmp(argv[1], "usb")) {
		argv++;
		argc--;
	}

	return do_fastboot_usb(argc, argv, buf_addr, buf_size);
}

U_BOOT_CMD(
	fastboot, CONFIG_SYS_MAXARGS, 1, do_fastboot,
	"run as a fastboot usb or udp device",
	"[-l addr] [-s size] usb <controller> | udp\n"
	"\taddr - address of buffer used during data transfers ("
	__stringify(CONFIG_FASTBOOT_BUF_ADDR) ")\n"
	"\tsize - size of buffer used during data transfers ("
	__stringify(CONFIG_FASTBOOT_BUF_SIZE) ")"
);
