/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "ike_redirect.h"

#include <daemon.h>
#include <processing/jobs/delete_ike_sa_job.h>

typedef struct private_ike_redirect_t private_ike_redirect_t;

/**
 * Private members
 */
struct private_ike_redirect_t {

	/**
	 * Public interface
	 */
	ike_redirect_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * Gateway ID to redirect to
	 */
	identification_t *gateway;
};

METHOD(task_t, build_i, status_t,
	private_ike_redirect_t *this, message_t *message)
{
	chunk_t data;

	DBG1(DBG_IKE, "redirecting peer to %Y", this->gateway);
	data = redirect_data_create(this->gateway, chunk_empty);
	message->add_notify(message, FALSE, REDIRECT, data);
	chunk_free(&data);
	this->ike_sa->set_condition(this->ike_sa, COND_REDIRECTED, TRUE);
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_redirect_t *this, message_t *message)
{
	notify_payload_t *notify;
	identification_t *to;

	notify = message->get_notify(message, REDIRECT);
	if (!notify)
	{
		return SUCCESS;
	}

	to = redirect_data_parse(notify->get_notification_data(notify), NULL);
	if (!to)
	{
		DBG1(DBG_IKE, "received invalid REDIRECT notify");
	}
	else
	{
		this->ike_sa->handle_redirect(this->ike_sa, to);
		to->destroy(to);
	}
	return SUCCESS;
}

METHOD(task_t, build_r, status_t,
	private_ike_redirect_t *this, message_t *message)
{
	/* not called because SUCCESS is returned above */
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_ike_redirect_t *this, message_t *message)
{
	delete_ike_sa_job_t *job;

	/* if the peer does not delete the SA we do so after a while */
	job = delete_ike_sa_job_create(this->ike_sa->get_id(this->ike_sa), TRUE);
	lib->scheduler->schedule_job(lib->scheduler, (job_t*)job,
					lib->settings->get_int(lib->settings,
							"%s.half_open_timeout", HALF_OPEN_IKE_SA_TIMEOUT,
							lib->ns));
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_redirect_t *this)
{
	return TASK_IKE_REDIRECT;
}

METHOD(task_t, migrate, void,
	private_ike_redirect_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_redirect_t *this)
{
	DESTROY_IF(this->gateway);
	free(this);
}

/*
 * Described in header.
 */
ike_redirect_t *ike_redirect_create(ike_sa_t *ike_sa, identification_t *to)
{
	private_ike_redirect_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.build = _build_r,
				.process = _process_r,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
	);

	if (to)
	{
		this->gateway = to->clone(to);
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}

	return &this->public;
}
