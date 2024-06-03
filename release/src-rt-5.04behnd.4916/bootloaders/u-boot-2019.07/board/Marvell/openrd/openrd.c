// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Net Insight <www.netinsight.net>
 * Written-by: Simon Kagstrom <simon.kagstrom@netinsight.net>
 *
 * Based on sheevaplug.c:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <miiphy.h>
#include <asm/mach-types.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include "openrd.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(OPENRD_OE_VAL_LOW,
			  OPENRD_OE_VAL_HIGH,
			  OPENRD_OE_LOW, OPENRD_OE_HIGH);

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
		MPP12_SD_CLK,
		MPP13_SD_CMD, /* Alt UART1_TXD */
		MPP14_SD_D0,  /* Alt UART1_RXD */
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GE1_0,
		MPP21_GE1_1,
		MPP22_GE1_2,
		MPP23_GE1_3,
		MPP24_GE1_4,
		MPP25_GE1_5,
		MPP26_GE1_6,
		MPP27_GE1_7,
		MPP28_GPIO,
		MPP29_TSMP9,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GPIO,   /* UART1 / SD sel */
		MPP35_TDM_CH0_TX_QL,
		MPP36_TDM_SPI_CS1,
		MPP37_TDM_CH2_TX_QL,
		MPP38_TDM_CH2_RX_QL,
		MPP39_AUDIO_I2SBCLK,
		MPP40_AUDIO_I2SDO,
		MPP41_AUDIO_I2SLRC,
		MPP42_AUDIO_I2SMCLK,
		MPP43_AUDIO_I2SDI,
		MPP44_AUDIO_EXTCLK,
		MPP45_TDM_PCLK,
		MPP46_TDM_FS,
		MPP47_TDM_DRX,
		MPP48_TDM_DTX,
		MPP49_TDM_CH0_RX_QL,
		0
	};

	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/*
	 * arch number of board
	 */
#if defined(CONFIG_BOARD_IS_OPENRD_BASE)
	gd->bd->bi_arch_number = MACH_TYPE_OPENRD_BASE;
#elif defined(CONFIG_BOARD_IS_OPENRD_CLIENT)
	gd->bd->bi_arch_number = MACH_TYPE_OPENRD_CLIENT;
#elif defined(CONFIG_BOARD_IS_OPENRD_ULTIMATE)
	gd->bd->bi_arch_number = MACH_TYPE_OPENRD_ULTIMATE;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
/* Configure and enable MV88E1116/88E1121 PHY */
void mv_phy_init(char *name)
{
	u16 reg;
	u16 devadr;

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, (u16 *)&devadr)) {
		printf("Err..%s could not read PHY dev address\n", __func__);
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

	printf(PHY_NO" Initialized on %s\n", name);
}

void reset_phy(void)
{
	mv_phy_init("egiga0");

#ifdef CONFIG_BOARD_IS_OPENRD_CLIENT
	/* Kirkwood ethernet driver is written with the assumption that in case
	 * of multiple PHYs, their addresses are consecutive. But unfortunately
	 * in case of OpenRD-Client, PHY addresses are not consecutive.*/
	miiphy_write("egiga1", 0xEE, 0xEE, 24);
#endif

#if defined(CONFIG_BOARD_IS_OPENRD_CLIENT) || \
	defined(CONFIG_BOARD_IS_OPENRD_ULTIMATE)
	/* configure and initialize both PHY's */
	mv_phy_init("egiga1");
#endif
}
#endif /* CONFIG_RESET_PHY_R */
