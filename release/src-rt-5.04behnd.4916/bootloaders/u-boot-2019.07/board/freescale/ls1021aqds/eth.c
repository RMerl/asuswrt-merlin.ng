// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * This file handles the board muxing between the RGMII/SGMII PHYs on
 * Freescale LS1021AQDS board. The RGMII PHYs are the three on-board 1Gb
 * ports. The SGMII PHYs are provided by the standard Freescale four-port
 * SGMII riser card.
 *
 * Muxing is handled via the PIXIS BRDCFG4 register. The EMI1 bits control
 * muxing among the RGMII PHYs and the SGMII PHYs. The value for RGMII depends
 * on which port is used. The value for SGMII depends on which slot the riser
 * is inserted in.
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <malloc.h>

#include "../common/sgmii_riser.h"
#include "../common/qixis.h"

#define EMI1_MASK       0x1f
#define EMI1_RGMII0     1
#define EMI1_RGMII1     2
#define EMI1_RGMII2     3
#define EMI1_SGMII1     0x1c
#define EMI1_SGMII2     0x1d

struct ls1021a_mdio {
	struct mii_dev *realbus;
};

static void ls1021a_mux_mdio(int addr)
{
	u8 brdcfg4;

	brdcfg4 = QIXIS_READ(brdcfg[4]);
	brdcfg4 &= EMI1_MASK;

	switch (addr) {
	case EMI1_RGMII0:
		brdcfg4 |= 0;
		break;
	case EMI1_RGMII1:
		brdcfg4 |= 0x20;
		break;
	case EMI1_RGMII2:
		brdcfg4 |= 0x40;
		break;
	case EMI1_SGMII1:
		brdcfg4 |= 0x60;
		break;
	case EMI1_SGMII2:
		brdcfg4 |= 0x80;
		break;
	default:
		brdcfg4 |= 0xa0;
		break;
	}

	QIXIS_WRITE(brdcfg[4], brdcfg4);
}

static int ls1021a_mdio_read(struct mii_dev *bus, int addr, int devad,
			     int regnum)
{
	struct ls1021a_mdio *priv = bus->priv;

	ls1021a_mux_mdio(addr);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int ls1021a_mdio_write(struct mii_dev *bus, int addr, int devad,
			      int regnum, u16 value)
{
	struct ls1021a_mdio *priv = bus->priv;

	ls1021a_mux_mdio(addr);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int ls1021a_mdio_reset(struct mii_dev *bus)
{
	struct ls1021a_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int ls1021a_mdio_init(char *realbusname, char *fakebusname)
{
	struct ls1021a_mdio *lsmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate LS102xA MDIO bus\n");
		return -1;
	}

	lsmdio = malloc(sizeof(*lsmdio));
	if (!lsmdio) {
		printf("Failed to allocate LS102xA private data\n");
		free(bus);
		return -1;
	}

	bus->read = ls1021a_mdio_read;
	bus->write = ls1021a_mdio_write;
	bus->reset = ls1021a_mdio_reset;
	strcpy(bus->name, fakebusname);

	lsmdio->realbus = miiphy_get_dev_by_name(realbusname);

	if (!lsmdio->realbus) {
		printf("No bus with name %s\n", realbusname);
		free(bus);
		free(lsmdio);
		return -1;
	}

	bus->priv = lsmdio;

	return mdio_register(bus);
}

int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[3];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if (is_serdes_configured(SGMII_TSEC1)) {
		puts("eTSEC1 is in sgmii mode\n");
		tsec_info[num].flags |= TSEC_SGMII;
		tsec_info[num].mii_devname = "LS1021A_SGMII_MDIO";
	} else {
		tsec_info[num].mii_devname = "LS1021A_RGMII_MDIO";
	}
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	if (is_serdes_configured(SGMII_TSEC2)) {
		puts("eTSEC2 is in sgmii mode\n");
		tsec_info[num].flags |= TSEC_SGMII;
		tsec_info[num].mii_devname = "LS1021A_SGMII_MDIO";
	} else {
		tsec_info[num].mii_devname = "LS1021A_RGMII_MDIO";
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	tsec_info[num].mii_devname = "LS1021A_RGMII_MDIO";
	num++;
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_init(tsec_info, num);
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	/* Register the virtual MDIO front-ends */
	ls1021a_mdio_init(DEFAULT_MII_NAME, "LS1021A_RGMII_MDIO");
	ls1021a_mdio_init(DEFAULT_MII_NAME, "LS1021A_SGMII_MDIO");

	tsec_eth_init(bis, tsec_info, num);

	return pci_eth_init(bis);
}
