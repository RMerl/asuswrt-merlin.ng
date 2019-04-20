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

#define FT_REGION	BIT(8)	/* system reboot */
#define FT_CENTRAL_LED		BIT(9)

#if defined(MAPAC2200) || defined(RTAC92U)
#define FT_BHBLOCK	BIT(10)	/* normal client blocking in backhaul */
#endif
#ifdef RTCONFIG_WIFI_SON
#define FT_SPCMD	BIT(11)	/* special command */
#endif
#define FT_LP55XX_LED	BIT(12)	/* control lp55xx led */


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
	{ "central_led",	FT_CENTRAL_LED,	"reset_led" },
	/* END */
	{ "region", FT_REGION, 	"reboot" },
#ifdef RTCONFIG_WIFI_SON
#if defined(MAPAC2200) || defined(RTAC92U)
	{ "bhblock", 	FT_BHBLOCK,	"restart_bhblock" },
#endif
	{ "spcmd", 	FT_SPCMD,	"restart_spcmd" },
#endif
	{ "lp55xx_led", FT_LP55XX_LED,	"reset_led" },
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
	SUBFT_RADIUS_2G,
	SUBFT_RADIUS_5G,
	SUBFT_RADIUS_5G1,
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
	SUBFT_ADVANCED_2G_G1,
	SUBFT_ADVANCED_2G_G2,
	SUBFT_ADVANCED_2G_G3,
	SUBFT_ADVANCED_5G_G1,
	SUBFT_ADVANCED_5G_G2,
	SUBFT_ADVANCED_5G_G3,
	SUBFT_ADVANCED_5G1_G1,
	SUBFT_ADVANCED_5G1_G2,
	SUBFT_ADVANCED_5G1_G3,

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
#if defined(MAPAC2200) || defined(RTAC92U)
	SUBFT_NCB,
#endif
#ifdef RTCONFIG_WIFI_SON
	SUBFT_SPCMD,
