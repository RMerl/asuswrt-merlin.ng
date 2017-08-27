/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_sql_sqlite.c
 * @brief SQLite driver.
 *
 * @copyright 2013 Network RADIUS SARL <info@networkradius.com>
 * @copyright 2007 Apple Inc.
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <sqlite3.h>

#include "rlm_sql.h"
#include "config.h"

#define BOOTSTRAP_MAX (1048576 * 10)

/*
 *	Allow us to use versions < 3.6.0 beta0
 */
#ifndef SQLITE_OPEN_NOMUTEX
#  define SQLITE_OPEN_NOMUTEX 0
#endif

typedef struct rlm_sql_sqlite_conn {
	sqlite3 *db;
	sqlite3_stmt *statement;
	int col_count;
} rlm_sql_sqlite_conn_t;

typedef struct rlm_sql_sqlite_config {
	char const *filename;
	char const *bootstrap;
} rlm_sql_sqlite_config_t;

static const CONF_PARSER driver_config[] = {
	{"filename", PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED,
	 offsetof(rlm_sql_sqlite_config_t, filename), NULL, NULL},
	{"bootstrap", PW_TYPE_FILE_INPUT,
	 offsetof(rlm_sql_sqlite_config_t, bootstrap), NULL, NULL},

	{NULL, -1, 0, NULL, NULL}
};

static int sql_check_error(sqlite3 *db)
{
	int error = sqlite3_errcode(db);
	switch(error) {
	/*
	 *	Not errors
	 */
	case SQLITE_OK:
	case SQLITE_DONE:
	case SQLITE_ROW:
		return 0;
	/*
	 *	User/transient errors
	 */
	case SQLITE_ERROR:	/* SQL error or missing database */
	case SQLITE_FULL:
	case SQLITE_CONSTRAINT:
	case SQLITE_MISMATCH:
		ERROR("rlm_sql_sqlite: Error (%d): %s", error, sqlite3_errmsg(db));

		return -1;
		break;

	/*
	 *	Errors with the handle, that probably require reinitialisation
	 */
	default:
		ERROR("rlm_sql_sqlite: Handle is unusable, error (%d): %s", error, sqlite3_errmsg(db));
		return RLM_SQL_RECONNECT;
		break;
	}
}

#ifdef HAVE_SQLITE_V2_API
static int sql_loadfile(TALLOC_CTX *ctx, sqlite3 *db, char const *filename)
{
	ssize_t len;
	char *buffer;
	char *p, *q, *s;
	int cl;
	FILE *f;
	struct stat finfo;

	int status;
	sqlite3_stmt *statement;
	char const *z_tail;

	INFO("rlm_sql_sqlite: Executing SQL statements from file \"%s\"", filename);

	f = fopen(filename, "r");
	if (!f) {
		ERROR("rlm_sql_sqlite: Failed opening SQL file \"%s\": %s", filename,
		       strerror(errno));

		return -1;
	}

	if (fstat(fileno(f), &finfo) < 0) {
		ERROR("rlm_sql_sqlite: Failed stating SQL file \"%s\": %s", filename,
		       strerror(errno));

		fclose(f);

		return -1;
	}

	if (finfo.st_size > BOOTSTRAP_MAX) {
		too_big:
		ERROR("rlm_sql_sqlite: Size of SQL (%zu) file exceeds limit (%uk)",
		       (size_t) finfo.st_size / 1024, BOOTSTRAP_MAX / 1024);

		fclose(f);

		return -1;
	}

	MEM(buffer = talloc_array(ctx, char, finfo.st_size + 1));
	len = fread(buffer, sizeof(char), finfo.st_size + 1, f);
	if (len > finfo.st_size) {
		talloc_free(buffer);
		goto too_big;
	}

	if (!len) {
		if (ferror(f)) {
			ERROR("rlm_sql_sqlite: Error reading SQL file: %s", strerror(errno));

			fclose(f);
			talloc_free(buffer);

			return -1;
		}

		DEBUG("rlm_sql_sqlite: Ignoring empty SQL file");

		fclose(f);
		talloc_free(buffer);

		return 0;
	}

	buffer[len] = '\0';
	fclose(f);

	/*
	 *	Check if input data is UTF-8.  Allow CR/LF \t, too.
	 */
	for (p = buffer; p < (buffer + len); p += cl) {
		if (*p < ' ') {
			if ((*p != 0x0a) && (*p != 0x0d) && (*p != '\t')) break;
			cl = 1;
		} else {
			cl = fr_utf8_char((uint8_t *) p);
			if (!cl) break;
		}
	}

	if ((p - buffer) != len) {
		ERROR("rlm_sql_sqlite: Bootstrap file contains non-UTF8 char at offset %zu", p - buffer);
		talloc_free(buffer);
		return -1;
	}

	/*
	 *	Statement delimiter is ;\n
	 */
	s = p = buffer;
	while ((q = strchr(p, ';'))) {
		if (q[1] != '\n') {
			p = q + 1;
			continue;
		}

		*q = '\0';

		(void) sqlite3_prepare_v2(db, s, len, &statement, &z_tail);
		if (sql_check_error(db)) {
			talloc_free(buffer);
			return -1;
		}

		(void) sqlite3_step(statement);
		status = sql_check_error(db);

		(void) sqlite3_finalize(statement);
		if (status || sql_check_error(db)) {
			talloc_free(buffer);
			return -1;
		}

		p = s = q + 1;
	}

	talloc_free(buffer);
	return 0;
}
#endif

