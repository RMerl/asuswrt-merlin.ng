/*
** amas_apg_shared.h
**
**
**
*/
#ifndef __APG_SHAREDH__
#define __APG_SHAREDH__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include "shutils.h"
#include <shared.h>

#if defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
#define APG_HAVE_VLAN0
#endif

#define IS_ZERO_MAC(MACADDR)	( (memcmp(MACADDR, "\x00\x00\x00\x00\x00\x00", 6) == 0) )
#define IS_CAP() 			( (sw_mode() == SW_MODE_ROUTER || access_point_mode()) )
#define IS_RE()  			( (nvram_get_int("re_mode") == 1) )

#define NV_APG_STARTED				"apg_started"
#define NV_APG_IFNAMES              "apg_ifnames"
#define NV_APG_BRX_FH_WLIFNAMES     "apg_%s_fh_wlifnames"
#define NV_APG_BRX_FH_ETHIFNAMES    "apg_%s_fh_ethifnames"
#define NV_APG_BRX_BH_WLIFNAMES     "apg_%s_bh_wlifnames"
#define NV_APG_BRX_BH_ETHIFNAMES    "apg_%s_bh_ethifnames"
#define NV_APG_WDS_VLAN_IFNAMES     "apg_wds_vlan_ifnames"

#define APG_MAXINUM             64
#define MAX_APG_BRIDGE_IF       256        
#define MAX_AP_RULE_LIST        64
#define MAX_WLIF_BUFFER_SIZE    (64*18)
#define MAX_LANIF_BUFFER_SIZE   (64*18)
#define MAX_MLO_RULE_FIELDS    3
#define MAX_AP_WIFI_RULE_FIELDS    3
#define MAX_AP_LANIF_RULE_FIELDS   2
#define NV_AP_WIFI_RL   "ap_wifi_rl"  
#define NV_AP_LANIF_RL  "ap_lanif_rl"
#define NV_WGN_VLAN_RL  "vlan_rulelist"
#define NV_MLO_RL   "mlo_rl"

#if MTLAN_MAXINUM > APG_MAXINUM
#undef APG_MAXINUM
#define APG_MAXINUM	MTLAN_MAXINUM
#endif	// MTLAN_MAXINUM > APG_MAXINUM)

#if MTLAN_MAXINUM > MAX_APG_BRIDGE_IF
#undef MAX_APG_BRIDGE_IF
#define MAX_APG_BRIDGE_IF MTLAN_MAXINUM
#endif	// MTLAN_MAXINUM > MAX_APG_BRIDGE_IF

#if MTLAN_MAXINUM > MAX_AP_RULE_LIST
#undef MAX_AP_RULE_LIST
#define MAX_AP_RULE_LIST MTLAN_MAXINUM
#endif	// MTLAN_MAXINUM > MAX_AP_RULE_LIST

typedef struct ap_wifi_rule_t {
    int vid;
    char wlif_set[MAX_WLIF_BUFFER_SIZE+1];
	int sdn_idx; 
} ap_wifi_rule_st;

typedef struct ap_lanif_rule_t {
    int vid;
    char lanif_set[MAX_LANIF_BUFFER_SIZE+1];
} ap_lanif_rule_st;

typedef struct mlo_rule_t {
    int idx;
    int mlo;
    char wlif_set[MAX_LANIF_BUFFER_SIZE+1];
} mlo_rule_st;

#define APG_BR_INFO_NVRAM       "apg_br_info"
#define MAX_APG_BR_INFO_FIELDS  2
#define MAX_APG_BR_INFO_BUFFER_SIZE    128 * MAX_AP_RULE_LIST
typedef struct apg_br_info_t {
    int vlan_id;
    char br_name[IFNAMSIZ+1];
} apg_br_info_st;

#if !defined(BIT_XX)
#define BIT_XX(x)  ((1 << x))
#endif

#define VIF_TYPE_NO_USED	BIT_XX(0)
#define VIF_TYPE_PRELINK	BIT_XX(1)
#define VIF_TYPE_FRONTHAUL 	BIT_XX(2)
#define VIF_TYPE_OB 		BIT_XX(3)
#define VIF_TYPE_DEBUG 		BIT_XX(4)
#define VIF_TYPE_MAIN		BIT_XX(5)
#define VIF_TYPE_OWE_TRANS	BIT_XX(6)
#define VIF_TYPE_MLO_DWB 	BIT_XX(7)

