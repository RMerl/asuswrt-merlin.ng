// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <env.h>
#include <fsl_esdhc_imx.h>
#include <hwconfig.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <linux/delay.h>

#include "common.h"

/* MMC */
static iomux_v3_cfg_t const gw5904_emmc_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET  | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};
/* 4-bit microSD on SD2 */
static iomux_v3_cfg_t const gw5904_mmc_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* CD */
	IOMUX_PADS(PAD_NANDF_CS0__GPIO6_IO11 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};
/* 8-bit eMMC on SD2/NAND */
static iomux_v3_cfg_t const gw560x_emmc_sd2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__SD2_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__SD2_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__SD2_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__SD2_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__GPIO7_IO00  | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

/*
 * Baseboard specific GPIO
 */
static iomux_v3_cfg_t const gw51xx_gpio_pads[] = {
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* GPS_SHDN */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw52xx_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* GPS_SHDN */
	IOMUX_PADS(PAD_ENET_RXD0__GPIO1_IO27 | DIO_PAD_CFG),
	/* USBOTG_SEL */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw53xx_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* GPS_SHDN */
	IOMUX_PADS(PAD_ENET_RXD0__GPIO1_IO27 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
	/* J6_PWREN */
	IOMUX_PADS(PAD_EIM_DA15__GPIO3_IO15 | DIO_PAD_CFG),
	/* PCIEGBE_EN */
	IOMUX_PADS(PAD_EIM_DA14__GPIO3_IO14 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw54xx_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* MIPI_DIO */
	IOMUX_PADS(PAD_SD1_DAT3__GPIO1_IO21 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_EIM_D24__GPIO3_IO24 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
	/* J7_PWREN */
	IOMUX_PADS(PAD_EIM_DA15__GPIO3_IO15 | DIO_PAD_CFG),
	/* PCIEGBE_EN */
	IOMUX_PADS(PAD_EIM_DA14__GPIO3_IO14 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw551x_gpio_pads[] = {
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw552x_gpio_pads[] = {
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* USBOTG_SEL */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* MX6_DIO[4:9] */
	IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT4__GPIO5_IO22 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT5__GPIO5_IO23 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT7__GPIO5_IO25 | DIO_PAD_CFG),
	/* PCIEGBE1_OFF# */
	IOMUX_PADS(PAD_GPIO_1__GPIO1_IO01 | DIO_PAD_CFG),
	/* PCIEGBE2_OFF# */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw553x_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw560x_gpio_pads[] = {
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
	/* 12V0_PWR_EN */
	IOMUX_PADS(PAD_DISP0_DAT5__GPIO4_IO26 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5901_gpio_pads[] = {
	/* ETH1_EN */
	IOMUX_PADS(PAD_GPIO_1__GPIO1_IO01 | DIO_PAD_CFG),
	/* PMIC reset */
	IOMUX_PADS(PAD_DISP0_DAT8__WDOG1_B | DIO_PAD_CFG),
	/* COM_CFGA/B/C/D */
	IOMUX_PADS(PAD_DISP0_DAT20__GPIO5_IO14 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_DISP0_DAT21__GPIO5_IO15 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_DISP0_DAT22__GPIO5_IO16 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
	/* ETI_IRQ# */
	IOMUX_PADS(PAD_GPIO_5__GPIO1_IO05 | DIO_PAD_CFG),
	/* DIO_IRQ# */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* FIBER_SIGDET */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5902_gpio_pads[] = {
	/* UART1_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* 5V_UVLO */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
	/* ETI_IRQ# */
	IOMUX_PADS(PAD_GPIO_5__GPIO1_IO05 | DIO_PAD_CFG),
	/* DIO_IRQ# */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5903_gpio_pads[] = {
	/* BKLT_12VEN */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* EMMY_PDN# */
	IOMUX_PADS(PAD_NANDF_D2__GPIO2_IO02 | DIO_PAD_CFG),
	/* EMMY_CFG1# */
	IOMUX_PADS(PAD_NANDF_D3__GPIO2_IO03 | DIO_PAD_CFG),
	/* EMMY_CFG1# */
	IOMUX_PADS(PAD_NANDF_D4__GPIO2_IO04 | DIO_PAD_CFG),
	/* USBH1_PEN (EHCI) */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* USBDPC_PEN */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* TOUCH_RST */
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08 | DIO_PAD_CFG),
	/* AUDIO_RST# */
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
	/* UART1_TEN# */
	IOMUX_PADS(PAD_CSI0_DAT12__GPIO5_IO30 | DIO_PAD_CFG),
	/* LVDS_BKLEN # */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
	/* RGMII_PDWN# */
	IOMUX_PADS(PAD_ENET_CRS_DV__GPIO1_IO25 | DIO_PAD_CFG),
	/* TOUCH_IRQ# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* TOUCH_RST# */
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5904_gpio_pads[] = {
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* UART_RS485 */
	IOMUX_PADS(PAD_DISP0_DAT2__GPIO4_IO23 | DIO_PAD_CFG),
	/* UART_HALF */
	IOMUX_PADS(PAD_DISP0_DAT3__GPIO4_IO24 | DIO_PAD_CFG),
	/* SKT1_WDIS# */
	IOMUX_PADS(PAD_DISP0_DAT17__GPIO5_IO11 | DIO_PAD_CFG),
	/* SKT1_RST# */
	IOMUX_PADS(PAD_DISP0_DAT18__GPIO5_IO12 | DIO_PAD_CFG),
	/* SKT2_WDIS# */
	IOMUX_PADS(PAD_DISP0_DAT19__GPIO5_IO13 | DIO_PAD_CFG),
	/* SKT2_RST# */
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | DIO_PAD_CFG),
	/* M2_OFF# */
	IOMUX_PADS(PAD_SD2_DAT0__GPIO1_IO15 | DIO_PAD_CFG),
	/* M2_WDIS# */
	IOMUX_PADS(PAD_SD2_DAT1__GPIO1_IO14 | DIO_PAD_CFG),
	/* M2_RST# */
	IOMUX_PADS(PAD_SD2_DAT2__GPIO1_IO13 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5905_gpio_pads[] = {
	/* EMMY_PDN# */
	IOMUX_PADS(PAD_NANDF_D3__GPIO2_IO03 | DIO_PAD_CFG),
	/* MIPI_RST */
	IOMUX_PADS(PAD_SD2_DAT0__GPIO1_IO15 | DIO_PAD_CFG),
	/* MIPI_PWDN */
	IOMUX_PADS(PAD_SD2_DAT1__GPIO1_IO14 | DIO_PAD_CFG),
	/* USBEHCI_SEL */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* LVDS_BKLEN # */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_18__GPIO7_IO13 | DIO_PAD_CFG),
	/* SPK_SHDN# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* DECT_RST# */
	IOMUX_PADS(PAD_DISP0_DAT20__GPIO5_IO14 | DIO_PAD_CFG),
	/* USBH1_PEN (EHCI) */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* LVDS_PWM */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
	/* CODEC_RST */
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
	/* GYRO_CONTROL/DATA_EN */
	IOMUX_PADS(PAD_CSI0_DAT8__GPIO5_IO26 | DIO_PAD_CFG),
	/* TOUCH_RST */
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08 | DIO_PAD_CFG),
	/* TOUCH_IRQ */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5910_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* RF_RESET# */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* RF_BOOT */
	IOMUX_PADS(PAD_GPIO_8__GPIO1_IO08 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw5912_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | DIO_PAD_CFG),
};

/* Digital I/O */
struct dio_cfg gw51xx_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{ IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18) },
		IMX_GPIO_NR(1, 18),
		{ IOMUX_PADS(PAD_SD1_CMD__PWM4_OUT) },
		4
	},
};

struct dio_cfg gw52xx_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{ IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw53xx_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw54xx_dio[] = {
	{
		{ IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09) },
		IMX_GPIO_NR(1, 9),
		{ IOMUX_PADS(PAD_GPIO_9__PWM1_OUT) },
		1
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD4_DAT1__GPIO2_IO09) },
		IMX_GPIO_NR(2, 9),
		{ IOMUX_PADS(PAD_SD4_DAT1__PWM3_OUT) },
		3
	},
	{
		{ IOMUX_PADS(PAD_SD4_DAT2__GPIO2_IO10) },
		IMX_GPIO_NR(2, 10),
		{ IOMUX_PADS(PAD_SD4_DAT2__PWM4_OUT) },
		4
	},
};

struct dio_cfg gw551x_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
};

struct dio_cfg gw552x_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18) },
		IMX_GPIO_NR(5, 18),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20) },
		IMX_GPIO_NR(5, 20),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21) },
		IMX_GPIO_NR(5, 21),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT4__GPIO5_IO22) },
		IMX_GPIO_NR(5, 22),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT5__GPIO5_IO23) },
		IMX_GPIO_NR(5, 23),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT7__GPIO5_IO25) },
		IMX_GPIO_NR(5, 25),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw553x_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{ IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18) },
		IMX_GPIO_NR(1, 18),
		{ IOMUX_PADS(PAD_SD1_CMD__PWM4_OUT) },
		4
	},
};

