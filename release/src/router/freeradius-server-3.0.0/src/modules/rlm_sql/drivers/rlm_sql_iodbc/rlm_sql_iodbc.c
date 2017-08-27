/*
 * sql_iodbc.c	iODBC support for FreeRadius
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
 * Copyright 2000  Jeff Carneal <jeff@apex.net>
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API

#include <freeradius-devel/radiusd.h>

#include <sys/stat.h>

#include <isql.h>
#include <isqlext.h>
#include <sqltypes.h>

#include "rlm_sql.h"

#define IODBC_MAX_ERROR_LEN 256

typedef struct rlm_sql_iodbc_conn {
	HENV    env_handle;
	HDBC    dbc_handle;
	HSTMT   stmt_handle;
	int	id;

	rlm_sql_row_t row;

	struct sql_socket *next;

	SQLCHAR error[IODBC_MAX_ERROR_LEN];
	void	*conn;
} rlm_sql_iodbc_conn_t;

static char const *sql_error(rlm_sql_handle_t *handle, rlm_sql_config_t *config);
static int sql_num_fields(rlm_sql_handle_t *handle, rlm_sql_config_t *config);

static int sql_socket_destructor(void *c)
{
	rlm_sql_iodbc_conn_t *conn = c;

	DEBUG2("rlm_sql_iodbc: Socket destructor called, closing socket");

	if (conn->stmt_handle) {
		SQLFreeStmt(conn->stmt_handle, SQL_DROP);
	}

	if (conn->dbc_handle) {
		SQLDisconnect(conn->dbc_handle);
		SQLFreeConnect(conn->dbc_handle);
	}

	if (conn->env_handle) {
		SQLFreeEnv(conn->env_handle);
	}

	return 0;
}

/*************************************************************************
 *
 *	Function: sql_socket_init
 *
 *	Purpose: Establish connection to the db
 *
 *************************************************************************/
static sql_rcode_t sql_socket_init(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {

	rlm_sql_iodbc_conn_t *conn;
	SQLRETURN rcode;

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_iodbc_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	rcode = SQLAllocEnv(&conn->env_handle);
	if (!SQL_SUCCEEDED(rcode)) {
		ERROR("sql_create_socket: SQLAllocEnv failed:  %s",
				sql_error(handle, config));
		return -1;
	}

	rcode = SQLAllocConnect(conn->env_handle,
				&conn->dbc_handle);
	if (!SQL_SUCCEEDED(rcode)) {
		ERROR("sql_create_socket: SQLAllocConnect failed:  %s",
				sql_error(handle, config));
		return -1;
	}

	/*
	 *	The iodbc API doesn't qualify arguments as const even when they should be.
	 */
	{
		SQLCHAR *server, *login, *password;

		memcpy(&server, &config->sql_server, sizeof(server));
		memcpy(&login, &config->sql_login, sizeof(login));
		memcpy(&password, &config->sql_password, sizeof(password));

		rcode = SQLConnect(conn->dbc_handle, server, SQL_NTS, login, SQL_NTS, password, SQL_NTS);
	}
	if (!SQL_SUCCEEDED(rcode)) {
		ERROR("sql_create_socket: SQLConnectfailed:  %s",
				sql_error(handle, config));
		return -1;
	}

	return 0;
}

/*************************************************************************
 *
 *	Function: sql_query
 *
 *	Purpose: Issue a non-SELECT query (ie: update/delete/insert) to
 *	       the database.
 *
 *************************************************************************/
static sql_rcode_t sql_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config, char const *query) {

	rlm_sql_iodbc_conn_t *conn = handle->conn;
	SQLRETURN rcode;

	rcode = SQLAllocStmt(conn->dbc_handle,
			     &conn->stmt_handle);
	if (!SQL_SUCCEEDED(rcode)) {
		ERROR("sql_create_socket: SQLAllocStmt failed:  %s",
				sql_error(handle, config));
		return -1;
	}

	if (!conn->dbc_handle) {
		ERROR("sql_query:  Socket not connected");
		return -1;
	}

	{
		SQLCHAR *statement;

		memcpy(&statement, &query, sizeof(statement));
		rcode = SQLExecDirect(conn->stmt_handle, statement, SQL_NTS);
	}

	if (!SQL_SUCCEEDED(rcode)) {
		ERROR("sql_query: failed:  %s",
				sql_error(handle, config));
		return -1;
	}

	return 0;
}


