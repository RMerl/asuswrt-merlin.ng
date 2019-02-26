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

#include <library.h>
#include <threading/spinlock.h>

typedef struct private_spinlock_t private_spinlock_t;

/**
 * private data of spinlock
 */
struct private_spinlock_t {

	/**
	 * public functions
	 */
	spinlock_t public;

	/**
	 * wrapped critical section
	 */
	CRITICAL_SECTION cs;
};

METHOD(spinlock_t, lock, void,
	private_spinlock_t *this)
{
	EnterCriticalSection(&this->cs);
}

METHOD(spinlock_t, unlock, void,
	private_spinlock_t *this)
{
	LeaveCriticalSection(&this->cs);
}

METHOD(spinlock_t, destroy, void,
	private_spinlock_t *this)
{
	DeleteCriticalSection(&this->cs);
	free(this);
}

/*
 * see header file
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

	/* Usually the wait time in a spinlock should be short, so we could have
	 * a high spincount. But having a large/INFINITE spincount does not scale
	 * that well where a spinlock is not the perfect choice for a lock. We
	 * choose the spincount quite arbitrary, so we go to wait if it is not
	 * much more expensive than spinning. */
	InitializeCriticalSectionAndSpinCount(&this->cs, 256);

	return &this->public;
}
