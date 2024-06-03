// SPDX-License-Identifier: GPL-2.0+
/*
 * pinmux setup for siemens pxm2 board
 *
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/mux.c
 *
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
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{OFFSET(nnmi), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_TXD */
	{-1},
};

#ifdef CONFIG_NAND
static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD0 */
	{OFFSET(gpmc_ad1), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD1 */
	{OFFSET(gpmc_ad2), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD2 */
	{OFFSET(gpmc_ad3), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD3 */
	{OFFSET(gpmc_ad4), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD4 */
	{OFFSET(gpmc_ad5), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD5 */
	{OFFSET(gpmc_ad6), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD6 */
	{OFFSET(gpmc_ad7), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD7 */
	{OFFSET(gpmc_wait0), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* NAND WAIT */
	{OFFSET(gpmc_wpn), (MODE(7) | PULLUP_EN | RXACTIVE)},	/* NAND_WPN */
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUDEN)},	/* NAND_CS0 */
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)},	/* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},	/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},	/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},	/* NAND_BE_CLE */
	{OFFSET(gpmc_a11), MODE(7) | RXACTIVE | PULLUP_EN}, /* RGMII2_RD0 */
	{OFFSET(mcasp0_ahclkx), MODE(7) | PULLUDEN},	/* MCASP0_AHCLKX */
	{-1},
};
#endif

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux i2c1_pin_mux[] = {
	{OFFSET(spi0_d1), (MODE(2) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{OFFSET(spi0_cs0), (MODE(2) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

#ifndef CONFIG_NO_ETH
static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(2)},			/* RGMII1_TCTL */
	{OFFSET(mii1_rxdv), MODE(2) | RXACTIVE},	/* RGMII1_RCTL */
	{OFFSET(mii1_txd3), MODE(2)},			/* RGMII1_TD3 */
	{OFFSET(mii1_txd2), MODE(2)},			/* RGMII1_TD2 */
	{OFFSET(mii1_txd1), MODE(2)},			/* RGMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(2)},			/* RGMII1_TD0 */
	{OFFSET(mii1_txclk), MODE(2)},			/* RGMII1_TCLK */
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE},	/* RGMII1_RCLK */
	{OFFSET(mii1_rxd3), MODE(2) | RXACTIVE},	/* RGMII1_RD3 */
	{OFFSET(mii1_rxd2), MODE(2) | RXACTIVE},	/* RGMII1_RD2 */
	{OFFSET(mii1_rxd1), MODE(2) | RXACTIVE},	/* RGMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(2) | RXACTIVE},	/* RGMII1_RD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux rgmii2_pin_mux[] = {
	{OFFSET(gpmc_a0), MODE(2)},			/* RGMII2_TCTL */
	{OFFSET(gpmc_a1), MODE(2) | RXACTIVE},		/* RGMII2_RCTL */
	{OFFSET(gpmc_a2), MODE(2)},			/* RGMII2_TD3 */
	{OFFSET(gpmc_a3), MODE(2)},			/* RGMII2_TD2 */
	{OFFSET(gpmc_a4), MODE(2)},			/* RGMII2_TD1 */
	{OFFSET(gpmc_a5), MODE(2)},			/* RGMII2_TD0 */
	{OFFSET(gpmc_a6), MODE(7)},			/* RGMII2_TCLK */
	{OFFSET(gpmc_a7), MODE(2) | RXACTIVE},		/* RGMII2_RCLK */
	{OFFSET(gpmc_a8), MODE(2) | RXACTIVE},		/* RGMII2_RD3 */
	{OFFSET(gpmc_a9), MODE(7)},			/* RGMII2_RD2 */
	{OFFSET(gpmc_a10), MODE(2) | RXACTIVE},		/* RGMII2_RD1 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};
#endif

#ifdef CONFIG_MMC
static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUDEN)},	/* MMC0_CD */
	{-1},
};
#endif

static struct module_pin_mux lcdc_pin_mux[] = {
	{OFFSET(lcd_data0), (MODE(0) | PULLUDDIS)},	/* LCD_DAT0 */
	{OFFSET(lcd_data1), (MODE(0) | PULLUDDIS)},	/* LCD_DAT1 */
	{OFFSET(lcd_data2), (MODE(0) | PULLUDDIS)},	/* LCD_DAT2 */
	{OFFSET(lcd_data3), (MODE(0) | PULLUDDIS)},	/* LCD_DAT3 */
	{OFFSET(lcd_data4), (MODE(0) | PULLUDDIS)},	/* LCD_DAT4 */
	{OFFSET(lcd_data5), (MODE(0) | PULLUDDIS)},	/* LCD_DAT5 */
	{OFFSET(lcd_data6), (MODE(0) | PULLUDDIS)},	/* LCD_DAT6 */
	{OFFSET(lcd_data7), (MODE(0) | PULLUDDIS)},	/* LCD_DAT7 */
	{OFFSET(lcd_data8), (MODE(0) | PULLUDDIS)},	/* LCD_DAT8 */
	{OFFSET(lcd_data9), (MODE(0) | PULLUDDIS)},	/* LCD_DAT9 */
	{OFFSET(lcd_data10), (MODE(0) | PULLUDDIS)},	/* LCD_DAT10 */
	{OFFSET(lcd_data11), (MODE(0) | PULLUDDIS)},	/* LCD_DAT11 */
	{OFFSET(lcd_data12), (MODE(0) | PULLUDDIS)},	/* LCD_DAT12 */
	{OFFSET(lcd_data13), (MODE(0) | PULLUDDIS)},	/* LCD_DAT13 */
	{OFFSET(lcd_data14), (MODE(0) | PULLUDDIS)},	/* LCD_DAT14 */
	{OFFSET(lcd_data15), (MODE(0) | PULLUDDIS)},	/* LCD_DAT15 */
	{OFFSET(gpmc_ad8), (MODE(1))},			/* LCD_DAT16 */
	{OFFSET(gpmc_ad9), (MODE(1))},		/* LCD_DAT17 */
	{OFFSET(gpmc_ad10), (MODE(1))},		/* LCD_DAT18 */
	{OFFSET(gpmc_ad11), (MODE(1))},		/* LCD_DAT19 */
	{OFFSET(gpmc_ad12), (MODE(1))},		/* LCD_DAT20 */
	{OFFSET(gpmc_ad13), (MODE(1))},		/* LCD_DAT21 */
	{OFFSET(gpmc_ad14), (MODE(1))},		/* LCD_DAT22 */
	{OFFSET(gpmc_ad15), (MODE(1))},		/* LCD_DAT23 */
	{OFFSET(lcd_vsync), (MODE(0))},		/* LCD_VSYNC */
	{OFFSET(lcd_hsync), (MODE(0))},		/* LCD_HSYNC */
	{OFFSET(lcd_pclk), (MODE(0))},		/* LCD_PCLK */
	{OFFSET(lcd_ac_bias_en), (MODE(0))},	/* LCD_AC_BIAS_EN */
	{-1},
};

static struct module_pin_mux ecap0_pin_mux[] = {
	{OFFSET(ecap0_in_pwm0_out), (MODE(0))},
	{-1},
};

static struct module_pin_mux gpio_pin_mux[] = {
	{OFFSET(mcasp0_fsx), MODE(7)}, /* GPIO3_15 LCD power*/
	{OFFSET(mcasp0_axr0), MODE(7)}, /* GPIO3_16 Backlight */
	{OFFSET(gpmc_a9), MODE(7)}, /* GPIO1_25 Touch power */
	{-1},
};
void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
	configure_module_pin_mux(i2c1_pin_mux);
#ifdef CONFIG_NAND
	configure_module_pin_mux(nand_pin_mux);
#endif
#ifndef CONFIG_NO_ETH
	configure_module_pin_mux(rgmii1_pin_mux);
	configure_module_pin_mux(rgmii2_pin_mux);
#endif
#ifdef CONFIG_MMC
	configure_module_pin_mux(mmc0_pin_mux);
#endif
	configure_module_pin_mux(lcdc_pin_mux);
	configure_module_pin_mux(gpio_pin_mux);
	configure_module_pin_mux(ecap0_pin_mux);
}
