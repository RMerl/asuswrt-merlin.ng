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

#include "isakmp_delete.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>

typedef struct private_isakmp_delete_t private_isakmp_delete_t;

/**
 * Private members of a isakmp_delete_t task.
 */
struct private_isakmp_delete_t {

	/**
	 * Public methods and task_t interface.
	 */
	isakmp_delete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;
};

METHOD(task_t, build_i, status_t,
	private_isakmp_delete_t *this, message_t *message)
{
	delete_payload_t *delete_payload;
	ike_sa_id_t *id;

	DBG0(DBG_IKE, "deleting IKE_SA %s[%d] between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));

	delete_payload = delete_payload_create(PLV1_DELETE, PROTO_IKE);
	id = this->ike_sa->get_id(this->ike_sa);
	delete_payload->set_ike_spi(delete_payload, id->get_initiator_spi(id),
								id->get_responder_spi(id));
	message->add_payload(message, (payload_t*)delete_payload);

	DBG1(DBG_IKE, "sending DELETE for IKE_SA %s[%d]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa));

	this->ike_sa->set_state(this->ike_sa, IKE_DELETING);
	charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_isakmp_delete_t *this, message_t *message)
{
	return FAILED;
}

METHOD(task_t, process_r, status_t,
	private_isakmp_delete_t *this, message_t *message)
{
	DBG1(DBG_IKE, "received DELETE for IKE_SA %s[%d]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa));
	DBG0(DBG_IKE, "deleting IKE_SA %s[%d] between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));

	if (this->ike_sa->get_state(this->ike_sa) == IKE_ESTABLISHED)
	{
		this->ike_sa->set_state(this->ike_sa, IKE_DELETING);
		this->ike_sa->reestablish(this->ike_sa);
	}
	this->ike_sa->set_state(this->ike_sa, IKE_DELETING);
	charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
	return DESTROY_ME;
}

METHOD(task_t, build_r, status_t,
	private_isakmp_delete_t *this, message_t *message)
{
	return FAILED;
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_delete_t *this)
{
	return TASK_ISAKMP_DELETE;
}

METHOD(task_t, migrate, void,
	private_isakmp_delete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_isakmp_delete_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
isakmp_delete_t *isakmp_delete_create(ike_sa_t *ike_sa, bool initiator)
{
	private_isakmp_delete_t *this;

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
