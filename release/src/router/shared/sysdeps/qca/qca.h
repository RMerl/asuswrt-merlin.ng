/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _QCA_H_
#define _QCA_H_

#include <iwlib.h>

#include "rtconfig.h"

#define MAX_INI_PARM_NAME_LEN	128
#define MAX_INI_PARM_VAL_LEN	128
#define MAX_INI_PARM_LINE_LEN	(MAX_INI_PARM_NAME_LEN + 1 + MAX_INI_PARM_VAL_LEN + 1)
#define GLOBAL_INI_TOPDIR	"/etc/Wireless/ini"
#define GLOBAL_INI		GLOBAL_INI_TOPDIR "/global.ini"
#define GLOBAL_I_INI		GLOBAL_INI_TOPDIR "/internal/global_i.ini"
#define QCA8074_I_INI		GLOBAL_INI_TOPDIR "/internal/QCA8074_i.ini"
#define QCA8074V2_I_INI		GLOBAL_INI_TOPDIR "/internal/QCA8074V2_i.ini"
#if defined(RTCONFIG_SOC_IPQ60XX)
#define QCA6018_I_INI		GLOBAL_INI_TOPDIR "/internal/QCA6018_i.ini"
#endif
#define CNSS2_SYSFS_COLDBOOT	"/sys/module/cnss2/parameters/cold_boot_support"
#define CNSS2_SYSFS_DAEMON	"/sys/module/cnss2/parameters/daemon_support"

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
#define EXTRA_PBUF_CORE0_FN		"/proc/sys/dev/nss/n2hcfg/extra_pbuf_core0"
#define N2H_QUEUE_LIMIT_CORE0_FN	"/proc/sys/dev.nss/n2hcfg/n2h_queue_limit_core0"
#define N2H_QUEUE_LIMIT_CORE1_FN	"/proc/sys/dev.nss/n2hcfg/n2h_queue_limit_core1"
#else
#define EXTRA_PBUF_CORE0_FN		"/proc/sys/dev/nss/general/extra_pbuf_core0"
#endif
#define N2H_HIGH_WATER_CORE0_FN		"/proc/sys/dev/nss/n2hcfg/n2h_high_water_core0"
#define N2H_WIFI_POOL_BUF_FN		"/proc/sys/dev/nss/n2hcfg/n2h_wifi_pool_buf"

#define NAWDS_SH_FMT	"/etc/Wireless/sh/nawds_%s.sh"

#if defined(RTCONFIG_GLOBAL_INI)
#define WLFW_CAL_01_BIN			"/data/vendor/wifi/wlfw_cal_01.bin"	/* SPF10 or above */
#else
#define WLFW_CAL_01_BIN			"/data/misc/wifi/wlfw_cal_01.bin"
#endif

extern const char WIF_2G[];
extern const char WIF_5G[];
extern const char BR_GUEST[];
extern const char WIF_5G_BH[];
extern const char APMODE_BRGUEST_IP[];
extern const char WDSIF_5G[];
extern const char STA_2G[];
extern const char STA_5G[];
extern const char VPHY_2G[];
extern const char VPHY_5G[];
extern const char WSUP_DRV[];
extern const char WSUP_DRV_60G[];
extern const char WIF_5G2[];
extern const char STA_5G2[];
extern const char VPHY_5G2[];
extern const char WIF_60G[];
extern const char STA_60G[];
extern const char VPHY_60G[];

extern const char *max_2g_ax_mode;
extern const char *max_5g_ax_mode;
extern const char *max_2g_n_mode;
extern const char *max_5g_ac_mode;
extern const char *bw20[2], *bw40[2], *bw80[2];
extern const char *bw80_80_tbl[2], *bw160_tbl[2];

#define SSDK_DEV_ID	"/sys/ssdk/dev_id"
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
#define SWID_IPQ807X    "sw0"
#define SWID_QCA8337    "sw1"
#elif defined(RTCONFIG_SWITCH_IPQ50XX_QCA8337)
#define SWID_IPQ50XX    "sw0"
#define SWID_QCA8337    "sw1"
#endif

#define QWPA_CLI		"/usr/bin/wpa_cli"		/* wpa_cli, installed by qca-hostap */
#define QHOSTAPD_CTRL_IFACE	"/var/run/hostapd/global"	/* ctrl_interface of hostapd_athX.conf must be directory and can't use this directive. */
#define QHOSTAPD_PID_PATH	"/var/run/hostapd_global.pid"	/* for RTCONFIG_SINGLE_HOSTAPD */

#define URE	"apcli0"

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif
#define MAX_NUMBER_OF_MAC	64

#define MODE_CCK		0
#define MODE_OFDM		1
#define MODE_HTMIX		2
#define MODE_HTGREENFIELD	3
#define MODE_VHT		4

#define BW_20			0
#define BW_40			1
#define BW_BOTH			2
#define BW_80			2
#define BW_10			3

#define INIC_VLAN_ID_START	4 //first vlan id used for RT3352 iNIC MII
#define INIC_VLAN_IDX_START	2 //first available index to set vlan id and its group.

