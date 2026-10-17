// SPDX-License-Identifier: GPL-2.0
/*
 * List, select, and deselect mux controllers on the fly.
 *
 * Copyright (c) 2020 Texas Instruments Inc.
 * Author: Pratyush Yadav <p.yadav@ti.com>
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <mux.h>
#include <mux-internal.h>
#include <linux/err.h>
#include <dt-bindings/mux/mux.h>

#define COLUMN_SIZE 16

/*
 * Print a member of a column. The total size of the text printed, including
 * trailing whitespace, will always be COLUMN_SIZE.
 */
#define PRINT_COLUMN(fmt, args...) do {					\
	char buf[COLUMN_SIZE + 1];					\
	snprintf(buf, COLUMN_SIZE + 1, fmt, ##args);			\
	printf("%-*s", COLUMN_SIZE, buf);				\
} while (0)

/*
 * Find a mux based on its device name in argv[1] and index in the chip in
 * argv[2].
 */
static struct mux_control *cmd_mux_find(char *const argv[])
{
	struct udevice *dev;
	struct mux_chip *chip;
	int ret;
	unsigned long id;

	ret = strict_strtoul(argv[2], 10, &id);
	if (ret)
		return ERR_PTR(ret);

	ret = uclass_get_device_by_name(UCLASS_MUX, argv[1], &dev);
	if (ret)
		return ERR_PTR(ret);

	chip = dev_get_uclass_priv(dev);
	if (!chip)
		return ERR_PTR(-EINVAL);

	if (id >= chip->controllers)
		return ERR_PTR(-EINVAL);

	return &chip->mux[id];
}

/*
 * Print the details of a mux. The columns printed correspond to: "Selected",
 * "Current State", "Idle State", and "Num States".
 */
static void print_mux(struct mux_control *mux)
{
	PRINT_COLUMN("%s", mux->in_use ? "yes" : "no");

	if (mux->cached_state == MUX_IDLE_AS_IS)
		PRINT_COLUMN("%s", "unknown");
	else
		PRINT_COLUMN("0x%x", mux->cached_state);

	if (mux->idle_state == MUX_IDLE_AS_IS)
		PRINT_COLUMN("%s", "as-is");
	else if (mux->idle_state == MUX_IDLE_DISCONNECT)
		PRINT_COLUMN("%s", "disconnect");
	else
		PRINT_COLUMN("0x%x", mux->idle_state);

	PRINT_COLUMN("0x%x", mux->states);

	printf("\n");
}

static int do_mux_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	struct mux_chip *chip;
	int j;

	for (uclass_first_device(UCLASS_MUX, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		chip = dev_get_uclass_priv(dev);
		if (!chip) {
			dev_err(dev, "can't find mux chip\n");
			continue;
		}

		printf("%s:\n", dev->name);

		printf("    ");
		PRINT_COLUMN("ID");
		PRINT_COLUMN("Selected");
		PRINT_COLUMN("Current State");
		PRINT_COLUMN("Idle State");
		PRINT_COLUMN("Num States");
		printf("\n");
		for (j = 0; j < chip->controllers; j++) {
			printf("    ");
			PRINT_COLUMN("%d", j);
			print_mux(&chip->mux[j]);
		}
		printf("\n");
	}

	return 0;
}

static int do_mux_select(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct mux_control *mux;
	int ret;
	unsigned long state;

	if (argc != 4)
		return CMD_RET_USAGE;

	mux = cmd_mux_find(argv);
	if (IS_ERR_OR_NULL(mux)) {
		printf("Failed to find the specified mux\n");
		return CMD_RET_FAILURE;
	}

	ret = strict_strtoul(argv[3], 16, &state);
	if (ret) {
		printf("Invalid state\n");
		return CMD_RET_FAILURE;
	}

	ret = mux_control_select(mux, state);
	if (ret) {
		printf("Failed to select requested state\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_mux_deselect(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct mux_control *mux;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	mux = cmd_mux_find(argv);
	if (IS_ERR_OR_NULL(mux)) {
		printf("Failed to find the specified mux\n");
		return CMD_RET_FAILURE;
	}

	ret = mux_control_deselect(mux);
	if (ret) {
		printf("Failed to deselect mux\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(mux,
	"list - List all Muxes and their states\n"
	"select <chip> <id> <state> - Select the given mux state\n"
	"deselect <chip> <id> - Deselect the given mux and reset it to its idle state");

U_BOOT_CMD_WITH_SUBCMDS(mux, "List, select, and deselect muxes", mux_help_text,
			U_BOOT_SUBCMD_MKENT(list, 1, 1, do_mux_list),
			U_BOOT_SUBCMD_MKENT(select, 4, 0, do_mux_select),
			U_BOOT_SUBCMD_MKENT(deselect, 3, 0, do_mux_deselect));
