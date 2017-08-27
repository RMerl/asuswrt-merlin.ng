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
	 * Mutex to synchronize logging
	 */
	mutex_t *mutex;
};

METHOD(logger_t, log_, void,
	private_vici_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t* ike_sa, const char *msg)
{
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
			this->dispatcher->raise_event(this->dispatcher, "log", 0, message);
		}
	}
	this->recursive--;

	this->mutex->unlock(this->mutex);
}

METHOD(logger_t, get_level, level_t,
	private_vici_logger_t *this, debug_t group)
{
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
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
