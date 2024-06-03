// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Chunhe Lan <Chunhe.Lan@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <asm/fsl_serdes.h>
#include <hwconfig.h>

#include "../common/fman.h"
#include "t4rdb.h"

void fdt_fixup_board_enet(void *fdt)
{
	return;
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FMAN_ENET)
	int i, interface;
	struct memac_mdio_info dtsec_mdio_info;
	struct memac_mdio_info tgec_mdio_info;
	struct mii_dev *dev;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1, srds_prtcl_s2;

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	srds_prtcl_s2 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	srds_prtcl_s2 >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;

	dtsec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM2_DTSEC_MDIO_ADDR;

	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fm_memac_mdio_init(bis, &dtsec_mdio_info);

	tgec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM2_TGEC_MDIO_ADDR;
	tgec_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the 10G MDIO bus */
	fm_memac_mdio_init(bis, &tgec_mdio_info);

	if ((srds_prtcl_s1 == 28) || (srds_prtcl_s1 == 27)) {
		/* SGMII */
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_PHY_ADDR1);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_PHY_ADDR2);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_PHY_ADDR3);
		fm_info_set_phy_address(FM1_DTSEC4, SGMII_PHY_ADDR4);
	} else {
		puts("Invalid SerDes1 protocol for T4240RDB\n");
	}

	fm_disable_port(FM1_DTSEC5);
	fm_disable_port(FM1_DTSEC6);

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_SGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

	for (i = FM1_10GEC1; i < FM1_10GEC1 + CONFIG_SYS_NUM_FM1_10GEC; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

#if (CONFIG_SYS_NUM_FMAN == 2)
	if ((srds_prtcl_s2 == 56) || (srds_prtcl_s2 == 55)) {
		/* SGMII && XFI */
		fm_info_set_phy_address(FM2_DTSEC1, SGMII_PHY_ADDR5);
		fm_info_set_phy_address(FM2_DTSEC2, SGMII_PHY_ADDR6);
		fm_info_set_phy_address(FM2_DTSEC3, SGMII_PHY_ADDR7);
		fm_info_set_phy_address(FM2_DTSEC4, SGMII_PHY_ADDR8);
		fm_info_set_phy_address(FM1_10GEC1, FM1_10GEC1_PHY_ADDR);
		fm_info_set_phy_address(FM1_10GEC2, FM1_10GEC2_PHY_ADDR);
		fm_info_set_phy_address(FM2_10GEC1, FM2_10GEC2_PHY_ADDR);
		fm_info_set_phy_address(FM2_10GEC2, FM2_10GEC1_PHY_ADDR);
	} else {
		puts("Invalid SerDes2 protocol for T4240RDB\n");
	}

	fm_disable_port(FM2_DTSEC5);
	fm_disable_port(FM2_DTSEC6);
	for (i = FM2_DTSEC1; i < FM2_DTSEC1 + CONFIG_SYS_NUM_FM2_DTSEC; i++) {
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_SGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

	for (i = FM2_10GEC1; i < FM2_10GEC1 + CONFIG_SYS_NUM_FM2_10GEC; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}
#endif /* CONFIG_SYS_NUM_FMAN */

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}
