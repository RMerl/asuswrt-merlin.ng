// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <command.h>
#include <dm/root.h>
#include <dm/util.h>

static int do_dm_dump_driver_compat(struct cmd_tbl *cmdtp, int flag, int argc,
				    char * const argv[])
{
	dm_dump_driver_compat();

	return 0;
}

static int do_dm_dump_devres(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	dm_dump_devres();

	return 0;
}

static int do_dm_dump_drivers(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	dm_dump_drivers();

	return 0;
}

#if CONFIG_IS_ENABLED(DM_STATS)
static int do_dm_dump_mem(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct dm_stats mem;

	dm_get_mem(&mem);
	dm_dump_mem(&mem);

	return 0;
}
#endif /* DM_STATS */

static int do_dm_dump_static_driver_info(struct cmd_tbl *cmdtp, int flag,
					 int argc, char * const argv[])
{
	dm_dump_static_driver_info();

	return 0;
}

static int do_dm_dump_tree(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	bool extended = false, sort = false;
	char *device = NULL;

	for (; argc > 1; argc--, argv++) {
		if (argv[1][0] != '-')
			break;

		if (!strcmp(argv[1], "-e")) {
			extended = true;
		} else if (!strcmp(argv[1], "-s")) {
			sort = true;
		} else {
			printf("Unknown parameter: %s\n", argv[1]);
			return 0;
		}
	}
	if (argc > 1)
		device = argv[1];

	dm_dump_tree(device, extended, sort);

	return 0;
}

static int do_dm_dump_uclass(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	bool extended = false;
	char *uclass = NULL;

	if (argc > 1) {
		if (!strcmp(argv[1], "-e")) {
			extended = true;
			argc--;
			argv++;
		}
		if (argc > 1)
			uclass = argv[1];
	}

	dm_dump_uclass(uclass, extended);

	return 0;
}

#if CONFIG_IS_ENABLED(DM_STATS)
#define DM_MEM_HELP	"dm mem           Provide a summary of memory usage\n"
#define DM_MEM		U_BOOT_SUBCMD_MKENT(mem, 1, 1, do_dm_dump_mem),
#else
#define DM_MEM_HELP
#define DM_MEM
#endif

U_BOOT_LONGHELP(dm,
	"compat        Dump list of drivers with compatibility strings\n"
	"dm devres        Dump list of device resources for each device\n"
	"dm drivers       Dump list of drivers with uclass and instances\n"
	DM_MEM_HELP
	"dm static        Dump list of drivers with static platform data\n"
	"dm tree [-s][-e][name]   Dump tree of driver model devices (-s=sort)\n"
	"dm uclass [-e][name]     Dump list of instances for each uclass");

U_BOOT_CMD_WITH_SUBCMDS(dm, "Driver model low level access", dm_help_text,
	U_BOOT_SUBCMD_MKENT(compat, 1, 1, do_dm_dump_driver_compat),
	U_BOOT_SUBCMD_MKENT(devres, 1, 1, do_dm_dump_devres),
	U_BOOT_SUBCMD_MKENT(drivers, 1, 1, do_dm_dump_drivers),
	DM_MEM
	U_BOOT_SUBCMD_MKENT(static, 1, 1, do_dm_dump_static_driver_info),
	U_BOOT_SUBCMD_MKENT(tree, 4, 1, do_dm_dump_tree),
	U_BOOT_SUBCMD_MKENT(uclass, 3, 1, do_dm_dump_uclass));
