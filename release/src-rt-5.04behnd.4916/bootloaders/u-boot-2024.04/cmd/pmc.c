// SPDX-License-Identifier: GPL-2.0
/*
 * Intel PMC command
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <power/acpi_pmc.h>

static int get_pmc_dev(struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &dev);
	if (ret) {
		printf("Could not find device (err=%d)\n", ret);
		return ret;
	}
	ret = pmc_init(dev);
	if (ret) {
		printf("Could not init device (err=%d)\n", ret);
		return ret;
	}
	*devp = dev;

	return 0;
}

static int do_pmc_init(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	int ret;

	ret = get_pmc_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int do_pmc_info(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	int ret;

	ret = get_pmc_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;
	pmc_dump_info(dev);

	return 0;
}

static struct cmd_tbl cmd_pmc_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_pmc_init, "", ""),
	U_BOOT_CMD_MKENT(info, 0, 1, do_pmc_info, "", ""),
};

static int do_pmc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct cmd_tbl *cp;

	if (argc < 2) /* no subcommand */
		return cmd_usage(cmdtp);

	cp = find_cmd_tbl(argv[1], &cmd_pmc_sub[0], ARRAY_SIZE(cmd_pmc_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	pmc, 2, 1, do_pmc, "Power-management controller info",
	"info - read state and show info about the PMC\n"
	"pmc init - read state from the PMC\n"
	);
