// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 */

#include <common.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <phy.h>

int board_eth_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_FMAN_ENET
	struct fsl_pq_mdio_info dtsec_mdio_info;

	printf("Initializing Fman\n");

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	/* DTESC1/2 don't have a PHY, they are temporarily disabled
	 * so that u-boot doesn't try to unsuccessfuly enable them */
	fm_disable_port(FM1_DTSEC1);
	fm_disable_port(FM1_DTSEC2);

	/*
	 * Program RGMII DTSEC5 (FM1 MAC5) on the EC2 physical itf
	 * This is the debug interface, the only one used in u-boot
	 */
	fm_info_set_phy_address(FM1_DTSEC5, CONFIG_SYS_FM1_DTSEC5_PHY_ADDR);
	fm_info_set_mdio(FM1_DTSEC5,
			 miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));

	ret = cpu_eth_init(bis);

	/* reenable DTSEC1/2 for later (kernel) */
	fm_enable_port(FM1_DTSEC1);
	fm_enable_port(FM1_DTSEC2);
#endif

	return ret;
}

#if defined(CONFIG_PHYLIB) && defined(CONFIG_PHY_MARVELL)

#define mv88E1118_PAGE_REG	22

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->addr == CONFIG_SYS_FM1_DTSEC5_PHY_ADDR) {
		/* driver config is good */
		if (phydev->drv->config)
			phydev->drv->config(phydev);

		/* but we still need to fix the LEDs */
		phy_write(phydev, MDIO_DEVAD_NONE, mv88E1118_PAGE_REG, 0x0003);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x0840);
		phy_write(phydev, MDIO_DEVAD_NONE, mv88E1118_PAGE_REG, 0x0000);
	}

	return 0;
}
#endif
