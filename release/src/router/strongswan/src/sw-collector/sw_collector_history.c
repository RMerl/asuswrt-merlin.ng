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
#include <stdio.h>
#include <time.h>

#include "sw_collector_history.h"
#include "sw_collector_dpkg.h"

#include <swima/swima_event.h>
#include <swid_gen/swid_gen_info.h>

typedef struct private_sw_collector_history_t private_sw_collector_history_t;

/**
 * Private data of an sw_collector_history_t object.
 */
struct private_sw_collector_history_t {

	/**
	 * Public members of sw_collector_history_state_t
	 */
	sw_collector_history_t public;

	/**
	 * Software Event Source Number
	 */
	uint8_t source;

	/**
	 * Reference to OS info object
	 */
	swid_gen_info_t *info;

	/**
	 * Reference to collector database
	 */
	sw_collector_db_t *db;

};

/**
 * Define auxiliary package_t list item object
 */
typedef struct package_t package_t;

struct package_t {
	char *package;
	char *version;
	char *old_version;
	char *sw_id;
	char *old_sw_id;
};

/**
 * Create package_t list item object
 */
static package_t* create_package(swid_gen_info_t *info, chunk_t package,
								 chunk_t version, chunk_t old_version)
{
	package_t *this;

	INIT(this,
		.package = strndup(package.ptr, package.len),
		.version = strndup(version.ptr, version.len),
		.old_version = strndup(old_version.ptr, old_version.len),
	)

	this->sw_id = info->create_sw_id(info, this->package, this->version);
	if (old_version.len)
	{
		this->old_sw_id = info->create_sw_id(info, this->package,
												   this->old_version);
	}

	return this;
}

/**
 * Free package_t list item object
 */
static void free_package(package_t *this)
{
	if (this)
	{
		free(this->package);
		free(this->version);
		free(this->old_version);
		free(this->sw_id);
		free(this->old_sw_id);
		free(this);
	}
}

/**
 * Extract and parse a single package item
 */
static package_t* extract_package(chunk_t item, swid_gen_info_t *info,
												sw_collector_history_op_t op)
{
	chunk_t package, package_stripped, version, old_version;
	package_t *p;

	/* extract package name */
	if (!extract_token(&package, ' ', &item))
	{
		fprintf(stderr, "version not found.\n");
		return NULL;
	}
	item = chunk_skip(item, 1);

	/* strip architecture suffix if present */
	if (extract_token(&package_stripped, ':', &package))
	{
		package = package_stripped;
	}

	/* extract versions */
	version = old_version = chunk_empty;

	if (item.len > 0)
	{
		if (extract_token(&version, ',', &item))
		{
			eat_whitespace(&item);
			if (!match("automatic", &item))
			{
				old_version = version;
				version = item;
			}
		}
		else
		{
			version = item;
		}
	}
	p = create_package(info, package, version, old_version);

	/* generate log entry */
	if (op == SW_OP_UPGRADE)
	{
		DBG2(DBG_IMC, "    %s (%s, %s)", p->package, p->old_version, p->version);
		DBG2(DBG_IMC, "      +%s", p->sw_id);
		DBG2(DBG_IMC, "      -%s", p->old_sw_id);
	}
	else
	{
		DBG2(DBG_IMC, "    %s (%s)", p->package, p->version);
		DBG2(DBG_IMC, "      %s%s", (op == SW_OP_INSTALL) ? "+" : "-", p->sw_id);
	}

	return p;
}

METHOD(sw_collector_history_t, extract_timestamp, bool,
	private_sw_collector_history_t *this, chunk_t args, char *buf)
{
	struct tm loc, utc;
	chunk_t t1, t2;
	time_t t;

	/* Break down local time with format t1 = yyyy-mm-dd and t2 = hh:mm:ss */
	if (!eat_whitespace(&args) || !extract_token(&t1, ' ', &args) ||
		!eat_whitespace(&args) || t1.len != 10 || args.len != 8)
	{
		DBG1(DBG_IMC, "unable to parse start-date");
		return FALSE;
	}
	t2 = args;

	if (sscanf(t1.ptr, "%4d-%2d-%2d",
						&loc.tm_year, &loc.tm_mon, &loc.tm_mday) != 3)
	{
		DBG1(DBG_IMC, "unable to parse date format yyyy-mm-dd");
		return FALSE;
	}
	loc.tm_year -= 1900;
	loc.tm_mon  -= 1;
	loc.tm_isdst = -1;

	if (sscanf(t2.ptr, "%2d:%2d:%2d",
						&loc.tm_hour, &loc.tm_min, &loc.tm_sec) != 3)
	{
		DBG1(DBG_IMC, "unable to parse time format hh:mm:ss");
		return FALSE;
	}

	/* Convert from local time to UTC */
	t = mktime(&loc);
	gmtime_r(&t, &utc);
	utc.tm_year += 1900;
	utc.tm_mon  += 1;

	/* Form timestamp according to RFC 3339 (20 characters) */
	snprintf(buf, 21, "%4d-%02d-%02dT%02d:%02d:%02dZ",
			 utc.tm_year, utc.tm_mon, utc.tm_mday,
			 utc.tm_hour, utc.tm_min, utc.tm_sec);

	return TRUE;
}

