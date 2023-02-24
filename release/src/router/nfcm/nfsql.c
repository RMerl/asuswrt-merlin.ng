#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <shutils.h>

#include "log.h"
#include "list.h"
#include "nfcm.h"
#include "nfsql.h"

#if defined(CODB)
#include "cosql_utils.h"
#endif

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
        /* if the result isn't "ok", database is broken, remove this db */
        if (verify == 0) {
            eval("rm", "-f", db_path);
            //info("%s: remove broken database : %s\n", __FUNCTION__, db_path);
            ret = 0;
        }

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
    while (f_exists(buf)) {
        cnt++;
        eval("rm", "-f", buf);
        if (cnt > 30) {
            break;
        }
        //info("remove %s", buf);
    }
#if 0
    if (f_exists(buf)) {
        printf("delete %s fail\n", buf);
    } else {
        printf("delete %s OK\n", buf);
    }
#endif
}

#if defined(CODB)
#define NUM(a)    (sizeof (a) / sizeof (a[0]))

/*
 *  the db_version should be changed if the fields/attributes had changed
 */
#define DB_VER "v0.11"

sql_column_prototype_t app_columns[] = {
    {"timestamp", COLUMN_TYPE_INT},        //INTEGER NOT NULL
    {"is_v4",     COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"proto",     COLUMN_TYPE_UINT},       //UNSIGNED INTEGER
    {"src_ip",    COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"src6_ip",    COLUMN_TYPE_TEXT},       //VARCHAR(50) NOT NULL
    {"src_port",  COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"dst_ip",    COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"dst6_ip",    COLUMN_TYPE_TEXT},       //VCHAR(50) NOT NULL
    {"dst_port",  COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"up_bytes",  COLUMN_TYPE_UINT64},     //UNSIGNED BIG INT
    {"up_speed",  COLUMN_TYPE_INT64},      //UNSIGNED BIG INT
    {"up_dif_bytes",  COLUMN_TYPE_UINT64},      //UNSIGNED BIG INT
    {"dn_bytes",  COLUMN_TYPE_UINT64},     //UNSIGNED BIG INT
    {"dn_speed",  COLUMN_TYPE_INT64},      //UNSIGNED BIG INT
    {"dn_dif_bytes",  COLUMN_TYPE_UINT64},      //UNSIGNED BIG INT
    {"phy_type",  COLUMN_TYPE_INT},        //INTEGER NOT NULL
    {"phy_port",  COLUMN_TYPE_INT},        //INTEGER NOT NULL
    {"src_mac",   COLUMN_TYPE_TEXT},       //VARCHAR(20) NOT NULL
};

sql_column_prototype_t sum_columns[] = {
    {"timestamp",    COLUMN_TYPE_INT},     //INTEGER NOT NULL
    {"is_v4",       COLUMN_TYPE_UINT},    //UNSIGNED INTEGER NOT NULL
    {"src_ip",       COLUMN_TYPE_UINT},    //UNSIGNED INTEGER NOT NULL
    {"src6_ip",       COLUMN_TYPE_TEXT},    //VARCHAR(50) NOT NULL
    {"src_mac",      COLUMN_TYPE_TEXT},    //VARCHAR(20) NOT NULL
    {"up_ttl_bytes", COLUMN_TYPE_UINT64},  //UNSIGNED BIG INT
    {"up_dif_bytes", COLUMN_TYPE_UINT64},  //UNSIGNED BIG INT
    {"up_speed",     COLUMN_TYPE_INT64},   //UNSIGNED BIG INT
    {"dn_ttl_bytes", COLUMN_TYPE_UINT64},  //UNSIGNED BIG INT
    {"dn_dif_bytes", COLUMN_TYPE_UINT64},  //UNSIGNED BIG INT
    {"dn_speed",     COLUMN_TYPE_INT64},   //UNSIGNED BIG INT
    {"ifname",       COLUMN_TYPE_TEXT},
    {"is_guest",     COLUMN_TYPE_INT},
    {"phy_type",     COLUMN_TYPE_INT},     //INTEGER NOT NULL
    {"phy_port",     COLUMN_TYPE_INT},     //INTEGER NOT NULL
};

