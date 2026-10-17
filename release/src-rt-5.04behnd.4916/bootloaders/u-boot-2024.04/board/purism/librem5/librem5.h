/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Purism
 */

#ifndef __LIBREM5_H__
#define __LIBREM5_H__

#define CAMERA_EN IMX_GPIO_NR(1, 0)
#define SD_EN IMX_GPIO_NR(1, 3)
#define AUDIO_EN IMX_GPIO_NR(1, 4)
#define DSI_EN IMX_GPIO_NR(1, 5)
#define SMC_EN IMX_GPIO_NR(1, 6)
#define TYPEC_MUX_EN IMX_GPIO_NR(1, 11)
#define HUB_NRESET IMX_GPIO_NR(1, 12)
#define HUB_EN IMX_GPIO_NR(1, 14)
#define VOL_UP IMX_GPIO_NR(1, 16)
#define VOL_DOWN IMX_GPIO_NR(1, 17)
#define DSI_BIAS_EN IMX_GPIO_NR(1, 20)
#define FLASH_EN IMX_GPIO_NR(1, 23)
#define WWAN_NRESET IMX_GPIO_NR(3, 1)
#define CHG_EN IMX_GPIO_NR(3, 2)
#define CHG_OTG_OUT_EN IMX_GPIO_NR(3, 4)
#define WIFI_EN IMX_GPIO_NR(3, 10)
#define GPS_EN IMX_GPIO_NR(3, 12)
#define BL_EN IMX_GPIO_NR(3, 14)
#define WWAN_EN IMX_GPIO_NR(3, 18)
#define NFC_EN IMX_GPIO_NR(4, 28)
#define LED_G IMX_GPIO_NR(5, 2)
#define LED_R IMX_GPIO_NR(5, 3)
#define LED_B IMX_GPIO_NR(1, 13)
#define MOTO IMX_GPIO_NR(5, 5)
#define SPI1_SCLK IMX_GPIO_NR(5, 6)
#define SPI1_MOSI IMX_GPIO_NR(5, 7)
#define SPI1_MISO IMX_GPIO_NR(5, 8)
#define SPI1_SS0 IMX_GPIO_NR(5, 9)

#define UART1_TX IMX_GPIO_NR(5, 23)
#define UART1_RX IMX_GPIO_NR(5, 22)
#define UART2_TX IMX_GPIO_NR(5, 25)
#define UART2_RX IMX_GPIO_NR(5, 24)
#define UART3_TX IMX_GPIO_NR(5, 27)
#define UART3_RX IMX_GPIO_NR(5, 26)
#define UART4_TX IMX_GPIO_NR(5, 11)
#define UART4_RX IMX_GPIO_NR(5, 10)

#define TPS_RESET IMX_GPIO_NR(3, 24)

#define PURISM_VID	0x316d
#define PURISM_PID	0x4c05

#define BOARD_REV_ERROR		"unknown"
#define BOARD_REV_BIRCH		"1"
#define BOARD_REV_CHESTNUT	"2"
#define BOARD_REV_DOGWOOD	"3"
#define BOARD_REV_EVERGREEN	"4"
/* Could be ASPEN, BIRCH or CHESTNUT. assume CHESTNUT */
#define BOARD_REV_UNKNOWN	BOARD_REV_CHESTNUT

#ifdef CONFIG_SPL_BUILD
static const iomux_v3_cfg_t configure_pads[] = {
	IMX8MQ_PAD_GPIO1_IO00__GPIO1_IO0 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_GPIO1_IO03__GPIO1_IO3 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_GPIO1_IO04__GPIO1_IO4 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_GPIO1_IO05__GPIO1_IO5 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_GPIO1_IO06__GPIO1_IO6 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_GPIO1_IO11__GPIO1_IO11 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_GPIO1_IO12__GPIO1_IO12 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_GPIO1_IO13__GPIO1_IO13 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_GPIO1_IO14__GPIO1_IO14 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_ENET_MDC__GPIO1_IO16 | MUX_PAD_CTRL(PAD_CTL_PUE),
	IMX8MQ_PAD_ENET_MDIO__GPIO1_IO17 | MUX_PAD_CTRL(PAD_CTL_PUE),
	IMX8MQ_PAD_ENET_TD1__GPIO1_IO20 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_ENET_TXC__GPIO1_IO23 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_CE0_B__GPIO3_IO1 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_CE1_B__GPIO3_IO2 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_CE3_B__GPIO3_IO4 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_DATA04__GPIO3_IO10 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_DATA06__GPIO3_IO12 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_DQS__GPIO3_IO14 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_NAND_WP_B__GPIO3_IO18 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_SAI3_RXFS__GPIO4_IO28 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_SAI3_MCLK__GPIO5_IO2 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_SPDIF_TX__GPIO5_IO3 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
	IMX8MQ_PAD_SAI5_RXD3__GPIO3_IO24 | MUX_PAD_CTRL(PAD_CTL_DSE6) | MUX_MODE_SION,
};

