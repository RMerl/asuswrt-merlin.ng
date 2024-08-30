#ifndef __CFG_CAPABILITY_H__
#define __CFG_CAPABILITY_H__

#include "amas_cap.h"

#if defined(BIT)
#undef BIT
#endif
#define BIT(x)	((1 << x))

typedef struct _capability_ss {
	unsigned int type;
	unsigned int subtype;
	unsigned int capSupportRole;
} capability_s;

typedef struct _led_capability_ss {
	unsigned int type;
	unsigned int subtype;
} led_capability_s;

#ifdef RTCONFIG_MLO
struct mlo_band_mapping_s {
	int *all_band;
	int *mlo_band;
};

static struct mlo_band_mapping_s mlo_band_mapping_list[] __attribute__ ((unused)) = {
	{ WIFI_BAND_2G | WIFI_BAND_5GL | WIFI_BAND_5GH | WIFI_BAND_6G,	WIFI_BAND_2G | WIFI_BAND_5GH | WIFI_BAND_6G}, //2556, mlo:2/5-2/6
	{ WIFI_BAND_2G | WIFI_BAND_5G | WIFI_BAND_6GL | WIFI_BAND_6GH,	WIFI_BAND_2G | WIFI_BAND_5G | WIFI_BAND_6GL}, //2566, mlo:2/5/6-1
	{ WIFI_BAND_2G | WIFI_BAND_5G | WIFI_BAND_6G,					WIFI_BAND_2G | WIFI_BAND_5G | WIFI_BAND_6G}, //256, mlo:2/5/6
	{ WIFI_BAND_2G | WIFI_BAND_5GL | WIFI_BAND_5GH,					WIFI_BAND_2G | WIFI_BAND_5GH}, //255, mlo:2/5-2
	{ WIFI_BAND_2G | WIFI_BAND_5G,									WIFI_BAND_2G | WIFI_BAND_5G}, //25, mlo:2/5
	{ -1, 		-1}
};
#endif

extern json_object *cm_generateCapability(unsigned int role, capability_s *capablity);
extern int cm_checkWifiAuthCap(char *mac, int capBandNum, int reBandNum, int type, char *name, char *outAuth, int outAuthLen);
extern int cm_isCapSupported(char *reMac, int capType, int capSubtype);
extern int cm_getCapabilityIntValue(char *mac, int capType);
#ifdef RTCONFIG_AMAS_CENTRAL_ADS
extern int cm_getAdsDsCapByUnit(int unit);
#endif

/* type */
enum capabilityType {
	LED_CONTROL = 1,
	REBOOT_CTL = 2,
#ifdef RTCONFIG_BHCOST_OPT
	FORCE_TOPOLOGY_CTL = 3,
#endif
	RC_SUPPORT = 4,
	LINK_AGGREGATION = 5,
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	GUEST_NETWORK_NO_2G = 6,
	GUEST_NETWORK_NO_5G = 7,
	GUEST_NETWORK_NO_5GH = 8,
#endif
	STA_BAND0_AUTH = 9,			/* supported authentication for band0 sta (upstream) */
	STA_BAND1_AUTH = 10,			/* supported authentication for band1 sta (upstream) */
	STA_BAND2_AUTH = 11,			/* supported authentication for band2 sta (upstream) */
	AP_BAND0_AUTH = 12,			/* supported authentication for band0 ap (downstream) */
	AP_BAND1_AUTH = 13,			/* supported authentication for band1 ap (downstream) */
	AP_BAND2_AUTH = 14,			/* supported authentication for band2 ap (downstream) */
	WANS_CAP = 15,
	RE_RECONNECT = 16,
	FORCE_ROAMING = 17,
#ifdef RTCONFIG_FRONTHAUL_DWB
	FRONTHAUL_AP_CTL = 18,
#endif
#ifdef RTCONFIG_STA_AP_BAND_BIND
	STA_BINDING_AP = 19,
#endif
	RESET_DEFAULT = 20,
#ifdef RTCONFIG_BHCOST_OPT
	CONN_UPLINK_PORTS = 21,
#endif
	WIFI_RADIO_CTL = 22,
#ifdef RTCONFIG_BHCOST_OPT
	CONN_EAP_MODE = 23,
#endif

#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	GUEST_NETWORK_NO_6G = 24, 	
#endif

#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
    CHANNEL_PLAN = 25,
#endif

#ifdef RTCONFIG_MULTILAN_CFG
    WIFI_BAND_CAP = 26,
    LAN_PORT_CAP = 27,
	WAN_PORT_CAP = 28,
#endif // RTCONFIG_MULTILAN_CFG

#ifdef RTCONFIG_AMAS_CENTRAL_ADS
	DIVERSITY_PORT_STATE_BAND0 = 29,
	DIVERSITY_PORT_STATE_BAND1 = 30,
	DIVERSITY_PORT_STATE_BAND2 = 31,
	DIVERSITY_PORT_STATE_BAND3 = 32,
#endif

#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	GUEST_NETWORK_NO_6GH = 34, 	
#endif

#ifdef RTCONFIG_MULTILAN_CFG
	MAX_MTLAN = 35,
	AVAILABLE_SDN_INDEX = 36,
#endif	// RTCONFIG_MULTILAN_CFG

#ifdef RTCONFIG_MLO
	MLO_RADIO = 40,
#endif
	CAPABILITY_MAX
};