sql_column_prototype_t tcp_columns[] = {
    {"timestamp", COLUMN_TYPE_INT},        //INTEGER NOT NULL
    {"is_v4",     COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"state",     COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"time_in",   COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"src_ip",    COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"src6_ip",    COLUMN_TYPE_TEXT},       //VARCHAR(50) NOT NULL
    {"dst_ip",    COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
    {"dst6_ip",    COLUMN_TYPE_TEXT},       //VARCHAR(50) NOT NULL
    {"dst_port",  COLUMN_TYPE_UINT},       //UNSIGNED INTEGER NOT NULL
};
#endif // defined(CODB)

sqlite3* sqlite_open(NFCM_DB_TYPE db_type, sqlite3 *db, char *fname)
{
    // if fname is existed, delete it
    //if(!access(fname, F_OK))
    //	unlink(fname);

#if defined(CODB)
    char db_ver[MAX_VERSION_LEN];
    db = cosql_open(fname);
    if (!db) {
        info("Can't open database %s", sqlite3_errmsg(db));
        return NULL;
    }

    switch (db_type) {
    case NFCM_DB_APP:
        //printf("sizeof(app_columns)=[%d], sizeof(sql_column_prototype_t)=[%d], app_column_count=[%d]\n",
        //       sizeof(app_columns), sizeof(app_columns[0]), NUM(app_columns));
        cosql_get_db_version(db, db_ver);
        if (strcmp(db_ver, DB_VER)) { // the DB_VER is not the same
            cosql_drop_db(db);
            cosql_create_table(db, DB_VER, NUM(app_columns), app_columns);
        }
        break;

    case NFCM_DB_SUM:
        //printf("sizeof(sum_columns)=[%d], sizeof(sql_column_prototype_t)=[%d], sum_column_count=[%d]\n",
        //       sizeof(sum_columns), sizeof(sum_columns[0]), NUM(sum_columns));
        cosql_get_db_version(db, db_ver);
        if (strcmp(db_ver, DB_VER)) { // the DB_VER is not the same
            cosql_drop_db(db);
            cosql_create_table(db, DB_VER, NUM(sum_columns), sum_columns);
        }
        break;

    case NFCM_DB_TCP:
        //printf("sizeof(tcp_columns)=[%d], sizeof(sql_column_prototype_t)=[%d], tcp_column_count=[%d]\n",
        //       sizeof(tcp_columns), sizeof(tcp_columns[0]), NUM(tcp_columns));
        cosql_get_db_version(db, db_ver);
        if (strcmp(db_ver, DB_VER)) { // the DB_VER is not the same
            cosql_drop_db(db);
            cosql_create_table(db, DB_VER, NUM(tcp_columns), tcp_columns);
        }
        break;
    default:
        break;
    }
#else // !defined(CODB)
    int ret;
    char *err;

    if ((ret = sqlite3_open(fname, &db)) != SQLITE_OK) {
        info("Can't open database %s", sqlite3_errmsg(db));
        return NULL;
    }

    switch (db_type) {
    case NFCM_DB_APP: // nfcm_app
        // create nfcm_app table
        ret = sqlite3_exec(db,
                           "CREATE TABLE IF NOT EXISTS nfcm_app ("
                           "timestamp INTEGER NOT NULL,"
                           "is_v4 UNSIGNED INTEGER NOT NULL,"
                           "proto UNSIGNED INTEGER,"
                           "src_ip UNSIGNED INTEGER NOT NULL,"
                           "src6_ip VARCHAR(50) NOT NULL,"
                           "src_port UNSIGNED INTEGER NOT NULL,"
                           "dst_ip UNSIGNED INTEGER NOT NULL,"
                           "dst6_ip VARCHAR(50) NOT NULL,"
                           "dst_port UNSIGNED INTEGER NOT NULL,"
                           "up_bytes UNSIGNED BIG INT,"
                           "up_speed UNSIGNED BIG INT,"
                           "up_dif_bytes UNSIGNED BIG INT,"
                           "dn_bytes UNSIGNED BIG INT,"
                           "dn_speed UNSIGNED BIG INT,"
                           "dn_dif_bytes UNSIGNED BIG INT,"
                           "phy_type INTEGER NOT NULL,"
                           "phy_port INTEGER NOT NULL,"
                           "src_mac VARCHAR(20) NOT NULL,"
                           "PRIMARY KEY (timestamp, proto, src_ip, src_port, dst_ip, dst_port))",
                           NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("nfcm_app create table error: %s\n", err);
            sqlite3_free(err);
        }
#if defined(__SQL_INDEX__)
        ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS time_src_dst ON nfcm_app (timestamp, src_ip, dst_ip)", NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("nfcm_app create index error: %s\n", err);
            sqlite3_free(err);
        }
#endif
        break;

    case NFCM_DB_SUM: // nfcm_sum
        // create nfcm_sum table
        ret = sqlite3_exec(db,
                           "CREATE TABLE IF NOT EXISTS nfcm_sum ("
                           "timestamp INTEGER NOT NULL,"
                           "is_v4 UNSIGNED INTEGER NOT NULL,"
                           "src_ip UNSIGNED INTEGER NOT NULL,"
                           "src6_ip VARCHAR(50) NOT NULL,"
                           "src_mac VARCHAR(20) NOT NULL,"
                           "up_bytes UNSIGNED BIG INT,"
                           "up_ttl_bytes UNSIGNED BIG INT,"
                           "up_speed UNSIGNED BIG INT,"
                           "dn_bytes UNSIGNED BIG INT,"
                           "dn_ttl_bytes UNSIGNED BIG INT,"
                           "dn_speed UNSIGNED BIG INT,"
                           "phy_type INTEGER NOT NULL,"
                           "phy_port INTEGER NOT NULL,"
                           "PRIMARY KEY (timestamp, src_ip))",
                           NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL)  {
			printf("nfcm_sum create table error: %s\n", err);
			sqlite3_free(err);
		}
#if defined(__SQL_INDEX__)
        ret = sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS time_srcip ON nfcm_sum (timestamp, src_ip)", NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("nfcm_sum create index error: %s\n", err);
            sqlite3_free(err);
        }
#endif
        break;

    case NFCM_DB_TCP: // tcp not ok table
        // create nfcm_tcp table
        ret = sqlite3_exec(db,
                           "CREATE TABLE IF NOT EXISTS nfcm_tcp ("
                           "timestamp INTEGER NOT NULL,"
                           "is_v4 UNSIGNED INTEGER NOT NULL,"
                           "state UNSIGNED INTEGER NOT NULL,"
                           "time_in UNSIGNED INTEGER NOT NULL,"
                           "src_ip UNSIGNED INTEGER NOT NULL,"
                           "src6_ip VARCHAR(50) NOT NULL,"
                           "dst_ip UNSIGNED INTEGER NOT NULL,"
                           "dst6_ip VARCHAR(50) NOT NULL,"
                           "dst_port UNSIGNED INTEGER NOT NULL,"
                           "PRIMARY KEY (timestamp, src_ip, dst_ip, dst_port))",
                           NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL)  {
            printf("nfcm_tcp create table error: %s\n", err);
            sqlite3_free(err);
        }
        break;

    default:
        printf("\n\ninvalid db_type %d\n\n", db_type);
        return NULL;
    }
