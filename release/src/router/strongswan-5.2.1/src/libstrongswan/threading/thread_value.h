/*
 * Copyright (C) 2009 Tobias Brunner
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
 * @defgroup thread_value thread_value
 * @{ @ingroup threading
 */

#ifndef THREADING_THREAD_VALUE_H_
#define THREADING_THREAD_VALUE_H_

#include <threading/thread.h>

typedef struct thread_value_t thread_value_t;

/**
 * Wrapper for thread-specific values.
 */
struct thread_value_t {

	/**
	 * Set a thread-specific value.
	 *
	 * @param val		thread specific value
	 */
	void (*set)(thread_value_t *this, void *val);

	/**
	 * Get a thread-specific value.
	 *
	 * @return			the value specific to the current thread
	 */
	void *(*get)(thread_value_t *this);

	/**
	 * Destroys this thread specific value wrapper. There is no check for
	 * non-NULL values which are currently assigned to the calling thread, no
	 * destructor is called.
	 */
	void (*destroy)(thread_value_t *this);

};

/**
 * Create a new thread-specific value wrapper.
 *
 * The optional destructor is called whenever a thread terminates, with the
 * assigned value as argument. It is not called if that value is NULL.
 *
 * @param destructor	destructor
 * @return				thread-specific value wrapper
 */
thread_value_t *thread_value_create(thread_cleanup_t destructor);

#endif /** THREADING_THREAD_VALUE_H_ @} */