#define MAX_SSID_STR_LEN        64
#define MAX_SCHED_STR_LEN       24
#define MAX_EXPIRETIME_STR_LEN  256
#define MAX_MAC_STR_LEN         24
#define MAX_PSK_STR_LEN         64
#define MAX_AUTH_MODE_STR_LEN   32
#define MAX_CRYPTO_STR_LEN      32
#define MAX_LAN_PORTIDX_STR_LEN 64
#define MAX_SECURITY_LIST_SIZE  8
#define MAX_SCHED_LIST_SIZE     64
#define MAX_MAC_LIST_SIZE       64
#define MAX_DUT_LIST_SIZE       32

#define NV_APG_X_ENABLE		    "apg%d_enable"
#define NV_APG_X_SSID		    "apg%d_ssid"
#define NV_APG_X_HIDE_SSID		"apg%d_hide_ssid"
#define NV_APG_X_SECURITY	    "apg%d_security"
#define NV_APG_X_BW_LIMIT	    "apg%d_bw_limit"
#define NV_APG_X_TIMESCHED      "apg%d_timesched"
#define NV_APG_X_SCHED		    "apg%d_sched"
#define NV_APG_X_AP_ISOLATE	    "apg%d_ap_isolate"
#define NV_APG_X_MACMODE        "apg%d_macmode"
#define NV_APG_X_MACLIST	    "apg%d_maclist"
#define NV_APG_X_IOT_MAX_CMPT   "apg%d_iot_max_cmpt"
#define NV_APG_X_DUT_LIST	    "apg%d_dut_list"
#define NV_APG_X_MLO	        "apg%d_mlo"
#define NV_APG_X_EXPIRETIME     "apg%d_expiretime"

const static char APGx_NVRAM_LIST[] = {
    NV_APG_X_ENABLE","\
    NV_APG_X_SSID","\
    NV_APG_X_HIDE_SSID","\
    NV_APG_X_SECURITY","\
    NV_APG_X_BW_LIMIT","\
    NV_APG_X_TIMESCHED","\
    NV_APG_X_SCHED","\
    NV_APG_X_AP_ISOLATE","\
    NV_APG_X_MACMODE","\
    NV_APG_X_MACLIST","\
    NV_APG_X_IOT_MAX_CMPT","\
    NV_APG_X_DUT_LIST","\
    NV_APG_X_MLO","\
    NV_APG_X_EXPIRETIME
};

const static char NV_APG_X_SUFFIX[] = {
	"ssid,"\
	"auth_mode_x,"\
	"crypto,"\
	"wpa_psk,"\
    "radius_ipaddr,"\
    "radius_key,"\
    "radius_port,"\
    "radius_acct_ipaddr,"\
    "radius_acct_key,"\
    "radius_acct_port,"\
    "radius2_ipaddr,"\
    "radius2_key,"\
    "radius2_port,"\
    "radius2_acct_ipaddr,"\
    "radius2_acct_key,"\
    "radius2_acct_port,"\
	"bw_dl,"\
	"bw_enabled,"\
	"bw_ul,"\
	"ap_isolate,"\
	"closed,"\
    "timesched,"\
	"sched_v2,"\
    "expiretime,"\
    "macmode,"\
	"maclist_x,"\
	"iot_max_cmpt"
};

struct _security_t {
    unsigned short band_set;
    char auth_mode[MAX_AUTH_MODE_STR_LEN+1];
    char crypto[MAX_CRYPTO_STR_LEN+1];
    char psk[MAX_PSK_STR_LEN+1];
    int radius_idx;
};

struct _bwlimit_t {
    unsigned int enable;
    unsigned long ul;
    unsigned long dl;
};

struct _sched_t {
    char sched[MAX_SCHED_STR_LEN+1];
};

struct _maclist_t {
    char mac[MAX_MAC_STR_LEN+1];
};

struct _dutlist_t {
    char mac[MAX_MAC_STR_LEN+1];
    unsigned short wifi_band;
    char lan_port_index[MAX_LAN_PORTIDX_STR_LEN+1];
};

