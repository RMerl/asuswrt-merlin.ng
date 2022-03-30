/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * vme8349pin.h -- esd VME8349 MPC8349 I/O pin definition.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 */

#ifndef __VME8349PIN_H__
#define __VME8349PIN_H__

#define GPIO2_V_SCON		0x80000000 /* In:  from tsi148 1: is syscon */
#define GPIO2_VME_RESET_N	0x20000000 /* Out: to tsi148                */
#define GPIO2_TSI_PLL_RESET_N	0x08000000 /* Out: to tsi148                */
#define GPIO2_TSI_POWERUP_RESET_N 0x00800000 /* Out: to tsi148              */
#define GPIO2_L_RESET_EN_N	0x00100000 /* Out: 0:vme can assert cpu lrst*/

#endif /* of ifndef __VME8349PIN_H__ */
