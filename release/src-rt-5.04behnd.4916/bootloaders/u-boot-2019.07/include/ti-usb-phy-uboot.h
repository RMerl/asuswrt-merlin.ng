/* SPDX-License-Identifier: GPL-2.0+ */
/* include/ti_usb_phy_uboot.h
 *
 * Copyright (c) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * USB2 and USB3 PHY uboot init
 */

#ifndef __TI_USB_PHY_UBOOT_H_
#define __TI_USB_PHY_UBOOT_H_

struct ti_usb_phy_device {
	void *pll_ctrl_base;
	void *usb2_phy_power;
	void *usb3_phy_power;
	int index;
};

int ti_usb_phy_uboot_init(struct ti_usb_phy_device *dev);
void ti_usb_phy_uboot_exit(int index);
#endif /* __TI_USB_PHY_UBOOT_H_ */
