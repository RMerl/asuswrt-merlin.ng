/*
 * Copyright (C) 2009-2012 Tobias Brunner
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

#define _GNU_SOURCE
#include <pthread.h>

#include <library.h>

#include "thread_value.h"

typedef struct private_thread_value_t private_thread_value_t;

struct private_thread_value_t {
	/**
	 * Public interface.
	 */
	thread_value_t public;

	/**
	 * Key to access thread-specific values.
	 */
	pthread_key_t key;

	/**
	 * Destructor to cleanup the value of the thread destroying this object
	 */
	thread_cleanup_t destructor;

};

METHOD(thread_value_t, set, void,
	private_thread_value_t *this, void *val)
{
	pthread_setspecific(this->key, val);
}

METHOD(thread_value_t, get, void*,
	private_thread_value_t *this)
{
	return pthread_getspecific(this->key);
}

METHOD(thread_value_t, destroy, void,
	private_thread_value_t *this)
{
	void *val;

	/* the destructor is not called automatically for the thread calling
	 * pthread_key_delete() */
	if (this->destructor)
	{
		val = pthread_getspecific(this->key);
		if (val)
		{
			this->destructor(val);
		}
	}
	pthread_key_delete(this->key);
	free(this);
}

/**
 * Described in header.
 */
thread_value_t *thread_value_create(thread_cleanup_t destructor)
{
	private_thread_value_t *this;

	INIT(this,
		.public = {
			.set = _set,
			.get = _get,
			.destroy = _destroy,
		},
		.destructor = destructor,
	);

	pthread_key_create(&this->key, destructor);
	return &this->public;
}

