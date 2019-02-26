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

#include "imv_policy_manager_usage.h"
#include "imv_workitem.h"

#include <library.h>
#include <utils/debug.h>

#include <tncif_names.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* The default policy group #1 is assumed to always exist */
#define DEFAULT_GROUP_ID	1

/**
 * global debug output variables
 */
static int debug_level = 1;
static bool stderr_quiet = FALSE;

/**
 * attest dbg function
 */
static void stderr_dbg(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= debug_level)
	{
		if (!stderr_quiet)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			fprintf(stderr, "\n");
			va_end(args);
		}
	}
}

/**
 * Collect all enforcements by iterating up through parent groups
 */
static bool iterate_enforcements(database_t *db, int device_id, int session_id,
								 int group_id)
{
	int id, type, file, dir, arg_int, parent, policy, max_age;
	int p_rec_fail, p_rec_noresult, e_rec_fail, e_rec_noresult, latest_rec;
	bool latest_success;
	char *argument;
	time_t now;
	enumerator_t *e, *e1, *e2;

	now = time(NULL);

	while (group_id)
	{
		e1 = db->query(db,
				"SELECT e.id, p.type, p.argument, p.file, p.dir, p.rec_fail, "
				"p.rec_noresult, e.policy, e.max_age, e.rec_fail, e.rec_noresult "
				"FROM enforcements AS e JOIN policies as p ON e.policy = p.id "
				"WHERE e.group_id = ?", DB_INT, group_id,
				 DB_INT, DB_INT, DB_TEXT, DB_INT, DB_INT, DB_INT, DB_INT,
				 DB_INT, DB_INT, DB_INT, DB_INT);
		if (!e1)
		{
			return FALSE;
		}
		while (e1->enumerate(e1, &id, &type, &argument, &file, &dir,
								 &p_rec_fail, &p_rec_noresult, &policy, &max_age,
								 &e_rec_fail, &e_rec_noresult))
		{
			/* check if the latest measurement of the device was successful */
			latest_success = FALSE;

			if (device_id)
			{
				e2 = db->query(db,
						"SELECT r.rec FROM results AS r "
						"JOIN sessions AS s ON s.id = r.session "
						"WHERE r.policy = ? AND s.device = ? AND s.time > ? "
						"ORDER BY s.time DESC",
						DB_INT, policy, DB_INT, device_id,
						DB_UINT, now - max_age,	DB_INT);
				if (!e2)
				{
					e1->destroy(e1);
					return FALSE;
				}
				if (e2->enumerate(e2, &latest_rec) &&
					latest_rec == TNC_IMV_ACTION_RECOMMENDATION_ALLOW)
				{
					latest_success = TRUE;
				}
				e2->destroy(e2);
			}

			if (latest_success)
			{
				/*skipping enforcement */
				printf("skipping enforcement %d\n", id);
				continue;
			}

			/* determine arg_int */
			switch ((imv_workitem_type_t)type)
			{
				case IMV_WORKITEM_FILE_REF_MEAS:
				case IMV_WORKITEM_FILE_MEAS:
				case IMV_WORKITEM_FILE_META:
					arg_int = file;
					break;
				case IMV_WORKITEM_DIR_REF_MEAS:
				case IMV_WORKITEM_DIR_MEAS:
				case IMV_WORKITEM_DIR_META:
					arg_int = dir;
					break;
				case IMV_WORKITEM_SWID_TAGS:
					/* software [identifier] inventory by default */
					arg_int = 0;

					/* software identifiers only? */
					if (device_id && strchr(argument, 'R'))
					{
						/* get last EID in order to set earliest EID */
						e2 = db->query(db,
							"SELECT eid FROM swid_events where device == ? "
							"ORDER BY eid DESC", DB_UINT, device_id, DB_INT);
						if (e2)
						{
							if (e2->enumerate(e2, &arg_int))
							{
								arg_int++;
							}
							else
							{
								arg_int = 1;
							}
							e2->destroy(e2);
						}
					}
					break;
				default:
					arg_int = 0;
			}

			/* insert a workitem */
			if (db->execute(db, NULL,
				"INSERT INTO workitems (session, enforcement, type, arg_str, "
				"arg_int, rec_fail, rec_noresult) VALUES (?, ?, ?, ?, ?, ?, ?)",
				DB_INT, session_id, DB_INT, id, DB_INT, type, DB_TEXT, argument,
				DB_INT, arg_int, DB_INT, e_rec_fail ? e_rec_fail : p_rec_fail,
				DB_INT, e_rec_noresult ? e_rec_noresult : p_rec_noresult) != 1)
			{
				e1->destroy(e1);
				fprintf(stderr, "could not insert workitem\n");
				return FALSE;
			}
		}
		e1->destroy(e1);

		e = db->query(db,
				"SELECT parent FROM groups WHERE id = ?",
				 DB_INT, group_id, DB_INT);
		if (!e)
		{
			return FALSE;
		}
		if (e->enumerate(e, &parent))
		{
			group_id = parent;
		}
		else
		{
			fprintf(stderr, "group information not found\n");
			group_id = 0;
		}
		e->destroy(e);
	}
	return TRUE;
}

