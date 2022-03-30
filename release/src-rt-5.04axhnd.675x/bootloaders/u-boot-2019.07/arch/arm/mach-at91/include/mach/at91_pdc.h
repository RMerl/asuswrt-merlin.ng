/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 */

#ifndef AT91_PDC_H
#define AT91_PDC_H

typedef struct at91_pdc {
	u32	rpr;		/* 0x100 Receive Pointer Register */
	u32	rcr;		/* 0x104 Receive Counter Register */
	u32	tpr;		/* 0x108 Transmit Pointer Register */
	u32	tcr;		/* 0x10C Transmit Counter Register */
	u32	pnpr;		/* 0x110 Receive Next Pointer Register */
	u32	pncr;		/* 0x114 Receive Next Counter Register */
	u32	tnpr;		/* 0x118 Transmit Next Pointer Register */
	u32	tncr;		/* 0x11C Transmit Next Counter Register */
	u32	ptcr;		/* 0x120 Transfer Control Register */
	u32	ptsr;		/* 0x124 Transfer Status Register */
} at91_pdc_t;

#endif
