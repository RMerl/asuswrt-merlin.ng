/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup tnc_ifmap_renew_session_job tnc_ifmap_renew_session_job
 * @{ @ingroup tnc_ifmap
 */

#ifndef TNC_IFMAP_RENEW_SESSION_JOB_H_
#define TNC_IFMAP_RENEW_SESSION_JOB_H_

typedef struct tnc_ifmap_renew_session_job_t tnc_ifmap_renew_session_job_t;

#include "tnc_ifmap_soap.h"

#include <library.h>
#include <processing/jobs/job.h>

/**
 * Job periodically sending an IF-MAP RenewSession request.
 */
struct tnc_ifmap_renew_session_job_t {

	/**
	 * implements job_t interface
	 */
	job_t job_interface;
};

/**
 * Creates an tnc_ifmap_renew_session job.
 *
 * @param ifmap		TNC IF-MAP object
 * @param reschedule	reschedule time in seconds
 */
tnc_ifmap_renew_session_job_t *tnc_ifmap_renew_session_job_create(
								tnc_ifmap_soap_t *ifmap, uint32_t reschedule);

#endif /** TNC_IFMAP_RENEW_SESSION_JOB_H_ @}*/
