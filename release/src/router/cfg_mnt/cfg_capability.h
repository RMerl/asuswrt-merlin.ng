#ifndef __CFG_CAPABILITY_H__
#define __CFG_CAPABILITY_H__

#if defined(BIT)
#undef BIT
#endif
#define BIT(x)	((1 << x))

typedef struct _capability_ss {
	unsigned int type;
	unsigned int subtype;
} capability_s;

extern json_object *cm_generateCapability(capability_s *capablity);
extern int cm_checkWifiAuthCap(char *mac, int capBandNum, int reBandNum, int type, char *name, char *outAuth, int outAuthLen);

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
	CAPABILITY_MAX
};

/* subtype */
/* for LED_CONTROL */
#define CENTRAL_LED		BIT(0)
#define LP55XX_LED		BIT(1)


#ifdef RTCONFIG_BHCOST_OPT
/* for FORCE_TOPOLOGY_CTL */
#define GENERAL_MODE   BIT(0)
#endif

/* for REBOOT_CTL */
#define MANUAL_REBOOT     BIT(0)

/* for RC_SUPPORT */
#define USBX			BIT(0)
#define GUEST_NETWORK		BIT(1)
#define WPA3			BIT(2)

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

#endif /* __CFG_CAPABILITY_H__ */
/* End of cfg_capability.h */
