/*
 * sql_mysql.c		SQL Module
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000-2007  The FreeRADIUS server project
 * Copyright 2000  Mike Machado <mike@innercite.com>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <sys/stat.h>

#include "config.h"

#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/mysql_version.h>
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#else
#ifdef HAVE_MYSQL_H
#include <mysql_version.h>
#include <errmsg.h>
#include <mysql.h>
#endif
#endif

#include "rlm_sql.h"

typedef struct rlm_sql_mysql_conn {
	MYSQL db;
	MYSQL *sock;
	MYSQL_RES *result;
	rlm_sql_row_t row;
} rlm_sql_mysql_conn_t;

/* Prototypes */
static sql_rcode_t sql_free_result(rlm_sql_handle_t*, rlm_sql_config_t*);

static int sql_socket_destructor(void *c)
{
	rlm_sql_mysql_conn_t *conn = c;

	DEBUG2("rlm_sql_mysql: Socket destructor called, closing socket");

	if (conn->sock){
		mysql_close(conn->sock);
	}

	return 0;
}

/*************************************************************************
 *
 *	Function: sql_create_socket
 *
 *	Purpose: Establish connection to the db
 *
 *************************************************************************/
static sql_rcode_t sql_socket_init(rlm_sql_handle_t *handle, rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn;
	unsigned long sql_flags;

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_mysql_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	DEBUG("rlm_sql_mysql: Starting connect to MySQL server");

	mysql_init(&(conn->db));
	mysql_options(&(conn->db), MYSQL_READ_DEFAULT_GROUP, "freeradius");

#if (MYSQL_VERSION_ID >= 50000)
	if (config->query_timeout) {
		unsigned int timeout = config->query_timeout;

		/*
		 *	3 retries are hard-coded into the MySQL library.
		 *	We ensure that the REAL timeout is what the user
		 *	set by accounting for that.
		 */
		if (timeout > 3) timeout /= 3;

		mysql_options(&(conn->db), MYSQL_OPT_CONNECT_TIMEOUT,
			      &timeout);
		mysql_options(&(conn->db), MYSQL_OPT_READ_TIMEOUT,
			      &timeout);
		mysql_options(&(conn->db), MYSQL_OPT_WRITE_TIMEOUT,
			      &timeout);
	}
#endif

#if (MYSQL_VERSION_ID >= 40100)
	sql_flags = CLIENT_MULTI_RESULTS | CLIENT_FOUND_ROWS;
#else
	sql_flags = CLIENT_FOUND_ROWS;
#endif

#ifdef CLIENT_MULTI_STATEMENTS
	sql_flags |= CLIENT_MULTI_STATEMENTS;
#endif
	conn->sock = mysql_real_connect(&(conn->db),
					config->sql_server,
					config->sql_login,
					config->sql_password,
					config->sql_db,
					atoi(config->sql_port),
					NULL,
					sql_flags);
	if (!conn->sock) {
		ERROR("rlm_sql_mysql: Couldn't connect socket to MySQL "
		       "server %s@%s:%s", config->sql_login, config->sql_server,
		       config->sql_db);
		ERROR("rlm_sql_mysql: Mysql error '%s'",
		       mysql_error(&conn->db));

		conn->sock = NULL;

		return -1;
	}


	return 0;
}

/*************************************************************************
 *
 *	Function: sql_check_error
 *
 *	Purpose: check the error to see if the server is down
 *
 *************************************************************************/
static int sql_check_error(int error)
{
	switch(error) {
	case CR_SERVER_GONE_ERROR:
	case CR_SERVER_LOST:
	case -1:
		DEBUG("rlm_sql_mysql: MYSQL check_error: %d, returning RLM_SQL_RECONNECT", error);
		return RLM_SQL_RECONNECT;
		break;
	case 0:
		return 0;
		break;
	case CR_OUT_OF_MEMORY:
	case CR_COMMANDS_OUT_OF_SYNC:
	case CR_UNKNOWN_ERROR:
	default:
		DEBUG("rlm_sql_mysql: MYSQL check_error: %d received", error);
		return -1;
		break;
	}
}


