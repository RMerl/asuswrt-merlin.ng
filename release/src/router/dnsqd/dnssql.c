#if !defined(X86_APP)
#include <shutils.h>
#endif
#include "dnssql.h"
#include "app_domain_list.h"
#include "app_protocols_list.h"

void init_default_app_domain_table (sqlite3 *db) {
  int ret;
  char *err = NULL;
  char sql[512];
  unsigned int resolve_ip;
  
  int app_list_len = sizeof(app_list)/sizeof(app_list[0]);
  int app_domain_len = sizeof(domain_list)/sizeof(domain_list[0]);
  
  for (int i = 0 ; i< app_list_len; i++) 
  { 
    sprintf(sql, "REPLACE INTO app_name (app_id, name, catalog, map_cat_id, map_app_id) VALUES (%u, '%s', '%s', %u, %u)",
            app_list[i].app_id, app_list[i].name, app_list[i].catalog,
            app_list[i].map_cat_id,
            app_list[i].map_app_id);
    ret = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (ret != SQLITE_OK) {
      if(err != NULL) {
        sqlite3_free(err);
      }
    }  
  }

  for (int i = 0 ; i< app_domain_len; i++) 
  { 
    sprintf(sql, "REPLACE INTO app_domain (app_id, domain) VALUES (%u, '%s')",
            domain_list[i].app_id, domain_list[i].domain);
    ret = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (ret != SQLITE_OK) {
      if(err != NULL) {
        sqlite3_free(err);
      }
    }  
  }
}

void init_default_pattern_table (sqlite3 *db) {
  int ret;
  char *err = NULL;
  char sql[512];
  unsigned int resolve_ip;
  
  int pattern_len = sizeof(dev_domain_list)/sizeof(dev_domain_list[0]);
  
  for (int i = 0 ; i< pattern_len; i++) 
  { 
    sprintf(sql, "INSERT INTO exclusive_domain (type, domain, tx, rx) VALUES (%u, '%s', %llu, %llu)",
            dev_domain_list[i].dev_id, dev_domain_list[i].domain, dev_domain_list[i].tx, dev_domain_list[i].rx);
    ret = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (ret != SQLITE_OK) {
      if(err != NULL) {
        //dnsdbg("sql err: %s\n", err);
        sqlite3_free(err);
      }
    }  
  }
}

void get_date_time(char* buff, int len)
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buff, len, "%Y-%m-%d %H:%M:%S", timeinfo);
}


int has_ver_table(sqlite3 *db)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  int count = 0;
  char sql[128] ={0};
 
  sprintf(sql, "SELECT COUNT(*) AS count FROM sqlite_master WHERE type='table' AND name='%s'", DB_VER_TABLE);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err)
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }

    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
  index = col;
  
  for(int i=0; i<row; i++)
  {
    count = atoi(result[index++]);
  }

  sqlite3_free_table(result);

  return count;
}

int get_db_ver(sqlite3 *db)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  int ver = 0;
  char sql[128] ={0};
  char name[30] = {0};

  sprintf(sql, "SELECT name, ver FROM %s", DB_VER_TABLE);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err)
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
  index = col;
  
  for(int i=0; i<row; i++)
  {
    strcpy(name, result[index++]);
    ver = atoi(result[index++]);
  }

  sqlite3_free_table(result);

  return ver;
}


int set_db_ver(sqlite3 *db, int new_ver)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  int count = 0;
  char sql[128] ={0};
 
  sprintf(sql, "REPLACE INTO %s (name, ver) VALUES ('%s', %u)", DB_VER_TABLE, DB_NAME_COLUMN_VALUE, new_ver);

  ret = sqlite3_exec(db, sql, NULL, NULL, &err);
  if(ret != SQLITE_OK)
  {
    if(err)
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    return DNSSQL_ERROR;
  }

  return SQLITE_OK;

}

int drop_table(char *table_name, sqlite3 *db)
{
   int ret;
   char *err;
   char sql[128] = {0};

   sprintf(sql, "DROP TABLE %s", table_name);

   ret = sqlite3_exec(db, sql, NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
         dnsdbg("drop db error %s", err);
         sqlite3_free(err);
   }
  
   return ret;
}


int create_ver_table(sqlite3 *db)
{
   int ret;
   char *err;
   char sql[128] = {0};

   sprintf(sql, 
          "CREATE TABLE IF NOT EXISTS %s ("
          "name VARCHAR(20) NOT NULL,"
	  "ver INTEGER NOT NULL,"
          "CONSTRAINT unq UNIQUE (name))",
          DB_VER_TABLE);
   ret = sqlite3_exec(db, sql, NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
         dnsdbg("create db error %s", err);
         sqlite3_free(err);
   }
  
   return ret;
}

int upgrade_dq_to_ver_1(sqlite3 *db)
{
  int ret;
  char *err;
  // app_name
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_name ("
          "app_id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "name VARCHAR(128) NOT NULL,"
          "catalog VARCHAR(128) NOT NULL )",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
   }
  
  // app_domain
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_domain ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "domain VARCHAR(128) NOT NULL,"
          "app_id INTEGER NOT NULL,"
          "CONSTRAINT unq UNIQUE (domain),"
          "FOREIGN KEY (app_id) REFERENCES app_name (app_id))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }

  // dns_query
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS dns_query ("
          "client_ip UINT NOT NULL,"
          "name VARCHAR(128) NOT NULL,"
          "resolve_ip UINT NOT NULL,"
          "mac VARCHAR(128) NOT NULL,"
          "app_id INTEGER,"
          "timestamp INTEGER,"
          "CONSTRAINT unq UNIQUE (client_ip, name, resolve_ip))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }
 
  // index
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS src_dstx_app ON dns_query (client_ip, resolve_ip, app_id)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("dns_query create index error: %s\n", err);
            sqlite3_free(err);
  }

  init_default_app_domain_table(db);
  set_db_ver(db, 1);
  
  return 0;
}


int upgrade_dq_to_ver_2(sqlite3 *db)
{
  int ret;
  char *err;

  // too many NOT NULL new field, drop table instead alter table
  drop_table(DNS_QUERY_TABLE, db);
  drop_table(APP_DOMAIN_TABLE, db);
  drop_table(APP_NAME_TABLE, db);

  // app_name
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_name ("
          "app_id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "name VARCHAR(128) NOT NULL,"
          "catalog VARCHAR(128) NOT NULL )",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
   }
  
  // app_domain
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_domain ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "domain VARCHAR(128) NOT NULL,"
          "app_id INTEGER NOT NULL,"
          "CONSTRAINT unq UNIQUE (domain),"
          "FOREIGN KEY (app_id) REFERENCES app_name (app_id))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }

  // dns_query
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS dns_query ("
          "is_v4 UINT NOT NULL,"
          "client_ip UINT NOT NULL,"
          "client_ip6 VARCHAR(50) NOT NULL,"
          "name VARCHAR(128) NOT NULL,"
          "resolve_ip UINT NOT NULL,"
          "resolve_ip6 VARCHAR(50) NOT NULL,"
          "mac VARCHAR(128) NOT NULL,"
          "app_id INTEGER,"
          "timestamp INTEGER,"
          "CONSTRAINT unq UNIQUE (client_ip, client_ip6, name, resolve_ip, resolve_ip6))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }
 
  // index
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS src_dstx_app ON dns_query (client_ip, resolve_ip, app_id)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("dns_query create index error: %s\n", err);
            sqlite3_free(err);
  }

  init_default_app_domain_table(db);
  set_db_ver(db, 2);
  
  return 0;
}

