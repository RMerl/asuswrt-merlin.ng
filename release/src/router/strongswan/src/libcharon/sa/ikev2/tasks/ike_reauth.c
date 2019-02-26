/*
 * Copyright (C) 2006-2008 Martin Willi
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

#include "ike_reauth.h"

#include <daemon.h>
#include <sa/ikev2/tasks/ike_delete.h>


typedef struct private_ike_reauth_t private_ike_reauth_t;

/**
 * Private members of a ike_reauth_t task.
 */
struct private_ike_reauth_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_reauth_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * reused ike_delete task
	 */
	ike_delete_t *ike_delete;
};

METHOD(task_t, build_i, status_t,
	private_ike_reauth_t *this, message_t *message)
{
	return this->ike_delete->task.build(&this->ike_delete->task, message);
}

METHOD(task_t, process_i, status_t,
	private_ike_reauth_t *this, message_t *message)
{
	/* process delete response first */
	this->ike_delete->task.process(&this->ike_delete->task, message);

	/* reestablish the IKE_SA with all children */
	if (this->ike_sa->reestablish(this->ike_sa) != SUCCESS)
	{
		DBG1(DBG_IKE, "reauthenticating IKE_SA failed");
		return FAILED;
	}

	/* we always destroy the obsolete IKE_SA */
	return DESTROY_ME;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_reauth_t *this)
{
	return TASK_IKE_REAUTH;
}

METHOD(task_t, migrate, void,
	private_ike_reauth_t *this, ike_sa_t *ike_sa)
{
	this->ike_delete->task.migrate(&this->ike_delete->task, ike_sa);
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_reauth_t *this)
{
	this->ike_delete->task.destroy(&this->ike_delete->task);
	free(this);
}

/*
 * Described in header.
 */
ike_reauth_t *ike_reauth_create(ike_sa_t *ike_sa)
{
	private_ike_reauth_t *this;

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
		.ike_delete = ike_delete_create(ike_sa, TRUE),
	);

	return &this->public;
}