/* subtype */

#ifdef RTCONFIG_BHCOST_OPT
/* for FORCE_TOPOLOGY_CTL & CONN_EAP_MODE */
#define GENERAL_MODE   BIT(0)
#define PREFER_NODE_APPLY    BIT(1)
#endif

#ifdef RTCONFIG_FRONTHAUL_DWB
/* for FRONTHAUL_AP_CTL */
#define FRONTHAUL_AP_OPTION_OFF	BIT(0)
#define FRONTHAUL_AP_OPTION_AUTO	BIT(1)
#define FRONTHAUL_AP_OPTION_ON	BIT(2)
#endif

/* for REBOOT_CTL */
#define MANUAL_REBOOT     BIT(0)

/* for RC_SUPPORT */
#define USBX			BIT(0)
#define GUEST_NETWORK		BIT(1)
#define WPA3			BIT(2)
#define VIF_ONBOARDING			BIT(3)
#define WL_SCHED_V2		BIT(4)
#define WIFI_RADIO		BIT(5)
#define WL_SCHED_V3		BIT(6)
#define REVERT_FW		BIT(7)
#define SWITCHCTRL		BIT(8)
#define PORT_STATUS		BIT(9)
#define LOCAL_ACCESS		BIT(10)
#define CABLE_DIAG		BIT(11)
#define NO_FW_MANUAL		BIT(12)
#define UPDATE			BIT(13)
#define ROLLBACK_FW		BIT(14)
#define SDN				BIT(15)
#define WPA3_ENT		BIT(16)
#ifdef RTCONFIG_AMAS_CENTRAL_OPTMZ
#define CENTRAL_OPTMZ		BIT(17)
#endif
#ifdef RTCONFIG_MLO
#define MLO_BACKHAUL		BIT(20)
#define MLO_FRONTHAUL		BIT(21)
#endif
#ifdef RTCONFIG_SMART_HOME_MASTER_UI
#define SMART_HOME_MASTER_UI	BIT(22)
#endif

#define WIFI7_SUPPORT  BIT(23)


/* for LINK_AGGREGATION */
#define LACP_ENABLE                    BIT(0)

/* for GUEST_NETWORK_NO */
#define ONE_GUEST_NETWORK	BIT(0)
#define TWO_GUEST_NETWORK	BIT(1)
#define THREE_GUEST_NETWORK	BIT(2)

