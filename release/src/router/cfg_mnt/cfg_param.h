#ifndef __CFG_PARAM_H__
#define __CFG_PARAM_H__

/* parameter list table */
#define BIT(x)	((1 << x))

/* feature */
#define FT_WIRELESS	BIT(0)
#define FT_LOGIN	BIT(1)
#define FT_TIME		BIT(2)

struct feature_mapping_s {
	char *name;
	int index;
	char *service;
};

struct feature_mapping_s feature_mapping_list[] = {
	{ "wireless", 	FT_WIRELESS,	"restart_wireless" },
	{ "login", 	FT_LOGIN,	"chpass" },
	{ "time", 	FT_TIME,	"restart_time" },
	/* END */
	{ NULL, 0, 0 }
};

struct subfeature_mapping_s {
	char *name;
	int index;
	int feature;
};

enum {
	SUBFT_END = 0,

	/* sub feature for wireless */
	SUBFT_BASIC_2G = 1,
	SUBFT_BASIC_5G,
	SUBFT_BASIC_5G1,
	SUBFT_CHANNEL_2G,
	SUBFT_CHANNEL_5G,
	SUBFT_CHANNEL_5G1,
	SUBFT_MACFILTER_2G,
	SUBFT_MACFILTER_5G,
	SUBFT_MACFILTER_5G1,
	SUBFT_TIMESCHED_2G,
	SUBFT_TIMESCHED_5G,
	SUBFT_TIMESCHED_5G1,
	SUBFT_BASIC_2G_G1,
	SUBFT_BASIC_5G_G1,
	SUBFT_BASIC_5G1_G1,
	SUBFT_MACFILTER_2G_G1,
	SUBFT_MACFILTER_5G_G1,
	SUBFT_MACFILTER_5G1_G1,
	SUBFT_BASIC_2G_G2,
	SUBFT_BASIC_5G_G2,
	SUBFT_BASIC_5G1_G2,
	SUBFT_MACFILTER_2G_G2,
	SUBFT_MACFILTER_5G_G2,
	SUBFT_MACFILTER_5G1_G2,
	SUBFT_BASIC_2G_G3,
	SUBFT_BASIC_5G_G3,
	SUBFT_BASIC_5G1_G3,
	SUBFT_MACFILTER_2G_G3,
	SUBFT_MACFILTER_5G_G3,
	SUBFT_MACFILTER_5G1_G3,

	/* sub feature for administration */
	SUBFT_ROUTER_LOGIN,
	SUBFT_TIMEZONE,
	SUBFT_NTP_SERVER
};

struct subfeature_mapping_s subfeature_mapping_list[] = {
	{ "basic_2g",	 	SUBFT_BASIC_2G,		FT_WIRELESS },
	{ "basic_5g", 		SUBFT_BASIC_5G,		FT_WIRELESS },
	{ "basic_5g1",		SUBFT_BASIC_5G1,	FT_WIRELESS },
	{ "channel_2g",		SUBFT_CHANNEL_2G,	FT_WIRELESS },
	{ "channel_5g",		SUBFT_CHANNEL_5G,	FT_WIRELESS },
	{ "channel_5g1",	SUBFT_CHANNEL_5G1,	FT_WIRELESS },
	{ "macfilter_2g", 	SUBFT_MACFILTER_2G,	FT_WIRELESS },
	{ "macfilter_5g", 	SUBFT_MACFILTER_5G,	FT_WIRELESS },
	{ "macfilter_5g1",	SUBFT_MACFILTER_5G1,	FT_WIRELESS },
	{ "timesched_2g", 	SUBFT_TIMESCHED_2G,	FT_WIRELESS },
	{ "timesched_5g", 	SUBFT_TIMESCHED_5G,	FT_WIRELESS },
	{ "timesched_5g1",	SUBFT_TIMESCHED_5G1,	FT_WIRELESS },
	{ "basic_2g_g1", 	SUBFT_BASIC_2G_G1,	FT_WIRELESS },
	{ "basic_5g_g1", 	SUBFT_BASIC_5G_G1,	FT_WIRELESS },
	{ "basic_5g1_g1",	SUBFT_BASIC_5G1_G1,	FT_WIRELESS },
	{ "macfilter_2g_g1", 	SUBFT_MACFILTER_2G_G1,	FT_WIRELESS },
	{ "macfilter_5g_g1", 	SUBFT_MACFILTER_5G_G1,	FT_WIRELESS },
	{ "macfilter_5g1_g1",	SUBFT_MACFILTER_5G1_G1,	FT_WIRELESS },
	{ "basic_2g_g2", 	SUBFT_BASIC_2G_G2,	FT_WIRELESS },
	{ "basic_5g_g2", 	SUBFT_BASIC_5G_G2,	FT_WIRELESS },
	{ "basic_5g1_g2",	SUBFT_BASIC_5G1_G2,	FT_WIRELESS },
	{ "macfilter_2g_g2", 	SUBFT_MACFILTER_2G_G2,	FT_WIRELESS },
	{ "macfilter_5g_g2", 	SUBFT_MACFILTER_5G_G2,	FT_WIRELESS },
	{ "macfilter_5g1_g2",	SUBFT_MACFILTER_5G1_G2,	FT_WIRELESS },
	{ "basic_2g_g3", 	SUBFT_BASIC_2G_G3,	FT_WIRELESS },
	{ "basic_5g_g3", 	SUBFT_BASIC_5G_G3,	FT_WIRELESS },
	{ "basic_5g1_g3",	SUBFT_BASIC_5G1_G3,	FT_WIRELESS },
	{ "macfilter_2g_g3", 	SUBFT_MACFILTER_2G_G3,	FT_WIRELESS },
	{ "macfilter_5g_g3", 	SUBFT_MACFILTER_5G_G3,	FT_WIRELESS },
	{ "macfilter_5g1_g3",	SUBFT_MACFILTER_5G1_G3,	FT_WIRELESS },
	{ "router_login", 	SUBFT_ROUTER_LOGIN,	FT_LOGIN },
	{ "time_zone",		SUBFT_TIMEZONE,		FT_TIME },
	{ "ntp_server",		SUBFT_NTP_SERVER,	FT_TIME },
	/* END */
	{ NULL, 0, 0}
};

