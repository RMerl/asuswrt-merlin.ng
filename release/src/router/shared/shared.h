#ifndef __SHARED_H__
#define __SHARED_H__

#include <rtconfig.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <endian.h>
#include <dirent.h>
#if defined(RTCONFIG_QCA)
#include <net/ethernet.h>	//struct ethjdr
#include <netinet/if_ether.h>	//struct ethhdr
#include <netinet/ether.h>	//struct ether_addr
#endif	
#ifndef _LINUX_IF_H
#include <net/if.h>
#endif
#include <rtstate.h>
#include <stdarg.h>

#ifdef CONFIG_BCMWL5
#ifdef RTCONFIG_BCMWL6
#include "bcmwifi_channels.h"
#else
#include "bcmwifi.h"
#endif
#endif

#ifdef RTCONFIG_REALTEK
#include "realtek_common.h"
#include <time.h>
#endif
#if defined(RTCONFIG_USB) || defined(RPAC51)
#include <mntent.h>	// !!TB
#endif

#ifdef RTCONFIG_TRAFFIC_LIMITER
#include <tld_utils.h>
#endif

#ifdef RTCONFIG_AMAS_WGN
#include "amas_wgn_shared.h"
#endif	/* RTCONFIG_AMAS_WGN */

#ifdef HND_ROUTER
#include "ethswctl.h"
#endif

#ifdef RTCONFIG_LP5523
#include "lp5523led.h"
#endif /* RTCONFIG_LP5523 */

#ifdef RTCONFIG_GEFORCENOW
#include <nvgfn.h>
#endif

#include <sys/stat.h>
#include <ftw.h>
#include "network_utility.h"

#ifdef RTCONFIG_AHS
#include "notify_ahs.h"
#endif /* RTCONFIG_AHS */

#ifdef RTCONFIG_SCHED_V2
#include "sched_v2.h"
#endif /* RTCONFIG_SCHED_V2 */

#if defined(RTCONFIG_PTHSAFE_POPEN)
#if defined(RTCONFIG_QCA)
#define	popen	PS_popen
#define	pclose	PS_pclose
#endif
#define	PS_SOCK	"/tmp/ps_sock"
extern FILE *PS_popen(const char *, const char *);
extern int PS_pclose(FILE *);
#endif

#ifdef RTCONFIG_EXTPHY_BCM84880
// BCM84880, BCM54991
#define EXTPHY_ADDR 0x1e
#define EXTPHY_ADDR_STR "0x1e"
// RTL8226
#define EXTPHY_RTL_ADDR 0x03
#define EXTPHY_RTL_ADDR_STR "0x03"

#if defined(GTAX11000)
#define PHY_ID_54991E "3590:5099"
#elif defined(RTAX86U) || defined(RTAX5700) || defined(GTAXE11000)
#define PHY_ID_54991EL "3590:5089"
#endif
#endif

#ifdef CONFIG_BCMWL5
#if defined(RTAX56_XD4) || defined(CTAX56_XD4)
#define WL_IF_PREFIX "wl%d"
#define WL_IF_PREFIX_2 "wl"
#else
#define WL_IF_PREFIX "eth%d"
#define WL_IF_PREFIX_2 "eth"
#endif
#endif

/* Endian conversion functions. */
#define __bswap16(x) (uint16_t)	( \
				(((uint16_t)(x) & 0x00ffu) << 8) | \
				(((uint16_t)(x) & 0xff00u) >> 8))

#define __bswap32(x) (uint32_t)	( \
				(((uint32_t)(x) & 0xff000000u) >> 24) | \
				(((uint32_t)(x) & 0x00ff0000u) >>  8) | \
				(((uint32_t)(x) & 0x0000ff00u) <<  8) | \
				(((uint32_t)(x) & 0x000000ffu) << 24))

#define __bswap64(x) (uint64_t)	( \
				(((uint64_t)(x) & 0xff00000000000000ull) >> 56) | \
				(((uint64_t)(x) & 0x00ff000000000000ull) >> 40) | \
				(((uint64_t)(x) & 0x0000ff0000000000ull) >> 24) | \
				(((uint64_t)(x) & 0x000000ff00000000ull) >>  8) | \
				(((uint64_t)(x) & 0x00000000ff000000ull) <<  8) | \
				(((uint64_t)(x) & 0x0000000000ff0000ull) << 24) | \
				(((uint64_t)(x) & 0x000000000000ff00ull) << 40) | \
				(((uint64_t)(x) & 0x00000000000000ffull) << 56))

#if __BYTE_ORDER == __BIG_ENDIAN
#ifndef le16_to_cpu
#define le16_to_cpu(x) __bswap16(x)
#endif
#ifndef le32_to_cpu
#define le32_to_cpu(x) __bswap32(x)
#endif
#ifndef le64_to_cpu
#define le64_to_cpu(x) __bswap64(x)
#endif
#ifndef be16_to_cpu
#define be16_to_cpu(x) (x)
#endif
#ifndef be32_to_cpu
#define be32_to_cpu(x) (x)
#endif
#ifndef cpu_to_le16
#define cpu_to_le16(x) __bswap16(x)
#endif
#ifndef cpu_to_le32
#define cpu_to_le32(x) __bswap32(x)
#endif
#ifndef cpu_to_be32
#define cpu_to_be32(x) (x)
#endif
#else	/* __BYTE_ORDER != __BIG_ENDIAN */
#ifndef le16_to_cpu
#define le16_to_cpu(x) (x)
#endif
#ifndef le32_to_cpu
#define le32_to_cpu(x) (x)
#endif
#ifndef le64_to_cpu
#define le64_to_cpu(x) (x)
#endif
#ifndef be16_to_cpu
#define be16_to_cpu(x) __bswap16(x)
#endif
#ifndef be32_to_cpu
#define be32_to_cpu(x) __bswap32(x)
#endif
#ifndef cpu_to_le16
#define cpu_to_le16(x) (x)
#endif
#ifndef cpu_to_le32
#define cpu_to_le32(x) (x)
#endif
#ifndef cpu_to_be32
#define cpu_to_be32(x) __bswap32(x)
#endif
#endif	/* __BYTE_ORDER == __BIG_ENDIAN */

/* Used to bridge untagged ports on different switches. */
#define SW_IPTV_BRIDGE_VID		(960)		/* 3 ~ 4095 */

/* btn_XXX_gpio, led_XXX_gpio */
#define GPIO_ACTIVE_LOW 0x1000
#define GPIO_BLINK_LED	0x2000
#define GPIO_PIN_MASK	0x00FF
/* bit8:  USB port 1
 * bit9:  USB port 2
 * bit10: USB port 3
 * bit11: USB port 4
 * If bit8~bit11 is equal to zero, all USB ports.
 */
#define GPIO_EJUSB_MASK	0x0F00

#define GPIO_EJUSB_SHIFT	(8)
#define GPIO_EJUSB_PORT(p)	(1 << (GPIO_EJUSB_SHIFT + p - 1))	/* p = 1 ~ 4 */
#define GPIO_EJUSB_ALL_PORT	(0 << GPIO_EJUSB_SHIFT)

#define GPIO_DIR_IN	0
#define GPIO_DIR_OUT	1

#define PROC_IRQ		"/proc/irq"
#define SYS_CLASS_MTD		"/sys/class/mtd"
#define SYS_CLASS_NET		"/sys/class/net"
#define SYS_CLASS_THERMAL	"/sys/class/thermal"
#define FAN_CUR_STATE		"/sys/class/thermal/cooling_device0/cur_state"
#define FAN_RPM			"/sys/devices/platform/gpio-fan/hwmon/hwmon0/fan1_input"

#define LINUX_MTD_NAME		"linux"
#define LINUX2_MTD_NAME		"linux2"

#if defined(RTCONFIG_LANTIQ)
#define WLREADY			"wave_ready"
#else
#define WLREADY			"wlready"
#endif

#if defined(RTCONFIG_LANTIQ)
#define WLREADY			"wave_ready"
#else
#define WLREADY			"wlready"
#endif

#if defined(BCM6750) || defined(BCM63178)
#define WAN_IF_ETH	"eth4"
#else
#define WAN_IF_ETH	"eth0"
#endif

/**
 * skb->mark usage
 * 1.	bit 28~31:	Load-balance rule, IPTABLES_MARK_LB_*
 * 2.	bit  6~7 :	Facebook Wi-Fi, FBWIFI_MARK_*
 * 3.	bit  0~5 :	QoS (T.QoS: bit 0~2, BWLIT: bit 0~5)
 * 4.   bit 31~8 :	TrendMicro adaptive QoS needs bit 8~31.
 */
#define IPTABLES_MARK_LB_SET(x)	((((x)&0xFU)|0x8)<<28)			/* mark for load-balance, bit 28~31, bit31 is always 1. */
#define IPTABLES_MARK_LB_MASK	IPTABLES_MARK_LB_SET(0xf)
#define FBWIFI_MARK_SET(x)	(((x)&0x3U)<<6)				/* Facebook Wi-Fi: bit 6~7 */
#define FBWIFI_MARK_MASK	FBWIFI_MARK_SET(0x3)
#define FBWIFI_MARK_INV_MASK	(~(FBWIFI_MARK_SET(0x3)))
#define QOS_MASK		(0x3F)
#define QOS_ROG_MARK_HIGH	6
#define QOS_ROG_MARK_MID	5
#define QOS_ROG_MARK_LOW	4

/* QoS related define */
#define IS_TQOS()               (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 0)   // T.QoS
#define IS_AQOS()               (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 1)   // A.QoS
#define IS_BW_QOS()             (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 2)   // Bandwidth limiter
#define IS_GFN_QOS()            (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 3)   // GeForce NOW QoS (Nvidia)
#define IS_NON_AQOS()           (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") != 1)   // non A.QoS = others QoS (T.QoS / bandwidth monitor ... etc.)
#define IS_NON_FC_QOS()         (nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") != 1 && nvram_get_int("qos_type") != 2) // non FC QoS= others QoS except A.QOS / BW QOS
#define IS_ROG_QOS()            (nvram_get_int("qos_enable") == 0 && nvram_get_int("rog_enable") == 1) // QoS Disable, Gear Accelerator enable

/* Guest network mark */
#define GUEST_INIT_MARKNUM 10   /*10 ~ 30 for Guest Network. */
#define INITIAL_MARKNUM    30   /*30 ~ X  for LAN . */

#ifdef RTCONFIG_INTERNAL_GOBI
#define DEF_SECOND_WANIF	"usb"
#elif (defined(RTCONFIG_WANPORT2) && defined(BRTAC828))
#define DEF_SECOND_WANIF	"wan2"
#elif defined(DSL_AX82U)
#define DEF_SECOND_WANIF	"wan"
#else
#define DEF_SECOND_WANIF	"none"
#endif

#define ACTION_LOCK		"/var/lock/action"

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#define OBSS_RXRSSI_TH		"obss_rx_rssi_th"
#else
#define OBSS_RXRSSI_TH		"obss_rxrssi_th"
#endif

#if defined(RTCONFIG_CFG80211)
#define IWPRIV			"cfg80211tool"
#define HE_DLOFDMA		"he_dl_ofdma"
#define HE_ULOFDMA		"he_ul_ofdma"
#define HE_ULMIMO		"he_ul_mimo"
#else
#define IWPRIV			"iwpriv"
#define HE_DLOFDMA		"he_dlofdma"
#define HE_ULOFDMA		"he_ulofdma"
#define HE_ULMIMO		"he_ulmumimo"
#endif

#ifdef RTCONFIG_PUSH_EMAIL
//#define logmessage logmessage_push
#define logmessage logmessage_normal	//tmp
#else
#define logmessage logmessage_normal
#endif

#ifdef RTCONFIG_DSL
#define SYNC_LOG_FILE "/tmp/adsl/sync_status_log.txt"
#define LOG_RECORD_FILE "/tmp/adsl/log_record.txt"
#endif

#define Y2K			946684800UL		// seconds since 1970

#define ASIZE(array)	(sizeof(array) / sizeof(array[0]))

#ifdef LINUX26
#define	MTD_DEV(arg)	"/dev/mtd"#arg
#define	MTD_BLKDEV(arg)	"/dev/mtdblock"#arg
#define	DEV_GPIO(arg)	"/dev/gpio"#arg
#else
#define	MTD_DEV(arg)	"/dev/mtd/"#arg
#define	MTD_BLKDEV(arg)	"/dev/mtdblock/"#arg
#define	DEV_GPIO(arg)	"/dev/gpio/"#arg
#endif

#if defined(RTCONFIG_DEFAULT_AP_MODE)
#define DUT_DOMAIN_NAME "ap.asus.com"
#elif defined(RTCONFIG_CONCURRENTREPEATER)
#define DUT_DOMAIN_NAME "repeater.asus.com"
#else
#define DUT_DOMAIN_NAME "router.asus.com"
#endif
#define OLD_DUT_DOMAIN_NAME1 "www.asusnetwork.net"
#define OLD_DUT_DOMAIN_NAME2 "www.asusrouter.com"

#define NETWORKMAP_PAGE "index.asp"

#if defined(RTCONFIG_SAVEJFFS)
#define JFFS_CFGS_HDR	"JCFG"
#define JFFS_CFGS	"/tmp/jffs_cfgs"
#endif

#ifdef VZWAC1300
#define SSID_PREFIX	"VZW"
#else
#define SSID_PREFIX	"ASUS"
#endif

//version.c
extern const char *rt_version;
extern const char *rt_serialno;
extern const char *rt_rcno;
extern const char *rt_extendno;
extern const char *rt_buildname;
extern const char *rt_buildinfo;
extern const char *rt_swpjverno;

extern void set_basic_fw_name(void);

#ifdef DEBUG_NOISY
#define _dprintf		cprintf
#define csprintf		cprintf
#define dprintf			cprintf
#else
#define _dprintf(args...)	do { } while(0)
#define csprintf(args...)	do { } while(0)
#define dprintf(args...)	do { } while(0)
#endif

#define ASUS_STOP_COMMIT	"asus_stop_commit"
#define LED_CTRL_HIPRIO 	"LED_CTRL_HIPRIO"

/* NOTE: Do not insert new entries in the middle of this enum,
 * always add them to the end! */
enum {
	IPV4_DISABLED = 0,
	IPV4_DHCP,
	IPV4_STATIC,
	IPV4_PPPOE,
	IPV4_PPTP,
	IPV4_L2TP,
	IPV4_DSLITE,
	IPV4_MAPE
};

#ifdef RTCONFIG_IPV6
/* NOTE: Do not insert new entries in the middle of this enum,
 * always add them to the end! */
enum {
	IPV6_DISABLED = 0,
	IPV6_NATIVE_DHCP,
	IPV6_6TO4,
	IPV6_6IN4,
	IPV6_6RD,
	IPV6_MANUAL,
	IPV6_PASSTHROUGH
};

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP		0x0001  /* route usable			*/
#define RTF_GATEWAY     0x0002  /* destination is a gateway	*/
#define RTF_HOST	0x0004  /* host entry (net otherwise)	*/
#define RTF_REINSTATE   0x0008  /* reinstate route after tmout	*/
#define RTF_DYNAMIC     0x0010  /* created dyn. (by redirect)	*/
#define RTF_MODIFIED    0x0020  /* modified dyn. (by redirect)	*/
#define RTF_REJECT      0x0200  /* Reject route			*/
#endif
#ifndef RTF_DEFAULT
#define	RTF_DEFAULT	0x00010000	/* default - learned via ND	*/
#define	RTF_ADDRCONF	0x00040000	/* addrconf route - RA		*/
#define	RTF_CACHE	0x01000000	/* cache entry			*/
#endif
#define IPV6_MASK (RTF_GATEWAY|RTF_HOST|RTF_DEFAULT|RTF_ADDRCONF|RTF_CACHE)
#endif

#ifdef RTCONFIG_LANTIQ
enum {
	WAVE_FLAG_NORMAL=0,
	WAVE_FLAG_QIS,
	WAVE_FLAG_ACL,
	WAVE_FLAG_WPS,
	WAVE_FLAG_WDS,
	WAVE_FLAG_ADV,
	WAVE_FLAG_VAP,
	WAVE_FLAG_NETWORKMAP,
	WAVE_FLAG_APP_NORMAL,
	WAVE_FLAG_APP_ADV,
	WAVE_FLAG_APP_NORMAL_ADV,
	WAVE_FLAG_APP_WPS,
	WAVE_FLAG_APP_VAP,
	WAVE_FLAG_APP_NETWORKMAP
};

enum {
	WAVE_ACTION_IDLE=0,
	WAVE_ACTION_INIT=1,
	WAVE_ACTION_WEB=3,
	WAVE_ACTION_RE_AP2G_ON=4,
	WAVE_ACTION_RE_AP2G_OFF=5,
	WAVE_ACTION_RE_AP5G_ON=6,
	WAVE_ACTION_RE_AP5G_OFF=7,
	WAVE_ACTION_SET_CHANNEL_2G=8,
	WAVE_ACTION_SET_CHANNEL_5G=9,
	WAVE_ACTION_OPENACL_FOR_OBD=10,
	WAVE_ACTION_RECOVERACL_FOR_OBD=11,
	WAVE_ACTION_SETALLOWACL_2G=12,
	WAVE_ACTION_SETALLOWACL_5G=13,
	WAVE_ACTION_CLIENT2G_ON=14,
	WAVE_ACTION_CLIENT2G_OFF=15,
	WAVE_ACTION_CLIENT5G_ON=16,
	WAVE_ACTION_CLIENT5G_OFF=17,
	WAVE_ACTION_SET_WPS2G_CONFIGURED=18,
	WAVE_ACTION_SET_WPS5G_CONFIGURED=19,
#ifdef RTCONFIG_AMAS
	WAVE_ACTION_ADD_BEACON_VSIE=20,
	WAVE_ACTION_DEL_BEACON_VSIE=21,
	WAVE_ACTION_ADD_PROBE_REQ_VSIE=22,
	WAVE_ACTION_DEL_PROBE_REQ_VSIE=23,
	WAVE_ACTION_CLEAR_ALL_PROBE_REQ_VSIE=24,
#endif
	WAVE_ACTION_SET_STA_CONFIG=25,
};
#endif

