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

#define _GNU_SOURCE /* for asprintf() */

#include "swima_collector.h"

#include <swid_gen/swid_gen.h>

#include <collections/linked_list.h>
#include <utils/debug.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

#define SOURCE_ID_GENERATOR		1
#define SOURCE_ID_COLLECTOR		2

#ifndef SWID_DIRECTORY
#define SWID_DIRECTORY NULL
#endif

/**
 * Directories to be skipped by collector
 */
static const char* skip_directories[] = {
	"/usr/share/doc",
	"/usr/share/help",
	"/usr/share/icons",
	"/usr/share/gnome/help"
};

typedef struct private_swima_collector_t private_swima_collector_t;

/**
 * Private data of a swima_collector_t object.
 *
 */
struct private_swima_collector_t {

	/**
	 * Public swima_collector_t interface.
	 */
	swima_collector_t public;

	/**
	 * Collect Software Identifiers only
	 */
	bool sw_id_only;

	/**
	 * Software Collector Database [if it exists]
	 */
	database_t *db;

	/**
	 * List of Software [Identifier] records
	 */
	swima_inventory_t *inventory;

	/**
	 * List of Software [Identifier] events
	 */
	swima_events_t *events;

};

/**
 * Extract Software Identifier from SWID tag
 */
static status_t extract_sw_id(chunk_t swid_tag, chunk_t *sw_id)
{
	char *pos, *tag, *tagid, *regid;
	size_t len, tagid_len, regid_len;
	status_t status = NOT_FOUND;

	/* Copy at most 1023 bytes of the SWID tag and null-terminate it */
	len = min(1023, swid_tag.len);
	pos = tag = strndup(swid_tag.ptr, len);

	tagid= strstr(pos, "tagId=\"");
	if (tagid == NULL)
	{
		goto end;
	}
	tagid += 7;
	len -= tagid - pos - 7;

	pos = strchr(tagid, '"');
	if (pos == NULL)
	{
		goto end;
	}
	tagid_len = pos - tagid;

	regid= strstr(pos, "regid=\"");
	if (regid == NULL)
	{
		goto end;
	}
	regid += 7;
	len -= regid - pos - 7;

	pos = strchr(regid, '"');
	if (pos == NULL)
	{
		goto end;
	}
	regid_len = pos - regid;

	*sw_id = chunk_cat("ccc", chunk_create(regid, regid_len),
							  chunk_from_chars('_','_'),
							  chunk_create(tagid, tagid_len));
	status = SUCCESS;
end:
	free(tag);

	return status;
}

static status_t retrieve_inventory(private_swima_collector_t *this,
								   swima_inventory_t *targets)
{
	char *name;
	uint32_t record_id, source;
	swima_record_t *sw_record;
	chunk_t sw_id;
	enumerator_t *e;

	/* Retrieve complete software identifier inventory */
	e = this->db->query(this->db,
			"SELECT id, name, source FROM sw_identifiers WHERE installed = 1 "
			"ORDER BY name ASC", DB_UINT, DB_TEXT, DB_UINT);
	if (!e)
	{
		DBG1(DBG_IMC, "database query for installed sw_identifiers failed");
		return FAILED;
	}
	while (e->enumerate(e, &record_id, &name, &source))
	{
		sw_id = chunk_from_str(name);
		sw_record = swima_record_create(record_id, sw_id, chunk_empty);
		sw_record->set_source_id(sw_record, source);
		this->inventory->add(this->inventory, sw_record);
	}
	e->destroy(e);

	return SUCCESS;
}

