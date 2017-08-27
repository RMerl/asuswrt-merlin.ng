/*
 * timestr.c	See if a string like 'Su2300-0700' matches (UUCP style).
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>

#include <ctype.h>

int		timestr_match(char const *, time_t);

static char const *days[] =
	{ "su", "mo", "tu", "we", "th", "fr", "sa", "wk", "any", "al" };

#define DAYMIN		(24*60)
#define WEEKMIN		(24*60*7)
#define val(x)		(( (x) < 48 || (x) > 57) ? 0 : ((x) - 48))

#if 0 /* Set to 1 if you're a developer and want to debug this code */
#	define timestr_debug DEBUG2
#	define do_timestr_debug 1
#else
#	define timestr_debug if (0) printf
#endif

/*
 *	String code.
 */
static int strcode (char const **str)
{
	int i;
	size_t l;

	timestr_debug("strcode %s called\n", *str);

	for (i = 0; i < 10; i++) {
		l = strlen(days[i]);
		if (l > strlen(*str))
			continue;
		if (strncmp(*str, days[i], l) == 0) {
			*str += l;
			break;
		}
	}
	timestr_debug("strcode result %d\n", i);

	return (i >= 10) ? -1 : i;

}

/*
 *	Fill bitmap with hours/mins.
 */
static int hour_fill(char *bitmap, char const *tm)
{
	char *p;
	int start, end;
	int i, bit, byte;

	timestr_debug("hour_fill called for %s\n", tm);

	/*
	 *	Get timerange in start and end.
	 */
	end = -1;
	if ((p = strchr(tm, '-')) != NULL) {
		p++;
		if (p - tm != 5 || strlen(p) < 4 || !isdigit((int) *p))
			return 0;
		end = 600 * val(p[0]) + 60 * val(p[1]) + atoi(p + 2);
	}
	if (*tm == 0) {
		start = 0;
		end = DAYMIN - 1;
	} else {
		if (strlen(tm) < 4 || !isdigit((int) *tm))
			return 0;
		start = 600 * val(tm[0]) + 60 * val(tm[1]) + atoi(tm + 2);
		if (end < 0) end = start;
	}
	/* Treat 2400 as 0000, and do some more silent error checks. */
	if (end < 0) end = 0;
	if (start < 0) start = 0;
	if (end >= DAYMIN) end = DAYMIN - 1;
	if (start >= DAYMIN) start = DAYMIN - 1;

	timestr_debug("hour_fill: range from %d to %d\n", start, end);

	/*
	 *	Fill bitmap.
	 */
	i = start;
	while (1) {
		byte = (i / 8);
		bit = i % 8;
		timestr_debug("setting byte %d, bit %d\n", byte, bit);
		bitmap[byte] |= (1 << bit);
		if (i == end) break;
		i++;
		i %= DAYMIN;
	}
	return 1;
}

/*
 *	Call the fill bitmap function for every day listed.
 */
static int day_fill(char *bitmap, char const *tm)
{
	char const *hr;
	int n;
	int start, end;

	for (hr = tm; *hr; hr++)
		if (isdigit((int) *hr))
			break;
	if (hr == tm)
		tm = "Al";

	timestr_debug("dayfill: hr %s    tm %s\n", hr, tm);

	while ((start = strcode(&tm)) >= 0) {
		/*
		 *	Find start and end weekdays and
		 *	build a valid range 0 - 6.
		 */
		if (*tm == '-') {
			tm++;
			if ((end = strcode(&tm)) < 0)
				break;
		} else
			end = start;
		if (start == 7) {
			start = 1;
			end = 5;
		}
		if (start > 7) {
			start = 0;
			end = 6;
		}
		n = start;
		timestr_debug("day_fill: range from %d to %d\n", start, end);
		while (1) {
			hour_fill(bitmap + 180 * n, hr);
			if (n == end) break;
			n++;
			n %= 7;
		}
	}

	return 1;
}

/*
 *	Fill the week bitmap with allowed times.
 */
static int week_fill(char *bitmap, char const *tm)
{
	char *s;
	char tmp[128];

	strlcpy(tmp, tm, 128);
	tmp[127] = 0;
	for (s = tmp; *s; s++)
		if (isupper(*s)) *s = tolower(*s);

	s = strtok(tmp, ",|");
	while (s) {
		day_fill(bitmap, s);
		s = strtok(NULL, ",|");
	}

	return 0;
}

/*
 *	Match a timestring and return seconds left.
 *	-1 for no match, 0 for unlimited.
 */
int timestr_match(char const *tmstr, time_t t)
{
	struct tm *tm, s_tm;
	char bitmap[WEEKMIN / 8];
	int now, tot, i;
	int byte, bit;
#ifdef do_timestr_debug
	int y;
	char *s;
	char null[8];
#endif

	tm = localtime_r(&t, &s_tm);
	now = tm->tm_wday * DAYMIN + tm->tm_hour * 60 + tm->tm_min;
	tot = 0;
	memset(bitmap, 0, sizeof(bitmap));
	week_fill(bitmap, tmstr);

#ifdef do_timestr_debug
	memset(null, 0, 8);
	for (i = 0; i < 7; i++) {
		timestr_debug("%d: ", i);
		s = bitmap + 180 * i;
		for (y = 0; y < 23; y++) {
			s = bitmap + 180 * i + (75 * y) / 10;
			timestr_debug("%c", memcmp(s, null, 8) == 0 ? '.' : '#');
		}
		timestr_debug("\n");
	}
#endif

	/*
	 *	See how many minutes we have.
	 */
	i = now;
	while (1) {
		byte = i / 8;
		bit = i % 8;
		timestr_debug("READ: checking byte %d bit %d\n", byte, bit);
		if (!(bitmap[byte] & (1 << bit)))
			break;
		tot += 60;
		i++;
		i %= WEEKMIN;
		if (i == now)
			break;
	}

	if (tot == 0)
		return -1;

	return (i == now) ? 0 : tot;
}

#ifdef STANDALONE

int main(int argc, char **argv)
{
	int l;

	if (argc != 2) {
		fprintf(stderr, "Usage: test timestring\n");
		exit(1);
	}
	l = timestr_match(argv[1], time(NULL));
	printf ("%s: %d seconds left\n", argv[1], l);
	return 0;
}

#endif

