/*
 * Copyright (C) 2008-2012 Tobias Brunner
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

#define _GNU_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#include <library.h>
#include <utils/debug.h>

#include "thread.h"
#include "condvar.h"
#include "mutex.h"
#include "lock_profiler.h"

typedef struct private_mutex_t private_mutex_t;
typedef struct private_r_mutex_t private_r_mutex_t;
typedef struct private_condvar_t private_condvar_t;

/**
 * private data of mutex
 */
struct private_mutex_t {

	/**
	 * public functions
	 */
	mutex_t public;

	/**
	 * wrapped pthread mutex
	 */
	pthread_mutex_t mutex;

	/**
	 * is this a recursiv emutex, implementing private_r_mutex_t?
	 */
	bool recursive;

	/**
	 * profiling info, if enabled
	 */
	lock_profile_t profile;
};

/**
 * private data of mutex, extended by recursive locking information
 */
struct private_r_mutex_t {

	/**
	 * Extends private_mutex_t
	 */
	private_mutex_t generic;

	/**
	 * thread which currently owns mutex
	 */
	thread_t *thread;

	/**
	 * times the current thread locked the mutex
	 */
	u_int times;
};

/**
 * private data of condvar
 */
struct private_condvar_t {

	/**
	 * public functions
	 */
	condvar_t public;

	/**
	 * wrapped pthread condvar
	 */
	pthread_cond_t condvar;

};


METHOD(mutex_t, lock, void,
	private_mutex_t *this)
{
	int err;

	profiler_start(&this->profile);
	err = pthread_mutex_lock(&this->mutex);
	if (err)
	{
		DBG1(DBG_LIB, "!!! MUTEX LOCK ERROR: %s !!!", strerror(err));
	}
	profiler_end(&this->profile);
}

METHOD(mutex_t, unlock, void,
	private_mutex_t *this)
{
	int err;

	err = pthread_mutex_unlock(&this->mutex);
	if (err)
	{
		DBG1(DBG_LIB, "!!! MUTEX UNLOCK ERROR: %s !!!", strerror(err));
	}
}

METHOD(mutex_t, lock_r, void,
	private_r_mutex_t *this)
{
	thread_t *self = thread_current();

	if (cas_ptr(&this->thread, self, self))
	{
		this->times++;
	}
	else
	{
		lock(&this->generic);
		cas_ptr(&this->thread, NULL, self);
		this->times = 1;
	}
}

METHOD(mutex_t, unlock_r, void,
	private_r_mutex_t *this)
{
	if (--this->times == 0)
	{
		cas_ptr(&this->thread, thread_current(), NULL);
		unlock(&this->generic);
	}
}

METHOD(mutex_t, mutex_destroy, void,
	private_mutex_t *this)
{
	profiler_cleanup(&this->profile);
	pthread_mutex_destroy(&this->mutex);
	free(this);
}

METHOD(mutex_t, mutex_destroy_r, void,
	private_r_mutex_t *this)
{
	profiler_cleanup(&this->generic.profile);
	pthread_mutex_destroy(&this->generic.mutex);
	free(this);
}

/*
 * see header file
 */
mutex_t *mutex_create(mutex_type_t type)
{
	switch (type)
	{
		case MUTEX_TYPE_RECURSIVE:
		{
			private_r_mutex_t *this;

			INIT(this,
				.generic = {
					.public = {
						.lock = _lock_r,
						.unlock = _unlock_r,
						.destroy = _mutex_destroy_r,
					},
					.recursive = TRUE,
				},
			);

			pthread_mutex_init(&this->generic.mutex, NULL);
			profiler_init(&this->generic.profile);

			return &this->generic.public;
		}
		case MUTEX_TYPE_DEFAULT:
		default:
		{
			private_mutex_t *this;

			INIT(this,
				.public = {
					.lock = _lock,
					.unlock = _unlock,
					.destroy = _mutex_destroy,
				},
			);

			pthread_mutex_init(&this->mutex, NULL);
			profiler_init(&this->profile);

			return &this->public;
		}
	}
}


METHOD(condvar_t, wait_, void,
	private_condvar_t *this, private_mutex_t *mutex)
{
	if (mutex->recursive)
	{
		private_r_mutex_t* recursive = (private_r_mutex_t*)mutex;
		thread_t *self = thread_current();
		u_int times;

		/* keep track of the number of times this thread locked the mutex */
		times = recursive->times;
		/* mutex owner gets cleared during condvar wait */
		cas_ptr(&recursive->thread, self, NULL);
		pthread_cond_wait(&this->condvar, &mutex->mutex);
		cas_ptr(&recursive->thread, NULL, self);
		recursive->times = times;
	}
	else
	{
		pthread_cond_wait(&this->condvar, &mutex->mutex);
	}
}

/* use the monotonic clock based version of this function if available */
#ifdef HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC
#define pthread_cond_timedwait pthread_cond_timedwait_monotonic
#endif

METHOD(condvar_t, timed_wait_abs, bool,
	private_condvar_t *this, private_mutex_t *mutex, timeval_t time)
{
	struct timespec ts;
	bool timed_out;

	ts.tv_sec = time.tv_sec;
	ts.tv_nsec = time.tv_usec * 1000;

	if (mutex->recursive)
	{
		private_r_mutex_t* recursive = (private_r_mutex_t*)mutex;
		thread_t *self = thread_current();
		u_int times;

		times = recursive->times;
		cas_ptr(&recursive->thread, self, NULL);
		timed_out = pthread_cond_timedwait(&this->condvar, &mutex->mutex,
										   &ts) == ETIMEDOUT;
		cas_ptr(&recursive->thread, NULL, self);
		recursive->times = times;
	}
	else
	{
		timed_out = pthread_cond_timedwait(&this->condvar, &mutex->mutex,
										   &ts) == ETIMEDOUT;
	}
	return timed_out;
}

METHOD(condvar_t, timed_wait, bool,
	private_condvar_t *this, private_mutex_t *mutex, u_int timeout)
{
	timeval_t tv;
	u_int s, ms;

	time_monotonic(&tv);

	s = timeout / 1000;
	ms = timeout % 1000;

	tv.tv_sec += s;
	timeval_add_ms(&tv, ms);
	return timed_wait_abs(this, mutex, tv);
}

METHOD(condvar_t, signal_, void,
	private_condvar_t *this)
{
	pthread_cond_signal(&this->condvar);
}

METHOD(condvar_t, broadcast, void,
	private_condvar_t *this)
{
	pthread_cond_broadcast(&this->condvar);
}

METHOD(condvar_t, condvar_destroy, void,
	private_condvar_t *this)
{
	pthread_cond_destroy(&this->condvar);
	free(this);
}

/*
 * see header file
 */
condvar_t *condvar_create(condvar_type_t type)
{
	switch (type)
	{
		case CONDVAR_TYPE_DEFAULT:
		default:
		{
			private_condvar_t *this;

			INIT(this,
				.public = {
					.wait = (void*)_wait_,
					.timed_wait = (void*)_timed_wait,
					.timed_wait_abs = (void*)_timed_wait_abs,
					.signal = _signal_,
					.broadcast = _broadcast,
					.destroy = _condvar_destroy,
				}
			);

#ifdef HAVE_PTHREAD_CONDATTR_INIT
			{
				pthread_condattr_t condattr;
				pthread_condattr_init(&condattr);
#ifdef HAVE_CONDATTR_CLOCK_MONOTONIC
				pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
#endif
				pthread_cond_init(&this->condvar, &condattr);
				pthread_condattr_destroy(&condattr);
			}
#endif

			return &this->public;
		}
	}
}

