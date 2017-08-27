/*
 * sql_fbapi.c Part of Firebird rlm_sql driver
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2006  The FreeRADIUS server project
 * Copyright 2006  Vitaly Bodzhgua <vitaly@eastera.net>
 */

RCSID("$Id$")

#include "sql_fbapi.h"

#include <stdarg.h>

static void fb_set_tpb(rlm_sql_firebird_conn_t *conn, int count, ...)
{
	int i;
	va_list arg;

	va_start(arg, count);
	conn->tpb = malloc(count);

	for (i = 0; i < count; i++) {
		conn->tpb[i] = (char) va_arg(arg, int);
	}

	conn->tpb_len = count;

	va_end(arg);
}


static void fb_dpb_add_str(char **dpb, char name, char const *value)
{
	int l;

	if (!value) {
		return;
	}

	l = strlen(value);

	*(*dpb)++= name;
	*(*dpb)++= (char) l;

	memmove(*dpb, value, l);

	*dpb += l;
}

static void fb_set_sqlda(XSQLDA *sqlda) {
	int i;

	for (i = 0; i < sqlda->sqld; i++) {
		if ((sqlda->sqlvar[i].sqltype & ~1) == SQL_VARYING) {
			sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen + sizeof(short));
		} else {
			sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen);
		}

		if (sqlda->sqlvar[i].sqltype & 1) {
			sqlda->sqlvar[i].sqlind = (short*)calloc(sizeof(short), 1);
		} else {
			sqlda->sqlvar[i].sqlind = 0;
		}
	}
}

DIAG_OFF(deprecated-declarations)
int fb_error(rlm_sql_firebird_conn_t *conn)
{
	ISC_SCHAR error[2048];	/* Only 1024 bytes should be written to this, but were playing it extra safe */
	ISC_STATUS *pstatus;

	conn->sql_code = 0;

	/*
	 *	Free any previous errors.
	 */
	TALLOC_FREE(conn->error);

	/*
	 *	Check if the status array contains an error
	 */
	if (IS_ISC_ERROR(conn->status)) {
		conn->sql_code = isc_sqlcode(conn->status);

		/*
		 *	pstatus is a pointer into the status array which is
		 *	advanced by isc_interprete. It's initialised to the
		 *	first element of the status array.
		 */
		pstatus = &conn->status[0];

		/*
		 *	It's deprecated because the size of the buffer isn't
		 *	passed and this isn't safe. But as were passing a very
		 *	large buffer it's unlikely this will be an issue, and
		 *	allows us to maintain compatibility with the interbase
		 *	API.
		 */
		isc_interprete(&error[0], &pstatus);
		conn->error = talloc_asprintf(conn, "%s. ", &error[0]);

		while (isc_interprete(&error[0], &pstatus)) {
			conn->error = talloc_asprintf_append(conn->error, "%s. ", &error[0]);
		}

		memset(&conn->status, 0, sizeof(conn->status));
	}

	return conn->sql_code;
}
DIAG_ON(deprecated-declarations)

void fb_free_sqlda(XSQLDA *sqlda)
{
	int i;
	for (i = 0; i < sqlda->sqld; i++) {
		free(sqlda->sqlvar[i].sqldata);
		free(sqlda->sqlvar[i].sqlind);
	}
	sqlda->sqld = 0;
}



//Macro for NULLs check
#define IS_NULL(x) (x->sqltype & 1) && (*x->sqlind < 0)

//Structure to manage a SQL_VARYING Firebird's data types
typedef struct vary_fb {
	 short vary_length;
	 char vary_string[1];
} VARY;

