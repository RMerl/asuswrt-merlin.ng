// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */

#include <common.h>
#include <command.h>
#include <cmd_spl.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

static const char **subcmd_list[] = {

	[SPL_EXPORT_FDT] = (const char * []) {
#ifdef CONFIG_OF_LIBFDT
		"start",
		"loados",
	#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
		"ramdisk",
	#endif
		"fdt",
		"cmdline",
		"bdt",
		"prep",
#endif
		NULL,
	},
	[SPL_EXPORT_ATAGS] = (const char * []) {
#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
		"start",
		"loados",
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
		"ramdisk",
#endif
		"cmdline",
		"bdt",
		"prep",
#endif
		NULL,
	},
	NULL
};

/* Calls bootm with the parameters given */
static int call_bootm(int argc, char * const argv[], const char *subcommand[])
{
	char *bootm_argv[5];

	int i = 0;
	int ret = 0;
	int j;

	/* create paramter array */
	bootm_argv[0] = "do_bootm";
	switch (argc) {
	case 3:
		bootm_argv[4] = argv[2]; /* fdt addr */
	case 2:
		bootm_argv[3] = argv[1]; /* initrd addr */
	case 1:
		bootm_argv[2] = argv[0]; /* kernel addr */
	}


	/*
	 * - do the work -
	 * exec subcommands of do_bootm to init the images
	 * data structure
	 */
	while (subcommand[i] != NULL) {
		bootm_argv[1] = (char *)subcommand[i];
		debug("args %d: %s %s ", argc, bootm_argv[0], bootm_argv[1]);
		for (j = 0; j < argc; j++)
			debug("%s ", bootm_argv[j + 2]);
		debug("\n");

		ret = do_bootm(find_cmd("do_bootm"), 0, argc+2,
			bootm_argv);
		debug("Subcommand retcode: %d\n", ret);
		i++;
	}

	if (ret) {
		printf("ERROR prep subcommand failed!\n");
		return -1;
	}

	return 0;
}

static cmd_tbl_t cmd_spl_export_sub[] = {
	U_BOOT_CMD_MKENT(fdt, 0, 1, (void *)SPL_EXPORT_FDT, "", ""),
	U_BOOT_CMD_MKENT(atags, 0, 1, (void *)SPL_EXPORT_ATAGS, "", ""),
};

static int spl_export(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const cmd_tbl_t *c;

	if (argc < 2) /* no subcommand */
		return cmd_usage(cmdtp);

	c = find_cmd_tbl(argv[1], &cmd_spl_export_sub[0],
		ARRAY_SIZE(cmd_spl_export_sub));
	if ((c) && ((long)c->cmd <= SPL_EXPORT_LAST)) {
		argc -= 2;
		argv += 2;
		if (call_bootm(argc, argv, subcmd_list[(long)c->cmd]))
			return -1;
		switch ((long)c->cmd) {
#ifdef CONFIG_OF_LIBFDT
		case SPL_EXPORT_FDT:
			printf("Argument image is now in RAM: 0x%p\n",
				(void *)images.ft_addr);
			env_set_addr("fdtargsaddr", images.ft_addr);
			env_set_hex("fdtargslen", fdt_totalsize(images.ft_addr));
#ifdef CONFIG_CMD_SPL_WRITE_SIZE
			if (fdt_totalsize(images.ft_addr) >
			    CONFIG_CMD_SPL_WRITE_SIZE)
				puts("WARN: FDT size > CMD_SPL_WRITE_SIZE\n");
#endif
			break;
#endif
		case SPL_EXPORT_ATAGS:
			printf("Argument image is now in RAM at: 0x%p\n",
				(void *)gd->bd->bi_boot_params);
			break;
		}
	} else {
		/* Unrecognized command */
		return cmd_usage(cmdtp);
	}

	return 0;
}

static cmd_tbl_t cmd_spl_sub[] = {
	U_BOOT_CMD_MKENT(export, 0, 1, (void *)SPL_EXPORT, "", ""),
};

static int do_spl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const cmd_tbl_t *c;
	int cmd;

	if (argc < 2) /* no subcommand */
		return cmd_usage(cmdtp);

	c = find_cmd_tbl(argv[1], &cmd_spl_sub[0], ARRAY_SIZE(cmd_spl_sub));
	if (c) {
		cmd = (long)c->cmd;
		switch (cmd) {
		case SPL_EXPORT:
			argc--;
			argv++;
			if (spl_export(cmdtp, flag, argc, argv))
				printf("Subcommand failed\n");
			break;
		default:
			/* unrecognized command */
			return cmd_usage(cmdtp);
		}
	} else {
		/* Unrecognized command */
		return cmd_usage(cmdtp);
	}
	return 0;
}

U_BOOT_CMD(
	spl, 6 , 1, do_spl, "SPL configuration",
	"export <img=atags|fdt> [kernel_addr] [initrd_addr] [fdt_addr]\n"
	"\timg\t\t\"atags\" or \"fdt\"\n"
	"\tkernel_addr\taddress where a kernel image is stored.\n"
	"\t\t\tkernel is loaded as part of the boot process, but it is not started.\n"
	"\tinitrd_addr\taddress of initial ramdisk\n"
	"\t\t\tcan be set to \"-\" if fdt_addr without initrd_addr is used.\n"
	"\tfdt_addr\tin case of fdt, the address of the device tree.\n"
	);
