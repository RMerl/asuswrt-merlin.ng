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

#include "vici_socket.h"

#include <threading/mutex.h>
#include <threading/condvar.h>
#include <threading/thread.h>
#include <collections/array.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

#include <errno.h>
#include <string.h>

typedef struct private_vici_socket_t private_vici_socket_t;

/**
 * Private members of vici_socket_t
 */
struct private_vici_socket_t {

	/**
	 * public functions
	 */
	vici_socket_t public;

	/**
	 * Inbound message callback
	 */
	vici_inbound_cb_t inbound;

	/**
	 * Client connect callback
	 */
	vici_connect_cb_t connect;

	/**
	 * Client disconnect callback
	 */
	vici_disconnect_cb_t disconnect;

	/**
	 * Next client connection identifier
	 */
	u_int nextid;

	/**
	 * User data for callbacks
	 */
	void *user;

	/**
	 * Service accepting vici connections
	 */
	stream_service_t *service;

	/**
	 * Client connections, as entry_t
	 */
	linked_list_t *connections;

	/**
	 * mutex for client connections
	 */
	mutex_t *mutex;
};

/**
 * Data to securely reference an entry
 */
typedef struct {
	/* reference to socket instance */
	private_vici_socket_t *this;
	/** connection identifier of entry */
	u_int id;
} entry_selector_t;

/**
 * Partially processed message
 */
typedef struct {
	/** bytes of length header sent/received */
	u_char hdrlen;
	/** bytes of length header */
	char hdr[sizeof(u_int32_t)];
	/** send/receive buffer on heap */
	chunk_t buf;
	/** bytes sent/received in buffer */
	u_int32_t done;
} msg_buf_t;

/**
 * Client connection entry
 */
typedef struct {
	/** reference to socket */
	private_vici_socket_t *this;
	/** associated stream */
	stream_t *stream;
	/** queued messages to send, as msg_buf_t pointers */
	array_t *out;
	/** input message buffer */
	msg_buf_t in;
	/** queued input messages to process, as chunk_t */
	array_t *queue;
	/** do we have job processing input queue? */
	bool has_processor;
	/** client connection identifier */
	u_int id;
	/** any users reading over this connection? */
	int readers;
	/** any users writing over this connection? */
	int writers;
	/** condvar to wait for usage  */
	condvar_t *cond;
} entry_t;

/**
 * Destroy an connection entry
 */
CALLBACK(destroy_entry, void,
	entry_t *entry)
{
	msg_buf_t *out;
	chunk_t chunk;

	entry->stream->destroy(entry->stream);
	entry->this->disconnect(entry->this->user, entry->id);
	entry->cond->destroy(entry->cond);

	while (array_remove(entry->out, ARRAY_TAIL, &out))
	{
		chunk_clear(&out->buf);
		free(out);
	}
	array_destroy(entry->out);
	while (array_remove(entry->queue, ARRAY_TAIL, &chunk))
	{
		chunk_clear(&chunk);
	}
	array_destroy(entry->queue);
	chunk_clear(&entry->in.buf);
	free(entry);
}

/**
 * Find entry by stream (if given) or id, claim use
 */
static entry_t* find_entry(private_vici_socket_t *this, stream_t *stream,
						   u_int id, bool reader, bool writer)
{
	enumerator_t *enumerator;
	entry_t *entry, *found = NULL;
	bool candidate = TRUE;

	this->mutex->lock(this->mutex);
	while (candidate && !found)
	{
		candidate = FALSE;
		enumerator = this->connections->create_enumerator(this->connections);
		while (enumerator->enumerate(enumerator, &entry))
		{
			if (stream)
			{
				if (entry->stream != stream)
				{
					continue;
				}
			}
			else
			{
				if (entry->id != id)
				{
					continue;
				}
			}
			candidate = TRUE;

			if ((reader && entry->readers) ||
				(writer && entry->writers))
			{
				entry->cond->wait(entry->cond, this->mutex);
				break;
			}
			if (reader)
			{
				entry->readers++;
			}
			if (writer)
			{
				entry->writers++;
			}
			found = entry;
			break;
		}
		enumerator->destroy(enumerator);
	}
	this->mutex->unlock(this->mutex);

	return found;
}

/**
 * Remove entry by id, claim use
 */
