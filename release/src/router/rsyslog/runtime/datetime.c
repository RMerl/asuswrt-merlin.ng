/* The datetime object. It contains date and time related functions.
 *
 * Module begun 2008-03-05 by Rainer Gerhards, based on some code
 * from syslogd.c. The main intension was to move code out of syslogd.c
 * in a useful manner. It is still undecided if all functions will continue
 * to stay here or some will be moved into parser modules (once we have them).
 *
 * Copyright 2008-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#	include <sys/time.h>
#endif

#include "rsyslog.h"
#include "obj.h"
#include "modules.h"
#include "datetime.h"
#include "srUtils.h"
#include "stringbuf.h"
#include "errmsg.h"

/* static data */
DEFobjStaticHelpers

/* the following table of ten powers saves us some computation */
static const int tenPowers[6] = { 1, 10, 100, 1000, 10000, 100000 };

/* the following table saves us from computing an additional date to get
 * the ordinal day of the year - at least from 1967-2099
 * Note: non-2038+ compliant systems (Solaris) will generate compiler
 * warnings on the post 2038-rollover years.
 */
static const int yearInSec_startYear = 1967;
/* for x in $(seq 1967 2099) ; do
 *   printf %s', ' $(date --date="Dec 31 ${x} UTC 23:59:59" +%s)
 * done |fold -w 70 -s */
static const long long yearInSecs[] = {
	-63158401, -31536001, -1, 31535999, 63071999, 94694399, 126230399,
	157766399, 189302399, 220924799, 252460799, 283996799, 315532799,
	347155199, 378691199, 410227199, 441763199, 473385599, 504921599,
	536457599, 567993599, 599615999, 631151999, 662687999, 694223999,
	725846399, 757382399, 788918399, 820454399, 852076799, 883612799,
	915148799, 946684799, 978307199, 1009843199, 1041379199, 1072915199,
	1104537599, 1136073599, 1167609599, 1199145599, 1230767999,
	1262303999, 1293839999, 1325375999, 1356998399, 1388534399,
	1420070399, 1451606399, 1483228799, 1514764799, 1546300799,
	1577836799, 1609459199, 1640995199, 1672531199, 1704067199,
	1735689599, 1767225599, 1798761599, 1830297599, 1861919999,
	1893455999, 1924991999, 1956527999, 1988150399, 2019686399,
	2051222399, 2082758399, 2114380799, 2145916799, 2177452799,
	2208988799, 2240611199, 2272147199, 2303683199, 2335219199,
	2366841599, 2398377599, 2429913599, 2461449599, 2493071999,
	2524607999, 2556143999, 2587679999, 2619302399, 2650838399,
	2682374399, 2713910399, 2745532799, 2777068799, 2808604799,
	2840140799, 2871763199, 2903299199, 2934835199, 2966371199,
	2997993599, 3029529599, 3061065599, 3092601599, 3124223999,
	3155759999, 3187295999, 3218831999, 3250454399, 3281990399,
	3313526399, 3345062399, 3376684799, 3408220799, 3439756799,
	3471292799, 3502915199, 3534451199, 3565987199, 3597523199,
	3629145599, 3660681599, 3692217599, 3723753599, 3755375999,
	3786911999, 3818447999, 3849983999, 3881606399, 3913142399,
	3944678399, 3976214399, 4007836799, 4039372799, 4070908799,
	4102444799};

