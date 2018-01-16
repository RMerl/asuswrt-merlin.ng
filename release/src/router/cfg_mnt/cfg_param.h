#ifndef __CFG_PARAM_H__
#define __CFG_PARAM_H__

/* parameter list table */
#if defined(BIT)
#undef BIT
#endif
#define BIT(x)	((1 << x))

/* feature */
#define FT_WIRELESS	BIT(0)
#define FT_LOGIN	BIT(1)
#define FT_TIME		BIT(2)
#define FT_MISC		BIT(3)
#define FT_LOGGER		BIT(4)
#define FT_FEEDBACK		BIT(5)
#define FT_DIAGNOSTIC	BIT(6)
#define FT_BACKHAULCTRL	BIT(7)
#if defined(MAPAC2200)
#define FT_BHBLOCK	BIT(8)	/* normal client blocking in backhaul */
#endif
#ifdef RTCONFIG_WIFI_SON
#define FT_SPCMD	BIT(9)	/* special command */
#endif


struct feature_mapping_s {
	char *name;
	int index;
	char *service;
};

struct feature_mapping_s feature_mapping_list[] = {
	{ "wireless", 	FT_WIRELESS,	"restart_wireless" },
	{ "login", 	FT_LOGIN,		"chpass" },
	{ "time",		FT_TIME,			"restart_time" },
	{ "misc",		FT_MISC,			NULL },
	{ "logger",	FT_LOGGER,		"restart_logger" },
	{ "feedback",	FT_FEEDBACK,	"restart_sendmail" },
	{ "diagnostic",	FT_DIAGNOSTIC,	"restart_dblog" },
	{ "backhalctrl", 	FT_BACKHAULCTRL,	"restart_amas_bhctrl"},
#if defined(MAPAC2200)
	{ "bhblock", 	FT_BHBLOCK,	"restart_bhblock" },
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd", 	FT_SPCMD,	"restart_spcmd" },
#endif
	/* END */
	{ NULL, 0, NULL }
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
	SUBFT_ADVANCED_2G,
	SUBFT_ADVANCED_5G,
	SUBFT_ADVANCED_5G1,

	/* sub feature for administration */
	SUBFT_ROUTER_LOGIN,
	SUBFT_TIMEZONE,
	SUBFT_NTP_SERVER,
	SUBFT_TELNET_SERVER,
	SUBFT_SSH_SERVER,
	SUBFT_LOG_SERVER,
	SUBFT_LOCATION,
	SUBFT_MISCELLANEOUS,

	/* sub feature for feedback and diagnostic */
	SUBFT_FEEDBACK,
	SUBFT_DIAGNOSTIC,

	/* sub feature for amas */
	SUBFT_BACKHAULCTRL,	/* backhaul ctrl */

	/* sub feature for smart connect */
	SUBFT_SMART_CONNECT,	/* smart connect */
#if defined(MAPAC2200)
	SUBFT_NCB,
#endif
#ifdef RTCONFIG_WIFI_SON
	SUBFT_SPCMD,
#endif
};

