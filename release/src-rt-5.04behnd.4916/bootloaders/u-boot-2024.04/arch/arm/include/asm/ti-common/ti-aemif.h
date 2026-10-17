/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * AEMIF definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _AEMIF_H_
#define _AEMIF_H_

#define AEMIF_NUM_CS               4
#define AEMIF_MODE_NOR             0
#define AEMIF_MODE_NAND            1
#define AEMIF_MODE_ONENAND         2
#define AEMIF_PRESERVE             -1

struct aemif_config {
	unsigned mode;
	unsigned select_strobe;
	unsigned extend_wait;
	unsigned wr_setup;
	unsigned wr_strobe;
	unsigned wr_hold;
	unsigned rd_setup;
	unsigned rd_strobe;
	unsigned rd_hold;
	unsigned turn_around;
	enum {
		AEMIF_WIDTH_8	= 0,
		AEMIF_WIDTH_16	= 1,
		AEMIF_WIDTH_32	= 2,
	} width;
};

void aemif_init(int num_cs, struct aemif_config *config);

#endif
