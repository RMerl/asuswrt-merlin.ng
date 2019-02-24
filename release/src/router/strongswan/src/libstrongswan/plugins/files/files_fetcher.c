/*
 * Copyright (C) 2015 Tobias Brunner
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

#include <errno.h>

#include <library.h>
#include <utils/debug.h>

#include "files_fetcher.h"

typedef struct private_files_fetcher_t private_files_fetcher_t;

/**
 * private data of a files_fetcher_t object.
 */
struct private_files_fetcher_t {

	/**
	 * Public data
	 */
	files_fetcher_t public;

	/**
	 * Callback function
	 */
	fetcher_callback_t cb;
};

METHOD(fetcher_t, fetch, status_t,
	private_files_fetcher_t *this, char *uri, void *userdata)
{
	chunk_t *data;
	status_t status = FAILED;

	if (this->cb == fetcher_default_callback)
	{
		*(chunk_t*)userdata = chunk_empty;
	}
	if (!strpfx(uri, "file://"))
	{
		return NOT_SUPPORTED;
	}
	uri = uri + strlen("file://");
	data = chunk_map(uri, FALSE);
	if (!data)
	{
		DBG1(DBG_LIB, "  opening '%s' failed: %s", uri, strerror(errno));
		return FAILED;
	}
	if (this->cb(userdata, *data))
	{
		status = SUCCESS;
	}
	chunk_unmap(data);
	return status;
}

METHOD(fetcher_t, set_option, bool,
	private_files_fetcher_t *this, fetcher_option_t option, ...)
{
	bool supported = TRUE;
	va_list args;

	va_start(args, option);
	switch (option)
	{
		case FETCH_CALLBACK:
		{
			this->cb = va_arg(args, fetcher_callback_t);
			break;
		}
		default:
			supported = FALSE;
			break;
	}
	va_end(args);
	return supported;
}

METHOD(fetcher_t, destroy, void,
	private_files_fetcher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
files_fetcher_t *files_fetcher_create()
{
	private_files_fetcher_t *this;

	INIT(this,
		.public = {
			.interface = {
				.fetch = _fetch,
				.set_option = _set_option,
				.destroy = _destroy,
			},
		},
		.cb = fetcher_default_callback,
	);

	return &this->public;
}
