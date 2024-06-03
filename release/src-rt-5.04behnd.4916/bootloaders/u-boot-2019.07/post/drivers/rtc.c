// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * RTC test
 *
 * The Real Time Clock (RTC) operation is verified by this test.
 * The following features are verified:
 *   o) RTC Power Fault
 *	This is verified by analyzing the rtc_get() return status.
 *   o) Time uniformity
 *      This is verified by reading RTC in polling within
 *      a short period of time.
 *   o) Passing month boundaries
 *      This is checked by setting RTC to a second before
 *      a month boundary and reading it after its passing the
 *      boundary. The test is performed for both leap- and
 *      nonleap-years.
 */

#include <post.h>
#include <rtc.h>

#if CONFIG_POST & CONFIG_SYS_POST_RTC

static int rtc_post_skip (ulong * diff)
{
	struct rtc_time tm1;
	struct rtc_time tm2;
	ulong start1;
	ulong start2;

	rtc_get (&tm1);
	start1 = get_timer (0);

	while (1) {
		rtc_get (&tm2);
		start2 = get_timer (0);
		if (tm1.tm_sec != tm2.tm_sec)
			break;
		if (start2 - start1 > 1500)
			break;
	}

	if (tm1.tm_sec != tm2.tm_sec) {
		*diff = start2 - start1;

		return 0;
	} else {
		return -1;
	}
}

static void rtc_post_restore (struct rtc_time *tm, unsigned int sec)
{
	time_t t = rtc_mktime(tm) + sec;
	struct rtc_time ntm;

	rtc_to_tm(t, &ntm);

	rtc_set (&ntm);
}

int rtc_post_test (int flags)
{
	ulong diff;
	unsigned int i;
	struct rtc_time svtm;
	static unsigned int daysnl[] =
			{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static unsigned int daysl[] =
			{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned int ynl = 1999;
	unsigned int yl = 2000;
	unsigned int skipped = 0;
	int reliable;

	/* Time reliability */
	reliable = rtc_get (&svtm);

	/* Time uniformity */
	if (rtc_post_skip (&diff) != 0) {
		post_log ("Timeout while waiting for a new second !\n");

		return -1;
	}

	for (i = 0; i < 5; i++) {
		if (rtc_post_skip (&diff) != 0) {
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		if (diff < 950 || diff > 1050) {
			post_log ("Invalid second duration !\n");

			return -1;
		}
	}

	/* Passing month boundaries */

	if (rtc_post_skip (&diff) != 0) {
		post_log ("Timeout while waiting for a new second !\n");

		return -1;
	}
	rtc_get (&svtm);

	for (i = 0; i < 12; i++) {
		time_t t;
		struct rtc_time tm;

		tm.tm_year = ynl;
		tm.tm_mon = i + 1;
		tm.tm_mday = daysnl[i];
		tm.tm_hour = 23;
		tm.tm_min = 59;
		tm.tm_sec = 59;
		t = rtc_mktime(&tm);
		rtc_to_tm(t, &tm);
		rtc_set (&tm);

		skipped++;
		if (rtc_post_skip (&diff) != 0) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		rtc_get (&tm);
		if (tm.tm_mon == i + 1) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Month %d boundary is not passed !\n", i + 1);

			return -1;
		}
	}

	for (i = 0; i < 12; i++) {
		time_t t;
		struct rtc_time tm;

		tm.tm_year = yl;
		tm.tm_mon = i + 1;
		tm.tm_mday = daysl[i];
		tm.tm_hour = 23;
		tm.tm_min = 59;
		tm.tm_sec = 59;
		t = rtc_mktime(&tm);

		rtc_to_tm(t, &tm);
		rtc_set (&tm);

		skipped++;
		if (rtc_post_skip (&diff) != 0) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		rtc_get (&tm);
		if (tm.tm_mon == i + 1) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Month %d boundary is not passed !\n", i + 1);

			return -1;
		}
	}
	rtc_post_restore (&svtm, skipped);

	/* If come here, then RTC operates correcty, check the correctness
	 * of the time it reports.
	 */
	if (reliable < 0) {
		post_log ("RTC Time is not reliable! Power fault? \n");

		return -1;
	}

	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_RTC */
