/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_ETH_H__
#define __MESON_ETH_H__

/* Generate an unique MAC address based on the HW serial */
int meson_generate_serial_ethaddr(void);

#endif /* __MESON_ETH_H__ */
