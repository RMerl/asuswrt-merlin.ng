// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 BayLibre SAS
 */

#include <common.h>
#include <dm.h>
#include <net.h>

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	if (CONFIG_IS_ENABLED(USB_GADGET)) {
		ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, 0, &dev);
		if (ret) {
			pr_err("%s: Cannot find USB device\n", __func__);
			return ret;
		}
	}

	if (CONFIG_IS_ENABLED(USB_ETHER))
		usb_ether_init();

	return 0;
}
