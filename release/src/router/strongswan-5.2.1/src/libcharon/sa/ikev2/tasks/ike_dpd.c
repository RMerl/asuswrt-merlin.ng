/*
 * Copyright (C) 2007 Martin Willi
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

#include "ike_dpd.h"

#include <daemon.h>


typedef struct private_ike_dpd_t private_ike_dpd_t;

/**
 * Private members of a ike_dpd_t task.
 */
struct private_ike_dpd_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_dpd_t public;
};

METHOD(task_t, return_need_more, status_t,
	private_ike_dpd_t *this, message_t *message)
{
	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_dpd_t *this)
{
	return TASK_IKE_DPD;
}


METHOD(task_t, migrate, void,
	private_ike_dpd_t *this, ike_sa_t *ike_sa)
{

}

METHOD(task_t, destroy, void,
	private_ike_dpd_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_dpd_t *ike_dpd_create(bool initiator)
{
	private_ike_dpd_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
	);

	if (initiator)
	{
		this->public.task.build = _return_need_more;
		this->public.task.process = (void*)return_success;
	}
	else
	{
		this->public.task.build = (void*)return_success;
		this->public.task.process = _return_need_more;
	}

	return &this->public;
}
