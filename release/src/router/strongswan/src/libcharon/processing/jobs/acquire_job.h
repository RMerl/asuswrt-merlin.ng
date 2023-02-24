/*
 * Copyright (C) 2006 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup acquire_job acquire_job
 * @{ @ingroup cjobs
 */

#ifndef ACQUIRE_JOB_H_
#define ACQUIRE_JOB_H_

typedef struct acquire_job_t acquire_job_t;

#include <library.h>
#include <kernel/kernel_interface.h>
#include <processing/jobs/job.h>

/**
 * Class representing an ACQUIRE Job.
 *
 * This job initiates a CHILD SA on kernel request.
 */
struct acquire_job_t {
	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a job of type ACQUIRE.
 *
 * @param reqid     reqid of the triggered policy
 * @param data		data from the acquire
 * @return			acquire_job_t object
 */
acquire_job_t *acquire_job_create(uint32_t reqid, kernel_acquire_data_t *data);

#endif /** REKEY_CHILD_SA_JOB_H_ @}*/
