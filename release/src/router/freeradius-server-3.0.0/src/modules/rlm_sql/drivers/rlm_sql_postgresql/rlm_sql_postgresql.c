/*
 * sql_postgresql.c		Postgresql rlm_sql driver
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
 */

/*
 * April 2001:
 *
 * Use blocking queries and delete unused functions. In
 * rlm_sql_postgresql replace all functions that are not really used
 * with the not_implemented function.
 *
 * Add a new field to the rlm_sql_postgres_conn_t struct to store the
 * number of rows affected by a query because the sql module calls
 * finish_query before it retrieves the number of affected rows from the
 * driver
 *
 * Bernhard Herzog <bh@intevation.de>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <sys/stat.h>

#include <libpq-fe.h>
#include "rlm_sql.h"
#include "sql_postgresql.h"

typedef struct rlm_sql_postgres_conn {
   PGconn	  *db;
   PGresult	*result;
   int	     cur_row;
   int	     num_fields;
   int		   affected_rows;
   char	    **row;
} rlm_sql_postgres_conn_t;

/* Internal function. Return true if the postgresql status value
 * indicates successful completion of the query. Return false otherwise
static int
status_is_ok(ExecStatusType status)
{
	return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK;
}
*/


/* Internal function. Return the number of affected rows of the result
 * as an int instead of the string that postgresql provides */
static int affected_rows(PGresult * result)
{
	return atoi(PQcmdTuples(result));
}

/* Internal function. Free the row of the current result that's stored
 * in the conn struct. */
static void free_result_row(rlm_sql_postgres_conn_t *conn)
{
	int i;
	if (conn->row != NULL) {
		for (i = conn->num_fields-1; i >= 0; i--) {
			if (conn->row[i] != NULL) {
				free(conn->row[i]);
			}
		}
		free((char*)conn->row);
		conn->row = NULL;
		conn->num_fields = 0;
	}
}


/*************************************************************************
*	Function: check_fatal_error
*
*	Purpose:  Check error type and behave accordingly
*
*************************************************************************/

static int check_fatal_error (char *errorcode)
{
	int x = 0;

	/*
	Check the error code to see if we should reconnect or not
	Error Code table taken from
	http://www.postgresql.org/docs/8.1/interactive/errcodes-appendix.html
	*/

	if (!errorcode) return -1;

	while(errorcodes[x].errorcode != NULL){
		if (strcmp(errorcodes[x].errorcode, errorcode) == 0){
			DEBUG("rlm_sql_postgresql: Postgresql Fatal Error: [%s: %s] Occurred!!", errorcode, errorcodes[x].meaning);
			if (errorcodes[x].shouldreconnect == 1)
				return RLM_SQL_RECONNECT;
			else
				return -1;
		}
		x++;
	}

	DEBUG("rlm_sql_postgresql: Postgresql Fatal Error: [%s] Occurred!!", errorcode);
	/*	We don't seem to have a matching error class/code */
	return -1;
}

static int sql_socket_destructor(void *c)
{
	rlm_sql_postgres_conn_t  *conn = c;

	DEBUG2("rlm_sql_postgresql: Socket destructor called, closing socket");

	if (!conn->db) {
		return 0;
	}

	/* PQfinish also frees the memory used by the PGconn structure */
	PQfinish(conn->db);

	return 0;
}

/*************************************************************************
 *
 *	Function: sql_create_socket
 *
 *	Purpose: Establish connection to the db
 *
 *************************************************************************/