static inline void init_pinmux(void)
{
	imx_iomux_v3_setup_multiple_pads(configure_pads, ARRAY_SIZE(configure_pads));

	gpio_request(LED_R, "LED_R");
	gpio_request(LED_G, "LED_G");
	gpio_request(LED_B, "LED_B");
	gpio_request(VOL_UP, "VOL_UP");
	gpio_request(VOL_DOWN, "VOL_DOWN");

	gpio_request(NFC_EN, "NFC_EN");
	gpio_request(CHG_EN, "CHG_EN");
	gpio_request(CHG_OTG_OUT_EN, "CHG_OTG_OUT_EN");

	gpio_request(TYPEC_MUX_EN, "TYPEC_MUX_EN");

	gpio_request(TPS_RESET, "TPS_RESET");

	gpio_request(WWAN_EN, "WWAN_EN");
	gpio_request(WWAN_NRESET, "WWAN_NRESET");

	gpio_request(HUB_EN, "HUB_EN");
	gpio_request(HUB_NRESET, "HUB_NRESET");
	gpio_request(SD_EN, "SD_EN");
	gpio_request(AUDIO_EN, "AUDIO_EN");
	gpio_request(DSI_EN, "DSI_EN");
	gpio_request(SMC_EN, "SMC_EN");
	gpio_request(CAMERA_EN, "CAMERA_EN");
	gpio_request(FLASH_EN, "FLASH_EN");
	gpio_request(DSI_BIAS_EN, "DSI_BIAS_EN");
	gpio_request(GPS_EN, "GPS_EN");
	gpio_request(BL_EN, "BL_EN");
#ifndef CONSOLE_ON_UART4
	gpio_request(WIFI_EN, "WIFI_EN");
	gpio_direction_output(WIFI_EN, 0);
#endif /* CONSOLE_ON_UART4 */
	gpio_direction_input(VOL_UP);
	gpio_direction_input(VOL_DOWN);

	/* ensure charger is in the automated mode */
	gpio_direction_output(NFC_EN, 0);
	gpio_direction_output(CHG_EN, 0);
	gpio_direction_output(CHG_OTG_OUT_EN, 0);

	gpio_direction_input(TYPEC_MUX_EN);

	gpio_direction_output(TPS_RESET, 0);

	gpio_direction_output(WWAN_EN, 0);
	gpio_direction_output(WWAN_NRESET, 1);

	gpio_direction_output(HUB_EN, 1);
	gpio_direction_output(HUB_NRESET, 1);
	mdelay(10);
	gpio_direction_output(SD_EN, 1);
	gpio_direction_output(SMC_EN, 0);
	gpio_direction_output(CAMERA_EN, 0);
	gpio_direction_output(FLASH_EN, 0);
	gpio_direction_output(DSI_BIAS_EN, 0);
	gpio_direction_output(GPS_EN, 0);
	gpio_direction_output(BL_EN, 0);

	/* turn these on for i2c busses */
	gpio_direction_output(AUDIO_EN, 1);
	gpio_direction_output(DSI_EN, 1);
}
#endif /* CONFIG_SPL_BUILD */

#define USB1_BASE_ADDR         0x38100000
#define USB2_BASE_ADDR         0x38200000
#define USB1_PHY_BASE_ADDR     0x381F0000
#define USB2_PHY_BASE_ADDR     0x382F0000

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)
#define USB_PHY_CTRL0_SSC_RANGE_MASK	GENMASK(23, 21)
#define USB_PHY_CTRL0_SSC_RANGE_4003PPM	(0x2 << 21)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

#define USB_PHY_CTRL6			0x18
#define USB_PHY_CTRL6_RXTERM_OVERRIDE_SEL	BIT(29)

extern struct dram_timing_info dram_timing_b0;

#endif
