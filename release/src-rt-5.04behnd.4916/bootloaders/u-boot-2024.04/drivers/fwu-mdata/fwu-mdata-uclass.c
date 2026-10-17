// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_FWU_MDATA

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>

#include <linux/errno.h>
#include <linux/types.h>

/**
 * fwu_read_mdata() - Wrapper around fwu_mdata_ops.read_mdata()
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_read_mdata(struct udevice *dev, struct fwu_mdata *mdata, bool primary)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->read_mdata) {
		log_debug("read_mdata() method not defined\n");
		return -ENOSYS;
	}

	return ops->read_mdata(dev, mdata, primary);
}

/**
 * fwu_write_mdata() - Wrapper around fwu_mdata_ops.write_mdata()
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_write_mdata(struct udevice *dev, struct fwu_mdata *mdata, bool primary)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->write_mdata) {
		log_debug("write_mdata() method not defined\n");
		return -ENOSYS;
	}

	return ops->write_mdata(dev, mdata, primary);
}

UCLASS_DRIVER(fwu_mdata) = {
	.id		= UCLASS_FWU_MDATA,
	.name		= "fwu-mdata",
};
