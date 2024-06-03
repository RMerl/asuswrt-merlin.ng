/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_USB_H__
#define __MESON_USB_H__

int dwc3_meson_g12a_force_mode(struct udevice *dev, enum usb_dr_mode mode);

#endif /* __MESON_USB_H__ */
