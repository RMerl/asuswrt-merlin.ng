/*
 * Copyright (C) 2008 Martin Willi
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

#include "user.h"

typedef struct private_user_t private_user_t;

/**
 * private data of user
 */
struct private_user_t {

	/**
	 * public functions
	 */
	user_t public;

	/**
	 * user id, if we are logged in; otherwise 0
	 */
	u_int user;
};

METHOD(user_t, set_user, void,
	private_user_t *this, u_int id)
{
	this->user = id;
}

METHOD(user_t, get_user, u_int,
	private_user_t *this)
{
	return this->user;
}

METHOD(fast_context_t, destroy, void,
	private_user_t *this)
{
	free(this);
}

/*
 * see header file
 */
user_t *user_create(void *param)
{
	private_user_t *this;

	INIT(this,
		.public = {
			.set_user = _set_user,
			.get_user = _get_user,
			.context = {
				.destroy = _destroy,
			},
		},
	);

	return &this->public;
}
