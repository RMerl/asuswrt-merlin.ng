// SPDX-License-Identifier: GPL-2.0+
/*
 * Muxing for Gumstix Pepper and AM335x-based boards
 *
 * Copyright (C) 2014, Gumstix, Incorporated - http://www.gumstix.com/
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},  /* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},              /* UART0_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},  /* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},   /* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},   /* MMC0_CMD */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},   /* MMC0_CD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	/* I2C_DATA */
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	/* I2C_SCLK */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(2)},                   /* RGMII1_TCTL */
	{OFFSET(mii1_rxdv), MODE(2) | RXACTIVE},        /* RGMII1_RCTL */
	{OFFSET(mii1_txd3), MODE(2)},                   /* RGMII1_TD3 */
	{OFFSET(mii1_txd2), MODE(2)},                   /* RGMII1_TD2 */
	{OFFSET(mii1_txd1), MODE(2)},                   /* RGMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(2)},                   /* RGMII1_TD0 */
	{OFFSET(mii1_txclk), MODE(2)},                  /* RGMII1_TCLK */
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE},       /* RGMII1_RCLK */
	{OFFSET(mii1_rxd3), MODE(2) | RXACTIVE},        /* RGMII1_RD3 */
	{OFFSET(mii1_rxd2), MODE(2) | RXACTIVE},        /* RGMII1_RD2 */
	{OFFSET(mii1_rxd1), MODE(2) | RXACTIVE},        /* RGMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(2) | RXACTIVE},        /* RGMII1_RD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},        /* MDIO_CLK */
	{OFFSET(rmii1_refclk), MODE(7) | RXACTIVE},     /* ETH_INT */
	{OFFSET(mii1_col), MODE(7) | PULLUP_EN},        /* PHY_NRESET */
	{OFFSET(xdma_event_intr1), MODE(3)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

/*
 * Do board-specific muxes.
 */
void enable_board_pin_mux(void)
{
	/* I2C0 */
	configure_module_pin_mux(i2c0_pin_mux);
	/* SD Card */
	configure_module_pin_mux(mmc0_pin_mux);
	/* Ethernet pinmux. */
	configure_module_pin_mux(rgmii1_pin_mux);
}