static const char* monthNames[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* ------------------------------ methods ------------------------------ */


/**
 * Convert struct timeval to syslog_time
 */
static void
timeval2syslogTime(struct timeval *tp, struct syslogTime *t, const int inUTC)
{
	struct tm *tm;
	struct tm tmBuf;
	long lBias;
	time_t secs;
/* AIXPORT : fix build error : "tm_gmtoff" is not a member of "struct tm"
 *           Choose the HPUX code path, only for this function.
 *           This is achieved by adding a check to _AIX wherever _hpux is checked
 */


#if defined(__hpux) || defined(_AIX)
	struct timezone tz;
#	endif
	secs = tp->tv_sec;
	if(inUTC)
		tm = gmtime_r(&secs, &tmBuf);
	else
		tm = localtime_r(&secs, &tmBuf);

	t->year = tm->tm_year + 1900;
	t->month = tm->tm_mon + 1;
	t->day = tm->tm_mday;
	t->hour = tm->tm_hour;
	t->minute = tm->tm_min;
	t->second = tm->tm_sec;
	t->secfrac = tp->tv_usec;
	t->secfracPrecision = 6;

	if(inUTC) {
		t->OffsetMode = '+';
		lBias = 0;
	} else {
#		if defined(__sun)
			/* Solaris uses a different method of exporting the time zone.
			 * It is UTC - localtime, which is the opposite sign of mins east of GMT.
			 */
			lBias = -(tm->tm_isdst ? altzone : timezone);
#		elif defined(__hpux)|| defined(_AIX)
			lBias = tz.tz_dsttime ? - tz.tz_minuteswest : 0;
#		else
			lBias = tm->tm_gmtoff;
#		endif
		if(lBias < 0) {
			t->OffsetMode = '-';
			lBias *= -1;
		} else
			t->OffsetMode = '+';
	}
	t->OffsetHour = lBias / 3600;
	t->OffsetMinute = (lBias % 3600) / 60;
	t->timeType = TIME_TYPE_RFC5424; /* we have a high precision timestamp */
	t->inUTC = inUTC;
}

/**
 * Get the current date/time in the best resolution the operating
 * system has to offer (well, actually at most down to the milli-
 * second level.
 *
 * The date and time is returned in separate fields as this is
 * most portable and removes the need for additional structures
 * (but I have to admit it is somewhat "bulky";)).
 *
 * Obviously, *t must not be NULL...
 *
 * rgerhards, 2008-10-07: added ttSeconds to provide a way to
 * obtain the second-resolution UNIX timestamp. This is needed
 * in some situations to minimize time() calls (namely when doing
 * output processing). This can be left NULL if not needed.
 */
static void getCurrTime(struct syslogTime *t, time_t *ttSeconds, const int inUTC)
{
	struct timeval tp;
/* AIXPORT : fix build error : "tm_gmtoff" is not a member of "struct tm"
 *           Choose the HPUX code path, only for this function.
 *           This is achieved by adding a check to _AIX wherever _hpux is checked
 */


#if defined(__hpux) || defined(_AIX)
	struct timezone tz;
#	endif

	assert(t != NULL);
#if defined(__hpux) || defined(_AIX)
		/* TODO: check this: under HP UX, the tz information is actually valid
		 * data. So we need to obtain and process it there.
		 */
		gettimeofday(&tp, &tz);
#	else
		gettimeofday(&tp, NULL);
#	endif
	if(ttSeconds != NULL)
		*ttSeconds = tp.tv_sec;

	timeval2syslogTime(&tp, t, inUTC);
}


/* A fast alternative to getCurrTime() and time() that only obtains
 * a timestamp like time() does. I was told that gettimeofday(), at
 * least under Linux, is much faster than time() and I could confirm
 * this testing. So I created that function as a replacement.
 * rgerhards, 2009-11-12
 */
time_t
getTime(time_t *ttSeconds)
{
	struct timeval tp;

	if(gettimeofday(&tp, NULL) == -1)
		return -1;

	if(ttSeconds != NULL)
		*ttSeconds = tp.tv_sec;
	return tp.tv_sec;
}

dateTimeFormat_t getDateTimeFormatFromStr(const char * const __restrict__ s) {
	assert(s != NULL);

	if (strcmp(s, "date-rfc3164") == 0)
		return DATE_RFC3164;
	if (strcmp(s, "date-rfc3339") == 0)
		return DATE_RFC3339;
	if (strcmp(s, "date-unix") == 0)
		return DATE_UNIX;

	return DATE_INVALID;
}

/*******************************************************************
 * BEGIN CODE-LIBLOGGING                                           *
 *******************************************************************
 * Code in this section is borrowed from liblogging. This is an
 * interim solution. Once liblogging is fully integrated, this is
 * to be removed (see http://www.monitorware.com/liblogging for
 * more details. 2004-11-16 rgerhards
 *
 * Please note that the orginal liblogging code is modified so that
 * it fits into the context of the current version of syslogd.c.
 *
 * DO NOT PUT ANY OTHER CODE IN THIS BEGIN ... END BLOCK!!!!
 */


/**
 * Parse a 32 bit integer number from a string. We do not permit
 * integer overruns, this the guard against INT_MAX.
 *
 * \param ppsz Pointer to the Pointer to the string being parsed. It
 *             must be positioned at the first digit. Will be updated
 *             so that on return it points to the first character AFTER
 *             the integer parsed.
 * \param pLenStr pointer to string length, decremented on exit by
 *                characters processed
 * 		  Note that if an empty string (len < 1) is passed in,
 * 		  the method always returns zero.
 * \retval The number parsed.
 */
static int
srSLMGParseInt32(uchar** ppsz, int *pLenStr)
{
	register int i;

	i = 0;
	while(*pLenStr > 0 && **ppsz >= '0' && **ppsz <= '9' && i < INT_MAX/10-1) {
		i = i * 10 + **ppsz - '0';
		++(*ppsz);
		--(*pLenStr);
	}

	return i;
}


/**
 * Parse a TIMESTAMP-3339.
 * updates the parse pointer position. The pTime parameter
 * is guranteed to be updated only if a new valid timestamp
 * could be obtained (restriction added 2008-09-16 by rgerhards).
 * This method now also checks the maximum string length it is passed.
 * If a *valid* timestamp is found, the string length is decremented
 * by the number of characters processed. If it is not a valid timestamp,
 * the length is kept unmodified. -- rgerhards, 2009-09-23
 */
static rsRetVal
ParseTIMESTAMP3339(struct syslogTime *pTime, uchar** ppszTS, int *pLenStr)
{
	uchar *pszTS = *ppszTS;
	/* variables to temporarily hold time information while we parse */
	int year;
	int month;
	int day;
	int hour; /* 24 hour clock */
	int minute;
	int second;
	int secfrac;	/* fractional seconds (must be 32 bit!) */
	int secfracPrecision;
	char OffsetMode;	/* UTC offset + or - */
	int OffsetHour;	/* UTC offset in hours */
	int OffsetMinute;	/* UTC offset in minutes */
	int lenStr;
	/* end variables to temporarily hold time information while we parse */
	DEFiRet;

	assert(pTime != NULL);
	assert(ppszTS != NULL);
	assert(pszTS != NULL);

	lenStr = *pLenStr;
	year = srSLMGParseInt32(&pszTS, &lenStr);

	/* We take the liberty to accept slightly malformed timestamps e.g. in
	 * the format of 2003-9-1T1:0:0. This doesn't hurt on receiving. Of course,
	 * with the current state of affairs, we would never run into this code
	 * here because at postion 11, there is no "T" in such cases ;)
	 */
	if(lenStr == 0 || *pszTS++ != '-' || year < 0 || year >= 2100) {
		DBGPRINTF("ParseTIMESTAMP3339: invalid year: %d, pszTS: '%c'\n", year, *pszTS);
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	}
	--lenStr;
	month = srSLMGParseInt32(&pszTS, &lenStr);
	if(month < 1 || month > 12)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != '-')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;
	day = srSLMGParseInt32(&pszTS, &lenStr);
	if(day < 1 || day > 31)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != 'T')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;

	hour = srSLMGParseInt32(&pszTS, &lenStr);
	if(hour < 0 || hour > 23)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != ':')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;
	minute = srSLMGParseInt32(&pszTS, &lenStr);
	if(minute < 0 || minute > 59)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != ':')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;
	second = srSLMGParseInt32(&pszTS, &lenStr);
	if(second < 0 || second > 60)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	/* Now let's see if we have secfrac */
	if(lenStr > 0 && *pszTS == '.') {
		--lenStr;
		uchar *pszStart = ++pszTS;
		secfrac = srSLMGParseInt32(&pszTS, &lenStr);
		secfracPrecision = (int) (pszTS - pszStart);
	} else {
		secfracPrecision = 0;
		secfrac = 0;
	}

	/* check the timezone */
	if(lenStr == 0)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(*pszTS == 'Z') {
		--lenStr;
		pszTS++; /* eat Z */
		OffsetMode = 'Z';
		OffsetHour = 0;
		OffsetMinute = 0;
	} else if((*pszTS == '+') || (*pszTS == '-')) {
		OffsetMode = *pszTS;
		--lenStr;
		pszTS++;

		OffsetHour = srSLMGParseInt32(&pszTS, &lenStr);
		if(OffsetHour < 0 || OffsetHour > 23)
			ABORT_FINALIZE(RS_RET_INVLD_TIME);

		if(lenStr == 0 || *pszTS != ':')
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		--lenStr;
		pszTS++;
		OffsetMinute = srSLMGParseInt32(&pszTS, &lenStr);
		if(OffsetMinute < 0 || OffsetMinute > 59)
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
	} else {
		/* there MUST be TZ information */
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	}

	/* OK, we actually have a 3339 timestamp, so let's indicated this */
	if(lenStr > 0) {
		if(*pszTS != ' ') /* if it is not a space, it can not be a "good" time - 2010-02-22 rgerhards */
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		++pszTS; /* just skip past it */
		--lenStr;
	}

	/* we had success, so update parse pointer and caller-provided timestamp */
	*ppszTS = pszTS;
	pTime->timeType = 2;
	pTime->year = year;
	pTime->month = month;
	pTime->day = day;
	pTime->hour = hour;
	pTime->minute = minute;
	pTime->second = second;
	pTime->secfrac = secfrac;
	pTime->secfracPrecision = secfracPrecision;
	pTime->OffsetMode = OffsetMode;
	pTime->OffsetHour = OffsetHour;
	pTime->OffsetMinute = OffsetMinute;
	*pLenStr = lenStr;

