/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "delete_ike_sa_job.h"

#include <daemon.h>

typedef struct private_delete_ike_sa_job_t private_delete_ike_sa_job_t;

/**
 * Private data of an delete_ike_sa_job_t Object
 */
struct private_delete_ike_sa_job_t {
	/**
	 * public delete_ike_sa_job_t interface
	 */
	delete_ike_sa_job_t public;

	/**
	 * ID of the ike_sa to delete
	 */
	ike_sa_id_t *ike_sa_id;

	/**
	 * Should the IKE_SA be deleted if it is in ESTABLISHED state?
	 */
	bool delete_if_established;
};


METHOD(job_t, destroy, void,
	private_delete_ike_sa_job_t *this)
{
	this->ike_sa_id->destroy(this->ike_sa_id);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_delete_ike_sa_job_t *this)
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
		if (this->delete_if_established)
		{
			if (ike_sa->delete(ike_sa) == DESTROY_ME)
			{
				charon->ike_sa_manager->checkin_and_destroy(
												charon->ike_sa_manager, ike_sa);
			}
			else
			{
				charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
			}
		}
		else
		{
			/* destroy IKE_SA only if it did not complete connecting phase */
			if (ike_sa->get_state(ike_sa) != IKE_CONNECTING)
			{
				charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
			}
			else if (ike_sa->get_version(ike_sa) == IKEV1 &&
					 ike_sa->has_condition(ike_sa, COND_ORIGINAL_INITIATOR))
			{	/* as initiator we waited for the peer to initiate e.g. an
				 * XAuth exchange, reauth the SA to eventually trigger DPD */
				DBG1(DBG_JOB, "peer did not initiate expected exchange, "
					 "reestablishing IKE_SA");
				ike_sa->reauth(ike_sa);
				charon->ike_sa_manager->checkin_and_destroy(
												charon->ike_sa_manager, ike_sa);
			}
			else
			{
				DBG1(DBG_JOB, "deleting half open IKE_SA after timeout");
				charon->bus->alert(charon->bus, ALERT_HALF_OPEN_TIMEOUT);
				charon->ike_sa_manager->checkin_and_destroy(
												charon->ike_sa_manager, ike_sa);
			}
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_delete_ike_sa_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
delete_ike_sa_job_t *delete_ike_sa_job_create(ike_sa_id_t *ike_sa_id,
											  bool delete_if_established)
{
	private_delete_ike_sa_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.ike_sa_id = ike_sa_id->clone(ike_sa_id),
		.delete_if_established = delete_if_established,
	);

	return &(this->public);
}
