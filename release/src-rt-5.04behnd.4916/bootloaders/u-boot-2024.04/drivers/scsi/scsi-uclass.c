// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016 Xilinx, Inc
 * Written by Michal Simek
 *
 * Based on ahci-uclass.c
 */

#define LOG_CATEGORY UCLASS_SCSI

#include <common.h>
#include <dm.h>
#include <scsi.h>

int scsi_exec(struct udevice *dev, struct scsi_cmd *pccb)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->exec)
		return -ENOSYS;

	return ops->exec(dev, pccb);
}

int scsi_bus_reset(struct udevice *dev)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->bus_reset)
		return -ENOSYS;

	return ops->bus_reset(dev);
}

UCLASS_DRIVER(scsi) = {
	.id		= UCLASS_SCSI,
	.name		= "scsi",
	.per_device_plat_auto	= sizeof(struct scsi_plat),
};
