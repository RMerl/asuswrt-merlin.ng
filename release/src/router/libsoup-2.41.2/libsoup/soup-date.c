/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-date.c: Date/time handling
 *
 * Copyright (C) 2005, Novell, Inc.
 * Copyright (C) 2007, Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "soup-date.h"
#include "soup.h"

/**
 * SoupDate:
 * @year: the year, 1 to 9999
 * @month: the month, 1 to 12
 * @day: day of the month, 1 to 31
 * @hour: hour of the day, 0 to 23
 * @minute: minute, 0 to 59
 * @second: second, 0 to 59 (or up to 61 in the case of leap seconds)
 * @utc: %TRUE if the date is in UTC
 * @offset: offset from UTC

 * A date and time. The date is assumed to be in the (proleptic)
 * Gregorian calendar. The time is in UTC if @utc is %TRUE. Otherwise,
 * the time is a local time, and @offset gives the offset from UTC in
 * minutes (such that adding @offset to the time would give the
 * correct UTC time). If @utc is %FALSE and @offset is 0, then the
 * %SoupDate represents a "floating" time with no associated timezone
 * information.
 **/

/* Do not internationalize */
static const char *const months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* Do not internationalize */
static const char *const days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const int nonleap_days_in_month[] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const int nonleap_days_before[] = {
	0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static inline gboolean
is_leap_year (int year)
{
	return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

/* Computes the number of days since proleptic Gregorian 0000-12-31.
 * (That is, 0001-01-01 is "1", and 1970-01-01 is 719163.
 */
static int
rata_die_day (SoupDate *date)
{
	int day;

	day = (date->year - 1) * 365 + ((date->year - 1) / 4) -
		((date->year - 1) / 100) + ((date->year - 1) / 400);
	day += nonleap_days_before[date->month] + date->day;
	if (is_leap_year (date->year) && date->month > 2)
		day++;
	return day;
}

#define TIME_T_EPOCH_RATA_DIE_DAY 719163

static inline int
days_in_month (int month, int year)
{
	if (month == 2 && is_leap_year (year))
		return 29;
	else
		return nonleap_days_in_month[month];
}

G_DEFINE_BOXED_TYPE (SoupDate, soup_date, soup_date_copy, soup_date_free)

static void
soup_date_fixup (SoupDate *date)
{
	/* We only correct date->second if it's negative or too high
	 * to be a leap second.
	 */
	if (date->second < 0 || date->second > 61) {
		date->minute += date->second / 60;
		date->second %= 60;
		if (date->second < 0)
			date->second += 60;
	}

	if (date->minute < 0 || date->minute > 59) {
		date->hour += date->minute / 60;
		date->minute %= 60;
		if (date->minute < 0)
			date->minute += 60;
	}

	if (date->hour < 0 || date->hour > 23) {
		date->day += date->hour / 24;
		date->hour %= 24;
		if (date->hour < 0)
			date->hour += 24;
	}

	/* Have to make sure month is valid before we can look at the
	 * day.
	 */
	if (date->month < 1 || date->month > 12) {
		date->year += ((date->month - 1) / 12) + 1;
		date->month = ((date->month - 1) % 12) + 1;
		if (date->month < 1)
			date->month += 12;
	}

	if (date->day < 0) {
		while (date->day < 0) {
			if (date->month == 1) {
				date->month = 12;
				date->year--;
			} else
				date->month--;
			date->day += days_in_month (date->month, date->year);
		}
	} else {
		while (date->day > days_in_month (date->month, date->year)) {
			date->day -= days_in_month (date->month, date->year);
			if (date->month == 12) {
				date->month = 1;
				date->year++;
			} else
				date->month++;
		}
	}
}

/**
 * soup_date_new:
 * @year: the year (1-9999)
 * @month: the month (1-12)
 * @day: the day of the month (1-31, as appropriate for @month)
 * @hour: the hour (0-23)
 * @minute: the minute (0-59)
 * @second: the second (0-59, or up to 61 for leap seconds)
 *
 * Creates a #SoupDate representing the indicated time, UTC.
 *
 * Return value: a new #SoupDate
 **/
SoupDate *
soup_date_new (int year, int month, int day, 
	       int hour, int minute, int second)
{
	SoupDate *date = g_slice_new (SoupDate);

	date->year   = year;
	date->month  = month;
	date->day    = day;
	date->hour   = hour;
	date->minute = minute;
	date->second = second;
	date->utc    = TRUE;
	date->offset = 0;

	return date;
}

/**
 * soup_date_new_from_now:
 * @offset_seconds: offset from current time
 *
 * Creates a #SoupDate representing a time @offset_seconds after the
 * current time (or before it, if @offset_seconds is negative). If
 * offset_seconds is 0, returns the current time.
 *
 * If @offset_seconds would indicate a time not expressible as a
 * #time_t, the return value will be clamped into range.
 *
 * Return value: a new #SoupDate
 **/
SoupDate *
soup_date_new_from_now (int offset_seconds)
{
	time_t now = time (NULL);
	time_t then = now + offset_seconds;

	if (sizeof (time_t) == 4) {
		if (offset_seconds < 0 && then > now)
			return soup_date_new_from_time_t (-G_MAXINT);
		else if (offset_seconds > 0 && then < now)
			return soup_date_new_from_time_t (G_MAXINT);
	}
	return soup_date_new_from_time_t (then);
}

static gboolean
parse_iso8601_date (SoupDate *date, const char *date_string)
{
	gulong val;

	if (strlen (date_string) < 15)
		return FALSE;
	if (date_string[4] == '-' &&
	    date_string[7] == '-' &&
	    date_string[10] == 'T') {
		/* YYYY-MM-DD */
		date->year  = atoi (date_string);
		date->month = atoi (date_string + 5);
		date->day   = atoi (date_string + 8);
		date_string += 11;
	} else if (date_string[8] == 'T') {
		/* YYYYMMDD */
		val = atoi (date_string);
		date->year = val / 10000;
		date->month = (val % 10000) / 100;
		date->day = val % 100;
		date_string += 9;
	} else
		return FALSE;

	if (strlen (date_string) >= 8 &&
	    date_string[2] == ':' && date_string[5] == ':') {
		/* HH:MM:SS */
		date->hour   = atoi (date_string);
		date->minute = atoi (date_string + 3);
		date->second = atoi (date_string + 6);
		date_string += 8;
	} else if (strlen (date_string) >= 6) {
		/* HHMMSS */
		val = strtoul (date_string, (char **)&date_string, 10);
		date->hour   = val / 10000;
		date->minute = (val % 10000) / 100;
		date->second = val % 100;
	} else
		return FALSE;

	if (*date_string == '.' || *date_string == ',')
		(void) strtoul (date_string + 1, (char **)&date_string, 10);

	if (*date_string == 'Z') {
		date_string++;
		date->utc = TRUE;
		date->offset = 0;
	} else if (*date_string == '+' || *date_string == '-') {
		int sign = (*date_string == '+') ? -1 : 1;
		val = strtoul (date_string + 1, (char **)&date_string, 10);
		if (*date_string == ':')
			val = 60 * val + strtoul (date_string + 1, (char **)&date_string, 10);
		else
			val = 60 * (val / 100) + (val % 100);
		date->offset = sign * val;
		date->utc = !val;
	} else {
		date->offset = 0;
		date->utc = FALSE;
	}

	return !*date_string;
}

static inline gboolean
parse_day (SoupDate *date, const char **date_string)
{
	char *end;

	date->day = strtoul (*date_string, &end, 10);
	if (end == (char *)*date_string)
		return FALSE;

	while (*end == ' ' || *end == '-')
		end++;
	*date_string = end;
	return TRUE;
}

static inline gboolean
parse_month (SoupDate *date, const char **date_string)
{
	int i;

	for (i = 0; i < G_N_ELEMENTS (months); i++) {
		if (!g_ascii_strncasecmp (*date_string, months[i], 3)) {
			date->month = i + 1;
			*date_string += 3;
			while (**date_string == ' ' || **date_string == '-')
				(*date_string)++;
			return TRUE;
		}
	}
	return FALSE;
}

static inline gboolean
parse_year (SoupDate *date, const char **date_string)
{
	char *end;

	date->year = strtoul (*date_string, &end, 10);
	if (end == (char *)*date_string)
		return FALSE;

	if (end == (char *)*date_string + 2) {
		if (date->year < 70)
			date->year += 2000;
		else
			date->year += 1900;
	} else if (end == (char *)*date_string + 3)
		date->year += 1900;

	while (*end == ' ' || *end == '-')
		end++;
	*date_string = end;
	return TRUE;
}

static inline gboolean
parse_time (SoupDate *date, const char **date_string)
{
	char *p, *end;

	date->hour = strtoul (*date_string, &end, 10);
	if (end == (char *)*date_string || *end++ != ':')
		return FALSE;
	p = end;
	date->minute = strtoul (p, &end, 10);
	if (end == p || *end++ != ':')
		return FALSE;
	p = end;
	date->second = strtoul (p, &end, 10);
	if (end == p)
		return FALSE;
	p = end;

	while (*p == ' ')
		p++;
	*date_string = p;
	return TRUE;
}

static inline gboolean
parse_timezone (SoupDate *date, const char **date_string)
{
	if (!**date_string) {
		date->utc = FALSE;
		date->offset = 0;
	} else if (**date_string == '+' || **date_string == '-') {
		gulong val;
		int sign = (**date_string == '+') ? -1 : 1;
		val = strtoul (*date_string + 1, (char **)date_string, 10);
		if (**date_string == ':')
			val = 60 * val + strtoul (*date_string + 1, (char **)date_string, 10);
		else
			val =  60 * (val / 100) + (val % 100);
		date->offset = sign * val;
		date->utc = (sign == -1) && !val;
	} else if (**date_string == 'Z') {
		date->offset = 0;
		date->utc = TRUE;
		(*date_string)++;
	} else if (!strcmp (*date_string, "GMT") ||
		   !strcmp (*date_string, "UTC")) {
		date->offset = 0;
		date->utc = TRUE;
		(*date_string) += 3;
	} else if (strchr ("ECMP", **date_string) &&
		   ((*date_string)[1] == 'D' || (*date_string)[1] == 'S') &&
		   (*date_string)[2] == 'T') {
		date->offset = -60 * (5 * strcspn ("ECMP", *date_string));
		if ((*date_string)[1] == 'D')
			date->offset += 60;
		date->utc = FALSE;
	} else
		return FALSE;
	return TRUE;
}

static gboolean
parse_textual_date (SoupDate *date, const char *date_string)
{
	/* If it starts with a word, it must be a weekday, which we skip */
	if (g_ascii_isalpha (*date_string)) {
		while (g_ascii_isalpha (*date_string))
			date_string++;
		if (*date_string == ',')
			date_string++;
		while (g_ascii_isspace (*date_string))
			date_string++;
	}

	/* If there's now another word, this must be an asctime-date */
	if (g_ascii_isalpha (*date_string)) {
		/* (Sun) Nov  6 08:49:37 1994 */
		if (!parse_month (date, &date_string) ||
		    !parse_day (date, &date_string) ||
		    !parse_time (date, &date_string) ||
		    !parse_year (date, &date_string))
			return FALSE;

		/* There shouldn't be a timezone, but check anyway */
		parse_timezone (date, &date_string);
	} else {
		/* Non-asctime date, so some variation of
		 * (Sun,) 06 Nov 1994 08:49:37 GMT
		 */
		if (!parse_day (date, &date_string) ||
		    !parse_month (date, &date_string) ||
		    !parse_year (date, &date_string) ||
		    !parse_time (date, &date_string))
			return FALSE;

		/* This time there *should* be a timezone, but we
		 * survive if there isn't.
		 */
		parse_timezone (date, &date_string);
	}
	return TRUE;
}

/**
 * SoupDateFormat:
 * @SOUP_DATE_HTTP: RFC 1123 format, used by the HTTP "Date" header. Eg
 * "Sun, 06 Nov 1994 08:49:37 GMT"
 * @SOUP_DATE_COOKIE: The format for the "Expires" timestamp in the
 * Netscape cookie specification. Eg, "Sun, 06-Nov-1994 08:49:37 GMT".
 * @SOUP_DATE_RFC2822: RFC 2822 format, eg "Sun, 6 Nov 1994 09:49:37 -0100"
 * @SOUP_DATE_ISO8601_COMPACT: ISO 8601 date/time with no optional
 * punctuation. Eg, "19941106T094937-0100".
 * @SOUP_DATE_ISO8601_FULL: ISO 8601 date/time with all optional
 * punctuation. Eg, "1994-11-06T09:49:37-01:00".
 * @SOUP_DATE_ISO8601_XMLRPC: ISO 8601 date/time as used by XML-RPC.
 * Eg, "19941106T09:49:37".
 * @SOUP_DATE_ISO8601: An alias for @SOUP_DATE_ISO8601_FULL.
 *
 * Date formats that soup_date_to_string() can use.
 *
 * @SOUP_DATE_HTTP and @SOUP_DATE_COOKIE always coerce the time to
 * UTC. @SOUP_DATE_ISO8601_XMLRPC uses the time as given, ignoring the
 * offset completely. @SOUP_DATE_RFC2822 and the other ISO 8601
 * variants use the local time, appending the offset information if
 * available.
 *
 * This enum may be extended with more values in future releases.
 **/

/**
 * soup_date_new_from_string:
 * @date_string: the date in some plausible format
 *
 * Parses @date_string and tries to extract a date from it. This
 * recognizes all of the "HTTP-date" formats from RFC 2616, all ISO
 * 8601 formats containing both a time and a date, RFC 2822 dates,
 * and reasonable approximations thereof. (Eg, it is lenient about
 * whitespace, leading "0"s, etc.)
 *
 * Return value: a new #SoupDate, or %NULL if @date_string could not
 * be parsed.
 **/
SoupDate *
soup_date_new_from_string (const char *date_string)
{
	SoupDate *date;
	gboolean success;

	g_return_val_if_fail (date_string != NULL, NULL);

	date = g_slice_new (SoupDate);

	while (g_ascii_isspace (*date_string))
		date_string++;

	/* If it starts with a digit, it's either an ISO 8601 date, or
	 * an RFC2822 date without the optional weekday; in the later
	 * case, there will be a month name later on, so look for one
	 * of the month-start letters.
	 */
	if (g_ascii_isdigit (*date_string) &&
	    !strpbrk (date_string, "JFMASOND"))
		success = parse_iso8601_date (date, date_string);
	else
		success = parse_textual_date (date, date_string);

	if (!success) {
		g_slice_free (SoupDate, date);
		return NULL;
	}

	if (date->year < 1 || date->year > 9999 ||
	    date->month < 1 || date->month > 12 ||
	    date->day < 1 ||
	    date->day > days_in_month (date->month, date->year) ||
	    date->hour < 0 || date->hour > 24 ||
	    date->minute < 0 || date->minute > 59 ||
	    date->second < 0 || date->second > 61) {
		soup_date_free (date);
		return NULL;
	}
	if (date->hour == 24) {
		/* ISO8601 allows this explicitly. We allow it for
		 * other types as well just for simplicity.
		 */
		if (date->minute == 0 && date->second == 0)
			soup_date_fixup (date);
		else {
			soup_date_free (date);
			return NULL;
		}
	}

	return date;
}

/**
 * soup_date_new_from_time_t:
 * @when: a #time_t
 *
 * Creates a #SoupDate corresponding to @when
 *
 * Return value: a new #SoupDate
 **/
SoupDate *
soup_date_new_from_time_t (time_t when)
{
	struct tm tm;

#ifdef HAVE_GMTIME_R
	gmtime_r (&when, &tm);
#else
	tm = *gmtime (&when);
#endif

	return soup_date_new (tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			      tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static const char *
soup_date_weekday (SoupDate *date)
{
	/* Proleptic Gregorian 0001-01-01 was a Monday, which
	 * corresponds to 1 in the days[] array.
	 */
	return days[rata_die_day (date) % 7];
}

/**
 * soup_date_to_string:
 * @date: a #SoupDate
 * @format: the format to generate the date in
 *
 * Converts @date to a string in the format described by @format.
 *
 * Return value: @date as a string
 **/
char *
soup_date_to_string (SoupDate *date, SoupDateFormat format)
{
	g_return_val_if_fail (date != NULL, NULL);

	if (format == SOUP_DATE_HTTP || format == SOUP_DATE_COOKIE) {
		/* HTTP and COOKIE formats require UTC timestamp, so coerce
		 * @date if it's non-UTC.
		 */
		SoupDate utcdate;

		if (date->offset != 0) {
			memcpy (&utcdate, date, sizeof (SoupDate));
			utcdate.minute += utcdate.offset;
			utcdate.offset = 0;
			utcdate.utc = TRUE;
			soup_date_fixup (&utcdate);
			date = &utcdate;
		}

		switch (format) {
		case SOUP_DATE_HTTP:
			/* "Sun, 06 Nov 1994 08:49:37 GMT" */
			return g_strdup_printf (
				"%s, %02d %s %04d %02d:%02d:%02d GMT",
				soup_date_weekday (date), date->day,
				months[date->month - 1], date->year,
				date->hour, date->minute, date->second);

		case SOUP_DATE_COOKIE:
			/* "Sun, 06-Nov-1994 08:49:37 GMT" */
			return g_strdup_printf (
				"%s, %02d-%s-%04d %02d:%02d:%02d GMT",
				soup_date_weekday (date), date->day,
				months[date->month - 1], date->year,
				date->hour, date->minute, date->second);

		default:
			g_return_val_if_reached (NULL);
		}
	} else if (format == SOUP_DATE_ISO8601_XMLRPC) {
		/* Always "floating", ignore offset */
		return g_strdup_printf ("%04d%02d%02dT%02d:%02d:%02d",
					date->year, date->month, date->day,
					date->hour, date->minute, date->second);
	} else {
		int hour_offset, minute_offset;
		char zone[8], sign;

		/* For other ISO8601 formats or RFC2822, use the
		 * offset given in @date. For ISO8601 formats, use "Z"
		 * for UTC, +-offset for non-UTC, and nothing for
		 * floating. For RFC2822, use +-offset for UTC or
		 * non-UTC, and -0000 for floating.
		 */
		hour_offset = abs (date->offset) / 60;
		minute_offset = abs (date->offset) - hour_offset * 60;

		switch (format) {
		case SOUP_DATE_ISO8601_COMPACT:
			/* "19941106T084937[zone]" */
			if (date->utc)
				strcpy (zone, "Z");
			else if (date->offset) {
				g_snprintf (zone, sizeof (zone), "%c%02d%02d",
					    date->offset > 0 ? '-' : '+',
					    hour_offset, minute_offset);
			} else
				*zone = '\0';			

			return g_strdup_printf (
				"%04d%02d%02dT%02d%02d%02d%s",
				date->year, date->month, date->day,
				date->hour, date->minute, date->second,
				zone);

		case SOUP_DATE_ISO8601_FULL:
			/* "1994-11-06T08:49:37[zone]" */
			if (date->utc)
				strcpy (zone, "Z");
			else if (date->offset) {
				g_snprintf (zone, sizeof (zone), "%c%02d:%02d",
					    date->offset > 0 ? '-' : '+',
					    hour_offset, minute_offset);
			} else
				*zone = '\0';			

			return g_strdup_printf (
				"%04d-%02d-%02dT%02d:%02d:%02d%s",
				date->year, date->month, date->day,
				date->hour, date->minute, date->second,
				zone);

		case SOUP_DATE_RFC2822:
			/* "Sun, 6 Nov 1994 09:49:37 -0100" */
			if (date->offset)
				sign = (date->offset > 0) ? '-' : '+';
			else
				sign = date->utc ? '+' : '-';
			return g_strdup_printf (
				"%s, %d %s %04d %02d:%02d:%02d %c%02d%02d",
				soup_date_weekday (date), date->day,
				months[date->month - 1], date->year,
				date->hour, date->minute, date->second,
				sign, hour_offset, minute_offset);

		default:
			return NULL;
		}
	}
}

/**
 * soup_date_to_time_t:
 * @date: a #SoupDate
 *
 * Converts @date to a %time_t.
 *
 * If @date is not representable as a %time_t, it will be clamped into
 * range. (In particular, some HTTP cookies have expiration dates
 * after "Y2.038k" (2038-01-19T03:14:07Z).)
 *
 * Return value: @date as a %time_t
 **/
time_t
soup_date_to_time_t (SoupDate *date)
{
	time_t tt;
	GTimeVal val;

	g_return_val_if_fail (date != NULL, 0);

	/* FIXME: offset, etc */

	if (date->year < 1970)
		return 0;

	/* If the year is later than 2038, we're guaranteed to
	 * overflow a 32-bit time_t. (If it's exactly 2038, we'll
	 * *probably* overflow, but only by a little, and it's easiest
	 * to test that at the end by seeing if the result has turned
	 * negative.)
	 */
	if (sizeof (time_t) == 4 && date->year > 2038)
		return (time_t)0x7fffffff;

	soup_date_to_timeval (date, &val);
	tt = val.tv_sec;

	if (sizeof (time_t) == 4 && tt < 0)
		return (time_t)0x7fffffff;
	return tt;
}

/**
 * soup_date_to_timeval:
 * @date: a #SoupDate
 * @time: (out): a #GTimeVal structure in which to store the converted time.
 *
 * Converts @date to a #GTimeVal.
 *
 * Since: 2.24
 */
void
soup_date_to_timeval (SoupDate *date, GTimeVal *time)
{
	g_return_if_fail (date != NULL);
	g_return_if_fail (time != NULL);

	/* FIXME: offset, etc */

	time->tv_sec = rata_die_day (date) - TIME_T_EPOCH_RATA_DIE_DAY;
	time->tv_sec = ((((time->tv_sec * 24) + date->hour) * 60) + date->minute) * 60 + date->second;
	time->tv_usec = 0;
}

/**
 * soup_date_is_past:
 * @date: a #SoupDate
 *
 * Determines if @date is in the past.
 *
 * Return value: %TRUE if @date is in the past
 *
 * Since: 2.24
 **/
gboolean
soup_date_is_past (SoupDate *date)
{
	g_return_val_if_fail (date != NULL, TRUE);

	/* optimization */
	if (date->year < 2010)
		return TRUE;

	return soup_date_to_time_t (date) < time (NULL);
}

/**
 * soup_date_get_year:
 * @date: a #SoupDate
 *
 * Gets @date's year.
 *
 * Return value: @date's year
 *
 * Since: 2.32
 **/
int
soup_date_get_year (SoupDate *date)
{
	return date->year;
}

/**
 * soup_date_get_month:
 * @date: a #SoupDate
 *
 * Gets @date's month.
 *
 * Return value: @date's month
 *
 * Since: 2.32
 **/
int
soup_date_get_month (SoupDate *date)
{
	return date->month;
}

/**
 * soup_date_get_day:
 * @date: a #SoupDate
 *
 * Gets @date's day.
 *
 * Return value: @date's day
 *
 * Since: 2.32
 **/
int
soup_date_get_day (SoupDate *date)
{
	return date->day;
}

/**
 * soup_date_get_hour:
 * @date: a #SoupDate
 *
 * Gets @date's hour.
 *
 * Return value: @date's hour
 *
 * Since: 2.32
 **/
int
soup_date_get_hour (SoupDate *date)
{
	return date->hour;
}

/**
 * soup_date_get_minute:
 * @date: a #SoupDate
 *
 * Gets @date's minute.
 *
 * Return value: @date's minute
 *
 * Since: 2.32
 **/
int
soup_date_get_minute (SoupDate *date)
{
	return date->minute;
}

/**
 * soup_date_get_second:
 * @date: a #SoupDate
 *
 * Gets @date's second.
 *
 * Return value: @date's second
 *
 * Since: 2.32
 **/
int
soup_date_get_second (SoupDate *date)
{
	return date->second;
}

/**
 * soup_date_get_utc:
 * @date: a #SoupDate
 *
 * Gets @date's UTC flag
 *
 * Return value: %TRUE if @date is UTC.
 *
 * Since: 2.32
 **/
gboolean
soup_date_get_utc (SoupDate *date)
{
	return date->utc;
}

/**
 * soup_date_get_offset:
 * @date: a #SoupDate
 *
 * Gets @date's offset from UTC.
 *
 * Return value: @date's offset from UTC. If soup_date_get_utc()
 * returns %FALSE but soup_date_get_offset() returns 0, that means the
 * date is a "floating" time with no associated offset information.
 *
 * Since: 2.32
 **/
int
soup_date_get_offset (SoupDate *date)
{
	return date->offset;
}

/**
 * soup_date_copy:
 * @date: a #SoupDate
 *
 * Copies @date.
 *
 * Since: 2.24
 **/
SoupDate *
soup_date_copy (SoupDate *date)
{
	SoupDate *copy;

	g_return_val_if_fail (date != NULL, NULL);

	copy = g_slice_new (SoupDate);
	memcpy (copy, date, sizeof (SoupDate));
	return copy;
}

/**
 * soup_date_free:
 * @date: a #SoupDate
 *
 * Frees @date.
 *
 * Since: 2.24
 **/
void
soup_date_free (SoupDate *date)
{
	g_return_if_fail (date != NULL);

	g_slice_free (SoupDate, date);
}