#define GIF_LINKLOCAL  0x0001  /* return link-local addr */
#define GIF_PREFIXLEN  0x0002  /* return prefix length */
#define GIF_PREFIX     0x0004  /* return prefix, not addr */

#define EXTEND_AIHOME_API_LEVEL		21

#define EXTEND_HTTPD_AIHOME_VER		0

#define EXTEND_ASSIA_API_LEVEL		1

#define S_53134		53134
#define S_RTL8365MB		8365
#define S_RTL8370MB		8370

#ifdef RTCONFIG_CFGSYNC
#define CFG_PREFIX      "CFG"

#ifdef RTCONFIG_ADV_RAST
enum romaingEvent {
	EID_RM_STA_MON = 1,
	EID_RM_STA_MON_REPORT = 2,
	EID_RM_STA_CANDIDATE = 3,
	EID_RM_STA_ACL = 4,
	EID_RM_STA_FILTER = 5,
	EID_RM_STA_EX_AP_CHECK = 6,
	EID_RM_STA_FORCE_ROAMING = 7,
	EID_RM_STA_BINDING_UPDATE = 8,
	EID_RM_MAX
};
enum conndiagEvent {
	EID_CD_STA_CHK_ONE = 1,
	EID_CD_STA_CHK_ONE_RSP,
	EID_CD_MAX
};
#define RAST_IPC_MAX_CONNECTION		5
#define RAST_IPC_SOCKET_PATH		"/var/run/rast_ipc_socket"
#define BSD_IPC_SOCKET_PATH		"/var/run/bsd_ipc_socket"
#define RAST_INTERNAL_IPC_SOCKET_PATH	"/var/run/rast_internal_ipc_socket"
#define CONNDIAG_IPC_SOCKET_PATH	"/var/run/conndiag_ipc_socket"
#define RAST_PREFIX     "RAST"
#define CHKSTA_PREFIX   "CHKSTA"
/* key name of json from rast */
#define RAST_EVENT_ID   "EID"
#define RAST_STA        "STA"
#define RAST_STA_2G	"STA_2G"
#define RAST_STA_5G	"STA_5G"
#define RAST_AP         "AP"
#define RAST_RSSI       "RSSI"
#define RAST_BAND       "BAND"
#define RAST_PEERIP     "PIP"
#define RAST_STATUS     "STATUS"
#define RAST_CANDIDATE_AP "CANDIDATE"
#define RAST_STA_RSSI	"STA_RSSI"
#define RAST_CANDIDATE_AP_RSSI	"AP_RSSI"
#define RAST_CANDIDATE_AP_RSSI_CRITERIA  "AP_RSSI_CRITERIA"
#define RAST_ENABLE	"ENABLE"
#define RAST_RATE       "RATE"
#define RAST_TXRATE     "TXRATE"
#define RAST_RXRATE     "RXRATE"
#define RAST_TXNRATE    "TXNRATE"
#define RAST_RXNRATE    "RXNRATE"
#define RAST_DATA       "DATA"
#define RAST_MODE       "MODE"
#define RAST_SERVED_AP_BSSID	"SERVED_AP_BSSID"
#define RAST_RCPI       "RCPI"
#define RAST_STA_RCPI	"STA_RCPI"
#define RAST_CANDIDATE_AP_RCPI	"AP_RCPI"
#define RAST_AP_TARGET_MAC "AP_TARGET_MAC"
#ifdef RTCONFIG_CONN_EVENT_TO_EX_AP
#define RAST_STA_EX_AP_IP	"STA_EX_AP_IP"
#endif
#define RAST_BSSIDX "BSSIDX"
#define RAST_VIFIDX "VIFIDX"
#define RAST_TRIGGER_STA_AP_BAND_BIND "TRIGGER_STA_AP_BAND_BIND"
#define RAST_JVALUE_BAND_2G "2"
#define RAST_JVALUE_BAND_5G "1"
#define RAST_BLOCK_TIME	"BLOCK_TIME"
#define RAST_DEF_BLOCK_TIME	3

#endif	//END RTCONFIG_ADV_RAST

#ifdef RTCONFIG_HND_ROUTER_AX
#define RMD_IPC_SOCKET_PATH    "/etc/rmd_ipc_socket"
#endif
#endif	//END RTCONFIG_CFGSYNC

#ifdef RTCONFIG_FORCE_ROAMING
#define RAST_FORCE_ROAMING "FORCE_ROAMING"
#define RAST_FORCE_ROAMING_BLOCKTIME "FORCE_ROAMING_BLOCKTIME"
#define RAST_FORCE_ROAMING_TARGET "FORCE_ROAMING_TARGET"
#endif


#ifdef RTCONFIG_BCN_RPT
#define RAST_RSSI_INFO_GATHER_METHOD "RSSI_INFO_GATHER_METHOD"
#define RSSI_INFO_GATHER_NONE		0
#define RSSI_INFO_GATHER_BY_11K		1
#define RSSI_INFO_GATHER_BY_STAMON	2
#define RSSI_INFO_GATHER_DEFAULT	RSSI_INFO_GATHER_BY_11K
#endif

#ifdef RTCONFIG_BTM_11V
#define RAST_SUPPORT_11K "SUPPORT_11K"
#define BTM_RET_ACCEPT_TARGETMAC_NOTSELF	0
#define BTM_RET_ACCEPT_TARGETMAC_SELF		1
#define BTM_RET_REJECT	2
#define BTM_CMD_FAIL	3
#define BTM_TIMEOUT		4
#define BTM_OTHER		5
#endif

#ifdef RTCONFIG_AMAS
#define OUI_ASUS      "\xF8\x32\xE4"
#endif

#ifdef RTCONFIG_STA_AP_BAND_BIND
#define MAC_STR_LEN 17
#define RAST_ACL_STAAPBANDBIND_REMOVE 0
#define RAST_ACL_STAAPBANDBIND_NEW 1
#define RAST_ACL_STAAPBANDBIND_CHANGE 2
#define RAST_ACL_STAAPBANDBIND_NOTHING 3

#define RAST_NOT_A_STAAPBANDBIND_ACTION 0
#define RAST_STAAPBANDBIND_ACTION_BLOCK 1
#define RAST_STAAPBANDBIND_ACTION_UNBLOCK 2

struct staapbandbind_sta_list {
	char stamac[MAC_STR_LEN+1];
	int  allowband;
	int status;//0:nothing 1:changed 2:remove
	struct staapbandbind_sta_list *next;
};
#endif

enum {
	FROM_BROWSER,
	FROM_ASUSROUTER,
	FROM_DUTUtil,
	FROM_ASSIA,
	FROM_IFTTT,
	FROM_ALEXA,
	FROM_WebView,
	FROM_UNKNOWN
};

enum {
	ACT_IDLE,
	ACT_TFTP_UPGRADE_UNUSED,
	ACT_WEB_UPGRADE,
	ACT_WEBS_UPGRADE_UNUSED,
	ACT_SW_RESTORE,
	ACT_HW_RESTORE,
	ACT_ERASE_NVRAM,
	ACT_NVRAM_COMMIT,
	ACT_REBOOT,
	ACT_UNKNOWN
};

typedef struct {
	int count;
	struct {
		struct in_addr addr;
		unsigned short port;
	} dns[3];
} dns_list_t;

typedef struct {
	int count;
	struct {
		char name[IFNAMSIZ + 1];
		char ip[sizeof("xxx.xxx.xxx.xxx") + 1];
	} iface[2];
} wanface_list_t;

#define IP2UINT(a,b,c,d)	(((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | ((uint32_t)d))
struct ip_mask_s {
	uint32_t	ip;	/* host-endian IPv4 address */
	uint32_t	cidr;	/* CIDR netmask */
};

#if defined(RTCONFIG_TAGGED_BASED_VLAN)
/*  First VLAN br interface is br3  */
#define VLAN_START_IFNAMES_INDEX	5
#define VLAN_MAX_NUM			8
#elif defined(RTCONFIG_PORT_BASED_VLAN)
#define VLAN_START_IFNAMES_INDEX	1
#define VLAN_MAX_NUM			8	/* FIXME */
#endif

#define DHCP_STATICLIST_EXAMPLE		"<00:03:7f:00:00:02>192.168.100.200>192.192.168.100.001"
#define SUBNET_RULE_EXAMPLE		"<192.168.100.254>255.255.255.128>1>192.168.100.102>192.168.100.253>864000>12345678901234567890123456789012>192.168.100.100>192.168.100.099>1>"
#define SUBNET_STATICLIST_EXAMPLE	"00:03:7f:20:00:01 192.168.120.101;"
#define STATIC_MAC_IP_BINDING_PER_LAN	64
#define STATIC_MAC_IP_BINDING_PER_VLAN	8

#define EXCLUDE_NET_USB_MODEM		(1U << 0)
#define EXCLUDE_NET_LAN			(1U << 1)
#define EXCLUDE_NET_WAN			(1U << 2)
#define EXCLUDE_NET_PPTP_SERVER		(1U << 3)
#define EXCLUDE_NET_OPENVPN_SERVER	(1U << 4)
#define EXCLUDE_NET_IPSEC_SERVER	(1U << 5)
#define EXCLUDE_NET_IPSEC_CLIENT	(1U << 6)
#define EXCLUDE_NET_VLAN		(1U << 7)
#define EXCLUDE_NET_FREE_WIFI		(1U << 8)
#define EXCLUDE_NET_CAPTIVE_PORTAL	(1U << 9)

/* Combination of above EXCLUDE_NET_XXX */
#define EXCLUDE_NET_ALL_EXCEPT_LAN_VLAN	(~0U & ~(EXCLUDE_NET_LAN | EXCLUDE_NET_VLAN))
#define EXCLUDE_NET_ALL_EXCEPT_COOVACHILLI	(~0U & ~(EXCLUDE_NET_FREE_WIFI | EXCLUDE_NET_CAPTIVE_PORTAL))

#if defined(RTCONFIG_TAGGED_BASED_VLAN) || defined(RTCONFIG_PORT_BASED_VLAN)
struct vlan_rule_s {
	int	enable;
	char	br_if[IFNAMSIZ];	/* maybe empty string if this rule is not enabled. */
#if defined(RTCONFIG_TAGGED_BASED_VLAN)
/* vlan_rulelist example
 * 	<1>1>0>0000>00010001>000F>000F>default>1>0
 * 	<1>2>0>0000>00020002>0000>0000>192.168.5.88/24>1>0
 * 	<1>3>0>0000>00040004>0000>0000>192.168.6.88/24>1>0
 * 	<1>4>0>0000>00080008>0000>0000>192.168.7.88/24>1>0
 * 	<1>5>0>0000>00100010>0000>0000>192.168.8.88/24>1>0
 * 	<1>6>0>0000>00200020>0000>0000>192.168.9.88/24>1>0
 * 	<1>7>0>0000>00400040>0000>0000>192.168.11.88/24>1>0
 * 	<1>8>0>0000>00800080>0000>0000>192.168.12.88/24>1>0
 */
	char	vid[8];
	char	prio[4];
	char	wanportset[6];
	char	lanportset[10];
	char	wl2gset[6];
	char	wl5gset[6];
	char	subnet_name[32];
	char	internet[4];
	char	public_vlan[4];
#elif defined(RTCONFIG_PORT_BASED_VLAN)
	char	desc[32];
	char	portset[32];
	char	wlmap[16];
	char	subnet_name[32];
	char	intranet[16];
#endif
};

struct vlan_rules_s {
	unsigned int		nr_rules;
	struct vlan_rule_s	rules[VLAN_MAX_NUM];
};
#endif

extern char *read_whole_file(const char *target);
extern char *get_line_from_buffer(const char *buf, char *line, const int line_size);
#if defined(HND_ROUTER)
// defined (__GLIBC__) && !defined(__UCLIBC__)
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
#endif

/*
* Concatenate two strings together into a caller supplied buffer
* @param      s1    first string
* @param      s2    second string
* @param      buf  buffer large enough to hold both strings
* @param       buf_len the length of buf
* @return      buf
*/
static inline char * strlcat_r(const char *s1, const char *s2, char *buf, const size_t buf_len)
{
        strlcpy(buf, s1, buf_len);
        strlcat(buf, s2, buf_len);
        return buf;
}

static inline int legal_vlanid(int vid) { return (vid < 0 || vid >= 4096)? 0 : 1; }

extern in_addr_t inet_addr_(const char *addr);
extern int inet_equal(const char *addr1, const char *mask1, const char *addr2, const char *mask2);
extern int inet_intersect(const char *addr1, const char *mask1, const char *addr2, const char *mask2);
extern int inet_deconflict(const char *addr1, const char *mask1, const char *addr2, const char *mask2, struct in_addr *result);

extern void chld_reap(int sig);
extern int get_wan_proto(char *prefix);
extern int get_ipv4_service(void);
extern int get_ipv4_service_by_unit(int unit);
#ifdef RTCONFIG_IPV6
extern char *ipv6_nvname(const char *name);
extern char *ipv6_nvname_by_unit(const char *name, int unit);
extern int get_ipv6_service(void);
extern int get_ipv6_service_by_unit(int unit);
#define ipv6_enabled()	(get_ipv6_service() != IPV6_DISABLED)
extern const char *ipv6_router_address(struct in6_addr *in6addr);
extern const char *ipv6_gateway_address(void);
#else
#define ipv6_enabled()	(0)
#endif
extern void notice_set(const char *path, const char *format, ...);
#if defined(RTAC1200HP)
extern void led_5g_onoff(void);
#endif
extern void set_action(int a);
extern int check_action(void);
extern int wait_action_idle(int n);
extern int wl_client(int unit, int subunit);
extern const char *_getifaddr(const char *ifname, int family, int flags, char *buf, int size);
extern const char *getifaddr(const char *ifname, int family, int flags);
extern long uptime(void);
extern float uptime2(void);
extern int _vstrsep(char *buf, const char *sep, ...);
#if defined(RTCONFIG_QCA)
extern char *wl_ether_etoa(const struct ether_addr *n);
#endif
extern void shortstr_encrypt(unsigned char *src, unsigned char *dst, unsigned char *shift);
extern void shortstr_decrypt(unsigned char *src, unsigned char *dst, unsigned char shift);
extern char *enc_str(char *str, char *enc_buf);
extern char *dec_str(char *ec_str, char *dec_buf);
extern int generate_wireless_key(unsigned char *key);
extern int strArgs(int argc, char **argv, char *fmt, ...);
extern char *trimNL(char *str);
extern char *get_process_name_by_pid(const int pid);
extern int writefile(char *fname,char *content);
extern char *readfile(char *fname,int *fsize);
extern char *wl_nvprefix(char *prefix, int prefix_size, int unit, int subunit);
extern char *wl_nvname(const char *nv, int unit, int subunit);
extern char *wl_nband_name(const char *nband);
extern int wl_wave_unit(int wps_band);
extern char *wl_vifname_wave(int unit, int subunit);
#ifdef RTCONFIG_LANTIQ
extern int wl_client_unit(int unit);
extern char *get_staifname(int unit);
extern int get_wifname_band(char *name);
extern char *get_wififname(int band);
#endif
extern unsigned int get_radio_status(char *ifname);
extern int get_radio(int unit, int subunit);
extern void set_radio(int on, int unit, int subunit);

extern char *nvram_get_r(const char *name, char *buf, size_t buflen);
#define nvram_safe_get_r(name, buf, bufsize) (nvram_get_r(name, buf, bufsize) ? : "")
extern char *nvram_pf_get(const char *prefix, const char *name);
#define nvram_pf_safe_get(prefix, name) (nvram_pf_get(prefix, name) ? : "")
extern int nvram_pf_set(const char *prefix, const char *name, const char *value);
#define nvram_pf_unset(prefix, name) nvram_pf_set(prefix, name, NULL)
extern int nvram_get_int(const char *key);
extern int nvram_pf_get_int(const char *prefix, const char *key);
extern int nvram_set_int(const char *key, int value);
extern int nvram_pf_set_int(const char *prefix, const char *key, int value);
extern int nvram_pf_match(char *prefix, char *name, char *match);
extern int nvram_pf_invmatch(char *prefix, char *name, char *invmatch);
extern double nvram_get_double(const char *key);
extern int nvram_set_double(const char *key, double value);
extern int nvram_get_hex(const char *key);
extern int nvram_set_hex(const char *key, int value);
extern int nvram_valid_get_int(const char *key, int min, int max, int def);
#ifdef HND_ROUTER
extern char *nvram_split_get(const char *key, char *buffer, int maxlen, int maxinst);
extern int nvram_split_set(const char *key, char *value, int size, int maxinst);
#endif

//	extern long nvram_xget_long(const char *name, long min, long max, long def);
extern int nvram_contains_word(const char *key, const char *word);
extern int nvram_is_empty(const char *key);
extern void nvram_commit_x(void);
extern int connect_timeout(int fd, const struct sockaddr *addr, socklen_t len, int timeout);
extern int mtd_getinfo(const char *mtdname, int *part, int *size);
#if defined(RTCONFIG_UBIFS)
extern int ubi_getinfo(const char *ubiname, int *dev, int *part, int *size);
#endif
extern int foreach_wif(int include_vifs, void *param,
	int (*func)(int idx, int unit, int subunit, void *param));