typedef struct _apg_rule_t {
    int index;
    unsigned int enable;
    char ssid[MAX_SSID_STR_LEN+1];
	int hide_ssid;
    int security_list_size;
    struct _security_t security[MAX_SECURITY_LIST_SIZE];
    struct _bwlimit_t bw_limit;
    int timesched;
    int sched_list_size;
    struct _sched_t sched[MAX_SCHED_LIST_SIZE];
    char expiretime[MAX_EXPIRETIME_STR_LEN+1];
    unsigned int ap_isolate;
    char macmode[33];
    int mac_list_size;
    struct _maclist_t maclist[MAX_MAC_LIST_SIZE];
    int iot_max_compatibity;
    int dut_list_size;
    struct _dutlist_t dut_list[MAX_DUT_LIST_SIZE];
    int dut_for_all;
    unsigned short mlo_support;
} apg_rule_st;

extern struct _security_t* get_apg_security(int nvram_idx, struct _security_t* list, int max_list_size, int *ret_list_size);
extern struct _bwlimit_t* get_apg_bwlimit(int nvram_idx, struct _bwlimit_t* bwlimit);
extern struct _sched_t* get_apg_sched(int nvram_idx, struct _sched_t* list, int max_list_size, int *ret_list_size);
extern struct _maclist_t* get_apg_maclist(int nvram_idx, struct _maclist_t* list, int max_list_size, int *ret_list_size);
extern struct _dutlist_t* get_apg_dutlist(int nvram_idx, struct _dutlist_t* list, int max_list_size, int *ret_list_size);

extern int              get_rm_sdn_vid_by_apg_rule(apg_rule_st* apg_rule);
extern int              get_sdn_vid_by_apg_rule(apg_rule_st* apg_rule);
extern char* get_sdn_type_by_apg_rule(apg_rule_st* apg_rule, char *type, int type_bsize);
extern apg_rule_st*     get_apg_rule_by_idx(int idx, apg_rule_st* apg_rule);
extern apg_rule_st*     get_apg_rule_by_vid(int vid, apg_rule_st* apg_rule);
extern apg_rule_st*     get_apg_rule_by_dut(char *dut_mac, unsigned short wifi_band, apg_rule_st* apg_rule);
extern unsigned short 	get_wifi_band_by_dut_mac(char *dut_mac, apg_rule_st *apg_rule);
extern char*            get_apg_value(apg_rule_st* apg_rule, unsigned short wifi_band, char *item, char *ret_value, int ret_value_bsize);
extern int              get_mtlan_enable_by_idx(const unsigned int idx);
extern int              get_mtlan_enable_by_vid(const unsigned int vid);
extern int              find_mtvlan(const unsigned int vid);

#define MAX_IFNAME_STR_LEN		32
#define MAX_VID_TRUNK_LIST_SIZE    512
#define NV_VLAN_TRUNK_RULE   "vlan_trunk_rl"
typedef struct _vlan_trunk_rule_t {
    char ifname[MAX_IFNAME_STR_LEN+1];
    int vid_list_size;
    int vid[MAX_VID_TRUNK_LIST_SIZE];
} vlan_trunk_rule_st;

extern vlan_trunk_rule_st* get_vlan_trunk_rule_list(vlan_trunk_rule_st *list, int max_list_size, int *ret_list_size);
extern vlan_trunk_rule_st* get_vlan_trunk_rule_list_from_buffer(char *buffer, vlan_trunk_rule_st *list, int max_list_size, int *ret_list_size);
extern int is_apg_trunk_vid(char *ifname, int vid);
extern int is_apg_trunk_if(char *ifname);

#define NV_VLAN_RULE    "vlan_rl"
extern struct __vlan_rl_t__*  get_vlan_rule_list(struct __vlan_rl_t__ *list, int max_list_size, int *ret_list_size);

// for br_ioctl
enum { BRIOCTL_ADDBR = 0, BRIOCTL_DELBR, BRIOCTL_ADDIF, BRIOCTL_DELIF };

