/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "ike_verify_peer_cert.h"

#include <daemon.h>
#include <sa/ikev2/tasks/ike_delete.h>

typedef struct private_ike_verify_peer_cert_t private_ike_verify_peer_cert_t;

/**
 * Private members
 */
struct private_ike_verify_peer_cert_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_verify_peer_cert_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Child ike_delete task, if necessary
	 */
	ike_delete_t *ike_delete;
};

METHOD(task_t, build_i, status_t,
	private_ike_verify_peer_cert_t *this, message_t *message)
{
	if (!this->ike_sa->verify_peer_certificate(this->ike_sa))
	{
		DBG1(DBG_IKE, "peer certificate verification failed, deleting SA");
		this->ike_delete = ike_delete_create(this->ike_sa, TRUE);
		return this->ike_delete->task.build(&this->ike_delete->task, message);
	}
	DBG1(DBG_IKE, "peer certificate successfully verified");
	message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_ike_verify_peer_cert_t *this, message_t *message)
{
	if (this->ike_delete)
	{
		this->ike_delete->task.process(&this->ike_delete->task, message);
		/* try to reestablish the IKE_SA and all children */
		this->ike_sa->reestablish(this->ike_sa);
	}
	return DESTROY_ME;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_verify_peer_cert_t *this)
{
	return TASK_IKE_VERIFY_PEER_CERT;
}

METHOD(task_t, migrate, void,
	private_ike_verify_peer_cert_t *this, ike_sa_t *ike_sa)
{
	if (this->ike_delete)
	{
		this->ike_delete->task.migrate(&this->ike_delete->task, ike_sa);
	}
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_verify_peer_cert_t *this)
{
	if (this->ike_delete)
	{
		this->ike_delete->task.destroy(&this->ike_delete->task);
	}
	free(this);
}

/*
 * Described in header.
 */
ike_verify_peer_cert_t *ike_verify_peer_cert_create(ike_sa_t *ike_sa)
{
	private_ike_verify_peer_cert_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.build = _build_i,
				.process = _process_i,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
	);

	return &this->public;
}
