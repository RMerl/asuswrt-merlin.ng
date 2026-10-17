// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * made from cmd_ext2, which was:
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <env.h>
#include <part.h>
#include <vsprintf.h>

enum cmd_part_info {
	CMD_PART_INFO_START = 0,
	CMD_PART_INFO_SIZE,
	CMD_PART_INFO_NUMBER
};

static int do_part_uuid(int argc, char *const argv[])
{
	int part;
	struct blk_desc *dev_desc;
	struct disk_partition info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[0], argv[1], &dev_desc, &info, 0);
	if (part < 0)
		return 1;

	if (argc > 2)
		env_set(argv[2], info.uuid);
	else
		printf("%s\n", info.uuid);

	return 0;
}

static int do_part_list(int argc, char *const argv[])
{
	int ret;
	struct blk_desc *desc;
	char *var = NULL;
	bool bootable = false;
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2) {
		for (i = 2; i < argc ; i++) {
			if (argv[i][0] == '-') {
				if (!strcmp(argv[i], "-bootable")) {
					bootable = true;
				} else {
					printf("Unknown option %s\n", argv[i]);
					return CMD_RET_USAGE;
				}
			} else {
				var = argv[i];
				break;
			}
		}

		/* Loops should have been exited at the last argument, which
		 * as it contained the variable */
		if (argc != i + 1)
			return CMD_RET_USAGE;
	}

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	if (var != NULL) {
		int p;
		char str[3 * MAX_SEARCH_PARTITIONS] = { '\0', };
		struct disk_partition info;

		for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
			char t[5];
			int r = part_get_info(desc, p, &info);

			if (r != 0)
				continue;

			if (bootable && !info.bootable)
				continue;

			sprintf(t, "%s%x", str[0] ? " " : "", p);
			strcat(str, t);
		}
		env_set(var, str);
		return 0;
	}

	part_print(desc);

	return 0;
}

static int do_part_info(int argc, char *const argv[], enum cmd_part_info param)
{
	struct blk_desc *desc;
	struct disk_partition info;
	char buf[512] = { 0 };
	char *endp;
	int part;
	int err;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	part = simple_strtoul(argv[2], &endp, 0);
	if (*endp == '\0') {
		err = part_get_info(desc, part, &info);
		if (err)
			return 1;
	} else {
		part = part_get_info_by_name(desc, argv[2], &info);
		if (part < 0)
			return 1;
	}

	switch (param) {
	case CMD_PART_INFO_START:
		snprintf(buf, sizeof(buf), LBAF, info.start);
		break;
	case CMD_PART_INFO_SIZE:
		snprintf(buf, sizeof(buf), LBAF, info.size);
		break;
	case CMD_PART_INFO_NUMBER:
		snprintf(buf, sizeof(buf), "0x%x", part);
		break;
	default:
		printf("** Unknown cmd_part_info value: %d\n", param);
		return 1;
	}

	if (argc > 3)
		env_set(argv[3], buf);
	else
		printf("%s\n", buf);

	return 0;
}

static int do_part_start(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_START);
}

static int do_part_size(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_SIZE);
}

static int do_part_number(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_NUMBER);
}

static int do_part_set(int argc, char *const argv[])
{
	const char *devname, *partstr, *typestr;
	struct blk_desc *desc;
	int dev;

	if (argc < 3)
		return CMD_RET_USAGE;

	/* Look up the device */
	devname = argv[0];
	partstr = argv[1];
	typestr = argv[2];
	dev = blk_get_device_by_str(devname, partstr, &desc);
	if (dev < 0) {
		printf("** Bad device specification %s %s **\n", devname,
		       partstr);
		return CMD_RET_FAILURE;
	}

	desc->part_type = part_get_type_by_name(typestr);
	if (!desc->part_type) {
		printf("Unknown partition type '%s'\n", typestr);
		return CMD_RET_FAILURE;
	}
	part_print(desc);

	return 0;
}

#ifdef CONFIG_PARTITION_TYPE_GUID
static int do_part_type(int argc, char *const argv[])
{
	int part;
	struct blk_desc *dev_desc;
	struct disk_partition info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[0], argv[1], &dev_desc, &info, 0);
	if (part < 0)
		return 1;

	if (argc > 2)
		env_set(argv[2], info.type_guid);
	else
		printf("%s\n", info.type_guid);

	return 0;
}
#endif

static int do_part_types(int argc, char * const argv[])
{
	struct part_driver *drv = ll_entry_start(struct part_driver,
						 part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;
	int i = 0;

	puts("Supported partition tables");

	for (entry = drv; entry != drv + n_ents; entry++) {
		printf("%c %s", i ? ',' : ':', entry->name);
		i++;
	}
	if (!i)
		puts(": <none>");
	puts("\n");
	return CMD_RET_SUCCESS;
}

static int do_part(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "uuid"))
		return do_part_uuid(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "list"))
		return do_part_list(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "start"))
		return do_part_start(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "size"))
		return do_part_size(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "number"))
		return do_part_number(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "types"))
		return do_part_types(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "set"))
		return do_part_set(argc - 2, argv + 2);
#ifdef CONFIG_PARTITION_TYPE_GUID
	else if (!strcmp(argv[1], "type"))
		return do_part_type(argc - 2, argv + 2);
#endif
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	part,	CONFIG_SYS_MAXARGS,	1,	do_part,
	"disk partition related commands",
	"uuid <interface> <dev>:<part>\n"
	"    - print partition UUID\n"
	"part uuid <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition UUID\n"
	"part list <interface> <dev>\n"
	"    - print a device's partition table\n"
	"part list <interface> <dev> [flags] <varname>\n"
	"    - set environment variable to the list of partitions\n"
	"      flags can be -bootable (list only bootable partitions)\n"
	"part start <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the start of the partition (in blocks)\n"
	"      part can be either partition number or partition name\n"
	"part size <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the size of the partition (in blocks)\n"
	"      part can be either partition number or partition name\n"
	"part number <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the partition number using the partition name\n"
	"      part must be specified as partition name\n"
#ifdef CONFIG_PARTITION_TYPE_GUID
	"part type <interface> <dev>:<part>\n"
	"    - print partition type\n"
	"part type <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition type\n"
#endif
	"part set <interface> <dev> type\n"
	"    - set partition type for a device\n"
	"part types\n"
	"    - list supported partition table types"
);
