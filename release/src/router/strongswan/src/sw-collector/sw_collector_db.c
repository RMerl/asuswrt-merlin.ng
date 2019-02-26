/*
 * Copyright (C) 2017 Andreas Steffen
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "sw_collector_db.h"

#include "swima/swima_event.h"

typedef struct private_sw_collector_db_t private_sw_collector_db_t;

/**
 * Private data of an sw_collector_db_t object.
 */
struct private_sw_collector_db_t {

	/**
	 * Public members of sw_collector_db_state_t
	 */
	sw_collector_db_t public;

	/**
	 * Epoch
	 */
	uint32_t epoch;

	/**
	 * Event ID of last event stored in database
	 */
	uint32_t last_eid;

	/**
	 * Software collector database
	 */
	database_t *db;

};

METHOD(sw_collector_db_t, add_event, uint32_t,
	private_sw_collector_db_t *this, char *timestamp)
{
	uint32_t eid = 0;

	if (this->db->execute(this->db, &eid,
			"INSERT INTO events (epoch, timestamp) VALUES (?, ?)",
			 DB_UINT, this->epoch, DB_TEXT, timestamp) != 1)
	{
		DBG1(DBG_IMC, "unable to insert event into database");
		return 0;
	}

	return eid;
}

METHOD(sw_collector_db_t, get_last_event, bool,
	private_sw_collector_db_t *this, uint32_t *eid, uint32_t *epoch,
	char **last_time)
{
	char *timestamp;
	enumerator_t *e;

	e = this->db->query(this->db,
			"SELECT id, epoch, timestamp FROM events ORDER BY timestamp DESC",
			DB_UINT, DB_UINT, DB_TEXT);
	if (!e)
	{
		DBG1(DBG_IMC, "database query for event failed");
		return FALSE;
	}
	if (e->enumerate(e, eid, epoch, &timestamp))
	{
		if (last_time)
		{
			*last_time = strdup(timestamp);
		}
	}
	else
	{
		*eid = 0;
	}
	e->destroy(e);

	return TRUE;
}

METHOD(sw_collector_db_t, add_sw_event, bool,
	private_sw_collector_db_t *this, uint32_t eid, uint32_t sw_id,
	uint8_t action)
{
	if (this->db->execute(this->db, NULL,
			"INSERT INTO sw_events (eid, sw_id, action) VALUES (?, ?, ?)",
			 DB_UINT, eid, DB_UINT, sw_id, DB_UINT, action) != 1)
	{
		DBG1(DBG_IMC, "unable to insert sw_event into database");
		return FALSE;
	}

	return TRUE;
}

METHOD(sw_collector_db_t, set_sw_id, uint32_t,
	private_sw_collector_db_t *this, char *name,  char *package, char *version,
	uint8_t source, bool installed)
{
	uint32_t sw_id;

	if (this->db->execute(this->db, &sw_id,
			"INSERT INTO sw_identifiers "
			"(name, package, version, source, installed) VALUES (?, ?, ?, ?, ?)",
			 DB_TEXT, name, DB_TEXT, package, DB_TEXT, version, DB_UINT, source,
			 DB_UINT, installed) != 1)
	{
		DBG1(DBG_IMC, "unable to insert sw_id into database");
		return 0;
	}

	return sw_id;
}

METHOD(sw_collector_db_t, get_sw_id, uint32_t,
	private_sw_collector_db_t *this, char *name, char **package, char **version,
	uint8_t *source, bool *installed)
{
	char *sw_package, *sw_version;
	uint32_t sw_id = 0, sw_source, sw_installed;
	enumerator_t *e;

	/* Does software identifier already exist in database? */
	e = this->db->query(this->db,
			"SELECT id, package, version, source, installed "
			"FROM sw_identifiers WHERE name = ?",
			DB_TEXT, name, DB_UINT, DB_TEXT, DB_TEXT, DB_UINT, DB_UINT);
	if (!e)
	{
		DBG1(DBG_IMC, "database query for sw_identifier failed");
		return 0;
	}
	if (e->enumerate(e, &sw_id, &sw_package, &sw_version, &sw_source,
						&sw_installed))
	{
		if (package)
		{
			*package = strdup(sw_package);
		}
		if (version)
		{
			*version = strdup(sw_version);
		}
		if (source)
		{
			*source = sw_source;
		}
		if (installed)
		{
			*installed = sw_installed;
		}
	}
	e->destroy(e);

	return sw_id;
}

