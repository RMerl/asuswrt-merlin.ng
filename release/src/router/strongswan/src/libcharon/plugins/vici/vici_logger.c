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

#include "vici_logger.h"
#include "vici_builder.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

typedef struct private_vici_logger_t private_vici_logger_t;

/**
 * Private data of an vici_logger_t object.
 */
struct private_vici_logger_t {

	/**
	 * Public vici_logger_t interface.
	 */
	vici_logger_t public;

	/**
	 * Dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * Recursiveness avoidance counter
	 */
	int recursive;

	/**
	 * List of messages to raise async events
	 */
	linked_list_t *queue;

	/**
	 * Mutex to synchronize logging
	 */
	mutex_t *mutex;
};

/**
 * Async callback to raise events for queued messages
 */
static job_requeue_t raise_events(private_vici_logger_t *this)
{
	vici_message_t *message;
	u_int count;

	this->mutex->lock(this->mutex);
	count = this->queue->get_count(this->queue);
	this->queue->remove_first(this->queue, (void**)&message);
	this->mutex->unlock(this->mutex);

	if (count > 0)
	{
		this->dispatcher->raise_event(this->dispatcher, "log", 0, message);
	}
	if (count > 1)
	{
		return JOB_REQUEUE_DIRECT;
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Queue a message for async processing
 */
static void queue_messsage(private_vici_logger_t *this, vici_message_t *message)
{
	this->queue->insert_last(this->queue, message);
	if (this->queue->get_count(this->queue) == 1)
	{
		lib->processor->queue_job(lib->processor, (job_t*)
					callback_job_create((callback_job_cb_t)raise_events,
										this, NULL, NULL));
	}
}

METHOD(logger_t, log_, void,
	private_vici_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t* ike_sa, const char *msg)
{
	if (!this->dispatcher->has_event_listeners(this->dispatcher, "log"))
	{
		return;
	}

	this->mutex->lock(this->mutex);

	/* avoid recursive invocations by the vici subsystem */
	if (this->recursive++ == 0)
	{
		vici_message_t *message;
		vici_builder_t *builder;

		builder = vici_builder_create();
		builder->add_kv(builder, "group", "%N", debug_names, group);
		builder->add_kv(builder, "level", "%d", level);
		builder->add_kv(builder, "thread", "%d", thread);
		if (ike_sa)
		{
			builder->add_kv(builder, "ikesa-name", "%s",
							ike_sa->get_name(ike_sa));
			builder->add_kv(builder, "ikesa-uniqueid", "%u",
							ike_sa->get_unique_id(ike_sa));
		}
		builder->add_kv(builder, "msg", "%s", msg);

		message = builder->finalize(builder);
		if (message)
		{
			queue_messsage(this, message);
		}
	}
	this->recursive--;

	this->mutex->unlock(this->mutex);
}

METHOD(logger_t, get_level, level_t,
	private_vici_logger_t *this, debug_t group)
{
	/* anything higher might produce a loop as sending messages or listening
	 * for clients might cause log messages itself */
	return LEVEL_CTRL;
}

/**
 * (Un-)register dispatcher functions/events
 */
static void manage_commands(private_vici_logger_t *this, bool reg)
{
	this->dispatcher->manage_event(this->dispatcher, "log", reg);
}

METHOD(vici_logger_t, destroy, void,
	private_vici_logger_t *this)
{
	manage_commands(this, FALSE);
	this->queue->destroy_offset(this->queue, offsetof(vici_message_t, destroy));
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
vici_logger_t *vici_logger_create(vici_dispatcher_t *dispatcher)
{
	private_vici_logger_t *this;

	INIT(this,
		.public = {
			.logger = {
				.log = _log_,
				.get_level = _get_level,
			},
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.queue = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