/* copied from _ieee80211.h of qca-wifi */
enum ieee80211_phymode {
    IEEE80211_MODE_AUTO             = 0,    /* autoselect */
    IEEE80211_MODE_11A              = 1,    /* 5GHz, OFDM */
    IEEE80211_MODE_11B              = 2,    /* 2GHz, CCK */
    IEEE80211_MODE_11G              = 3,    /* 2GHz, OFDM */
    IEEE80211_MODE_FH               = 4,    /* 2GHz, GFSK */
    IEEE80211_MODE_TURBO_A          = 5,    /* 5GHz, OFDM, 2x clock dynamic turbo */
    IEEE80211_MODE_TURBO_G          = 6,    /* 2GHz, OFDM, 2x clock dynamic turbo */
    IEEE80211_MODE_11NA_HT20        = 7,    /* 5Ghz, HT20 */
    IEEE80211_MODE_11NG_HT20        = 8,    /* 2Ghz, HT20 */
    IEEE80211_MODE_11NA_HT40PLUS    = 9,    /* 5Ghz, HT40 (ext ch +1) */
    IEEE80211_MODE_11NA_HT40MINUS   = 10,   /* 5Ghz, HT40 (ext ch -1) */
    IEEE80211_MODE_11NG_HT40PLUS    = 11,   /* 2Ghz, HT40 (ext ch +1) */
    IEEE80211_MODE_11NG_HT40MINUS   = 12,   /* 2Ghz, HT40 (ext ch -1) */
    IEEE80211_MODE_11NG_HT40        = 13,   /* 2Ghz, Auto HT40 */
    IEEE80211_MODE_11NA_HT40        = 14,   /* 5Ghz, Auto HT40 */
    IEEE80211_MODE_11AC_VHT20       = 15,   /* 5Ghz, VHT20 */
    IEEE80211_MODE_11AC_VHT40PLUS   = 16,   /* 5Ghz, VHT40 (Ext ch +1) */
    IEEE80211_MODE_11AC_VHT40MINUS  = 17,   /* 5Ghz  VHT40 (Ext ch -1) */
    IEEE80211_MODE_11AC_VHT40       = 18,   /* 5Ghz, VHT40 */
    IEEE80211_MODE_11AC_VHT80       = 19,   /* 5Ghz, VHT80 */
    IEEE80211_MODE_11AC_VHT160      = 20,   /* 5Ghz, VHT160 */
    IEEE80211_MODE_11AC_VHT80_80    = 21,   /* 5Ghz, VHT80_80 */
    IEEE80211_MODE_11AXA_HE20       = 22,   /* 5GHz, HE20 */
    IEEE80211_MODE_11AXG_HE20       = 23,   /* 2GHz, HE20 */
    IEEE80211_MODE_11AXA_HE40PLUS   = 24,   /* 5GHz, HE40 (ext ch +1) */
    IEEE80211_MODE_11AXA_HE40MINUS  = 25,   /* 5GHz, HE40 (ext ch -1) */
    IEEE80211_MODE_11AXG_HE40PLUS   = 26,   /* 2GHz, HE40 (ext ch +1) */
    IEEE80211_MODE_11AXG_HE40MINUS  = 27,   /* 2GHz, HE40 (ext ch -1) */
    IEEE80211_MODE_11AXA_HE40       = 28,   /* 5GHz, HE40 */
    IEEE80211_MODE_11AXG_HE40       = 29,   /* 2GHz, HE40 */
    IEEE80211_MODE_11AXA_HE80       = 30,   /* 5GHz, HE80 */
    IEEE80211_MODE_11AXA_HE160      = 31,   /* 5GHz, HE160 */
    IEEE80211_MODE_11AXA_HE80_80    = 32,   /* 5GHz, HE80_80 */

    IEEE80211_MODE_MAX
};

typedef struct _WLANCONFIG_LIST {
	char addr[18];
	unsigned int aid;
	unsigned int chan;
	char txrate[10];
	char rxrate[10];
	int rssi;
	char conn_time[12];
	char mode[31];
	char subunit_id;	/* '0': main 2G/5G network, '1' ~ '7': Guest network (MAX_NO_MSSID = 8), 'B': Facebook Wi-Fi, 'F': Free Wi-Fi, 'C': Captive Portal */
} WLANCONFIG_LIST;

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    (defined(RTCONFIG_WIFI_QCN5024_QCN5054) && !defined(RTCONFIG_SOC_IPQ60XX))
#define MAX_STA_NUM 512
#else
#define MAX_STA_NUM 256
#endif

typedef struct _WIFI_STA_TABLE {
	int Num;
	WLANCONFIG_LIST Entry[ MAX_STA_NUM ];
} WIFI_STA_TABLE;

// MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!!
typedef union  _MACHTTRANSMIT_SETTING {
	struct  {
	unsigned short	MCS:7;	// MCS
	unsigned short	BW:1;	//channel bandwidth 20MHz or 40 MHz
	unsigned short	ShortGI:1;
	unsigned short	STBC:2;	//SPACE
	unsigned short  eTxBF:1;
	unsigned short  rsv:1;
	unsigned short  iTxBF:1;
	unsigned short  MODE:2;	// Use definition MODE_xxx.
	} field;
	unsigned short	word;
 } MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

// MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!!
typedef union  _MACHTTRANSMIT_SETTING_2G {
	struct  {
	unsigned short	MCS:7;	// MCS
	unsigned short	BW:1;	//channel bandwidth 20MHz or 40 MHz
	unsigned short	ShortGI:1;
	unsigned short	STBC:2;	//SPACE
	unsigned short	rsv:3;
	unsigned short	MODE:2;	// Use definition MODE_xxx.
	} field;
	unsigned short	word;
 } MACHTTRANSMIT_SETTING_2G, *PMACHTTRANSMIT_SETTING_2G;