//shutils.c
#define modprobe(mod, args...) ({ char *argv[] = { "modprobe", "-q", "-s", mod, ## args, NULL }; _eval(argv, NULL, 0, NULL); })
extern int modprobe_r(const char *mod);
extern void dbgprintf (const char * format, ...); //Ren
extern void cprintf(const char *format, ...);
extern int _eval(char *const argv[], const char *path, int timeout, int *ppid);
extern char *enc_str(char *str, char *enc_buf);
extern char *dec_str(char *ec_str, char *dec_buf);
extern int modprobe_r(const char *mod);
extern int load_kmods(char *kmods_list);
extern int remove_kmods(char *kmods_list);
extern int num_of_wl_if(void);

// usb.c
#ifdef RTCONFIG_USB
extern int probe_fs(char *device, char **fstype, char *label, char *uuid);
extern char *detect_fs_type(char *device);
extern int find_label_or_uuid(char *device, char *label, char *uuid);
extern struct mntent *findmntents(char *file, int swp,
	int (*func)(struct mntent *mnt, uint flags), uint flags);
extern void add_remove_usbhost(char *host, int add);

#define DEV_DISCS_ROOT	"/dev/discs"

/* Flags used in exec_for_host calls
 */
#define EFH_1ST_HOST	0x00000001	/* func is called for the 1st time for this host */
#define EFH_1ST_DISC	0x00000002	/* func is called for the 1st time for this disc */
#define EFH_HUNKNOWN	0x00000004	/* host is unknown */
#define EFH_USER	0x00000008	/* process is user-initiated - either via Web GUI or a script */
#define EFH_SHUTDN	0x00000010	/* exec_for_host is called at shutdown - system is stopping */
#define EFH_HP_ADD	0x00000020	/* exec_for_host is called from "add" hotplug event */
#define EFH_HP_REMOVE	0x00000040	/* exec_for_host is called from "remove" hotplug event */
#define EFH_PRINT	0x00000080	/* output partition list to the web response */

typedef int (*host_exec)(char *dev_name, int host_num, char *dsc_name, char *pt_name, uint flags);
extern int exec_for_host(int host, int obsolete, uint flags, host_exec func);
extern int is_no_partition(const char *discname);
#ifdef RTCONFIG_USB_CDROM
extern int is_cdrom_device(const char *discname);
#endif
#endif //RTCONFIG_USB

/* MODEL_*, SWITCH_* and model.c */
#include "model.h"

#define RTCONFIG_NVRAM_VER "1"

/* NOTE: Do not insert new entries in the middle of this enum,
 * always add them to the end! The numeric Hardware ID value is
 * stored in the configuration file, and is used to determine
 * whether or not this config file can be restored on the router.
 */
enum {
	HW_BCM4702,
	HW_BCM4712,
	HW_BCM5325E,
	HW_BCM4704_BCM5325F,
	HW_BCM5352E,
	HW_BCM5354G,
	HW_BCM4712_BCM5325E,
	HW_BCM4704_BCM5325F_EWC,
	HW_BCM4705L_BCM5325E_EWC,
	HW_BCM5350,
	HW_BCM5356,
	HW_BCM4716,
	HW_BCM4718,
	HW_BCM4717,
	HW_BCM5365,
	HW_BCM4785,
	HW_UNKNOWN
};

#define SUP_SES			(1 << 0)
#define SUP_BRAU		(1 << 1)
#define SUP_AOSS_LED		(1 << 2)
#define SUP_WHAM_LED		(1 << 3)
#define SUP_HPAMP		(1 << 4)
#define SUP_NONVE		(1 << 5)
#define SUP_80211N		(1 << 6)
#define SUP_1000ET		(1 << 7)

extern int check_hw_type(void);
//	extern int get_hardware(void) __attribute__ ((weak, alias ("check_hw_type")));
extern int supports(unsigned long attr);

// pids.c
extern int pids(char *appname);
extern pid_t* find_pid_by_name(const char *);

// process.c
extern char *psname(int pid, char *buffer, int maxlen);
extern int pidof(const char *name);
extern int killall(const char *name, int sig);
extern void killall_tk_period_wait(const char *name, int wait);
extern int process_exists(pid_t pid);
extern int module_loaded(const char *module);
extern int ppid(int pid);

// files.c
extern int check_if_dir_empty(const char *dirpath);
extern int _file_lock(const char *dir, const char *tag);
extern int _file_unlock(int lockfd);
extern int file_lock(const char *tag);
extern void file_unlock(int lockfd);

#define FW_CREATE	0
#define FW_APPEND	1
#define FW_NEWLINE	2
#define FW_SILENT	4	/* Don't print error message even write file fail. */

#define ACTION_LOCK_FILE "/var/lock/a_w_l" // action write lock

extern unsigned long f_size(const char *path);
extern int f_exists(const char *path);
extern int l_exists(const char *path);
extern int d_exists(const char *path);
extern int f_read_excl(const char *path, void *buffer, int max);
extern int f_read(const char *file, void *buffer, int max);												// returns bytes read
extern int f_write_excl(const char *path, const void *buffer, int len, unsigned flags, unsigned cmode);
extern int f_write(const char *file, const void *buffer, int len, unsigned flags, unsigned cmode);		//
extern int f_read_string(const char *file, char *buffer, int max);										// returns bytes read, not including term; max includes term
extern int f_write_string(const char *file, const char *buffer, unsigned flags, unsigned cmode);		//
extern int f_read_alloc(const char *path, char **buffer, int max);
extern int f_read_alloc_string(const char *path, char **buffer, int max);
extern int f_wait_exists(const char *name, int max);
extern int f_wait_notexists(const char *name, int max);
extern long file_copy_offset(const char *src_name, long src_offset, const char *dst_name, long dst_offset);
extern long file_copy(const char *src, const char *dst);
extern long file_append(const char *src_file, const char *dst_file);

/* Boost key mode. (RTCONFIG_TURBO_BTN, BTN_TURBO=y) */
enum boost_mode {
	BOOST_LED_SW = 0,	/* LED ON/OFF, except Aura RGB LED */
	BOOST_ACS_DFS_SW,	/* DFS in ACS ON/OFF */
	BOOST_AURA_RGB_SW,	/* Aura RGB LED ON/OFF */
	BOOST_GAME_BOOST_SW,	/* BWDPI Game Boost ON/OFF, need EULA. */
	BOOST_AURA_SHUFFLE_SW,	/* Aura shuffle */
	BOOST_GEFORCENOW,
	BOOST_MODE_MAX
};

// button & led
enum btn_id {
	BTN_RESET = 0,
	BTN_WPS,
	BTN_FAN,
	BTN_HAVE_FAN,
#ifdef RTCONFIG_SWMODE_SWITCH
	BTN_SWMODE_SW_ROUTER,
	BTN_SWMODE_SW_REPEATER,
	BTN_SWMODE_SW_AP,
#endif
#ifdef RTCONFIG_WIRELESS_SWITCH
	BTN_WIFI_SW,
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
	BTN_WIFI_TOG,
#endif
#ifdef RTCONFIG_TURBO_BTN
	BTN_TURBO,
#endif
#ifdef RTCONFIG_LED_BTN
	BTN_LED,
#endif
#ifdef RT4GAC55U
	BTN_LTE,
#endif
#ifdef RTCONFIG_EJUSB_BTN
	BTN_EJUSB1,
	BTN_EJUSB2,	/* If two USB LED and two EJECT USB button are true, map USB3 port to this button. */
#endif

	BTN_ID_MAX,	/* last item */
};

#if defined(RTCONFIG_CONCURRENTREPEATER)
enum led_color {
	LED_NONE = 0,
	LED_RED,
	LED_GREEN,
	LED_BLUE,
	LED_ORANGE,
	LED_GREEN2,
	LED_GREEN3,
	LED_GREEN4,
	ALL_LED,
	LED_SL4,
	LED_SL3,
	LED_SL2,
	LED_SL1
};
#endif

enum led_id {
	LED_POWER = 0,
#if defined(RTCONFIG_PWRRED_LED)
	LED_POWER_RED,
#endif
	LED_USB,
	LED_WPS,
	FAN,
	HAVE_FAN,
	LED_WAN,
#if defined(RTCONFIG_WANLEDX2)
	LED_WAN2,
#endif
#ifdef HND_ROUTER
	LED_WAN_NORMAL,
#endif
#if defined(RTCONFIG_R10G_LED)
	LED_R10G,		/* 10G RJ-45 LED */
#endif
#if defined(RTCONFIG_SFPP_LED)
	LED_SFPP,		/* SFP+ LED */
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
	LED_EXTPHY,
#endif
	LED_2G,
	LED_5G,
	LED_5G2,
	LED_60G,
	LED_USB3,
#ifdef RTCONFIG_LAN4WAN_LED
	LED_LAN1,
	LED_LAN2,
	LED_LAN3,
	LED_LAN4,
#else
	LED_LAN,
#endif
#if defined(RTAX86U) || defined(RTAX5700)
	LED_LAN,
#endif
#ifdef RTCONFIG_LOGO_LED
	LED_LOGO,
#endif
	LED_WAN_RED,
#if defined(RTCONFIG_WANLEDX2) && defined(RTCONFIG_WANRED_LED)
	LED_WAN2_RED,
#endif
#if defined(RTCONFIG_FAILOVER_LED)
	LED_FAILOVER,
#endif
	LED_SATA,
#ifdef RTCONFIG_TURBO_BTN
	LED_TURBO,
#endif
#ifdef RTCONFIG_QTN
	BTN_QTN_RESET,	/* QTN platform uses led_control() to control this gpio. */
#endif
#ifdef RTCONFIG_LED_ALL
	LED_ALL,
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
#if defined(RT4GAC53U)
	LED_LTE_OFF,
	LED_POWER_RED,
#else
	LED_3G,
	LED_LTE,
#endif
	LED_SIG1,
	LED_SIG2,
	LED_SIG3,
#if defined(RT4GAC53U)
	LED_SIG4,
#endif
#endif
#if (defined(PLN12) || defined(PLAC56))
	PLC_WAKE,
	LED_POWER_RED,
	LED_2G_GREEN,
	LED_2G_ORANGE,
	LED_2G_RED,
	LED_5G_GREEN,
	LED_5G_ORANGE,
	LED_5G_RED,
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	LED_BLUE,
	LED_GREEN,
	LED_RED,
#if defined(RTAC59_CD6R) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
	LED_WHITE,
#endif
#endif
#ifdef RPAC53
	LED_POWER_RED ,
	LED_2G_ORANGE,
	LED_2G_GREEN,
	LED_2G_RED,
	LED_5G_ORANGE,
	LED_5G_GREEN,
	LED_5G_RED,
#endif
#if defined(RPAC66)
	LED_ORANGE_POWER ,
	LED_2G_BLUE,
	LED_2G_GREEN,
	LED_2G_RED,
	LED_5G_BLUE,
	LED_5G_GREEN,
	LED_5G_RED,
#endif
#if defined(RPAC51)
	LED_RED_POWER ,
	LED_SINGLE,
	LED_FAR,
	LED_NEAR,
#endif
#if defined(RPAC87)
	LED_2G_GREEN1 ,
	LED_2G_GREEN2,
	LED_2G_GREEN3,
	LED_2G_GREEN4,
	LED_5G_GREEN1,
	LED_5G_GREEN2,
	LED_5G_GREEN3,
	LED_5G_GREEN4,
#endif
#ifdef RPAC55
	LED_POWER_RED,
	LED_WIFI,
	LED_SIG1,
	LED_SIG2,
#endif
#ifdef RPAC92
	LED_POWER_RED,
	LED_WIFI,
	LED_SIG1,
	LED_SIG2,
	LED_PURPLE,
#endif
#ifdef RPAX56
	LED_RED_GPIO,
	LED_GREEN_GPIO,
	LED_BLUE_GPIO,
	LED_WHITE_GPIO,
	LED_YELLOW_GPIO,
	LED_PURPLE_GPIO,
#endif
#ifdef BLUECAVE
	LED_CENTRAL_SIG1,
	LED_CENTRAL_SIG2,
	LED_CENTRAL_SIG3,
	LED_INDICATOR_SIG1,
	LED_INDICATOR_SIG2,
#endif
#ifdef RTCONFIG_MMC_LED
	LED_MMC,
#endif
#ifdef RTCONFIG_RESET_SWITCH
	LED_RESET_SWITCH,
#endif
#if defined(RTAC5300) || defined(GTAC5300)
	RPM_FAN,	/* use to control FAN RPM (Hi/Lo) */
#endif
#if defined(RTCONFIG_USB) || defined (RTCONFIG_LED_BTN) || defined (RTCONFIG_WPS_ALLLED_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
	PWR_USB,
#endif
#if defined(RTCONFIG_USB) \
 && (defined(RTAX89U) || defined(GTAXY16000))
	PWR_USB2,
#endif
#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(RTAX82_XD6)
	BT_RESET,
	BT_DISABLE,
	LED_RGB1_RED,
	LED_RGB1_GREEN,
	LED_RGB1_BLUE,
#endif
#if defined(CTAX56_XD4)
	BT_RESET,
	BT_DISABLE,
	LED_RGB1_RED,
	LED_RGB1_GREEN,
	LED_RGB1_BLUE,
#endif
#if defined(RTAX56_XD4)
	IND_BT,
	IND_PA,
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400)
	LED_GROUP1_RED,
	LED_GROUP1_GREEN,
	LED_GROUP1_BLUE,
	LED_GROUP2_RED,
	LED_GROUP2_GREEN,
	LED_GROUP2_BLUE,
	LED_GROUP3_RED,
	LED_GROUP3_GREEN,
	LED_GROUP3_BLUE,
	LED_GROUP4_RED,
	LED_GROUP4_GREEN,
	LED_GROUP4_BLUE,
#if defined(GSAX3000) || defined(GSAX5400)
	LED_GROUP5_RED,
	LED_GROUP5_GREEN,
	LED_GROUP5_BLUE,
#endif
#endif
#if defined(DSL_AX82U)
	LED_WIFI,
#endif
	LED_ID_MAX,	/* last item */
};

enum led_fan_mode_id {
	LED_OFF = 0,
	LED_ON,
	FAN_OFF,
	FAN_ON,
	HAVE_FAN_OFF,
	HAVE_FAN_ON,

	LED_FAN_MODE_MAX,	/* last item */
};

static inline int have_usb3_led(int model)
{
	/* Return true if a model has USB LED and USB3 LED both. */
	switch (model) {
		case MODEL_RTN18U:
		case MODEL_RTAC56U:
		case MODEL_RTAC56S:
		case MODEL_RTAC68U:
#ifndef RTCONFIG_ETRON_XHCI_USB3_LED
		case MODEL_RTAC58U:
		case MODEL_RTAC82U:
		case MODEL_RTAC55U:
		case MODEL_RTAC55UHP:
#endif
		case MODEL_DSLAC68U:
		case MODEL_RTAC3200:
		case MODEL_BRTAC828:
		case MODEL_RTAD7200:
		case MODEL_RTAC88U:
		case MODEL_RTAC86U:
		case MODEL_RTAC3100:
		case MODEL_RTAC5300:
		case MODEL_GTAC5300:
		case MODEL_RTAX88U:
			return 1;
	}
	return 0;
}

#if defined(RTCONFIG_M2_SSD)
static inline int have_sata_led(int model)
{
	/* Return true if a model has SATA LED */
	switch (model) {
		case MODEL_BRTAC828:
			return 1;
	}
	return 0;
}
#else
static inline int have_sata_led(__attribute__ ((unused)) int model) { return 0; }
#endif

#define MAX_NO_BRIDGE 	2
#define MAX_NO_MSSID	max_no_mssid()

static inline int max_no_mssid(void)
{
#if defined(BRTAC828)
	int max_no_mssid = 8;
#elif defined(RTCONFIG_PSR_GUEST)
	int max_no_mssid = 5;
#else
	int max_no_mssid = 4;
#endif

#if defined(RTCONFIG_FRONTHAUL_DWB)
	max_no_mssid++;
	if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
		max_no_mssid++;
#endif
#if defined(RTCONFIG_MSSID_PRELINK)
	max_no_mssid++;
	if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
		max_no_mssid++;
#endif

	return max_no_mssid;
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ary) (sizeof(ary) / sizeof((ary)[0]))
#endif

#if defined(RTCONFIG_WIGIG)
/* 2-nd 5G may not exist! Remember to check existence of wl2_nband in firmware
 * or 5G-2 rc_support in UI for that band.  At compile-time, RTCONFIG_HAS_5G_2
 * can be used to test.  But, non-QCA platform haven't support RTCONFIG_HAS_5G_2.
 */
#define MAX_NR_WL_IF			4
#elif defined(RTCONFIG_HAS_5G_2)
#define MAX_NR_WL_IF			3
#elif defined(RTCONFIG_HAS_5G)
#define MAX_NR_WL_IF			2
#else	/* ! RTCONFIG_HAS_5G */
#define MAX_NR_WL_IF			1	/* Single 2G */
#endif	/* ! RTCONFIG_HAS_5G */

enum wl_band_id {
	WL_2G_BAND = 0,
	WL_5G_BAND = 1,
	WL_5G_2_BAND = 2,
	WL_60G_BAND = 3,

	WL_NR_BANDS				/* Maximum number of Wireless bands of all models. */
};

#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
enum wl_bandwidth_id {
	WL_BW_20 = 0,
	WL_BW_AUTO = 1,		/* 2G: 20/40MHz, 5G: 20/40/80/(80+80)/(160) */
	WL_BW_40 = 2,
	WL_BW_80 = 3,
	WL_BW_80_80 = 4,
	WL_BW_160 = 5,
	WL_BW_2160 = 6,		/* 60G: 2.16GHz */
	WL_BW_4320 = 7,		/* 60G: 4.32GHz */
	WL_BW_6480 = 8,		/* 60G: 6.48GHz */
	WL_BW_8640 = 9,		/* 60G: 8.64GHz */

