// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010, 2018
 * Allied Telesis <www.alliedtelesis.com>
 */

#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <led.h>
#include <linux/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>

#define SBX81LIFXCAT_OE_LOW		(~0)
#define SBX81LIFXCAT_OE_HIGH		(~BIT(11))
#define SBX81LIFXCAT_OE_VAL_LOW		(0)
#define SBX81LIFXCAT_OE_VAL_HIGH	(BIT(11))

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(SBX81LIFXCAT_OE_VAL_LOW,
			  SBX81LIFXCAT_OE_VAL_HIGH,
			  SBX81LIFXCAT_OE_LOW, SBX81LIFXCAT_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_UART1_TXD,
		MPP14_UART1_RXD,
		MPP15_GPIO,
		MPP16_GPIO,
		MPP17_GPIO,
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
		MPP28_GE1_8,
		MPP29_GE1_9,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GPIO,
		MPP35_GPIO,
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

#ifdef CONFIG_RESET_PHY_R
/* automatically defined by kirkwood config.h */
void reset_phy(void)
{
}
#endif

#ifdef CONFIG_MV88E61XX_SWITCH
int mv88e61xx_hw_reset(struct phy_device *phydev)
{
	phydev->advertising = ADVERTISED_10baseT_Half | ADVERTISED_10baseT_Full;

	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	struct udevice *dev;
	int ret;

	ret = led_get_by_label("status:ledp", &dev);
	if (!ret)
		led_set_state(dev, LEDST_ON);

	ret = led_get_by_label("status:ledn", &dev);
	if (!ret)
		led_set_state(dev, LEDST_OFF);

	return 0;
}
#endif
