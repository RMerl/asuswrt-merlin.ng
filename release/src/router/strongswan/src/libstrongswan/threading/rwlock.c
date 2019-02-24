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

#include <library.h>
#include <utils/debug.h>

#include "rwlock.h"
#include "rwlock_condvar.h"
#include "thread.h"
#include "condvar.h"
#include "mutex.h"
#include "lock_profiler.h"

#ifdef __APPLE__
/* while pthread_rwlock_rdlock(3) says that it supports multiple read locks,
 * this does not seem to be true. After releasing a recursive rdlock,
 * a subsequent wrlock fails... */
# undef HAVE_PTHREAD_RWLOCK_INIT
#endif

typedef struct private_rwlock_t private_rwlock_t;
typedef struct private_rwlock_condvar_t private_rwlock_condvar_t;

/**
 * private data of rwlock
 */
struct private_rwlock_t {

	/**
	 * public functions
	 */
	rwlock_t public;

#ifdef HAVE_PTHREAD_RWLOCK_INIT

	/**
	 * wrapped pthread rwlock
	 */
	pthread_rwlock_t rwlock;

#else

	/**
	 * mutex to emulate a native rwlock
	 */
	mutex_t *mutex;

	/**
	 * condvar to handle writers
	 */
	condvar_t *writers;

	/**
	 * condvar to handle readers
	 */
	condvar_t *readers;

	/**
	 * number of waiting writers
	 */
	u_int waiting_writers;

	/**
	 * number of readers holding the lock
	 */
	u_int reader_count;

	/**
	 * TRUE, if a writer is holding the lock currently
	 */
	bool writer;

#endif /* HAVE_PTHREAD_RWLOCK_INIT */

	/**
	 * profiling info, if enabled
	 */
	lock_profile_t profile;
};

/**
 * private data of condvar
 */
struct private_rwlock_condvar_t {

	/**
	 * public interface
	 */
	rwlock_condvar_t public;

	/**
	 * mutex used to implement rwlock condvar
	 */
	mutex_t *mutex;

	/**
	 * regular condvar to implement rwlock condvar
	 */
	condvar_t *condvar;
};


#ifdef HAVE_PTHREAD_RWLOCK_INIT

METHOD(rwlock_t, read_lock, void,
	private_rwlock_t *this)
{
	int err;

	profiler_start(&this->profile);
	err = pthread_rwlock_rdlock(&this->rwlock);
	if (err != 0)
	{
		DBG1(DBG_LIB, "!!! RWLOCK READ LOCK ERROR: %s !!!", strerror(err));
	}
	profiler_end(&this->profile);
}

METHOD(rwlock_t, write_lock, void,
	private_rwlock_t *this)
{
	int err;

	profiler_start(&this->profile);
	err = pthread_rwlock_wrlock(&this->rwlock);
	if (err != 0)
	{
		DBG1(DBG_LIB, "!!! RWLOCK WRITE LOCK ERROR: %s !!!", strerror(err));
	}
	profiler_end(&this->profile);
}

METHOD(rwlock_t, try_write_lock, bool,
	private_rwlock_t *this)
{
	return pthread_rwlock_trywrlock(&this->rwlock) == 0;
}

METHOD(rwlock_t, unlock, void,
	private_rwlock_t *this)
{
	int err;

	err = pthread_rwlock_unlock(&this->rwlock);
	if (err != 0)
	{
		DBG1(DBG_LIB, "!!! RWLOCK UNLOCK ERROR: %s !!!", strerror(err));
	}
}

METHOD(rwlock_t, destroy, void,
	private_rwlock_t *this)
{
	pthread_rwlock_destroy(&this->rwlock);
	profiler_cleanup(&this->profile);
	free(this);
}

/*
 * see header file
 */
rwlock_t *rwlock_create(rwlock_type_t type)
{
	switch (type)
	{
		case RWLOCK_TYPE_DEFAULT:
		default:
		{
			private_rwlock_t *this;

			INIT(this,
				.public = {
					.read_lock = _read_lock,
					.write_lock = _write_lock,
					.try_write_lock = _try_write_lock,
					.unlock = _unlock,
					.destroy = _destroy,
				}
			);

			pthread_rwlock_init(&this->rwlock, NULL);
			profiler_init(&this->profile);

			return &this->public;
		}
	}
}

