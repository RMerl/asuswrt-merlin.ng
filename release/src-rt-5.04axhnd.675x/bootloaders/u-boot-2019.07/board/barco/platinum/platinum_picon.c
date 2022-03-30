// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Barco (www.barco.com)
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <i2c.h>
#include <miiphy.h>

#include "platinum.h"

#define GPIO_IP_NCONFIG		IMX_GPIO_NR(5, 18)
#define GPIO_HK_NCONFIG		IMX_GPIO_NR(7, 13)
#define GPIO_LS_NCONFIG		IMX_GPIO_NR(5, 19)

#define GPIO_I2C0_SEL0		IMX_GPIO_NR(5,  2)
#define GPIO_I2C0_SEL1		IMX_GPIO_NR(1, 11)
#define GPIO_I2C0_ENBN		IMX_GPIO_NR(1, 13)

#define GPIO_I2C2_SEL0		IMX_GPIO_NR(1, 17)
#define GPIO_I2C2_SEL1		IMX_GPIO_NR(1, 20)
#define GPIO_I2C2_ENBN		IMX_GPIO_NR(1, 14)

#define GPIO_USB_RESET		IMX_GPIO_NR(1,  5)

iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK		| MUX_PAD_CTRL(ECSPI1_PAD_CLK),
	MX6_PAD_EIM_D17__ECSPI1_MISO		| MUX_PAD_CTRL(ECSPI_PAD_MISO),
	MX6_PAD_EIM_D18__ECSPI1_MOSI		| MUX_PAD_CTRL(ECSPI_PAD_MOSI),
	MX6_PAD_CSI0_DAT7__ECSPI1_SS0		| MUX_PAD_CTRL(ECSPI_PAD_SS),
	MX6_PAD_EIM_D24__ECSPI1_SS2		| MUX_PAD_CTRL(ECSPI_PAD_SS),
	MX6_PAD_EIM_D25__ECSPI1_SS3		| MUX_PAD_CTRL(ECSPI_PAD_SS),
};

iomux_v3_cfg_t const ecspi2_pads[] = {
	MX6_PAD_EIM_CS0__ECSPI2_SCLK		| MUX_PAD_CTRL(ECSPI2_PAD_CLK),
	MX6_PAD_EIM_OE__ECSPI2_MISO		| MUX_PAD_CTRL(ECSPI_PAD_MISO),
	MX6_PAD_EIM_CS1__ECSPI2_MOSI		| MUX_PAD_CTRL(ECSPI_PAD_MOSI),
	MX6_PAD_EIM_RW__ECSPI2_SS0		| MUX_PAD_CTRL(ECSPI_PAD_SS),
	MX6_PAD_EIM_LBA__ECSPI2_SS1		| MUX_PAD_CTRL(ECSPI_PAD_SS),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_CRS_DV__ENET_RX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_GPIO_16__ENET_REF_CLK		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RX_ER__ENET_RX_ER		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TX_EN__ENET_TX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

/* PHY nRESET */
iomux_v3_cfg_t const phy_reset_pad = {
	MX6_PAD_SD1_DAT2__GPIO1_IO19		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_CSI0_DAT12__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT13__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT16__UART4_RTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT17__UART4_CTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart5_pads[] = {
	MX6_PAD_CSI0_DAT14__UART5_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT15__UART5_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT18__UART5_RTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT19__UART5_CTS_B   | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const i2c0_mux_pads[] = {
	MX6_PAD_EIM_A25__GPIO5_IO02		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD2_CMD__GPIO1_IO11		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD2_DAT2__GPIO1_IO13		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const i2c2_mux_pads[] = {
	MX6_PAD_SD1_DAT1__GPIO1_IO17		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD1_CLK__GPIO1_IO20		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD2_DAT1__GPIO1_IO14		| MUX_PAD_CTRL(NO_PAD_CTRL),
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
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06		| PC,
		.gp = IMX_GPIO_NR(1, 6)
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
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	unsigned phy_reset = IMX_GPIO_NR(1, 19);

	/* First configure PHY reset GPIO pin */
	imx_iomux_v3_setup_pad(phy_reset_pad);

	/* Reconfigure enet muxing while PHY is in reset */
	gpio_direction_output(phy_reset, 0);
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
	mdelay(10);
	gpio_set_value(phy_reset, 1);
	udelay(100);

	/* set GPIO_16 as ENET_REF_CLK_OUT */
	setbits_le32(&iomux->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}

int platinum_setup_i2c(void)
{
	imx_iomux_v3_setup_multiple_pads(i2c0_mux_pads,
					 ARRAY_SIZE(i2c0_mux_pads));
	imx_iomux_v3_setup_multiple_pads(i2c2_mux_pads,
					 ARRAY_SIZE(i2c2_mux_pads));

	mdelay(10);

	/* Disable i2c mux 0 */
	gpio_direction_output(GPIO_I2C0_SEL0, 0);
	gpio_direction_output(GPIO_I2C0_SEL1, 0);
	gpio_direction_output(GPIO_I2C0_ENBN, 1);

	/* Disable i2c mux 1 */
	gpio_direction_output(GPIO_I2C2_SEL0, 0);
	gpio_direction_output(GPIO_I2C2_SEL1, 0);
	gpio_direction_output(GPIO_I2C2_ENBN, 1);

	udelay(10);

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

	/* Disable all leds */
	i2c_set_bus_num(0);
	i2c_reg_write(0x60, 0x05, 0x55);

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
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));

	return 0;
}

int platinum_phy_config(struct phy_device *phydev)
{
	/* Use generic infrastructure, no specific setup */
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int platinum_init_gpio(void)
{
	/* Reset FPGA's */
	gpio_direction_output(GPIO_IP_NCONFIG, 0);
	gpio_direction_output(GPIO_HK_NCONFIG, 0);
	gpio_direction_output(GPIO_LS_NCONFIG, 0);
	udelay(3);
	gpio_set_value(GPIO_IP_NCONFIG, 1);
	gpio_set_value(GPIO_HK_NCONFIG, 1);
	gpio_set_value(GPIO_LS_NCONFIG, 1);

	/* no dmd configuration yet */

	return 0;
}

int platinum_init_usb(void)
{
	/* Reset usb hub */
	gpio_direction_output(GPIO_USB_RESET, 0);
	udelay(100);
	gpio_set_value(GPIO_USB_RESET, 1);

	return 0;
}

int platinum_init_finished(void)
{
	/* Enable led 0 */
	i2c_set_bus_num(0);
	i2c_reg_write(0x60, 0x05, 0x54);

	return 0;
}
