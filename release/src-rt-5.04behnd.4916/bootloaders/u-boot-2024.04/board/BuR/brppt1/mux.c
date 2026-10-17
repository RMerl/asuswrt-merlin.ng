// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * Pinmux Setting for B&R BRPPT1 Board(s)
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

static struct module_pin_mux uart0_pin_mux[] = {
	/* UART0_RTS */
	{OFFSET(uart0_rtsn), (MODE(0) | PULLUDEN)},
	/* UART0_CTS */
	{OFFSET(uart0_ctsn), (MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART0_RXD */
	{OFFSET(uart0_rxd), (MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART0_TXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	/* UART1_RTS as I2C2-SCL */
	{OFFSET(uart1_rtsn), (MODE(3) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART1_CTS as I2C2-SDA */
	{OFFSET(uart1_ctsn), (MODE(3) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART1_RXD */
	{OFFSET(uart1_rxd), (MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* UART1_TXD */
	{OFFSET(uart1_txd), (MODE(0) | PULLUDEN)},
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

static struct module_pin_mux i2c0_pin_mux[] = {
	/* I2C_DATA */
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C_SCLK */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	/* SPI0_SCLK */
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},
	/* SPI0_D0 */
	{OFFSET(spi0_d0), (MODE(0) | RXACTIVE |	PULLUDEN | PULLUP_EN)},
	/* SPI0_D1 */
	{OFFSET(spi0_d1), (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},
	/* SPI0_CS0 */
	{OFFSET(spi0_cs0), (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},
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
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux mii2_pin_mux[] = {
	{OFFSET(gpmc_a0), MODE(1)},		/* MII2_TXEN */
	{OFFSET(gpmc_a1), MODE(1) | RXACTIVE},	/* MII2_RXDV */
	{OFFSET(gpmc_a2), MODE(1)},		/* MII2_TXD3 */
	{OFFSET(gpmc_a3), MODE(1)},		/* MII2_TXD2 */
	{OFFSET(gpmc_a4), MODE(1)},		/* MII2_TXD1 */
	{OFFSET(gpmc_a5), MODE(1)},		/* MII2_TXD0 */
	{OFFSET(gpmc_a6), MODE(1) | RXACTIVE},	/* MII2_TXCLK */
	{OFFSET(gpmc_a7), MODE(1) | RXACTIVE},	/* MII2_RXCLK */
	{OFFSET(gpmc_a8), MODE(1) | RXACTIVE},	/* MII2_RXD3 */
	{OFFSET(gpmc_a9), MODE(1) | RXACTIVE},	/* MII2_RXD2 */
	{OFFSET(gpmc_a10), MODE(1) | RXACTIVE},	/* MII2_RXD1 */
	{OFFSET(gpmc_a11), MODE(1) | RXACTIVE},	/* MII2_RXD0 */
	{OFFSET(gpmc_wpn), (MODE(1) | RXACTIVE)},/* MII2_RXERR */
	{OFFSET(gpmc_wait0), (MODE(1) | RXACTIVE | PULLUP_EN)},
						/*
						 * MII2_CRS is shared with
						 * NAND_WAIT0
						 */
	{OFFSET(gpmc_be1n), (MODE(1) | RXACTIVE)},/* MII1_COL */
	{-1},
};

static struct module_pin_mux gpIOs[] = {
	/* GPIO0_6  (SPI0_CS1) - 3v3_PWR_nEN (Display Power Supply) */
	{OFFSET(spi0_cs1),  (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* TIMER5   (MMC0_DAT3) - TIMER5 (Buzzer) */
	{OFFSET(mmc0_dat3), (MODE(3) | PULLUDEN | RXACTIVE)},
	/* TIMER6   (MMC0_DAT2) - PWM_BACK_3V3 */
	{OFFSET(mmc0_dat2), (MODE(3) | PULLUDEN | RXACTIVE)},
	/* GPIO2_28 (MMC0_DAT1)	 - MII_nNAND */
	{OFFSET(mmc0_dat1), (MODE(7) | PULLUDEN | RXACTIVE)},
	/* GPIO2_29 (MMC0_DAT0)	 - NAND_1n0 */
	{OFFSET(mmc0_dat0), (MODE(7) | PULLUDEN | RXACTIVE)},
	/* GPIO2_30 (MMC0_CLK) - nRESET (PHY) */
	{OFFSET(mmc0_clk), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO3_18 (MCASP0_ACLKR) - CPLD JTAG TDI */
	{OFFSET(mcasp0_aclkr), (MODE(7) | PULLUDEN | PULLUP_EN  | RXACTIVE)},
	/* GPIO3_19 (MCASP0_FSR) - CPLD JTAG TMS */
	{OFFSET(mcasp0_fsr), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO3_20 (MCASP0_AXR1) - CPLD JTAG TCK */
	{OFFSET(mcasp0_axr1), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO3_21 (MCASP0_AHCLKX) - CPLD JTAG TDO */
	{OFFSET(mcasp0_ahclkx), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO2_0  (GPMC_nCS3) - DCOK */
	{OFFSET(gpmc_csn3), (MODE(7) | PULLUDDIS | RXACTIVE) },
	/* GPIO0_29 (RMII1_REFCLK) - eMMC nRST */
	{OFFSET(rmii1_refclk), (MODE(7) | PULLUDDIS | RXACTIVE) },
	/*
	 * GPIO0_7 (PWW0 OUT)
	 * DISPLAY_ONOFF (Backlight Enable at LVDS Versions)
	 */
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUDEN | RXACTIVE)},
	/* GPIO0_19 (DMA_INTR0) - DISPLAY_MODE (CPLD) */
	{OFFSET(xdma_event_intr0), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	/* GPIO0_20 (DMA_INTR1) - REP-Switch */
	{OFFSET(xdma_event_intr1), (MODE(7) | PULLUP_EN | RXACTIVE)},
	/* GPIO3_14 (MCASP0_ACLKX) - frei / PP709 */
	{OFFSET(mcasp0_aclkx), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE) },
	/* GPIO3_15 (MCASP0_FSX) - PMIC_nRESET */
	{OFFSET(mcasp0_fsx),   (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE) },
	/* GPIO3_16 (MCASP0_AXR0) - ETH1_LEDY */
	{OFFSET(mcasp0_axr0),  (MODE(7) | PULLUDDIS) },
	/* GPIO3_17 (MCASP0_AHCLKR) - ETH2_LEDY */
	{OFFSET(mcasp0_ahclkr), (MODE(7) | PULLUDDIS) },
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
	configure_module_pin_mux(mii2_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(lcd_pin_mux);
	configure_module_pin_mux(uart1_pin_mux);
	configure_module_pin_mux(gpIOs);
}
