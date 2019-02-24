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

#include "lookip_socket.h"

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

#include "lookip_msg.h"

typedef struct private_lookip_socket_t private_lookip_socket_t;

/**
 * Private data of an lookip_socket_t object.
 */
struct private_lookip_socket_t {

	/**
	 * Public lookip_socket_t interface.
	 */
	lookip_socket_t public;

	/**
	 * lookip
	 */
	lookip_listener_t *listener;

	/**
	 * stream service accepting connections
	 */
	stream_service_t *service;

	/**
	 * List of connected clients, as entry_t
	 */
	linked_list_t *connected;

	/**
	 * Mutex to lock clients list
	 */
	mutex_t *mutex;
};

/**
 * List entry for a connected stream
 */
typedef struct {
	/* stream to write to */
	stream_t *stream;
	/* registered for up events? */
	bool up;
	/* registered for down events? */
	bool down;
	/** backref to this for unregistration */
	private_lookip_socket_t *this;
} entry_t;

/**
 * Clean up a connection entry
 */
static void entry_destroy(entry_t *entry)
{
	entry->stream->destroy(entry->stream);
	free(entry);
}

/**
 * Data for async disconnect job
 */
typedef struct {
	/** socket ref */
	private_lookip_socket_t *this;
	/** stream to disconnect */
	stream_t *stream;
} disconnect_data_t;

/**
 * Disconnect a stream asynchronously, remove connection entry
 */
static job_requeue_t disconnect_async(disconnect_data_t *data)
{
	private_lookip_socket_t *this = data->this;
	enumerator_t *enumerator;
	entry_t *entry;

	this->mutex->lock(this->mutex);
	enumerator = this->connected->create_enumerator(this->connected);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->stream == data->stream)
		{
			this->connected->remove_at(this->connected, enumerator);
			if (entry->up || entry->down)
			{
				this->listener->remove_listener(this->listener, entry);
			}
			entry_destroy(entry);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
	return JOB_REQUEUE_NONE;
}

/**
 * Queue async disconnect job
 */
static void disconnect(private_lookip_socket_t *this, stream_t *stream)
{
	disconnect_data_t *data;

	INIT(data,
		.this = this,
		.stream = stream,
	);

	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create((void*)disconnect_async, data,
										free, NULL));
}

/**
 * Callback function for listener up/down events
 */
static bool event_cb(entry_t *entry, bool up, host_t *vip, host_t *other,
					 identification_t *id, char *name, u_int unique_id)
{
	lookip_response_t resp = {
		.unique_id = htonl(unique_id),
	};

	if (up)
	{
		if (!entry->up)
		{
			return TRUE;
		}
		resp.type = htonl(LOOKIP_NOTIFY_UP);
	}
	else
	{
		if (!entry->down)
		{
			return TRUE;
		}
		resp.type = htonl(LOOKIP_NOTIFY_DOWN);
	}

	snprintf(resp.vip, sizeof(resp.vip), "%H", vip);
	snprintf(resp.ip, sizeof(resp.ip), "%H", other);
	snprintf(resp.id, sizeof(resp.id), "%Y", id);
	snprintf(resp.name, sizeof(resp.name), "%s", name);

	if (entry->stream->write_all(entry->stream, &resp, sizeof(resp)))
	{
		return TRUE;
	}
	switch (errno)
	{
		case ECONNRESET:
		case EPIPE:
			/* client disconnected, adios */
			break;
		default:
			DBG1(DBG_CFG, "sending lookip event failed: %s", strerror(errno));
			break;
	}
	/* don't unregister, as we return FALSE */
	entry->up = entry->down = FALSE;
	disconnect(entry->this, entry->stream);
	return FALSE;
}

/**
 * Callback function for queries
 */
static bool query_cb(stream_t *stream, bool up, host_t *vip, host_t *other,
					 identification_t *id, char *name, u_int unique_id)
{
	lookip_response_t resp = {
		.type = htonl(LOOKIP_ENTRY),
		.unique_id = htonl(unique_id),
	};

	snprintf(resp.vip, sizeof(resp.vip), "%H", vip);
	snprintf(resp.ip, sizeof(resp.ip), "%H", other);
	snprintf(resp.id, sizeof(resp.id), "%Y", id);
	snprintf(resp.name, sizeof(resp.name), "%s", name);

	if (stream->write_all(stream, &resp, sizeof(resp)))
	{
		return TRUE;
	}
	switch (errno)
	{
		case ECONNRESET:
		case EPIPE:
			/* client disconnected, adios */
			break;
		default:
			DBG1(DBG_CFG, "sending lookip response failed: %s", strerror(errno));
			break;
	}
	return FALSE;
}

