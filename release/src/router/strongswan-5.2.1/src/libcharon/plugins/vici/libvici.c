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

#include "libvici.h"
#include "vici_builder.h"
#include "vici_dispatcher.h"
#include "vici_socket.h"

#include <library.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/hashtable.h>

#include <errno.h>

/**
 * Event registration
 */
typedef struct {
	/** name of event */
	char *name;
	/** callback function */
	vici_event_cb_t cb;
	/** user data for callback */
	void *user;
} event_t;

/**
 * Wait state signaled by asynchronous on_read callback
 */
typedef enum {
	WAIT_IDLE = 0,
	WAIT_SUCCESS,
	WAIT_FAILURE,
	WAIT_READ_ERROR,
} wait_state_t;

/**
 * Private vici connection contex.
 */
struct vici_conn_t {
	/** connection stream */
	stream_t *stream;
	/** event registrations, as char* => event_t */
	hashtable_t *events;
	/** connection lock */
	mutex_t *mutex;
	/** condvar to signal incoming response */
	condvar_t *cond;
	/** queued response message */
	chunk_t queue;
	/** asynchronous read error */
	int error;
	/** wait state */
	wait_state_t wait;
};

/**
 * Private vici request message.
 */
struct vici_req_t {
	/** connection context */
	vici_conn_t *conn;
	/** name of request message */
	char *name;
	/** message builder */
	vici_builder_t *b;
};

/**
 * Private vici response/event message.
 */
struct vici_res_t {
	/** response message */
	vici_message_t *message;
	/** allocated strings */
	linked_list_t *strings;
	/** item enumerator */
	enumerator_t *enumerator;
	/** currently enumerating type */
	vici_type_t type;
	/** currently enumerating name */
	char *name;
	/** currently enumerating value */
	chunk_t value;
	/** section nesting level of callback parser */
	int level;
};

/**
 * Signal wait result for waiting user thread
 */
static bool wait_result(vici_conn_t *conn, wait_state_t wait)
{
	conn->mutex->lock(conn->mutex);
	conn->wait = wait;
	conn->mutex->unlock(conn->mutex);
	conn->cond->signal(conn->cond);
	return FALSE;
}

/**
 * Signal wait error result for waiting user thread
 */
static bool read_error(vici_conn_t *conn, int err)
{
	conn->error = err;
	return wait_result(conn, WAIT_READ_ERROR);
}

/**
 * Handle a command response message
 */
static bool handle_response(vici_conn_t *conn, u_int32_t len)
{
	chunk_t buf;

	buf = chunk_alloc(len);
	if (!conn->stream->read_all(conn->stream, buf.ptr, buf.len))
	{
		free(buf.ptr);
		return read_error(conn, errno);
	}
	conn->queue = buf;
	return wait_result(conn, WAIT_SUCCESS);
}

/**
 * Dispatch received event message
 */
static bool handle_event(vici_conn_t *conn, u_int32_t len)
{
	vici_message_t *message;
	event_t *event;
	u_int8_t namelen;
	char name[257], *buf;

	if (len < sizeof(namelen))
	{
		return read_error(conn, EBADMSG);
	}
	if (!conn->stream->read_all(conn->stream, &namelen, sizeof(namelen)))
	{
		return read_error(conn, errno);
	}
	if (namelen > len - sizeof(namelen))
	{
		return read_error(conn, EBADMSG);
	}
	if (!conn->stream->read_all(conn->stream, name, namelen))
	{
		return read_error(conn, errno);
	}
	name[namelen] = '\0';
	len -= sizeof(namelen) + namelen;
	buf = malloc(len);
	if (!conn->stream->read_all(conn->stream, buf, len))
	{
		free(buf);
		return read_error(conn, errno);
	}
	message = vici_message_create_from_data(chunk_create(buf, len), TRUE);

	conn->mutex->lock(conn->mutex);
	event = conn->events->get(conn->events, name);
	if (event)
	{
		vici_res_t res = {
			.message = message,
			.enumerator = message->create_enumerator(message),
			.strings = linked_list_create(),
		};

		event->cb(event->user, name, &res);

		res.enumerator->destroy(res.enumerator);
		res.strings->destroy_function(res.strings, free);
	}
	conn->mutex->unlock(conn->mutex);

	message->destroy(message);

	return TRUE;
}