#endif // !defined(CODB)

    return db;
}

void to_upper(char* string)
{
    const char OFFSET = 'a' - 'A';
    while (*string)
    {
        *string = (*string >= 'a' && *string <= 'z') ? *string -= OFFSET : *string;
        string++;
    }
}

#if defined(CODB)
int nf_node_app_columns_format(time_t timestamp, sql_column_t *cols, nf_node_t *nn)
{
    if (!nn || !cols)
        return -1;

    cols[0].name = "timestamp"; cols[0].type = COLUMN_TYPE_INT;    cols[0].value.i    = timestamp;
    cols[1].name = "proto";     cols[1].type = COLUMN_TYPE_UINT;   cols[1].value.ui   = nn->proto;
    cols[2].name = "is_v4";     cols[2].type = COLUMN_TYPE_UINT;   cols[2].value.ui   = nn->isv4;
    cols[3].name = "src_ip";    cols[3].type = COLUMN_TYPE_UINT;   cols[3].value.ui   = nn->srcv4.s_addr;
    cols[4].name = "src6_ip";    cols[4].type = COLUMN_TYPE_TEXT;   cols[4].value.t   = nn->src6_ip;
    cols[5].name = "src_port";  cols[5].type = COLUMN_TYPE_UINT;   cols[5].value.ui   = nn->src_port;
    cols[6].name = "dst_ip";    cols[6].type = COLUMN_TYPE_UINT;   cols[6].value.ui   = nn->dstv4.s_addr;
    cols[7].name = "dst6_ip";    cols[7].type = COLUMN_TYPE_TEXT;   cols[7].value.t   = nn->dst6_ip;
    cols[8].name = "dst_port";  cols[8].type = COLUMN_TYPE_UINT;   cols[8].value.ui   = nn->dst_port;
    cols[9].name = "up_bytes";  cols[9].type = COLUMN_TYPE_UINT64; cols[9].value.ui64 = nn->up_bytes;
    cols[10].name = "up_speed";  cols[10].type = COLUMN_TYPE_UINT64; cols[10].value.ui64 = nn->up_speed;
    cols[11].name = "up_dif_bytes";  cols[11].type = COLUMN_TYPE_UINT64; cols[11].value.ui64 = nn->up_dif_bytes;
    cols[12].name = "dn_bytes";  cols[12].type = COLUMN_TYPE_UINT64; cols[12].value.ui64 = nn->dn_bytes;
    cols[13].name = "dn_speed";  cols[13].type = COLUMN_TYPE_UINT64; cols[13].value.ui64 = nn->dn_speed;
    cols[14].name = "dn_dif_bytes";  cols[14].type = COLUMN_TYPE_UINT64; cols[14].value.ui64 = nn->dn_dif_bytes;
    cols[15].name = "phy_type"; cols[15].type = COLUMN_TYPE_INT;   cols[15].value.i   = nn->layer1_info.eth_type;
    cols[16].name = "phy_port"; cols[16].type = COLUMN_TYPE_INT;   cols[16].value.i   = nn->layer1_info.eth_port;
    cols[17].name = "src_mac"; cols[17].type = COLUMN_TYPE_TEXT;   cols[17].value.t   = nn->src_mac;

    return 0;
}
#endif