static int sql_init_socket(rlm_sql_handle_t *handle, rlm_sql_config_t *config) {
	char dbstring[2048];
	char const *port, *host;
	rlm_sql_postgres_conn_t *conn;

#ifdef HAVE_OPENSSL_CRYPTO_H
	static int ssl_init = 0;

	if (!ssl_init) {
		PQinitSSL(0);
		ssl_init = 1;
	}

#endif

	if (config->sql_server[0] != '\0') {
		host = " host=";
	} else {
		host = "";
	}

	if (config->sql_port[0] != '\0') {
		port = " port=";
	} else {
		port = "";
	}

	MEM(conn = handle->conn = talloc_zero(handle, rlm_sql_postgres_conn_t));
	talloc_set_destructor((void *) conn, sql_socket_destructor);

	snprintf(dbstring, sizeof(dbstring),
			"dbname=%s%s%s%s%s user=%s password=%s",
			config->sql_db, host, config->sql_server,
			port, config->sql_port,
			config->sql_login, config->sql_password);

	conn->db = PQconnectdb(dbstring);

	if (PQstatus(conn->db) != CONNECTION_OK) {
		ERROR("rlm_sql_postgresql: Couldn't connect socket to "
		       "PostgreSQL server %s@%s:%s", config->sql_login,
		       config->sql_server, config->sql_db);
		return -1;
	}

	return 0;
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
	rlm_sql_postgres_conn_t *conn = handle->conn;
	int numfields = 0;
	char *errorcode;
	char *errormsg;

	if (!conn->db) {
		ERROR("rlm_sql_postgresql: Socket not connected");
		return RLM_SQL_RECONNECT;
	}

	conn->result = PQexec(conn->db, query);
		/*
		 * Returns a PGresult pointer or possibly a null pointer.
		 * A non-null pointer will generally be returned except in
		 * out-of-memory conditions or serious errors such as inability
		 * to send the command to the server. If a null pointer is
		 * returned, it should be treated like a PGRES_FATAL_ERROR
		 * result.
		 */
	if (!conn->result)
	{
		ERROR("rlm_sql_postgresql: PostgreSQL Query failed Error: %s",
				PQerrorMessage(conn->db));
		/* As this error COULD be a connection error OR an out-of-memory
		 * condition return value WILL be wrong SOME of the time regardless!
		 * Pick your poison....
		 */
		return  RLM_SQL_RECONNECT;
	} else {
		ExecStatusType status = PQresultStatus(conn->result);
		DEBUG("rlm_sql_postgresql: Status: %s", PQresStatus(status));

		switch (status){

			case PGRES_COMMAND_OK:
				/*Successful completion of a command returning no data.*/

				/*affected_rows function only returns
				the number of affected rows of a command
				returning no data...
				*/
				conn->affected_rows	= affected_rows(conn->result);
				DEBUG("rlm_sql_postgresql: query affected rows = %i", conn->affected_rows);
				return 0;

			break;

			case PGRES_TUPLES_OK:
				/*Successful completion of a command returning data (such as a SELECT or SHOW).*/

				conn->cur_row = 0;
 				conn->affected_rows = PQntuples(conn->result);
				numfields = PQnfields(conn->result); /*Check row storing functions..*/
				DEBUG("rlm_sql_postgresql: query affected rows = %i , fields = %i", conn->affected_rows, numfields);
				return 0;

			break;

			case PGRES_BAD_RESPONSE:
				/*The server's response was not understood.*/
				DEBUG("rlm_sql_postgresql: Bad Response From Server!!");
				return -1;

			break;

			case PGRES_NONFATAL_ERROR:
				/*A nonfatal error (a notice or warning) occurred. Possibly never returns*/

				return -1;

			break;

			case PGRES_FATAL_ERROR:
#if defined(PG_DIAG_SQLSTATE) && defined(PG_DIAG_MESSAGE_PRIMARY)
				/*A fatal error occurred.*/

				errorcode = PQresultErrorField(conn->result, PG_DIAG_SQLSTATE);
				errormsg  = PQresultErrorField(conn->result, PG_DIAG_MESSAGE_PRIMARY);
				DEBUG("rlm_sql_postgresql: Error %s", errormsg);
				return check_fatal_error(errorcode);
#endif

			break;

			default:
				/* FIXME: An unhandled error occurred.*/

				/* PGRES_EMPTY_QUERY PGRES_COPY_OUT PGRES_COPY_IN */

				return -1;

			break;


		}

		/*
			Note to self ... sql_store_result returns 0 anyway
			after setting the handle->affected_rows..
			sql_num_fields returns 0 at worst case which means the check below
			has a really small chance to return false..
			lets remove it then .. yuck!!
		*/
		/*
		} else {
			if ((sql_store_result(handle, config) == 0)
					&& (sql_num_fields(handle, config) >= 0))
				return 0;
			else
				return -1;
		}
		*/
	}
	return -1;
}


