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

#define _GNU_SOURCE
#include <string.h>

#include "user_controller.h"

#include <library.h>

typedef struct private_user_controller_t private_user_controller_t;

/**
 * private data of the user_controller
 */
struct private_user_controller_t {

	/**
	 * public functions
	 */
	user_controller_t public;

	/**
	 * database connection
	 */
	database_t *db;

	/**
	 * user session
	 */
	user_t *user;

	/**
	 * minimum required password length
	 */
	u_int password_length;
};

/**
 * hash the password for database storage
 */
static chunk_t hash_password(char *login, char *password)
{
	hasher_t *hasher;
	chunk_t hash, data;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher)
	{
		return chunk_empty;
	}
	data = chunk_cata("cc",	chunk_create(login, strlen(login)),
							chunk_create(password, strlen(password)));
	if (!hasher->allocate_hash(hasher, data, &hash))
	{
		hasher->destroy(hasher);
		return chunk_empty;
	}
	hasher->destroy(hasher);
	return hash;
}

/**
 * Login a user.
 */
static void login(private_user_controller_t *this, fast_request_t *request)
{
	if (request->get_query_data(request, "submit"))
	{
		char *login, *password;

		login = request->get_query_data(request, "login");
		password = request->get_query_data(request, "password");

		if (login && password)
		{
			enumerator_t *query;
			u_int id = 0;
			chunk_t hash;

			hash = hash_password(login, password);
			query = this->db->query(this->db,
						"SELECT id FROM user WHERE login = ? AND password = ?",
						DB_TEXT, login, DB_BLOB, hash, DB_UINT);
			if (query)
			{
				query->enumerate(query, &id);
				query->destroy(query);
			}
			free(hash.ptr);
			if (id)
			{
				this->user->set_user(this->user, id);
				return request->redirect(request, "peer/list");
			}
		}
		request->setf(request, "error=Invalid username or password.");
	}
	request->render(request, "templates/user/login.cs");
}

/**
 * Logout a user.
 */
static void logout(private_user_controller_t *this, fast_request_t *request)
{
	request->redirect(request, "user/login");
	request->close_session(request);
}

/**
 * verify a user entered username for validity
 */
static bool verify_login(private_user_controller_t *this,
						 fast_request_t *request, char *login)
{
	if (!login || *login == '\0')
	{
		request->setf(request, "error=Username is missing.");
		return FALSE;
	}
	while (*login != '\0')
	{
		switch (*login)
		{
			case 'a' ... 'z':
			case 'A' ... 'Z':
			case '0' ... '9':
			case '-':
			case '_':
			case '@':
			case '.':
				login++;
				continue;
			default:
				request->setf(request, "error=Username invalid, "
							  "valid characters: A-Z a-z 0-9 - _ @ .");
		}
	}
	return TRUE;
}

/**
 * verify a user entered password for validity
 */
static bool verify_password(private_user_controller_t *this,
							fast_request_t *request,
							char *password, char *confirm)
{
	if (!password || *password == '\0')
	{
		request->setf(request, "error=Password is missing.");
		return FALSE;
	}
	if (strlen(password) < this->password_length)
	{
		request->setf(request, "error=Password requires at least %d characters.",
					  this->password_length);
		return FALSE;
	}
	if (!confirm || !streq(password, confirm))
	{
		request->setf(request, "error=Password not confirmed.");
		return FALSE;
	}
	return TRUE;
}

/**
 * Register a user.
 */
static void add(private_user_controller_t *this, fast_request_t *request)
{
	char *login = "";

	while (request->get_query_data(request, "register"))
	{
		char *password, *confirm;
		chunk_t hash;
		u_int id;

		login = request->get_query_data(request, "new_login");
		password = request->get_query_data(request, "new_password");
		confirm = request->get_query_data(request, "confirm_password");

		if (!verify_login(this, request, login) ||
			!verify_password(this, request, password, confirm))
		{
			break;
		}

		hash = hash_password(login, password);
		if (!hash.ptr || this->db->execute(this->db, &id,
							"INSERT INTO user (login, password) VALUES (?, ?)",
							DB_TEXT, login, DB_BLOB, hash) < 0)
		{
			request->setf(request, "error=Username already exists.");
			free(hash.ptr);
			break;
		}
		free(hash.ptr);
		this->user->set_user(this->user, id);
		return request->redirect(request, "peer/list");
	}
	request->set(request, "new_login", login);
	request->setf(request, "password_length=%d", this->password_length);
	request->render(request, "templates/user/add.cs");
}

