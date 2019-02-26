/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup initiate_tasks_job initiate_tasks_job
 * @{ @ingroup cjobs
 */

#ifndef INITIATE_TASKS_JOB_H_
#define INITIATE_TASKS_JOB_H_

typedef struct initiate_tasks_job_t initiate_tasks_job_t;

#include <library.h>
#include <processing/jobs/job.h>
#include <sa/ike_sa_id.h>

/**
 * Job triggering initiation of any queued IKE_SA tasks.
 */
struct initiate_tasks_job_t {

	/**
	 * Implements job_t interface
	 */
	job_t job_interface;
};

/**
 * Creates a job to trigger IKE_SA task initiation.
 *
 * @param ike_sa_id		ID of IKE_SA to trigger tasks for (gets cloned)
 * @return				job instance
 */
initiate_tasks_job_t *initiate_tasks_job_create(ike_sa_id_t *ike_sa_id);

#endif /** INITIATE_TASKS_JOB_H_ @}*/
