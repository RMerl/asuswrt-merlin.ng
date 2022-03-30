// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 * Author: Mingkai Hu <Mingkai.hu@freescale.com>
 */

/*
 * The RGMII PHYs are provided by the two on-board PHY. The SGMII PHYs
 * are provided by the three on-board PHY or by the standard Freescale
 * four-port SGMII riser card. We need to change the phy-handle in the
 * kernel dts file to point to the correct PHY according to serdes mux
 * and serdes protocol selection.
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_dtsec.h>

#include "cpld.h"
#include "../common/fman.h"

#ifdef CONFIG_FMAN_ENET
/*
 * Mapping of all 18 SERDES lanes to board slots. A value of '0' here means
 * that the mapping must be determined dynamically, or that the lane maps to
 * something other than a board slot
 */
static u8 lane_to_slot[] = {
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0
};

static int riser_phy_addr[] = {
	CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC2_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC3_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC4_RISER_PHY_ADDR,
};

/*
 * Initialize the lane_to_slot[] array.
 *
 * On the P2040RDB board the mapping is controlled by CPLD register.
 */
static void initialize_lane_to_slot(void)
{
	u8 mux = CPLD_READ(serdes_mux);

	lane_to_slot[6] = (mux & SERDES_MUX_LANE_6_MASK) ? 0 : 1;
	lane_to_slot[10] = (mux & SERDES_MUX_LANE_A_MASK) ? 0 : 2;
	lane_to_slot[12] = (mux & SERDES_MUX_LANE_C_MASK) ? 0 : 2;
	lane_to_slot[13] = (mux & SERDES_MUX_LANE_D_MASK) ? 0 : 2;
}

/*
 * Given the following ...
 *
 * 1) A pointer to an Fman Ethernet node (as identified by the 'compat'
 * compatible string and 'addr' physical address)
 *
 * 2) An Fman port
 *
 * ... update the phy-handle property of the Ethernet node to point to the
 * right PHY.  This assumes that we already know the PHY for each port.
 *
 * The offset of the Fman Ethernet node is also passed in for convenience, but
 * it is not used, and we recalculate the offset anyway.
 *
 * Note that what we call "Fman ports" (enum fm_port) is really an Fman MAC.
 * Inside the Fman, "ports" are things that connect to MACs.  We only call them
 * ports in U-Boot because on previous Ethernet devices (e.g. Gianfar), MACs
 * and ports are the same thing.
 *
 */
void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	phy_interface_t intf = fm_info_get_enet_if(port);
	char phy[16];

	/* The RGMII PHY is identified by the MAC connected to it */
	if (intf == PHY_INTERFACE_MODE_RGMII) {
		sprintf(phy, "phy_rgmii_%u", port == FM1_DTSEC5 ? 0 : 1);
		fdt_set_phy_handle(fdt, compat, addr, phy);
	}

	/* The SGMII PHY is identified by the MAC connected to it */
	if (intf == PHY_INTERFACE_MODE_SGMII) {
		int lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + port);
		u8 slot;
		if (lane < 0)
			return;
		slot = lane_to_slot[lane];
		if (slot) {
			sprintf(phy, "phy_sgmii_%x",
					CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR
					+ (port - FM1_DTSEC1));
			fdt_set_phy_handle(fdt, compat, addr, phy);
		} else {
			sprintf(phy, "phy_sgmii_%x",
					CONFIG_SYS_FM1_DTSEC1_PHY_ADDR
					+ (port - FM1_DTSEC1));
			fdt_set_phy_handle(fdt, compat, addr, phy);
		}
	}

	if (intf == PHY_INTERFACE_MODE_XGMII) {
		/* XAUI */
		int lane = serdes_get_first_lane(XAUI_FM1);
		if (lane >= 0) {
			/* The XAUI PHY is identified by the slot */
			sprintf(phy, "phy_xgmii_%u", lane_to_slot[lane]);
			fdt_set_phy_handle(fdt, compat, addr, phy);
		}
	}
}
#endif /* #ifdef CONFIG_FMAN_ENET */

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct fsl_pq_mdio_info dtsec_mdio_info;
	struct tgec_mdio_info tgec_mdio_info;
	unsigned int i, slot;
	int lane;

	printf("Initializing Fman\n");

	initialize_lane_to_slot();

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	tgec_mdio_info.regs =
		(struct tgec_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	tgec_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the real 10G MDIO bus */
	fm_tgec_mdio_init(bis, &tgec_mdio_info);

	/*
	 * Program the three on-board SGMII PHY addresses. If the SGMII Riser
	 * card used, we'll override the PHY address later. For any DTSEC that
	 * is RGMII, we'll also override its PHY address later. We assume that
	 * DTSEC4 and DTSEC5 are used for RGMII.
	 */
	fm_info_set_phy_address(FM1_DTSEC1, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, CONFIG_SYS_FM1_DTSEC2_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC3, CONFIG_SYS_FM1_DTSEC3_PHY_ADDR);

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			if (slot)
				fm_info_set_phy_address(i, riser_phy_addr[i]);
			break;
		case PHY_INTERFACE_MODE_RGMII:
			/* Only DTSEC4 and DTSEC5 can be routed to RGMII */
			fm_info_set_phy_address(i, i == FM1_DTSEC5 ?
					CONFIG_SYS_FM1_DTSEC5_PHY_ADDR :
					CONFIG_SYS_FM1_DTSEC4_PHY_ADDR);
			break;
		default:
			printf("Fman1: DTSEC%u set to unknown interface %i\n",
			       idx + 1, fm_info_get_enet_if(i));
			break;
		}

		fm_info_set_mdio(i,
			miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));
	}

	lane = serdes_get_first_lane(XAUI_FM1);
	if (lane >= 0) {
		slot = lane_to_slot[lane];
		if (slot)
			fm_info_set_phy_address(FM1_10GEC1,
					CONFIG_SYS_FM1_10GEC1_PHY_ADDR);
	}

	fm_info_set_mdio(FM1_10GEC1,
			miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME));
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
