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

/**
 * @defgroup dpd_timeout_job dpd_timeout_job
 * @{ @ingroup cjobs
 */

#ifndef DPD_TIMEOUT_JOB_H_
#define DPD_TIMEOUT_JOB_H_

typedef struct dpd_timeout_job_t dpd_timeout_job_t;

#include <library.h>
#include <processing/jobs/job.h>
#include <sa/ike_sa_id.h>

/**
 * Job enforcing DPD timeout.
 *
 * This job detects if a DPD response has been received during the DPD timeout
 * interval, and if not, enforced the DPD action.
 */
struct dpd_timeout_job_t {

	/**
	 * implements job_t interface
	 */
	job_t job_interface;
};

/**
 * Creates a DPD timeout job.
 *
 * @param ike_sa_id		ike_sa_id_t, gets cloned
 * @return				initiate_ike_sa_job_t object
 */
dpd_timeout_job_t *dpd_timeout_job_create(ike_sa_id_t *ike_sa_id);

#endif /** DPD_TIMEOUT_JOB_H_ @}*/
