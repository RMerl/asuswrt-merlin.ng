// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <malloc.h>
#include <sysreset.h>
#include <wdt.h>
#include <linux/printk.h>

struct wdt_reboot_plat {
	struct udevice *wdt;
};

static int wdt_reboot_request(struct udevice *dev, enum sysreset_t type)
{
	struct wdt_reboot_plat *plat = dev_get_plat(dev);
	int ret;

	switch (type) {
	case SYSRESET_COLD:
	case SYSRESET_WARM:
#if !defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BUILD)
		ret = wdt_expire_now(plat->wdt, 0);
		if (ret)
			return ret;
#else
		printf("Please revisit: wdt_expire_now %s:%d\n", __FILE__, __LINE__);
		plat=NULL;
		ret=-EPROTONOSUPPORT;
		return ret;
#endif
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops wdt_reboot_ops = {
	.request = wdt_reboot_request,
};

static int wdt_reboot_of_to_plat(struct udevice *dev)
{
	struct wdt_reboot_plat *plat = dev_get_plat(dev);
	int err;

	err = uclass_get_device_by_phandle(UCLASS_WDT, dev,
					   "wdt", &plat->wdt);
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
	.of_to_plat	= wdt_reboot_of_to_plat,
	.plat_auto	= sizeof(struct wdt_reboot_plat),
	.ops = &wdt_reboot_ops,
};

#if IS_ENABLED(CONFIG_SYSRESET_WATCHDOG_AUTO)
int sysreset_register_wdt(struct udevice *dev)
{
	struct wdt_reboot_plat *plat = malloc(sizeof(*plat));
	int ret;

	if (!plat)
		return -ENOMEM;

	plat->wdt = dev;

	ret = device_bind(dev, DM_DRIVER_GET(wdt_reboot),
			  dev->name, plat, ofnode_null(), NULL);
	if (ret) {
		free(plat);
		return ret;
	}

	return 0;
}
#endif