CALLBACK(on_read, bool,
	vici_conn_t *conn, stream_t *stream)
{
	u_int32_t len;
	u_int8_t op;
	ssize_t hlen;

	hlen = stream->read(stream, &len, sizeof(len), FALSE);
	if (hlen <= 0)
	{
		if (errno == EWOULDBLOCK)
		{
			return TRUE;
		}
		return read_error(conn, errno);
	}
	if (hlen < sizeof(len))
	{
		if (!stream->read_all(stream, ((void*)&len) + hlen, sizeof(len) - hlen))
		{
			return read_error(conn, errno);
		}
	}

	len = ntohl(len);
	if (len > VICI_MESSAGE_SIZE_MAX)
	{
		return read_error(conn, EBADMSG);
	}
	if (len-- < sizeof(op))
	{
		return read_error(conn, EBADMSG);
	}
	if (!stream->read_all(stream, &op, sizeof(op)))
	{
		return read_error(conn, errno);
	}
	switch (op)
	{
		case VICI_EVENT:
			return handle_event(conn, len);
		case VICI_CMD_RESPONSE:
			return handle_response(conn, len);
		case VICI_EVENT_CONFIRM:
			return wait_result(conn, WAIT_SUCCESS);
		case VICI_CMD_UNKNOWN:
		case VICI_EVENT_UNKNOWN:
			return wait_result(conn, WAIT_FAILURE);
		case VICI_CMD_REQUEST:
		case VICI_EVENT_REGISTER:
		case VICI_EVENT_UNREGISTER:
		default:
			return read_error(conn, EBADMSG);
	}
}

vici_conn_t* vici_connect(char *uri)
{
	vici_conn_t *conn;
	stream_t *stream;

	stream = lib->streams->connect(lib->streams, uri ?: VICI_DEFAULT_URI);
	if (!stream)
	{
		return NULL;
	}

	INIT(conn,
		.stream = stream,
		.events = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.cond = condvar_create(CONDVAR_TYPE_DEFAULT),
	);

	stream->on_read(stream, on_read, conn);

	return conn;
}

void vici_disconnect(vici_conn_t *conn)
{
	enumerator_t *enumerator;
	event_t *event;

	conn->stream->destroy(conn->stream);
	enumerator = conn->events->create_enumerator(conn->events);
	while (enumerator->enumerate(enumerator, NULL, &event))
	{
		free(event->name);
		free(event);
	}
	enumerator->destroy(enumerator);
	conn->events->destroy(conn->events);
	conn->mutex->destroy(conn->mutex);
	conn->cond->destroy(conn->cond);
	free(conn);
}

vici_req_t* vici_begin(char *name)
{
	vici_req_t *req;

	INIT(req,
		.name = strdup(name),
		.b = vici_builder_create(),
	);

	return req;
}

void vici_begin_section(vici_req_t *req, char *name)
{
	req->b->add(req->b, VICI_SECTION_START, name);
}

void vici_end_section(vici_req_t *req)
{
	req->b->add(req->b, VICI_SECTION_END);
}

void vici_add_key_value(vici_req_t *req, char *key, void *buf, int len)
{
	req->b->add(req->b, VICI_KEY_VALUE, key, chunk_create(buf, len));
}

void vici_add_key_valuef(vici_req_t *req, char *key, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	req->b->vadd_kv(req->b, key, fmt, args);
	va_end(args);
}

void vici_begin_list(vici_req_t *req, char *name)
{
	req->b->add(req->b, VICI_LIST_START, name);
}

void vici_add_list_item(vici_req_t *req, void *buf, int len)
{
	req->b->add(req->b, VICI_LIST_ITEM, chunk_create(buf, len));
}

void vici_add_list_itemf(vici_req_t *req, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	req->b->vadd_li(req->b, fmt, args);
	va_end(args);
}

void vici_end_list(vici_req_t *req)
{
	req->b->add(req->b, VICI_LIST_END);
}

