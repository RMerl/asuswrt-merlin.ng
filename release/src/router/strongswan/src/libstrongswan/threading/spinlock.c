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

#include <pthread.h>

#include <library.h>
#include <utils/debug.h>

#include "spinlock.h"
#include "mutex.h"
#include "lock_profiler.h"

typedef struct private_spinlock_t private_spinlock_t;

/**
 * private data
 */
struct private_spinlock_t {

	/**
	 * public functions
	 */
	spinlock_t public;

#ifdef HAVE_PTHREAD_SPIN_INIT

	/**
	 * wrapped pthread spin lock
	 */
	pthread_spinlock_t spinlock;

	/**
	 * profiling info, if enabled (the mutex below does profile itself)
	 */
	lock_profile_t profile;

#else /* HAVE_PTHREAD_SPIN_INIT */

	/**
	 * use a mutex if spin locks are not available
	 */
	mutex_t *mutex;

#endif /* HAVE_PTHREAD_SPIN_INIT */
};

METHOD(spinlock_t, lock, void,
	private_spinlock_t *this)
{
#ifdef HAVE_PTHREAD_SPIN_INIT
	int err;

	profiler_start(&this->profile);
	err = pthread_spin_lock(&this->spinlock);
	if (err)
	{
		DBG1(DBG_LIB, "!!! SPIN LOCK LOCK ERROR: %s !!!", strerror(err));
	}
	profiler_end(&this->profile);
#else
	this->mutex->lock(this->mutex);
#endif
}

METHOD(spinlock_t, unlock, void,
	private_spinlock_t *this)
{
#ifdef HAVE_PTHREAD_SPIN_INIT
	int err;

	err = pthread_spin_unlock(&this->spinlock);
	if (err)
	{
		DBG1(DBG_LIB, "!!! SPIN LOCK UNLOCK ERROR: %s !!!", strerror(err));
	}
#else
	this->mutex->unlock(this->mutex);
#endif
}

METHOD(spinlock_t, destroy, void,
	private_spinlock_t *this)
{
#ifdef HAVE_PTHREAD_SPIN_INIT
	profiler_cleanup(&this->profile);
	pthread_spin_destroy(&this->spinlock);
#else
	this->mutex->destroy(this->mutex);
#endif
	free(this);
}

/*
 * Described in header
 */
spinlock_t *spinlock_create()
{
	private_spinlock_t *this;

	INIT(this,
		.public = {
			.lock = _lock,
			.unlock = _unlock,
			.destroy = _destroy,
		},
	);

#ifdef HAVE_PTHREAD_SPIN_INIT
	pthread_spin_init(&this->spinlock, PTHREAD_PROCESS_PRIVATE);
	profiler_init(&this->profile);
#else
	this->mutex = mutex_create(MUTEX_TYPE_DEFAULT);
#endif

	return &this->public;
}