//function fb_store_row based on fiebird's apifull example
void fb_store_row(rlm_sql_firebird_conn_t *conn)
{
	int dtype;
	struct tm times;
	ISC_QUAD bid;
	int i;
	XSQLVAR *var;
	VARY * vary;

	/* assumed: id, username, attribute, value, op */
	if (conn->row_fcount<conn->sqlda_out->sqld)  {
		i = conn->row_fcount;
		conn->row_fcount = conn->sqlda_out->sqld;
		conn->row = (char **) realloc(conn->row, conn->row_fcount * sizeof(char *));
		conn->row_sizes = (int *) realloc(conn->row_sizes, conn->row_fcount * sizeof(int));

		while( i <conn->row_fcount) {
			conn->row[i] = 0;
			conn->row_sizes[i++] = 0;
		}
	}

	for (i = 0, var = conn->sqlda_out->sqlvar; i < conn->sqlda_out->sqld; var++, i++) {
		/*
		 *	Initial buffer size to store field's data is 256 bytes
		 */
		if (conn->row_sizes[i]<256) {
			conn->row[i] = (char *) realloc(conn->row[i], 256);
			conn->row_sizes[i] = 256;
		}

		if (IS_NULL(var)) {
			strcpy(conn->row[i], "NULL");
			continue;
		}

		dtype = var->sqltype & ~1;

		switch (dtype) {
		case SQL_TEXT:
			if (conn->row_sizes[i]<= var->sqllen) {
				conn->row_sizes[i] = var->sqllen + 1;
				conn->row[i] = realloc(conn->row[i],
						       conn->row_sizes[i]);
			}

			memmove(conn->row[i], var->sqldata, var->sqllen);
			conn->row[i][var->sqllen] = 0;

			break;
		case SQL_VARYING:
			vary = (VARY*) var->sqldata;
			if (conn->row_sizes[i] <= vary->vary_length) {
				conn->row_sizes[i] = vary->vary_length + 1;
				conn->row[i] = realloc(conn->row[i],
						       conn->row_sizes[i]);
		   	}
			memmove(conn->row[i], vary->vary_string, vary->vary_length);
			conn->row[i][vary->vary_length] = 0;

			break;

		case SQL_FLOAT:
			snprintf(conn->row[i], conn->row_sizes[i], "%15g",
				 *(float ISC_FAR *) (var->sqldata));
			break;
		case SQL_SHORT:
		case SQL_LONG:
		case SQL_INT64:
			{
				ISC_INT64 value = 0;
				short field_width = 0;
				short dscale = 0;
				char *p;
				p = conn->row[i];

				switch (dtype) {
				case SQL_SHORT:
					value = (ISC_INT64) *(short *)var->sqldata;
					field_width = 6;
					break;
				case SQL_LONG:
					value = (ISC_INT64) *(int *)var->sqldata;
					field_width = 11;
					break;
				case SQL_INT64:
					value = (ISC_INT64) *(ISC_INT64 *)var->sqldata;
					field_width = 21;
					break;
				}
				dscale = var->sqlscale;

				if (dscale < 0) {
					ISC_INT64 tens;
					short j;

					tens = 1;
					for (j = 0; j > dscale; j--) {
						tens *= 10;
					}

					if (value >= 0) {
						sprintf(p, "%*lld.%0*lld",
							field_width - 1 + dscale,
							(ISC_INT64) value / tens,
							-dscale,
							(ISC_INT64) value % tens);
					} else if ((value / tens) != 0) {
						sprintf (p, "%*lld.%0*lld",
							field_width - 1 + dscale,
							(ISC_INT64) (value / tens),
							-dscale,
							(ISC_INT64) -(value % tens));
					} else {
						sprintf(p, "%*s.%0*lld", field_width - 1 + dscale,
							"-0", -dscale, (ISC_INT64) - (value % tens));
					}
				} else if (dscale) {
					sprintf(p, "%*lld%0*d", field_width,
						(ISC_INT64) value, dscale, 0);
				} else {
					sprintf(p, "%*lld", field_width,
						(ISC_INT64) value);
				}
			}
			break;

		case SQL_D_FLOAT:
		case SQL_DOUBLE:
			snprintf(conn->row[i], conn->row_sizes[i], "%24f",
				 *(double ISC_FAR *) (var->sqldata));
			break;

		case SQL_TIMESTAMP:
			isc_decode_timestamp((ISC_TIMESTAMP ISC_FAR *)var->sqldata, &times);
			snprintf(conn->row[i], conn->row_sizes[i], "%04d-%02d-%02d %02d:%02d:%02d.%04d",
				 times.tm_year + 1900,
				 times.tm_mon + 1,
				 times.tm_mday,
				 times.tm_hour,
				 times.tm_min,
				 times.tm_sec,
				 ((ISC_TIMESTAMP *)var->sqldata)->timestamp_time % 10000);
			break;

		case SQL_TYPE_DATE:
			isc_decode_sql_date((ISC_DATE ISC_FAR *)var->sqldata, &times);
			snprintf(conn->row[i], conn->row_sizes[i], "%04d-%02d-%02d",
				 times.tm_year + 1900,
				 times.tm_mon + 1,
				 times.tm_mday);
			break;

		case SQL_TYPE_TIME:
			isc_decode_sql_time((ISC_TIME ISC_FAR *)var->sqldata, &times);
			snprintf(conn->row[i], conn->row_sizes[i], "%02d:%02d:%02d.%04d",
				 times.tm_hour,
				 times.tm_min,
				 times.tm_sec,
				 (*((ISC_TIME *)var->sqldata)) % 10000);
			break;

		case SQL_BLOB:
		case SQL_ARRAY:
			/* Print the blob id on blobs or arrays */
			bid = *(ISC_QUAD ISC_FAR *) var->sqldata;
			snprintf(conn->row[i], conn->row_sizes[i], "%08" ISC_LONG_FMT "x:%08" ISC_LONG_FMT "x",
				 bid.gds_quad_high, bid.gds_quad_low);
			break;

		}
	}
}

