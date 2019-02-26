/*
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
 * @defgroup backtrace backtrace
 * @{ @ingroup utils
 */

#ifndef BACKTRACE_H_
#define BACKTRACE_H_

typedef struct backtrace_t backtrace_t;

#include <stdio.h>

#include <library.h>

/**
 * A backtrace registers the frames on the stack during creation.
 */
struct backtrace_t {

	/**
	 * Log the backtrace to a FILE stream.
	 *
	 * If no file pointer is given, the backtrace is reported over the debug
	 * framework to the registered dbg() callback function.
	 *
	 * @param file		FILE to log backtrace to, NULL for dbg() function
	 * @param detailed	TRUE to resolve line/file using addr2line (slow)
	 */
	void (*log)(backtrace_t *this, FILE *file, bool detailed);

	/**
	 * Check if the backtrace contains a frame having a function in a list.
	 *
	 * @param		function name array
	 * @param		number of elements in function array
	 * @return		TRUE if one of the functions is in the stack
	 */
	bool (*contains_function)(backtrace_t *this, char *function[], int count);

	/**
	 * Check two backtraces for equality.
	 *
	 * @param other	backtrace to compare to this
	 * @return		TRUE if backtraces are equal
	 */
	bool (*equals)(backtrace_t *this, backtrace_t *other);

	/**
	 * Create a copy of this backtrace.
	 *
	 * @return		cloned copy
	 */
	backtrace_t* (*clone)(backtrace_t *this);

	/**
	 * Create an enumerator over the stack frame addresses.
	 *
	 * @return		enumerator_t over void*
	 */
	enumerator_t* (*create_frame_enumerator)(backtrace_t *this);

	/**
	 * Destroy a backtrace instance.
	 */
	void (*destroy)(backtrace_t *this);
};

/**
 * Create a backtrace of the current stack.
 *
 * @param skip		how many of the innerst frames to skip
 * @return			backtrace
 */
backtrace_t *backtrace_create(int skip);

/**
 * Create a backtrace, dump it and clean it up.
 *
 * @param label		description to print for this backtrace, or NULL
 * @param file		FILE to log backtrace to, NULL to dbg() function
 * @param detailed	TRUE to resolve line/file using addr2line (slow)
 */
void backtrace_dump(char *label, FILE *file, bool detailed);

/**
 * Initialize backtracing framework.
 */
void backtrace_init();

/**
 * Deinitialize backtracing framework.
 */
void backtrace_deinit();

#endif /** BACKTRACE_H_ @}*/
