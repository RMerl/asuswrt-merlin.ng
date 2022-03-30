/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009, Matthias Fuchs <matthias.fuchs@esd.eu>
 *
 * SJA1000 register layout for basic CAN mode
 */

#ifndef _SJA1000_H_
#define _SJA1000_H_

/*
 * SJA1000 register layout in basic can mode
 */
struct sja1000_basic_s {
	u8 cr;
	u8 cmr;
	u8 sr;
	u8 ir;
	u8 ac;
	u8 am;
	u8 btr0;
	u8 btr1;
	u8 oc;
	u8 txb[10];
	u8 rxb[10];
	u8 unused;
	u8 cdr;
};

/* control register */
#define CR_RR		0x01

/* output control register */
#define OC_MODE0	0x01
#define OC_MODE1	0x02
#define OC_POL0		0x04
#define OC_TN0		0x08
#define OC_TP0		0x10
#define OC_POL1		0x20
#define OC_TN1		0x40
#define OC_TP1		0x80

#endif