finalize_it:
	RETiRet;
}


/**
 * Parse a TIMESTAMP-3164. The pTime parameter
 * is guranteed to be updated only if a new valid timestamp
 * could be obtained (restriction added 2008-09-16 by rgerhards). This
 * also means the caller *must* provide a valid (probably current)
 * timstamp in pTime when calling this function. a 3164 timestamp contains
 * only partial information and only that partial information is updated.
 * So the "output timestamp" is a valid timestamp only if the "input
 * timestamp" was valid, too. The is actually an optimization, as it
 * permits us to use a pre-aquired timestamp and thus avoids to do
 * a (costly) time() call. Thanks to David Lang for insisting on
 * time() call reduction ;).
 * This method now also checks the maximum string length it is passed.
 * If a *valid* timestamp is found, the string length is decremented
 * by the number of characters processed. If it is not a valid timestamp,
 * the length is kept unmodified. -- rgerhards, 2009-09-23
 *
 * We support this format:
 * [yyyy] Mon mm [yyyy] hh:mm:ss[.subsec][ [yyyy ]/[TZSTRING:]]
 * Note that [yyyy] and [.subsec] are non-standard but frequently occur.
 * Also [yyyy] can only occur once -- if it occurs twice, we flag the
 * timestamp as invalid. if bParseTZ is true, we try to obtain a
 * TZSTRING. Note that in this case it MUST be terminated by a colon
 * (Cisco format). This option is a bit dangerous, as it could already
 * by the tag. So it MUST only be enabled in specialised parsers.
 * subsec, [yyyy] in front, TZSTRING was added in 2014-07-08 rgerhards
 * Similarly, we try to detect a year after the timestamp if
 * bDetectYearAfterTime is set. This is mutally exclusive with bParseTZ.
 * Note: bDetectYearAfterTime will misdetect hostnames in the range
 * 2000..2100 as years, so this option should explicitly be turned on
 * and is not meant for general consumption.
 */
