// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * Pinmux Setting for B&R LEIT Board(s)
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>

static struct module_pin_mux spi0_pin_mux[] = {
	/* SPI1_SCLK */
	{OFFSET(spi0_sclk),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI1_D0 */
	{OFFSET(spi0_d0),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI1_D1 */
	{OFFSET(spi0_d1),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI1_CS0 */
	{OFFSET(spi0_cs0),	MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE},
	/* SPI1_CS1 */
	{OFFSET(spi0_cs1),	MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE},
	{-1},
};

static struct module_pin_mux dcan0_pin_mux[] = {
	/* DCAN0 TX */
	{OFFSET(uart1_ctsn),   MODE(2) | PULLUDEN | PULLUP_EN},
	/* DCAN0 RX */
	{OFFSET(uart1_rtsn),   MODE(2) | RXACTIVE},
	{-1},
};

static struct module_pin_mux dcan1_pin_mux[] = {
	/* DCAN1 TX */
	{OFFSET(uart1_rxd),   MODE(2) | PULLUDEN | PULLUP_EN},
	/* DCAN1 RX */
	{OFFSET(uart1_txd),   MODE(2) | RXACTIVE},
	{-1},
};

static struct module_pin_mux gpios[] = {
	/* GPIO0_7  (PWW0 OUT) - CAN TERM */
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO0_19 (DMA_INTR0) - TA602 */
	{OFFSET(xdma_event_intr0), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO0_20 (DMA_INTR1) - SPI0 nCS1 */
	{OFFSET(xdma_event_intr1), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO0_29 (RMII1_REFCLK) - eMMC nRST */
	{OFFSET(rmii1_refclk), (MODE(7) | PULLUDDIS)},
	/* GPIO0_30 (GPMC_WAIT0) - TA601 */
	{OFFSET(gpmc_wait0), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO0_31 (GPMC_nWP) - SW601 PushButton */
	{OFFSET(gpmc_wpn), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_28 (GPMC_nWE) - FRAM_nWP */
	{OFFSET(gpmc_be1n), (MODE(7) | PULLUDDIS)},
	/* GPIO1_29 (gpmc_csn0) - MMC nRST */
	{OFFSET(gpmc_csn0), (MODE(7) | PULLUDDIS)},
	/* GPIO2_0  (GPMC_nCS3)	- VBAT_OK */
	{OFFSET(gpmc_csn3), (MODE(7) | PULLUDDIS | RXACTIVE) },
	/* GPIO2_2  (GPMC_nADV_ALE) - DCOK */
	{OFFSET(gpmc_advn_ale), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO2_4  (GPMC_nWE) - TST_BAST */
	{OFFSET(gpmc_wen), (MODE(7) | PULLUDDIS)},
	/* GPIO2_5  (gpmc_be0n_cle) - DISPLAY_ON_OFF */
	{OFFSET(gpmc_be0n_cle), (MODE(7) | PULLUDDIS)},
	/* GPIO3_16 (mcasp0_axr0) - ETH-LED green */
	{OFFSET(mcasp0_axr0), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_17 (mcasp0_ahclkr) - CAN_STB */
	{OFFSET(mcasp0_ahclkr), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_18 (MCASP0_ACLKR) - SW601 CNTup, mapped to Counter eQEB0A_in */
	{OFFSET(mcasp0_aclkr), (MODE(1) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_19 (MCASP0_FSR) - SW601 CNTdown, mapped to Counter eQEB0B_in */
	{OFFSET(mcasp0_fsr), (MODE(1) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_20 (MCASP0_AXR1) - SW601 CNTdown, map to Counter eQEB0_index */
	{OFFSET(mcasp0_axr1), (MODE(1) | PULLUDDIS | RXACTIVE)},
	{-1},
};

static struct module_pin_mux uart0_pin_mux[] = {
	/* UART0_CTS */
	{OFFSET(uart0_ctsn), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART0_RXD */
	{OFFSET(uart0_rxd), (MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART0_TXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	/* I2C_DATA */
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C_SCLK */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux mii1_pin_mux[] = {
	{OFFSET(mii1_crs), MODE(0) | RXACTIVE},		/* MII1_CRS */
	{OFFSET(mii1_col), MODE(0) | RXACTIVE},		/* MII1_COL */
	{OFFSET(mii1_rxerr), MODE(0) | RXACTIVE},	/* MII1_RXERR */
	{OFFSET(mii1_txen), MODE(0)},			/* MII1_TXEN */
	{OFFSET(mii1_rxdv), MODE(0) | RXACTIVE},	/* MII1_RXDV */
	{OFFSET(mii1_txd3), MODE(0)},			/* MII1_TXD3 */
	{OFFSET(mii1_txd2), MODE(0)},			/* MII1_TXD2 */
	{OFFSET(mii1_txd1), MODE(0)},			/* MII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(0)},			/* MII1_TXD0 */
	{OFFSET(mii1_txclk), MODE(0) | RXACTIVE},	/* MII1_TXCLK */
	{OFFSET(mii1_rxclk), MODE(0) | RXACTIVE},	/* MII1_RXCLK */
	{OFFSET(mii1_rxd3), MODE(0) | RXACTIVE},	/* MII1_RXD3 */
	{OFFSET(mii1_rxd2), MODE(0) | RXACTIVE},	/* MII1_RXD2 */
	{OFFSET(mii1_rxd1), MODE(0) | RXACTIVE},	/* MII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(0) | RXACTIVE},	/* MII1_RXD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad7), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT7 */
	{OFFSET(gpmc_ad6), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT6 */
	{OFFSET(gpmc_ad5), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT5 */
	{OFFSET(gpmc_ad4), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT4 */
	{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{OFFSET(gpmc_csn0), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_WP */
	{OFFSET(gpmc_advn_ale), (MODE(7) | RXACTIVE | PULLUP_EN)},/* MMC1_CD */

	{-1},
};

static struct module_pin_mux lcd_pin_mux[] = {
	{OFFSET(lcd_data0), (MODE(0) | PULLUDDIS)},	/* LCD-Data(0) */
	{OFFSET(lcd_data1), (MODE(0) | PULLUDDIS)},	/* LCD-Data(1) */
	{OFFSET(lcd_data2), (MODE(0) | PULLUDDIS)},	/* LCD-Data(2) */
	{OFFSET(lcd_data3), (MODE(0) | PULLUDDIS)},	/* LCD-Data(3) */
	{OFFSET(lcd_data4), (MODE(0) | PULLUDDIS)},	/* LCD-Data(4) */
	{OFFSET(lcd_data5), (MODE(0) | PULLUDDIS)},	/* LCD-Data(5) */
	{OFFSET(lcd_data6), (MODE(0) | PULLUDDIS)},	/* LCD-Data(6) */
	{OFFSET(lcd_data7), (MODE(0) | PULLUDDIS)},	/* LCD-Data(7) */
	{OFFSET(lcd_data8), (MODE(0) | PULLUDDIS)},	/* LCD-Data(8) */
	{OFFSET(lcd_data9), (MODE(0) | PULLUDDIS)},	/* LCD-Data(9) */
	{OFFSET(lcd_data10), (MODE(0) | PULLUDDIS)},	/* LCD-Data(10) */
	{OFFSET(lcd_data11), (MODE(0) | PULLUDDIS)},	/* LCD-Data(11) */
	{OFFSET(lcd_data12), (MODE(0) | PULLUDDIS)},	/* LCD-Data(12) */
	{OFFSET(lcd_data13), (MODE(0) | PULLUDDIS)},	/* LCD-Data(13) */
	{OFFSET(lcd_data14), (MODE(0) | PULLUDDIS)},	/* LCD-Data(14) */
	{OFFSET(lcd_data15), (MODE(0) | PULLUDDIS)},	/* LCD-Data(15) */

	{OFFSET(gpmc_ad8), (MODE(1) | PULLUDDIS)},	/* LCD-Data(16) */
	{OFFSET(gpmc_ad9), (MODE(1) | PULLUDDIS)},	/* LCD-Data(17) */
	{OFFSET(gpmc_ad10), (MODE(1) | PULLUDDIS)},	/* LCD-Data(18) */
	{OFFSET(gpmc_ad11), (MODE(1) | PULLUDDIS)},	/* LCD-Data(19) */
	{OFFSET(gpmc_ad12), (MODE(1) | PULLUDDIS)},	/* LCD-Data(20) */
	{OFFSET(gpmc_ad13), (MODE(1) | PULLUDDIS)},	/* LCD-Data(21) */
	{OFFSET(gpmc_ad14), (MODE(1) | PULLUDDIS)},	/* LCD-Data(22) */
	{OFFSET(gpmc_ad15), (MODE(1) | PULLUDDIS)},	/* LCD-Data(23) */

	{OFFSET(lcd_vsync), (MODE(0) | PULLUDDIS)},	/* LCD-VSync */
	{OFFSET(lcd_hsync), (MODE(0) | PULLUDDIS)},	/* LCD-HSync */
	{OFFSET(lcd_ac_bias_en), (MODE(0) | PULLUDDIS)},/* LCD-DE */
	{OFFSET(lcd_pclk), (MODE(0) | PULLUDDIS)},	/* LCD-CLK */

	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(mii1_pin_mux);
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(dcan0_pin_mux);
	configure_module_pin_mux(dcan1_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
	configure_module_pin_mux(lcd_pin_mux);
	configure_module_pin_mux(gpios);
}
