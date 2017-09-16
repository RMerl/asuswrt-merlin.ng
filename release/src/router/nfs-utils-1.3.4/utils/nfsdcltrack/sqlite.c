/*
 * Copyright (C) 2011  Red Hat, Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Explanation:
 *
 * This file contains the code to manage the sqlite backend database for the
 * nfsdcltrack usermodehelper upcall program.
 *
 * The main database is called main.sqlite and contains the following tables:
 *
 * parameters: simple key/value pairs for storing database info
 *
 * clients: an "id" column containing a BLOB with the long-form clientid as
 * 	    sent by the client, a "time" column containing a timestamp (in
 * 	    epoch seconds) of when the record was last updated, and a
 * 	    "has_session" column containing a boolean value indicating
 * 	    whether the client has sessions (v4.1+) or not (v4.0).
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <errno.h>
#include <event.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sqlite3.h>
#include <linux/limits.h>

#include "xlog.h"

#define CLTRACK_SQLITE_LATEST_SCHEMA_VERSION 2

/* in milliseconds */
#define CLTRACK_SQLITE_BUSY_TIMEOUT 10000

/* private data structures */

/* global variables */

/* reusable pathname and sql command buffer */
static char buf[PATH_MAX];

/* global database handle */
static sqlite3 *dbh;

/* forward declarations */

/* make a directory, ignoring EEXIST errors unless it's not a directory */
static int
mkdir_if_not_exist(const char *dirname)
{
	int ret;
	struct stat statbuf;

	ret = mkdir(dirname, S_IRWXU);
	if (ret && errno != EEXIST)
		return -errno;

	ret = stat(dirname, &statbuf);
	if (ret)
		return -errno;

	if (!S_ISDIR(statbuf.st_mode))
		ret = -ENOTDIR;

	return ret;
}

static int
sqlite_query_schema_version(void)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	/* prepare select query */
	ret = sqlite3_prepare_v2(dbh,
		"SELECT value FROM parameters WHERE key == \"version\";",
		 -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to prepare select statement: %s",
			sqlite3_errmsg(dbh));
		ret = 0;
		goto out;
	}

	/* query schema version */
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		xlog(L_ERROR, "Select statement execution failed: %s",
				sqlite3_errmsg(dbh));
		ret = 0;
		goto out;
	}

	ret = sqlite3_column_int(stmt, 0);
out:
	sqlite3_finalize(stmt);
	return ret;
}

static int
sqlite_maindb_update_v1_to_v2(void)
{
	int ret, ret2;
	char *err;

	/* begin transaction */
	ret = sqlite3_exec(dbh, "BEGIN EXCLUSIVE TRANSACTION;", NULL, NULL,
				&err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to begin transaction: %s", err);
		goto rollback;
	}

	/*
	 * Check schema version again. This time, under an exclusive
	 * transaction to guard against racing DB setup attempts
	 */
	ret = sqlite_query_schema_version();
	switch (ret) {
	case 1:
		/* Still at v1 -- do conversion */
		break;
	case CLTRACK_SQLITE_LATEST_SCHEMA_VERSION:
		/* Someone else raced in and set it up */
		ret = 0;
		goto rollback;
	default:
		/* Something went wrong -- fail! */
		ret = -EINVAL;
		goto rollback;
	}

	/* create v2 clients table */
	ret = sqlite3_exec(dbh, "ALTER TABLE clients ADD COLUMN "
				"has_session INTEGER;",
				NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to update clients table: %s", err);
		goto rollback;
	}

	ret = snprintf(buf, sizeof(buf), "UPDATE parameters SET value = %d "
			"WHERE key = \"version\";",
			CLTRACK_SQLITE_LATEST_SCHEMA_VERSION);
	if (ret < 0) {
		xlog(L_ERROR, "sprintf failed!");
		goto rollback;
	} else if ((size_t)ret >= sizeof(buf)) {
		xlog(L_ERROR, "sprintf output too long! (%d chars)", ret);
		ret = -EINVAL;
		goto rollback;
	}

	ret = sqlite3_exec(dbh, (const char *)buf, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to update schema version: %s", err);
		goto rollback;
	}

	ret = sqlite3_exec(dbh, "COMMIT TRANSACTION;", NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to commit transaction: %s", err);
		goto rollback;
	}
out:
	sqlite3_free(err);
	return ret;
rollback:
	ret2 = sqlite3_exec(dbh, "ROLLBACK TRANSACTION;", NULL, NULL, &err);
	if (ret2 != SQLITE_OK)
		xlog(L_ERROR, "Unable to rollback transaction: %s", err);
	goto out;
}

