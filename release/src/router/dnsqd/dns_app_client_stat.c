#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/stat.h> 
#include <fcntl.h>
#include <stdarg.h>

#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "dns_app_client.h"
#include "dnssql.h"
#include "log.h"

#define QUERY_LEN 960
#define LEN_MAX   12000
#define HOURSEC 3600
#define DAY_SEC 86400
#define MON_SEC 86400 * 31
#define DNS_APP_CLIENT_DB "/jffs/app_client.db"
#define DNS_APP_SUM_DB "/jffs/app_sum.db"

//#define DEBUG_SQL 1
#if 0 
#define safe_atoi atoi
#define strlcpy strncpy
int websWrite(int wp, const char *format, ...)
{
	//FILE *f;
	//int nfd;
	va_list args;
  int n;
  //nfd = open(stdout, O_WRONLY);
 	//if(nfd >= 0) {
      // fdopen will associate nfd and f, so just close f is fine
	
  //    if((f = fdopen(nfd, "w")) != NULL) {
	
        va_start(args, format);
  
				n = vprintf(format, args);
  			va_end(args);
	//			fclose(f);
	//		} else
  //			close(nfd);
  //}
  return n;
}

#endif 
//traffic analyze api align with bwdpi
int sql_get_table(sqlite3 *db, const char *sql, char ***pazResult, int *pnRow, int *pnColumn)
{
	int ret;
	char *errMsg = NULL;
	
	ret = sqlite3_get_table(db, sql, pazResult, pnRow, pnColumn, &errMsg);
	if (ret != SQLITE_OK)
	{
		if (errMsg) sqlite3_free(errMsg);
	}

	return ret;
}

/* ---- Traffic Analyzer START ---- */
void dns_appStat(char *buff, char *group, char *where, char *having, int len)
{
	int lock; // file lock
	int ret;
	char *db_path = NULL;
	sqlite3 *db = NULL;
	int rows;
	int cols;
	long long int tx = 0, rx = 0;
	int first_row = 1;
	char **result;
	char sql_query[QUERY_LEN];
	char select[64];
	char mac[18];
	char app_name[40];
	char buff_t[LEN_MAX];

	memset(select, 0, sizeof(select));
	memset(sql_query, 0, sizeof(sql_query));
	memset(mac, 0, sizeof(mac));
	memset(app_name, 0, sizeof(app_name));
	memset(buff_t, 0, sizeof(buff_t));

	db_path = DNS_APP_SUM_DB;

#if !defined(X86_APP)
	lock = file_lock("app_sum");
#endif
	ret = sqlite3_open(db_path, &db);

	if (ret) {
		printf("Can't open database %s\n", sqlite3_errmsg(db));
		if (db != NULL) sqlite3_close(db);
#if !defined(X86_APP)    
		file_unlock(lock);
#endif    
		return;
	}

	// initial SELECT string
	snprintf(select, sizeof(select), "mac, app_name, timestamp, SUM(tx), SUM(rx)");
 	// append WHERE / GROUP / HAVING string into query string
	if (group != NULL) {
		if (having == NULL && !strcmp(where, ""))
			snprintf(sql_query, sizeof(sql_query), "SELECT %s FROM app_sum GROUP BY %s", select, group);
		else if (having != NULL && !strcmp(where, ""))
			snprintf(sql_query, sizeof(sql_query), "SELECT %s FROM %s GROUP BY %s", select, having, group);
		else if (having != NULL && strcmp(where, ""))
			snprintf(sql_query, sizeof(sql_query), "SELECT %s FROM %s WHERE %s GROUP BY %s", select, having, where, group);
	}
	else {
		if (db != NULL) sqlite3_close(db);
#if !defined(X86_APP)  
		file_unlock(lock);
#endif    
		return;
	}

#if DEBUG_SQL	
	dnsdbg("sql_query = %s", sql_query);
#endif
	if (sql_get_table(db, sql_query, &result, &rows, &cols) == SQLITE_OK)
	{
#if DEBUG_SQL	
		dnsdbg("rows=%d, cols=%d", rows, cols);
#endif
		int i = 0;
		int j = 0;
		int index = cols;

		if (!strcmp(group, "timestamp"))
		{
			for (i = 0; i < rows; i++) {
				for (j = 0; j < cols; j++) {
#if DEBUG_SQL	          
					dnsdbg("[%d/%d] result: %s/%s", i, j, result[j], result[index]);
#endif          
					if (j == 3) tx += atoll(result[index]);
					if (j == 4) rx += atoll(result[index]);
					++index;
				}
			}
			snprintf(buff, len, "[%llu, %llu]", tx, rx);
		}
		else if (!strcmp(group, "mac"))
		{
			for (i = 0; i < rows; i++) {
				for (j = 0; j < cols; j++) {
#if DEBUG_SQL	          
					dnsdbg("[%d/%d] result: %s/%s", i, j, result[j], result[index]);
#endif          
					if (j == 0) strlcpy(mac, result[index], sizeof(mac));
					if (j == 3) tx = atoll(result[index]);
					if (j == 4) rx = atoll(result[index]);
					++index;
				}

				if (first_row) {
					first_row = 0;
					snprintf(buff, len, "[\"%s\", %llu, %llu]", mac, tx, rx);
				}
				else {
					snprintf(buff_t, len, "%s", buff);
					snprintf(buff, len, "%s, [\"%s\", %llu, %llu]", buff_t, mac, tx, rx);
				}
#if DEBUG_SQL	        
				dnsdbg("[sqlite] buff = %s", buff);
#endif        
			}
		}
		else if(!strcmp(group, "app_name"))
		{
			for (i = 0; i < rows; i++) {
				for (j = 0; j < cols; j++) {
#if DEBUG_SQL	          
					dnsdbg("[%d/%d] result: %s/%s", i, j, result[j], result[index]);
#endif          
					if (j == 1) strlcpy(app_name, result[index], sizeof(app_name));
					if (j == 3) tx = atoll(result[index]);
					if (j == 4) rx = atoll(result[index]);
					++index;
				}

				if(first_row) {
					first_row = 0;
					snprintf(buff, len, "[\"%s\", %llu, %llu]", app_name, tx, rx);
				}
				else {
					snprintf(buff_t, len, "%s", buff);
					snprintf(buff, len, "%s, [\"%s\", %llu, %llu]", buff_t, app_name, tx, rx);
				}
#if DEBUG_SQL	      
				dnsdbg("[sqlite] buff = %s", buff);
#endif      
			}
		}
#if DEBUG_SQL	
		dnsdbg("[sqlite] buff = %s", buff);
#endif		
		sqlite3_free_table(result);
	}
	if (db != NULL) sqlite3_close(db);
#if !defined(X86_APP)
	file_unlock(lock);
#endif  
}