#else /* HAVE_PTHREAD_RWLOCK_INIT */

/**
 * This implementation of the rwlock_t interface uses mutex_t and condvar_t
 * primitives, if the pthread_rwlock_* group of functions is not available or
 * don't allow recursive locking for readers.
 *
 * The following constraints are enforced:
 *   - Multiple readers can hold the lock at the same time.
 *   - Only a single writer can hold the lock at any given time.
 *   - A writer must block until all readers have released the lock before
 *     obtaining the lock exclusively.
 *   - Readers that don't hold any read lock and arrive while a writer is
 *     waiting to acquire the lock will block until after the writer has
 *     obtained and released the lock.
 * These constraints allow for read sharing, prevent write sharing, prevent
 * read-write sharing and (largely) prevent starvation of writers by a steady
 * stream of incoming readers.  Reader starvation is not prevented (this could
 * happen if there are more writers than readers).
 *
 * The implementation supports recursive locking of the read lock but not of
 * the write lock.  Readers must not acquire the lock exclusively at the same
 * time and vice-versa (this is not checked or enforced so behave yourself to
 * prevent deadlocks).
 *
 * Since writers are preferred a thread currently holding the read lock that
 * tries to acquire the read lock recursively while a writer is waiting would
 * result in a deadlock.  In order to avoid having to use a thread-specific
 * value for each rwlock_t (or a list of threads) to keep track if a thread
 * already acquired the read lock we use a single thread-specific value for all
 * rwlock_t objects that keeps track of how many read locks a thread currently
 * holds.  Preferring readers that already hold ANY read locks prevents this
 * deadlock while it still largely avoids writer starvation (for locks that can
 * only be acquired while holding another read lock this will obviously not
 * work).
 */

/**
 * Keep track of how many read locks a thread holds.
 */
static pthread_key_t is_reader;

/**
 * Only initialize the read lock counter once.
 */
static pthread_once_t is_reader_initialized = PTHREAD_ONCE_INIT;

/**
 * Initialize the read lock counter.
 */
static void initialize_is_reader()
{
	pthread_key_create(&is_reader, NULL);
}

METHOD(rwlock_t, read_lock, void,
	private_rwlock_t *this)
{
	uintptr_t reading;
	bool old;

	reading = (uintptr_t)pthread_getspecific(is_reader);
	profiler_start(&this->profile);
	this->mutex->lock(this->mutex);
	if (!this->writer && reading > 0)
	{
		/* directly allow threads that hold ANY read locks, to avoid a deadlock
		 * caused by preferring writers in the loop below */
	}
	else
	{
		old = thread_cancelability(FALSE);
		while (this->writer || this->waiting_writers)
		{
			this->readers->wait(this->readers, this->mutex);
		}
		thread_cancelability(old);
	}
	this->reader_count++;
	profiler_end(&this->profile);
	this->mutex->unlock(this->mutex);
	pthread_setspecific(is_reader, (void*)(reading + 1));
}

METHOD(rwlock_t, write_lock, void,
	private_rwlock_t *this)
{
	bool old;

	profiler_start(&this->profile);
	this->mutex->lock(this->mutex);
	this->waiting_writers++;
	old = thread_cancelability(FALSE);
	while (this->writer || this->reader_count)
	{
		this->writers->wait(this->writers, this->mutex);
	}
	thread_cancelability(old);
	this->waiting_writers--;
	this->writer = TRUE;
	profiler_end(&this->profile);
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_t, try_write_lock, bool,
	private_rwlock_t *this)
{
	bool res = FALSE;
	this->mutex->lock(this->mutex);
	if (!this->writer && !this->reader_count)
	{
		res = this->writer = TRUE;
	}
	this->mutex->unlock(this->mutex);
	return res;
}

METHOD(rwlock_t, unlock, void,
	private_rwlock_t *this)
{
	this->mutex->lock(this->mutex);
	if (this->writer)
	{
		this->writer = FALSE;
	}
	else
	{
		uintptr_t reading;

		this->reader_count--;
		reading = (uintptr_t)pthread_getspecific(is_reader);
		pthread_setspecific(is_reader, (void*)(reading - 1));
	}
	if (!this->reader_count)
	{
		if (this->waiting_writers)
		{
			this->writers->signal(this->writers);
		}
		else
		{
			this->readers->broadcast(this->readers);
		}
	}
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_t, destroy, void,
	private_rwlock_t *this)
{
	this->mutex->destroy(this->mutex);
	this->writers->destroy(this->writers);
	this->readers->destroy(this->readers);
	profiler_cleanup(&this->profile);
	free(this);
}