/*************************************************************************
 *
 *	Function: sql_query
 *
 *	Purpose: Issue a query to the database
 *
 *************************************************************************/
static sql_rcode_t sql_query(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config, char const *query)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;

	if (!conn->sock) {
		ERROR("rlm_sql_mysql: Socket not connected");
		return RLM_SQL_RECONNECT;
	}

	mysql_query(conn->sock, query);
	return sql_check_error(mysql_errno(conn->sock));
}


/*************************************************************************
 *
 *	Function: sql_store_result
 *
 *	Purpose: database specific store_result function. Returns a result
 *	       set for the query. In case of multiple results, get the
 *	       first non-empty one.
 *
 *************************************************************************/
static sql_rcode_t sql_store_result(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;
	int status;

	if (!conn->sock) {
		ERROR("rlm_sql_mysql: Socket not connected");
		return RLM_SQL_RECONNECT;
	}
retry_store_result:
	if (!(conn->result = mysql_store_result(conn->sock))) {
		status = sql_check_error(mysql_errno(conn->sock));
		if (status != 0) {
			ERROR("rlm_sql_mysql: Cannot store result");
			ERROR("rlm_sql_mysql: MySQL error '%s'",
			       mysql_error(conn->sock));
			return status;
		}
#if (MYSQL_VERSION_ID >= 40100)
		status = mysql_next_result(conn->sock);
		if (status == 0) {
			/* there are more results */
			goto retry_store_result;
		} else if (status > 0) {
			ERROR("rlm_sql_mysql: Cannot get next result");
			ERROR("rlm_sql_mysql: MySQL error '%s'",
			       mysql_error(conn->sock));
			return sql_check_error(status);
		}
#endif
	}
	return 0;
}


/*************************************************************************
 *
 *	Function: sql_num_fields
 *
 *	Purpose: database specific num_fields function. Returns number
 *	       of columns from query
 *
 *************************************************************************/
static int sql_num_fields(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	int     num = 0;
	rlm_sql_mysql_conn_t *conn = handle->conn;

#if MYSQL_VERSION_ID >= 32224
	if (!(num = mysql_field_count(conn->sock))) {
#else
	if (!(num = mysql_num_fields(conn->sock))) {
#endif
		ERROR("rlm_sql_mysql: MYSQL Error: No Fields");
		ERROR("rlm_sql_mysql: MYSQL error: %s",
		       mysql_error(conn->sock));
	}
	return num;
}


/*************************************************************************
 *
 *	Function: sql_select_query
 *
 *	Purpose: Issue a select query to the database
 *
 *************************************************************************/
static sql_rcode_t sql_select_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config, char const *query)
{
	int ret;

	ret = sql_query(handle, config, query);
	if(ret)
		return ret;
	ret = sql_store_result(handle, config);
	if (ret) {
		return ret;
	}

	/* Why? Per http://www.mysql.com/doc/n/o/node_591.html,
	 * this cannot return an error.  Perhaps just to complain if no
	 * fields are found?
	 */
	sql_num_fields(handle, config);

	return ret;
}


/*************************************************************************
 *
 *	Function: sql_num_rows
 *
 *	Purpose: database specific num_rows. Returns number of rows in
 *	       query
 *
 *************************************************************************/
static int sql_num_rows(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;

	if (conn->result)
		return mysql_num_rows(conn->result);

	return 0;
}


/*************************************************************************
 *
 *	Function: sql_fetch_row
 *
 *	Purpose: database specific fetch_row. Returns a rlm_sql_row_t struct
 *	       with all the data for the query in 'handle->row'. Returns
 *		 0 on success, -1 on failure, RLM_SQL_RECONNECT if database is down.
 *
 *************************************************************************/