	WL_NR_BW
};
#elif defined(BCMWL5)
enum wl_bandwidth_id {
	WL_BW_AUTO = 0,		/* 2G: 20/40MHz, 5G: 20/40/80/(80+80)/(160) */
	WL_BW_20 = 1,
	WL_BW_40 = 2,
	WL_BW_80 = 3,
	WL_BW_80_80 = 4,
	WL_BW_160 = 5,

	WL_NR_BW
};
#endif

/* Used in wanports_bond */
enum bs_port_id {
	BS_WAN_PORT_ID = 0,
	BS_LAN1_PORT_ID = 1,
	BS_LAN2_PORT_ID = 2,
	BS_LAN3_PORT_ID = 3,
	BS_LAN4_PORT_ID = 4,
	BS_LAN5_PORT_ID = 5,
	BS_LAN6_PORT_ID = 6,
	BS_LAN7_PORT_ID = 7,
	BS_LAN8_PORT_ID = 8,

	BS_10GR_PORT_ID = 30,	/* 10G base-T, RJ-45 */
	BS_10GS_PORT_ID = 31,	/* 10G SFP+ */

	BS_MAX_PORT_ID
};

#define BS_WAN_PORT_MASK	(1U << BS_WAN_PORT_ID)
#define BS_LAN1_PORT_MASK	(1U << BS_LAN1_PORT_ID)
#define BS_LAN2_PORT_MASK	(1U << BS_LAN2_PORT_ID)
#define BS_LAN3_PORT_MASK	(1U << BS_LAN3_PORT_ID)
#define BS_LAN4_PORT_MASK	(1U << BS_LAN4_PORT_ID)
#define BS_LAN5_PORT_MASK	(1U << BS_LAN5_PORT_ID)
#define BS_LAN6_PORT_MASK	(1U << BS_LAN6_PORT_ID)
#define BS_LAN7_PORT_MASK	(1U << BS_LAN7_PORT_ID)
#define BS_LAN8_PORT_MASK	(1U << BS_LAN8_PORT_ID)
#define BS_10GR_PORT_MASK	(1U << BS_10GR_PORT_ID)
#define BS_10GS_PORT_MASK	(1U << BS_10GS_PORT_ID)

#if defined(RTCONFIG_BONDING_WAN)
static inline int bond_wan_enabled(void)
{
	return nvram_match("bond_wan", "1");
}
#else
static inline int bond_wan_enabled(void) { return 0; }
#endif

#if defined(RTCONFIG_LACP)
static inline int lacp_enabled(void)
{
	return nvram_match("lacp_enabled", "1");
}
#else
static inline int lacp_enabled(void) { return 0; }
#endif

/* Check Wireless band existance based on compile option only.
 * This is used when DUT is restored and restore_defaults() hasn't been executed.
 */
static inline int __absent_band(enum wl_band_id band)
{
	if (band < WL_2G_BAND || band >= WL_NR_BANDS)
		return 1;
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	if (band >= MAX_NR_WL_IF)
		return 1;
#if !defined(RTCONFIG_HAS_5G)
	if (band == WL_5G_BAND)
		return 1;
#if !defined(RTCONFIG_HAS_5G_2)
	if (band == WL_5G_2_BAND)
		return 1;
#endif	/* RTCONFIG_HAS_5G_2 */
#endif	/* RTCONFIG_HAS_5G */
#endif	/* RTCONFIG_RALINK || RTCONFIG_QCA */
#if !defined(RTCONFIG_WIGIG)
	if (band == WL_60G_BAND)
		return 1;
#endif

	return 0;
}

/* Same as __absent_band(), except it checks existance of wlX_nband nvram variable. */
static inline int absent_band(enum wl_band_id band)
{
	if (__absent_band(band) == 1)
		return 1;

	if (!nvram_get(wl_nvname("nband", band, 0)))
		return 1;

	return 0;
}

#define SKIP_ABSENT_FAKE_IFACE(iface)		if (!strncmp(iface, "FAKE", 4)) { continue; }
#define SKIP_ABSENT_BAND(u)			if (absent_band(u)) { continue; }
#define SKIP_ABSENT_BAND_AND_INC_UNIT(u)	if (absent_band(u)) { ++u; continue; }

#if defined(RTCONFIG_AMAS)
static inline int __aimesh_re_node(int sw_mode)
{
	return (sw_mode == SW_MODE_AP && nvram_get_int("re_mode") == 1);
}

static inline int aimesh_re_node(void)
{
	return __aimesh_re_node(sw_mode());
}
#else
static inline int __aimesh_re_node(int __attribute__((__unused__)) sw_mode) { return 0; }
static inline int aimesh_re_node(void) { return 0; }
#endif

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) && !defined(RTCONFIG_SOC_IPQ60XX)
static inline char *sta_default_mode(int band)
{
	if (band == WL_5G_BAND || band == WL_5G_2_BAND) {
		char prefix[sizeof("wlXXXXXX_")];

		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		if (nvram_pf_match(prefix, "country_code", "JP") || !strncmp(nvram_safe_get("territory_code"), "JP/", 3))
			return "11AHE80";
		else
			return "11AHE160";
	} else
		return "AUTO";
}
#else
static inline char *sta_default_mode(int __attribute__((__unused__)) band) { return "AUTO"; }
#endif

#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ)
static inline int __access_point_mode(int sw_mode)
{
	return (sw_mode == SW_MODE_AP
#if defined(RTCONFIG_AMAS)
		&& !nvram_get_int("re_mode")
#endif
#if defined(RTCONFIG_PROXYSTA) && defined(RTCONFIG_LANTIQ)
		&& !nvram_get_int("wlc_psta")
#endif
		);
}

static inline int access_point_mode(void)
{
	return __access_point_mode(sw_mode());
}

#if defined(RTCONFIG_WIRELESSREPEATER)
static inline int __repeater_mode(int sw_mode)
{
	return (sw_mode == SW_MODE_REPEATER
#if defined(RTCONFIG_PROXYSTA)
		&& nvram_get_int("wlc_psta") != 1
#endif
		);
}
static inline int repeater_mode(void)
{
	return __repeater_mode(sw_mode());
}
#else
static inline int __repeater_mode(int __attribute__((__unused__)) sw_mode) { return 0; }
static inline int repeater_mode(void) { return 0; }
#endif

#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_PROXYSTA)
static inline int __mediabridge_mode(int sw_mode)
{
#ifdef RTCONFIG_LANTIQ
	return (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1);
#else
	return (sw_mode == SW_MODE_REPEATER && nvram_get_int("wlc_psta") == 1);
#endif
}
static inline int mediabridge_mode(void)
{
	return __mediabridge_mode(sw_mode());
}
#else
static inline int __mediabridge_mode(int __attribute__((__unused__)) sw_mode) { return 0; }
static inline int mediabridge_mode(void) { return 0; }
#endif
#else
/* Broadcom platform and others */
static inline int __access_point_mode(int sw_mode)
{
	return (sw_mode == SW_MODE_AP
#if defined(RTCONFIG_PROXYSTA)
		&& !nvram_get_int("wlc_psta")
#endif
		);
}

static inline int access_point_mode(void)
{
	return __access_point_mode(sw_mode());
}

#if defined(RTCONFIG_WIRELESSREPEATER)
static inline int __repeater_mode(int sw_mode)
{
	return (sw_mode == SW_MODE_REPEATER);
}
static inline int repeater_mode(void)
{
	return __repeater_mode(sw_mode());
}
#else
static inline int __repeater_mode(int sw_mode) { return 0; }
static inline int repeater_mode(void) { return 0; }
#endif

#ifdef RTCONFIG_PROXYSTA
static inline int __mediabridge_mode(int sw_mode)
{
	return (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1);
}
static inline int mediabridge_mode(void)
{
	return __mediabridge_mode(sw_mode());
}
#else
static inline int __mediabridge_mode(__attribute__ ((unused)) int sw_mode) { return 0; }
static inline int mediabridge_mode(void) { return 0; }
#endif
#endif	/* RTCONFIG_RALINK || RTCONFIG_QCA */
#ifdef RTCONFIG_REALTEK
static inline int __re_mode(int sw_mode) {
	return (sw_mode == SW_MODE_AP && nvram_get_int("re_mode") == 1 );
}
static inline int re_mode(void) {
	return __re_mode(sw_mode());
}

#endif

static inline int client_mode(void)
{
	if  (repeater_mode() || mediabridge_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		|| ((sw_mode() == SW_MODE_AP) && nvram_get_int("wlc_psta"))
#endif
	)
		return 1;

	return 0;
}

#ifdef RTCONFIG_DPSTA
static inline int dpsta_mode()
{
	return ((sw_mode() == SW_MODE_AP) && (nvram_get_int("wlc_psta") == 2) && (nvram_get_int("wlc_dpsta") == 1));
}
#else
static inline int dpsta_mode()
{
	return 0;
}
#endif

static inline int dpsr_mode()
{
	return ((sw_mode() == SW_MODE_AP) && (nvram_get_int("wlc_psta") == 2) && (nvram_get_int("wlc_dpsta") == 2));
}

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
static inline int psr_mode()
{
        return (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 2 && !nvram_get_int("wlc_dpsta"));
}
#else
static inline int psr_mode()
{
	return 0;
}
#endif

static inline int get_wps_multiband(void)
{
#if defined(RTCONFIG_WPSMULTIBAND)
	return nvram_get_int("wps_multiband");
#else
	return 0;
#endif
}

static inline int get_radio_band(int band)
{
#if defined(RTCONFIG_WPSMULTIBAND)
	(void) band;
	return -1;
#else
	return band;
#endif
}

#ifdef RTCONFIG_DUALWAN
static inline int eth_wantype(int unit);

static inline int dualwan_unit__usbif(int unit)
{
	int type = get_dualwan_by_unit(unit);
	return (type == WANS_DUALWAN_IF_USB || type == WANS_DUALWAN_IF_USB2
			);
}

static inline int dualwan_unit__nonusbif(int unit)
{
	int type = get_dualwan_by_unit(unit);
	return (eth_wantype(unit) || type == WANS_DUALWAN_IF_DSL
#ifdef RTCONFIG_MULTICAST_IPTV
		|| type == WAN_UNIT_IPTV || type == WAN_UNIT_VOIP
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
		|| (WAN_UNIT_MULTISRV_BASE < unit && unit < WAN_UNIT_MULTISRV_MAX)
#endif
			);
}
extern int get_usbif_dualwan_unit(void);
extern int get_primaryif_dualwan_unit(void);
#else // RTCONFIG_DUALWAN
static inline int dualwan_unit__usbif(int unit)
{
#ifdef RTCONFIG_USB_MODEM
	return (unit == WAN_UNIT_SECOND);
#else
	return 0;
#endif
}

static inline int dualwan_unit__nonusbif(int unit)
{
	return (unit == WAN_UNIT_FIRST
#ifdef RTCONFIG_MULTICAST_IPTV
		|| unit == WAN_UNIT_IPTV || unit == WAN_UNIT_VOIP
#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
		|| (WAN_UNIT_MULTISRV_BASE < unit && unit < WAN_UNIT_MULTISRV_MAX)
#endif
		);

}
static inline int get_usbif_dualwan_unit(void)
{
	return get_wanunit_by_type(WANS_DUALWAN_IF_USB);
}

static inline int get_primaryif_dualwan_unit(void)
{
	return wan_primary_ifunit();
}

static inline int get_wans_dualwan(void) {
#ifdef RTCONFIG_USB_MODEM
	return WANSCAP_WAN | WANSCAP_USB;
#else
	return WANSCAP_WAN;
#endif
}

static inline int get_dualwan_by_unit(int unit) {
#ifdef RTCONFIG_MULTICAST_IPTV
	if(unit == WAN_UNIT_IPTV)
		return WAN_UNIT_IPTV;
	if(unit == WAN_UNIT_VOIP)
		return WAN_UNIT_VOIP;
#endif
#ifdef RTCONFIG_USB_MODEM
	return (unit == WAN_UNIT_FIRST) ? WANS_DUALWAN_IF_WAN : WANS_DUALWAN_IF_USB;
#else
	return (unit == WAN_UNIT_FIRST) ? WANS_DUALWAN_IF_WAN : WANS_DUALWAN_IF_NONE;
#endif
}

static inline int get_nr_wan_unit(void) { return 1; }
#endif // RTCONFIG_DUALWAN

static inline int eth_wantype(int unit)
{
	int type = get_dualwan_by_unit(unit);

	if (type == WANS_DUALWAN_IF_WAN ||
	    type == WANS_DUALWAN_IF_LAN ||
	    type == WANS_DUALWAN_IF_WAN2 ||
	    type == WANS_DUALWAN_IF_SFPP)
		return 1;

	return 0;
}

#ifdef CONFIG_BCMWL5
extern int get_ifname_unit(const char* ifname, int *unit, int *subunit);

static inline int guest_wlif(char *ifname)
{
	int unit = -1, subunit = -1;

	if (!ifname || strncmp(ifname, "wl", 2)) return 0;

	if (get_ifname_unit(ifname, &unit, &subunit) < 0)
		return 0;

	if (sw_mode() == SW_MODE_REPEATER && unit == nvram_get_int("wlc_band") && subunit == 1)
		return 0;

#ifdef RTCONFIG_AMAS
	return nvram_get_int("re_mode") ? (subunit > 1) : (subunit > 0);
#else
	return (subunit > 0);
#endif
}
#elif defined RTCONFIG_RALINK
static inline int guest_wlif(char *ifname)
{
	return strncmp(ifname, "ra", 2) == 0 && !strchr(ifname, '0');
}
#elif defined(RTCONFIG_QCA)
/**
 * This function doesn't check wlX_vifnames nvram variable.
 * If required, check it in caller.
 * @return:
 * 	0:	@ifname is not any guest network interface name.
 *  otherwise:	@ifname is one of gueset network interface name.
 */
static inline int guest_wlif(char *ifname)
{
	int r, r1, v;
	char *p = ifname + 4;

	r = !strncmp(ifname, "ath0", 4) || !strncmp(ifname, "ath1", 4) || !strncmp(ifname, "ath2", 4);
	if (r) {
		v = atoi(p);
		if (*p == '\0' || v <= 0 || v > 16)
			r = 0;
	}

	r1 = !strncmp(ifname, "wlan", 4);
	if (r1) {
		/* Multiple SSID haven't been supported by kernel v3.4 Wigig 802.11ad driver.
		 * Guest network interface is not possible yet.
		 */
		return 0;
	}

	return r;
}

#ifdef RTCONFIG_AIR_TIME_FAIRNESS
/** Whether air-time fairness is enabled on any wireless band.
 * @return:
 * 	0:	none of any wireless band enable air-time fairness.
 *  otherwise:	one or more wireless band enable air-time fairness.
 */
static inline int atf_enabled(void)
{
	int ret = 0, band;
	char prefix[sizeof("wlXXXX_")];

	for (band = WL_2G_BAND; !ret && band <= MAX_NR_WL_IF; ++band) {
		SKIP_ABSENT_BAND(band);
		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		if (nvram_pf_match(prefix, "atf", "1"))
			ret = 1;
	}

	return ret;
}
#else
static inline int atf_enabled(void) { return 0; }
#endif

#if defined(RTCONFIG_GLOBAL_INI)
extern int get_internal_ini_filename(char *fn, size_t fn_size);
#else
static inline int get_internal_ini_filename(__attribute__ ((unused)) char *fn, __attribute__ ((unused)) size_t fn_size) { return -1; }
#endif

static inline unsigned int bitCount(unsigned int value)
{
	unsigned int c = 0;
	while (value > 0) {
		if (value & 1)
			c++;
		value >>= 1;
	}
	return c;
}

#elif defined RTCONFIG_REALTEK
static inline int guest_wlif(char *ifname)
{
	return strncmp(ifname, "wl", 2) == 0 && !strchr(ifname, '0');
}
#elif defined RTCONFIG_LANTIQ
static inline int guest_wlif(char *ifname)
{
	char *p = ifname + 5;
	int v;

	if (strncmp(ifname, "wlan", 4) || (*p == '\0'))
		return 0;

	v = atoi(p+1);
	if (v < 0 || v > 4)
		return 0;

	return 1;
}
#elif defined RTCONFIG_ALPINE
#define GUEST_IF_NUM 6
static inline int guest_wlif(char *ifname)
{
	// char *p = ifname + 5;
	int i;

	char *guest_wlif_name[GUEST_IF_NUM] = {
		"wifi2_1", "wifi2_2", "wifi2_3",
		"wifi0_1", "wifi0_2", "wifi0_3"};

	for( i = 0 ; i < GUEST_IF_NUM; ++i){
		if(strcmp(ifname, guest_wlif_name[i]) == 0) return 1;
	}

	return 0;
}
#endif

extern int init_gpio(void);
extern int set_pwr_usb(int boolOn);
#ifdef RT4GAC68U
extern int set_pwr_modem(int boolOn);
#endif
extern int button_pressed(int which);
#if defined(RTAX86U) || defined(RTAX5700)
void config_ext_wan_led(int onoff);
#endif
extern int led_control(int which, int mode);

/* api-*.c */
extern uint32_t gpio_dir(uint32_t gpio, int dir);
extern uint32_t set_gpio(uint32_t gpio, uint32_t value);
extern uint32_t get_gpio(uint32_t gpio);
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(RTAX58U) || defined(TUFAX3000) || defined(RTAX82U) || defined(RTAX82_XD6) || defined(GSAX3000) || defined(GSAX5400)
extern uint32_t get_gpio2(uint32_t gpio);
#endif
extern int get_switch_model(void);
/* phy port related start */
#define MAX_PHY_PORT 16
#ifdef RTCONFIG_NEW_PHYMAP
#define PHY_PORT_CAP_WAN 0x01
#define PHY_PORT_CAP_LAN 0x02
#define PHY_PORT_CAP_GAME 0x04
#define PHY_PORT_CAP_PLC 0x08

