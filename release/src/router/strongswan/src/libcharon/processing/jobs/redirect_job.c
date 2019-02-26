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

#include <daemon.h>

#include "redirect_job.h"

typedef struct private_redirect_job_t private_redirect_job_t;

/**
 * Private data
 */
struct private_redirect_job_t {

	/**
	 * Public interface
	 */
	redirect_job_t public;

	/**
	 * ID of the IKE_SA to redirect
	 */
	ike_sa_id_t *ike_sa_id;

	/**
	 * Target gateway identity
	 */
	identification_t *gateway;
};


METHOD(job_t, destroy, void,
	private_redirect_job_t *this)
{
	this->ike_sa_id->destroy(this->ike_sa_id);
	this->gateway->destroy(this->gateway);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_redirect_job_t *this)
{
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
											  this->ike_sa_id);
	if (ike_sa)
	{
		if (ike_sa->get_state(ike_sa) == IKE_PASSIVE)
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
			return JOB_REQUEUE_NONE;
		}
		if (ike_sa->redirect(ike_sa, this->gateway) == DESTROY_ME)
		{
			charon->ike_sa_manager->checkin_and_destroy(
											charon->ike_sa_manager, ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_redirect_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
redirect_job_t *redirect_job_create(ike_sa_id_t *ike_sa_id,
									identification_t *gateway)
{
	private_redirect_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.ike_sa_id = ike_sa_id->clone(ike_sa_id),
		.gateway = gateway->clone(gateway),
	);

	return &(this->public);
}