vici_res_t* vici_submit(vici_req_t *req, vici_conn_t *conn)
{
	vici_message_t *message;
	vici_res_t *res;
	chunk_t data;
	u_int32_t len;
	u_int8_t namelen, op;

	message = req->b->finalize(req->b);
	if (!message)
	{
		errno = EINVAL;
		return NULL;
	}

	op = VICI_CMD_REQUEST;
	namelen = strlen(req->name);
	data = message->get_encoding(message);
	len = htonl(sizeof(op) + sizeof(namelen) + namelen + data.len);

	if (!conn->stream->write_all(conn->stream, &len, sizeof(len)) ||
		!conn->stream->write_all(conn->stream, &op, sizeof(op)) ||
		!conn->stream->write_all(conn->stream, &namelen, sizeof(namelen)) ||
		!conn->stream->write_all(conn->stream, req->name, namelen) ||
		!conn->stream->write_all(conn->stream, data.ptr, data.len))
	{
		free(req->name);
		free(req);
		message->destroy(message);
		return NULL;
	}
	free(req->name);
	free(req);
	message->destroy(message);

	message = NULL;
	conn->mutex->lock(conn->mutex);
	while (conn->wait == WAIT_IDLE)
	{
		conn->cond->wait(conn->cond, conn->mutex);
	}
	switch (conn->wait)
	{
		case WAIT_SUCCESS:
			message = vici_message_create_from_data(conn->queue, TRUE);
			conn->queue = chunk_empty;
			break;
		case WAIT_READ_ERROR:
			errno = conn->error;
			break;
		case WAIT_FAILURE:
		default:
			errno = ENOENT;
			break;
	}
	conn->wait = WAIT_IDLE;
	conn->mutex->unlock(conn->mutex);

	conn->stream->on_read(conn->stream, on_read, conn);

	if (message)
	{
		INIT(res,
			.message = message,
			.enumerator = message->create_enumerator(message),
			.strings = linked_list_create(),
		);
		return res;
	}
	return NULL;
}

void vici_free_req(vici_req_t *req)
{
	vici_message_t *message;

	free(req->name);
	message = req->b->finalize(req->b);
	if (message)
	{
		message->destroy(message);
	}
	free(req);
}

int vici_dump(vici_res_t *res, char *label, int pretty, FILE *out)
{
	if (res->message->dump(res->message, label, pretty, out))
	{
		return 0;
	}
	errno = EBADMSG;
	return 1;
}

vici_parse_t vici_parse(vici_res_t *res)
{
	if (!res->enumerator->enumerate(res->enumerator,
									&res->type, &res->name, &res->value))
	{
		return VICI_PARSE_ERROR;
	}
	switch (res->type)
	{
		case VICI_END:
			return VICI_PARSE_END;
		case VICI_SECTION_START:
			return VICI_PARSE_BEGIN_SECTION;
		case VICI_SECTION_END:
			return VICI_PARSE_END_SECTION;
		case VICI_LIST_START:
			return VICI_PARSE_BEGIN_LIST;
		case VICI_LIST_ITEM:
			return VICI_PARSE_LIST_ITEM;
		case VICI_LIST_END:
			return VICI_PARSE_END_LIST;
		case VICI_KEY_VALUE:
			return VICI_PARSE_KEY_VALUE;
		default:
			return VICI_PARSE_ERROR;
	}
}

char* vici_parse_name(vici_res_t *res)
{
	char *name;

	switch (res->type)
	{
		case VICI_SECTION_START:
		case VICI_LIST_START:
		case VICI_KEY_VALUE:
			name = strdup(res->name);
			res->strings->insert_last(res->strings, name);
			return name;
		default:
			errno = EINVAL;
			return NULL;
	}
}

int vici_parse_name_eq(vici_res_t *res, char *name)
{
	switch (res->type)
	{
		case VICI_SECTION_START:
		case VICI_LIST_START:
		case VICI_KEY_VALUE:
			return streq(name, res->name) ? 1 : 0;
		default:
			return 0;
	}
}

void* vici_parse_value(vici_res_t *res, int *len)
{
	switch (res->type)
	{
		case VICI_LIST_ITEM:
		case VICI_KEY_VALUE:
			*len = res->value.len;
			return res->value.ptr;
		default:
			*len = 0;
			errno = EINVAL;
			return NULL;
	}
}

char* vici_parse_value_str(vici_res_t *res)
{
	char *val;

	switch (res->type)
	{
		case VICI_LIST_ITEM:
		case VICI_KEY_VALUE:
			if (!chunk_printable(res->value, NULL, 0))
			{
				errno = EBADMSG;
				return NULL;
			}
			val = strndup(res->value.ptr, res->value.len);
			res->strings->insert_last(res->strings, val);
			return val;
		default:
			errno = EINVAL;
			return NULL;
	}
}

