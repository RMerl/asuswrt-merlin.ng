/*
 * Copyright (C) 2012 Andreas Steffen
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
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <sys/stat.h>

#include "imv_os_state.h"

#include <library.h>
#include <utils/debug.h>

typedef enum pacman_state_t pacman_state_t;

enum pacman_state_t {
	PACMAN_STATE_BEGIN_PACKAGE,
	PACMAN_STATE_VERSION,
	PACMAN_STATE_END_PACKAGE
};

typedef struct stats_t stats_t;

struct stats_t {
	time_t release;
	int product;
	int packages;
	int new_packages;
	int new_versions;
	int updated_versions;
	int deleted_versions;
};

/**
 * global debug output variables
 */
static int debug_level = 1;
static bool stderr_quiet = TRUE;

/**
 * pacman dbg function
 */
static void pacman_dbg(debug_t group, level_t level, char *fmt, ...)
{
	int priority = LOG_INFO;
	char buffer[8192];
	char *current = buffer, *next;
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
}

/**
 * atexit handler to close everything on shutdown
 */
static void cleanup(void)
{
	closelog();
	library_deinit();
}

static void usage(void)
{
 	printf("Usage:\n"
		   "ipsec pacman --product <name> --file <filename> [--update]\n");
}

/**
 * Update the package database
 */
static bool update_database(database_t *db, char *package, char *version,
							bool security, stats_t *stats)
{
	char *cur_version, *version_update = NULL, *version_delete = NULL;
	int cur_security, security_update = 0, security_delete = 0;
	int pac_id = 0, vid = 0, vid_update = 0, vid_delete = 0;
	u_int cur_time;
	bool add_version = TRUE;
	enumerator_t *e;

	/* increment package count */
	stats->packages++;

	/* check if package is already in database */
	e = db->query(db, "SELECT id FROM packages WHERE name = ?",
					  DB_TEXT, package, DB_INT);
	if (!e)
	{
		return FALSE;
	}
	if (!e->enumerate(e, &pac_id))
	{
		pac_id = 0;
	}
	e->destroy(e);

	if (!pac_id && security)
	{
		if (db->execute(db, &pac_id, "INSERT INTO packages (name) VALUES (?)",
						DB_TEXT, package) != 1)
		{
			fprintf(stderr, "could not store package '%s' to database\n",
							 package);
			return FALSE;
		}
		stats->new_packages++;
	}

	/* check for package versions already in database */
	e = db->query(db,
			"SELECT id, release, security, time FROM versions "
			"WHERE package = ? AND product = ?", DB_INT, pac_id,
			 DB_INT, stats->product, DB_INT, DB_TEXT, DB_INT, DB_UINT);
	if (!e)
	{
		return FALSE;
	}

	while (e->enumerate(e, &vid, &cur_version, &cur_security, &cur_time))
	{
		if (streq(version, cur_version))
		{
			/* already in data base */
			add_version = FALSE;
			break;
		}
		else if (stats->release >= cur_time)
		{
			if (security)
			{
				if (cur_security)
				{
					vid_update = vid;
					version_update = strdup(cur_version);
					security_update = cur_security;
				}
				else
				{
					vid_delete = vid;
					version_delete = strdup(cur_version);
					security_delete = cur_security;
				}
			}
			else
			{
				if (!cur_security)
				{
					vid_update = vid;
					version_update = strdup(cur_version);
					security_update = cur_security;
				}
			}
		}
		else
		{
			if (security == cur_security)
			{
				add_version = FALSE;
			}
		}
	}
	e->destroy(e);

	if ((!vid && !security) || (vid && !add_version))
	{
		free(version_update);
		free(version_delete);
		return TRUE;
	}

	if ((!vid && security) || (vid && !vid_update))
	{
		printf("%s (%s) %s\n", package, version, security ? "[s]" : "");

		if (db->execute(db, &vid,
			"INSERT INTO versions "
			"(package, product, release, security, time) "
			"VALUES (?, ?, ?, ?, ?)", DB_INT, pac_id, DB_INT, stats->product,
			DB_TEXT, version, DB_INT, security, DB_INT, stats->release) != 1)
		{
			fprintf(stderr, "could not store version '%s' to database\n",
							 version);
			free(version_update);
			free(version_delete);
			return FALSE;
		}
		stats->new_versions++;
	}
	else
	{
		printf("%s (%s) %s updated by\n",
			   package, version_update, security_update ? "[s]" : "");
		printf("%s (%s) %s\n", package, version, security ? "[s]" : "");

		if (db->execute(db, NULL,
			"UPDATE versions SET release = ?, time = ? WHERE id = ?",
			DB_TEXT, version, DB_INT, stats->release, DB_INT, vid_update) <= 0)
		{
			fprintf(stderr, "could not update version '%s' to database\n",
							 version);
			free(version_update);
			free(version_delete);
			return FALSE;
		}
		stats->updated_versions++;
	}

	if (vid_delete)
	{
		printf("%s (%s) %s deleted\n",
			   package, version_delete, security_delete ? "[s]" : "");
			if (db->execute(db, NULL,
			"DELETE FROM  versions WHERE id = ?",
			DB_INT, vid_delete) <= 0)
		{
			fprintf(stderr, "could not delete version '%s' from database\n",
							 version_delete);
			free(version_update);
			free(version_delete);
			return FALSE;
		}
		stats->deleted_versions++;
	}
	free(version_update);
	free(version_delete);

	return TRUE;
}

