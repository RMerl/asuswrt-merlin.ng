/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * RealTime Clock
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __MCFRTC_H__
#define __MCFRTC_H__

/* Real time Clock */
typedef struct rtc_ctrl {
	u32 hourmin;		/* 0x00 Hours and Minutes Counter Register */
	u32 seconds;		/* 0x04 Seconds Counter Register */
	u32 alrm_hm;		/* 0x08 Hours and Minutes Alarm Register */
	u32 alrm_sec;		/* 0x0C Seconds Alarm Register */
	u32 cr;			/* 0x10 Control Register */
	u32 isr;		/* 0x14 Interrupt Status Register */
	u32 ier;		/* 0x18 Interrupt Enable Register */
	u32 stpwatch;		/* 0x1C Stopwatch Minutes Register */
	u32 days;		/* 0x20 Days Counter Register */
	u32 alrm_day;		/* 0x24 Days Alarm Register */
	void *extended;
} rtc_t;

/* Bit definitions and macros for HOURMIN */
#define RTC_HOURMIN_MINUTES(x)	(((x)&0x0000003F))
#define RTC_HOURMIN_HOURS(x)	(((x)&0x0000001F)<<8)

/* Bit definitions and macros for SECONDS */
#define RTC_SECONDS_SECONDS(x)	(((x)&0x0000003F))

/* Bit definitions and macros for ALRM_HM */
#define RTC_ALRM_HM_MINUTES(x)	(((x)&0x0000003F))
#define RTC_ALRM_HM_HOURS(x)	(((x)&0x0000001F)<<8)

/* Bit definitions and macros for ALRM_SEC */
#define RTC_ALRM_SEC_SECONDS(x)	(((x)&0x0000003F))

/* Bit definitions and macros for CR */
#define RTC_CR_SWR		(0x00000001)
#define RTC_CR_XTL(x)		(((x)&0x00000003)<<5)
#define RTC_CR_EN		(0x00000080)
#define RTC_CR_32768		(0x0)
#define RTC_CR_32000		(0x1)
#define RTC_CR_38400		(0x2)

/* Bit definitions and macros for ISR */
#define RTC_ISR_SW		(0x00000001)
#define RTC_ISR_MIN		(0x00000002)
#define RTC_ISR_ALM		(0x00000004)
#define RTC_ISR_DAY		(0x00000008)
#define RTC_ISR_1HZ		(0x00000010)
#define RTC_ISR_HR		(0x00000020)
#define RTC_ISR_2HZ		(0x00000080)
#define RTC_ISR_SAM0		(0x00000100)
#define RTC_ISR_SAM1		(0x00000200)
#define RTC_ISR_SAM2		(0x00000400)
#define RTC_ISR_SAM3		(0x00000800)
#define RTC_ISR_SAM4		(0x00001000)
#define RTC_ISR_SAM5		(0x00002000)
#define RTC_ISR_SAM6		(0x00004000)
#define RTC_ISR_SAM7		(0x00008000)

/* Bit definitions and macros for IER */
#define RTC_IER_SW		(0x00000001)
#define RTC_IER_MIN		(0x00000002)
#define RTC_IER_ALM		(0x00000004)
#define RTC_IER_DAY		(0x00000008)
#define RTC_IER_1HZ		(0x00000010)
#define RTC_IER_HR		(0x00000020)
#define RTC_IER_2HZ		(0x00000080)
#define RTC_IER_SAM0		(0x00000100)
#define RTC_IER_SAM1		(0x00000200)
#define RTC_IER_SAM2		(0x00000400)
#define RTC_IER_SAM3		(0x00000800)
#define RTC_IER_SAM4		(0x00001000)
#define RTC_IER_SAM5		(0x00002000)
#define RTC_IER_SAM6		(0x00004000)
#define RTC_IER_SAM7		(0x00008000)

/* Bit definitions and macros for STPWCH */
#define RTC_STPWCH_CNT(x)	(((x)&0x0000003F))

/* Bit definitions and macros for DAYS */
#define RTC_DAYS_DAYS(x)	(((x)&0x0000FFFF))

/* Bit definitions and macros for ALRM_DAY */
#define RTC_ALRM_DAY_DAYS(x)	(((x)&0x0000FFFF))

#endif				/* __MCFRTC_H__ */
