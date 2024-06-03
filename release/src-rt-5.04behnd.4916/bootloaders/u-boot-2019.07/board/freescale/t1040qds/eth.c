// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

/*
 * The RGMII PHYs are provided by the two on-board PHY connected to
 * dTSEC instances 4 and 5. The SGMII PHYs are provided by one on-board
 * PHY or by the standard four-port SGMII riser card (VSC).
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
#include "../common/qixis.h"

#include "t1040qds_qixis.h"

#ifdef CONFIG_FMAN_ENET
 /* - In T1040 there are only 8 SERDES lanes, spread across 2 SERDES banks.
 *   Bank 1 -> Lanes A, B, C, D
 *   Bank 2 -> Lanes E, F, G, H
 */

 /* Mapping of 8 SERDES lanes to T1040 QDS board slots. A value of '0' here
  * means that the mapping must be determined dynamically, or that the lane
  * maps to something other than a board slot.
  */
static u8 lane_to_slot[] = {
	0, 0, 0, 0, 0, 0, 0, 0
};

/* On the Vitesse VSC8234XHG SGMII riser card there are 4 SGMII PHYs
 * housed.
 */
static int riser_phy_addr[] = {
	CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC2_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC3_RISER_PHY_ADDR,
	CONFIG_SYS_FM1_DTSEC4_RISER_PHY_ADDR,
};

/* Slot2 does not have EMI connections */
#define EMI_NONE	0xFFFFFFFF
#define EMI1_RGMII0	0
#define EMI1_RGMII1	1
#define EMI1_SLOT1	2
#define EMI1_SLOT3	3
#define EMI1_SLOT4	4
#define EMI1_SLOT5	5
#define EMI1_SLOT6	6
#define EMI1_SLOT7	7
#define EMI2		8

static int mdio_mux[NUM_FM_PORTS];

static const char * const mdio_names[] = {
	"T1040_QDS_MDIO0",
	"T1040_QDS_MDIO1",
	"T1040_QDS_MDIO2",
	"T1040_QDS_MDIO3",
	"T1040_QDS_MDIO4",
	"T1040_QDS_MDIO5",
	"T1040_QDS_MDIO6",
	"T1040_QDS_MDIO7",
};

struct t1040_qds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static const char *t1040_qds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u8 muxval)
{
	struct mii_dev *bus;
	const char *name = t1040_qds_mdio_name_for_muxval(muxval);

	if (!name) {
		printf("No bus for muxval %x\n", muxval);
		return NULL;
	}

	bus = miiphy_get_dev_by_name(name);

	if (!bus) {
		printf("No bus by name %s\n", name);
		return NULL;
	}

	return bus;
}

