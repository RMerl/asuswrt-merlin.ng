// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * (C) Copyright 2016
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUDEN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDDIS)},		/* UART0_TXD */
	{OFFSET(uart0_ctsn), (MODE(0) | PULLUDEN | RXACTIVE)},	/* UART0_CTS */
	{OFFSET(uart0_rtsn), (MODE(0) | PULLUDDIS)},		/* UART0_RTS */
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	{OFFSET(uart1_rxd), (MODE(0) | PULLUDDIS | RXACTIVE)},	/* UART1_RXD */
	{OFFSET(uart1_txd), (MODE(0) | PULLUDDIS)},		/* UART1_TXD */
	{OFFSET(uart1_ctsn), (MODE(0) | PULLUDEN | RXACTIVE)},	/* UART1_CTS */
	{OFFSET(uart1_rtsn), (MODE(0) | PULLUDDIS)},		/* UART1_RTS */
	{-1},
};

static struct module_pin_mux uart2_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(1) | PULLUDDIS | RXACTIVE)},	/* UART2_RXD */
	{OFFSET(spi0_d0), (MODE(1) | PULLUDDIS)},		/* UART2_TXD */
	{-1},
};

static struct module_pin_mux spi1_pin_mux[] = {
	{OFFSET(mcasp0_aclkx), (MODE(3) | PULLUDEN | RXACTIVE)},/* SPI1_SCLK */
	{OFFSET(mcasp0_fsx), (MODE(3) | PULLUDEN | RXACTIVE)},/* SPI1_D0 */
	{OFFSET(mcasp0_axr0), (MODE(3) | PULLUDEN | RXACTIVE)},/* SPI1_D1 */
	{OFFSET(mcasp0_ahclkr), (MODE(3) | PULLUDEN | RXACTIVE)},/* SPI1_CS0 */
	{-1},
};

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUP_EN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUDDIS)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUDDIS)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUDDIS)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUDDIS)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUDDIS)},	/* MMC0_CMD */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUDDIS)},	/* MMC0_CD */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad7), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad6), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad5), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad4), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUDDIS)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{-1},
};

static struct module_pin_mux mmc2_pin_mux[] = {
	{OFFSET(gpmc_ad12), (MODE(3) | PULLUDDIS | RXACTIVE)}, /* MMC2_DAT0 */
	{OFFSET(gpmc_ad13), (MODE(3) | PULLUDDIS | RXACTIVE)}, /* MMC2_DAT1 */
	{OFFSET(gpmc_ad14), (MODE(3) | PULLUDDIS | RXACTIVE)}, /* MMC2_DAT2 */
	{OFFSET(gpmc_ad15), (MODE(3) | PULLUDDIS | RXACTIVE)}, /* MMC2_DAT3 */
	{OFFSET(gpmc_csn3), (MODE(3) | RXACTIVE | PULLUDDIS)}, /* MMC2_CMD */
	{OFFSET(gpmc_clk), (MODE(3) | RXACTIVE | PULLUDDIS)},  /* MMC2_CLK */
	{-1},
};
static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDDIS)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDDIS)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux gpio0_7_pin_mux[] = {
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUP_EN)},	/* GPIO0_7 */
	{-1},
};