static status_t retrieve_events(private_swima_collector_t *this,
							  	swima_inventory_t *targets)
{
	enumerator_t *e;
	char *name, *timestamp;
	uint32_t record_id, source, action, eid, earliest_eid;
	chunk_t sw_id, ev_ts;
	swima_record_t *sw_record;
	swima_event_t *sw_event;

	earliest_eid = targets->get_eid(targets, NULL);

	/* Retrieve complete software identifier inventory */
	e = this->db->query(this->db,
		"SELECT e.id, e.timestamp, i.id, i.name, i.source, s.action "
		"FROM sw_events as s JOIN events AS e ON s.eid = e.id "
		"JOIN sw_identifiers as i ON s.sw_id = i.id WHERE s.eid >= ?"
		"ORDER BY s.eid, i.name, s.action ASC", DB_UINT, earliest_eid,
		 DB_UINT, DB_TEXT, DB_UINT, DB_TEXT, DB_UINT, DB_UINT);
	if (!e)
	{
		DBG1(DBG_IMC, "database query for sw_events failed");
		return FAILED;
	}
	while (e->enumerate(e, &eid, &timestamp, &record_id, &name, &source, &action))
	{
		sw_id = chunk_from_str(name);
		ev_ts = chunk_from_str(timestamp);
		sw_record = swima_record_create(record_id, sw_id, chunk_empty);
		sw_record->set_source_id(sw_record, source);
		sw_event = swima_event_create(eid, ev_ts, action, sw_record);
		this->events->add(this->events, sw_event);
	}
	e->destroy(e);

	return SUCCESS;
}

static status_t generate_tags(private_swima_collector_t *this,
							  swima_inventory_t *targets, bool pretty, bool full)
{
	swid_gen_t *swid_gen;
	swima_record_t *target, *sw_record;
	enumerator_t *enumerator;
	status_t status = SUCCESS;

	swid_gen = swid_gen_create();

	if (targets->get_count(targets) == 0)
	{
		chunk_t out, sw_id, swid_tag = chunk_empty;

		DBG2(DBG_IMC, "SWID tag%s generation by package manager",
					   this->sw_id_only ? " ID" : "");

		enumerator = swid_gen->create_tag_enumerator(swid_gen, this->sw_id_only,
													 full, pretty);
		if (enumerator)
		{
			while (enumerator->enumerate(enumerator, &out))
			{
				if (this->sw_id_only)
				{
					sw_id = out;
				}
				else
				{
					swid_tag = out;
					status = extract_sw_id(swid_tag, &sw_id);
					if (status != SUCCESS)
					{
						DBG1(DBG_IMC, "software id could not be extracted "
									  "from tag");
						chunk_free(&swid_tag);
						break;
					}
				}
				sw_record = swima_record_create(0, sw_id, chunk_empty);
				sw_record->set_source_id(sw_record, SOURCE_ID_GENERATOR);
				if (!this->sw_id_only)
				{
					sw_record->set_record(sw_record, swid_tag);
					chunk_free(&swid_tag);
				}
				this->inventory->add(this->inventory, sw_record);
				chunk_free(&sw_id);
			}
			enumerator->destroy(enumerator);
		}
		else
		{
			status = NOT_SUPPORTED;
		}
	}
	else if (!this->sw_id_only)
	{
		DBG2(DBG_IMC, "targeted SWID tag generation");

		enumerator = targets->create_enumerator(targets);
		while (enumerator->enumerate(enumerator, &target))
		{
			swima_record_t *sw_record;
			char *tag = NULL, *name, *package, *version;
			u_int installed;
			chunk_t sw_id;
			enumerator_t *e;

			sw_id = target->get_sw_id(target, NULL);
			name = strndup(sw_id.ptr, sw_id.len);

			if (this->db)
			{
				e = this->db->query(this->db,
						"SELECT package, version, installed "
						"FROM sw_identifiers WHERE name = ?", DB_TEXT, name,
						 DB_TEXT, DB_TEXT, DB_UINT);
				if (!e)
				{
					DBG1(DBG_IMC, "database query for sw_identifiers failed");
					status = FAILED;
					free(name);
					break;
				}
				if (e->enumerate(e, &package, &version, &installed))
				{
					tag = swid_gen->generate_tag(swid_gen, name, package,
									version, full && installed, pretty);
				}
				e->destroy(e);
			}
			else
			{
				tag = swid_gen->generate_tag(swid_gen, name, NULL, NULL,
											 full, pretty);
			}
			free(name);

			if (tag)
			{
				DBG2(DBG_IMC, "  %.*s", sw_id.len, sw_id.ptr);
				sw_record = swima_record_create(0, sw_id, chunk_empty);
				sw_record->set_source_id(sw_record, SOURCE_ID_GENERATOR);
				sw_record->set_record(sw_record, chunk_from_str(tag));
				this->inventory->add(this->inventory, sw_record);
				free(tag);
			}
		}
		enumerator->destroy(enumerator);
	}
	swid_gen->destroy(swid_gen);

	return status;
}

