// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015
 * Gerald Kerma <dreagle@doukki.net>
 * Tony Dinh <mibodhi@gmail.com>
 */

#include <common.h>
#include <miiphy.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/io.h>
#include "nsa310s.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(NSA310S_VAL_LOW, NSA310S_VAL_HIGH,
			  NSA310S_OE_LOW, NSA310S_OE_HIGH);

	/* (all LEDs & power off active high) */
	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_GPIO,
		MPP14_GPIO,
		MPP15_GPIO,
		MPP16_GPIO,
		MPP17_GPIO,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GPIO,
		MPP21_GPIO,
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
	u16 reg;
	u16 phyaddr;
	char *name = "egiga0";

	if (miiphy_set_current_dev(name))
		return;

	/* read PHY dev address */
	if (miiphy_read(name, 0xee, 0xee, (u16 *) &phyaddr)) {
		printf("could not read PHY dev address\n");
		return;
	}

	/* set RGMII delay */
	miiphy_write(name, phyaddr, MV88E1318_PGADR_REG, MV88E1318_MAC_CTRL_PG);
	miiphy_read(name, phyaddr, MV88E1318_MAC_CTRL_REG, &reg);
	reg |= (MV88E1318_RGMII_RX_CTRL | MV88E1318_RGMII_TX_CTRL);
	miiphy_write(name, phyaddr, MV88E1318_MAC_CTRL_REG, reg);
	miiphy_write(name, phyaddr, MV88E1318_PGADR_REG, 0);

	/* reset PHY */
	if (miiphy_reset(name, phyaddr))
		return;

	/*
	 * ZyXEL NSA310S uses the 88E1310S Alaska (interface identical to 88E1318)
	 * and has an MCU attached to the LED[2] via tristate interrupt
	 */

	/* switch to LED register page */
	miiphy_write(name, phyaddr, MV88E1318_PGADR_REG, MV88E1318_LED_PG);
	/* read out LED polarity register */
	miiphy_read(name, phyaddr, MV88E1318_LED_POL_REG, &reg);
	/* clear 4, set 5 - LED2 low, tri-state */
	reg &= ~(MV88E1318_LED2_4);
	reg |= (MV88E1318_LED2_5);
	/* write back LED polarity register */
	miiphy_write(name, phyaddr, MV88E1318_LED_POL_REG, reg);
	/* jump back to page 0, per the PHY chip documenation. */
	miiphy_write(name, phyaddr, MV88E1318_PGADR_REG, 0);

	/* set PHY back to auto-negotiation mode */
	miiphy_write(name, phyaddr, 0x4, 0x1e1);
	miiphy_write(name, phyaddr, 0x9, 0x300);
	/* downshift */
	miiphy_write(name, phyaddr, 0x10, 0x3860);
	miiphy_write(name, phyaddr, 0x0, 0x9140);
}
#endif /* CONFIG_RESET_PHY_R */
