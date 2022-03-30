/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "evm.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(pincntl70), PULLUP_EN | MODE(0x01)},	/* UART0_RXD */
	{OFFSET(pincntl71), PULLUP_EN | MODE(0x01)},	/* UART0_TXD */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(pincntl1), PULLUP_EN | MODE(0x01)},	/* SD1_CLK */
	{OFFSET(pincntl2), PULLUP_EN | MODE(0x01)},	/* SD1_CMD */
	{OFFSET(pincntl3), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[0] */
	{OFFSET(pincntl4), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[1] */
	{OFFSET(pincntl5), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[2] */
	{OFFSET(pincntl6), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[3] */
	{OFFSET(pincntl74), PULLUP_EN | MODE(0x40)},	/* SD1_POW */
	{OFFSET(pincntl75), MODE(0x40)},		/* SD1_SDWP */
	{OFFSET(pincntl80), PULLUP_EN | MODE(0x02)},	/* SD1_SDCD */
	{-1},
};

static struct module_pin_mux enet_pin_mux[] = {
	{OFFSET(pincntl232), MODE(0x01)},		/* EMAC_RMREFCLK */
	{OFFSET(pincntl233), PULLUP_EN | MODE(0x01)},	/* MDCLK */
	{OFFSET(pincntl234), PULLUP_EN | MODE(0x01)},	/* MDIO */
	{OFFSET(pincntl235), MODE(0x01)},		/* EMAC[0]_MTCLK */
	{OFFSET(pincntl236), MODE(0x01)},		/* EMAC[0]_MCOL */
	{OFFSET(pincntl237), MODE(0x01)},		/* EMAC[0]_MCRS */
	{OFFSET(pincntl238), MODE(0x01)},		/* EMAC[0]_MRXER */
	{OFFSET(pincntl239), MODE(0x01)},		/* EMAC[0]_MRCLK */
	{OFFSET(pincntl240), MODE(0x01)},		/* EMAC[0]_MRXD[0] */
	{OFFSET(pincntl241), MODE(0x01)},		/* EMAC[0]_MRXD[1] */
	{OFFSET(pincntl242), MODE(0x01)},		/* EMAC[0]_MRXD[2] */
	{OFFSET(pincntl243), MODE(0x01)},		/* EMAC[0]_MRXD[3] */
	{OFFSET(pincntl244), MODE(0x01)},		/* EMAC[0]_MRXD[4] */
	{OFFSET(pincntl245), MODE(0x01)},		/* EMAC[0]_MRXD[5] */
	{OFFSET(pincntl246), MODE(0x01)},		/* EMAC[0]_MRXD[6] */
	{OFFSET(pincntl247), MODE(0x01)},		/* EMAC[0]_MRXD[7] */
	{OFFSET(pincntl248), MODE(0x01)},		/* EMAC[0]_MRXDV */
	{OFFSET(pincntl249), MODE(0x01)},		/* EMAC[0]_GMTCLK */
	{OFFSET(pincntl250), MODE(0x01)},		/* EMAC[0]_MTXD[0] */
	{OFFSET(pincntl251), MODE(0x01)},		/* EMAC[0]_MTXD[1] */
	{OFFSET(pincntl252), MODE(0x01)},		/* EMAC[0]_MTXD[2] */
	{OFFSET(pincntl253), MODE(0x01)},		/* EMAC[0]_MTXD[3] */
	{OFFSET(pincntl254), MODE(0x01)},		/* EMAC[0]_MTXD[4] */
	{OFFSET(pincntl255), MODE(0x01)},		/* EMAC[0]_MTXD[5] */
	{OFFSET(pincntl256), MODE(0x01)},		/* EMAC[0]_MTXD[6] */
	{OFFSET(pincntl257), MODE(0x01)},		/* EMAC[0]_MTXD[7] */
	{OFFSET(pincntl258), MODE(0x01)},		/* EMAC[0]_MTXEN */
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_mmc1_pin_mux(void)
{
	configure_module_pin_mux(mmc1_pin_mux);
}

void enable_enet_pin_mux(void)
{
	configure_module_pin_mux(enet_pin_mux);
}