struct dio_cfg gw560x_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw5901_dio[] = {
	{
		{ IOMUX_PADS(PAD_DISP0_DAT20__GPIO5_IO14) },
		IMX_GPIO_NR(5, 14),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT21__GPIO5_IO15) },
		IMX_GPIO_NR(5, 15),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT22__GPIO5_IO16) },
		IMX_GPIO_NR(5, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17) },
		IMX_GPIO_NR(5, 17),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw5902_dio[] = {
	{
		{ IOMUX_PADS(PAD_DISP0_DAT20__GPIO5_IO14) },
		IMX_GPIO_NR(5, 14),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT21__GPIO5_IO15) },
		IMX_GPIO_NR(5, 15),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT22__GPIO5_IO16) },
		IMX_GPIO_NR(5, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17) },
		IMX_GPIO_NR(5, 17),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw5903_dio[] = {
};

struct dio_cfg gw5904_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D0__GPIO2_IO00) },
		IMX_GPIO_NR(2, 0),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D1__GPIO2_IO01) },
		IMX_GPIO_NR(2, 1),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D2__GPIO2_IO02) },
		IMX_GPIO_NR(2, 2),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D3__GPIO2_IO03) },
		IMX_GPIO_NR(2, 3),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D4__GPIO2_IO04) },
		IMX_GPIO_NR(2, 4),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D5__GPIO2_IO05) },
		IMX_GPIO_NR(2, 5),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D6__GPIO2_IO06) },
		IMX_GPIO_NR(2, 6),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_NANDF_D7__GPIO2_IO07) },
		IMX_GPIO_NR(2, 7),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw5906_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
		IMX_GPIO_NR(1, 20),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw5913_dio[] = {
	{
		{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
		IMX_GPIO_NR(1, 16),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
		IMX_GPIO_NR(1, 19),
		{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
		2
	},
	{
		{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
		IMX_GPIO_NR(1, 17),
		{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
		3
	},
	{
		{ IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18) },
		IMX_GPIO_NR(1, 18),
		{ IOMUX_PADS(PAD_SD1_CMD__PWM4_OUT) },
		4
	},
	{
		{ IOMUX_PADS(PAD_SD2_DAT0__GPIO1_IO15) },
		IMX_GPIO_NR(1, 15),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_SD2_DAT1__GPIO1_IO14) },
		IMX_GPIO_NR(1, 14),
		{ 0, 0 },
		0
	},
	{
		{ IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05) },
		IMX_GPIO_NR(4, 5),
		{ 0, 0 },
		0
	},
};

