/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#ifndef PBLIMAGE_H
#define PBLIMAGE_H

#define RCW_BYTES	64
#define RCW_PREAMBLE	0xaa55aa55
#define RCW_HEADER	0x010e0100

struct pbl_header {
	uint32_t preamble;
	uint32_t rcwheader;
	uint8_t rcw_data[RCW_BYTES];
};

#endif /* PBLIMAGE_H */
