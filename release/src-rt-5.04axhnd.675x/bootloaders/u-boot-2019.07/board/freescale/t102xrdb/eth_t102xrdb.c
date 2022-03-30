// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
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
#include "../common/fman.h"

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FMAN_ENET)
	int i, interface;
	struct memac_mdio_info dtsec_mdio_info;
	struct memac_mdio_info tgec_mdio_info;
	struct mii_dev *dev;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) &
					FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

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

	/* Set the on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY1_ADDR);

	switch (srds_s1) {
#ifdef CONFIG_TARGET_T1024RDB
	case 0x95:
		/* set the on-board RGMII2  PHY */
		fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY2_ADDR);

		/* set 10G XFI with Aquantia AQR105 PHY */
		fm_info_set_phy_address(FM1_10GEC1, FM1_10GEC1_PHY_ADDR);
		break;
#endif
	case 0x6a:
	case 0x6b:
	case 0x77:
	case 0x135:
		/* set the on-board 2.5G SGMII AQR105 PHY */
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_AQR_PHY_ADDR);
#ifdef CONFIG_TARGET_T1023RDB
		/* set the on-board 1G SGMII RTL8211F PHY */
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_RTK_PHY_ADDR);
#endif
		break;
	default:
		printf("SerDes protocol 0x%x is not supported on T102xRDB\n",
		       srds_s1);
		break;
	}

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_RGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		case PHY_INTERFACE_MODE_SGMII:
#if defined(CONFIG_TARGET_T1023RDB)
			dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
#elif defined(CONFIG_TARGET_T1024RDB)
			dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
#endif
			fm_info_set_mdio(i, dev);
			break;
		case PHY_INTERFACE_MODE_SGMII_2500:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
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

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}

void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
#if defined(CONFIG_TARGET_T1024RDB)
	if (((fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII_2500) ||
	     (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII)) &&
			(port == FM1_DTSEC3)) {
		fdt_set_phy_handle(fdt, compat, addr, "sg_2500_aqr105_phy4");
		fdt_setprop_string(fdt, offset, "phy-connection-type",
				   "sgmii-2500");
		fdt_status_disabled_by_alias(fdt, "xg_aqr105_phy3");
	}
#endif
}

void fdt_fixup_board_enet(void *fdt)
{
}
