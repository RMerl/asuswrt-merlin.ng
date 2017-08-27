/*
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup retransmit_job retransmit_job
 * @{ @ingroup cjobs
 */

#ifndef RETRANSMIT_JOB_H_
#define RETRANSMIT_JOB_H_

typedef struct retransmit_job_t retransmit_job_t;

#include <library.h>
#include <processing/jobs/job.h>
#include <sa/ike_sa_id.h>

/**
 * Class representing an retransmit Job.
 *
 * This job is scheduled every time a request is sent over the
 * wire. If the response to the request is not received at schedule
 * time, the retransmission will be initiated.
 */
struct retransmit_job_t {
	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a job of type retransmit.
 *
 * @param message_id		message_id of the request to resend
 * @param ike_sa_id			identification of the ike_sa as ike_sa_id_t
 * @return					retransmit_job_t object
 */
retransmit_job_t *retransmit_job_create(u_int32_t message_id,
										ike_sa_id_t *ike_sa_id);

#endif /** RETRANSMIT_JOB_H_ @}*/