/**
 * Process a package file and store updates in the database
 */
static void process_packages(char *filename, char *product, bool security)
{
	char *uri, line[BUF_LEN], *pos, *package = NULL, *version = NULL;
	pacman_state_t pacman_state;
	enumerator_t *e;
	database_t *db;
	int pid;
	FILE *file;
	stats_t stats;
	bool success;

	/* initialize statistics */
	memset(&stats, 0x00, sizeof(stats_t));

	/* Set release date to current time */
	stats.release = time(NULL);

	/* opening package file */
	printf("loading\"%s\"\n", filename);
	file = fopen(filename, "r");
	if (!file)
	{
		fprintf(stderr, "could not open \"%s\"\n", filename);
		exit(EXIT_FAILURE);
	}

	/* connect package database */
	uri = lib->settings->get_str(lib->settings, "pacman.database", NULL);
	if (!uri)
	{
		fprintf(stderr, "database URI pacman.database not set\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	db = lib->db->create(lib->db, uri);
	if (!db)
	{
		fprintf(stderr, "could not connect to database '%s'\n", uri);
		fclose(file);
		exit(EXIT_FAILURE);
	}

	/* check if product is already in database */
	e = db->query(db, "SELECT id FROM products WHERE name = ?",
				  DB_TEXT, product, DB_INT);
	if (e)
	{
		if (e->enumerate(e, &pid))
		{
			stats.product = pid;
		}
		e->destroy(e);
	}
	if (!stats.product)
	{
		if (db->execute(db, &pid, "INSERT INTO products (name) VALUES (?)",
						DB_TEXT, product) != 1)
		{
			fprintf(stderr, "could not store product '%s' to database\n",
							 product);
			fclose(file);
			db->destroy(db);
			exit(EXIT_FAILURE);
		}
		stats.product = pid;
	}

	pacman_state = PACMAN_STATE_BEGIN_PACKAGE;

	while (fgets(line, sizeof(line), file))
	{
		/* set read pointer to beginning of line */
		pos = line;

		switch (pacman_state)
		{
			case PACMAN_STATE_BEGIN_PACKAGE:
				pos = strstr(pos, "Package: ");
				if (!pos)
				{
					continue;
				}
				pos += 9;
				package = pos;
				pos = strchr(pos, '\n');
				if (pos)
				{
					package = strndup(package, pos - package);
					pacman_state = PACMAN_STATE_VERSION;
				}
				break;
			case PACMAN_STATE_VERSION:
				pos = strstr(pos, "Version: ");
				if (!pos)
				{
					continue;
				}
				pos += 9;
				version = pos;
				pos = strchr(pos, '\n');
				if (pos)
				{
					version = strndup(version, pos - version);
					pacman_state = PACMAN_STATE_END_PACKAGE;
				}
				break;
			case PACMAN_STATE_END_PACKAGE:
				if (*pos != '\n')
				{
					continue;
				}
				success = update_database(db, package, version, security, &stats);
				free(package);
				free(version);
				if (!success)
				{
					fclose(file);
					db->destroy(db);
					exit(EXIT_FAILURE);
				}
				pacman_state = PACMAN_STATE_BEGIN_PACKAGE;
		}
	}
	fclose(file);
	db->destroy(db);

	printf("processed %d packages, %d new packages, %d new versions, "
		   "%d updated versions, %d deleted versions\n",
			stats.packages, stats.new_packages, stats.new_versions,
			stats.updated_versions, stats.deleted_versions);
}

static void do_args(int argc, char *argv[])
{
	char *filename = NULL, *product = NULL;
	bool security = FALSE;

	/* reinit getopt state */
	optind = 0;

	while (TRUE)
	{
		int c;

		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "file", required_argument, NULL, 'f' },
			{ "product", required_argument, NULL, 'p' },
			{ "security", no_argument, NULL, 's' },
			{ 0,0,0,0 }
		};

		c = getopt_long(argc, argv, "", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			case 'f':
				filename = optarg;
				continue;
			case 'p':
				product = optarg;
				continue;
			case 's':
				security = TRUE;
				continue;
		}
		break;
	}

	if (filename && product)
	{
		process_packages(filename, product, security);
	}
	else
	{
		usage();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	/* enable attest debugging hook */
	dbg = pacman_dbg;
	openlog("pacman", 0, LOG_DEBUG);

	atexit(cleanup);

	/* initialize library */
	if (!library_init(NULL, "pacman"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "pacman.load", "sqlite")))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	do_args(argc, argv);

	exit(EXIT_SUCCESS);
}