int fb_init_socket(rlm_sql_firebird_conn_t *conn)
{
	memset(conn, 0, sizeof(*conn));
	conn->sqlda_out = (XSQLDA ISC_FAR *) calloc(XSQLDA_LENGTH (5), 1);
	conn->sqlda_out->sqln = 5;
	conn->sqlda_out->version =  SQLDA_VERSION1;
	conn->sql_dialect = 3;
#ifdef _PTHREAD_H
	pthread_mutex_init (&conn->mut, NULL);
	DEBUG("Init mutex %p\n", &conn->mut);
#endif

	/*
	 *	Set tpb to read_committed/wait/no_rec_version
	 */
	fb_set_tpb(conn, 5, isc_tpb_version3, isc_tpb_wait, isc_tpb_write,
		   isc_tpb_read_committed, isc_tpb_no_rec_version);
	if (!conn->tpb) {
		return -1;
	}

	return 0;
}

int fb_connect(rlm_sql_firebird_conn_t * conn, rlm_sql_config_t *config)
{
	char *p;
	char *database;

	conn->dpb_len = 4;
	if (config->sql_login) {
		conn->dpb_len+= strlen(config->sql_login) + 2;
	}

	if (config->sql_password) {
		conn->dpb_len += strlen(config->sql_password) + 2;
	}

	conn->dpb = (char *) malloc(conn->dpb_len);
	p = conn->dpb;

	*conn->dpb++= isc_dpb_version1;
	*conn->dpb++= isc_dpb_num_buffers;
	*conn->dpb++= 1;
	*conn->dpb++= 90;

	fb_dpb_add_str(&conn->dpb, isc_dpb_user_name, config->sql_login);
	fb_dpb_add_str(&conn->dpb, isc_dpb_password, config->sql_password);

	conn->dpb = p;

	/*
	 *	Check if database and server in the form of server:database.
	 *	If config->sql_server contains ':', then config->sql_db
	 *	parameter ignored.
	 */
	if (strchr(config->sql_server, ':')) {
		database = strdup(config->sql_server);
	} else {
		/*
		 *	Make database and server to be in the form
		 *	of server:database
		 */
		int ls = strlen(config->sql_server);
		int ld = strlen(config->sql_db);
		database = (char *) calloc(ls + ld + 2, 1);
		strcpy(database, config->sql_server);
		database[ls] = ':';
		memmove(database + ls + 1, config->sql_db, ld);
	}
	isc_attach_database(conn->status, 0, database, &conn->dbh,
			    conn->dpb_len, conn->dpb);
	free(database);

	return fb_error(conn);
}


int fb_fetch(rlm_sql_firebird_conn_t *conn)
{
	long fetch_stat;
	if (conn->statement_type!= isc_info_sql_stmt_select) {
		return 100;
	}

	fetch_stat = isc_dsql_fetch(conn->status, &conn->stmt,
				    SQL_DIALECT_V6, conn->sqlda_out);
	if (fetch_stat) {
		if (fetch_stat!= 100L) {
			fb_error(conn);
		} else {
			conn->sql_code = 0;
		}
	}

	return fetch_stat;
}

