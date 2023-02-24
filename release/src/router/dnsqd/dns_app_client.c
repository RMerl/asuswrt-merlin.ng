#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "dns_app_client.h"
#include "dnssql.h"
#include "log.h"

int dns_app_stats_json(unsigned long start, unsigned long end, int app_or_cli, int up_or_dn, json_object **retJsonObj)
{
  int ret;
  sqlite3 *db = NULL;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  struct json_object *json_obj;
  struct json_object *ary_obj = json_object_new_array();
  // debug parameters
  dnsdbg("s:%ld e:%ld app_or_cli:%d up_or_dn:%d !!", start, end, app_or_cli, up_or_dn);
  
  ret = sqlite3_open(APP_CLIENT_DB_FILE, &db);
  if (ret != SQLITE_OK)
    return DNS_APP_CLIENT_DB_FAIL;
  // run query
  //appstats_node_t *ap;

  sprintf(sql, "select is_v4, client_ip, client_ip6, name, SUM(up_dif_bytes) as up, SUM(dn_dif_bytes) as dn, mac "\
  "from app_client where timestamp between %ld and %ld group by is_v4, client_ip, mac, name "\
  " order by %s, %s DESC;",
  start, end, app_or_cli == ORDER_BY_APP ? "name":"client_ip",
  up_or_dn == ORDER_BY_UPLOAD ? "up":"dn");
  
  //dnsdbg("dns_app_client sql:%s", sql);

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
      sqlite3_close(db);
    }
    return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  { 
    char *stop;
 
    json_obj = json_object_new_object();
    if (json_obj == NULL)  {
        dnsdbg("new json object failed.");
        sqlite3_free_table(result);
        sqlite3_close(db);
        return DNSSQL_ERROR;
    }
    json_object_object_add(json_obj, "is_v4", json_object_new_int(atoi(result[index++])));
    json_object_object_add(json_obj, "client_ip", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "client_ip6", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "host", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "up_bytes", json_object_new_int64(strtoull(result[index++], &stop, 10)));
    json_object_object_add(json_obj, "dn_bytes", json_object_new_int64(strtoull(result[index++], &stop, 10)));
    json_object_object_add(json_obj, "mac", json_object_new_string(result[index++]));
    json_object_array_add(ary_obj, json_obj);
  }
  sqlite3_free_table(result);
  if(db)
    sqlite3_close(db);

  json_object *rootObj = json_object_new_object();
  json_object_object_add(rootObj, "contents", ary_obj);
  *retJsonObj = rootObj;

  return DNSSQL_OK;
}

//block history
int dns_block_stats_json(unsigned long start, unsigned long end, json_object **retJsonObj)
{
  int ret;
  sqlite3 *app_client_db = NULL;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  struct json_object *json_obj;
  struct json_object *ary_obj = json_object_new_array();
  // debug parameters
  
  ret = sqlite3_open(APP_CLIENT_DB_FILE, &app_client_db);
  if (ret != SQLITE_OK)
    return DNS_APP_CLIENT_DB_FAIL;
  // run query
  
  sprintf(sql, "select is_v4, client_ip, client_ip6, name, timestamp "\
  "from block_history where timestamp between %ld and %ld "\
  "order by timestamp desc;",
  start, end);
  
  ret = sqlite3_get_table(app_client_db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
      sqlite3_close(app_client_db);
    }
    return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  { 
    char *stop;

    json_obj = json_object_new_object();
    if (json_obj == NULL)  {
        dnsdbg("new json object failed.");
        sqlite3_free_table(result);
        sqlite3_close(app_client_db);
        return DNSSQL_ERROR;
    }
   
    json_object_object_add(json_obj, "is_v4", json_object_new_int(atoi(result[index++])));
    json_object_object_add(json_obj, "client_ip", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "client_ip6", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "host", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "timestamp", json_object_new_int(strtoul(result[index++], &stop, 10)));
    json_object_array_add(ary_obj, json_obj);
  }
  sqlite3_free_table(result);
  if(app_client_db)
    sqlite3_close(app_client_db);

  json_object *rootObj = json_object_new_object();
  json_object_object_add(rootObj, "contents", ary_obj);
  *retJsonObj = rootObj;

  return DNSSQL_OK;
}

