/*
 * Copyright (C) 2011 Tobias Brunner
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

#include <library.h>

#if defined(HAVE_CLOCK_GETTIME) && \
	(defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
	 defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
/* if we use MONOTONIC times, we can't use POSIX_SEMAPHORES since they use
 * times based on CLOCK_REALTIME */
#undef HAVE_SEM_TIMEDWAIT
#endif /* HAVE_CLOCK_GETTIME && ... */

#ifdef HAVE_SEM_TIMEDWAIT
#include <semaphore.h>
#else /* !HAVE_SEM_TIMEDWAIT */
#include <threading/thread.h>
#include <threading/condvar.h>
#endif /* HAVE_SEM_TIMEDWAIT */

#include "semaphore.h"

typedef struct private_semaphore_t private_semaphore_t;

/**
 * private data of a semaphore
 */
struct private_semaphore_t {
	/**
	 * public interface
	 */
	semaphore_t public;

#ifdef HAVE_SEM_TIMEDWAIT
	/**
	 * wrapped POSIX semaphore object
	 */
	sem_t sem;
#else /* !HAVE_SEM_TIMEDWAIT */

	/**
	 * Mutex to lock count variable
	 */
	mutex_t *mutex;

	/**
	 * Condvar to signal count increase
	 */
	condvar_t *cond;

	/**
	 * Semaphore count value
	 */
	u_int count;
#endif /* HAVE_SEM_TIMEDWAIT */
};

METHOD(semaphore_t, wait_, void,
	private_semaphore_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_wait(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	thread_cleanup_push((void*)this->mutex->unlock, this->mutex);
	while (this->count == 0)
	{
		this->cond->wait(this->cond, this->mutex);
	}
	this->count--;
	thread_cleanup_pop(TRUE);
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(semaphore_t, timed_wait_abs, bool,
	private_semaphore_t *this, timeval_t tv)
{
#ifdef HAVE_SEM_TIMEDWAIT
	timespec_t ts;

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000;

	/* there are errors other than ETIMEDOUT possible, but we consider them
	 * all as timeout */
	return sem_timedwait(&this->sem, &ts) == -1;
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	thread_cleanup_push((void*)this->mutex->unlock, this->mutex);
	while (this->count == 0)
	{
		if (this->cond->timed_wait_abs(this->cond, this->mutex, tv))
		{
			thread_cleanup_pop(TRUE);
			return TRUE;
		}
	}
	this->count--;
	thread_cleanup_pop(TRUE);
	return FALSE;
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(semaphore_t, timed_wait, bool,
	private_semaphore_t *this, u_int timeout)
{
	timeval_t tv, add;

	add.tv_sec = timeout / 1000;
	add.tv_usec = (timeout % 1000) * 1000;

	time_monotonic(&tv);
	timeradd(&tv, &add, &tv);

	return timed_wait_abs(this, tv);
}

METHOD(semaphore_t, post, void,
	private_semaphore_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_post(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex->lock(this->mutex);
	this->count++;
	this->mutex->unlock(this->mutex);
	this->cond->signal(this->cond);
#endif /* HAVE_SEM_TIMEDWAIT */
}

METHOD(semaphore_t, destroy, void,
	private_semaphore_t *this)
{
#ifdef HAVE_SEM_TIMEDWAIT
	sem_destroy(&this->sem);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->cond->destroy(this->cond);
	this->mutex->destroy(this->mutex);
#endif /* HAVE_SEM_TIMEDWAIT */
	free(this);
}

/*
 * Described in header
 */
semaphore_t *semaphore_create(u_int value)
{
	private_semaphore_t *this;

	INIT(this,
		.public = {
			.wait = _wait_,
			.timed_wait = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.post = _post,
			.destroy = _destroy,
		},
	);

#ifdef HAVE_SEM_TIMEDWAIT
	sem_init(&this->sem, 0, value);
#else /* !HAVE_SEM_TIMEDWAIT */
	this->mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	this->cond = condvar_create(CONDVAR_TYPE_DEFAULT);
	this->count = value;
#endif /* HAVE_SEM_TIMEDWAIT */

	return &this->public;
}
