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

#include <threading/thread_value.h>


typedef struct private_thread_value_t private_thread_value_t;

/**
 * Unified thread_value_t implementation
 */
struct private_thread_value_t {

	/**
	 * Public interface.
	 */
	thread_value_t public;

	union {

		/**
		 * Cleanup function
		 */
		thread_cleanup_t cleanup;

		/**
		 * Windows TLS index, if used
		 */
		DWORD index;
	};
};

/**
 * TLS entry
 */
typedef struct {
	/** TLS value */
	void *value;
	/** cleanup handler function */
	thread_cleanup_t cleanup;
} entry_t;

/**
 * See windows/thread.h
 */
void thread_tls_cleanup(void *value)
{
	entry_t *entry = (entry_t*)value;

	if (entry->cleanup)
	{
		entry->cleanup(entry->value);
	}
	free(entry);
}

METHOD(thread_value_t, tls_set, void,
	private_thread_value_t *this, void *val)
{
	entry_t *entry;

	if (val)
	{
		INIT(entry,
			.cleanup = this->cleanup,
			.value = val,
		);

		free(thread_tls_put(this, entry));
	}
	else
	{
		free(thread_tls_remove(this));
	}
}

METHOD(thread_value_t, tls_get, void*,
	private_thread_value_t *this)
{
	entry_t *entry;

	entry = thread_tls_get(this);
	if (entry)
	{
		return entry->value;
	}
	return NULL;
}

METHOD(thread_value_t, tls_destroy, void,
	private_thread_value_t *this)
{
	entry_t *entry;

	entry = thread_tls_remove(this);
	if (entry)
	{
		thread_tls_cleanup(entry);
	}
	free(this);
}

METHOD(thread_value_t, tls_set_index, void,
	private_thread_value_t *this, void *val)
{
	TlsSetValue(this->index, val);
}

METHOD(thread_value_t, tls_get_index, void*,
	private_thread_value_t *this)
{
	return TlsGetValue(this->index);
}

METHOD(thread_value_t, tls_destroy_index, void,
	private_thread_value_t *this)
{
	TlsFree(this->index);
	free(this);
}

/**
 * Described in header.
 */
thread_value_t *thread_value_create(thread_cleanup_t cleanup)
{
	private_thread_value_t *this;
	DWORD index = TLS_OUT_OF_INDEXES;

	/* we have two implementations: Windows Tls* functions do not support
	 * callbacks and has limited instances. We use it nonetheless if possible,
	 * especially as leak detective relies on TLS, but we have to mangle
	 * leak detective state for TLS storage. */

	if (!cleanup)
	{
		index = TlsAlloc();
	}

	if (index == TLS_OUT_OF_INDEXES)
	{
		INIT(this,
			.public = {
				.set = _tls_set,
				.get = _tls_get,
				.destroy = _tls_destroy,
			},
			.cleanup = cleanup,
		);
	}
	else
	{
		INIT(this,
			.public = {
				.set = _tls_set_index,
				.get = _tls_get_index,
				.destroy = _tls_destroy_index,
			},
			.index = index,
		);
	}

	return &this->public;
}