int upgrade_dq_to_ver_3(sqlite3 *db)
{
  int ret;
  char *err;

  // too many NOT NULL new field, drop table instead alter table
  drop_table(DNS_QUERY_TABLE, db);
  drop_table(APP_DOMAIN_TABLE, db);
  drop_table(APP_NAME_TABLE, db);

  // app_name
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_name ("
          "app_id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "name VARCHAR(128) NOT NULL,"
          "catalog VARCHAR(128) NOT NULL,"
          "map_cat_id INTEGER NOT NULL,"
          "map_app_id INTEGER NOT NULL )",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
   }
  
  // app_domain
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS app_domain ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "domain VARCHAR(128) NOT NULL,"
          "app_id INTEGER NOT NULL,"
          "CONSTRAINT unq UNIQUE (domain),"
          "FOREIGN KEY (app_id) REFERENCES app_name (app_id))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }

  // dns_query
  ret = sqlite3_exec(db,
          "CREATE TABLE IF NOT EXISTS dns_query ("
          "is_v4 UINT NOT NULL,"
          "client_ip UINT NOT NULL,"
          "client_ip6 VARCHAR(50) NOT NULL,"
          "name VARCHAR(128) NOT NULL,"
          "resolve_ip UINT NOT NULL,"
          "resolve_ip6 VARCHAR(50) NOT NULL,"
          "mac VARCHAR(128) NOT NULL,"
          "app_id INTEGER,"
          "timestamp INTEGER,"
          "CONSTRAINT unq UNIQUE (client_ip, client_ip6, name, resolve_ip, resolve_ip6))",
          NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
            dnsdbg("create db error %s", err);
            sqlite3_free(err);
  }
 
  // index
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS src_dstx_app ON dns_query (client_ip, resolve_ip, app_id)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("dns_query create index error: %s\n", err);
            sqlite3_free(err);
  }

  init_default_app_domain_table(db);
  set_db_ver(db, 3);
  
  return 0;
}


int upgrade_ac_to_ver_1(sqlite3 *db)
{
  int ret;
  char *err;
  
  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS app_client ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "data_id INTEGER,"
        "client_ip UINT NOT NULL,"
        "name VARCHAR(128) NOT NULL,"
        "resolve_ip UINT NOT NULL,"
        "src_port UNSIGNED INTEGER NOT NULL,"
        "dst_port UNSIGNED INTEGER NOT NULL,"
        "up_bytes UNSIGNED BIG INT,"
        "dn_bytes UNSIGNED BIG INT,"
        "up_dif_bytes UNSIGNED BIG INT,"
        "dn_dif_bytes UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "app_name VARCHAR(128)," 
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }
     
   ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS task_status ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "table_name VARCHAR(128) NOT NULL,"
        "status INT DEFAULT 0,"
        "last_update INTEGER NOT NULL,"
        "CONSTRAINT unq UNIQUE (table_name, last_update))",
        NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

   ret = sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS block_list ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name VARCHAR(128) NOT NULL,"
      "type INT DEFAULT 0,"             // 0: black list 1: white list
      "CONSTRAINT unq UNIQUE (name))",
      NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

   ret = sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS block_history ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "client_ip UINT NOT NULL,"
      "name VARCHAR(128) NOT NULL,"
      "timestamp INTEGER NOT NULL,"             // 0: black list 1: white list
      "CONSTRAINT unq UNIQUE (client_ip, timestamp))",
      NULL, NULL, &err);
      if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
    }
 
  // index
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS ip_mac_app ON app_client (client_ip, mac, app_name)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_client create index error: %s\n", err);
            sqlite3_free(err);
   }

  set_db_ver(db, 1);
  
  return 0;
}


int upgrade_ac_to_ver_2(sqlite3 *db)
{
  int ret;
  char *err;

  // too many NOT NULL new field, drop table instead alter table
  drop_table(APP_CLIENT_TABLE, db);
  drop_table(DNS_BLOCK_LIST, db);
  drop_table(DNS_BLOCK_HISTORY, db);
  drop_table(TASK_STATUS_TABLE, db);

  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS app_client ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "data_id INTEGER,"
        "is_v4 UINT NOT NULL,"
        "client_ip UINT NOT NULL,"
        "client_ip6 VARCHAR(50) NOT NULL,"
        "name VARCHAR(128) NOT NULL,"
        "resolve_ip UINT NOT NULL,"
        "resolve_ip6 VARCHAR(50) NOT NULL,"
        "src_port UNSIGNED INTEGER NOT NULL,"
        "dst_port UNSIGNED INTEGER NOT NULL,"
        "up_bytes UNSIGNED BIG INT,"
        "dn_bytes UNSIGNED BIG INT,"
        "up_dif_bytes UNSIGNED BIG INT,"
        "dn_dif_bytes UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "app_name VARCHAR(128)," 
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }
     
   ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS task_status ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "table_name VARCHAR(128) NOT NULL,"
        "status INT DEFAULT 0,"
        "last_update INTEGER NOT NULL,"
        "CONSTRAINT unq UNIQUE (table_name, last_update))",
        NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

   ret = sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS block_list ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name VARCHAR(128) NOT NULL,"
      "type INT DEFAULT 0,"             // 0: black list 1: white list
      "CONSTRAINT unq UNIQUE (name))",
      NULL, NULL, &err);
   if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

   ret = sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS block_history ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "is_v4 UINT NOT NULL,"
      "client_ip UINT NOT NULL,"
      "client_ip6 VARCHAR(50) NOT NULL,"
      "name VARCHAR(128) NOT NULL,"
      "timestamp INTEGER NOT NULL,"             // 0: black list 1: white list
      "CONSTRAINT unq UNIQUE (client_ip, client_ip6, timestamp))",
      NULL, NULL, &err);
      if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
    }
 
  // index
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS ip_mac_app ON app_client (client_ip, client_ip6, mac, app_name)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_client create index error: %s\n", err);
            sqlite3_free(err);
   }

  set_db_ver(db, 2);
  
  return 0;
}


int upgrade_as_to_ver_1(sqlite3 *db)
{
  int ret;
  char *err;
  
  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS app_sum ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "client_ip UINT NOT NULL,"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "app_name VARCHAR(128)," 
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
  }
  
  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS exclusive_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "client_ip UINT NOT NULL,"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "domain_name VARCHAR(128),"
        "dev_type VARCHAR(20),"
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS exclusive_domain ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type INT," 
        "domain VARCHAR(128),"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"
        "CONSTRAINT unq UNIQUE (type, domain))",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);  
          sqlite3_free(err);
  }
 
  // index - app_sum 1/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS app_name ON app_sum (app_name ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index -[app_name] error: %s\n", err);
            sqlite3_free(err);
  }
  
  // index - app_sum 2/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS mac ON app_sum (mac ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index - [mac] error: %s\n", err);
            sqlite3_free(err);
  }

  // index - app_sum 3/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS timestamp ON app_sum (timestamp ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index - [timestamp] error: %s\n", err);
            sqlite3_free(err);
  }
  // index - exclusive_history
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS mac_dev ON exclusive_history (mac, dev_type)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("exclusive_history create index error: %s\n", err);
            sqlite3_free(err);
  }

  init_default_pattern_table(db);
  set_db_ver(db, 1);
  
  return 0;
}


