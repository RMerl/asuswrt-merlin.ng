/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup retry_initiate_job retry_initiate_job
 * @{ @ingroup cjobs
 */

#ifndef RETRY_INITIATE_JOB_H_
#define RETRY_INITIATE_JOB_H_

typedef struct retry_initiate_job_t retry_initiate_job_t;

#include <library.h>
#include <sa/ike_sa_id.h>
#include <processing/jobs/job.h>

/**
 * This job retries initiating an IKE_SA in case of e.g. a failed DNS lookup.
 */
struct retry_initiate_job_t {
	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a retry_initiate_job_t object.
 *
 * @param ike_sa_id		ID of the IKE_SA to initiate
 * @return				retry_initiate_job_t object
 */
retry_initiate_job_t *retry_initiate_job_create(ike_sa_id_t *ike_sa_id);

#endif /** RETRY_INITIATE_JOB_H_ @}*/
