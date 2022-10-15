#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <json.h>
#include "cosql_utils.h"
#include "log.h"
#include "codb_config.h"
// codb_config_t g_codb_config[MAX_CONFIG_SIZE];

// list_head pointer
LIST_HEAD(g_codb_config_list);

int g_config_index = 0;
int g_is_config_init = 0;

#define XDIGIT_TO_NUM(h) ((h) < 'A' ? (h) - '0' : toupper (h) - 'A' + 10)

static int cosql_exec(sqlite3 *pdb, const char *fmt, ...) 
{
	int ret;
	char *errMsg = NULL;
	char *sql;
	va_list ap;

	va_start(ap, fmt);

	sql = sqlite3_vmprintf(fmt, ap);
	ret = sqlite3_exec(pdb, sql, 0, 0, &errMsg);
	if( ret != SQLITE_OK ) {
		if (errMsg) {
			codbg(pdb, "errMsg=%s", errMsg);
			sqlite3_free(errMsg);
		}
	}
	sqlite3_free(sql);

	if( ret != SQLITE_OK ) {
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

static int cosql_get_int_field(sqlite3 *pdb, const char *fmt, ...) 
{
	va_list		ap;
	int		counter, result;
	char		*sql;
	int		ret;
	sqlite3_stmt	*stmt;
	
	va_start(ap, fmt);

	if (pdb == NULL) {
		codbg(pdb, "pdb is NULL");
		return -1;
	}

	sql = sqlite3_vmprintf(fmt, ap);
	codbg(pdb, "sql=%s", sql);

	switch (sqlite3_prepare_v2(pdb, sql, -1, &stmt, NULL)) {
		case SQLITE_OK:
			break;
		default:
			codbg(pdb, "prepare failed: %s, %s", sqlite3_errmsg(pdb), sql);
			sqlite3_free(sql);
			return 0;
	}

	for (counter = 0;
		 ((result = sqlite3_step(stmt)) == SQLITE_BUSY || result == SQLITE_LOCKED) && counter < 2;
		 counter++) {
		 /* While SQLITE_BUSY has a built in timeout,
			SQLITE_LOCKED does not, so sleep */
		 if (result == SQLITE_LOCKED)
		 	sleep(1);
	}

	switch (result) {
		case SQLITE_DONE:
			/* no rows returned */
			ret = 0;
			break;
		case SQLITE_ROW:
			if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
				ret = 0;
				break;
			}
			ret = sqlite3_column_int(stmt, 0);
			break;
		default:
			codbg(pdb, "%s: step failed: %s, %s", __func__, sqlite3_errmsg(pdb), sql);
			ret = 0;
			break;
 	}

	sqlite3_free(sql);
	sqlite3_finalize(stmt);

	return ret;
}

static double cosql_get_double_field(sqlite3 *pdb, const char *fmt, ...) 
{
	va_list		ap;
	int		counter, result;
	char		*sql;
	double		ret;
	sqlite3_stmt	*stmt;
	
	va_start(ap, fmt);

	if (pdb == NULL) {
		codbg(pdb, "pdb is NULL");
		return 0;
	}

	sql = sqlite3_vmprintf(fmt, ap);

	codbg(pdb, "sql=%s", sql);

	switch (sqlite3_prepare_v2(pdb, sql, -1, &stmt, NULL)) {
		case SQLITE_OK:
			break;
		default:
			codbg(pdb, "prepare failed: %s, %s", sqlite3_errmsg(pdb), sql);
			sqlite3_free(sql);
			return 0;
	}

	for (counter = 0;
		 ((result = sqlite3_step(stmt)) == SQLITE_BUSY || result == SQLITE_LOCKED) && counter < 2;
		 counter++) {
		 /* While SQLITE_BUSY has a built in timeout,
			SQLITE_LOCKED does not, so sleep */
		 if (result == SQLITE_LOCKED)
		 	sleep(1);
	}

	switch (result) {
		case SQLITE_DONE:
			/* no rows returned */
			ret = 0;
			break;
		case SQLITE_ROW:
			if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
				ret = 0;
				break;
			}
			ret = sqlite3_column_double(stmt, 0);
			break;
		default:
			codbg(pdb, "%s: step failed: %s, %s", __func__, sqlite3_errmsg(pdb), sql);
			ret = 0;
			break;
 	}

	sqlite3_free(sql);
	sqlite3_finalize(stmt);
	return ret;
}

static char *cosql_get_text_field(sqlite3 *pdb, const char *fmt, ...) 
{
	va_list		 ap;
	int			 counter, result, len;
	char			*sql;
	char			*str;
	sqlite3_stmt	*stmt;

	va_start(ap, fmt);

	if (pdb == NULL) {
		codbg(pdb, "pdb is NULL");
		return NULL;
	}

	sql = sqlite3_vmprintf(fmt, ap);

	switch (sqlite3_prepare_v2(pdb, sql, -1, &stmt, NULL))
	{
		case SQLITE_OK:
			break;
		default:
			codbg(pdb, "prepare failed: %s, %s", sqlite3_errmsg(pdb), sql);
			sqlite3_free(sql);
			return NULL;
	}
	sqlite3_free(sql);

	for (counter = 0;
		 ((result = sqlite3_step(stmt)) == SQLITE_BUSY || result == SQLITE_LOCKED) && counter < 2;
		 counter++) {
		/* While SQLITE_BUSY has a built in timeout,
		 * SQLITE_LOCKED does not, so sleep */
		if (result == SQLITE_LOCKED)
			sleep(1);
	}

	switch (result) {
		case SQLITE_DONE:
			/* no rows returned */
			str = NULL;
			break;

		case SQLITE_ROW:
			if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
				str = NULL;
				break;
			}

			len = sqlite3_column_bytes(stmt, 0);
			if ((str = sqlite3_malloc(len + 1)) == NULL) {
				codbg(pdb, "malloc failed");
				break;
			}

			strncpy(str, (char *)sqlite3_column_text(stmt, 0), len + 1);
			break;

		default:
			codbg(pdb, "SQL step failed: %s", sqlite3_errmsg(pdb));
			str = NULL;
			break;
	}

	sqlite3_finalize(stmt);
	return str;
}

static int cosql_get_table(sqlite3 *pdb, const char *sql, char ***pazResult, int *pnRow, int *pnColumn) 
{
	int ret;
	char *errMsg = NULL;
	
	ret = sqlite3_get_table(pdb, sql, pazResult, pnRow, pnColumn, &errMsg);
	if( ret != SQLITE_OK ) {
		if (errMsg) {
			codbg(pdb, "errMsg=%s", errMsg);
			sqlite3_free(errMsg);
		}

		return COSQL_ERROR;
	}

	return COSQL_OK;
}

static int cosql_free_result(char **result)
{
	sqlite3_free_table(result);
	return COSQL_OK;
}

static char* get_operations_symbol(operation_type_e operation_type) 
{
	if (operation_type==OP_TYPE_EQUAL) {
		return "=";
	}
	else if (operation_type==OP_TYPE_NOT_EQUAL) {
		return "!=";
	}
	else if (operation_type==OP_TYPE_GREATER_THAN) {
		return ">";
	}
	else if (operation_type==OP_TYPE_LESSER_THAN) {
		return "<";
	}
	else if (operation_type==OP_TYPE_GREATER_THAN_OR_EQUAL) {
		return ">=";
	}
	else if (operation_type==OP_TYPE_LESSER_THAN_OR_EQUAL) {
		return "<=";
	}

	return NULL;
}