static struct module_pin_mux jtag_pin_mux[] = {
	{OFFSET(xdma_event_intr0), (MODE(6) | RXACTIVE | PULLUDDIS)},
	{OFFSET(xdma_event_intr1), (MODE(6) | RXACTIVE | PULLUDDIS)},
	{OFFSET(nresetin_out), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(nnmi), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(tms), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(tdi), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(tdo), (MODE(0) | PULLUP_EN)},
	{OFFSET(tck), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(ntrst), (MODE(0) | RXACTIVE)},
	{OFFSET(emu0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(emu1), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(pmic_power_en), (MODE(0) | PULLUP_EN)},
	{OFFSET(rsvd2), (MODE(0) | PULLUP_EN)},
	{OFFSET(rtc_porz), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ext_wakeup), (MODE(0) | RXACTIVE)},
	{OFFSET(enz_kaldo_1p8v), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_drvvbus), (MODE(0) | PULLUDEN)},
	{OFFSET(usb1_drvvbus), (MODE(0) | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux gpio_pin_mux[] = {
	{OFFSET(gpmc_ad8), (MODE(7) | PULLUDDIS)},	/* gpio0[22] - LED_PWR_BL (external pull-down) */
	{OFFSET(gpmc_ad9), (MODE(7) | PULLUDDIS)},	/* gpio0[23] - LED_PWR_RD (external pull-down) */
	{OFFSET(gpmc_ad10), (MODE(7) | PULLUDDIS)},	/* gpio0[26] - LED_LAN_RD (external pull-down) */
	{OFFSET(gpmc_ad11), (MODE(7) | PULLUDDIS)},	/* gpio0[27] - #WIFI_RST (external pull-down) */
	{OFFSET(gpmc_a0), (MODE(7) | PULLUDDIS)},	/* gpio1[16] - WIFI_REGEN */
	{OFFSET(gpmc_a1), (MODE(7) | PULLUDDIS)},	/* gpio1[17] - LED_LAN_BL */
	{OFFSET(gpmc_a2), (MODE(7) | PULLUDDIS)},	/* gpio1[18] - LED_Cloud_BL */
	{OFFSET(gpmc_a3), (MODE(7) | PULLUDDIS)},	/* gpio1[19] -  LED_PWM as GPIO */
	{OFFSET(gpmc_a4), (MODE(7))},			/* gpio1[20] -  #eMMC_RST */
	{OFFSET(gpmc_a5), (MODE(7) | PULLUDDIS)},	/* gpio1[21] -  #Z-Wave_RST */
	{OFFSET(gpmc_a6), (MODE(7) | PULLUDDIS)},	/* gpio1[22] -  ENOC_RST */
	{OFFSET(gpmc_a7), (MODE(7) | PULLUP_EN)},	/* gpio1[23] -  WIFI_MODE */
	{OFFSET(gpmc_a8), (MODE(7) | RXACTIVE | PULLUDDIS)},	/* gpio1[24] -  #BIDCOS_RST */
	{OFFSET(gpmc_a9), (MODE(7) | RXACTIVE | PULLUDDIS)},	/* gpio1[25] -  USR_BUTTON */
	{OFFSET(gpmc_a10), (MODE(7) | RXACTIVE | PULLUDDIS)},	/* gpio1[26] -  #USB1_OC */
	{OFFSET(gpmc_a11), (MODE(7) | RXACTIVE | PULLUDDIS)},	/* gpio1[27] -  BIDCOS_PROG */
	{OFFSET(gpmc_be1n), (MODE(7) | PULLUP_EN)},	/* gpio1[28] -  ZIGBEE_PC7 */
	{OFFSET(gpmc_csn0), (MODE(7) | RXACTIVE | PULLUDDIS)},	/* gpio1[29] -  RESET_BUTTON */
	{OFFSET(gpmc_advn_ale), (MODE(7) | PULLUDDIS)},	/* gpio2[2] -  LED_Cloud_RD */
	{OFFSET(gpmc_oen_ren), (MODE(7) | PULLUDDIS | RXACTIVE)}, /* gpio2[3] -  #WIFI_POR */
	{OFFSET(gpmc_wen), (MODE(7) | PULLUDDIS)},	/* gpio2[4] -  N/C */
	{OFFSET(gpmc_be0n_cle), (MODE(7) | PULLUDDIS)},	/* gpio2[5] -  EEPROM_WP */
	{OFFSET(lcd_data0), (MODE(7) | PULLUDDIS)},	/* gpio2[6] */
	{OFFSET(lcd_data1), (MODE(7) | PULLUDDIS)},	/* gpio2[7] */
	{OFFSET(lcd_data2), (MODE(7) | PULLUDDIS)},	/* gpio2[8] */
	{OFFSET(lcd_data3), (MODE(7) | PULLUDDIS)},	/* gpio2[9] */
	{OFFSET(lcd_data4), (MODE(7) | PULLUDDIS)},	/* gpio2[10] */
	{OFFSET(lcd_data5), (MODE(7) | PULLUDDIS)},	/* gpio2[11] */
	{OFFSET(lcd_data6), (MODE(7) | PULLUDDIS)},	/* gpio2[12] */
	{OFFSET(lcd_data7), (MODE(7) | PULLUDDIS)},	/* gpio2[13] */
	{OFFSET(lcd_data8), (MODE(7) | PULLUDDIS)},	/* gpio2[14] */
	{OFFSET(lcd_data9), (MODE(7) | PULLUDDIS)},	/* gpio2[15] */
	{OFFSET(lcd_data10), (MODE(7) | PULLUDDIS)},	/* gpio2[16] */
	{OFFSET(lcd_data11), (MODE(7) | PULLUDDIS)},	/* gpio2[17] */
	{OFFSET(lcd_data12), (MODE(7) | PULLUDDIS)},	/* gpio0[8] */
	{OFFSET(lcd_data13), (MODE(7) | PULLUDDIS)},	/* gpio0[9] */
	{OFFSET(lcd_data14), (MODE(7) | PULLUDDIS)},	/* gpio0[10] */
	{OFFSET(lcd_data15), (MODE(7) | PULLUDDIS)},	/* gpio0[11] */
	{OFFSET(lcd_vsync), (MODE(7) | PULLUDDIS)},	/* gpio2[22] */
	{OFFSET(lcd_hsync), (MODE(7) | PULLUDDIS)},	/* gpio2[23] */
	{OFFSET(lcd_pclk), (MODE(7) | PULLUDDIS)},	/* gpio2[24] */
	{OFFSET(lcd_ac_bias_en), (MODE(7) | PULLUDDIS)},/* gpio2[25] */
	{OFFSET(spi0_d1), (MODE(7) | PULLUDDIS)},	/* gpio0[4] */
	{OFFSET(spi0_cs0), (MODE(7) | PULLUDDIS)},	/* gpio0[5] */
	{OFFSET(mcasp0_aclkr), (MODE(7) | PULLUDDIS)},	/* gpio3[18] - #ZIGBEE_RST */
	{OFFSET(mcasp0_fsr), (MODE(7)) | PULLUDDIS},	/* gpio3[19] - ZIGBEE_BOOT */
	{OFFSET(mcasp0_axr1), (MODE(7) | RXACTIVE)},	/* gpio3[19] - ZIGBEE_BOOT */
	{OFFSET(mcasp0_ahclkx), (MODE(7) | RXACTIVE | PULLUP_EN)},/* gpio3[21] - ZIGBEE_PC5 */
	{-1},
};

static struct module_pin_mux mii1_pin_mux[] = {
	{OFFSET(mii1_col), MODE(0) | RXACTIVE},
	{OFFSET(mii1_crs), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxerr), MODE(0) | RXACTIVE},
	{OFFSET(mii1_txen), MODE(0)},
	{OFFSET(mii1_rxdv), MODE(0) | RXACTIVE},
	{OFFSET(mii1_txd3), MODE(0)},
	{OFFSET(mii1_txd2), MODE(0)},
	{OFFSET(mii1_txd1), MODE(0) | RXACTIVE},
	{OFFSET(mii1_txd0), MODE(0) | RXACTIVE},
	{OFFSET(mii1_txclk), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxclk), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxd3), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxd2), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxd1), MODE(0) | RXACTIVE},
	{OFFSET(mii1_rxd0), MODE(0) | RXACTIVE},
	{OFFSET(rmii1_refclk), MODE(7) | RXACTIVE},
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},
	{-1},
};