int vici_parse_cb(vici_res_t *res, vici_parse_section_cb_t section,
				  vici_parse_value_cb_t kv, vici_parse_value_cb_t li,
				  void *user)
{
	char *name, *list = NULL;
	void *value;
	int base, len, ret;

	base = res->level;

	while (TRUE)
	{
		switch (vici_parse(res))
		{
			case VICI_PARSE_KEY_VALUE:
				if (res->level == base)
				{
					if (kv)
					{
						name = vici_parse_name(res);
						value = vici_parse_value(res, &len);
						if (name && value)
						{
							ret = kv(user, res, name, value, len);
							if (ret)
							{
								return ret;
							}
						}
					}
				}
				break;
			case VICI_PARSE_BEGIN_SECTION:
				if (res->level++ == base)
				{
					if (section)
					{
						name = vici_parse_name(res);
						if (name)
						{
							ret = section(user, res, name);
							if (ret)
							{
								return ret;
							}
						}
					}
				}
				break;
			case VICI_PARSE_END_SECTION:
				if (res->level-- == base)
				{
					return 0;
				}
				break;
			case VICI_PARSE_END:
				res->level = 0;
				return 0;
			case VICI_PARSE_BEGIN_LIST:
				if (res->level == base)
				{
					list = vici_parse_name(res);
				}
				break;
			case VICI_PARSE_LIST_ITEM:
				if (list && li)
				{
					value = vici_parse_value(res, &len);
					if (value)
					{
						ret = li(user, res, list, value, len);
						if (ret)
						{
							return ret;
						}
					}
				}
				break;
			case VICI_PARSE_END_LIST:
				if (res->level == base)
				{
					list = NULL;
				}
				break;
			case VICI_PARSE_ERROR:
				res->level = 0;
				errno = EBADMSG;
				return 1;
		}
	}
}

void* vici_find(vici_res_t *res, int *len, char *fmt, ...)
{
	va_list args;
	chunk_t value;

	va_start(args, fmt);
	value = res->message->vget_value(res->message, chunk_empty, fmt, args);
	va_end(args);

	*len = value.len;
	return value.ptr;
}

char* vici_find_str(vici_res_t *res, char *def, char *fmt, ...)
{
	va_list args;
	char *str;

	va_start(args, fmt);
	str = res->message->vget_str(res->message, def, fmt, args);
	va_end(args);

	return str;
}

int vici_find_int(vici_res_t *res, int def, char *fmt, ...)
{
	va_list args;
	int val;

	va_start(args, fmt);
	val = res->message->vget_int(res->message, def, fmt, args);
	va_end(args);

	return val;
}

void vici_free_res(vici_res_t *res)
{
	res->strings->destroy_function(res->strings, free);
	res->message->destroy(res->message);
	res->enumerator->destroy(res->enumerator);
	free(res);
}

int vici_register(vici_conn_t *conn, char *name, vici_event_cb_t cb, void *user)
{
	event_t *event;
	u_int32_t len;
	u_int8_t namelen, op;
	int ret = 1;

	op = cb ? VICI_EVENT_REGISTER : VICI_EVENT_UNREGISTER;
	namelen = strlen(name);
	len = htonl(sizeof(op) + sizeof(namelen) + namelen);
	if (!conn->stream->write_all(conn->stream, &len, sizeof(len)) ||
		!conn->stream->write_all(conn->stream, &op, sizeof(op)) ||
		!conn->stream->write_all(conn->stream, &namelen, sizeof(namelen)) ||
		!conn->stream->write_all(conn->stream, name, namelen))
	{
		return 1;
	}

	conn->mutex->lock(conn->mutex);
	while (conn->wait == WAIT_IDLE)
	{
		conn->cond->wait(conn->cond, conn->mutex);
	}
	switch (conn->wait)
	{
		case WAIT_SUCCESS:
			ret = 0;
			break;
		case WAIT_READ_ERROR:
			errno = conn->error;
			break;
		case WAIT_FAILURE:
		default:
			errno = ENOENT;
			break;
	}
	conn->wait = WAIT_IDLE;
	conn->mutex->unlock(conn->mutex);

	conn->stream->on_read(conn->stream, on_read, conn);

	if (ret == 0)
	{
		conn->mutex->lock(conn->mutex);
		if (cb)
		{
			INIT(event,
				.name = strdup(name),
				.cb = cb,
				.user = user,
			);
			event = conn->events->put(conn->events, event->name, event);
		}
		else
		{
			event = conn->events->remove(conn->events, name);
		}
		conn->mutex->unlock(conn->mutex);

		if (event)
		{
			free(event->name);
			free(event);
		}
	}
	return ret;
}

void vici_init()
{
	library_init(NULL, "vici");
	if (lib->processor->get_total_threads(lib->processor) < 4)
	{
		dbg_default_set_level(0);
		lib->processor->set_threads(lib->processor, 4);
		dbg_default_set_level(1);
	}
}

void vici_deinit()
{
	lib->processor->cancel(lib->processor);
	library_deinit();
}
