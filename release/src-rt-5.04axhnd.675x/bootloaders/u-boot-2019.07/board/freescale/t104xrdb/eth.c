// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_dtsec.h>
#include <vsc9953.h>

#include "../common/fman.h"

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct memac_mdio_info memac_mdio_info;
	unsigned int i;
	int phy_addr = 0;
#ifdef CONFIG_VSC9953
	phy_interface_t phy_int;
	struct mii_dev *bus;
#endif

	printf("Initializing Fman\n");

	memac_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;
	memac_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fm_memac_mdio_init(bis, &memac_mdio_info);

	/*
	 * Program on board RGMII, SGMII PHY addresses.
	 */
	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {
#if defined(CONFIG_TARGET_T1040RDB) || defined(CONFIG_TARGET_T1040D4RDB)
		case PHY_INTERFACE_MODE_SGMII:
			/* T1040RDB & T1040D4RDB only supports SGMII on
			 * DTSEC3
			 */
			fm_info_set_phy_address(FM1_DTSEC3,
						CONFIG_SYS_SGMII1_PHY_ADDR);
			break;
#endif
#ifdef CONFIG_TARGET_T1042RDB
		case PHY_INTERFACE_MODE_SGMII:
			/* T1042RDB doesn't supports SGMII on DTSEC1 & DTSEC2 */
			if ((FM1_DTSEC1 == i) || (FM1_DTSEC2 == i))
				fm_info_set_phy_address(i, 0);
			/* T1042RDB only supports SGMII on DTSEC3 */
			fm_info_set_phy_address(FM1_DTSEC3,
						CONFIG_SYS_SGMII1_PHY_ADDR);
			break;
#endif
#ifdef CONFIG_TARGET_T1042D4RDB
		case PHY_INTERFACE_MODE_SGMII:
			/* T1042D4RDB supports SGMII on DTSEC1, DTSEC2
			 *  & DTSEC3
			 */
			if (FM1_DTSEC1 == i)
				phy_addr = CONFIG_SYS_SGMII1_PHY_ADDR;
			if (FM1_DTSEC2 == i)
				phy_addr = CONFIG_SYS_SGMII2_PHY_ADDR;
			if (FM1_DTSEC3 == i)
				phy_addr = CONFIG_SYS_SGMII3_PHY_ADDR;
			fm_info_set_phy_address(i, phy_addr);
			break;
#endif
		case PHY_INTERFACE_MODE_RGMII:
			if (FM1_DTSEC4 == i)
				phy_addr = CONFIG_SYS_RGMII1_PHY_ADDR;
			if (FM1_DTSEC5 == i)
				phy_addr = CONFIG_SYS_RGMII2_PHY_ADDR;
			fm_info_set_phy_address(i, phy_addr);
			break;
		case PHY_INTERFACE_MODE_QSGMII:
			fm_info_set_phy_address(i, 0);
			break;
		case PHY_INTERFACE_MODE_NONE:
			fm_info_set_phy_address(i, 0);
			break;
		default:
			printf("Fman1: DTSEC%u set to unknown interface %i\n",
			       idx + 1, fm_info_get_enet_if(i));
			fm_info_set_phy_address(i, 0);
			break;
		}
		if (fm_info_get_enet_if(i) == PHY_INTERFACE_MODE_QSGMII ||
		    fm_info_get_enet_if(i) == PHY_INTERFACE_MODE_NONE)
			fm_info_set_mdio(i, NULL);
		else
			fm_info_set_mdio(i,
					 miiphy_get_dev_by_name(
							DEFAULT_FM_MDIO_NAME));
	}

#ifdef CONFIG_VSC9953
	/* SerDes configured for QSGMII */
	if (serdes_get_first_lane(FSL_SRDS_1, QSGMII_SW1_A) >= 0) {
		for (i = 0; i < 4; i++) {
			bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			phy_addr = CONFIG_SYS_FM1_QSGMII11_PHY_ADDR + i;
			phy_int = PHY_INTERFACE_MODE_QSGMII;

			vsc9953_port_info_set_mdio(i, bus);
			vsc9953_port_info_set_phy_address(i, phy_addr);
			vsc9953_port_info_set_phy_int(i, phy_int);
			vsc9953_port_enable(i);
		}
	}
	if (serdes_get_first_lane(FSL_SRDS_1, QSGMII_SW1_B) >= 0) {
		for (i = 4; i < 8; i++) {
			bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			phy_addr = CONFIG_SYS_FM1_QSGMII21_PHY_ADDR + i - 4;
			phy_int = PHY_INTERFACE_MODE_QSGMII;

			vsc9953_port_info_set_mdio(i, bus);
			vsc9953_port_info_set_phy_address(i, phy_addr);
			vsc9953_port_info_set_phy_int(i, phy_int);
			vsc9953_port_enable(i);
		}
	}

	/* Connect DTSEC1 to L2 switch if it doesn't have a PHY */
	if (serdes_get_first_lane(FSL_SRDS_1, SGMII_FM1_DTSEC1) < 0)
		vsc9953_port_enable(8);

	/* Connect DTSEC2 to L2 switch if it doesn't have a PHY */
	if (serdes_get_first_lane(FSL_SRDS_1, SGMII_FM1_DTSEC2) < 0) {
		/* Enable L2 On MAC2 using SCFG */
		struct ccsr_scfg *scfg = (struct ccsr_scfg *)
				CONFIG_SYS_MPC85xx_SCFG;

		out_be32(&scfg->esgmiiselcr, in_be32(&scfg->esgmiiselcr) |
			 (0x80000000));
		vsc9953_port_enable(9);
	}
#endif

	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
