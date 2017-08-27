/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup rwlock_condvar rwlock_condvar
 * @{ @ingroup threading
 */

#ifndef RWLOCK_CONDVAR_H_
#define RWLOCK_CONDVAR_H_

typedef struct rwlock_condvar_t rwlock_condvar_t;

#include "rwlock.h"

/**
 * A special condvar implementation that can be used in conjunction
 * with rwlock_t (the write lock to be precise).
 *
 * @note The implementation does not verify that the current thread actually
 * holds the write lock and not the read lock, so watch out.
 */
struct rwlock_condvar_t {

	/**
	 * Wait on a condvar until it gets signalized.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 */
	void (*wait)(rwlock_condvar_t *this, rwlock_t *lock);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 * @param timeout		timeout im ms
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait)(rwlock_condvar_t *this, rwlock_t *lock, u_int timeout);

	/**
	 * Wait on a condvar until it gets signalized, or times out.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param lock			lock to release while waiting (write lock)
	 * @param tv			absolute time until timeout
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait_abs)(rwlock_condvar_t *this, rwlock_t *lock,
						   timeval_t tv);

	/**
	 * Wake up a single thread in a condvar.
	 */
	void (*signal)(rwlock_condvar_t *this);

	/**
	 * Wake up all threads in a condvar.
	 */
	void (*broadcast)(rwlock_condvar_t *this);

	/**
	 * Destroy a condvar and free its resources.
	 */
	void (*destroy)(rwlock_condvar_t *this);
};

/**
 * Create a condvar instance.
 *
 * @return			condvar instance
 */
rwlock_condvar_t *rwlock_condvar_create();

#endif /** RWLOCK_CONDVAR_H_ @} */

