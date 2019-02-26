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

#include "stream_manager.h"

#include "stream_tcp.h"
#include "stream_service_tcp.h"
#ifndef WIN32
# include "stream_unix.h"
# include "stream_service_unix.h"
#endif
#ifdef USE_SYSTEMD
# include "stream_service_systemd.h"
#endif

#include <threading/rwlock.h>

typedef struct private_stream_manager_t private_stream_manager_t;

/**
 * Private data of an stream_manager_t object.
 */
struct private_stream_manager_t {

	/**
	 * Public stream_manager_t interface.
	 */
	stream_manager_t public;

	/**
	 * List of registered stream constructors, as stream_entry_t
	 */
	linked_list_t *streams;

	/**
	 * List of registered service constructors, as service_entry_t
	 */
	linked_list_t *services;

	/**
	 * Lock for all lists
	 */
	rwlock_t *lock;
};

/**
 * Registered stream backend
 */
typedef struct {
	/** URI prefix */
	char *prefix;
	/** constructor function */
	stream_constructor_t create;
} stream_entry_t;

/**
 * Registered service backend
 */
typedef struct {
	/** URI prefix */
	char *prefix;
	/** constructor function */
	stream_service_constructor_t create;
} service_entry_t;

METHOD(stream_manager_t, connect_, stream_t*,
	private_stream_manager_t *this, char *uri)
{
	enumerator_t *enumerator;
	stream_entry_t *entry;
	stream_t *stream = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->streams->create_enumerator(this->streams);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (strpfx(uri, entry->prefix))
		{
			stream = entry->create(uri);
			if (stream)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return stream;
}

METHOD(stream_manager_t, create_service, stream_service_t*,
	private_stream_manager_t *this, char *uri, int backlog)
{
	enumerator_t *enumerator;
	service_entry_t *entry;
	stream_service_t *service = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->services->create_enumerator(this->services);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (strpfx(uri, entry->prefix))
		{
			service = entry->create(uri, backlog);
			if (service)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return service;
}

METHOD(stream_manager_t, add_stream, void,
	private_stream_manager_t *this, char *prefix, stream_constructor_t create)
{
	stream_entry_t *entry;

	INIT(entry,
		.prefix = strdup(prefix),
		.create = create,
	);

	this->lock->write_lock(this->lock);
	this->streams->insert_last(this->streams, entry);
	this->lock->unlock(this->lock);
}

METHOD(stream_manager_t, remove_stream, void,
	private_stream_manager_t *this, stream_constructor_t create)
{
	enumerator_t *enumerator;
	stream_entry_t *entry;

	this->lock->write_lock(this->lock);
	enumerator = this->streams->create_enumerator(this->streams);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create == create)
		{
			this->streams->remove_at(this->streams, enumerator);
			free(entry->prefix);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(stream_manager_t, add_service, void,
	private_stream_manager_t *this, char *prefix,
	stream_service_constructor_t create)
{
	service_entry_t *entry;

	INIT(entry,
		.prefix = strdup(prefix),
		.create = create,
	);

	this->lock->write_lock(this->lock);
	this->services->insert_last(this->services, entry);
	this->lock->unlock(this->lock);
}

METHOD(stream_manager_t, remove_service, void,
	private_stream_manager_t *this, stream_service_constructor_t create)
{
	enumerator_t *enumerator;
	service_entry_t *entry;

	this->lock->write_lock(this->lock);
	enumerator = this->services->create_enumerator(this->services);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create == create)
		{
			this->services->remove_at(this->services, enumerator);
			free(entry->prefix);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(stream_manager_t, destroy, void,
	private_stream_manager_t *this)
{
	remove_stream(this, stream_create_tcp);
	remove_service(this, stream_service_create_tcp);
#ifndef WIN32
	remove_stream(this, stream_create_unix);
	remove_service(this, stream_service_create_unix);
#endif
#ifdef USE_SYSTEMD
	remove_service(this, stream_service_create_systemd);
#endif

	this->streams->destroy(this->streams);
	this->services->destroy(this->services);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
stream_manager_t *stream_manager_create()
{
	private_stream_manager_t *this;

	INIT(this,
		.public = {
			.connect = _connect_,
			.create_service = _create_service,
			.add_stream = _add_stream,
			.remove_stream = _remove_stream,
			.add_service = _add_service,
			.remove_service = _remove_service,
			.destroy = _destroy,
		},
		.streams = linked_list_create(),
		.services = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	add_stream(this, "tcp://", stream_create_tcp);
	add_service(this, "tcp://", stream_service_create_tcp);
#ifndef WIN32
	add_stream(this, "unix://", stream_create_unix);
	add_service(this, "unix://", stream_service_create_unix);
#endif
#ifdef USE_SYSTEMD
	add_service(this, "systemd://", stream_service_create_systemd);
#endif

	return &this->public;
}
