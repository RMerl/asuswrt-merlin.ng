/*
* Copyright 2024, ASUSTeK Inc.
* All Rights Reserved.
*/

#ifndef _hns_common_h_
#define _hns_common_h_

/* parameters define */
#define TMP_HNS             "/tmp/bwdpi/"
#define TMP_HNS_WAN         "/tmp/bwdpi/dev_wan"
#define UFWD                "ufwd"
#define UFWD_TIMEOUT        20
#define UFWD_APP_TIMEOUT    "3600"
#define UFWD_USER_TIMEOUT   "3600"
#define UFWD_NFQ_MARK       "0x80000000"
#define UFWD_CPUNUM         TMP_HNS"CPUNUM"
#define UFWD_HNS_PATH       "/usr/hns/"
#define UFWD_WAITING        10

/* HNS function bit */
#define HNS_APP_ID        0x0001
#define HNS_DEV_ID        0x0002
#define VIRTUAL_PATCH     0x0004
#define WRS_WEB_PATROL    0x0008
#define WRS_WEB_SECURITY  0x0020
#define ANOMALY           0x0040
#define WRS_WBL           0x0400
#define IOTRS             0x1000

/* simplize the feature name */
#define HNS_FULL         nvram_get_int("wrs_protect_enable")
#define HNS_MALS         nvram_get_int("wrs_mals_enable")
#define HNS_VP           nvram_get_int("wrs_vp_enable")
#define HNS_CC           nvram_get_int("wrs_cc_enable")
#define WEB_FILTER       nvram_get_int("wrs_enable")
#define WEB_HISTORY      nvram_get_int("bwdpi_wh_enable")

/* HNS database related */
#define HNS_HIS_DIR      "/jffs/.sys/WebHistory"
#define HNS_HIS_DB       "/jffs/.sys/WebHistory/WebHistory.db"
#define HNS_MON_DIR      "/jffs/.sys/AiProtectionMonitor"
#define HNS_MON_DB       "/jffs/.sys/AiProtectionMonitor/AiProtectionMonitor.db"
#define HNS_INFECTED_CC  "/jffs/.sys/AiProtectionMonitor/CCevent.log"
#define HNS_MALS_EVENT   "/jffs/.sys/AiProtectionMonitor/MALSevent.log"
#define HNS_HIS_DB_SIZE  3072
#define HNS_MON_DB_SIZE  3072
#define HNS_DOMAIN_LOG   "/tmp/hns_domain_log"
#define HNS_WRS_URL_LOG  "/tmp/hns_wrs_url_log"
#define HNS_VP_LOG       "/tmp/hns_vp_log"
#define HNS_WRS_CONF     "/tmp/bwdpi/hns_wrs_conf"
#define HNS_WBL_DIR      "/jffs/.sys/WBL"
#define HNS_SIG_DIR      "/jffs/signature/"
#define HNS_SIG_ZIP      "/jffs/signature/rule.zip"
#define HNS_SIG_VER      "/tmp/bwdpi/sig_ver"

/* WRS config */
#define HNS_MAX_CATEGORIES  4
#define HNS_MAX_CATIDS      20

/* WRS / WBL config bit */
#define HNS_WBL_ENABLE      0x01
#define HNS_WRS_ENABLE      0x02

/* eum : BLOCKED REASON */
enum {
	HNS_BLOCKED_NONE            = 0, // 0 : normal, not to block
	HNS_BLOCKED_DEFAULT         = 1, // 1 : general block
	HNS_BLOCKED_DUALWAN_LB      = 2, // 2 : not support dualwan load-balance mode
	HNS_BLOCKED_STOP_FORCE      = 3  // 3 : bwdpi_stop_force
};

/* iptables */
#define HNS_UFWD_CHAIN      "HNS-UFWD"

/* function */
#define is_UFWD_pid()       ((pidof(UFWD) > 0) ? 1 : 0)
#define is_UFWD_service()   ((is_UFWD_pid() && check_hns_setting()) ? 1 : 0)

/* DEBUG FUNCTION */
#define HNS_DEBUG           "/tmp/HNS_DEBUG"
#define HNS_DEBUG_JFFS      "/jffs/HNS_DEBUG"
#define HNS_DBG(fmt,args...) \
	if(f_exists(HNS_DEBUG) > 0 || f_exists(HNS_DEBUG_JFFS) > 0) { \
		_dprintf("[HNS][%s:(%d)] "fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define HNS_DB_DEBUG        "/tmp/HNS_DB_DEBUG"
#define HNS_DB_DEBUG_JFFS   "/jffs/HNS_DB_DEBUG"
#define HNS_DB_DBG(fmt,args...) \
	if(nvram_get_int("hns_debug") || f_exists(HNS_DB_DEBUG) > 0 || f_exists(HNS_DB_DEBUG_JFFS) > 0) { \
		_dprintf("[HNS_DB][%s:(%d)] "fmt, __FUNCTION__, __LINE__, ##args); \
	}

typedef enum {
	HNS_DPI_MALS = 0,
	HNS_DPI_VP,
	HNS_DPI_CC,
	HNS_WEBS_FILTER,
	HNS_WEB_HISTORY,
	HNS_ADAPTIVE_QOS,
	HNS_FEATURE_MAX
} hnsFeature_e;

struct hnsSupport_t {
	char *model;
	int feature[HNS_FEATURE_MAX];
};

extern int HNSisSupport(const char *name);

/* HNS services */
extern void disable_hns_setting(void);
extern void add_hns_ufwd_rules(FILE *fp);
extern void add_hns_ufwd_input_rules(FILE *fp);
extern void add_hns_ufwd_forward_rules(FILE *fp);
extern void add_hns_iptables_rules();
extern void del_hns_iptables_rules();
extern void sign_hns_eula();
extern void stop_hns_service_store();
extern void stop_hns_engine();
extern void hns_engine_configs();
extern void start_hns_engine();
extern void wan_start_hns_engine(int wan_unit);
extern int check_hns_switch();
extern int check_hns_setting();
extern void check_hns_alive_service();
extern int HNSisSupport(const char *name);

/* HNS DB */
extern void exe_hns_history();
extern void exe_hns_protection();
extern void hns_clean_db(int type);
#ifdef RTCONFIG_NOTIFICATION_CENTER
extern void hns_protect_event_cc();
#endif

#endif /* _hns_common_h_ */
