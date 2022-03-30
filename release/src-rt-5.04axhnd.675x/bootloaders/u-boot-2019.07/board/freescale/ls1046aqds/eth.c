// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <fsl_dtsec.h>
#include <malloc.h>
#include <asm/arch/fsl_serdes.h>

#include "../common/qixis.h"
#include "../common/fman.h"
#include "ls1046aqds_qixis.h"

#define EMI_NONE	0xFF
#define EMI1_RGMII1	0
#define EMI1_RGMII2	1
#define EMI1_SLOT1	2
#define EMI1_SLOT2	3
#define EMI1_SLOT4	4

static int mdio_mux[NUM_FM_PORTS];

static const char * const mdio_names[] = {
	"LS1046AQDS_MDIO_RGMII1",
	"LS1046AQDS_MDIO_RGMII2",
	"LS1046AQDS_MDIO_SLOT1",
	"LS1046AQDS_MDIO_SLOT2",
	"LS1046AQDS_MDIO_SLOT4",
	"NULL",
};

/* Map SerDes 1 & 2 lanes to default slot. */
static u8 lane_to_slot[] = {1, 1, 1, 1, 0, 4, 0 , 0};

static const char *ls1046aqds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u8 muxval)
{
	struct mii_dev *bus;
	const char *name;

	if (muxval > EMI1_SLOT4)
		return NULL;

	name = ls1046aqds_mdio_name_for_muxval(muxval);

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

struct ls1046aqds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static void ls1046aqds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;

	if (muxval < 7) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int ls1046aqds_mdio_read(struct mii_dev *bus, int addr, int devad,
			      int regnum)
{
	struct ls1046aqds_mdio *priv = bus->priv;

	ls1046aqds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int ls1046aqds_mdio_write(struct mii_dev *bus, int addr, int devad,
			       int regnum, u16 value)
{
	struct ls1046aqds_mdio *priv = bus->priv;

	ls1046aqds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad,
				    regnum, value);
}

static int ls1046aqds_mdio_reset(struct mii_dev *bus)
{
	struct ls1046aqds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int ls1046aqds_mdio_init(char *realbusname, u8 muxval)
{
	struct ls1046aqds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate ls1046aqds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate ls1046aqds private data\n");
		free(bus);
		return -1;
	}

	bus->read = ls1046aqds_mdio_read;
	bus->write = ls1046aqds_mdio_write;
	bus->reset = ls1046aqds_mdio_reset;
	sprintf(bus->name, ls1046aqds_mdio_name_for_muxval(muxval));

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
	const u32 *handle;
	const char *prop = NULL;
	int off;

	if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII) {
		switch (port) {
		case FM1_DTSEC9:
			fdt_set_phy_handle(fdt, compat, addr, "sgmii-s1-p1");
			break;
		case FM1_DTSEC10:
			fdt_set_phy_handle(fdt, compat, addr, "sgmii-s1-p2");
			break;
		case FM1_DTSEC5:
			fdt_set_phy_handle(fdt, compat, addr, "sgmii-s1-p3");
			break;
		case FM1_DTSEC6:
			fdt_set_phy_handle(fdt, compat, addr, "sgmii-s1-p4");
			break;
		case FM1_DTSEC2:
			fdt_set_phy_handle(fdt, compat, addr, "sgmii-s4-p1");
			break;
		default:
			break;
		}
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII_2500) {
		/* 2.5G SGMII interface */
		f_link.phy_id = cpu_to_fdt32(port);
		f_link.duplex = cpu_to_fdt32(1);
		f_link.link_speed = cpu_to_fdt32(1000);
		f_link.pause = 0;
		f_link.asym_pause = 0;
		/* no PHY for 2.5G SGMII on QDS */
		fdt_delprop(fdt, offset, "phy-handle");
		fdt_setprop(fdt, offset, "fixed-link", &f_link, sizeof(f_link));
		fdt_setprop_string(fdt, offset, "phy-connection-type",
				   "sgmii-2500");
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_QSGMII) {
		switch (port) {
		case FM1_DTSEC1:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii-s2-p4");
			break;
		case FM1_DTSEC5:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii-s2-p2");
			break;
		case FM1_DTSEC6:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii-s2-p1");
			break;
		case FM1_DTSEC10:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii-s2-p3");
			break;
		default:
			break;
		}
		fdt_delprop(fdt, offset, "phy-connection-type");
		fdt_setprop_string(fdt, offset, "phy-connection-type",
				   "qsgmii");
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_XGMII &&
		   (port == FM1_10GEC1 || port == FM1_10GEC2)) {
		handle = fdt_getprop(fdt, offset, "phy-handle", NULL);
		prop = NULL;
		if (handle) {
			off = fdt_node_offset_by_phandle(fdt,
							 fdt32_to_cpu(*handle));
			prop = fdt_getprop(fdt, off, "backplane-mode", NULL);
		}
		if (!prop || strcmp(prop, "10gbase-kr")) {
			/* XFI interface */
			f_link.phy_id = cpu_to_fdt32(port);
			f_link.duplex = cpu_to_fdt32(1);
			f_link.link_speed = cpu_to_fdt32(10000);
			f_link.pause = 0;
			f_link.asym_pause = 0;
			/* no PHY for XFI */
			fdt_delprop(fdt, offset, "phy-handle");
			fdt_setprop(fdt, offset, "fixed-link", &f_link,
				    sizeof(f_link));
			fdt_setprop_string(fdt, offset, "phy-connection-type",
					   "xgmii");
		}
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	int i;

	for (i = FM1_DTSEC1; i < NUM_FM_PORTS; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
		case PHY_INTERFACE_MODE_QSGMII:
			switch (mdio_mux[i]) {
			case EMI1_SLOT1:
				fdt_status_okay_by_alias(fdt, "emi1-slot1");
				break;
			case EMI1_SLOT2:
				fdt_status_okay_by_alias(fdt, "emi1-slot2");
				break;
			case EMI1_SLOT4:
				fdt_status_okay_by_alias(fdt, "emi1-slot4");
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	int i, idx, lane, slot, interface;
	struct memac_mdio_info dtsec_mdio_info;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1, srds_s2;
	u8 brdcfg12;

	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	srds_s2 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	/* Initialize the mdio_mux array so we can recognize empty elements */
	for (i = 0; i < NUM_FM_PORTS; i++)
		mdio_mux[i] = EMI_NONE;

	dtsec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;

	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fm_memac_mdio_init(bis, &dtsec_mdio_info);

	/* Register the muxing front-ends to the MDIO buses */
	ls1046aqds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII1);
	ls1046aqds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII2);
	ls1046aqds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT1);
	ls1046aqds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT2);
	ls1046aqds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT4);

	/* Set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);

	switch (srds_s1) {
	case 0x3333:
		/* SGMII on slot 1, MAC 9 */
		fm_info_set_phy_address(FM1_DTSEC9, SGMII_CARD_PORT1_PHY_ADDR);
	case 0x1333:
	case 0x2333:
		/* SGMII on slot 1, MAC 10 */
		fm_info_set_phy_address(FM1_DTSEC10, SGMII_CARD_PORT2_PHY_ADDR);
	case 0x1133:
	case 0x2233:
		/* SGMII on slot 1, MAC 5/6 */
		fm_info_set_phy_address(FM1_DTSEC5, SGMII_CARD_PORT3_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC6, SGMII_CARD_PORT4_PHY_ADDR);
		break;
	case 0x1040:
	case 0x2040:
		/* QSGMII on lane B, MAC 6/5/10/1 */
		fm_info_set_phy_address(FM1_DTSEC6,
					QSGMII_CARD_PORT1_PHY_ADDR_S2);
		fm_info_set_phy_address(FM1_DTSEC5,
					QSGMII_CARD_PORT2_PHY_ADDR_S2);
		fm_info_set_phy_address(FM1_DTSEC10,
					QSGMII_CARD_PORT3_PHY_ADDR_S2);
		fm_info_set_phy_address(FM1_DTSEC1,
					QSGMII_CARD_PORT4_PHY_ADDR_S2);
		break;
	case 0x3363:
		/* SGMII on slot 1, MAC 9/10 */
		fm_info_set_phy_address(FM1_DTSEC9, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC10, SGMII_CARD_PORT2_PHY_ADDR);
	case 0x1163:
	case 0x2263:
	case 0x2223:
		/* SGMII on slot 1, MAC 6 */
		fm_info_set_phy_address(FM1_DTSEC6, SGMII_CARD_PORT4_PHY_ADDR);
		break;
	default:
		printf("Invalid SerDes protocol 0x%x for LS1046AQDS\n",
		       srds_s1);
		break;
	}

	if (srds_s2 == 0x5a59 || srds_s2 == 0x5a06)
		/* SGMII on slot 4, MAC 2 */
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT1_PHY_ADDR);

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		idx = i - FM1_DTSEC1;
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_SGMII:
		case PHY_INTERFACE_MODE_QSGMII:
			if (interface == PHY_INTERFACE_MODE_SGMII) {
				if (i == FM1_DTSEC5) {
					/* route lane 2 to slot1 so to have
					 * one sgmii riser card supports
					 * MAC5 and MAC6.
					 */
					brdcfg12 = QIXIS_READ(brdcfg[12]);
					QIXIS_WRITE(brdcfg[12],
						    brdcfg12 | 0x80);
				}
				lane = serdes_get_first_lane(FSL_SRDS_1,
						SGMII_FM1_DTSEC1 + idx);
			} else {
				/* clear the bit 7 to route lane B on slot2. */
				brdcfg12 = QIXIS_READ(brdcfg[12]);
				QIXIS_WRITE(brdcfg[12], brdcfg12 & 0x7f);

				lane = serdes_get_first_lane(FSL_SRDS_1,
						QSGMII_FM1_A);
				lane_to_slot[lane] = 2;
			}

			if (i == FM1_DTSEC2)
				lane = 5;

			if (lane < 0)
				break;

			slot = lane_to_slot[lane];
			debug("FM1@DTSEC%u expects SGMII in slot %u\n",
			      idx + 1, slot);
			if (QIXIS_READ(present2) & (1 << (slot - 1)))
				fm_disable_port(i);

			switch (slot) {
			case 1:
				mdio_mux[i] = EMI1_SLOT1;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			case 2:
				mdio_mux[i] = EMI1_SLOT2;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			case 4:
				mdio_mux[i] = EMI1_SLOT4;
				fm_info_set_mdio(i, mii_dev_for_muxval(
						 mdio_mux[i]));
				break;
			default:
				break;
			}
			break;
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_TXID:
			if (i == FM1_DTSEC3)
				mdio_mux[i] = EMI1_RGMII1;
			else if (i == FM1_DTSEC4)
				mdio_mux[i] = EMI1_RGMII2;
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
