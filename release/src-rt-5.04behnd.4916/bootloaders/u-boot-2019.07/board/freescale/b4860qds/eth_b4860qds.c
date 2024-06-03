// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 * Author: Sandeep Kumar Singh <sandeep@freescale.com>
 */

/* This file is based on board/freescale/corenet_ds/eth_superhydra.c */

/*
 * This file handles the board muxing between the Fman Ethernet MACs and
 * the RGMII/SGMII/XGMII PHYs on a Freescale B4860 "Centaur". The SGMII
 * PHYs are the two on-board 1Gb ports. There are no RGMII PHY on board.
 * The 10Gb XGMII PHY is provided via the XAUI riser card. There is only
 * one Fman device on B4860. The SERDES configuration is used to determine
 * where the SGMII and XAUI cards exist, and also which Fman MACs are routed
 * to which PHYs. So for a given Fman MAC, there is one and only PHY it
 * connects to. MACs cannot be routed to PHYs dynamically. This configuration
 * is done at boot time by reading SERDES protocol from RCW.
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fdt_support.h>
#include <fsl_dtsec.h>

#include "../common/ngpixis.h"
#include "../common/fman.h"
#include "../common/qixis.h"
#include "b4860qds_qixis.h"

#define EMI_NONE       0xFFFFFFFF

#ifdef CONFIG_FMAN_ENET

/*
 * Mapping of all 16 SERDES lanes to board slots. A value n(>0) will mean that
 * lane at index is mapped to slot number n. A value of '0' will mean
 * that the mapping must be determined dynamically, or that the lane maps to
 * something other than a board slot
 */
static u8 lane_to_slot[] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	1, 1, 1, 1,
	0, 0, 0, 0
};

/*
 * This function initializes the lane_to_slot[] array. It reads RCW to check
 * if Serdes2{E,F,G,H} is configured as slot 2 or as SFP and initializes
 * lane_to_slot[] accordingly
 */
static void initialize_lane_to_slot(void)
{
	unsigned int  serdes2_prtcl;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
		FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	debug("Initializing lane to slot: Serdes2 protocol: %x\n",
			serdes2_prtcl);

	switch (serdes2_prtcl) {
	case 0x17:
	case 0x18:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B,C,D: SGMII
		 * Lanes: E,F: Aur
		 * Lanes: G,H: SRIO
		 */
	case 0x91:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B: SGMII
		 * Lanes: C,D: SRIO2
		 * Lanes: E,F,G,H: XAUI2
		 */
	case 0x93:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B,C,D: SGMII
		 * Lanes: E,F,G,H: XAUI2
		 */
	case 0x98:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B,C,D: XAUI2
		 * Lanes: E,F,G,H: XAUI2
		 */
	case 0x9a:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B: PCI
		 * Lanes: C,D: SGMII
		 * Lanes: E,F,G,H: XAUI2
		 */
	case 0x9e:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B,C,D: PCI
		 * Lanes: E,F,G,H: XAUI2
		 */
	case 0xb1:
	case 0xb2:
	case 0x8c:
	case 0x8d:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B,C,D: PCI
		 * Lanes: E,F: SGMII 3&4
		 * Lanes: G,H: XFI
		 */
	case 0xc2:
		/*
		 * Configuration:
		 * SERDES: 2
		 * Lanes: A,B: SGMII
		 * Lanes: C,D: SRIO2
		 * Lanes: E,F,G,H: XAUI2
		 */
		lane_to_slot[12] = 2;
		lane_to_slot[13] = lane_to_slot[12];
		lane_to_slot[14] = lane_to_slot[12];
		lane_to_slot[15] = lane_to_slot[12];
		break;

	default:
		printf("Fman: Unsupported SerDes2 Protocol 0x%02x\n",
				serdes2_prtcl);
			break;
	}
	return;
}

