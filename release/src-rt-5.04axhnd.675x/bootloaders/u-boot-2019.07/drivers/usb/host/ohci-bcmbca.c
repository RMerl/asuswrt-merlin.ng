// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <dm/ofnode.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/ioport.h>
#include <linux/io.h>

#include "ohci.h"

struct bcmbca_ohci {
	ohci_t ohci;
};

static int ohci_usb_probe (struct udevice *dev)
{
	struct ohci_regs *regs;
	int ret;
	struct resource res;

	ret = dev_read_resource_byname (dev, "usb-ohci", &res);
	if (ret)
	{
		dev_err(dev, "can't get usb-ohci register for usb (ret=%d)\n", ret);
		return ret;
	}

	regs = devm_ioremap (dev, res.start, resource_size(&res));
	debug("bcmbca ohci device regs %p\n", regs);
	
	ohci_register(dev, regs);

	return 0;
}

static int ohci_usb_remove(struct udevice *dev)
{
	int ret;
	struct udevice *next_devp = dev;
	struct udevice *ctrl_dev;

	debug("remove bcmbca-ohci %p seq %d\n", dev, dev->seq);

	ret = ohci_deregister(dev);

	if (ret)
	{
		printf("failed to deregister bcmbca ohci seq %d\n", dev->seq);
		return ret;
	}

	ret = uclass_find_next_device (&next_devp);
	if (!ret && !next_devp)
	{
		uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(ctrl_bcmbca_drv), &ctrl_dev);
		if (ctrl_dev)
			device_remove(ctrl_dev, DM_REMOVE_NORMAL);
	}
	return ret;
}


static const struct udevice_id ohci_usb_ids[] = {
	{ .compatible = "brcm,bcmbca-ohci" },
	{ }
};

U_BOOT_DRIVER(ohci_bcmbca) = {
	.name	= "ohci-bcmbca",
	.id	= UCLASS_USB,
	.of_match = ohci_usb_ids,
	.probe = ohci_usb_probe,
	.remove = ohci_usb_remove,
	.ops	= &ohci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct bcmbca_ohci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
