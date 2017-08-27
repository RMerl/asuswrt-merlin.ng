/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * Hochschule fuer Technik Rapperswil
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

#include "ipsec_event_relay.h"

#include <library.h>
#include <utils/debug.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <collections/blocking_queue.h>
#include <processing/jobs/callback_job.h>

typedef struct private_ipsec_event_relay_t private_ipsec_event_relay_t;

/**
 * Private additions to ipsec_event_relay_t.
 */
struct private_ipsec_event_relay_t {

	/**
	 * Public members
	 */
	ipsec_event_relay_t public;

	/**
	 * Registered listeners
	 */
	linked_list_t *listeners;

	/**
	 * Lock to safely access the list of listeners
	 */
	rwlock_t *lock;

	/**
	 * Blocking queue for events
	 */
	blocking_queue_t *queue;
};

/**
 * Helper struct used to manage events in a queue
 */
typedef struct {

	/**
	 * Type of the event
	 */
	enum {
		IPSEC_EVENT_EXPIRE,
	} type;

	/**
	 * Reqid of the SA, if any
	 */
	u_int32_t reqid;

	/**
	 * SPI of the SA, if any
	 */
	u_int32_t spi;

	/**
	 * Additional data for specific event types
	 */
	union {

		struct {
			/** Protocol of the SA */
			u_int8_t protocol;
			/** TRUE in case of a hard expire */
			bool hard;
		} expire;

	} data;

} ipsec_event_t;

/**
 * Dequeue events and relay them to listeners
 */
static job_requeue_t handle_events(private_ipsec_event_relay_t *this)
{
	enumerator_t *enumerator;
	ipsec_event_listener_t *current;
	ipsec_event_t *event;

	event = this->queue->dequeue(this->queue);

	this->lock->read_lock(this->lock);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		switch (event->type)
		{
			case IPSEC_EVENT_EXPIRE:
				if (current->expire)
				{
					current->expire(event->reqid, event->data.expire.protocol,
									event->spi, event->data.expire.hard);
				}
				break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	free(event);
	return JOB_REQUEUE_DIRECT;
}

METHOD(ipsec_event_relay_t, expire, void,
	private_ipsec_event_relay_t *this, u_int32_t reqid, u_int8_t protocol,
	u_int32_t spi, bool hard)
{
	ipsec_event_t *event;

	INIT(event,
		.type = IPSEC_EVENT_EXPIRE,
		.reqid = reqid,
		.spi = spi,
		.data = {
			.expire = {
				.protocol = protocol,
				.hard = hard,
			},
		},
	);
	this->queue->enqueue(this->queue, event);
}

METHOD(ipsec_event_relay_t, register_listener, void,
	private_ipsec_event_relay_t *this, ipsec_event_listener_t *listener)
{
	this->lock->write_lock(this->lock);
	this->listeners->insert_last(this->listeners, listener);
	this->lock->unlock(this->lock);
}

METHOD(ipsec_event_relay_t, unregister_listener, void,
	private_ipsec_event_relay_t *this, ipsec_event_listener_t *listener)
{
	this->lock->write_lock(this->lock);
	this->listeners->remove(this->listeners, listener, NULL);
	this->lock->unlock(this->lock);
}

METHOD(ipsec_event_relay_t, destroy, void,
	private_ipsec_event_relay_t *this)
{
	this->queue->destroy_function(this->queue, free);
	this->listeners->destroy(this->listeners);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Described in header.
 */
ipsec_event_relay_t *ipsec_event_relay_create()
{
	private_ipsec_event_relay_t *this;

	INIT(this,
		.public = {
			.expire = _expire,
			.register_listener = _register_listener,
			.unregister_listener = _unregister_listener,
			.destroy = _destroy,
		},
		.listeners = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.queue = blocking_queue_create(),
	);

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create((callback_job_cb_t)handle_events, this,
			NULL, (callback_job_cancel_t)return_false));

	return &this->public;
}
