/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Edge Port Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __EPORT_H__
#define __EPORT_H__

/* Edge Port Module (EPORT) */
typedef struct eport {
	u16 par;	/* 0x00 Pin Assignment */
	u8 ddr;		/* 0x02 Data Direction */
	u8 ier;		/* 0x03 Interrupt Enable */
	u8 dr;		/* 0x04 Data */
	u8 pdr;		/* 0x05 Pin Data */
	u8 fr;		/* 0x06 Flag */
	u8 res0;
} eport_t;

/* EPPAR */
#define EPORT_PAR_EPPA1(x)		(((x)&0x0003)<<2)
#define EPORT_PAR_EPPA2(x)		(((x)&0x0003)<<4)
#define EPORT_PAR_EPPA3(x)		(((x)&0x0003)<<6)
#define EPORT_PAR_EPPA4(x)		(((x)&0x0003)<<8)
#define EPORT_PAR_EPPA5(x)		(((x)&0x0003)<<10)
#define EPORT_PAR_EPPA6(x)		(((x)&0x0003)<<12)
#define EPORT_PAR_EPPA7(x)		(((x)&0x0003)<<14)
#define EPORT_PAR_LEVEL			(0)
#define EPORT_PAR_RISING		(1)
#define EPORT_PAR_FALLING		(2)
#define EPORT_PAR_BOTH			(3)
#define EPORT_PAR_EPPA7_LEVEL		(0x0000)
#define EPORT_PAR_EPPA7_RISING		(0x4000)
#define EPORT_PAR_EPPA7_FALLING		(0x8000)
#define EPORT_PAR_EPPA7_BOTH		(0xC000)
#define EPORT_PAR_EPPA6_LEVEL		(0x0000)
#define EPORT_PAR_EPPA6_RISING		(0x1000)
#define EPORT_PAR_EPPA6_FALLING		(0x2000)
#define EPORT_PAR_EPPA6_BOTH		(0x3000)
#define EPORT_PAR_EPPA5_LEVEL		(0x0000)
#define EPORT_PAR_EPPA5_RISING		(0x0400)
#define EPORT_PAR_EPPA5_FALLING		(0x0800)
#define EPORT_PAR_EPPA5_BOTH		(0x0C00)
#define EPORT_PAR_EPPA4_LEVEL		(0x0000)
#define EPORT_PAR_EPPA4_RISING		(0x0100)
#define EPORT_PAR_EPPA4_FALLING		(0x0200)
#define EPORT_PAR_EPPA4_BOTH		(0x0300)
#define EPORT_PAR_EPPA3_LEVEL		(0x0000)
#define EPORT_PAR_EPPA3_RISING		(0x0040)
#define EPORT_PAR_EPPA3_FALLING		(0x0080)
#define EPORT_PAR_EPPA3_BOTH		(0x00C0)
#define EPORT_PAR_EPPA2_LEVEL		(0x0000)
#define EPORT_PAR_EPPA2_RISING		(0x0010)
#define EPORT_PAR_EPPA2_FALLING		(0x0020)
#define EPORT_PAR_EPPA2_BOTH		(0x0030)
#define EPORT_PAR_EPPA1_LEVEL		(0x0000)
#define EPORT_PAR_EPPA1_RISING		(0x0004)
#define EPORT_PAR_EPPA1_FALLING		(0x0008)
#define EPORT_PAR_EPPA1_BOTH		(0x000C)

/* EPDDR */
#define EPORT_DDR_EPDD1			(0x02)
#define EPORT_DDR_EPDD2			(0x04)
#define EPORT_DDR_EPDD3			(0x08)
#define EPORT_DDR_EPDD4			(0x10)
#define EPORT_DDR_EPDD5			(0x20)
#define EPORT_DDR_EPDD6			(0x40)
#define EPORT_DDR_EPDD7			(0x80)

/* EPIER */
#define EPORT_IER_EPIE1			(0x02)
#define EPORT_IER_EPIE2			(0x04)
#define EPORT_IER_EPIE3			(0x08)
#define EPORT_IER_EPIE4			(0x10)
#define EPORT_IER_EPIE5			(0x20)
#define EPORT_IER_EPIE6			(0x40)
#define EPORT_IER_EPIE7			(0x80)

/* EPDR */
#define EPORT_DR_EPD1			(0x02)
#define EPORT_DR_EPD2			(0x04)
#define EPORT_DR_EPD3			(0x08)
#define EPORT_DR_EPD4			(0x10)
#define EPORT_DR_EPD5			(0x20)
#define EPORT_DR_EPD6			(0x40)
#define EPORT_DR_EPD7			(0x80)

/* EPPDR */
#define EPORT_PDR_EPPD1			(0x02)
#define EPORT_PDR_EPPD2			(0x04)
#define EPORT_PDR_EPPD3			(0x08)
#define EPORT_PDR_EPPD4			(0x10)
#define EPORT_PDR_EPPD5			(0x20)
#define EPORT_PDR_EPPD6			(0x40)
#define EPORT_PDR_EPPD7			(0x80)

/* EPFR */
#define EPORT_FR_EPF1			(0x02)
#define EPORT_FR_EPF2			(0x04)
#define EPORT_FR_EPF3			(0x08)
#define EPORT_FR_EPF4			(0x10)
#define EPORT_FR_EPF5			(0x20)
#define EPORT_FR_EPF6			(0x40)
#define EPORT_FR_EPF7			(0x80)

#endif				/* __EPORT_H__ */
