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

#include "ike_reauth_complete.h"

#include <daemon.h>
#include <processing/jobs/delete_ike_sa_job.h>


typedef struct private_ike_reauth_complete_t private_ike_reauth_complete_t;

/**
 * Private members of a ike_reauth_complete_t task.
 */
struct private_ike_reauth_complete_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_reauth_complete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Reauthenticated IKE_SA identifier
	 */
	ike_sa_id_t *id;
};

METHOD(task_t, build_i, status_t,
	private_ike_reauth_complete_t *this, message_t *message)
{
	message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
	lib->processor->queue_job(lib->processor,
							  (job_t*)delete_ike_sa_job_create(this->id, TRUE));
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_ike_reauth_complete_t *this, message_t *message)
{
	return DESTROY_ME;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_reauth_complete_t *this)
{
	return TASK_IKE_REAUTH_COMPLETE;
}

METHOD(task_t, migrate, void,
	private_ike_reauth_complete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_reauth_complete_t *this)
{
	this->id->destroy(this->id);
	free(this);
}

/*
 * Described in header.
 */
ike_reauth_complete_t *ike_reauth_complete_create(ike_sa_t *ike_sa,
												  ike_sa_id_t *id)
{
	private_ike_reauth_complete_t *this;

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
		.id = id->clone(id),
	);

	return &this->public;
}
