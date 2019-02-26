/*
 * Copyright (C) 2008-2009 Tobias Brunner
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
 * @defgroup mutex mutex
 * @{ @ingroup threading
 */

#ifndef THREADING_MUTEX_H_
#define THREADING_MUTEX_H_

typedef struct mutex_t mutex_t;
typedef enum mutex_type_t mutex_type_t;

/**
 * Type of mutex.
 */
enum mutex_type_t {
	/** default mutex */
	MUTEX_TYPE_DEFAULT	= 0,
	/** allow recursive locking of the mutex */
	MUTEX_TYPE_RECURSIVE	= 1,
};

/**
 * Mutex wrapper implements simple, portable and advanced mutex functions.
 */
struct mutex_t {

	/**
	 * Acquire the lock to the mutex.
	 */
	void (*lock)(mutex_t *this);

	/**
	 * Release the lock on the mutex.
	 */
	void (*unlock)(mutex_t *this);

	/**
	 * Destroy a mutex instance.
	 */
	void (*destroy)(mutex_t *this);
};

/**
 * Create a mutex instance.
 *
 * @param type		type of mutex to create
 * @return			unlocked mutex instance
 */
mutex_t *mutex_create(mutex_type_t type);

#endif /** THREADING_MUTEX_H_ @} */

