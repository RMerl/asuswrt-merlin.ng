/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "duplicheck_notify.h"
#include "duplicheck_msg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>
#include <threading/mutex.h>
#include <threading/thread.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>


typedef struct private_duplicheck_notify_t private_duplicheck_notify_t;

/**
 * Private data of an duplicheck_notify_t object.
 */
struct private_duplicheck_notify_t {

	/**
	 * Public duplicheck_notify_t interface.
	 */
	duplicheck_notify_t public;

	/**
	 * Mutex to lock list
	 */
	mutex_t *mutex;

	/**
	 * List of connected clients, as stream_t
	 */
	linked_list_t *connected;

	/**
	 * stream service accepting connections
	 */
	stream_service_t *service;
};

/**
 * Accept duplicheck notification connections
 */
static bool on_accept(private_duplicheck_notify_t *this, stream_t *stream)
{
	this->mutex->lock(this->mutex);
	this->connected->insert_last(this->connected, stream);
	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(duplicheck_notify_t, send_, void,
	private_duplicheck_notify_t *this, identification_t *id)
{
	enumerator_t *enumerator;
	stream_t *stream;
	u_int16_t nlen;
	char buf[512];
	int len;

	len = snprintf(buf, sizeof(buf), "%Y", id);
	if (len > 0 && len < sizeof(buf))
	{
		nlen = htons(len);

		this->mutex->lock(this->mutex);
		enumerator = this->connected->create_enumerator(this->connected);
		while (enumerator->enumerate(enumerator, &stream))
		{
			if (!stream->write_all(stream, &nlen, sizeof(nlen)) ||
				!stream->write_all(stream, buf, len))
			{
				DBG1(DBG_CFG, "sending duplicheck notify failed: %s",
					 strerror(errno));
				this->connected->remove_at(this->connected, enumerator);
				stream->destroy(stream);
			}
		}
		enumerator->destroy(enumerator);
		this->mutex->unlock(this->mutex);
	}
}

METHOD(duplicheck_notify_t, destroy, void,
	private_duplicheck_notify_t *this)
{
	DESTROY_IF(this->service);
	this->connected->destroy_offset(this->connected, offsetof(stream_t, destroy));
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
duplicheck_notify_t *duplicheck_notify_create()
{
	private_duplicheck_notify_t *this;
	char *uri;

	INIT(this,
		.public = {
			.send = _send_,
			.destroy = _destroy,
		},
		.connected = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	uri = lib->settings->get_str(lib->settings,
					"%s.plugins.duplicheck.socket", "unix://" DUPLICHECK_SOCKET,
					lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 3);
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
