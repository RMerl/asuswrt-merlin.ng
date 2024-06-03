// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <micrel.h>
#include <nand.h>
#include <netdev.h>
#include <phy.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_defs.h>
#include <asm/arch/spr_misc.h>
#include <linux/mtd/fsmc_nand.h>
#include "fpga.h"

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];

int board_init(void)
{
	/*
	 * X600 is equipped with an M41T82 RTC. This RTC has the
	 * HT bit (Halt Update), which needs to be cleared upon
	 * power-up. Otherwise the RTC is halted.
	 */
	rtc_reset();

	return spear_board_init(MACH_TYPE_SPEAR600);
}

int board_late_init(void)
{
	/*
	 * Monitor and env protection on by default
	 */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE +
		      CONFIG_SYS_SPL_LEN + CONFIG_SYS_MONITOR_LEN +
		      2 * CONFIG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);

	/* Init FPGA subsystem */
	x600_init_fpga();

	return 0;
}

/*
 * board_nand_init - Board specific NAND initialization
 * @nand:	mtd private chip structure
 *
 * Called by nand_init_chip to initialize the board specific functions
 */

void board_nand_init(void)
{
	struct misc_regs *const misc_regs_p =
		(struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	struct nand_chip *nand = &nand_chip[0];

	if (!(readl(&misc_regs_p->auto_cfg_reg) & MISC_NANDDIS))
		fsmc_nand_init(nand);
}

int board_phy_config(struct phy_device *phydev)
{
	unsigned short id1, id2;

	/* check whether KSZ9031 or AR8035 has to be configured */
	id1 = phy_read(phydev, MDIO_DEVAD_NONE, 2);
	id2 = phy_read(phydev, MDIO_DEVAD_NONE, 3);

	if ((id1 == 0x22) && ((id2 & 0xFFF0) == 0x1620)) {
		/* PHY configuration for Micrel KSZ9031 */
		printf("PHY KSZ9031 detected - ");

		phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, 0x1c00);

		/* control data pad skew - devaddr = 0x02, register = 0x04 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x0000);
		/* rx data pad skew - devaddr = 0x02, register = 0x05 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x0000);
		/* tx data pad skew - devaddr = 0x02, register = 0x05 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x0000);
		/* gtx and rx clock pad skew - devaddr = 0x02, reg = 0x08 */
		ksz9031_phy_extended_write(phydev, 0x02,
					   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
					   MII_KSZ9031_MOD_DATA_NO_POST_INC,
					   0x03FF);
	} else {
		/* PHY configuration for Vitesse VSC8641 */
		printf("PHY VSC8641 detected - ");

		/* Extended PHY control 1, select GMII */
		phy_write(phydev, MDIO_DEVAD_NONE, 23, 0x0020);

		/* Software reset necessary after GMII mode selction */
		phy_reset(phydev);

		/* Enable extended page register access */
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0001);

		/* 17e: Enhanced LED behavior, needs to be written twice */
		phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x09ff);
		phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x09ff);

		/* 16e: Enhanced LED method select */
		phy_write(phydev, MDIO_DEVAD_NONE, 16, 0xe0ea);

		/* Disable extended page register access */
		phy_write(phydev, MDIO_DEVAD_NONE, 31, 0x0000);

		/* Enable clock output pin */
		phy_write(phydev, MDIO_DEVAD_NONE, 18, 0x0049);
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

	if (designware_initialize(CONFIG_SPEAR_ETHBASE,
				  PHY_INTERFACE_MODE_GMII) >= 0)
		ret++;

	return ret;
}
