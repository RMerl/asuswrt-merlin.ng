/*
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

#include "fetcher_manager.h"

#include <utils/debug.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>

typedef struct private_fetcher_manager_t private_fetcher_manager_t;

/**
 * private data of fetcher_manager
 */
struct private_fetcher_manager_t {

	/**
	 * public functions
	 */
	fetcher_manager_t public;

	/**
	 * list of registered fetchers, as entry_t
	 */
	linked_list_t *fetchers;

	/**
	 * read write lock to list
	 */
	rwlock_t *lock;
};

typedef struct {
	/** associated fetcher construction function */
	fetcher_constructor_t create;
	/** URL this fetcher support */
	char *url;
} entry_t;

/**
 * destroy an entry_t
 */
static void entry_destroy(entry_t *entry)
{
	free(entry->url);
	free(entry);
}

METHOD(fetcher_manager_t, fetch, status_t,
	private_fetcher_manager_t *this, char *url, void *userdata, ...)
{
	enumerator_t *enumerator;
	status_t status = NOT_SUPPORTED;
	entry_t *entry;
	bool capable = FALSE;

	this->lock->read_lock(this->lock);
	enumerator = this->fetchers->create_enumerator(this->fetchers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		fetcher_option_t opt;
		fetcher_t *fetcher;
		bool good = TRUE;
		host_t *host;
		va_list args;

		/* check URL support of fetcher */
		if (strncasecmp(entry->url, url, strlen(entry->url)))
		{
			continue;
		}
		/* create fetcher instance and set options */
		fetcher = entry->create();
		if (!fetcher)
		{
			continue;
		}
		va_start(args, userdata);
		while (good)
		{
			opt = va_arg(args, int);
			switch (opt)
			{
				case FETCH_REQUEST_DATA:
					good = fetcher->set_option(fetcher, opt,
											va_arg(args, chunk_t));
					continue;
				case FETCH_REQUEST_TYPE:
				case FETCH_REQUEST_HEADER:
					good = fetcher->set_option(fetcher, opt,
											va_arg(args, char*));
					continue;
				case FETCH_HTTP_VERSION_1_0:
					good = fetcher->set_option(fetcher, opt);
					continue;
				case FETCH_TIMEOUT:
					good = fetcher->set_option(fetcher, opt,
											va_arg(args, u_int));
					continue;
				case FETCH_CALLBACK:
					good = fetcher->set_option(fetcher, opt,
											va_arg(args, fetcher_callback_t));
					continue;
				case FETCH_RESPONSE_CODE:
					good = fetcher->set_option(fetcher, opt,
											va_arg(args, u_int*));
					continue;
				case FETCH_SOURCEIP:
					host = va_arg(args, host_t*);
					if (host && !host->is_anyaddr(host))
					{
						good = fetcher->set_option(fetcher, opt, host);
					}
					continue;
				case FETCH_END:
					break;
			}
			break;
		}
		va_end(args);
		if (!good)
		{	/* fetcher does not support supplied options, try another */
			fetcher->destroy(fetcher);
			continue;
		}

		status = fetcher->fetch(fetcher, url, userdata);
		fetcher->destroy(fetcher);
		/* try another fetcher only if this one does not support that URL */
		if (status == NOT_SUPPORTED)
		{
			continue;
		}
		capable = TRUE;
		break;
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	if (!capable)
	{
		DBG1(DBG_LIB, "unable to fetch from %s, no capable fetcher found", url);
	}
	return status;
}

METHOD(fetcher_manager_t, add_fetcher, void,
	private_fetcher_manager_t *this, fetcher_constructor_t create, char *url)
{
	entry_t *entry;

	INIT(entry,
		.url = strdup(url),
		.create = create,
	);
	this->lock->write_lock(this->lock);
	this->fetchers->insert_last(this->fetchers, entry);
	this->lock->unlock(this->lock);
}

METHOD(fetcher_manager_t, remove_fetcher, void,
	private_fetcher_manager_t *this, fetcher_constructor_t create)
{
	enumerator_t *enumerator;
	entry_t *entry;

	this->lock->write_lock(this->lock);
	enumerator = this->fetchers->create_enumerator(this->fetchers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create == create)
		{
			this->fetchers->remove_at(this->fetchers, enumerator);
			entry_destroy(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(fetcher_manager_t, destroy, void,
	private_fetcher_manager_t *this)
{
	this->fetchers->destroy_function(this->fetchers, (void*)entry_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
fetcher_manager_t *fetcher_manager_create()
{
	private_fetcher_manager_t *this;

	INIT(this,
		.public = {
			.fetch = _fetch,
			.add_fetcher = _add_fetcher,
			.remove_fetcher = _remove_fetcher,
			.destroy = _destroy,
		},
		.fetchers = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
