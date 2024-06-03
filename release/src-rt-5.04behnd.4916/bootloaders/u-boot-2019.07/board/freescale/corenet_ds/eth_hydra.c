// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 * Author: Timur Tabi <timur@freescale.com>
 */

/*
 * This file handles the board muxing between the Fman Ethernet MACs and
 * the RGMII/SGMII/XGMII PHYs on a Freescale P3041/P5020 "Hydra" reference
 * board. The RGMII PHYs are the two on-board 1Gb ports.  The SGMII PHYs are
 * provided by the standard Freescale four-port SGMII riser card.  The 10Gb
 * XGMII PHY is provided via the XAUI riser card.  Since there is only one
 * Fman device on a P3041 and P5020, we only support one SGMII card and one
 * RGMII card.
 *
 * Muxing is handled via the PIXIS BRDCFG1 register.  The EMI1 bits control
 * muxing among the RGMII PHYs and the SGMII PHYs.  The value for RGMII is
 * always the same (0).  The value for SGMII depends on which slot the riser is
 * inserted in.  The EMI2 bits control muxing for the the XGMII.  Like SGMII,
 * the value is based on which slot the XAUI is inserted in.
 *
 * The SERDES configuration is used to determine where the SGMII and XAUI cards
 * exist, and also which Fman MACs are routed to which PHYs.  So for a given
 * Fman MAC, there is one and only PHY it connects to.  MACs cannot be routed
 * to PHYs dynamically.
 *
 *
 * This file also updates the device tree in three ways:
 *
 * 1) The status of each virtual MDIO node that is referenced by an Ethernet
 *    node is set to "okay".
 *
 * 2) The phy-handle property of each active Ethernet MAC node is set to the
 *    appropriate PHY node.
 *
 * 3) The "mux value" for each virtual MDIO node is set to the correct value,
 *    if necessary.  Some virtual MDIO nodes do not have configurable mux
 *    values, so those values are hard-coded in the DTS.  On the HYDRA board,
 *    the virtual MDIO node for the SGMII card needs to be updated.
 *
 * For all this to work, the device tree needs to have the following:
 *
 * 1) An alias for each PHY node that an Ethernet node could be routed to.
 *
 * 2) An alias for each real and virtual MDIO node that is disabled by default
 * and might need to be enabled, and also might need to have its mux-value
 * updated.
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

#ifdef CONFIG_FMAN_ENET

#define BRDCFG1_EMI1_SEL_MASK	0x78
#define BRDCFG1_EMI1_SEL_SLOT1	0x10
#define BRDCFG1_EMI1_SEL_SLOT2	0x20
#define BRDCFG1_EMI1_SEL_SLOT5	0x30
#define BRDCFG1_EMI1_SEL_SLOT6	0x40
#define BRDCFG1_EMI1_SEL_SLOT7	0x50
#define BRDCFG1_EMI1_SEL_RGMII	0x00
#define BRDCFG1_EMI1_EN		0x08
#define BRDCFG1_EMI2_SEL_MASK	0x06
#define BRDCFG1_EMI2_SEL_SLOT1	0x00
#define BRDCFG1_EMI2_SEL_SLOT2	0x02

#define BRDCFG2_REG_GPIO_SEL	0x20

#define PHY_BASE_ADDR		0x00

/*
 * BRDCFG1 mask and value for each MAC
 *
 * This array contains the BRDCFG1 values (in mask/val format) that route the
 * MDIO bus to a particular RGMII or SGMII PHY.
 */
struct {
	u8 mask;
	u8 val;
} mdio_mux[NUM_FM_PORTS];

/*
 * Mapping of all 18 SERDES lanes to board slots. A value of '0' here means
 * that the mapping must be determined dynamically, or that the lane maps to
 * something other than a board slot
 */
static u8 lane_to_slot[] = {
	7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 0, 0
};

/*
 * Set the board muxing for a given MAC
 *
 * The MDIO layer calls this function every time it wants to talk to a PHY.
 */
void hydra_mux_mdio(u8 mask, u8 val)
{
	clrsetbits_8(&pixis->brdcfg1, mask, val);
}

struct hydra_mdio {
	u8 mask;
	u8 val;
	struct mii_dev *realbus;
};