//- Avoid SQL injection
static int is_valid_text(const char* input) 
{
	int i,len;
	char whitelist[] = ":.-,=/_ []()%";
	char tmp[2] = {0};
	
	if(!input) {
		return FORMAT_ERROR;
	}

	len = strlen(input);

	if (len > MAX_VALUE_LEN) {
		return FORMAT_ERROR;
	}

	for (i=0; i<len; i++) {
		tmp[0] = input[i];
		
		if( isdigit(tmp[0]) || isalpha(tmp[0]) || strstr(whitelist, tmp) )
			continue;
		
		return FORMAT_ERROR;
	}

	return FORMAT_OK;
}

static int is_valid_text_mac(const char* input) 
{
	if(!input) {
		return FORMAT_ERROR;
	}

	int len = strlen(input);

	if (len > 17) {
		return FORMAT_ERROR;
	}

	// MAC format XX-XX-XX-XX-XX-XX or XX:XX:XX:XX:XX:XX
	int i = 0;
    int s = 0;

    while (*input) {
       if (isxdigit(*input)) {
          i++;
       }
       else if (*input == ':' || *input == '-') {

          if (i == 0 || i / 2 - 1 != s)
            break;

          ++s;
       }
       else {
           s = -1;
       }


       ++input;
    }

    return ((i == 12 && (s == 5 || s == 0)) == 1) ? FORMAT_OK : FORMAT_ERROR;
}

static int is_valid_text_ipv4(const char *input) 
{
	if(!input) {
		return FORMAT_ERROR;
	}

	int len = strlen(input);

	if (len > 15) {
		return FORMAT_ERROR;
	}

  	char str[31], temp[31];
	int a, b, c, d;

	//- ipv4 format 
	if (sscanf(input, "%d.%d.%d.%d", &a, &b, &c, &d) == 4 && 
		a >= 0 && a <= 255 && 
		b >= 0 && b <= 255 && 
		c >= 0 && c <= 255 && 
		d >= 0 && d <= 255) {
		
		sprintf(temp, "%d.%d.%d.%d", a, b, c, d);
		
		if (strcmp(temp, input) == 0) {
			return FORMAT_OK; //success
		}
	}

	return FORMAT_ERROR;
}

static int is_valid_text_ipv6(const char *input) 
{
	if(!input) {
		return FORMAT_ERROR;
	}

	int len = strlen(input);

	if (len > 128) {
		return FORMAT_ERROR;
	}

	/* Use lower-case for these to avoid clash with system headers.  */
	enum {
		ns_inaddrsz  = 4,
		ns_in6addrsz = 16,
		ns_int16sz   = 2
	};

	const char *curtok;
	int tp;
	const char *colonp;
	int saw_xdigit;
	unsigned int val;
	const char *end = input + strlen(input);

	tp = 0;
	colonp = NULL;

	if (input == end)
		return FORMAT_ERROR;

	/* Leading :: requires some special handling. */
	if (*input == ':')
	{
		++input;
		if (input == end || *input != ':')
		return FORMAT_ERROR;
	}

	curtok = input;
	saw_xdigit = 0;
	val = 0;

	while (input < end)
	{
		int ch = *input++;

		/* if ch is a number, add it to val. */
		if (isxdigit(ch))
		{
			val <<= 4;
			val |= XDIGIT_TO_NUM (ch);
			if (val > 0xffff)
				return FORMAT_ERROR;
			saw_xdigit = 1;
			continue;
		}

		/* if ch is a colon ... */
		if (ch == ':') {
			curtok = input;
			if (saw_xdigit==0)
			{
				if (colonp != NULL)
					return FORMAT_ERROR;
				colonp = input + tp;
				continue;
			}
			else if (input == end)
				return FORMAT_ERROR;

			if (tp > ns_in6addrsz - ns_int16sz)
				return FORMAT_ERROR;

			tp += ns_int16sz;
			saw_xdigit = 0;
			val = 0;
			continue;
		}

		/* if ch is a dot ... */
		// if (ch == '.' && (tp <= ns_in6addrsz - ns_inaddrsz)
		// 	&& is_valid_ipv4_address (curtok, end) == 1) {
		// 	tp += ns_inaddrsz;
		// 	saw_xdigit = 0;
		// 	break;
		// }

		return FORMAT_ERROR;
	}

	if (saw_xdigit==1) {
		if (tp > ns_in6addrsz - ns_int16sz)
			return FORMAT_ERROR;
		tp += ns_int16sz;
	}

	if (colonp != NULL){
		if (tp == ns_in6addrsz)
			return FORMAT_ERROR;
		tp = ns_in6addrsz;
	}

	if (tp != ns_in6addrsz)
		return FORMAT_ERROR;

	return FORMAT_OK;
}

static int is_valid_text_ip(const char *input) 
{
	if (is_valid_text_ipv4(input)==FORMAT_OK) {
		return FORMAT_OK;
	}

	if (is_valid_text_ipv6(input)==FORMAT_OK) {
		return FORMAT_OK;
	}

	return FORMAT_ERROR;
}

static int is_valid_text_json(const char *input) 
{
	if(!input) {
		return FORMAT_ERROR;
	}

	json_object *rootObj = json_tokener_parse(input);
	if (rootObj!=NULL) {
		json_object_put(rootObj);
		return FORMAT_OK;
	}

	return FORMAT_ERROR;
}

