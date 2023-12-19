#ifndef __DNSSQL_H__
#define __DNSSQL_H__

#include <sqlite3.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "dns.h"
#include "nfapp.h"
#include "nfdev.h"
#include "appstats.h"
#include "block_history.h"
#include "block_entry.h"
#include "log.h"

#define DEFAULT_APP_ID 0
#define DEFAULT_CAT_ID 0
#define DEFAULT_APP_NAME "others"
#define DEFAULT_APP_DOMAIN "unknown"
#define DEFAULT_APP_CATALOG "generic"
#define DEFAULT_MAC "11:22:33:44:55:66"

#define DEFAULT_IPV6_ADDR "fec0::1"

extern sqlite3 *sqlite_open(DNS_DB_TYPE db_type, sqlite3 *db, char *fname);
extern int sqlite_close(sqlite3 *db);
extern int sqlite_dns_insert(sqlite3 *db, dns_msg *msg, char *mac, int app_id);
extern int sqlite_dns_lookup(sqlite3 *db, char *domain, int *app_id, int *cat_id, char *app_name);
extern int sqlite_app_name_lookup(sqlite3 *db, int is_v4, char *mac, u_int32_t ipv4, char *ipv6, u_int16_t dst_port, int *app_id, int *cat_id, char *app_name);


extern int sqlite_dns_map_ip(sqlite3 *db, nfapp_node_t *ap);

extern int sqlite_nfcm_select(sqlite3 *db, char *sql, struct list_head *list);
extern int sqlite_free_result(char **result);

extern int sqlite_app_sum_aggregate(sqlite3 *db, int start, int end, struct list_head *list);
extern int sqlite_app_sum_insert(sqlite3 *db, struct list_head *list);

extern int sqlite_app_exclusive_statistics(sqlite3 *db, int start, int end, struct list_head *list);
extern int sqlite_app_exclusive_history_insert(sqlite3 *db, struct list_head *list);

extern int sqlite_dev_type_query(sqlite3 *db, struct list_head *list);

extern int sqlite_app_client_insert(sqlite3 *db, struct list_head *list);

extern int sqlite_task_status_insert_or_update(sqlite3 *db, char *table_name, int time, int status);
extern int sqlite_task_status_get_last(sqlite3 *db, char *name, int *status, int *time);

extern int sqlite_app_client_statistics(sqlite3 *db, int start, int end, struct list_head *list);


extern int sqlite_block_history_insert(sqlite3 *db, uint32_t client_ip, char *host, int timestamp);
extern int sqlite_block_query_statistics(sqlite3 *db, int start, int end, struct list_head *list);

extern int sqlite_block_list_has_name(sqlite3 *db, char *name, int *type);
extern int sqlite_block_list_insert_or_update(sqlite3 *db, char *host, int type);
extern int sqlite_block_list_all(sqlite3 *db, struct list_head *list);

extern int sqlite_integrity_check(sqlite3 *db, char *db_path);
extern void sqlite_remove_journal(char *db_file);

#define DNSSQL_OK 0
#define DNSSQL_ERROR 1

#define DNS_BLOCK_WHITE_LIST_FOUND 2
#define DNS_BLOCK_BLACK_LIST_FOUND 3
#define DNS_BLOCK_NOT_FOUND 4

#define DNS_DB_UPGRADE_FAIL 10

#define TASK_STATUS_TABLE "task_status"
#define APP_CLIENT_TABLE "app_client"
#define APP_SUM_TABLE "app_sum"
#define APP_NAME_TABLE "app_name"
#define APP_DOMAIN_TABLE "app_domain"
#define DNS_QUERY_TABLE "dns_query"
#define DNS_BLOCK_LIST  "block_list"
#define DNS_BLOCK_HISTORY  "block_history"
#define EXCLUSIVE_DOMAIN_TABLE  "exclusive_domain"
#define EXCLUSIVE_HISTORY_TABLE  "exclusive_history"

#define DB_VER_TABLE "db_ver"
#define DB_NAME_COLUMN_VALUE "ver"

#define DNS_QUERY_DB_VER 3
#define APP_CLIENT_DB_VER 2
#define APP_SUM_DB_VER 2


#define NFCM_APP_TABLE "DATA_INFO"

#define DNS_QUERY_DB_FILE "/jffs/dns_query.db"
#define NFCM_APP_DB_FILE "/jffs/nfcm_app.db"
#define APP_CLIENT_DB_FILE "/jffs/app_client.db"
#define APP_SUM_DB_FILE "/jffs/app_sum.db"

#define JSON_DEVICE_TYPE_QUERY_FILE "/jffs/dev_type_query.json"
#define JSON_OUTPUT_APP_FILE "/tmp/appstats.json"
#define JSON_OUTPUT_BLOCK_HISTORY_FILE "/tmp/block_history.json"
#define JSON_OUTPUT_BLOCK_ENTRY_FILE "/tmp/block_entry.json"

#define JSON_BW_MON_TRAFFIC_FILE "/tmp/bw_mon_trf.json"

#define APP_CLIENT_DB_LOCK_NAME "app_client"
#define APP_SUM_DB_LOCK_NAME "app_sum"
#define DNS_QUERY_DB_LOCK_NAME "dns_query"
#define NFCM_APP_DB_LOCK_NAME "nfcm_app"
//#define DNS_MAC_LIST_LOCK_NAME "dns_mac_list"
#define DNS_BW_MON_TRAF_LOCK_NAME "dns_bw_mon_trf"


#define DNSQD_PID_FILE "/var/run/dnsqd.pid"

#define BULK_INSERT_SIZE 40

#endif /* __DNSSQL_H__ */
