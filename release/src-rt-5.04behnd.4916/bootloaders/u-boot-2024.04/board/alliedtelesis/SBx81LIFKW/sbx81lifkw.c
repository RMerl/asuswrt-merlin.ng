// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010, 2018
 * Allied Telesis <www.alliedtelesis.com>
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <led.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm-generic/gpio.h>
#include <dm.h>

/* Note: GPIO differences between specific boards
 *
 * We're trying to avoid having multiple build targets for all the Kirkwood
 * based boards one area where things tend to differ is GPIO usage. For the
 * most part the GPIOs driven by the bootloader are similar enough in function
 * that there is no harm in driving them.
 *
 *         XZ4  XS6     XS16  GS24A         GT40   GP24A         GT24A
 * GPIO39  -    INT(<)  NC    MUX_RST_N(>)  NC     POE_DIS_N(>)  NC
 */

#define SBX81LIFKW_OE_LOW	~(BIT(31) | BIT(30) | BIT(28) | BIT(27) | \
				  BIT(18) | BIT(17) | BIT(13) | BIT(12) | \
				  BIT(10))
#define SBX81LIFKW_OE_HIGH	~(BIT(0) | BIT(1) | BIT(7))
#define SBX81LIFKW_OE_VAL_LOW	 (BIT(31) | BIT(30) | BIT(28) | BIT(27))
#define SBX81LIFKW_OE_VAL_HIGH	 (BIT(0) | BIT(1))

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(SBX81LIFKW_OE_VAL_LOW,
			  SBX81LIFKW_OE_VAL_HIGH,
			  SBX81LIFKW_OE_LOW, SBX81LIFKW_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_UART0_RXD,
		MPP5_UART0_TXD,
		MPP6_SYSRST_OUTn,
		MPP7_PEX_RST_OUTn,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_GPO,
		MPP11_GPIO,
		MPP12_GPO,
		MPP13_GPIO,
		MPP14_GPIO,
		MPP15_UART0_RTS,
		MPP16_UART0_CTS,
		MPP17_GPIO,
		MPP18_GPO,
		MPP19_GPO,
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
	/* Power-down unused subsystems. The required
	 * subsystems are:
	 *
	 *  GE0         b0
	 *  PEX0 PHY    b1
	 *  PEX0.0      b2
	 *  TSU         b5
	 *  SDRAM       b6
	 *  RUNIT       b7
	 */
	writel((BIT(0) | BIT(1) | BIT(2) |
		BIT(5) | BIT(6) | BIT(7)),
		KW_CPU_REG_BASE + 0x1c);

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
	struct gpio_desc desc;
	int ret;

	ret = dm_gpio_lookup_name("mvebu0_27", &desc);
	if (ret)
		return ret;

	ret = dm_gpio_request(&desc, "linkstreet_rst");
	if (ret)
		return ret;

	/* Ensure the 88e6097 gets at least 10ms Reset
	 */
	dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	dm_gpio_set_value(&desc, 0);
	mdelay(20);
	dm_gpio_set_value(&desc, 1);
	mdelay(20);

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