static bool policy_start(database_t *db, int session_id)
{
	enumerator_t *e;
	int device_id, product_id, gid, group_id = DEFAULT_GROUP_ID;
	u_int created;

	/* get session data */
	e = db->query(db,
			"SELECT s.device, s.product, d.created FROM sessions AS s "
			"LEFT JOIN devices AS d ON s.device = d.id WHERE s.id = ?",
			 DB_INT, session_id, DB_INT, DB_INT, DB_UINT);
	if (!e || !e->enumerate(e, &device_id, &product_id, &created))
	{
		DESTROY_IF(e);
		fprintf(stderr, "session %d not found\n", session_id);
		return FALSE;
	}
	e->destroy(e);

	/* if a device ID with a creation date exists, get all group memberships */
	if (device_id && created)
	{
		e = db->query(db,
				"SELECT group_id FROM groups_members WHERE device_id = ?",
				 DB_INT, device_id, DB_INT);
		if (!e)
		{
			return FALSE;
		}
		while (e->enumerate(e, &group_id))
		{
			if (!iterate_enforcements(db, device_id, session_id, group_id))
			{
				e->destroy(e);
				return FALSE;
			}
		}
		e->destroy(e);

		return TRUE;
	}

	/* determine if a default product group exists */
	e = db->query(db,
			"SELECT group_id FROM groups_product_defaults "
			"WHERE product_id = ?", DB_INT, product_id, DB_INT);
	if (!e)
	{
		return FALSE;
	}
	if (e->enumerate(e, &gid))
	{
		group_id = gid;
	}
	e->destroy(e);

	if (device_id && !created)
	{
		/* assign a newly created device to a default group */
		if (db->execute(db, NULL,
			"INSERT INTO groups_members (device_id, group_id) "
			"VALUES (?, ?)", DB_INT, device_id, DB_INT, group_id) != 1)
		{
			fprintf(stderr, "could not assign device to a default group\n");
			return FALSE;
		}

		/* set the creation date if it hasn't been set yet */
		if (db->execute(db, NULL,
				"UPDATE devices SET created = ? WHERE id = ?",
				DB_UINT, time(NULL), DB_INT, device_id) != 1)
		{
			fprintf(stderr, "creation date of device could not be set\n");
			return FALSE;
		}
	}

	return iterate_enforcements(db, device_id, session_id, group_id);
}

