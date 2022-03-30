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
#include "../common/qixis.h"
#include "../common/fman.h"
#include "t102xqds_qixis.h"

#define EMI_NONE	0xFFFFFFFF
#define EMI1_RGMII1	0
#define EMI1_RGMII2	1
#define EMI1_SLOT1	2
#define EMI1_SLOT2	3
#define EMI1_SLOT3	4
#define EMI1_SLOT4	5
#define EMI1_SLOT5	6
#define EMI2		7

static int mdio_mux[NUM_FM_PORTS];

static const char * const mdio_names[] = {
	"T1024QDS_MDIO_RGMII1",
	"T1024QDS_MDIO_RGMII2",
	"T1024QDS_MDIO_SLOT1",
	"T1024QDS_MDIO_SLOT2",
	"T1024QDS_MDIO_SLOT3",
	"T1024QDS_MDIO_SLOT4",
	"T1024QDS_MDIO_SLOT5",
	"T1024QDS_MDIO_10GC",
	"NULL",
};

/* Map SerDes1 4 lanes to default slot, will be initialized dynamically */
static u8 lane_to_slot[] = {2, 3, 4, 5};

static const char *t1024qds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u8 muxval)
{
	struct mii_dev *bus;
	const char *name;

	if (muxval > EMI2)
		return NULL;

	name = t1024qds_mdio_name_for_muxval(muxval);

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

struct t1024qds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static void t1024qds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;

	if (muxval < 7) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int t1024qds_mdio_read(struct mii_dev *bus, int addr, int devad,
			      int regnum)
{
	struct t1024qds_mdio *priv = bus->priv;

	t1024qds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int t1024qds_mdio_write(struct mii_dev *bus, int addr, int devad,
			       int regnum, u16 value)
{
	struct t1024qds_mdio *priv = bus->priv;

	t1024qds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int t1024qds_mdio_reset(struct mii_dev *bus)
{
	struct t1024qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int t1024qds_mdio_init(char *realbusname, u8 muxval)
{
	struct t1024qds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate t1024qds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate t1024qds private data\n");
		free(bus);
		return -1;
	}

	bus->read = t1024qds_mdio_read;
	bus->write = t1024qds_mdio_write;
	bus->reset = t1024qds_mdio_reset;
	strcpy(bus->name, t1024qds_mdio_name_for_muxval(muxval));

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

void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	struct fixed_link f_link;

	if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_RGMII) {
		if (port == FM1_DTSEC3) {
			fdt_set_phy_handle(fdt, compat, addr, "rgmii_phy2");
			fdt_setprop_string(fdt, offset, "phy-connection-type",
					   "rgmii");
			fdt_status_okay_by_alias(fdt, "emi1_rgmii1");
		}
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII) {
		if (port == FM1_DTSEC1) {
			fdt_set_phy_handle(fdt, compat, addr,
					   "sgmii_vsc8234_phy_s5");
		} else if (port == FM1_DTSEC2) {
			fdt_set_phy_handle(fdt, compat, addr,
					   "sgmii_vsc8234_phy_s4");
		}
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII_2500) {
		if (port == FM1_DTSEC3) {
			fdt_set_phy_handle(fdt, compat, addr,
					   "sgmii_aqr105_phy_s3");
		}
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_QSGMII) {
		switch (port) {
		case FM1_DTSEC1:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_phy_p1");
			break;
		case FM1_DTSEC2:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_phy_p2");
			break;
		case FM1_DTSEC3:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_phy_p3");
			break;
		case FM1_DTSEC4:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_phy_p4");
			break;
		default:
			break;
		}
		fdt_delprop(fdt, offset, "phy-connection-type");
		fdt_setprop_string(fdt, offset, "phy-connection-type",
				   "qsgmii");
		fdt_status_okay_by_alias(fdt, "emi1_slot2");
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_XGMII) {
		/* XFI interface */
		f_link.phy_id = port;
		f_link.duplex = 1;
		f_link.link_speed = 10000;
		f_link.pause = 0;
		f_link.asym_pause = 0;
		/* no PHY for XFI */
		fdt_delprop(fdt, offset, "phy-handle");
		fdt_setprop(fdt, offset, "fixed-link", &f_link, sizeof(f_link));
		fdt_setprop_string(fdt, offset, "phy-connection-type", "xgmii");
	}
}

void fdt_fixup_board_enet(void *fdt)
{
}

/*
 * This function reads RCW to check if Serdes1{A:D} is configured
 * to slot 1/2/3/4/5 and update the lane_to_slot[] array accordingly
 */
static void initialize_lane_to_slot(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;

	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	switch (srds_s1) {
	case 0x46:
	case 0x47:
		lane_to_slot[1] = 2;
		break;
	default:
		break;
	}
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FMAN_ENET)
	int i, idx, lane, slot, interface;
	struct memac_mdio_info dtsec_mdio_info;
	struct memac_mdio_info tgec_mdio_info;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) &
					FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	initialize_lane_to_slot();

	/* Initialize the mdio_mux array so we can recognize empty elements */
	for (i = 0; i < NUM_FM_PORTS; i++)
		mdio_mux[i] = EMI_NONE;

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

	/* Register the muxing front-ends to the MDIO buses */
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII1);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII2);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT1);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT2);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT3);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT4);
	t1024qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT5);
	t1024qds_mdio_init(DEFAULT_FM_TGEC_MDIO_NAME, EMI2);

	/* Set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY2_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY1_ADDR);

	switch (srds_s1) {
	case 0xd5:
	case 0xd6:
		/* QSGMII in Slot2 */
		fm_info_set_phy_address(FM1_DTSEC1, 0x8);
		fm_info_set_phy_address(FM1_DTSEC2, 0x9);
		fm_info_set_phy_address(FM1_DTSEC3, 0xa);
		fm_info_set_phy_address(FM1_DTSEC4, 0xb);
		break;
	case 0x95:
	case 0x99:
		/*
		 * XFI does not need a PHY to work, but to avoid U-Boot use
		 * default PHY address which is zero to a MAC when it found
		 * a MAC has no PHY address, we give a PHY address to XFI
		 * MAC, and should not use a real XAUI PHY address, since
		 * MDIO can access it successfully, and then MDIO thinks the
		 * XAUI card is used for the XFI MAC, which will cause error.
		 */
		fm_info_set_phy_address(FM1_10GEC1, 4);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x6f:
		/* SGMII in Slot3, Slot4, Slot5 */
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_AQ_PHY_ADDR_S5);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_AQ_PHY_ADDR_S4);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x7f:
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_AQ_PHY_ADDR_S5);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_AQ_PHY_ADDR_S4);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_AQ_PHY_ADDR_S3);
		break;
	case 0x47:
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x77:
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_AQ_PHY_ADDR_S3);
		break;
	case 0x5a:
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x6a:
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x5b:
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	case 0x6b:
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_PORT1_PHY_ADDR);
		break;
	default:
		break;
	}

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		idx = i - FM1_DTSEC1;
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_SGMII:
		case PHY_INTERFACE_MODE_SGMII_2500:
		case PHY_INTERFACE_MODE_QSGMII:
			if (interface == PHY_INTERFACE_MODE_SGMII) {
				lane = serdes_get_first_lane(FSL_SRDS_1,
						SGMII_FM1_DTSEC1 + idx);
			} else if (interface == PHY_INTERFACE_MODE_SGMII_2500) {
				lane = serdes_get_first_lane(FSL_SRDS_1,
						SGMII_2500_FM1_DTSEC1 + idx);
			} else {
				lane = serdes_get_first_lane(FSL_SRDS_1,
						QSGMII_FM1_A);
			}

			if (lane < 0)
				break;

			slot = lane_to_slot[lane];
			debug("FM1@DTSEC%u expects SGMII in slot %u\n",
			      idx + 1, slot);
			if (QIXIS_READ(present2) & (1 << (slot - 1)))
				fm_disable_port(i);

			switch (slot) {
			case 2:
				mdio_mux[i] = EMI1_SLOT2;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			case 3:
				mdio_mux[i] = EMI1_SLOT3;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			case 4:
				mdio_mux[i] = EMI1_SLOT4;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			case 5:
				mdio_mux[i] = EMI1_SLOT5;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			}
			break;
		case PHY_INTERFACE_MODE_RGMII:
			if (i == FM1_DTSEC3)
				mdio_mux[i] = EMI1_RGMII2;
			else if (i == FM1_DTSEC4)
				mdio_mux[i] = EMI1_RGMII1;
			fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
			break;
		default:
			break;
		}
	}

	for (i = FM1_10GEC1; i < FM1_10GEC1 + CONFIG_SYS_NUM_FM1_10GEC; i++) {
		idx = i - FM1_10GEC1;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			lane = serdes_get_first_lane(FSL_SRDS_1,
						     XFI_FM1_MAC1 + idx);
			if (lane < 0)
				break;
			mdio_mux[i] = EMI2;
			fm_info_set_mdio(i, mii_dev_for_muxval(mdio_mux[i]));
			break;
		default:
			break;
		}
	}

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}