METHOD(sw_collector_history_t, extract_packages, bool,
	private_sw_collector_history_t *this, chunk_t args, uint32_t eid,
	sw_collector_history_op_t op)
{
	bool success = FALSE;
	package_t *p = NULL;
	chunk_t item;

	eat_whitespace(&args);

	while (extract_token(&item, ')', &args))
	{
		char *del_sw_id = NULL, *del_version = NULL;
		char *nx, *px, *vx, *v1;
		bool installed;
		u_int sw_idx, ix;
		uint32_t sw_id, sw_id_epoch_less = 0;
		enumerator_t *e;

		p = extract_package(item, this->info, op);
		if (!p)
		{
			goto end;
		}

		/* packages without version information cannot be handled */
		if (strlen(p->version) == 0)
		{
			free_package(p);
			continue;
		}

		switch (op)
		{
			case SW_OP_REMOVE:
				/* prepare subsequent deletion sw event */
				del_sw_id = p->sw_id;
				del_version = p->version;
				break;
			case SW_OP_UPGRADE:
				/* prepare subsequent deletion sw event */
				del_sw_id = p->old_sw_id;
				del_version = p->old_version;
				/* fall through to next case */
			case SW_OP_INSTALL:
				sw_id = this->db->get_sw_id(this->db, p->sw_id, NULL, NULL,
											NULL, &installed);
				if (sw_id)
				{
					/* sw identifier exists - update state to 'installed' */
					if (installed)
					{
						/* this case should not occur */
						DBG1(DBG_IMC, "  warning:  sw_id %d is already "
									  "installed", sw_id);
					}
					else if (!this->db->update_sw_id(this->db, sw_id, NULL,
													 NULL, TRUE))
					{
						goto end;
					}
				}
				else
				{
					/* new sw identifier - create with state 'installed' */
					sw_id = this->db->set_sw_id(this->db, p->sw_id, p->package,
												p->version,	this->source, TRUE);
					if (!sw_id)
					{
						goto end;
					}
				}

				/* add creation sw event with current eid */
				if (!this->db->add_sw_event(this->db, eid, sw_id,
									SWIMA_EVENT_ACTION_CREATION))
				{
					goto end;
				}
				break;
		}

		if (op != SW_OP_INSTALL)
		{
			sw_id = 0;

			/* look for existing installed package versions */
			e = this->db->create_sw_enumerator(this->db, SW_QUERY_INSTALLED,
											   p->package);
			if (!e)
			{
				goto end;
			}

			while (e->enumerate(e, &sw_idx, &nx, &px, &vx, &ix))
			{
				if (streq(vx, del_version))
				{
					/* full match with epoch */
					sw_id = sw_idx;
					break;
				}
				v1 = strchr(vx, ':');
				if (v1 && streq(++v1, del_version))
				{
					/* match with stripped epoch */
					sw_id_epoch_less = sw_idx;
				}
			}
			e->destroy(e);

			if (!sw_id && sw_id_epoch_less)
			{
				/* no full match - fall back to epoch-less match */
				sw_id = sw_id_epoch_less;
			}
			if (sw_id)
			{
				/* sw identifier exists - update state to 'removed' */
				if (!this->db->update_sw_id(this->db, sw_id, NULL, NULL, FALSE))
				{
					goto end;
				}
			}
			else
			{
				/* new sw identifier - create with state 'removed' */
				sw_id = this->db->set_sw_id(this->db, del_sw_id, p->package,
									del_version, this->source, FALSE);

				/* add creation sw event with eid = 1 */
				if (!sw_id || !this->db->add_sw_event(this->db, 1, sw_id,
											SWIMA_EVENT_ACTION_CREATION))
				{
					goto end;
				}
			}

			/* add creation sw event with current eid */
			if (!this->db->add_sw_event(this->db, eid, sw_id,
								SWIMA_EVENT_ACTION_DELETION))
			{
				goto end;
			}
		}
		free_package(p);

		if (args.len < 2)
		{
			break;
		}
		args = chunk_skip(args, 2);
	}
	p = NULL;
	success = TRUE;

end:
	free_package(p);

	return success;
}

