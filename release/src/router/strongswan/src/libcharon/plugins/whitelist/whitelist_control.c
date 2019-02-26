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

#include "whitelist_control.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>
#include <collections/linked_list.h>

#include "whitelist_msg.h"

typedef struct private_whitelist_control_t private_whitelist_control_t;

/**
 * Private data of an whitelist_control_t object.
 */
struct private_whitelist_control_t {

	/**
	 * Public whitelist_control_t interface.
	 */
	whitelist_control_t public;

	/**
	 * Whitelist
	 */
	whitelist_listener_t *listener;

	/**
	 * Whitelist stream service
	 */
	stream_service_t *service;
};

/*
 * List whitelist entries using a read-copy
 */
static void list(private_whitelist_control_t *this,
				 stream_t *stream, identification_t *id)
{
	identification_t *current;
	enumerator_t *enumerator;
	linked_list_t *list;
	whitelist_msg_t msg = {
		.type = htonl(WHITELIST_LIST),
	};

	list = linked_list_create();
	enumerator = this->listener->create_enumerator(this->listener);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->matches(current, id))
		{
			list->insert_last(list, current->clone(current));
		}
	}
	enumerator->destroy(enumerator);

	while (list->remove_first(list, (void**)&current) == SUCCESS)
	{
		snprintf(msg.id, sizeof(msg.id), "%Y", current);
		current->destroy(current);
		if (!stream->write_all(stream, &msg, sizeof(msg)))
		{
			DBG1(DBG_CFG, "listing whitelist failed: %s", strerror(errno));
			break;
		}
	}
	list->destroy_offset(list, offsetof(identification_t, destroy));

	msg.type = htonl(WHITELIST_END);
	memset(msg.id, 0, sizeof(msg.id));
	stream->write_all(stream, &msg, sizeof(msg));
}

/**
 * Dispatch a received message
 */
static bool on_accept(private_whitelist_control_t *this, stream_t *stream)
{
	identification_t *id;
	whitelist_msg_t msg;

	while (stream->read_all(stream, &msg, sizeof(msg)))
	{
		msg.id[sizeof(msg.id) - 1] = 0;
		id = identification_create_from_string(msg.id);
		switch (ntohl(msg.type))
		{
			case WHITELIST_ADD:
				this->listener->add(this->listener, id);
				break;
			case WHITELIST_REMOVE:
				this->listener->remove(this->listener, id);
				break;
			case WHITELIST_LIST:
				list(this, stream, id);
				break;
			case WHITELIST_FLUSH:
				this->listener->flush(this->listener, id);
				break;
			case WHITELIST_ENABLE:
				this->listener->set_active(this->listener, TRUE);
				break;
			case WHITELIST_DISABLE:
				this->listener->set_active(this->listener, FALSE);
				break;
			default:
				DBG1(DBG_CFG, "received unknown whitelist command");
				break;
		}
		id->destroy(id);
	}

	return FALSE;
}

METHOD(whitelist_control_t, destroy, void,
	private_whitelist_control_t *this)
{
	this->service->destroy(this->service);
	free(this);
}

/**
 * See header
 */
whitelist_control_t *whitelist_control_create(whitelist_listener_t *listener)
{
	private_whitelist_control_t *this;
	char *uri;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.listener = listener,
	);

	uri = lib->settings->get_str(lib->settings,
				"%s.plugins.whitelist.socket", "unix://" WHITELIST_SOCKET,
				lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 10);
	if (!this->service)
	{
		DBG1(DBG_CFG, "creating whitelist socket failed");
		free(this);
		return NULL;
	}

	this->service->on_accept(this->service, (stream_service_cb_t)on_accept,
							 this, JOB_PRIO_CRITICAL, 0);

	return &this->public;
}
