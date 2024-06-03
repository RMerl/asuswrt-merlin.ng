/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2HK EVM : Board common header
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _KS2_BOARD
#define _KS2_BOARD

#include <asm/ti-common/keystone_net.h>
#include "../common/board_detect.h"

#if defined(CONFIG_TI_I2C_BOARD_DETECT)
static inline int board_is_k2g_gp(void)
{
	return board_ti_is("66AK2GGP");
}
static inline int board_is_k2g_g1(void)
{
	return board_ti_is("66AK2GG1");
}
static inline int board_is_k2g_ice(void)
{
	return board_ti_is("66AK2GIC");
}
#else
static inline int board_is_k2g_gp(void)
{
	return false;
}
static inline int board_is_k2g_ice(void)
{
	return false;
}
#endif

void spl_init_keystone_plls(void);

#endif
