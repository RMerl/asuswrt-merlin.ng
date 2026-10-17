// SPDX-License-Identifier: GPL-2.0+
/*
 * Read a bootflow from SPI flash
 *
 * Copyright 2022 Google LLC
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>
#include <env.h>
#include <malloc.h>
#include <spi_flash.h>

static int sf_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			   struct bootflow *bflow)
{
	struct udevice *sf = dev_get_parent(dev);
	uint size;
	char *buf;
	int ret;

	/* We only support the whole device, not partitions */
	if (iter->part)
		return log_msg_ret("max", -ESHUTDOWN);

	size = env_get_hex("script_size_f", 0);
	if (!size)
		return log_msg_ret("sz", -EINVAL);

	buf = malloc(size + 1);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	ret = spi_flash_read_dm(sf, env_get_hex("script_offset_f", 0),
				size, buf);
	if (ret)
		return log_msg_ret("cmd", -EINVAL);
	bflow->state = BOOTFLOWST_MEDIA;

	ret = bootmeth_set_bootflow(bflow->method, bflow, buf, size);
	if (ret) {
		free(buf);
		return log_msg_ret("method", ret);
	}

	return 0;
}

static int sf_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST;

	return 0;
}

struct bootdev_ops sf_bootdev_ops = {
	.get_bootflow	= sf_get_bootflow,
};

static const struct udevice_id sf_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-sf" },
	{ }
};

U_BOOT_DRIVER(sf_bootdev) = {
	.name		= "sf_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &sf_bootdev_ops,
	.bind		= sf_bootdev_bind,
	.of_match	= sf_bootdev_ids,
};

BOOTDEV_HUNTER(sf_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_SPI_FLASH,
	.drv		= DM_DRIVER_REF(sf_bootdev),
};
