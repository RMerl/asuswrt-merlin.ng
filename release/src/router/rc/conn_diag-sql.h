#ifndef _CONN_DIAG_SQL_H_
#define _CONN_DIAG_SQL_H_
#include <limits.h>
#include <sqlite3.h>
#include <pthread.h>
#include <conn_diag_log.h>

#define RTCONFIG_UPLOADER

#define DIAG_TAB_NAME "conn_diag"
#define DATA_TAB_NAME "diag_data"
// #define MAX_DB_SIZE 512000 //500k
#define MAX_DB_SIZE 2097152 // 2M
#define MAX_DATA 8192
#define MAX_DB_COUNT 2
#define DB_BACKUP_NO 0
#define DB_BACKUP_JFFS 1
#define DB_BACKUP_USB 2
#define DB_BACKUP_EXTERNAL 3
#define DB_SIZE_CHECK_PERIOD 10

#define DIAG_DB_FOLDER ".diag"

#define JFFS_DIR         	"/jffs"
#define DIAG_JFFS_DB_DIR     	JFFS_DIR"/"DIAG_DB_FOLDER

#define TMP_DIR	"/tmp"
#define DIAG_TMP_DB_DIR TMP_DIR"/"DIAG_DB_FOLDER

#ifdef RTCONFIG_UPLOADER
#define DIAG_CLOUD_DIR  "/tmp/diag_db_cloud"
#define DIAG_CLOUD_UPLOAD  DIAG_CLOUD_DIR"/upload"
#define DIAG_CLOUD_DOWNLOAD  DIAG_CLOUD_DIR"/download"
#endif

#define DIAG_PORT_STATUS_FILE "/tmp/diag_port_status.json"

int check_if_cable_diag_can_run(void);
enum {
	INIT_DB_NO=0,
	INIT_DB_YES,
	INIT_DB_CLOUD,
	INIT_DB_MAX
};

enum {
	DB_TH_TYPE_NOREPEAT,
	DB_TH_TYPE_HOUR,
	DB_TH_TYPE_MAXNUM,
	DB_TH_TYPE_END
};

#define DIAG_MAX_USB_HUB_PORT 6
#define DIAG_MAX_MOCA_DEVICES 1

#define DIAG_MAX_MOCA_NODES 16
#define DIAG_MAX_MOCA_NUM_CHANNELS	5

typedef struct _DIAG_MOCA_NODE_INFO
{
	int active;
	int node_id;
	char macaddr[18];
	char moca_ver[8];
	int phyrate[DIAG_MAX_MOCA_NODES];
	char node_mac[DIAG_MAX_MOCA_NODES][18];
	char node_moca_ver[DIAG_MAX_MOCA_NODES][8];
	unsigned char rx_snr[DIAG_MAX_MOCA_NODES][DIAG_MAX_MOCA_NUM_CHANNELS];
}DIAG_MOCA_NODE_INFO;

struct amas_eth_port {
	char label_name[8];
	unsigned int cap;
	uint8 is_on;
	unsigned int link_speed;
	unsigned int link_speed_max;
	int brown;
	int brown_len;
	int blue;
	int blue_len;
	int green;
	int green_len;
	int orange;
	int orange_len;
	usb_device_info_t usb_devices[DIAG_MAX_USB_HUB_PORT];
	DIAG_MOCA_NODE_INFO moca_devices[DIAG_MAX_MOCA_DEVICES];
	struct amas_eth_port *next;
	int cable_diag_triger_link_st;
	time_t cmd_time;
	int seq_no;
	char ui_display[32];
	int phy_port_id;
	int ext_port_id;
	char ifname[32];
	unsigned int flag;
};

struct amas_eth_port_table {
	char node_mac[18];
	struct amas_eth_port *portlist;
	struct amas_eth_port_table *next;
	uint8 cable_diag_active;//indicate cable-idag state of this node
};

struct stainfo {
	char sta_mac[18];
	double tx_rate;
	double rx_rate;
	int conn_time;
	int inactive_flag;
	//time_t last_update;
	struct stainfo *next;
};

struct stainfo_table {
	char node_mac[18];
	struct stainfo *stalist;
	struct stainfo_table *next;
};

#define COLUMN_TYPE_MASK 0x0000FFFF
#define COLUMN_IGNORE 0x00010000


#define COLUMN_TYPE_ALL_STR 0xFFFFFFFF //for db_value_types

struct CONNDIAG_DB_t { 
	int mode; int db_type; char *db_name; int db_th_type; int db_th; int db_ext_column_num;
	struct sql_column_prototype *db_sql_columns;
};

