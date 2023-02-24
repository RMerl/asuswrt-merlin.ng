/*
 * Copyright (C) 2020 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#ifndef HASHTABLE_PROFILER_H_
#define HASHTABLE_PROFILER_H_

#ifdef HASHTABLE_PROFILER

#include <time.h>
#include <utils/backtrace.h>

typedef struct hashtable_profile_t hashtable_profile_t;

struct hashtable_profile_t {

	/**
	 * Some stats to profile lookups in the table
	 */
	struct {
		size_t count;
		size_t probes;
		size_t longest;
	} success, failure;

	/**
	 * Stats on the memory usage of the table
	 */
	struct {
		size_t count;
		size_t size;
	} max;

	/**
	 * Keep track of where the hash table was created
	 */
	backtrace_t *backtrace;
};

/**
 * Print and cleanup profiling data
 */
static inline void profiler_cleanup(hashtable_profile_t *profile, u_int count,
									u_int size)
{
	if (profile->success.count || profile->failure.count)
	{
		fprintf(stderr, "%zu elements [max. %zu], %zu buckets [%zu], %zu "
				"successful / %zu failed lookups, %.4f [%zu] / %.4f "
				"[%zu] avg. probes in table created at:",
				count, profile->max.count, size, profile->max.size,
				profile->success.count, profile->failure.count,
				(double)profile->success.probes/profile->success.count,
				profile->success.longest,
				(double)profile->failure.probes/profile->failure.count,
				profile->failure.longest);
		profile->backtrace->log(profile->backtrace, stderr, TRUE);
	}
	profile->backtrace->destroy(profile->backtrace);
}

/**
 * Initialize profiling data
 */
static inline void profiler_init(hashtable_profile_t *profile, int skip)
{
	profile->backtrace = backtrace_create(skip);
}

#define lookup_start() \
	u_int _lookup_probes = 0;

#define lookup_probing() \
	_lookup_probes++;

#define _lookup_done(profile, result) \
	(profile)->result.count++; \
	(profile)->result.probes += _lookup_probes; \
	(profile)->result.longest = max((profile)->result.longest, _lookup_probes);

#define lookup_success(profile) _lookup_done(profile, success);
#define lookup_failure(profile) _lookup_done(profile, failure);

static inline void profile_size(hashtable_profile_t *profile, u_int size)
{
	profile->max.size = max(profile->max.size, size);
}

static inline void profile_count(hashtable_profile_t *profile, u_int count)
{
	profile->max.count = max(profile->max.count, count);
}

#else /* !HASHTABLE_PROFILER */

#define hashtable_profile_t struct {}
#define profiler_cleanup(...) {}
#define profiler_init(...) {}
#define lookup_start(...) {}
#define lookup_probing(...) {}
#define lookup_success(...) {}
#define lookup_failure(...) {}
#define profile_size(...) {}
#define profile_count(...) {}

#endif /* HASHTABLE_PROFILER */

#endif /* HASHTABLE_PROFILER_H_ */
