/*
 * Copyright (C) 2013-2015 Andreas Steffen
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "imv_database.h"

#include <tncif_identity.h>

#include <utils/debug.h>
#include <threading/mutex.h>

typedef struct private_imv_database_t private_imv_database_t;

/**
 * Private data of a imv_database_t object.
 */
struct private_imv_database_t {

	/**
	 * Public imv_database_t interface.
	 */
	imv_database_t public;

	/**
	 * database instance
	 */
	database_t *db;

	/**
	 * policy script
	 */
	char *script;

};

METHOD(imv_database_t, get_database, database_t*,
	private_imv_database_t *this)
{
	return this->db;
}

/**
 * Create a session entry in the IMV database
 */
static bool create_session(private_imv_database_t *this, imv_session_t *session)
{
	enumerator_t *enumerator, *e;
	imv_os_info_t *os_info;
	chunk_t device_id;
	tncif_identity_t *tnc_id;
	TNC_ConnectionID conn_id;
	char *product, *device;
	int session_id = 0, pid = 0, did = 0, trusted = 0, created;
	bool first = TRUE, success = TRUE;

	/* get product info string */
	os_info = session->get_os_info(session);
	product = os_info->get_info(os_info);
	if (!product)
	{
		DBG1(DBG_IMV, "imv_db: product info is not available");
		return FALSE;
	}

	/* get primary key of product info string if it exists */
	e = this->db->query(this->db,
			"SELECT id FROM products WHERE name = ?", DB_TEXT, product, DB_INT);
	if (e)
	{
		e->enumerate(e, &pid);
		e->destroy(e);
	}

	/* if product info string has not been found - register it */
	if (!pid)
	{
		this->db->execute(this->db, &pid,
			"INSERT INTO products (name) VALUES (?)", DB_TEXT, product);
	}

	if (!pid)
	{
		DBG1(DBG_IMV, "imv_db: registering product info failed");
		return FALSE;
	}

	/* get device ID string */
	if (!session->get_device_id(session, &device_id))
	{
		DBG1(DBG_IMV, "imv_db: device ID is not available");
		return FALSE;
	}
	device = strndup(device_id.ptr, device_id.len);

	/* get primary key of device ID if it exists */
	e = this->db->query(this->db,
			"SELECT id, trusted FROM devices WHERE value = ? AND product = ?",
			 DB_TEXT, device, DB_INT, pid, DB_INT, DB_INT);
	if (e)
	{
		e->enumerate(e, &did, &trusted);
		e->destroy(e);
	}

	/* if device ID is trusted, set trust in session */
	if (trusted)
	{
		session->set_device_trust(session, TRUE);
	}

	/* if device ID has not been found - register it */
	if (!did)
	{
		this->db->execute(this->db, &did,
			"INSERT INTO devices "
			"(value, description, product, trusted, inactive) "
			"VALUES (?, '', ?, 0, 0)", DB_TEXT, device, DB_INT, pid);
	}
	free(device);

	if (!did)
	{
		DBG1(DBG_IMV, "imv_db: registering device ID failed");
		return FALSE;
	}

	/* create a new session entry */
	created = time(NULL);
	conn_id = session->get_connection_id(session);
	this->db->execute(this->db, &session_id,
			"INSERT INTO sessions (time, connection, product, device) "
			"VALUES (?, ?, ?, ?)",
			DB_INT, created, DB_INT, conn_id, DB_INT, pid, DB_INT, did);

	if (session_id)
	{
		DBG2(DBG_IMV, "assigned session ID %d to Connection ID %d",
					   session_id, conn_id);
	}
	else
	{
		DBG1(DBG_IMV, "imv_db: registering session failed");
		return FALSE;
	}
	session->set_session_id(session, session_id, pid, did);
	session->set_creation_time(session, created);

	enumerator = session->create_ar_identities_enumerator(session);
	while (enumerator->enumerate(enumerator, &tnc_id))
	{
		pen_type_t ar_id_type;
		chunk_t ar_id_value;
		int ar_id = 0, si_id = 0;

		ar_id_type = tnc_id->get_identity_type(tnc_id);
		ar_id_value = tnc_id->get_identity_value(tnc_id);

		if (ar_id_type.vendor_id != PEN_TCG || ar_id_value.len == 0)
		{
			continue;
		}

		/* get primary key of AR identity if it exists */
		e = this->db->query(this->db,
				"SELECT id FROM identities WHERE type = ? AND value = ?",
				DB_INT, ar_id_type.type, DB_BLOB, ar_id_value, DB_INT);
		if (e)
		{
			e->enumerate(e, &ar_id);
			e->destroy(e);
		}

		/* if AR identity has not been found - register it */
		if (!ar_id)
		{
			this->db->execute(this->db, &ar_id,
				"INSERT INTO identities (type, value) VALUES (?, ?)",
				 DB_INT, ar_id_type.type, DB_BLOB, ar_id_value);
		}
		if (!ar_id)
		{
			DBG1(DBG_IMV, "imv_db: registering access requestor failed");
			success = FALSE;
			break;
		}

		this->db->execute(this->db, &si_id,
				"INSERT INTO sessions_identities (session_id, identity_id) "
				"VALUES (?, ?)",
				 DB_INT, session_id, DB_INT, ar_id);

		if (!si_id)
		{
			DBG1(DBG_IMV, "imv_db: assigning identity to session failed");
			success = FALSE;
			break;
		}

		if (first)
		{
			this->db->execute(this->db, NULL,
				"UPDATE sessions SET identity = ? WHERE id = ?",
				 DB_INT, ar_id, DB_INT, session_id);
			first = FALSE;
		}
	}
	enumerator->destroy(enumerator);

	return success;
}

