/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "thread.h"

#include <utils/debug.h>
#include <threading/mutex.h>
#include <threading/condvar.h>

typedef struct private_mutex_t private_mutex_t;
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
	 * wrapped critical section
	 */
	CRITICAL_SECTION cs;

	/**
	 * Recursive lock count
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
	 * wrapped condition variable
	 */
	CONDITION_VARIABLE cv;
};


METHOD(mutex_t, lock, void,
	private_mutex_t *this)
{
	EnterCriticalSection(&this->cs);
	this->times++;
}

METHOD(mutex_t, unlock, void,
	private_mutex_t *this)
{
	this->times--;
	LeaveCriticalSection(&this->cs);
}

METHOD(mutex_t, mutex_destroy, void,
	private_mutex_t *this)
{
	DeleteCriticalSection(&this->cs);
	free(this);
}

/*
 * see header file
 */
mutex_t *mutex_create(mutex_type_t type)
{
	private_mutex_t *this;

	INIT(this,
		.public = {
			.lock = _lock,
			.unlock = _unlock,
			.destroy = _mutex_destroy,
		},
	);

	/* CriticalSections are recursive, we use it for all mutex types. */
	InitializeCriticalSection(&this->cs);

	return &this->public;
}

METHOD(condvar_t, timed_wait, bool,
	private_condvar_t *this, mutex_t *pubmutex, u_int timeout)
{
	private_mutex_t *mutex = (private_mutex_t*)pubmutex;
	u_int times;
	bool ret;

	thread_set_active_condvar(&this->cv);

	/* while a CriticalSection is recursive, waiting in a condvar releases
	 * only one mutex. So release (and reaquire) all locks except the last. */
	times = mutex->times;
	while (mutex->times-- > 1)
	{
		LeaveCriticalSection(&mutex->cs);
	}

	ret = SleepConditionVariableCS(&this->cv, &mutex->cs, timeout);

	while (++mutex->times < times)
	{
		EnterCriticalSection(&mutex->cs);
	}

	thread_set_active_condvar(NULL);

	return ret == 0;
}

METHOD(condvar_t, wait_, void,
	private_condvar_t *this, mutex_t *mutex)
{
	timed_wait(this, mutex, INFINITE);
}

METHOD(condvar_t, timed_wait_abs, bool,
	private_condvar_t *this, mutex_t *mutex, timeval_t tv)
{
	DWORD timeout;
	timeval_t now, diff;

	time_monotonic(&now);
	if (timercmp(&now, &tv, >))
	{
		return TRUE;
	}
	timersub(&tv, &now, &diff);
	timeout = diff.tv_sec * 1000 + diff.tv_usec / 1000;

	return timed_wait(this, mutex, timeout);
}

METHOD(condvar_t, signal_, void,
	private_condvar_t *this)
{
	WakeConditionVariable(&this->cv);
}

METHOD(condvar_t, broadcast, void,
	private_condvar_t *this)
{
	WakeAllConditionVariable(&this->cv);
}

METHOD(condvar_t, condvar_destroy, void,
	private_condvar_t *this)
{
	free(this);
}

/*
 * see header file
 */
condvar_t *condvar_create(condvar_type_t type)
{
	private_condvar_t *this;

	INIT(this,
		.public = {
			.wait = _wait_,
			.timed_wait = _timed_wait,
			.timed_wait_abs = _timed_wait_abs,
			.signal = _signal_,
			.broadcast = _broadcast,
			.destroy = _condvar_destroy,
		}
	);

	InitializeConditionVariable(&this->cv);

	return &this->public;
}
