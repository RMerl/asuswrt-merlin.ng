#ifndef __DNS_APP_CLIENT_H__
#define ___DNS_APP_CLIENT_H__

#include <json.h>
#include <httpd.h>
#include <shutils.h>

#define ORDER_BY_APP 0
#define ORDER_BY_CLIENT 1
#define ORDER_BY_UPLOAD 0
#define ORDER_BY_DOWNLOAD 1

#define DNS_APP_CLIENT_DB_OK 0
#define DNS_APP_CLIENT_DB_FAIL -1

#define IN_BLACK_LIST 0
#define IN_WHITE_LIST 1

extern int dns_app_stats_json(unsigned long start, unsigned long end, int app_or_cli, int up_or_dn, json_object **retJsonObj);

//block history
extern int dns_block_stats_json(unsigned long start, unsigned long end, json_object **retJsonObj);

//block list management
extern int dns_block_list_json(json_object **retJsonObj);
extern int dns_block_list_insert_or_update(char *host, int type);

//traffic analyze api align with bwdpi
extern void dns_sqlite_Stat_hook(int type, char *client, char *mode, char *dura, char *date, int *retval, webs_t wp);

extern void delete_dns_app_sum_records();
#endif
