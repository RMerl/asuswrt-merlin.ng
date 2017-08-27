/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "vici_dispatcher.h"
#include "vici_socket.h"

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <threading/thread.h>
#include <collections/array.h>
#include <collections/hashtable.h>

typedef struct private_vici_dispatcher_t private_vici_dispatcher_t;

/**
 * Private data of an vici_dispatcher_t object.
 */
struct private_vici_dispatcher_t {

	/**
	 * Public vici_dispatcher_t interface.
	 */
	vici_dispatcher_t public;

	/**
	 * Socket to send/receive messages
	 */
	vici_socket_t *socket;

	/**
	 * List of registered commands (char* => command_t*)
	 */
	hashtable_t *cmds;

	/**
	 * List of known events, and registered clients (char* => event_t*)
	 */
	hashtable_t *events;

	/**
	 * Mutex to lock hashtables
	 */
	mutex_t *mutex;

	/**
	 * Condvar to signal command termination
	 */
	condvar_t *cond;
};

/**
 * Registered command
 */
typedef struct {
	/** command name */
	char *name;
	/** callback for command */
	vici_command_cb_t cb;
	/** user data to pass to callback */
	void *user;
	/** command currently in use? */
	u_int uses;
} command_t;

/**
 * Registered event
 */
typedef struct {
	/** event name */
	char *name;
	/** registered clients, as u_int */
	array_t *clients;
	/** event currently in use? */
	u_int uses;
} event_t;

/**
 * Send a operation code, optionally with name and message
 */
static void send_op(private_vici_dispatcher_t *this, u_int id,
					vici_operation_t op, char *name, vici_message_t *message)
{
	bio_writer_t *writer;
	u_int len;

	len = sizeof(u_int8_t);
	if (name)
	{
		len += sizeof(u_int8_t) + strlen(name);
	}
	if (message)
	{
		len += message->get_encoding(message).len;
	}
	writer = bio_writer_create(len);
	writer->write_uint8(writer, op);
	if (name)
	{
		writer->write_data8(writer, chunk_from_str(name));
	}
	if (message)
	{
		writer->write_data(writer, message->get_encoding(message));
	}
	this->socket->send(this->socket, id, writer->extract_buf(writer));
	writer->destroy(writer);
}

/**
 * Register client for event
 */
static void register_event(private_vici_dispatcher_t *this, char *name,
						   u_int id)
{
	event_t *event;

	this->mutex->lock(this->mutex);
	while (TRUE)
	{
		event = this->events->get(this->events, name);
		if (!event)
		{
			break;
		}
		if (!event->uses)
		{
			array_insert(event->clients, ARRAY_TAIL, &id);
			break;
		}
		this->cond->wait(this->cond, this->mutex);
	}
	this->mutex->unlock(this->mutex);

	if (event)
	{
		DBG2(DBG_CFG, "vici client %u registered for: %s", id, name);
		send_op(this, id, VICI_EVENT_CONFIRM, NULL, NULL);
	}
	else
	{
		DBG1(DBG_CFG, "vici client %u invalid registration: %s", id, name);
		send_op(this, id, VICI_EVENT_UNKNOWN, NULL, NULL);
	}
}

/**
 * Unregister client for event
 */
static void unregister_event(private_vici_dispatcher_t *this, char *name,
							 u_int id)
{
	enumerator_t *enumerator;
	event_t *event;
	u_int *current;
	bool found = FALSE;

	this->mutex->lock(this->mutex);
	while (TRUE)
	{
		event = this->events->get(this->events, name);
		if (!event)
		{
			break;
		}
		if (!event->uses)
		{
			enumerator = array_create_enumerator(event->clients);
			while (enumerator->enumerate(enumerator, &current))
			{
				if (*current == id)
				{
					array_remove_at(event->clients, enumerator);
					found = TRUE;
					break;
				}
			}
			enumerator->destroy(enumerator);
			break;
		}
		this->cond->wait(this->cond, this->mutex);
	}
	this->mutex->unlock(this->mutex);

	DBG2(DBG_CFG, "vici client %u unregistered for: %s", id, name);

	if (found)
	{
		send_op(this, id, VICI_EVENT_CONFIRM, NULL, NULL);
	}
	else
	{
		send_op(this, id, VICI_EVENT_UNKNOWN, NULL, NULL);
	}
}