/* for Wireless Authentication */
#define OPEN_SYSTEM			BIT(0)
#define SHARED_KEY				BIT(1)
#define WPA_PERSONAL			BIT(2)
#define WPA2_PERSONAL			BIT(3)
#define WPA3_PERSONAL			BIT(4)
#define WPA_WPA2_PERSONAL	BIT(5)
#define WPA2_WPA3_PERSONAL	BIT(6)
#define WPA_ENTERPRISE			BIT(7)
#define WPA2_ENTERPRISE		BIT(8)
#define WPA3_ENTERPRISE		BIT(9)
#define WPA_WPA2_ENTERPRISE	BIT(10)
#define WPA2_WPA3_ENTERPRISE	BIT(11)
#define RADIUS_WITH_8021X		BIT(12)
#define OWE						BIT(13)

/* for wan capability */
#define WANS_CAP_WAN        BIT(0)

/* for re reconnect */
#define MANUAL_RECONN		BIT(0)

/* for sta force roaming */
#define MANUAL_FORCE_ROAMING		BIT(0)

#ifdef RTCONFIG_STA_AP_BAND_BIND
/* for sta binding ap */
#define MANUAL_STA_BINDING		BIT(0)
#endif

/* for reset to default */
#define MANUAL_RESET_DEFAULT		BIT(0)

/* for WIFI_RADIO_NO */
#define WIFI_RADIO_2G		BIT(0)
#define WIFI_RADIO_5G		BIT(1)
#define WIFI_RADIO_5GH		BIT(2)
#define WIFI_RADIO_6G		BIT(3)
#define WIFI_RADIO_6GH		BIT(4)

/* for channel plan */
#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
#define CHANNEL_PLAN_CAP_OFF	BIT(0)
#define CHANNEL_PLAN_CAP_ON	BIT(1)
#define CHANNEL_PLAN_CAP_MANUAL	BIT(2)
#define CHANNEL_PLAN_CAP_CENTRAL	BIT(3)
#endif

/* Capability support on role */
#define CAP_SUPPORT		BIT(0)
#define RE_SUPPORT		BIT(1)

/* capability */
static capability_s capability_list[] __attribute__ ((unused)) = {
#ifdef RTCONFIG_BHCOST_OPT
	{ FORCE_TOPOLOGY_CTL, GENERAL_MODE | PREFER_NODE_APPLY, RE_SUPPORT |CAP_SUPPORT},
#endif
	{ REBOOT_CTL, MANUAL_REBOOT, RE_SUPPORT | CAP_SUPPORT},
#if defined(RTCONFIG_LACP)
	{ LINK_AGGREGATION, LACP_ENABLE, RE_SUPPORT},
#endif
#ifdef RTCONFIG_RE_RECONNECT
	{ RE_RECONNECT, MANUAL_RECONN, RE_SUPPORT |CAP_SUPPORT},
#endif
#ifdef RTCONFIG_FORCE_ROAMING
	{ FORCE_ROAMING, MANUAL_FORCE_ROAMING, RE_SUPPORT |CAP_SUPPORT},
#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
	{ FRONTHAUL_AP_CTL,
	FRONTHAUL_AP_OPTION_OFF
#if defined(RTCONFIG_FRONTHAUL_AP_AUTO_OPT)
	| FRONTHAUL_AP_OPTION_AUTO
#endif
	| FRONTHAUL_AP_OPTION_ON,
	RE_SUPPORT | CAP_SUPPORT},
#endif
#ifdef RTCONFIG_STA_AP_BAND_BIND
	{ STA_BINDING_AP, MANUAL_STA_BINDING, RE_SUPPORT |CAP_SUPPORT},
#endif
	{ RESET_DEFAULT, MANUAL_RESET_DEFAULT, RE_SUPPORT |CAP_SUPPORT},
#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
	{ CHANNEL_PLAN, CHANNEL_PLAN_CAP_OFF | CHANNEL_PLAN_CAP_ON | CHANNEL_PLAN_CAP_MANUAL, RE_SUPPORT |CAP_SUPPORT},
#endif

	/* END */
	{ 0, 0 }
};

#endif /* __CFG_CAPABILITY_H__ */
/* End of cfg_capability.h */
