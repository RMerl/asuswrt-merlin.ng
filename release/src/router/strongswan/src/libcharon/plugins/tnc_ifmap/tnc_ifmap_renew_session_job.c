/*
 * Copyright (C) 2013 Andreas Steffen
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

#include <stdlib.h>

#include "tnc_ifmap_renew_session_job.h"

#include <daemon.h>


typedef struct private_tnc_ifmap_renew_session_job_t private_tnc_ifmap_renew_session_job_t;

/**
 * Private data
 */
struct private_tnc_ifmap_renew_session_job_t {

	/**
	 * public tnc_ifmap_renew_session_job_t interface
	 */
	tnc_ifmap_renew_session_job_t public;

	/**
	 * TNC IF-MAP 2.0 SOAP interface
	 */
	tnc_ifmap_soap_t *ifmap;

	/**
	 * Reschedule time interval in seconds
	 */
	uint32_t reschedule;
};

METHOD(job_t, destroy, void,
	private_tnc_ifmap_renew_session_job_t *this)
{
	this->ifmap->destroy(this->ifmap);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_tnc_ifmap_renew_session_job_t *this)
{
	char *session_id;

	if (this->ifmap->orphaned(this->ifmap))
	{
		session_id = this->ifmap->get_session_id(this->ifmap);
		DBG2(DBG_TNC, "removing orphaned ifmap renewSession job for '%s'",
					   session_id);
		return JOB_REQUEUE_NONE;
	}
	else
	{
		if (!this->ifmap->renewSession(this->ifmap))
		{
			DBG1(DBG_TNC, "sending ifmap renewSession failed");
			/* TODO take some action */
		}
		return JOB_RESCHEDULE(this->reschedule);
	}
}

METHOD(job_t, get_priority, job_priority_t,
	private_tnc_ifmap_renew_session_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
tnc_ifmap_renew_session_job_t *tnc_ifmap_renew_session_job_create(
								tnc_ifmap_soap_t *ifmap, uint32_t reschedule)
{
	private_tnc_ifmap_renew_session_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.ifmap = ifmap,
		.reschedule = reschedule,
	);

	return &this->public;
}
