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
#include <threading/semaphore.h>

typedef struct private_semaphore_t private_semaphore_t;

/**
 * private data of a semaphore
 */
struct private_semaphore_t {
	/**
	 * public interface
	 */
	semaphore_t public;

	/**
	 * Handle to semaphore
	 */
	HANDLE handle;
};

METHOD(semaphore_t, timed_wait, bool,
	private_semaphore_t *this, u_int timeout)
{
	/* use alertable wait to allow cancellation */
	return WaitForSingleObjectEx(this->handle, timeout, TRUE) == WAIT_TIMEOUT;
}

METHOD(semaphore_t, timed_wait_abs, bool,
	private_semaphore_t *this, timeval_t tv)
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

	return timed_wait(this, timeout);
}

METHOD(semaphore_t, wait_, void,
	private_semaphore_t *this)
{
	timed_wait(this, INFINITE);
}

METHOD(semaphore_t, post, void,
	private_semaphore_t *this)
{
	ReleaseSemaphore(this->handle, 1, NULL);
}

METHOD(semaphore_t, destroy, void,
	private_semaphore_t *this)
{
	CloseHandle(this->handle);
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
		/* our API does not have an upper limit, but Windows requires one.
		 * 0xFFFFFFF (268435455) is the highest value for which Windows does
		 * not return ERROR_INVALID_PARAMETER, and should be sufficient. */
		.handle = CreateSemaphore(NULL, value, 0xFFFFFFF, NULL),
	);

	return &this->public;
}