int upgrade_as_to_ver_2(sqlite3 *db)
{
  int ret;
  char *err;
  // too many NOT NULL new field, drop table instead alter table
  drop_table(APP_SUM_TABLE, db);
  drop_table(EXCLUSIVE_DOMAIN_TABLE, db);
  drop_table(EXCLUSIVE_HISTORY_TABLE, db);

  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS app_sum ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "is_v4 UINT NOT NULL,"
        "client_ip UINT NOT NULL,"
        "client_ip6 VARCHAR(50) NOT NULL,"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "app_name VARCHAR(128)," 
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
  }
  
  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS exclusive_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "is_v4 UINT NOT NULL,"
        "client_ip UINT NOT NULL,"
        "client_ip6 VARCHAR(50) NOT NULL,"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"    
        "mac VARCHAR(128),"
        "domain_name VARCHAR(128),"
        "dev_type VARCHAR(20),"
        "timestamp INTEGER NOT NULL )",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);
          sqlite3_free(err);
   }

  ret = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS exclusive_domain ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type INT," 
        "domain VARCHAR(128),"
        "tx UNSIGNED BIG INT,"
        "rx UNSIGNED BIG INT,"
        "CONSTRAINT unq UNIQUE (type, domain))",
        NULL, NULL, &err);
  if(ret != SQLITE_OK && err != NULL) {
          dnsdbg("create db error %s", err);  
          sqlite3_free(err);
  }
 
  // index - app_sum 1/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS app_name ON app_sum (app_name ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index -[app_name] error: %s\n", err);
            sqlite3_free(err);
  }
  
  // index - app_sum 2/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS mac ON app_sum (mac ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index - [mac] error: %s\n", err);
            sqlite3_free(err);
  }

  // index - app_sum 3/3
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS timestamp ON app_sum (timestamp ASC)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("app_sum create index - [timestamp] error: %s\n", err);
            sqlite3_free(err);
  }
  // index - exclusive_history
  ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS mac_dev ON exclusive_history (mac, dev_type)", NULL, NULL, &err);
  if (ret != SQLITE_OK && err != NULL) {
            dnsdbg("exclusive_history create index error: %s\n", err);
            sqlite3_free(err);
  }

  init_default_pattern_table(db);
  set_db_ver(db, 2);
  
  return 0;
}

int process_db_upgrade(DNS_DB_TYPE db_type, sqlite3 *db, int old_ver, int new_ver)
{
  int ret;

  dnsdbg("migrade db[%d] from ver[%d] to ver[%d]", db_type, old_ver, new_ver);
  if(db_type == DNS_QUERY)
  {
    if(old_ver < 1)
    {
	// upgrade to ver 1 
        ret = upgrade_dq_to_ver_1(db);
	old_ver = get_db_ver(db);
    }

    if(old_ver < 2)
    {
      // upgrade to ver 2 
       ret = upgrade_dq_to_ver_2(db);
       old_ver = get_db_ver(db);
    }
    
    if(old_ver < 3)
    {
      // upgrade to ver 3
       ret = upgrade_dq_to_ver_3(db);
       old_ver = get_db_ver(db);
    }
  }
  else if(db_type == APP_CLIENT)
  {
    if(old_ver < 1)
    {
	// upgrade to ver 1 
        ret = upgrade_ac_to_ver_1(db);
	old_ver = get_db_ver(db);
    }

    if(old_ver < 2)
    {
	// upgrade to ver 2
        ret = upgrade_ac_to_ver_2(db);
	old_ver = get_db_ver(db);
    }

  }

  else if(db_type == APP_SUM)
  {
    if(old_ver < 1)
    {
	// upgrade to ver 1 
        ret = upgrade_as_to_ver_1(db);
	old_ver = get_db_ver(db);
    }

    if(old_ver < 2)
    {
	// upgrade to ver 2
        ret = upgrade_as_to_ver_2(db);
	old_ver = get_db_ver(db);
    }
  }
  
  if(ret != 0 || old_ver != new_ver)
  {
    ret = DNS_DB_UPGRADE_FAIL; 
  }

  return ret;
}
   
int db_migration(DNS_DB_TYPE db_type, sqlite3 *db, int new_ver)
{
   int ret = 0;
   int old_ver;

   ret = has_ver_table(db);
   if(ret<=0)
   {
     dnsdbg("db version table not exist");
     //drop table 
     if(db_type == DNS_QUERY)
     {
         drop_table(DNS_QUERY_TABLE, db);
	 drop_table(APP_DOMAIN_TABLE, db);
	 drop_table(APP_NAME_TABLE, db);
     }
     if(db_type == APP_CLIENT)
     {
         drop_table(APP_CLIENT_TABLE, db);
	 drop_table(DNS_BLOCK_LIST, db);
	 drop_table(DNS_BLOCK_HISTORY, db);
	 drop_table(TASK_STATUS_TABLE, db);
     }
     if(db_type == APP_SUM)
     {
         drop_table(APP_SUM_TABLE, db);
	 drop_table(EXCLUSIVE_DOMAIN_TABLE, db);
	 drop_table(EXCLUSIVE_HISTORY_TABLE, db);
     }

     //create ver table and give initial ver
     create_ver_table(db);
     set_db_ver(db, 0);
   }
   
   old_ver = get_db_ver(db);
   if(old_ver != new_ver)
   { 
      ret = process_db_upgrade(db_type, db, old_ver, new_ver); 
   }

   return ret;
}

sqlite3 *sqlite_open(DNS_DB_TYPE db_type, sqlite3 *db, char *fname) 
{
  int ret;
  char *err;

  if((ret = sqlite3_open(fname, &db)) != SQLITE_OK) {
      dnsdbg("Can't open database %s", sqlite3_errmsg(db));
      return NULL;
  }
  
  switch(db_type) {
    case DNS_QUERY:
      db_migration(db_type, db, DNS_QUERY_DB_VER);
      break;
    case APP_CLIENT:
      db_migration(db_type, db, APP_CLIENT_DB_VER);
      break;
    case APP_SUM:
      db_migration(db_type, db, APP_SUM_DB_VER);
      break;
  }
  
  return db;
}

int  get_primary_domain(char *full_dns, char *primary_dns)
{
  int i = 0, count;
  char *sub; 
  char name[8][128];
  char dns[128] ={0};
  strcpy(dns, full_dns);
  sub = strtok(dns, ".");
  while (sub != NULL && i<8) {
      strcpy(name[i], sub);
      i++;
      sub = strtok(NULL, ".");
  }
  count = i;
  // assemble
  if(count > 2) 
  { 
    //strcpy(primary_dns, "\%");
    for(i = 1 ; i< count; i++) 
    {
      if (i > 1) {
        strcat(primary_dns, ".");
        strcat(primary_dns, name[i]);
      }
      else {
        strcpy(primary_dns, name[i]);
      }
    }
  } 
  else
  {
     strcpy(primary_dns, "\%"); 
     strcat(primary_dns, full_dns); 
  }

  return count;
}

