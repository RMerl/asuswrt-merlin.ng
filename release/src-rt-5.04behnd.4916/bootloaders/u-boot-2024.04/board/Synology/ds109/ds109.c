// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009-2012
 * Wojciech Dubowik <wojciech.dubowik@neratec.com>
 * Luka Perkov <luka@openwrt.org>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <linux/delay.h>
#include "ds109.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(DS109_OE_VAL_LOW,
			  DS109_OE_VAL_HIGH,
			  DS109_OE_LOW, DS109_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,		/* SPI Flash */
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_GPIO,
		MPP5_GPO,
		MPP6_SYSRST_OUTn,	/* Reset signal */
		MPP7_GPO,
		MPP8_TW_SDA,		/* I2C */
		MPP9_TW_SCK,		/* I2C */
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_UART1_TXD,
		MPP14_UART1_RXD,
		MPP15_GPIO,
		MPP16_GPIO,
		MPP17_GPIO,
		MPP18_GPO,
		MPP19_GPO,
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP22_GPIO,		/* HDD2 FAIL LED */
		MPP23_GPIO,		/* HDD1 FAIL LED */
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GPIO,
		MPP31_GPIO,		/* HDD2 */
		MPP32_GPIO,		/* FAN A */
		MPP33_GPIO,		/* FAN B */
		MPP34_GPIO,		/* FAN C */
		MPP35_GPIO,		/* FAN SENSE */
		MPP36_GPIO,
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
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

/* Synology reset uses UART */
#include <ns16550.h>
#define SOFTWARE_SHUTDOWN   0x31
#define SOFTWARE_REBOOT     0x43
#define CFG_SYS_NS16550_COM2		KW_UART1_BASE
void reset_misc(void)
{
	int b_d;
	printf("Synology reset...");
	udelay(50000);

	b_d = ns16550_calc_divisor((struct ns16550 *)CFG_SYS_NS16550_COM2,
				   CFG_SYS_NS16550_CLK, 9600);
	ns16550_init((struct ns16550 *)CFG_SYS_NS16550_COM2, b_d);
	ns16550_putc((struct ns16550 *)CFG_SYS_NS16550_COM2,
		     SOFTWARE_REBOOT);
}

#ifdef CONFIG_RESET_PHY_R
/* Configure and enable MV88E1116 PHY */
void reset_phy(void)
{
	u16 reg;
	u16 devadr;
	char *name = "egiga0";

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, (u16 *)&devadr)) {
		printf("Error: 88E1116 could not read PHY dev address\n");
		return;
	}

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, devadr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, devadr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	miiphy_reset(name, devadr);

	printf("88E1116 Initialized on %s\n", name);
}
#endif /* CONFIG_RESET_PHY_R */