static int hydra_mdio_read(struct mii_dev *bus, int addr, int devad,
				int regnum)
{
	struct hydra_mdio *priv = bus->priv;

	hydra_mux_mdio(priv->mask, priv->val);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int hydra_mdio_write(struct mii_dev *bus, int addr, int devad,
				int regnum, u16 value)
{
	struct hydra_mdio *priv = bus->priv;

	hydra_mux_mdio(priv->mask, priv->val);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int hydra_mdio_reset(struct mii_dev *bus)
{
	struct hydra_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static void hydra_mdio_set_mux(char *name, u8 mask, u8 val)
{
	struct mii_dev *bus = miiphy_get_dev_by_name(name);
	struct hydra_mdio *priv = bus->priv;

	priv->mask = mask;
	priv->val = val;
}

static int hydra_mdio_init(char *realbusname, char *fakebusname)
{
	struct hydra_mdio *hmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate Hydra MDIO bus\n");
		return -1;
	}

	hmdio = malloc(sizeof(*hmdio));
	if (!hmdio) {
		printf("Failed to allocate Hydra private data\n");
		free(bus);
		return -1;
	}

	bus->read = hydra_mdio_read;
	bus->write = hydra_mdio_write;
	bus->reset = hydra_mdio_reset;
	strcpy(bus->name, fakebusname);

	hmdio->realbus = miiphy_get_dev_by_name(realbusname);

	if (!hmdio->realbus) {
		printf("No bus with name %s\n", realbusname);
		free(bus);
		free(hmdio);
		return -1;
	}

	bus->priv = hmdio;

	return mdio_register(bus);
}

/*
 * Given an alias or a path for a node, set the mux value of that node.
 *
 * If 'alias' is not a valid alias, then it is treated as a full path to the
 * node.  No error checking is performed.
 *
 * This function is normally called to set the fsl,hydra-mdio-muxval property
 * of a virtual MDIO node.
 */
static void fdt_set_mdio_mux(void *fdt, const char *alias, u32 mux)
{
	const char *path = fdt_get_alias(fdt, alias);

	if (!path)
		path = alias;

	do_fixup_by_path(fdt, path, "reg",
			 &mux, sizeof(mux), 1);
	do_fixup_by_path(fdt, path, "fsl,hydra-mdio-muxval",
			 &mux, sizeof(mux), 1);
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
 * right PHY.  This assumes that we already know the PHY for each port.  That
 * information is stored in mdio_mux[].
 *
 * The offset of the Fman Ethernet node is also passed in for convenience, but
 * it is not used, and we recalculate the offset anyway.
 *
 * Note that what we call "Fman ports" (enum fm_port) is really an Fman MAC.
 * Inside the Fman, "ports" are things that connect to MACs.  We only call them
 * ports in U-Boot because on previous Ethernet devices (e.g. Gianfar), MACs
 * and ports are the same thing.
 *
 * Note that this code would be cleaner if had a function called
 * fm_info_get_phy_address(), which returns a value from the fm1_dtsec_info[]
 * array.  That's because all we're doing is figuring out the PHY address for
 * a given Fman MAC and writing it to the device tree.  Well, we already did
 * the hard work to figure that out in board_eth_init(), so it's silly to
 * repeat that here.
 */
void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	unsigned int mux = mdio_mux[port].val & mdio_mux[port].mask;
	char phy[16];

	if (port == FM1_10GEC1) {
		/* XAUI */
		int lane = serdes_get_first_lane(XAUI_FM1);
		if (lane >= 0) {
			/* The XAUI PHY is identified by the slot */
			sprintf(phy, "phy_xgmii_%u", lane_to_slot[lane]);
			fdt_set_phy_handle(fdt, compat, addr, phy);
		}
		return;
	}

	if (mux == (BRDCFG1_EMI1_SEL_RGMII | BRDCFG1_EMI1_EN)) {
		/* RGMII */
		/* The RGMII PHY is identified by the MAC connected to it */
		sprintf(phy, "phy_rgmii_%u", port == FM1_DTSEC4 ? 0 : 1);
		fdt_set_phy_handle(fdt, compat, addr, phy);
		return;
	}

	/* If it's not RGMII or XGMII, it must be SGMII */
	if (mux) {
		/* The SGMII PHY is identified by the MAC connected to it */
		sprintf(phy, "phy_sgmii_%x",
			CONFIG_SYS_FM1_DTSEC1_PHY_ADDR + (port - FM1_DTSEC1));
		fdt_set_phy_handle(fdt, compat, addr, phy);
	}
}

#define PIXIS_SW2_LANE_23_SEL		0x80
#define PIXIS_SW2_LANE_45_SEL		0x40
#define PIXIS_SW2_LANE_67_SEL_MASK	0x30
#define PIXIS_SW2_LANE_67_SEL_5		0x00
#define PIXIS_SW2_LANE_67_SEL_6		0x20
#define PIXIS_SW2_LANE_67_SEL_7		0x10
#define PIXIS_SW2_LANE_8_SEL		0x08
#define PIXIS_SW2_LANE_1617_SEL		0x04

/*
 * Initialize the lane_to_slot[] array.
 *
 * On the P4080DS "Expedition" board, the mapping of SERDES lanes to board
 * slots is hard-coded.  On the Hydra board, however, the mapping is controlled
 * by board switch SW2, so the lane_to_slot[] array needs to be dynamically
 * initialized.
 */
static void initialize_lane_to_slot(void)
{
	u8 sw2 = in_8(&PIXIS_SW(2));

	lane_to_slot[2] = (sw2 & PIXIS_SW2_LANE_23_SEL) ? 7 : 4;
	lane_to_slot[3] = lane_to_slot[2];

	lane_to_slot[4] = (sw2 & PIXIS_SW2_LANE_45_SEL) ? 7 : 6;
	lane_to_slot[5] = lane_to_slot[4];

	switch (sw2 & PIXIS_SW2_LANE_67_SEL_MASK) {
	case PIXIS_SW2_LANE_67_SEL_5:
		lane_to_slot[6] = 5;
		break;
	case PIXIS_SW2_LANE_67_SEL_6:
		lane_to_slot[6] = 6;
		break;
	case PIXIS_SW2_LANE_67_SEL_7:
		lane_to_slot[6] = 7;
		break;
	}
	lane_to_slot[7] = lane_to_slot[6];

	lane_to_slot[8] = (sw2 & PIXIS_SW2_LANE_8_SEL) ? 3 : 0;

	lane_to_slot[16] = (sw2 & PIXIS_SW2_LANE_1617_SEL) ? 1 : 0;
	lane_to_slot[17] = lane_to_slot[16];
}

#endif /* #ifdef CONFIG_FMAN_ENET */

/*
 * Configure the status for the virtual MDIO nodes
 *
 * Rather than create the virtual MDIO nodes from scratch for each active
 * virtual MDIO, we expect the DTS to have the nodes defined already, and we
 * only enable the ones that are actually active.
 *
 * We assume that the DTS already hard-codes the status for all the
 * virtual MDIO nodes to "disabled", so all we need to do is enable the
 * active ones.
 *
 * For SGMII, we also need to set the mux value in the node.
 */
void fdt_fixup_board_enet(void *fdt)
{
#ifdef CONFIG_FMAN_ENET
	unsigned int i;
	int lane;

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane >= 0) {
				fdt_status_okay_by_alias(fdt, "emi1_sgmii");
				/* Also set the MUX value */
				fdt_set_mdio_mux(fdt, "emi1_sgmii",
						 mdio_mux[i].val);
			}
			break;
		case PHY_INTERFACE_MODE_RGMII:
			fdt_status_okay_by_alias(fdt, "emi1_rgmii");
			break;
		default:
			break;
		}
	}

	lane = serdes_get_first_lane(XAUI_FM1);
	if (lane >= 0)
		fdt_status_okay_by_alias(fdt, "emi2_xgmii");