static char* gen_match_query_string(int match_columns_count, sql_column_match_t* match_columns, char* match_query_string, const char* split_string)
{
	if (match_columns==NULL) {
		return NULL;
	}

	int i=0;
	int match_query_string_size = 0;
	char buff[MAX_BUF_LEN];

	//- precount query command size
	sql_column_match_t* org_columns = match_columns;
	for (i=0; i<match_columns_count; i++) {
		char *column_name = match_columns->name;
		if (is_valid_text(column_name)==FORMAT_ERROR) {
			match_columns++;
			continue;
		}

		column_type_e column_type = match_columns->type;
		char* operation_symbol = get_operations_symbol(match_columns->operation);
		if (operation_symbol==NULL) {
			match_columns++;
			continue;
		}

		if (column_name==NULL || strlen(column_name)<=0) {
			match_columns++;
			continue;
		}

		memset(buff, 0, MAX_BUF_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				//- ex. column_name=column_value
				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_mac(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				//- ex. column_name=column_value
				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_ip(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				//- ex. column_name=column_value
				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_json(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				//- ex. column_name=column_value
				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;


			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId16, column_name, operation_symbol, match_columns->value.i16);
				break;
			
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu16, column_name, operation_symbol, match_columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId32, column_name, operation_symbol, match_columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu32, column_name, operation_symbol, match_columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId64, column_name, operation_symbol, match_columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu64, column_name, operation_symbol, match_columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%lf", column_name, operation_symbol, match_columns->value.f);
				break;

			default:
				match_columns++;
				continue;
		}
		
		match_query_string_size += strlen(buff) + strlen(split_string);

		match_columns++;
	}

	match_query_string = (char*)malloc(sizeof(char) * (match_query_string_size+1));
	if (match_query_string==NULL) {
		return NULL;
	}

	memset(match_query_string, 0, match_query_string_size+1);
	
	match_columns = org_columns;

	for (i=0; i<match_columns_count; i++) {

		char *column_name = match_columns->name;
		if (is_valid_text(column_name)==FORMAT_ERROR) {
			match_columns++;
			continue;
		}

		column_type_e column_type = match_columns->type;
		char* operation_symbol = get_operations_symbol(match_columns->operation);
		if (operation_symbol==NULL) {
			match_columns++;
			continue;
		}

		if (column_name==NULL || strlen(column_name)<=0) {
			match_columns++;
			continue;
		}

		memset(buff, 0, MAX_BUF_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_mac(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_ip(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (match_columns->value.t==NULL) {
					match_columns++;
					continue;
				}

				if (is_valid_text_json(match_columns->value.t)==FORMAT_ERROR) {
					match_columns++;
					continue;
				}

				snprintf(buff, MAX_BUF_LEN, "%s%s'%s'", column_name, operation_symbol, match_columns->value.t);
				break;

			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId16, column_name, operation_symbol, match_columns->value.i16);
				break;
			
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu16, column_name, operation_symbol, match_columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId32, column_name, operation_symbol, match_columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu32, column_name, operation_symbol, match_columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRId64, column_name, operation_symbol, match_columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_BUF_LEN, "%s%s%"PRIu64, column_name, operation_symbol, match_columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_BUF_LEN, "%s%s%lf", column_name, operation_symbol, match_columns->value.f);
				break;

			default:
				match_columns++;
				continue;
		}
		
		if (strlen(match_query_string)>0) {
			strncat(match_query_string, split_string, strlen(split_string));
		}

		strncat(match_query_string, buff, strlen(buff));

		match_columns++;
	}

	return match_query_string;
}

static char* gen_upsert_query_string(int columns_count, sql_column_t* columns, char* match_query_string, const char* split_string)
{
	int i=0;
	int match_query_string_size = 0;
	char buff[MAX_VALUE_LEN];

	//- precount query command size
	sql_column_t* org_columns = columns;
	for (i=0; i<columns_count; i++) {

		char *column_name = columns->name;
		if (is_valid_text(column_name)==FORMAT_ERROR) {
			columns++;
			continue;
		}

		column_type_e column_type = columns->type;

		if (column_name==NULL || strlen(column_name)<=0 || strlen(column_name)>MAX_BUF_LEN) {
			columns++;
			continue;
		}

		memset(buff, 0, MAX_VALUE_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_mac(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_ip(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_json(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId16, column_name, columns->value.i16);
				break;
			
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu16, column_name, columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId32, column_name, columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu32, column_name, columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId64, column_name, columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu64, column_name, columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%lf", column_name, columns->value.f);
				break;

			default:
				columns++;
				continue;
		}
		
		match_query_string_size += strlen(buff) + strlen(split_string);

		columns++;
	}

	match_query_string = (char*)malloc(sizeof(char) * (match_query_string_size+1));
	if (match_query_string==NULL) {
		return NULL;
	}

	memset(match_query_string, 0, match_query_string_size+1);
	
	columns = org_columns;

	for (i=0; i<columns_count; i++) {

		char *column_name = columns->name;
		if (is_valid_text(column_name)==FORMAT_ERROR) {
			columns++;
			continue;
		}

		column_type_e column_type = columns->type;
		
		if (column_name==NULL || strlen(column_name)<=0 || strlen(column_name)>MAX_BUF_LEN) {
			columns++;
			continue;
		}

		memset(buff, 0, MAX_VALUE_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_mac(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_ip(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}

				if (is_valid_text_json(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff, MAX_VALUE_LEN, "%s='%s'", column_name, columns->value.t);
				break;

			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId16, column_name, columns->value.i16);
				break;
			
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu16, column_name, columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId32, column_name, columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu32, column_name, columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRId64, column_name, columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_VALUE_LEN, "%s=%"PRIu64, column_name, columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_VALUE_LEN, "%s=%lf", column_name, columns->value.f);
				break;

			default:
				columns++;
				continue;
		}
		
		if (strlen(match_query_string)>0) {
			strncat(match_query_string, split_string, strlen(split_string));
		}

		strncat(match_query_string, buff, strlen(buff));

		columns++;
	}

	return match_query_string;
}

int format_sql_between_data_time(int start_data_time, int end_data_time, char* ret_sql) 
{
	if (ret_sql==NULL) {
		return COSQL_ERROR;
	}

	if (start_data_time==0 && end_data_time==0) {
		return COSQL_ERROR;
	}

	snprintf(ret_sql, MAX_BUF_LEN, " AND (data_time BETWEEN %d AND %d)", start_data_time, end_data_time);

	return COSQL_OK;
}

codb_config_t* codb_config_new()
{
    codb_config_t *li = (codb_config_t *)calloc(1, sizeof(codb_config_t));
    if (!li) return NULL;

    INIT_LIST_HEAD(&li->list);

    return li;
}

void codb_config_free(codb_config_t *the_cfg)
{
    if (the_cfg)
        free(the_cfg);

    return;
}
////////////////////////////////////////////////////////////////////////////////////

sqlite3 *cosql_open(const char* db_file) 
{
	sqlite3* sql_db = NULL;

	if (SQLITE_OK != sqlite3_open(db_file, &(sql_db))) {
		fprintf(stdout, "Fail to open db %s\n", db_file);
		return NULL;
	}

	// fprintf(stdout, "Success to open db %s\n", db_file);

	//- initialize g_codb_config
	// if (g_is_config_init==0) {
	// 	cosql_init_config();
	// 	g_is_config_init=1;
	// }

	codb_config_t default_cfg;
	default_cfg.enable_debug = 0;

	cosql_set_config(sql_db, &default_cfg);
	
	return sql_db;
}

int cosql_set_config(sqlite3* pdb, codb_config_t* cfg)
{
	if (pdb == NULL || cfg==NULL) {
		return COSQL_ERROR;
	}

	codb_config_t* the_cfg = cosql_get_config(pdb);
	if (the_cfg==NULL) {
		the_cfg = codb_config_new();
		the_cfg->pdb = pdb;
		the_cfg->enable_debug = 0;
		list_add(&the_cfg->list, &g_codb_config_list);
	}
	else {
		the_cfg->enable_debug = cfg->enable_debug;
	}

	return COSQL_OK;
}

codb_config_t* cosql_get_config(sqlite3* pdb)
{
	if (pdb==NULL) {
		return NULL;
	}

	codb_config_t *the_cfg;

    list_for_each_entry(the_cfg, &g_codb_config_list, list) {
        if (the_cfg->pdb==pdb) {
			return the_cfg;
		}
    }
	
	return NULL;
}

int cosql_remove_config(sqlite3* pdb)
{
	if (pdb==NULL) {
		return COSQL_ERROR;
	}
	
	codb_config_t *the_cfg, *the_cfg_tmp;

    list_for_each_entry_safe(the_cfg, the_cfg_tmp, &g_codb_config_list, list) {
        list_del(&the_cfg->list);
        codb_config_free(the_cfg);
    }

	return COSQL_OK;
}

void cosql_enable_debug(sqlite3* pdb, int enable)
{
	if (pdb==NULL) {
		return;
	}

	codb_config_t* the_cfg = cosql_get_config(pdb);
	if (the_cfg!=NULL) {
		the_cfg->enable_debug = enable;
	}
}

int cosql_close(sqlite3 * pdb) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}
	
	if (SQLITE_OK != sqlite3_close(pdb)) { 	
		codbg(pdb, "Fail to close db");
		return COSQL_ERROR;
	}

	cosql_remove_config(pdb);

	codbg(pdb, "Success to close db");

	return COSQL_OK;
}

int cosql_create_table(sqlite3 *pdb, const char* db_version, int columns_count, sql_column_prototype_t* columns) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	int count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM sqlite_master WHERE type='table' AND name='%s'", DATA_TABLE_NAME);
	if (count!=0) {
		//- If the table exists and the version is the same, COSQL_OK is returned, and there is no need to create it again.
		char this_db_version[MAX_VERSION_LEN];
		cosql_get_db_version(pdb, this_db_version);

		codbg(pdb, "this_db_version=%s, db_version=%s", this_db_version, db_version);
		
		if (db_version!=NULL && strncmp(this_db_version, db_version, strlen(db_version))==0) {
			return COSQL_OK;
		}

		codbg(pdb, "db version is mismatch");

		return COSQL_DB_VERSION_MISMATCH;
	}

	int i, ret = 0;
	sql_column_prototype_t *org_columns = columns;
	char buff[MAX_BUF_LEN];

	//- precount query command size
	int sql_create_table_size = 0;
	for (i=0; i<columns_count; i++) {
		char *column_name = columns->name;
		column_type_e column_type = columns->type;

		if (column_name==NULL || strlen(column_name)<=0) {
			columns++;
			continue;
		}

		memset(buff, 0, MAX_BUF_LEN);
		
		switch (column_type) {
			case COLUMN_TYPE_TEXT:
			case COLUMN_TYPE_TEXT_MAC:
			case COLUMN_TYPE_TEXT_IP:
			case COLUMN_TYPE_TEXT_JSON:
				snprintf(buff, MAX_BUF_LEN, "%s TEXT DEFAULT '' NOT NULL", column_name);
				break;

			case COLUMN_TYPE_INT:
			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_BUF_LEN, "%s INT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_UINT:
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_BUF_LEN, "%s UNSIGNED INT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_BUF_LEN, "%s BIGINT DEFAULT 0 NOT NULL", column_name);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_BUF_LEN, "%s UNSIGNED BIGINT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_BUF_LEN, "%s DOUBLE DEFAULT 0 NOT NULL", column_name);
				break;

			default:
				columns++;
				continue;
		}
		
		sql_create_table_size += strlen(buff)+1;

		columns++;
	}

	//- malloc() allocate the memory for sql_create_table_size+1 chars 
	char* sql_create_table = (char*)malloc(sizeof(char) * (sql_create_table_size+1));
	if (sql_create_table==NULL) {
		codbg(pdb, "fail to create table %s", DATA_TABLE_NAME);
		return COSQL_ERROR;
	}

	memset(sql_create_table, 0, sql_create_table_size+1);
	/////////////////////////////////////////////////////////////

	columns = org_columns;

	for (i=0; i<columns_count; i++) {

		char *column_name = columns->name;
		column_type_e column_type = columns->type;

		if (column_name==NULL || strlen(column_name)<=0) {
			columns++;
			continue;
		}

		memset(buff, 0, MAX_BUF_LEN);
		
		switch (column_type) {
			case COLUMN_TYPE_TEXT:
			case COLUMN_TYPE_TEXT_MAC:
			case COLUMN_TYPE_TEXT_IP:
			case COLUMN_TYPE_TEXT_JSON:
				snprintf(buff, MAX_BUF_LEN, "%s TEXT DEFAULT '' NOT NULL", column_name);
				break;

			case COLUMN_TYPE_INT:
			case COLUMN_TYPE_INT16:
				snprintf(buff, MAX_BUF_LEN, "%s INT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_UINT:
			case COLUMN_TYPE_UINT16:
				snprintf(buff, MAX_BUF_LEN, "%s UNSIGNED INT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff, MAX_BUF_LEN, "%s BIGINT DEFAULT 0 NOT NULL", column_name);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff, MAX_BUF_LEN, "%s UNSIGNED BIGINT DEFAULT 0 NOT NULL", column_name);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff, MAX_BUF_LEN, "%s DOUBLE DEFAULT 0 NOT NULL", column_name);
				break;

			default:
				columns++;
				continue;
		}
		
		if (strlen(sql_create_table)>0) {
			strncat(sql_create_table, ",", 1);
		}

		strncat(sql_create_table, buff, strlen(buff));

		columns++;
	}

	// fprintf(stdout, "CREATE TABLE %s (%s)\n", DATA_TABLE_NAME, sql_create_table);

	ret = cosql_exec(pdb, "CREATE TABLE %s (data_id INTEGER PRIMARY KEY AUTOINCREMENT, %s, data_time TIMESTAMP)", DATA_TABLE_NAME, sql_create_table);
	if (sql_create_table!=NULL) {
		free(sql_create_table);
	}

	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to create table %s", DATA_TABLE_NAME);
		return COSQL_ERROR;
	}
	//////////////////////////////////////////////////////////////

	ret = cosql_exec(pdb, "CREATE TABLE %s (info_name TEXT, info_value TEXT)", DB_TABLE_NAME);
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to create table %s", DB_TABLE_NAME);
		return COSQL_ERROR;
	}
	
	ret = cosql_exec(pdb, "INSERT INTO %s (info_name, info_value) VALUES ('db_version', '%s')", DB_TABLE_NAME, db_version);
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to inert table %s", DB_TABLE_NAME);
		return COSQL_ERROR;
	}
	//////////////////////////////////////////////////////////////

	ret = cosql_exec(pdb, "CREATE INDEX idx_%s ON %s (data_id)", DATA_TABLE_NAME, DATA_TABLE_NAME);
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to create table idx_%s", DATA_TABLE_NAME);
		return COSQL_ERROR;
	}
	//////////////////////////////////////////////////////////////

	return COSQL_OK;
}