extern apg_br_info_st*  get_apg_br_list(apg_br_info_st *list, int max_list_size, int *ret_list_size);
extern int              set_apg_br_list(apg_br_info_st *list, int list_size);
extern void             update_apg_br_list(int action, char *br_name, int vlan_id);
extern mlo_rule_st* get_mlo_rl_from_buffer(char *buffer, mlo_rule_st *list, int max_list_size, int *ret_list_size);
extern mlo_rule_st* get_mlo_rl_from_nvram(mlo_rule_st *list, int max_list_size, int *ret_list_size);
extern char* get_mlo_ifnames_by_idx(int idx, char *ret_ifnames, int ret_ifnames_bsize);
extern int get_mld_ifnames_by_idx(int idx, char *mlo_rule, char *ret_ifnames, int ret_ifnames_bsize);
extern ap_wifi_rule_st* get_ap_wifi_rl_from_buffer(char *buffer, ap_wifi_rule_st *list, int max_list_size, int *ret_list_size);
extern ap_wifi_rule_st* get_ap_wifi_rl_from_nvram(ap_wifi_rule_st *list, int max_list_size, int *ret_list_size);
extern char*            get_ap_wifi_ifnames_by_vid(int vid, char *ret_ifnames, int ret_ifnames_bsize);
extern ap_wifi_rule_st* get_ap_wifi_rule_by_vid(int vid, ap_wifi_rule_st *ap_wifi_rule);
extern ap_wifi_rule_st* get_ap_wifi_rule_by_ifname(char *ifname, ap_wifi_rule_st *ap_wifi_rule);
extern ap_lanif_rule_st* get_ap_lanif_rl_from_buffer(char *buffer, ap_lanif_rule_st *list, int max_list_size, int *ret_list_size);
extern ap_lanif_rule_st* get_ap_lanif_rl_from_nvram(ap_lanif_rule_st *list, int max_list_size, int *ret_list_size);
extern ap_lanif_rule_st* get_ap_lanif_rule_by_vid(int vid, ap_lanif_rule_st *ap_lanif_rule);
extern char*            apg_check_br_name_conflict(char *name, char *fix_name, size_t fix_name_bsize);
extern int              is_ap_group_if(char *ifname);
extern void 			apg_clean_nvram(void);
extern int              is_wl_if(char *ifname);
extern int              br_ioctl(int action, char *br, char *brif);
extern int              ifexists(char *ifname);
extern int              check_vlan_invalid(char *word, char *iface);
extern char*            get_wl_ifname(int unit, int subunit, char *buffer, size_t buffer_size);
extern int              get_ifmac(char *ifname, unsigned char *ret_mac);
extern int 				get_unit_by_ifname(char *ifname, int *unit, int *subunit);
extern int              get_apg_vid_by_ifname(char *ifname);
extern int              get_apg_status(int vid, char *ret_msg, size_t ret_msg_bsize);
extern char*            max_of_mssid_ifnames(int unit, char *ret_ifnames, int buffer_size); 
extern int              get_unit_by_band(int band_type);
extern int              get_band_by_unit(int unit);


struct apg_wl_suffix_t {
	char *apg_suffix;
	char *wl_suffix;
};
const static struct apg_wl_suffix_t apg_wl_suffix_mapping[] = {
    { "enable\0",       "\0"                            },
	{ "ssid\0",			"ssid\0"	 					},
	{ "hide_ssid\0", 	"closed\0"						},
	{ "security\0", 	"auth_mode_x,crypto,wpa_psk\0" 	},
	{ "bw_limit\0",		"bw_dl,bw_enabled,bw_ul\0"		},
    { "timesched\0",    "timesched\0"                   },
	{ "sched\0",		"sched_v2\0"					},
	{ "ap_isolate\0",	"ap_isolate\0"					},
	{ "macmode\0", 		"macmode\0"						},
	{ "maclist\0",		"maclist_x\0"					},
	{ "iot_max_cmpt\0", "iot_max_cmpt\0"				},
    { "expiretime\0",   "expiretime\0"                  },
	{ NULL,				NULL							},
};

extern int              check_apgX_suffix(char *name);
extern char* 			apg_suffix_to_wl_suffix(char *apg_suffix, char *ret_item, size_t ret_item_bsize);