/**
 * Data to release on thread cancellation
 */
typedef struct {
	private_vici_dispatcher_t *this;
	command_t *cmd;
	vici_message_t *request;
} release_data_t;

/**
 * Release command after execution/cancellation
 */
CALLBACK(release_command, void,
	release_data_t *release)
{
	release->request->destroy(release->request);

	release->this->mutex->lock(release->this->mutex);
	if (--release->cmd->uses == 0)
	{
		release->this->cond->broadcast(release->this->cond);
	}
	release->this->mutex->unlock(release->this->mutex);

	free(release);
}

/**
 * Process a request message
 */
void process_request(private_vici_dispatcher_t *this, char *name, u_int id,
					 chunk_t data)
{
	vici_message_t *response = NULL;
	release_data_t *release;
	command_t *cmd;

	this->mutex->lock(this->mutex);
	cmd = this->cmds->get(this->cmds, name);
	if (cmd)
	{
		cmd->uses++;
	}
	this->mutex->unlock(this->mutex);

	if (cmd)
	{
		INIT(release,
			.this = this,
			.cmd = cmd,
		);

		DBG2(DBG_CFG, "vici client %u requests: %s", id, name);

		thread_cleanup_push(release_command, release);

		release->request = vici_message_create_from_data(data, FALSE);
		response = release->cmd->cb(cmd->user, cmd->name, id, release->request);

		thread_cleanup_pop(TRUE);

		if (response)
		{
			send_op(this, id, VICI_CMD_RESPONSE, NULL, response);
			response->destroy(response);
		}
	}
	else
	{
		DBG1(DBG_CFG, "vici client %u invalid request: %s", id, name);
		send_op(this, id, VICI_CMD_UNKNOWN, NULL, NULL);
	}
}

CALLBACK(inbound, void,
	private_vici_dispatcher_t *this, u_int id, chunk_t data)
{
	bio_reader_t *reader;
	chunk_t chunk;
	u_int8_t type;
	char name[257];

	reader = bio_reader_create(data);
	if (reader->read_uint8(reader, &type))
	{
		switch (type)
		{
			case VICI_EVENT_REGISTER:
				if (reader->read_data8(reader, &chunk) &&
					vici_stringify(chunk, name, sizeof(name)))
				{
					register_event(this, name, id);
				}
				else
				{
					DBG1(DBG_CFG, "invalid vici register message");
				}
				break;
			case VICI_EVENT_UNREGISTER:
				if (reader->read_data8(reader, &chunk) &&
					vici_stringify(chunk, name, sizeof(name)))
				{
					unregister_event(this, name, id);
				}
				else
				{
					DBG1(DBG_CFG, "invalid vici unregister message");
				}
				break;
			case VICI_CMD_REQUEST:
				if (reader->read_data8(reader, &chunk) &&
					vici_stringify(chunk, name, sizeof(name)))
				{
					thread_cleanup_push((void*)reader->destroy, reader);
					process_request(this, name, id, reader->peek(reader));
					thread_cleanup_pop(FALSE);
				}
				else
				{
					DBG1(DBG_CFG, "invalid vici request message");
				}
				break;
			case VICI_CMD_RESPONSE:
			case VICI_EVENT_CONFIRM:
			case VICI_EVENT_UNKNOWN:
			case VICI_EVENT:
			default:
				DBG1(DBG_CFG, "unsupported vici operation: %u", type);
				break;
		}
	}
	else
	{
		DBG1(DBG_CFG, "invalid vici message");
	}
	reader->destroy(reader);
}

CALLBACK(connect_, void,
	private_vici_dispatcher_t *this, u_int id)
{
	DBG2(DBG_CFG, "vici client %u connected", id);
}

