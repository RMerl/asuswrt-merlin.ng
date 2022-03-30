/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _JR2_DATA_H_
#define _JR2_DATA_H_

#define SERDES1G(x)     (x)
#define SERDES1G_MAX    SERDES1G(10)
#define SERDES6G(x)     (SERDES1G_MAX + 1 + (x))
#define SERDES6G_MAX    SERDES6G(17)
#define SERDES_MAX      (SERDES6G_MAX + 1)

/* similar with phy_interface_t */
#define PHY_MODE_SGMII  2
#define PHY_MODE_QSGMII 4

#endif
