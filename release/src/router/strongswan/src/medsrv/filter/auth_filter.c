/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Philip Boetschi, Adrian Doerig
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

#include "auth_filter.h"

#include <utils/debug.h>

typedef struct private_auth_filter_t private_auth_filter_t;

/**
 * private data of auth_filter
 */
struct private_auth_filter_t {
	/**
	 * public functions
	 */
	auth_filter_t public;

	/**
	 * user session
	 */
	user_t *user;

	/**
	 * database connection
	 */
	database_t *db;
};

METHOD(fast_filter_t, run, bool,
	private_auth_filter_t *this, fast_request_t *request, char *controller,
	char *action, char *p2, char *p3, char *p4, char *p5)
{
	if (this->user->get_user(this->user))
	{
		enumerator_t *query;
		char *login;

		query = this->db->query(this->db, "SELECT login FROM user WHERE id = ?",
								DB_INT, this->user->get_user(this->user),
								DB_TEXT);
		if (query && query->enumerate(query, &login))
		{
			request->set(request, "login", login);
			query->destroy(query);
			return TRUE;
		}
		DESTROY_IF(query);
		this->user->set_user(this->user, 0);
	}
	if (controller && streq(controller, "user") && action &&
		(streq(action, "add") || streq(action, "login") || streq(action, "help")))
	{	/* add/login allowed */
		return TRUE;
	}
	request->redirect(request, "user/login");
	return FALSE;
}

METHOD(fast_filter_t, destroy, void,
	private_auth_filter_t *this)
{
	free(this);
}

/*
 * see header file
 */
fast_filter_t *auth_filter_create(user_t *user, database_t *db)
{
	private_auth_filter_t *this;

	INIT(this,
		.public = {
			.filter = {
				.destroy = _destroy,
				.run = _run,
			},
		},
		.user = user,
		.db = db,
	);

	return &this->public.filter;
}
