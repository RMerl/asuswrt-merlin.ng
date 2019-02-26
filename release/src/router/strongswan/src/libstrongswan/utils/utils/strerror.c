/*
 * Copyright (C) 2012-2014 Tobias Brunner
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

#include <stdlib.h>
#include <string.h>

#include <library.h>
#include <threading/thread_value.h>
#include <threading/spinlock.h>

#include "strerror.h"

/**
 * The size of the thread-specific error buffer
 */
#define STRERROR_BUF_LEN 256

/**
 * Thread specific strerror buffer, as char*
 */
static thread_value_t *strerror_buf;

#ifndef HAVE_STRERROR_R
/**
 * Lock to access strerror() safely
 */
static spinlock_t *strerror_lock;
#endif /* HAVE_STRERROR_R */

/**
 * Retrieve the error buffer assigned to the current thread (or create it)
 */
static inline char *get_strerror_buf()
{
	char *buf;
	bool old = FALSE;

	if (!strerror_buf)
	{
		return NULL;
	}

	buf = strerror_buf->get(strerror_buf);
	if (!buf)
	{
		if (lib->leak_detective)
		{
			old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
		}
		buf = malloc(STRERROR_BUF_LEN);
		strerror_buf->set(strerror_buf, buf);
		if (lib->leak_detective)
		{
			lib->leak_detective->set_state(lib->leak_detective, old);
		}
	}
	return buf;
}

/**
 * Use real strerror() below
 */
#undef strerror

/*
 * Described in header.
 */
const char *strerror_safe(int errnum)
{
	char *buf, *msg;

	buf = get_strerror_buf();
	if (!buf)
	{
		/* library not initialized? fallback */
		return strerror(errnum);
	}
#ifdef HAVE_STRERROR_R
# ifdef STRERROR_R_CHAR_P
	/* char* version which may or may not return the original buffer */
	msg = strerror_r(errnum, buf, STRERROR_BUF_LEN);
# else
	/* int version returns 0 on success */
	msg = strerror_r(errnum, buf, STRERROR_BUF_LEN) ? "Unknown error" : buf;
# endif
#else /* HAVE_STRERROR_R */
	/* use a lock to ensure calling strerror(3) is thread-safe */
	strerror_lock->lock(strerror_lock);
	msg = strncpy(buf, strerror(errnum), STRERROR_BUF_LEN);
	strerror_lock->unlock(strerror_lock);
	buf[STRERROR_BUF_LEN - 1] = '\0';
#endif /* HAVE_STRERROR_R */
	return msg;
}

/**
 * free() with disabled leak detective
 */
static void free_no_ld(void *buf)
{
	bool old = FALSE;

	if (lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}
	free(buf);
	if (lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}
}

/**
 * See header
 */
void strerror_init()
{
	strerror_buf = thread_value_create(free_no_ld);
#ifndef HAVE_STRERROR_R
	strerror_lock = spinlock_create();
#endif
}

/**
 * See header
 */
void strerror_deinit()
{
	strerror_buf->destroy(strerror_buf);
	strerror_buf = NULL;
#ifndef HAVE_STRERROR_R
	strerror_lock->destroy(strerror_lock);
#endif
}
