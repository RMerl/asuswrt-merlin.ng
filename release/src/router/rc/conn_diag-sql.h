#include <sqlite3.h>
#include <pthread.h>

#define RTCONFIG_UPLOADER

#define DIAG_TAB_NAME "conn_diag"
#define DATA_TAB_NAME "diag_data"
#define MAX_DB_SIZE 4194304 // 4MB
#define MAX_DATA 1024
#define MAX_DB_COUNT 2

#define SYS_DIR         "/jffs/.sys"
#define DIAG_DB_DIR     SYS_DIR"/diag_db"
#ifdef RTCONFIG_UPLOADER
#define DIAG_CLOUD_DIR  "/tmp/diag_db_cloud"
#define DIAG_CLOUD_UPLOAD  DIAG_CLOUD_DIR"/upload"
#define DIAG_CLOUD_DOWNLOAD  DIAG_CLOUD_DIR"/download"
#endif

enum {
	INIT_DB_NO=0,
	INIT_DB_YES,
	INIT_DB_CLOUD,
	INIT_DB_MAX
};

typedef struct _json_result json_result_t;
struct _json_result {
	char db_path[PATH_MAX];
	int row_count;
	int col_count;
	char **result;
	json_result_t *next;
};

#if 0
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
#endif

int diag_dbg;
int diag_syslog;
int diag_max_db_size;
int diag_max_db_count;
int diag_portinfo;

#define LOG_TITLE_CHK "CHKSTA"
#define LOG_TITLE_DIAG "CONNDIAG"


#define CHK_LOG(LV, fmt, arg...) \
	do { \
		if(diag_dbg >= LV) \
			_dprintf("%lu: "fmt"\n", time(NULL), ##arg); \
		if(diag_syslog >= LV) \
			syslog(LV, "%s: "fmt, LOG_TITLE_CHK, ##arg); \
	} while (0)

#define DIAG_LOG(LV, fmt, arg...) \
	do { \
		if(diag_dbg >= LV) \
			_dprintf("%lu: "fmt"\n", time(NULL), ##arg); \
		if(diag_syslog >= LV) \
			syslog(LV, "%s: "fmt, LOG_TITLE_DIAG, ##arg); \
	} while (0)

#ifdef RTCONFIG_LIBASUSLOG
#define DBG_CHK_DATA "chksta_data.log"

#define CHK_DATA(fmt, arg...) \
	do { \
		asusdebuglog(0, DBG_CHK_DATA, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
	} while (0)
#else
#define CHK_DATA(fmt, arg...) \
	do { \
		syslog(0, "CHKSTA: "fmt, ##arg); \
	} while (0)
#endif


extern void diag_log_status();
extern int get_ts_from_db_name(char *str, unsigned long *ts1, unsigned long *ts2);
extern int save_data_in_sql(const char *event, char *raw);
extern int specific_data_on_day(unsigned long specific_ts, const char *where, int *row_count, int *field_count, char ***raw);
// Get data produced after the specific timestamp. If specific timestamp is 0, get all data of today.
extern int get_sql_on_day(unsigned long specific_ts, const char *event, const char *node_ip, const char *node_mac,
		int *row_count, int *field_count, char ***raw);
// Get data produced after the specific timestamp. If specific timestamp is 0, get all data of today.
extern int get_json_on_day(unsigned long specific_ts, const char *event, const char *node_ip, const char *node_mac,
		int *row_count, int *field_count, char ***raw);
extern int get_json_in_period(unsigned long start_ts, unsigned long end_ts, const char *event, const char *node_ip, const char *node_mac,
		json_result_t **json_result);
extern void free_json_result(json_result_t **json_result);
extern int merge_data_in_sql(const char *dst_file, const char *src_file);
#ifdef RTCONFIG_UPLOADER
extern int run_upload_file_at_ts(unsigned long ts, unsigned long ts2);
extern int run_upload_file_by_name(const char *uploaded_file);
extern int run_download_file_at_ts(unsigned long ts, unsigned long ts2);
extern int run_download_file_by_name(const char *downloaded_file);
#endif