CALLBACK(disconnect, void,
	private_vici_dispatcher_t *this, u_int id)
{
	enumerator_t *events, *ids;
	event_t *event;
	u_int *current;

	/* deregister client from all events */
	this->mutex->lock(this->mutex);
	events = this->events->create_enumerator(this->events);
	while (events->enumerate(events, NULL, &event))
	{
		while (event->uses)
		{
			this->cond->wait(this->cond, this->mutex);
		}
		ids = array_create_enumerator(event->clients);
		while (ids->enumerate(ids, &current))
		{
			if (id == *current)
			{
				array_remove_at(event->clients, ids);
			}
		}
		ids->destroy(ids);
	}
	events->destroy(events);
	this->mutex->unlock(this->mutex);

	DBG2(DBG_CFG, "vici client %u disconnected", id);
}

METHOD(vici_dispatcher_t, manage_command, void,
	private_vici_dispatcher_t *this, char *name,
	vici_command_cb_t cb, void *user)
{
	command_t *cmd;

	this->mutex->lock(this->mutex);
	if (cb)
	{
		INIT(cmd,
			.name = strdup(name),
			.cb = cb,
			.user = user,
		);
		cmd = this->cmds->put(this->cmds, cmd->name, cmd);
	}
	else
	{
		cmd = this->cmds->remove(this->cmds, name);
	}
	if (cmd)
	{
		while (cmd->uses)
		{
			this->cond->wait(this->cond, this->mutex);
		}
		free(cmd->name);
		free(cmd);
	}
	this->mutex->unlock(this->mutex);
}

METHOD(vici_dispatcher_t, manage_event, void,
	private_vici_dispatcher_t *this, char *name, bool reg)
{
	event_t *event;

	this->mutex->lock(this->mutex);
	if (reg)
	{
		INIT(event,
			.name = strdup(name),
			.clients = array_create(sizeof(u_int), 0),
		);
		event = this->events->put(this->events, event->name, event);
	}
	else
	{
		event = this->events->remove(this->events, name);
	}
	if (event)
	{
		while (event->uses)
		{
			this->cond->wait(this->cond, this->mutex);
		}
		array_destroy(event->clients);
		free(event->name);
		free(event);
	}
	this->mutex->unlock(this->mutex);
}

METHOD(vici_dispatcher_t, raise_event, void,
	private_vici_dispatcher_t *this, char *name, u_int id,
	vici_message_t *message)
{
	enumerator_t *enumerator;
	event_t *event;
	u_int *current;

	this->mutex->lock(this->mutex);
	event = this->events->get(this->events, name);
	if (event)
	{
		event->uses++;
		this->mutex->unlock(this->mutex);

		enumerator = array_create_enumerator(event->clients);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (id == 0 || id == *current)
			{
				send_op(this, *current, VICI_EVENT, name, message);
			}
		}
		enumerator->destroy(enumerator);

		this->mutex->lock(this->mutex);
		if (--event->uses == 0)
		{
			this->cond->broadcast(this->cond);
		}
	}
	this->mutex->unlock(this->mutex);

	message->destroy(message);
}

METHOD(vici_dispatcher_t, destroy, void,
	private_vici_dispatcher_t *this)
{
	DESTROY_IF(this->socket);
	this->mutex->destroy(this->mutex);
	this->cond->destroy(this->cond);
	this->cmds->destroy(this->cmds);
	this->events->destroy(this->events);
	free(this);
}

/**
 * See header
 */
vici_dispatcher_t *vici_dispatcher_create(char *uri)
{
	private_vici_dispatcher_t *this;

	INIT(this,
		.public = {
			.manage_command = _manage_command,
			.manage_event = _manage_event,
			.raise_event = _raise_event,
			.destroy = _destroy,
		},
		.cmds = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1),
		.events = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.cond = condvar_create(CONDVAR_TYPE_DEFAULT),
	);

	this->socket = vici_socket_create(uri, inbound, connect_, disconnect, this);
	if (!this->socket)
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