//start from column0
#define COLUMN_X_IS_STR(x) (unsigned int)(1 << x) 
#define IS_COLUMN_X_STR(types,x) ((unsigned int)(types >> x) & 1) 

enum {
	DB_NONE=0,
	DB_DEFAULT,
	DB_SYS_DETECT,
	DB_SYS_SETTING,
	DB_WIFI_DETECT,
	DB_WIFI_SETTING,
	DB_STAINFO,
	DB_NET_DETECT,
	DB_ETH_DETECT,
	DB_PORTINFO,
	DB_WIFI_DFS,
	DB_SITE_SURVEY,
	DB_CHANNEL_CHANGE,
	DB_PORT_STATUS_CHANGE,
	DB_CABLEDIAG,
	DB_PORT_STATUS_USB_CHANGE,
	DB_STAINFO_STABLE,
	DB_IPERF_SERVER,
	DB_IPERF_CLIENT,
	DB_WLC_EVENT,
	DB_WIFI_CBP,
        DB_PORT_STATUS_MOCA_CHANGE,
	DB_MAX
};

#define CABLEDIAG_STATUS_RUN -2
//start from 0
#define CABLEDIAG_STATUS_INVALID CD_INVALID
#define CABLEDIAG_STATUS_OK CD_OK
#define CABLEDIAG_STATUS_OPEN CD_OPEN
#define CABLEDIAG_STATUS_INTRA_SHORT CD_INTRA_SHORT
#define CABLEDIAG_STATUS_INTER_SHORT CD_INTER_SHORT
#define CABLEDIAG_STATUS_ENABLED CD_ENABLED
#define CABLEDIAG_STATUS_DISABLED CD_DISABLED
#define CABLEDIAG_STATUS_NOT_SUPPORTED CD_NOT_SUPPORTED

#define DB_TYPE_MASK 0x0000FFFF
#define DB_SUBTYPE_MASK 0xFFFF0000

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
#define LOG_TITLE_DIAG_SQL "CONNDIAGSQL"
#define CONNDIAG_DBG 1

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
		if(CONNDIAG_DBG) \
			Cdbg(CONNDIAG_DBG, fmt, ##arg);	\
	} while (0)
    // CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);

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
extern int save_data_in_sql(const char *event, char *raw,int db_type);
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
extern int is_valid_event(const char *name);
#endif
extern unsigned long get_mem_info(char *name);
extern int special_alphasort(const void *d1, const void *d2);
extern struct CONNDIAG_DB_t *find_db_profile_by_mode_and_version(int db_mode,char *version);
extern struct CONNDIAG_DB_t *find_db_profile_by_type_and_version(int db_type,char *version);
extern int diagmode_to_dbidx(int mode);
extern int get_node_eth_port_status(char *node_mac,char **buf);
extern void free_node_eth_port_status(char **buf);
extern int query_stainfo(char *sta_mac,char **buf);
extern int get_wifi_txrxbyte_avg(char *bandmac,char *mac,double *txbyte,double *rxbyte,int diff_range);
extern int get_eth_txrxbyte_avg(int is_bh,char *mac,double *txbyte,double *rxbyte,int diff_range);
extern int get_ethphy_txrxbyte_avg(int is_bh,char *mac,double *txbyte,double *rxbyte,int diff_range);
extern int get_sta_txrxbyte_avg(char *sta_mac,char *mac,double *txbyte,double *rxbyte,int diff_range);
extern int get_staphy_txrxbyte_avg(char *sta_mac,char *mac,double *txbyte,double *rxbyte,int diff_range);
extern int exec_force_cable_diag(char *node_mac,char *label_name);
extern int exec_wifi_dfs_diag(char *json_data);
#ifdef RTCONFIG_CD_IPERF
extern int exec_iperf(char* caller, char *server_mac,char *client_mac);
#endif

extern int query_stainfo(char *sta_mac,char **buf);
extern void free_stainfo(char **buf);

struct json_object* get_byte_field_string_json_object(unsigned char value, char *buf, int buf_len);
struct json_object* get_int_field_string_json_object(int value, char *buf, int buf_len);
struct json_object* get_uint_field_string_json_object(unsigned int value, char *buf, int buf_len);
struct json_object* get_uint64_field_string_json_object(unsigned long long value, char *buf, int buf_len);
struct json_object* get_rate_field_string_json_object(double value, char *buf, int buf_len);
extern int _get_node_eth_port_status(char *node_mac,char **buf);

#ifdef RTCONFIG_AWSIOT
extern int wifi_dfs_on_all_channels_process();
#endif
#endif	/* !_CONN_DIAG_SQL_H_ */
