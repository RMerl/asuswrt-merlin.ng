// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Compulab, Ltd.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>
#include "board.h"

static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen),  MODE(2)},
	{OFFSET(mii1_txd3),  MODE(2)},
	{OFFSET(mii1_txd2),  MODE(2)},
	{OFFSET(mii1_txd1),  MODE(2)},
	{OFFSET(mii1_txd0),  MODE(2)},
	{OFFSET(mii1_txclk), MODE(2)},
	{OFFSET(mii1_rxdv),  MODE(2) | RXACTIVE | PULLDOWN_EN},
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE | PULLDOWN_EN},
	{OFFSET(mii1_rxd3),  MODE(2) | RXACTIVE | PULLDOWN_EN},
	{OFFSET(mii1_rxd2),  MODE(2) | RXACTIVE | PULLDOWN_EN},
	{OFFSET(mii1_rxd1),  MODE(2) | RXACTIVE | PULLDOWN_EN},
	{OFFSET(mii1_rxd0),  MODE(2) | RXACTIVE | PULLDOWN_EN},
	{-1},
};

static struct module_pin_mux rgmii2_pin_mux[] = {
	{OFFSET(gpmc_a0),  MODE(2)}, /* txen */
	{OFFSET(gpmc_a2),  MODE(2)}, /* txd3 */
	{OFFSET(gpmc_a3),  MODE(2)}, /* txd2 */
	{OFFSET(gpmc_a4),  MODE(2)}, /* txd1 */
	{OFFSET(gpmc_a5),  MODE(2)}, /* txd0 */
	{OFFSET(gpmc_a6),  MODE(2)}, /* txclk */
	{OFFSET(gpmc_a1),  MODE(2) | RXACTIVE | PULLDOWN_EN}, /* rxvd */
	{OFFSET(gpmc_a7),  MODE(2) | RXACTIVE | PULLDOWN_EN}, /* rxclk */
	{OFFSET(gpmc_a8),  MODE(2) | RXACTIVE | PULLDOWN_EN}, /* rxd3 */
	{OFFSET(gpmc_a9),  MODE(2) | RXACTIVE | PULLDOWN_EN}, /* rxd2 */
	{OFFSET(gpmc_a10), MODE(2) | RXACTIVE | PULLDOWN_EN}, /* rxd1 */
	{OFFSET(gpmc_a11), MODE(2) | RXACTIVE | PULLUP_EN},   /* rxd0 */
	{-1},
};

static struct module_pin_mux mdio_pin_mux[] = {
	{OFFSET(mdio_data), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(mdio_clk),  (MODE(0) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE  | SLEWCTRL)},
	{OFFSET(uart0_txd), (MODE(0) | PULLUDDIS | PULLUP_EN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_clk),  (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(mmc0_cmd),  (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(mmc0_dat0), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(mmc0_dat1), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(mmc0_dat2), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(mmc0_dat3), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{-1},
};

static struct module_pin_mux i2c_pin_mux[] = {
	{OFFSET(i2c0_sda),  (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(i2c0_scl),  (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(spi2_sclk), (MODE(1) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(spi2_cs0),  (MODE(1) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad1),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad2),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad3),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad4),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad5),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad6),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad7),	(MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_wait0),	(MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(gpmc_wpn),	(MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_csn0),	(MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_wen),	(MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_oen_ren),	(MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_advn_ale),	(MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_be0n_cle),	(MODE(0) | PULLDOWN_EN)},
	{-1},
};

static struct module_pin_mux emmc_pin_mux[] = {
	{OFFSET(gpmc_csn1), (MODE(2) | PULLUDDIS | RXACTIVE)}, /* EMMC_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_CMD */
	{OFFSET(gpmc_ad8),  (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT0 */
	{OFFSET(gpmc_ad9),  (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT1 */
	{OFFSET(gpmc_ad10), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT2 */
	{OFFSET(gpmc_ad11), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT3 */
	{OFFSET(gpmc_ad12), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT4 */
	{OFFSET(gpmc_ad13), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT5 */
	{OFFSET(gpmc_ad14), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT6 */
	{OFFSET(gpmc_ad15), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* EMMC_DAT7 */
	{-1},
};

static struct module_pin_mux spi_flash_pin_mux[] = {
	{OFFSET(spi0_d0),   (MODE(0) | RXACTIVE | PULLUDEN)},
	{OFFSET(spi0_d1),   (MODE(0) | RXACTIVE | PULLUDEN)},
	{OFFSET(spi0_cs0),  (MODE(0) | RXACTIVE | PULLUDEN)},
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN)},
	{-1},
};

void set_uart_mux_conf(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void set_mdio_pin_mux(void)
{
	configure_module_pin_mux(mdio_pin_mux);
}

void set_rgmii_pin_mux(void)
{
	configure_module_pin_mux(rgmii1_pin_mux);
	configure_module_pin_mux(rgmii2_pin_mux);
}

void set_mux_conf_regs(void)
{
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(emmc_pin_mux);
	configure_module_pin_mux(i2c_pin_mux);
	configure_module_pin_mux(spi_flash_pin_mux);
	configure_module_pin_mux(nand_pin_mux);
}

void set_i2c_pin_mux(void)
{
	configure_module_pin_mux(i2c_pin_mux);
}