static rsRetVal
ParseTIMESTAMP3164(struct syslogTime *pTime, uchar** ppszTS, int *pLenStr,
	const int bParseTZ,
	const int bDetectYearAfterTime)
{
	/* variables to temporarily hold time information while we parse */
	int month;
	int day;
	int year = 0; /* 0 means no year provided */
	int hour; /* 24 hour clock */
	int minute;
	int second;
	int secfrac;	/* fractional seconds (must be 32 bit!) */
	int secfracPrecision;
	char tzstring[16];
	char OffsetMode = '\0';	/* UTC offset: \0 -> indicate no update */
	char OffsetHour = 0;	/* UTC offset in hours */
	int OffsetMinute = 0;	/* UTC offset in minutes */
	/* end variables to temporarily hold time information while we parse */
	int lenStr;
	uchar *pszTS;
	DEFiRet;

	assert(ppszTS != NULL);
	pszTS = *ppszTS;
	assert(pszTS != NULL);
	assert(pTime != NULL);
	assert(pLenStr != NULL);
	lenStr = *pLenStr;

	if(lenStr < 3)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	/* first check if we have a year in front of the timestamp. some devices (e.g. Brocade)
	 * do this. As it is pretty straightforward to detect and chance of misinterpretation
	 * is low, we try to parse it.
	 */
	if(*pszTS >= '0' && *pszTS <= '9') {
		/* OK, either we have a prepended year or an invalid format! */
		year = srSLMGParseInt32(&pszTS, &lenStr);
		if(year < 1970 || year > 2100 || *pszTS != ' ')
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		++pszTS; /* skip SP */
	}

	/* If we look at the month (Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec),
	 * we may see the following character sequences occur:
	 *
	 * J(an/u(n/l)), Feb, Ma(r/y), A(pr/ug), Sep, Oct, Nov, Dec
	 *
	 * We will use this for parsing, as it probably is the
	 * fastest way to parse it.
	 *
	 * 2009-08-17: we now do case-insensitive comparisons, as some devices obviously do not
	 * obey to the RFC-specified case. As we need to guess in any case, we can ignore case
	 * in the first place -- rgerhards
	 *
	 * 2005-07-18, well sometimes it pays to be a bit more verbose, even in C...
	 * Fixed a bug that lead to invalid detection of the data. The issue was that
	 * we had an if(++pszTS == 'x') inside of some of the consturcts below. However,
	 * there were also some elseifs (doing the same ++), which than obviously did not
	 * check the orginal character but the next one. Now removed the ++ and put it
	 * into the statements below. Was a really nasty bug... I didn't detect it before
	 * june, when it first manifested. This also lead to invalid parsing of the rest
	 * of the message, as the time stamp was not detected to be correct. - rgerhards
	 */
	switch(*pszTS++)
	{
	case 'j':
	case 'J':
		if(*pszTS == 'a' || *pszTS == 'A') {
			++pszTS;
			if(*pszTS == 'n' || *pszTS == 'N') {
				++pszTS;
				month = 1;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else if(*pszTS == 'u' || *pszTS == 'U') {
			++pszTS;
			if(*pszTS == 'n' || *pszTS == 'N') {
				++pszTS;
				month = 6;
			} else if(*pszTS == 'l' || *pszTS == 'L') {
				++pszTS;
				month = 7;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'f':
	case 'F':
		if(*pszTS == 'e' || *pszTS == 'E') {
			++pszTS;
			if(*pszTS == 'b' || *pszTS == 'B') {
				++pszTS;
				month = 2;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'm':
	case 'M':
		if(*pszTS == 'a' || *pszTS == 'A') {
			++pszTS;
			if(*pszTS == 'r' || *pszTS == 'R') {
				++pszTS;
				month = 3;
			} else if(*pszTS == 'y' || *pszTS == 'Y') {
				++pszTS;
				month = 5;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'a':
	case 'A':
		if(*pszTS == 'p' || *pszTS == 'P') {
			++pszTS;
			if(*pszTS == 'r' || *pszTS == 'R') {
				++pszTS;
				month = 4;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else if(*pszTS == 'u' || *pszTS == 'U') {
			++pszTS;
			if(*pszTS == 'g' || *pszTS == 'G') {
				++pszTS;
				month = 8;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 's':
	case 'S':
		if(*pszTS == 'e' || *pszTS == 'E') {
			++pszTS;
			if(*pszTS == 'p' || *pszTS == 'P') {
				++pszTS;
				month = 9;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'o':
	case 'O':
		if(*pszTS == 'c' || *pszTS == 'C') {
			++pszTS;
			if(*pszTS == 't' || *pszTS == 'T') {
				++pszTS;
				month = 10;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'n':
	case 'N':
		if(*pszTS == 'o' || *pszTS == 'O') {
			++pszTS;
			if(*pszTS == 'v' || *pszTS == 'V') {
				++pszTS;
				month = 11;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	case 'd':
	case 'D':
		if(*pszTS == 'e' || *pszTS == 'E') {
			++pszTS;
			if(*pszTS == 'c' || *pszTS == 'C') {
				++pszTS;
				month = 12;
			} else
				ABORT_FINALIZE(RS_RET_INVLD_TIME);
		} else
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		break;
	default:
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	}

	lenStr -= 3;

	/* done month */

	if(lenStr == 0 || *pszTS++ != ' ')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;

	/* we accept a slightly malformed timestamp when receiving. This is
	 * we accept one-digit days
	 */
	if(*pszTS == ' ') {
		--lenStr;
		++pszTS;
	}

	day = srSLMGParseInt32(&pszTS, &lenStr);
	if(day < 1 || day > 31)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != ' ')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;

	/* time part */
	hour = srSLMGParseInt32(&pszTS, &lenStr);
	if(year == 0 && hour > 1970 && hour < 2100) {
		/* if so, we assume this actually is a year. This is a format found
		 * e.g. in Cisco devices.
		 * (if you read this 2100+ trying to fix a bug, congratulate me
		 * to how long the code survived - me no longer ;)) -- rgerhards, 2008-11-18
		 */
		year = hour;

		/* re-query the hour, this time it must be valid */
		if(lenStr == 0 || *pszTS++ != ' ')
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		--lenStr;
		hour = srSLMGParseInt32(&pszTS, &lenStr);
	}

	if(hour < 0 || hour > 23)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != ':')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;
	minute = srSLMGParseInt32(&pszTS, &lenStr);
	if(minute < 0 || minute > 59)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	if(lenStr == 0 || *pszTS++ != ':')
		ABORT_FINALIZE(RS_RET_INVLD_TIME);
	--lenStr;
	second = srSLMGParseInt32(&pszTS, &lenStr);
	if(second < 0 || second > 60)
		ABORT_FINALIZE(RS_RET_INVLD_TIME);

	/* as an extension e.g. found in CISCO IOS, we support sub-second resultion.
	 * It's presence is indicated by a dot immediately following the second.
	 */
	if(lenStr > 0 && *pszTS == '.') {
		--lenStr;
		uchar *pszStart = ++pszTS;
		secfrac = srSLMGParseInt32(&pszTS, &lenStr);
		secfracPrecision = (int) (pszTS - pszStart);
	} else {
		secfracPrecision = 0;
		secfrac = 0;
	}

	/* try to parse the TZSTRING if we are instructed to do so */
	if(bParseTZ && lenStr > 2 && *pszTS == ' ') {
		int i;
		for(  ++pszTS, --lenStr, i = 0
		    ; lenStr > 0 && i < (int) sizeof(tzstring) - 1 && *pszTS != ':' && *pszTS != ' '
		    ; --lenStr)
			tzstring[i++] = *pszTS++;
		if(i > 0) {
			/* found TZ, apply it */
			tzinfo_t* tzinfo;
			tzstring[i] = '\0';
			if((tzinfo = glblFindTimezoneInfo((char*) tzstring)) == NULL) {
				DBGPRINTF("ParseTIMESTAMP3164: invalid TZ string '%s' -- ignored\n",
					  tzstring);
			} else {
				OffsetMode = tzinfo->offsMode;
				OffsetHour = tzinfo->offsHour;
				OffsetMinute = tzinfo->offsMin;
			}
		}
	}
	if(bDetectYearAfterTime && year == 0 && lenStr > 5 && *pszTS == ' ') {
		int j;
		int y = 0;
		for(j = 1 ; j < 5 ; ++j) {
			if(pszTS[j] < '0' || pszTS[j] > '9')
				break;
			y = 10 * y + pszTS[j] - '0';
		}
		if(lenStr > 6 && pszTS[5] != ' ')
			y = 0; /* no year! */
		if(2000 <= y && y < 2100) {
			year = y;
			pszTS += 5; /* we need to preserve the SP, checked below */
			lenStr -= 5;
		}
	}

	/* we provide support for an extra ":" after the date. While this is an
	 * invalid format, it occurs frequently enough (e.g. with Cisco devices)
	 * to permit it as a valid case. -- rgerhards, 2008-09-12
	 */
	if(lenStr > 0 && *pszTS == ':') {
		++pszTS; /* just skip past it */
		--lenStr;
	}
	if(lenStr > 0) {
		if(*pszTS != ' ') /* if it is not a space, it can not be a "good" time - 2010-02-22 rgerhards */
			ABORT_FINALIZE(RS_RET_INVLD_TIME);
		++pszTS; /* just skip past it */
		--lenStr;
	}

	/* we had success, so update parse pointer and caller-provided timestamp
	 * fields we do not have are not updated in the caller's timestamp. This
	 * is the reason why the caller must pass in a correct timestamp.
	 */
	*ppszTS = pszTS; /* provide updated parse position back to caller */
	pTime->timeType = 1;
	pTime->month = month;
	if(year > 0)
		pTime->year = year; /* persist year if detected */
	pTime->day = day;
	pTime->hour = hour;
	pTime->minute = minute;
	pTime->second = second;
	pTime->secfrac = secfrac;
	pTime->secfracPrecision = secfracPrecision;
	if(OffsetMode != '\0') { /* need to update TZ info? */
		pTime->OffsetMode = OffsetMode;
		pTime->OffsetHour = OffsetHour;
		pTime->OffsetMinute = OffsetMinute;
	}
	*pLenStr = lenStr;

finalize_it:
	RETiRet;
}

void
applyDfltTZ(struct syslogTime *pTime, char *tz)
{
	pTime->OffsetMode = tz[0];
	pTime->OffsetHour = (tz[1] - '0') * 10 + (tz[2] - '0');
	pTime->OffsetMinute = (tz[4] - '0') * 10 + (tz[5] - '0');

}

/*******************************************************************
 * END CODE-LIBLOGGING                                             *
 *******************************************************************/

/**
 * Format a syslogTimestamp into format required by MySQL.
 * We are using the 14 digits format. For example 20041111122600
 * is interpreted as '2004-11-11 12:26:00'.
 * The caller must provide the timestamp as well as a character
 * buffer that will receive the resulting string. The function
 * returns the size of the timestamp written in bytes (without
 * the string terminator). If 0 is returend, an error occured.
 */
static int
formatTimestampToMySQL(struct syslogTime *ts, char* pBuf)
{
	/* currently we do not consider localtime/utc. This may later be
	 * added. If so, I recommend using a property replacer option
	 * and/or a global configuration option. However, we should wait
	 * on user requests for this feature before doing anything.
	 * rgerhards, 2007-06-26
	 */
	assert(ts != NULL);
	assert(pBuf != NULL);

	pBuf[0] = (ts->year / 1000) % 10 + '0';
	pBuf[1] = (ts->year / 100) % 10 + '0';
	pBuf[2] = (ts->year / 10) % 10 + '0';
	pBuf[3] = ts->year % 10 + '0';
	pBuf[4] = (ts->month / 10) % 10 + '0';
	pBuf[5] = ts->month % 10 + '0';
	pBuf[6] = (ts->day / 10) % 10 + '0';
	pBuf[7] = ts->day % 10 + '0';
	pBuf[8] = (ts->hour / 10) % 10 + '0';
	pBuf[9] = ts->hour % 10 + '0';
	pBuf[10] = (ts->minute / 10) % 10 + '0';
	pBuf[11] = ts->minute % 10 + '0';
	pBuf[12] = (ts->second / 10) % 10 + '0';
	pBuf[13] = ts->second % 10 + '0';
	pBuf[14] = '\0';
	return 15;

}

static int
formatTimestampToPgSQL(struct syslogTime *ts, char *pBuf)
{
	/* see note in formatTimestampToMySQL, applies here as well */
	assert(ts != NULL);
	assert(pBuf != NULL);

	pBuf[0] = (ts->year / 1000) % 10 + '0';
	pBuf[1] = (ts->year / 100) % 10 + '0';
	pBuf[2] = (ts->year / 10) % 10 + '0';
	pBuf[3] = ts->year % 10 + '0';
	pBuf[4] = '-';
	pBuf[5] = (ts->month / 10) % 10 + '0';
	pBuf[6] = ts->month % 10 + '0';
	pBuf[7] = '-';
	pBuf[8] = (ts->day / 10) % 10 + '0';
	pBuf[9] = ts->day % 10 + '0';
	pBuf[10] = ' ';
	pBuf[11] = (ts->hour / 10) % 10 + '0';
	pBuf[12] = ts->hour % 10 + '0';
	pBuf[13] = ':';
	pBuf[14] = (ts->minute / 10) % 10 + '0';
	pBuf[15] = ts->minute % 10 + '0';
	pBuf[16] = ':';
	pBuf[17] = (ts->second / 10) % 10 + '0';
	pBuf[18] = ts->second % 10 + '0';
	pBuf[19] = '\0';
	return 19;
}


/**
 * Format a syslogTimestamp to just the fractional seconds.
 * The caller must provide the timestamp as well as a character
 * buffer that will receive the resulting string. The function
 * returns the size of the timestamp written in bytes (without
 * the string terminator). If 0 is returend, an error occured.
 * The buffer must be at least 7 bytes large.
 * rgerhards, 2008-06-06
 */
static int
formatTimestampSecFrac(struct syslogTime *ts, char* pBuf)
{
	int iBuf;
	int power;
	int secfrac;
	short digit;

	assert(ts != NULL);
	assert(pBuf != NULL);

	iBuf = 0;
	if(ts->secfracPrecision > 0)
	{
		power = tenPowers[(ts->secfracPrecision - 1) % 6];
		secfrac = ts->secfrac;
		while(power > 0) {
			digit = secfrac / power;
			secfrac -= digit * power;
			power /= 10;
			pBuf[iBuf++] = digit + '0';
		}
	} else {
		pBuf[iBuf++] = '0';
	}
	pBuf[iBuf] = '\0';

	return iBuf;
}


/**
 * Format a syslogTimestamp to a RFC3339 timestamp string (as
 * specified in syslog-protocol).
 * The caller must provide the timestamp as well as a character
 * buffer that will receive the resulting string. The function
 * returns the size of the timestamp written in bytes (without
 * the string terminator). If 0 is returend, an error occured.
 */
static int
formatTimestamp3339(struct syslogTime *ts, char* pBuf)
{
	int iBuf;
	int power;
	int secfrac;
	short digit;

	BEGINfunc
	assert(ts != NULL);
	assert(pBuf != NULL);

	/* start with fixed parts */
	/* year yyyy */
	pBuf[0] = (ts->year / 1000) % 10 + '0';
	pBuf[1] = (ts->year / 100) % 10 + '0';
	pBuf[2] = (ts->year / 10) % 10 + '0';
	pBuf[3] = ts->year % 10 + '0';
	pBuf[4] = '-';
	/* month */
	pBuf[5] = (ts->month / 10) % 10 + '0';
	pBuf[6] = ts->month % 10 + '0';
	pBuf[7] = '-';
	/* day */
	pBuf[8] = (ts->day / 10) % 10 + '0';
	pBuf[9] = ts->day % 10 + '0';
	pBuf[10] = 'T';
	/* hour */
	pBuf[11] = (ts->hour / 10) % 10 + '0';
	pBuf[12] = ts->hour % 10 + '0';
	pBuf[13] = ':';
	/* minute */
	pBuf[14] = (ts->minute / 10) % 10 + '0';
	pBuf[15] = ts->minute % 10 + '0';
	pBuf[16] = ':';
	/* second */
	pBuf[17] = (ts->second / 10) % 10 + '0';
	pBuf[18] = ts->second % 10 + '0';

	iBuf = 19; /* points to next free entry, now it becomes dynamic! */

	if(ts->secfracPrecision > 0) {
		pBuf[iBuf++] = '.';
		power = tenPowers[(ts->secfracPrecision - 1) % 6];
		secfrac = ts->secfrac;
		while(power > 0) {
			digit = secfrac / power;
			secfrac -= digit * power;
			power /= 10;
			pBuf[iBuf++] = digit + '0';
		}
	}

	if(ts->OffsetMode == 'Z') {
		pBuf[iBuf++] = 'Z';
	} else {
		pBuf[iBuf++] = ts->OffsetMode;
		pBuf[iBuf++] = (ts->OffsetHour / 10) % 10 + '0';
		pBuf[iBuf++] = ts->OffsetHour % 10 + '0';
		pBuf[iBuf++] = ':';
		pBuf[iBuf++] = (ts->OffsetMinute / 10) % 10 + '0';
		pBuf[iBuf++] = ts->OffsetMinute % 10 + '0';
	}

	pBuf[iBuf] = '\0';

	ENDfunc
	return iBuf;
}

/**
 * Format a syslogTimestamp to a RFC3164 timestamp sring.
 * The caller must provide the timestamp as well as a character
 * buffer that will receive the resulting string. The function
 * returns the size of the timestamp written in bytes (without
 * the string termnator). If 0 is returend, an error occured.
 * rgerhards, 2010-03-05: Added support to for buggy 3164 dates,
 * where a zero-digit is written instead of a space for the first
 * day character if day < 10. syslog-ng seems to do that, and some
 * parsing scripts (in migration cases) rely on that.
 */
static int
formatTimestamp3164(struct syslogTime *ts, char* pBuf, int bBuggyDay)
{
	int iDay;
	assert(ts != NULL);
	assert(pBuf != NULL);
	
	pBuf[0] = monthNames[(ts->month - 1)% 12][0];
	pBuf[1] = monthNames[(ts->month - 1) % 12][1];
	pBuf[2] = monthNames[(ts->month - 1) % 12][2];
	pBuf[3] = ' ';
	iDay = (ts->day / 10) % 10; /* we need to write a space if the first digit is 0 */
	pBuf[4] = (bBuggyDay || iDay > 0) ? iDay + '0' : ' ';
	pBuf[5] = ts->day % 10 + '0';
	pBuf[6] = ' ';
	pBuf[7] = (ts->hour / 10) % 10 + '0';
	pBuf[8] = ts->hour % 10 + '0';
	pBuf[9] = ':';
	pBuf[10] = (ts->minute / 10) % 10 + '0';
	pBuf[11] = ts->minute % 10 + '0';
	pBuf[12] = ':';
	pBuf[13] = (ts->second / 10) % 10 + '0';
	pBuf[14] = ts->second % 10 + '0';
	pBuf[15] = '\0';
	return 16;	/* traditional: number of bytes written */
}


/**
 * convert syslog timestamp to time_t
 * Note: it would be better to use something similar to mktime() here.
 * Unfortunately, mktime() semantics are problematic: first of all, it
 * works on local time, on the machine's time zone. In syslog, we have
 * to deal with multiple time zones at once, so we cannot plainly rely
 * on the local zone, and so we cannot rely on mktime(). One solution would
 * be to refactor all time-related functions so that they are all guarded
 * by a mutex to ensure TZ consistency (which would also enable us to
 * change the TZ at will for specific function calls). But that would
 * potentially mean a lot of overhead.
 * Also, mktime() has some side effects, at least setting of tzname. With
 * a refactoring as described above that should probably not be a problem,
 * but would also need more work. For some more thoughts on this topic,
 * have a look here:
 * http://stackoverflow.com/questions/18355101/is-standard-c-mktime-thread-safe-on-linux
 * In conclusion, we keep our own code for generating the unix timestamp.
 * rgerhards, 2016-03-02
 */
static time_t
syslogTime2time_t(const struct syslogTime *ts)
{
	long MonthInDays, NumberOfYears, NumberOfDays;
	int utcOffset;
	time_t TimeInUnixFormat;

	if(ts->year < 1970 || ts->year > 2100) {
		TimeInUnixFormat = 0;
		LogError(0, RS_RET_ERR, "syslogTime2time_t: invalid year %d "
			"in timestamp - returning 1970-01-01 instead", ts->year);
		goto done;
	}

	/* Counting how many Days have passed since the 01.01 of the
	 * selected Year (Month level), according to the selected Month*/

	switch(ts->month)
	{
		case 1:
			MonthInDays = 0;         //until 01 of January
			break;
		case 2:
			MonthInDays = 31;        //until 01 of February - leap year handling down below!
			break;
		case 3:
			MonthInDays = 59;        //until 01 of March
			break;
		case 4:
			MonthInDays = 90;        //until 01 of April
			break;
		case 5:
			MonthInDays = 120;       //until 01 of Mai
			break;
		case 6:
			MonthInDays = 151;       //until 01 of June
			break;
		case 7:
			MonthInDays = 181;       //until 01 of July
			break;
		case 8:
			MonthInDays = 212;       //until 01 of August
			break;
		case 9:
			MonthInDays = 243;       //until 01 of September
			break;
		case 10:
			MonthInDays = 273;       //until 01 of Oktober
			break;
		case 11:
			MonthInDays = 304;       //until 01 of November
			break;
		case 12:
			MonthInDays = 334;       //until 01 of December
			break;
		default: /* this cannot happen (and would be a program error)
		          * but we need the code to keep the compiler silent.
			  */
			MonthInDays = 0;	/* any value fits ;) */
			break;
	}
	/* adjust for leap years */
	if((ts->year % 100 != 0 && ts->year % 4 == 0) || (ts->year == 2000)) {
		if(ts->month > 2)
			MonthInDays++;
	}


	/*	1) Counting how many Years have passed since 1970
		2) Counting how many Days have passed since the 01.01 of the selected Year
			(Day level) according to the Selected Month and Day. Last day doesn't count,
			it should be until last day
		3) Calculating this period (NumberOfDays) in seconds*/

	NumberOfYears = ts->year - yearInSec_startYear - 1;
	NumberOfDays = MonthInDays + ts->day - 1;
	TimeInUnixFormat = (time_t) (yearInSecs[NumberOfYears] + 1) + NumberOfDays * 86400;

	/*Add Hours, minutes and seconds */
	TimeInUnixFormat += ts->hour*60*60;
	TimeInUnixFormat += ts->minute*60;
	TimeInUnixFormat += ts->second;
	/* do UTC offset */
	utcOffset = ts->OffsetHour*3600 + ts->OffsetMinute*60;
	if(ts->OffsetMode == '+')
		utcOffset *= -1; /* if timestamp is ahead, we need to "go back" to UTC */
	TimeInUnixFormat += utcOffset;
done:
	return TimeInUnixFormat;
}


/**
 * format a timestamp as a UNIX timestamp; subsecond resolution is
 * discarded.
 * Note that this code can use some refactoring. I decided to use it
 * because mktime() requires an upfront TZ update as it works on local
 * time. In any case, it is worth reconsidering to move to mktime() or
 * some other method.
 * Important: pBuf must point to a buffer of at least 11 bytes.
 * rgerhards, 2012-03-29
 */
static int
formatTimestampUnix(struct syslogTime *ts, char *pBuf)
{
	snprintf(pBuf, 11, "%u", (unsigned) syslogTime2time_t(ts));
	return 11;
}

/* 0 - Sunday, 1, Monday, ...
 * Note that we cannot use strftime() and helpers as they rely on the TZ
 * variable (again, arghhhh). So we need to do it ourselves...
 * Note: in the year 2100, this algorithm does not work correctly (due to
 * leap time rules. To fix it then (*IF* this code really still exists then),
 * just use 2100 as new anchor year and adapt the initial day number.
 */
int getWeekdayNbr(struct syslogTime *ts)
{
	int wday;
	int g, f;

	g = ts->year;
	if(ts->month < 3) {
		g--;
		f = ts->month + 13;
	} else {
		f = ts->month + 1;
	}
	wday = ((36525*g)/100) + ((306*f)/10) + ts->day - 621049;
	wday %= 7;
	return wday;
}

/* getOrdinal - 1-366 day of the year
 * I've given little thought to leap seconds here.
 */
int getOrdinal(struct syslogTime *ts)
{
	int yday;
	time_t thistime;
	time_t previousyears;
	int utcOffset;
	time_t seconds_into_year;

	if(ts->year < 1970 || ts->year > 2100) {
		yday = 0;
		LogError(0, RS_RET_ERR, "getOrdinal: invalid year %d "
			"in timestamp - returning 1970-01-01 instead", ts->year);
		goto done;
	}

	thistime = syslogTime2time_t(ts);

	previousyears = (time_t) yearInSecs[ts->year - yearInSec_startYear - 1];

	/* adjust previous years to match UTC offset */
	utcOffset = ts->OffsetHour*3600 + ts->OffsetMinute*60;
	if(ts->OffsetMode == '+')
		utcOffset += -1; /* if timestamp is ahead, we need to "go back" to UTC */
	previousyears += utcOffset;

	/* subtract seconds from previous years */
	seconds_into_year = thistime - previousyears;

	/* divide by seconds in a day and truncate to int */
	yday = seconds_into_year / 86400;
done:
	return yday;
}

/* getWeek - 1-52 week of the year */
int getWeek(struct syslogTime *ts)
{
	int weekNum;
	struct syslogTime yt;
	int curDow;
	int jan1Dow;
	int curYearDay;

	/* initialize a timestamp for january 1st of the current year */
	yt.year = ts->year;
	yt.month = 1;
	yt.day = 1;
	yt.hour = 0;
	yt.minute = 0;
	yt.second = 0;
	yt.secfracPrecision = 0;
	yt.secfrac = 0;
	yt.OffsetMinute = ts->OffsetMinute;
	yt.OffsetHour = ts->OffsetHour;
	yt.OffsetMode = ts->OffsetMode;
	yt.timeType = TIME_TYPE_RFC3164; /* low-res time */

	/* get current day in year, current day of week
	 * and the day of week of 1/1 */
	curYearDay = getOrdinal(ts);
	curDow = getWeekdayNbr(ts);
	jan1Dow = getWeekdayNbr(&yt);

	/* calculate week of year for given date by pinning 1/1 as start
	 * of year, then going back and adjusting for the actual week day. */
	weekNum = ((curYearDay + 6) / 7);
	if (curDow < jan1Dow) {
		++weekNum;
	}
	return weekNum;
}

void
timeConvertToUTC(const struct syslogTime *const __restrict__ local,
	struct syslogTime *const __restrict__ utc)
{
	struct timeval tp;
	tp.tv_sec = syslogTime2time_t(local);
	tp.tv_usec = local->secfrac;
	timeval2syslogTime(&tp, utc, 1);
}

/**
 * Format a UNIX timestamp.
 */
static int
formatUnixTimeFromTime_t(time_t unixtime, const char *format, char *pBuf,
	__attribute__((unused)) uint pBufMax) {

	struct tm lt;

	assert(format != NULL);
	assert(pBuf != NULL);

	// Convert to struct tm
	if (gmtime_r(&unixtime, &lt) == NULL) {
		DBGPRINTF("Unexpected error calling gmtime_r().\n");
		return -1;
	}

	// Do our conversions
	if (strcmp(format, "date-rfc3164") == 0) {
		assert(pBufMax >= 16);

		// Unlikely to run into this situation, but you never know...
		if (lt.tm_mon < 0 || lt.tm_mon > 11) {
			DBGPRINTF("lt.tm_mon is out of range. Value: %d\n", lt.tm_mon);
			return -1;
		}

		// MMM dd HH:mm:ss
		sprintf(pBuf, "%s %2d %.2d:%.2d:%.2d",
			monthNames[lt.tm_mon], lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec
		);
	} else if (strcmp(format, "date-rfc3339") == 0) {
		assert(pBufMax >= 26);

		// YYYY-MM-DDTHH:mm:ss+00:00
		sprintf(pBuf, "%d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
			lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec
		);
	}

	return strlen(pBuf);
}

/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(datetime)
CODESTARTobjQueryInterface(datetime)
	if(pIf->ifVersion != datetimeCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->getCurrTime = getCurrTime;
	pIf->GetTime = getTime;
	pIf->timeval2syslogTime = timeval2syslogTime;
	pIf->ParseTIMESTAMP3339 = ParseTIMESTAMP3339;
	pIf->ParseTIMESTAMP3164 = ParseTIMESTAMP3164;
	pIf->formatTimestampToMySQL = formatTimestampToMySQL;
	pIf->formatTimestampToPgSQL = formatTimestampToPgSQL;
	pIf->formatTimestampSecFrac = formatTimestampSecFrac;
	pIf->formatTimestamp3339 = formatTimestamp3339;
	pIf->formatTimestamp3164 = formatTimestamp3164;
	pIf->formatTimestampUnix = formatTimestampUnix;
	pIf->syslogTime2time_t = syslogTime2time_t;
	pIf->formatUnixTimeFromTime_t = formatUnixTimeFromTime_t;
finalize_it:
ENDobjQueryInterface(datetime)


/* Initialize the datetime class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(datetime, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
ENDObjClassInit(datetime)

/* vi:set ai:
 */
