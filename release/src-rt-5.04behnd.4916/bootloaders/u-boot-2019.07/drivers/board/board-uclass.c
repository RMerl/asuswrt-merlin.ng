// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <board.h>

int board_get(struct udevice **devp)
{
	return uclass_first_device_err(UCLASS_BOARD, devp);
}

int board_detect(struct udevice *dev)
{
	struct board_ops *ops = board_get_ops(dev);

	if (!ops->detect)
		return -ENOSYS;

	return ops->detect(dev);
}

int board_get_bool(struct udevice *dev, int id, bool *val)
{
	struct board_ops *ops = board_get_ops(dev);

	if (!ops->get_bool)
		return -ENOSYS;

	return ops->get_bool(dev, id, val);
}

int board_get_int(struct udevice *dev, int id, int *val)
{
	struct board_ops *ops = board_get_ops(dev);

	if (!ops->get_int)
		return -ENOSYS;

	return ops->get_int(dev, id, val);
}

int board_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct board_ops *ops = board_get_ops(dev);

	if (!ops->get_str)
		return -ENOSYS;

	return ops->get_str(dev, id, size, val);
}

UCLASS_DRIVER(board) = {
	.id		= UCLASS_BOARD,
	.name		= "board",
	.post_bind	= dm_scan_fdt_dev,
};
