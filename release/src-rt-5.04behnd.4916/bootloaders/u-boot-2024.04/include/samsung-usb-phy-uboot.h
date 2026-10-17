/* SPDX-License-Identifier: GPL-2.0+ */
/* include/samsung-usb-phy-uboot.h
 *
 * Copyright (c) 2015 Samsung Electronics
 *
 * USB3 (DWC3) PHY uboot init
 */

#ifndef __SAMSUNG_USB_PHY_UBOOT_H_
#define __SAMSUNG_USB_PHY_UBOOT_H_

#include <asm/arch/xhci-exynos.h>

void exynos5_usb3_phy_init(struct exynos_usb3_phy *phy);
#endif /* __SAMSUNG_USB_PHY_UBOOT_H_ */