int sqlite_dns_lookup(sqlite3 *db, char *domain, int *app_id, int *cat_id, char *app_name)
{
  
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  char match_domain[128] = {0};
  char db_domain[256] = {0};
  char catalog[64] = {0};
  int  sub_domains;
  
  sub_domains = get_primary_domain(domain, match_domain);
  
  if(sub_domains > 2)
  {
   sprintf(sql, "select d.app_id, d.name, d.catalog, c.domain from app_domain as c \ 
    inner join app_name as d on c.app_id = d.app_id where c.domain like '%%%s' or c.domain='%s';",
    domain, match_domain);
  }
  else  
  {
    sprintf(sql, "select d.app_id, d.name, d.catalog, c.domain from app_domain as c \ 
    inner join app_name as d on c.app_id = d.app_id where c.domain like '%s';",
    match_domain);
  }

  //dnsdbg("lookup sql: %s\n", sql);

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK || row == 0)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    *app_id = DEFAULT_APP_ID;
    strcpy(app_name, DEFAULT_APP_NAME);
    *cat_id = DEFAULT_CAT_ID;
    //if(row == 0)
    //  sqlite3_free_table(result);
    
    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  {
    char *stop;
    // app_name.app_id
    *app_id = strtoul(result[index++], &stop, 10);
    // app_name.name
    strcpy(app_name, result[index++]);
    strcpy(catalog, result[index++]);
    strcpy(db_domain, result[index++]);
    
    //TODO: lookup catalog name to id
    *cat_id = DEFAULT_CAT_ID;
    if(!strcmp(domain, db_domain))
      break;
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

int sqlite_app_name_lookup(sqlite3 *db, int is_v4, char *mac, u_int32_t ipv4, char *ipv6, u_int16_t dst_port, int *app_id, int *cat_id, char *app_name)
{
  
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  char match_domain[128] = {0};
  char catalog[64] = {0};
  int protocols_len = sizeof(protocol_list)/sizeof(protocol_list[0]);
 
  if(is_v4)
  {
   sprintf(sql, "select d.app_id, d.name, d.catalog from dns_query as c \ 
    inner join app_name as d on c.app_id = d.app_id where c.resolve_ip=%u and c.app_id<>0 and c.mac='%s';",
    ipv4, mac);
  }
  else  
  {
    sprintf(sql, "select d.app_id, d.name, d.catalog from dns_query as c \ 
    inner join app_name as d on c.app_id = d.app_id where c.resolve_ip6='%s' and c.app_id<>0 and c.mac='%s';",
    ipv6, mac);
  }

  *app_id = DEFAULT_APP_ID;
  strcpy(app_name, DEFAULT_APP_NAME);
  *cat_id = DEFAULT_CAT_ID;
   
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }

  if(row > 1) 
    goto fall_thr_protocol;

  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  {
    char *stop;
    // app_name.app_id
    *app_id = strtoul(result[index++], &stop, 10);
    // app_name.name
    strcpy(app_name, result[index++]);
    strcpy(catalog, result[index++]);
    //TODO: lookup catalog name to id
    *cat_id = DEFAULT_CAT_ID;
    
  }

fall_thr_protocol:
    // fallthough to give a chance to map protocol as app name 
  if(*app_id == DEFAULT_APP_ID)
  {
     for(int j=0; j < protocols_len; j++)
     {
         if(dst_port == protocol_list[j].port)
	  {
	     strcpy(app_name, protocol_list[j].name);
       /* TODO: app_id remap for all known protocol */
       if(protocol_list[j].id == 0)
            *app_id = 50000 + j;
       else 
            *app_id = protocol_list[j].id;
             break;
          }
     }
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}


// TODO: remove mac, insert/update timestamp
int sqlite_dns_insert(sqlite3 *db, dns_msg *msg, char *mac, int app_id)
{
	int ret;
	char *err = NULL;
	// char datetime[32];
	char sql[512];
	unsigned int resolve_ip;
        char client_ip6[INET6_ADDRSTRLEN];
        char resolve_ip6[INET6_ADDRSTRLEN];
        bool is_resolve_v4;
  
  for (int i = 0 ; i< msg->question.ans.n_rec; i++) 
  {
    // only keep ipv4 client get ipv4 domain query answer and ipv6 client get ipv6 domain answer
    // sometimes ipv4 client can get both of ipv4 domain and ipv6 domain answer
    // but flow might ipv4/ipv6 client use ipv4 to get both of ipv4/ipv6 (i.e ipv6tf.org)
    // then use ipv6 to access (then conntrack ipv6 records can't link back domain name
    // so, decide to save this case, is_v4 is used for whether resolve_ip or resolve_ip6 is valid
    // used mac and resolve_ip/resolve_ip6 and mac to link conntrack table
    //dnsdbg("client isv4:%d get answer isv4:%d", msg->isv4, msg->question.ans.isv4[i]);
    memset(client_ip6, 0, INET6_ADDRSTRLEN);
    resolve_ip = 0;
    is_resolve_v4 = true;

    if(msg->isv4 && msg->question.ans.isv4[i])
    {
      is_resolve_v4 = true;
      inet_pton(AF_INET, msg->question.ans.recs[i], &resolve_ip);
      strcpy(client_ip6, DEFAULT_IPV6_ADDR);
      strcpy(resolve_ip6, DEFAULT_IPV6_ADDR);
      
    } else if(msg->isv4 && !msg->question.ans.isv4[i]) {
    
      is_resolve_v4 = false;
      resolve_ip = 0;
      //strcpy(client_ip6, msg->dip); // v4 str
      strcpy(client_ip6, DEFAULT_IPV6_ADDR);
      strcpy(resolve_ip6, msg->question.ans.recs[i]); // already v6 str
   }  else if(!msg->isv4 && !msg->question.ans.isv4[i]) {
    
      is_resolve_v4 = false;
      msg->dst_ip = 0; //already 0
      resolve_ip = 0;
      strcpy(client_ip6, msg->dip); // already v6 str
      strcpy(resolve_ip6, msg->question.ans.recs[i]); // already v6 str
 
   }  else if(!msg->isv4 && msg->question.ans.isv4[i]) {
    
      is_resolve_v4 = true;
      msg->dst_ip = 0; //already 0
      inet_pton(AF_INET, msg->question.ans.recs[i], &resolve_ip);
      strcpy(client_ip6, msg->dip); // already v6 str
      strcpy(resolve_ip6, DEFAULT_IPV6_ADDR);
    } else {
          continue;
    }
    //printf("resolve_ip %x\n", resolve_ip);
    sprintf(sql, "REPLACE INTO dns_query (is_v4, client_ip, client_ip6, name, resolve_ip, resolve_ip6, mac, app_id, timestamp) VALUES (%u, %u, '%s', '%s', %u,'%s', '%s', %u, %ld)",
                        is_resolve_v4,
                        msg->dst_ip,
                        client_ip6, 
                        msg->question.name, 
                        resolve_ip,
                        resolve_ip6,
                        mac,
                        app_id,
                        time(NULL)
                        );
    //printf("sql=[%s]", sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (ret != SQLITE_OK) {
      if(err != NULL) {
        //printf("sql err: %s\n", err);
        sqlite3_free(err);
      }
    }  
  }
 
	return DNSSQL_OK;
}