#endif
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct fsl_pq_mdio_info dtsec_mdio_info;
	struct tgec_mdio_info tgec_mdio_info;
	unsigned int i, slot;
	int lane;
	struct mii_dev *bus;

	printf("Initializing Fman\n");

	initialize_lane_to_slot();

	/* We want to use the PIXIS to configure MUX routing, not GPIOs. */
	setbits_8(&pixis->brdcfg2, BRDCFG2_REG_GPIO_SEL);

	memset(mdio_mux, 0, sizeof(mdio_mux));

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

	/* Register the three virtual MDIO front-ends */
	hydra_mdio_init(DEFAULT_FM_MDIO_NAME, "HYDRA_RGMII_MDIO");
	hydra_mdio_init(DEFAULT_FM_MDIO_NAME, "HYDRA_SGMII_MDIO");

	/*
	 * Program the DTSEC PHY addresses assuming that they are all SGMII.
	 * For any DTSEC that's RGMII, we'll override its PHY address later.
	 * We assume that DTSEC5 is only used for RGMII.
	 */
	fm_info_set_phy_address(FM1_DTSEC1, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, CONFIG_SYS_FM1_DTSEC2_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC3, CONFIG_SYS_FM1_DTSEC3_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, CONFIG_SYS_FM1_DTSEC4_PHY_ADDR);

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			mdio_mux[i].mask = BRDCFG1_EMI1_SEL_MASK;
			switch (slot) {
			case 1:
				/* Always DTSEC5 on Bank 3 */
				mdio_mux[i].val = BRDCFG1_EMI1_SEL_SLOT1 |
						  BRDCFG1_EMI1_EN;
				break;
			case 2:
				mdio_mux[i].val = BRDCFG1_EMI1_SEL_SLOT2 |
						  BRDCFG1_EMI1_EN;
				break;
			case 5:
				mdio_mux[i].val = BRDCFG1_EMI1_SEL_SLOT5 |
						  BRDCFG1_EMI1_EN;
				break;
			case 6:
				mdio_mux[i].val = BRDCFG1_EMI1_SEL_SLOT6 |
						  BRDCFG1_EMI1_EN;
				break;
			case 7:
				mdio_mux[i].val = BRDCFG1_EMI1_SEL_SLOT7 |
						  BRDCFG1_EMI1_EN;
				break;
			};

			hydra_mdio_set_mux("HYDRA_SGMII_MDIO",
					mdio_mux[i].mask, mdio_mux[i].val);
			fm_info_set_mdio(i,
				miiphy_get_dev_by_name("HYDRA_SGMII_MDIO"));
			break;
		case PHY_INTERFACE_MODE_RGMII:
			/*
			 * If DTSEC4 is RGMII, then it's routed via via EC1 to
			 * the first on-board RGMII port.  If DTSEC5 is RGMII,
			 * then it's routed via via EC2 to the second on-board
			 * RGMII port. The other DTSECs cannot be routed to
			 * RGMII.
			 */
			fm_info_set_phy_address(i, i == FM1_DTSEC4 ? 0 : 1);
			mdio_mux[i].mask = BRDCFG1_EMI1_SEL_MASK;
			mdio_mux[i].val  = BRDCFG1_EMI1_SEL_RGMII |
					   BRDCFG1_EMI1_EN;
			hydra_mdio_set_mux("HYDRA_RGMII_MDIO",
					mdio_mux[i].mask, mdio_mux[i].val);
			fm_info_set_mdio(i,
				miiphy_get_dev_by_name("HYDRA_RGMII_MDIO"));
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

	bus = miiphy_get_dev_by_name("HYDRA_SGMII_MDIO");
	set_sgmii_phy(bus, FM1_DTSEC1, CONFIG_SYS_NUM_FM1_DTSEC, PHY_BASE_ADDR);

	/*
	 * For 10G, we only support one XAUI card per Fman.  If present, then we
	 * force its routing and never touch those bits again, which removes the
	 * need for Linux to do any muxing.  This works because of the way
	 * BRDCFG1 is defined, but it's a bit hackish.
	 *
	 * The PHY address for the XAUI card depends on which slot it's in. The
	 * macros we use imply that the PHY address is based on which FM, but
	 * that's not true.  On the P4080DS, FM1 could only use XAUI in slot 5,
	 * and FM2 could only use a XAUI in slot 4.  On the Hydra board, we
	 * check the actual slot and just use the macros as-is, even though
	 * the P3041 and P5020 only have one Fman.
	 */
	lane = serdes_get_first_lane(XAUI_FM1);
	if (lane >= 0) {
		slot = lane_to_slot[lane];
		if (slot == 1) {
			/* XAUI card is in slot 1 */
			clrsetbits_8(&pixis->brdcfg1, BRDCFG1_EMI2_SEL_MASK,
				     BRDCFG1_EMI2_SEL_SLOT1);
			fm_info_set_phy_address(FM1_10GEC1,
						CONFIG_SYS_FM1_10GEC1_PHY_ADDR);
		} else {
			/* XAUI card is in slot 2 */
			clrsetbits_8(&pixis->brdcfg1, BRDCFG1_EMI2_SEL_MASK,
				     BRDCFG1_EMI2_SEL_SLOT2);
			fm_info_set_phy_address(FM1_10GEC1,
						CONFIG_SYS_FM2_10GEC1_PHY_ADDR);
		}
	}

	fm_info_set_mdio(FM1_10GEC1,
			miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME));

	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