static bool policy_stop(database_t *db, int session_id)
{
	enumerator_t *e;
	int rec, policy, final_rec, id_type;
	chunk_t id_value;
	char *result, *format, *ip_address = NULL;
	char command[512];
	bool success = TRUE;

	/* store all workitem results for this session in the results table */
	e = db->query(db,
			"SELECT w.rec_final, w.result, e.policy FROM workitems AS w "
			"JOIN enforcements AS e ON w.enforcement = e.id "
			"WHERE w.session = ? AND w.result IS NOT NULL",
			 DB_INT, session_id, DB_INT, DB_TEXT, DB_INT);
	if (e)
	{
		while (e->enumerate(e, &rec, &result, &policy))
		{
			db->execute(db, NULL,
				"INSERT INTO results (session, policy, rec, result) "
				"VALUES (?, ?, ?, ?)", DB_INT, session_id, DB_INT, policy,
				 DB_INT, rec, DB_TEXT, result);
		}
		e->destroy(e);
	}
	else
	{
		success = FALSE;
	}

	/* delete all workitems for this session from the database */
	if (db->execute(db, NULL,
					"DELETE FROM workitems WHERE session = ?",
					DB_UINT, session_id) < 0)
	{
		success = FALSE;
	}

	final_rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;

	/* retrieve the final recommendation for this session */
	e = db->query(db,
			"SELECT rec FROM sessions WHERE id = ?",
			 DB_INT, session_id, DB_INT);
	if (e)
	{
		if (!e->enumerate(e, &final_rec))
		{
			success = FALSE;
		}
		e->destroy(e);
	}
	else
	{
		success = FALSE;
	}

	/* retrieve client IP address for this session */
	e = db->query(db,
			"SELECT i.type, i.value FROM identities AS i "
			"JOIN sessions_identities AS si ON si.identity_id = i.id "
			"WHERE si.session_id = ? AND (i.type = ? OR i.type = ?)",
			 DB_INT, session_id, DB_INT, TNC_ID_IPV4_ADDR, DB_INT,
			 TNC_ID_IPV6_ADDR, DB_INT, DB_BLOB);
	if (e)
	{
		if (e->enumerate(e, &id_type, &id_value))
		{
			ip_address = strndup(id_value.ptr, id_value.len);
		}
		else
		{
			success = FALSE;
		}
		e->destroy(e);
	}
	else
	{
		success = FALSE;
	}

	fprintf(stderr, "recommendation for access requestor %s is %N\n",
			ip_address ? ip_address : "0.0.0.0",
			TNC_IMV_Action_Recommendation_names, final_rec);

	if (final_rec == TNC_IMV_ACTION_RECOMMENDATION_ALLOW)
	{
		format = lib->settings->get_str(lib->settings,
						"imv_policy_manager.command_allow", NULL);
	}
	else
	{
		format = lib->settings->get_str(lib->settings,
						"imv_policy_manager.command_block", NULL);
	}
	if (format && ip_address)
	{
		/* the IP address can occur at most twice in the command string */
		snprintf(command, sizeof(command), format, ip_address, ip_address);
		success = system(command) == 0;
		fprintf(stderr, "%s system command: %s\n",
			    success ? "successful" : "failed", command);
	}
	free(ip_address);

	return success;
}

int main(int argc, char *argv[])
{
	database_t *db;
	char *uri;
	int session_id;
	bool start, success;

	/* enable attest debugging hook */
	dbg = stderr_dbg;

	atexit(library_deinit);

	/* initialize library */
	if (!library_init(NULL, "imv_policy_manager"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "imv_policy_manager.load",
				 "sqlite")))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	if (argc < 3)
	{
		usage();
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	if (streq(argv[1], "start"))
	{
		start = TRUE;
	}
	else if (streq(argv[1], "stop"))
	{
		start = FALSE;
	}
	else
	{
		usage();
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	session_id = atoi(argv[2]);

	/* attach IMV database */
	uri = lib->settings->get_str(lib->settings,
			"imv_policy_manager.database",
			lib->settings->get_str(lib->settings,
				"charon.imcv.database",
				lib->settings->get_str(lib->settings,
					"libimcv.database", NULL)));
	if (!uri)
	{
		fprintf(stderr, "database uri not defined.\n");
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	db = lib->db->create(lib->db, uri);
	if (!db)
	{
		fprintf(stderr, "opening database failed.\n");
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	if (start)
	{
		success = policy_start(db, session_id);
	}
	else
	{
		success = policy_stop(db, session_id);
	}
	db->destroy(db);

	fprintf(stderr, "imv_policy_manager %s %s\n", start ? "start" : "stop",
			success ? "successful" : "failed");

	exit(EXIT_SUCCESS);
}
