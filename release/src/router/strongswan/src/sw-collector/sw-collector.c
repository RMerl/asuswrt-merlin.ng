/*
 * Copyright (C) 2017-2021 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sysexits.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif

#include "sw_collector_db.h"
#include "sw_collector_history.h"
#include "sw_collector_rest_api.h"
#include "sw_collector_dpkg.h"

#include <library.h>
#include <utils/debug.h>
#include <utils/lexparser.h>
#include <collections/hashtable.h>

#include <swid_gen/swid_gen.h>
#include <swid_gen/swid_gen_info.h>

#define DEFAULT_HISTORY_LOG		"/var/log/apt/history.log"
#define NO_ITERATION			-1

/**
 * global debug output variables
 */
static int debug_level = 2;
static bool stderr_quiet = FALSE;
static int max_count = 0;

typedef enum collector_op_t collector_op_t;

enum collector_op_t {
	COLLECTOR_OP_EXTRACT,
	COLLECTOR_OP_LIST,
	COLLECTOR_OP_UNREGISTERED,
	COLLECTOR_OP_GENERATE,
	COLLECTOR_OP_MIGRATE,
	COLLECTOR_OP_CHECK
};

/**
 * sw_collector dbg function
 */
static void sw_collector_dbg(debug_t group, level_t level, char *fmt, ...)
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

#ifdef HAVE_SYSLOG
		{
			int priority = LOG_INFO;
			char buffer[8192];
			char *current = buffer, *next;

			/* write in memory buffer first */
			va_start(args, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);

			/* do a syslog with every line */
			while (current)
			{
				next = strchr(current, '\n');
				if (next)
				{
					*(next++) = '\0';
				}
				syslog(priority, "%s\n", current);
				current = next;
			}
		}
#endif /* HAVE_SYSLOG */
	}
}

/**
 * atexit handler
 */
static void cleanup(void)
{
	library_deinit();
#ifdef HAVE_SYSLOG
	closelog();
#endif
}

/**
 * Display usage of sw-collector command
 */
static void usage(void)
{
	printf("\
Usage:\n\
  sw-collector --help\n\
  sw-collector [--debug <level>] [--quiet] [--count <event count>]\n\
  sw-collector [--debug <level>] [--quiet] [--installed|--removed] \
--list|-unregistered\n\
  sw-collector [--debug <level>] [--quiet] [--installed|--removed] \
[--full] --generate\n\
  sw-collector [--debug <level>] [--quiet] --migrate\n\
  sw-collector [--debug <level>] [--quiet] --check\n");
}

/**
 * Parse command line options
 */
static collector_op_t do_args(int argc, char *argv[], bool *full_tags,
							  sw_collector_db_query_t *query_type)
{
	collector_op_t op = COLLECTOR_OP_EXTRACT;
	bool installed = FALSE, removed = FALSE, full = FALSE;

	/* reinit getopt state */
	optind = 0;

	while (TRUE)
	{
		int c;

		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "check", no_argument, NULL, 'C' },
			{ "count", required_argument, NULL, 'c' },
			{ "debug", required_argument, NULL, 'd' },
			{ "full", no_argument, NULL, 'f' },
			{ "generate", no_argument, NULL, 'g' },
			{ "installed", no_argument, NULL, 'i' },
			{ "list", no_argument, NULL, 'l' },
			{ "migrate", no_argument, NULL, 'm' },
			{ "quiet", no_argument, NULL, 'q' },
			{ "removed", no_argument, NULL, 'r' },
			{ "unregistered", no_argument, NULL, 'u' },
			{ 0,0,0,0 }
		};

		c = getopt_long(argc, argv, "hCc:d:fgilmqru", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				usage();
				exit(SUCCESS);
			case 'C':
				op = COLLECTOR_OP_CHECK;
				continue;
			case 'c':
				max_count = atoi(optarg);
				continue;
			case 'd':
				debug_level = atoi(optarg);
				continue;
			case 'f':
				full = TRUE;
				continue;
			case 'g':
				op = COLLECTOR_OP_GENERATE;
				continue;
			case 'i':
				installed = TRUE;
				continue;
			case 'l':
				op = COLLECTOR_OP_LIST;
				continue;
			case 'm':
				op = COLLECTOR_OP_MIGRATE;
				continue;
			case 'q':
				stderr_quiet = TRUE;
				continue;
			case 'r':
				removed = TRUE;
				continue;
			case 'u':
				op = COLLECTOR_OP_UNREGISTERED;
				continue;
			default:
				usage();
				exit(EXIT_FAILURE);
		}
		break;
	}

	if ((!installed && !removed) || (installed && removed))
	{
		*query_type = SW_QUERY_ALL;
	}
	else if (installed)
	{
		*query_type = SW_QUERY_INSTALLED;
	}
	else
	{
		*query_type = SW_QUERY_REMOVED;
	}
	*full_tags = full;

	return op;
}

