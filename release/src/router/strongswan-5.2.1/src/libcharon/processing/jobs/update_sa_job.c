/*
 * Copyright (C) 2008 Martin Willi
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

#include <stdlib.h>

#include "update_sa_job.h"

#include <sa/ike_sa.h>
#include <daemon.h>


typedef struct private_update_sa_job_t private_update_sa_job_t;

/**
 * Private data of an update_sa_job_t Object
 */
struct private_update_sa_job_t {
	/**
	 * public update_sa_job_t interface
	 */
	update_sa_job_t public;

	/**
	 * reqid of the CHILD_SA
	 */
	u_int32_t reqid;

	/**
	 * New SA address and port
	 */
	host_t *new;
};

METHOD(job_t, destroy, void,
	private_update_sa_job_t *this)
{
	this->new->destroy(this->new);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_update_sa_job_t *this)
{
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout_by_id(charon->ike_sa_manager,
													this->reqid, TRUE);
	if (ike_sa == NULL)
	{
		DBG1(DBG_JOB, "CHILD_SA with reqid %d not found for update", this->reqid);
	}
	else
	{
		ike_sa->update_hosts(ike_sa, NULL, this->new, FALSE);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_update_sa_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
update_sa_job_t *update_sa_job_create(u_int32_t reqid, host_t *new)
{
	private_update_sa_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.reqid = reqid,
		.new = new,
	);

	return &this->public;
}