static entry_t* remove_entry(private_vici_socket_t *this, u_int id)
{
	enumerator_t *enumerator;
	entry_t *entry, *found = NULL;
	bool candidate = TRUE;

	this->mutex->lock(this->mutex);
	while (candidate && !found)
	{
		candidate = FALSE;
		enumerator = this->connections->create_enumerator(this->connections);
		while (enumerator->enumerate(enumerator, &entry))
		{
			if (entry->id == id)
			{
				candidate = TRUE;
				if (entry->readers || entry->writers)
				{
					entry->cond->wait(entry->cond, this->mutex);
					break;
				}
				this->connections->remove_at(this->connections, enumerator);
				found = entry;
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	this->mutex->unlock(this->mutex);

	return found;
}

/**
 * Release a claimed entry
 */
static void put_entry(private_vici_socket_t *this, entry_t *entry,
					  bool reader, bool writer)
{
	this->mutex->lock(this->mutex);
	if (reader)
	{
		entry->readers--;
	}
	if (writer)
	{
		entry->writers--;
	}
	entry->cond->signal(entry->cond);
	this->mutex->unlock(this->mutex);
}

/**
 * Asynchronous callback to disconnect client
 */
CALLBACK(disconnect_async, job_requeue_t,
	entry_selector_t *sel)
{
	entry_t *entry;

	entry = remove_entry(sel->this, sel->id);
	if (entry)
	{
		destroy_entry(entry);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Disconnect a connected client
 */
static void disconnect(private_vici_socket_t *this, u_int id)
{
	entry_selector_t *sel;

	INIT(sel,
		.this = this,
		.id = id,
	);

	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create(disconnect_async, sel, free, NULL));
}

/**
 * Write queued output data
 */
static bool do_write(private_vici_socket_t *this, entry_t *entry,
					 stream_t *stream)
{
	msg_buf_t *out;
	ssize_t len;

	while (array_get(entry->out, ARRAY_HEAD, &out))
	{
		/* write header */
		while (out->hdrlen < sizeof(out->hdr))
		{
			len = stream->write(stream, out->hdr + out->hdrlen,
								sizeof(out->hdr) - out->hdrlen, FALSE);
			if (len == 0)
			{
				return FALSE;
			}
			if (len < 0)
			{
				if (errno == EWOULDBLOCK)
				{
					return TRUE;
				}
				DBG1(DBG_CFG, "vici header write error: %s", strerror(errno));
				return FALSE;
			}
			out->hdrlen += len;
		}

		/* write buffer buffer */
		while (out->buf.len > out->done)
		{
			len = stream->write(stream, out->buf.ptr + out->done,
								out->buf.len - out->done, FALSE);
			if (len == 0)
			{
				DBG1(DBG_CFG, "premature vici disconnect");
				return FALSE;
			}
			if (len < 0)
			{
				if (errno == EWOULDBLOCK)
				{
					return TRUE;
				}
				DBG1(DBG_CFG, "vici write error: %s", strerror(errno));
				return FALSE;
			}
			out->done += len;
		}

		if (array_remove(entry->out, ARRAY_HEAD, &out))
		{
			chunk_clear(&out->buf);
			free(out);
		}
	}
	return TRUE;
}

/**
 * Send pending messages
 */
CALLBACK(on_write, bool,
	private_vici_socket_t *this, stream_t *stream)
{
	entry_t *entry;
	bool ret = FALSE;

	entry = find_entry(this, stream, 0, FALSE, TRUE);
	if (entry)
	{
		ret = do_write(this, entry, stream);
		if (ret)
		{
			/* unregister if we have no more messages to send */
			ret = array_count(entry->out) != 0;
		}
		else
		{
			disconnect(entry->this, entry->id);
		}
		put_entry(this, entry, FALSE, TRUE);
	}

	return ret;
}

/**
 * Read in available header with data, non-blocking cumulating to buffer
 */
static bool do_read(private_vici_socket_t *this, entry_t *entry,
					stream_t *stream)
{
	u_int32_t msglen;
	ssize_t len;

	/* assemble the length header first */
	while (entry->in.hdrlen < sizeof(entry->in.hdr))
	{
		len = stream->read(stream, entry->in.hdr + entry->in.hdrlen,
						   sizeof(entry->in.hdr) - entry->in.hdrlen, FALSE);
		if (len == 0)
		{
			return FALSE;
		}
		if (len < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				return TRUE;
			}
			DBG1(DBG_CFG, "vici header read error: %s", strerror(errno));
			return FALSE;
		}
		entry->in.hdrlen += len;
		if (entry->in.hdrlen == sizeof(entry->in.hdr))
		{
			msglen = untoh32(entry->in.hdr);
			if (msglen > VICI_MESSAGE_SIZE_MAX)
			{
				DBG1(DBG_CFG, "vici message length %u exceeds %u bytes limit, "
					 "ignored", msglen, VICI_MESSAGE_SIZE_MAX);
				return FALSE;
			}
			/* header complete, continue with data */
			entry->in.buf = chunk_alloc(msglen);
		}
	}

	/* assemble buffer */
	while (entry->in.buf.len > entry->in.done)
	{
		len = stream->read(stream, entry->in.buf.ptr + entry->in.done,
						   entry->in.buf.len - entry->in.done, FALSE);
		if (len == 0)
		{
			DBG1(DBG_CFG, "premature vici disconnect");
			return FALSE;
		}
		if (len < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				return TRUE;
			}
			DBG1(DBG_CFG, "vici read error: %s", strerror(errno));
			return FALSE;
		}
		entry->in.done += len;
	}

	return TRUE;
}

/**
 * Callback processing incoming requestes in strict order
 */
CALLBACK(process_queue, job_requeue_t,
	entry_selector_t *sel)
{
	entry_t *entry;
	chunk_t chunk;
	bool found;
	u_int id;

	while (TRUE)
	{
		entry = find_entry(sel->this, NULL, sel->id, TRUE, FALSE);
		if (!entry)
		{
			break;
		}

		found = array_remove(entry->queue, ARRAY_HEAD, &chunk);
		if (!found)
		{
			entry->has_processor = FALSE;
		}
		id = entry->id;
		put_entry(sel->this, entry, TRUE, FALSE);
		if (!found)
		{
			break;
		}

		thread_cleanup_push(free, chunk.ptr);
		sel->this->inbound(sel->this->user, id, chunk);
		thread_cleanup_pop(TRUE);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Process incoming messages
 */
CALLBACK(on_read, bool,
	private_vici_socket_t *this, stream_t *stream)
{
	entry_selector_t *sel;
	entry_t *entry;
	bool ret = FALSE;

	entry = find_entry(this, stream, 0, TRUE, FALSE);
	if (entry)
	{
		ret = do_read(this, entry, stream);
		if (!ret)
		{
			disconnect(this, entry->id);
		}
		else if (entry->in.hdrlen == sizeof(entry->in.hdr) &&
				 entry->in.buf.len == entry->in.done)
		{
			array_insert(entry->queue, ARRAY_TAIL, &entry->in.buf);
			entry->in.buf = chunk_empty;
			entry->in.hdrlen = entry->in.done = 0;

			if (!entry->has_processor)
			{
				INIT(sel,
					.this = this,
					.id = entry->id,
				);
				lib->processor->queue_job(lib->processor,
							(job_t*)callback_job_create(process_queue,
														sel, free, NULL));
				entry->has_processor = TRUE;
			}
		}
		put_entry(this, entry, TRUE, FALSE);
	}

	return ret;
}

/**
 * Process connection request
 */
CALLBACK(on_accept, bool,
	private_vici_socket_t *this, stream_t *stream)
{
	entry_t *entry;
	u_int id;

	id = ref_get(&this->nextid);

	INIT(entry,
		.this = this,
		.stream = stream,
		.id = id,
		.out = array_create(0, 0),
		.queue = array_create(sizeof(chunk_t), 0),
		.cond = condvar_create(CONDVAR_TYPE_DEFAULT),
		.readers = 1,
	);

	this->mutex->lock(this->mutex);
	this->connections->insert_last(this->connections, entry);
	this->mutex->unlock(this->mutex);

	stream->on_read(stream, on_read, this);

	put_entry(this, entry, TRUE, FALSE);

	this->connect(this->user, id);

	return TRUE;
}

/**
 * Async callback to enable writer
 */
CALLBACK(enable_writer, job_requeue_t,
	entry_selector_t *sel)
{
	entry_t *entry;

	entry = find_entry(sel->this, NULL, sel->id, FALSE, TRUE);
	if (entry)
	{
		entry->stream->on_write(entry->stream, on_write, sel->this);
		put_entry(sel->this, entry, FALSE, TRUE);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(vici_socket_t, send_, void,
	private_vici_socket_t *this, u_int id, chunk_t msg)
{
	if (msg.len <= VICI_MESSAGE_SIZE_MAX)
	{
		entry_selector_t *sel;
		msg_buf_t *out;
		entry_t *entry;

		entry = find_entry(this, NULL, id, FALSE, TRUE);
		if (entry)
		{
			INIT(out,
				.buf = msg,
			);
			htoun32(out->hdr, msg.len);

			array_insert(entry->out, ARRAY_TAIL, out);
			if (array_count(entry->out) == 1)
			{	/* asynchronously re-enable on_write callback when we get data */
				INIT(sel,
					.this = this,
					.id = entry->id,
				);
				lib->processor->queue_job(lib->processor,
							(job_t*)callback_job_create(enable_writer,
														sel, free, NULL));
			}
			put_entry(this, entry, FALSE, TRUE);
		}
		else
		{
			DBG1(DBG_CFG, "vici connection %u unknown", id);
			chunk_clear(&msg);
		}
	}
	else
	{
		DBG1(DBG_CFG, "vici message size %zu exceeds maximum size of %u, "
			 "discarded", msg.len, VICI_MESSAGE_SIZE_MAX);
		chunk_clear(&msg);
	}
}

METHOD(vici_socket_t, destroy, void,
	private_vici_socket_t *this)
{
	DESTROY_IF(this->service);
	this->connections->destroy_function(this->connections, destroy_entry);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * see header file
 */
vici_socket_t *vici_socket_create(char *uri, vici_inbound_cb_t inbound,
								  vici_connect_cb_t connect,
								  vici_disconnect_cb_t disconnect, void *user)
{
	private_vici_socket_t *this;

	INIT(this,
		.public = {
			.send = _send_,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.connections = linked_list_create(),
		.inbound = inbound,
		.connect = connect,
		.disconnect = disconnect,
		.user = user,
	);

	this->service = lib->streams->create_service(lib->streams, uri, 3);
	if (!this->service)
	{
		DBG1(DBG_CFG, "creating vici socket failed");
		destroy(this);
		return NULL;
	}
	this->service->on_accept(this->service, on_accept, this,
							 JOB_PRIO_CRITICAL, 0);

	return &this->public;
}
