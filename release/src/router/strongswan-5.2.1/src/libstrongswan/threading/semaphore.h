/*
 * Copyright (C) 2011 Tobias Brunner
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
 * @defgroup semaphore semaphore
 * @{ @ingroup threading
 */

#ifndef THREADING_SEMAPHORE_H_
#define THREADING_SEMAPHORE_H_

#ifdef __APPLE__
/* Mach uses a semaphore_create() call, use a different name for ours */
#define semaphore_create(x) strongswan_semaphore_create(x)
#endif /* __APPLE__ */

typedef struct semaphore_t semaphore_t;

/**
 * A semaphore is basically an integer whose value is never allowed to be
 * lower than 0.  Two operations can be performed on it: increment the
 * value by one, and decrement the value by one.  If the value is currently
 * zero, then the decrement operation will blcok until the value becomes
 * greater than zero.
 */
struct semaphore_t {

	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases.
	 */
	void (*wait)(semaphore_t *this);

	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases, or the
	 * call times out.
	 *
	 * @param timeout		timeout im ms
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait)(semaphore_t *this, u_int timeout);

	/**
	 * Decrease the value by one, if it is greater than zero. Otherwise the
	 * current thread is blocked and it waits until the value increases, or the
	 * call times out.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param tv			absolute time until timeout
	 * @return				TRUE if timed out, FALSE otherwise
	 */
	bool (*timed_wait_abs)(semaphore_t *this, timeval_t tv);

	/**
	 * Increase the value by one. If the value becomes greater than zero, then
	 * another thread waiting will be woken up.
	 */
	void (*post)(semaphore_t *this);

	/**
	 * Destroy a semaphore and free its resources.
	 */
	void (*destroy)(semaphore_t *this);
};

/**
 * Create a semaphore instance.
 *
 * @param value		initial value (typically 0)
 * @return			semaphore instance
 */
semaphore_t *semaphore_create(u_int value);

#endif /** THREADING_SEMAPHORE_H_ @} */

