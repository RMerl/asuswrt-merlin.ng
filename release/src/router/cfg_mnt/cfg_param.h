#ifndef __CFG_PARAM_H__
#define __CFG_PARAM_H__

/* parameter list table */
#if defined(BIT_ULL)
#undef BIT_ULL
#endif
#define BIT_ULL(x) (x + 1)

/* feature */
#define FT_WIRELESS	BIT_ULL(0)
#define FT_LOGIN	BIT_ULL(1)
#define FT_TIME		BIT_ULL(2)
#define FT_MISC		BIT_ULL(3)
#define FT_LOGGER		BIT_ULL(4)
#define FT_FEEDBACK		BIT_ULL(5)
#define FT_DIAGNOSTIC	BIT_ULL(6)
#define FT_BACKHAULCTRL	BIT_ULL(7)

#define FT_REGION	BIT_ULL(8)	/* system reboot */
#define FT_CENTRAL_LED		BIT_ULL(9)

#if defined(MAPAC2200) || defined(RTAC95U)
#define FT_BHBLOCK	BIT_ULL(10)	/* normal client blocking in backhaul */
#endif
#ifdef RTCONFIG_WIFI_SON
#define FT_SPCMD	BIT_ULL(11)	/* special command */
#endif
#define FT_LP55XX_LED	BIT_ULL(12)	/* control lp55xx led */
#define FT_LINK_AGGREGATION    BIT_ULL(13) /* link aggregation */
#define FT_CTRL_LED		BIT_ULL(14)
#define FT_AURARGB		BIT_ULL(15)
#if defined(RTCONFIG_STA_AP_BAND_BIND)
#define FT_STA_BIND_AP		BIT_ULL(16)
#endif
#ifdef RTCONFIG_DWB
#define FT_DWBCTRL	BIT_ULL(17) /* DWB feature */
#endif
#ifdef RTCONFIG_BHCOST_OPT
#define	FT_PREFERAP	BIT_ULL(18)  /* Prefer AP feature */
#endif
#if defined(RTCONFIG_FANCTRL)
#define FT_FANCTRL		BIT_ULL(19)
#endif
#ifdef RTCONFIG_AMAS_WGN
#define FT_BW_LIMIT BIT_ULL(20) /* Bandwidth limiter for guest network feature */
#endif
#define FT_PLC_MASTER BIT_ULL(21)
#define FT_LOCAL_ACCESS	BIT_ULL(22)
#ifdef RTCONFIG_AMAS_SYNC_LEDG
#define FT_LEDG	BIT_ULL(23)
#endif
#define FT_MOCA   BIT_ULL(33)
#define FT_PRIVACY_POLICY   BIT_ULL(34)
#define FT_NEW_EULA    BIT_ULL(41)

/* service */
#define RESTART_WIRELESS		"restart_wireless"
#define CHPASS		"chpass"
#define RESTART_TIME		"restart_time"
#define RESTART_FANCTRL		"restart_fanctrl"
#define RESTART_LOGGER		"restart_logger"
#define RESTART_SENDFEEDBACK		"restart_sendfeedback"
#define RESTART_DBLOG		"restart_dblog"
#define RESTART_AMAS_BHCTRL	"restart_amas_bhctrl"
#define RESET_LED		"reset_led"
#define REBOOT		"reboot"
#define RESTART_PLC_MASTER	"restart_plc_master"
#ifdef RTCONFIG_WIFI_SON
#if defined(MAPAC2200)
#define RESTART_BHBLOCK		"restart_bhblock"
#endif
#define RESTART_SPCMD		"restart_spcmd"
#endif
#define CTRL_LED			"ctrl_led"
#define START_AURARGB	"start_aurargb"
#if defined(RTCONFIG_STA_AP_BAND_BIND)
#define UPDATE_STA_BINDING	"update_sta_binding"
#endif
#define TRIGGER_OPT		"trigger_opt"
#ifdef RTCONFIG_AMAS_WGN
#define RESTART_BW_LIMIT	"restart_qos;restart_firewall"
#endif
#define RESTART_HTTPD	"restart_httpd"
#ifdef RTCONFIG_AMAS_SYNC_LEDG
#define RESTART_LEDG	"restart_ledg"
#endif
#define MOCA_SET_PRIVACY	"moca_set_privacy"

struct feature_mapping_s {
	char *name;
	int index;
	char *service;
};

static struct feature_mapping_s feature_mapping_list[] __attribute__ ((unused)) = {
	{ "wireless", 	FT_WIRELESS,	RESTART_WIRELESS },
	{ "login", 	FT_LOGIN,		CHPASS },
	{ "time",		FT_TIME,		RESTART_TIME },
	{ "misc",		FT_MISC,			NULL },
	{ "logger",	FT_LOGGER,		RESTART_LOGGER },
	{ "feedback",	FT_FEEDBACK,	RESTART_SENDFEEDBACK },
	{ "diagnostic",	FT_DIAGNOSTIC,	RESTART_DBLOG },
	{ "backhalctrl", 	FT_BACKHAULCTRL,	RESTART_AMAS_BHCTRL },
	{ "central_led",	FT_CENTRAL_LED,	RESET_LED },
	/* END */
	{ "region", FT_REGION, 	REBOOT },
#ifdef RTCONFIG_WIFI_SON
#if defined(MAPAC2200)
	{ "bhblock", 	FT_BHBLOCK,	RESTART_BHBLOCK },
#endif
	{ "spcmd", 	FT_SPCMD,	RESTART_SPCMD },
#endif
	{ "lp55xx_led", FT_LP55XX_LED,	RESET_LED },
	{ "link_aggregation", FT_LINK_AGGREGATION,	REBOOT },
	{ "ctrl_led", FT_CTRL_LED,	CTRL_LED },
	{ "aurargb", FT_AURARGB,	START_AURARGB },
#if defined(RTCONFIG_STA_AP_BAND_BIND)
	{ "sta_bind_ap", FT_STA_BIND_AP,	UPDATE_STA_BINDING },
#endif
#ifdef RTCONFIG_DWB
	{ "dwbctrl", FT_DWBCTRL, RESTART_WIRELESS },
#endif
#ifdef RTCONFIG_BHCOST_OPT
	{ "prefer_ap", FT_PREFERAP, TRIGGER_OPT },
#endif
#ifdef RTCONFIG_AMAS_WGN
	{ "bw_limiter", FT_BW_LIMIT, RESTART_BW_LIMIT},
#endif
#if defined(RTCONFIG_FANCTRL)
	{ "fanctrl",	FT_FANCTRL,	RESTART_FANCTRL },
#endif
#ifdef RTCONFIG_QCA_PLC2
	{ "plc_master",	FT_PLC_MASTER,	RESTART_PLC_MASTER },
#endif
	{ "local_access", 	FT_LOCAL_ACCESS,	RESTART_HTTPD },
#ifdef RTCONFIG_AMAS_SYNC_LEDG
	{ "ledg", 	FT_LEDG,	RESTART_LEDG },
#endif
	{ "moca", 	FT_MOCA,	MOCA_SET_PRIVACY },
	{ "privacy_policy",	FT_PRIVACY_POLICY,	NULL },
	{ "new_eula",	FT_NEW_EULA,	NULL },
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
	SUBFT_BASIC_BAND1 = 1,
	SUBFT_BASIC_BAND2,
	SUBFT_BASIC_BAND3,
	SUBFT_BASIC_BAND4,
	SUBFT_CHANNEL_BAND1,
	SUBFT_CHANNEL_BAND2,
	SUBFT_CHANNEL_BAND3,
	SUBFT_CHANNEL_BAND4,
	SUBFT_MACFILTER_BAND1,
	SUBFT_MACFILTER_BAND2,
	SUBFT_MACFILTER_BAND3,
	SUBFT_MACFILTER_BAND4,
	SUBFT_RADIUS_BAND1,
	SUBFT_RADIUS_BAND2,
	SUBFT_RADIUS_BAND3,
	SUBFT_RADIUS_BAND4,
	SUBFT_TIMESCHED_BAND1,
	SUBFT_TIMESCHED_BAND2,
	SUBFT_TIMESCHED_BAND3,
	SUBFT_TIMESCHED_BAND4,
	SUBFT_BASIC_BAND1_G1,
	SUBFT_BASIC_BAND2_G1,
	SUBFT_BASIC_BAND3_G1,
	SUBFT_BASIC_BAND4_G1,
	SUBFT_MACFILTER_BAND1_G1,
	SUBFT_MACFILTER_BAND2_G1,
	SUBFT_MACFILTER_BAND3_G1,
	SUBFT_MACFILTER_BAND4_G1,
	SUBFT_BASIC_BAND1_G2,
	SUBFT_BASIC_BAND2_G2,
	SUBFT_BASIC_BAND3_G2,
	SUBFT_BASIC_BAND4_G2,
	SUBFT_MACFILTER_BAND1_G2,
	SUBFT_MACFILTER_BAND2_G2,
	SUBFT_MACFILTER_BAND3_G2,
	SUBFT_MACFILTER_BAND4_G2,
	SUBFT_BASIC_BAND1_G3,
	SUBFT_BASIC_BAND2_G3,
	SUBFT_BASIC_BAND3_G3,
	SUBFT_BASIC_BAND4_G3,
	SUBFT_MACFILTER_BAND1_G3,
	SUBFT_MACFILTER_BAND2_G3,
	SUBFT_MACFILTER_BAND3_G3,
	SUBFT_MACFILTER_BAND4_G3,
	SUBFT_ADVANCED_BAND1,
	SUBFT_ADVANCED_BAND2,
	SUBFT_ADVANCED_BAND3,
	SUBFT_ADVANCED_BAND4,
	SUBFT_ADVANCED_BAND1_G1,
	SUBFT_ADVANCED_BAND1_G2,
	SUBFT_ADVANCED_BAND1_G3,
	SUBFT_ADVANCED_BAND2_G1,
	SUBFT_ADVANCED_BAND2_G2,
	SUBFT_ADVANCED_BAND2_G3,
	SUBFT_ADVANCED_BAND3_G1,
	SUBFT_ADVANCED_BAND3_G2,
	SUBFT_ADVANCED_BAND3_G3,
	SUBFT_ADVANCED_BAND4_G1,
	SUBFT_ADVANCED_BAND4_G2,
	SUBFT_ADVANCED_BAND4_G3,
	SUBFT_RADIO_BAND1,
	SUBFT_RADIO_BAND2,
	SUBFT_RADIO_BAND3,
	SUBFT_RADIO_BAND4,

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
#ifdef RTCONFIG_BHCOST_OPT
	SUBFT_BACKHAULCTRL_EAP, /* backhaul ctrl for EAP function */
#endif
#ifdef RTCONFIG_DWB
#ifdef RTCONFIG_FRONTHAUL_DWB
	SUBFT_DWBCTRL_FRONTHAUL, /* Fronthauk AP for DWB feature */
#endif
#endif
	/* sub feature for smart connect */
	SUBFT_SMART_CONNECT,	/* smart connect */
#if defined(MAPAC2200) || defined(RTAC95U)
	SUBFT_NCB,
#endif
#ifdef RTCONFIG_WIFI_SON
	SUBFT_SPCMD,
#endif
	SUBFT_CENTRAL_LED,
	SUBFT_LP55XX_LED,	/* LP55XX LED */