typedef union  _MACHTTRANSMIT_SETTING_11AC {
	struct  {
	unsigned short	MCS:7;	// MCS
	unsigned short	BW:2;	//channel bandwidth 20MHz or 40 MHz
	unsigned short	ShortGI:1;
	unsigned short	STBC:1;	//SPACE
	unsigned short	eTxBF:1;
	unsigned short	iTxBF:1;
	unsigned short	MODE:3;	// Use definition MODE_xxx.
	} field;
	unsigned short	word;
 } MACHTTRANSMIT_SETTING_11AC, *PMACHTTRANSMIT_SETTING_11AC;

typedef struct _RT_802_11_MAC_ENTRY_RT3352_iNIC {
    unsigned char	ApIdx;
    unsigned char	Addr[ETHER_ADDR_LEN];
    unsigned char	Aid;
    unsigned char	Psm;	// 0:PWR_ACTIVE, 1:PWR_SAVE
    unsigned char	MimoPs;	// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
    char		AvgRssi0;
    char		AvgRssi1;
    char		AvgRssi2;
    unsigned int	ConnectedTime;
    MACHTTRANSMIT_SETTING_2G	TxRate;
    MACHTTRANSMIT_SETTING_2G	MaxTxRate;
} RT_802_11_MAC_ENTRY_RT3352_iNIC, *PRT_802_11_MAC_ENTRY_RT3352_iNIC;

