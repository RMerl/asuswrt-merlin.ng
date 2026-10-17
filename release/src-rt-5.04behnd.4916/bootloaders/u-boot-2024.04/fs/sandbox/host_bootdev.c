// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for sandbox host
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>
#include <fs.h>

static int host_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			     struct bootflow *bflow)
{
	int ret;

	if (iter->part)
		return log_msg_ret("max", -ESHUTDOWN);

	bflow->name = strdup(dev->name);
	if (!bflow->name)
		return log_msg_ret("name", -ENOMEM);

	ret = bootmeth_check(bflow->method, iter);
	if (ret)
		return log_msg_ret("check", ret);

	bflow->state = BOOTFLOWST_MEDIA;
	bflow->fs_type = FS_TYPE_SANDBOX;

	ret = bootmeth_read_bootflow(bflow->method, bflow);
	if (ret)
		return log_msg_ret("method", ret);

	return 0;
}

struct bootdev_ops host_bootdev_ops = {
	.get_bootflow	= host_get_bootflow,
};

static const struct udevice_id host_bootdev_ids[] = {
	{ .compatible = "sandbox,bootdev-host" },
	{ }
};

U_BOOT_DRIVER(host_bootdev) = {
	.name		= "host_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &host_bootdev_ops,
	.of_match	= host_bootdev_ids,
};
