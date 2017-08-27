/*
 * Copyright (C) 2008-2009 Tobias Brunner
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
 * @defgroup rwlock rwlock
 * @{ @ingroup threading
 */

#ifndef THREADING_RWLOCK_H_
#define THREADING_RWLOCK_H_

typedef struct rwlock_t rwlock_t;
typedef enum rwlock_type_t rwlock_type_t;

/**
 * Type of read-write lock.
 */
enum rwlock_type_t {
	/** default condvar */
	RWLOCK_TYPE_DEFAULT = 0,
};

/**
 * Read-Write lock wrapper.
 */
struct rwlock_t {

	/**
	 * Acquire the read lock.
	 */
	void (*read_lock)(rwlock_t *this);

	/**
	 * Acquire the write lock.
	 */
	void (*write_lock)(rwlock_t *this);

	/**
	 * Try to acquire the write lock.
	 *
	 * Never blocks, but returns FALSE if the lock was already occupied.
	 *
	 * @return		TRUE if lock acquired
	 */
	bool (*try_write_lock)(rwlock_t *this);

	/**
	 * Release any acquired lock.
	 */
	void (*unlock)(rwlock_t *this);

	/**
	 * Destroy the read-write lock.
	 */
	void (*destroy)(rwlock_t *this);
};

/**
 * Create a read-write lock instance.
 *
 * @param type		type of rwlock to create
 * @return			unlocked rwlock instance
 */
rwlock_t *rwlock_create(rwlock_type_t type);

#endif /** THREADING_RWLOCK_H_ @} */

