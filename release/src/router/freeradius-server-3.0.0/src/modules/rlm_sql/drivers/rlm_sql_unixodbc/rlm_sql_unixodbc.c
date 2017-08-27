/*
 * sql_unixodbc.c	unixODBC rlm_sql driver
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
 * Copyright 2000  Dmitri Ageev <d_ageev@ortcc.ru>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <sqltypes.h>
#include "rlm_sql.h"

typedef struct rlm_sql_unixodbc_conn {
	SQLHENV env;
	SQLHDBC dbc;
	SQLHSTMT statement;
	rlm_sql_row_t row;
	void *conn;
} rlm_sql_unixodbc_conn_t;


#include <sql.h>
#include <sqlext.h>

/* Forward declarations */
static char const *sql_error(rlm_sql_handle_t *handle, rlm_sql_config_t *config);
static int sql_state(long err_handle, rlm_sql_handle_t *handle, rlm_sql_config_t *config);
static sql_rcode_t sql_free_result(rlm_sql_handle_t *handle, rlm_sql_config_t *config);
static int sql_affected_rows(rlm_sql_handle_t *handle, rlm_sql_config_t *config);
static int sql_num_fields(rlm_sql_handle_t *handle, rlm_sql_config_t *config);

static int sql_socket_destructor(void *c)
{
	rlm_sql_unixodbc_conn_t *conn = c;

	DEBUG2("rlm_sql_unixodbc: Socket destructor called, closing socket");

	if (conn->statement) {
		SQLFreeStmt(conn->statement, SQL_DROP);
	}

	if (conn->dbc) {
		SQLDisconnect(conn->dbc);
		SQLFreeConnect(conn->dbc);
	}

	if (conn->env) {
		SQLFreeEnv(conn->env);
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
	rlm_sql_unixodbc_conn_t *conn;
	long err_handle;

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_unixodbc_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	/* 1. Allocate environment handle and register version */
	err_handle = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&conn->env);
	if (sql_state(err_handle, handle, config)) {
		ERROR("rlm_sql_unixodbc: Can't allocate environment handle\n");
		return -1;
	}

	err_handle = SQLSetEnvAttr(conn->env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (sql_state(err_handle, handle, config)) {
		ERROR("rlm_sql_unixodbc: Can't register ODBC version\n");
		return -1;
	}

	/* 2. Allocate connection handle */
	err_handle = SQLAllocHandle(SQL_HANDLE_DBC, conn->env, &conn->dbc);
	if (sql_state(err_handle, handle, config)) {
		ERROR("rlm_sql_unixodbc: Can't allocate connection handle\n");
		return -1;
    	}

	/* 3. Connect to the datasource */
	{
		SQLCHAR *odbc_server, *odbc_login, *odbc_password;

		memcpy(&odbc_server, &config->sql_server, sizeof(odbc_server));
		memcpy(&odbc_login, &config->sql_login, sizeof(odbc_login));
		memcpy(&odbc_password, &config->sql_password, sizeof(odbc_password));
		err_handle = SQLConnect(conn->dbc,
					odbc_server, strlen(config->sql_server),
					odbc_login, strlen(config->sql_login),
					odbc_password, strlen(config->sql_password));
	}

	if (sql_state(err_handle, handle, config)) {
		ERROR("rlm_sql_unixodbc: Connection failed\n");
		return -1;
	}

	/* 4. Allocate the statement */
	err_handle = SQLAllocStmt(conn->dbc, &conn->statement);
	if (sql_state(err_handle, handle, config)) {
		ERROR("rlm_sql_unixodbc: Can't allocate the statement\n");
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
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	long err_handle;
	int state;

	/* Executing query */
	{
		SQLCHAR *odbc_query;

		memcpy(&odbc_query, &query, sizeof(odbc_query));
		err_handle = SQLExecDirect(conn->statement, odbc_query, strlen(query));
	}
	if ((state = sql_state(err_handle, handle, config))) {
		if(state == RLM_SQL_RECONNECT) {
			DEBUG("rlm_sql_unixodbc: rlm_sql will attempt to reconnect\n");
		}
		return state;
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
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	SQLINTEGER column;
	SQLLEN len;
	int numfields;
	int state;

	/* Only state = 0 means success */
	if ((state = sql_query(handle, config, query))) {
		return state;
	}

	numfields=sql_num_fields(handle, config);
	if (numfields < 0) {
		return -1;
	}

	/* Reserving memory for result */
	conn->row = (char **) rad_malloc((numfields+1)*sizeof(char *));
	conn->row[numfields] = NULL;

	for(column = 1; column <= numfields; column++) {
		SQLColAttributes(conn->statement,((SQLUSMALLINT) column),SQL_COLUMN_LENGTH,NULL,0,NULL,&len);
		conn->row[column-1] = (char*)rad_malloc((int)++len);
		SQLBindCol(conn->statement, column, SQL_C_CHAR, (SQLCHAR *)conn->row[column-1], len, NULL);
	}
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
	/* Not used */
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
static int sql_num_fields(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	long err_handle;
	SQLSMALLINT num_fields = 0;

	err_handle = SQLNumResultCols(conn->statement,&num_fields);
	if (sql_state(err_handle, handle, config)) {
		return -1;
	}

	return num_fields;
}


/*************************************************************************
 *
 *	Function: sql_num_rows
 *
 *	Purpose: database specific num_rows. Returns number of rows in
 *	       query
 *
 *************************************************************************/
static int sql_num_rows(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	return sql_affected_rows(handle, config);
}


/*************************************************************************
 *
 *	Function: sql_fetch_row
 *
 *	Purpose: database specific fetch_row. Returns a rlm_sql_row_t struct
 *	       with all the data for the query in 'handle->row'. Returns
 *		 0 on success, -1 on failure, RLM_SQL_RECONNECT if 'database is down'.
 *
 *************************************************************************/
static sql_rcode_t sql_fetch_row(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	long err_handle;
	int state;

	handle->row = NULL;

	err_handle = SQLFetch(conn->statement);
	if(err_handle == SQL_NO_DATA_FOUND) {
		return 0;
	}

	if ((state = sql_state(err_handle, handle, config))) {
		if(state == RLM_SQL_RECONNECT) {
	    		DEBUG("rlm_sql_unixodbc: rlm_sql will attempt to reconnect");
	    	}

		return state;
	}

	handle->row = conn->row;
	return 0;
}


/*************************************************************************
 *
 *	Function: sql_finish_select_query
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static sql_rcode_t sql_finish_select_query(rlm_sql_handle_t * handle, rlm_sql_config_t *config) {
	rlm_sql_unixodbc_conn_t *conn = handle->conn;

	sql_free_result(handle, config);
	SQLFreeStmt(conn->statement, SQL_CLOSE);
	conn->statement = NULL;

	return 0;
}

/*************************************************************************
 *
 *	Function: sql_finish_query
 *
 *	Purpose: End the query, such as freeing memory
 *
 *************************************************************************/
static sql_rcode_t sql_finish_query(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {
	rlm_sql_unixodbc_conn_t *conn = handle->conn;

	SQLFreeStmt(conn->statement, SQL_CLOSE);
	conn->statement = NULL;

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
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	int column, numfileds=sql_num_fields(handle, config);

	/* Freeing reserved memory */
	if(conn->row != NULL) {
		for(column=0; column<numfileds; column++) {
			if(conn->row[column] != NULL) {
				free(conn->row[column]);
				conn->row[column] = NULL;
			}
		}

		free(conn->row);
		conn->row = NULL;
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
static char const *sql_error(rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {
	SQLCHAR state[256];
	SQLCHAR error[256];
	SQLINTEGER errornum = 0;
	SQLSMALLINT length = 255;
	static char result[1024];	/* NOT thread-safe! */

	rlm_sql_unixodbc_conn_t *conn = handle->conn;

	error[0] = state[0] = '\0';

	SQLError(conn->env, conn->dbc, conn->statement, state, &errornum,
		 error, 256, &length);

	sprintf(result, "%s %s", state, error);
	result[sizeof(result) - 1] = '\0'; /* catch idiot thread issues */
	return result;
}

/*************************************************************************
 *
 *	Function: sql_state
 *
 *	Purpose: Returns 0 for success, RLM_SQL_RECONNECT if the error was
 *	       connection related or -1 for other errors
 *
 *************************************************************************/
static sql_rcode_t sql_state(long err_handle, rlm_sql_handle_t *handle, UNUSED rlm_sql_config_t *config) {
	SQLCHAR state[256];
	SQLCHAR error[256];
	SQLINTEGER errornum = 0;
	SQLSMALLINT length = 255;
	int res = -1;

	rlm_sql_unixodbc_conn_t *conn = handle->conn;

	if(SQL_SUCCEEDED(err_handle)) {
		return 0;		/* on success, just return 0 */
	}

	error[0] = state[0] = '\0';

	SQLError(conn->env, conn->dbc, conn->statement, state, &errornum,
		 error, 256, &length);

	if(state[0] == '0') {
		switch(state[1]) {
		/* SQLSTATE 01 class contains info and warning messages */
		case '1':
			INFO("rlm_sql_unixodbc: %s %s\n", state, error);
			/* FALL-THROUGH */
		case '0':		/* SQLSTATE 00 class means success */
			res = 0;
			break;

		/* SQLSTATE 08 class describes various connection errors */
		case '8':
			ERROR("rlm_sql_unixodbc: SQL down %s %s\n", state, error);
			res = RLM_SQL_RECONNECT;
			break;

		/* any other SQLSTATE means error */
		default:
			ERROR("rlm_sql_unixodbc: %s %s\n", state, error);
			res = -1;
		    	break;
		}
	}

	return res;
}

/*************************************************************************
 *
 *	Function: sql_affected_rows
 *
 *	Purpose: Return the number of rows affected by the query (update,
 *	       or insert)
 *
 *************************************************************************/
static int sql_affected_rows(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	rlm_sql_unixodbc_conn_t *conn = handle->conn;
	long err_handle;
	SQLLEN affected_rows;

	err_handle = SQLRowCount(conn->statement, &affected_rows);
	if (sql_state(err_handle, handle, config)) {
		return -1;
	}

	return affected_rows;
}


/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_unixodbc = {
	"rlm_sql_unixodbc",
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
