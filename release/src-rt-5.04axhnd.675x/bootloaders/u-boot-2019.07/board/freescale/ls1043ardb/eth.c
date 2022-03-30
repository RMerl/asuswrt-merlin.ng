// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>

#include "../common/fman.h"

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	int i;
	struct memac_mdio_info dtsec_mdio_info;
	struct memac_mdio_info tgec_mdio_info;
	struct mii_dev *dev;
	u32 srds_s1;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	dtsec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;

	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fm_memac_mdio_init(bis, &dtsec_mdio_info);

	tgec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	tgec_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the 10G MDIO bus */
	fm_memac_mdio_init(bis, &tgec_mdio_info);

	/* Set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);

	/* QSGMII on lane B, MAC 1/2/5/6 */
	fm_info_set_phy_address(FM1_DTSEC1, QSGMII_PORT1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, QSGMII_PORT2_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC5, QSGMII_PORT3_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC6, QSGMII_PORT4_PHY_ADDR);

	switch (srds_s1) {
	case 0x1455:
		break;
	default:
		printf("Invalid SerDes protocol 0x%x for LS1043ARDB\n",
		       srds_s1);
		break;
	}

	dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++)
		fm_info_set_mdio(i, dev);

	/* XFI on lane A, MAC 9 */
	fm_info_set_phy_address(FM1_10GEC1, FM1_10GEC1_PHY_ADDR);
	dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
	fm_info_set_mdio(FM1_10GEC1, dev);

	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