/*
 * see header file
 */
rwlock_t *rwlock_create(rwlock_type_t type)
{
	pthread_once(&is_reader_initialized,  initialize_is_reader);

	switch (type)
	{
		case RWLOCK_TYPE_DEFAULT:
		default:
		{
			private_rwlock_t *this;

			INIT(this,
				.public = {
					.read_lock = _read_lock,
					.write_lock = _write_lock,
					.try_write_lock = _try_write_lock,
					.unlock = _unlock,
					.destroy = _destroy,
				},
				.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
				.writers = condvar_create(CONDVAR_TYPE_DEFAULT),
				.readers = condvar_create(CONDVAR_TYPE_DEFAULT),
			);

			profiler_init(&this->profile);

			return &this->public;
		}
	}
}

#endif /* HAVE_PTHREAD_RWLOCK_INIT */


METHOD(rwlock_condvar_t, wait_, void,
	private_rwlock_condvar_t *this, rwlock_t *lock)
{
	/* at this point we have the write lock locked, to make signals more
	 * predictable we try to prevent other threads from signaling by acquiring
	 * the mutex while we still hold the write lock (this assumes they will
	 * hold the write lock themselves when signaling, which is not mandatory) */
	this->mutex->lock(this->mutex);
	/* unlock the rwlock and wait for a signal */
	lock->unlock(lock);
	/* if the calling thread enabled thread cancelability we want to replicate
	 * the behavior of the regular condvar, i.e. the lock will be held again
	 * before executing cleanup functions registered by the calling thread */
	thread_cleanup_push((thread_cleanup_t)lock->write_lock, lock);
	thread_cleanup_push((thread_cleanup_t)this->mutex->unlock, this->mutex);
	this->condvar->wait(this->condvar, this->mutex);
	/* we release the mutex to allow other threads into the condvar (might even
	 * be required so we can acquire the lock again below) */
	thread_cleanup_pop(TRUE);
	/* finally we reacquire the lock we held previously */
	thread_cleanup_pop(TRUE);
}

METHOD(rwlock_condvar_t, timed_wait_abs, bool,
	private_rwlock_condvar_t *this, rwlock_t *lock, timeval_t time)
{
	bool timed_out;

	/* see wait() above for details on what is going on here */
	this->mutex->lock(this->mutex);
	lock->unlock(lock);
	thread_cleanup_push((thread_cleanup_t)lock->write_lock, lock);
	thread_cleanup_push((thread_cleanup_t)this->mutex->unlock, this->mutex);
	timed_out = this->condvar->timed_wait_abs(this->condvar, this->mutex, time);
	thread_cleanup_pop(TRUE);
	thread_cleanup_pop(TRUE);
	return timed_out;
}

METHOD(rwlock_condvar_t, timed_wait, bool,
	private_rwlock_condvar_t *this, rwlock_t *lock, u_int timeout)
{
	timeval_t tv;
	u_int s, ms;

	time_monotonic(&tv);

	s = timeout / 1000;
	ms = timeout % 1000;

	tv.tv_sec += s;
	timeval_add_ms(&tv, ms);

	return timed_wait_abs(this, lock, tv);
}

METHOD(rwlock_condvar_t, signal_, void,
	private_rwlock_condvar_t *this)
{
	this->mutex->lock(this->mutex);
	this->condvar->signal(this->condvar);
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_condvar_t, broadcast, void,
	private_rwlock_condvar_t *this)
{
	this->mutex->lock(this->mutex);
	this->condvar->broadcast(this->condvar);
	this->mutex->unlock(this->mutex);
}

METHOD(rwlock_condvar_t, condvar_destroy, void,
	private_rwlock_condvar_t *this)
{
	this->condvar->destroy(this->condvar);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * see header file
 */
rwlock_condvar_t *rwlock_condvar_create()
{
	private_rwlock_condvar_t *this;

	INIT(this,
		.public = {
			.wait = _wait_,
			.timed_wait = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.signal = _signal_,
			.broadcast = _broadcast,
			.destroy = _condvar_destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
	);
	return &this->public;
}