#define NV_RADIUS_LIST      "radius_list"
struct _radius_t {
    unsigned int idx;
    char ipaddr[64];
    unsigned int port;
    char key[64];
    char acct_ipaddr[64];
    unsigned int acct_port;
    char acct_key[64];
    char ipaddr2[64];
    unsigned int port2;
    char key2[64];
    char acct_ipaddr2[64];
    unsigned int acct_port2;
    char acct_key2[64];
};

extern struct _radius_t* get_radius_rl_by_idx(int idx, struct _radius_t *radius_rl);

#define MAX_IFNAME_STR_LEN		32
#define MAX_VIF_LIST_SIZE		32
#define MAX_LABEL_NAME_LEN		32
#define MAX_BAND_CAP_LIST_SIZE	8
#define MAX_ETH_CAP_LIST_SIZE 	64
#define MAX_PORT_INDEX_STR_LEN  8

struct _vif_cap {
	unsigned short type;
	char ifname[MAX_IFNAME_STR_LEN+1];
	char prefix[MAX_IFNAME_STR_LEN+1];
};

typedef struct _wifi_band_cap {
	unsigned short band_type;
	unsigned int vif_count;
	struct _vif_cap vif_cap[MAX_VIF_LIST_SIZE];
} wifi_band_cap_st;

typedef struct _eth_port_cap {
	unsigned int index;
	char ifname[MAX_PORT_INDEX_STR_LEN+1];
	int phy_port_id;
    int ext_port_id;
	char label_name[MAX_LABEL_NAME_LEN+1];
	unsigned int max_rate;
    char iface_name[MAX_IFNAME_STR_LEN+1];
} eth_port_cap_st;

#define PHY_PORT_CAP_ALL 	99999
#define IS_EXT_SWITCH(NAME) ((get_ext_port_id_by_ifacename(NAME) > -1))
extern char* get_lan_iface_name(int port_index, char *ret_buf, size_t bsize);
extern int get_port_index_by_ifacename(char *name);
extern int get_ext_port_id_by_ifacename(char *name);
extern int get_ext_port_id(int port_index);
extern int match_lan_ifname(int port_index, char *name);
extern eth_port_cap_st* get_eth_port_cap(unsigned long cap, eth_port_cap_st* list, int max_list_size, int *ret_list_size);
extern wifi_band_cap_st* get_wifi_band_cap(wifi_band_cap_st* list, int max_list_size, int *ret_list_size);

extern char* get_bh_wl_ifnames(char *ret_ifnames, int buffer_size);
extern char* get_bh_eth_ifnames(char *ret_ifnames, int buffer_size);
extern char* get_fh_wl_ifnames(char *ret_ifnames, int buffer_size);
extern char* get_fh_eth_ifnames(char *ret_ifnames, int buffer_size);
extern int ifexists_br(const char *br_name, const char *if_name); 
extern char* get_own_mac();
extern int del_apg_dut_list(char *mac);
extern int del_vlan_trunklist(char *mac);

extern int ifname_is_used_in_sdn(char *ifname);
extern int get_mtlan_maxinum(void);
extern char* get_rm_apg_idx(char *b, size_t bsize, size_t *rm_apg_idx_cnt);
extern char* get_availabel_sdn_index(char *b, size_t bsize, size_t *total);
extern char* check_ap_wifi_rl(char *nv, char *ret_rl, size_t ret_rl_bsize);
extern char* check_ap_lanif_rl(char *nv, char *ret_rl, size_t ret_rl_bsize);
extern char* check_vlan_trunk_rl(char *nv, char *ret_rl, size_t ret_rl_bsize);

typedef struct vlan_rl_t {
    int idx;
    int vid;
    int port_isolation;
} vlan_rl_st;
extern vlan_rl_st* get_vlan_rl_from_buffer(char *buffer, vlan_rl_st *list, int max_list_size, int *ret_list_size);
extern int sdn_vlan_is_enabled(vlan_rl_st *vlan_rl);
extern int get_isolation_mode(int vid);

extern void enableSDNRuleForMlo();
extern char *get_ap_wifi_ifnames_by_sdn(int sdn_idx, int unit, char *ret_ifnames, int ret_bsize);
extern char *get_compatible_network(int unit, char *ret_ifnames, int ret_bsize);
#endif  /* !__APG_SHAREDH__ */
