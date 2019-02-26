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
#include <threading/rwlock.h>
#include <threading/rwlock_condvar.h>
#include <threading/thread_value.h>

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

	/**
	 * wrapped rwlock
	 */
	SRWLOCK srw;

	/**
	 * Thread specific shared lock count
	 */
	thread_value_t *shared;
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
	 * condition variable
	 */
	CONDITION_VARIABLE cv;
};

METHOD(rwlock_t, read_lock, void,
	private_rwlock_t *this)
{
	uintptr_t count;

	/* Recursive read locks are not supported. Use a thread specific
	 * recursiveness counter. */

	count = (uintptr_t)this->shared->get(this->shared);
	if (count == 0)
	{
		AcquireSRWLockShared(&this->srw);
	}
	this->shared->set(this->shared, (void*)(count + 1));
}

METHOD(rwlock_t, write_lock, void,
	private_rwlock_t *this)
{
	AcquireSRWLockExclusive(&this->srw);
}

METHOD(rwlock_t, try_write_lock, bool,
	private_rwlock_t *this)
{
	return TryAcquireSRWLockExclusive(&this->srw);
}

METHOD(rwlock_t, unlock, void,
	private_rwlock_t *this)
{
	uintptr_t count;

	count = (uintptr_t)this->shared->get(this->shared);
	switch (count)
	{
		case 0:
			ReleaseSRWLockExclusive(&this->srw);
			break;
		case 1:
			ReleaseSRWLockShared(&this->srw);
			/* fall */
		default:
			this->shared->set(this->shared, (void*)(count - 1));
			break;
	}
}

METHOD(rwlock_t, destroy, void,
	private_rwlock_t *this)
{
	this->shared->destroy(this->shared);
	free(this);
}

/*
 * see header file
 */
rwlock_t *rwlock_create(rwlock_type_t type)
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
		.shared = thread_value_create(NULL),
	);

	InitializeSRWLock(&this->srw);

	return &this->public;
}

METHOD(rwlock_condvar_t, timed_wait, bool,
	private_rwlock_condvar_t *this, rwlock_t *pubrwlock, u_int timeout)
{
	private_rwlock_t *rwlock = (private_rwlock_t*)pubrwlock;
	bool ret;

	thread_set_active_condvar(&this->cv);

	ret = SleepConditionVariableSRW(&this->cv, &rwlock->srw, timeout, 0);

	thread_set_active_condvar(NULL);

	return ret == 0;
}

METHOD(rwlock_condvar_t, wait_, void,
	private_rwlock_condvar_t *this, rwlock_t *lock)
{
	timed_wait(this, lock, INFINITE);
}

METHOD(rwlock_condvar_t, timed_wait_abs, bool,
	private_rwlock_condvar_t *this, rwlock_t *lock, timeval_t tv)
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

	return timed_wait(this, lock, timeout);
}

METHOD(rwlock_condvar_t, signal_, void,
	private_rwlock_condvar_t *this)
{
	WakeConditionVariable(&this->cv);
}

METHOD(rwlock_condvar_t, broadcast, void,
	private_rwlock_condvar_t *this)
{
	WakeAllConditionVariable(&this->cv);
}

METHOD(rwlock_condvar_t, condvar_destroy, void,
	private_rwlock_condvar_t *this)
{
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
	);

	InitializeConditionVariable(&this->cv);

	return &this->public;
}
