// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <linux/usb/ch9.h>

#define EXYNOS_G_DNL_THOR_VENDOR_NUM	0x04E8
#define EXYNOS_G_DNL_THOR_PRODUCT_NUM	0x685D

#define EXYNOS_G_DNL_UMS_VENDOR_NUM	0x0525
#define EXYNOS_G_DNL_UMS_PRODUCT_NUM	0xA4A5

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (!strcmp(name, "usb_dnl_thor")) {
		put_unaligned(EXYNOS_G_DNL_THOR_VENDOR_NUM, &dev->idVendor);
		put_unaligned(EXYNOS_G_DNL_THOR_PRODUCT_NUM, &dev->idProduct);
	} else if (!strcmp(name, "usb_dnl_ums")) {
		put_unaligned(EXYNOS_G_DNL_UMS_VENDOR_NUM, &dev->idVendor);
		put_unaligned(EXYNOS_G_DNL_UMS_PRODUCT_NUM, &dev->idProduct);
	} else {
		put_unaligned(CONFIG_USB_GADGET_VENDOR_NUM, &dev->idVendor);
		put_unaligned(CONFIG_USB_GADGET_PRODUCT_NUM, &dev->idProduct);
	}
	return 0;
}