/**
 * Extract software events from a specific apt history log file
 */
static int extract_history_file(sw_collector_db_t *db,
								sw_collector_history_t *history, char *last_time,
								uint32_t last_eid, char *path, int level)
{
	chunk_t *h, history_chunk, line, command;
	char rfc_time[21], *new_path = NULL, *cmd = NULL;
	int status = EXIT_FAILURE, rc;
	bool skip = TRUE, first_skip = TRUE;
	size_t len, cmd_len;
	uint32_t eid = 0;

	h = chunk_map(path, FALSE);
	if (!h)
	{
		return EX_NOINPUT;
	}
	DBG1(DBG_IMC, "opened '%s'", path);

	history_chunk = *h;

	/* parse history file */
	while (fetchline(&history_chunk, &line))
	{
		if (line.len == 0)
		{
			continue;
		}
		if (!extract_token(&command, ':', &line))
		{
			fprintf(stderr, "terminator symbol ':' not found.\n");
			goto end;
		}
		if (match("Start-Date", &command))
		{
			if (!history->extract_timestamp(history, line, rfc_time))
			{
				goto end;
			}

			/* have we reached new history entries? */
			if (skip && strcmp(rfc_time, last_time) > 0)
			{
				if (first_skip && level != NO_ITERATION)
				{
					DBG0(DBG_IMC, "   Warning: %s of first entry on level %d"
								  " is newer", rfc_time, level);

					/* try to parse history log on next level */
					len = strlen(path) + 6;
					new_path = malloc(len);
					snprintf(new_path, len, DEFAULT_HISTORY_LOG ".%d", ++level);
					rc = extract_history_file(db, history, last_time, last_eid,
											  new_path, level);
					if (rc == EX_NOINPUT)
					{
						/* try to uncompress history log */
						cmd_len = strlen(new_path) + 20;
						cmd = malloc(cmd_len);
						snprintf(cmd, cmd_len, "/usr/bin/gunzip %s.gz", new_path);
						if (system(cmd) == 0)
						{
							rc = extract_history_file(db, history, last_time,
													  last_eid, new_path, level);
							if (rc == EX_NOINPUT)
							{
								fprintf(stderr, "opening '%s' failed: %s\n",
										path, strerror(errno));
							}

							/* re-compress the history log */
							snprintf(cmd, cmd_len, "/usr/bin/gzip %s", new_path);
							if (system(cmd) != 0)
							{
								fprintf(stderr, "gzip command failed");
							}
						}
						else
						{
							/* no further [compressed] history log available */
							rc = EXIT_SUCCESS;
						}
						free(cmd);
					}
					free(new_path);

					if (rc != EXIT_SUCCESS)
					{
						goto end;
					}
				}
				skip = FALSE;
			}
			first_skip = FALSE;

			if (skip)
			{
				continue;
			}

			/* insert new event into database */
			eid = db->add_event(db, rfc_time);
			if (!eid)
			{
				goto end;
			}
			DBG1(DBG_IMC, "Start-Date: %s, eid = %u", rfc_time, eid);
		}
		else if (skip)
		{
			/* skip old history entries which have already been processed */
			continue;
		}
		else if (match("Install", &command))
		{
			DBG1(DBG_IMC, "  Install:");
			if (!history->extract_packages(history, line, eid, SW_OP_INSTALL))
			{
				goto end;
			}
		}
		else if (match("Upgrade", &command))
		{
			DBG1(DBG_IMC, "  Upgrade:");
			if (!history->extract_packages(history, line, eid, SW_OP_UPGRADE))
			{
				goto end;
			}
		}
		else if (match("Remove", &command))
		{
			DBG1(DBG_IMC, "  Remove:");
			if (!history->extract_packages(history, line, eid, SW_OP_REMOVE))
			{
				goto end;
			}
		}
		else if (match("Purge", &command))
		{
			DBG1(DBG_IMC, "  Purge:");
			if (!history->extract_packages(history, line, eid, SW_OP_REMOVE))
			{
				goto end;
			}
		}
		else if (match("End-Date", &command))
		{
			/* Process 'max_count' events at a time */
			if (max_count > 0 && eid - last_eid == max_count)
			{
				fprintf(stderr, "added %d events\n", max_count);
				goto end;
			}
		}
	}
	status = EXIT_SUCCESS;

end:
	chunk_unmap(h);

	return status;
}

