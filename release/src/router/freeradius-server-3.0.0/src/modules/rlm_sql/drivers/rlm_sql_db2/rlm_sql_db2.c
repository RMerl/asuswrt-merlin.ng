/*
 * sql_db2.c		IBM DB2 rlm_sql driver
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
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Mike Machado <mike@innercite.com>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 * Copyright 2001  Joerg Wendland <wendland@scan-plus.de>
 */

/*
 * Modification of rlm_sql_db2 to handle IBM DB2 UDB V7
 * by Joerg Wendland <wendland@scan-plus.de>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <sys/stat.h>

#include <sql.h>
#include <sqlcli.h>
#include "rlm_sql.h"

typedef struct rlm_sql_conn {
	SQLHANDLE hdbc;
	SQLHANDLE henv;
	SQLHANDLE stmt;
	char *error;
} rlm_sql_db2_conn_t;

static int sql_socket_destructor(void *c)
{
	rlm_sql_db2_conn_t *conn = c;

	DEBUG2("rlm_sql_db2: Socket destructor called, closing socket");

	if (conn->hdbc) {
		SQLDisconnect(conn->hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, conn->hdbc);
	}

	if (conn->henv) {
		SQLFreeHandle(SQL_HANDLE_ENV, conn->henv);
	}

	return RLM_SQL_OK;
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
	SQLRETURN retval;
	rlm_sql_db2_conn_t *conn;

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_db2_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	/* allocate handles */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(conn->henv));
	SQLAllocHandle(SQL_HANDLE_DBC, conn->henv, &(conn->hdbc));

	/*
	 *	The db2 API doesn't qualify arguments as const even when they should be.
	 */
	{
		SQLCHAR *server, *login, *password;

		memcpy(&server, &config->sql_server, sizeof(server));
		memcpy(&login, &config->sql_login, sizeof(login));
		memcpy(&password, &config->sql_password, sizeof(password));

		retval = SQLConnect(conn->hdbc,
				    server, SQL_NTS,
				    login,  SQL_NTS,
				    password, SQL_NTS);
	}


	if(retval != SQL_SUCCESS) {
		ERROR("could not connect to DB2 server %s", config->sql_server);

		return RLM_SQL_ERROR;
	}

	return RLM_SQL_OK;
}

/*************************************************************************
 *
 *	Function: sql_query
 *
 *	Purpose: Issue a query to the database
 *
 *************************************************************************/
static sql_rcode_t sql_query(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config, char const *query)
{
	SQLRETURN retval;
	rlm_sql_db2_conn_t *conn;

	conn = handle->conn;

	/* allocate handle for statement */
	SQLAllocHandle(SQL_HANDLE_STMT, conn->hdbc, &(conn->stmt));

	/* execute query */
	{
		SQLCHAR *db2_query;
		memcpy(&db2_query, &query, sizeof(query));

		retval = SQLExecDirect(conn->stmt, db2_query, SQL_NTS);
		if(retval != SQL_SUCCESS) {
			/* XXX Check if retval means we should return RLM_SQL_RECONNECT */
			ERROR("Could not execute statement \"%s\"\n", query);
			return RLM_SQL_ERROR;
		}
	}

	return RLM_SQL_OK;
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
	return sql_query(handle, config, query);
}


/*************************************************************************
 *
 *	Function: sql_num_fields
 *
 *	Purpose: database specific num_fields function. Returns number
 *	       of columns from query
 *
 *************************************************************************/
static int sql_num_fields(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	SQLSMALLINT c;
	rlm_sql_db2_conn_t *conn;

	conn = handle->conn;
	SQLNumResultCols(conn->stmt, &c);
	return c;
}


/*************************************************************************
 *
 *	Function: sql_fetch_row
 *
 *	Purpose: database specific fetch_row. Returns a rlm_sql_row_t struct
 *	       with all the data for the query in 'handle->row'. Returns
 *		 0 on success, -1 on failure, RLM_SQL_RECONNECT if 'database is down'
 *
 *************************************************************************/
static sql_rcode_t sql_fetch_row(rlm_sql_handle_t *handle, rlm_sql_config_t *config)
{
	int c, i;
	SQLINTEGER len, slen;
	rlm_sql_row_t retval;
	rlm_sql_db2_conn_t *conn;

	conn = handle->conn;

	c = sql_num_fields(handle, config);
	retval = (rlm_sql_row_t)rad_malloc(c*sizeof(char*)+1);
	memset(retval, 0, c*sizeof(char*)+1);

	/* advance cursor */
	if(SQLFetch(conn->stmt) == SQL_NO_DATA_FOUND) {
		handle->row = NULL;
		goto error;
	}

	for(i = 0; i < c; i++) {
		/* get column length */
		SQLColAttribute(conn->stmt, i+1, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &len);

		retval[i] = rad_malloc(len+1);

		/* get the actual column */
		SQLGetData(conn->stmt, i + 1, SQL_C_CHAR, retval[i], len+1, &slen);
		if(slen == SQL_NULL_DATA) {
			retval[i][0] = '\0';
		}
	}

	handle->row = retval;
	return RLM_SQL_OK;

error:
	for(i = 0; i < c; i++) {
		free(retval[i]);
	}
	free(retval);

	return RLM_SQL_ERROR;
}

/*************************************************************************
 *
 *	Function: sql_free_result
 *
 *	Purpose: database specific free_result. Frees memory allocated
 *	       for a result set
 *
 *************************************************************************/
static sql_rcode_t sql_free_result(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_db2_conn_t *conn;
	conn = handle->conn;
	SQLFreeHandle(SQL_HANDLE_STMT, conn->stmt);

	return RLM_SQL_OK;
}



/*************************************************************************
 *
 *	Function: sql_error
 *
 *	Purpose: database specific error. Returns error associated with
 *	       connection
 *
 *************************************************************************/
static char const *sql_error(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	/* this should really be enough, if not, you still got the sqlstate */
	char sqlstate[6];
	char msg[1024];
	SQLINTEGER err;
	SQLSMALLINT rl;
	rlm_sql_db2_conn_t *conn = handle->conn;

	TALLOC_FREE(conn->error);
	SQLGetDiagRec(SQL_HANDLE_STMT, conn->stmt, 1, (SQLCHAR *) sqlstate, &err, (SQLCHAR *) msg, sizeof(msg), &rl);
	conn->error = talloc_asprintf(conn, "sqlstate %s: %s", sqlstate, msg);

	return conn->error;
}


/*************************************************************************
 *
 *	Function: sql_finish_query
 *
 *	Purpose: End the query, such as freeing memory
 *
 *************************************************************************/
static sql_rcode_t sql_finish_query(UNUSED rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	return RLM_SQL_OK;
}



/*************************************************************************
 *
 *	Function: sql_finish_select_query
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static sql_rcode_t sql_finish_select_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config)
{
	return sql_finish_query(handle, config);
}


/*************************************************************************
 *
 *	Function: sql_affected_rows
 *
 *	Purpose: Return the number of rows affected by the last query.
 *
 *************************************************************************/
static int sql_affected_rows(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config)
{
	SQLINTEGER c;
	rlm_sql_db2_conn_t *conn = handle->conn;

	SQLRowCount(conn->stmt, &c);

	return c;
}

/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_db2 = {
	"rlm_sql_db2",
	NULL,
	sql_socket_init,
	sql_query,
	sql_select_query,
	NULL, /* sql_store_result */
	sql_num_fields,
	NULL, /* sql_num_rows */
	sql_fetch_row,
	sql_free_result,
	sql_error,
	sql_finish_query,
	sql_finish_select_query,
	sql_affected_rows,
};