#endif /* #ifdef CONFIG_FMAN_ENET */

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct memac_mdio_info memac_mdio_info;
	struct memac_mdio_info tg_memac_mdio_info;
	unsigned int i;
	unsigned int  serdes1_prtcl, serdes2_prtcl;
	int qsgmii;
	struct mii_dev *bus;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes1_prtcl = in_be32(&gur->rcwsr[4]) &
		FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	if (!serdes1_prtcl) {
		printf("SERDES1 is not enabled\n");
		return 0;
	}
	serdes1_prtcl >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	debug("Using SERDES1 Protocol: 0x%x:\n", serdes1_prtcl);

	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
		FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	if (!serdes2_prtcl) {
		printf("SERDES2 is not enabled\n");
		return 0;
	}
	serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	debug("Using SERDES2 Protocol: 0x%x:\n", serdes2_prtcl);

	printf("Initializing Fman\n");

	initialize_lane_to_slot();

	memac_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;
	memac_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fm_memac_mdio_init(bis, &memac_mdio_info);

	tg_memac_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	tg_memac_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the real 10G MDIO bus */
	fm_memac_mdio_init(bis, &tg_memac_mdio_info);

	/*
	 * Program the two on board DTSEC PHY addresses assuming that they are
	 * all SGMII. RGMII is not supported on this board. Setting SGMII 5 and
	 * 6 to on board SGMII phys
	 */
	fm_info_set_phy_address(FM1_DTSEC5, CONFIG_SYS_FM1_ONBOARD_PHY1_ADDR);
	fm_info_set_phy_address(FM1_DTSEC6, CONFIG_SYS_FM1_ONBOARD_PHY2_ADDR);

	switch (serdes1_prtcl) {
	case 0x29:
	case 0x2a:
		/* Serdes 1: A-B SGMII, Configuring DTSEC 5 and 6 */
		debug("Set phy addresses for FM1_DTSEC5:%x, FM1_DTSEC6:%x\n",
		      CONFIG_SYS_FM1_ONBOARD_PHY1_ADDR,
		      CONFIG_SYS_FM1_ONBOARD_PHY2_ADDR);
		fm_info_set_phy_address(FM1_DTSEC5,
				CONFIG_SYS_FM1_ONBOARD_PHY1_ADDR);
		fm_info_set_phy_address(FM1_DTSEC6,
				CONFIG_SYS_FM1_ONBOARD_PHY2_ADDR);
		break;
#ifdef CONFIG_ARCH_B4420
	case 0x17:
	case 0x18:
		/* Serdes 1: A-D SGMII, Configuring on board dual SGMII Phy */
		debug("Set phy addresses for FM1_DTSEC3:%x, FM1_DTSEC4:%x\n",
		      CONFIG_SYS_FM1_ONBOARD_PHY1_ADDR,
		      CONFIG_SYS_FM1_ONBOARD_PHY2_ADDR);
		/* Fixing Serdes clock by programming FPGA register */
		QIXIS_WRITE(brdcfg[4], QIXIS_SRDS1CLK_125);
		fm_info_set_phy_address(FM1_DTSEC3,
				CONFIG_SYS_FM1_ONBOARD_PHY1_ADDR);
		fm_info_set_phy_address(FM1_DTSEC4,
				CONFIG_SYS_FM1_ONBOARD_PHY2_ADDR);
		break;
#endif
	default:
		printf("Fman:  Unsupported SerDes1 Protocol 0x%02x\n",
				serdes1_prtcl);
		break;
	}
	switch (serdes2_prtcl) {
	case 0x17:
	case 0x18:
		debug("Set phy address on SGMII Riser for FM1_DTSEC1:%x\n",
		      CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC1,
				CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC2,
				CONFIG_SYS_FM1_DTSEC2_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3,
				CONFIG_SYS_FM1_DTSEC3_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC4,
				CONFIG_SYS_FM1_DTSEC4_RISER_PHY_ADDR);
		break;
	case 0x48:
	case 0x49:
		debug("Set phy address on SGMII Riser for FM1_DTSEC1:%x\n",
		      CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC1,
				CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC2,
				CONFIG_SYS_FM1_DTSEC2_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3,
				CONFIG_SYS_FM1_DTSEC3_RISER_PHY_ADDR);
		break;
	case 0xb1:
	case 0xb2:
	case 0x8c:
	case 0x8d:
		debug("Set phy addresses on SGMII Riser for FM1_DTSEC1:%x\n",
		      CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3,
				CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC4,
				CONFIG_SYS_FM1_DTSEC2_RISER_PHY_ADDR);
		/*
		 * XFI does not need a PHY to work, but to make U-Boot
		 * happy, assign a fake PHY address for a XFI port.
		 */
		fm_info_set_phy_address(FM1_10GEC1, 0);
		fm_info_set_phy_address(FM1_10GEC2, 1);
		break;
	case 0x98:
		/* XAUI in Slot1 and Slot2 */
		debug("Set phy address of AMC2PEX-2S for FM1_10GEC1:%x\n",
		      CONFIG_SYS_FM1_10GEC1_PHY_ADDR);
		fm_info_set_phy_address(FM1_10GEC1,
					CONFIG_SYS_FM1_10GEC1_PHY_ADDR);
		debug("Set phy address of AMC2PEX-2S for FM1_10GEC2:%x\n",
		      CONFIG_SYS_FM1_10GEC2_PHY_ADDR);
		fm_info_set_phy_address(FM1_10GEC2,
					CONFIG_SYS_FM1_10GEC2_PHY_ADDR);
		break;
	case 0x9E:
		/* XAUI in Slot2 */
		debug("Sett phy address of AMC2PEX-2S for FM1_10GEC2:%x\n",
		      CONFIG_SYS_FM1_10GEC2_PHY_ADDR);
		fm_info_set_phy_address(FM1_10GEC2,
					CONFIG_SYS_FM1_10GEC2_PHY_ADDR);
		break;
	default:
		printf("Fman:  Unsupported SerDes2 Protocol 0x%02x\n",
				serdes2_prtcl);
		break;
	}

	/*set PHY address for QSGMII Riser Card on slot2*/
	bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
	qsgmii = is_qsgmii_riser_card(bus, PHY_BASE_ADDR, PORT_NUM, REGNUM);

	if (qsgmii) {
		switch (serdes2_prtcl) {
		case 0xb2:
		case 0x8d:
			fm_info_set_phy_address(FM1_DTSEC3, PHY_BASE_ADDR);
			fm_info_set_phy_address(FM1_DTSEC4, PHY_BASE_ADDR + 1);
			break;
		default:
			break;
		}
	}

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			fm_info_set_mdio(i,
				miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));
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
	}

	for (i = FM1_10GEC1; i < FM1_10GEC1 + CONFIG_SYS_NUM_FM1_10GEC; i++) {
		int idx = i - FM1_10GEC1;

		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			fm_info_set_mdio(i,
					 miiphy_get_dev_by_name
					 (DEFAULT_FM_TGEC_MDIO_NAME));
			break;
		case PHY_INTERFACE_MODE_NONE:
			fm_info_set_phy_address(i, 0);
			break;
		default:
			printf("Fman1: TGEC%u set to unknown interface %i\n",
			       idx + 1, fm_info_get_enet_if(i));
			fm_info_set_phy_address(i, 0);
			break;
		}
	}

	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}

