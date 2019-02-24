/*
 * Copyright (C) 2010 Martin Willi
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

#include "inactivity_job.h"

#include <daemon.h>

typedef struct private_inactivity_job_t private_inactivity_job_t;

/**
 * Private data of an inactivity_job_t object.
 */
struct private_inactivity_job_t {

	/**
	 * Public inactivity_job_t interface.
	 */
	inactivity_job_t public;

	/**
	 * Unique CHILD_SA identifier to check
	 */
	uint32_t id;

	/**
	 * Inactivity timeout
	 */
	uint32_t timeout;

	/**
	 * Close IKE_SA if last remaining CHILD inactive?
	 */
	bool close_ike;
};

METHOD(job_t, destroy, void,
	private_inactivity_job_t *this)
{
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_inactivity_job_t *this)
{
	ike_sa_t *ike_sa;
	uint32_t reschedule = 0;

	ike_sa = charon->child_sa_manager->checkout_by_id(charon->child_sa_manager,
													  this->id, NULL);
	if (ike_sa)
	{
		enumerator_t *enumerator;
		child_sa_t *child_sa;
		uint32_t delete = 0;
		protocol_id_t proto = 0;
		int children = 0;
		status_t status = SUCCESS;

		enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, &child_sa))
		{
			if (child_sa->get_unique_id(child_sa) == this->id)
			{
				time_t in, out, install, diff;

				child_sa->get_usestats(child_sa, TRUE, &in, NULL, NULL);
				child_sa->get_usestats(child_sa, FALSE, &out, NULL, NULL);
				install = child_sa->get_installtime(child_sa);

				diff = time_monotonic(NULL) - max(max(in, out), install);

				if (diff >= this->timeout)
				{
					delete = child_sa->get_spi(child_sa, TRUE);
					proto = child_sa->get_protocol(child_sa);
				}
				else
				{
					reschedule = this->timeout - diff;
				}
			}
			children++;
		}
		enumerator->destroy(enumerator);

		if (delete)
		{
			if (children == 1 && this->close_ike)
			{
				DBG1(DBG_JOB, "deleting IKE_SA after %d seconds "
					 "of CHILD_SA inactivity", this->timeout);
				status = ike_sa->delete(ike_sa, FALSE);
			}
			else
			{
				DBG1(DBG_JOB, "deleting CHILD_SA after %d seconds "
					 "of inactivity", this->timeout);
				status = ike_sa->delete_child_sa(ike_sa, proto, delete, FALSE);
			}
		}
		if (status == DESTROY_ME)
		{
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
	if (reschedule)
	{
		return JOB_RESCHEDULE(reschedule);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_inactivity_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/**
 * See header
 */
inactivity_job_t *inactivity_job_create(uint32_t unique_id, uint32_t timeout,
										bool close_ike)
{
	private_inactivity_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.id = unique_id,
		.timeout = timeout,
		.close_ike = close_ike,
	);

	return &this->public;
}
