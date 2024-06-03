// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE) || defined(CONFIG_DM_RTC) || \
				defined(CONFIG_TIMESTAMP)

#define FEBRUARY		2
#define	STARTOFTIME		1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define	leapyear(year)		((year) % 4 == 0)
#define	days_in_year(a)		(leapyear(a) ? 366 : 365)
#define	days_in_month(a)	(month_days[(a) - 1])

static int month_offset[] = {
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/*
 * This only works for the Gregorian calendar - i.e. after 1752 (in the UK)
 */
int rtc_calc_weekday(struct rtc_time *tm)
{
	int leaps_to_date;
	int last_year;
	int day;

	if (tm->tm_year < 1753)
		return -1;
	last_year = tm->tm_year - 1;

	/* Number of leap corrections to apply up to end of last year */
	leaps_to_date = last_year / 4 - last_year / 100 + last_year / 400;

	/*
	 * This year is a leap year if it is divisible by 4 except when it is
	 * divisible by 100 unless it is divisible by 400
	 *
	 * e.g. 1904 was a leap year, 1900 was not, 1996 is, and 2000 is.
	 */
	if (tm->tm_year % 4 == 0 &&
	    ((tm->tm_year % 100 != 0) || (tm->tm_year % 400 == 0)) &&
	    tm->tm_mon > 2) {
		/* We are past Feb. 29 in a leap year */
		day = 1;
	} else {
		day = 0;
	}

	day += last_year * 365 + leaps_to_date + month_offset[tm->tm_mon - 1] +
			tm->tm_mday;
	tm->tm_wday = day % 7;

	return 0;
}

/*
 * Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year / 100 + year / 400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
unsigned long rtc_mktime(const struct rtc_time *tm)
{
	int mon = tm->tm_mon;
	int year = tm->tm_year;
	int days, hours;

	mon -= 2;
	if (0 >= (int)mon) {	/* 1..12 -> 11, 12, 1..10 */
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	days = (unsigned long)(year / 4 - year / 100 + year / 400 +
			367 * mon / 12 + tm->tm_mday) +
			year * 365 - 719499;
	hours = days * 24 + tm->tm_hour;
	return (hours * 60 + tm->tm_min) * 60 + tm->tm_sec;
}

#endif
