 /*
 * Copyright 2016, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <net/if.h>
#include <bcmnvram.h>
#include <bcmparams.h>
#include <utils.h>
#include <shutils.h>
#include <shared.h>
#include <rtstate.h>
#include <stdarg.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#include <httpd.h>

// TrendMicro header
#include <TrendMicro.h>

// ASUS debug
#include <bwdpi_common.h>

// command
#define WRED            nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/wred" : "wred"
#define AGENT           nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/tdts_rule_agent" : "tdts_rule_agent"
#define DATACOLLD       nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/dcd" : "dcd"
#define WRED_SET        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/wred_set_conf" : "wred_set_conf"  // white and black list setup conf
#define WRED_WBL        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/wred_set_wbl" : "wred_set_wbl"	// white and black list command
#define TCD             nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/tcd" : "tcd"
#define SHN_CTRL        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/shn_ctrl" : "shn_ctrl"            // TrendMicro new sample command line

// config and folder path
#define DATABASE        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/rule.trf" : "/tmp/bwdpi/rule.trf"
#define QOS_CONF        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/qosd.conf" : "/tmp/bwdpi/qosd.conf"
#define WRS_CONF        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/wred.conf" : "/tmp/bwdpi/wred.conf"
#define APP_SET_CONF    nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/app_patrol.conf" : "/tmp/bwdpi/app_patrol.conf"
#define LD_PATH         nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/" : "/usr/lib/"
#define SHN_LIB         nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/" : "/tmp/bwdpi/libshn_pctrl.so"
#define DPI_CERT        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/ntdasus2014.cert" : "/tmp/bwdpi/ntdasus2014.cert"
#define DCD_EULA        nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/dcd.conf" : "/tmp/bwdpi/dcd.conf"

// kernel module
#define TDTS            nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/tdts.ko" : "/usr/bwdpi/tdts.ko"
#define UDB             nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/tdts_udb.ko" : "/usr/bwdpi/tdts_udb.ko"
#define UDBFW           nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/tdts_udbfw.ko" : "/usr/bwdpi/tdts_udbfw.ko"

// pid
#define WREDPID         nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/wred.pid" : "/tmp/bwdpi/wred.pid"
#define DATAPID         nvram_get_int("bwdpi_debug_path") ? "/jffs/TM/dcd.pid" : "/tmp/bwdpi/dcd.pid"

// module setting
#define TDTSFW_PARA     "/sys/module/tdts_udb/parameters/"
#define DEV_WAN         TDTSFW_PARA"dev_wan"
#define DEV_LAN         TDTSFW_PARA"dev_lan"
#define QOS_WAN         TDTSFW_PARA"qos_wan"
#define QOS_LAN         TDTSFW_PARA"qos_lan"
#define BW_DPI_SET      "/proc/bw_dpi_conf"

// database hidden path and function path
#define BWDPI_DB_DIR    "/jffs/.sys"
#define BWDPI_ANA_DIR   BWDPI_DB_DIR"/TrafficAnalyzer"
#define BWDPI_HIS_DIR   BWDPI_DB_DIR"/WebHistory"
#define BWDPI_MON_DIR   BWDPI_DB_DIR"/AiProtectionMonitor"

// Traffic Analyzer database
#define BWDPI_ANA_DB    (strcmp(nvram_safe_get("bwdpi_ana_path"), "")) ? nvram_safe_get("bwdpi_ana_path") : BWDPI_ANA_DIR"/TrafficAnalyzer.db"

// Web History database
#define BWDPI_HIS_DB    (strcmp(nvram_safe_get("bwdpi_his_path"), "")) ? nvram_safe_get("bwdpi_his_path") : BWDPI_HIS_DIR"/WebHistory.db"

// OOM protection
#define IS_IDPFW()      f_exists("/dev/idpfw")

typedef struct cat_id cid_s;
struct cat_id{
	int id;
	cid_s *next;
};

typedef struct wrs wrs_s;
struct wrs{
	int enabled;
	char mac[18];
	cid_s *ids;
	wrs_s *next;
};

typedef struct mac_list mac_s;
struct mac_list{
	char mac[18];
	mac_s *next;
};

typedef struct mac_group mac_g;
struct mac_group{
	char group_name[128];
	mac_s *macs;
};

typedef struct bwdpi_client bwdpi_device;
struct bwdpi_client{
	char hostname[32];
	char vendor_name[100];
	char type_name[100];
	char device_name[100];
};

/* dpi_support index */
enum{
	INDEX_ALL = 0,
	INDEX_MALS = 1,
	INDEX_VP,
	INDEX_CC,
	INDEX_ADAPTIVE_QOS,
	INDEX_TRAFFIC_ANALYZER,
	INDEX_WEBS_FILTER,
	INDEX_APPS_FILTER,
	INDEX_WEB_HISTORY,        // NOTE: will remove in the future, replaced by web_mon
	INDEX_BANDWIDTH_MONITOR
};

//wrs.c
void free_id_list(cid_s **target_list);
cid_s *get_id_list(cid_s **target_list, char *target_string);
void print_id_list(cid_s *id_list);
void free_wrs_list(wrs_s **target_list);
extern wrs_s *get_all_wrs_list(wrs_s **wrs_list, char *setting_string);
void print_wrs_list(wrs_s *wrs_list);
wrs_s *match_enabled_wrs_list(wrs_s *wrs_list, wrs_s **target_list, int enabled);
void free_mac_list(mac_s **target_list);
mac_s *get_mac_list(mac_s **target_list, const char *target_string);
void print_mac_list(mac_s *mac_list);
void free_group_mac(mac_g **target);
mac_g *get_group_mac(mac_g **mac_group, const char *target);
void print_group_mac(mac_g *mac_group);

//stat.c
extern void get_traffic_stat(char *mode, char *name, char *dura, char *date);
extern void get_traffic_hook(char *mode, char *name, char *dura, char *date, int *retval, webs_t wp);
extern void get_device_stat();
extern int bwdpi_client_info(char *MAC, char *ipaddr, bwdpi_device *device);
extern void redirect_page_status(int cat_id, int *retval, webs_t wp);
