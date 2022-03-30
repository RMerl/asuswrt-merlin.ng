// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <board.h>

#include "sandbox.h"

struct board_sandbox_priv {
	bool called_detect;
	int test_i1;
	int test_i2;
};

char vacation_spots[][64] = {"R'lyeh", "Dreamlands", "Plateau of Leng",
			     "Carcosa", "Yuggoth", "The Nameless City"};

int board_sandbox_detect(struct udevice *dev)
{
	struct board_sandbox_priv *priv = dev_get_priv(dev);

	priv->called_detect = true;
	priv->test_i2 = 100;

	return 0;
}

int board_sandbox_get_bool(struct udevice *dev, int id, bool *val)
{
	struct board_sandbox_priv *priv = dev_get_priv(dev);

	switch (id) {
	case BOOL_CALLED_DETECT:
		/* Checks if the dectect method has been called */
		*val = priv->called_detect;
		return 0;
	}

	return -ENOENT;
}

int board_sandbox_get_int(struct udevice *dev, int id, int *val)
{
	struct board_sandbox_priv *priv = dev_get_priv(dev);

	switch (id) {
	case INT_TEST1:
		*val = priv->test_i1;
		/* Increments with every call */
		priv->test_i1++;
		return 0;
	case INT_TEST2:
		*val = priv->test_i2;
		/* Decrements with every call */
		priv->test_i2--;
		return 0;
	}

	return -ENOENT;
}

int board_sandbox_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct board_sandbox_priv *priv = dev_get_priv(dev);
	int i1 = priv->test_i1;
	int i2 = priv->test_i2;
	int index = (i1 * i2) % ARRAY_SIZE(vacation_spots);

	switch (id) {
	case STR_VACATIONSPOT:
		/* Picks a vacation spot depending on i1 and i2 */
		snprintf(val, size, vacation_spots[index]);
		return 0;
	}

	return -ENOENT;
}

static const struct udevice_id board_sandbox_ids[] = {
	{ .compatible = "sandbox,board_sandbox" },
	{ /* sentinel */ }
};

static const struct board_ops board_sandbox_ops = {
	.detect = board_sandbox_detect,
	.get_bool = board_sandbox_get_bool,
	.get_int = board_sandbox_get_int,
	.get_str = board_sandbox_get_str,
};

int board_sandbox_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(board_sandbox) = {
	.name           = "board_sandbox",
	.id             = UCLASS_BOARD,
	.of_match       = board_sandbox_ids,
	.ops		= &board_sandbox_ops,
	.priv_auto_alloc_size = sizeof(struct board_sandbox_priv),
	.probe          = board_sandbox_probe,
};