/*
 * Start an exclusive transaction and recheck the DB schema version. If it's
 * still zero (indicating a new database) then set it up. If that all works,
 * then insert schema version into the parameters table and commit the
 * transaction. On any error, rollback the transaction.
 */
int
sqlite_maindb_init_v2(void)
{
	int ret, ret2;
	char *err = NULL;

	/* Start a transaction */
	ret = sqlite3_exec(dbh, "BEGIN EXCLUSIVE TRANSACTION;", NULL, NULL,
				&err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to begin transaction: %s", err);
		return ret;
	}

	/*
	 * Check schema version again. This time, under an exclusive
	 * transaction to guard against racing DB setup attempts
	 */
	ret = sqlite_query_schema_version();
	switch (ret) {
	case 0:
		/* Query failed again -- set up DB */
		break;
	case CLTRACK_SQLITE_LATEST_SCHEMA_VERSION:
		/* Someone else raced in and set it up */
		ret = 0;
		goto rollback;
	default:
		/* Something went wrong -- fail! */
		ret = -EINVAL;
		goto rollback;
	}

	ret = sqlite3_exec(dbh, "CREATE TABLE parameters "
				"(key TEXT PRIMARY KEY, value TEXT);",
				NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to create parameter table: %s", err);
		goto rollback;
	}

	/* create the "clients" table */
	ret = sqlite3_exec(dbh, "CREATE TABLE clients (id BLOB PRIMARY KEY, "
				"time INTEGER, has_session INTEGER);",
				NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to create clients table: %s", err);
		goto rollback;
	}


	/* insert version into parameters table */
	ret = snprintf(buf, sizeof(buf), "INSERT OR FAIL INTO parameters "
			"values (\"version\", \"%d\");",
			CLTRACK_SQLITE_LATEST_SCHEMA_VERSION);
	if (ret < 0) {
		xlog(L_ERROR, "sprintf failed!");
		goto rollback;
	} else if ((size_t)ret >= sizeof(buf)) {
		xlog(L_ERROR, "sprintf output too long! (%d chars)", ret);
		ret = -EINVAL;
		goto rollback;
	}

	ret = sqlite3_exec(dbh, (const char *)buf, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to insert into parameter table: %s", err);
		goto rollback;
	}

	ret = sqlite3_exec(dbh, "COMMIT TRANSACTION;", NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to commit transaction: %s", err);
		goto rollback;
	}
out:
	sqlite3_free(err);
	return ret;

rollback:
	/* Attempt to rollback the transaction */
	ret2 = sqlite3_exec(dbh, "ROLLBACK TRANSACTION;", NULL, NULL, &err);
	if (ret2 != SQLITE_OK)
		xlog(L_ERROR, "Unable to rollback transaction: %s", err);
	goto out;
}

/* Open the database and set up the database handle for it */
int
sqlite_prepare_dbh(const char *topdir)
{
	int ret;

	/* Do nothing if the database handle is already set up */
	if (dbh)
		return 0;

	ret = snprintf(buf, PATH_MAX - 1, "%s/main.sqlite", topdir);
	if (ret < 0)
		return ret;

	buf[PATH_MAX - 1] = '\0';

	/* open a new DB handle */
	ret = sqlite3_open(buf, &dbh);
	if (ret != SQLITE_OK) {
		/* try to create the dir */
		ret = mkdir_if_not_exist(topdir);
		if (ret)
			goto out_close;

		/* retry open */
		ret = sqlite3_open(buf, &dbh);
		if (ret != SQLITE_OK)
			goto out_close;
	}

	/* set busy timeout */
	ret = sqlite3_busy_timeout(dbh, CLTRACK_SQLITE_BUSY_TIMEOUT);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to set sqlite busy timeout: %s",
				sqlite3_errmsg(dbh));
		goto out_close;
	}

	ret = sqlite_query_schema_version();
	switch (ret) {
	case CLTRACK_SQLITE_LATEST_SCHEMA_VERSION:
		/* DB is already set up. Do nothing */
		ret = 0;
		break;
	case 1:
		/* Old DB -- update to new schema */
		ret = sqlite_maindb_update_v1_to_v2();
		if (ret)
			goto out_close;
		break;
	case 0:
		/* Query failed -- try to set up new DB */
		ret = sqlite_maindb_init_v2();
		if (ret)
			goto out_close;
		break;
	default:
		/* Unknown DB version -- downgrade? Fail */
		xlog(L_ERROR, "Unsupported database schema version! "
			"Expected %d, got %d.",
			CLTRACK_SQLITE_LATEST_SCHEMA_VERSION, ret);
		ret = -EINVAL;
		goto out_close;
	}

	return ret;
