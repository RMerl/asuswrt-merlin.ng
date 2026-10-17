// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2021 Linaro, Rui Miguel Silva <rui.silva@linaro.org>
 *
 * based on original code from:
 * (c) 2007 Sebastian Siewior <bigeasy@linutronix.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/usb/otg.h>
#include <log.h>
#include <usb.h>

#include "isp1760-core.h"
#include "isp1760-regs.h"
#include "isp1760-uboot.h"

static int isp1760_of_to_plat(struct udevice *dev)
{
	struct isp1760_device *isp = dev_get_plat(dev);
	unsigned int devflags = 0;
	u32 bus_width = 0;
	ofnode dp;

	if (!dev_has_ofnode(dev)) {
		/* select isp1763 as the default device */
		devflags = ISP1760_FLAG_ISP1763 | ISP1760_FLAG_BUS_WIDTH_16;
		pr_err("isp1760: no platform data\n");
		goto isp_setup;
	}

	dp = dev_ofnode(dev);

	if (ofnode_device_is_compatible(dp, "nxp,usb-isp1761"))
		devflags |= ISP1760_FLAG_ISP1761;

	if (ofnode_device_is_compatible(dp, "nxp,usb-isp1763"))
		devflags |= ISP1760_FLAG_ISP1763;

	/*
	 * Some systems wire up only 8 of 16 data lines or
	 * 16 of the 32 data lines
	 */
	bus_width = ofnode_read_u32_default(dp, "bus-width", 16);
	if (bus_width == 16)
		devflags |= ISP1760_FLAG_BUS_WIDTH_16;
	else if (bus_width == 8)
		devflags |= ISP1760_FLAG_BUS_WIDTH_8;

	if (usb_get_dr_mode(dev_ofnode(dev)) == USB_DR_MODE_PERIPHERAL)
		devflags |= ISP1760_FLAG_PERIPHERAL_EN;

	if (ofnode_read_bool(dp, "analog-oc"))
		devflags |= ISP1760_FLAG_ANALOG_OC;

	if (ofnode_read_bool(dp, "dack-polarity"))
		devflags |= ISP1760_FLAG_DACK_POL_HIGH;

	if (ofnode_read_bool(dp, "dreq-polarity"))
		devflags |= ISP1760_FLAG_DREQ_POL_HIGH;

isp_setup:
	isp->devflags = devflags;
	isp->dev = dev;

	return 0;
}

static int isp1760_plat_probe(struct udevice *dev)
{
	struct isp1760_device *isp = dev_get_plat(dev);
	struct resource mem_res;
	struct resource irq_res;
	int ret;

	dev_read_resource(dev, 0, &mem_res);
	dev_read_resource(dev, 1, &irq_res);

	isp1760_init_kmem_once();

	ret = isp1760_register(isp, &mem_res, irq_res.start, irq_res.flags);
	if (ret < 0) {
		isp1760_deinit_kmem_cache();
		return ret;
	}

	return 0;
}

static int isp1760_plat_remove(struct udevice *dev)
{
	struct isp1760_device *isp = dev_get_plat(dev);

	isp1760_deinit_kmem_cache();
	isp1760_unregister(isp);

	return 0;
}

static const struct udevice_id isp1760_ids[] = {
	{ .compatible = "nxp,usb-isp1760", },
	{ .compatible = "nxp,usb-isp1761", },
	{ .compatible = "nxp,usb-isp1763", },
	{ },
};

U_BOOT_DRIVER(isp1760) = {
	.name		= "isp1760",
	.id		= UCLASS_USB,
	.of_match	= isp1760_ids,
	.of_to_plat	= isp1760_of_to_plat,
	.ops		= &isp1760_usb_ops,
	.probe		= isp1760_plat_probe,
	.remove		= isp1760_plat_remove,
	.plat_auto	= sizeof(struct isp1760_device),
	.priv_auto	= sizeof(struct isp1760_host_data),
};
