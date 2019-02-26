/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup spinlock spinlock
 * @{ @ingroup threading
 */

#ifndef THREADING_SPINLOCK_H_
#define THREADING_SPINLOCK_H_

typedef struct spinlock_t spinlock_t;

/**
 * Spin lock wrapper implements a lock with low overhead when the lock is held
 * only for a short time (waiting wastes processor cycles, though).
 *
 * If native spin locks are not available regular mutexes are used as fallback.
 */
struct spinlock_t {

	/**
	 * Acquire the lock.
	 */
	void (*lock)(spinlock_t *this);

	/**
	 * Release the lock.
	 */
	void (*unlock)(spinlock_t *this);

	/**
	 * Destroy the instance.
	 */
	void (*destroy)(spinlock_t *this);
};

/**
 * Create a spin lock instance.
 *
 * @return			unlocked instance
 */
spinlock_t *spinlock_create();

#endif /** THREADING_SPINLOCK_H_ @} */