#endif
	SUBFT_CENTRAL_LED,
	SUBFT_LP55XX_LED,	/* LP55XX LED */

	/* sub feature for Roaming Assistant */
	SUBFT_ROAMING_ASSISTANT,
	SUBFT_ROAMING_ASSISTANT_2G,
	SUBFT_ROAMING_ASSISTANT_5G,
	SUBFT_ROAMING_ASSISTANT_5G1,
	/* sub feature for Region */
	SUBFT_REGION,
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	SUBFT_BSD_STEERING_POLICY_2G,
	SUBFT_BSD_STEERING_POLICY_5G,
	SUBFT_BSD_STEERING_POLICY_5G1,
	SUBFT_BSD_STEERING_POLICY_5G_X,		// 5g only
	SUBFT_BSD_STEERING_POLICY_5G1_X,	// 5g only
	/* sub feature for smart connect rule (STA Selection Policy) */
	SUBFT_BSD_STA_SELECT_POLICY_2G,
	SUBFT_BSD_STA_SELECT_POLICY_5G,
	SUBFT_BSD_STA_SELECT_POLICY_5G1,
	SUBFT_BSD_STA_SELECT_POLICY_5G_X,	// 5g only
	SUBFT_BSD_STA_SELECT_POLICY_5G1_X,	// 5g only
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	SUBFT_BSD_IF_SELECT_2G,
	SUBFT_BSD_IF_SELECT_5G,
	SUBFT_BSD_IF_SELECT_5G1,
	SUBFT_BSD_IF_SELECT_5G_X,			// 5g only
	SUBFT_BSD_IF_SELECT_5G1_X,			// 5g only
	SUBFT_BSD_IF_QUALIFY_2G,
	SUBFT_BSD_IF_QUALIFY_5G,
	SUBFT_BSD_IF_QUALIFY_5G1,
	SUBFT_BSD_IF_QUALIFY_5G_X,			// 5g only
	SUBFT_BSD_IF_QUALIFY_5G1_X,			// 5g only
	/* sub feature for smart connect rule (Bounce Detect) */
	SUBFT_BSD_BOUNCE_DETECT,
	SUBFT_BSD_BOUNCE_DETECT_X,			// 5g only
	/* sub feature for WPS */
	SUBFT_WPS,
	/* sub feature for Reboot schedule */
	SUBFT_REBOOT_SCHEDULE
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
	{ "radius_2g", 	SUBFT_RADIUS_2G,	FT_WIRELESS },
	{ "radius_5g", 	SUBFT_RADIUS_5G,	FT_WIRELESS },
	{ "radius_5g1",	SUBFT_RADIUS_5G1,	FT_WIRELESS },
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
	{ "advanced_5g1",		SUBFT_ADVANCED_5G1,		FT_WIRELESS },
	{ "advanced_2g_g1", 	SUBFT_ADVANCED_2G_G1, 	FT_WIRELESS },
	{ "advanced_2g_g2", 	SUBFT_ADVANCED_2G_G2, 	FT_WIRELESS },
	{ "advanced_2g_g3", 	SUBFT_ADVANCED_2G_G3, 	FT_WIRELESS },
	{ "advanced_5g_g1", 	SUBFT_ADVANCED_5G_G1, 	FT_WIRELESS },
	{ "advanced_5g_g2", 	SUBFT_ADVANCED_5G_G2, 	FT_WIRELESS },
	{ "advanced_5g_g3", 	SUBFT_ADVANCED_5G_G3, 	FT_WIRELESS },
	{ "advanced_5g1_g1", 	SUBFT_ADVANCED_5G1_G1, 	FT_WIRELESS },
	{ "advanced_5g1_g2", 	SUBFT_ADVANCED_5G1_G2, 	FT_WIRELESS },
	{ "advanced_5g1_g3", 	SUBFT_ADVANCED_5G1_G3, 	FT_WIRELESS },
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
	{ "central_led",		SUBFT_CENTRAL_LED,	FT_CENTRAL_LED },
	/* Roaming Assistant */
	{ "roaming_assistant", SUBFT_ROAMING_ASSISTANT, FT_WIRELESS },
	{ "roaming_assistant_2g", SUBFT_ROAMING_ASSISTANT_2G, FT_WIRELESS },
	{ "roaming_assistant_5g", SUBFT_ROAMING_ASSISTANT_5G, FT_WIRELESS },
	{ "roaming_assistant_5g1", SUBFT_ROAMING_ASSISTANT_5G1, FT_WIRELESS },
	/* Region */
	{ "region", SUBFT_REGION, FT_REGION },
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	{ "bsd_steering_policy_2g",  SUBFT_BSD_STEERING_POLICY_2G, FT_WIRELESS },
	{ "bsd_steering_policy_5g",  SUBFT_BSD_STEERING_POLICY_5G, FT_WIRELESS },
	{ "bsd_steering_policy_5g_x",  SUBFT_BSD_STEERING_POLICY_5G_X, FT_WIRELESS },
	{ "bsd_steering_policy_5g1",  SUBFT_BSD_STEERING_POLICY_5G1, FT_WIRELESS },
	{ "bsd_steering_policy_5g1_x",  SUBFT_BSD_STEERING_POLICY_5G1_X, FT_WIRELESS },
	/* sub feature for smart connect rule (STA Selection Policy) */
	{ "bsd_sta_select_policy_2g", SUBFT_BSD_STA_SELECT_POLICY_2G, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g", SUBFT_BSD_STA_SELECT_POLICY_5G, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g_x", SUBFT_BSD_STA_SELECT_POLICY_5G_X, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g1", SUBFT_BSD_STA_SELECT_POLICY_5G1, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g1_x", SUBFT_BSD_STA_SELECT_POLICY_5G1_X, FT_WIRELESS },
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	{ "bsd_if_select_2g", SUBFT_BSD_IF_SELECT_2G, FT_WIRELESS },
	{ "bsd_if_select_5g", SUBFT_BSD_IF_SELECT_5G, FT_WIRELESS },
	{ "bsd_if_select_5g_x", SUBFT_BSD_IF_SELECT_5G_X, FT_WIRELESS },
	{ "bsd_if_select_5g1", SUBFT_BSD_IF_SELECT_5G1, FT_WIRELESS },
	{ "bsd_if_select_5g1_x", SUBFT_BSD_IF_SELECT_5G1_X, FT_WIRELESS },
	{ "bsd_if_qualify_2g", SUBFT_BSD_IF_QUALIFY_2G, FT_WIRELESS },
	{ "bsd_if_qualify_5g", SUBFT_BSD_IF_QUALIFY_5G, FT_WIRELESS },
	{ "bsd_if_qualify_5g_x", SUBFT_BSD_IF_QUALIFY_5G_X, FT_WIRELESS },
	{ "bsd_if_qualify_5g1", SUBFT_BSD_IF_QUALIFY_5G1, FT_WIRELESS },
	{ "bsd_if_qualify_5g1_x", SUBFT_BSD_IF_QUALIFY_5G1_X, FT_WIRELESS },
	/* sub feature for smart connect rule (Bounce Detect) */
	{ "bsd_bounce_detect", SUBFT_BSD_BOUNCE_DETECT, FT_WIRELESS },
	{ "bsd_bounce_detect_x", SUBFT_BSD_BOUNCE_DETECT_X, FT_WIRELESS },
	/* sub feature for WPS enable */
	{ "wps", SUBFT_WPS, FT_WIRELESS },
	/* sub feature for Reboot schedule */
	{ "reboot_schedule", SUBFT_REBOOT_SCHEDULE, FT_TIME },