int sqlite_dns_map_ip(sqlite3 *db, nfapp_node_t *ap)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  int protocols_len = sizeof(protocol_list)/sizeof(protocol_list[0]);
  char ip_str[INET6_ADDRSTRLEN];
  /*
  sprintf(sql, "select c.name, c.mac, d.name from dns_query as c inner join app_name as d \
    on c.app_id = d.app_id where c.client_ip=%u and c.resolve_ip=%u",
    ap->src_ip, ap->dst_ip);
  */
  if(ap->isv4)
  {

   // conntrack ipv4 use dns mac to link
   inet_ntop(AF_INET6, &ap->dstv6, ip_str, INET6_ADDRSTRLEN);
   sprintf(sql, "select c.name, d.name from dns_query as c inner join app_name as d \
     on c.app_id = d.app_id where c.mac='%s' and c.resolve_ip=%u",
     ap->mac, ap->dst_ip);
  } else {

   // conntrack ipv6 use dns mac to link
   inet_ntop(AF_INET6, &ap->dstv6, ip_str, INET6_ADDRSTRLEN);
   sprintf(sql, "select c.name, d.name from dns_query as c inner join app_name as d \
     on c.app_id = d.app_id where c.mac='%s' and c.resolve_ip6='%s'",
     ap->mac, ip_str);
  }
  
  //dnsdbg("dns_map_ip sql:%s", sql);

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }

    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
 
  index = col;
  strcpy(ap->name, DEFAULT_APP_DOMAIN);
  //strcpy(ap->mac, DEFAULT_MAC);
  strcpy(ap->app_name, DEFAULT_APP_NAME);
  if(row > 1)
    goto fall_thr_protocol;

  //TODO: use timestamp as clue to decide dns name due to mulitple dns map to one ip 
  /*
   	sqlite> select * from dns_query where resolve_ip=3995794318;
	client_ip|name|resolve_ip|mac|app_id|timestamp
	2352195776|encrypted-tbn0.gstatic.com|3995794318|00:0E:C6:F1:1B:95|0|1660042277
	2352195776|encrypted-vtbn0.gstatic.com|3995794318|00:0E:C6:F1:1B:95|0|1660042325
	2352195776|google.com|3995794318|00:0E:C6:F1:1B:95|0|1660043274
	2352195776|play.google.com|3995794318|00:0E:C6:F1:1B:95|0|1660044419
	2536745152|gcdn.2mdn.net|3995794318|0C:9D:92:D9:7D:5D|0|1660044696
	2536745152|docs.google.com|3995794318|0C:9D:92:D9:7D:5D|0|1660044700
	2352195776|clients2.google.com|3995794318|00:0E:C6:F1:1B:95|0|1660045951
	2352195776|aa.google.com|3995794318|00:0E:C6:F1:1B:95|0|1660045951
	2352195776|clients5.google.com|3995794318|00:0E:C6:F1:1B:95|0|1660045969
  */
  for(int i=0; i<row; i++)
  {
    // only need app_id and app_name to complete app_client records
    strcpy(ap->name, result[index++]);
   // strcpy(ap->mac, result[index++]);
    strcpy(ap->app_name, result[index++]);
    //printf("----------------------------------------------\n");
  }

fall_thr_protocol:
  // fallthough to give a chance to map protocol as app name 
  if(!strcmp(ap->app_name, DEFAULT_APP_NAME))
  {
     for(int i=0; i < protocols_len; i++)
     {
         if(ap->dst_port == protocol_list[i].port)
	  {
	     strcpy(ap->app_name, protocol_list[i].name);
             break;
          }
     }
  }
 
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
 
}


int sqlite_nfcm_select(sqlite3 *db, char *sql, struct list_head *list)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  nfapp_node_t *ap;
  char src6_ip[INET6_ADDRSTRLEN];
  char dst6_ip[INET6_ADDRSTRLEN];

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }

   sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  
  // int prev = 0;
  for(int i=0; i<row; i++)
  {
    char *stop;
    ap = nfapp_node_new();
    list_add_tail(&ap->list, list);
   
    /*
    for(int j=0; j<col; j++)
    { 
      printf("%d--%s : %s\n", i, result[j], result[index++]);
    }
    */
    memset(src6_ip, 0, INET6_ADDRSTRLEN);
    memset(dst6_ip, 0, INET6_ADDRSTRLEN);
    ap->data_id =  atoi(result[index++]);
    ap->timestamp = atoi(result[index++]);
    ap->isv4 = atoi(result[index++]);    
    ap->src_ip = strtoul(result[index++], &stop, 10);
    strcpy(src6_ip, result[index++]);
    ap->src_port = strtoul(result[index++], &stop, 10);
    ap->dst_ip = strtoul(result[index++], &stop, 10);
    strcpy(dst6_ip, result[index++]);
    ap->dst_port = strtoul(result[index++], &stop, 10);
    ap->up_bytes = strtoull(result[index++], &stop, 10);
    ap->up_dif_bytes = strtoull(result[index++], &stop, 10);
    ap->dn_bytes = strtoull(result[index++], &stop, 10);
    ap->dn_dif_bytes = strtoull(result[index++], &stop, 10);
    /* use nfcm_app src_mac to persist into app_client */
    strcpy(ap->mac, result[index++]); 
    
    if(ap->isv4 != 1)
    {
     inet_pton(AF_INET6, src6_ip, &ap->srcv6);
     inet_pton(AF_INET6, dst6_ip, &ap->dstv6);
    }
    //printf("----------------------------------------------\n");
    /*
    if (prev && ap->data_id - prev != 1)
    {
      printf("found not continous row prev=%u data_id=%u\n", prev, ap->data_id);
    }
    prev = ap->data_id;
    */
  }
  //printf("----------------------------------------------\n");
  //printf("total_id =%d\n", row);
 
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