static int mod_instantiate(CONF_SECTION *conf, rlm_sql_config_t *config)
{
	rlm_sql_sqlite_config_t *driver;
	int exists;

	if (sqlite3_libversion_number() != SQLITE_VERSION_NUMBER) {
		DEBUG2("rlm_sql_sqlite: SQLite library version (%s) is different from the version the server was "
		       "originally built against (%s), this may cause issues",
		       sqlite3_libversion(), SQLITE_VERSION);
	}

	MEM(driver = config->driver = talloc_zero(config, rlm_sql_sqlite_config_t));

	if (cf_section_parse(conf, driver, driver_config) < 0) {
		return -1;
	}

	INFO("rlm_sql_sqlite: SQLite library version: %s", sqlite3_libversion());
	if (!driver->filename) {
		MEM(driver->filename = talloc_asprintf(driver, "%s/%s", radius_dir, config->sql_db));
	}

	exists = rad_file_exists(driver->filename);
	if (exists < 0) {
		ERROR("rlm_sql_sqlite: Database exists, but couldn't be opened: %s", strerror(errno));

		return -1;
	}

	if (driver->bootstrap && !exists) {
#ifdef HAVE_SQLITE_V2_API
		int status;
		int ret;
		char *p;
		char *buff;
		sqlite3 *db = NULL;

		INFO("rlm_sql_sqlite: Database doesn't exist, creating it and loading schema");

		p = strrchr(driver->filename, '/');
		if (p) {
			size_t len = (p - driver->filename) + 1;

			buff = talloc_array(conf, char, len);
			strlcpy(buff, driver->filename, len);
		} else {
			MEM(buff = talloc_strdup(conf, driver->filename));
		}

		if (rad_mkdir(buff, 0700) < 0) {
			ERROR("rlm_sql_sqlite: Failed creating directory for SQLite database");

			talloc_free(buff);

			return -1;
		}

		talloc_free(buff);

		status = sqlite3_open_v2(driver->filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if (!db) {
			ERROR("rlm_sql_sqlite: Failed creating opening/creating SQLite database, error "
			       "code (%u)", status);

			goto unlink;
		}

		if (sql_check_error(db)) {
			(void) sqlite3_close(db);

			goto unlink;
		}

		ret = sql_loadfile(conf, db, driver->bootstrap);

		status = sqlite3_close(db);
		if (status != SQLITE_OK) {
			ERROR("rlm_sql_sqlite: Error closing SQLite handle, error code (%u)", status);
			goto unlink;
		}

		if (ret < 0) {
			unlink:
			if (unlink(driver->filename) < 0) {
				ERROR("rlm_sql_sqlite: Error removing partially initialised database: %s",
				       strerror(errno));
			}
			return -1;
		}
#else
		WDEBUG("rlm_sql_sqlite: sqlite3_open_v2() not available, cannot bootstrap database. "
		       "Upgrade to SQLite >= 3.5.1 if you need this functionality");
#endif
	}

	return 0;
}

static int sql_socket_destructor(void *c)
{
	int status = 0;
	rlm_sql_sqlite_conn_t * conn = c;

	DEBUG2("rlm_sql_sqlite: Socket destructor called, closing socket");

	if (conn->db) {
		status = sqlite3_close(conn->db);
		if (status != SQLITE_OK) {
			WDEBUG("rlm_sql_sqlite: Got SQLite error code (%u) when closing socket", status);
		}
	}

	return 0;
}

static sql_rcode_t sql_socket_init(rlm_sql_handle_t *handle, rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn;
	rlm_sql_sqlite_config_t *driver = config->driver;

	int status;

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_sqlite_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	INFO("rlm_sql_sqlite: Opening SQLite database \"%s\"", driver->filename);

#ifdef HAVE_SQLITE_V2_API
	status = sqlite3_open_v2(driver->filename, &(conn->db), SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);
#else
	status = sqlite3_open(driver->filename, &(conn->db));
#endif
	if (!conn->db) {
		ERROR("rlm_sql_sqlite: Failed creating opening/creating SQLite database error code (%u)",
		       status);

		return -1;
	}

	if (sql_check_error(conn->db)) {
		return -1;
	}

	/*
	 *	Enable extended return codes for extra debugging info.
	 */
	status = sqlite3_extended_result_codes(conn->db, 1);

	if (sql_check_error(conn->db)) {
		return -1;
	}

	return 0;
}

static sql_rcode_t sql_select_query(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config, char const *query)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;
	char const *z_tail;

#ifdef HAVE_SQLITE_V2_API
	(void) sqlite3_prepare_v2(conn->db, query, strlen(query), &conn->statement, &z_tail);
#else
	(void) sqlite3_prepare(conn->db, query, strlen(query), &conn->statement, &z_tail);
#endif

	conn->col_count = 0;

	return sql_check_error(conn->db);
}