/*
 * Board Specific GPIO
 */
struct ventana gpio_cfg[GW_UNKNOWN] = {
	/* GW5400proto */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = gw54xx_dio,
		.dio_num = ARRAY_SIZE(gw54xx_dio),
		.mezz_pwren = IMX_GPIO_NR(4, 7),
		.mezz_irq = IMX_GPIO_NR(4, 9),
		.rs485en = IMX_GPIO_NR(3, 24),
		.dioi2c_en = IMX_GPIO_NR(4,  5),
		.pcie_sson = IMX_GPIO_NR(1, 20),
		.mmc_cd = IMX_GPIO_NR(7, 0),
		.wdis = -1,
	},

	/* GW51xx */
	{
		.gpio_pads = gw51xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw51xx_gpio_pads)/2,
		.dio_cfg = gw51xx_dio,
		.dio_num = ARRAY_SIZE(gw51xx_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 2),
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW52xx */
	{
		.gpio_pads = gw52xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw52xx_gpio_pads)/2,
		.dio_cfg = gw52xx_dio,
		.dio_num = ARRAY_SIZE(gw52xx_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 27),
		.usb_sel = IMX_GPIO_NR(1, 2),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW53xx */
	{
		.gpio_pads = gw53xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw53xx_gpio_pads)/2,
		.dio_cfg = gw53xx_dio,
		.dio_num = ARRAY_SIZE(gw53xx_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 27),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW54xx */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = gw54xx_dio,
		.dio_num = ARRAY_SIZE(gw54xx_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.rs485en = IMX_GPIO_NR(7, 1),
		.dioi2c_en = IMX_GPIO_NR(4,  5),
		.pcie_sson = IMX_GPIO_NR(1, 20),
		.wdis = IMX_GPIO_NR(5, 17),
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW551x */
	{
		.gpio_pads = gw551x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw551x_gpio_pads)/2,
		.dio_cfg = gw551x_dio,
		.dio_num = ARRAY_SIZE(gw551x_dio),
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW552x */
	{
		.gpio_pads = gw552x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw552x_gpio_pads)/2,
		.dio_cfg = gw552x_dio,
		.dio_num = ARRAY_SIZE(gw552x_dio),
		.usb_sel = IMX_GPIO_NR(1, 7),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
	},

	/* GW553x */
	{
		.gpio_pads = gw553x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw553x_gpio_pads)/2,
		.dio_cfg = gw553x_dio,
		.dio_num = ARRAY_SIZE(gw553x_dio),
		.wdis = IMX_GPIO_NR(7, 12),
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW560x */
	{
		.gpio_pads = gw560x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw560x_gpio_pads)/2,
		.dio_cfg = gw560x_dio,
		.dio_num = ARRAY_SIZE(gw560x_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.rs232_en = GP_RS232_EN,
		.wdis = IMX_GPIO_NR(7, 12),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW5901 */
	{
		.gpio_pads = gw5901_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5901_gpio_pads)/2,
		.dio_cfg = gw5901_dio,
		.wdis = -1,
	},

	/* GW5902 */
	{
		.gpio_pads = gw5902_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5902_gpio_pads)/2,
		.dio_cfg = gw5902_dio,
		.rs232_en = GP_RS232_EN,
		.wdis = -1,
	},

	/* GW5903 */
	{
		.gpio_pads = gw5903_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5903_gpio_pads)/2,
		.dio_cfg = gw5903_dio,
		.dio_num = ARRAY_SIZE(gw5903_dio),
		.mmc_cd = IMX_GPIO_NR(6, 11),
		.wdis = -1,
	},

	/* GW5904 */
	{
		.gpio_pads = gw5904_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5904_gpio_pads)/2,
		.dio_cfg = gw5904_dio,
		.dio_num = ARRAY_SIZE(gw5904_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.wdis = -1,
	},

	/* GW5905 */
	{
		.gpio_pads = gw5905_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5905_gpio_pads)/2,
		.wdis = IMX_GPIO_NR(7, 13),
	},

	/* GW5906 */
	{
		.gpio_pads = gw552x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw552x_gpio_pads)/2,
		.dio_cfg = gw5906_dio,
		.dio_num = ARRAY_SIZE(gw5906_dio),
		.usb_sel = IMX_GPIO_NR(1, 7),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
	},

	/* GW5907 */
	{
		.gpio_pads = gw51xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw51xx_gpio_pads)/2,
		.dio_cfg = gw51xx_dio,
		.dio_num = ARRAY_SIZE(gw51xx_dio),
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW5908 */
	{
		.gpio_pads = gw53xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw53xx_gpio_pads)/2,
		.dio_cfg = gw53xx_dio,
		.dio_num = ARRAY_SIZE(gw53xx_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 27),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
	},

	/* GW5909 */
	{
		.gpio_pads = gw5904_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5904_gpio_pads)/2,
		.dio_cfg = gw5904_dio,
		.dio_num = ARRAY_SIZE(gw5904_dio),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.wdis = -1,
	},

	/* GW5910 */
	{
		.gpio_pads = gw5910_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5910_gpio_pads) / 2,
		.dio_cfg = gw52xx_dio,
		.dio_num = ARRAY_SIZE(gw52xx_dio),
		.wdis = IMX_GPIO_NR(7, 12),
		.rs232_en = GP_RS232_EN,
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW5912 */
	{
		.gpio_pads = gw5912_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5912_gpio_pads) / 2,
		.dio_cfg = gw54xx_dio,
		.dio_num = ARRAY_SIZE(gw54xx_dio),
		.wdis = IMX_GPIO_NR(1, 0),
		.rs232_en = GP_RS232_EN,
		.vsel_pin = IMX_GPIO_NR(6, 14),
		.mmc_cd = IMX_GPIO_NR(7, 0),
	},

	/* GW5913 */
	{
		.gpio_pads = gw5912_gpio_pads,
		.num_pads = ARRAY_SIZE(gw5912_gpio_pads) / 2,
		.dio_cfg = gw5913_dio,
		.dio_num = ARRAY_SIZE(gw5913_dio),
		.wdis = IMX_GPIO_NR(1, 0),
	},
};