static sql_rcode_t sql_fetch_row(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;
	int status;

	/*
	 *  Check pointer before de-referencing it.
	 */
	if (!conn->result) {
		return RLM_SQL_RECONNECT;
	}

retry_fetch_row:
	handle->row = mysql_fetch_row(conn->result);

	if (!handle->row) {
		status = sql_check_error(mysql_errno(conn->sock));
		if (status != 0) {
			ERROR("rlm_sql_mysql: Cannot fetch row");
			ERROR("rlm_sql_mysql: MySQL error '%s'",
			       mysql_error(conn->sock));
			return status;
		}
#if (MYSQL_VERSION_ID >= 40100)
		sql_free_result(handle, config);
		status = mysql_next_result(conn->sock);
		if (status == 0) {
			/* there are more results */
			if ((sql_store_result(handle, config) == 0)
			 && (conn->result != NULL))
				goto retry_fetch_row;
		} else if (status > 0) {
			ERROR("rlm_sql_mysql: Cannot get next result");
			ERROR("rlm_sql_mysql: MySQL error '%s'",
			       mysql_error(conn->sock));
			return sql_check_error(status);
		}
#endif
	}
	return 0;
}


/*************************************************************************
 *
 *	Function: sql_free_result
 *
 *	Purpose: database specific free_result. Frees memory allocated
 *	       for a result set
 *
 *************************************************************************/
static sql_rcode_t sql_free_result(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;

	if (conn->result) {
		mysql_free_result(conn->result);
		conn->result = NULL;
	}

	return 0;
}



/*************************************************************************
 *
 *	Function: sql_error
 *
 *	Purpose: database specific error. Returns error associated with
 *	       connection
 *
 *************************************************************************/
static char const *sql_error(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;

	if (!conn || !conn->sock) {
		return "rlm_sql_mysql: no connection to db";
	}
	return mysql_error(conn->sock);
}

/*************************************************************************
 *
 *	Function: sql_finish_query
 *
 *	Purpose: As a single SQL statement may return multiple results
 *	sets, (for example stored procedures) it is necessary to check
 *	whether more results exist and process them in turn if so.
 *
 *************************************************************************/
static sql_rcode_t sql_finish_query(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
#if (MYSQL_VERSION_ID >= 40100)
	rlm_sql_mysql_conn_t *conn = handle->conn;
	int status;

skip_next_result:
	status = sql_store_result(handle, config);
	if (status != 0) {
		return status;
	} else if (conn->result != NULL) {
		DEBUG("rlm_sql_mysql: SQL statement returned unexpected result");
		sql_free_result(handle, config);
	}
	status = mysql_next_result(conn->sock);
	if (status == 0) {
		/* there are more results */
		goto skip_next_result;
	}  else if (status > 0) {
		ERROR("rlm_sql_mysql: Cannot get next result");
		ERROR("rlm_sql_mysql: MySQL error '%s'",
		       mysql_error(conn->sock));
		return sql_check_error(status);
	}
#endif
	return 0;
}



/*************************************************************************
 *
 *	Function: sql_finish_select_query
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static sql_rcode_t sql_finish_select_query(rlm_sql_handle_t * handle, rlm_sql_config_t *config)
{
#if (MYSQL_VERSION_ID >= 40100)
	int status;
	rlm_sql_mysql_conn_t *conn = handle->conn;
#endif
	sql_free_result(handle, config);
#if (MYSQL_VERSION_ID >= 40100)
	status = mysql_next_result(conn->sock);
	if (status == 0) {
		/* there are more results */
		sql_finish_query(handle, config);
	}  else if (status > 0) {
		ERROR("rlm_sql_mysql: Cannot get next result");
		ERROR("rlm_sql_mysql: MySQL error '%s'",
		       mysql_error(conn->sock));
		return sql_check_error(status);
	}
#endif
	return 0;
}


/*************************************************************************
 *
 *	Function: sql_affected_rows
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static int sql_affected_rows(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_mysql_conn_t *conn = handle->conn;

	return mysql_affected_rows(conn->sock);
}


/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_mysql = {
	"rlm_sql_mysql",
	NULL,
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
	sql_finish_select_query,
	sql_affected_rows
};