int sqlite_app_insert(sqlite3 *db, struct list_head *iplist)
{
    nf_node_t *nn;
    //char src_ipstr[INET6_ADDRSTRLEN];
    //char dst_ipstr[INET6_ADDRSTRLEN];

#if defined(CODB)
    sql_column_t cols[NUM(app_columns)];
#else
    int ret;
    char *err = NULL;
    char datetime[32];
    char sql[512];
#endif

    time_t timestamp = time(NULL);
    //get_date_time(0, datetime, sizeof(datetime));

    list_for_each_entry(nn, iplist, list) {
        if (nn->isv4) {
            if (!is_acceptable_addr(nn)) continue;
            /*
            if (!is_in_lanv4(&nn->srcv4)) continue;
            if (is_multi_addr(&nn->dstv4)) continue;
            if (nn->dst_port == 53) continue; // DNS query
            */

            //inet_ntop(AF_INET, &nn->srcv4, src_ipstr, INET_ADDRSTRLEN);
            //inet_ntop(AF_INET, &nn->dstv4, dst_ipstr, INET_ADDRSTRLEN);
#if defined(__IPV6__)
        } else {
            if (!is_acceptable_addr(nn)) continue;
            inet_ntop(AF_INET6, &nn->srcv6, src_ipstr, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &nn->dstv6, dst_ipstr, INET6_ADDRSTRLEN);
#endif
        }
	
	to_upper(nn->src_mac);
#if defined(CODB)
        nf_node_app_columns_format(timestamp, cols, nn);
        cosql_insert_table(db, NUM(app_columns), cols);
#else
        sprintf(sql, "INSERT INTO nfcm_app VALUES ("
                "%ld, %u, %u, "
                "%u, '%s', %u, "
                "%u, '%s', %u, "
                "%llu, %llu, %llu, "
                "%llu, %llu, %llu, "
                "%d, %d, '%s')",
                timestamp, nn->isv4, nn->proto,
                nn->srcv4.s_addr, nn->src6_ip, nn->src_port,
                nn->dstv4.s_addr, nn->dst6_ip, nn->dst_port,
                nn->up_bytes, nn->up_speed, nn->up_dif_bytes,
                nn->dn_bytes, nn->dn_speed, nn->dn_dif_bytes,
                nn->layer1_info.eth_type, nn->layer1_info.eth_port, nn->src_mac);
        //info("sql=[%s]", sql);
        ret = sqlite3_exec(db, sql, NULL, NULL, &err);
        if (ret != SQLITE_OK) {
            if (err != NULL) {
				printf("error: %s\n", err);
                sqlite3_free(err);
            }
        }
#endif
    }

    return 0;
}

