// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 * Copyright 2019-2021 Traverse Technologies Australia
 */
#include <common.h>
#include <command.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

void reset_phy(void)
{
	mc_env_boot();
}

int board_phy_config(struct phy_device *phydev)
{
	/* These settings only apply to VSC8514 */
	if (phydev->phy_id == 0x70670) {
		/* First, ensure LEDs are driven to rails (not tristate)
		 * This is in the extended page 0x0010
		 */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1F, 0x0010);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x0E, 0x2000);
		/* Restore to page 0 */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1F, 0x0000);

		/* Disable blink on the left LEDs, and make the activity LEDs blink faster */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1E, 0xC03);

		phy_write(phydev, MDIO_DEVAD_NONE, 0x1D, 0x3421);
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
