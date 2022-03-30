/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ifndef _SPR_SSP_H
#define _SPR_SSP_H

struct ssp_regs {
	u32 sspcr0;
	u32 sspcr1;
	u32 sspdr;
	u32 sspsr;
	u32 sspcpsr;
	u32 sspimsc;
	u32 sspicr;
	u32 sspdmacr;
};

#define SSPCR0_FRF_MOT_SPI	0x0000
#define SSPCR0_DSS_16BITS	0x000f

#define SSPCR1_SSE		0x0002

#define SSPSR_TNF		0x2
#define SSPSR_TFE		0x1

#endif