/*************************************************************************
 *
 *	Function: sql_select_query
 *
 *	Purpose: Issue a select query to the database
 *
 *************************************************************************/
static sql_rcode_t sql_select_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config, char const *query) {

	int numfields = 0;
	int i = 0;
	char **row = NULL;
	long len = 0;
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	if(sql_query(handle, config, query) < 0) {
		return -1;
	}

	numfields = sql_num_fields(handle, config);

	row = (char **) rad_malloc(sizeof(char *) * (numfields+1));
	memset(row, 0, (sizeof(char *) * (numfields)));
	row[numfields] = NULL;

	for(i=1; i<=numfields; i++) {
		SQLColAttributes(conn->stmt_handle, ((SQLUSMALLINT) i), SQL_COLUMN_LENGTH, NULL, 0, NULL, &len);
		len++;

		/*
		 * Allocate space for each column
		 */
		row[i - 1] = rad_malloc((size_t) len);

		/*
		 * This makes me feel dirty, but, according to Microsoft, it works.
		 * Any ODBC datatype can be converted to a 'char *' according to
		 * the following:
		 *
		 * http://msdn.microsoft.com/library/psdk/dasdk/odap4o4z.htm
		 */
		SQLBindCol(conn->stmt_handle, i, SQL_C_CHAR, (SQLCHAR *)row[i-1], len, 0);
	}

	conn->row = row;

	return 0;
}


/*************************************************************************
 *
 *	Function: sql_store_result
 *
 *	Purpose: database specific store_result function. Returns a result
 *	       set for the query.
 *
 *************************************************************************/
static sql_rcode_t sql_store_result(UNUSED rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {

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
static int sql_num_fields(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {

	SQLSMALLINT count=0;
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	SQLNumResultCols(conn->stmt_handle, &count);

	return (int)count;
}

/*************************************************************************
 *
 *	Function: sql_num_rows
 *
 *	Purpose: database specific num_rows. Returns number of rows in
 *	       query
 *
 *************************************************************************/
static int sql_num_rows(UNUSED rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {
	/*
	 * I presume this function is used to determine the number of
	 * rows in a result set *before* fetching them.  I don't think
	 * this is possible in ODBC 2.x, but I'd be happy to be proven
	 * wrong.  If you know how to do this, email me at jeff@apex.net
	 */
	return 0;
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
static sql_rcode_t sql_fetch_row(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {

	SQLRETURN rc;
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	handle->row = NULL;

	if((rc = SQLFetch(conn->stmt_handle)) == SQL_NO_DATA_FOUND) {
		return 0;
	}
	/* XXX Check rc for database down, if so, return RLM_SQL_RECONNECT */

	handle->row = conn->row;
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
static sql_rcode_t sql_free_result(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {

	int i=0;
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	for(i=0; i<sql_num_fields(handle, config); i++) {
		free(conn->row[i]);
	}
	free(conn->row);
	conn->row=NULL;

	SQLFreeStmt( conn->stmt_handle, SQL_DROP );

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
static char const *sql_error(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {

	SQLINTEGER errornum = 0;
	SQLSMALLINT length = 0;
	SQLCHAR state[256] = "";
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	conn->error[0] = '\0';

	SQLError(conn->env_handle, conn->dbc_handle, conn->stmt_handle,
		state, &errornum, conn->error, IODBC_MAX_ERROR_LEN, &length);
	return (char const *) &conn->error;
}

/*************************************************************************
 *
 *	Function: sql_finish_query
 *
 *	Purpose: End the query, such as freeing memory
 *
 *************************************************************************/
static sql_rcode_t sql_finish_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {

	return sql_free_result(handle, config);
}

/*************************************************************************
 *
 *	Function: sql_finish_select_query
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static sql_rcode_t sql_finish_select_query(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	return sql_free_result(handle, config);
}

/*************************************************************************
 *
 *	Function: sql_affected_rows
 *
 *	Purpose: Return the number of rows affected by the query (update,
 *	       or insert)
 *
 *************************************************************************/
static int sql_affected_rows(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {

	long count;
	rlm_sql_iodbc_conn_t *conn = handle->conn;

	SQLRowCount(conn->stmt_handle, &count);
	return (int)count;
}

/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_iodbc = {
	"rlm_sql_iodbc",
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