static sql_rcode_t sql_query(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config, char const *query)
{
	int status;
	rlm_sql_sqlite_conn_t *conn = handle->conn;
	char const *z_tail;

#ifdef HAVE_SQLITE_V2_API
	status = sqlite3_prepare_v2(conn->db, query, strlen(query), &conn->statement, &z_tail);
#else
	status = sqlite3_prepare(conn->db, query, strlen(query), &conn->statement, &z_tail);
#endif
	if (status != SQLITE_OK) {
		return sql_check_error(conn->db);
	}

	(void) sqlite3_step(conn->statement);

	return sql_check_error(conn->db);
}

static sql_rcode_t sql_store_result(UNUSED rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	return 0;
}

static int sql_num_fields(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	if (conn->statement) {
		return sqlite3_column_count(conn->statement);
	}

	return 0;
}

static int sql_num_rows(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	if (conn->statement) {
		return sqlite3_data_count(conn->statement);
	}

	return 0;
}

static sql_rcode_t sql_fetch_row(rlm_sql_handle_t *handle, rlm_sql_config_t *config)
{
	int status;
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	int i = 0;

	char **row;

	/*
	 *	Executes the SQLite query and interates over the results
	 */
	status = sqlite3_step(conn->statement);

	/*
	 *	Error getting next row
	 */
	if (sql_check_error(conn->db)) {
		return -1;
	}

	/*
	 *	No more rows to process (were done)
	 */
	if (status == SQLITE_DONE) {
		return 1;
	}

	/*
	 *	We only need to do this once per result set, because
	 *	the number of columns won't change.
	 */
	if (conn->col_count == 0) {
		conn->col_count = sql_num_fields(handle, config);
		if (conn->col_count == 0) {
			return -1;
		}
	}

	/*
	 *	Free the previous result (also gets called on finish_query)
	 */
	talloc_free(handle->row);

	MEM(row = handle->row = talloc_zero_array(handle->conn, char *, conn->col_count + 1));

	for (i = 0; i < conn->col_count; i++) {
		switch (sqlite3_column_type(conn->statement, i)) {
		case SQLITE_INTEGER:
			MEM(row[i] = talloc_asprintf(row, "%d", sqlite3_column_int(conn->statement, i)));
			break;

		case SQLITE_FLOAT:
			MEM(row[i] = talloc_asprintf(row, "%f", sqlite3_column_double(conn->statement, i)));
			break;

		case SQLITE_TEXT:
			{
				char const *p;
				p = (char const *) sqlite3_column_text(conn->statement, i);

				if (p) {
					MEM(row[i] = talloc_strdup(row, p));
				}
			}
			break;

		case SQLITE_BLOB:
			{
				uint8_t const *p;
				size_t len;

				p = sqlite3_column_blob(conn->statement, i);
				if (p) {
					len = sqlite3_column_bytes(conn->statement, i);

					MEM(row[i] = talloc_zero_array(row, char, len + 1));
					memcpy(row[i], p, len);
				}
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

static sql_rcode_t sql_free_result(rlm_sql_handle_t *handle,
			   UNUSED rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	if (conn->statement) {
		TALLOC_FREE(handle->row);

		(void) sqlite3_finalize(conn->statement);
		conn->statement = NULL;
		conn->col_count = 0;
	}

	/*
	 *	There's no point in checking the code returned by finalize
	 *	as it'll have already been encountered elsewhere in the code.
	 *
	 *	It's just the last error that occurred processing the
	 *	statement.
	 */
	return 0;
}

static char const *sql_error(rlm_sql_handle_t *handle,
			     UNUSED rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	if (conn->db) {
		return sqlite3_errmsg(conn->db);
	}

	return "Invalid handle";
}

static sql_rcode_t sql_finish_query(rlm_sql_handle_t *handle,
			    UNUSED rlm_sql_config_t *config)
{
	return sql_free_result(handle, config);
}

static int sql_affected_rows(rlm_sql_handle_t *handle,
			     UNUSED rlm_sql_config_t *config)
{
	rlm_sql_sqlite_conn_t *conn = handle->conn;

	if (conn->db) {
		return sqlite3_changes(conn->db);
	}

	return -1;
}


/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_sqlite = {
	"rlm_sql_sqlite",
	mod_instantiate,
	sql_socket_init,
	sql_query,
	sql_select_query,
	sql_store_result,
	sql_num_fields,
	sql_num_rows,
	sql_fetch_row,
	sql_free_result,
	sql_error,
	sql_finish_query,
	sql_finish_query,
	sql_affected_rows
};
