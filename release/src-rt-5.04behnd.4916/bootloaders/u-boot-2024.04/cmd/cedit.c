// SPDX-License-Identifier: GPL-2.0+
/*
 * 'cedit' command
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <abuf.h>
#include <cedit.h>
#include <command.h>
#include <dm.h>
#include <expo.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm/ofnode.h>
#include <linux/sizes.h>

struct expo *cur_exp;

static int check_cur_expo(void)
{
	if (!cur_exp) {
		printf("No expo loaded\n");
		return -ENOENT;
	}

	return 0;
}

static int do_cedit_load(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	const char *fname;
	struct expo *exp;
	oftree tree;
	ulong size;
	void *buf;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;
	fname = argv[3];

	ret = fs_load_alloc(argv[1], argv[2], argv[3], SZ_1M, 0, &buf, &size);
	if (ret) {
		printf("File not found\n");
		return CMD_RET_FAILURE;
	}

	tree = oftree_from_fdt(buf);
	if (!oftree_valid(tree)) {
		printf("Cannot create oftree\n");
		return CMD_RET_FAILURE;
	}

	ret = expo_build(oftree_root(tree), &exp);
	oftree_dispose(tree);
	if (ret) {
		printf("Failed to build expo: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	cur_exp = exp;

	return 0;
}

static int do_cedit_write_fdt(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	const char *fname;
	struct abuf buf;
	loff_t bytes;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;
	fname = argv[3];

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	ret = cedit_write_settings(cur_exp, &buf);
	if (ret) {
		printf("Failed to write settings: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	if (fs_set_blk_dev(argv[1], argv[2], FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	ret = fs_write(fname, map_to_sysmem(abuf_data(&buf)), 0,
		       abuf_size(&buf), &bytes);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int do_cedit_read_fdt(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	const char *fname;
	void *buf;
	oftree tree;
	ulong size;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;
	fname = argv[3];

	ret = fs_load_alloc(argv[1], argv[2], argv[3], SZ_1M, 0, &buf, &size);
	if (ret) {
		printf("File not found\n");
		return CMD_RET_FAILURE;
	}

	tree = oftree_from_fdt(buf);
	if (!oftree_valid(tree)) {
		free(buf);
		printf("Cannot create oftree\n");
		return CMD_RET_FAILURE;
	}

	ret = cedit_read_settings(cur_exp, tree);
	oftree_dispose(tree);
	free(buf);
	if (ret) {
		printf("Failed to read settings: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cedit_write_env(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	bool verbose;
	int ret;

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	verbose = argc > 1 && !strcmp(argv[1], "-v");

	ret = cedit_write_settings_env(cur_exp, verbose);
	if (ret) {
		printf("Failed to write settings to environment: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cedit_read_env(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	bool verbose;
	int ret;

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	verbose = argc > 1 && !strcmp(argv[1], "-v");

	ret = cedit_read_settings_env(cur_exp, verbose);
	if (ret) {
		printf("Failed to read settings from environment: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cedit_write_cmos(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	struct udevice *dev;
	bool verbose = false;
	int ret;

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		verbose = true;
		argc--;
		argv++;
	}

	if (argc > 1)
		ret = uclass_get_device_by_name(UCLASS_RTC, argv[1], &dev);
	else
		ret = uclass_first_device_err(UCLASS_RTC, &dev);
	if (ret) {
		printf("Failed to get RTC device: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	if (cedit_write_settings_cmos(cur_exp, dev, verbose)) {
		printf("Failed to write settings to CMOS\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cedit_read_cmos(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	struct udevice *dev;
	bool verbose = false;
	int ret;

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		verbose = true;
		argc--;
		argv++;
	}

	if (argc > 1)
		ret = uclass_get_device_by_name(UCLASS_RTC, argv[1], &dev);
	else
		ret = uclass_first_device_err(UCLASS_RTC, &dev);
	if (ret) {
		printf("Failed to get RTC device: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = cedit_read_settings_cmos(cur_exp, dev, verbose);
	if (ret) {
		printf("Failed to read settings from CMOS: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_cedit_run(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ofnode node;
	int ret;

	if (check_cur_expo())
		return CMD_RET_FAILURE;

	node = ofnode_path("/bootstd/cedit-theme");
	if (ofnode_valid(node)) {
		ret = expo_apply_theme(cur_exp, node);
		if (ret)
			return CMD_RET_FAILURE;
	} else {
		log_warning("No theme found\n");
	}
	ret = cedit_run(cur_exp);
	if (ret) {
		log_err("Failed (err=%dE)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

U_BOOT_LONGHELP(cedit,
	"load <interface> <dev[:part]> <filename>   - load config editor\n"
	"cedit read_fdt <i/f> <dev[:part]> <filename>     - read settings\n"
	"cedit write_fdt <i/f> <dev[:part]> <filename>    - write settings\n"
	"cedit read_env [-v]                              - read settings from env vars\n"
	"cedit write_env [-v]                             - write settings to env vars\n"
	"cedit read_cmos [-v] [dev]                       - read settings from CMOS RAM\n"
	"cedit write_cmos [-v] [dev]                      - write settings to CMOS RAM\n"
	"cedit run                                        - run config editor");

U_BOOT_CMD_WITH_SUBCMDS(cedit, "Configuration editor", cedit_help_text,
	U_BOOT_SUBCMD_MKENT(load, 5, 1, do_cedit_load),
	U_BOOT_SUBCMD_MKENT(read_fdt, 5, 1, do_cedit_read_fdt),
	U_BOOT_SUBCMD_MKENT(write_fdt, 5, 1, do_cedit_write_fdt),
	U_BOOT_SUBCMD_MKENT(read_env, 2, 1, do_cedit_read_env),
	U_BOOT_SUBCMD_MKENT(write_env, 2, 1, do_cedit_write_env),
	U_BOOT_SUBCMD_MKENT(read_cmos, 2, 1, do_cedit_read_cmos),
	U_BOOT_SUBCMD_MKENT(write_cmos, 2, 1, do_cedit_write_cmos),
	U_BOOT_SUBCMD_MKENT(run, 1, 1, do_cedit_run),
);
