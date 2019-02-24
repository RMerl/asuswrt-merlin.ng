/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2007-2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup callback_job callback_job
 * @{ @ingroup jobs
 */

#ifndef CALLBACK_JOB_H_
#define CALLBACK_JOB_H_

typedef struct callback_job_t callback_job_t;

#include <library.h>
#include <processing/jobs/job.h>

/**
 * The callback function to use for the callback job.
 *
 * This is the function to use as callback for a callback job. It receives
 * a parameter supplied to the callback jobs constructor.
 *
 * @param data			param supplied to job
 * @return				requeing policy how to requeue the job
 */
typedef job_requeue_t (*callback_job_cb_t)(void *data);

/**
 * Cleanup function to use for data cleanup.
 *
 * The callback has an optional user argument which receives data. However,
 * this data may be cleaned up if it is allocated. This is the function
 * to supply to the constructor.
 *
 * @param data			param supplied to job
 */
typedef void (*callback_job_cleanup_t)(void *data);

/**
 * Cancellation function to use for the callback job.
 *
 * Optional function to be called when a job has to be canceled.
 *
 * See job_t.cancel() for details on the return value.
 *
 * @param data			param supplied to job
 * @return				TRUE if canceled, FALSE to explicitly cancel the thread
 */
typedef bool (*callback_job_cancel_t)(void *data);

/**
 * Class representing an callback Job.
 *
 * This is a special job which allows a simple callback function to
 * be executed by a thread of the thread pool. This allows simple execution
 * of asynchronous methods, without to manage threads.
 */
struct callback_job_t {

	/**
	 * The job_t interface.
	 */
	job_t job;

};

/**
 * Creates a callback job.
 *
 * The cleanup function is called when the job gets destroyed to destroy
 * the associated data.
 *
 * The cancel function is optional and should only be provided if the callback
 * function calls potentially blocking functions and/or always returns
 * JOB_REQUEUE_DIRECT.
 *
 * @param cb				callback to call from the processor
 * @param data				user data to supply to callback
 * @param cleanup			destructor for data on destruction, or NULL
 * @param cancel			function to cancel the job, or NULL
 * @return					callback_job_t object
 */
callback_job_t *callback_job_create(callback_job_cb_t cb, void *data,
									callback_job_cleanup_t cleanup,
									callback_job_cancel_t cancel);

/**
 * Creates a callback job, with priority.
 *
 * Same as callback_job_create(), but with different priorities than default.
 *
 * @param cb				callback to call from the processor
 * @param data				user data to supply to callback
 * @param cleanup			destructor for data on destruction, or NULL
 * @param cancel			function to cancel the job, or NULL
 * @param prio				job priority
 * @return					callback_job_t object
 */
callback_job_t *callback_job_create_with_prio(callback_job_cb_t cb, void *data,
				callback_job_cleanup_t cleanup, callback_job_cancel_t cancel,
				job_priority_t prio);

#endif /** CALLBACK_JOB_H_ @}*/
