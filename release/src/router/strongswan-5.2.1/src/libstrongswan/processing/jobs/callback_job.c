/*
 * Copyright (C) 2009-2012 Tobias Brunner
 * Copyright (C) 2007-2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "callback_job.h"

#include <threading/thread.h>
#include <threading/condvar.h>
#include <threading/semaphore.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>

typedef struct private_callback_job_t private_callback_job_t;

/**
 * Private data of an callback_job_t Object.
 */
struct private_callback_job_t {

	/**
	 * Public callback_job_t interface.
	 */
	callback_job_t public;

	/**
	 * Callback to call on execution
	 */
	callback_job_cb_t callback;

	/**
	 * parameter to supply to callback
	 */
	void *data;

	/**
	 * cleanup function for data
	 */
	callback_job_cleanup_t cleanup;

	/**
	 * cancel function
	 */
	callback_job_cancel_t cancel;

	/**
	 * Priority of this job
	 */
	job_priority_t prio;
};

METHOD(job_t, destroy, void,
	private_callback_job_t *this)
{
	if (this->cleanup)
	{
		this->cleanup(this->data);
	}
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_callback_job_t *this)
{
	return this->callback(this->data);
}

METHOD(job_t, cancel, bool,
	private_callback_job_t *this)
{
	return this->cancel(this->data);
}

METHOD(job_t, get_priority, job_priority_t,
	private_callback_job_t *this)
{
	return this->prio;
}

/*
 * Described in header.
 */
callback_job_t *callback_job_create_with_prio(callback_job_cb_t cb, void *data,
				callback_job_cleanup_t cleanup, callback_job_cancel_t cancel,
				job_priority_t prio)
{
	private_callback_job_t *this;

	INIT(this,
		.public = {
			.job = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.callback = cb,
		.data = data,
		.cleanup = cleanup,
		.cancel = cancel,
		.prio = prio,
	);

	if (cancel)
	{
		this->public.job.cancel = _cancel;
	}

	return &this->public;
}

/*
 * Described in header.
 */
callback_job_t *callback_job_create(callback_job_cb_t cb, void *data,
									callback_job_cleanup_t cleanup,
									callback_job_cancel_t cancel)
{
	return callback_job_create_with_prio(cb, data, cleanup, cancel,
										 JOB_PRIO_MEDIUM);
}