struct subfeature_mapping_s subfeature_mapping_list[] = {
	/* for wireless */
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
	{ "advanced_2g",	 	SUBFT_ADVANCED_2G,		FT_WIRELESS },
	{ "advanced_5g", 		SUBFT_ADVANCED_5G,		FT_WIRELESS },
	{ "advanced_5g1",		SUBFT_ADVANCED_5G1,	FT_WIRELESS },
	/* administration */
	{ "router_login", 	SUBFT_ROUTER_LOGIN,	FT_LOGIN },
	{ "time_zone",		SUBFT_TIMEZONE,		FT_TIME },
	{ "ntp_server",		SUBFT_NTP_SERVER,	FT_TIME },
	{ "telnet_server",		SUBFT_TELNET_SERVER,	FT_TIME },
	{ "ssh_server",		SUBFT_SSH_SERVER,	FT_TIME },
	{ "log_server",		SUBFT_LOG_SERVER,	FT_LOGGER },
	{ "location",		SUBFT_LOCATION,	FT_MISC },
	{ "misc",		SUBFT_MISCELLANEOUS,	FT_MISC },
	/* feedback and diagnostic */
	{ "feedback",		SUBFT_FEEDBACK,		FT_FEEDBACK },
	{ "diagnostic",		SUBFT_DIAGNOSTIC,	FT_DIAGNOSTIC},
	/* backhaul ctrl */
	{ "backhalctrl",		SUBFT_BACKHAULCTRL,	FT_BACKHAULCTRL },
	{ "smart_connect", 	SUBFT_SMART_CONNECT, FT_WIRELESS },
#if defined(MAPAC2200)
	{ "ncb",		SUBFT_NCB,	FT_BHBLOCK },
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd",		SUBFT_SPCMD,	FT_SPCMD },
#endif
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
	{ "wl0_closed", 	FT_WIRELESS, 		SUBFT_BASIC_2G},
	{ "wl0_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_crypto",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_wep_x",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key1",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key2",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key3",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_key4",		FT_WIRELESS,		SUBFT_BASIC_2G},
	{ "wl0_bw",		FT_WIRELESS,		SUBFT_CHANNEL_2G},
#ifdef RTCONFIG_BCMWL6
	{ "wl0_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
#else
	{ "wl0_channel",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
	{ "wl0_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_2G},
#endif
	{ "wl1_ssid", 		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_5G},
	{ "wl1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_crypto",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_wep_x",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key1",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key2",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key3",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_key4",		FT_WIRELESS,		SUBFT_BASIC_5G},
	{ "wl1_bw",		FT_WIRELESS,		SUBFT_CHANNEL_5G},
#ifdef RTCONFIG_BCMWL6
	{ "wl1_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
#else
	{ "wl1_channel",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
	{ "wl1_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_5G},
#endif
	{ "wl2_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_closed",	 	FT_WIRELESS, 		SUBFT_BASIC_5G1},
	{ "wl2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_crypto",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_wep_x",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key1",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key2",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key3",		FT_WIRELESS,		SUBFT_BASIC_5G1},
	{ "wl2_key4",		FT_WIRELESS,		SUBFT_BASIC_5G1},
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
	{ "wl2_timesched",	FT_WIRELESS,		SUBFT_TIMESCHED_5G1},
	{ "wl2_sched",	FT_WIRELESS,		SUBFT_TIMESCHED_5G1},
	/* guest network */
	{ "wl0.1_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_2G_G1},
	{ "wl0.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_expire",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl0.1_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_2G_G1},
	{ "wl1.1_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_expire",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl1.1_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G_G1},
	{ "wl2.1_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_expire",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
	{ "wl2.1_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G1_G1},
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
	{ "wl0.2_expire",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl0.2_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_2G_G2},
	{ "wl1.2_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_expire",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl1.2_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G_G2},
	{ "wl2.2_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_expire",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
	{ "wl2.2_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G1_G2},
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
	{ "wl0.3_expire",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl0.3_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_2G_G3},
	{ "wl1.3_ssid", 	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_expire",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl1.3_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G_G3},
	{ "wl2.3_ssid",		FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_expire",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl2.3_lanaccess",	FT_WIRELESS,		SUBFT_BASIC_5G1_G3},
	{ "wl0.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G_G3},
	{ "wl0.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G_G3},
	{ "wl1.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G_G3},
	{ "wl1.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G_G3},
	{ "wl2.3_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G3},
	{ "wl2.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1_G3},
	/* wireless advanced */
	{ "wl0_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_2G},
	{ "wl1_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_5G},
	{ "wl2_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_5G1},
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
	/* telnet server */
	{ "telnetd_enable", 	FT_TIME,		SUBFT_TELNET_SERVER},
	/* telnet server */
	{ "telnetd_enable", 	FT_TIME,		SUBFT_TELNET_SERVER},
	/* ssh server */
	{ "sshd_enable", 	FT_TIME,		SUBFT_SSH_SERVER},
	{ "sshd_port", 	FT_TIME,		SUBFT_SSH_SERVER},
	{ "sshd_pass", 	FT_TIME,		SUBFT_SSH_SERVER},
	{ "sshd_authkeys", 	FT_TIME,		SUBFT_SSH_SERVER},
	{ "shell_timeout", 	FT_TIME,		SUBFT_SSH_SERVER},
	/* log server */
	{ "log_ipaddr", 	FT_LOGGER,		SUBFT_LOG_SERVER},
	/* misc */
	{ "cfg_alias", 	FT_MISC,		SUBFT_LOCATION},
	{ "apps_sq", 	FT_MISC,		SUBFT_MISCELLANEOUS},
	{ "preferred_lang",	FT_MISC,	SUBFT_MISCELLANEOUS},
	/* feedback */
	{ "fb_transid",		FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_email_dbg", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_email", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_ptype", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_pdesc", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_country", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_browserInfo", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_syslog", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_cfgfile", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_modemlog", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
#ifdef RTCONFIG_DSL
	{ "fb_ISP", 				FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_availability", 			FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_Subscribed_Info", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_iptables", 		FT_FEEDBACK,		SUBFT_FEEDBACK},
#endif
	/* diagnostic */
	{ "dblog_enable", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_tousb", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_service", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_duration", 	FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_transid", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	/* backhaul ctrl */
	{ "amas_ethernet", 	FT_BACKHAULCTRL,	SUBFT_BACKHAULCTRL},
	{ "smart_connect_x", 	FT_WIRELESS,	SUBFT_SMART_CONNECT},
#if defined(MAPAC2200)
	/* normal client blocking in backhaul */
	{ "ncb_enable", 	FT_BHBLOCK,		SUBFT_NCB},
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd", 		FT_SPCMD,		SUBFT_SPCMD},
#endif
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
	{ "key1",		NULL },
	{ "key2",		NULL },
	{ "key3",		NULL },
	{ "key4",		NULL },
	{ "macmode", NULL },
	{ "maclist_x", NULL },
	{ NULL, 		NULL }
};

#endif /* __CFG_PARAM_H__ */
/* End of cfg_param.h */
