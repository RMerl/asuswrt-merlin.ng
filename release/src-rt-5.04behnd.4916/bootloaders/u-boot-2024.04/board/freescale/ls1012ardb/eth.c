// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <common.h>
#include <dm.h>
#include <net.h>
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
#include <linux/delay.h>
#include <net/pfe_eth/pfe_eth.h>
#include <dm/platform_data/pfe_dm_eth.h>
#include <i2c.h>

#define DEFAULT_PFE_MDIO_NAME "PFE_MDIO"

static inline void ls1012ardb_reset_phy(void)
{
#ifdef CONFIG_TARGET_LS1012ARDB
	/* Through reset IO expander reset both RGMII and SGMII PHYs */
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	/*
	 * The I2C IO-expander PCAL9555A is mouted on I2C1 bus(bus number is 0).
	 */
	ret = i2c_get_chip_for_busnum(0, I2C_MUX_IO2_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       0);
		return;
	}
	/* Config port 0
	 * - config pin IOXP_RST_ETH1_B and IOXP_RST_ETH2_B
	 *   are enabled as an output.
	 */
	dm_i2c_reg_write(dev, 6, __PHY_MASK);

	/*
	 * Set port 0 output a value to reset ETH2 interface
	 * - pin IOXP_RST_ETH2_B output 0b0
	 */
	dm_i2c_reg_write(dev, 2, __PHY_ETH2_MASK);
	mdelay(10);
	dm_i2c_reg_write(dev, 2, __PHY_ETH1_MASK);
	/*
	 * Set port 0 output a value to reset ETH1 interface
	 * - pin IOXP_RST_ETH1_B output 0b0
	 */
	mdelay(10);
	dm_i2c_reg_write(dev, 2, 0xFF);
#else
	i2c_reg_write(I2C_MUX_IO2_ADDR, 6, __PHY_MASK);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, __PHY_ETH2_MASK);
	mdelay(10);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, __PHY_ETH1_MASK);
	mdelay(10);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, 0xFF);
#endif
	mdelay(50);
#endif
}

int pfe_eth_board_init(struct udevice *dev)
{
	static int init_done;
	struct mii_dev *bus;
	struct pfe_mdio_info mac_mdio_info;
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	int srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	if (!init_done) {
		ls1012ardb_reset_phy();
		mac_mdio_info.reg_base = (void *)EMAC1_BASE_ADDR;
		mac_mdio_info.name = DEFAULT_PFE_MDIO_NAME;

		bus = pfe_mdio_init(&mac_mdio_info);
		if (!bus) {
			printf("Failed to register mdio\n");
			return -1;
		}
		init_done = 1;
	}

	pfe_set_mdio(priv->gemac_port,
		     miiphy_get_dev_by_name(DEFAULT_PFE_MDIO_NAME));

	switch (srds_s1) {
	case 0x3508:
		if (!priv->gemac_port) {
			/* MAC1 */
			pfe_set_phy_address_mode(priv->gemac_port,
						 CONFIG_PFE_EMAC1_PHY_ADDR,
						 PHY_INTERFACE_MODE_SGMII);
		} else {
			/* MAC2 */
			pfe_set_phy_address_mode(priv->gemac_port,
						 CONFIG_PFE_EMAC2_PHY_ADDR,
						 PHY_INTERFACE_MODE_RGMII_ID);
		}
		break;
	case 0x2208:
		if (!priv->gemac_port) {
			/* MAC1 */
			pfe_set_phy_address_mode(priv->gemac_port,
						 CONFIG_PFE_EMAC1_PHY_ADDR,
						 PHY_INTERFACE_MODE_2500BASEX);
		} else {
			/* MAC2 */
			pfe_set_phy_address_mode(priv->gemac_port,
						 CONFIG_PFE_EMAC2_PHY_ADDR,
						 PHY_INTERFACE_MODE_2500BASEX);
		}
		break;
	default:
		printf("unsupported SerDes PRCTL= %d\n", srds_s1);
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

U_BOOT_DRVINFO(ls1012a_pfe0) = {
	.name = "pfe_eth",
	.plat = &pfe_pdata0,
};

U_BOOT_DRVINFO(ls1012a_pfe1) = {
	.name = "pfe_eth",
	.plat = &pfe_pdata1,
};