static void t1040_qds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;
	if (muxval <= 7) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int t1040_qds_mdio_read(struct mii_dev *bus, int addr, int devad,
				int regnum)
{
	struct t1040_qds_mdio *priv = bus->priv;

	t1040_qds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int t1040_qds_mdio_write(struct mii_dev *bus, int addr, int devad,
				int regnum, u16 value)
{
	struct t1040_qds_mdio *priv = bus->priv;

	t1040_qds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int t1040_qds_mdio_reset(struct mii_dev *bus)
{
	struct t1040_qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int t1040_qds_mdio_init(char *realbusname, u8 muxval)
{
	struct t1040_qds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate t1040_qds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate t1040_qds private data\n");
		free(bus);
		return -1;
	}

	bus->read = t1040_qds_mdio_read;
	bus->write = t1040_qds_mdio_write;
	bus->reset = t1040_qds_mdio_reset;
	strcpy(bus->name, t1040_qds_mdio_name_for_muxval(muxval));

	pmdio->realbus = miiphy_get_dev_by_name(realbusname);

	if (!pmdio->realbus) {
		printf("No bus with name %s\n", realbusname);
		free(bus);
		free(pmdio);
		return -1;
	}

	pmdio->muxval = muxval;
	bus->priv = pmdio;

	return mdio_register(bus);
}

/*
 * Initialize the lane_to_slot[] array.
 *
 * On the T1040QDS board the mapping is controlled by ?? register.
 */
static void initialize_lane_to_slot(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	int serdes1_prtcl = (in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL)
		>> FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	QIXIS_WRITE(cms[0], 0x07);

	switch (serdes1_prtcl) {
	case 0x60:
	case 0x66:
	case 0x67:
	case 0x69:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 6;
		lane_to_slot[3] = 5;
		break;
	case 0x86:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 7;
		lane_to_slot[3] = 7;
		break;
	case 0x87:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 7;
		lane_to_slot[3] = 7;
		lane_to_slot[7] = 7;
		break;
	case 0x89:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 7;
		lane_to_slot[3] = 7;
		lane_to_slot[6] = 7;
		lane_to_slot[7] = 7;
		break;
	case 0x8d:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 7;
		lane_to_slot[3] = 7;
		lane_to_slot[5] = 3;
		lane_to_slot[6] = 3;
		lane_to_slot[7] = 3;
		break;
	case 0x8F:
	case 0x85:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 6;
		lane_to_slot[3] = 5;
		lane_to_slot[6] = 3;
		lane_to_slot[7] = 3;
		break;
	case 0xA5:
		lane_to_slot[1] = 7;
		lane_to_slot[6] = 3;
		lane_to_slot[7] = 3;
		break;
	case 0xA7:
		lane_to_slot[1] = 7;
		lane_to_slot[2] = 6;
		lane_to_slot[3] = 5;
		lane_to_slot[7] = 7;
		break;
	case 0xAA:
		lane_to_slot[1] = 7;
		lane_to_slot[6] = 7;
		lane_to_slot[7] = 7;
		break;
	case 0x40:
		lane_to_slot[2] = 7;
		lane_to_slot[3] = 7;
		break;
	default:
		printf("qds: Fman: Unsupported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	}
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
 * right PHY. This assumes that we already know the PHY for each port.
 *
 * The offset of the Fman Ethernet node is also passed in for convenience, but
 * it is not used, and we recalculate the offset anyway.
 *
 * Note that what we call "Fman ports" (enum fm_port) is really an Fman MAC.
 * Inside the Fman, "ports" are things that connect to MACs. We only call them
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
		sprintf(phy, "rgmii_phy%u", port == FM1_DTSEC4 ? 1 : 2);
		fdt_set_phy_handle(fdt, compat, addr, phy);
	}

	/* The SGMII PHY is identified by the MAC connected to it */
	if (intf == PHY_INTERFACE_MODE_SGMII) {
		int lane = serdes_get_first_lane(FSL_SRDS_1, SGMII_FM1_DTSEC1
						 + port);
		u8 slot;
		if (lane < 0)
			return;
		slot = lane_to_slot[lane];
		if (slot) {
			/* Slot housing a SGMII riser card */
			sprintf(phy, "phy_s%x_%02x", slot,
				(fm_info_get_phy_address(port - FM1_DTSEC1)-
				CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR + 1));
			fdt_set_phy_handle(fdt, compat, addr, phy);
		}
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	int i, lane, idx;

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		idx = i - FM1_DTSEC1;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(FSL_SRDS_1,
						     SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;

			switch (mdio_mux[i]) {
			case EMI1_SLOT3:
				fdt_status_okay_by_alias(fdt, "emi1_slot3");
				break;
			case EMI1_SLOT5:
				fdt_status_okay_by_alias(fdt, "emi1_slot5");
				break;
			case EMI1_SLOT6:
				fdt_status_okay_by_alias(fdt, "emi1_slot6");
				break;
			case EMI1_SLOT7:
				fdt_status_okay_by_alias(fdt, "emi1_slot7");
				break;
			}
		break;
		case PHY_INTERFACE_MODE_RGMII:
			if (i == FM1_DTSEC4)
				fdt_status_okay_by_alias(fdt, "emi1_rgmii0");

			if (i == FM1_DTSEC5)
				fdt_status_okay_by_alias(fdt, "emi1_rgmii1");
			break;
		default:
			break;
		}
	}
}
#endif /* #ifdef CONFIG_FMAN_ENET */

static void set_brdcfg9_for_gtx_clk(void)
{
	u8 brdcfg9;
	brdcfg9 = QIXIS_READ(brdcfg[9]);
/* Initializing EPHY2 clock to RGMII mode */
	brdcfg9 &= ~(BRDCFG9_EPHY2_MASK);
	brdcfg9 |= (BRDCFG9_EPHY2_VAL);
	QIXIS_WRITE(brdcfg[9], brdcfg9);
}

void t1040_handle_phy_interface_sgmii(int i)
{
	int lane, idx, slot;
	idx = i - FM1_DTSEC1;
	lane = serdes_get_first_lane(FSL_SRDS_1,
			SGMII_FM1_DTSEC1 + idx);

	if (lane < 0)
		return;
	slot = lane_to_slot[lane];

	switch (slot) {
	case 1:
		mdio_mux[i] = EMI1_SLOT1;
		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	case 3:
		if (FM1_DTSEC4 == i)
			fm_info_set_phy_address(i, riser_phy_addr[0]);
		if (FM1_DTSEC5 == i)
			fm_info_set_phy_address(i, riser_phy_addr[1]);

		mdio_mux[i] = EMI1_SLOT3;

		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	case 4:
		mdio_mux[i] = EMI1_SLOT4;
		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	case 5:
		/* Slot housing a SGMII riser card? */
		fm_info_set_phy_address(i, riser_phy_addr[0]);
		mdio_mux[i] = EMI1_SLOT5;
		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	case 6:
		/* Slot housing a SGMII riser card? */
		fm_info_set_phy_address(i, riser_phy_addr[0]);
		mdio_mux[i] = EMI1_SLOT6;
		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	case 7:
		if (FM1_DTSEC1 == i)
			fm_info_set_phy_address(i, riser_phy_addr[0]);
		if (FM1_DTSEC2 == i)
			fm_info_set_phy_address(i, riser_phy_addr[1]);
		if (FM1_DTSEC3 == i)
			fm_info_set_phy_address(i, riser_phy_addr[2]);
		if (FM1_DTSEC5 == i)
			fm_info_set_phy_address(i, riser_phy_addr[3]);

		mdio_mux[i] = EMI1_SLOT7;
		fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
		break;
	default:
		break;
	}
	fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
}
void t1040_handle_phy_interface_rgmii(int i)
{
	fm_info_set_phy_address(i, i == FM1_DTSEC5 ?
			CONFIG_SYS_FM1_DTSEC5_PHY_ADDR :
			CONFIG_SYS_FM1_DTSEC4_PHY_ADDR);
	mdio_mux[i] = (i == FM1_DTSEC5) ? EMI1_RGMII1 :
		EMI1_RGMII0;
	fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct memac_mdio_info memac_mdio_info;
	unsigned int i;
#ifdef CONFIG_VSC9953
	int lane;
	int phy_addr;
	phy_interface_t phy_int;
	struct mii_dev *bus;
#endif

	printf("Initializing Fman\n");
	set_brdcfg9_for_gtx_clk();

	initialize_lane_to_slot();

	/* Initialize the mdio_mux array so we can recognize empty elements */
	for (i = 0; i < NUM_FM_PORTS; i++)
		mdio_mux[i] = EMI_NONE;

	memac_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;
	memac_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fm_memac_mdio_init(bis, &memac_mdio_info);

	/* Register the muxing front-ends to the MDIO buses */
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII0);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII1);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT1);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT3);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT4);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT5);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT6);
	t1040_qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT7);

	/*
	 * Program on board RGMII PHY addresses. If the SGMII Riser
	 * card used, we'll override the PHY address later. For any DTSEC that
	 * is RGMII, we'll also override its PHY address later. We assume that
	 * DTSEC4 and DTSEC5 are used for RGMII.
	 */
	fm_info_set_phy_address(FM1_DTSEC4, CONFIG_SYS_FM1_DTSEC4_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC5, CONFIG_SYS_FM1_DTSEC5_PHY_ADDR);

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_QSGMII:
			fm_info_set_mdio(i, NULL);
			break;
		case PHY_INTERFACE_MODE_SGMII:
			t1040_handle_phy_interface_sgmii(i);
			break;

		case PHY_INTERFACE_MODE_RGMII:
			/* Only DTSEC4 and DTSEC5 can be routed to RGMII */
			t1040_handle_phy_interface_rgmii(i);
			break;
		default:
			break;
		}
	}