/*
	type : 
		0 : app, 1 : mac
	client :
		all , macaddr=XX:XX:XX:XX:XX:XX, app_name=XXXX
	mode :
		day , hour , detail
	dura :
		7 / 24 / 31
	date :
		timestamp
*/
void dns_sqlite_Stat_hook(int type, char *client, char *mode, char *dura, char *date, int *retval, webs_t wp)
{
	int date_min = 0, date_max = 0, date_init = 0;
	int i = 0;
	int first_row = 1;
	char group[64];
	char having[256];
	char where[64];
	char buff[LEN_MAX]; //group =  mac / app_name, maybe buff isn't enough, keep observing...
	int dura_int = 0;

#if DEBUG_SQL	
	dnsdbg("type=%d, client=%s, mode=%s, dura=%s, date=%s", type, client, mode, dura, date);
#endif
	date_init = safe_atoi(date) + 30; // allow 30 secs delay to collect right data in time interval
	date_init = date_init - (date_init % 3600) -1 ; // get the nearest 1-hour
	memset(group, 0, sizeof(group));
	memset(having, 0, sizeof(having));
	memset(where, 0, sizeof(where));
	memset(buff, 0, sizeof(buff));
	
	// client = all, mode != detail, type = 0 or 1
	if (!strcmp(client, "all") && strcmp(mode, "detail"))
	{
		snprintf(group, sizeof(group), "timestamp");
	}
	// client = all, mode = detail, type = 0
	else if (!strcmp(client, "all") && !strcmp(mode, "detail") && type == 0)
	{
		snprintf(group, sizeof(group), "mac");
	}
	// client = all, mode = detail, type = 1
	else if (!strcmp(client, "all") && !strcmp(mode, "detail") && type == 1)
	{
		snprintf(group, sizeof(group), "app_name");
	}
	// client != all, mode != detail, type = 0
	else if (strcmp(client, "all") && strcmp(mode, "detail") && type == 0)
	{
		snprintf(group, sizeof(group), "timestamp");
		snprintf(where, sizeof(where), "app_name=\"%s\"", client);
	}
	// client != all, mode != detail, type = 1
	else if (strcmp(client, "all") && strcmp(mode, "detail") && type == 1)
	{
		snprintf(group, sizeof(group), "timestamp");
		snprintf(where, sizeof(where), "mac=\"%s\"", client);
	}
	// client != all, mode != detail, type = 2
	else if (strcmp(client, "all") && strcmp(mode, "detail") && type == 2)
	{
		snprintf(group, sizeof(group), "app_name");
		snprintf(where, sizeof(where), "mac=\"%s\"", client);
	}
	// client != all, mode = detail, type = 0
	else if (strcmp(client, "all") && !strcmp(mode, "detail") && type == 0)
	{
		snprintf(group, sizeof(group), "mac");
		snprintf(where, sizeof(where), "app_name=\"%s\"", client);
	}
	// client != all, mode = detail, type = 1
	else if (strcmp(client, "all") && !strcmp(mode, "detail") && type == 1)
	{
		snprintf(group, sizeof(group), "app_name");
		snprintf(where, sizeof(where), "mac=\"%s\"", client);
	}
	else {
		printf("[sqlite] no such case!\n");
	}

	if (!strcmp(mode, "hour") && !strcmp(dura, "24"))
	{// day
		*retval += websWrite(wp, "[");
		for (i = 0; i < 24; i++) {
			memset(buff, 0, sizeof(buff));
			date_min = date_init - HOURSEC * (24 -i);
			date_max = date_init - HOURSEC * (23 -i);
			snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
			dns_appStat(buff, group, where, having, sizeof(buff));
			if (first_row) {
				first_row = 0;
				*retval += websWrite(wp, buff);
			}
			else
				*retval += websWrite(wp, ", %s", buff);
				
		}
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "hour") && type == 2)
	{// 8 hours
		dura_int = atoi(dura);
		if (dura_int < 0 || dura_int >8)
			dura_int = 1;

		*retval += websWrite(wp, "[");
		for (i = 0; i < dura_int; i++) {
			memset(buff, 0, sizeof(buff));
			date_min = date_init - HOURSEC * (dura_int -i);
			date_max = date_init - HOURSEC * ((dura_int -1)-i);
			snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
			dns_appStat(buff, group, where, having, sizeof(buff));
			if (first_row) {
				first_row = 0;
				*retval += websWrite(wp, "[%s]", buff);
			}
			else
				*retval += websWrite(wp, ", [%s]", buff);
		}
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "day") && !strcmp(dura, "7"))
	{// week
		*retval += websWrite(wp, "[");
		for (i = 0; i < 7; i++) {
			memset(buff, 0, sizeof(buff));
			date_min = date_init - DAY_SEC * (7 - i);
			date_max = date_init - DAY_SEC * (6 - i);
			snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
			dns_appStat(buff, group, where, having, sizeof(buff));
			if (first_row) {
				first_row = 0;
				*retval += websWrite(wp, buff);
			}
			else
				*retval += websWrite(wp, ", %s", buff);
				
		}
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "day") && !strcmp(dura, "31"))
	{// month
		*retval += websWrite(wp, "[");
		for (i = 0; i < 31; i++) {
			memset(buff, 0, sizeof(buff));
			date_min = date_init - DAY_SEC * (31 - i);
			date_max = date_init - DAY_SEC * (30 - i);
			snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
			dns_appStat(buff, group, where, having, sizeof(buff));
			if (first_row) {
				first_row = 0;
				*retval += websWrite(wp, buff);
			}
			else
				*retval += websWrite(wp, ", %s", buff);
				
		}
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "detail") && !strcmp(dura, "24"))
	{// detail for daily
		*retval += websWrite(wp, "[");
		date_min = date_init - DAY_SEC;
		date_max = date_init;
		snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
		dns_appStat(buff, group, where, having, sizeof(buff));
		*retval += websWrite(wp, buff);
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "detail") && !strcmp(dura, "7"))
	{// detail for weekly
		*retval += websWrite(wp, "[");
		date_min = date_init - DAY_SEC * 7;
		date_max = date_init;
		snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
		dns_appStat(buff, group, where, having, sizeof(buff));
		*retval += websWrite(wp, buff);
		*retval += websWrite(wp, "]");
	}
	else if (!strcmp(mode, "detail") && !strcmp(dura, "31"))
	{// detail for monthly
		*retval += websWrite(wp, "[");
		date_min = date_init - MON_SEC;
		date_max = date_init;
		snprintf(having, sizeof(having), "(SELECT * FROM app_sum WHERE timestamp BETWEEN '%d' AND '%d')", date_min, date_max);
		dns_appStat(buff, group, where, having, sizeof(buff));
		*retval += websWrite(wp, buff);
		*retval += websWrite(wp, "]");
	}
}
/* ---- Traffic Analyzer END ---- */

void delete_dns_app_sum_records()
{

	int lock; // file lock
	int ret;
	char *db_path = NULL;
	char *err = NULL;
	sqlite3 *db = NULL;
	char sql[128];


	db_path = DNS_APP_SUM_DB;

	lock = file_lock("app_sum");
	ret = sqlite3_open(db_path, &db);

	if (ret) {
		printf("Can't open database %s\n", sqlite3_errmsg(db));
		if (db != NULL) sqlite3_close(db);
		file_unlock(lock);
		return;
	}

       sprintf(sql, "delete from app_sum;");

       ret = sqlite3_exec(db, sql, NULL, NULL, &err);
       if (ret != SQLITE_OK) {
          if(err != NULL) {
             sqlite3_free(err);
         }
       }  
       
       sqlite3_close(db);
       file_unlock(lock);
       return;
}
