/*
 * Copyright (C) 2010 Martin Willi
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
 * @defgroup inactivity_job inactivity_job
 * @{ @ingroup cjobs
 */

#ifndef INACTIVITY_JOB_H_
#define INACTIVITY_JOB_H_

#include <library.h>
#include <processing/jobs/job.h>

typedef struct inactivity_job_t inactivity_job_t;

/**
 * Job checking for inactivity of CHILD_SA to close them.
 *
 * The inactivity job reschedules itself to check CHILD_SAs prediodically.
 */
struct inactivity_job_t {

	/**
	 * Implements job_t.
	 */
	job_t job_interface;
};

/**
 * Create a inactivity_job instance.
 *
 * @param unique_id	unique CHILD_SA identifier to check for inactivity
 * @param timeout	inactivity timeout in s
 * @param close_ike	close IKE_SA if the last remaining CHILD_SA is inactive?
 * @return			inactivity checking job
 */
inactivity_job_t *inactivity_job_create(uint32_t unique_id, uint32_t timeout,
										bool close_ike);

#endif /** INACTIVITY_JOB_H_ @}*/