static inline char *get_phy_port_cap_name(int cap, char *buf, int buf_len)
{
	int len = 0;

	if (!buf || !buf_len)
		return NULL;

	memset(buf, 0, buf_len);
	if (cap & PHY_PORT_CAP_WAN)
		len = snprintf(buf+len, buf_len-len, len ? ",%s" : "%s", "wan");
	if (cap & PHY_PORT_CAP_LAN)
		len = snprintf(buf+len, buf_len-len, len ? ",%s" : "%s", "lan");
	if (cap & PHY_PORT_CAP_GAME)
		len = snprintf(buf+len, buf_len-len, len ? ",%s" : "%s", "game");
	if (cap & PHY_PORT_CAP_PLC)
		len = snprintf(buf+len, buf_len-len, len ? ",%s" : "%s", "plc");

	return buf;
}

typedef struct _phy_port {
	int phy_port_id;     // port id for driver
	char *label_name;    // could be W0, L1, ..., L8
	uint8_t cap;         // WAN : 0x1, LAN : 0x2, GAME : 0x4, PLC : 0x8
	int max_rate;        // max support link rate (ex. 10, 100, 1000, 2500, 10000)
	char *ifname;        // the mapping interface name
} phy_port;
typedef struct _phy_port_mapping {
	int count;           // the amount of phy port
	int extsw_count;     // the amount of external switch port, like RTL8365MB
	phy_port port[MAX_PHY_PORT]; 
} phy_port_mapping;
extern phy_port_mapping get_phy_port_mapping(void); // for capability use
#endif
typedef struct _phy_info {
	int phy_port_id;     // port id for driver
	char label_name[8];  // could be W0, L1, ..., L8
	char cap_name[64];   // could be wan, lan, game or plc
	char state[8];       // could be up, down
	int link_rate;       // could be 10, 100, 1000, 2500, 10000
	char duplex[8];      // could be half, fulll
	uint32_t tx_packets;
	uint32_t rx_packets;
	uint64_t tx_bytes;
	uint64_t rx_bytes;
	uint32_t crc_errors;
} phy_info;
typedef struct _phy_info_list {
	int count;           // the amount of phy port
	phy_info phy_info[MAX_PHY_PORT];
} phy_info_list;
/* phy port related end.*/
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
extern uint32_t get_phy_status(int wan_unit);
extern uint32_t get_phy_speed(int wan_unit);
extern uint32_t set_phy_ctrl(int wan_unit, int ctrl);
#else
extern uint32_t get_phy_status(uint32_t portmask);
extern uint32_t get_phy_speed(uint32_t portmask);
extern uint32_t get_phy_duplex(uint32_t portmask);
extern uint64_t get_phy_mib(int port, char *type);
extern uint32_t set_phy_ctrl(uint32_t portmask, int ctrl);
#endif
extern char *get_wan_base_if(void);
extern char *__get_wan_base_if(char *wan_base_if) __attribute__((weak));
extern void set_jumbo_frame(void) __attribute__((weak));
extern void pre_config_switch(void);
extern void post_config_switch(void);
extern void post_start_lan(void);
extern void post_start_lan_wl(void);
extern void __pre_config_switch(void) __attribute__((weak));
extern void __post_config_switch(void) __attribute__((weak));
extern void __post_start_lan(void) __attribute__((weak));
extern void __post_start_lan_wl(void) __attribute__((weak));
extern int sw_based_iptv(void);
extern int __sw_based_iptv(void) __attribute__((weak));
extern int sw_bridge_iptv_different_switches(void);
extern int __sw_bridge_iptv_different_switches(void) __attribute__((weak));
extern int get_sw_bridge_iptv_vid(void);
extern int get_bonding_speed(char *bond_if);
extern int get_bonding_port_status(int port);
extern int __get_bonding_port_status(enum bs_port_id bs_port) __attribute__((weak));
extern int wl_max_no_vifs(int unit);
extern const char *bs_port_id_to_iface(enum bs_port_id bs_port);
extern int set_netdev_sysfs_param(const char *iface, const char *param, const char *val);
extern char *get_lan_mac_name(void);
extern char *get_wan_mac_name(void);
extern char *get_2g_hwaddr(void);
extern char *get_5g_hwaddr(void);
extern char *get_lan_hwaddr(void);
extern char *get_wan_hwaddr(void);
extern char *get_label_mac(void);
extern void __wgn_sysdep_swtich_unset(int vid) __attribute__((weak));
extern void __wgn_sysdep_swtich_set(int vid) __attribute__((weak));
#if defined(RTCONFIG_QCA)
extern char *__get_wlifname(int band, int subunit, char *buf);
extern int get_wlsubnet(int band, const char *ifname);
extern int get_wlif_unit(const char *wlifname, int *unit, int *subunit);
extern char *get_wififname(int band);
extern char *get_staifname(int band);
extern char *get_vphyifname(int band);
extern int get_sta_ifname_unit(const char *ifname);
extern int is_vap_ifname(const char *ifname);
extern int is_sta_ifname(const char *ifname);
extern int is_vphy_ifname(const char *ifname);
extern const char *get_5ghigh_ifname(int band);
#ifdef RTCONFIG_POWER_SAVE
extern void set_cpufreq_attr(char *attr, char *val);
#endif
#ifdef IWLIB_H
extern int get_ap_mac(const char *ifname, struct iwreq *pwrq);
#endif
extern int chk_assoc(const char *ifname);
extern int match_radio_status(int unit, int status);
#if defined(RTCONFIG_CFG80211)
extern int get_iwphy_name(int unit, char *iwphy, size_t size);
#endif
extern int create_vap(char *ifname, int unit, char *mode);
extern int destroy_vap(char *ifname);
extern int get_ch(int freq);
extern int get_channel(const char *ifname);
extern unsigned long long get_bitrate(const char *ifname);
extern int get_channel_list(const char *ifname, int ch_list[], int size);
extern int has_dfs_channel(void);
extern int get_radar_channel_list(const char *vphy, int radar_list[], int size);
extern int get_bw_nctrlsb(const char *ifname, int *bw, int *nctrlsb);
extern char *get_wpa_ctrl_sk(int band, char ctrl_sk[], int size);
extern void set_wpa_cli_cmd(int band, const char *cmd, int chk_reply);
#if defined(RTCONFIG_BT_CONN_UART)
extern void execute_bt_bscp();
#endif
#endif
#if defined(RTCONFIG_REALTEK)
extern char *get_wififname(int band);
extern char *get_staifname(int band);
extern int get_channel(const char *ifname);
extern int get_bw_nctrlsb(const char *ifname, int *bw, int *nctrlsb);
extern void set_channel(const char *ifname, int channel);
extern void set_bw_nctrlsb(const char *ifname, int bw, int nctrlsb);
extern int set_mib_acladdr(const char* ifname, char* addr);
#endif
extern char *get_wlifname(int unit, int subunit, int subunit_x, char *buf);
extern char *get_wlxy_ifname(int x, int y, char *buf);
#ifdef CONFIG_BCMWL5
extern char *wl_ifname(int unit, int subunit, char *buf);
extern int wl_check_is_primary_ifce(const char *ifname);
#endif
#if defined(RTCONFIG_RALINK_MT7620)
extern int get_mt7620_wan_unit_bytecount(int unit, unsigned long *tx, unsigned long *rx);
#elif defined(RTCONFIG_RALINK_MT7621)
extern int get_mt7621_wan_unit_bytecount(int unit, unsigned long *tx, unsigned long *rx);
#endif
#ifdef RTCONFIG_AMAS
static inline int rtconfig_amas(void) { return 1; }
extern int aimesh_re_mode(void);
extern void add_beacon_vsie(char *hexdata);
extern void add_beacon_vsie_by_unit(int unit, int subunit, char *hexdata);
extern void add_beacon_vsie_guest(char *hexdata);
extern void del_beacon_vsie(char *hexdata);
extern void del_beacon_vsie_by_unit(int unit, int subunit, char *hexdata);
extern void del_beacon_vsie_guest(char *hexdata);
#ifdef RTCONFIG_RALINK
extern void add_probe_req_vsie(char *hexdata);
extern void del_probe_req_vsie(char *hexdata);
#endif
extern int get_psta_status(int unit);
extern void Pty_stop_wlc_connect(int band);
#ifdef RTCONFIG_BHCOST_OPT
extern void Pty_start_wlc_connect(int band, char* bssid);
extern int amas_dfs_status(int band);
extern void trans_to_bhmode(int *amas_eth_bhmode, int *amas_wifi_bhmode, int *amas_costmode, int *amas_rssiscoremode);
extern unsigned int get_uplinkports_linkrate(char *ifname);
extern void trigger_opt();
#else
extern void Pty_start_wlc_connect(int band);
#endif
extern int Pty_get_upstream_rssi(int band);
extern void pre_addif_bridge(int iftype);
extern void pre_delif_bridge(int iftype);
extern void post_addif_bridge(int iftype);
extern void post_delif_bridge(int iftype);
extern int get_radar_status(int bssidx);
extern int Pty_procedure_check(int unit, int wlif_count);
extern int get_port_status(int unit);
extern int is_wlsta_exist(int unit, int vidx);
extern int get_wl_count();
extern int get_eth_count();
extern int cal_space(char *s1);
extern int is_self_optmz_stage(int band);
extern int enable_ETH_U(int unit);
extern void add_led_ctrl_capability(int val);
extern void del_led_ctrl_capability(int val);
#ifdef RTCONFIG_MSSID_PRELINK
extern void check_mssid_prelink_reset(uint32_t sf);
#endif
#if defined(RTCONFIG_BCMWL6)
extern int get_wl_sta_list(void);
extern int get_maxassoc(char *ifname);
extern int wl_add_ie(int unit, int subunit, uint32 pktflag, int ielen, uchar *oui, uchar *data);
extern void wl_del_ie_with_oui(int unit, int subunit, uchar *oui);
extern int add_interface_for_acsd(int unit);
extern int need_to_start_acsd();
#endif
#if defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_RALINK)
extern void wait_connection_finished(int band);
extern void wait_connection_finished(int band);
#endif
#if defined(RTCONFIG_LANTIQ)
extern int get_wl_sta_list(void);
//extern int get_maxassoc(char *ifname);
extern int get_psta_status(int unit);
#endif
extern int wl_get_bw(int unit);

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
extern int get_psta_status(int unit);
#endif

#if defined(RTCONFIG_QCA)
extern int get_psta_status(int unit);
#endif

#define WLSTA_JSON_FILE 				"/tmp/wl_sta_list.json"
#define MAX_STA_COUNT 128
#define MAX_SUBIF_NUM 4
#if defined(RTCONFIG_LANTIQ)
#define MACF    "%02x:%02x:%02x:%02x:%02x:%02x"
#define ETHERP_TO_MACF(ea)      ((struct ether_addr *) (ea))->ether_addr_octet[0], \
                                ((struct ether_addr *) (ea))->ether_addr_octet[1], \
                                ((struct ether_addr *) (ea))->ether_addr_octet[2], \
                                ((struct ether_addr *) (ea))->ether_addr_octet[3], \
                                ((struct ether_addr *) (ea))->ether_addr_octet[4], \
                                ((struct ether_addr *) (ea))->ether_addr_octet[5]
#define ETHER_TO_MACF(ea)       (ea).ether_addr_octet[0], \
                                (ea).ether_addr_octet[1], \
                                (ea).ether_addr_octet[2], \
                                (ea).ether_addr_octet[3], \
                                (ea).ether_addr_octet[4], \
                                (ea).ether_addr_octet[5]
#elif defined(RTCONFIG_QCA)
#define MACF	"%02x:%02x:%02x:%02x:%02x:%02x"
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->ether_addr_octet[0], \
				((struct ether_addr *) (ea))->ether_addr_octet[1], \
				((struct ether_addr *) (ea))->ether_addr_octet[2], \
				((struct ether_addr *) (ea))->ether_addr_octet[3], \
				((struct ether_addr *) (ea))->ether_addr_octet[4], \
				((struct ether_addr *) (ea))->ether_addr_octet[5]
#define ETHER_TO_MACF(ea)	(ea).ether_addr_octet[0], \
				(ea).ether_addr_octet[1], \
				(ea).ether_addr_octet[2], \
				(ea).ether_addr_octet[3], \
				(ea).ether_addr_octet[4], \
				(ea).ether_addr_octet[5]
#else
#define MACF	"%02x:%02x:%02x:%02x:%02x:%02x"
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->octet[0], \
				((struct ether_addr *) (ea))->octet[1], \
				((struct ether_addr *) (ea))->octet[2], \
				((struct ether_addr *) (ea))->octet[3], \
				((struct ether_addr *) (ea))->octet[4], \
				((struct ether_addr *) (ea))->octet[5]
#define ETHER_TO_MACF(ea)	(ea).octet[0], \
				(ea).octet[1], \
				(ea).octet[2], \
				(ea).octet[3], \
				(ea).octet[4], \
				(ea).octet[5]
#endif

#else	/* !RTCONFIG_AMAS */
static inline int rtconfig_amas(void) { return 0; }
#endif	/* RTCONFIG_AMAS */

/* sysdeps/ralink/ *.c */
#if defined(RTCONFIG_RALINK)
extern char *__get_wlifname(int band, int subunit, char *buf);
extern int rtkswitch_ioctl(int val, int val2);
extern unsigned int rtkswitch_wanPort_phyStatus(int wan_unit);
extern unsigned int rtkswitch_lanPorts_phyStatus(void);
extern unsigned int rtkswitch_WanPort_phySpeed(void);
extern int rtkswitch_WanPort_linkUp(void);
extern int rtkswitch_WanPort_linkDown(void);
extern int rtkswitch_LanPort_linkUp(void);
extern int rtkswitch_LanPort_linkDown(void);
extern int rtkswitch_AllPort_linkUp(void);
extern int rtkswitch_AllPort_linkDown(void);
extern int rtkswitch_Reset_Storm_Control(void);
extern int ralink_gpio_read_bit(int idx);
extern int ralink_gpio_write_bit(int idx, int value);
extern char *wif_to_vif(char *wif);
extern int ralink_gpio_init(unsigned int idx, int dir);
extern int config_rtkswitch(int argc, char *argv[]);
extern int config_mtkswitch(int argc, char *argv[]);
extern int get_channel_list_via_driver(int unit, char *buffer, int len);
extern int get_channel_list_via_country(int unit, const char *country_code, char *buffer, int len);
extern int get_mtk_wifi_driver_version(char *buffer, int len);
#if defined(RTCONFIG_RALINK_MT7620)
extern int __mt7620_wan_bytecount(int unit, unsigned long *tx, unsigned long *rx);
#elif defined(RTCONFIG_RALINK_MT7620)
extern int __mt7621_wan_bytecount(int unit, unsigned long *tx, unsigned long *rx);
#endif
extern int get_channel_list(int unit, int ch_list[], int size);
extern int get_radar_channel_list(int, int radar_list[], int size);
extern int set_acl_entry(const char *ifname, char *addr);
extern int set_channel(const char* ifname, int channel);
extern int set_bw_nctrlsb(const char* ifname, int bw, int nctrlsb);
extern int get_channel_info(const char *ifname, int *channel, int *bw, int *nctrlsb);
extern char *get_wififname(int band);
extern char *get_staifname(int band);

#elif defined(RTCONFIG_QCA)
extern int rtkswitch_ioctl(int val, int *val2);
extern unsigned int rtkswitch_wanPort_phyStatus(int wan_unit);
extern unsigned int rtkswitch_lanPorts_phyStatus(void);
extern unsigned int rtkswitch_WanPort_phySpeed(void);
extern int rtkswitch_WanPort_linkUp(void);
extern int rtkswitch_WanPort_linkDown(void);
extern int rtkswitch_LanPort_linkUp(void);
extern int rtkswitch_LanPort_linkDown(void);
extern int rtkswitch_AllPort_linkUp(void);
extern int rtkswitch_AllPort_linkDown(void);
extern int rtkswitch_Reset_Storm_Control(void);
extern char *wif_to_vif(char *wif);
extern int config_rtkswitch(int argc, char *argv[]);
extern int get_qca8337_PHY_power(int port);
#ifdef RTCONFIG_LAN4WAN_LED
extern int led_ctrl(void);
#endif
extern unsigned int rtkswitch_Port_phyStatus(unsigned int port_mask);
extern unsigned int rtkswitch_Port_phyLinkRate(unsigned int port_mask);
#if defined(RTCONFIG_SOC_IPQ40XX)
extern int get_channel_list_via_driver(int unit, char *buffer, int len);
extern int get_channel_list_via_country(int unit, const char *country_code, char *buffer, int len);
#endif
extern unsigned int __rtkswitch_WanPort_phySpeed(int wan_unit);
extern void ATE_port_status(phy_info_list *list);
#elif defined(RTCONFIG_ALPINE)
extern char *wl_vifname_qtn(int unit, int subunit);
extern char *wif_to_vif(char *wif);
extern int config_rtkswitch(int argc, char *argv[]);
extern unsigned int rtkswitch_wanPort_phyStatus(int wan_unit);
extern unsigned int rtkswitch_lanPorts_phyStatus(void);
extern unsigned int rtkswitch_WanPort_phySpeed(void);
extern void ATE_qca8337_port_status(void);
#elif defined(RTCONFIG_LANTIQ)
extern char *wif_to_vif(char *wif);
extern int config_rtkswitch(int argc, char *argv[]);
extern unsigned int rtkswitch_wanPort_phyStatus(int wan_unit);
extern unsigned int rtkswitch_lanPorts_phyStatus(void);
extern unsigned int rtkswitch_WanPort_phySpeed(void);
extern void ATE_qca8337_port_status(void);
#else
#define wif_to_vif(wif) (wif)
extern int config_rtkswitch(int argc, char *argv[]);
extern unsigned int rtkswitch_lanPorts_phyStatus(void);
#endif

