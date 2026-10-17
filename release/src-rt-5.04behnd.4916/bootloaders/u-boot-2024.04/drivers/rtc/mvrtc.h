/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 */

/*
 * Date & Time support for Marvell Integrated RTC
 */

#ifndef _MVRTC_H_
#define _MVRTC_H_

#include <asm/arch/soc.h>
#include <linux/compiler.h>

/* RTC registers */
struct mvrtc_registers {
	u32 time;
	u32 date;
};

/* Platform data */
struct mvrtc_pdata {
	phys_addr_t iobase;
};

/* time register */
#define MVRTC_SEC_SFT		0
#define MVRTC_SEC_MSK		0x7f
#define MVRTC_MIN_SFT		8
#define MVRTC_MIN_MSK		0x7f
#define MVRTC_HOUR_SFT		16
#define MVRTC_HOUR_MSK		0x3f
#define MVRTC_DAY_SFT		24
#define MVRTC_DAY_MSK		0x7

/*
 * Hour format bit
 *   1 = 12 hour clock
 *   0 = 24 hour clock
 */
#define MVRTC_HRFMT_MSK		0x00400000

/* date register */
#define MVRTC_DATE_SFT		0
#define MVRTC_DATE_MSK		0x3f
#define MVRTC_MON_SFT		8
#define MVRTC_MON_MSK		0x1f
#define MVRTC_YEAR_SFT		16
#define MVRTC_YEAR_MSK		0xff

#endif
