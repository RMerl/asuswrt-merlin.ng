/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup processor processor
 * @{ @ingroup processing
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

typedef struct processor_t processor_t;

#include <stdlib.h>

#include <library.h>
#include <processing/jobs/job.h>

/**
 * The processor uses threads to process queued jobs.
 */
struct processor_t {

	/**
	 * Get the total number of threads used by the processor.
	 *
	 * @return				size of thread pool
	 */
	u_int (*get_total_threads) (processor_t *this);

	/**
	 * Get the number of threads currently waiting for work.
	 *
	 * @return				number of idle threads
	 */
	u_int (*get_idle_threads) (processor_t *this);

	/**
	 * Get the number of threads currently working, per priority class.
	 *
	 * @param				priority to check
	 * @return				number of threads in priority working
	 */
	u_int (*get_working_threads)(processor_t *this, job_priority_t prio);

	/**
	 * Get the number of queued jobs for a specified priority.
	 *
	 * @param prio			priority class to get job load for
	 * @return				number of items in queue
	 */
	u_int (*get_job_load) (processor_t *this, job_priority_t prio);

	/**
	 * Adds a job to the queue.
	 *
	 * This function is non blocking and adds a job_t to the queue.
	 *
	 * @param job			job to add to the queue
	 */
	void (*queue_job) (processor_t *this, job_t *job);

	/**
	 * Directly execute a job with an idle worker thread.
	 *
	 * If no idle thread is available, the job gets executed by the calling
	 * thread.
	 *
	 * @param job			job, gets destroyed
	 */
	void (*execute_job)(processor_t *this, job_t *job);

	/**
	 * Set the number of threads to use in the processor.
	 *
	 * If the number of threads is smaller than number of currently running
	 * threads, thread count is decreased. Use 0 to disable the processor.
	 *
	 * This call does not block and wait for threads to terminate if the number
	 * of threads is reduced.  Instead use cancel() for that during shutdown.
	 *
	 * @param count			number of threads to allocate
	 */
	void (*set_threads)(processor_t *this, u_int count);

	/**
	 * Sets the number of threads to 0 and cancels all blocking jobs, then waits
	 * for all threads to be terminated.
	 */
	void (*cancel)(processor_t *this);

	/**
	 * Destroy a processor object.
	 */
	void (*destroy) (processor_t *processor);
};

/**
 * Create the thread pool without any threads.
 *
 * Use the set_threads method to start processing jobs.
 *
 * @return					processor_t object
 */
processor_t *processor_create();

#endif /** PROCESSOR_H_ @}*/
