/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "ike_establish.h"

#include <daemon.h>

typedef struct private_ike_establish_t private_ike_establish_t;

/**
 * Private members.
 */
struct private_ike_establish_t {

	/**
	 * Public interface.
	 */
	ike_establish_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;
};

METHOD(task_t, build_i, status_t,
	private_ike_establish_t *this, message_t *message)
{
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_establish_t *this, message_t *message)
{
	return NEED_MORE;
}

/**
 * Mark the IKE_SA as established, notify listeners.
 */
static void establish(private_ike_establish_t *this)
{
	DBG0(DBG_IKE, "IKE_SA %s[%d] established between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));
	this->ike_sa->set_state(this->ike_sa, IKE_ESTABLISHED);
	charon->bus->ike_updown(charon->bus, this->ike_sa, TRUE);
}

METHOD(task_t, build_r, status_t,
	private_ike_establish_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
	{
		establish(this);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_ike_establish_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
	{
		establish(this);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_establish_t *this)
{
	return TASK_IKE_ESTABLISH;
}

METHOD(task_t, migrate, void,
	private_ike_establish_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_establish_t *this)
{
	free(this);
}

/*
 * Described in header
 */
ike_establish_t *ike_establish_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_establish_t *this;

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
