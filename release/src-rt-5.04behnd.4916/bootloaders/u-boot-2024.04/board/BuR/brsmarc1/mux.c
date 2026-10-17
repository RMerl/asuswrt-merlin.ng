// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * Pinmux Setting for B&R BRSMARC1 Board (HW-Rev. 1)
 *
 * Copyright (C) 2017 Hannes Schmelzer <hannes.schmelzer@br-automation.com>
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>

static struct module_pin_mux spi0_pin_mux[] = {
	/* SPI0_SCLK */
	{OFFSET(spi0_sclk),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI0_D0 */
	{OFFSET(spi0_d0),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI0_D1 */
	{OFFSET(spi0_d1),	MODE(0) | PULLUDEN | RXACTIVE},
	/* SPI0_CS0 */
	{OFFSET(spi0_cs0),	MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},
	/* SPI0_CS1 */
	{OFFSET(spi0_cs1),	MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},
	{-1},
};

static struct module_pin_mux spi1_pin_mux[] = {
	/* SPI1_SCLK */
	{OFFSET(mcasp0_aclkx),	MODE(3) | PULLUDEN | RXACTIVE},
	/* SPI1_D0 */
	{OFFSET(mcasp0_fsx),	MODE(3) | PULLUDEN | RXACTIVE},
	/* SPI1_D1 */
	{OFFSET(mcasp0_axr0),	MODE(3) | PULLUDEN | RXACTIVE},
	/* SPI1_CS0 */
	{OFFSET(mcasp0_ahclkr),	MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},
	/* SPI1_CS1 */
	{OFFSET(xdma_event_intr0), MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},
	{-1},
};

static struct module_pin_mux dcan0_pin_mux[] = {
	/* DCAN0 TX */
	{OFFSET(uart1_ctsn),	MODE(2) | PULLUDEN | PULLUP_EN},
	/* DCAN0 RX */
	{OFFSET(uart1_rtsn),	MODE(2) | RXACTIVE},
	{-1},
};

static struct module_pin_mux dcan1_pin_mux[] = {
	/* DCAN1 TX */
	{OFFSET(uart0_ctsn),	MODE(2) | PULLUDEN | PULLUP_EN},
	/* DCAN1 RX */
	{OFFSET(uart0_rtsn),	MODE(2) | RXACTIVE},
	{-1},
};

static struct module_pin_mux gpios[] = {
	/* GPIO0_7 - LVDS_EN */
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO0_20 - BKLT_PWM (timer7) */
	{OFFSET(xdma_event_intr1), (MODE(4) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO2_4 - DISON */
	{OFFSET(gpmc_wen), (MODE(7) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO1_24 - RGB_EN */
	{OFFSET(gpmc_a8), (MODE(7) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO1_28 - nPD */
	{OFFSET(gpmc_be1n), (MODE(7) | PULLUDEN | PULLUP_EN)},
	/* GPIO2_5 - Watchdog */
	{OFFSET(gpmc_be0n_cle), (MODE(7) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO2_0 - ResetOut */
	{OFFSET(gpmc_csn3), (MODE(7) | PULLUDEN | PULLUP_EN)},
	/* GPIO2_2 - BKLT_EN */
	{OFFSET(gpmc_advn_ale), (MODE(7) | PULLUDDIS | PULLDOWN_EN)},
	/* GPIO1_17 - GPIO0 */
	{OFFSET(gpmc_a1), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_18 - GPIO1 */
	{OFFSET(gpmc_a2), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_19 - GPIO2 */
	{OFFSET(gpmc_a3), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_22 - GPIO3 */
	{OFFSET(gpmc_a6), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_23 - GPIO4 */
	{OFFSET(gpmc_a7), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO1_25 - GPIO5 */
	{OFFSET(gpmc_a9), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_7 - GPIO6 */
	{OFFSET(emu0), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_8 - GPIO7 */
	{OFFSET(emu1), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_18 - GPIO8 */
	{OFFSET(mcasp0_aclkr), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_19 - GPIO9 */
	{OFFSET(mcasp0_fsr), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_20 - GPIO10 */
	{OFFSET(mcasp0_axr1), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO3_21 - GPIO11 */
	{OFFSET(mcasp0_ahclkx), (MODE(7) | PULLUDDIS | RXACTIVE)},
	/* GPIO2_28 - DRAM-strapping */
	{OFFSET(mmc0_dat1), (MODE(7) | PULLUDEN | PULLUP_EN)},
	/* GPIO2_4 - not routed (Pin U6) */
	{OFFSET(gpmc_wen), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO2_5 - not routed (Pin T6) */
	{OFFSET(gpmc_be0n_cle), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO2_28 - not routed (Pin G15) */
	{OFFSET(mmc0_dat1), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO3_18 - not routed (Pin B12) */
	{OFFSET(mcasp0_aclkr), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	{-1},
};

static struct module_pin_mux uart0_pin_mux[] = {
	/* UART0_RXD */
	{OFFSET(uart0_rxd), (MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART0_TXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux uart234_pin_mux[] = {
	/* UART2_RXD */
	{OFFSET(mii1_txclk), (MODE(1) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART2_TXD */
	{OFFSET(mii1_rxclk), (MODE(1) | PULLUDEN)},

	/* UART3_RXD */
	{OFFSET(mii1_rxd3), (MODE(1) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART3_TXD */
	{OFFSET(mmc0_dat0), (MODE(3) | PULLUDEN)},
	/* UART3_RTS */
	{OFFSET(mmc0_cmd), (MODE(2) | PULLUDEN)},
	/* UART3_CTS */
	{OFFSET(mmc0_clk), (MODE(2) | PULLUDEN | PULLUP_EN | RXACTIVE)},

	/* UART4_RXD */
	{OFFSET(mii1_txd3), (MODE(3) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART4_TXD */
	{OFFSET(mii1_txd2), (MODE(3) | PULLUDEN)},
	/* UART4_RTS */
	{OFFSET(mmc0_dat2), (MODE(3) | PULLUDEN)},
	/* UART4_CTS */
	{OFFSET(mmc0_dat3), (MODE(3) | PULLUDEN | PULLUP_EN | RXACTIVE)},

	{-1},
};

static struct module_pin_mux i2c_pin_mux[] = {
	/* I2C0_DATA */
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C0_SCLK */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C1_DATA */
	{OFFSET(uart1_rxd), (MODE(3) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C1_SCLK */
	{OFFSET(uart1_txd), (MODE(3) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux eth_pin_mux[] = {
	/* ETH1 */
	{OFFSET(rmii1_refclk), MODE(0) | RXACTIVE},	/* ETH1_REFCLK */
	{OFFSET(mii1_crs), MODE(1) | RXACTIVE},		/* RMII1_CRSDV */
	{OFFSET(mii1_rxerr), MODE(1) | RXACTIVE},	/* RMII1_RXER */
	{OFFSET(mii1_txen), MODE(1)},			/* RMII1_TXEN */
	{OFFSET(mii1_rxd0), MODE(1) | RXACTIVE},	/* RMII1_RXD0 */
	{OFFSET(mii1_rxd1), MODE(1) | RXACTIVE},	/* RMII1_RXD1 */
	{OFFSET(mii1_txd0), MODE(1)},			/* RMII1_TXD0 */
	{OFFSET(mii1_txd1), MODE(1)},			/* RMII1_TXD1 */

	/* ETH2 */
	{OFFSET(mii1_col), MODE(1) | RXACTIVE},		/* ETH2_REFCLK */
	{OFFSET(gpmc_wait0), MODE(3) | RXACTIVE},	/* RMII2_CRSDV */
	{OFFSET(gpmc_wpn), MODE(3) | RXACTIVE},		/* RMII2_RXER */
	{OFFSET(gpmc_a0), MODE(3)},			/* RMII2_TXEN */
	{OFFSET(gpmc_a11), MODE(3) | RXACTIVE},		/* RMII2_RXD0 */
	{OFFSET(gpmc_a10), MODE(3) | RXACTIVE},		/* RMII2_RXD1 */
	{OFFSET(gpmc_a5), MODE(3)},			/* RMII2_TXD0 */
	{OFFSET(gpmc_a4), MODE(3)},			/* RMII2_TXD1 */

	/* gpio2_19, gpio 3_4, not connected on board */
	{OFFSET(mii1_rxd2), MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},
	{OFFSET(mii1_rxdv), MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE},

	/* ETH Management */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},	/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},		/* MDIO_CLK */

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
	configure_module_pin_mux(i2c_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(eth_pin_mux);
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(spi1_pin_mux);
	configure_module_pin_mux(dcan0_pin_mux);
	configure_module_pin_mux(dcan1_pin_mux);
	configure_module_pin_mux(uart234_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
	configure_module_pin_mux(lcd_pin_mux);
	configure_module_pin_mux(gpios);
}