#if defined(CODB)
int nf_node_sum_columns_format(time_t timestamp, sql_column_t *cols, nf_node_t *nn)
{
    if (!nn || !cols)
        return -1;

    cols[0].name = "timestamp";    cols[0].type = COLUMN_TYPE_INT;    cols[0].value.i    = timestamp;
    cols[1].name = "is_v4";        cols[1].type = COLUMN_TYPE_UINT;   cols[1].value.ui   = nn->isv4;
    cols[2].name = "src_ip";       cols[2].type = COLUMN_TYPE_UINT;   cols[2].value.ui   = nn->srcv4.s_addr;
    cols[3].name = "src6_ip";       cols[3].type = COLUMN_TYPE_TEXT;   cols[3].value.t   = nn->src6_ip;
    cols[4].name = "src_mac";      cols[4].type = COLUMN_TYPE_TEXT;   cols[4].value.t    = nn->src_mac;
    cols[5].name = "up_ttl_bytes"; cols[5].type = COLUMN_TYPE_UINT64; cols[5].value.ui64 = nn->up_ttl_bytes;
    cols[6].name = "up_dif_bytes"; cols[6].type = COLUMN_TYPE_UINT64; cols[6].value.ui64 = nn->up_dif_bytes;
    cols[7].name = "up_speed";     cols[7].type = COLUMN_TYPE_INT64;  cols[7].value.i64  = (int64_t)(nn->up_speed/nf_conntrack_period);
    cols[8].name = "dn_ttl_bytes"; cols[8].type = COLUMN_TYPE_UINT64; cols[8].value.ui64 = nn->dn_ttl_bytes;
    cols[9].name = "dn_dif_bytes"; cols[9].type = COLUMN_TYPE_UINT64; cols[9].value.ui64 = nn->dn_dif_bytes;
    cols[10].name = "dn_speed";     cols[10].type = COLUMN_TYPE_INT64;  cols[10].value.i64  = (int64_t)(nn->dn_speed/nf_conntrack_period);
    cols[11].name = "ifname";       cols[11].type = COLUMN_TYPE_TEXT;   cols[11].value.t    = nn->layer1_info.ifname;
    cols[12].name = "is_guest";    cols[12].type = COLUMN_TYPE_INT;   cols[12].value.i   = nn->layer1_info.is_guest;
    cols[13].name = "phy_type";    cols[13].type = COLUMN_TYPE_INT;   cols[13].value.i   = nn->layer1_info.eth_type;
    cols[14].name = "phy_port";    cols[14].type = COLUMN_TYPE_INT;   cols[14].value.i   = nn->layer1_info.eth_port;
    return 0;
}
#endif

