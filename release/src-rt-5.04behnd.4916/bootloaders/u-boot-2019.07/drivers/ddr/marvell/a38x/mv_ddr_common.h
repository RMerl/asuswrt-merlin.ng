/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_COMMON_H
#define _MV_DDR_COMMON_H

extern const char mv_ddr_build_message[];
extern const char mv_ddr_version_string[];

#define _1K	0x00000400
#define _4K	0x00001000
#define _8K	0x00002000
#define _16K	0x00004000
#define _32K	0x00008000
#define _64K	0x00010000
#define _128K	0x00020000
#define _256K	0x00040000
#define _512K	0x00080000

#define _1M	0x00100000
#define _2M	0x00200000
#define _4M	0x00400000
#define _8M	0x00800000
#define _16M	0x01000000
#define _32M	0x02000000
#define _64M	0x04000000
#define _128M	0x08000000
#define _256M	0x10000000
#define _512M	0x20000000

#define _1G	0x40000000
#define _2G	0x80000000
#define _4G	0x100000000
#define _8G	0x200000000
#define _16G	0x400000000
#define _32G	0x800000000
#define _64G	0x1000000000
#define _128G	0x2000000000

#define MEGA			1000000
#define MV_DDR_MEGABYTE		(1024 * 1024)
#define MV_DDR_32_BITS_MASK	0xffffffff

#define GET_MAX_VALUE(x, y) \
	(((x) > (y)) ? (x) : (y))

void mv_ddr_ver_print(void);
unsigned int ceil_div(unsigned int x, unsigned int y);
unsigned int time_to_nclk(unsigned int t, unsigned int tclk);
int round_div(unsigned int dividend, unsigned int divisor, unsigned int *quotient);

#endif /* _MV_DDR_COMMON_H */
