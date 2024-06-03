// SPDX-License-Identifier: GPL-2.0+
/*
 * mux.c
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 * Copyright (C) 2018 Robert Bosch Power Tools GmbH
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux adc_voltages_en[] = {
	{OFFSET(mcasp0_ahclkx), (MODE(7) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux asp_power_en[] = {
	{OFFSET(mcasp0_aclkx), (MODE(7) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux switch_off_3v6_pin_mux[] = {
	{OFFSET(mii1_txd0), (MODE(7) | PULLUP_EN)},
	/*
	 * The uart1 lines are made floating inputs, based on the Guardian
	 * A2 Sample Power Supply Schematics
	 */
	{OFFSET(uart1_rxd), (MODE(7) | PULLUDDIS)},
	{OFFSET(uart1_txd), (MODE(7) | PULLUDDIS)},
	{-1},
};

#ifdef CONFIG_NAND
static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad1),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad2),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad3),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad4),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad5),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad6),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad7),      (MODE(0) | PULLUDDIS | RXACTIVE)},
#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
	{OFFSET(gpmc_ad8),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad9),      (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad10),     (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad11),     (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad12),     (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad13),     (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad14),     (MODE(0) | PULLUDDIS | RXACTIVE)},
	{OFFSET(gpmc_ad15),     (MODE(0) | PULLUDDIS | RXACTIVE)},
#endif
	{OFFSET(gpmc_wait0),    (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(gpmc_wpn),      (MODE(7) | PULLUP_EN)},
	{OFFSET(gpmc_csn0),     (MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_wen),      (MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_oen_ren),  (MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLDOWN_EN)},
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLDOWN_EN)},
	{-1},
};
#endif

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
#ifdef CONFIG_NAND
	configure_module_pin_mux(nand_pin_mux);
#endif
	configure_module_pin_mux(adc_voltages_en);
	configure_module_pin_mux(asp_power_en);
	configure_module_pin_mux(switch_off_3v6_pin_mux);
}
