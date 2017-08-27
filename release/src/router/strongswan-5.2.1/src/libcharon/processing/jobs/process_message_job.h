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
 * @defgroup process_message_job process_message_job
 * @{ @ingroup cjobs
 */

#ifndef PROCESS_MESSAGE_JOB_H_
#define PROCESS_MESSAGE_JOB_H_

typedef struct process_message_job_t process_message_job_t;

#include <library.h>
#include <encoding/message.h>
#include <processing/jobs/job.h>

/**
 * Class representing an PROCESS_MESSAGE job.
 */
struct process_message_job_t {
	/**
	 * implements job_t interface
	 */
	job_t job_interface;
};

/**
 * Creates a job of type PROCESS_MESSAGE.
 *
 * @param message		message to process
 * @return				created process_message_job_t object
 */
process_message_job_t *process_message_job_create(message_t *message);

#endif /** PROCESS_MESSAGE_JOB_H_ @}*/