typedef struct _RT_802_11_MAC_ENTRY_RT3883 {
    unsigned char	ApIdx;
    unsigned char	Addr[ETHER_ADDR_LEN];
    unsigned char	Aid;
    unsigned char	Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
    unsigned char	MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
    char		AvgRssi0;
    char		AvgRssi1;
    char		AvgRssi2;
    unsigned int	ConnectedTime;
    MACHTTRANSMIT_SETTING_2G       TxRate;
    unsigned int	LastRxRate;
    short		StreamSnr[3];
    short		SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY_RT3883, *PRT_802_11_MAC_ENTRY_RT3883;

typedef struct _RT_802_11_MAC_ENTRY {
    unsigned char	ApIdx;
    unsigned char	Addr[ETHER_ADDR_LEN];
    unsigned char	Aid;
    unsigned char	Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
    unsigned char	MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
    char		AvgRssi0;
    char		AvgRssi1;
    char		AvgRssi2;
    unsigned int	ConnectedTime;
    MACHTTRANSMIT_SETTING       TxRate;
    unsigned int	LastRxRate;
    int			StreamSnr[3];
    int			SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_ENTRY_2G {
    unsigned char	ApIdx;
    unsigned char	Addr[ETHER_ADDR_LEN];
    unsigned char	Aid;
    unsigned char	Psm;	/* 0:PWR_ACTIVE, 1:PWR_SAVE */
    unsigned char	MimoPs;	/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
    char		AvgRssi0;
    char		AvgRssi1;
    char		AvgRssi2;
    unsigned int	ConnectedTime;
    MACHTTRANSMIT_SETTING_2G	TxRate;
    unsigned int	LastRxRate;
    short		StreamSnr[3];	/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
    short		SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY_2G, *PRT_802_11_MAC_ENTRY_2G;

typedef struct _RT_802_11_MAC_ENTRY_11AC {
    unsigned char	ApIdx;
    unsigned char	Addr[ETHER_ADDR_LEN];
    unsigned char	Aid;
    unsigned char	Psm;	/* 0:PWR_ACTIVE, 1:PWR_SAVE */
    unsigned char	MimoPs;	/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
    char		AvgRssi0;
    char		AvgRssi1;
    char		AvgRssi2;
    unsigned int	ConnectedTime;
    MACHTTRANSMIT_SETTING_11AC	TxRate;
    unsigned int	LastRxRate;
    short		StreamSnr[3];	/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
    short		SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY_11AC, *PRT_802_11_MAC_ENTRY_11AC;

typedef struct _RT_802_11_MAC_TABLE {
    unsigned long	Num;
    RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

typedef struct _RT_802_11_MAC_TABLE_2G {
    unsigned long	Num;
    RT_802_11_MAC_ENTRY_2G Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE_2G, *PRT_802_11_MAC_TABLE_2G;

typedef struct _SITE_SURVEY_RT3352_iNIC
{
	char channel[4];
	unsigned char ssid[33];
	char bssid[20];
	char authmode[15];	//security part1
	char encryption[8];	//security part2 and need to shift data
	char signal[9];
	char wmode[8];
	char extch[7];
	char nt[3];
	char wps[4];
	char dpid[4];
	char newline;
} SITE_SURVEY_RT3352_iNIC;

typedef struct _SITE_SURVEY 
{ 
	char channel[4];
	unsigned char ssid[33]; 
	char bssid[18];
	char encryption[9];
	char authmode[16];
	char signal[9];
	char wmode[8];
} SITE_SURVEY;

typedef struct _SITE_SURVEY_ARRAY
{ 
	SITE_SURVEY SiteSurvey[64];
} SSA;

#define SITE_SURVEY_APS_MAX	(16*1024)

//#if WIRELESS_EXT <= 11 
//#ifndef SIOCDEVPRIVATE 
//#define SIOCDEVPRIVATE 0x8BE0 
//#endif 
//#define SIOCIWFIRSTPRIV SIOCDEVPRIVATE 
//#endif 
//
//SET/GET CONVENTION :
// * ------------------
// * Simplistic summary :
// * o even numbered ioctls are SET, restricted to root, and should not
// * return arguments (get_args = 0).
// * o odd numbered ioctls are GET, authorised to anybody, and should
// * not expect any arguments (set_args = 0).
//
#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET		(SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_GSITESURVEY	(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_GET_MAC_TABLE	(SIOCIWFIRSTPRIV + 0x0F) //used by rt2860v2
#define	RTPRIV_IOCTL_SHOW		(SIOCIWFIRSTPRIV + 0x11)
#define RTPRIV_IOCTL_WSC_PROFILE	(SIOCIWFIRSTPRIV + 0x12)
#define RTPRIV_IOCTL_SWITCH		(SIOCIWFIRSTPRIV + 0x1D) //used by iNIC_RT3352 on RTN65U
#define RTPRIV_IOCTL_ASUSCMD		(SIOCIWFIRSTPRIV + 0x1E)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)	/* RT3352 iNIC driver doesn't support this ioctl. */
#define OID_802_11_DISASSOCIATE		0x0114
#define OID_802_11_BSSID_LIST_SCAN	0x0508
#define OID_802_11_SSID			0x0509
#define OID_802_11_BSSID		0x050A
#define RT_OID_802_11_RADIO		0x050B
#define OID_802_11_BSSID_LIST		0x0609
#define OID_802_3_CURRENT_ADDRESS	0x060A
#define OID_GEN_MEDIA_CONNECT_STATUS	0x060B
#define RT_OID_GET_PHY_MODE		0x0761
#define OID_GET_SET_TOGGLE		0x8000
#define RT_OID_SYNC_RT61		0x0D010750
#define RT_OID_WSC_QUERY_STATUS		((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE		((RT_OID_SYNC_RT61 + 0x02) & 0xffff)

enum ASUS_IOCTL_SUBCMD {
	ASUS_SUBCMD_UNKNOWN = 0,
	ASUS_SUBCMD_GSTAINFO,
	ASUS_SUBCMD_GSTAT,
	ASUS_SUBCMD_GRSSI,
	ASUS_SUBCMD_RADIO_STATUS,
	ASUS_SUBCMD_CHLIST,
	ASUS_SUBCMD_GROAM,
	ASUS_SUBCMD_CONN_STATUS,
	ASUS_SUBCMD_MAX
};

/* Below offset addresses, actually, based on start address of Flash.
 * Thus, there are absolute addresses and only valid on products
 * associated with parallel NOR Flash and SPI Flash.
 */

#if defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
#define ETH0_MAC_OFFSET			0x1002
#define ETH1_MAC_OFFSET			0x5006

#elif defined(RTCONFIG_SOC_IPQ8064)

#if defined(BRTAC828) || defined(RTAD7200) || defined(RTAC88S)
#define ETH0_MAC_OFFSET			0x1006	/* 2G EEPROM */
#define ETH1_MAC_OFFSET			0x5006	/* 5G EEPROM */
#elif defined(RTAC88N)
#define ETH0_MAC_OFFSET			0x5006	/* 2G EEPROM */
#define ETH1_MAC_OFFSET			0x1006	/* 5G EEPROM */
#else
#error
#endif

#elif defined(RTCONFIG_SOC_IPQ40XX)
#define ETH0_MAC_OFFSET			0x1006	/* 2G EEPROM */
#if defined(RTAC82U)
#define ETH1_MAC_OFFSET			0x9006	/* 5G EEPROM */
#else
#define ETH1_MAC_OFFSET			0x5006	/* 5G EEPROM */
#define ETH2_MAC_OFFSET			0x9006	/* 5G2 EEPROM */
#endif

#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054)
#define ETH0_MAC_OFFSET			0x1014	/* 2G+5G EEPROM, second set of MAC address. */
#define ETH1_MAC_OFFSET			0x100E	/* 2G+5G EEPROM, first set of MAC address. */

#elif defined(RTCONFIG_SOC_IPQ60XX)
#define ETH0_MAC_OFFSET			0x1014	/* 2G+5G EEPROM, second set of MAC address. */
#define ETH1_MAC_OFFSET			0x100E	/* 2G+5G EEPROM, first set of MAC address. */

#elif defined(RTCONFIG_SOC_IPQ50XX)
#define ETH0_MAC_OFFSET			0x100E			/* 2G   EEPROM, first set of MAC address. */
#if defined(ETJ)
#define ETH2_MAC_OFFSET			(0x25800+0x100E)	/* 6G EEPROM, first set of MAC address. */
#define ETH1_MAC_OFFSET			(0x4B000+0x100E)	/* 5G EEPROM, first set of MAC address. */
#else
#define ETH1_MAC_OFFSET			(0x25800+0x100E)	/* 5G-1 EEPROM, first set of MAC address. */
#define ETH2_MAC_OFFSET			(0x4B000+0x100E)	/* 2G-2 EEPROM, first set of MAC address. */
#endif

#else
#error Define MAC address offset.
#endif

#define MTD_FACTORY_BASE_ADDRESS	0x40000
#define OFFSET_MTD_FACTORY		(MTD_FACTORY_BASE_ADDRESS + 0x00000)

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
/* EEPROM size of QCN50x4 is up to 128KB now, may group up to 256KB,
 * Shifts OFFSET_XXX at lease 0x40000 bytes for QCN50x4 Wi-Fi platform.
 * Because start offset of EEPROM is 0x1000.  We mustn't define new
 * ASUSWRT parameters in 0x40000 ~ (0x40000 + 0x1000 - 1) on QCN50X4
 * Wi-Fi  platform.
 */
#if defined(RTCONFIG_SOC_IPQ60XX)
#define FTRY_PARM_SHIFT			(0xF000)
#else /* IPQ70xx */
#define FTRY_PARM_SHIFT			(0x40000)
#endif // end of RTCONFIG_SOC_IPQ60XX

#elif defined(RTCONFIG_SOC_IPQ60XX)
#define FTRY_PARM_SHIFT			(0xF000)

#elif defined(RTCONFIG_SOC_IPQ50XX)
#define FTRY_PARM_SHIFT			(0x5F800)  /* (150KB * 3) - (16KB + 0xD000) */

#else /* Non QCN50XX, Legacy */
#define FTRY_PARM_SHIFT			(0)
#endif

#define OFFSET_RFCA_COPIED		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D00A)	/* 4 bytes */
#define OFFSET_FORCE_USB3		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D010)	/* 1 bytes */
#define OFFSET_BOOT_VER			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D18A)	/* 0x4018A -> 0x4D18A */
#define OFFSET_COUNTRY_CODE		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D188)	/* 0x40188 -> 0x4D188 */
#define	FACTORY_COUNTRY_CODE_LEN	2
#define OFFSET_RTAG			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D19C)	/* 0x40188 -> 0x4D19C, len 4 */
#if defined(RTAC58U)
#define OFFSET_RTAG2			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1AC)	/* 4 bytes */
#endif

#if defined(RTCONFIG_WIFI_QCA9557_QCA9882) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
/* WAN: eth0
 * LAN: eth1
 * 2G: follow WAN
 * 5G: follow LAN
 */
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)	/* FIXME: How to map 2G/5G to eth0/1? */
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)	/* FIXME: How to map 2G/5G to eth0/1? */
#define	QCA9557_EEPROM_SIZE		1088
#define	QCA9557_EEPROM_MAC_OFFSET	(OFFSET_MAC_ADDR_2G & 0xFFF) // 2
#if defined(RTCONFIG_PCIE_QCA9888)
#define	QC98XX_EEPROM_SIZE_LARGEST	12064 // sync with driver
#else /* RTCONFIG_PCIE_AR9888 */
#define	QC98XX_EEPROM_SIZE_LARGEST	2116 // sync with driver
#endif
#define	QC98XX_EEPROM_MAC_OFFSET	(OFFSET_MAC_ADDR & 0xFFF) // 6
#elif defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
      defined(RTCONFIG_WIFI_QCA9994_QCA9994)

/* WAN: eth0	(FIXME)
 * LAN: eth1	(FIXME)
 * 2G: follow WAN
 * 5G: follow LAN
 */
#if defined(BRTAC828) || defined(RTAD7200) || defined(RTAC88S)
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)
#else
#error
#endif

#define	QC98XX_EEPROM_SIZE_LARGEST	12064 // sync with driver
#define	QC98XX_EEPROM_MAC_OFFSET	(OFFSET_MAC_ADDR & 0xFFF) // 6

#elif defined(RTCONFIG_SOC_IPQ40XX)
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)
#define OFFSET_MAC_ADDR_5G_2		(MTD_FACTORY_BASE_ADDRESS + ETH2_MAC_OFFSET)
#define	QC98XX_EEPROM_SIZE_LARGEST	12064 // sync with driver
#define	QC98XX_EEPROM_MAC_OFFSET	(OFFSET_MAC_ADDR & 0xFFF) // 6

#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054)
/* Because 2G and 5G share same EEPROM, only one MAC address is defined in EEPROM,
 * and ath0 of GT-AXY16000 is 5G.  Save 2G MAC address at GMAC0 MAC address space, out
 * of EEPROM, and assume MAC address in EEPROM as 5G MAC address.  2G MAC address
 * is equal to 5G MAC address + 01:00 without carry to left byte.
 */
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)	/* GMAC0 MAC address, out of EEPROM */
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)	/* MAC address in 2G+5G EEPROM */
#if defined(RTCONFIG_SOC_IPQ60XX)
#define	QCN50X4_EEPROM_SIZE		65536
#else // @RTCONFIG_SOC_IPQ60XX
#define	QCN50X4_EEPROM_SIZE		131072
#endif