/**
 * Extract software events from apt history log files
 */
static int extract_history(sw_collector_db_t *db)
{
	sw_collector_history_t *history = NULL;
	uint32_t epoch, last_eid;
	int status = EXIT_FAILURE;
	char *path, *last_time = NULL;
	int level;

	/* open history file for reading */
	path = lib->settings->get_str(lib->settings, "%s.history",
								  DEFAULT_HISTORY_LOG, lib->ns);

	/* retrieve last event in database */
	if (!db->get_last_event(db, &last_eid, &epoch, &last_time) || !last_eid)
	{
		goto end;
	}
	DBG0(DBG_IMC, "Last-Event: %s, eid = %u, epoch = %u",
				   last_time, last_eid, epoch);

	/* iterate through history log files in the default path only */
	level = streq(path, DEFAULT_HISTORY_LOG) ? 0 : NO_ITERATION;

	history = sw_collector_history_create(db, 1);
	if (!history)
	{
		goto end;
	}

	status = extract_history_file(db, history, last_time, last_eid, path, level);
	if (status == EXIT_SUCCESS)
	{
		if (!history->merge_installed_packages(history))
		{
			status = EXIT_FAILURE;
		}
	}
	else if (status == EX_NOINPUT)
	{
		fprintf(stderr, "opening '%s' failed: %s\n", path, strerror(errno));
	}
	history->destroy(history);

end:
	free(last_time);
	return status;
}

/**
 * List all endpoint software identifiers stored in local collector database
 */
static int list_identifiers(sw_collector_db_t *db, sw_collector_db_query_t type)
{
	enumerator_t *e;
	char *name, *package, *version;
	uint32_t sw_id, count = 0, installed_count = 0, installed;

	e = db->create_sw_enumerator(db, type, NULL);
	if (!e)
	{
		return EXIT_FAILURE;
	}
	while (e->enumerate(e, &sw_id, &name, &package, &version, &installed))
	{
		printf("%s,%s,%s,%d\n", name, package, version, installed);
		if (installed)
		{
			installed_count++;
		}
		count++;
	}
	e->destroy(e);

	switch (type)
	{
		case SW_QUERY_ALL:
			DBG1(DBG_IMC, "retrieved %u software identities with %u installed "
				 "and %u removed", count, installed_count,
				 count - installed_count);
			break;
		case SW_QUERY_INSTALLED:
			DBG1(DBG_IMC, "retrieved %u installed software identities", count);
			break;
		case SW_QUERY_REMOVED:
			DBG1(DBG_IMC, "retrieved %u removed software identities", count);
			break;
	}

	return EXIT_SUCCESS;
}

static bool query_registry(sw_collector_rest_api_t *rest_api, bool installed)
{
	sw_collector_db_query_t type;
	enumerator_t *enumerator;
	char *sw_id;
	int count = 0;

	type = installed ? SW_QUERY_INSTALLED : SW_QUERY_REMOVED;
	enumerator = rest_api->create_sw_enumerator(rest_api, type);
	if (!enumerator)
	{
		return FALSE;
	}
	while (enumerator->enumerate(enumerator, &sw_id))
	{
		printf("%s,%s\n", sw_id, installed ? "1" : "0");
		count++;
	}
	enumerator->destroy(enumerator);
	DBG1(DBG_IMC, "%d %s software identifiers not registered", count,
				   installed ? "installed" : "removed");
	return TRUE;
}