/**
 * Edit the logged in user
 */
static void edit(private_user_controller_t *this, fast_request_t *request)
{
	enumerator_t *query;
	char *old_login;

	/* lookup old login */
	query = this->db->query(this->db, "SELECT login FROM user WHERE id = ?",
							DB_INT, this->user->get_user(this->user),
							DB_TEXT);
	if (!query || !query->enumerate(query, &old_login))
	{
		DESTROY_IF(query);
		request->close_session(request);
		return request->redirect(request, "user/login");
	}
	old_login = strdupa(old_login);
	query->destroy(query);

	/* back pressed */
	if (request->get_query_data(request, "back"))
	{
		return request->redirect(request, "peer/list");
	}
	/* delete pressed */
	if (request->get_query_data(request, "delete"))
	{
		this->db->execute(this->db, NULL, "DELETE FROM user WHERE id = ?",
						  DB_UINT, this->user->get_user(this->user));
		this->db->execute(this->db, NULL,
						  "DELETE FROM peer WHERE user = ?",
						  DB_UINT, this->user->get_user(this->user));
		return logout(this, request);
	}
	/* save pressed */
	while (request->get_query_data(request, "save"))
	{
		char *new_login, *old_pass, *new_pass, *confirm;
		chunk_t old_hash, new_hash;

		new_login = request->get_query_data(request, "old_login");
		old_pass = request->get_query_data(request, "old_password");
		new_pass = request->get_query_data(request, "new_password");
		confirm = request->get_query_data(request, "confirm_password");

		if (!verify_login(this, request, new_login) ||
			!verify_password(this, request, new_pass, confirm))
		{
			old_login = new_login;
			break;
		}
		old_hash = hash_password(old_login, old_pass);
		new_hash = hash_password(new_login, new_pass);

		if (this->db->execute(this->db, NULL,
			"UPDATE user SET login = ?, password = ? "
			"WHERE id = ? AND password = ?",
			DB_TEXT, new_login, DB_BLOB, new_hash,
			DB_UINT, this->user->get_user(this->user), DB_BLOB, old_hash) <= 0)
		{
			free(new_hash.ptr);
			free(old_hash.ptr);
			old_login = new_login;
			request->setf(request, "error=Password verification failed.");
			break;
		}
		free(new_hash.ptr);
		free(old_hash.ptr);
		return request->redirect(request, "peer/list");
	}
	/* on error/template rendering */
	request->set(request, "old_login", old_login);
	request->setf(request, "password_length=%d", this->password_length);
	request->render(request, "templates/user/edit.cs");
}

METHOD(fast_controller_t, get_name, char*,
	private_user_controller_t *this)
{
	return "user";
}

METHOD(fast_controller_t, handle, void,
	private_user_controller_t *this, fast_request_t *request, char *action,
	char *p2, char *p3, char *p4, char *p5)
{
	if (action)
	{
		if (streq(action, "add"))
		{
			return add(this, request);
		}
		if (streq(action, "login"))
		{
			return login(this, request);
		}
		else if (streq(action, "logout"))
		{
			return logout(this, request);
		}
		else if (streq(action, "edit"))
		{
			return edit(this, request);
		}
		else if (streq(action, "help"))
		{
			return request->render(request, "templates/user/help.cs");
		}
	}
	request->redirect(request, "user/login");
}

METHOD(fast_controller_t, destroy, void,
	private_user_controller_t *this)
{
	free(this);
}

/*
 * see header file
 */
fast_controller_t *user_controller_create(user_t *user, database_t *db)
{
	private_user_controller_t *this;

	INIT(this,
		.public = {
			.controller = {
				.get_name = _get_name,
				.handle = _handle,
				.destroy = _destroy,
			},
		},
		.user = user,
		.db = db,
		.password_length = lib->settings->get_int(lib->settings,
												  "medsrv.password_length", 6),
	);

	return &this->public.controller;
}
