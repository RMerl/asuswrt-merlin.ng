/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2020 Gateworks Corporation */

#ifndef MP5416_H_
#define MP5416_H_

#define MP6416_REGULATOR_DRIVER "mp5416_regulator"

enum {
	MP5416_CTL0		= 0x00,
	MP5416_CTL1		= 0x01,
	MP5416_CTL2		= 0x02,
	MP5416_ILIMIT		= 0x03,
	MP5416_VSET_SW1		= 0x04,
	MP5416_VSET_SW2		= 0x05,
	MP5416_VSET_SW3		= 0x06,
	MP5416_VSET_SW4		= 0x07,
	MP5416_VSET_LDO2	= 0x08,
	MP5416_VSET_LDO3	= 0x09,
	MP5416_VSET_LDO4	= 0x0a,
	MP5416_VSET_LDO5	= 0x0b,
	MP5416_STATUS1		= 0x0d,
	MP5416_STATUS2		= 0x0e,
	MP5416_STATUS3		= 0x0f,
	MP5416_ID2		= 0x11,
	MP5416_NUM_OF_REGS	= 0x12,
};

#define MP5416_VSET_EN          BIT(7)
#define MP5416_VSET_SW1_GVAL(x) ((((x) & 0x7f) * 12500) + 600000)
#define MP5416_VSET_SW2_GVAL(x) ((((x) & 0x7f) * 25000) + 800000)
#define MP5416_VSET_SW3_GVAL(x) ((((x) & 0x7f) * 12500) + 600000)
#define MP5416_VSET_SW4_GVAL(x) ((((x) & 0x7f) * 25000) + 800000)
#define MP5416_VSET_LDO_GVAL(x) ((((x) & 0x7f) * 25000) + 800000)
#define MP5416_VSET_LDO_SVAL(x) ((((x) & 0x7f) * 25000) + 800000)
#define MP5416_VSET_SW1_SVAL(x) (((x) - 600000) / 12500)
#define MP5416_VSET_SW2_SVAL(x) (((x) - 800000) / 25000)
#define MP5416_VSET_SW3_SVAL(x) (((x) - 600000) / 12500)
#define MP5416_VSET_SW4_SVAL(x) (((x) - 800000) / 25000)

#endif
