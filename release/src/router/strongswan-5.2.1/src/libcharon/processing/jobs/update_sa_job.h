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

/**
 * @defgroup update_sa_job update_sa_job
 * @{ @ingroup cjobs
 */

#ifndef UPDATE_SA_JOB_H_
#define UPDATE_SA_JOB_H_

typedef struct update_sa_job_t update_sa_job_t;

#include <library.h>
#include <networking/host.h>
#include <processing/jobs/job.h>

/**
 * Update the addresses of an IKE and its CHILD_SAs.
 */
struct update_sa_job_t {

	/**
	 * implements job_t interface
	 */
	job_t job_interface;
};

/**
 * Creates a job to update IKE and CHILD_SA addresses.
 *
 * @param reqid			reqid of the CHILD_SA
 * @param new			new address and port
 * @return				update_sa_job_t object
 */
update_sa_job_t *update_sa_job_create(u_int32_t reqid, host_t *new);

#endif /** UPDATE_SA_JOB_H_ @}*/