static bool collect_tags(private_swima_collector_t *this, char *pathname,
						 swima_inventory_t *targets, bool is_swidtag_dir)
{
	char *rel_name, *abs_name, *suffix, *pos, *uri;
	chunk_t *swid_tag, sw_id, sw_locator;
	swima_record_t *sw_record;
	struct stat st;
	bool success = FALSE, skip, is_new_swidtag_dir;
	enumerator_t *enumerator;
	int i;

	if (!pathname)
	{
		return TRUE;
	}

	enumerator = enumerator_create_directory(pathname);
	if (!enumerator)
	{
		DBG1(DBG_IMC, "directory '%s' can not be opened, %s",
					   pathname, strerror(errno));
		return FALSE;
	}

	while (enumerator->enumerate(enumerator, &rel_name, &abs_name, &st))
	{
		if (S_ISDIR(st.st_mode))
		{
			skip = FALSE;

			for (i = 0; i < countof(skip_directories); i++)
			{
				if (streq(abs_name, skip_directories[i]))
				{
					skip = TRUE;
					break;
				}
			}

			if (skip)
			{
				continue;
			}

			is_new_swidtag_dir =  streq(rel_name, "swidtag");
			if (is_new_swidtag_dir)
			{
				DBG2(DBG_IMC, "entering %s", pathname);
			}
			if (!collect_tags(this, abs_name, targets, is_swidtag_dir ||
													   is_new_swidtag_dir))
			{
				goto end;
			}
			if (is_new_swidtag_dir)
			{
				DBG2(DBG_IMC, "leaving %s", pathname);
			}
		}

		if (!is_swidtag_dir)
		{
			continue;
		}

		/* found a swidtag file? */
		suffix = strstr(rel_name, ".swidtag");
		if (!suffix)
		{
			continue;
		}

		/* load the swidtag file */
		swid_tag = chunk_map(abs_name, FALSE);
		if (!swid_tag)
		{
			DBG1(DBG_IMC, "  opening '%s' failed: %s", abs_name,
						  strerror(errno));
			goto end;
		}

		/* extract software identity from SWID tag */
		if (extract_sw_id(*swid_tag, &sw_id) != SUCCESS)
		{
			DBG1(DBG_IMC, "software id could not be extracted from SWID tag");
			chunk_unmap(swid_tag);
			goto end;
		}

		/* In case of a targeted request */
		if (targets->get_count(targets))
		{
			enumerator_t *target_enumerator;
			swima_record_t *target;
			bool match = FALSE;

			target_enumerator = targets->create_enumerator(targets);
			while (target_enumerator->enumerate(target_enumerator, &target))
			{
				if (chunk_equals(target->get_sw_id(target, NULL), sw_id))
				{
					DBG2(DBG_IMC, "  %.*s", sw_id.len, sw_id.ptr);
					match = TRUE;
					break;
				}
			}
			target_enumerator->destroy(target_enumerator);

			if (!match)
			{
				chunk_unmap(swid_tag);
				chunk_free(&sw_id);
				continue;
			}
		}
		DBG2(DBG_IMC, "  %s", rel_name);

		sw_locator = chunk_empty;
		pos = strstr(pathname, "/swidtag");
		if (pos && asprintf(&uri, "file://%.*s", pos - pathname, pathname) > 0)
		{
			sw_locator = chunk_from_str(uri);
		}
		sw_record = swima_record_create(0, sw_id, sw_locator);
		sw_record->set_source_id(sw_record, SOURCE_ID_COLLECTOR);
		if (!this->sw_id_only)
		{
			sw_record->set_record(sw_record, *swid_tag);
		}
		this->inventory->add(this->inventory, sw_record);

		chunk_unmap(swid_tag);
		chunk_free(&sw_id);
		chunk_free(&sw_locator);
	}
	success = TRUE;

end:
	enumerator->destroy(enumerator);

	return success;
}

