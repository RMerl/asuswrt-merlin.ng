// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <wdt.h>

struct wdt_reboot_priv {
	struct udevice *wdt;
};

static int wdt_reboot_request(struct udevice *dev, enum sysreset_t type)
{
	struct wdt_reboot_priv *priv = dev_get_priv(dev);
	int ret;

	ret = wdt_expire_now(priv->wdt, 0);
	if (ret)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops wdt_reboot_ops = {
	.request = wdt_reboot_request,
};

int wdt_reboot_probe(struct udevice *dev)
{
	struct wdt_reboot_priv *priv = dev_get_priv(dev);
	int err;

	err = uclass_get_device_by_phandle(UCLASS_WDT, dev,
					   "wdt", &priv->wdt);
	if (err) {
		pr_err("unable to find wdt device\n");
		return err;
	}

	return 0;
}

static const struct udevice_id wdt_reboot_ids[] = {
	{ .compatible = "wdt-reboot" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(wdt_reboot) = {
	.name = "wdt_reboot",
	.id = UCLASS_SYSRESET,
	.of_match = wdt_reboot_ids,
	.ops = &wdt_reboot_ops,
	.priv_auto_alloc_size = sizeof(struct wdt_reboot_priv),
	.probe = wdt_reboot_probe,
};