#elif defined(RTCONFIG_SOC_IPQ60XX)
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)	/* GMAC0 MAC address, out of EEPROM */
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)	/* MAC address in 2G+5G EEPROM */
#define	QCN50X4_EEPROM_SIZE		65536

#elif defined(RTCONFIG_SOC_IPQ50XX)
#define OFFSET_MAC_ADDR_2G		(MTD_FACTORY_BASE_ADDRESS + ETH0_MAC_OFFSET)	/* GMAC0 MAC address, out of EEPROM */
#define OFFSET_MAC_ADDR			(MTD_FACTORY_BASE_ADDRESS + ETH1_MAC_OFFSET)	/* MAC address in 2G+5G EEPROM */
#define OFFSET_MAC_ADDR_5G_2		(MTD_FACTORY_BASE_ADDRESS + ETH2_MAC_OFFSET)
#define	QCN90XX_EEPROM_SIZE		131072

#else
#error Define EEPROM offset and size
#endif


/*
 * Define ASUSWRT specific parameters.
 */
#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	(0x10000 + FTRY_PARM_SHIFT)

#define OFFSET_PIN_CODE			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D180)	/*  8 bytes */
/*
 * PIB parameters of Powerline Communication (PLC)
 */
#ifdef RTCONFIG_QCA_PLC_UTILS
#define OFFSET_PLC_MAC			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D18E)	// 6
#define OFFSET_PLC_NMK			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D194)	// 16
#endif
/*
 * disable DHCP client and DHCP override during ATE
 */