void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	int phy;
	char alias[32];
	struct fixed_link f_link;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 prtcl2 = in_be32(&gur->rcwsr[4]) & FSL_CORENET2_RCWSR4_SRDS2_PRTCL;

	prtcl2 >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;

	if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII) {
		phy = fm_info_get_phy_address(port);

		sprintf(alias, "phy_sgmii_%x", phy);
		fdt_set_phy_handle(fdt, compat, addr, alias);
		fdt_status_okay_by_alias(fdt, alias);
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_XGMII) {
		/* check if it's XFI interface for 10g */
		switch (prtcl2) {
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0xb1:
		case 0xb2:
			f_link.phy_id = port;
			f_link.duplex = 1;
			f_link.link_speed = 10000;
			f_link.pause = 0;
			f_link.asym_pause = 0;

			fdt_delprop(fdt, offset, "phy-handle");
			fdt_setprop(fdt, offset, "fixed-link", &f_link,
				    sizeof(f_link));
			break;
		case 0x98: /* XAUI interface */
			strcpy(alias, "phy_xaui_slot1");
			fdt_status_okay_by_alias(fdt, alias);

			strcpy(alias, "phy_xaui_slot2");
			fdt_status_okay_by_alias(fdt, alias);
			break;
		case 0x9e: /* XAUI interface */
		case 0x9a:
		case 0x93:
		case 0x91:
			strcpy(alias, "phy_xaui_slot1");
			fdt_status_okay_by_alias(fdt, alias);
			break;
		case 0x97: /* XAUI interface */
		case 0xc3:
			strcpy(alias, "phy_xaui_slot2");
			fdt_status_okay_by_alias(fdt, alias);
			break;
		default:
			break;
		}
	}
}

/*
 * Set status to disabled for unused ethernet node
 */
void fdt_fixup_board_enet(void *fdt)
{
	int i;
	char alias[32];

	for (i = FM1_DTSEC1; i <= FM1_10GEC2; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_NONE:
			sprintf(alias, "ethernet%u", i);
			fdt_status_disabled_by_alias(fdt, alias);
			break;
		default:
			break;
		}
	}
}
