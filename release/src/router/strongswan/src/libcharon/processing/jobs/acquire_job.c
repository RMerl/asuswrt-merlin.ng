/*
 * Copyright (C) 2006-2009 Martin Willi
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
	 * reqid of the child to rekey
	 */
	uint32_t reqid;

	/**
	 * acquired source traffic selector
	 */
	traffic_selector_t *src_ts;

	/**
	 * acquired destination traffic selector
	 */
	traffic_selector_t *dst_ts;
};

METHOD(job_t, destroy, void,
	private_acquire_job_t *this)
{
	DESTROY_IF(this->src_ts);
	DESTROY_IF(this->dst_ts);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_acquire_job_t *this)
{
	charon->traps->acquire(charon->traps, this->reqid,
						   this->src_ts, this->dst_ts);
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
acquire_job_t *acquire_job_create(uint32_t reqid,
								  traffic_selector_t *src_ts,
								  traffic_selector_t *dst_ts)
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
		.src_ts = src_ts,
		.dst_ts = dst_ts,
	);

	return &this->public;
}

