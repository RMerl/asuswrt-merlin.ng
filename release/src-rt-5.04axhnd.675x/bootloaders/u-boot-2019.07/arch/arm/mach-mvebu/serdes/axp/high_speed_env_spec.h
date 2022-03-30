/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __HIGHSPEED_ENV_SPEC_H
#define __HIGHSPEED_ENV_SPEC_H

#include "../../../drivers/ddr/marvell/axp/ddr3_hw_training.h"

typedef enum {
	SERDES_UNIT_UNCONNECTED	= 0x0,
	SERDES_UNIT_PEX		= 0x1,
	SERDES_UNIT_SATA	= 0x2,
	SERDES_UNIT_SGMII0	= 0x3,
	SERDES_UNIT_SGMII1	= 0x4,
	SERDES_UNIT_SGMII2	= 0x5,
	SERDES_UNIT_SGMII3	= 0x6,
	SERDES_UNIT_QSGMII	= 0x7,
	SERDES_UNIT_SETM        = 0x8,
	SERDES_LAST_UNIT
} MV_BIN_SERDES_UNIT_INDX;


typedef enum {
	PEX_BUS_DISABLED	= 0,
	PEX_BUS_MODE_X1		= 1,
	PEX_BUS_MODE_X4		= 2,
	PEX_BUS_MODE_X8		= 3
} MV_PEX_UNIT_CFG;

typedef enum pex_type {
	MV_PEX_ROOT_COMPLEX,	/* root complex device */
	MV_PEX_END_POINT	/* end point device */
} MV_PEX_TYPE;

typedef struct serdes_change_m_phy {
	MV_BIN_SERDES_UNIT_INDX type;
	u32 reg_low_speed;
	u32 val_low_speed;
	u32 reg_hi_speed;
	u32 val_hi_speed;
} MV_SERDES_CHANGE_M_PHY;

/*
 * Configuration per SERDES line. Each nibble is MV_SERDES_LINE_TYPE
 */
typedef struct board_serdes_conf {
	MV_PEX_TYPE pex_type; /* MV_PEX_ROOT_COMPLEX MV_PEX_END_POINT */
	u32 line0_7; /* Lines 0 to 7 SERDES MUX one nibble per line */
	u32 line8_15; /* Lines 8 to 15 SERDES MUX one nibble per line */
	MV_PEX_UNIT_CFG pex_mode[4];

	/*
	 * Bus speed - one bit per SERDES line:
	 *		Low speed (0)		High speed (1)
	 * PEX		2.5 G (10 bit)		5 G (20 bit)
	 * SATA		1.5 G			3 G
	 * SGMII	1.25 Gbps		3.125 Gbps
	 */
	u32	bus_speed;

	MV_SERDES_CHANGE_M_PHY *serdes_m_phy_change;
} MV_BIN_SERDES_CFG;


#define BIN_SERDES_CFG {	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 0 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1,  2}, /* Lane 1 */	\
	{0, 1, -1 ,  2, -1, -1, -1, -1,  3}, /* Lane 2 */	\
	{0, 1, -1 , -1,  2, -1, -1,  3, -1}, /* Lane 3 */	\
	{0, 1,  2 , -1, -1,  3, -1, -1,  4}, /* Lane 4 */	\
	{0, 1,  2 , -1,  3, -1, -1,  4, -1}, /* Lane 5 */	\
	{0, 1,  2 ,  4, -1,  3, -1, -1, -1}, /* Lane 6 */	\
	{0, 1, -1 ,  2, -1, -1,  3, -1,  4}, /* Lane 7*/	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 8 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 9 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 10 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 11 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 12 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 13 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 14 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1, -1}, /* Lane 15 */	\
}

#endif /* __HIGHSPEED_ENV_SPEC_H */