/**
 * List all endpoint software identifiers stored in local collector database
 * that are not registered yet in central collector database
 */
static int unregistered_identifiers(sw_collector_db_t *db,
									sw_collector_db_query_t type)
{
	sw_collector_rest_api_t *rest_api;
	int status = EXIT_SUCCESS;

	rest_api = sw_collector_rest_api_create(db);
	if (!rest_api)
	{
		return EXIT_FAILURE;
	}

	/* List installed software identifiers not registered centrally */
	if (type != SW_QUERY_REMOVED && !query_registry(rest_api, TRUE))
	{
		status = EXIT_FAILURE;
	}

	/* List removed software identifiers not registered centrally */
	if (type != SW_QUERY_INSTALLED && !query_registry(rest_api, FALSE))
	{
		status = EXIT_FAILURE;
	}
	rest_api->destroy(rest_api);

	return status;
}

/**
 * Generate ISO 19770-2:2015 SWID tags for [installed|removed|all]
 * SW identifiers that are not registered centrally
 */
static int generate_tags(sw_collector_db_t *db, bool full_tags,
						 sw_collector_db_query_t type)
{
	swid_gen_t * swid_gen;
	sw_collector_rest_api_t *rest_api;
	char *name, *package, *version, *tag;
	enumerator_t *enumerator;
	uint32_t sw_id;
	bool installed;
	int count = 0, installed_count = 0, status = EXIT_FAILURE;

	swid_gen = swid_gen_create();
	rest_api = sw_collector_rest_api_create(db);
	if (!rest_api)
	{
		goto end;
	}

	enumerator = rest_api->create_sw_enumerator(rest_api, type);
	if (!enumerator)
	{
		goto end;
	}
	while (enumerator->enumerate(enumerator, &name))
	{
		sw_id = db->get_sw_id(db, name, &package, &version, NULL, &installed);
		if (sw_id)
		{
			tag = swid_gen->generate_tag(swid_gen, name, package, version,
										 full_tags && installed, FALSE);
			if (tag)
			{
				DBG2(DBG_IMC, "  creating %s", name);
				printf("%s\n", tag);
				free(tag);
				count++;
				if (installed)
				{
					installed_count++;
				}
			}
			free(package);
			free(version);
		}
	}
	enumerator->destroy(enumerator);
	status = EXIT_SUCCESS;

	switch (type)
	{
		case SW_QUERY_ALL:
			DBG1(DBG_IMC, "created %d tags for unregistered software "
				 "identifiers with %d installed and %d removed", count,
				 installed_count,  count - installed_count);
			break;
		case SW_QUERY_INSTALLED:
			DBG1(DBG_IMC, "created %d tags for unregistered installed software "
				 "identifiers", count);
			break;
		case SW_QUERY_REMOVED:
			DBG1(DBG_IMC, "created %d tags for unregistered removed software "
				 "identifiers", count);
			break;
	}

end:
	swid_gen->destroy(swid_gen);
	DESTROY_IF(rest_api);

	return status;
}

/**
 * Remove architecture suffix from package entries in the database
 */
static int migrate(sw_collector_db_t *db)
{
	sw_collector_dpkg_t *dpkg;

	char *package, *arch, *version;
	char package_filter[BUF_LEN];
	int res, count DBG_UNUSED = 0;
	int status = EXIT_SUCCESS;
	enumerator_t *enumerator;

	dpkg = sw_collector_dpkg_create();
	if (!dpkg)
	{
		return FAILED;
	}

	enumerator = dpkg->create_sw_enumerator(dpkg);
	while (enumerator->enumerate(enumerator, &package, &arch, &version))
	{

		/* Look for package names with architecture suffix */
		snprintf(package_filter, BUF_LEN, "%s:%%", package);

		res = db->update_package(db, package_filter, package);
		if (res < 0)
		{
				status = EXIT_FAILURE;
				break;
		}
		else if (res > 0)
		{
			count += res;
			DBG2(DBG_IMC, "%s: removed arch suffix %d times", package, res);
		}
	}
	enumerator->destroy(enumerator);
	dpkg->destroy(dpkg);

	DBG1(DBG_IMC, "migrated %d sw identifier records", count);

	return status;
}

