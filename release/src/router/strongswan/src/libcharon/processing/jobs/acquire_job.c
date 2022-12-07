/*
 * Copyright (C) 2006-2009 Martin Willi
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

#include "acquire_job.h"

#include <daemon.h>


typedef struct private_acquire_job_t private_acquire_job_t;

/**
 * Private data of an acquire_job_t object.
 */
struct private_acquire_job_t {
	/**
	 * Public acquire_job_t interface.
	 */
	acquire_job_t public;

	/**
	 * reqid of the triggered policy
	 */
	uint32_t reqid;

	/**
	 * Data from the acquire
	 */
	kernel_acquire_data_t data;
};

METHOD(job_t, destroy, void,
	private_acquire_job_t *this)
{
	DESTROY_IF(this->data.src);
	DESTROY_IF(this->data.dst);
	DESTROY_IF(this->data.label);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_acquire_job_t *this)
{
	charon->traps->acquire(charon->traps, this->reqid, &this->data);
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_acquire_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
acquire_job_t *acquire_job_create(uint32_t reqid, kernel_acquire_data_t *data)
{
	private_acquire_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.reqid = reqid,
		.data = *data,
	);

	if (this->data.src)
	{
		this->data.src = this->data.src->clone(this->data.src);
	}
	if (this->data.dst)
	{
		this->data.dst = this->data.dst->clone(this->data.dst);
	}
	if (this->data.label)
	{
		this->data.label = this->data.label->clone(this->data.label);
	}

	return &this->public;
}