/**
 * Perform a lookup
 */
static void query(private_lookip_socket_t *this, stream_t *stream,
				  lookip_request_t *req)
{

	host_t *vip = NULL;
	int matches = 0;

	if (req)
	{	/* lookup */
		req->vip[sizeof(req->vip) - 1] = 0;
		vip = host_create_from_string(req->vip, 0);
		if (vip)
		{
			matches = this->listener->lookup(this->listener, vip,
											 (void*)query_cb, stream);
			vip->destroy(vip);
		}
		if (matches == 0)
		{
			lookip_response_t resp = {
				.type = htonl(LOOKIP_NOT_FOUND),
			};

			snprintf(resp.vip, sizeof(resp.vip), "%s", req->vip);
			if (!stream->write_all(stream, &resp, sizeof(resp)))
			{
				DBG1(DBG_CFG, "sending lookip not-found failed: %s",
					 strerror(errno));
			}
		}
	}
	else
	{	/* dump */
		this->listener->lookup(this->listener, NULL,
							   (void*)query_cb, stream);
	}
}

/**
 * Subscribe to virtual IP events
 */
static void subscribe(private_lookip_socket_t *this, stream_t *stream, bool up)
{
	enumerator_t *enumerator;
	entry_t *entry;

	this->mutex->lock(this->mutex);
	enumerator = this->connected->create_enumerator(this->connected);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->stream == stream)
		{
			if (!entry->up && !entry->down)
			{	/* newly registered */
				this->listener->add_listener(this->listener,
											 (void*)event_cb, entry);
			}
			if (up)
			{
				entry->up = TRUE;
			}
			else
			{
				entry->down = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

/**
 * Check if a client is subscribed for notifications
 */
static bool subscribed(private_lookip_socket_t *this, stream_t *stream)
{
	enumerator_t *enumerator;
	bool subscribed = FALSE;
	entry_t *entry;

	this->mutex->lock(this->mutex);
	enumerator = this->connected->create_enumerator(this->connected);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->stream == stream)
		{
			subscribed = entry->up || entry->down;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	return subscribed;
}

/**
 * Dispatch from a socket, on-read callback
 */
static bool on_read(private_lookip_socket_t *this, stream_t *stream)
{
	lookip_request_t req;

	if (stream->read_all(stream, &req, sizeof(req)))
	{
		switch (ntohl(req.type))
		{
			case LOOKIP_LOOKUP:
				query(this, stream, &req);
				return TRUE;
			case LOOKIP_DUMP:
				query(this, stream, NULL);
				return TRUE;
			case LOOKIP_REGISTER_UP:
				subscribe(this, stream, TRUE);
				return TRUE;
			case LOOKIP_REGISTER_DOWN:
				subscribe(this, stream, FALSE);
				return TRUE;
			case LOOKIP_END:
				break;
			default:
				DBG1(DBG_CFG, "received unknown lookip command");
				break;
		}
	}
	else
	{
		if (errno != ECONNRESET)
		{
			DBG1(DBG_CFG, "receiving lookip request failed: %s",
				 strerror(errno));
		}
		disconnect(this, stream);
		return FALSE;
	}
	if (subscribed(this, stream))
	{
		return TRUE;
	}
	disconnect(this, stream);
	return FALSE;
}

/**
 * Accept client connections, dispatch
 */
static bool on_accept(private_lookip_socket_t *this, stream_t *stream)
{
	entry_t *entry;

	INIT(entry,
		.stream = stream,
		.this = this,
	);

	this->mutex->lock(this->mutex);
	this->connected->insert_last(this->connected, entry);
	this->mutex->unlock(this->mutex);

	stream->on_read(stream, (void*)on_read, this);

	return TRUE;
}

METHOD(lookip_socket_t, destroy, void,
	private_lookip_socket_t *this)
{
	DESTROY_IF(this->service);
	this->connected->destroy_function(this->connected, (void*)entry_destroy);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
lookip_socket_t *lookip_socket_create(lookip_listener_t *listener)
{
	private_lookip_socket_t *this;
	char *uri;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.listener = listener,
		.connected = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	uri = lib->settings->get_str(lib->settings,
							"%s.plugins.lookip.socket", "unix://" LOOKIP_SOCKET,
							lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 10);
	if (!this->service)
	{
		DBG1(DBG_CFG, "creating lookip socket failed");
		destroy(this);
		return NULL;
	}

	this->service->on_accept(this->service, (stream_service_cb_t)on_accept,
							 this, JOB_PRIO_CRITICAL, 1);

	return &this->public;
}
