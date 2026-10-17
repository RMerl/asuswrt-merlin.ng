// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <qfw.h>
#include <dm.h>

static struct udevice *qfw_dev;

static int qemu_fwcfg_cmd_list_firmware(void)
{
	int ret;
	struct fw_cfg_file_iter iter;
	struct fw_file *file;

	/* make sure fw_list is loaded */
	ret = qfw_read_firmware_list(qfw_dev);
	if (ret)
		return ret;

	for (file = qfw_file_iter_init(qfw_dev, &iter);
	     !qfw_file_iter_end(&iter);
	     file = qfw_file_iter_next(&iter)) {
		printf("%08lx %-56s\n", file->addr, file->cfg.name);
	}

	return 0;
}

static int qemu_fwcfg_do_list(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	if (qemu_fwcfg_cmd_list_firmware() < 0)
		return CMD_RET_FAILURE;

	return 0;
}

static int qemu_fwcfg_do_cpus(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	printf("%d cpu(s) online\n", qfw_online_cpus(qfw_dev));
	return 0;
}

static int qemu_fwcfg_do_load(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	char *env;
	ulong load_addr;
	ulong initrd_addr;

	env = env_get("loadaddr");
	load_addr = env ?
		hextoul(env, NULL) :
		CONFIG_SYS_LOAD_ADDR;

	env = env_get("ramdiskaddr");
	initrd_addr = env ?
		hextoul(env, NULL) :
#ifdef CFG_RAMDISK_ADDR
		CFG_RAMDISK_ADDR;
#else
		0;
#endif

	if (argc == 2) {
		load_addr = hextoul(argv[0], NULL);
		initrd_addr = hextoul(argv[1], NULL);
	} else if (argc == 1) {
		load_addr = hextoul(argv[0], NULL);
	}

	if (!load_addr || !initrd_addr) {
		printf("missing load or initrd address\n");
		return CMD_RET_FAILURE;
	}

	return qemu_fwcfg_setup_kernel(qfw_dev, load_addr, initrd_addr);
}

static struct cmd_tbl fwcfg_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, qemu_fwcfg_do_list, "", ""),
	U_BOOT_CMD_MKENT(cpus, 0, 1, qemu_fwcfg_do_cpus, "", ""),
	U_BOOT_CMD_MKENT(load, 2, 1, qemu_fwcfg_do_load, "", ""),
};

static int do_qemu_fw(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int ret;
	struct cmd_tbl *fwcfg_cmd;

	ret = qfw_get_dev(&qfw_dev);
	if (ret) {
		printf("QEMU fw_cfg interface not found\n");
		return CMD_RET_USAGE;
	}

	fwcfg_cmd = find_cmd_tbl(argv[1], fwcfg_commands,
				 ARRAY_SIZE(fwcfg_commands));
	argc -= 2;
	argv += 2;
	if (!fwcfg_cmd || argc > fwcfg_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = fwcfg_cmd->cmd(fwcfg_cmd, flag, argc, argv);

	return cmd_process_error(fwcfg_cmd, ret);
}

U_BOOT_CMD(
	qfw,	4,	1,	do_qemu_fw,
	"QEMU firmware interface",
	"<command>\n"
	"    - list                             : print firmware(s) currently loaded\n"
	"    - cpus                             : print online cpu number\n"
	"    - load <kernel addr> <initrd addr> : load kernel and initrd (if any), and setup for zboot\n"
);
