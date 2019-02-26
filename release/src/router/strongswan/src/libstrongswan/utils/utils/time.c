/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifdef WIN32
/* for GetTickCount64, Windows 7 */
# define _WIN32_WINNT 0x0601
#endif

#define _GNU_SOURCE
#include <utils/utils.h>

#include <inttypes.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

/**
 * Return monotonic time
 */
time_t time_monotonic(timeval_t *tv)
{
#ifdef WIN32
	ULONGLONG ms;
	time_t s;

	ms = GetTickCount64();
	s = ms / 1000;
	if (tv)
	{
		tv->tv_sec = s;
		tv->tv_usec = (ms - (s * 1000)) * 1000;
	}
	return s;
#else /* !WIN32 */
#if defined(HAVE_CLOCK_GETTIME) && \
	(defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
	 defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
	/* as we use time_monotonic() for condvar operations, we use the
	 * monotonic time source only if it is also supported by pthread. */
	timespec_t ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
	{
		if (tv)
		{
			tv->tv_sec = ts.tv_sec;
			tv->tv_usec = ts.tv_nsec / 1000;
		}
		return ts.tv_sec;
	}
#endif /* HAVE_CLOCK_GETTIME && (...) */
	/* Fallback to non-monotonic timestamps:
	 * On MAC OS X, creating monotonic timestamps is rather difficult. We
	 * could use mach_absolute_time() and catch sleep/wakeup notifications.
	 * We stick to the simpler (non-monotonic) gettimeofday() for now.
	 * But keep in mind: we need the same time source here as in condvar! */
	if (!tv)
	{
		return time(NULL);
	}
	if (gettimeofday(tv, NULL) != 0)
	{	/* should actually never fail if passed pointers are valid */
		return -1;
	}
	return tv->tv_sec;
#endif /* !WIN32 */
}

/*
 * Described in header
 */
bool timespan_from_string(char *str, char *defunit, time_t *val)
{
	char *endptr, unit;
	time_t timeval;

	if (str)
	{
		errno = 0;
		timeval = strtoull(str, &endptr, 10);
		if (endptr == str)
		{
			return FALSE;
		}
		if (errno == 0)
		{
			while (isspace(*endptr))
			{
				endptr++;
			}
			unit = *endptr;
			if (!unit && defunit)
			{
				unit = *defunit;
			}
			switch (unit)
			{
				case 'd':		/* time in days */
					timeval *= 24 * 3600;
					break;
				case 'h':		/* time in hours */
					timeval *= 3600;
					break;
				case 'm':		/* time in minutes */
					timeval *= 60;
					break;
				case 's':		/* time in seconds */
				case '\0':
					break;
				default:
					return FALSE;
			}
			if (val)
			{
				*val = timeval;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Described in header
 */
int time_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args)
{
	static const char* months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	time_t *time = *((time_t**)(args[0]));
	bool utc = *((int*)(args[1]));
	struct tm t, *ret = NULL;

	if (*time != UNDEFINED_TIME)
	{
		if (utc)
		{
			ret = gmtime_r(time, &t);
		}
		else
		{
			ret = localtime_r(time, &t);
		}
	}
	if (ret == NULL)
	{
		return print_in_hook(data, "--- -- --:--:--%s----",
							 utc ? " UTC " : " ");
	}
	return print_in_hook(data, "%s %02d %02d:%02d:%02d%s%04d",
						 months[t.tm_mon], t.tm_mday, t.tm_hour, t.tm_min,
						 t.tm_sec, utc ? " UTC " : " ", t.tm_year + 1900);
}

/*
 * Described in header
 */
int time_delta_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						   const void *const *args)
{
	char* unit = "second";
	time_t *arg1 = *((time_t**)(args[0]));
	time_t *arg2 = *((time_t**)(args[1]));
	uint64_t delta = llabs(*arg1 - *arg2);

	if (delta > 2 * 60 * 60 * 24)
	{
		delta /= 60 * 60 * 24;
		unit = "day";
	}
	else if (delta > 2 * 60 * 60)
	{
		delta /= 60 * 60;
		unit = "hour";
	}
	else if (delta > 2 * 60)
	{
		delta /= 60;
		unit = "minute";
	}
	return print_in_hook(data, "%" PRIu64 " %s%s", delta, unit,
						 (delta == 1) ? "" : "s");
}