extern int checkcrc(char *fname);
#if defined(RTCONFIG_QCA) && !defined(RTCONFIG_SOC_IPQ40XX)
extern int get_firmware_length(void *ptr);
#else
static inline int get_firmware_length(void *ptr) { return 0; }
#endif

/* sysdeps/qca/ *.c */
extern char *set_steer(const char *mac,int val);

#if defined(RTCONFIG_POWER_SAVE)
extern void set_power_save_mode(void);
#else
static inline void set_power_save_mode(void) { }
#endif

/* sysdeps/broadcom/ *.c */
#ifdef CONFIG_BCMWL5
/* The following PAGE and REG definitions are for BCM53134 and BCM5301x */
#define PAGE_MIB_BASE 0x20
#define REG_OFFSET_TX_BYTES 0x00
#define REG_OFFSET_RX_BYTES 0x50
#define REG_OFFSET_TX_BROADCAST_PACKETS 0x10
#define REG_OFFSET_TX_MULTICAST_PACKETS 0x14
#define REG_OFFSET_TX_UNICAST_PACKETS 0x18
#define REG_OFFSET_RX_BROADCAST_PACKETS 0x9c
#define REG_OFFSET_RX_MULTICAST_PACKETS 0x98
#define REG_OFFSET_RX_UNICAST_PACKETS 0x94
#define REG_OFFSET_RX_FCS_ERROR 0x84
#ifdef RTCONFIG_WIFI6E
#define TMPBUFSIZ 3072
#define TMPBUFSMSIZ 512
#else
#define TMPBUFSIZ 1024
#define TMPBUFSMSIZ 256
#endif
#ifdef RTCONFIG_BCMFA
extern int get_fa_rev(void);
extern int get_fa_dump(void);
#endif
#if defined(RTAX55) || defined(RTAX1800) || defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
	uint64 ifInOctets;
	uint32 dot3StatsFCSErrors;
	uint32 dot3StatsSymbolErrors;
	uint32 dot3InPauseFrames;
	uint32 dot3ControlInUnknownOpcodes;
	uint32 etherStatsFragments;
	uint32 etherStatsJabbers;
	uint32 ifInUcastPkts;
	uint32 etherStatsDropEvents;
	uint64 etherStatsOctets;
	uint32 etherStatsUndersizePkts;
	uint32 etherStatsOversizePkts;
	uint32 etherStatsPkts64Octets;
	uint32 etherStatsPkts65to127Octets;
	uint32 etherStatsPkts128to255Octets;
	uint32 etherStatsPkts256to511Octets;
	uint32 etherStatsPkts512to1023Octets;
	uint32 etherStatsPkts1024toMaxOctets;
	uint32 etherStatsMcastPkts;
	uint32 etherStatsBcastPkts;
	uint64 ifOutOctets;
	uint32 dot3StatsSingleCollisionFrames;
	uint32 dot3StatsMultipleCollisionFrames;
	uint32 dot3StatsDeferredTransmissions;
	uint32 dot3StatsLateCollisions;
	uint32 etherStatsCollisions;
	uint32 dot3StatsExcessiveCollisions;
	uint32 dot3OutPauseFrames;
	uint32 dot1dBasePortDelayExceededDiscards;
	uint32 dot1dTpPortInDiscards;
	uint32 ifOutUcastPkts;
	uint32 ifOutMulticastPkts;
	uint32 ifOutBrocastPkts;
	uint32 outOampduPkts;
	uint32 inOampduPkts;
	uint32 pktgenPkts;
	uint32 inMldChecksumError;
	uint32 inIgmpChecksumError;
	uint32 inMldSpecificQuery;
	uint32 inMldGeneralQuery;
	uint32 inIgmpSpecificQuery;
	uint32 inIgmpGeneralQuery;
	uint32 inMldLeaves;
	uint32 inIgmpLeaves;
	uint32 inIgmpJoinsSuccess;
	uint32 inIgmpJoinsFail;
	uint32 inMldJoinsSuccess;
	uint32 inMldJoinsFail;
	uint32 inReportSuppressionDrop;
	uint32 inLeaveSuppressionDrop;
	uint32 outIgmpReports;
	uint32 outIgmpLeaves;
	uint32 outIgmpGeneralQuery;
	uint32 outIgmpSpecificQuery;
	uint32 outMldReports;
	uint32 outMldLeaves;
	uint32 outMldGeneralQuery;
	uint32 outMldSpecificQuery;
	uint32 inKnownMulticastPkts;
	uint32 ifInMulticastPkts;
	uint32 ifInBroadcastPkts;
	uint32 ifOutDiscards;
} rtk_stat_port_cntr_t;
#endif
#ifdef HND_ROUTER
#if defined(RTCONFIG_HND_ROUTER_AX_6710)
extern uint32_t hnd_get_phy_status(char *ifname);
extern uint32_t hnd_get_phy_speed(char *ifname);
extern uint32_t hnd_get_phy_duplex(char *ifname);
extern uint64_t hnd_get_phy_mib(char *ifname, char *type);
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
extern uint32_t hnd_get_phy_status(int port);
extern uint32_t hnd_get_phy_speed(int port);
extern uint32_t hnd_get_phy_duplex(int port);
extern uint64_t hnd_get_phy_mib(int port, char *type);
#if defined(RTAX55) || defined(RTAX1800) || defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
extern int rtkswitch_port_speed(int port);
extern int rtkswitch_port_duplex(int port);
extern int rtkswitch_port_stat(int port);
extern int rtkswitch_port_mactable(int port);
#endif
extern int ethctl_set_phy(char *ifname, int ctrl);
#else
extern uint32_t hnd_get_phy_status(int port, int offs, unsigned int regv, unsigned int pmdv);
extern uint32_t hnd_get_phy_speed(int port, int offs, unsigned int regv, unsigned int pmdv);
extern uint32_t hnd_get_phy_duplex(int port, int offs, unsigned int regv, unsigned int pmdv);
extern uint64_t hnd_get_phy_mib(int port, int offs, char *type);
#endif
extern uint64_t hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata);
extern uint32_t set_ex53134_ctrl(uint32_t portmask, int ctrl);
extern int ethctl_phy_op(char* phy_type, int addr, unsigned int reg, unsigned int value, int wr);
#ifdef RTCONFIG_EXTPHY_BCM84880
extern int extphy_bit_op(unsigned int reg, unsigned int val, int wr, unsigned int start_bit, unsigned int end_bit, unsigned int wait_ms);
#endif
#if !defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_EXTPHY_BCM84880)
extern int ethctl_get_link_status(char *ifname);
#endif
#endif // HND_ROUTER
extern int fw_check(void);
#ifdef RTCONFIG_BCMWL6
extern int with_non_dfs_chspec(char *wif);
extern chanspec_t select_band1_chspec_with_same_bw(char *wif, chanspec_t chanspec);
extern chanspec_t select_band4_chspec_with_same_bw(char *wif, chanspec_t chanspec);
extern chanspec_t select_chspec_with_band_bw(char *wif, int band, int bw, chanspec_t chanspec);
extern void wl_list_5g_chans(int unit, int band, int war, char *buf, int len);
#endif
extern int wl_cap(int unit, char *cap_check);
#endif
#ifdef RTCONFIG_AMAS
//extern char *get_pap_bssid(int unit, char bssid_str[]);
#ifndef RTCONFIG_LANTIQ
extern int get_wlan_service_status(int bssidx, int vifidx);
extern void set_wlan_service_status(int bssidx, int vifidx, int enabled);
#endif
#endif
#ifdef RTCONFIG_LACP
extern uint32_t traffic_trunk(int port_num, uint32_t *rx, uint32_t *tx);
#endif
#ifdef RTCONFIG_JFFS_NVRAM
extern char * jffs_nvram_get(const char *name);
extern int jffs_nvram_set(const char *name, const char *value);
extern int jffs_nvram_unset(const char *name);
extern int large_nvram(const char *name);
extern void jffs_nvram_init();
extern int jffs_nvram_getall(int len_nvram, char *buf, int count);
#endif
#ifdef RTCONFIG_VAR_NVRAM
extern char *var_nvram_get(const char *name);
extern int var_nvram_set(const char *name, const char *value);
extern int var_nvram_unset(const char *name);
extern int is_var_nvram(const char *name);
extern void var_nvram_init();
extern int var_nvram_getall(char *buf, size_t n);
#endif

// base64.c
extern int base64_encode(const unsigned char *in, char *out, int inlen);		// returns amount of out buffer used
extern int base64_decode(const char *in, unsigned char *out, int inlen);	// returns amount of out buffer used
extern int base64_encoded_len(int len);
extern int base64_decoded_len(int len);										// maximum possible, not actual

/* boardapi.c */
extern int extract_gpio_pin(const char *gpio);
extern int lanport_status(void);
extern void get_gpio_values_once(int force);
#ifdef HND_ROUTER
extern int _gpio_active_low(int gpionr);
extern void dump_ledtable();
#endif

/* discover.c */
extern int discover_interfaces(int num, const char **current_wan_ifnames, int dhcp_det, int *got_inf);
extern int discover_interface(const char *current_wan_ifname, int dhcp_det);
extern int discover_all(int wan_unit);

// strings.c
extern int char_to_ascii_safe(const char *output, const char *input, int outsize);
extern void char_to_ascii(const char *output, const char *input);
#if defined(RTCONFIG_UTF8_SSID)
extern int char_to_ascii_safe_with_utf8(const char *output, const char *input, int outsize);
extern void char_to_ascii_with_utf8(const char *output, const char *input);
#endif
extern int ascii_to_char_safe(const char *output, const char *input, int outsize);
extern void ascii_to_char(const char *output, const char *input);
extern const char *find_word(const char *buffer, const char *word);
extern int remove_word(char *buffer, const char *word);
extern void trim_space(char *str);
extern void replace_char(char *str, char find, char replace);
extern int str_escape_quotes(const char *output, const char *input, int outsize);
extern void toLowerCase(char *str);
extern void toUpperCase(char *str);
extern void trim_colon(char *str);
extern void trim_char(char *str, char c);

/* ethtool.c */
extern int iface_exist(const char *iface);
extern int ethtool_glink(char *iface);
extern int ethtool_gset(char *iface, uint32_t *speed, int *duplex);

// file.c
extern int check_if_file_exist(const char *file);
extern int check_if_dir_exist(const char *file);
extern int check_if_dir_writable(const char *dir);
extern unsigned int __dir_size__;
extern int sum(const char *fpath, const struct stat *sb, int typeflag);
extern int d_size(const char *dirPath);

#if defined(RTCONFIG_SAVEJFFS)
/* jffs_cfgs.c */
extern int append_jffs_cfgs(FILE *fp_jc, char *nv_fn);
extern int restore_jffs_cfgs(char *nv_fn);
#endif

/* mdio.c */
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
extern int mdio_read(char *ifname, int location);
extern int mdio_write(char *ifname, int location, int value);
extern int mdio_phy_speed(char *ifname);
#endif
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
extern int is_aqr_phy_exist(void);
extern int aqr_phy_addr(void);
extern int parse_ssdk_sh(const char *cmd, const char *fmt, int cnt, ...);
extern int read_phy_reg(unsigned int phy, unsigned int reg);
extern int write_phy_reg(unsigned int phy, unsigned int reg, unsigned int value);
extern int mdio_phy_speed(unsigned int phy);
extern int ipq8074_port_speed(unsigned int port);
extern int qca8337_port_speed(unsigned int port);
extern int aqr_phy_speed(unsigned int phy);
#else
static inline int is_aqr_phy_exist(void)
{
#if defined(RTCONFIG_SWITCH_QCA8075_PHY_AQR107)
	return 1;
#else
	return 0;
#endif
}
#endif	/* RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033 */

/* misc.c */
extern char *get_unused_brif(unsigned int num, char *ret_buffer, size_t ret_buffer_size);
#if !defined(HND_ROUTER)
extern void ipt_account(FILE *fp, char *interface);
#endif
extern char *get_productid(void);
extern char *get_lan_hostname(void);
extern void logmessage_normal(char *logheader, char *fmt, ...);
extern char *get_logfile_path(void);
extern char *get_syslog_fname(unsigned int idx);
#ifdef RTCONFIG_USB_MODEM
extern char *get_modemlog_fname(void);
#endif
#if defined(RTCONFIG_HTTPS)
extern int nvram_get_file(const char *key, const char *fname, int max);
extern int nvram_set_file(const char *key, const char *fname, int max);
#endif
extern int free_caches(const char *clean_mode, const int clean_time, const unsigned int threshold);
extern int update_6rd_info(void);
extern int is_private_subnet(const char *ip);
extern const char *get_wanface(void);
extern const char *get_wanip(void);
extern int is_intf_up(const char* ifname);
extern uint32_t crc_calc(uint32_t crc, const char *buf, int len);
extern int illegal_ipv4_address(char *addr);
extern int illegal_ipv4_netmask(char *netmask);
extern void convert_mac_string(char *mac);
extern int test_and_get_free_uint_network(int t_class, uint32_t *exp_ip, uint32_t exp_cidr, uint32_t excl);
extern int test_and_get_free_char_network(int t_class, char *ip_cidr_str, uint32_t excl);
extern enum wan_unit_e get_first_connected_public_wan_unit(void);
#ifdef RTCONFIG_IPV6
extern const char *get_wan6face(void);
extern const char *ipv6_address(const char *ipaddr6);
extern const char *ipv6_prefix(struct in6_addr *in6addr);
#if 0 /* unused */
extern const char *ipv6_prefix(const char *ifname);
extern int ipv6_prefix_len(const char *ifname);
#endif
extern void reset_ipv6_linklocal_addr(const char *ifname, int flush);
extern int with_ipv6_linklocal_addr(const char *ifname);
#if 1 /* temporary till httpd route table redo */
extern void ipv6_set_flags(char *flagstr, int flags);
extern char* INET6_rresolve(struct sockaddr_in6 *sin6, int numeric);
#endif
#endif
#if defined(RTCONFIG_OPENVPN) || defined(RTCONFIG_IPSEC)
#define OVPN_FS_PATH	"/jffs/openvpn"
extern int set_crt_parsed(const char *name, char *file_path);
#endif
extern int get_upstream_wan_unit(void);
extern int __get_upstream_wan_unit(void) __attribute__((weak));
extern int get_wifi_unit(char *wif);
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
#ifdef RTCONFIG_DPSTA
extern int is_dpsta(int unit);
#endif
extern int is_dpsr(int unit);
extern int is_psta(int unit);
extern int is_psr(int unit);
extern int psta_exist(void);
extern int psta_exist_except(int unit);
extern int psr_exist(void);
extern int psr_exist_except(int unit);
#endif

struct ifino_s {
	char ifname[IFNAMSIZ];
	ino_t inode;
	unsigned long long last_rx, last_tx;
	unsigned long long shift_rx, shift_tx;
};

struct ifname_ino_tbl {
	unsigned nr_items;
	struct ifino_s items[50];
};

extern struct ifino_s *ifname_ino_ptr(struct ifname_ino_tbl *ifinotbl, const char *ifname);
extern ino_t get_iface_inode(const char *ifname);
extern uint32_t nums_str_to_u32_mask(const char *str);
extern unsigned int netdev_calc(char *ifname, char *ifname_desc, unsigned long long *rx, unsigned long long *tx, char *ifname_desc2, unsigned long long *rx2, unsigned long long *tx2, char *nv_lan_ifname, char *nv_lan_ifnames);
extern void disable_dpi_engine_setting(void);
extern int get_iface_hwaddr(char *name, unsigned char *hwaddr);
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
extern int __set_iface_ps(const char *ifname, int nr_rx_mask, const unsigned int *rx_mask, int nr_tx_mask, const unsigned int *tx_mask);
static inline int set_iface_ps2(const char *ifname, unsigned int rx_mask, unsigned tx_mask)
{
	const unsigned int rx_mask_ary[] = { rx_mask }, tx_mask_ary[] = { tx_mask };
	return __set_iface_ps(ifname, 0, rx_mask_ary, 0, tx_mask_ary);
}
static inline int set_iface_ps(const char *ifname, unsigned int rx_mask)
{
	const unsigned int rx_mask_ary[] = { rx_mask };
	return __set_iface_ps(ifname, 0, rx_mask_ary, -1, NULL);
}
extern int ctrl_gro(char *iface, int onoff);
extern int ctrl_wan_gro(int wan_unit, int onoff);
extern int ctrl_lan_gro(int onoff);
#else
static inline int __set_iface_ps(__attribute__ ((unused)) const char *ifname, __attribute__ ((unused)) int nr_rx_mask,\
	__attribute__ ((unused)) const unsigned int *rx_mask, __attribute__ ((unused)) int nr_tx_mask,\
	__attribute__ ((unused)) const unsigned int *tx_mask) { return 0; }
static inline int set_iface_ps2(__attribute__ ((unused)) const char *ifname, __attribute__ ((unused)) unsigned int rx_mask,\
	__attribute__ ((unused)) unsigned tx_mask) { return 0; }