#if defined(MAPAC2200) || defined(RTAC92U)
	{ "ncb",		SUBFT_NCB,	FT_BHBLOCK },
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd",		SUBFT_SPCMD,	FT_SPCMD },
#endif
	{ "lp55xx_led",		SUBFT_LP55XX_LED,	FT_LP55XX_LED },
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
	{ "wl0_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_2G},
	{ "wl0_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_2G},
	{ "wl0_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_2G},
	{ "wl0_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_2G},
	{ "wl0_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_2G},
	{ "wl0_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_2G},
	{ "wl0_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_2G},
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
	{ "wl1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_5G},
	{ "wl1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G},
	{ "wl1_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_5G},
	{ "wl1_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_5G},
	{ "wl1_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_5G},
	{ "wl1_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_5G},
	{ "wl1_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_5G},
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
	{ "wl2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	{ "wl2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_5G1},
	{ "wl2_timesched",	FT_WIRELESS,		SUBFT_TIMESCHED_5G1},
	{ "wl2_sched",	FT_WIRELESS,		SUBFT_TIMESCHED_5G1},
	{ "wl2_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_5G1},
	{ "wl2_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_5G1},
	{ "wl2_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_5G1},
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
	{ "fb_serviceno", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_ptype", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_pdesc", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_country", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_browserInfo", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_syslog", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_cfgfile", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_modemlog", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_wlanlog", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
#ifdef RTCONFIG_DSL
	{ "fb_ISP", 				FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_availability", 			FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "fb_Subscribed_Info", 	FT_FEEDBACK,		SUBFT_FEEDBACK},
	{ "PM_attach_iptables", 		FT_FEEDBACK,		SUBFT_FEEDBACK},
#endif
	{ "oauth_google_refresh_token", FT_FEEDBACK, SUBFT_FEEDBACK },
	{ "oauth_google_user_email", FT_FEEDBACK, SUBFT_FEEDBACK },
	{ "fb_email_provider", FT_FEEDBACK, SUBFT_FEEDBACK },
	/* diagnostic */
	{ "dblog_enable", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_tousb", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_service", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_duration", 	FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	{ "dblog_transid", 		FT_DIAGNOSTIC,		SUBFT_DIAGNOSTIC},
	/* backhaul ctrl */
	{ "amas_ethernet", 	FT_BACKHAULCTRL,	SUBFT_BACKHAULCTRL},
	{ "smart_connect_x", 	FT_WIRELESS,	SUBFT_SMART_CONNECT},
	/* led */
	{ "bc_ledLv",	FT_CENTRAL_LED,		SUBFT_CENTRAL_LED},	/* for BLUECAVE */
	{ "lp55xx_lp5523_user_enable",	FT_LP55XX_LED,		SUBFT_LP55XX_LED},	/* for Lyra */
	{ "lp55xx_lp5523_user_col",	FT_LP55XX_LED,		SUBFT_LP55XX_LED},	/* for Lyra */
	{ "lp55xx_lp5523_user_beh",	FT_LP55XX_LED,		SUBFT_LP55XX_LED},	/* for Lyra */
	/* Roaming Assistant */
	{ "rast_static_cli_enable", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT },
	{ "wl0_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_2G, },
	{ "wl1_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_5G, },
	{ "wl2_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_5G1, },
	/* Region */
	{ "location_code", FT_REGION, SUBFT_REGION },
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	{ "wl0_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_2G },
	{ "wl1_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G },
	{ "wl2_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G1 },
	{ "wl1_bsd_steering_policy_x", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G_X },
	{ "wl2_bsd_steering_policy_x", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G1_X },
	/* sub feature for smart connect rule (STA Selection Policy) */
	{ "wl0_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_2G },
	{ "wl1_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G },
	{ "wl2_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G1 },
	{ "wl1_bsd_sta_select_policy_x", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G_X },
	{ "wl2_bsd_sta_select_policy_x", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G1_X },
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	{ "wl0_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_2G },
	{ "wl1_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G },
	{ "wl2_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G1 },
	{ "wl1_bsd_if_select_policy_x", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G_X },
	{ "wl2_bsd_if_select_policy_x", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G1_X },
	{ "wl0_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_2G },
	{ "wl1_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G },
	{ "wl2_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G1 },
	{ "wl1_bsd_if_qualify_policy_x", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G_X },
	{ "wl2_bsd_if_qualify_policy_x", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G1_X },
	/* sub feature for smart connect rule (Bounce Detect) */
	{ "bsd_bounce_detect", FT_WIRELESS, SUBFT_BSD_BOUNCE_DETECT },
	{ "bsd_bounce_detect_x", FT_WIRELESS, SUBFT_BSD_BOUNCE_DETECT_X },
	/* sub feature for WPS enable */
	{ "wps_enable", FT_WIRELESS, SUBFT_WPS },
	/* sub feature for Reboot schedule */
	{ "reboot_schedule_enable", FT_TIME, SUBFT_REBOOT_SCHEDULE },
	{ "reboot_schedule", FT_TIME, SUBFT_REBOOT_SCHEDULE },
	/* sub feature for Wireless Professional */
	/* isolate */
	{ "wl0.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_2G_G1 },
	{ "wl0.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_2G_G2 },
	{ "wl0.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_2G_G3 },
	{ "wl1.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G_G1 },
	{ "wl1.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G_G2 },
	{ "wl1.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G_G3 },
	{ "wl2.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G1_G1 },
	{ "wl2.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G1_G2 },
	{ "wl2.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G1_G3 },
	{ "wl0_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* IGMP Snooping */
	{ "wl0_igs", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_igs", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_igs", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Multicast Rate(Mbps) */
	{ "wl0_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* AMPDU RTS */
	{ "wl0_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* RTS Threshold */
	{ "wl0_rts", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_rts", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_rts", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* DTIM Interval */
	{ "wl0_dtim", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_dtim", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_dtim", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Enable WMM APSD */
	{ "wl0_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Optimize AMPDU aggregation */
	{ "wl0_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Enable Airtime Fairness */
	{ "wl0_atf", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_atf", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_atf", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* 802.11ac Beamforming */
	{ "wl0_txbf", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_txbf", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_txbf", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Universal Beamforming */
	{ "wl0_itxbf", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_itxbf", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_itxbf", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
	/* Tx power adjustment */
	{ "wl0_txpower", FT_WIRELESS, SUBFT_ADVANCED_2G },
	{ "wl1_txpower", FT_WIRELESS, SUBFT_ADVANCED_5G },
	{ "wl2_txpower", FT_WIRELESS, SUBFT_ADVANCED_5G1 },
#if defined(MAPAC2200) || defined(RTAC92U)
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
	{ "closed", NULL },
	{ "radius_ipaddr", NULL },
	{ "radius_key", NULL },
	{ "radius_port", NULL },
	{ NULL, 		NULL }
};

#endif /* __CFG_PARAM_H__ */
/* End of cfg_param.h */