int cosql_get_db_version(sqlite3* pdb, char* db_version) 
{
	if (db_version==NULL) {
		return COSQL_ERROR;
	}

	char* version = cosql_get_text_field(pdb, "SELECT info_value FROM %s WHERE info_name='db_version' LIMIT 1", DB_TABLE_NAME);
	if (version==NULL) {
		return COSQL_ERROR;
	}
	
	snprintf(db_version, MAX_VERSION_LEN, "%s", version);

	sqlite3_free(version);

	return COSQL_OK;
}

int cosql_truncate_data_db(sqlite3* pdb)
{
	cosql_exec(pdb, "DELETE FROM %s", DATA_TABLE_NAME);
	cosql_exec(pdb, "DELETE FROM sqlite_sequence WHERE name='%s'", DATA_TABLE_NAME);
	return COSQL_OK;
}

int cosql_drop_db(sqlite3* pdb)
{
	cosql_exec(pdb, "DROP TABLE %s", DB_TABLE_NAME);
	cosql_exec(pdb, "DROP TABLE %s", DATA_TABLE_NAME);
	return COSQL_OK;
}

int cosql_integrity_check(sqlite3* pdb)
{
	int ret = cosql_exec(pdb, "PRAGMA integrity_check");
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to integrity check");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_clear_table(sqlite3* pdb) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	return cosql_exec(pdb, "DELETE FROM %s", DATA_TABLE_NAME);
}

int cosql_count_table(sqlite3* pdb, int* ret_count) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s", DATA_TABLE_NAME);

	return COSQL_OK;
}