METHOD(sw_collector_db_t, get_sw_id_count, uint32_t,
	private_sw_collector_db_t *this, sw_collector_db_query_t type)
{
	uint32_t count, installed;
	enumerator_t *e;

	if (type == SW_QUERY_ALL)
	{
		e = this->db->query(this->db,
			"SELECT COUNT(installed) FROM sw_identifiers", DB_UINT);
	}
	else
	{
		installed = (type == SW_QUERY_INSTALLED);
		e = this->db->query(this->db,
			"SELECT COUNT(installed) FROM sw_identifiers WHERE installed = ?",
			 DB_UINT, installed, DB_UINT);
	}

	if (!e)
	{
		DBG1(DBG_IMC, "database query for sw_identifier count failed");
		return 0;
	}
	if (!e->enumerate(e, &count))
	{
		count = 0;
	}
	e->destroy(e);

	return count;
}

METHOD(sw_collector_db_t, update_sw_id, bool,
	private_sw_collector_db_t *this, uint32_t sw_id, char *name, char *version,
	bool installed)
{
	int res;

	if (name && version)
	{
		res = this->db->execute(this->db, NULL,
			"UPDATE sw_identifiers SET name = ?, version = ?, installed = ? "
			"WHERE id = ?", DB_TEXT, name, DB_TEXT, version, DB_UINT, installed,
			 DB_UINT, sw_id);
	}
	else
	{
		res = this->db->execute(this->db, NULL,
			"UPDATE sw_identifiers SET installed = ? WHERE id = ?",
			 DB_UINT, installed, DB_UINT, sw_id);
	}
	if (res != 1)
	{
		DBG1(DBG_IMC, "unable to update software identifier in database");
		return FALSE;
	}
	return TRUE;
}

METHOD(sw_collector_db_t, update_package, int,
	private_sw_collector_db_t *this, char *package_filter, char *package)
{
	int count;

	count = this->db->execute(this->db, NULL,
			"UPDATE sw_identifiers SET package = ? WHERE package LIKE ?",
			 DB_TEXT, package, DB_TEXT, package_filter);
	if (count < 0)
	{
		DBG1(DBG_IMC, "unable to update package name in database");
	}

	return count;
}

METHOD(sw_collector_db_t, create_sw_enumerator, enumerator_t*,
	private_sw_collector_db_t *this, sw_collector_db_query_t type, char *package)
{
	enumerator_t *e;
	u_int installed;

	if (type == SW_QUERY_ALL)
	{
		if (package)
		{
			e = this->db->query(this->db,
				"SELECT id, name, package, version, installed "
				"FROM sw_identifiers WHERE package = ? ORDER BY name ASC",
				 DB_TEXT, package, DB_UINT, DB_TEXT, DB_TEXT, DB_TEXT, DB_UINT);
		}
		else
		{
			e = this->db->query(this->db,
				"SELECT id, name, package, version, installed "
				"FROM sw_identifiers ORDER BY name ASC",
				 DB_UINT, DB_TEXT, DB_TEXT, DB_TEXT, DB_UINT);
		}
	}
	else
	{
		installed = (type == SW_QUERY_INSTALLED);

		if (package)
		{
			e = this->db->query(this->db,
				"SELECT id, name, package, version, installed "
				"FROM sw_identifiers WHERE package = ? AND installed = ? "
				"ORDER BY name ASC", DB_TEXT, package, DB_UINT, installed,
				 DB_UINT, DB_TEXT, DB_TEXT, DB_TEXT, DB_UINT);
		}
		else
		{
			e = this->db->query(this->db,
				"SELECT id, name, package, version, installed "
				"FROM sw_identifiers WHERE installed = ? ORDER BY name ASC",
				 DB_UINT, installed, DB_UINT, DB_TEXT, DB_TEXT, DB_TEXT, DB_UINT);
		}
	}
	if (!e)
	{
		DBG1(DBG_IMC, "database query for sw_identifier count failed");
		return NULL;
	}

	return e;
}