#ifdef RTCONFIG_DEFAULT_AP_MODE
//#define OFFSET_FORCE_DISABLE_DHCP	(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1AA)	// 1
#define OFFSET_FORCE_DISABLE_DHCP	(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1AB)	// 1
#endif

#ifdef RTCONFIG_AMAS
#define OFFSET_AMAS_BUNDLE_FLAG		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1AB)	// 1
#define OFFSET_DEF_GROUPID		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1B0)  // reserved for CFGSYNC_GROUPID_LEN (32 bytes)
#define OFFSET_AMAS_BUNDLE_KEY		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1D0)	// 32 bytes, same as groupkey
#endif

#if defined(RTCONFIG_WIFI_DRV_DISABLE) || defined(MAPAC2200V) /* for IPQ40XX */
#define OFFSET_DISABLE_WIFI_DRV		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1F2)	// 1 byte
#endif

#if defined(RTCONFIG_CSR8811)
#define OFFSET_CSR8811_MAC		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1F3)	// 6 byte
#define OFFSET_CSR8811_CAL		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D1F9)	// 1 byte
#endif

#if defined(MAPAC2200V)
#define OFFSET_SPEAKER_VER		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D208)	// 1 byte
#define OFFSET_CERT_PEM			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0A200)    // 4096 byte
#define OFFSET_CERT_KEY			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0B200)	// 4096 byte
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
#define OFFSET_VOLTUP			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D300)	// 8 byte, uint64_t, little-endian
#define OFFSET_L2CEILING		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D308)	// 4 byte, uint32_t, little-endian
#define OFFSET_PWRCYCLECNT		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D30C)	// 4 byte, uint32_t, little-endian
#define OFFSET_AVGUPTIME		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0D310)	// 4 byte, uint32_t, little-endian
#endif

#define MAX_PASS_ENC_LEN 64
#define OFFSET_PASS_ENC			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FE60)  /*  64 bytes (MAX_PASS_ENC_LEN) */
#define OFFSET_EISN			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FEA0)  /*  32 bytes */
#if defined(RTCONFIG_ASUSCTRL)
#define OFFSET_ASUSCTRL_FLAGS		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FEC0)  /*  8 bytes */
#define ASUSCTRL_FLAGS_LENGTH		(8)
#define OFFSET_ASUSCTRL_CHG_SKU		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FEC8)  /*  2 bytes */
#define ASUSCTRL_CHG_SKU_LENGTH		(2)
#endif
#define OFFSET_SERIAL_NUMBER32		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FED0)  /*  32 bytes */
#define OFFSET_IPADDR_LAN               (MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FEF0)

#define OFFSET_HWID			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF00)  /*  4 bytes */
#define HWID_LENGTH			(4)
#define OFFSET_HWVERSION		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF10)  /*  8 bytes */
#define HWVERSION_LENGTH		(8)
#define OFFSET_DATECODE			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF18)  /*  8 bytes */
#define DATECODE_LENGTH			(8)
#define OFFSET_HWBOM			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF20)  /* 32 bytes */
#define HWBOM_LENGTH			(32)

#define OFFSET_PSK			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF60)  /* 15 bytes */
#ifdef RTCONFIG_32BYTES_ODMPID
#define OFFSET_32BYTES_ODMPID		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF70)	/* 32 bytes */
#endif
#define OFFSET_TERRITORY_CODE		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FF90)	/*  5 bytes, e.g., US/01, US/02, TW/01, etc. */
#define OFFSET_DEV_FLAGS		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFA0)	//device dependent flags
#define OFFSET_ODMPID			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFB0)	//the shown model name (for Bestbuy and others)
#define OFFSET_FAIL_RET			(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFC0)
#define OFFSET_FAIL_BOOT_LOG		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFD0)	//bit operation for max 100
#define OFFSET_FAIL_DEV_LOG		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFE0)	//bit operation for max 100
#define OFFSET_SERIAL_NUMBER		(MTD_FACTORY_BASE_ADDRESS + FTRY_PARM_SHIFT + 0x0FFF0)  /*  16 bytes */

#define PASS_OFFSET	OFFSET_PASS_ENC
#define PASS_LEN	MAX_PASS_ENC_LEN

/*
 * EEPROM definitions of each band.
 */
#if defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
#define QCA_5G_EEPROM_CSUM_OFFSET	(2)
#define QCA_5G_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)

#elif defined(RTCONFIG_SOC_IPQ8064)

#define QCA_2G_EEPROM_CSUM_OFFSET	(2)
#define QCA_5G_EEPROM_CSUM_OFFSET	(2)
#define QCA_2G_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)
#define QCA_5G_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)

#elif defined(RTCONFIG_SOC_IPQ40XX)

