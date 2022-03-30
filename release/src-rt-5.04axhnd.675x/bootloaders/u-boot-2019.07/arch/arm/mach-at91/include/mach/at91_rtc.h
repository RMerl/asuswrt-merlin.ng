/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) SAN People
 *
 * Real Time Clock (RTC) - System peripheral registers.
 * Based on AT91RM9200 datasheet revision E.
 */

#ifndef AT91_RTC_H
#define AT91_RTC_H

/* Control Register */
#define AT91_RTC_CR		(ATMEL_BASE_RTC + 0x00)
#define AT91_RTC_UPDTIM		(1 <<  0)	/* Update Request Time */
#define AT91_RTC_UPDCAL		(1 <<  1)	/* Update Request Calendar */
#define AT91_RTC_TIMEVSEL	(3 <<  8)	/* Time Event Selection */
#define AT91_RTC_TIMEVSEL_MINUTE (0 << 8)
#define AT91_RTC_TIMEVSEL_HOUR	(1 << 8)
#define AT91_RTC_TIMEVSEL_DAY24	(2 << 8)
#define AT91_RTC_TIMEVSEL_DAY12	(3 << 8)
#define AT91_RTC_CALEVSEL	(3 << 16)	/* Calendar Event Selection */
#define AT91_RTC_CALEVSEL_WEEK	(0 << 16)
#define AT91_RTC_CALEVSEL_MONTH	(1 << 16)
#define AT91_RTC_CALEVSEL_YEAR	(2 << 16)

#define AT91_RTC_MR		(ATMEL_BASE_RTC + 0x04)	/* Mode Register */
#define AT91_RTC_HRMOD		(1 <<  0)		/* 12/24 Hour Mode */

#define AT91_RTC_TIMR		(ATMEL_BASE_RTC + 0x08)	/* Time Register */
#define AT91_RTC_SEC		(0x7f <<  0)		/* Current Second */
#define AT91_RTC_MIN		(0x7f <<  8)		/* Current Minute */
#define AT91_RTC_HOUR		(0x3f << 16)		/* Current Hour */
#define AT91_RTC_AMPM		(1    << 22)		/* AM/PM */

#define AT91_RTC_CALR		(ATMEL_BASE_RTC + 0x0c)	/* Calendar Register */
#define AT91_RTC_CENT		(0x7f <<  0)		/* Current Century */
#define AT91_RTC_YEAR		(0xff <<  8)		/* Current Year */
#define AT91_RTC_MONTH		(0x1f << 16)		/* Current Month */
#define AT91_RTC_DAY		(7    << 21)		/* Current Day */
#define AT91_RTC_DATE		(0x3f << 24)		/* Current Date */

#define AT91_RTC_TIMALR		(ATMEL_BASE_RTC + 0x10)	/* Time Alarm */
#define AT91_RTC_SECEN		(1 <<  7)		/* Second Alarm Enab */
#define AT91_RTC_MINEN		(1 << 15)		/* Minute Alarm Enab */
#define AT91_RTC_HOUREN		(1 << 23)		/* Hour Alarm Enable */

#define AT91_RTC_CALALR		(ATMEL_BASE_RTC + 0x14)	/* Calendar Alarm */
#define AT91_RTC_MTHEN		(1 << 23)		/* Month Alarm Enable */
#define AT91_RTC_DATEEN		(1 << 31)		/* Date Alarm Enable */

#define AT91_RTC_SR		(ATMEL_BASE_RTC + 0x18)	/* Status Register */
#define AT91_RTC_ACKUPD		(1 <<  0)		/* Ack for Update */
#define AT91_RTC_ALARM		(1 <<  1)		/* Alarm Flag */
#define AT91_RTC_SECEV		(1 <<  2)		/* Second Event */
#define AT91_RTC_TIMEV		(1 <<  3)		/* Time Event */
#define AT91_RTC_CALEV		(1 <<  4)		/* Calendar Event */

#define AT91_RTC_SCCR		(ATMEL_BASE_RTC + 0x1c)	/* Status Clear Cmd */
#define AT91_RTC_IER		(ATMEL_BASE_RTC + 0x20)	/* Interrupt Enable */
#define AT91_RTC_IDR		(ATMEL_BASE_RTC + 0x24)	/* Interrupt Disable */
#define AT91_RTC_IMR		(ATMEL_BASE_RTC + 0x28)	/* Interrupt Mask */

#define AT91_RTC_VER		(ATMEL_BASE_RTC + 0x2c)	/* Valid Entry */
#define AT91_RTC_NVTIM		(1 <<  0)		/* Non-valid Time */
#define AT91_RTC_NVCAL		(1 <<  1)		/* Non-valid Calendar */
#define AT91_RTC_NVTIMALR	(1 <<  2)		/* .. Time Alarm */
#define AT91_RTC_NVCALALR	(1 <<  3)		/* .. Calendar Alarm */

#endif
