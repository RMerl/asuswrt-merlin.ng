#include "sqlite3.h"
#include "json.h"
#include "time.h"

#define DAY_SEC     86400
#define QUERY_LEN   960
#define LEN_MAX     32000

#define ID_FILE     "/tmp/bwdpi/bwdpi.rule.db"
#define ID_SIZE     512

// copy from httpd.h
typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)
#define websWrite(wp, fmt, args...) ({ int TMPVAR = fprintf(wp, fmt, ## args); fflush(wp); TMPVAR; })

extern int sql_get_table(sqlite3 *db, const char *sql, char ***pazResult, int *pnRow, int *pnColumn);
extern int hns_sql_exec(sqlite3 *db, const char *sql);
extern int hns_integrity_check(sqlite3 *db, char *db_path);
extern int hns_remove_journal(const char *db_file);
extern void hns_sql_resize(sqlite3 *db, char *db_path, char *type);
extern void hns_json_to_db(sqlite3 *db);
extern void hns_table_result(int *tmp, char **result, int rows, int cols);
extern int hns_get_final_timestamp(sqlite3 *db, int type);
extern int hns_get_count_result(sqlite3 *db, int type);
extern void hns_mac2host(char *mac, int dir, char *src, int src_len, char *dst, int dst_len);
extern void transfer_ID_into_Desc(char *key, char *val, int len_t);
extern time_t Date_Of_Timestamp(time_t now);
extern void hns_history_write_data(sqlite3 *db, char *mac, const char *domain, int time);
extern void hns_history_json_to_db(sqlite3 *db);
extern void hns_protection_write_data(sqlite3 *db, int id, char *mac, int time, const char *url, int dir, const char *sip);
extern void hns_mals_cc_json_to_db(sqlite3 *db);
extern void hns_vp_json_to_db(sqlite3 *db);

/* hook */
extern void get_web_hook(char *mac, char *page, char *num, int *retval, webs_t wp);
extern void bwdpi_maclist_db(char *type, int *retval, webs_t wp);
extern void bwdpi_monitor_stat(int *retval, webs_t wp);
extern void bwdpi_monitor_info(char *type, char *event, int *retval, webs_t wp);
extern void bwdpi_monitor_ips(char *type, char *date, int *retval, webs_t wp);
extern void bwdpi_monitor_nonips(char *type, char *date, int *retval, webs_t wp);
extern void hns_cgi_mon_to_json(char *type, char *start, char *end, FILE *stream);
extern void hns_cgi_mon_del_db(char *type, FILE *stream);

/* wrs_wbl */
extern int WRS_WBL_WRITE_LIST(int bflag, char *mac, char *input_type, char *input);
extern int WRS_WBL_DEL_LIST(int bflag, char *mac, char *input_type, char *input);
extern void WRS_WBL_READ_LIST(int bflag, char *mac, FILE *file);
extern int wbl_setup_global_rule(char *mac, json_object *profile);
