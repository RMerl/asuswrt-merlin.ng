/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "error_notify_socket.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

#include "error_notify_msg.h"

typedef struct private_error_notify_socket_t private_error_notify_socket_t;

/**
 * Private data of an error_notify_socket_t object.
 */
struct private_error_notify_socket_t {

	/**
	 * Public error_notify_socket_t interface.
	 */
	error_notify_socket_t public;

	/**
	 * Service accepting connections
	 */
	stream_service_t *service;

	/**
	 * List of connected clients, as stream_t
	 */
	linked_list_t *connected;

	/**
	 * Mutex to lock clients list
	 */
	mutex_t *mutex;
};

METHOD(error_notify_socket_t, has_listeners, bool,
	private_error_notify_socket_t *this)
{
	int count;

	this->mutex->lock(this->mutex);
	count = this->connected->get_count(this->connected);
	this->mutex->unlock(this->mutex);

	return count != 0;
}

METHOD(error_notify_socket_t, notify, void,
	private_error_notify_socket_t *this, error_notify_msg_t *msg)
{
	enumerator_t *enumerator;
	stream_t *stream;

	this->mutex->lock(this->mutex);
	enumerator = this->connected->create_enumerator(this->connected);
	while (enumerator->enumerate(enumerator, &stream))
	{
		if (!stream->write_all(stream, msg, sizeof(*msg)))
		{
			switch (errno)
			{
				case ECONNRESET:
				case EPIPE:
					/* disconnect, remove this listener */
					this->connected->remove_at(this->connected, enumerator);
					stream->destroy(stream);
					break;
				default:
					DBG1(DBG_CFG, "sending notify failed: %s", strerror(errno));
					break;
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

/**
 * Accept client connections
 */
static bool on_accept(private_error_notify_socket_t *this, stream_t *stream)
{
	this->mutex->lock(this->mutex);
	this->connected->insert_last(this->connected, stream);
	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(error_notify_socket_t, destroy, void,
	private_error_notify_socket_t *this)
{
	DESTROY_IF(this->service);
	this->connected->destroy_offset(this->connected, offsetof(stream_t, destroy));
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
error_notify_socket_t *error_notify_socket_create()
{
	private_error_notify_socket_t *this;
	char *uri;

	INIT(this,
		.public = {
			.notify = _notify,
			.has_listeners = _has_listeners,
			.destroy = _destroy,
		},
		.connected = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	uri = lib->settings->get_str(lib->settings,
				"%s.plugins.error-notify.socket", "unix://" ERROR_NOTIFY_SOCKET,
				lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 10);
	if (!this->service)
	{
		DBG1(DBG_CFG, "creating duplicheck socket failed");
		destroy(this);
		return NULL;
	}
	this->service->on_accept(this->service, (stream_service_cb_t)on_accept,
							 this, JOB_PRIO_CRITICAL, 1);

	return &this->public;
}