static struct module_pin_mux pwm_pin_mux[] = {
	{OFFSET(gpmc_a3), (MODE(6) | PULLUDDIS)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart1_pin_mux(void)
{
	configure_module_pin_mux(uart1_pin_mux);
}

void enable_uart2_pin_mux(void)
{
	configure_module_pin_mux(uart2_pin_mux);
}

void enable_uart3_pin_mux(void)
{
}

void enable_uart4_pin_mux(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_uart5_pin_mux(void)
{
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_shc_board_pwm_pin_mux(void)
{
	configure_module_pin_mux(pwm_pin_mux);
}

void enable_shc_board_pin_mux(void)
{
	/* Do board-specific muxes. */
	if (board_is_c3_sample() || board_is_series()) {
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
		configure_module_pin_mux(mmc1_pin_mux);
		configure_module_pin_mux(mmc2_pin_mux);
		configure_module_pin_mux(i2c0_pin_mux);
		configure_module_pin_mux(gpio0_7_pin_mux);
		configure_module_pin_mux(gpio_pin_mux);
		configure_module_pin_mux(uart1_pin_mux);
		configure_module_pin_mux(uart2_pin_mux);
		configure_module_pin_mux(uart4_pin_mux);
		configure_module_pin_mux(spi1_pin_mux);
		configure_module_pin_mux(jtag_pin_mux);
	} else {
		puts("Unknown board, cannot configure pinmux.");
		hang();
	}
}