#define QCA_2G_EEPROM_CSUM_OFFSET	(0x2)
#define QCA_5G_EEPROM_CSUM_OFFSET	(0x2)
#define QCA_5G2_EEPROM_CSUM_OFFSET	(0x2)
#define QCA_2G_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)
#define QCA_5G_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)
#define QCA_5G2_EEPROM_SIZE		(QC98XX_EEPROM_SIZE_LARGEST)

#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054)

#define QCA_2G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_5G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_2G_EEPROM_SIZE		(QCN50X4_EEPROM_SIZE)
#define QCA_5G_EEPROM_SIZE		(QCN50X4_EEPROM_SIZE)

#elif defined(RTCONFIG_SOC_IPQ60XX)
#define QCA_2G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_5G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_2G_EEPROM_SIZE		(QCN50X4_EEPROM_SIZE)
#define QCA_5G_EEPROM_SIZE		(QCN50X4_EEPROM_SIZE)

#elif defined(RTCONFIG_SOC_IPQ50XX)
#define QCA_2G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_5G_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_5G2_EEPROM_CSUM_OFFSET	(0xa)
#define QCA_2G_EEPROM_SIZE		(QCN90XX_EEPROM_SIZE)
#define QCA_5G_EEPROM_SIZE		(QCN90XX_EEPROM_SIZE)
#define QCA_5G2_EEPROM_SIZE		(QCN90XX_EEPROM_SIZE)

#endif

/*
 * LED/Button GPIO# definitions
 */
//#define QCA_LED_ON		0	// low active (all 5xx series)
//#define QCA_LED_OFF		1

/* RT-AC55U */
#define	QCA_BTN_RESET		17
#define	QCA_BTN_WPS		16
#define	QCA_LED_POWER		19
#define	QCA_LED_USB		 4	/* high active */
#define	QCA_LED_USB3		 0	/* FIXME: Needs xhci driver */
#define	QCA_LED_WAN		15	/* high active, blue */
#define	QCA_LED_WAN_RED		14	/* high active, red */
#define	QCA_LED_LAN		18

#define GPIO_DIR_OUT		1
#define GPIO_DIR_IN		0

/*
 * interface of CPU to LAN
 */
#if defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
#if defined(RTN19)
#define MII_IFNAME	"eth1"
#else
#define MII_IFNAME	"eth0"
#endif
#elif defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
#define MII_IFNAME	"switch0"
#elif defined(RTCONFIG_QCA953X) || defined(RTCONFIG_SOC_IPQ40XX)
#define MII_IFNAME	"eth1"
#elif defined(RTCONFIG_SOC_IPQ60XX)
#define MII_IFNAME	"eth0"
#elif defined(RTCONFIG_SOC_IPQ50XX)
#define MII_IFNAME	"eth1"	/* SGMII+ */
#else
#error Define MII_IFNAME interface!
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
#define UNSQUASHFS	"/usr/sbin/unsquashfs"
#define SQUASHFS_ROOT	"/tmp/.squashfs-root"
#endif	/* RTCONFIG_SOC_IPQ8074 */

unsigned long task_mask;

