// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinmux configuration for Compulab CM-T335 board
 *
 * Copyright (C) 2013, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Ilya Ledvich <ilya@compulab.co.il>
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	{OFFSET(uart1_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(uart1_txd), (MODE(0) | PULLUDEN)},
	{OFFSET(uart1_ctsn), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(uart1_rtsn), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDDIS | SLEWCTRL)},
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDDIS | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux i2c1_pin_mux[] = {
	/* I2C_DATA */
	{OFFSET(uart0_ctsn), (MODE(3) | RXACTIVE | PULLUDDIS | SLEWCTRL)},
	/* I2C_SCLK */
	{OFFSET(uart0_rtsn), (MODE(3) | RXACTIVE | PULLUDDIS | SLEWCTRL)},
	{-1},
};

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
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

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
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUDEN)},		/* NAND_CS0 */
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)},	/* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},	/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},	/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},	/* NAND_BE_CLE */
	{-1},
};

static struct module_pin_mux eth_phy_rst_pin_mux[] = {
	{OFFSET(emu0), (MODE(7) | PULLUDDIS)},	/* GPIO3_7 */
	{-1},
};

static struct module_pin_mux status_led_pin_mux[] = {
	{OFFSET(gpmc_csn3), (MODE(7) | PULLUDEN)},	/* GPIO2_0 */
	{-1},
};

void set_uart_mux_conf(void)
{
	configure_module_pin_mux(uart0_pin_mux);
	configure_module_pin_mux(uart1_pin_mux);
}

void set_mux_conf_regs(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(i2c1_pin_mux);
	configure_module_pin_mux(rgmii1_pin_mux);
	configure_module_pin_mux(eth_phy_rst_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(nand_pin_mux);
	configure_module_pin_mux(status_led_pin_mux);
}
