/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_ETH_H__
#define __MESON_ETH_H__

#include <phy.h>

enum {
	/* Use Internal RMII PHY */
	MESON_USE_INTERNAL_RMII_PHY = 1,
};

/* Configure the Ethernet MAC with the requested interface mode
 * with some optional flags.
 */
void meson_eth_init(phy_interface_t mode, unsigned int flags);

#endif /* __MESON_ETH_H__ */