METHOD(sw_collector_db_t, destroy, void,
	private_sw_collector_db_t *this)
{
	this->db->destroy(this->db);
	free(this);
}

/**
 * Determine file creation data and convert it into RFC 3339 format
 */
bool get_file_creation_date(char *pathname, char *timestamp)
{
	struct stat st;
	struct tm ct;

	if (stat(pathname, &st))
	{
		DBG1(DBG_IMC, "unable to obtain statistics on '%s'", pathname);
		return FALSE;
	}

	/* Convert from local time to UTC */
	gmtime_r(&st.st_mtime, &ct);
	ct.tm_year += 1900;
	ct.tm_mon += 1;

	/* Form timestamp according to RFC 3339 (20 characters) */
	snprintf(timestamp, 21, "%4d-%02d-%02dT%02d:%02d:%02dZ",
			 ct.tm_year, ct.tm_mon, ct.tm_mday,
			 ct.tm_hour, ct.tm_min, ct.tm_sec);

	return TRUE;
}

/**
 * Described in header.
 */
sw_collector_db_t *sw_collector_db_create(char *uri)
{
	private_sw_collector_db_t *this;
	uint32_t first_eid, last_eid;
	char first_time_buf[21], *first_time, *first_file;

	INIT(this,
		.public = {
			.add_event = _add_event,
			.get_last_event = _get_last_event,
			.add_sw_event = _add_sw_event,
			.set_sw_id = _set_sw_id,
			.get_sw_id = _get_sw_id,
			.get_sw_id_count = _get_sw_id_count,
			.update_sw_id = _update_sw_id,
			.update_package = _update_package,
			.create_sw_enumerator = _create_sw_enumerator,
			.destroy = _destroy,
		},
		.db = lib->db->create(lib->db, uri),
	);

	if (!this->db)
	{
		DBG1(DBG_IMC, "opening database URI '%s' failed", uri);
		free(this);
		return NULL;
	}

	/* Retrieve last event in database */
	if (!get_last_event(this, &last_eid, &this->epoch, NULL))
	{
		destroy(this);
		return NULL;
	}

	/* Create random epoch and first event if no events exist yet */
	if (!last_eid)
	{
		rng_t *rng;

		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng ||
			!rng->get_bytes(rng, sizeof(uint32_t), (uint8_t*)&this->epoch))
		{
			DESTROY_IF(rng);
			destroy(this);
			DBG1(DBG_IMC, "generating random epoch value failed");
			return NULL;
		}
		rng->destroy(rng);

		/* strongTNC workaround - limit epoch to 31 bit unsigned integer */
		this->epoch &= 0x7fffffff;

		/* Create first event when the OS was installed */
		first_file = lib->settings->get_str(lib->settings,
						"sw-collector.first_file", "/var/log/bootstrap.log");
		first_time = lib->settings->get_str(lib->settings,
						"sw-collector.first_time", NULL);
		if (!first_time)
		{
			if (get_file_creation_date(first_file, first_time_buf))
			{
				first_time = first_time_buf;
			}
			else
			{
				first_time = "0000-00-00T00:00:00Z";
			}
		}
		first_eid = add_event(this, first_time);

		if (!first_eid)
		{
			destroy(this);
			return NULL;
		}
		DBG0(DBG_IMC, "First-Date: %s, eid = %u, epoch = %u",
					   first_time, first_eid, this->epoch);
	}

	return &this->public;
}
