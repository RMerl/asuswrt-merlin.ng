// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <common.h>
#include <bootdev.h>
#include <command.h>
#include <dm.h>
#include <malloc.h>
#include <extension_board.h>
#include <mapmem.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

static LIST_HEAD(extension_list);

static int extension_apply(struct extension *extension)
{
	char *overlay_cmd;
	ulong extrasize, overlay_addr;
	struct fdt_header *blob;

	if (!working_fdt) {
		printf("No FDT memory address configured. Please configure\n"
		       "the FDT address via \"fdt addr <address>\" command.\n");
		return CMD_RET_FAILURE;
	}

	overlay_cmd = env_get("extension_overlay_cmd");
	if (!overlay_cmd) {
		printf("Environment extension_overlay_cmd is missing\n");
		return CMD_RET_FAILURE;
	}

	overlay_addr = env_get_hex("extension_overlay_addr", 0);
	if (!overlay_addr) {
		printf("Environment extension_overlay_addr is missing\n");
		return CMD_RET_FAILURE;
	}

	env_set("extension_overlay_name", extension->overlay);
	if (run_command(overlay_cmd, 0) != 0)
		return CMD_RET_FAILURE;

	extrasize = env_get_hex("filesize", 0);
	if (!extrasize)
		return CMD_RET_FAILURE;

	fdt_shrink_to_minimum(working_fdt, extrasize);

	blob = map_sysmem(overlay_addr, 0);
	if (!fdt_valid(&blob))
		return CMD_RET_FAILURE;

	/* apply method prints messages on error */
	if (fdt_overlay_apply_verbose(working_fdt, blob))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_extension_list(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	int i = 0;
	struct extension *extension;

	if (list_empty(&extension_list)) {
		printf("No extension registered - Please run \"extension scan\"\n");
		return CMD_RET_SUCCESS;
	}

	list_for_each_entry(extension, &extension_list, list) {
		printf("Extension %d: %s\n", i++, extension->name);
		printf("\tManufacturer: \t\t%s\n", extension->owner);
		printf("\tVersion: \t\t%s\n", extension->version);
		printf("\tDevicetree overlay: \t%s\n", extension->overlay);
		printf("\tOther information: \t%s\n", extension->other);
	}
	return CMD_RET_SUCCESS;
}

static int extension_scan(bool show)
{
	struct extension *extension, *next;
	int extension_num;

	list_for_each_entry_safe(extension, next, &extension_list, list) {
		list_del(&extension->list);
		free(extension);
	}
	extension_num = extension_board_scan(&extension_list);
	if (show && extension_num >= 0)
		printf("Found %d extension board(s).\n", extension_num);

	/* either the number of extensions, or -ve for error */
	return extension_num;
}


static int do_extension_scan(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	int extension_num;

	extension_num = extension_scan(true);
	if (extension_num < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_extension_apply(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	struct extension *extension = NULL;
	struct list_head *entry;
	int i = 0, extension_id, ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "all") == 0) {
		ret = CMD_RET_FAILURE;
		list_for_each_entry(extension, &extension_list, list) {
			ret = extension_apply(extension);
			if (ret != CMD_RET_SUCCESS)
				break;
		}
	} else {
		extension_id = simple_strtol(argv[1], NULL, 10);
		list_for_each(entry, &extension_list) {
			if (i == extension_id) {
				extension = list_entry(entry, struct extension,  list);
				break;
			}
			i++;
		}

		if (!extension) {
			printf("Wrong extension number\n");
			return CMD_RET_FAILURE;
		}

		ret = extension_apply(extension);
	}

	return ret;
}

static struct cmd_tbl cmd_extension[] = {
	U_BOOT_CMD_MKENT(scan, 1, 1, do_extension_scan, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 0, do_extension_list, "", ""),
	U_BOOT_CMD_MKENT(apply, 2, 0, do_extension_apply, "", ""),
};

static int do_extensionops(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct cmd_tbl *cp;

	/* Drop the extension command */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_extension, ARRAY_SIZE(cmd_extension));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(extension, 3, 1, do_extensionops,
	"Extension board management sub system",
	"scan - scan plugged extension(s) board(s)\n"
	"extension list - lists available extension(s) board(s)\n"
	"extension apply <extension number|all> - applies DT overlays corresponding to extension boards\n"
);

static int extension_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	ret = env_set_hex("extension_overlay_addr",
			  env_get_hex("fdtoverlay_addr_r", 0));
	if (ret)
		return log_msg_ret("env", ret);

	ret = extension_scan(show);
	if (ret < 0)
		return log_msg_ret("ext", ret);

	return 0;
}

/* extensions should have a uclass - for now we use UCLASS_SIMPLE_BUS uclass */
BOOTDEV_HUNTER(extension_bootdev_hunter) = {
	.prio		= BOOTDEVP_1_PRE_SCAN,
	.uclass		= UCLASS_SIMPLE_BUS,
	.hunt		= extension_bootdev_hunt,
};
