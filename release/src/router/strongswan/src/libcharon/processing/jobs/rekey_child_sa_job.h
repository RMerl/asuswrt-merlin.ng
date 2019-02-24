/*
 * Copyright (C) 2006 Martin Willi
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
 * @defgroup rekey_child_sa_job rekey_child_sa_job
 * @{ @ingroup cjobs
 */

#ifndef REKEY_CHILD_SA_JOB_H_
#define REKEY_CHILD_SA_JOB_H_

typedef struct rekey_child_sa_job_t rekey_child_sa_job_t;

#include <library.h>
#include <sa/ike_sa_id.h>
#include <processing/jobs/job.h>
#include <crypto/proposal/proposal.h>

/**
 * Class representing an REKEY_CHILD_SA Job.
 *
 * This job initiates the rekeying of a CHILD SA.
 */
struct rekey_child_sa_job_t {
	/**
	 * The job_t interface.
	 */
	job_t job_interface;
};

/**
 * Creates a job of type REKEY_CHILD_SA.
 *
 * @param protocol	protocol of the CHILD_SA
 * @param spi		security parameter index of the CHILD_SA
 * @param dst		SA destination address
 * @return			rekey_child_sa_job_t object
 */
rekey_child_sa_job_t *rekey_child_sa_job_create(protocol_id_t protocol,
												uint32_t spi, host_t *dst);

#endif /** REKEY_CHILD_SA_JOB_H_ @}*/
