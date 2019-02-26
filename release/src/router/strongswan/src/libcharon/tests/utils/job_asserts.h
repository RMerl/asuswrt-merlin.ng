/*
 * Copyright (C) 2016 Tobias Brunner
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
 * Special assertions against job handling.
 *
 * @defgroup job_asserts job_asserts
 * @{ @ingroup test_utils_c
 */

#ifndef JOB_ASSERTS_H_
#define JOB_ASSERTS_H_

/**
 * Initialize an assertion that enforces that no jobs were scheduled.
 * Must be matched by a call to assert_scheduler().
 */
#define assert_no_jobs_scheduled() _assert_jobs_scheduled(0)

/**
 * Initialize an assertion that enforces that a specific number of jobs was
 * scheduled.
 * Must be matched by a call to assert_scheduler().
 *
 * @param count			expected number of jobs getting scheduled
 */
#define assert_jobs_scheduled(count) _assert_jobs_scheduled(count)

/**
 * Initialize assertions against job scheduling.
 * Must be matched by a call to assert_scheduler().
 */
#define _assert_jobs_scheduled(count) \
do { \
	u_int _initial = lib->scheduler->get_job_load(lib->scheduler); \
	u_int _expected = count

/**
 * Enforce scheduler asserts.
 */
#define assert_scheduler() \
	u_int _actual = lib->scheduler->get_job_load(lib->scheduler) - _initial; \
	test_assert_msg(_expected == _actual, "unexpected number of jobs " \
					"scheduled (%u != %u)", _expected, _actual); \
} while(FALSE)

#endif /** JOB_ASSERTS_H_ @}*/