METHOD(sw_collector_history_t, merge_installed_packages, bool,
	private_sw_collector_history_t *this)
{
	uint32_t sw_id, count = 0;
	char *package, *arch, *version, *v1, *name, *n1;
	bool installed, success = FALSE;
	sw_collector_dpkg_t *dpkg;
	enumerator_t *enumerator;

	DBG1(DBG_IMC, "Merging:");

	dpkg = sw_collector_dpkg_create();
	if (!dpkg)
	{
		return FALSE;
	}

	enumerator = dpkg->create_sw_enumerator(dpkg);
	while (enumerator->enumerate(enumerator, &package, &arch, &version))
	{
		name = this->info->create_sw_id(this->info, package, version);
		DBG3(DBG_IMC, "  %s merged", name);

		sw_id = this->db->get_sw_id(this->db, name, NULL, NULL, NULL,
									&installed);
		if (sw_id)
		{
			if (!installed)
			{
				DBG1(DBG_IMC, "  warning: existing sw_id %u"
							  " is not installed", sw_id);

				if (!this->db->update_sw_id(this->db, sw_id, name, version,
											TRUE))
				{
					free(name);
					goto end;
				}
			}
		}
		else
		{
			/* check for a Debian epoch number */
			v1 = strchr(version, ':');
			if (v1)
			{
				/* check for existing and installed epoch-less version */
				n1 = this->info->create_sw_id(this->info, package, ++v1);
				sw_id = this->db->get_sw_id(this->db, n1, NULL, NULL, NULL,
											&installed);
				free(n1);

				if (sw_id && installed)
				{
					/* add epoch to existing version */
					if (!this->db->update_sw_id(this->db, sw_id, name, version,
												installed))
					{
						free(name);
						goto end;
					}
				}
				else
				{
					sw_id = 0;
				}
			}
		}

		if (!sw_id)
		{
			/* new sw identifier - create with state 'installed' */
			sw_id = this->db->set_sw_id(this->db, name, package, version,
										this->source, TRUE);

			/* add creation sw event with eid = 1 */
			if (!sw_id || !this->db->add_sw_event(this->db, 1, sw_id,
										SWIMA_EVENT_ACTION_CREATION))
			{
				free(name);
				goto end;
			}

		}
		free(name);
		count++;
	}
	success = TRUE;

	DBG1(DBG_IMC, "  merged %u installed packages, %u registered in database",
		 count, this->db->get_sw_id_count(this->db, SW_QUERY_INSTALLED));

end:
	enumerator->destroy(enumerator);
	dpkg->destroy(dpkg);

	return success;
}

METHOD(sw_collector_history_t, destroy, void,
	private_sw_collector_history_t *this)
{
	this->info->destroy(this->info);
	free(this);
}

/**
 * Described in header.
 */
sw_collector_history_t *sw_collector_history_create(sw_collector_db_t *db,
													uint8_t source)
{
	private_sw_collector_history_t *this;
	swid_gen_info_t *info;
	os_type_t os_type;
	char *os;

	info = swid_gen_info_create();

	/* check if OS supports apg/dpkg history logs */
	info->get_os(info, &os);
	os_type = info->get_os_type(info);
	if (os_type != 	OS_TYPE_DEBIAN && os_type != OS_TYPE_UBUNTU)
	{
		DBG1(DBG_IMC, "%.*s not supported", os);
		info->destroy(info);
		return NULL;
	}

	INIT(this,
		.public = {
			.extract_timestamp = _extract_timestamp,
			.extract_packages = _extract_packages,
			.merge_installed_packages = _merge_installed_packages,
			.destroy = _destroy,
		},
		.source = source,
		.info = info,
		.db = db,
	);

	return &this->public;
}
