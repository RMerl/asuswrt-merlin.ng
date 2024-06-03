// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <qfw.h>

/*
 * This function prepares kernel for zboot. It loads kernel data
 * to 'load_addr', initrd to 'initrd_addr' and kernel command
 * line using qemu fw_cfg interface.
 */
static int qemu_fwcfg_setup_kernel(void *load_addr, void *initrd_addr)
{
	char *data_addr;
	uint32_t setup_size, kernel_size, cmdline_size, initrd_size;

	qemu_fwcfg_read_entry(FW_CFG_SETUP_SIZE, 4, &setup_size);
	qemu_fwcfg_read_entry(FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	if (setup_size == 0 || kernel_size == 0) {
		printf("warning: no kernel available\n");
		return -1;
	}

	data_addr = load_addr;
	qemu_fwcfg_read_entry(FW_CFG_SETUP_DATA,
			      le32_to_cpu(setup_size), data_addr);
	data_addr += le32_to_cpu(setup_size);

	qemu_fwcfg_read_entry(FW_CFG_KERNEL_DATA,
			      le32_to_cpu(kernel_size), data_addr);
	data_addr += le32_to_cpu(kernel_size);

	data_addr = initrd_addr;
	qemu_fwcfg_read_entry(FW_CFG_INITRD_SIZE, 4, &initrd_size);
	if (initrd_size == 0) {
		printf("warning: no initrd available\n");
	} else {
		qemu_fwcfg_read_entry(FW_CFG_INITRD_DATA,
				      le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
	}

	qemu_fwcfg_read_entry(FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	if (cmdline_size) {
		qemu_fwcfg_read_entry(FW_CFG_CMDLINE_DATA,
				      le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		if (*data_addr != '\0') {
			if (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %p size %x", load_addr,
	       le32_to_cpu(kernel_size));
	if (initrd_size)
		printf(" initrd %p size %x\n",
		       initrd_addr,
		       le32_to_cpu(initrd_size));
	else
		printf("\n");

	return 0;
}

static int qemu_fwcfg_list_firmware(void)
{
	int ret;
	struct fw_cfg_file_iter iter;
	struct fw_file *file;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret)
		return ret;


	for (file = qemu_fwcfg_file_iter_init(&iter);
	     !qemu_fwcfg_file_iter_end(&iter);
	     file = qemu_fwcfg_file_iter_next(&iter)) {
		printf("%-56s\n", file->cfg.name);
	}

	return 0;
}

static int qemu_fwcfg_do_list(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	if (qemu_fwcfg_list_firmware() < 0)
		return CMD_RET_FAILURE;

	return 0;
}

static int qemu_fwcfg_do_cpus(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int ret = qemu_fwcfg_online_cpus();
	if (ret < 0) {
		printf("QEMU fw_cfg interface not found\n");
		return CMD_RET_FAILURE;
	}

	printf("%d cpu(s) online\n", qemu_fwcfg_online_cpus());

	return 0;
}

static int qemu_fwcfg_do_load(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	char *env;
	void *load_addr;
	void *initrd_addr;

	env = env_get("loadaddr");
	load_addr = env ?
		(void *)simple_strtoul(env, NULL, 16) :
#ifdef CONFIG_LOADADDR
		(void *)CONFIG_LOADADDR;
#else
		NULL;
#endif

	env = env_get("ramdiskaddr");
	initrd_addr = env ?
		(void *)simple_strtoul(env, NULL, 16) :
#ifdef CONFIG_RAMDISK_ADDR
		(void *)CONFIG_RAMDISK_ADDR;
#else
		NULL;
#endif

	if (argc == 2) {
		load_addr = (void *)simple_strtoul(argv[0], NULL, 16);
		initrd_addr = (void *)simple_strtoul(argv[1], NULL, 16);
	} else if (argc == 1) {
		load_addr = (void *)simple_strtoul(argv[0], NULL, 16);
	}

	if (!load_addr || !initrd_addr) {
		printf("missing load or initrd address\n");
		return CMD_RET_FAILURE;
	}

	return qemu_fwcfg_setup_kernel(load_addr, initrd_addr);
}

static cmd_tbl_t fwcfg_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, qemu_fwcfg_do_list, "", ""),
	U_BOOT_CMD_MKENT(cpus, 0, 1, qemu_fwcfg_do_cpus, "", ""),
	U_BOOT_CMD_MKENT(load, 2, 1, qemu_fwcfg_do_load, "", ""),
};

static int do_qemu_fw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	cmd_tbl_t *fwcfg_cmd;

	if (!qemu_fwcfg_present()) {
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
)