extern int cosql_count_matchs(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	int* ret_count) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE (%s) AND (%s)", DATA_TABLE_NAME, sql_and_where, sql_or_where);
	}
	else if (sql_and_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE %s", DATA_TABLE_NAME, sql_and_where);
	}
	else if (sql_or_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE %s", DATA_TABLE_NAME, sql_or_where);
	}
	else {
		*ret_count = 0;
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

extern int cosql_count_matchs_between_time(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	int start_data_time, int end_data_time,
	int* ret_count) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char sql_between_data_time[MAX_BUF_LEN];
	memset(sql_between_data_time, 0, MAX_BUF_LEN);
	format_sql_between_data_time(start_data_time, end_data_time, sql_between_data_time);

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE (%s) AND (%s) %s", DATA_TABLE_NAME, sql_and_where, sql_or_where, sql_between_data_time);
	}
	else if (sql_and_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE %s %s", DATA_TABLE_NAME, sql_and_where, sql_between_data_time);
	}
	else if (sql_or_where!=NULL) {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE %s %s", DATA_TABLE_NAME, sql_or_where, sql_between_data_time);
	}
	else {
		*ret_count = cosql_get_int_field(pdb, "SELECT COUNT(*) AS count FROM %s WHERE 1 %s", DATA_TABLE_NAME, sql_between_data_time);
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

int cosql_insert_table(sqlite3* pdb, int columns_count, sql_column_t* columns) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}
	
	int i=0, ret=0;
	
	time_t current_time = time(NULL);
	char buff_column_name[MAX_BUF_LEN];
	char buff_insert_value[MAX_VALUE_LEN];

	//- precount query command size
	sql_column_t* org_columns = columns;
	int sql_insert_columns_size = 0;
	int sql_insert_values_size = 0;
	for (i=0; i<columns_count; i++) {
		char *column_name = columns->name;
		column_type_e column_type = columns->type;

		if (column_name==NULL || strlen(column_name)<=0 || strlen(column_name)>MAX_BUF_LEN) {
			columns++;
			continue;
		}

		memset(buff_column_name, 0, MAX_BUF_LEN);
		memset(buff_insert_value, 0, MAX_VALUE_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_mac(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_ip(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_json(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_INT16:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId16, columns->value.i16);
				break;

			case COLUMN_TYPE_UINT16:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu16, columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId32, columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu32, columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId64, columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu64, columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%lf", columns->value.f);
				break;

			default:
				columns++;
				continue;
		}

		snprintf(buff_column_name, MAX_BUF_LEN, "%s", column_name);
		sql_insert_columns_size += strlen(buff_column_name)+1;

		sql_insert_values_size += strlen(buff_insert_value)+1;

		columns++;
	}
	
	//- malloc() allocate the memory for sql_insert_columns_size+1 chars 
	char* sql_column_names = (char*)malloc(sizeof(char) * (sql_insert_columns_size+1));
	if (sql_column_names==NULL) {
		codbg(pdb, "fail to alloc sql buffer");
		return COSQL_ERROR;
	}

	memset(sql_column_names, 0, sql_insert_columns_size+1);

	//- malloc() allocate the memory for sql_insert_values_size+1 chars 
	char* sql_insert_values = (char*)malloc(sizeof(char) * (sql_insert_values_size+1));
	if (sql_insert_values==NULL) {
		codbg(pdb, "fail to alloc sql buffer");
		return COSQL_ERROR;
	}

	memset(sql_insert_values, 0, sql_insert_values_size+1);
	/////////////////////////////////////////////////////////////
	
	columns = org_columns;

	for (i=0; i<columns_count; i++) {

		char *column_name = columns->name;
		column_type_e column_type = columns->type;

		if (column_name==NULL || strlen(column_name)<=0 || strlen(column_name)>MAX_BUF_LEN) {
			columns++;
			continue;
		}

		memset(buff_column_name, 0, MAX_BUF_LEN);
		memset(buff_insert_value, 0, MAX_VALUE_LEN);

		switch (column_type) {
			case COLUMN_TYPE_TEXT:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_MAC:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_mac(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_IP:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_ip(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_TEXT_JSON:
				if (columns->value.t==NULL) {
					columns++;
					continue;
				}
				
				if (is_valid_text_json(columns->value.t)==FORMAT_ERROR) {
					columns++;
					continue;
				}

				snprintf(buff_insert_value, MAX_VALUE_LEN, "'%s'", columns->value.t);
				break;

			case COLUMN_TYPE_INT16:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId16, columns->value.i16);
				break;
			
			case COLUMN_TYPE_UINT16:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu16, columns->value.ui16);
				break;

			case COLUMN_TYPE_INT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId32, columns->value.i);
				break;

			case COLUMN_TYPE_UINT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu32, columns->value.ui);
				break;

			case COLUMN_TYPE_INT64:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRId64, columns->value.i64);
				break;
			
			case COLUMN_TYPE_UINT64:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%"PRIu64, columns->value.ui64);
				break;

			case COLUMN_TYPE_FLOAT:
				snprintf(buff_insert_value, MAX_VALUE_LEN, "%lf", columns->value.f);
				break;

			default:
				columns++;
				continue;
		}
		
		snprintf(buff_column_name, MAX_BUF_LEN, "%s", column_name);

		if (strlen(sql_column_names)>0) {
			strncat(sql_column_names, ",", 1);
		}

		if (strlen(sql_insert_values)>0) {
			strncat(sql_insert_values, ",", 1);
		}

		strncat(sql_column_names, buff_column_name, strlen(buff_column_name));
		strncat(sql_insert_values, buff_insert_value, strlen(buff_insert_value));

		columns++;
	}
	
	codbg(pdb, "INSERT INTO %s (%s,data_time) VALUES (%s,%ld)", DATA_TABLE_NAME, sql_column_names, sql_insert_values, current_time);

	ret = cosql_exec(pdb, "INSERT INTO %s (%s,data_time) VALUES (%s,%ld)", DATA_TABLE_NAME, sql_column_names, sql_insert_values, current_time);

	if (sql_column_names!=NULL) {
		free(sql_column_names);
	}

	if (sql_insert_values!=NULL) {
		free(sql_insert_values);
	}

	return ret;
}

int cosql_upsert_table(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	int columns_count, sql_column_t* columns) 
{	
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	int data_id = -1;
	if (cosql_get_int_value(pdb, match_and_columns_count, match_and_columns, match_or_columns_count, match_or_columns, "data_id", &data_id)!=COSQL_OK) {
		return COSQL_ERROR;
	}

	//- insert data
	if (data_id<=0) {
		if (cosql_insert_table(pdb, columns_count, columns)!=COSQL_OK){
			return COSQL_ERROR;
		}
		return COSQL_OK;
	}

	//- update data
	char* sql_update_value = NULL;
	sql_update_value = gen_upsert_query_string(columns_count, columns, sql_update_value, ",");
	if (sql_update_value==NULL) {
		return COSQL_ERROR;
	}

	codbg(pdb, "update data id [%d], value [%s]", data_id, sql_update_value);

	int ret = cosql_exec(pdb, "UPDATE %s SET %s WHERE data_id=%d", DATA_TABLE_NAME, sql_update_value, data_id);

	if (sql_update_value!=NULL) {
		free(sql_update_value);
	}

	return ret;
}

int cosql_get_int_value(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	const char* column_name, int* ret_value) 
{	
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_value = cosql_get_int_field(pdb, "SELECT %s FROM %s WHERE (%s) AND (%s)", column_name, DATA_TABLE_NAME, sql_and_where, sql_or_where);
	}
	else if (sql_and_where!=NULL) {
		*ret_value = cosql_get_int_field(pdb, "SELECT %s FROM %s WHERE %s", column_name, DATA_TABLE_NAME, sql_and_where);
	}
	else if (sql_or_where!=NULL) {
		*ret_value = cosql_get_int_field(pdb, "SELECT %s FROM %s WHERE %s", column_name, DATA_TABLE_NAME, sql_or_where);
	}
	else {
		*ret_value = 0;
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}
	
	return COSQL_OK;
}