int sqlite_sum_insert(sqlite3 *db, struct list_head *smlist)
{
    nf_node_t *nn;
#if defined(CODB)
    sql_column_t cols[NUM(sum_columns)];
#else
    int ret;
    char *err = NULL;
    char sql[512];
#endif
    time_t timestamp = time(NULL);

    list_for_each_entry(nn, smlist, list) {
#if defined(CODB)
        nf_node_sum_columns_format(timestamp, cols, nn);
        cosql_insert_table(db, NUM(sum_columns), cols);
#else
        sprintf(sql, "INSERT INTO nfcm_sum VALUES ("
                     "%ld, %u, %u, '%s', '%s',"
                     "%llu, %llu, %llu, "
                     "%llu, %llu, %llu, "
                     "%d, %d)",
                timestamp, nn->isv4, nn->srcv4.s_addr, nn->src6_ip, nn->src_mac,
                nn->up_ttl_bytes, nn->up_dif_bytes, nn->up_speed/nf_conntrack_period,
                nn->dn_ttl_bytes, nn->dn_dif_bytes, nn->dn_speed/nf_conntrack_period,
                nn->layer1_info.eth_type, nn->layer1_info.eth_port);
        //info("sql=[%s]", sql);
        ret = sqlite3_exec(db, sql, NULL, NULL, &err);
        if (ret != SQLITE_OK) {
            if (err != NULL) {
				printf("error: %s\n", err);
                sqlite3_free(err);
            }
        }
#endif
    }

    return 0;
}

#if defined(CODB)
int nf_node_tcp_columns_format(time_t timestamp, sql_column_t *cols, nf_node_t *nn)
{
    if (!nn || !cols)
        return -1;

    cols[0].name = "timestamp"; cols[0].type = COLUMN_TYPE_INT;  cols[0].value.i  = timestamp;
    cols[1].name = "is_v4";     cols[1].type = COLUMN_TYPE_UINT; cols[1].value.ui = nn->isv4;
    cols[2].name = "state";     cols[2].type = COLUMN_TYPE_UINT; cols[2].value.ui = nn->state;
    cols[3].name = "time_in";   cols[3].type = COLUMN_TYPE_UINT; cols[3].value.ui = nn->time_in;
    cols[4].name = "src_ip";    cols[4].type = COLUMN_TYPE_UINT; cols[4].value.ui = nn->srcv4.s_addr;
    cols[5].name = "src6_ip";   cols[5].type = COLUMN_TYPE_TEXT; cols[5].value.t = nn->src6_ip;
    cols[6].name = "dst_ip";    cols[6].type = COLUMN_TYPE_UINT; cols[6].value.ui = nn->dstv4.s_addr;
    cols[7].name = "dst6_ip";   cols[7].type = COLUMN_TYPE_TEXT; cols[7].value.t = nn->dst6_ip;
    cols[8].name = "dst_port";  cols[8].type = COLUMN_TYPE_UINT; cols[8].value.ui = nn->dst_port;

    return 0;
}
#endif

