 /*
 * Copyright 2017, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/*
	2017/04/12 lantiq : include/list.h will conflict with trendmicro's list.h, so move some defination and extern function here
*/

/* DEBUG DEFINE */
#define BWDPI_DEBUG             "/tmp/BWDPI_DEBUG"
#define BWDPI_SIG_DEBUG         "/tmp/BWSIG_DEBUG"
#define BWDPI_SQLITE_DEBUG      "/tmp/BWSQL_DEBUG"
#define BWDPI_SQLITE_DELOG      "/tmp/BWSQL_LOG"
#define BWDPI_MON_DEBUG         "/tmp/BWMON_DEBUG"
#define BWDPI_MON_DELOG         "/tmp/BWMON_LOG"
#define BWDPI_SUP_DEBUG         "/tmp/BWSUP_DEBUG"

/* DEBUG FUNCTION */
#define BWDPI_DBG(fmt,args...) \
	if(f_exists(BWDPI_DEBUG) > 0) { \
		printf("[BWDPI][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define BWSIG_DBG(fmt,args...) \
	if(f_exists(BWDPI_SIG_DEBUG) > 0) { \
		dbg("[BWSIG][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define BWSQL_DBG(fmt,args...) \
	if(f_exists(BWDPI_SQLITE_DEBUG) > 0) { \
		dbg("[BWSQL][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define BWSQL_LOG(fmt,args...) \
	if(f_exists(BWDPI_SQLITE_DELOG) > 0) { \
		char info[1024]; \
		snprintf(info, sizeof(info), "echo \"[BWDPI_SQLITE]"fmt"\" >> /tmp/BWSQL.log", ##args); \
		system(info); \
	}

#define BWMON_DBG(fmt,args...) \
	if(f_exists(BWDPI_MON_DEBUG) > 0) { \
		dbg("[BWMON][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define BWMON_LOG(fmt,args...) \
	if(f_exists(BWDPI_MON_DELOG) > 0) { \
		char info[1024]; \
		snprintf(info, sizeof(info), "echo \"[BWMON]"fmt"\" >> /tmp/BWMON.log", ##args); \
		system(info); \
	}

#define BWSUP_DBG(fmt,args...) \
	if(f_exists(BWDPI_SUP_DEBUG) > 0) { \
		dbg("[BWSUP][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

// folder path
#define TMP_BWDPI       nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/" : "/tmp/bwdpi/"

// signature check
#define APPDB           nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/bwdpi.app.db" : "/tmp/bwdpi/bwdpi.app.db"
#define CATDB           nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/bwdpi.cat.db" : "/tmp/bwdpi/bwdpi.cat.db"
#define RULEV           nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/rule.version" : "/tmp/bwdpi/rule.version"

// log and tmp file
#define WRS_FULL_LOG    "/tmp/wrs_full.txt"
#define VP_FULL_LOG     "/tmp/vp_full.txt"
#define TMP_WRS_LOG     "/tmp/tmp_wrs.txt"
#define TMP_VP_LOG      "/tmp/tmp_vp.txt"
#define WRS_VP_LOG      "/jffs/wrs_vp.txt"
#define SIG_VER         "/proc/nk_policy"
#define DPI_VER         "/proc/ips_info"
#define WAN_TMP         "/tmp/bwdpi/dev_wan"
#define QOS_TMP         "/tmp/bwdpi/qos_rul"

// node
#if defined(RTCONFIG_HND_ROUTER_AX)
#define DEVNODE         "/dev/idp"
#else
#define DEVNODE         "/dev/detector"
#endif

// database hidden path and function path
#define BWDPI_DB_DIR    "/jffs/.sys"
#define BWDPI_ANA_DIR   BWDPI_DB_DIR"/TrafficAnalyzer"
#define BWDPI_HIS_DIR   BWDPI_DB_DIR"/WebHistory"
#define BWDPI_MON_DIR   BWDPI_DB_DIR"/AiProtectionMonitor"

// AiProtection Monitor database
#define BWDPI_MON_DB    (strcmp(nvram_safe_get("bwdpi_mon_path"), "")) ? nvram_safe_get("bwdpi_mon_path") : BWDPI_MON_DIR"/AiProtectionMonitor.db"
#define BWDPI_MON_CC    BWDPI_MON_DIR"/AiProtectionMonitorCCevent.txt"
#define BWDPI_MON_VP    BWDPI_MON_DIR"/AiProtectionMonitorVPevent.txt"
#define BWDPI_MON_MALS  BWDPI_MON_DIR"/AiProtectionMonitorMALSevent.txt"

// Avoid the trigger event loop issue, add a copy file for nt_center usage
#define NT_MON_CC    BWDPI_MON_DIR"/NT-AiMonitorCCevent.txt"
#define NT_MON_VP    BWDPI_MON_DIR"/NT-AiMonitorVPevent.txt"
#define NT_MON_MALS  BWDPI_MON_DIR"/NT-AiMonitorMALSevent.txt"

//iqos.c
extern void check_qosd_wan_setting(char *dev_wan, int len);
extern void setup_qos_conf();
extern void stop_tm_qos();
extern void start_tm_qos();
extern int tm_qos_main(char *cmd);
extern void start_qosd();
extern void stop_qosd();
extern int qosd_main(char *cmd);

//wrs.c
extern void setup_wrs_conf();
extern void stop_wrs();
extern void start_wrs();
extern int wrs_main(char *cmd);
extern void setup_vp_conf();

//stat.c
extern int stat_main(char *mode, char *name, char *dura, char *date);
extern int device_main();
extern int device_info_main(char *MAC, char *ipaddr);
extern int wrs_url_main();
extern int get_anomaly_main(char *cmd);
extern int get_app_patrol_main();

//dpi.c
extern int check_daulwan_mode();
extern int tdts_check_wan_changed();
extern void stop_dpi_engine_service(int forced);
extern void run_dpi_engine_service();
extern void start_dpi_engine_service();
extern void save_version_of_bwdpi();
extern void setup_dpi_conf_bit(int input);

//wrs_app.c
extern int wrs_app_main(char *cmd);
extern int wrs_app_service(int cmd);

//data_collect.c
extern void stop_dc();
extern void start_dc(char *path);
extern int data_collect_main(char *cmd, char *path);

//watchdog_check.c
extern void auto_sig_check();
extern void sqlite_db_check();
extern void tm_eula_check();

//dpi_support.c
extern int dump_dpi_support(int index);
