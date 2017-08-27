/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup start_action_job start_action_job
 * @{ @ingroup cjobs
 */

#ifndef START_ACTION_JOB_H_
#define START_ACTION_JOB_H_

typedef struct start_action_job_t start_action_job_t;

#include <library.h>
#include <processing/jobs/job.h>

/**
 * Class representing a start_action Job.
 *
 * This job handles all child configurations stored in an [SQL database]
 * backend according to their start_action field (start, route, none).
 */
struct start_action_job_t {
	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a job of type start_action.
 *
 * @return			start_action_job_t object
 */
start_action_job_t *start_action_job_create(void);

#endif /** START_ACTION_JOB_H_ @}*/
