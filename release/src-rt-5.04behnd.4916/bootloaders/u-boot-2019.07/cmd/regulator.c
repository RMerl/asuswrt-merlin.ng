// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <power/regulator.h>

#define LIMIT_DEVNAME	20
#define LIMIT_OFNAME	32
#define LIMIT_INFO	18

static struct udevice *currdev;

static int failure(int ret)
{
	printf("Error: %d (%s)\n", ret, errno_str(ret));

	return CMD_RET_FAILURE;
}

static int do_dev(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	const char *name;
	int ret = -ENXIO;

	switch (argc) {
	case 2:
		name = argv[1];
		ret = regulator_get_by_platname(name, &currdev);
		if (ret) {
			printf("Can't get the regulator: %s!\n", name);
			return failure(ret);
		}
	case 1:
		if (!currdev) {
			printf("Regulator device is not set!\n\n");
			return CMD_RET_USAGE;
		}

		uc_pdata = dev_get_uclass_platdata(currdev);
		if (!uc_pdata) {
			printf("%s: no regulator platform data!\n", currdev->name);
			return failure(ret);
		}

		printf("dev: %s @ %s\n", uc_pdata->name, currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int curr_dev_and_platdata(struct udevice **devp,
				 struct dm_regulator_uclass_platdata **uc_pdata,
				 bool allow_type_fixed)
{
	*devp = NULL;
	*uc_pdata = NULL;

	if (!currdev) {
		printf("First, set the regulator device!\n");
		return CMD_RET_FAILURE;
	}

	*devp = currdev;

	*uc_pdata = dev_get_uclass_platdata(*devp);
	if (!*uc_pdata) {
		pr_err("Regulator: %s - missing platform data!\n", currdev->name);
		return CMD_RET_FAILURE;
	}

	if (!allow_type_fixed && (*uc_pdata)->type == REGULATOR_TYPE_FIXED) {
		printf("Operation not allowed for fixed regulator!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int ret;

	printf("| %-*.*s| %-*.*s| %s\n",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Device",
	       LIMIT_OFNAME, LIMIT_OFNAME, "regulator-name",
	       "Parent");

	for (ret = uclass_find_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret)
			continue;

		uc_pdata = dev_get_uclass_platdata(dev);
		printf("| %-*.*s| %-*.*s| %s\n",
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
		       LIMIT_OFNAME, LIMIT_OFNAME, uc_pdata->name,
		       dev->parent->name);
	}

	return ret;
}

static int constraint(const char *name, int val, const char *val_name)
{
	printf("%-*s", LIMIT_INFO, name);
	if (val < 0) {
		printf(" %s (err: %d)\n", errno_str(val), val);
		return val;
	}

	if (val_name)
		printf(" %d (%s)\n", val, val_name);
	else
		printf(" %d\n", val);

	return 0;
}

static const char *get_mode_name(struct dm_regulator_mode *mode,
				 int mode_count,
				 int mode_id)
{
	while (mode_count--) {
		if (mode->id == mode_id)
			return mode->name;
		mode++;
	}

	return NULL;
}

static int do_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct dm_regulator_mode *modes;
	const char *parent_uc;
	int mode_count;
	int ret;
	int i;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	parent_uc = dev_get_uclass_name(dev->parent);

	printf("%s\n%-*s %s\n%-*s %s\n%-*s %s\n%-*s %s\n%-*s\n",
	       "Regulator info:",
	       LIMIT_INFO, "* regulator-name:", uc_pdata->name,
	       LIMIT_INFO, "* device name:", dev->name,
	       LIMIT_INFO, "* parent name:", dev->parent->name,
	       LIMIT_INFO, "* parent uclass:", parent_uc,
	       LIMIT_INFO, "* constraints:");

	constraint("  - min uV:", uc_pdata->min_uV, NULL);
	constraint("  - max uV:", uc_pdata->max_uV, NULL);
	constraint("  - min uA:", uc_pdata->min_uA, NULL);
	constraint("  - max uA:", uc_pdata->max_uA, NULL);
	constraint("  - always on:", uc_pdata->always_on,
		   uc_pdata->always_on ? "true" : "false");
	constraint("  - boot on:", uc_pdata->boot_on,
		   uc_pdata->boot_on ? "true" : "false");

	mode_count = regulator_mode(dev, &modes);
	constraint("* op modes:", mode_count, NULL);

	for (i = 0; i < mode_count; i++, modes++)
		constraint("  - mode id:", modes->id, modes->name);

	return CMD_RET_SUCCESS;
}

static void do_status_detail(struct udevice *dev,
			     struct dm_regulator_uclass_platdata *uc_pdata)
{
	int current, value, mode;
	const char *mode_name;
	bool enabled;

	printf("Regulator %s status:\n", uc_pdata->name);

	enabled = regulator_get_enable(dev);
	constraint(" * enable:", enabled, enabled ? "true" : "false");

	value = regulator_get_value(dev);
	constraint(" * value uV:", value, NULL);

	current = regulator_get_current(dev);
	constraint(" * current uA:", current, NULL);

	mode = regulator_get_mode(dev);
	mode_name = get_mode_name(uc_pdata->mode, uc_pdata->mode_count, mode);
	constraint(" * mode id:", mode, mode_name);
}

static void do_status_line(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *pdata;
	int current, value, mode;
	const char *mode_name;
	bool enabled;

	pdata = dev_get_uclass_platdata(dev);
	enabled = regulator_get_enable(dev);
	value = regulator_get_value(dev);
	current = regulator_get_current(dev);
	mode = regulator_get_mode(dev);
	mode_name = get_mode_name(pdata->mode, pdata->mode_count, mode);
	printf("%-20s %-10s ", pdata->name, enabled ? "enabled" : "disabled");
	if (value >= 0)
		printf("%10d ", value);
	else
		printf("%10s ", "-");
	if (current >= 0)
		printf("%10d ", current);
	else
		printf("%10s ", "-");
	if (mode >= 0)
		printf("%-10s", mode_name);
	else
		printf("%-10s", "-");
	printf("\n");
}

static int do_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int ret;

	if (currdev && (argc < 2 || strcmp(argv[1], "-a"))) {
		ret = curr_dev_and_platdata(&dev, &uc_pdata, true);
		if (ret)
			return CMD_RET_FAILURE;
		do_status_detail(dev, uc_pdata);
		return 0;
	}

	/* Show all of them in a list, probing them as needed */
	printf("%-20s %-10s %10s %10s %-10s\n", "Name", "Enabled", "uV", "mA",
	       "Mode");
	for (ret = uclass_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_next_device(&dev))
		do_status_line(dev);

	return CMD_RET_SUCCESS;
}

static int do_value(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int value;
	int force;
	int ret;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, argc == 1);
	if (ret)
		return ret;

	if (argc == 1) {
		ret = regulator_get_value(dev);
		if (ret < 0) {
			printf("Regulator: %s - can't get the Voltage!\n",
			       uc_pdata->name);
			return failure(ret);
		}

		printf("%d uV\n", ret);
		return CMD_RET_SUCCESS;
	}

	if (argc == 3)
		force = !strcmp("-f", argv[2]);
	else
		force = 0;

	value = simple_strtoul(argv[1], NULL, 0);
	if ((value < uc_pdata->min_uV || value > uc_pdata->max_uV) && !force) {
		printf("Value exceeds regulator constraint limits %d..%d uV\n",
		       uc_pdata->min_uV, uc_pdata->max_uV);
		return CMD_RET_FAILURE;
	}

	if (!force)
		ret = regulator_set_value(dev, value);
	else
		ret = regulator_set_value_force(dev, value);
	if (ret) {
		printf("Regulator: %s - can't set the Voltage!\n",
		       uc_pdata->name);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static int do_current(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int current;
	int ret;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, argc == 1);
	if (ret)
		return ret;

	if (argc == 1) {
		ret = regulator_get_current(dev);
		if (ret < 0) {
			printf("Regulator: %s - can't get the Current!\n",
			       uc_pdata->name);
			return failure(ret);
		}

		printf("%d uA\n", ret);
		return CMD_RET_SUCCESS;
	}

	current = simple_strtoul(argv[1], NULL, 0);
	if (current < uc_pdata->min_uA || current > uc_pdata->max_uA) {
		printf("Current exceeds regulator constraint limits\n");
		return CMD_RET_FAILURE;
	}

	ret = regulator_set_current(dev, current);
	if (ret) {
		printf("Regulator: %s - can't set the Current!\n",
		       uc_pdata->name);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static int do_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int mode;
	int ret;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, false);
	if (ret)
		return ret;

	if (argc == 1) {
		ret = regulator_get_mode(dev);
		if (ret < 0) {
			printf("Regulator: %s - can't get the operation mode!\n",
			       uc_pdata->name);
			return failure(ret);
		}

		printf("mode id: %d\n", ret);
		return CMD_RET_SUCCESS;
	}

	mode = simple_strtoul(argv[1], NULL, 0);

	ret = regulator_set_mode(dev, mode);
	if (ret) {
		printf("Regulator: %s - can't set the operation mode!\n",
		       uc_pdata->name);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static int do_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int ret;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	ret = regulator_set_enable(dev, true);
	if (ret) {
		printf("Regulator: %s - can't enable!\n", uc_pdata->name);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static int do_disable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int ret;

	ret = curr_dev_and_platdata(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	ret = regulator_set_enable(dev, false);
	if (ret) {
		printf("Regulator: %s - can't disable!\n", uc_pdata->name);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 1, do_info, "", ""),
	U_BOOT_CMD_MKENT(status, 2, 1, do_status, "", ""),
	U_BOOT_CMD_MKENT(value, 3, 1, do_value, "", ""),
	U_BOOT_CMD_MKENT(current, 3, 1, do_current, "", ""),
	U_BOOT_CMD_MKENT(mode, 2, 1, do_mode, "", ""),
	U_BOOT_CMD_MKENT(enable, 1, 1, do_enable, "", ""),
	U_BOOT_CMD_MKENT(disable, 1, 1, do_disable, "", ""),
};

static int do_regulator(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], subcmd, ARRAY_SIZE(subcmd));
	if (cmd == NULL || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(regulator, CONFIG_SYS_MAXARGS, 1, do_regulator,
	"uclass operations",
	"list             - list UCLASS regulator devices\n"
	"regulator dev [regulator-name] - show/[set] operating regulator device\n"
	"regulator info                 - print constraints info\n"
	"regulator status [-a]          - print operating status [for all]\n"
	"regulator value [val] [-f]     - print/[set] voltage value [uV] (force)\n"
	"regulator current [val]        - print/[set] current value [uA]\n"
	"regulator mode [id]            - print/[set] operating mode id\n"
	"regulator enable               - enable the regulator output\n"
	"regulator disable              - disable the regulator output\n"
);
