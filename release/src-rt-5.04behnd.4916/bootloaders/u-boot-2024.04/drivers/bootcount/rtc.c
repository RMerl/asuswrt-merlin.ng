// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <bootcount.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>

static const u8 bootcount_magic = 0xbc;

struct bootcount_rtc_priv {
	struct udevice *rtc;
	u32 offset;
};

static int bootcount_rtc_set(struct udevice *dev, const u32 a)
{
	struct bootcount_rtc_priv *priv = dev_get_priv(dev);
	const u16 val = bootcount_magic << 8 | (a & 0xff);

	if (rtc_write16(priv->rtc, priv->offset, val) < 0) {
		debug("%s: rtc_write16 failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int bootcount_rtc_get(struct udevice *dev, u32 *a)
{
	struct bootcount_rtc_priv *priv = dev_get_priv(dev);
	u16 val;

	if (rtc_read16(priv->rtc, priv->offset, &val) < 0) {
		debug("%s: rtc_write16 failed\n", __func__);
		return -EIO;
	}

	if (val >> 8 == bootcount_magic) {
		*a = val & 0xff;
		return 0;
	}

	debug("%s: bootcount magic does not match on %04x\n", __func__, val);
	return -EIO;
}

static int bootcount_rtc_probe(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	struct bootcount_rtc_priv *priv = dev_get_priv(dev);
	struct udevice *rtc;

	if (dev_read_phandle_with_args(dev, "rtc", NULL, 0, 0, &phandle_args)) {
		debug("%s: rtc backing device not specified\n", dev->name);
		return -ENOENT;
	}

	if (uclass_get_device_by_ofnode(UCLASS_RTC, phandle_args.node, &rtc)) {
		debug("%s: could not get backing device\n", dev->name);
		return -ENODEV;
	}

	priv->rtc = rtc;
	priv->offset = dev_read_u32_default(dev, "offset", 0);

	return 0;
}

static const struct bootcount_ops bootcount_rtc_ops = {
	.get = bootcount_rtc_get,
	.set = bootcount_rtc_set,
};

static const struct udevice_id bootcount_rtc_ids[] = {
	{ .compatible = "u-boot,bootcount-rtc" },
	{ }
};

U_BOOT_DRIVER(bootcount_rtc) = {
	.name	= "bootcount-rtc",
	.id	= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_rtc_priv),
	.probe	= bootcount_rtc_probe,
	.of_match = bootcount_rtc_ids,
	.ops	= &bootcount_rtc_ops,
};
