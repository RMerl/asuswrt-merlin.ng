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
#include <signal.h>


//#include "list.h"
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

#define COMPARE_MAC 0
#define COMPARE_APP 1
#define COMPARE_ALL 2

typedef struct bw_stat_s {
    char src_mac[ETHER_ADDR_LENGTH];
    char app_name[64];
    uint64_t tx;
    uint64_t rx;
    int app_id;
    int cat_id;
    struct list_head list;
} bw_stat_t;


bw_stat_t* bw_stat_new()
{
    bw_stat_t *nn;

    nn = (bw_stat_t *)calloc(1, sizeof(bw_stat_t));
    if (!nn) return NULL;

    INIT_LIST_HEAD(&nn->list);

    return nn;
}

void bw_stat_node_free(bw_stat_t *nn)
{
    if (nn)
        free(nn);

    return;
}

void bw_stat_list_free(struct list_head *list)
{
    bw_stat_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, list, list) {
        list_del(&nn->list);
        bw_stat_node_free(nn);
    }
    return;
}

int bw_stat_list_size(struct list_head *list)
{
    bw_stat_t *nn;
    int cnt = 0;

    list_for_each_entry(nn, list, list) {
        cnt++;
    }

    return cnt;
}


void bw_stat_dump(bw_stat_t *nn)
{
    //printf("src_mac:%pM\n", nn->src_mac);
    printf("src_mac:%s\n", nn->src_mac);
    printf("app:       %s\n", nn->app_name);
    printf("tx:       %llu\n", nn->tx);
    printf("rx:       %llu\n", nn->rx);
    printf("app_id:       %d\n", nn->app_id);
    printf("cat_id:       %d\n", nn->cat_id);
    return;
}


void bw_stat_list_dump(char *title, struct list_head *list)
{
    bw_stat_t * nn;

    printf("[%s]%s: %s, count=[%d]\n", __FILE__, __FUNCTION__, title, bw_stat_list_size(list));
    list_for_each_entry(nn, list, list) {
        bw_stat_dump(nn);
    }
    printf("=======================\n");
}


bw_stat_t*  bw_stat_list_search(int comparsion, char *mac, int app_id,
                          struct list_head *bw_stat_list)
{
    bw_stat_t *nn;
    list_for_each_entry(nn, bw_stat_list, list) {
//    printf("bw_sta_list_se cmp=%d mac=%s app=%s mac_src=%s app_src=%s\n", comparsion, nn->src_mac, nn->app_name,
//           mac, app);
      
        if (comparsion == COMPARE_MAC) {
            if (!strcmp(nn->src_mac, mac))
            {
                return nn;
            }
        } else if (comparsion == COMPARE_APP) {
            if (nn->app_id == app_id)
            {
                return nn;
            }
        } else if (comparsion == COMPARE_ALL) {
            if (!strcmp(nn->src_mac, mac) && nn->app_id == app_id)
            {
                return nn;
            }
        }
    }
    return NULL;
}





//#define DEBUG_SQL 1
#define FAKE_WEBSWRITE 0

#if FAKE_WEBSWRITE
#define websWrite fake_websWrite

