/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

/**
 * @defgroup time_i time
 * @{ @ingroup utils_i
 */

#ifndef TIME_H_
#define TIME_H_

/**
 * time_t not defined
 */
#define UNDEFINED_TIME 0

/**
 * Maximum time since epoch causing wrap-around on Jan 19 03:14:07 UTC 2038
 */
#define TIME_32_BIT_SIGNED_MAX	0x7fffffff

/**
 * Handle struct timeval like an own type.
 */
typedef struct timeval timeval_t;

/**
 * Handle struct timespec like an own type.
 */
typedef struct timespec timespec_t;

/**
 * Get a timestamp from a monotonic time source.
 *
 * While the time()/gettimeofday() functions are affected by leap seconds
 * and system time changes, this function returns ever increasing monotonic
 * time stamps.
 *
 * @param tv		timeval struct receiving monotonic timestamps, or NULL
 * @return			monotonic timestamp in seconds
 */
time_t time_monotonic(timeval_t *tv);

/**
 * Add the given number of milliseconds to the given timeval struct
 *
 * @param tv		timeval struct to modify
 * @param ms		number of milliseconds
 */
static inline void timeval_add_ms(timeval_t *tv, u_int ms)
{
	tv->tv_usec += ms * 1000;
	while (tv->tv_usec >= 1000000 /* 1s */)
	{
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
}

/**
 * Parse the given string as time span and return the number of seconds,
 * optionally with a default unit ('s' for seconds, 'm' for minutes, 'h' for
 * hours, 'd' for days - default is 's').
 *
 * @param str		value to parse
 * @param defunit	optional default unit
 * @param[out] val	parsed value
 * @return			TRUE if a value was parsed
 */
bool timespan_from_string(char *str, char *defunit, time_t *val);

/**
 * printf hook for time_t.
 *
 * Arguments are:
 *	time_t* time, bool utc
 */
int time_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args);

/**
 * printf hook for time_t deltas.
 *
 * Arguments are:
 *	time_t* begin, time_t* end
 */
int time_delta_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						   const void *const *args);

#endif /** TIME_H_ @} */