int cosql_get_column_values(sqlite3* pdb, 
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    int query_columns_count, sql_column_prototype_t* query_columns,
	 int start_data_time, int end_data_time,
    const char* order_column_name,
    const char* order_by,
    int limit,
    int* ret_rows,
	char ***ret_result) 
{
	int i;

	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (query_columns_count==0) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}
	//////////////////////////////////////////////////////////

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}
	//////////////////////////////////////////////////////////

	//- precount query column size
	sql_column_prototype_t* org_columns = query_columns;
	int sql_query_columns_size = 0;
	
	for (i=0; i<query_columns_count; i++) {
		char *column_name = query_columns->name;
		if (column_name!=NULL && strlen(column_name)>0) {
			sql_query_columns_size += strlen(column_name)+1;
		}

		query_columns++;
	}
	
	//- malloc() allocate the memory for sql_query_columns_size+1 chars 
	char* sql_column_names = (char*)malloc(sizeof(char) * (sql_query_columns_size+1));
	if (sql_column_names==NULL) {
		codbg(pdb, "fail to alloc sql buffer");
		return COSQL_ERROR;
	}

	memset(sql_column_names, 0, sql_query_columns_size+1);

	query_columns = org_columns;

	for (i=0; i<query_columns_count; i++) {
		char *column_name = query_columns->name;

		if (column_name!=NULL && strlen(column_name)>0) {

			if (strlen(sql_column_names)>0) {
				strncat(sql_column_names, ",", 1);
			}

			strncat(sql_column_names, column_name, strlen(column_name));
		}

		query_columns++;
	}
	//////////////////////////////////////////////////////////
	
	//- malloc() allocate the memory for sql_query_columns_size+1 chars 
	int sql_query_size = sql_query_columns_size+1024;
	char* sql_query = (char*)malloc(sizeof(char) * (sql_query_size));
	if (sql_query==NULL) {
		codbg(pdb, "fail to alloc sql buffer");
		return COSQL_ERROR;
	}

	memset(sql_query, 0, sql_query_size);

	snprintf(sql_query, sql_query_size, "SELECT %s FROM %s", sql_column_names, DATA_TABLE_NAME);

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		strncat(sql_query, " WHERE (", 8);
		strncat(sql_query, sql_and_where, strlen(sql_and_where));
		strncat(sql_query, ") AND (", 7);
		strncat(sql_query, sql_or_where, strlen(sql_or_where));
		strncat(sql_query, ")", 1);
	}
	else if (sql_and_where!=NULL) {
		strncat(sql_query, " WHERE (", 8);
		strncat(sql_query, sql_and_where, strlen(sql_and_where));
		strncat(sql_query, ")", 1);
	}
	else if (sql_or_where!=NULL) {
		strncat(sql_query, " WHERE (", 8);
		strncat(sql_query, sql_or_where, strlen(sql_or_where));
		strncat(sql_query, ")", 1);
	}
	else {
		strncat(sql_query, " WHERE 1", 8);
	}

	if (start_data_time>0 && end_data_time>0) {
		char sql_between_data_time[MAX_BUF_LEN];
		memset(sql_between_data_time, 0, MAX_BUF_LEN);
		format_sql_between_data_time(start_data_time, end_data_time, sql_between_data_time);

		strncat(sql_query, sql_between_data_time, strlen(sql_between_data_time));
	}

	if (order_column_name!=NULL && order_by!=NULL) {
		strncat(sql_query, " ORDER BY ", 10);
		strncat(sql_query, order_column_name, strlen(order_column_name));
		strncat(sql_query, " ", 1);
		strncat(sql_query, order_by, strlen(order_by));
	}
	
	if (limit>0) {
		strncat(sql_query, " LIMIT ", 7);

		char buf_limit[5];
		snprintf(buf_limit, 5, "%d", limit);
		strncat(sql_query, buf_limit, strlen(buf_limit));
	}
	
	codbg(pdb, "sql_query=%s", sql_query);
	
	if (cosql_get_table(pdb, sql_query, ret_result, ret_rows, NULL) != COSQL_OK) {
		
		if (sql_and_where!=NULL) {
			free(sql_and_where);
		}

		if (sql_or_where!=NULL) {
			free(sql_or_where);
		}

		if (sql_column_names!=NULL) {
			free(sql_column_names);
		}

		if (sql_query!=NULL) {
			free(sql_query);
		}

		return COSQL_ERROR;
	}
	
	// fprintf(stdout, "sql_query=%s\n", sql_query);

	// fprintf(stdout, "query rows=%d\n", ret_rows);
	// int j;
	// for (i=0; i<rows; i++) {
	// 	for (j=0; j<query_columns_count; j++) {
	// 		int start_index = query_columns_count + i*query_columns_count;
	// 		char* colume_value = ret_result[start_index + j];
	// 		fprintf(stdout, "i=%d, colume_value=%s\n", i, colume_value);
	// 	}			
	// }							
	
	// *ret_rows = rows;
	
	// cosql_free_result(result);

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	if (sql_column_names!=NULL) {
		free(sql_column_names);
	}

	if (sql_query!=NULL) {
		free(sql_query);
	}

	return COSQL_OK;
}

int cosql_free_column_values(char **result)
{
	cosql_free_result(result);
	return COSQL_OK;
}

int cosql_get_last_xth_double_value(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	const char* column_name, int last_xnd_index, 
	double* ret_value, time_t* ret_time) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}
	
	int sql_query_size = 256;

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
		sql_query_size += strlen(sql_and_where);
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
		sql_query_size += strlen(sql_or_where);
	}

	last_xnd_index = last_xnd_index - 1;
	if (last_xnd_index<0) last_xnd_index = 0;
	
	char* sql_query = (char*)malloc(sizeof(char) * sql_query_size);
	if (sql_query==NULL) {
		codbg(pdb, "fail to alloc sql_query");
		return COSQL_ERROR;
	}

	memset(sql_query, 0, sql_query_size);
	
	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		snprintf(sql_query, sql_query_size, "SELECT %s, data_time"
			" FROM %s"
			" WHERE (%s) AND (%s)"
			" ORDER BY data_id DESC"
			" LIMIT %d, 1", 
			column_name, 
			DATA_TABLE_NAME, 
			sql_and_where, sql_or_where, 
			last_xnd_index);
	}
	else if (sql_and_where!=NULL) {
		snprintf(sql_query, sql_query_size, "SELECT %s, data_time"
			" FROM %s"
			" WHERE %s"
			" ORDER BY data_id DESC"
			" LIMIT %d, 1", 
			column_name, 
			DATA_TABLE_NAME, 
			sql_and_where, 
			last_xnd_index);
	}
	else if (sql_or_where!=NULL) {
		snprintf(sql_query, sql_query_size, "SELECT %s, data_time"
			" FROM %s"
			" WHERE %s"
			" ORDER BY data_id DESC"
			" LIMIT %d, 1", 
			column_name, 
			DATA_TABLE_NAME, 
			sql_or_where, 
			last_xnd_index);
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}
	
	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	int rows;
	char **result;
	int column_count = 2;

	if (cosql_get_table(pdb, sql_query, &result, &rows, NULL) != COSQL_OK) {
		cosql_free_result(result);
		return COSQL_ERROR;
	}

	// fprintf(stdout, "rows=%d\n", rows);
	if (!rows) {
		*ret_value = 0;
		*ret_time = 0;

		if (sql_query!=NULL) {
			free(sql_query);
		}

		return COSQL_OK;
	}

	char* colume_value = result[column_count];
	char* row_time = result[column_count+1];								
	
	*ret_value = atof(colume_value);
	*ret_time = atol(row_time);
	
	cosql_free_result(result);

	if (sql_query!=NULL) {
		free(sql_query);
	}

	return COSQL_OK;
}

int cosql_sum_between_time(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	const char* column_name, 
	int start_data_time, int end_data_time, 
	double* ret_value) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}

	if (start_data_time<=0 || end_data_time<=0 || start_data_time>end_data_time) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char sql_between_data_time[MAX_BUF_LEN];
	memset(sql_between_data_time, 0, MAX_BUF_LEN);
	format_sql_between_data_time(start_data_time, end_data_time, sql_between_data_time);

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s)"
				" FROM %s"
				" WHERE (%s) AND (%s)"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_and_where,
				sql_or_where,
				sql_between_data_time);
	}
	else if (sql_and_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s)"
				" FROM %s"
				" WHERE %s"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_and_where,
				sql_between_data_time);
	}
	else if (sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s)"
				" FROM %s"
				" WHERE %s"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_or_where,
				sql_between_data_time);
	}
	else {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s)"
				" FROM %s"
				" WHERE 1 %s",
				column_name, 
				DATA_TABLE_NAME,
				sql_between_data_time);
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}
	
	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

