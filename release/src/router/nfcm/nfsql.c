#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "list.h"
#include "nfcm.h"
#include "nfsql.h"

void get_date_time(char* buff, int len)
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buff, len, "%Y-%m-%d %H:%M:%S", timeinfo);
}

sqlite3 *sqlite_open(bool is_app, sqlite3 *db, char *fname) 
{
	int ret;
	char *err;

    // if fname is existed, delete it
    //if(!access(fname, F_OK))
    //	unlink(fname);

    if((ret = sqlite3_open(fname, &db)) != SQLITE_OK) {
        info("Can't open database %s", sqlite3_errmsg(db));
        return NULL;
    }

    if (is_app) {
        // create nfcm_app table
        ret = sqlite3_exec(db,
                "CREATE TABLE IF NOT EXISTS nfcm_app ("
                "timestamp INTEGER NOT NULL,"
                "datetime VARCHAR(30) NOT NULL,"
                "proto INTEGER,"
                "src_ip VARCHAR(20) NOT NULL,"
                "src_port INTEGER,"
                "dst_ip VARCHAR(20) NOT NULL,"
                "dst_port INTEGER,"
                "up_packets UNSIGNED BIG INT,"
                "up_bytes UNSIGNED BIG INT,"
                "up_speed UNSIGNED BIG INT,"
                "dn_packets UNSIGNED BIG INT,"
                "dn_bytes UNSIGNED BIG INT,"
                "dn_speed UNSIGNED BIG INT,"
                "phy_type INTEGER NOT NULL,"
                "phy_port INTEGER NOT NULL,"
                "PRIMARY KEY (timestamp, src_ip, dst_ip, dst_port))",
                NULL, NULL, &err);
        if(ret != SQLITE_OK && err != NULL) 
            sqlite3_free(err);

        ret = sqlite3_exec(db, "CREATE INDEX timestamp ON nfcm_app (timestamp ASC)", NULL, NULL, &err);
        if(ret != SQLITE_OK && err != NULL) 
            sqlite3_free(err);

        ret = sqlite3_exec(db, "CREATE INDEX src_ip ON nfcm_app (src_ip ASC)", NULL, NULL, &err);
        if(ret != SQLITE_OK && err != NULL) 
            sqlite3_free(err);
    } else {
        // create nfcm_sum table
        ret = sqlite3_exec(db,
                "CREATE TABLE IF NOT EXISTS nfcm_sum ("
                "timestamp INTEGER NOT NULL,"
                "datetime VARCHAR(30) NOT NULL,"
                "src_ip VARCHAR(20) NOT NULL,"
                "up_packets UNSIGNED BIG INT,"
                "up_bytes UNSIGNED BIG INT,"
                "up_ttl_bytes UNSIGNED BIG INT,"
                "up_speed UNSIGNED BIG INT,"
                "dn_packets UNSIGNED BIG INT,"
                "dn_bytes UNSIGNED BIG INT,"
                "dn_ttl_bytes UNSIGNED BIG INT,"
                "dn_speed UNSIGNED BIG INT,"
                "phy_type INTEGER NOT NULL,"
                "phy_port INTEGER NOT NULL,"
                "PRIMARY KEY (timestamp, src_ip))",
                NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL)  
            sqlite3_free(err);

        ret = sqlite3_exec(db, "CREATE INDEX timestamp ON nfcm_sum (timestamp ASC)", NULL, NULL, &err);
        if(ret != SQLITE_OK && err != NULL) 
            sqlite3_free(err);

        ret = sqlite3_exec(db, "CREATE INDEX src_ip ON nfcm_sum (src_ip ASC)", NULL, NULL, &err);
        if(ret != SQLITE_OK && err != NULL) 
            sqlite3_free(err);
    }

	return db;
}

int sqlite_app_insert(sqlite3 *db, struct list_head *iplist)
{
	int ret;
	char *err = NULL;
	char datetime[32];
	char sql[512];
	nf_node_t *nn;
	char src_ipstr[INET6_ADDRSTRLEN];
    char dst_ipstr[INET6_ADDRSTRLEN];

	get_date_time(datetime, sizeof(datetime));

	list_for_each_entry(nn, iplist, list) {
		if (nn->isv4) {
			if (!is_in_lanv4(&nn->srcv4))
				continue;
			inet_ntop(AF_INET, &nn->srcv4, src_ipstr, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &nn->dstv4, dst_ipstr, INET_ADDRSTRLEN);
		} else {
			inet_ntop(AF_INET6, &nn->srcv6, src_ipstr, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &nn->dstv6, dst_ipstr, INET6_ADDRSTRLEN);
		}
        sprintf(sql, "INSERT INTO nfcm_app VALUES (%ld, '%s', '%d', '%s', '%d', '%s', '%d', "
                     "%llu, %llu, %llu, %llu, %llu, %llu, %d, %d)",
                    time(NULL), datetime, 
                    nn->proto, src_ipstr, nn->src_port, dst_ipstr, nn->dst_port,
                    nn->up_pkts, nn->up_bytes, nn->up_speed, 
                    nn->dn_pkts, nn->dn_bytes, nn->dn_speed, 
                    nn->layer1_info.eth_type, nn->layer1_info.eth_port);
		//info("sql=[%s]", sql);
		ret = sqlite3_exec(db, sql, NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			if(err != NULL) {
				sqlite3_free(err);
			}
		}
	}

	return 0;
}

int sqlite_sum_insert(sqlite3 *db, struct list_head *smlist)
{
	int ret;
	char *err = NULL;
	char datetime[32];
	char sql[512];
	nf_node_t *nn;
	char src_ipstr[INET6_ADDRSTRLEN];
    char dst_ipstr[INET6_ADDRSTRLEN];

	get_date_time(datetime, sizeof(datetime));

	list_for_each_entry(nn, smlist, list) {
		if (nn->isv4) {
			if (!is_in_lanv4(&nn->srcv4))
				continue;
			inet_ntop(AF_INET, &nn->srcv4, src_ipstr, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &nn->dstv4, dst_ipstr, INET_ADDRSTRLEN);
		} else {
			inet_ntop(AF_INET6, &nn->srcv6, src_ipstr, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &nn->dstv6, dst_ipstr, INET6_ADDRSTRLEN);
		}
        sprintf(sql, "INSERT INTO nfcm_sum VALUES (%ld, '%s', '%s', "
                     "%llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %d, %d)",
                    time(NULL), datetime, src_ipstr,
                    nn->up_pkts, nn->up_bytes, nn->up_ttl_bytes, nn->up_speed, 
                    nn->dn_pkts, nn->dn_bytes, nn->dn_ttl_bytes, nn->dn_speed,
                    nn->layer1_info.eth_type, nn->layer1_info.eth_port);
		//info("sql=[%s]", sql);
		ret = sqlite3_exec(db, sql, NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			if(err != NULL) {
				sqlite3_free(err);
			}
		}
	}

	return 0;
}

static int select_cb(void *data, int argc, char **argv, char **azColName)
{
    int i;

    printf("%s: \n", (const char*)data);

    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n======================\n");

    return 0;
}

int sqlite_select(sqlite3 *db, char *sql)
{
	int ret;
	char *err = NULL;
	const char *data = "Callback function called";

	info("sql=[%s]", sql);
	ret = sqlite3_exec(db, sql, select_cb, (void *)data, &err);
	if (ret != SQLITE_OK && err != NULL) 
			sqlite3_free(err);

	return 0;
}

int sqlite_close(sqlite3 *db) {
	if (db != NULL)
		sqlite3_close(db);

	return 0;
}