METHOD(swima_collector_t, collect_inventory, swima_inventory_t*,
	private_swima_collector_t *this, bool sw_id_only, swima_inventory_t *targets)
{
	bool pretty, full;
	char *directory;
	status_t status;

	directory = lib->settings->get_str(lib->settings,
									"%s.plugins.imc-swima.swid_directory",
									 SWID_DIRECTORY, lib->ns);
	pretty = lib->settings->get_bool(lib->settings,
									"%s.plugins.imc-swima.swid_pretty",
									 FALSE, lib->ns);
	full = lib->settings->get_bool(lib->settings,
									"%s.plugins.imc-swima.swid_full",
									 FALSE, lib->ns);

	/**
	 * Re-initialize collector
	 */
	this->sw_id_only = sw_id_only;
	this->inventory->clear(this->inventory);

	/**
	 * Source 1: Tags are generated by a package manager
	 */
	if (sw_id_only && this->db)
	{
		status = retrieve_inventory(this, targets);
	}
	else
	{
		status = generate_tags(this, targets, pretty, full);
	}

	/**
	 * Source 2: Collect swidtag files by iteratively entering all
	 *           directories in the tree under the "directory" path.
	 */
	DBG2(DBG_IMC, "SWID tag%s collection", sw_id_only ? " ID" : "");
	collect_tags(this, directory, targets, FALSE);

	return status == SUCCESS ? this->inventory : NULL;
}

METHOD(swima_collector_t, collect_events, swima_events_t*,
	private_swima_collector_t *this, bool sw_id_only, swima_inventory_t *targets)
{
	if (!sw_id_only || !this->db)
	{
		return NULL;
	}

	/**
	 * Re-initialize collector
	 */
	this->sw_id_only = sw_id_only;
	this->events->clear(this->events);

	return retrieve_events(this, targets) == SUCCESS ? this->events : NULL;
}

METHOD(swima_collector_t, destroy, void,
	private_swima_collector_t *this)
{
	DESTROY_IF(this->db);
	this->inventory->destroy(this->inventory);
	this->events->destroy(this->events);
	free(this);
}

/**
 * See header
 */
swima_collector_t *swima_collector_create(void)
{
	private_swima_collector_t *this;
	char *database;
	uint32_t last_eid = 1, eid_epoch = 0x11223344;

	INIT(this,
		.public = {
			.collect_inventory = _collect_inventory,
			.collect_events = _collect_events,
			.destroy = _destroy,
		},
		.inventory = swima_inventory_create(),
		.events = swima_events_create(),
	);

	database = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-swima.swid_database", NULL, lib->ns);

	/* If we have an URI, try to connect to sw_collector database */
	if (database)
	{
		database_t *db = lib->db->create(lib->db, database);

		if (db)
		{
			enumerator_t *e;

			/* Get last event ID and corresponding epoch */
			e = db->query(db,
					"SELECT id, epoch FROM events ORDER BY timestamp DESC",
					 DB_UINT, DB_UINT);
			if (!e || !e->enumerate(e, &last_eid, &eid_epoch))
			{
				DBG1(DBG_IMC, "database query for last event failed");
				DESTROY_IF(e);
				db->destroy(db);
			}
			else
			{
				/* The query worked, attach collector database permanently */
				e->destroy(e);
				this->db = db;
			}
		}
		else
		{
			DBG1(DBG_IMC, "opening sw-collector database URI '%s' failed",
						   database);
		}
	}
	if (!this->db)
	{
		/* Set the event ID epoch and last event ID smanually */
		eid_epoch = lib->settings->get_int(lib->settings,
								"%s.plugins.imc-swima.eid_epoch",
								 eid_epoch, lib->ns);
	}
	this->inventory->set_eid(this->inventory, last_eid, eid_epoch);
	this->events->set_eid(this->events, last_eid, eid_epoch);

	return &this->public;
}