	/* sub feature for Roaming Assistant */
	SUBFT_ROAMING_ASSISTANT,
	SUBFT_ROAMING_ASSISTANT_BAND1,
	SUBFT_ROAMING_ASSISTANT_BAND2,
	SUBFT_ROAMING_ASSISTANT_BAND3,
	SUBFT_ROAMING_ASSISTANT_BAND4,
	/* sub feature for Region */
	SUBFT_REGION,
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	SUBFT_BSD_STEERING_POLICY_BAND1,
	SUBFT_BSD_STEERING_POLICY_BAND2,
	SUBFT_BSD_STEERING_POLICY_BAND3,
	SUBFT_BSD_STEERING_POLICY_BAND4,
	SUBFT_BSD_STEERING_POLICY_5G_X,		// 5g only
	SUBFT_BSD_STEERING_POLICY_5G1_X,	// 5g only
	/* sub feature for smart connect rule (STA Selection Policy) */
	SUBFT_BSD_STA_SELECT_POLICY_BAND1,
	SUBFT_BSD_STA_SELECT_POLICY_BAND2,
	SUBFT_BSD_STA_SELECT_POLICY_BAND3,
	SUBFT_BSD_STA_SELECT_POLICY_BAND4,
	SUBFT_BSD_STA_SELECT_POLICY_5G_X,	// 5g only
	SUBFT_BSD_STA_SELECT_POLICY_5G1_X,	// 5g only
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	SUBFT_BSD_IF_SELECT_BAND1,
	SUBFT_BSD_IF_SELECT_BAND2,
	SUBFT_BSD_IF_SELECT_BAND3,
	SUBFT_BSD_IF_SELECT_BAND4,
	SUBFT_BSD_IF_SELECT_5G_X,			// 5g only
	SUBFT_BSD_IF_SELECT_5G1_X,			// 5g only
	SUBFT_BSD_IF_QUALIFY_BAND1,
	SUBFT_BSD_IF_QUALIFY_BAND2,
	SUBFT_BSD_IF_QUALIFY_BAND3,
	SUBFT_BSD_IF_QUALIFY_BAND4,
	SUBFT_BSD_IF_QUALIFY_5G_X,			// 5g only
	SUBFT_BSD_IF_QUALIFY_5G1_X,			// 5g only
	/* sub feature for smart connect rule (Bounce Detect) */
	SUBFT_BSD_BOUNCE_DETECT,
	SUBFT_BSD_BOUNCE_DETECT_X,			// 5g only
	/* sub feature for WPS */
	SUBFT_WPS,
	/* sub feature for Reboot schedule */
	SUBFT_REBOOT_SCHEDULE,
	/* sub feature for bandwidth 160 support */
	SUBFT_BW_160_BAND1,
	SUBFT_BW_160_BAND2,
	SUBFT_BW_160_BAND3,
	SUBFT_BW_160_BAND4,
	/* sub feature for HE ddfeatures */
	SUBFT_HE_FEATURES_BAND1,
	SUBFT_HE_FEATURES_BAND2,
	SUBFT_HE_FEATURES_BAND3,
	SUBFT_HE_FEATURES_BAND4,
	/* sub feature for ACS include DFS */
	SUBFT_ACS_INCLUDE_DFS,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_VLAN_RULELIST,
#endif	/* RTCONFIG_AMAS_WGN */
	SUBFT_GUEST_MISC_BAND1_G1,
	SUBFT_GUEST_MISC_BAND1_G2,
	SUBFT_GUEST_MISC_BAND1_G3,
	SUBFT_GUEST_MISC_BAND2_G1,
	SUBFT_GUEST_MISC_BAND2_G2,
	SUBFT_GUEST_MISC_BAND2_G3,
	SUBFT_GUEST_MISC_BAND3_G1,
	SUBFT_GUEST_MISC_BAND3_G2,
	SUBFT_GUEST_MISC_BAND3_G3,
	SUBFT_GUEST_MISC_BAND4_G1,
	SUBFT_GUEST_MISC_BAND4_G2,
	SUBFT_GUEST_MISC_BAND4_G3,
	/* sub feature for roamast rssi gather method */
	SUBFT_RSSI_METHOD,
#ifdef RTCONFIG_BHCOST_OPT    
	/* sub feature for force topology */
	SUBFT_FORCE_TOPOLOGY,
#endif
	/* sub feature for link aggregation */
	SUBFT_LINK_AGGREGATION,

	/* sub feature for connection diagmostic */
	SUBFT_CONNECTION_DIAGMOSTIC,

	/* sub feature for control led on/of or brightness */
	SUBFT_CTRL_LED,

	/* sub feature for control aura rgb */
	SUBFT_AURARGB,

	/* sub feature for 802.11ax/Wi-Fi 6 mode */
	SUBFT_11_AX_BAND1,
	SUBFT_11_AX_BAND2,
	SUBFT_11_AX_BAND3,
	SUBFT_11_AX_BAND4,

	/* sub feature for OFDMA */
	SUBFT_OFDMA_BAND1,
	SUBFT_OFDMA_BAND2,
	SUBFT_OFDMA_BAND3,
	SUBFT_OFDMA_BAND4,
	
#if defined(RTCONFIG_WIFI7) 
#if defined(RTCONFIG_BCMWL6) 
	/* sub feature for be_ofdma */
	SUBFT_BE_OFDMA_BAND1,
	SUBFT_BE_OFDMA_BAND2,
	SUBFT_BE_OFDMA_BAND3,
	SUBFT_BE_OFDMA_BAND4,
	
	/* sub feature for be_mumimo */
	SUBFT_BE_MUMIMO_BAND1,
	SUBFT_BE_MUMIMO_BAND2,
	SUBFT_BE_MUMIMO_BAND3,
	SUBFT_BE_MUMIMO_BAND4,
#endif
#endif

#if defined(RTCONFIG_STA_AP_BAND_BIND)
	/* sub feature for sta binding ap */
	SUBFT_STA_BIND_AP,
#endif

	/* wifi schedule v2 */
	SUBFT_TIMESCHEDV2_BAND1,
	SUBFT_TIMESCHEDV2_BAND2,
	SUBFT_TIMESCHEDV2_BAND3,
	SUBFT_TIMESCHEDV2_BAND4,

#if defined(RTCONFIG_AMAS_WGN)
	/* bandwidth limiter for guest network */
    SUBFT_BW_LIMIT,
    SUBFT_BW_LIMIT_BAND1_G1,
    SUBFT_BW_LIMIT_BAND1_G2,
    SUBFT_BW_LIMIT_BAND1_G3,
    SUBFT_BW_LIMIT_BAND2_G1,
    SUBFT_BW_LIMIT_BAND2_G2,
    SUBFT_BW_LIMIT_BAND2_G3,
    SUBFT_BW_LIMIT_BAND3_G1,
    SUBFT_BW_LIMIT_BAND3_G2,
    SUBFT_BW_LIMIT_BAND3_G3,
    SUBFT_BW_LIMIT_BAND4_G1,
    SUBFT_BW_LIMIT_BAND4_G2,
    SUBFT_BW_LIMIT_BAND4_G3,
#endif

#if defined(RTCONFIG_FANCTRL)
	SUBFT_FANCTRL,
#endif
	SUBFT_PLC_MASTER,
	SUBFT_LOCAL_ACCESS,

	/* sub feature for Target Wake Time */
	SUBFT_TWT_BAND1,
	SUBFT_TWT_BAND2,
	SUBFT_TWT_BAND3,
	SUBFT_TWT_BAND4,

#ifdef RTCONFIG_AMAS_SYNC_LEDG
	SUBFT_LEDG,
#endif

	SUBFT_MOCA,

#ifdef RTCONFIG_ROUTERBOOST
	SUBFT_RB_ENABLE,
#endif
	SUBFT_GENCERT,
	SUBFT_PRIVACY_POLICY,
	SUBFT_NEW_EULA,

	/* sub feature for disable 11b */
	SUBFT_DISABLE_11B_BAND1,
	SUBFT_DISABLE_11B_BAND2,
	SUBFT_DISABLE_11B_BAND3,
	SUBFT_DISABLE_11B_BAND4,

	SUBFT_MAX
};

