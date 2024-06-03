/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Chen-Yu Tsai <wens@csie.org>
 */

#ifndef _SUNXI_TZPC_H
#define _SUNXI_TZPC_H

#ifndef __ASSEMBLY__
struct sunxi_tzpc {
	u32 r0size;		/* 0x00 Size of secure RAM region */
	u32 decport0_status;	/* 0x04 Status of decode protection port 0 */
	u32 decport0_set;	/* 0x08 Set decode protection port 0 */
	u32 decport0_clear;	/* 0x0c Clear decode protection port 0 */
	/* For A80 and later SoCs */
	u32 decport1_status;	/* 0x10 Status of decode protection port 1 */
	u32 decport1_set;	/* 0x14 Set decode protection port 1 */
	u32 decport1_clear;	/* 0x18 Clear decode protection port 1 */
	u32 decport2_status;	/* 0x1c Status of decode protection port 2 */
	u32 decport2_set;	/* 0x20 Set decode protection port 2 */
	u32 decport2_clear;	/* 0x24 Clear decode protection port 2 */
};
#endif

#define SUN6I_TZPC_DECPORT0_RTC	(1 << 1)

#define SUN8I_H3_TZPC_DECPORT0_ALL  0xbe
#define SUN8I_H3_TZPC_DECPORT1_ALL  0xff
#define SUN8I_H3_TZPC_DECPORT2_ALL  0x7f

void tzpc_init(void);

#endif /* _SUNXI_TZPC_H */