extern int switch_init(void);
extern void switch_fini(void);
extern int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq);
extern int verify_qca_eeprom_csum(void *eeprom, unsigned int eeprom_length);
extern int calc_qca_eeprom_csum(void *ptr, unsigned int eeprom_size, unsigned int eeprom_csum_offset);
#if defined(RTAC58U)
extern int check_mid(char *mid);
#endif
/* for ATE Get_WanLanStatus command */
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
#define MAX_NR_SWITCH_PORTS	8
#elif defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
#define MAX_NR_SWITCH_PORTS	11
#else
#define MAX_NR_SWITCH_PORTS	5
#endif
typedef struct {
	unsigned int link[MAX_NR_SWITCH_PORTS];
	unsigned int speed[MAX_NR_SWITCH_PORTS];
} phyState;

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#define BD_2G_PREFIX	"boardData_QCA9984_CUS260_2G_"
#define BD_5G_PREFIX	"boardData_QCA9984_CUS239_5G_"
#define BD_2G_CHIP_DIR	"QCA9984"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"QCA9984"
#define BD_5G_HW_DIR	"hw.1"
#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054)
/* 2G and 5G share same boarddata. */
#define BD_2G_PREFIX	"bdwlan"
#if defined(RTCONFIG_SOC_IPQ60XX) // SPF11.0 depends on QCN5024_QCN5054
#define BD_2G_CHIP_DIR	"IPQ6018"
#else // @RTCONFIG_SOC_IPQ60XX
#define BD_2G_CHIP_DIR	"IPQ8074"
#endif
#define BD_2G_HW_DIR	"."
#define BD_5G_PREFIX	BD_2G_PREFIX
#define BD_5G_CHIP_DIR	BD_2G_CHIP_DIR
#define BD_5G_HW_DIR	BD_2G_HW_DIR
#elif defined(RTCONFIG_SOC_IPQ60XX)  // SPF11.4 separate from QCN5024_QCN5054
#define BD_2G_PREFIX	"bdwlan"
#define BD_2G_CHIP_DIR	"IPQ6018"
#define BD_2G_HW_DIR	"."
#define BD_5G_PREFIX	BD_2G_PREFIX
#define BD_5G_CHIP_DIR	BD_2G_CHIP_DIR
#define BD_5G_HW_DIR	BD_2G_HW_DIR
#elif defined(RTCONFIG_SOC_IPQ50XX)
#define BD_2G_PREFIX	"bdwlan"
#define BD_5G_PREFIX	"bdwlan"
#define BD_5G2_PREFIX	"bdwlan"
#define BD_2G_CHIP_DIR	"IPQ5018"
#define BD_2G_HW_DIR	"."
#define BD_5G_CHIP_DIR	"qcn9000"
#define BD_5G_HW_DIR	"."
#define BD_5G2_CHIP_DIR	"qcn9100"
#define BD_5G2_HW_DIR	"."
#elif defined(RTAC58U) || defined(VZWAC1300) || defined(SHAC1300) || defined(RT4GAC53U)
#define BD_2G_PREFIX	"boardData_1_0_IPQ4019_Y9803_wifi0"
#define BD_5G_PREFIX	"boardData_1_0_IPQ4019_Y9803_wifi1"
#define BD_2G_CHIP_DIR	"IPQ4019"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"IPQ4019"
#define BD_5G_HW_DIR	"hw.1"
#elif defined(RTAC82U)
#define BD_2G_PREFIX	"boardData_1_0_IPQ4019_DK04_2G"
#define BD_5G_PREFIX	"boardData_QCA9984_CUS238_5G_v1_003"
#define BD_2G_CHIP_DIR	"IPQ4019"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"QCA9984"
#define BD_5G_HW_DIR	"hw.1"
#elif defined(MAPAC1300)
#define BD_2G_PREFIX	"boardData_1_0_IPQ4019_YA131_wifi0"
#define BD_5G_PREFIX	"boardData_1_0_IPQ4019_YA131_wifi1"
#define BD_2G_CHIP_DIR	"IPQ4019"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"IPQ4019"
#define BD_5G_HW_DIR	"hw.1"
#elif defined(MAPAC2200)
#define BD_2G_PREFIX	"boardData_1_0_IPQ4019_DK04_2G"
#define BD_5G_PREFIX	"boardData_1_0_IPQ4019_DK04_5G"
#define BD_5G2_PREFIX	"boardData_2_0_QCA9888_5G_Y9484"
#define BD_2G_CHIP_DIR	"IPQ4019"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"IPQ4019"
#define BD_5G_HW_DIR	"hw.1"
#define BD_5G2_CHIP_DIR	"QCA9888"
#define BD_5G2_HW_DIR	"hw.2"
#elif defined(RTAC95U)
#define BD_2G_PREFIX	"boardData_1_0_IPQ4019_DK04_2G"
#define BD_5G_PREFIX	"boardData_1_0_IPQ4019_DK04_5G"
#define BD_5G2_PREFIX	"boardData_QCA9984_CUS239_high_band_5G_v1_006"
#define BD_2G_CHIP_DIR	"IPQ4019"
#define BD_2G_HW_DIR	"hw.1"
#define BD_5G_CHIP_DIR	"IPQ4019"
#define BD_5G_HW_DIR	"hw.1"
#define BD_5G2_CHIP_DIR	"QCA9984"
#define BD_5G2_HW_DIR	"hw.1"
#elif defined(RTAC59U)
#define BD_5G_PREFIX	"boardData_2_0_QCA9888_5G_Y9690"
#define BD_5G_CHIP_DIR	"QCA9888"
#define BD_5G_HW_DIR	"hw.2"
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N)
#define BD_5G_PREFIX	"boardData_2_0_QCA9888_5G_YA105"
#define BD_5G_CHIP_DIR	"QCA9888"
#define BD_5G_HW_DIR	"hw.2"
#elif defined(RPAC51)
#define BD_5G_PREFIX	"boardData_2_0_QCA9888_5G_Y9484"
#define BD_5G_CHIP_DIR	"QCA9888"
#define BD_5G_HW_DIR	"hw.2"
#endif

#define	QCA_MACCMD	"maccmd"
#define	QCA_GETCMD	"get_maccmd"
#define	QCA_ADDMAC	"addmac"
#define	QCA_DELMAC	"delmac"
#define	QCA_GETMAC	"getmac"

#if defined(RTCONFIG_LYRA_5G_SWAP)
extern int swap_5g_band(int band);
#else
#define swap_5g_band(b) (b)
#endif

/* qca.c */
#if defined(RTCONFIG_SOC_IPQ8074)
extern unsigned char get_soc_version_major(void);
#else
static inline unsigned char get_soc_version_major(void) { return 0; }
#endif
extern int get_parameter_from_ini_file(const char *param_name, char *param_val, size_t param_val_size, const char *ini_fn);
extern int get_board_or_default_parameter_from_ini_file(const char *board_name, const char *param_name, char *param_val, size_t param_val_size, const char *ini_fn);
extern int get_integer_parameter_from_ini_file(const char *param_name, int *param_val, const char *ini_fn);
extern int get_channf(int band, const char *ifname);
extern int __get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, int (*handler)(const WLANCONFIG_LIST *rptr, void *arg), void *arg);
extern int get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, WIFI_STA_TABLE *sta_info);
#if defined(RTCONFIG_AMAS_WGN)
extern char* get_all_lan_ifnames(void);
extern int check_vlan_invalid(char *word,char *iface);
#endif

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994) \
 || defined(RTCONFIG_WIFI_QCN5024_QCN5054)
extern int nss_wifi_offloading(void);
#else
static inline int nss_wifi_offloading(void) { return 0; }
#endif

#endif	/* _QCA_H_ */