int fake_websWrite(int wp, const char *format, ...)
{
	va_list args;
  int n;
  va_start(args, format);
  n = vprintf(format, args);
  va_end(args);
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

int get_bw_mac_list(struct list_head *bw_mac_list, int *bw_mac_len, json_object *root)
{
  int ret = 0;
  int bw_ct_len, len = 0;
  int i;
  json_object *arryObj = NULL;
  json_object *bwCtObj = NULL;
  json_object *macObj = NULL;
  json_object *appObj = NULL;
  json_object *txObj = NULL;
  json_object *rxObj = NULL;
  json_object *appIdObj = NULL;
  json_object *catIdObj = NULL;
  char *p_mac = NULL;
  //unsigned char mac[6];
  bw_stat_t *node = NULL;
  
//  printf("json_object_object_get_ex\n");
  json_object_object_get_ex(root, "contents", &arryObj);
  bw_ct_len = json_object_array_length(arryObj);
//  printf("bw_ct_len=%d\n", bw_ct_len);

  for(i = 0; i < bw_ct_len; i++)
  {
  //  printf("i=%d\n", i);

    bwCtObj = json_object_array_get_idx(arryObj, i);
  //  printf("macObj\n");

    json_object_object_get_ex(bwCtObj, "mac", &macObj);
    p_mac = json_object_get_string(macObj);
  //  printf("p_mac=%s\n", p_mac);
    //ether_atoe(p_mac, mac);
    node = bw_stat_list_search(COMPARE_MAC, p_mac, 0, bw_mac_list);
    if(!node) {
      node = bw_stat_new();
      if(!node)
        return -1;
      strcpy(node->src_mac, p_mac);   
      list_add_tail(&node->list, bw_mac_list);
      len = len + 1;
    }
  }
  *bw_mac_len = len;
//  printf("bw_mac_len=%d\n", *bw_mac_len);

  return ret;
}

int check_bw_mon_trf_valid(json_object *root)
{
  int ret = 0;
  int ts;
  json_object *tsObj = NULL;
  
  json_object_object_get_ex(root, "ts", &tsObj);
  ts = json_object_get_int(tsObj);
  if(time(NULL) - ts < 30)
    ret = 1;
  
  return ret;
}

int get_bw_stat_list(struct list_head *bw_stat_list, int *bw_stat_len, json_object *root)
{
  int ret = 0;
  int bw_ct_len, len = 0;
  int i;
  json_object *arryObj = NULL;
  json_object *bwCtObj = NULL;
  json_object *macObj = NULL;
  json_object *appObj = NULL;
  json_object *txObj = NULL;
  json_object *rxObj = NULL;
  json_object *appIdObj = NULL;
  json_object *catIdObj = NULL;
  
  char *p_mac = NULL;
  char *p_app = NULL;
  uint64_t tx;
  uint64_t rx;
  int app_id, cat_id;
  //unsigned char mac[6];
  bw_stat_t *node = NULL;
  
  json_object_object_get_ex(root, "contents", &arryObj);
  bw_ct_len = json_object_array_length(arryObj);
 // printf("bw_ct_len=%d\n", bw_ct_len);
  for(i = 0; i < bw_ct_len; i++)
  {
 //   printf("i=%d\n", i);
    bwCtObj = json_object_array_get_idx(arryObj, i);
    json_object_object_get_ex(bwCtObj, "mac", &macObj);
    p_mac = json_object_get_string(macObj);

    json_object_object_get_ex(bwCtObj, "app", &appObj);
    p_app = json_object_get_string(appObj);

    json_object_object_get_ex(bwCtObj, "tx", &txObj);
    tx = json_object_get_int64(txObj);

    json_object_object_get_ex(bwCtObj, "rx", &rxObj);
    rx = json_object_get_int64(rxObj);
    
    json_object_object_get_ex(bwCtObj, "app_id", &appIdObj);
    app_id = json_object_get_int(appIdObj);

    json_object_object_get_ex(bwCtObj, "cat_id", &catIdObj);
    cat_id = json_object_get_int(catIdObj);
    
    //printf("got node mac=%s ap=%s tx=%llu rx=%llu ap_id=%d\n", p_mac, p_app, tx, rx, app_id);

    //ether_atoe(p_mac, mac);
    node = bw_stat_list_search(COMPARE_ALL, p_mac, app_id, bw_stat_list);
    if(!node) {
      node = bw_stat_new();
      if(!node)
        return -1;

      strcpy(node->src_mac, p_mac);
      strcpy(node->app_name, p_app);
      node->tx = tx;
      node->rx = rx;
      node->app_id = app_id;      
      node->cat_id = cat_id;
      list_add_tail(&node->list, bw_stat_list);
      len = len + 1;
    } else {
      //printf("+++++hit mac=%s ap=%s tx=%llu rx=%llu\n", node->src_mac, node->app_name, node->tx, node->rx);
      node->tx += tx;
      node->rx += rx;
      //printf("----hit mac=%s ap=%s tx=%llu rx=%llu\n", node->src_mac, node->app_name, node->tx, node->rx);

    }
  }
  *bw_stat_len = len;

  return ret;
}


/*
	if status = all 		=> [TX, RX]
	if status = "" && name = ""	=> [[MAC0, TX, RX], [MAC1, TX, RX], ...]
	if status = "" && name = "MAC"	=> [[APP0, TX, RX], [APP1, TX, RX], ...]
*/
static void get_dns_client_hook(int *retval, webs_t wp, char *status, char *name)
{
	int ret = 0;
	int first_row = 1;
	unsigned int index = 0;
	unsigned long app_count = 0;
	unsigned long app_down = 0, app_up = 0;
	unsigned long total_down = 0, total_up = 0;
  int lock = 0;
  json_object *bw_mon_trf_obj = NULL;
	
	//udb_ioctl_entry_t *usr_lst = NULL;
	//app_ioctl_entry_t *app_lst = NULL;
	uint32_t usr_cnt = 0, app_cnt = 0;
	uint32_t bw_mac_len = 0, bw_stat_len = 0;
	char cat_name[64];
	char app_name[64];

  bw_stat_t *mac_node = NULL, *app_node = NULL;


	LIST_HEAD(bw_mac_list);
	LIST_HEAD(bw_stat_list);

	memset(cat_name, 0 , sizeof(cat_name));
	memset(app_name, 0 , sizeof(app_name));

  lock = file_lock(DNS_BW_MON_TRAF_LOCK_NAME);
  bw_mon_trf_obj = json_object_from_file(JSON_BW_MON_TRAFFIC_FILE);
  file_unlock(lock);

// TODO: convert json into device and app view
  if(!bw_mon_trf_obj) {
//    printf("bw_mon_trf_obj null\n");
    *retval += websWrite(wp, "[]");
    json_object_put(bw_mon_trf_obj);
    return;
  }

   if(!check_bw_mon_trf_valid(bw_mon_trf_obj)) {
    *retval += websWrite(wp, "[]");
    json_object_put(bw_mon_trf_obj);
    return;
  }
//  printf("get_bw_mac_list\n");
  
  ret = get_bw_mac_list(&bw_mac_list, &bw_mac_len, bw_mon_trf_obj);
//  printf("get_bw_stat_list\n");

  ret = get_bw_stat_list(&bw_stat_list, &bw_stat_len, bw_mon_trf_obj);

//  printf("json_object_put\n");

  json_object_put(bw_mon_trf_obj);

#if 1
//	bw_stat_list_dump("mac list", &bw_mac_list);
 // bw_stat_list_dump("bw ct list", &bw_stat_list);
#endif

#if 1

	if (!strcmp(status, "")) {
		*retval += websWrite(wp, "[");
	}

	if (bw_mac_len > 0)
	{
		//usr_cnt = usr_buf_len / sizeof(*usr_lst);

		//if (app_lst)
	  //	{
		//	app_cnt = app_buf_len / sizeof(*app_lst);
		//}

		total_down = total_up = 0;
		// each device
		list_for_each_entry(mac_node, &bw_mac_list, list)
		{
			//char buff[18];
			//if (usr_lst[i].available <= 0) break;

			app_down = app_up = app_count = 0;
			//snprintf(buff, sizeof(buff), MAC_OCTET_FMT, MAC_OCTET_EXPAND(usr_lst[i].mac));

			if (!strcmp(status, "") && strcmp(name, "")) {
				// get certain client's app
				//printf("mac=%s, name=%s\n", mac_node->src_mac, name);
				if (strcmp(mac_node->src_mac, name)) continue;
				first_row = 1;

				//for (j = 0; j < app_cnt; j++)
				list_for_each_entry(app_node, &bw_stat_list, list)
        {
					//if (app_lst[j].available <= 0) break;
          memset(cat_name, 0 , sizeof(cat_name));
	        memset(app_name, 0 , sizeof(app_name));

					if (!strcmp(app_node->src_mac, mac_node->src_mac))
					{
						//if (app_node->cat_id == 0 && app_node->app_id == 0)
            strlcpy(cat_name , "General", sizeof(cat_name));						
            if (app_node->app_id == 0)
						{
							strlcpy(app_name , "General", sizeof(app_name));
						}
						else
						{
              
							strlcpy(app_name , app_node->app_name, sizeof(app_name));
						}

#if 0
            if(!strcmp(app_name, "youtube") || app_node->app_id == 99) {
              dnsdbg("[\"%s\", \"%" PRIu64 "\", \"%" PRIu64 "\", \"%u\", \"%u\"]", 
									app_name, app_node->tx, app_node->rx,
									app_node->cat_id, app_node->app_id);
            }
#endif

						//BWSQL_LOG("[%5u] cat/app= %s/%s, cat/app id = %u/%u, up/down = %" PRIu64 "/%" PRIu64,
						//	j, cat_name, app_name, app_lst[j].cat_id, app_lst[j].app_id,
            //                                            app_lst[j].up_recent_accl, app_lst[j].down_recent_accl);
            int re_id  = app_node->app_id % 1000;
            int re_cat = app_node->app_id / 1000;
						if (first_row == 1) {
							first_row = 0;
							*retval += websWrite(wp, "[\"%s\", \"%" PRIu64 "\", \"%" PRIu64 "\", \"%u\", \"%u\"]", 
									app_name, app_node->tx, app_node->rx,
									re_cat, re_id);
						}
						else {
							*retval += websWrite(wp, ", [\"%s\", \"%" PRIu64 "\", \"%" PRIu64 "\", \"%u\", \"%u\"]", 
									app_name, app_node->tx, app_node->rx,
									re_cat, re_id);
						}
						app_count++;
					}
				}
				if (!strcmp(mac_node->src_mac, name)) break;
			}
			else {
				// each app in device
				//for (j = 0; j < app_cnt; j++)
				list_for_each_entry(app_node, &bw_stat_list, list)
        {
					//if (app_lst[j].available <= 0) break;
					if (!strcmp(app_node->src_mac, mac_node->src_mac))
					{
						app_down += app_node->rx;
						app_up += app_node->tx;
						app_count++;
					}
				}

				if (!strcmp(status, "all")) { // get all
					total_down += app_down;
					total_up += app_up;
				//	printf("total_up/total_down=%lu/%lu\n", total_up, total_down);
				}
				else if (!strcmp(status, "") && !strcmp(name, "")) { // get each client
					if (index == 0)
						*retval += websWrite(wp, "[\"%s\", \"%lu\", \"%lu\"]", mac_node->src_mac, app_up, app_down);
					else
						*retval += websWrite(wp, ", [\"%s\", \"%lu\", \"%lu\"]", mac_node->src_mac, app_up, app_down);
					//printf("mac=%s, up/_down=%lu/%lu\n", mac_node->src_mac, app_up, app_down);
          index++;
				}
			}
		}
	}

	if (!strcmp(status, "")) {
		*retval += websWrite(wp, "]");
	}

  bw_stat_list_free(&bw_mac_list);
  bw_stat_list_free(&bw_stat_list);
#endif
#if 0
  srand(time(NULL));
  total_up  = 100000 * (rand()/3);
  total_down  = 100000 * (rand()/3);
	*retval += websWrite(wp, "[\"%lu\", \"%lu\"]", total_up, total_down);
  return;
#endif  
	if (!strcmp(status, "all")) {
		*retval += websWrite(wp, "[\"%lu\", \"%lu\"]", total_up, total_down);
	}
}


/*
 * Kills process whose PID is stored in plaintext in pidfile
 * @param       pidfile PID file
 * @sig         signal to be send
 * @rm          whether to remove this pid file (1) or not (0).
 * @return      0 on success and errno on failure
 */

int notify_dnsqd_bw_active(char *pidfile, int sig, int rm)
{
        FILE *fp;
        char buf[256];

        if ((fp = fopen(pidfile, "r")) != NULL) {
                if (fgets(buf, sizeof(buf), fp)) {
                        pid_t pid = strtoul(buf, NULL, 0);
                        fclose(fp);
                        if(rm)
                                unlink(pidfile);
                        return kill(pid, sig);
                }
                fclose(fp);
        }
        return errno;
}
/*
	get_dns_traffic_hook for web hook <% bwdpi_status() %>

	mode : traffic / traffic_wan / app / client_apps / client_web
	name : NULL / MAC / APP_NAME
	dura : realtime / month / week / day
	date : NULL / date
*/
void get_dns_traffic_hook(char *mode, char *name, char *dura, char *date, int *retval, webs_t wp)
{
	//BWSQL_LOG("mode=%s, name=%s, dura=%s, date=%s", mode, name, dura, date);

	// send singal to wake up to check bwdpi engine
	notify_dnsqd_bw_active(DNSQD_PID_FILE, SIGUSR2, 0);

	if (!strcasecmp(mode, "traffic_wan") && !strcasecmp(dura, "realtime")) {
		get_dns_client_hook(retval, wp, "all", "");
	}
	else if (!strcasecmp(mode, "traffic") && !strcasecmp(dura, "realtime")) {
		get_dns_client_hook(retval, wp, "", name);
	}
	else {
		*retval += websWrite(wp, "[]");
	}
}