#define SETUP_GPIO_OUTPUT(gpio, name, level) \
	gpio_request(gpio, name); \
	gpio_direction_output(gpio, level);
#define SETUP_GPIO_INPUT(gpio, name) \
	gpio_request(gpio, name); \
	gpio_direction_input(gpio);
void setup_iomux_gpio(int board)
{
	if (board >= GW_UNKNOWN)
		return;

	/* board specific iomux */
	imx_iomux_v3_setup_multiple_pads(gpio_cfg[board].gpio_pads,
					 gpio_cfg[board].num_pads);

	/* RS232_EN# */
	if (gpio_cfg[board].rs232_en) {
		gpio_request(gpio_cfg[board].rs232_en, "rs232_en#");
		gpio_direction_output(gpio_cfg[board].rs232_en, 0);
	}

	/* MSATA Enable - default to PCI */
	if (gpio_cfg[board].msata_en) {
		gpio_request(gpio_cfg[board].msata_en, "msata_en");
		gpio_direction_output(gpio_cfg[board].msata_en, 0);
	}

	/* Expansion Mezzanine IO */
	if (gpio_cfg[board].mezz_pwren) {
		gpio_request(gpio_cfg[board].mezz_pwren, "mezz_pwr");
		gpio_direction_output(gpio_cfg[board].mezz_pwren, 0);
	}
	if (gpio_cfg[board].mezz_irq) {
		gpio_request(gpio_cfg[board].mezz_irq, "mezz_irq#");
		gpio_direction_input(gpio_cfg[board].mezz_irq);
	}

	/* RS485 Transmit Enable */
	if (gpio_cfg[board].rs485en) {
		gpio_request(gpio_cfg[board].rs485en, "rs485_en");
		gpio_direction_output(gpio_cfg[board].rs485en, 0);
	}

	/* GPS_SHDN */
	if (gpio_cfg[board].gps_shdn) {
		gpio_request(gpio_cfg[board].gps_shdn, "gps_shdn");
		gpio_direction_output(gpio_cfg[board].gps_shdn, 1);
	}

	/* DIOI2C_DIS# */
	if (gpio_cfg[board].dioi2c_en) {
		gpio_request(gpio_cfg[board].dioi2c_en, "dioi2c_dis#");
		gpio_direction_output(gpio_cfg[board].dioi2c_en, 0);
	}

	/* PCICK_SSON: disable spread-spectrum clock */
	if (gpio_cfg[board].pcie_sson) {
		gpio_request(gpio_cfg[board].pcie_sson, "pci_sson");
		gpio_direction_output(gpio_cfg[board].pcie_sson, 0);
	}

	/* USBOTG mux routing */
	if (gpio_cfg[board].usb_sel) {
		gpio_request(gpio_cfg[board].usb_sel, "usb_pcisel");
		gpio_direction_output(gpio_cfg[board].usb_sel, 0);
	}

	/* PCISKT_WDIS# (Wireless disable GPIO to miniPCIe sockets) */
	if (gpio_cfg[board].wdis != -1) {
		gpio_request(gpio_cfg[board].wdis, "wlan_dis");
		gpio_direction_output(gpio_cfg[board].wdis, 1);
	}

	/* sense vselect pin to see if we support uhs-i */
	if (gpio_cfg[board].vsel_pin) {
		gpio_request(gpio_cfg[board].vsel_pin, "sd3_vselect");
		gpio_direction_input(gpio_cfg[board].vsel_pin);
		gpio_cfg[board].usd_vsel = !gpio_get_value(gpio_cfg[board].vsel_pin);
	}

	/* microSD CD */
	if (gpio_cfg[board].mmc_cd) {
		gpio_request(gpio_cfg[board].mmc_cd, "sd_cd");
		gpio_direction_input(gpio_cfg[board].mmc_cd);
	}

	/* Anything else board specific */
	switch(board) {
	case GW53xx:
		gpio_request(IMX_GPIO_NR(3, 15), "j6_pwren");
		gpio_direction_output(IMX_GPIO_NR(3, 15), 1);
		gpio_request(IMX_GPIO_NR(3, 14), "gbe_en");
		gpio_direction_output(IMX_GPIO_NR(3, 14), 1);
		break;
	case GW54xx:
		gpio_request(IMX_GPIO_NR(3, 15), "j7_pwren");
		gpio_direction_output(IMX_GPIO_NR(3, 15), 1);
		gpio_request(IMX_GPIO_NR(3, 14), "gbe_en");
		gpio_direction_output(IMX_GPIO_NR(3, 14), 1);
		break;
	case GW560x:
		gpio_request(IMX_GPIO_NR(4, 26), "12p0_en");
		gpio_direction_output(IMX_GPIO_NR(4, 26), 1);
		break;
	case GW5902:
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(7, 12), "5P0V_EN", 1);
		break;
	case GW5903:
		gpio_request(IMX_GPIO_NR(3, 31) , "usbh1-ehci_pwr");
		gpio_direction_output(IMX_GPIO_NR(3, 31), 1);
		gpio_request(IMX_GPIO_NR(4, 15) , "usbh2-otg_pwr");
		gpio_direction_output(IMX_GPIO_NR(4, 15), 1);
		gpio_request(IMX_GPIO_NR(4, 7) , "usbdpc_pwr");
		gpio_direction_output(IMX_GPIO_NR(4, 15), 1);
		gpio_request(IMX_GPIO_NR(1, 25) , "rgmii_en");
		gpio_direction_output(IMX_GPIO_NR(1, 25), 1);
		gpio_request(IMX_GPIO_NR(4, 6) , "touch_irq#");
		gpio_direction_input(IMX_GPIO_NR(4, 6));
		gpio_request(IMX_GPIO_NR(4, 8) , "touch_rst");
		gpio_direction_output(IMX_GPIO_NR(4, 8), 1);
		gpio_request(IMX_GPIO_NR(1, 7) , "bklt_12ven");
		gpio_direction_output(IMX_GPIO_NR(1, 7), 1);
		break;
	case GW5909:
	case GW5904:
		gpio_request(IMX_GPIO_NR(4, 23), "rs485_en");
		gpio_direction_output(IMX_GPIO_NR(4, 23), 0);
		gpio_request(IMX_GPIO_NR(5, 11), "skt1_wdis#");
		gpio_direction_output(IMX_GPIO_NR(5, 11), 1);
		gpio_request(IMX_GPIO_NR(5, 12), "skt1_rst#");
		gpio_direction_output(IMX_GPIO_NR(5, 12), 1);
		gpio_request(IMX_GPIO_NR(5, 13), "skt2_wdis#");
		gpio_direction_output(IMX_GPIO_NR(5, 13), 1);
		gpio_request(IMX_GPIO_NR(1, 15), "m2_off#");
		gpio_direction_output(IMX_GPIO_NR(1, 15), 1);
		gpio_request(IMX_GPIO_NR(1, 14), "m2_wdis#");
		gpio_direction_output(IMX_GPIO_NR(1, 14), 1);
		gpio_request(IMX_GPIO_NR(1, 13), "m2_rst#");
		gpio_direction_output(IMX_GPIO_NR(1, 13), 1);
		break;
	case GW5905:
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 7), "usb_pcisel", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 9), "lvds_cabc", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 14), "mipi_pdwn", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 15), "mipi_rst#", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(2, 3), "emmy_pdwn#", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(4, 5), "spk_shdn#", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(4, 8), "touch_rst", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(4, 6), "touch_irq", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(5, 5), "flash_en1", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(5, 6), "flash_en2", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(5, 14), "dect_rst#", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(5, 17), "codec_rst#", 0);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(5, 26), "imu_den", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(7, 12), "lvds_cabc", 0);
		mdelay(100);
		/*
		 * gauruntee touch controller comes out of reset with INT
		 * low for address
		 */
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(4, 8), "touch_rst", 1);
		break;
	case GW5910:
		/* CC1352 */
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 7), "rf_reset#", 1);
		SETUP_GPIO_OUTPUT(IMX_GPIO_NR(1, 8), "rf_boot", 1);
		break;
	}
}