int sqlite_app_sum_aggregate(sqlite3 *db, int start, int end, struct list_head *list)
{
  int ret;
  char sql[1024] = {0};
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  nfapp_node_t *ap;
  char client_ip6[INET6_ADDRSTRLEN];

  sprintf(sql, "select is_v4, client_ip, client_ip6, mac, app_name, SUM(up_dif_bytes) as tx, SUM(dn_dif_bytes) as rx "\
  " from app_client where timestamp between %ld and %ld and mac <> '%s' group by is_v4, client_ip, mac, app_name;",
   start, end, DEFAULT_MAC);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    
   sqlite3_free_table(result);
   return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  
  // int prev = 0;
  for(int i=0; i<row; i++)
  {
    char *stop;
    ap = nfapp_node_new();
    list_add_tail(&ap->list, list);
    
    memset(client_ip6, 0, INET6_ADDRSTRLEN);
    /*
    for(int j=0; j<col; j++)
    { 
      printf("%d--%s : %s\n", i, result[j], result[index++]);
    }
    */
    ap->isv4 = atoi(result[index++]);
    ap->src_ip = strtoul(result[index++], &stop, 10);
    strcpy(client_ip6, result[index++]); 
    strcpy(ap->mac, result[index++]);
    strcpy(ap->app_name, result[index++]);
    ap->tx = strtoull(result[index++], &stop, 10);
    ap->rx = strtoull(result[index++], &stop, 10);
   
    if(ap->isv4) 
       inet_pton(AF_INET, DEFAULT_IPV6_ADDR, &ap->srcv6);
    else
       inet_pton(AF_INET6, client_ip6, &ap->srcv6);
   
    ap->timestamp = start + ((end-start)/2);
    

    //printf("sql s: %ld  e: %ld avg: %ld\n", start, end, start + ((end-start)/2));
    //printf("----------------------------------------------\n");
    /*
    if (prev && ap->data_id - prev != 1)
    {
      printf("found not continous row prev=%u data_id=%u\n", prev, ap->data_id);
    }
    prev = ap->data_id;
    */
  }
  //printf("----------------------------------------------\n");
  //printf("total_id =%d\n", row);
 
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

int sqlite_app_exclusive_statistics(sqlite3 *db, int start, int end, struct list_head *list)
{
  int ret;
  char sql[1024] = {0};
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  nfdev_node_t *dev;
  char client_ip6[INET6_ADDRSTRLEN];

  int exclusive_len = sizeof(dev_domain_list)/sizeof(dev_domain_list[0]);
  
  for (int i=0; i < exclusive_len; i++) {
    /*
    sprintf(sql, "select client_ip, mac, SUM(up_dif_bytes), SUM(dn_dif_bytes), name "\
      " from app_client where timestamp between %ld and %ld and mac = '%s' and name = '%s' "\
      ";", start, end, mac, dev_domain_list[i].domain);
    */
    if(dev_domain_list[i].wild_card == 0) {

    sprintf(sql, "select * from (select is_v4, client_ip, client_ip6, mac, SUM(up_dif_bytes) as tx, SUM(dn_dif_bytes) as rx, name "\
      " from app_client where timestamp between %ld and %ld group by is_v4, client_ip, mac, name ) where name = '%s' and mac <> '%s'"\
      ";", start, end, dev_domain_list[i].domain, DEFAULT_MAC);

    //dnsdbg("search exclusive - sql -%s\n", sql);
    } else {

    sprintf(sql, "select * from (select is_v4, client_ip, client_ip6, mac, SUM(up_dif_bytes) as tx, SUM(dn_dif_bytes) as rx, name "\
      " from app_client where timestamp between %ld and %ld group by is_v4, client_ip, mac, name ) where name like '%%%s' and mac <> '%s'"\
      ";", start, end, dev_domain_list[i].domain, DEFAULT_MAC);

   // dnsdbg("wild card search exclusive - sql -%s\n", sql);
   }   
   // dnsdbg("sql -%s\n", sql);
    row = 0;
    col = 0;
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
    if(ret != SQLITE_OK)
    {
      if(err) 
      {
        dnsdbg("sql err %s", err);
        sqlite3_free(err);
      }
      
      sqlite3_free_table(result);
      return DNSSQL_ERROR;
    }
   
    memset(client_ip6, 0, INET6_ADDRSTRLEN); 
    // print only , then add into list
    index = col;
    
    // int prev = 0;
    for(int j=0; j<row; j++)
    {
      char *stop;
      dev = nfdev_node_new();
      list_add_tail(&dev->list, list);
      dev->isv4 = atoi(result[index++]);
      dev->client_ip = strtoul(result[index++], &stop, 10);
      strcpy(client_ip6, result[index++]);
      strcpy(dev->mac, result[index++]);
      dev->tx = strtoull(result[index++], &stop, 10);
      dev->rx = strtoull(result[index++], &stop, 10);
      strcpy(dev->domain_name, result[index++]);
      strcpy(dev->dev_type, dev_domain_list[i].dev_type);
      dev->timestamp = start;
      
      if(dev->isv4) 
         inet_pton(AF_INET, DEFAULT_IPV6_ADDR, &dev->srcv6);
      else
         inet_pton(AF_INET6, client_ip6, &dev->srcv6);

    }
    // free here 
    sqlite3_free_table(result);
  }
 
  return DNSSQL_OK;
}

//TODO - add more filter to boost query speed
int sqlite_dev_type_query(sqlite3 *db, struct list_head *list)
{

  int ret;
  char sql[1024] = {0};
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  nfdev_node_t *dev;
  char client_ip6[INET6_ADDRSTRLEN];

  sprintf(sql, "select is_v4, client_ip, client_ip6, mac, dev_type, MAX(count) from "\
  "(select is_v4, client_ip, client_ip6, mac, dev_type, count(*) as count from "\ 
  "exclusive_history group by is_v4, client_ip, mac, dev_type) group by mac;");
  
  //dnsdbg("sql -%s\n", sql);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    
    sqlite3_free_table(result);
    return DNSSQL_ERROR;
  }
  
  // print only , then add into list
  index = col;
  
  // int prev = 0;
  for(int i=0; i<row; i++)
  {
    char *stop;
    memset(client_ip6, 0, INET6_ADDRSTRLEN);
    dev = nfdev_node_new();
    list_add_tail(&dev->list, list);
    dev->isv4 = atoi(result[index++]);
    dev->client_ip = strtoul(result[index++], &stop, 10);
    strcpy(client_ip6, result[index++]);
    strcpy(dev->mac, result[index++]);
    strcpy(dev->dev_type, result[index++]);
    dev->count = strtoull(result[index++], &stop, 10);
    if(dev->isv4) 
       inet_pton(AF_INET, DEFAULT_IPV6_ADDR, &dev->srcv6);
    else
       inet_pton(AF_INET6, client_ip6, &dev->srcv6);
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

int get_list_len(struct list_head *list)
{
   int i=0;
   nfapp_node_t *ap;
   list_for_each_entry(ap, list, list) {
     i++;
   }
   return i;
}

int sqlite_app_sum_insert(sqlite3 *db, struct list_head *list)
{

  int ret;
  char *err = NULL;
  // char datetime[32];
  char sql[10240] = {0};
  char tmp[512] = {0};
  // nf_node_t *nn;
  nfapp_node_t *ap;
  int len = get_list_len(list);
  int batch = BULK_INSERT_SIZE;
  char client_ip6[INET6_ADDRSTRLEN]; 
 
  //printf("%s:\n", __FUNCTION__);
  // printf("%s: total_len-%d\n", __FUNCTION__, len);
  int i = 0;
  int first = 1;
  // int timestamp = time(NULL);
  list_for_each_entry(ap, list, list) {
   i++;
   memset(client_ip6, 0, INET6_ADDRSTRLEN);
   if(ap->isv4)
      strcpy(client_ip6, DEFAULT_IPV6_ADDR);
   else
      inet_ntop(AF_INET6, (struct in6_addr *)&ap->srcv6, client_ip6, INET6_ADDRSTRLEN);
   if (first)
   {
     sprintf(sql, "INSERT INTO app_sum \
     (is_v4, client_ip, client_ip6, tx, rx, mac, app_name, timestamp)\
     VALUES (%u, %u, '%s', %llu, %llu, '%s', '%s', %ld)",
          ap->isv4,
          ap->src_ip,
          client_ip6,
          ap->tx,
          ap->rx,
          ap->mac,
          ap->app_name,
          ap->timestamp
          );
    first = 0;       
   } 
   else 
   {
    sprintf(tmp, ", (%u, %u, '%s', %llu, %llu, '%s', '%s', %ld)",
          ap->isv4,
          ap->src_ip,
          client_ip6,
          ap->tx,
          ap->rx,
          ap->mac,
          ap->app_name,
          ap->timestamp
          );  
    strcat(sql, tmp);
    if (i%batch == 0 )
      first = 1;
   }
  
   if( i%batch == 0 || i == len)
   {  
      strcat(sql, " ;");
      
      ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert fail %s", err);
          sqlite3_free(err);
        }
      }  
   }

  }
  
  /*
  strcat(sql, " ;");
  printf("%d at %ld len=%d sql=%s\n", i++, time(NULL), strlen(sql), sql);      
      
  ret = sqlite3_exec(db, sql, NULL, NULL, &err);
  if (ret != SQLITE_OK) {
    if(err != NULL) {
      printf("insert fail %s\n", err);
      sqlite3_free(err);
    }
  } 
  */ 
	return DNSSQL_OK;
}


