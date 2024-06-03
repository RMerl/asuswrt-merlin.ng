/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * -------------------------------------------------------------------------
 *
 *  linux/include/asm-arm/arch-davinci/hardware.h
 *
 *  Copyright (C) 2006 Texas Instruments.
 */
#ifndef __ASM_DAVINCI_RTC_H
#define __ASM_DAVINCI_RTC_H

struct davinci_rtc {
	unsigned int	second;
	unsigned int	minutes;
	unsigned int	hours;
	unsigned int	day;
	unsigned int	month; /* 0x10 */
	unsigned int	year;
	unsigned int	dotw;
	unsigned int	resv1;
	unsigned int	alarmsecond; /* 0x20 */
	unsigned int	alarmminute;
	unsigned int	alarmhour;
	unsigned int	alarmday;
	unsigned int	alarmmonth; /* 0x30 */
	unsigned int	alarmyear;
	unsigned int	resv2[2];
	unsigned int	ctrl; /* 0x40 */
	unsigned int	status;
	unsigned int	irq;
	unsigned int	complsb;
	unsigned int	compmsb; /* 0x50 */
	unsigned int	osc;
	unsigned int	resv3[2];
	unsigned int	scratch0; /* 0x60 */
	unsigned int	scratch1;
	unsigned int	scratch2;
	unsigned int	kick0r;
	unsigned int	kick1r; /* 0x70 */
};

#define RTC_STATE_BUSY	0x01
#define RTC_STATE_RUN	0x02

#define RTC_KICK0R_WE	0x83e70b13
#define RTC_KICK1R_WE	0x95a4f1e0
#endif