int cosql_avg_between_time(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns,
	const char* column_name, 
	int start_data_time, int end_data_time, 
	double* ret_value) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}

	if (start_data_time<=0 || end_data_time<=0 || start_data_time>end_data_time) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}
	
	char sql_between_data_time[MAX_BUF_LEN];
	memset(sql_between_data_time, 0, MAX_BUF_LEN);
	format_sql_between_data_time(start_data_time, end_data_time, sql_between_data_time);

	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s)"
				" FROM %s"
				" WHERE (%s) AND (%s)"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_and_where,
				sql_or_where,
				sql_between_data_time);
	}
	else if (sql_and_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s)"
				" FROM %s"
				" WHERE %s"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_and_where,
				sql_between_data_time);
	}
	else if (sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s)"
				" FROM %s"
				" WHERE %s"
				"%s",
				column_name, 
				DATA_TABLE_NAME,
				sql_or_where,
				sql_between_data_time);
	}
	else {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s)"
				" FROM %s"
				" WHERE 1 %s",
				column_name, 
				DATA_TABLE_NAME,
				sql_between_data_time);
	}
	
	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

int cosql_sum_latest_count_limit(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns,
	const char* column_name, int latest_count_limit, 
	double* ret_value) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}
	
	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s) AS sum_value"
			" FROM %s"
			" WHERE data_id IN ("
			" SELECT data_id"
			" FROM %s"
			" WHERE (%s) AND (%s)"
			" ORDER BY data_id DESC"
			" LIMIT %d"
			" )",
			column_name, 
			DATA_TABLE_NAME,
			DATA_TABLE_NAME,
			sql_and_where,
			sql_or_where,
			latest_count_limit);
	}
	else if (sql_and_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s) AS sum_value"
			" FROM %s"
			" WHERE data_id IN ("
			" SELECT data_id"
			" FROM %s"
			" WHERE %s"
			" ORDER BY data_id DESC"
			" LIMIT %d"
			" )",
			column_name, 
			DATA_TABLE_NAME,
			DATA_TABLE_NAME,
			sql_and_where,
			latest_count_limit);
	}
	else if (sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT SUM(%s) AS sum_value"
			" FROM %s"
			" WHERE data_id IN ("
			" SELECT data_id"
			" FROM %s"
			" WHERE %s"
			" ORDER BY data_id DESC"
			" LIMIT %d"
			" )",
			column_name, 
			DATA_TABLE_NAME,
			DATA_TABLE_NAME,
			sql_or_where,
			latest_count_limit);
	}
	else {
		*ret_value = 0;
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

int cosql_avg_latest_count_limit(sqlite3* pdb, 
	int match_and_columns_count, sql_column_match_t* match_and_columns, 
	int match_or_columns_count, sql_column_match_t* match_or_columns, 
	const char* column_name, int latest_count_limit, 
	double* ret_value)
{

	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	if (strlen(column_name)==0) {
		return COSQL_ERROR;
	}

	char* sql_and_where = NULL;
	if (match_and_columns_count>0) {
		sql_and_where = gen_match_query_string(match_and_columns_count, match_and_columns, sql_and_where, " AND ");
		if (sql_and_where==NULL) {
			return COSQL_ERROR;
		}
	}

	char* sql_or_where = NULL;
	if (match_or_columns_count>0) {
		sql_or_where = gen_match_query_string(match_or_columns_count, match_or_columns, sql_or_where, " OR ");
		if (sql_or_where==NULL) {
			return COSQL_ERROR;
		}
	}
	
	if (sql_and_where!=NULL && sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s) AS avg_value"
				" FROM %s"
				" WHERE data_id IN ("
				" SELECT data_id"
				" FROM %s"
				" WHERE (%s) AND (%s)"
				" ORDER BY data_id DESC"
				" LIMIT %d"
				" )",
				column_name, 
				DATA_TABLE_NAME,
				DATA_TABLE_NAME,
				sql_and_where,
				sql_or_where,
				latest_count_limit);
	}
	else if (sql_and_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s) AS avg_value"
				" FROM %s"
				" WHERE data_id IN ("
				" SELECT data_id"
				" FROM %s"
				" WHERE %s"
				" ORDER BY data_id DESC"
				" LIMIT %d"
				" )",
				column_name, 
				DATA_TABLE_NAME,
				DATA_TABLE_NAME,
				sql_and_where,
				latest_count_limit);
	}
	else if (sql_or_where!=NULL) {
		*ret_value = cosql_get_double_field(pdb, "SELECT AVG(%s) AS avg_value"
				" FROM %s"
				" WHERE data_id IN ("
				" SELECT data_id"
				" FROM %s"
				" WHERE %s"
				" ORDER BY data_id DESC"
				" LIMIT %d"
				" )",
				column_name, 
				DATA_TABLE_NAME,
				DATA_TABLE_NAME,
				sql_or_where,
				latest_count_limit);
	}
	else {
		*ret_value = 0;
	}

	if (sql_and_where!=NULL) {
		free(sql_and_where);
	}

	if (sql_or_where!=NULL) {
		free(sql_or_where);
	}

	return COSQL_OK;
}

int cosql_get_oldest_event_time(sqlite3* pdb, time_t* ret_time) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	*ret_time = cosql_get_int_field(pdb, "SELECT data_time FROM %s ORDER BY data_id ASC LIMIT 1", DATA_TABLE_NAME);
	return COSQL_OK;
}

int cosql_get_latest_event_time(sqlite3* pdb, time_t* ret_time) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	*ret_time = cosql_get_int_field(pdb, "SELECT data_time FROM %s ORDER BY data_id DESC LIMIT 1", DATA_TABLE_NAME);
	return COSQL_OK;
}

