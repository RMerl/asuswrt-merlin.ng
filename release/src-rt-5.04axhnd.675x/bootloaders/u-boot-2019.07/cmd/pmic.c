// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>

#define LIMIT_DEV	32
#define LIMIT_PARENT	20

static struct udevice *currdev;

static int failure(int ret)
{
	printf("Error: %d (%s)\n", ret, errno_str(ret));

	return CMD_RET_FAILURE;
}

static int do_dev(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *name;
	int ret = -ENODEV;

	switch (argc) {
	case 2:
		name = argv[1];
		ret = pmic_get(name, &currdev);
		if (ret) {
			printf("Can't get PMIC: %s!\n", name);
			return failure(ret);
		}
	case 1:
		if (!currdev) {
			printf("PMIC device is not set!\n\n");
			return CMD_RET_USAGE;
		}

		printf("dev: %d @ %s\n", currdev->seq, currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int do_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int ret;

	printf("| %-*.*s| %-*.*s| %s @ %s\n",
	       LIMIT_DEV, LIMIT_DEV, "Name",
	       LIMIT_PARENT, LIMIT_PARENT, "Parent name",
	       "Parent uclass", "seq");

	for (ret = uclass_first_device(UCLASS_PMIC, &dev); dev;
	     ret = uclass_next_device(&dev)) {
		if (ret)
			continue;

		printf("| %-*.*s| %-*.*s| %s @ %d\n",
		       LIMIT_DEV, LIMIT_DEV, dev->name,
		       LIMIT_PARENT, LIMIT_PARENT, dev->parent->name,
		       dev_get_uclass_name(dev->parent), dev->parent->seq);
	}

	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct uc_pmic_priv *priv;
	struct udevice *dev;
	char fmt[16];
	uint reg;
	int ret;

	if (!currdev) {
		printf("First, set the PMIC device!\n");
		return CMD_RET_USAGE;
	}

	dev = currdev;
	priv = dev_get_uclass_priv(dev);
	printf("Dump pmic: %s registers\n", dev->name);

	sprintf(fmt, "%%%d.%dx ", priv->trans_len * 2,
		priv->trans_len * 2);

	for (reg = 0; reg < pmic_reg_count(dev); reg++) {
		ret = pmic_reg_read(dev, reg);
		if (ret < 0) {
			printf("Can't read register: %d\n", reg);
			return failure(ret);
		}

		if (!(reg % 16))
			printf("\n0x%02x: ", reg);

		printf(fmt, ret);
	}
	printf("\n");

	return CMD_RET_SUCCESS;
}

static int do_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct uc_pmic_priv *priv;
	struct udevice *dev;
	int regs, ret;
	char fmt[24];
	uint reg;

	if (!currdev) {
		printf("First, set the PMIC device!\n");
		return CMD_RET_USAGE;
	}

	dev = currdev;
	priv = dev_get_uclass_priv(dev);

	if (argc != 2)
		return CMD_RET_USAGE;

	reg = simple_strtoul(argv[1], NULL, 0);
	regs = pmic_reg_count(dev);
	if (reg > regs) {
		printf("PMIC max reg: %d\n", regs);
		return failure(-EFAULT);
	}

	ret = pmic_reg_read(dev, reg);
	if (ret < 0) {
		printf("Can't read PMIC register: %d!\n", reg);
		return failure(ret);
	}

	sprintf(fmt, "0x%%02x: 0x%%%d.%dx\n", priv->trans_len * 2,
		priv->trans_len * 2);
	printf(fmt, reg, ret);

	return CMD_RET_SUCCESS;
}

static int do_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	uint reg, value;
	int regs, ret;

	if (!currdev) {
		printf("First, set the PMIC device!\n");
		return CMD_RET_USAGE;
	}

	dev = currdev;

	if (argc != 3)
		return CMD_RET_USAGE;

	reg = simple_strtoul(argv[1], NULL, 0);
	regs = pmic_reg_count(dev);
	if (reg > regs) {
		printf("PMIC max reg: %d\n", regs);
		return failure(-EFAULT);
	}

	value = simple_strtoul(argv[2], NULL, 0);

	ret = pmic_reg_write(dev, reg, value);
	if (ret) {
		printf("Can't write PMIC register: %d!\n", reg);
		return failure(ret);
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(dump, 1, 1, do_dump, "", ""),
	U_BOOT_CMD_MKENT(read, 2, 1, do_read, "", ""),
	U_BOOT_CMD_MKENT(write, 3, 1, do_write, "", ""),
};

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc,
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

U_BOOT_CMD(pmic, CONFIG_SYS_MAXARGS, 1, do_pmic,
	"PMIC sub-system",
	"list          - list pmic devices\n"
	"pmic dev [name]    - show or [set] operating PMIC device\n"
	"pmic dump          - dump registers\n"
	"pmic read address  - read byte of register at address\n"
	"pmic write address - write byte to register at address\n"
);
