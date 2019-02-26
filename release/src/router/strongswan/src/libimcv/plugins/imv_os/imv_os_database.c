/*
 * Copyright (C) 2012-2017 Andreas Steffen
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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "imv_os_database.h"

#include <utils/debug.h>

typedef struct private_imv_os_database_t private_imv_os_database_t;

/**
 * Private data of a imv_os_database_t object.
 *
 */
struct private_imv_os_database_t {

	/**
	 * Public imv_os_database_t interface.
	 */
	imv_os_database_t public;

	/**
	 * database instance
	 */
	database_t *db;

};

METHOD(imv_os_database_t, check_packages, status_t,
	private_imv_os_database_t *this, imv_os_state_t *os_state,
	enumerator_t *package_enumerator)
{
	imv_state_t *state;
	imv_session_t *session;
	imv_os_info_t *os_info;
	os_type_t os_type;
	char *product, *package, *release, *cur_release;
	chunk_t name, version;
	int pid, gid, security, blacklist;
	int count = 0, count_ok = 0, count_security = 0, count_blacklist = 0;
	enumerator_t *e;
	status_t status = SUCCESS;
	bool found, match;

	state = &os_state->interface;
	session = state->get_session(state);
	session->get_session_id(session, &pid, NULL);
	os_info = session->get_os_info(session);
	os_type = os_info->get_type(os_info);
	product = os_info->get_info(os_info);

	if (os_type == OS_TYPE_ANDROID)
	{
		/*no package dependency on Android version */
		product = enum_to_name(os_type_names, os_type);

		/* Get primary key of product */
		e = this->db->query(this->db,
					"SELECT id FROM products WHERE name = ?",
					DB_TEXT, product, DB_INT);
		if (!e)
		{
			return FAILED;
		}
		if (!e->enumerate(e, &pid))
		{
			e->destroy(e);
			return NOT_FOUND;
		}
		e->destroy(e);
	}
	DBG1(DBG_IMV, "processing installed '%s' packages", product);

	while (package_enumerator->enumerate(package_enumerator, &name, &version))
	{
		/* Convert package name chunk to a string */
		package = strndup(name.ptr, name.len);
		count++;

		/* Get primary key of package */
		e = this->db->query(this->db,
					"SELECT id FROM packages WHERE name = ?",
					DB_TEXT, package, DB_INT);
		if (!e)
		{
			free(package);
			return FAILED;
		}
		if (!e->enumerate(e, &gid))
		{
			/* package not present in database for any product - skip */
			DBG2(DBG_IMV, "package '%s' (%.*s) not found",
						   package, version.len, version.ptr);
			free(package);
			e->destroy(e);
			continue;
		}
		e->destroy(e);

		/* Convert package version chunk to a string */
		release = strndup(version.ptr, version.len);

		/* Enumerate over all acceptable versions */
		e = this->db->query(this->db,
				"SELECT release, security, blacklist FROM versions "
				"WHERE product = ? AND package = ?",
				DB_INT, pid, DB_INT, gid, DB_TEXT, DB_INT, DB_INT);
		if (!e)
		{
			free(package);
			free(release);
			return FAILED;
		}
		found = FALSE;
		match = FALSE;

		while (e->enumerate(e, &cur_release, &security, &blacklist))
		{
			found = TRUE;
			if (streq(release, cur_release) || streq("*", cur_release))
			{
				match = TRUE;
				break;
			}
		}
		e->destroy(e);

		if (found)
		{
			if (match)
			{
				if (blacklist)
				{
					DBG1(DBG_IMV, "package '%s' (%s) is blacklisted",
								   package, release);
					count_blacklist++;
					os_state->add_bad_package(os_state, package,
											  OS_PACKAGE_STATE_BLACKLIST);
				}
				else if (security)
				{
					DBG1(DBG_IMV, "package '%s' (%s) is vulnerable",
								   package, release);
					os_state->add_bad_package(os_state, package,
											  OS_PACKAGE_STATE_SECURITY);
					count_security++;
				}
				else
				{
					DBG2(DBG_IMV, "package '%s' (%s) is ok",
								   package, release);
					count_ok++;
				}
			}
			else
			{
				DBG1(DBG_IMV, "package '%s' (%s) no match", package, release);
			}
		}
		else
		{
			DBG2(DBG_IMV, "package '%s' (%s) unknown", package, release);
		}
		free(package);
		free(release);
	}
	os_state->set_count(os_state, count, count_security, count_blacklist,
						count_ok);

	return status;
}

METHOD(imv_os_database_t, destroy, void,
	private_imv_os_database_t *this)
{
	free(this);
}

/**
 * See header
 */
imv_os_database_t *imv_os_database_create(imv_database_t *imv_db)
{
	private_imv_os_database_t *this;

	if (!imv_db)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.check_packages = _check_packages,
			.destroy = _destroy,
		},
		.db = imv_db->get_database(imv_db),
	);

	return &this->public;
}