int cosql_remove_data_between_time(sqlite3* pdb, int start_data_time, int end_data_time) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;

	ret = cosql_exec(pdb, "DELETE FROM %s WHERE data_time BETWEEN %d AND %d", DATA_TABLE_NAME, start_data_time, end_data_time);
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to remove data between time");
		return COSQL_ERROR;
	}

	ret = cosql_exec(pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_remove_data_between_column_value(sqlite3* pdb, const char* column_name, int start_value, int end_value)
{
	if (pdb == NULL || column_name == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;

	ret = cosql_exec(pdb, "DELETE FROM %s WHERE %s BETWEEN %d AND %d", DATA_TABLE_NAME, column_name, start_value, end_value);
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to remove data between time");
		return COSQL_ERROR;
	}

	ret = cosql_exec(pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_resize_table_by_reserved_count(sqlite3* pdb, int reserved_newest_data_count) 
{
	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;

	ret = cosql_exec(pdb, "DELETE FROM %s"
						  " WHERE data_id NOT IN"
						  " ("
						  " SELECT data_id FROM %s ORDER BY data_id DESC LIMIT %d"
						  " )",
						  DATA_TABLE_NAME,
						  DATA_TABLE_NAME,
						  reserved_newest_data_count);

	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to remove data by count");
		return COSQL_ERROR;
	}

	ret = cosql_exec(pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int xxcosql_resize_table_by_count(sqlite3* pdb, const char* table_name, const char* field_name, int reserved_newest_data_count) {

	if (pdb == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;

	// int total_count = cosql_count(pdb, table_name);
	// int delete_count = 0;

	// if (reserved_newest_data_count>=total_count) {
	// 	delete_count = total_count;
	// }
	// else if (reserved_newest_data_count<0) {
	// 	delete_count = 0;
	// }
	// else {
	// 	delete_count = total_count - reserved_newest_data_count;
	// }

	// fprintf(stdout, "DELETE FROM %s LIMIT %d", table_name, delete_count);
	// ret = cosql_exec(pdb, "DELETE FROM %s LIMIT %d", table_name, delete_count);

	ret = cosql_exec(pdb, "DELETE FROM %s"
						  " WHERE %s NOT IN"
						  " ("
						  " SELECT %s FROM %s ORDER BY %s DESC LIMIT %d"
						  " )",
						  table_name,
						  field_name,
						  field_name,
						  table_name,
						  field_name,
						  reserved_newest_data_count);

	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to remove data by count");
		return COSQL_ERROR;
	}

	ret = cosql_exec(pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_backup_and_remove_data_between_time(sqlite3* src_pdb, sqlite3* dst_pdb, const char* backup_data_columns, int start_data_time, int end_data_time) 
{
	if (src_pdb == NULL || dst_pdb == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;
	const char *dst_db_path = sqlite3_db_filename(dst_pdb, "main");

	// attach target database.
	ret = cosql_exec(src_pdb, "ATTACH '%s' AS target", dst_db_path);
	if( ret != COSQL_OK ) {
		return COSQL_ERROR;
	}

	// insert data from main database into target database.
	if (backup_data_columns==NULL || strlen(backup_data_columns)<=0) {
		ret = cosql_exec(src_pdb, "INSERT INTO target.%s SELECT * FROM main.%s WHERE data_time BETWEEN %d AND %d", 
			DATA_TABLE_NAME, DATA_TABLE_NAME, start_data_time, end_data_time);
	}
	else {
		ret = cosql_exec(src_pdb, "INSERT INTO target.%s(%s, data_time) SELECT %s, data_time FROM main.%s WHERE data_time BETWEEN %d AND %d", 
			DATA_TABLE_NAME, backup_data_columns, backup_data_columns, DATA_TABLE_NAME, start_data_time, end_data_time);
	}
	
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to insert data to target db %s.", dst_db_path);
		int ins_err = sqlite3_errcode(src_pdb);

		// dettach target database.
		ret = cosql_exec(src_pdb, "DETACH target");
		if( ret != COSQL_OK )
			codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);

		if (ins_err == SQLITE_FULL)
			return COSQL_DB_OR_DISK_FULL;
		else
			return COSQL_ERROR;
	}

	// dettach target database.
	ret = cosql_exec(src_pdb, "DETACH target");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);
		//return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "DELETE FROM %s WHERE data_time BETWEEN %d AND %d", DATA_TABLE_NAME, start_data_time, end_data_time);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to remove data between time.");
		return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to VACUUM database.");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_backup_and_remove_data_between_column_value(sqlite3* src_pdb, sqlite3* dst_pdb, 
	const char* column_name, int start_value, int end_value)
{
	if (src_pdb == NULL || dst_pdb == NULL || column_name == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;
	const char *dst_db_path = sqlite3_db_filename(dst_pdb, "main");

	// attach target database.
	ret = cosql_exec(src_pdb, "ATTACH '%s' AS target", dst_db_path);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to attach target db %s.", dst_db_path);
		return COSQL_ERROR;
	}

	// insert data from main database into target database.
	ret = cosql_exec(src_pdb, "INSERT INTO target.%s SELECT * FROM main.%s WHERE %s BETWEEN %d AND %d", 
		DATA_TABLE_NAME, DATA_TABLE_NAME, column_name, start_value, end_value);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to insert data to target db %s.", dst_db_path);
		int ins_err = sqlite3_errcode(src_pdb);

		// dettach target database.
		ret = cosql_exec(src_pdb, "DETACH target");
		if( ret != COSQL_OK )
			codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);

		if (ins_err == SQLITE_FULL)
			return COSQL_DB_OR_DISK_FULL;
		else
			return COSQL_ERROR;
	}

	// dettach target database.
	ret = cosql_exec(src_pdb, "DETACH target");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);
		//return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "DELETE FROM %s WHERE %s BETWEEN %d AND %d", DATA_TABLE_NAME, column_name, start_value, end_value);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to remove data between time");
		return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}

int cosql_backup_and_resize_table_by_reserved_count(sqlite3* src_pdb, sqlite3* dst_pdb, int reserved_newest_data_count) 
{
	if (src_pdb == NULL || dst_pdb == NULL) {
		return COSQL_ERROR;
	}

	int ret = 0;
	const char *dst_db_path = sqlite3_db_filename(dst_pdb, "main");

	// attach target database.
	ret = cosql_exec(src_pdb, "ATTACH '%s' AS target", dst_db_path);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to attach target db %s.", dst_db_path);
		return COSQL_ERROR;
	}

	// insert data from main database into target database.
	ret = cosql_exec(src_pdb, "INSERT INTO target.%s SELECT * FROM main.%s "
						  " WHERE data_id NOT IN"
						  " ("
						  " SELECT data_id FROM main.%s ORDER BY data_id DESC LIMIT %d"
						  " )",
							DATA_TABLE_NAME, 
							DATA_TABLE_NAME,
							DATA_TABLE_NAME,
							reserved_newest_data_count);
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to insert data to target db %s.", dst_db_path);
		int ins_err = sqlite3_errcode(src_pdb);

		// dettach target database.
		ret = cosql_exec(src_pdb, "DETACH target");
		if( ret != COSQL_OK )
			codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);

		if (ins_err == SQLITE_FULL)
			return COSQL_DB_OR_DISK_FULL;
		else
			return COSQL_ERROR;
	}

	// dettach target database.
	ret = cosql_exec(src_pdb, "DETACH target");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to dettach target db %s.", dst_db_path);
		//return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "DELETE FROM %s"
						  " WHERE data_id NOT IN"
						  " ("
						  " SELECT data_id FROM %s ORDER BY data_id DESC LIMIT %d"
						  " )",
						  DATA_TABLE_NAME,
						  DATA_TABLE_NAME,
						  reserved_newest_data_count);

	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to remove data by count");
		return COSQL_ERROR;
	}

	ret = cosql_exec(src_pdb, "VACUUM");
	if( ret != COSQL_OK ) {
		codbg(src_pdb, "fail to VACUUM database");
		return COSQL_ERROR;
	}

	return COSQL_OK;
}
////////////////////////////////////////////////////////////////////////////////////

int cosql_free_query_columns(sql_column_prototype_t* query_columns, int query_columns_count) {

	int i = 0;

	if (query_columns==NULL) {
		return COSQL_ERROR;
	}

	sql_column_prototype_t* query_columns_idx = query_columns;

	for (i=0; i<query_columns_count; i++) {

	   if (query_columns_idx->name!=NULL) {
	       free(query_columns_idx->name);
	   }

	   query_columns_idx++;
	}

	free(query_columns);
    
	return COSQL_OK;
}

int cosql_free_match_columns(sql_column_match_t* match_columns, int match_columns_count) {
	
	int i = 0;

	if (match_columns==NULL) {
		return COSQL_ERROR;
	}

	sql_column_match_t* match_columns_idx = match_columns;

	for (i=0; i<match_columns_count; i++) {

	   if (match_columns_idx->name!=NULL) {
	       free(match_columns_idx->name);
	   }

	   if ((match_columns_idx->type==COLUMN_TYPE_TEXT ||
	   	  match_columns_idx->type==COLUMN_TYPE_TEXT_MAC ||
	   	  match_columns_idx->type==COLUMN_TYPE_TEXT_IP ||
	   	  match_columns_idx->type==COLUMN_TYPE_TEXT_JSON) && match_columns_idx->value.t!=NULL) {
	       free(match_columns_idx->value.t);
	   }

	   match_columns_idx++;
	}

	free(match_columns);
    
   return COSQL_OK;
}
////////////////////////////////////////////////////////////////////////////////////