int sqlite_app_exclusive_history_insert(sqlite3 *db, struct list_head *list)
{

  int ret;
  char *err = NULL;
  // char datetime[32];
  char sql[10240] = {0};
  char tmp[512] = {0};
  // nf_node_t *nn;
  nfdev_node_t *dev;
  int len = get_list_len(list);
  int batch = BULK_INSERT_SIZE;
  char client_ip6[INET6_ADDRSTRLEN];

  //printf("%s:\n", __FUNCTION__);
  // printf("%s: total_len-%d\n", __FUNCTION__, len);
  int i = 0;
  int first = 1;
  // int timestamp = time(NULL);
  list_for_each_entry(dev, list, list) {
   i++;
   memset(client_ip6, 0, INET6_ADDRSTRLEN);
   if(dev->isv4)
      strcpy(client_ip6, DEFAULT_IPV6_ADDR);
   else
      inet_ntop(AF_INET6, (struct in6_addr *)&dev->srcv6, client_ip6, INET6_ADDRSTRLEN);

   if (first)
   {
     sprintf(sql, "INSERT INTO exclusive_history \
     (is_v4, client_ip, client_ip6, mac, tx, rx, domain_name, dev_type, timestamp)\
     VALUES (%u, %u, '%s', '%s', %llu, %llu, '%s', '%s', %ld)",
          dev->isv4,
          dev->client_ip,
          client_ip6,
          dev->mac,
          dev->tx,
          dev->rx,
          dev->domain_name,
          dev->dev_type,
          dev->timestamp
          );
    first = 0;       
   } 
   else 
   {
    sprintf(tmp, ", (%u, %u, '%s', '%s', %llu, %llu, '%s', '%s', %ld)",
          dev->isv4,
          dev->client_ip,
          client_ip6,
          dev->mac,
          dev->tx,
          dev->rx,
          dev->domain_name,
          dev->dev_type,
          dev->timestamp
          );  
    strcat(sql, tmp);
    if (i%batch == 0 )
      first = 1;
   }
  
   if( i%batch == 0 || i == len)
   {  
      strcat(sql, " ;");
      
      ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert fail %s", err);
          sqlite3_free(err);
        }
      }  
   }

  }
  return DNSSQL_OK;
}


int sqlite_app_client_insert(sqlite3 *db, struct list_head *list)
{

  int ret;
  char *err = NULL;
  // char datetime[32];
  char sql[10240] = {0};
  char tmp[512] = {0};
 // nf_node_t *nn;
  nfapp_node_t *ap;
  int len = get_list_len(list);
  int batch = BULK_INSERT_SIZE;
  char client_ip6[INET6_ADDRSTRLEN];
  char resolve_ip6[INET6_ADDRSTRLEN];

  //printf("%s:\n", __FUNCTION__);
  // printf("%s: total_len-%d\n", __FUNCTION__, len);
  int i = 0;
  int first = 1;
  // int timestamp = time(NULL);
  list_for_each_entry(ap, list, list) {
   i++;
   memset(client_ip6, 0, INET6_ADDRSTRLEN);
   memset(resolve_ip6, 0, INET6_ADDRSTRLEN);
   
   if(ap->isv4)
   {
       strcpy(client_ip6, DEFAULT_IPV6_ADDR);
       strcpy(resolve_ip6, DEFAULT_IPV6_ADDR);
   } else {
       
       inet_ntop(AF_INET6, (struct in6_addr*)&ap->srcv6, client_ip6, INET6_ADDRSTRLEN);
       inet_ntop(AF_INET6, (struct in6_addr*)&ap->dstv6, resolve_ip6, INET6_ADDRSTRLEN);

   } 

   if (first)
   {
     sprintf(sql, "INSERT INTO app_client \
     (data_id, is_v4, client_ip, client_ip6, src_port, name, resolve_ip, resolve_ip6, dst_port, up_bytes, up_dif_bytes, dn_bytes, dn_dif_bytes, mac, app_name, timestamp)\
     VALUES (%d, %u, %u, '%s', %u, '%s', %u, '%s', %u, %llu, %llu, %llu, %llu, '%s', '%s', %ld)",
          ap->data_id,
          ap->isv4,
          ap->src_ip,
          client_ip6,
          ap->src_port,
          ap->name,
          ap->dst_ip,
          resolve_ip6,
          ap->dst_port,
          ap->up_bytes,
          ap->up_dif_bytes,
          ap->dn_bytes,
          ap->dn_dif_bytes,
          ap->mac,
          ap->app_name,
          ap->timestamp
          );
    first = 0;       
   } 
   else 
   {
    sprintf(tmp, ", (%d, %u, %u, '%s', %u, '%s', %u, '%s', %u, %llu, %llu, %llu, %llu, '%s', '%s', %ld)",
          ap->data_id,
          ap->isv4,
          ap->src_ip,
          client_ip6,
          ap->src_port,
          ap->name,
          ap->dst_ip,
          resolve_ip6,
          ap->dst_port,
          ap->up_bytes,
          ap->up_dif_bytes,
          ap->dn_bytes,
          ap->dn_dif_bytes,
          ap->mac,
          ap->app_name,
          ap->timestamp
          );  
    strcat(sql, tmp);
    if (i%batch == 0 )
      first = 1;
   }
  
   if( i%batch == 0 || i == len)
   {  
      strcat(sql, " ;");
      
      ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert fail %s", err);
          sqlite3_free(err);
        }
      }  
   }

  }
  
   return DNSSQL_OK;
}


int sqlite_task_status_insert_or_update(sqlite3 *db, char *table_name, int time, int status)
{ 
  int ret;
  int old_status;
  int old_time;
  char sql[512] = {0};
  char *err = NULL;
  ret = sqlite_task_status_get_last(db, table_name, &old_status, &old_time);

#ifdef DNSQD_DEBUG
  dnsdbg("o-s: %d  o-time: %d", old_status, old_time);
#endif

  if (ret != DNSSQL_OK)
  { 
    sprintf(sql, "insert into %s (table_name, status, last_update) values ('%s', %d, %ld);",
    TASK_STATUS_TABLE, table_name, status, time);
  } else
  {
    sprintf(sql, "update %s set status=%d, last_update=%ld where table_name='%s';",
    TASK_STATUS_TABLE, status, time, table_name);
  }
  //dnsdbg("run sql=%s", sql);
  ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert fail %s", err);
          sqlite3_free(err);
        }
  }  
  return ret;
}

