// SPDX-License-Identifier: GPL-2.0
/*
 * TI Generic PHY Init to register any TI Ethernet PHYs
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Copyright (C) 2019-20 Texas Instruments Inc.
 */

#include <phy.h>
#include "ti_phy_init.h"

U_BOOT_PHY_DRIVER(dp83822) = {
	.name = "TI DP83822",
	.uid = 0x2000a240,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83826nc) = {
	.name = "TI DP83826NC",
	.uid = 0x2000a110,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83826c) = {
	.name = "TI DP83826C",
	.uid = 0x2000a130,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825s) = {
	.name = "TI DP83825S",
	.uid = 0x2000a140,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825i) = {
	.name = "TI DP83825I",
	.uid = 0x2000a150,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825m) = {
	.name = "TI DP83825M",
	.uid = 0x2000a160,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825cs) = {
	.name = "TI DP83825CS",
	.uid = 0x2000a170,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};
