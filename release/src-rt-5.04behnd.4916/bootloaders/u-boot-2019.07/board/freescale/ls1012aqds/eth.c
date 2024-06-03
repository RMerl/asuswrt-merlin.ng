// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/types.h>
#include <fsl_dtsec.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/config.h>
#include <asm/arch-fsl-layerscape/immap_lsch2.h>
#include <asm/arch/fsl_serdes.h>
#include "../common/qixis.h"
#include <net/pfe_eth/pfe_eth.h>
#include <dm/platform_data/pfe_dm_eth.h>
#include "ls1012aqds_qixis.h"

#define EMI_NONE	0xFF
#define EMI1_RGMII	1
#define EMI1_SLOT1	2
#define EMI1_SLOT2	3

#define DEFAULT_PFE_MDIO_NAME "PFE_MDIO"
#define DEFAULT_PFE_MDIO1_NAME "PFE_MDIO1"

static const char * const mdio_names[] = {
	"NULL",
	"LS1012AQDS_MDIO_RGMII",
	"LS1012AQDS_MDIO_SLOT1",
	"LS1012AQDS_MDIO_SLOT2",
	"NULL",
};

static const char *ls1012aqds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct ls1012aqds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static void ls1012aqds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;

	if (muxval < 7) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int ls1012aqds_mdio_read(struct mii_dev *bus, int addr, int devad,
				int regnum)
{
	struct ls1012aqds_mdio *priv = bus->priv;

	ls1012aqds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int ls1012aqds_mdio_write(struct mii_dev *bus, int addr, int devad,
				 int regnum, u16 value)
{
	struct ls1012aqds_mdio *priv = bus->priv;

	ls1012aqds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int ls1012aqds_mdio_reset(struct mii_dev *bus)
{
	struct ls1012aqds_mdio *priv = bus->priv;

	if (priv->realbus->reset)
		return priv->realbus->reset(priv->realbus);
	else
		return -1;
}

static int ls1012aqds_mdio_init(char *realbusname, u8 muxval)
{
	struct ls1012aqds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate ls1012aqds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate ls1012aqds private data\n");
		free(bus);
		return -1;
	}

	bus->read = ls1012aqds_mdio_read;
	bus->write = ls1012aqds_mdio_write;
	bus->reset = ls1012aqds_mdio_reset;
	sprintf(bus->name, ls1012aqds_mdio_name_for_muxval(muxval));

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

int pfe_eth_board_init(struct udevice *dev)
{
	static int init_done;
	struct mii_dev *bus;
	static const char *mdio_name;
	struct pfe_mdio_info mac_mdio_info;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	u8 data8;
	struct pfe_eth_dev *priv = dev_get_priv(dev);

	int srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	ls1012aqds_mux_mdio(EMI1_SLOT1);

	if (!init_done) {
		mac_mdio_info.reg_base = (void *)EMAC1_BASE_ADDR;
		mac_mdio_info.name = DEFAULT_PFE_MDIO_NAME;

		bus = pfe_mdio_init(&mac_mdio_info);
		if (!bus) {
			printf("Failed to register mdio\n");
			return -1;
		}
		init_done = 1;
	}

	if (priv->gemac_port) {
		mac_mdio_info.reg_base = (void *)EMAC2_BASE_ADDR;
		mac_mdio_info.name = DEFAULT_PFE_MDIO1_NAME;

		bus = pfe_mdio_init(&mac_mdio_info);
		if (!bus) {
			printf("Failed to register mdio\n");
			return -1;
		}
	}

	switch (srds_s1) {
	case 0x3508:
		printf("ls1012aqds:supported SerDes PRCTL= %d\n", srds_s1);
#ifdef CONFIG_PFE_RGMII_RESET_WA
		/*
		 * Work around for FPGA registers initialization
		 * This is needed for RGMII to work.
		 */
		printf("Reset RGMII WA....\n");
		data8 = QIXIS_READ(rst_frc[0]);
		data8 |= 0x2;
		QIXIS_WRITE(rst_frc[0], data8);
		data8 = QIXIS_READ(rst_frc[0]);

		data8 = QIXIS_READ(res8[6]);
		data8 |= 0xff;
		QIXIS_WRITE(res8[6], data8);
		data8 = QIXIS_READ(res8[6]);
#endif
	if (priv->gemac_port) {
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_RGMII);
		if (ls1012aqds_mdio_init(DEFAULT_PFE_MDIO_NAME, EMI1_RGMII)
		    < 0) {
			printf("Failed to register mdio for %s\n", mdio_name);
		}

		/* MAC2 */
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_RGMII);
		bus = miiphy_get_dev_by_name(mdio_name);
		pfe_set_mdio(priv->gemac_port, bus);
		pfe_set_phy_address_mode(priv->gemac_port,
					 CONFIG_PFE_EMAC2_PHY_ADDR,
					 PHY_INTERFACE_MODE_RGMII);

	} else {
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT1);
		if (ls1012aqds_mdio_init(DEFAULT_PFE_MDIO_NAME, EMI1_SLOT1)
		< 0) {
			printf("Failed to register mdio for %s\n", mdio_name);
		}

		/* MAC1 */
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT1);
		bus = miiphy_get_dev_by_name(mdio_name);
		pfe_set_mdio(priv->gemac_port, bus);
		pfe_set_phy_address_mode(priv->gemac_port,
					 CONFIG_PFE_EMAC1_PHY_ADDR,
					 PHY_INTERFACE_MODE_SGMII);
	}

		break;

	case 0x2205:
		printf("ls1012aqds:supported SerDes PRCTL= %d\n", srds_s1);
		/*
		 * Work around for FPGA registers initialization
		 * This is needed for RGMII to work.
		 */
		printf("Reset SLOT1 SLOT2....\n");
		data8 = QIXIS_READ(rst_frc[2]);
		data8 |= 0xc0;
		QIXIS_WRITE(rst_frc[2], data8);
		mdelay(100);
		data8 = QIXIS_READ(rst_frc[2]);
		data8 &= 0x3f;
		QIXIS_WRITE(rst_frc[2], data8);

	if (priv->gemac_port) {
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT2);
		if (ls1012aqds_mdio_init(DEFAULT_PFE_MDIO_NAME, EMI1_SLOT2)
		< 0) {
			printf("Failed to register mdio for %s\n", mdio_name);
		}
		/* MAC2 */
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT2);
		bus = miiphy_get_dev_by_name(mdio_name);
		pfe_set_mdio(1, bus);
		pfe_set_phy_address_mode(1, CONFIG_PFE_SGMII_2500_PHY2_ADDR,
					 PHY_INTERFACE_MODE_SGMII_2500);

		data8 = QIXIS_READ(brdcfg[12]);
		data8 |= 0x20;
		QIXIS_WRITE(brdcfg[12], data8);

	} else {
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT1);
		if (ls1012aqds_mdio_init(DEFAULT_PFE_MDIO_NAME, EMI1_SLOT1)
		    < 0) {
			printf("Failed to register mdio for %s\n", mdio_name);
		}

		/* MAC1 */
		mdio_name = ls1012aqds_mdio_name_for_muxval(EMI1_SLOT1);
		bus = miiphy_get_dev_by_name(mdio_name);
		pfe_set_mdio(0, bus);
		pfe_set_phy_address_mode(0,
					 CONFIG_PFE_SGMII_2500_PHY1_ADDR,
					 PHY_INTERFACE_MODE_SGMII_2500);
	}
		break;

	default:
		printf("ls1012aqds:unsupported SerDes PRCTL= %d\n", srds_s1);
		break;
	}
	return 0;
}

