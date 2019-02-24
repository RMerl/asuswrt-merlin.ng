/*
 * Copyright (C) 2007 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "ike_auth_lifetime.h"

#include <time.h>

#include <daemon.h>
#include <encoding/payloads/notify_payload.h>


typedef struct private_ike_auth_lifetime_t private_ike_auth_lifetime_t;

/**
 * Private members of a ike_auth_lifetime_t task.
 */
struct private_ike_auth_lifetime_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_auth_lifetime_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;
};

/**
 * add the AUTH_LIFETIME notify to the message
 */
static void add_auth_lifetime(private_ike_auth_lifetime_t *this, message_t *message)
{
	chunk_t chunk;
	uint32_t lifetime;

	lifetime = this->ike_sa->get_statistic(this->ike_sa, STAT_REAUTH);
	if (lifetime)
	{
		lifetime -= time_monotonic(NULL);
		chunk = chunk_from_thing(lifetime);
		*(uint32_t*)chunk.ptr = htonl(lifetime);
		message->add_notify(message, FALSE, AUTH_LIFETIME, chunk);
	}
}

/**
 * read notifys from message and evaluate them
 */
static void process_payloads(private_ike_auth_lifetime_t *this, message_t *message)
{
	notify_payload_t *notify;
	chunk_t data;
	uint32_t lifetime;

	notify = message->get_notify(message, AUTH_LIFETIME);
	if (notify)
	{
		data = notify->get_notification_data(notify);
		lifetime = ntohl(*(uint32_t*)data.ptr);
		this->ike_sa->set_auth_lifetime(this->ike_sa, lifetime);
	}
}

METHOD(task_t, build_i, status_t,
	private_ike_auth_lifetime_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		add_auth_lifetime(this, message);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_auth_lifetime_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		process_payloads(this, message);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_auth_lifetime_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->get_state(this->ike_sa) == IKE_ESTABLISHED)
	{
		add_auth_lifetime(this, message);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_ike_auth_lifetime_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->get_state(this->ike_sa) == IKE_ESTABLISHED)
	{
		process_payloads(this, message);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_auth_lifetime_t *this)
{
	return TASK_IKE_AUTH_LIFETIME;
}

METHOD(task_t, migrate, void,
	private_ike_auth_lifetime_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_auth_lifetime_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_auth_lifetime_t *ike_auth_lifetime_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_auth_lifetime_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