static inline int set_iface_ps(__attribute__ ((unused)) const char *ifname, __attribute__ ((unused)) unsigned int rx_mask) { return 0; }
static inline int ctrl_gro(__attribute__ ((unused)) char *iface, __attribute__ ((unused)) int onoff) { return 0; }
static inline int ctrl_wan_gro(__attribute__ ((unused)) int wan_unit, __attribute__ ((unused)) int onoff) { return 0; }
static inline int ctrl_lan_gro(__attribute__ ((unused)) int onoff) { return 0; }
#endif
extern int u_readl(unsigned long addr, unsigned long *value);
extern int set_irq_smp_affinity(unsigned int irq, unsigned int cpu_mask);
extern int set_irq_smp_affinity_by_name(const char *name, int order, unsigned int cpu_mask);
extern int get_active_fw_num(void);
extern int exec_and_parse(const char *cmd, const char *keyword, const char *fmt, int cnt, ...);
extern char *iwpriv_get(const char *iface, char *cmd);
extern int iwpriv_get_int(const char *iface, char *cmd, int *result);
extern int readdir_wrapper(const char *path, const char *keyword, int (*handler)(const char *path, const struct dirent *de, void *arg), void *arg);
extern char *get_qos_prefix(int unit, char *buf);
extern int internet_ready(void);
extern void set_no_internet_ready(void);
extern unsigned int num_of_mssid_support(unsigned int unit);
extern enum led_id get_wl_led_id(int band);
extern char *get_wl_led_gpio_nv(int band);
#if defined(RTCONFIG_QCA)
extern char *get_wsup_drvname(int band);
extern void disassoc_sta(char *ifname, char *sta_addr);
/* mode: 0:disable, 1:allow, 2:deny */
void set_maclist_mode(char *ifname, int mode);
extern void set_maclist_add_kick(char *ifname, int mode, char *sta_addr);
extern void set_maclist_del_kick(char *ifname, int mode, char *sta_addr);
#include <stdio.h>
void set_macfilter_unit(int unit, int subnet, FILE *fp);
void set_macfilter_all(FILE *fp);
extern int get_wifi_temperature(enum wl_band_id band);
#else
static inline char *get_wsup_drvname(__attribute__ ((unused)) int band) { return ""; }
#endif
#ifdef RTCONFIG_TRAFFIC_LIMITER
extern unsigned int traffic_limiter_read_bit(const char *type);
extern void traffic_limiter_set_bit(const char *type, int unit);
extern void traffic_limiter_clear_bit(const char *type, int unit);
extern double traffic_limiter_get_realtime(int unit);
#endif
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
extern struct vlan_rules_s *get_vlan_rules(void);
#endif
#if defined(RTCONFIG_BONDING)
#if defined(HND_ROUTER)
extern int get_bonding_status();
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
extern int get_bonding_speed(char *bond_if);
#endif
#endif // RTCONFIG_BONDING
extern int isValidMacAddress(const char* mac);
extern int isValidMacAddr_and_isNotMulticast(const char* mac);
extern int isValidEnableOption(const char* option, int range);
extern int isValid_digit_string(const char *string);
extern int is_valid_hostname(const char *name);
extern int is_valid_domainname(const char *name);

/* scripts.c */
extern void run_custom_script(char *name, int timeout, char *arg1, char *arg2);
extern void run_postconf(char *name, char *config);
extern void use_custom_config(char *config, char *target);
extern void append_custom_config(char *config, FILE *fp);

/* mt7620.c */
#if defined(RTCONFIG_RALINK_MT7620)
extern void ATE_mt7620_esw_port_status(void);
#elif defined(RTCONFIG_RALINK_MT7621)
extern void ATE_mt7621_esw_port_status(void);
#endif

/* mt7628.c */
extern void ATE_mt7628_esw_port_status(void);

/* notify_rc.c */
extern int notify_rc(const char *event_name);
extern int notify_rc_after_wait(const char *event_name);
extern int notify_rc_after_period_wait(const char *event_name, int wait);
extern int notify_rc_and_wait(const char *event_name);
extern int notify_rc_and_wait_1min(const char *event_name);
extern int notify_rc_and_wait_2min(const char *event_name);
extern int notify_rc_and_period_wait(const char *event_name, int wait);

/* wl.c */
#ifdef CONFIG_BCMWL5
#ifdef __CONFIG_DHDAP__
extern int dhd_probe(char *name);
extern int dhd_ioctl(char *name, int cmd, void *buf, int len);
extern int dhd_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern int dhd_iovar_setint(char *ifname, char *iovar, int val);
#endif
#endif

/* rtstate.c */
extern char *get_wanx_ifname(int unit);
extern int get_lanports_status(void);
extern int set_wan_primary_ifunit(const int unit);
#ifdef RTCONFIG_USB
extern char *get_usb_xhci_port(int port);
#endif
#ifdef RTCONFIG_DUALWAN
extern int get_nr_wan_unit(void);
#endif // RTCONFIG_DUALWAN

static inline int iptv_enabled(void)
{
	int stb_x;

	/* switch_wantag may be NULL if restore_defaults() hasn't been executed. */
	if ((strlen(nvram_safe_get("switch_wantag")) > 0 && !nvram_match("switch_wantag", "none")) ||
	    (nvram_match("switch_wantag", "none") && ((stb_x = nvram_get_int("switch_stb_x")) > 0 && stb_x <= 6)))
		return 1;

	return 0;
}
extern int get_nr_guest_network(int band);
extern int get_gate_num(void);
extern void set_lan_phy(char *phy);
extern void add_lan_phy(char *phy);
extern void set_wan_phy(char *phy);
extern void add_wan_phy(char *phy);

/* semaphore.c */
extern void init_spinlock(void);

#if defined(RTCONFIG_WPS_LED)
static inline int wps_led_control(int onoff)
{
	return led_control(LED_WPS, onoff);
}

static inline int __wps_led_control(int onoff)
{
	return led_control(LED_WPS, onoff);
}
#else
static inline int wps_led_control(int onoff)
{
	if (nvram_get_int("led_pwr_gpio") != nvram_get_int("led_wps_gpio"))
		return led_control(LED_WPS, onoff);
	else
		return led_control(LED_POWER, onoff);
}

/* If individual WPS LED absents, don't do anything. */
static inline int __wps_led_control(int __attribute__((__unused__)) onoff) { return 0; }
#endif

enum {
	YADNS_DISABLED = -1,
	YADNS_BASIC,
	YADNS_SAFE,
	YADNS_FAMILY,
	YADNS_COUNT,
	YADNS_FIRST = YADNS_DISABLED + 1,
};

#ifdef RTCONFIG_YANDEXDNS
#define YADNS_DNSPORT 1253
int get_yandex_dns(int family, int mode, char **server, int max_count);
#endif

#if defined(RTCONFIG_PWRRED_LED)
static inline int power_red_led_control(int onoff)
{
	return led_control(LED_POWER_RED, onoff);
}
#else
static inline int power_red_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_FAILOVER_LED)
static inline int failover_led_control(int onoff)
{
	return led_control(LED_FAILOVER, onoff);
}
#else
static inline int failover_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_WIGIG) && !defined(GTAXY16000)
static inline int wigig_led_control(int onoff)
{
	return led_control(LED_60G, onoff);
}
#else
static inline int wigig_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_M2_SSD)
static inline int sata_led_control(int onoff)
{
	return led_control(LED_SATA, onoff);
}
#else
static inline int sata_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_R10G_LED)
static inline int r10g_led_control(int onoff)
{
	return led_control(LED_R10G, onoff);
}
#else
static inline int r10g_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_SFPP_LED)
static inline int sfpp_led_control(int onoff)
{
	return led_control(LED_SFPP, onoff);
}
#else
static inline int sfpp_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_WANRED_LED)
static inline int wan_red_led_control(int onoff)
{
	return led_control(LED_WAN_RED, onoff);
}
#else
static inline int wan_red_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif	/* RTCONFIG_WANRED_LED */

#if defined(RTCONFIG_WANRED_LED) && defined(RTCONFIG_WANLEDX2)
static inline int wan2_red_led_control(int onoff)
{
	return led_control(LED_WAN2_RED, onoff);
}
#else
static inline int wan2_red_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_LOGO_LED)
static inline int logo_led_control(int onoff)
{
	return led_control(LED_LOGO, onoff);
}
#else
static inline int logo_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_LED_ALL)
static inline int all_led_control(int onoff)
{
	return led_control(LED_ALL, onoff);
}
#else
static inline int all_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_TURBO_BTN)
static inline int turbo_led_control(int onoff)
{
	return led_control(LED_TURBO, onoff);
}

static inline int boost_led_control(int onoff)
{
#if defined(GTAX11000) || defined(GTAXE11000)
	return led_control(LED_LOGO, onoff);
#else
	return turbo_led_control(onoff);
#endif
}
#else
static inline int turbo_led_control(__attribute__ ((unused)) int onoff) { return 0; }
static inline int boost_led_control(__attribute__ ((unused)) int onoff) { return 0; }
#endif

#if defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
static inline int inhibit_led_on(void) { return !nvram_get_int("AllLED"); }
#else
static inline int inhibit_led_on(void) { return 0; }
#endif

/* bled.c */
#if defined(RTCONFIG_BLINK_LED)
extern int __config_netdev_bled(const char *led_gpio, const char *ifname, unsigned int min_blink_speed, unsigned int interval);
extern int config_netdev_bled(const char *led_gpio, const char *ifname);
extern int set_bled_udef_pattern(const char *led_gpio, unsigned int interval, const char *pattern);
extern int set_bled_udef_tigger(const char *main_led_gpio, const char *tigger);
extern int set_bled_normal_mode(const char *led_gpio);
extern int set_bled_udef_pattern_mode(const char *led_gpio);
extern int start_bled(unsigned int gpio_nr);
extern int stop_bled(unsigned int gpio_nr);
extern int del_bled(unsigned int gpio_nr);
extern int append_netdev_bled_if(const char *led_gpio, const char *ifname);
extern int remove_netdev_bled_if(const char *led_gpio, const char *ifname);
extern int __config_swports_bled(const char *led_gpio, unsigned int port_mask, unsigned int min_blink_speed, unsigned int interval, int sleep);
extern int update_swports_bled(const char *led_gpio, unsigned int port_mask);
extern int __config_usbbus_bled(const char *led_gpio, char *bus_list, unsigned int min_blink_speed, unsigned int interval);
extern int is_swports_bled(const char *led_gpio);
extern int __config_interrupt_bled(const char *led_gpio, char *interrupt_list, unsigned int min_blink_speed, unsigned int interval);
extern int config_interrupt_bled(const char *led_gpio, char *interrupt_list);
extern int add_gpio_to_bled(const char *main_led_gpio, const char *led_gpio);
#if (defined(PLN12) || defined(PLAC56))
extern void set_wifiled(int mode);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
/* color */
#define RGBLED_OFF			0x0
#define RGBLED_BLED			0x1
#define RGBLED_GLED			0x2
#define RGBLED_RLED			0x4
#define RGBLED_WLED			0x8
#define RGBLED_COLOR_MESK		(RGBLED_BLED | RGBLED_GLED | RGBLED_RLED | RGBLED_WLED)
#define RGBLED_BLUE			RGBLED_BLED
#define RGBLED_GREEN			RGBLED_GLED
#define RGBLED_RED			RGBLED_RLED
#if defined(PLAX56_XP4)
#define RGBLED_WHITE			RGBLED_COLOR_MESK
#else
#define RGBLED_WHITE			(RGBLED_BLED | RGBLED_GLED | RGBLED_RLED)
#endif
#define RGBLED_NIAGARA_BLUE		(RGBLED_BLED | RGBLED_GLED)
#define RGBLED_YELLOW			(RGBLED_GLED | RGBLED_RLED)
#define RGBLED_PURPLE			(RGBLED_BLED | RGBLED_RLED)
/* blink */
#define RGBLED_SBLINK			0x10
#define RGBLED_3ON1OFF			0x20
#define RGBLED_ATE_MODE			0x40
#define RGBLED_3ON3OFF			0x80
#define RGBLED_BLINK_MESK		(RGBLED_SBLINK | RGBLED_3ON1OFF | RGBLED_ATE_MODE | RGBLED_3ON3OFF)
/* color+blink */
#define RGBLED_GREEN_3ON1OFF		(RGBLED_GREEN | RGBLED_3ON1OFF)
#define RGBLED_GREEN_3ON3OFF		(RGBLED_GREEN | RGBLED_3ON3OFF)
#define RGBLED_BLUE_3ON1OFF		(RGBLED_BLUE | RGBLED_3ON1OFF)
#define RGBLED_BLUE_3ON3OFF		(RGBLED_BLUE | RGBLED_3ON3OFF)
#define RGBLED_PURPLE_3ON1OFF		(RGBLED_PURPLE | RGBLED_3ON1OFF)
#define RGBLED_WHITE_SBLINK		(RGBLED_WHITE | RGBLED_SBLINK)
#define RGBLED_YELLOW_SBLINK		(RGBLED_YELLOW | RGBLED_SBLINK)
/* event */
#if defined(MAPAC1750)
#define RGBLED_BOOTING			RGBLED_BLUE_3ON3OFF
#define RGBLED_DEFAULT_STANDBY		RGBLED_WHITE
#define RGBLED_APPLY_EVENT		RGBLED_BOOTING
#define RGBLED_BT_CONNECT		RGBLED_WHITE_SBLINK
#define RGBLED_PRESS_RSTBTN		RGBLED_YELLOW_SBLINK
#define RGBLED_RST_EVENT		RGBLED_YELLOW
#define RGBLED_WPS_EVENT		RGBLED_GREEN_3ON1OFF
#define RGBLED_SYNC_EVENT		RGBLED_WPS_EVENT
#define RGBLED_RE_JOIN			RGBLED_GREEN		/* unnecessary */
#define RGBLED_CONNECTED		RGBLED_NIAGARA_BLUE
#define RGBLED_DISCONNECTED		RGBLED_RED
#define RGBLED_AP_MODE_CONNECTED	RGBLED_YELLOW		/* unnecessary */
#define RGBLED_ETH_BACKHAUL		RGBLED_GREEN
#define RGBLED_WEAK_BACKHAUL		RGBLED_YELLOW		/* unnecessary */
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
#define RGBLED_BOOTING			RGBLED_GREEN_3ON3OFF
#define RGBLED_DEFAULT_STANDBY		RGBLED_BLUE
#define RGBLED_APPLY_EVENT		RGBLED_BOOTING
#define RGBLED_BT_CONNECT		RGBLED_WHITE_SBLINK	/* unnecessary */
#define RGBLED_PRESS_RSTBTN		RGBLED_YELLOW_SBLINK
#define RGBLED_RST_EVENT		RGBLED_YELLOW
#define RGBLED_WPS_EVENT		RGBLED_BLUE_3ON1OFF
#define RGBLED_SYNC_EVENT		RGBLED_WPS_EVENT
#define RGBLED_RE_JOIN			RGBLED_GREEN
#define RGBLED_CONNECTED		RGBLED_WHITE
#define RGBLED_DISCONNECTED		RGBLED_RED
#define RGBLED_AP_MODE_CONNECTED	RGBLED_YELLOW
#define RGBLED_ETH_BACKHAUL		RGBLED_WHITE
#define RGBLED_WEAK_BACKHAUL		RGBLED_YELLOW
#endif
extern void set_rgbled(unsigned int mode);
#endif

static inline void enable_wifi_bled(char *ifname)
{
	int unit;
	int v = LED_ON;

	if (!ifname || *ifname == '\0')
		return;
	unit = get_wifi_unit(ifname);
	if (unit < 0 || unit > MAX_NR_WL_IF) {
		return;
	}

	if (!guest_wlif(ifname)) {
#if defined(RTCONFIG_QCA)
		v = LED_OFF;	/* WiFi not ready. Don't turn on WiFi LED here. */
#endif
#if defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined(RTAC1200GU) || defined(RTAC85U) || defined(RTAC85P) || defined(RTACRH26) || defined(TUFAC1750)
		if(!get_radio(1, 0) && unit==1) //*5G WiFi not ready. Don't turn on WiFi GPIO LED . */
		 	v=LED_OFF;
#endif
#if defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined(RTAC1200GU) || defined(RTAC85U) || defined(RTAC85P) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750)
		if(!get_radio(0, 0) && unit==0) //*2G WiFi not ready. Don't turn on WiFi GPIO LED . */
		 	v=LED_OFF;
#endif
		led_control(get_wl_led_id(unit), v);
	} else {
		append_netdev_bled_if(get_wl_led_gpio_nv(unit), ifname);
	}
}

static inline void disable_wifi_bled(char *ifname)
{
	int unit;

	if (!ifname || *ifname == '\0')
		return;
	unit = get_wifi_unit(ifname);
	if (absent_band(unit))
		return;

	if (!guest_wlif(ifname)) {
		led_control(get_wl_led_id(unit), LED_OFF);
	} else {
		remove_netdev_bled_if(get_wl_led_gpio_nv(unit), ifname);
	}
}

static inline int config_swports_bled(const char *led_gpio, unsigned int port_mask)
{
	unsigned int min_blink_speed = 10;	/* KB/s */
	unsigned int interval = 100;		/* ms */

	return __config_swports_bled(led_gpio, port_mask, min_blink_speed, interval, 0);
}

static inline int config_swports_bled_sleep(const char *led_gpio, unsigned int port_mask)
{
	unsigned int min_blink_speed = 10;	/* KB/s */
	unsigned int interval = 100;		/* ms */

	return __config_swports_bled(led_gpio, port_mask, min_blink_speed, interval, 1);
}

static inline int config_usbbus_bled(const char *led_gpio, char *bus_list)
{
	unsigned int min_blink_speed = 50;	/* KB/s */
	unsigned int interval = 100;		/* ms */

	return __config_usbbus_bled(led_gpio, bus_list, min_blink_speed, interval);
}

/* model-specific helper function for bled */
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
/* Implement below functions for bled in model-specific code if and only if
 * it has complex network infrastructure and virtual port mask is used to
 * simplify switch configuration code.
 */
extern const char *vport_to_iface_name(unsigned int vport);
extern unsigned int vportmask_to_rportmask(unsigned int vportmask);
#else
#if defined(RTCONFIG_SOC_IPQ60XX)
extern const char *vport_to_iface_name(unsigned int vport);
#else
static inline const char *vport_to_iface_name(__attribute__ ((unused)) unsigned int vport) { return NULL; }
#endif
static inline unsigned int vportmask_to_rportmask(unsigned int vportmask) { return vportmask; };
#endif

