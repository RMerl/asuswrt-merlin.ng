/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 */

#ifndef AT91_ST_H
#define AT91_ST_H

typedef struct at91_st {

	u32	cr;
	u32	pimr;
	u32	wdmr;
	u32	rtmr;
	u32	sr;
	u32	ier;
	u32	idr;
	u32	imr;
	u32	rtar;
	u32	crtr;
} at91_st_t ;

#define AT91_ST_CR_WDRST	1

#define AT91_ST_WDMR_WDV(x)	(x & 0xFFFF)
#define AT91_ST_WDMR_RSTEN	0x00010000
#define AT91_ST_WDMR_EXTEN 	0x00020000

#endif