struct param_mapping_s {
	char *param;
	int feature;
	int subfeature;
};

struct param_mapping_s param_mapping_list[] = {
	{ "wl0_ssid", 		FT_WIRELESS, 		SUBFT_BASIC_2G},
	{ "wl0_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_crypto",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_wep_x",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_wep_key",	FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_bw",		FT_WIRELESS,		SUBFT_CHANNEL_2G},
#ifdef RTCONFIG_BCMWL6
	{ "wl0_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
#else
	{ "wl0_channel",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
	{ "wl0_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
#endif
	{ "wl1_ssid", 		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_crypto",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_wep_x",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_wep_key",	FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_bw",		FT_WIRELESS,		SUBFT_CHANNEL_5G},
#ifdef RTCONFIG_BCMWL6
	{ "wl1_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
#else
	{ "wl1_channel",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
	{ "wl1_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
#endif
	{ "wl2_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_crypto",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_wep_x",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_wep_key",	FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_bw",		FT_WIRELESS,		SUBFT_CHANNEL_5G1},
#ifdef RTCONFIG_BCMWL6
	{ "wl2_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_5G1},
#else
	{ "wl2_channel",	FT_WIRELESS,		SUBFT_CHANNEL_5G1},
	{ "wl2_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_5G1},
#endif
	{ "wl0_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G},
	{ "wl0_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G},
	{ "wl1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G},
	{ "wl1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G},
	{ "wl2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	{ "wl2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	{ "wl0_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_2G},
	{ "wl0_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_2G},
	{ "wl1_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_5G},
	{ "wl1_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_5G},
	{ "wl2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	{ "wl2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	/* guest network */
	{ "wl0.1_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_2G_G1},
	{ "wl0.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl1.1_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
        { "wl2.1_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
        { "wl2.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
        { "wl2.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
        { "wl2.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
        { "wl2.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl0.1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G_G1},
	{ "wl0.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G_G1},
	{ "wl1.1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G_G1},
	{ "wl1.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G_G1},
        { "wl2.1_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G1},
        { "wl2.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G1},
	{ "wl0.2_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_2G_G2},
	{ "wl0.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl0.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl0.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl0.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl1.2_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl2.2_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl0.2_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G_G2},
	{ "wl0.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G_G2},
	{ "wl1.2_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G_G2},
	{ "wl1.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G_G2},
        { "wl2.2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G2},
        { "wl2.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G2},
	{ "wl0.3_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_2G_G3},
	{ "wl0.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl0.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl0.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl0.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl1.3_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
        { "wl2.3_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
        { "wl2.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
        { "wl2.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
        { "wl2.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
        { "wl2.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl0.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G_G3},
	{ "wl0.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G_G3},
	{ "wl1.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G_G3},
	{ "wl1.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G_G3},
        { "wl2.3_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G3},
        { "wl2.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G3},
	/* http login */
	{ "http_username", 	FT_LOGIN,		SUBFT_ROUTER_LOGIN},
	{ "http_passwd",	FT_LOGIN,		SUBFT_ROUTER_LOGIN},
	/* time zone */
	{ "time_zone", 		FT_TIME,		SUBFT_TIMEZONE},
	{ "time_zone_dst", 	FT_TIME,		SUBFT_TIMEZONE},
	{ "time_zone_dstoff", 	FT_TIME,		SUBFT_TIMEZONE},
	/* ntp server */
	{ "ntp_server0", 	FT_TIME,		SUBFT_NTP_SERVER},
	{ "ntp_server1", 	FT_TIME,		SUBFT_NTP_SERVER},
	/* END */
	{ NULL, 0, 0 }
};

struct wlcsuffix_mapping_s {
	char *name;
	char *converted_name;
};

struct wlcsuffix_mapping_s wlcsuffix_mapping_list[] = {
	{ "ssid",		NULL },
	{ "wpa_psk",	NULL },
	{ "crypto",	NULL },
	{ "auth_mode_x",	"auth_mode" },
	{ "wep_x",		"wep" },
	{ "key",		NULL },
	{ "wep_key",	NULL },
	{ NULL, 		NULL }
};

#endif /* __CFG_PARAM_H__ */
/* End of cfg_param.h */