int dns_block_list_json(json_object **retJsonObj)
{
  int ret;
  sqlite3 *app_client_db = NULL;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  struct json_object *json_obj;
  struct json_object *ary_obj = json_object_new_array();
  // debug parameters
  
  ret = sqlite3_open(APP_CLIENT_DB_FILE, &app_client_db);
  if (ret != SQLITE_OK)
    return DNS_APP_CLIENT_DB_FAIL;
  // run query
  
  sprintf(sql, "select name, type from block_list ;");
  
  ret = sqlite3_get_table(app_client_db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
      sqlite3_close(app_client_db);
    }
    return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  { 
    char *stop;
    json_obj = json_object_new_object();
    if (json_obj == NULL)  {
        dnsdbg("new json object failed.");
        sqlite3_free_table(result);
        sqlite3_close(app_client_db);
        return DNSSQL_ERROR;
    }
    json_object_object_add(json_obj, "host", json_object_new_string(result[index++]));
    json_object_object_add(json_obj, "type", json_object_new_int(strtoul(result[index++], &stop, 10)));
    json_object_array_add(ary_obj, json_obj);
  }
  sqlite3_free_table(result);
  if(app_client_db)
    sqlite3_close(app_client_db);

  json_object *rootObj = json_object_new_object();
  json_object_object_add(rootObj, "contents", ary_obj);
  *retJsonObj = rootObj;

  return DNSSQL_OK;
}

int api_block_list_has_name(sqlite3 *db, char *name, int *type)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char block_host[128] = {0};
  char sql[512] = {0};
  sprintf(sql, "select name, type from %s where name='%s';",
  DNS_BLOCK_LIST, name);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  { 
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    return DNSSQL_ERROR;
  }
  
  if(row == 0)
  { 
    return DNS_BLOCK_NOT_FOUND;
  }

  index = col;
  for(int i=0; i<row; i++)
  { 
    strcpy(block_host, result[index++]);
    *type = atoi(result[index++]);
    // dnsdbg("[%s]-[%d]", block_host, *type);
  }
 
  // free here 
  sqlite3_free_table(result);
  if(*type == NAME_IN_BLACK_LIST )
    return DNS_BLOCK_BLACK_LIST_FOUND;
  else
    return DNS_BLOCK_WHITE_LIST_FOUND;
}


int api_block_list_insert_or_update(sqlite3 *db, char *host, int type)
{
  int ret;
  int old_type = 0;
  int is_update = 0;
  char sql[512] = {0};
  char *err = NULL;
  ret = api_block_list_has_name(db, host, &old_type);
  //dnsdbg("type:%d o_type: %d", type, old_type);
  if (ret == DNS_BLOCK_NOT_FOUND || ret == DNSSQL_ERROR)
  { 
    sprintf(sql, "insert into %s (name, type) values ('%s', %d);",
    DNS_BLOCK_LIST, host, type);
  } else if (ret == DNS_BLOCK_WHITE_LIST_FOUND || ret == DNS_BLOCK_BLACK_LIST_FOUND)
  {
    sprintf(sql, "update %s set type=%d where name='%s';",
    DNS_BLOCK_LIST, type, host);
    is_update = 1;
  }
  //dnsdbg("run sql=%s", sql);
  if (is_update && type == old_type)
  {
    // no need update
    return DNSSQL_OK;
  }
  ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert or update fail %s", err);
          sqlite3_free(err);
        }
  }  
  return ret;
}


int dns_block_list_insert_or_update(char *host, int type)
{
  int ret;
  sqlite3 *app_client_db = NULL;
  
  ret = sqlite3_open(APP_CLIENT_DB_FILE, &app_client_db);
  if (ret != SQLITE_OK)
    return DNS_APP_CLIENT_DB_FAIL;

  ret = api_block_list_insert_or_update(app_client_db, host, type);
  sqlite3_close(app_client_db);
  return ret;
}
