// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * Copyright (C) 2013 Lothar Felten <lothar.felten@gmail.com>
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include "board.h"

/* UART0 pins E15(rx),E16(tx) [E17(rts),E18(cts)] */
static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

/* unused: UART1 pins D15(tx),D16(rx),D17(cts),D18(rts) */

/* I2C pins C16(scl)/C17(sda) */
static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
					PULLUDEN | SLEWCTRL)}, /* I2C0_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
					PULLUDEN | SLEWCTRL)}, /* I2C0_SCLK */
	{-1},
};

/* MMC0 pins */
static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_CMD */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},  /* MMC0_CD */
	{-1},
};

/* MII pins */
static struct module_pin_mux mii1_pin_mux[] = {
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

/* NAND pins */
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
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)}, /* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},	/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},	/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},	/* NAND_BE_CLE */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_board_pin_mux()
{
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(uart0_pin_mux);
	configure_module_pin_mux(mii1_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(nand_pin_mux);
}
