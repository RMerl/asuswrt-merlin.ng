// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#define ETH_PHY_CTRL_REG		0
#define ETH_PHY_CTRL_POWER_DOWN_BIT	11
#define ETH_PHY_CTRL_POWER_DOWN_MASK	(1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2011.12-2014_T1.0" for the board rd78460gp aka
 * "RD-AXP-GP rev 1.0".
 *
 * GPPs
 * MPP#		NAME			IN/OUT
 * ----------------------------------------------
 * 21		SW_Reset_		OUT
 * 25		Phy_Int#		IN
 * 28		SDI_WP			IN
 * 29		SDI_Status		IN
 * 54-61	On GPP Connector	?
 * 62		Switch Interrupt	IN
 * 63-65	Reserved from SW Board	?
 * 66		SW_BRD connected	IN
 */
#define RD_78460_GP_GPP_OUT_ENA_LOW	(~(BIT(21) | BIT(20)))
#define RD_78460_GP_GPP_OUT_ENA_MID	(~(BIT(26) | BIT(27)))
#define RD_78460_GP_GPP_OUT_ENA_HIGH	(~(0x0))

#define RD_78460_GP_GPP_OUT_VAL_LOW	(BIT(21) | BIT(20))
#define RD_78460_GP_GPP_OUT_VAL_MID	(BIT(26) | BIT(27))
#define RD_78460_GP_GPP_OUT_VAL_HIGH	0x0

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x00000000, MVEBU_MPP_BASE + 0x00);
	writel(0x00000000, MVEBU_MPP_BASE + 0x04);
	writel(0x33000000, MVEBU_MPP_BASE + 0x08);
	writel(0x11000000, MVEBU_MPP_BASE + 0x0c);
	writel(0x11111111, MVEBU_MPP_BASE + 0x10);
	writel(0x00221100, MVEBU_MPP_BASE + 0x14);
	writel(0x00000003, MVEBU_MPP_BASE + 0x18);
	writel(0x00000000, MVEBU_MPP_BASE + 0x1c);
	writel(0x00000000, MVEBU_MPP_BASE + 0x20);

	/* Configure GPIO */
	writel(RD_78460_GP_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(RD_78460_GP_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(RD_78460_GP_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);
	writel(RD_78460_GP_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
	writel(RD_78460_GP_GPP_OUT_VAL_HIGH, MVEBU_GPIO2_BASE + 0x00);
	writel(RD_78460_GP_GPP_OUT_ENA_HIGH, MVEBU_GPIO2_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: Marvell DB-MV784MP-GP\n");

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}

int board_phy_config(struct phy_device *phydev)
{
	u16 reg;

	/* Enable QSGMII AN */
	/* Set page to 4 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 4);
	/* Enable AN */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0, 0x1140);
	/* Set page to 0 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0);

	/* Phy C_ANEG */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 0x4);
	reg |= 0x1E0;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x4, reg);

	/* Soft-Reset */
	phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE, 0, 0x9140);

	/* Power up the phy */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, ETH_PHY_CTRL_REG);
	reg &= ~(ETH_PHY_CTRL_POWER_DOWN_MASK);
	phy_write(phydev, MDIO_DEVAD_NONE, ETH_PHY_CTRL_REG, reg);

	printf("88E1545 Initialized\n");
	return 0;
}
