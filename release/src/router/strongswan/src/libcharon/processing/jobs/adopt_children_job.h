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
 * @defgroup adopt_children_job adopt_children_job
 * @{ @ingroup cjobs
 */

#ifndef ADOPT_CHILDREN_JOB_H_
#define ADOPT_CHILDREN_JOB_H_

#include <library.h>
#include <processing/jobs/job.h>
#include <sa/ike_sa_id.h>
#include <sa/task.h>

typedef struct adopt_children_job_t adopt_children_job_t;

/**
 * Job adopting children after IKEv1 reauthentication from old SA.
 */
struct adopt_children_job_t {

	/**
	 * Implements job_t.
	 */
	job_t job_interface;

	/**
	 * Queue a job for execution after completing migration.
	 *
	 * @param task			task to queue for execution
	 */
	void (*queue_task)(adopt_children_job_t *this, task_t *task);
};

/**
 * Create a adopt_children_job instance.
 *
 * @param id		ike_sa_id_t of old ISAKMP SA to adopt children from
 * @return			job
 */
adopt_children_job_t *adopt_children_job_create(ike_sa_id_t *id);

#endif /** ADOPT_CHILDREN_JOB_H_ @}*/
