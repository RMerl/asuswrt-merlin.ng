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

/**
 * @defgroup redirect_job redirect_job
 * @{ @ingroup cjobs
 */

#ifndef REDIRECT_JOB_H_
#define REDIRECT_JOB_H_

typedef struct redirect_job_t redirect_job_t;

#include <library.h>
#include <sa/ike_sa_id.h>
#include <processing/jobs/job.h>

/**
 * Job used to redirect an IKE_SA.
 */
struct redirect_job_t {

	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a job to redirect an IKE_SA.
 *
 * @param ike_sa_id			id of the IKE_SA to redirect (cloned)
 * @param gateway			gateway identity (IP or FQDN) of target (cloned)
 * @return					created redirect_job_t object
 */
redirect_job_t *redirect_job_create(ike_sa_id_t *ike_sa_id,
									identification_t *gateway);

#endif /** REDIRECT_JOB_H_ @}*/
