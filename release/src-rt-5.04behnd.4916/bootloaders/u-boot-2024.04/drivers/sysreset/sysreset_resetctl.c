// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <reset.h>

struct resetctl_reboot_priv {
	struct reset_ctl_bulk resets;
};

static int resetctl_reboot_request(struct udevice *dev, enum sysreset_t type)
{
	struct resetctl_reboot_priv *priv = dev_get_priv(dev);

	return reset_assert_bulk(&priv->resets);
}

static struct sysreset_ops resetctl_reboot_ops = {
	.request = resetctl_reboot_request,
};

static int resetctl_reboot_probe(struct udevice *dev)
{
	struct resetctl_reboot_priv *priv = dev_get_priv(dev);

	return reset_get_bulk(dev, &priv->resets);
}

static const struct udevice_id resetctl_reboot_ids[] = {
	{ .compatible = "resetctl-reboot" },
	{ }
};

U_BOOT_DRIVER(resetctl_reboot) = {
	.id = UCLASS_SYSRESET,
	.name = "resetctl_reboot",
	.of_match = resetctl_reboot_ids,
	.ops = &resetctl_reboot_ops,
	.priv_auto	= sizeof(struct resetctl_reboot_priv),
	.probe = resetctl_reboot_probe,
};