#ifdef CONFIG_VSC9953
	for (i = 0; i < VSC9953_MAX_PORTS; i++) {
		lane = -1;
		phy_addr = 0;
		phy_int = PHY_INTERFACE_MODE_NONE;
		switch (i) {
		case 0:
		case 1:
		case 2:
		case 3:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_SW1_A);
			/* PHYs connected over QSGMII */
			if (lane >= 0) {
				phy_addr = CONFIG_SYS_FM1_QSGMII21_PHY_ADDR +
						i;
				phy_int = PHY_INTERFACE_MODE_QSGMII;
				break;
			}
			lane = serdes_get_first_lane(FSL_SRDS_1,
					SGMII_SW1_MAC1 + i);

			if (lane < 0)
				break;

			/* PHYs connected over QSGMII */
			if (i != 3 || lane_to_slot[lane] == 7)
				phy_addr = CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR
					+ i;
			else
				phy_addr = CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR;
			phy_int = PHY_INTERFACE_MODE_SGMII;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_SW1_B);
			/* PHYs connected over QSGMII */
			if (lane >= 0) {
				phy_addr = CONFIG_SYS_FM1_QSGMII11_PHY_ADDR +
						i - 4;
				phy_int = PHY_INTERFACE_MODE_QSGMII;
				break;
			}
			lane = serdes_get_first_lane(FSL_SRDS_1,
					SGMII_SW1_MAC1 + i);
			/* PHYs connected over SGMII */
			if (lane >= 0) {
				phy_addr = CONFIG_SYS_FM1_DTSEC1_RISER_PHY_ADDR
						+ i - 3;
				phy_int = PHY_INTERFACE_MODE_SGMII;
			}
			break;
		case 8:
			if (serdes_get_first_lane(FSL_SRDS_1,
						  SGMII_FM1_DTSEC1) < 0)
				/* FM1@DTSEC1 is connected to SW1@PORT8 */
				vsc9953_port_enable(i);
			break;
		case 9:
			if (serdes_get_first_lane(FSL_SRDS_1,
						  SGMII_FM1_DTSEC2) < 0) {
				/* Enable L2 On MAC2 using SCFG */
				struct ccsr_scfg *scfg = (struct ccsr_scfg *)
						CONFIG_SYS_MPC85xx_SCFG;

				out_be32(&scfg->esgmiiselcr,
					 in_be32(&scfg->esgmiiselcr) |
					 (0x80000000));
				vsc9953_port_enable(i);
			}
			break;
		}

		if (lane >= 0) {
			bus = mii_dev_for_muxval(lane_to_slot[lane]);
			vsc9953_port_info_set_mdio(i, bus);
			vsc9953_port_enable(i);
		}
		vsc9953_port_info_set_phy_address(i, phy_addr);
		vsc9953_port_info_set_phy_int(i, phy_int);
	}

#endif
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
