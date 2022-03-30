// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Barco (www.barco.com)
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <miiphy.h>
#include <micrel.h>

#include "platinum.h"

iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK		| MUX_PAD_CTRL(ECSPI1_PAD_CLK),
	MX6_PAD_EIM_D17__ECSPI1_MISO		| MUX_PAD_CTRL(ECSPI_PAD_MISO),
	MX6_PAD_EIM_D18__ECSPI1_MOSI		| MUX_PAD_CTRL(ECSPI_PAD_MOSI),
	MX6_PAD_CSI0_DAT7__ECSPI1_SS0		| MUX_PAD_CTRL(ECSPI_PAD_SS),
	/* non mounted spi nor flash for booting */
	MX6_PAD_EIM_D19__ECSPI1_SS1		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D24__ECSPI1_SS2		| MUX_PAD_CTRL(ECSPI_PAD_SS),
	MX6_PAD_EIM_D25__ECSPI1_SS3		| MUX_PAD_CTRL(ECSPI_PAD_SS),
};

iomux_v3_cfg_t const ecspi2_pads[] = {
	MX6_PAD_EIM_CS0__ECSPI2_SCLK		| MUX_PAD_CTRL(ECSPI2_PAD_CLK),
	MX6_PAD_EIM_OE__ECSPI2_MISO		| MUX_PAD_CTRL(ECSPI_PAD_MISO),
	MX6_PAD_EIM_CS1__ECSPI2_MOSI		| MUX_PAD_CTRL(ECSPI_PAD_MOSI),
	MX6_PAD_EIM_RW__ECSPI2_SS0		| MUX_PAD_CTRL(ECSPI_PAD_SS),
};

iomux_v3_cfg_t const enet_pads1[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* pin 35 - 1 (PHY_AD2) on reset */
	MX6_PAD_RGMII_RXC__GPIO6_IO30		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 32 - 1 - (MODE0) all */
	MX6_PAD_RGMII_RD0__GPIO6_IO25		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 31 - 1 - (MODE1) all */
	MX6_PAD_RGMII_RD1__GPIO6_IO27		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 28 - 1 - (MODE2) all */
	MX6_PAD_RGMII_RD2__GPIO6_IO28		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 27 - 1 - (MODE3) all */
	MX6_PAD_RGMII_RD3__GPIO6_IO29		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
	MX6_PAD_RGMII_RX_CTL__GPIO6_IO24	| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 42 PHY nRST */
	MX6_PAD_EIM_D23__GPIO3_IO23		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads2[] = {
	MX6_PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA   | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA   | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D28__UART2_DTE_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D29__UART2_RTS_B     | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_CSI0_DAT12__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT13__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT16__UART4_RTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT17__UART4_CTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
};

struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode  = MX6_PAD_CSI0_DAT9__I2C1_SCL	| PC_SCL,
		.gpio_mode = MX6_PAD_CSI0_DAT9__GPIO5_IO27	| PC_SCL,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6_PAD_CSI0_DAT8__I2C1_SDA		| PC,
		.gpio_mode = MX6_PAD_CSI0_DAT8__GPIO5_IO26	| PC,
		.gp = IMX_GPIO_NR(5, 26)
	 }
};

struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL		| PC_SCL,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03		| PC_SCL,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA		| PC,
		.gpio_mode = MX6_PAD_GPIO_16__GPIO7_IO11	| PC,
		.gp = IMX_GPIO_NR(7, 11)
	 }
};

/*
 * This enet related pin-muxing and GPIO handling is done
 * in SPL U-Boot. For early initialization. And to give the
 * PHY some time to come out of reset before the U-Boot
 * ethernet driver tries to access its registers via MDIO.
 */
int platinum_setup_enet(void)
{
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);
	gpio_direction_output(IMX_GPIO_NR(6, 30), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 27), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1);
	imx_iomux_v3_setup_multiple_pads(enet_pads1, ARRAY_SIZE(enet_pads1));
	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);

	/* Need delay 10ms according to KSZ9021 spec */
	mdelay(10);
	gpio_set_value(IMX_GPIO_NR(3, 23), 1);
	udelay(100);

	imx_iomux_v3_setup_multiple_pads(enet_pads2, ARRAY_SIZE(enet_pads2));

	return 0;
}

int platinum_setup_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

	return 0;
}

int platinum_setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));

	return 0;
}

int platinum_setup_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));

	return 0;
}

int platinum_phy_config(struct phy_device *phydev)
{
	/* min rx data delay */
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW,
				   0x0);
	/* min tx data delay */
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW,
				   0x0);
	/* max rx/tx clock delay, min rx/tx control */
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_CLOCK_SKEW,
				   0xf0f0);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int platinum_init_gpio(void)
{
	/* Default GPIO's */
	/* Toggle CONFIG_n to reset fpga on every boot */
	gpio_direction_output(IMX_GPIO_NR(5, 18), 0);
	/* Need delay >=2uS */
	udelay(3);
	gpio_set_value(IMX_GPIO_NR(5, 18), 1);

	/* Default pin 1,15 high - DLP_FLASH_WPZ */
	gpio_direction_output(IMX_GPIO_NR(1, 15), 1);

	return 0;
}

int platinum_init_usb(void)
{
	return 0;
}

int platinum_init_finished(void)
{
	return 0;
}
