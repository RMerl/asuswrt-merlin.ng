#ifndef __CFG_LIB_H__
#define __CFG_LIB_H__

#include <rtconfig.h>

extern int igr_wlioctl;

#define MAX_2G_CHANNEL_LIST_NUM	16
#define MAX_5G_CHANNEL_LIST_NUM	32
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
#define MAX_6G_CHANNEL_LIST_NUM	128
#endif

typedef struct _avbl_chanspec_t
{
	unsigned int bw2g;
	unsigned int bw5g;
	unsigned int bw6g;
	unsigned int channelList2g[MAX_2G_CHANNEL_LIST_NUM];
	unsigned int channelList5g[MAX_5G_CHANNEL_LIST_NUM];
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	unsigned int channelList6g[MAX_6G_CHANNEL_LIST_NUM];
#endif
	unsigned int existTribandRe;
	unsigned int existDual5gRe;
	unsigned int existDual6gRe;
}AVBL_CHANSPEC_T;

struct wlcsuffix_mapping_s {
	char *name;
	char *converted_name;
	int wlc_trans_to_wl;
};

static struct wlcsuffix_mapping_s wlcsuffix_mapping_list[] __attribute__ ((unused)) = {
	{ "ssid",		NULL, 1 },
	{ "wpa_psk",	NULL, 1 },
	{ "crypto",	NULL, 1 },
	{ "auth_mode_x",	"auth_mode", 1 },
	{ "wep_x",		"wep", 1 },
	{ "key",		NULL, 1 },
	{ "key1",		NULL, 0 },
	{ "key2",		NULL, 0 },
	{ "key3",		NULL, 0 },
	{ "key4",		NULL, 0 },
	{ "macmode", NULL, 0 },
	{ "maclist_x", NULL, 0 },
	{ "closed", NULL, 0 },
	{ "radius_ipaddr", NULL, 1 },
	{ "radius_key", NULL, 1 },
	{ "radius_port", NULL, 1 },
	{ "ap_isolate", NULL, 0},
#ifdef RTCONFIG_WIFI7
	{ "11be",		NULL, 1 },
#endif
	{ NULL, 		NULL, 0 }
};

extern int send_cfgmnt_event(char *msg);
extern int get_chanspec_info(AVBL_CHANSPEC_T *avblChannel);
extern int send_event_to_roamast(char *data);
#if defined(RTCONFIG_NOWL)
extern int is_5g_high_band(int unit);
#endif
extern char *get_rebandtype_chanspc_by_unit (char *mac, int unit ,int reBandNum , char *rebandtype, int rebandtypeLen);
extern char *cap_get_final_paramname(char *mac, char *input_param,int reBandNum , char *finalparamname, int finalparamnamelen);

#ifdef RTCONFIG_ROUTERBOOST
//extern int cm_sendEventToAsusRbd(unsigned char *data, int len);
extern int send_event_to_asusrbd(unsigned char *data, int len);
#endif //RTCONFIG_ROUTERBOOST

extern int cm_isCapSupported(char *reMac, int capType, int capSubtype);
extern int get_unit_chanspc_by_bandtype (char *mac, char *bandtype);

#ifdef RTCONFIG_MULTILAN_MWL
extern int fix_mlo_dwb_for_bhfh(int dwb_subunit,int profile_type);
#endif

extern int check_suffix_type(char *suffix);

#endif /* __CFG_LIB_H__ */
/* End of cfg_lib.h */