/**
 * Free hashtable entry
 */
static void free_entry(void *value, void *key)
{
	free(value);
	free(key);
}

/**
 * Check consistency of installed software identifiers in collector database
 */
static int check(sw_collector_db_t *db)
{
	sw_collector_dpkg_t *dpkg;
	swid_gen_info_t *info;
	hashtable_t *table;
	enumerator_t *e;
	char *dpkg_name, *name, *package, *arch, *version;
	uint32_t sw_id, count = 0, installed;

	dpkg = sw_collector_dpkg_create();
	if (!dpkg)
	{
		return EXIT_FAILURE;
	}
	info = swid_gen_info_create();
	table = hashtable_create(hashtable_hash_str, hashtable_equals_str, 4096);

	/* Store all installed sw identifiers (according to dpkg) in hashtable */
	e = dpkg->create_sw_enumerator(dpkg);
	while (e->enumerate(e, &package, &arch, &version))
	{
		dpkg_name = info->create_sw_id(info, package, version);
		table->put(table, strdup(package), dpkg_name);
	}
	e->destroy(e);

	info->destroy(info);
	dpkg->destroy(dpkg);

	e = db->create_sw_enumerator(db, SW_QUERY_ALL, NULL);
	if (!e)
	{
		table->destroy_function(table, (void*)free_entry);
		return EXIT_FAILURE;
	}
	while (e->enumerate(e, &sw_id, &name, &package, &version, &installed))
	{
		dpkg_name = table->get(table, package);
		if (installed)
		{
			if (!dpkg_name)
			{
				printf("%4d %s erroneously noted as installed\n", sw_id, name);
			}
			else if (!streq(name, dpkg_name))
			{
				printf("%4d %s erroneously noted as installed instead of\n "
					   "    %s\n", sw_id, name, dpkg_name);
			}
		}
		else
		{
			if (dpkg_name && streq(name, dpkg_name))
			{
				printf("%4d %s erroneously noted as removed\n", sw_id, name);
			}
		}
		count++;
	}
	e->destroy(e);

	table->destroy_function(table, (void*)free_entry);
	printf("checked %d software identifiers\n", count);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	sw_collector_db_t *db = NULL;
	sw_collector_db_query_t query_type;
	collector_op_t op;
	bool full_tags;
	char *uri;
	int status = EXIT_FAILURE;

	op = do_args(argc, argv, &full_tags, &query_type);

	/* enable sw_collector debugging hook */
	dbg = sw_collector_dbg;
#ifdef HAVE_SYSLOG
	openlog("sw-collector", 0, LOG_DEBUG);
#endif

	atexit(cleanup);

	/* initialize library */
	if (!library_init(NULL, "sw-collector"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}

	/* load sw-collector plugins */
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "%s.load", PLUGINS, lib->ns)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	/* connect to sw-collector database */
	uri = lib->settings->get_str(lib->settings, "%s.database", NULL, lib->ns);
	if (!uri)
	{
		fprintf(stderr, "sw-collector.database URI not set.\n");
		exit(EXIT_FAILURE);
	}
	db = sw_collector_db_create(uri);
	if (!db)
	{
		fprintf(stderr, "connection to sw-collector database failed.\n");
		exit(EXIT_FAILURE);
	}

	switch (op)
	{
		case COLLECTOR_OP_EXTRACT:
			status = extract_history(db);
			break;
		case COLLECTOR_OP_LIST:
			status = list_identifiers(db, query_type);
			break;
		case COLLECTOR_OP_UNREGISTERED:
			status = unregistered_identifiers(db, query_type);
			break;
		case COLLECTOR_OP_GENERATE:
			status = generate_tags(db, full_tags, query_type);
			break;
		case COLLECTOR_OP_MIGRATE:
			status = migrate(db);
			break;
		case COLLECTOR_OP_CHECK:
			status = check(db);
			break;
	}
	db->destroy(db);

	exit(status);
}