static int fb_prepare(rlm_sql_firebird_conn_t *conn, char const *query)
{
	static char stmt_info[] = { isc_info_sql_stmt_type };
	char info_buffer[128];
	short l;

	if (!conn->trh) {
		isc_start_transaction(conn->status, &conn->trh, 1, &conn->dbh,
				      conn->tpb_len, conn->tpb);
		if (!conn->trh) {
			return -4;
		}
	}

	fb_free_statement(conn);
	if (!conn->stmt) {
		isc_dsql_allocate_statement(conn->status, &conn->dbh,
					    &conn->stmt);
		if (!conn->stmt) {
			return -1;
		}
	}

	fb_free_sqlda(conn->sqlda_out);
	isc_dsql_prepare(conn->status, &conn->trh, &conn->stmt, 0, query,
			 conn->sql_dialect, conn->sqlda_out);
	if (IS_ISC_ERROR(conn->status)) {
		return -2;
	}

	if (conn->sqlda_out->sqln<conn->sqlda_out->sqld) {
		conn->sqlda_out->sqln = conn->sqlda_out->sqld;
		conn->sqlda_out = (XSQLDA ISC_FAR *) realloc(conn->sqlda_out,
							     XSQLDA_LENGTH(conn->sqlda_out->sqld));
		isc_dsql_describe(conn->status, &conn->stmt, SQL_DIALECT_V6,
				  conn->sqlda_out);

		if (IS_ISC_ERROR(conn->status)) {
			return -3;
		}
	}
	/*
	 *	Get statement type
	 */
	isc_dsql_sql_info(conn->status, &conn->stmt, sizeof(stmt_info),
			  stmt_info, sizeof(info_buffer), info_buffer);
	if (IS_ISC_ERROR(conn->status)) return -4;

	l = (short) isc_vax_integer((char ISC_FAR *) info_buffer + 1, 2);
	conn->statement_type = isc_vax_integer((char ISC_FAR *) info_buffer + 3,
					       l);

	if (conn->sqlda_out->sqld) {
		fb_set_sqlda(conn->sqlda_out); //set out sqlda
	}

	return 0;
}


int fb_sql_query(rlm_sql_firebird_conn_t *conn, char const *query) {
	if (fb_prepare(conn, query)) {
		return fb_error(conn);
	}

	switch (conn->statement_type) {
		case isc_info_sql_stmt_exec_procedure:
			isc_dsql_execute2(conn->status, &conn->trh, &conn->stmt,
					  SQL_DIALECT_V6, 0, conn->sqlda_out);
			break;
		default:
			isc_dsql_execute(conn->status, &conn->trh, &conn->stmt,
					 SQL_DIALECT_V6, 0);
			break;
	}
	return fb_error(conn);
}

int fb_affected_rows(rlm_sql_firebird_conn_t *conn) {
	static char count_info[] = {isc_info_sql_records};
	char info_buffer[128];
	char *p ;
	int affected_rows = -1;

	if (!conn->stmt) return -1;

	isc_dsql_sql_info(conn->status, &conn->stmt,
			  sizeof (count_info), count_info,
			  sizeof (info_buffer), info_buffer);

	if (IS_ISC_ERROR(conn->status)) {
		return fb_error(conn);
	}

	p = info_buffer + 3;
	while (*p != isc_info_end) {
		p++;
		short len = (short)isc_vax_integer(p, 2);
		p += 2;

		affected_rows = isc_vax_integer(p, len);
		if (affected_rows > 0) {
			break;
		}
		p += len;
	}
	return affected_rows;
}

int fb_close_cursor(rlm_sql_firebird_conn_t *conn) {
	isc_dsql_free_statement(conn->status, &conn->stmt, DSQL_close);

	return fb_error(conn);
}

void fb_free_statement(rlm_sql_firebird_conn_t *conn) {
	if (conn->stmt) {
		isc_dsql_free_statement(conn->status, &conn->stmt, DSQL_drop);
		conn->stmt = 0;
	}
}

int fb_rollback(rlm_sql_firebird_conn_t *conn) {
	conn->sql_code = 0;
	if (conn->trh)  {
		isc_rollback_transaction(conn->status, &conn->trh);
//		conn->in_use = 0;
#ifdef _PTHREAD_H
		pthread_mutex_unlock(&conn->mut);
#endif

		if (IS_ISC_ERROR(conn->status)) {
			return fb_error(conn);
		}
	}
	return conn->sql_code;
}

int fb_commit(rlm_sql_firebird_conn_t *conn) {
	conn->sql_code = 0;
	if (conn->trh)  {
		isc_commit_transaction (conn->status, &conn->trh);
		if (IS_ISC_ERROR(conn->status)) {
			fb_error(conn);
			ERROR("Fail to commit. Error: %s. Try to rollback.", conn->error);
			return fb_rollback(conn);
		}
	}
//	conn->in_use = 0;
#ifdef _PTHREAD_H
	pthread_mutex_unlock(&conn->mut);
#endif
	return conn->sql_code;
}