#else	/* !RTCONFIG_BLINK_LED */
static inline int __config_netdev_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) const char *ifname, __attribute__ ((unused)) unsigned int min_blink_speed, __attribute__ ((unused)) unsigned int interval) { return 0; }
static inline int config_netdev_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) const char *ifname) { return 0; }
static inline int set_bled_udef_pattern(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) unsigned int interval, __attribute__ ((unused)) const char *pattern) { return 0; }
static inline int set_bled_udef_tigger(__attribute__ ((unused)) const char *main_led_gpio, __attribute__ ((unused)) const char *tigger) { return 0; }
static inline int set_bled_normal_mode(__attribute__ ((unused)) const char *led_gpio) { return 0; }
static inline int set_bled_udef_pattern_mode(__attribute__ ((unused)) const char *led_gpio) { return 0; }
static inline int start_bled(__attribute__ ((unused)) unsigned int gpio_nr) { return 0; }
static inline int stop_bled(__attribute__ ((unused)) unsigned int gpio_nr) { return 0; }
static inline int chg_bled_state(__attribute__ ((unused)) unsigned int gpio_nr) { return 0; }
static inline int del_bled(__attribute__ ((unused)) unsigned int gpio_nr) { return 0; }
static inline int append_netdev_bled_if(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) const char *ifname) { return 0; }
static inline int remove_netdev_bled_if(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) const char *ifname) { return 0; }
static inline int __config_swports_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) unsigned int port_mask, __attribute__ ((unused)) unsigned int min_blink_speed, __attribute__ ((unused)) unsigned int interval, __attribute__ ((unused)) int sleep) { return 0; }
static inline int update_swports_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) unsigned int port_mask) { return 0; }
static inline int __config_usbbus_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) char *bus_list, __attribute__ ((unused)) unsigned int min_blink_speed, __attribute__ ((unused)) unsigned int interval) { return 0; }
static inline int __config_interrupt_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) char *interrupt_list, __attribute__ ((unused)) unsigned int min_blink_speed, __attribute__ ((unused)) unsigned int interval) { return 0; }
static inline int config_interrupt_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) char *interrupt_list) { return 0; }

static inline void enable_wifi_bled(__attribute__ ((unused)) char *ifname) { }
static inline void disable_wifi_bled(__attribute__ ((unused)) char *ifname) { }
static inline int config_swports_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) unsigned int port_mask) { return 0; }
static inline int config_swports_bled_sleep(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) unsigned int port_mask) { return 0; }
static inline int config_usbbus_bled(__attribute__ ((unused)) const char *led_gpio, __attribute__ ((unused)) char *bus_list) { return 0; }
static inline int is_swports_bled(__attribute__ ((unused)) const char *led_gpio) { return 0; }
static inline int add_gpio_to_bled(__attribute__ ((unused)) const char *main_led_gpio, __attribute__ ((unused)) const char *led_gpio) { return 0; }

#endif	/* RTCONFIG_BLINK_LED */

/* bwdpi_utils.c */
#if defined(RTCONFIG_BWDPI)
extern int check_wrs_switch();
extern int check_bwdpi_nvram_setting();
#endif
extern void erase_symbol(char *old, char *sym);
extern void StampToDate(unsigned long timestamp, char *date);
extern int check_filesize_over(char *path, long int size);
extern time_t get_last_month_timestamp();

#if defined(RTCONFIG_USB)
static inline int is_usb3_port(char *usb_node)
{
	if (!usb_node)
		return 0;

#if defined(RTCONFIG_QCA)
	/* If your platform has two USB3 bus, first item of ehci_port must be checked. */
	if (strstr(usb_node, get_usb_xhci_port(0)) ||
	    strstr(usb_node, get_usb_ehci_port(0))
	   )
		return 1;

	return 0;
#else
	if (strstr(usb_node, get_usb_xhci_port(0)) || strstr(usb_node, get_usb_xhci_port(1)))
		return 1;

	return 0;
#endif
}
#endif

#if defined(RTCONFIG_M2_SSD)
static inline int is_m2ssd_port(char *usb_node)
{
	if (!usb_node)
		return 0;

	if (strstr(usb_node, get_usb_ehci_port(2)))
		return 1;

	return 0;
}
#else
static inline int is_m2ssd_port(__attribute__ ((unused)) char *usb_node) { return 0; }
#endif

#ifdef RTCONFIG_BCM5301X_TRAFFIC_MONITOR

#if defined(RTN18U) || defined(RTAC56U) || defined(RTAC56S) || defined(RTAC68U) || defined(RTAC3200) || defined(DSL_AC68U)
#define WAN0DEV "vlan2"
#endif

#if defined(RTAC5300)
#define WAN0DEV "vlan2"
#endif

#if defined(RTAC88U) || defined(RTAC3100)
#ifdef RTCONFIG_EXT_RTL8365MB
#define WAN0DEV "vlan2"
#else
#define WAN0DEV "vlan2"
#endif
#endif

#ifdef RTAC87U
#define WAN0DEV "vlan2"
#endif
#endif	/* RTCONFIG_BCM5301X_TRAFFIC_MONITOR */

#ifdef RTCONFIG_CAPTIVE_PORTAL
extern void deauth_guest_sta(char *, char *);
extern int FindBrifByWlif(char *wl_ifname, char *brif_name, int size);
#endif

#ifdef RTCONFIG_HTTPS
#define HTTPD_CERT	"/etc/cert.pem"
#define HTTPD_KEY	"/etc/key.pem"
#define LIGHTTPD_CERTKEY	"/etc/server.pem"
#define UPLOAD_CERT_FOLDER	"/jffs/.cert"
#define UPLOAD_CERT	"/jffs/.cert/cert.pem"
#define UPLOAD_KEY	"/jffs/.cert/key.pem"
#ifdef RTCONFIG_LETSENCRYPT
#define ACME_CERTHOME	"/jffs/.le"
#endif
#endif

#ifdef RTAC68U
extern int is_ac66u_v2_series();
extern int is_n66u_v2();
extern int is_ac68u_v3_series();
extern int hw_usb_cap();
extern int is_ssid_rev3_series();
#ifdef RTCONFIG_TCODE
extern unsigned int hardware_flag();
#endif
extern int is_dpsta_repeater();
extern void ac68u_cofs();
#endif

#ifdef DSL_AX82U
extern int is_ax5400_i1();
#endif

/* rtstate.c */
extern char *get_default_ssid(int unit, int subunit);

extern int wanport_status(int wan_unit);

/* mac_name_tab.c */
extern char *search_mnt(char *mac);

/* bwdpi_utils.c */
extern void erase_symbol(char *old, char *sym);

/* pwenc.c */
#if defined(RTCONFIG_NVRAM_ENCRYPT) || defined(RTCONFIG_ASD)
extern int pw_enc(const char *input, char *output);
extern int pw_dec(const char *input, char *output, int len);
extern int pw_enc_blen(const char *input);
extern int pw_dec_len(const char *input);
#endif
#ifdef RTCONFIG_NVRAM_ENCRYPT
#define NVRAM_ENC_LEN	1024
#define NVRAM_ENC_MAXLEN	4096
extern int set_enc_nvram(char *name, char *input, char *output);
extern int enc_nvram(char *name, char *input, char *output);
extern int dec_nvram(char *name, char *input, char *output);
extern int start_enc_nvram(void);
extern int start_dec_nvram(void);
extern int init_enc_nvram(void);
#endif

/* amas_utils.c */
#ifdef RTCONFIG_AMAS
extern int is_wlsta_connect(int unit, int vidx, char *macaddr);
extern void set_deauth_sta(int bssidx, int vifidx, char *mac_addr);
#endif
#ifdef RTCONFIG_BHCOST_OPT
extern int gen_uplinkport_describe(char *port_def, char *type, char *subtype, int index);
#endif

#ifdef RTCONFIG_ISP_CUSTOMIZE
extern char *find_customize_setting_by_name(const char *name);
#endif

enum {
	CKN_STR_DEFAULT_ASUS = 0,
	CKN_STR1,
	CKN_STR2,
	CKN_STR3,
	CKN_STR4,
	CKN_STR5,
	CKN_STR6,
	CKN_STR7,
	CKN_STR8,
	CKN_STR10   = 10,
	CKN_STR12   = 12,
	CKN_STR15   = 15,
	CKN_STR16,
	CKN_STR17,
	CKN_STR20   = 20,
	CKN_STR32   = 32,
	CKN_STR39   = 39,
	CKN_STR63   = 63,
	CKN_STR64   = 64,
	CKN_STR65,
	CKN_STR100  = 100,
	CKN_STR128  = 128,
	CKN_STR255  = 255,
	CKN_STR256,
	CKN_STR512  = 512,
	CKN_STR1024 = 1024,
	CKN_STR2048 = 2048,
	CKN_STR2500 = 2500,
	CKN_STR2999 = 2999,
	CKN_STR3999 = 3999,
	CKN_STR4096 = 4096,
	CKN_STR5500 = 5500,
	CKN_STR7999 = 7999,
	CKN_STR8192 = 8192,
	CKN_STR_MAX = 65535
};

#define CKN_STR_DEFAULT 1024	// Otherwise a whole bunch of nvram can't be changed by webui

enum {
	CKN_TYPE_DEFAULT = 0
};

enum {
	CKN_ACC_LEVEL_DEFAULT = 0
};

enum {
	CKN_ENC_DEFAULT = 0,
	CKN_ENC_SVR
};

#ifdef RTCONFIG_COOVACHILLI
extern void deauth_guest_sta(char *, char *);
#endif

#ifdef RTCONFIG_CFGSYNC
#define	CFGSYNC_GROUPID_LEN	CKN_STR32
#define CLIENT_STALIST_JSON_PATH	"/tmp/stalist.json"
extern int is_valid_group_id(const char *);
extern char *if_nametoalias(char *name, char *alias, int alias_len);
extern int check_re_in_macfilter(int unit, char *mac);
extern void update_macfilter_relist();
#endif

#if defined(RTCONFIG_DETWAN) && (defined(RTCONFIG_SOC_IPQ40XX))
extern void vlan_accept_vid_via_switch(int accept, int wan, int lan);
extern int detwan_set_def_vid(const char *ifname, int vid, int needTagged, int avoidVid);
#endif

extern int IPTV_ports_cnt(void);

#ifdef RTCONFIG_BCMWL6
#define WL_5G_BAND_1	1 << (1 - 1)
#define WL_5G_BAND_2	1 << (2 - 1)
#define WL_5G_BAND_3	1 << (3 - 1)
#define WL_5G_BAND_4	1 << (4 - 1)
#endif

enum {
	UI_SW_MODE_NONE=0,
	UI_SW_MODE_ROUTER,
	UI_SW_MODE_REPEATER,
	UI_SW_MODE_AP,
	UI_SW_MODE_MB,
	UI_SW_MODE_HOTSPOT,
	UI_SW_MODE_EXPRESS_2G,
	UI_SW_MODE_EXPRESS_5G
};

static inline int get_sw_mode(void)
{
	int wlc_psta = nvram_get_int("wlc_psta");
	int wlc_express = nvram_get_int("wlc_express");

	switch(sw_mode()){
		case 1:
			return UI_SW_MODE_ROUTER;
		case 2:
			if(wlc_psta==0){
				if(wlc_express==0)	/* BCM/RALINK/QCA/LANTIQ/REALTEK */
					return UI_SW_MODE_REPEATER;
				else if(wlc_express==1)	/* REALTEK RP-series */
					return UI_SW_MODE_EXPRESS_2G;
				else if(wlc_express==2)	/* REALTEK RP-series */
					return UI_SW_MODE_EXPRESS_5G;
			}else if(wlc_psta==1)	/* RALINK / QCA / LANTIQ */
				return UI_SW_MODE_MB;
			break;
		case 3:
			if(wlc_psta==0)	/* BCM/RALINK/QCA/LANTIQ/REALTEK */
				return UI_SW_MODE_AP;
			else if(wlc_psta==1)	/* BCM/REALTEK */
				return UI_SW_MODE_MB;
			else if(wlc_psta==2 || wlc_psta==3)	/* BCM */
				return UI_SW_MODE_REPEATER;
			break;
		case 4:
			return UI_SW_MODE_HOTSPOT;
	}

	return UI_SW_MODE_NONE;
}

/**
 * @ch:		channel
 * @return:
 * 	0:	@ch is not DFS channel or invalid parameter.
 * 	1:	@ch is DFS channel.
 */
static inline int is_dfs_channel(int ch)
{
	if (ch >= 52 && ch <= 144)
		return 1;
	return 0;
}

#if defined(RTCONFIG_SW_HW_AUTH)
// hw_auth.c
extern int HwCheckResult();
extern char *DoHardwareComponent(char *index);
#endif

#if defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS)
/*
	AMAS define type bitmap
*/
#define AMAS_CAP 0x0001
#define AMAS_RE  0x0002

/*
	API define and bitmap define in sw-hw-auth
	getAmasSupportMode() : global API for amas usage
	0 : not support AMAS
	1 : support CAP only
	2 : support RE only
	3 : CAP + RE
 */
extern int getAmasSupportMode();

#endif  /* defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS) */

#if defined(RTCONFIG_AMAS)

typedef enum __amaslib_action__t_
{
       AMASLIB_ACTION_NODE_MAC_UPDATE = 0,
       AMASLIB_ACTION_DEVICE_IP_QUERY
} AMASLIB_ACTION_T;
/*
       AMAS_LIB define for event and socket
*/
typedef struct __amaslib_notification__t_
{
       AMASLIB_ACTION_T action;
       char sta2g[18];     /* Node 2G MAC */
       char sta5g[18];     /* Node 5G MAC */
       int  flag;          /* Node status */
       char ip[16];        /* Device IP */
} AMASLIB_EVENT_T;

#define AMASLIB_PID_PATH           "/var/run/amas_lib.pid"
#define AMASLIB_SOCKET_PATH        "/var/run/amas_lib_socket"

#endif /* defined(RTCONFIG_AMAS) */


extern int get_discovery_ssid(char *ssid_g, int size);
extern int get_index_page(char *page, int size);
extern int get_chance_to_control(void);

#ifdef RTCONFIG_AMAS_WGN
extern int get_iptv_and_dualwan_info(int *iptv_vids,int size, unsigned int *wan_deny_list, unsigned int *lan_deny_list);
#endif	// RTCONFIG_AMAS_WGN

#ifdef RTCONFIG_GEFORCENOW
extern int wl_set_wifiscan(char *ifname, int val);
extern int wl_set_mcsindex(char *ifname, int *is_auto, int *idx, char *idx_type, int *stream);
#endif

#if defined(RTCONFIG_BCM_CLED)
enum {
	BCM_CLED_RED = 0,
	BCM_CLED_GREEN,
	BCM_CLED_BLUE,
	BCM_CLED_YELLOW,
	BCM_CLED_WHITE,
	BCM_CLED_OFF
};
enum {
	BCM_CLED_STEADY_NOBLINK = 0,
	BCM_CLED_STEADY_NOBLINK_DIM,
	BCM_CLED_STEADY_BLINK,
	BCM_CLED_PULSATING,
	BCM_CLED_SLOW_BLINK,
	BCM_CLED_MODE_END
};
#endif

extern int amazon_wss_ap_isolate_support(char *prefix);
extern void firmware_downgrade_check(uint32_t sf);

#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400)
#define LEDG_OFF			0
#define LEDG_STEADY_MODE		1
#define LEDG_FADING_REVERSE_MODE	2
#define LEDG_PULSATING_MODE		3
#define LEDG_PULSATING_WITH_DELAY_MODE	4
#define LEDG_RUNNING_LIGHT_MODE		5
#define LEDG_COLOR_CYCLE_MODE		6
#define LEDG_RAINBOW_MODE		7
#define LEDG_WATER_FLOW_MODE		8
#define LEDG_SCROLLING_MODE		9
#define LEDG_BLINKING_MODE		10
#define LEDG_MODE_MAX			11

#define LEDG_SCHEME_OFF			0
#define LEDG_SCHEME_GRADIENT		1
#define LEDG_SCHEME_STEADY_RED		2
#define LEDG_SCHEME_PULSATING		3
#define LEDG_SCHEME_COLOR_CYCLE		4
#define LEDG_SCHEME_RAINBOW		5
#define LEDG_SCHEME_WATER_FLOW		6
#define LEDG_SCHEME_SCROLLING		7
#define LEDG_SCHEME_CUSTOM		8
#define LEDG_SCHEME_BLINKING		9
#define LEDG_SCHEME_MAX			10

struct cled_config0 {
	uint32_t mode : 2;
	uint32_t reserved : 1;
	uint32_t flash_ctl : 3;
	uint32_t bright_ctl : 8;
	uint32_t repeat_cycle : 1;
	uint32_t bright_change_dir : 1;
	uint32_t phase_1_bright : 1;
	uint32_t phase_2_bright : 1;
	uint32_t reserved2 : 2;
	uint32_t initial_delay : 4;
	uint32_t final_delay : 4;
	uint32_t color_blend_ctrl : 4;
};

struct cled_config1 {
	uint32_t bstep1 : 4;
	uint32_t tstep1 : 4;
	uint32_t nstep1 : 4;
	uint32_t bstep2 : 4;
	uint32_t tstep2 : 4;
	uint32_t nstep2 : 4;
	uint32_t reserved : 8;
};

struct cled_config2 {
	uint32_t bstep3 : 4;
	uint32_t tstep3 : 4;
	uint32_t nstep3 : 4;
	uint32_t final_step: 4;
	uint32_t reserved : 16;
};

struct cled_config3 {
	uint32_t phase_delay1: 16;
	uint32_t phase_delay2: 16;
};
#endif

#endif	/* !__SHARED_H__ */
