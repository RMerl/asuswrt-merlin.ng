/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Samsung Electronics
 */

#ifndef __TZPC_H_
#define __TZPC_H_

#ifndef __ASSEMBLY__
struct exynos_tzpc {
	unsigned int r0size;
	char res1[0x7FC];
	unsigned int decprot0stat;
	unsigned int decprot0set;
	unsigned int decprot0clr;
	unsigned int decprot1stat;
	unsigned int decprot1set;
	unsigned int decprot1clr;
	unsigned int decprot2stat;
	unsigned int decprot2set;
	unsigned int decprot2clr;
	unsigned int decprot3stat;
	unsigned int decprot3set;
	unsigned int decprot3clr;
	char res2[0x7B0];
	unsigned int periphid0;
	unsigned int periphid1;
	unsigned int periphid2;
	unsigned int periphid3;
	unsigned int pcellid0;
	unsigned int pcellid1;
	unsigned int pcellid2;
	unsigned int pcellid3;
};

#define EXYNOS4_NR_TZPC_BANKS		6
#define EXYNOS5_NR_TZPC_BANKS		10

/* TZPC : Register Offsets */
#define TZPC_BASE_OFFSET		0x10000

/*
 * TZPC Register Value :
 * R0SIZE: 0x0 : Size of secured ram
 */
#define R0SIZE			0x0

/*
 * TZPC Decode Protection Register Value :
 * DECPROTXSET: 0xFF : Set Decode region to non-secure
 */
#define DECPROTXSET		0xFF
void tzpc_init(void);

#endif

#endif
