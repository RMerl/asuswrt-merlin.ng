/*
 * Copyright (C) 2007 Martin Willi
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

#include "auth_controller.h"
#include "../manager.h"

#include <library.h>


typedef struct private_auth_controller_t private_auth_controller_t;

/**
 * private data of the task manager
 */
struct private_auth_controller_t {

	/**
	 * public functions
	 */
	auth_controller_t public;

	/**
	 * manager instance
	 */
	manager_t *manager;
};

static void login(private_auth_controller_t *this, fast_request_t *request)
{
	request->set(request, "action", "check");
	request->set(request, "title", "Login");
	request->render(request, "templates/auth/login.cs");
}

static void check(private_auth_controller_t *this, fast_request_t *request)
{
	char *username, *password;

	username = request->get_query_data(request, "username");
	password = request->get_query_data(request, "password");
	if (username && password &&
		this->manager->login(this->manager, username, password))
	{
		request->redirect(request, "ikesa/list");
	}
	else
	{
		request->redirect(request, "auth/login");
	}
}

static void logout(private_auth_controller_t *this, fast_request_t *request)
{
	this->manager->logout(this->manager);
	request->redirect(request, "auth/login");
}

METHOD(fast_controller_t, get_name, char*,
	private_auth_controller_t *this)
{
	return "auth";
}

METHOD(fast_controller_t, handle, void,
	private_auth_controller_t *this, fast_request_t *request, char *action,
	char *p2, char *p3, char *p4, char *p5)
{
	if (action)
	{
		if (streq(action, "login"))
		{
			return login(this, request);
		}
		else if (streq(action, "check"))
		{
			return check(this, request);
		}
		else if (streq(action, "logout"))
		{
			return logout(this, request);
		}
	}
	request->redirect(request, "auth/login");
}

METHOD(fast_controller_t, destroy, void,
	private_auth_controller_t *this)
{
	free(this);
}

/*
 * see header file
 */
fast_controller_t *auth_controller_create(fast_context_t *context, void *param)
{
	private_auth_controller_t *this;

	INIT(this,
		.public = {
			.controller = {
				.get_name = _get_name,
				.handle = _handle,
				.destroy = _destroy,
			},
		},
		.manager = (manager_t*)context,
	);

	return &this->public.controller;
}