static struct pfe_eth_pdata pfe_pdata0 = {
	.pfe_eth_pdata_mac = {
		.iobase = (phys_addr_t)EMAC1_BASE_ADDR,
		.phy_interface = 0,
	},

	.pfe_ddr_addr = {
		.ddr_pfe_baseaddr = (void *)CONFIG_DDR_PFE_BASEADDR,
		.ddr_pfe_phys_baseaddr = CONFIG_DDR_PFE_PHYS_BASEADDR,
	},
};

static struct pfe_eth_pdata pfe_pdata1 = {
	.pfe_eth_pdata_mac = {
		.iobase = (phys_addr_t)EMAC2_BASE_ADDR,
		.phy_interface = 1,
	},

	.pfe_ddr_addr = {
		.ddr_pfe_baseaddr = (void *)CONFIG_DDR_PFE_BASEADDR,
		.ddr_pfe_phys_baseaddr = CONFIG_DDR_PFE_PHYS_BASEADDR,
	},
};

U_BOOT_DEVICE(ls1012a_pfe0) = {
	.name = "pfe_eth",
	.platdata = &pfe_pdata0,
};

U_BOOT_DEVICE(ls1012a_pfe1) = {
	.name = "pfe_eth",
	.platdata = &pfe_pdata1,
};