int sqlite_tcp_insert(sqlite3 *db, struct list_head *tbklist)
{
    nf_node_t *nn;
#if defined(CODB)
    sql_column_t cols[NUM(tcp_columns)];
#else
    int ret;
    char *err = NULL;
    char sql[512];
#endif

    //char srcip[INET_ADDRSTRLEN];
    //char dstip[INET_ADDRSTRLEN];

    time_t timestamp = time(NULL);

    list_for_each_entry(nn, tbklist, list) {
#if defined(CODB)
        nf_node_tcp_columns_format(timestamp, cols, nn);
        cosql_insert_table(db, NUM(tcp_columns), cols);
#else
        //inet_ntop(AF_INET, &nn->srcv4, srcip, INET_ADDRSTRLEN);
        //inet_ntop(AF_INET, &nn->dstv4, dstip, INET_ADDRSTRLEN);
        //printf("TCP: %s[%u] --[%u][%ld]--> %s[%u]\n", srcip, nn->src_port, nn->state, nn->time_in, dstip, nn->dst_port);
        sprintf(sql, "INSERT INTO nfcm_tcp VALUES ("
                "%ld, %u, %u, %ld, %u, '%s', %u, '%s', %u)",
                timestamp, nn->isv4, nn->state, nn->time_in,
                nn->srcv4.s_addr, nn->src6_ip, nn->dstv4.s_addr, nn->dst6_ip, nn->dst_port);
        //info("sql=[%s]", sql);
        ret = sqlite3_exec(db, sql, NULL, NULL, &err);
        if (ret != SQLITE_OK) {
            if (err != NULL) {
                printf("error: %s\n", err);
                sqlite3_free(err);
            }
        }
#endif
    }

    return 0;
}

#if !defined(CODB)
static int select_db_timestamp_cb(void *data, int argc, char **argv, char **azColName)
{
    int i;
    time_t *timestamp = (time_t *)data;

    switch (sizeof(time_t)) {
    case 4:
        *timestamp = atoi(argv[0]) & 0xffffffff;
        break;
    case 8:
        *timestamp = atoi(argv[0]) & 0xffffffffffffffff;
        break;
    }

#if 0
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n======================\n");
#endif

    return 0;
}

int sqlite_db_timestamp_select(sqlite3 *db, time_t *timestamp, char *sql)
{
    int ret;
    char *err = NULL;

    //info("sql=[%s]", sql);
    ret = sqlite3_exec(db, sql, select_db_timestamp_cb, (void *)timestamp, &err);
    if (ret != SQLITE_OK && err != NULL) sqlite3_free(err);

    return 0;
}
#endif

int sqlite_get_timestamp(sqlite3 *db, time_t *timestamp)
{
#if defined(CODB)
    int ret_rows = 0;
    char **result;
    int i,j;
    sql_column_prototype_t query_columns[1];
    int query_columns_count = 1;

    int start_index;
    int array_index;
    char* column_value;

    query_columns[0].name = "timestamp";

    cosql_get_column_values(db,
        0, NULL,
        0, NULL, 
        query_columns_count, query_columns,
        0, 0,
        "timestamp",
        "ASC",
        1,
        &ret_rows,
        &result);

    for (i=0; i<ret_rows; i++) {
        for (j=0; j<query_columns_count; j++) {
            start_index = query_columns_count + i*query_columns_count;
            array_index = start_index + j;
            column_value = result[array_index];
            *timestamp = atoi(column_value);
            //printf("row index=%d, column_index=%d, array_index=%d, column_value=%s\n", i, j, array_index, column_value);
        }
    }

    //- free result
    cosql_free_column_values(result);
#else
    sqlite_db_timestamp_select(db, timestamp, "select timestamp from nfcm_app order by timestamp asc limit 1;");
#endif

    return 0;
}

int sqlite_close(sqlite3 *db)
{
#if defined(CODB)
    if (db != NULL)
        cosql_close(db);
#else
    if (db != NULL)
        sqlite3_close(db);
#endif

    db = NULL;
    return 0;
}