/*************************************************************************
 *
 *	Function: sql_select_query
 *
 *	Purpose: Issue a select query to the database
 *
 *************************************************************************/
static sql_rcode_t sql_select_query(rlm_sql_handle_t * handle, rlm_sql_config_t *config, char const *query) {
	return sql_query(handle, config, query);
}

/*************************************************************************
 *
 *	Function: sql_fetch_row
 *
 *	Purpose: database specific fetch_row. Returns a rlm_sql_row_t struct
 *	with all the data for the query in 'handle->row'. Returns
 *	0 on success, -1 on failure, RLM_SQL_RECONNECT if 'database is down'.
 *
 *************************************************************************/
static sql_rcode_t sql_fetch_row(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config) {

	int records, i, len;
	rlm_sql_postgres_conn_t *conn = handle->conn;

	handle->row = NULL;

	if (conn->cur_row >= PQntuples(conn->result))
		return 0;

	free_result_row(conn);

	records = PQnfields(conn->result);
	conn->num_fields = records;

	if ((PQntuples(conn->result) > 0) && (records > 0)) {
		conn->row = (char **)rad_malloc((records+1)*sizeof(char *));
		memset(conn->row, '\0', (records+1)*sizeof(char *));

		for (i = 0; i < records; i++) {
			len = PQgetlength(conn->result, conn->cur_row, i);
			conn->row[i] = (char *)rad_malloc(len+1);
			memset(conn->row[i], '\0', len+1);
			strlcpy(conn->row[i], PQgetvalue(conn->result, conn->cur_row,i),len + 1);
		}
		conn->cur_row++;
		handle->row = conn->row;
	}

	return 0;
}

/*************************************************************************
 *
 *      Function: sql_num_fields
 *
 *      Purpose: database specific num_fields. Returns number of rows in
 *	       query
 *
 *************************************************************************/
static int sql_num_fields(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config)
{
	rlm_sql_postgres_conn_t *conn = handle->conn;

	conn->affected_rows = PQntuples(conn->result);
	if (conn->result)
		return PQnfields(conn->result);

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
static sql_rcode_t sql_free_result(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config) {

	rlm_sql_postgres_conn_t *conn = handle->conn;

	if (conn->result) {
		PQclear(conn->result);
		conn->result = NULL;
	}

	free_result_row(conn);

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
static char const *sql_error(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config) {

	rlm_sql_postgres_conn_t *conn = handle->conn;

	return PQerrorMessage(conn->db);
}

/*************************************************************************
 *
 *	Function: sql_finish_query
 *
 *	Purpose: End the query, such as freeing memory
 *
 *************************************************************************/
static sql_rcode_t sql_finish_query(rlm_sql_handle_t * handle, rlm_sql_config_t *config) {

	return sql_free_result(handle, config);
}

/*************************************************************************
 *
 *	Function: sql_finish_select_query
 *
 *	Purpose: End the select query, such as freeing memory or result
 *
 *************************************************************************/
static sql_rcode_t sql_finish_select_query(rlm_sql_handle_t * handle, rlm_sql_config_t *config) {

	return sql_free_result(handle, config);
}


/*************************************************************************
 *
 *	Function: sql_affected_rows
 *
 *	Purpose: Return the number of rows affected by the last query.
 *
 *************************************************************************/
static int sql_affected_rows(rlm_sql_handle_t * handle, UNUSED rlm_sql_config_t *config) {
	rlm_sql_postgres_conn_t *conn = handle->conn;

	return conn->affected_rows;
}

/* Exported to rlm_sql */
rlm_sql_module_t rlm_sql_postgresql = {
	"rlm_sql_postgresql",
	NULL,
	sql_init_socket,
	sql_query,
	sql_select_query,
	NULL, /* sql_store_result */
	sql_num_fields,
	NULL, /* sql_num_rows */
	sql_fetch_row,
	NULL, /* sql_free_result */
	sql_error,
	sql_finish_query,
	sql_finish_select_query,
	sql_affected_rows,
};
