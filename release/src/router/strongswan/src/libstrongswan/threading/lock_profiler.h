/*
 * Copyright (C) 2008 Tobias Brunner
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

#ifndef THREADING_LOCK_PROFILER_H_
#define THREADING_LOCK_PROFILER_H_

#ifdef LOCK_PROFILER

#include <time.h>

/**
 * Do not report mutexes with an overall waiting time smaller than this (in us)
 */
#define PROFILE_WAIT_TRESHHOLD 10000

/**
 * Do not report mutexes with an overall lock count smaller than this
 */
#define PROFILE_LOCK_TRESHHOLD 1000

#include <utils/backtrace.h>

typedef struct lock_profile_t lock_profile_t;

struct lock_profile_t {
	/**
	 * how long threads have waited for the lock in this mutex so far
	 */
	timeval_t waited;

	/**
	 * How many times the lock has been invoked
	 */
	u_int locked;

	/**
	 * backtrace where mutex has been created
	 */
	backtrace_t *backtrace;
};

/**
 * Print and cleanup mutex profiler
 */
static inline void profiler_cleanup(lock_profile_t *profile)
{
	if (profile->waited.tv_sec > 0 ||
		profile->waited.tv_usec > PROFILE_WAIT_TRESHHOLD ||
		profile->locked > PROFILE_LOCK_TRESHHOLD)
	{
		fprintf(stderr, "%d.%03ds / %d times in lock created at:",
			profile->waited.tv_sec, profile->waited.tv_usec, profile->locked);
		profile->backtrace->log(profile->backtrace, stderr, TRUE);
	}
	profile->backtrace->destroy(profile->backtrace);
}

/**
 * Initialize mutex profiler
 */
static inline void profiler_init(lock_profile_t *profile)
{
	profile->backtrace = backtrace_create(2);
	timerclear(&profile->waited);
	profile->locked = 0;
}

#define profiler_start(profile) { \
	struct timeval _start, _end, _diff; \
	(profile)->locked++; \
	time_monotonic(&_start);

#define profiler_end(profile) \
	time_monotonic(&_end); \
	timersub(&_end, &_start, &_diff); \
	timeradd(&(profile)->waited, &_diff, &(profile)->waited); }

#else /* !LOCK_PROFILER */

#define lock_profile_t struct {}
#define profiler_cleanup(...) {}
#define profiler_init(...) {}
#define profiler_start(...) {}
#define profiler_end(...) {}

#endif /* LOCK_PROFILER */

#endif /* THREADING_LOCK_PROFILER_H_ */

