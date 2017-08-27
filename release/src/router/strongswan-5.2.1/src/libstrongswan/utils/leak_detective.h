/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup leak_detective leak_detective
 * @{ @ingroup utils
 */

#ifndef LEAK_DETECTIVE_H_
#define LEAK_DETECTIVE_H_

typedef struct leak_detective_t leak_detective_t;

#include <library.h>
#include <utils/backtrace.h>

/**
 * Callback function to report leak/usage information
 *
 * @param user			user specific data
 * @param count			number of allocations
 * @param bytes			total size of allocations
 * @param bt			backtrace of allocation
 * @param detailed		TRUE to show a detailed backtrace
 */
typedef void (*leak_detective_report_cb_t)(void *user, int count, size_t bytes,
										   backtrace_t *bt, bool detailed);

/**
 * Callback function to report leak/usage summary information
 *
 * @param user			user specific data
 * @param count			total number of allocations
 * @param bytes			total size of all reported allocations
 * @param whitelisted	number of allocations suppressed by whitelist
 */
typedef void (*leak_detective_summary_cb_t)(void* user, int count, size_t bytes,
										    int whitelisted);

/**
 * Leak detective finds leaks and invalid frees using malloc hooks.
 *
 * @todo Build an API for leak detective, allowing leak enumeration, statistics
 * and dynamic whitelisting.
 */
struct leak_detective_t {

	/**
	 * Report leaks to the registered callback functions.
	 *
	 * @param detailed 		TRUE to resolve line/filename of leaks (slow)
	 */
	void (*report)(leak_detective_t *this, bool detailed);

	/**
	 * Set callback functions invoked when report() is called.
	 *
	 * @param cb			callback invoked for each detected leak
	 * @param scb			summary callback invoked at end of report
	 * @param user			user data to supply to callbacks
	 */
	void (*set_report_cb)(leak_detective_t *this, leak_detective_report_cb_t cb,
						  leak_detective_summary_cb_t scb, void *user);

	/**
	 * Report current memory usage using callback functions.
	 *
	 * @param cb			callback invoked for each allocation
	 * @param scb			summary callback invoked at end of usage report
	 * @param user			user data to supply to callbacks
	 */
	void (*usage)(leak_detective_t *this, leak_detective_report_cb_t cb,
				  leak_detective_summary_cb_t scb, void *user);

	/**
	 * Number of detected leaks.
	 *
	 * @return				number of leaks
	 */
	int (*leaks)(leak_detective_t *this);

	/**
	 * Enable/disable leak detective hooks for the current thread.
	 *
	 * @param				TRUE to enable, FALSE to disable
	 * @return				state active before calling set_state
	 */
	bool (*set_state)(leak_detective_t *this, bool enabled);

	/**
	 * Destroy a leak_detective instance.
	 */
	void (*destroy)(leak_detective_t *this);
};

/**
 * Create a leak_detective instance, unless the LEAK_DETECTIVE_DISABLE
 * environment variable is set.
 *
 * @return					leak detective instance
 */
leak_detective_t *leak_detective_create();

#endif /** LEAK_DETECTIVE_H_ @}*/
