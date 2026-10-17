// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Beniamino Galvani
 *
 * Author: Beniamino Galvani <b.galvani@gmail.com>
 * Author: Vyacheslav Bocharov <adeep@lexina.in>
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 * Author: Alexey Romanov <avromanov@sberdevices.ru>
 */

#include <command.h>
#include <common.h>
#include <env.h>
#include <asm/arch/sm.h>
#include <stdlib.h>
#include <display_options.h>

static int do_sm_serial(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulong address;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	address = simple_strtoul(argv[1], NULL, 0);

	ret = meson_sm_get_serial((void *)address, SM_SERIAL_SIZE);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

#define MAX_REBOOT_REASONS 14

static const char *reboot_reasons[MAX_REBOOT_REASONS] = {
	[REBOOT_REASON_COLD] = "cold_boot",
	[REBOOT_REASON_NORMAL] = "normal",
	[REBOOT_REASON_RECOVERY] = "recovery",
	[REBOOT_REASON_UPDATE] = "update",
	[REBOOT_REASON_FASTBOOT] = "fastboot",
	[REBOOT_REASON_SUSPEND_OFF] = "suspend_off",
	[REBOOT_REASON_HIBERNATE] = "hibernate",
	[REBOOT_REASON_BOOTLOADER] = "bootloader",
	[REBOOT_REASON_SHUTDOWN_REBOOT] = "shutdown_reboot",
	[REBOOT_REASON_RPMBP] = "rpmbp",
	[REBOOT_REASON_CRASH_DUMP] = "crash_dump",
	[REBOOT_REASON_KERNEL_PANIC] = "kernel_panic",
	[REBOOT_REASON_WATCHDOG_REBOOT] = "watchdog_reboot",
};

static int do_sm_reboot_reason(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	const char *reason_str;
	char *destarg = NULL;
	int reason;

	if (argc > 1)
		destarg = argv[1];

	reason = meson_sm_get_reboot_reason();
	if (reason < 0)
		return CMD_RET_FAILURE;

	if (reason >= MAX_REBOOT_REASONS ||
	    !reboot_reasons[reason])
		reason_str = "unknown";
	else
		reason_str = reboot_reasons[reason];

	if (destarg)
		env_set(destarg, reason_str);
	else
		printf("reboot reason: %s (%x)\n", reason_str, reason);

	return CMD_RET_SUCCESS;
}

static int do_efuse_read(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulong address, offset, size;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	offset = simple_strtoul(argv[1], NULL, 0);
	size = simple_strtoul(argv[2], NULL, 0);

	address = simple_strtoul(argv[3], NULL, 0);

	ret = meson_sm_read_efuse(offset, (void *)address, size);
	if (ret != size)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_efuse_write(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulong address, offset, size;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	offset = simple_strtoul(argv[1], NULL, 0);
	size = simple_strtoul(argv[2], NULL, 0);

	address = simple_strtoul(argv[3], NULL, 0);

	ret = meson_sm_write_efuse(offset, (void *)address, size);
	if (ret != size)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_efuse_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulong offset, size;
	u8 *buffer;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	offset = simple_strtoul(argv[1], NULL, 0);
	size = simple_strtoul(argv[2], NULL, 0);
	buffer = malloc(size);
	if (!buffer) {
		pr_err("Failed to allocate %lu bytes\n", size);
		return CMD_RET_FAILURE;
	}

	ret = meson_sm_read_efuse(offset, (void *)buffer, size);
	if (ret != size) {
		ret = CMD_RET_FAILURE;
		goto free_buffer;
	}

	print_buffer(0, buffer, 1, size, 0);

free_buffer:
	free(buffer);
	return ret;
}

static struct cmd_tbl cmd_sm_sub[] = {
	U_BOOT_CMD_MKENT(serial, 2, 1, do_sm_serial, "", ""),
	U_BOOT_CMD_MKENT(reboot_reason, 1, 1, do_sm_reboot_reason, "", ""),
	U_BOOT_CMD_MKENT(efuseread, 4, 1, do_efuse_read, "", ""),
	U_BOOT_CMD_MKENT(efusewrite, 4, 0, do_efuse_write, "", ""),
	U_BOOT_CMD_MKENT(efusedump, 3, 1, do_efuse_dump, "", ""),
};

static int do_sm(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'sm' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sm_sub[0], ARRAY_SIZE(cmd_sm_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sm, 5, 0, do_sm,
	"Secure Monitor Control",
	"serial <address> - read chip unique id to memory address\n"
	"sm reboot_reason [name] - get reboot reason and store to environment\n"
	"sm efuseread <offset> <size> <address> - read efuse to memory address\n"
	"sm efusewrite <offset> <size> <address> - write into efuse from memory address\n"
	"sm efusedump <offset> <size> - dump efuse data range to console"
);
