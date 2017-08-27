/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "dpd_timeout_job.h"

#include <sa/ike_sa.h>
#include <daemon.h>


typedef struct private_dpd_timeout_job_t private_dpd_timeout_job_t;

/**
 * Private data
 */
struct private_dpd_timeout_job_t {

	/**
	 * public dpd_timeout_job_t interface
	 */
	dpd_timeout_job_t public;

	/**
	 * IKE_SA identifier
	 */
	ike_sa_id_t *ike_sa_id;

	/**
	 * Timestamp of first DPD check
	 */
	time_t check;
};

METHOD(job_t, destroy, void,
	private_dpd_timeout_job_t *this)
{
	this->ike_sa_id->destroy(this->ike_sa_id);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_dpd_timeout_job_t *this)
{
	time_t use_time, current;
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
											  this->ike_sa_id);
	if (ike_sa)
	{
		use_time = ike_sa->get_statistic(ike_sa, STAT_INBOUND);

		enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, &child_sa))
		{
			child_sa->get_usestats(child_sa, TRUE, &current, NULL, NULL);
			use_time = max(use_time, current);
		}
		enumerator->destroy(enumerator);

		/* check if no incoming packet during timeout, reestablish SA */
		if (use_time < this->check)
		{
			DBG1(DBG_JOB, "DPD check timed out, enforcing DPD action");
			charon->bus->alert(charon->bus, ALERT_RETRANSMIT_SEND_TIMEOUT, NULL);
			charon->bus->ike_updown(charon->bus, ike_sa, FALSE);
			ike_sa->reestablish(ike_sa);
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_dpd_timeout_job_t *this)
{
	return JOB_PRIO_HIGH;
}

/*
 * Described in header
 */
dpd_timeout_job_t *dpd_timeout_job_create(ike_sa_id_t *ike_sa_id)
{
	private_dpd_timeout_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.ike_sa_id = ike_sa_id->clone(ike_sa_id),
		.check = time_monotonic(NULL),
	);

	return &this->public;
}