out_close:
	sqlite3_close(dbh);
	dbh = NULL;
	return ret;
}

/*
 * Create a client record
 *
 * Returns a non-zero sqlite error code, or SQLITE_OK (aka 0)
 */
int
sqlite_insert_client(const unsigned char *clname, const size_t namelen,
			const bool has_session, const bool zerotime)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	if (zerotime)
		ret = sqlite3_prepare_v2(dbh, "INSERT OR REPLACE INTO clients "
				"VALUES (?, 0, ?);", -1, &stmt, NULL);
	else
		ret = sqlite3_prepare_v2(dbh, "INSERT OR REPLACE INTO clients "
				"VALUES (?, strftime('%s', 'now'), ?);", -1,
				&stmt, NULL);

	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: insert statement prepare failed: %s",
			__func__, sqlite3_errmsg(dbh));
		return ret;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s", __func__,
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_int(stmt, 2, (int)has_session);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind int failed: %s", __func__,
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from insert: %s",
				__func__, sqlite3_errmsg(dbh));

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/* Remove a client record */
int
sqlite_remove_client(const unsigned char *clname, const size_t namelen)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "DELETE FROM clients WHERE id==?", -1,
				 &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: statement prepare failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s", __func__,
				sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from delete: %d",
				__func__, ret);

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/*
 * Is the given clname in the clients table? If so, then update its timestamp
 * and return success. If the record isn't present, or the update fails, then
 * return an error.
 */
int
sqlite_check_client(const unsigned char *clname, const size_t namelen,
			const bool has_session)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "SELECT count(*) FROM clients WHERE "
				      "id==?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: unable to prepare update statement: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		xlog(L_ERROR, "%s: unexpected return code from select: %d",
				__func__, ret);
		goto out_err;
	}

	ret = sqlite3_column_int(stmt, 0);
	xlog(D_GENERAL, "%s: select returned %d rows", __func__, ret);
	if (ret != 1) {
		ret = -EACCES;
		goto out_err;
	}

	/* Only update timestamp for v4.0 clients */
	if (has_session) {
		ret = SQLITE_OK;
		goto out_err;
	}

	sqlite3_finalize(stmt);
	stmt = NULL;
	ret = sqlite3_prepare_v2(dbh, "UPDATE OR FAIL clients SET "
				      "time=strftime('%s', 'now') WHERE id==?",
				 -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: unable to prepare update statement: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_bind_blob(stmt, 1, (const void *)clname, namelen,
				SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind blob failed: %s",
				__func__, sqlite3_errmsg(dbh));
		goto out_err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE)
		ret = SQLITE_OK;
	else
		xlog(L_ERROR, "%s: unexpected return code from update: %s",
				__func__, sqlite3_errmsg(dbh));

out_err:
	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_finalize(stmt);
	return ret;
}

/*
 * remove any client records that were not reclaimed since grace_start.
 */
int
sqlite_remove_unreclaimed(time_t grace_start)
{
	int ret;
	char *err = NULL;

	ret = snprintf(buf, sizeof(buf), "DELETE FROM clients WHERE time < %ld",
			grace_start);
	if (ret < 0) {
		return ret;
	} else if ((size_t)ret >= sizeof(buf)) {
		ret = -EINVAL;
		return ret;
	}

	ret = sqlite3_exec(dbh, buf, NULL, NULL, &err);
	if (ret != SQLITE_OK)
		xlog(L_ERROR, "%s: delete failed: %s", __func__, err);

	xlog(D_GENERAL, "%s: returning %d", __func__, ret);
	sqlite3_free(err);
	return ret;
}

/*
 * Are there any clients that are possibly still reclaiming? Return a positive
 * integer (usually number of clients) if so. If not, then return 0. On any
 * error, return non-zero.
 */
int
sqlite_query_reclaiming(const time_t grace_start)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	ret = sqlite3_prepare_v2(dbh, "SELECT count(*) FROM clients WHERE "
				      "time < ? OR has_session != 1", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: unable to prepare select statement: %s",
				__func__, sqlite3_errmsg(dbh));
		return ret;
	}

	ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)grace_start);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "%s: bind int64 failed: %s",
				__func__, sqlite3_errmsg(dbh));
		return ret;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		xlog(L_ERROR, "%s: unexpected return code from select: %s",
				__func__, sqlite3_errmsg(dbh));
		return ret;
	}

	ret = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	xlog(D_GENERAL, "%s: there are %d clients that have not completed "
			"reclaim", __func__, ret);
	return ret;
}
