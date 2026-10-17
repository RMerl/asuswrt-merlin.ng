/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 BayLibre SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#ifndef _ARCH_MESON_USB_GX_H_
#define _ARCH_MESON_USB_GX_H_

#include <generic-phy.h>
#include <linux/usb/otg.h>

/* TOFIX add set_mode to struct phy_ops */
void phy_meson_gxl_usb2_set_mode(struct phy *phy, enum usb_dr_mode mode);

int dwc3_meson_gxl_force_mode(struct udevice *dev, enum usb_dr_mode mode);

#endif
