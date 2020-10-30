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

extern json_object *cm_generateCapability(unsigned int role, capability_s *capablity);
extern int cm_checkWifiAuthCap(char *mac, int capBandNum, int reBandNum, int type, char *name, char *outAuth, int outAuthLen);
extern int cm_isCapSupported(char *reMac, int capType, int capSubtype);

/* type */
enum capabilityType {
	LED_CONTROL = 1,
	REBOOT_CTL = 2,
#ifdef RTCONFIG_BHCOST_OPT
	FORCE_TOPOLOGY_CTL = 3,
#endif
	RC_SUPPORT = 4,
	LINK_AGGREGATION = 5,
#ifdef RTCONFIG_AMAS_WGN
	GUEST_NETWORK_NO_2G = 6,
	GUEST_NETWORK_NO_5G = 7,
	GUEST_NETWORK_NO_5GH = 8,
#endif
	STA_2G_AUTH = 9,			/* supported authentication for 2g sta (upstream) */
	STA_5G_AUTH = 10,			/* supported authentication for 5g/5g low sta (upstream) */
	STA_5GH_AUTH = 11,			/* supported authentication for 5g high sta (upstream) */
	AP_2G_AUTH = 12,			/* supported authentication for 2g ap (downstream) */
	AP_5G_AUTH = 13,			/* supported authentication for 5g/5g low ap (downstream) */
	AP_5GH_AUTH = 14,			/* supported authentication for 5g high ap (downstream) */
	WANS_CAP = 15,
	RE_RECONNECT = 16,
	FORCE_ROAMING = 17,
#ifdef RTCONFIG_FRONTHAUL_DWB
	DWB_CTL = 18,
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
	CAPABILITY_MAX
};

/* subtype */

#ifdef RTCONFIG_BHCOST_OPT
/* for FORCE_TOPOLOGY_CTL & CONN_EAP_MODE */
#define GENERAL_MODE   BIT(0)
#endif

#ifdef RTCONFIG_FRONTHAUL_DWB
/* for DWB_CTL */
#define DWB_CTL_FRONTHAUL_AP	BIT(0)
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

/* Capability support on role */
#define CAP_SUPPORT		BIT(0)
#define RE_SUPPORT		BIT(1)

/* capability */
static capability_s capability_list[] __attribute__ ((unused)) = {
#ifdef RTCONFIG_BHCOST_OPT
	{ FORCE_TOPOLOGY_CTL, GENERAL_MODE, RE_SUPPORT |CAP_SUPPORT},
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
	{ DWB_CTL, DWB_CTL_FRONTHAUL_AP, RE_SUPPORT},
#endif
#ifdef RTCONFIG_STA_AP_BAND_BIND
	{ STA_BINDING_AP, MANUAL_STA_BINDING, RE_SUPPORT |CAP_SUPPORT},
#endif
	{ RESET_DEFAULT, MANUAL_RESET_DEFAULT, RE_SUPPORT |CAP_SUPPORT},

	/* END */
	{ 0, 0 }
};

#endif /* __CFG_CAPABILITY_H__ */
/* End of cfg_capability.h */