static struct subfeature_mapping_s subfeature_mapping_list[] __attribute__ ((unused)) = {
	/* for wireless */
	{ "basic_2g",	 	SUBFT_BASIC_BAND1,		FT_WIRELESS },
	{ "basic_5g", 		SUBFT_BASIC_BAND2,		FT_WIRELESS },
	{ "basic_5g1",		SUBFT_BASIC_BAND3,	FT_WIRELESS },
	{ "basic_b4",		SUBFT_BASIC_BAND4,	FT_WIRELESS },
	{ "channel_2g",		SUBFT_CHANNEL_BAND1,	FT_WIRELESS },
	{ "channel_5g",		SUBFT_CHANNEL_BAND2,	FT_WIRELESS },
	{ "channel_5g1",	SUBFT_CHANNEL_BAND3,	FT_WIRELESS },
	{ "channel_b4",		SUBFT_CHANNEL_BAND4,	FT_WIRELESS },
	{ "macfilter_2g", 	SUBFT_MACFILTER_BAND1,	FT_WIRELESS },
	{ "macfilter_5g", 	SUBFT_MACFILTER_BAND2,	FT_WIRELESS },
	{ "macfilter_5g1",	SUBFT_MACFILTER_BAND3,	FT_WIRELESS },
	{ "macfilter_b4",	SUBFT_MACFILTER_BAND4,	FT_WIRELESS },
	{ "radius_2g", 	SUBFT_RADIUS_BAND1,	FT_WIRELESS },
	{ "radius_5g", 	SUBFT_RADIUS_BAND2,	FT_WIRELESS },
	{ "radius_5g1",	SUBFT_RADIUS_BAND3,	FT_WIRELESS },
	{ "radius_b4",	SUBFT_RADIUS_BAND4,	FT_WIRELESS },
	{ "timesched_2g", 	SUBFT_TIMESCHED_BAND1,	FT_WIRELESS },
	{ "timesched_5g", 	SUBFT_TIMESCHED_BAND2,	FT_WIRELESS },
	{ "timesched_5g1",	SUBFT_TIMESCHED_BAND3,	FT_WIRELESS },
	{ "timesched_b4",	SUBFT_TIMESCHED_BAND4,	FT_WIRELESS },
#ifdef RTCONFIG_WL_SCHED_V2
	{ "timeschedv2_2g", 	SUBFT_TIMESCHEDV2_BAND1,	FT_MISC },
	{ "timeschedv2_5g", 	SUBFT_TIMESCHEDV2_BAND2,	FT_MISC },
	{ "timeschedv2_5g1",	SUBFT_TIMESCHEDV2_BAND3,	FT_MISC },
	{ "timeschedv2_b4",	SUBFT_TIMESCHEDV2_BAND4,	FT_MISC },
#endif
	{ "basic_2g_g1", 	SUBFT_BASIC_BAND1_G1,	FT_WIRELESS },
	{ "basic_5g_g1", 	SUBFT_BASIC_BAND2_G1,	FT_WIRELESS },
	{ "basic_5g1_g1",	SUBFT_BASIC_BAND3_G1,	FT_WIRELESS },
	{ "basic_b4_g1",	SUBFT_BASIC_BAND4_G1,	FT_WIRELESS },
	{ "macfilter_2g_g1", 	SUBFT_MACFILTER_BAND1_G1,	FT_WIRELESS },
	{ "macfilter_5g_g1", 	SUBFT_MACFILTER_BAND2_G1,	FT_WIRELESS },
	{ "macfilter_5g1_g1",	SUBFT_MACFILTER_BAND3_G1,	FT_WIRELESS },
	{ "macfilter_b4_g1",	SUBFT_MACFILTER_BAND4_G1,	FT_WIRELESS },
	{ "basic_2g_g2", 	SUBFT_BASIC_BAND1_G2,	FT_WIRELESS },
	{ "basic_5g_g2", 	SUBFT_BASIC_BAND2_G2,	FT_WIRELESS },
	{ "basic_5g1_g2",	SUBFT_BASIC_BAND3_G2,	FT_WIRELESS },
	{ "basic_b4_g2",	SUBFT_BASIC_BAND4_G2,	FT_WIRELESS },
	{ "macfilter_2g_g2", 	SUBFT_MACFILTER_BAND1_G2,	FT_WIRELESS },
	{ "macfilter_5g_g2", 	SUBFT_MACFILTER_BAND2_G2,	FT_WIRELESS },
	{ "macfilter_5g1_g2",	SUBFT_MACFILTER_BAND3_G2,	FT_WIRELESS },
	{ "macfilter_b4_g2",	SUBFT_MACFILTER_BAND4_G2,	FT_WIRELESS },
	{ "basic_2g_g3", 	SUBFT_BASIC_BAND1_G3,	FT_WIRELESS },
	{ "basic_5g_g3", 	SUBFT_BASIC_BAND2_G3,	FT_WIRELESS },
	{ "basic_5g1_g3",	SUBFT_BASIC_BAND3_G3,	FT_WIRELESS },
	{ "basic_b4_g3",	SUBFT_BASIC_BAND4_G3,	FT_WIRELESS },
	{ "macfilter_2g_g3", 	SUBFT_MACFILTER_BAND1_G3,	FT_WIRELESS },
	{ "macfilter_5g_g3", 	SUBFT_MACFILTER_BAND2_G3,	FT_WIRELESS },
	{ "macfilter_5g1_g3",	SUBFT_MACFILTER_BAND3_G3,	FT_WIRELESS },
	{ "macfilter_b4_g3",	SUBFT_MACFILTER_BAND4_G3,	FT_WIRELESS },
	{ "advanced_2g",	 	SUBFT_ADVANCED_BAND1,		FT_WIRELESS },
	{ "advanced_5g", 		SUBFT_ADVANCED_BAND2,		FT_WIRELESS },
	{ "advanced_5g1",		SUBFT_ADVANCED_BAND3,		FT_WIRELESS },
	{ "advanced_b4",		SUBFT_ADVANCED_BAND4,		FT_WIRELESS },
	{ "advanced_2g_g1", 	SUBFT_ADVANCED_BAND1_G1, 	FT_WIRELESS },
	{ "advanced_2g_g2", 	SUBFT_ADVANCED_BAND1_G2, 	FT_WIRELESS },
	{ "advanced_2g_g3", 	SUBFT_ADVANCED_BAND1_G3, 	FT_WIRELESS },
	{ "advanced_5g_g1", 	SUBFT_ADVANCED_BAND2_G1, 	FT_WIRELESS },
	{ "advanced_5g_g2", 	SUBFT_ADVANCED_BAND2_G2, 	FT_WIRELESS },
	{ "advanced_5g_g3", 	SUBFT_ADVANCED_BAND2_G3, 	FT_WIRELESS },
	{ "advanced_5g1_g1", 	SUBFT_ADVANCED_BAND3_G1, 	FT_WIRELESS },
	{ "advanced_5g1_g2", 	SUBFT_ADVANCED_BAND3_G2, 	FT_WIRELESS },
	{ "advanced_5g1_g3", 	SUBFT_ADVANCED_BAND3_G3, 	FT_WIRELESS },
	{ "advanced_b4_g1", 	SUBFT_ADVANCED_BAND4_G1, 	FT_WIRELESS },
	{ "advanced_b4_g2", 	SUBFT_ADVANCED_BAND4_G2, 	FT_WIRELESS },
	{ "advanced_b4_g3", 	SUBFT_ADVANCED_BAND4_G3, 	FT_WIRELESS },
	{ "radio_2g",	SUBFT_RADIO_BAND1,		FT_WIRELESS },
	{ "radio_5g",	SUBFT_RADIO_BAND2,		FT_WIRELESS },
	{ "radio_5g1",		SUBFT_RADIO_BAND3,	FT_WIRELESS },
	{ "radio_b4",		SUBFT_RADIO_BAND4,	FT_WIRELESS },