#ifdef CONFIG_FSL_ESDHC_IMX
static struct fsl_esdhc_cfg usdhc_cfg[2];

int board_mmc_init(struct bd_info *bis)
{
	int ret;

	switch (board_type) {
	case GW52xx:
	case GW53xx:
	case GW54xx:
	case GW553x:
	case GW5910:
	case GW5912:
		/* usdhc3: 4bit microSD */
		SETUP_IOMUX_PADS(usdhc3_pads);
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		usdhc_cfg[0].max_bus_width = 4;
		return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
	case GW560x:
		/* usdhc2: 8-bit eMMC */
		SETUP_IOMUX_PADS(gw560x_emmc_sd2_pads);
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		usdhc_cfg[0].max_bus_width = 8;
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
		if (ret)
			return ret;
		/* usdhc3: 4-bit microSD */
		SETUP_IOMUX_PADS(usdhc3_pads);
		usdhc_cfg[1].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		usdhc_cfg[1].max_bus_width = 4;
		return fsl_esdhc_initialize(bis, &usdhc_cfg[1]);
	case GW5903:
		/* usdhc3: 8-bit eMMC */
		SETUP_IOMUX_PADS(gw5904_emmc_pads);
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		usdhc_cfg[0].max_bus_width = 8;
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
		if (ret)
			return ret;
		/* usdhc2: 4-bit microSD */
		SETUP_IOMUX_PADS(gw5904_mmc_pads);
		usdhc_cfg[1].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		usdhc_cfg[1].max_bus_width = 4;
		return fsl_esdhc_initialize(bis, &usdhc_cfg[1]);
	case GW5904:
	case GW5905:
	case GW5909:
		/* usdhc3: 8bit eMMC */
		SETUP_IOMUX_PADS(gw5904_emmc_pads);
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		usdhc_cfg[0].max_bus_width = 8;
		return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
	default:
		/* doesn't have MMC */
		printf("None");
		return -1;
	}
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int gpio = gpio_cfg[board_type].mmc_cd;

	/* Card Detect */
	switch (board_type) {
	case GW560x:
		/* emmc is always present */
		if (cfg->esdhc_base == USDHC2_BASE_ADDR)
			return 1;
		break;
	case GW5903:
	case GW5904:
	case GW5905:
	case GW5909:
		/* emmc is always present */
		if (cfg->esdhc_base == USDHC3_BASE_ADDR)
			return 1;
		break;
	}

	if (gpio) {
		debug("%s: gpio%d=%d\n", __func__, gpio, gpio_get_value(gpio));
		return !gpio_get_value(gpio);
	}

	return -1;
}

#endif /* CONFIG_FSL_ESDHC_IMX */