static bool add_workitems(private_imv_database_t *this, imv_session_t *session)
{
	char *arg_str;
	int id, arg_int, rec_fail, rec_noresult;
	imv_workitem_t *workitem;
	imv_workitem_type_t type;
	enumerator_t *e;

	e = this->db->query(this->db,
			"SELECT id, type, arg_str, arg_int, rec_fail, rec_noresult "
			"FROM workitems WHERE session = ?",
			 DB_INT, session->get_session_id(session, NULL, NULL),
			 DB_INT, DB_INT, DB_TEXT, DB_INT,DB_INT, DB_INT);
	if (!e)
	{
		DBG1(DBG_IMV, "imv_db: no workitem enumerator returned");
		return FALSE;
	}
	while (e->enumerate(e, &id, &type, &arg_str, &arg_int, &rec_fail,
						   &rec_noresult))
	{
		DBG2(DBG_IMV, "%N workitem %d", imv_workitem_type_names, type, id);
		workitem = imv_workitem_create(id, type, arg_str, arg_int, rec_fail,
									   rec_noresult);
		session->insert_workitem(session, workitem);
	}
	e->destroy(e);

	return TRUE;
}

METHOD(imv_database_t, add_recommendation, void,
	private_imv_database_t *this, imv_session_t *session,
	TNC_IMV_Action_Recommendation rec)
{
	/* add final recommendation to session DB entry */
	this->db->execute(this->db, NULL,
			"UPDATE sessions SET rec = ? WHERE id = ?",
			 DB_INT, rec, DB_INT, session->get_session_id(session, NULL, NULL));
}

METHOD(imv_database_t, policy_script, bool,
	private_imv_database_t *this, imv_session_t *session, bool start)
{
	char command[512], resp[128], *last;
	FILE *shell;

	if (start)
	{
		if (session->get_policy_started(session))
		{
			DBG1(DBG_IMV, "policy script as already been started");
			return FALSE;
		}

		/* add product info and device ID to session DB entry */
		if (!create_session(this, session))
		{
			return FALSE;
		}
	}
	else
	{
		if (!session->get_policy_started(session))
		{
			DBG1(DBG_IMV, "policy script as already been stopped");
			return FALSE;
		}
	}

	/* call the policy script */
	snprintf(command, sizeof(command), "2>&1 %s %s %d",
			 this->script, start ? "start" : "stop",
			 session->get_session_id(session, NULL, NULL));
	DBG3(DBG_IMV, "running policy script: %s", command);

	shell = popen(command, "r");
	if (shell == NULL)
	{
		DBG1(DBG_IMV, "could not execute policy script '%s'",
			 this->script);
		return FALSE;
	}
	while (TRUE)
	{
		if (fgets(resp, sizeof(resp), shell) == NULL)
		{
			if (ferror(shell))
			{
				DBG1(DBG_IMV, "error reading output from policy script");
			}
			break;
		}
		else
		{
			last = resp + strlen(resp) - 1;
			if (last >= resp && *last == '\n')
			{
				/* replace trailing '\n' */
				*last = '\0';
			}
			DBG1(DBG_IMV, "policy: %s", resp);
		}
	}
	pclose(shell);

	if (start)
	{
		/* add workitem list generated by policy manager to session object */
		if (!add_workitems(this, session))
		{
			return FALSE;
		}
		session->set_policy_started(session, TRUE);
	}
	else
	{
		session->set_policy_started(session, FALSE);
	}

	return TRUE;
}

METHOD(imv_database_t, finalize_workitem, bool,
	private_imv_database_t *this, imv_workitem_t *workitem)
{
	char *result;
	int rec;

	rec = workitem->get_result(workitem, &result);

	return this->db->execute(this->db, NULL,
				"UPDATE workitems SET result = ?, rec_final = ? WHERE id = ?",
				DB_TEXT, result, DB_INT, rec,
				DB_INT, workitem->get_id(workitem)) == 1;
}

METHOD(imv_database_t, destroy, void,
	private_imv_database_t *this)
{
	DESTROY_IF(this->db);
	free(this);
}

/**
 * See header
 */
imv_database_t *imv_database_create(char *uri, char *script)
{
	private_imv_database_t *this;

	INIT(this,
		.public = {
			.get_database = _get_database,
			.policy_script = _policy_script,
			.finalize_workitem = _finalize_workitem,
			.add_recommendation = _add_recommendation,
			.destroy = _destroy,
		},
		.db = lib->db->create(lib->db, uri),
		.script = script,
	);

	if (!this->db)
	{
		DBG1(DBG_IMV,
			 "failed to connect to IMV database '%s'", uri);
		destroy(this);
		return NULL;
	}

	return &this->public;
}