	/* administration */
	{ "router_login", 	SUBFT_ROUTER_LOGIN,	FT_LOGIN },
	{ "local_access", 	SUBFT_LOCAL_ACCESS,	FT_LOCAL_ACCESS },
	{ "time_zone",		SUBFT_TIMEZONE,		FT_TIME },
	{ "ntp_server",		SUBFT_NTP_SERVER,	FT_TIME },
	{ "telnet_server",		SUBFT_TELNET_SERVER,	FT_TIME },
	{ "ssh_server",		SUBFT_SSH_SERVER,	FT_TIME },
	{ "log_server",		SUBFT_LOG_SERVER,	FT_LOGGER },
	{ "location",		SUBFT_LOCATION,	FT_MISC },
	{ "misc",		SUBFT_MISCELLANEOUS,	FT_MISC },
	{ "rootcerts",	 	SUBFT_GENCERT,		FT_MISC },		/* fake nvram variable, use it to ask CAP to send root cert/key */
	/* feedback and diagnostic */
	{ "feedback",		SUBFT_FEEDBACK,		FT_FEEDBACK },
	{ "diagnostic",		SUBFT_DIAGNOSTIC,	FT_DIAGNOSTIC},
	/* backhaul ctrl */
	{ "backhalctrl",		SUBFT_BACKHAULCTRL,	FT_BACKHAULCTRL },
#ifdef RTCONFIG_BHCOST_OPT
	{ "backhalctrl_eap",	SUBFT_BACKHAULCTRL_EAP, FT_BACKHAULCTRL },
#endif
#ifdef RTCONFIG_DWB
#ifdef RTCONFIG_FRONTHAUL_DWB
	{ "dwbctrl",	SUBFT_DWBCTRL_FRONTHAUL, FT_DWBCTRL },
#endif
#endif
	{ "smart_connect", 	SUBFT_SMART_CONNECT, FT_WIRELESS },
	{ "central_led",		SUBFT_CENTRAL_LED,	FT_CENTRAL_LED },
	/* Roaming Assistant */
	{ "roaming_assistant", SUBFT_ROAMING_ASSISTANT, FT_WIRELESS },
	{ "roaming_assistant_2g", SUBFT_ROAMING_ASSISTANT_BAND1, FT_WIRELESS },
	{ "roaming_assistant_5g", SUBFT_ROAMING_ASSISTANT_BAND2, FT_WIRELESS },
	{ "roaming_assistant_5g1", SUBFT_ROAMING_ASSISTANT_BAND3, FT_WIRELESS },
	{ "roaming_assistant_b4", SUBFT_ROAMING_ASSISTANT_BAND4, FT_WIRELESS },
	/* Region */
	{ "region", SUBFT_REGION, FT_REGION },
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	{ "bsd_steering_policy_2g",  SUBFT_BSD_STEERING_POLICY_BAND1, FT_WIRELESS },
	{ "bsd_steering_policy_5g",  SUBFT_BSD_STEERING_POLICY_BAND2, FT_WIRELESS },
	{ "bsd_steering_policy_5g_x",  SUBFT_BSD_STEERING_POLICY_5G_X, FT_WIRELESS },
	{ "bsd_steering_policy_5g1",  SUBFT_BSD_STEERING_POLICY_BAND3, FT_WIRELESS },
	{ "bsd_steering_policy_5g1_x",  SUBFT_BSD_STEERING_POLICY_5G1_X, FT_WIRELESS },
	{ "bsd_steering_policy_b4",  SUBFT_BSD_STEERING_POLICY_BAND4, FT_WIRELESS },
	/* sub feature for smart connect rule (STA Selection Policy) */
	{ "bsd_sta_select_policy_2g", SUBFT_BSD_STA_SELECT_POLICY_BAND1, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g", SUBFT_BSD_STA_SELECT_POLICY_BAND2, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g_x", SUBFT_BSD_STA_SELECT_POLICY_5G_X, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g1", SUBFT_BSD_STA_SELECT_POLICY_BAND3, FT_WIRELESS },
	{ "bsd_sta_select_policy_5g1_x", SUBFT_BSD_STA_SELECT_POLICY_5G1_X, FT_WIRELESS },
	{ "bsd_sta_select_policy_b4", SUBFT_BSD_STA_SELECT_POLICY_BAND4, FT_WIRELESS },
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	{ "bsd_if_select_2g", SUBFT_BSD_IF_SELECT_BAND1, FT_WIRELESS },
	{ "bsd_if_select_5g", SUBFT_BSD_IF_SELECT_BAND2, FT_WIRELESS },
	{ "bsd_if_select_5g_x", SUBFT_BSD_IF_SELECT_5G_X, FT_WIRELESS },
	{ "bsd_if_select_5g1", SUBFT_BSD_IF_SELECT_BAND3, FT_WIRELESS },
	{ "bsd_if_select_5g1_x", SUBFT_BSD_IF_SELECT_5G1_X, FT_WIRELESS },
	{ "bsd_if_select_b4", SUBFT_BSD_IF_SELECT_BAND4, FT_WIRELESS },
	{ "bsd_if_qualify_2g", SUBFT_BSD_IF_QUALIFY_BAND1, FT_WIRELESS },
	{ "bsd_if_qualify_5g", SUBFT_BSD_IF_QUALIFY_BAND2, FT_WIRELESS },
	{ "bsd_if_qualify_5g_x", SUBFT_BSD_IF_QUALIFY_5G_X, FT_WIRELESS },
	{ "bsd_if_qualify_5g1", SUBFT_BSD_IF_QUALIFY_BAND3, FT_WIRELESS },
	{ "bsd_if_qualify_5g1_x", SUBFT_BSD_IF_QUALIFY_5G1_X, FT_WIRELESS },
	{ "bsd_if_qualify_b4", SUBFT_BSD_IF_QUALIFY_BAND4, FT_WIRELESS },
	/* sub feature for smart connect rule (Bounce Detect) */
	{ "bsd_bounce_detect", SUBFT_BSD_BOUNCE_DETECT, FT_WIRELESS },
	{ "bsd_bounce_detect_x", SUBFT_BSD_BOUNCE_DETECT_X, FT_WIRELESS },
	/* sub feature for WPS enable */
	{ "wps", SUBFT_WPS, FT_WIRELESS },
	/* sub feature for Reboot schedule */
	{ "reboot_schedule", SUBFT_REBOOT_SCHEDULE, FT_TIME },
#if defined(MAPAC2200) || defined(RTAC95U)
	{ "ncb",		SUBFT_NCB,	FT_BHBLOCK },
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd",		SUBFT_SPCMD,	FT_SPCMD },
#endif
#if defined(RTCONFIG_AMAS_WGN)
	{ "vlan_rulelist", SUBFT_VLAN_RULELIST, FT_WIRELESS },
#endif	/* RTCONFIG_AMAS_WGN */		
	{ "guest_misc_2g_g1", SUBFT_GUEST_MISC_BAND1_G1, FT_WIRELESS },
	{ "guest_misc_2g_g2", SUBFT_GUEST_MISC_BAND1_G2, FT_WIRELESS },
	{ "guest_misc_2g_g3", SUBFT_GUEST_MISC_BAND1_G3, FT_WIRELESS },
	{ "guest_misc_5g_g1", SUBFT_GUEST_MISC_BAND2_G1, FT_WIRELESS },
	{ "guest_misc_5g_g2", SUBFT_GUEST_MISC_BAND2_G2, FT_WIRELESS },
	{ "guest_misc_5g_g3", SUBFT_GUEST_MISC_BAND2_G3, FT_WIRELESS },
	{ "guest_misc_5g1_g1", SUBFT_GUEST_MISC_BAND3_G1, FT_WIRELESS },
	{ "guest_misc_5g2_g2", SUBFT_GUEST_MISC_BAND3_G2, FT_WIRELESS },
	{ "guest_misc_5g3_g3", SUBFT_GUEST_MISC_BAND3_G3, FT_WIRELESS },
	{ "guest_misc_b4_g1", SUBFT_GUEST_MISC_BAND4_G1, FT_WIRELESS },
	{ "guest_misc_b4_g2", SUBFT_GUEST_MISC_BAND4_G2, FT_WIRELESS },
	{ "guest_misc_b4_g3", SUBFT_GUEST_MISC_BAND4_G3, FT_WIRELESS },
	{ "lp55xx_led",		SUBFT_LP55XX_LED,	FT_LP55XX_LED },
#if defined(RTCONFIG_BW160M)
	/* sub feature for bandwidth 160 support */
	{ "bw_160_2g", SUBFT_BW_160_BAND1, FT_WIRELESS },
	{ "bw_160_5g", SUBFT_BW_160_BAND2, FT_WIRELESS },
	{ "bw_160_5g1", SUBFT_BW_160_BAND3, FT_WIRELESS },
	{ "bw_160_b4", SUBFT_BW_160_BAND4, FT_WIRELESS },
#endif
#if defined(RTCONFIG_HND_ROUTER_AX)
	/* HE feaure */
	{ "he_features_2g", SUBFT_HE_FEATURES_BAND1, FT_WIRELESS },
	{ "he_features_5g", SUBFT_HE_FEATURES_BAND2, FT_WIRELESS },
	{ "he_features_5g1", SUBFT_HE_FEATURES_BAND3, FT_WIRELESS },
	{ "he_features_b4", SUBFT_HE_FEATURES_BAND4, FT_WIRELESS },
	/* sub feature for ACS include DFS */
	{ "acs_dfs", SUBFT_ACS_INCLUDE_DFS, FT_WIRELESS },
#endif
	/* 802.11ax/Wi-Fi 6 mode */
	{ "11ax_2g", SUBFT_11_AX_BAND1, FT_WIRELESS },
	{ "11ax_5g", SUBFT_11_AX_BAND2, FT_WIRELESS },
	{ "11ax_5g1", SUBFT_11_AX_BAND3, FT_WIRELESS },
	{ "11ax_b4", SUBFT_11_AX_BAND4, FT_WIRELESS },
	/* ofdma */
	{ "ofdma_2g", SUBFT_OFDMA_BAND1, FT_WIRELESS },
	{ "ofdma_5g", SUBFT_OFDMA_BAND2, FT_WIRELESS },
	{ "ofdma_5g1", SUBFT_OFDMA_BAND3, FT_WIRELESS },
	{ "ofdma_b4", SUBFT_OFDMA_BAND4, FT_WIRELESS },

#if defined(RTCONFIG_WIFI7) 
#if defined(RTCONFIG_BCMWL6) 
	/* be_ofdma */
	{ "be_ofdma_b1", SUBFT_BE_OFDMA_BAND1, FT_WIRELESS },
	{ "be_ofdma_b2", SUBFT_BE_OFDMA_BAND2, FT_WIRELESS },
	{ "be_ofdma_b3", SUBFT_BE_OFDMA_BAND3, FT_WIRELESS },
	{ "be_ofdma_b4", SUBFT_BE_OFDMA_BAND4, FT_WIRELESS },
	
	/* be_mumimo */
	/* be_ofdma */
	{ "be_mumimo_b1", SUBFT_BE_MUMIMO_BAND1, FT_WIRELESS },
	{ "be_mumimo_b2", SUBFT_BE_MUMIMO_BAND2, FT_WIRELESS },
	{ "be_mumimo_b3", SUBFT_BE_MUMIMO_BAND3, FT_WIRELESS },
	{ "be_mumimo_b4", SUBFT_BE_MUMIMO_BAND4, FT_WIRELESS },
#endif
#endif

#if defined(RTCONFIG_BCN_RPT)
	{ "rssi_method", SUBFT_RSSI_METHOD, FT_WIRELESS },
#endif
#ifdef RTCONFIG_BHCOST_OPT
	{ "force_topology", SUBFT_FORCE_TOPOLOGY, FT_PREFERAP},
#endif
	/* link aggregation */
	{ "link_aggregation", SUBFT_LINK_AGGREGATION, FT_LINK_AGGREGATION},

	/* connection diagnostic */
	{ "connection_diagnostic", SUBFT_CONNECTION_DIAGMOSTIC, FT_MISC},

	/* contrl led */
	{ "ctrl_led", SUBFT_CTRL_LED, FT_CTRL_LED},

	/* contrl aura rgb */
	{ "aurargb", SUBFT_AURARGB, FT_AURARGB},

#if defined(RTCONFIG_STA_AP_BAND_BIND)
	/* sta binding ap */
	{ "sta_bind_ap", SUBFT_STA_BIND_AP, FT_STA_BIND_AP},
#endif

#if defined(RTCONFIG_AMAS_WGN)
	/* bandwidth limiter for guest network */
    { "bw_limiter", SUBFT_BW_LIMIT, FT_BW_LIMIT },
    { "bw_limiter_2g_g1", SUBFT_BW_LIMIT_BAND1_G1, FT_BW_LIMIT },
    { "bw_limiter_2g_g2", SUBFT_BW_LIMIT_BAND1_G2, FT_BW_LIMIT },
    { "bw_limiter_2g_g3", SUBFT_BW_LIMIT_BAND1_G3, FT_BW_LIMIT },
    { "bw_limiter_5g_g1", SUBFT_BW_LIMIT_BAND2_G1, FT_BW_LIMIT },
    { "bw_limiter_5g_g2", SUBFT_BW_LIMIT_BAND2_G2, FT_BW_LIMIT },
    { "bw_limiter_5g_g3", SUBFT_BW_LIMIT_BAND2_G3, FT_BW_LIMIT },
    { "bw_limiter_5g1_g1", SUBFT_BW_LIMIT_BAND3_G1, FT_BW_LIMIT },
    { "bw_limiter_5g1_g2", SUBFT_BW_LIMIT_BAND3_G2, FT_BW_LIMIT },
    { "bw_limiter_5g1_g3", SUBFT_BW_LIMIT_BAND3_G3, FT_BW_LIMIT },
	{ "bw_limiter_b4_g1", SUBFT_BW_LIMIT_BAND4_G1, FT_BW_LIMIT },
	{ "bw_limiter_b4_g2", SUBFT_BW_LIMIT_BAND4_G2, FT_BW_LIMIT },
	{ "bw_limiter_b4_g3", SUBFT_BW_LIMIT_BAND4_G3, FT_BW_LIMIT },
#endif

#if defined(RTCONFIG_FANCTRL)
	{ "fanctrl",		SUBFT_FANCTRL,	FT_FANCTRL },
#endif
#ifdef RTCONFIG_QCA_PLC2
	{ "plc_master",		SUBFT_PLC_MASTER,	FT_PLC_MASTER },
#endif

	/* Target Wake Time */
	{ "twt_b1", SUBFT_TWT_BAND1, FT_WIRELESS },
	{ "twt_b2", SUBFT_TWT_BAND2, FT_WIRELESS },
	{ "twt_b3", SUBFT_TWT_BAND3, FT_WIRELESS },
	{ "twt_b4", SUBFT_TWT_BAND4, FT_WIRELESS },

#ifdef RTCONFIG_AMAS_SYNC_LEDG
	/* ledg */
	{ "ledg", SUBFT_LEDG, FT_LEDG },
#endif

	/* moca */
	{ "moca", SUBFT_MOCA, FT_MOCA },

#ifdef RTCONFIG_ROUTERBOOST
	/* ledg */
	{ "re_rb_enable", SUBFT_RB_ENABLE, FT_WIRELESS },
#endif
	{ "privacy_policy",	SUBFT_PRIVACY_POLICY,	FT_PRIVACY_POLICY },

	/* new eula */
	{ "new_eula",	SUBFT_NEW_EULA,	FT_NEW_EULA },

	/* Diable 11b */
	{ "dis_11b_b1", SUBFT_DISABLE_11B_BAND1, FT_WIRELESS },
	{ "dis_11b_b2", SUBFT_DISABLE_11B_BAND2, FT_WIRELESS },
	{ "dis_11b_b3", SUBFT_DISABLE_11B_BAND3, FT_WIRELESS },
	{ "dis_11b_b4", SUBFT_DISABLE_11B_BAND4, FT_WIRELESS },

	/* END */
	{ NULL, 0, 0}
};

struct param_mapping_s {
	char *param;
	int feature;
	int subfeature;
	char *value;
};

static struct param_mapping_s param_mapping_list[] __attribute__ ((unused)) = {
	{ "wl0_ssid", 		FT_WIRELESS, 		SUBFT_BASIC_BAND1,		"ASUS"},
	{ "wl0_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1,		"0"},
	{ "wl0_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND1,		""},
	{ "wl0_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND1,		"open"},
	{ "wl0_crypto",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		"tkip+aes"},
	{ "wl0_wep_x",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		"0"},
	{ "wl0_key",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		"1"},
	{ "wl0_key1",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		""},
	{ "wl0_key2",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		""},
	{ "wl0_key3",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		""},
	{ "wl0_key4",		FT_WIRELESS,		SUBFT_BASIC_BAND1,		""},
	{ "wl0_bw",		FT_WIRELESS,		SUBFT_CHANNEL_BAND1,		"0"},
#ifdef RTCONFIG_BCMWL6
	{ "wl0_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_BAND1,		"0"},
#else
	{ "wl0_channel",	FT_WIRELESS,		SUBFT_CHANNEL_BAND1,		"0"},
	{ "wl0_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_BAND1,		"lower"},
#endif
	{ "wl0_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND1,		"disabled"},
	{ "wl0_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND1,		""},
	{ "wl0_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_BAND1,		"0"},
#ifdef RTCONFIG_WL_SCHED_V2
	{ "wl0_sched_v2",		FT_MISC,		SUBFT_TIMESCHEDV2_BAND1,		""},
#else
	{ "wl0_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_BAND1,		"000000"},
#endif
	{ "wl0_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_BAND1,		""},
	{ "wl0_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_BAND1,		""},
	{ "wl0_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_BAND1,		"1812"},
	{ "wl0_radio",	FT_WIRELESS,		SUBFT_RADIO_BAND1,		"1"},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP) \
 || defined(RTCONFIG_MT798X)
	{ "wl0_mbo_enable",	FT_WIRELESS,		SUBFT_BASIC_BAND1,		"1"},
#endif
	{ "wl1_ssid", 		FT_WIRELESS,		SUBFT_BASIC_BAND2,		"ASUS"},
	{ "wl1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND2,		"0"},
	{ "wl1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND2,		""},
	{ "wl1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND2,		"open"},
	{ "wl1_crypto",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		"tkip+aes"},
	{ "wl1_wep_x",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		"0"},
	{ "wl1_key",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		"1"},
	{ "wl1_key1",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		""},
	{ "wl1_key2",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		""},
	{ "wl1_key3",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		""},
	{ "wl1_key4",		FT_WIRELESS,		SUBFT_BASIC_BAND2,		""},
	{ "wl1_bw",		FT_WIRELESS,		SUBFT_CHANNEL_BAND2,		"0"},
#ifdef RTCONFIG_BCMWL6
	{ "wl1_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_BAND2,		"0"},
#else
	{ "wl1_channel",	FT_WIRELESS,		SUBFT_CHANNEL_BAND2,		"0"},
	{ "wl1_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_BAND2,		"lower"},
#endif
	{ "wl1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND2,		"disabled"},
	{ "wl1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND2,		""},
	{ "wl1_timesched", 	FT_WIRELESS,		SUBFT_TIMESCHED_BAND2,		"0"},
#ifdef RTCONFIG_WL_SCHED_V2
	{ "wl1_sched_v2",		FT_MISC,		SUBFT_TIMESCHEDV2_BAND2,		""},
#else
	{ "wl1_sched",		FT_WIRELESS,		SUBFT_TIMESCHED_BAND2,		"000000"},
#endif
	{ "wl1_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_BAND2,		""},
	{ "wl1_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_BAND2,		""},
	{ "wl1_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_BAND2,		"1812"},
	{ "wl1_radio",		FT_WIRELESS,		SUBFT_RADIO_BAND2,		"1"},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP) \
 || defined(RTCONFIG_MT798X)
	{ "wl1_mbo_enable",	FT_WIRELESS,		SUBFT_BASIC_BAND2,		"1"},
#endif
	{ "wl2_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		"ASUS"},
	{ "wl2_closed",	 	FT_WIRELESS, 		SUBFT_BASIC_BAND3,		"0"},
	{ "wl2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND3,		""},
	{ "wl2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND3,		"open"},
	{ "wl2_crypto",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		"tkip+aes"},
	{ "wl2_wep_x",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		"0"},
	{ "wl2_key",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		"1"},
	{ "wl2_key1",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		""},
	{ "wl2_key2",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		""},
	{ "wl2_key3",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		""},
	{ "wl2_key4",		FT_WIRELESS,		SUBFT_BASIC_BAND3,		""},
	{ "wl2_bw",		FT_WIRELESS,		SUBFT_CHANNEL_BAND3,		"0"},
#ifdef RTCONFIG_BCMWL6
	{ "wl2_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_BAND3,		"0"},
#else
	{ "wl2_channel",	FT_WIRELESS,		SUBFT_CHANNEL_BAND3,		"0"},
	{ "wl2_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_BAND3,		"lower"},
#endif
	{ "wl2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3,		"disabled"},
	{ "wl2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3,		""},
	{ "wl2_timesched",	FT_WIRELESS,		SUBFT_TIMESCHED_BAND3,		"0"},
#ifdef RTCONFIG_WL_SCHED_V2
	{ "wl2_sched_v2",	FT_MISC,		SUBFT_TIMESCHEDV2_BAND3,		""},
#else
	{ "wl2_sched",	FT_WIRELESS,		SUBFT_TIMESCHED_BAND3,		"000000"},
#endif
	{ "wl2_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_BAND3,		""},
	{ "wl2_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_BAND3,		""},
	{ "wl2_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_BAND3,		"1812"},
	{ "wl2_radio",	FT_WIRELESS,		SUBFT_RADIO_BAND3,		"1"},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP) \
 || defined(RTCONFIG_MT798X)
	{ "wl2_mbo_enable",	FT_WIRELESS,		SUBFT_BASIC_BAND3,		"1"},
#endif

	{ "wl3_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		"ASUS"},
	{ "wl3_closed",	 	FT_WIRELESS, 		SUBFT_BASIC_BAND4,		"0"},
	{ "wl3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND4,		""},
	{ "wl3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND4,		"open"},
	{ "wl3_crypto",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		"tkip+aes"},
	{ "wl3_wep_x",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		"0"},
	{ "wl3_key",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		"1"},
	{ "wl3_key1",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		""},
	{ "wl3_key2",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		""},
	{ "wl3_key3",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		""},
	{ "wl3_key4",		FT_WIRELESS,		SUBFT_BASIC_BAND4,		""},
	{ "wl3_bw",		FT_WIRELESS,		SUBFT_CHANNEL_BAND4,		"0"},
#ifdef RTCONFIG_BCMWL6
	{ "wl3_chanspec",	FT_WIRELESS,		SUBFT_CHANNEL_BAND4,		"0"},
#else
	{ "wl3_channel",	FT_WIRELESS,		SUBFT_CHANNEL_BAND4,		"0"},
	{ "wl3_nctrlsb",	FT_WIRELESS,		SUBFT_CHANNEL_BAND4,		"lower"},
#endif
	{ "wl3_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4,		"disabled"},
	{ "wl3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4,		""},
	{ "wl3_timesched",	FT_WIRELESS,		SUBFT_TIMESCHED_BAND4,		"0"},
#ifdef RTCONFIG_WL_SCHED_V2
	{ "wl3_sched_v2",	FT_MISC,		SUBFT_TIMESCHEDV2_BAND4,		""},
#else
	{ "wl3_sched",	FT_WIRELESS,		SUBFT_TIMESCHED_BAND4,		"000000"},
#endif
	{ "wl3_radius_ipaddr",	FT_WIRELESS,		SUBFT_RADIUS_BAND4,		""},
	{ "wl3_radius_key",	FT_WIRELESS,		SUBFT_RADIUS_BAND4,		""},
	{ "wl3_radius_port",	FT_WIRELESS,		SUBFT_RADIUS_BAND4,		"1812"},
	{ "wl3_radio",	FT_WIRELESS,		SUBFT_RADIO_BAND4,		"1"},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP) \
 || defined(RTCONFIG_MT798X)
	{ "wl3_mbo_enable",	FT_WIRELESS,		SUBFT_BASIC_BAND4,		"1"},
#endif
	/* guest network */
	{ "wl0.1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G1,		"0"},
	{ "wl0.1_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G1,		"ASUS"},
	{ "wl0.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G1,		""},
	{ "wl0.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G1,		"0"},
	{ "wl0.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G1,		"open"},
	{ "wl0.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G1,		"aes"},
	{ "wl0.1_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND1_G1,		"0"},
	{ "wl0.1_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND1_G1,		"off"},
	{ "wl1.1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND2_G1,		"0"},
	{ "wl1.1_ssid", 	FT_WIRELESS,		SUBFT_BASIC_BAND2_G1,		"ASUS"},
	{ "wl1.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G1,		""},
	{ "wl1.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G1,		"0"},
	{ "wl1.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G1,		"open"},
	{ "wl1.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G1,		"aes"},
	{ "wl1.1_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND2_G1,		"0"},
	{ "wl1.1_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND2_G1,		"off"},
	{ "wl2.1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND3_G1,		"0"},
	{ "wl2.1_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND3_G1,		"ASUS"},
	{ "wl2.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G1,		""},
	{ "wl2.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G1,		"0"},
	{ "wl2.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G1,		"open"},
	{ "wl2.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G1,		"aes"},
	{ "wl2.1_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND3_G1,		"0"},
	{ "wl2.1_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND3_G1,		"off"},
	{ "wl3.1_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND4_G1,		"0"},
	{ "wl3.1_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND4_G1,		"ASUS"},
	{ "wl3.1_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G1,		""},
	{ "wl3.1_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G1,		"0"},
	{ "wl3.1_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G1,		"open"},
	{ "wl3.1_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G1,		"aes"},
	{ "wl3.1_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND4_G1,		"0"},
	{ "wl3.1_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND4_G1,		"off"},
	{ "wl0.1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G1,		"disabled"},
	{ "wl0.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G1,		""},
	{ "wl1.1_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G1,		"disabled"},
	{ "wl1.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G1,		""},
	{ "wl2.1_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G1,		"disabled"},
	{ "wl2.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G1,		""},
	{ "wl3.1_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G1,		"disabled"},
	{ "wl3.1_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G1,		""},
	{ "wl0.2_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G2,		"0"},
	{ "wl0.2_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G2,		"ASUS"},
	{ "wl0.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G2,		""},
	{ "wl0.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G2,		"open"},
	{ "wl0.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G2,		"aes"},
	{ "wl0.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G2,		"0"},
	{ "wl0.2_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND1_G2,		"0"},
	{ "wl0.2_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND1_G2,		"off"},
	{ "wl1.2_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND2_G2,		"0"},
	{ "wl1.2_ssid", 	FT_WIRELESS,		SUBFT_BASIC_BAND2_G2,		"ASUS"},
	{ "wl1.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G2,		""},
	{ "wl1.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G2,		"open"},
	{ "wl1.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G2,		"aes"},
	{ "wl1.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G2,		"0"},
	{ "wl1.2_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND2_G2,		"0"},
	{ "wl1.2_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND2_G2,		"off"},
    { "wl2.2_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND3_G2,		"0"},
	{ "wl2.2_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND3_G2,		"ASUS"},
	{ "wl2.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G2,		""},
	{ "wl2.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G2,		"open"},
	{ "wl2.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G2,		"aes"},
	{ "wl2.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G2,		"0"},
	{ "wl2.2_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND3_G2,		"0"},
	{ "wl2.2_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND3_G2,		"off"},
    { "wl3.2_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND4_G2,		"0"},
	{ "wl3.2_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND4_G2,		"ASUS"},
	{ "wl3.2_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G2,		""},
	{ "wl3.2_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G2,		"open"},
	{ "wl3.2_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G2,		"aes"},
	{ "wl3.2_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G2,		"0"},
	{ "wl3.2_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND4_G2,		"0"},
	{ "wl3.2_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND4_G2,		"off"},
	{ "wl0.2_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G2,		"disabled"},
	{ "wl0.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G2,		""},
	{ "wl1.2_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G2,		"disabled"},
	{ "wl1.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G2,		""},
	{ "wl2.2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G2,		"disabled"},
	{ "wl2.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G2,		""},
	{ "wl3.2_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G2,		"disabled"},
	{ "wl3.2_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G2,		""},
	{ "wl0.3_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G3,		"0"},
	{ "wl0.3_ssid", 	FT_WIRELESS, 		SUBFT_BASIC_BAND1_G3,		"ASUS"},
	{ "wl0.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G3,		""},
	{ "wl0.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G3,		"open"},
	{ "wl0.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G3,		"aes"},
	{ "wl0.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND1_G3,		"0"},
	{ "wl0.3_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND1_G3,		"0"},
	{ "wl0.3_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND1_G3,		"off"},
	{ "wl1.3_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND2_G3,		"0"},
	{ "wl1.3_ssid", 	FT_WIRELESS,		SUBFT_BASIC_BAND2_G3,		"ASUS"},
	{ "wl1.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G3,		""},
	{ "wl1.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G3,		"open"},
	{ "wl1.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G3,		"aes"},
	{ "wl1.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND2_G3,		"0"},
	{ "wl1.3_expire",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND2_G3,		"0"},
	{ "wl1.3_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND2_G3,		"off"},
	{ "wl2.3_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND3_G3,		"0"},
	{ "wl2.3_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND3_G3,		"ASUS"},
	{ "wl2.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G3,		""},
	{ "wl2.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G3,		"open"},
	{ "wl2.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G3,		"aes"},
	{ "wl2.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND3_G3,		"0"},
	{ "wl2.3_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND3_G3,		"0"},
	{ "wl2.3_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND3_G3,		"off"},
	{ "wl3.3_closed", 	FT_WIRELESS, 		SUBFT_BASIC_BAND4_G3,		"0"},
	{ "wl3.3_ssid",		FT_WIRELESS,		SUBFT_BASIC_BAND4_G3,		"ASUS"},
	{ "wl3.3_wpa_psk",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G3,		""},
	{ "wl3.3_auth_mode_x",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G3,		"open"},
	{ "wl3.3_crypto",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G3,		"aes"},
	{ "wl3.3_bss_enabled",	FT_WIRELESS,		SUBFT_BASIC_BAND4_G3,		"0"},
	{ "wl3.3_expire",	FT_WIRELESS,		SUBFT_GUEST_MISC_BAND4_G3,		"0"},
	{ "wl3.3_lanaccess",	FT_WIRELESS,	SUBFT_GUEST_MISC_BAND4_G3,		"off"},
	{ "wl0.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G3,		"disabled"},
	{ "wl0.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND1_G3,		""},
	{ "wl1.3_macmode", 	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G3,		"disabled"},
	{ "wl1.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND2_G3,		""},
	{ "wl2.3_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G3,		"disabled"},
	{ "wl2.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND3_G3,		""},
	{ "wl3.3_macmode",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G3,		"disabled"},
	{ "wl3.3_maclist_x",	FT_WIRELESS,		SUBFT_MACFILTER_BAND4_G3,		""},
#if defined(RTCONFIG_ROUTERBOOST)
  { "re_rb_enable",     FT_WIRELESS,    SUBFT_RB_ENABLE,		"0"},
#endif
#if defined(RTCONFIG_AMAS_WGN)
    /* bandwidth limiter for guest network */
    { "qos_enable",     FT_BW_LIMIT,    SUBFT_BW_LIMIT,		"0"},
    { "qos_type",       FT_BW_LIMIT,    SUBFT_BW_LIMIT,		"0"},
    { "wl0.1_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G1,		"0"},
    { "wl0.1_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G1,		""},
    { "wl0.1_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G1,		""},
    { "wl0.2_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G2,		"0"},
    { "wl0.2_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G2,		""},
    { "wl0.2_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G2,		""},
    { "wl0.3_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G3,		"0"},
    { "wl0.3_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G3,		""},
    { "wl0.3_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND1_G3,		""},
    { "wl1.1_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G1,		"0"},
    { "wl1.1_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G1,		""},
    { "wl1.1_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G1,		""},
    { "wl1.2_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G2,		"0"},
    { "wl1.2_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G2,		""},
    { "wl1.2_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G2,		""},
    { "wl1.3_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G3,		"0"},
    { "wl1.3_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G3,		""},
    { "wl1.3_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND2_G3,		""},
    { "wl2.1_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G1,		"0"},
    { "wl2.1_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G1,		""},
    { "wl2.1_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G1,		""},
    { "wl2.2_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G2,		"0"},
    { "wl2.2_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G2,		""},
    { "wl2.2_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G2,		""},
    { "wl2.3_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G3,		"0"},
    { "wl2.3_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G3,		""},
    { "wl2.3_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND3_G3,		""},
	{ "wl3.1_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G1,		"0"},
	{ "wl3.1_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G1,		""},
	{ "wl3.1_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G1,		""},
	{ "wl3.2_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G2,		"0"},
	{ "wl3.2_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G2,		""},
	{ "wl3.2_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G2,		""},
	{ "wl3.3_bw_enabled",   FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G3,		"0"},
	{ "wl3.3_bw_ul",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G3,		""},
	{ "wl3.3_bw_dl",    FT_BW_LIMIT,    SUBFT_BW_LIMIT_BAND4_G3,		""},
#endif
	/* wireless advanced */
	{ "wl0_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_BAND1,		"-70"},
	{ "wl1_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_BAND2,		"-70"},
	{ "wl2_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_BAND3,		"-70"},
	{ "wl3_user_rssi", 		FT_WIRELESS, 		SUBFT_ADVANCED_BAND4,		"-70"},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	/* Extended NSS */
	{ "wl1_ext_nss", 		FT_WIRELESS, 		SUBFT_ADVANCED_BAND2,		"1"},
	/* Agile DFS (preCACen) */
	{ "wl1_precacen",		FT_WIRELESS,		SUBFT_ADVANCED_BAND2,		"1"},
#endif
	/* http login */
	{ "http_username", 	FT_LOGIN,		SUBFT_ROUTER_LOGIN,		"admin"},
	{ "http_passwd",	FT_LOGIN,		SUBFT_ROUTER_LOGIN,		"admin"},
	/* local access config */
	{ "http_enable", 	FT_LOCAL_ACCESS,	SUBFT_LOCAL_ACCESS,		"0"},
	{ "https_lanport",	FT_LOCAL_ACCESS,	SUBFT_LOCAL_ACCESS,		"80"},
	/* time zone */
	{ "time_zone", 		FT_TIME,		SUBFT_TIMEZONE,		"GMT0"},
	{ "time_zone_dst", 	FT_TIME,		SUBFT_TIMEZONE,		"0"},
	{ "time_zone_dstoff", 	FT_TIME,		SUBFT_TIMEZONE,		"M3.2.0/2,M10.2.0/2"},
	/* ntp server */
	{ "ntp_server0", 	FT_TIME,		SUBFT_NTP_SERVER,		"pool.ntp.org"},
	{ "ntp_server1", 	FT_TIME,		SUBFT_NTP_SERVER,		"time.nist.gov"},
	/* telnet server */
	{ "telnetd_enable", 	FT_TIME,		SUBFT_TELNET_SERVER,		"0"},
	/* ssh server */
	{ "sshd_enable", 	FT_TIME,		SUBFT_SSH_SERVER,		"0"},
	{ "sshd_port", 	FT_TIME,		SUBFT_SSH_SERVER,		""},
	{ "sshd_pass", 	FT_TIME,		SUBFT_SSH_SERVER,		"1"},
	{ "sshd_authkeys", 	FT_TIME,		SUBFT_SSH_SERVER,		""},
	{ "shell_timeout", 	FT_TIME,		SUBFT_SSH_SERVER,		"1200"},
	/* log server */
	{ "log_ipaddr", 	FT_LOGGER,		SUBFT_LOG_SERVER,		""},
	/* misc */
	{ "cfg_alias", 	FT_MISC,		SUBFT_LOCATION,		""},
	{ "apps_sq", 	FT_MISC,		SUBFT_MISCELLANEOUS,		""},
	{ "preferred_lang",	FT_MISC,	SUBFT_MISCELLANEOUS,		"EN"},
	{ "rootcerts",		FT_MISC,		SUBFT_GENCERT,		"" },
	/* feedback */
	{ "fb_transid",		FT_FEEDBACK,		SUBFT_FEEDBACK,		"123456789ABCDEF0"},
	{ "fb_email_dbg", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_email", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_serviceno", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_ptype", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_pdesc", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_country", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_browserInfo", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_attach_syslog", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_attach_cfgfile", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_attach_modemlog", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_attach_wlanlog", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
#ifdef RTCONFIG_DSL
	{ "fb_ISP", 				FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_availability", 			FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_Subscribed_Info", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
	{ "fb_attach_iptables", 		FT_FEEDBACK,		SUBFT_FEEDBACK,		""},
#endif
	{ "fb_email_provider", FT_FEEDBACK, SUBFT_FEEDBACK,		""},
	/* diagnostic */
	{ "dblog_enable", 		FT_FEEDBACK,		SUBFT_FEEDBACK,		"0"},
	{ "dblog_tousb", 		FT_FEEDBACK,		SUBFT_FEEDBACK,		"0"},
	{ "dblog_service", 		FT_FEEDBACK,		SUBFT_FEEDBACK,		"0"},
	{ "dblog_duration", 	FT_FEEDBACK,		SUBFT_FEEDBACK,		"0"},
	{ "dblog_transid", 		FT_FEEDBACK,		SUBFT_FEEDBACK,		"0123456789ABCDEF"},
	/* backhaul ctrl */
	{ "amas_ethernet", 	FT_BACKHAULCTRL,	SUBFT_BACKHAULCTRL,		"3"},
#ifdef RTCONFIG_BHCOST_OPT
	{ "amas_eap_bhmode",    FT_BACKHAULCTRL,        SUBFT_BACKHAULCTRL_EAP,		"0"},
#endif
#ifdef RTCONFIG_DWB
#ifdef RTCONFIG_FRONTHAUL_DWB
	{ "fh_ap_enabled",    FT_DWBCTRL,        SUBFT_DWBCTRL_FRONTHAUL,		"0"},
#endif
#endif
	{ "smart_connect_x", 	FT_WIRELESS,	SUBFT_SMART_CONNECT,		"0"},
	/* led */
	{ "bc_ledLv",	FT_CENTRAL_LED,		SUBFT_CENTRAL_LED,		"2"},	/* for BLUECAVE */
	{ "lp55xx_lp5523_user_enable",	FT_LP55XX_LED,		SUBFT_LP55XX_LED,		"0"},	/* for Lyra */
	{ "lp55xx_lp5523_user_col",	FT_LP55XX_LED,		SUBFT_LP55XX_LED,		"0"},	/* for Lyra */
	{ "lp55xx_lp5523_user_beh",	FT_LP55XX_LED,		SUBFT_LP55XX_LED,		"0"},	/* for Lyra */
	/* Roaming Assistant */
	{ "rast_static_cli_enable", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT,		"0"},
	{ "wl0_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_BAND1,		""},
	{ "wl1_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_BAND2,		""},
	{ "wl2_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_BAND3,		""},
	{ "wl3_rast_static_client", FT_WIRELESS, SUBFT_ROAMING_ASSISTANT_BAND4,		""},
	/* Region */
	{ "location_code", FT_REGION, SUBFT_REGION,		""},
	/* sub feature for smart connect rule (Steering Trigger Condition) */
	{ "wl0_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_BAND1,		"0 5 3 -62 0 0 0x20"},
	{ "wl1_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_BAND2,		"0 5 3 -82 0 0 0x820"},
	{ "wl2_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_BAND3,		""},
	{ "wl3_bsd_steering_policy", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_BAND4,		"0 5 3 -82 0 0 0x420"},
	{ "wl1_bsd_steering_policy_x", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G_X,		"0 5 3 0 0 600 0x20"},
	{ "wl2_bsd_steering_policy_x", FT_WIRELESS, SUBFT_BSD_STEERING_POLICY_5G1_X,		"0 5 3 0 700 0 0x28"},
	/* sub feature for smart connect rule (STA Selection Policy) */
	{ "wl0_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_BAND1,		"30 -62 0 0 0 1 1 0 0 0 0x20"},
	{ "wl1_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_BAND2,		"30 -82 0 0 0 1 1 0 0 0 0x8020"},
	{ "wl2_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_BAND3,		"30 -82 0 0 0 1 1 0 0 0 0x4020"},
	{ "wl3_bsd_sta_select_policy", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_BAND4,		""},
	{ "wl1_bsd_sta_select_policy_x", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G_X,		"30 0 0 500 0 1 1 0 0 0 0x60"},
	{ "wl2_bsd_sta_select_policy_x", FT_WIRELESS, SUBFT_BSD_STA_SELECT_POLICY_5G1_X,		"30 0 900 0 0 1 1 0 0 0 0x68"},
	/* sub feature for smart connect rule (Interface Select and Qualify Procedures) */
	{ "wl0_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_BAND1,		"eth3 eth2"},
	{ "wl1_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_BAND2,		"eth1 eth3"},
	{ "wl2_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_BAND3,		"eth1 eth2"},
	{ "wl3_bsd_if_select_policy", FT_WIRELESS, SUBFT_BSD_IF_SELECT_BAND4,		"eth1 eth3"},
	{ "wl1_bsd_if_select_policy_x", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G_X,		"eth3"},
	{ "wl2_bsd_if_select_policy_x", FT_WIRELESS, SUBFT_BSD_IF_SELECT_5G1_X,		"eth2"},
	{ "wl0_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_BAND1,		"0 0x0 -100"},
	{ "wl1_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_BAND2,		"0 0x400 -100"},
	{ "wl2_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_BAND3,		"0 0x0 -100"},
	{ "wl3_bsd_if_qualify_policy", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_BAND4,		""},
	{ "wl1_bsd_if_qualify_policy_x", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G_X,		"0 0x0 -100"},
	{ "wl2_bsd_if_qualify_policy_x", FT_WIRELESS, SUBFT_BSD_IF_QUALIFY_5G1_X,		"0 0x4 -100"},
	/* sub feature for smart connect rule (Bounce Detect) */
	{ "bsd_bounce_detect", FT_WIRELESS, SUBFT_BSD_BOUNCE_DETECT,		"60 2 180"},
	{ "bsd_bounce_detect_x", FT_WIRELESS, SUBFT_BSD_BOUNCE_DETECT_X,		"60 2 180"},
	/* sub feature for WPS enable */
	{ "wps_enable", FT_WIRELESS, SUBFT_WPS,		"1"},
	/* sub feature for Reboot schedule */
	{ "reboot_schedule_enable", FT_TIME, SUBFT_REBOOT_SCHEDULE,		"0"},
	{ "reboot_schedule", FT_TIME, SUBFT_REBOOT_SCHEDULE,		"00000000000"},
	/* sub feature for Wireless Professional */
	/* isolate */
	{ "wl0.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND1_G1,		"0"},
	{ "wl0.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND1_G2,		"0"},
	{ "wl0.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND1_G3,		"0"},
	{ "wl1.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND2_G1,		"0"},
	{ "wl1.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND2_G2,		"0"},
	{ "wl1.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND2_G3,		"0"},
	{ "wl2.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND3_G1,		"0"},
	{ "wl2.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND3_G2,		"0"},
	{ "wl2.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND3_G3,		"0"},
	{ "wl3.1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND4_G1,		"0"},
	{ "wl3.2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND4_G2,		"0"},
	{ "wl3.3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND4_G3,		"0"},
	{ "wl0_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"0"},
	{ "wl1_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"0"},
	{ "wl2_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"0"},
	{ "wl3_ap_isolate", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"0"},
	/* IGMP Snooping */
	{ "wl0_igs", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"0"},
	{ "wl1_igs", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"0"},
	{ "wl2_igs", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"0"},
	{ "wl3_igs", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"0"},
	/* Multicast Rate(Mbps) */
	{ "wl0_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"0"},
	{ "wl1_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"0"},
	{ "wl2_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"0"},
	{ "wl3_mrate_x", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"0"},
	/* AMPDU RTS */
	{ "wl0_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"1"},
	{ "wl1_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"1"},
	{ "wl2_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"1"},
	{ "wl3_ampdu_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"1"},
	/* RTS Threshold */
	{ "wl0_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"2347"},
	{ "wl1_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"2347"},
	{ "wl2_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"2347"},
	{ "wl3_rts", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"2347"},
	/* DTIM Interval */
	{ "wl0_dtim", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"1"},
	{ "wl1_dtim", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"1"},
	{ "wl2_dtim", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"1"},
	{ "wl3_dtim", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"1"},
	/* Enable WMM APSD */
	{ "wl0_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"on"},
	{ "wl1_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"on"},
	{ "wl2_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"on"},
	{ "wl3_wme_apsd", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"on"},
	/* Optimize AMPDU aggregation */
	{ "wl0_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"0"},
	{ "wl1_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"0"},
	{ "wl2_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"0"},
	{ "wl3_ampdu_mpdu", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"0"},
	/* Enable Airtime Fairness */
	{ "wl0_atf", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"1"},
	{ "wl1_atf", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"1"},
	{ "wl2_atf", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"1"},
	{ "wl3_atf", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"1"},
	/* 802.11ac Beamforming */
	{ "wl0_txbf", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"1"},
	{ "wl1_txbf", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"1"},
	{ "wl2_txbf", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"1"},
	{ "wl3_txbf", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"1"},
	/* Universal Beamforming */
	{ "wl0_itxbf", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"1"},
	{ "wl1_itxbf", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"1"},
	{ "wl2_itxbf", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"1"},
	{ "wl3_itxbf", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"1"},
	/* Tx power adjustment */
	{ "wl0_txpower", FT_WIRELESS, SUBFT_ADVANCED_BAND1,		"100"},
	{ "wl1_txpower", FT_WIRELESS, SUBFT_ADVANCED_BAND2,		"100"},
	{ "wl2_txpower", FT_WIRELESS, SUBFT_ADVANCED_BAND3,		"100"},
	{ "wl3_txpower", FT_WIRELESS, SUBFT_ADVANCED_BAND4,		"100"},
#if defined(MAPAC2200) || defined(RTAC95U)
	/* normal client blocking in backhaul */
	{ "ncb_enable", 	FT_BHBLOCK,		SUBFT_NCB,		"1"},
#endif
#ifdef RTCONFIG_WIFI_SON
	{ "spcmd", 		FT_SPCMD,		SUBFT_SPCMD,		"0"},
#endif
#if defined(RTCONFIG_AMAS_WGN)
	{ "vlan_rulelist", FT_WIRELESS, SUBFT_VLAN_RULELIST,		""},
	{ "wl0.1_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND1_G1,		"0"},
	{ "wl0.2_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND1_G2,		"0"},
	{ "wl0.3_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND1_G3,		"0"},
	{ "wl1.1_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND2_G1,		"0"},
	{ "wl1.2_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND2_G2,		"0"},
	{ "wl1.3_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND2_G3,		"0"},
	{ "wl2.1_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND3_G1,		"0"},
	{ "wl2.2_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND3_G2,		"0"},
	{ "wl2.3_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND3_G3,		"0"},
	{ "wl3.1_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND4_G1,		"0"},
	{ "wl3.2_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND4_G2,		"0"},
	{ "wl3.3_sync_node", FT_WIRELESS, SUBFT_GUEST_MISC_BAND4_G3,		"0"},
#endif	/* RTCONFIG_AMAS_WGN */
#if defined(RTCONFIG_BW160M)
	/* Bandwidth 160 support */
	{ "wl0_bw_160", FT_WIRELESS, SUBFT_BW_160_BAND1,		"0"},
	{ "wl1_bw_160", FT_WIRELESS, SUBFT_BW_160_BAND2,		"0"},
	{ "wl2_bw_160", FT_WIRELESS, SUBFT_BW_160_BAND3,		"0"},
	{ "wl3_bw_160", FT_WIRELESS, SUBFT_BW_160_BAND4,		"0"},
#endif
#if defined(RTCONFIG_HND_ROUTER_AX)
	{ "wl0_he_features", FT_WIRELESS, SUBFT_HE_FEATURES_BAND1,		"3"},
	{ "wl1_he_features", FT_WIRELESS, SUBFT_HE_FEATURES_BAND2,		"3"},
	{ "wl2_he_features", FT_WIRELESS, SUBFT_HE_FEATURES_BAND3,		"3"},
	{ "wl3_he_features", FT_WIRELESS, SUBFT_HE_FEATURES_BAND4,		"3"},
	/* ACS include DFS channels */
	{ "acs_dfs", FT_WIRELESS, SUBFT_ACS_INCLUDE_DFS,		"0"},
#endif
	/* 802.11ax/Wi-Fi 6 mode */
	{ "wl0_11ax", FT_WIRELESS, SUBFT_11_AX_BAND1,		"1"},
	{ "wl1_11ax", FT_WIRELESS, SUBFT_11_AX_BAND2,		"1"},
	{ "wl2_11ax", FT_WIRELESS, SUBFT_11_AX_BAND3,		"1"},
	{ "wl3_11ax", FT_WIRELESS, SUBFT_11_AX_BAND4,		"1"},
	/* ofdma */
	{ "wl0_ofdma", FT_WIRELESS, SUBFT_OFDMA_BAND1,		"1"},
	{ "wl1_ofdma", FT_WIRELESS, SUBFT_OFDMA_BAND2,		"1"},
	{ "wl2_ofdma", FT_WIRELESS, SUBFT_OFDMA_BAND3,		"1"},
	{ "wl3_ofdma", FT_WIRELESS, SUBFT_OFDMA_BAND4,		"1"},
	
#if defined(RTCONFIG_WIFI7) 
#if defined(RTCONFIG_BCMWL6) 
	/* be_ofdma */
	{ "wl0_be_ofdma", FT_WIRELESS, SUBFT_BE_OFDMA_BAND1,		"3"},
	{ "wl1_be_ofdma", FT_WIRELESS, SUBFT_BE_OFDMA_BAND2,		"3"},
	{ "wl2_be_ofdma", FT_WIRELESS, SUBFT_BE_OFDMA_BAND3,		"3"},
	{ "wl3_be_ofdma", FT_WIRELESS, SUBFT_BE_OFDMA_BAND4,		"3"},
	
	/* be_mumimo */
	{ "wl0_be_mumimo", FT_WIRELESS, SUBFT_BE_MUMIMO_BAND1,		"3"},
	{ "wl1_be_mumimo", FT_WIRELESS, SUBFT_BE_MUMIMO_BAND2,		"3"},
	{ "wl2_be_mumimo", FT_WIRELESS, SUBFT_BE_MUMIMO_BAND3,		"3"},
	{ "wl3_be_mumimo", FT_WIRELESS, SUBFT_BE_MUMIMO_BAND4,		"3"},
#endif
#endif

#if defined(RTCONFIG_BCN_RPT)
	{ "rssi_method", FT_WIRELESS, SUBFT_RSSI_METHOD,		""},
#endif
#ifdef RTCONFIG_BHCOST_OPT
	{ "amas_wlc_target_bssid", FT_PREFERAP, SUBFT_FORCE_TOPOLOGY,		""},
	{ "amas_wlc0_target_bssid", FT_PREFERAP, SUBFT_FORCE_TOPOLOGY,		""},
	{ "amas_wlc1_target_bssid", FT_PREFERAP, SUBFT_FORCE_TOPOLOGY,		""},
	{ "amas_wlc2_target_bssid", FT_PREFERAP, SUBFT_FORCE_TOPOLOGY,		""},
	{ "amas_wlc3_target_bssid", FT_PREFERAP, SUBFT_FORCE_TOPOLOGY,		""},
#endif
	/* link aggregation */
	{ "lacp_enabled", FT_LINK_AGGREGATION, SUBFT_LINK_AGGREGATION,		"0"},

	/* connection diagnostic */
	{ "enable_diag", 	FT_MISC,	SUBFT_CONNECTION_DIAGMOSTIC,		"2"},
	{ "diag_interval", 	FT_MISC,	SUBFT_CONNECTION_DIAGMOSTIC,		"60"},

	/* control led */
	{ "led_val",	FT_CTRL_LED,		SUBFT_CTRL_LED,		"1"},

	/* control aura rgb */
	{ "aurargb_enable",	FT_AURARGB,		SUBFT_AURARGB,		"1"},
	{ "aurargb_val",	FT_AURARGB,		SUBFT_AURARGB,		"255,0,0,1,0,0"},

#if defined(RTCONFIG_STA_AP_BAND_BIND)
	/* sta bind ap */
	{ "sta_binding_list",	FT_STA_BIND_AP,		SUBFT_STA_BIND_AP,		""},
#endif

#if defined(RTCONFIG_FANCTRL)
	{ "fanctrl_dutycycle",	FT_FANCTRL,	SUBFT_FANCTRL,		"0"},
	{ "fanctrl_trip_points",	FT_FANCTRL,	SUBFT_FANCTRL,		"45>4<50>5<93>8<105>6"},
	{ "fanctrl_inact_time",	FT_FANCTRL,	SUBFT_FANCTRL,		"00:00<00:00"},
#endif
#ifdef RTCONFIG_QCA_PLC2
	{ "cfg_plc_master",	FT_PLC_MASTER,	SUBFT_PLC_MASTER,		""},
#endif

	/* Target Wake Time */
	{ "wl0_twt", FT_WIRELESS, SUBFT_TWT_BAND1,		"0"},
	{ "wl1_twt", FT_WIRELESS, SUBFT_TWT_BAND2,		"0"},
	{ "wl2_twt", FT_WIRELESS, SUBFT_TWT_BAND3,		"0"},
	{ "wl3_twt", FT_WIRELESS, SUBFT_TWT_BAND4,		"0"},

#ifdef RTCONFIG_AMAS_SYNC_LEDG
	{ "ledg_scheme", FT_LEDG, SUBFT_LEDG,		"2"},
	{ "ledg_rgb1", FT_LEDG, SUBFT_LEDG,		"128,0,0,128,20,0,128,50,0"},
	{ "ledg_rgb2", FT_LEDG, SUBFT_LEDG,		"10,0,128,0,0,128,0,0,12"},
	{ "ledg_rgb3", FT_LEDG, SUBFT_LEDG,		"128,0,0,128,0,0,128,0,0"},
	{ "ledg_rgb7", FT_LEDG, SUBFT_LEDG,		"128,0,0,128,0,0,128,0,0"},
#endif

	{ "moca_privacy_enable", FT_MOCA, SUBFT_MOCA,		"1"},
	{ "moca_password", FT_MOCA, SUBFT_MOCA,		"12345678901234567"},
	{ "moca_epassword", FT_MOCA, SUBFT_MOCA,		"12345678901234567"},
	{ "moca_sceu_mode", FT_MOCA, SUBFT_MOCA,		"7"},

	/* new eula */
	{ "ASUS_NEW_EULA", FT_NEW_EULA, SUBFT_NEW_EULA,		"0"},
	{ "ASUS_NEW_EULA_from", FT_NEW_EULA, SUBFT_NEW_EULA,		""},
	{ "ASUS_NEW_EULA_ts", FT_NEW_EULA, SUBFT_NEW_EULA,		""},
	{ "ASUS_NEW_EULA_time", FT_NEW_EULA, SUBFT_NEW_EULA,		""},

	/* disable 11b */
	{ "wl0_rateset", FT_WIRELESS, SUBFT_DISABLE_11B_BAND1,		"default"},
	{ "wl1_rateset", FT_WIRELESS, SUBFT_DISABLE_11B_BAND2,		"default"},
	{ "wl2_rateset", FT_WIRELESS, SUBFT_DISABLE_11B_BAND3,		"default"},
	{ "wl3_rateset", FT_WIRELESS, SUBFT_DISABLE_11B_BAND4,		"default"},

	/* END */
	{ NULL, 0, 0,		NULL}
};

struct wlcsuffix_mapping_s {
	char *name;
	char *converted_name;
};

static struct wlcsuffix_mapping_s wlcsuffix_mapping_list[] __attribute__ ((unused)) = {
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
	{ "ap_isolate", NULL},
	{ NULL, 		NULL }
};

struct smart_connect_nvsuffix_t {
	char *name;
	char *converted_name;
};

static struct smart_connect_nvsuffix_t smart_connect_nvsuffix_list[] = {
	{ "ssid\0", NULL },
	{ "wpa_psk\0", NULL },
	{ "crypto\0", NULL },
	{ "auth_mode_x\0", "auth_mode\0" },
	{ "wep_x\0", "wep\0" },
	{ "key\0", NULL },
	{ "key1\0", NULL },
	{ "key2\0", NULL },
	{ "key3\0", NULL },
	{ "key4\0", NULL },
	{ "closed\0", NULL },
	{ "radius_ipaddr\0", NULL },
	{ "radius_key\0", NULL },
	{ "radius_port\0", NULL },
	{ NULL }
};
#endif /* __CFG_PARAM_H__ */
/* End of cfg_param.h */