int sqlite_task_status_get_last(sqlite3 *db, char *name, int *status, int *time)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char table[128] = {0};
  char sql[512] = {0};
  
  sprintf(sql, "select table_name, status, last_update from %s where table_name='%s';",
  TASK_STATUS_TABLE, name);

  //dnsdbg("sql=%s", sql);
  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK || row == 0)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
    if(row == 0)
    {
      dnsdbg("now row - # of row - %d", row);
      //sqlite3_free_table(result);
    }
    *status = -1;
    *time = 0;
    
   sqlite3_free_table(result);
   return DNSSQL_ERROR;
  }
 
  index = col;
  for(int i=0; i<row; i++)
  {
    strcpy(table, result[index++]);
    *status = atoi(result[index++]);
    *time = atoi(result[index++]);
//    dnsdbg("[%d]-[%s]-[%d]-[%d]", i, table, *status, *time);
  }
 
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

// app_client statistics
int sqlite_app_client_statistics(sqlite3 *db, int start, int end, struct list_head *list)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  appstats_node_t *ap;
  char client_ip6[INET6_ADDRSTRLEN];

  sprintf(sql, "select is_v4, client_ip, client_ip6, name, SUM(up_dif_bytes) as up, SUM(dn_dif_bytes) as dn "\
  "from app_client where timestamp between %ld and %ld group by is_v4, client_ip, name "\
  " order by client_ip, dn DESC;",
  start, end);
  //dnsdbg("sql:%s", sql);

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
     
   sqlite3_free_table(result);
   return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  {
    char *stop;
    memset(client_ip6, 0, INET6_ADDRSTRLEN);

    ap = appstats_node_new();
    list_add_tail(&ap->list, list);
    ap->isv4 = atoi(result[index++]);   
    ap->client_ip = strtoul(result[index++], &stop, 10);
    strcpy(client_ip6, result[index++]);
    strcpy(ap->name, result[index++]);
    ap->up_bytes = strtoull(result[index++], &stop, 10);
    ap->dn_bytes = strtoull(result[index++], &stop, 10);
    
    if(ap->isv4)
       inet_pton(AF_INET, DEFAULT_IPV6_ADDR, &ap->srcv6);
    else
       inet_pton(AF_INET, client_ip6, &ap->srcv6);
   
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

//block_history
int sqlite_block_history_insert(sqlite3 *db, uint32_t client_ip, char *host, int timestamp)
{
  int ret;
  char *err = NULL;
  char sql[512] = {0};

    
  sprintf(sql, "INSERT INTO block_history (is_v4, client_ip, client_ip6, name, timestamp)\
     VALUES (%u, %u, '%s', '%s', %ld);",
          1,
          client_ip,
          DEFAULT_IPV6_ADDR,
          host,
          timestamp
          );
  ret = sqlite3_exec(db, sql, NULL, NULL, &err);
      if (ret != SQLITE_OK) {
        if(err != NULL) {
          dnsdbg("insert fail %s", err);
          sqlite3_free(err);
        }
  }  
  return ret;
}

//block_history query
int sqlite_block_query_statistics(sqlite3 *db, int start, int end, struct list_head *list)
{
  int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  block_history_node_t *bh;

  sprintf(sql, "select client_ip, name, timestamp "\
  "from block_history where timestamp between %ld and %ld "\
  "order by timestamp desc;",
  start, end);

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
  
  sqlite3_free_table(result);
  return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  {
    char *stop;
    bh = block_history_node_new();
    list_add_tail(&bh->list, list);
   
    bh->client_ip = strtoul(result[index++], &stop, 10);
    strcpy(bh->name, result[index++]);
    bh->timestamp = atoi(result[index++]);   
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}


//block_list
int sqlite_block_list_has_name(sqlite3 *db, char *name, int *type)
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
  
  sqlite3_free_table(result);
  return DNSSQL_ERROR;
  }
  
  if(row == 0)
  { 
    sqlite3_free_table(result);
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


int sqlite_block_list_insert_or_update(sqlite3 *db, char *host, int type)
{
  int ret;
  int old_type = 0;
  int is_update = 0;
  char sql[512] = {0};
  char *err = NULL;
  ret = sqlite_block_list_has_name(db, host, &old_type);
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

int sqlite_block_list_all(sqlite3 *db, struct list_head *list)
{
int ret;
  char *err = NULL;
  char **result;
  int row;
  int col;
  int index;
  char sql[512] = {0};
  block_entry_node_t *be;

  sprintf(sql, "select name, type from block_list ;");

  ret = sqlite3_get_table(db, sql, &result, &row, &col, &err);
  if(ret != SQLITE_OK)
  {
    if(err) 
    {
      dnsdbg("sql err %s", err);
      sqlite3_free(err);
    }
  
  sqlite3_free_table(result);
  return DNSSQL_ERROR;
  }
  // print only , then add into list
  index = col;
  for(int i=0; i<row; i++)
  {
    char *stop;
    be = block_entry_node_new();
    list_add_tail(&be->list, list);
   
    strcpy(be->name, result[index++]);
    be->type = atoi(result[index++]);   
  }
  // free here 
  sqlite3_free_table(result);
  return DNSSQL_OK;
}

int sqlite_free_result(char **result)
{
  sqlite3_free_table(result);
  return DNSSQL_OK;
}


int sqlite_close(sqlite3 *db) {
	if (db != NULL)
		sqlite3_close(db);

	return DNSSQL_OK;
}


/*
    integrity check
    Check database integrity : if it's broken, delete this database
*/
int sqlite_integrity_check(sqlite3 *db, char *db_path)
{
    int ret = 1;
    sqlite3_stmt *integrity;
    int verify = 0;
    int chk;
 
    chk = sqlite3_prepare_v2(db, "PRAGMA integrity_check;", -1, &integrity, NULL);
    if (chk == SQLITE_OK) {
        while (sqlite3_step(integrity) == SQLITE_ROW) {
            const unsigned char *result = sqlite3_column_text(integrity, 0);
            if (result && strcmp((const char *)result, (const char *)"ok") == 0) {
                verify = 1;
                ret = 1;
                break;
            }
        }
        sqlite3_finalize(integrity);
#if !defined(X86_APP)
        /* if the result isn't "ok", database is broken, remove this db */
        if (verify == 0) {
            eval("rm", "-f", db_path);
            //info("%s: remove broken database : %s\n", __FUNCTION__, db_path);
            ret = 0;
        }
#endif        
    }
 
    return ret;
}

/*
    remove *-journal file
*/
void sqlite_remove_journal(char *fname)
{
    char buf[160] = {0};
    int cnt = 0;

    snprintf(buf, sizeof(buf), "%s-journal", fname);
#if !defined(X86_APP)
    while (f_exists(buf)) {
        cnt++;
        eval("rm", "-f", buf);
        if (cnt > 30) {
            break;
        }
        //info("remove %s", buf);
    }
#endif    
#if 0
    if (f_exists(buf)) {
        printf("delete %s fail\n", buf);
    } else {
        printf("delete %s OK\n", buf);
    }
#endif
}
