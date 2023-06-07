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
#ifndef RTXXXXH
#define RTXXXXH

#include <iwlib.h>
#include <rtconfig.h>


extern const char WIF_2G[];
extern const char WIF_5G[];
extern const char WDSIF_5G[];
extern const char APCLI_5G[];
extern const char APCLI_2G[];
#define URE	"apcli0"

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif
#if defined(RTCONFIG_MT798X) // 288 - 544
#define MAX_NUMBER_OF_MAC	544
#else
#define MAX_NUMBER_OF_MAC	64
#endif

#define MODE_CCK		0
#define MODE_OFDM		1
#define MODE_HTMIX		2
#define MODE_HTGREENFIELD	3
#define MODE_VHT		4
#if defined(RTCONFIG_WLMODULE_MT7915D_AP) || defined(RTCONFIG_MT798X)
#define MODE_HE 5
#define MODE_HE_SU	8
#define MODE_HE_24G 7
#define MODE_HE_5G 6
#define MODE_HE_EXT_SU	9
#define MODE_HE_TRIG	10
#define MODE_HE_MU	11
#endif

#define BW_20			0
#define BW_40			1
#define BW_BOTH			2
#define BW_80			2
#if defined(RTCONFIG_MT798X)
#define BW_160			3
#define BW_10			4
#define BW_5			5
#define BW_8080			6
#else
#define BW_10			3
#define BW_160			3
#endif

#if defined(RTCONFIG_RALINK_MT7622) || defined(RTCONFIG_WLMODULE_MT7629_AP)
#define RT_802_11_MAC_ENTRY_for_5G			RT_802_11_MAC_ENTRY
#define MACHTTRANSMIT_SETTING_for_5G		HTTRANSMIT_SETTING
#define RT_802_11_MAC_ENTRY_for_2G			RT_802_11_MAC_ENTRY
#define MACHTTRANSMIT_SETTING_for_2G		HTTRANSMIT_SETTING
#define RT_802_11_MAC_TABLE_5G				RT_802_11_MAC_TABLE
#define RT_802_11_MAC_TABLE_2G				RT_802_11_MAC_TABLE

#define MAC_ADDR_LEN			6
/* MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!! */
typedef union _HTTRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		unsigned short MODE:3;	/* Use definition MODE_xxx. */
		unsigned short iTxBF:1;
		unsigned short eTxBF:1;
		unsigned short STBC:1;	/* only support in HT/VHT mode with MCS0~7 */
		unsigned short ShortGI:1;
		unsigned short BW:2;	/* channel bandwidth 20MHz/40/80 MHz */
		unsigned short ldpc:1;
		unsigned short MCS:6;	/* MCS */
	} field;
#else
	struct {
		unsigned short MCS:6;
		unsigned short ldpc:1;
		unsigned short BW:2;
		unsigned short ShortGI:1;
		unsigned short STBC:1;
		unsigned short eTxBF:1;
		unsigned short iTxBF:1;
		unsigned short MODE:3;
	} field;
#endif
	unsigned short word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[MAC_ADDR_LEN];
	unsigned char Aid;
	unsigned char Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	signed char AvgRssi0;
	signed char AvgRssi1;
	signed char AvgRssi2;
	unsigned int ConnectedTime;
	HTTRANSMIT_SETTING TxRate;
	unsigned int LastRxRate;
	/*
		sync with WEB UI's structure for ioctl usage.
	*/
	unsigned short StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	unsigned short SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
	/*	SHORT TxPER;	*/					/* TX PER over the last second. Percent */
	/*	SHORT reserved;*/
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

#else
#define INIC_VLAN_ID_START	4 //first vlan id used for RT3352 iNIC MII
#define INIC_VLAN_IDX_START	2 //first available index to set vlan id and its group.

#if defined(RTCONFIG_WLMODULE_MT7610_AP) || defined(RTCONFIG_WLMODULE_MT7663E_AP)
#define RT_802_11_MAC_ENTRY_for_5G		RT_802_11_MAC_ENTRY_11AC
#define MACHTTRANSMIT_SETTING_for_5G		MACHTTRANSMIT_SETTING_11AC
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP) || defined(RTCONFIG_MT798X)
#define RT_802_11_MAC_ENTRY_for_5G		RT_802_11_MAC_ENTRY_11AX
#define MACHTTRANSMIT_SETTING_for_5G		MACHTTRANSMIT_SETTING_11AX
#else
#define RT_802_11_MAC_ENTRY_for_5G		RT_802_11_MAC_ENTRY_RT3883
#define MACHTTRANSMIT_SETTING_for_5G		MACHTTRANSMIT_SETTING_2G
#endif

#if defined(RTN65U)
#define RT_802_11_MAC_ENTRY_for_2G		RT_802_11_MAC_ENTRY_RT3352_iNIC
#elif defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined(RTAC1200GU)
#define RT_802_11_MAC_ENTRY_for_2G		RT_802_11_MAC_ENTRY_7603E
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP) || defined(RTCONFIG_MT798X)
#define RT_802_11_MAC_ENTRY_for_2G		RT_802_11_MAC_ENTRY_11AX
#else
#define RT_802_11_MAC_ENTRY_for_2G		RT_802_11_MAC_ENTRY_2G
#endif

#if defined(RTCONFIG_WLMODULE_MT7915D_AP) || defined(RTCONFIG_MT798X)
#define MACHTTRANSMIT_SETTING_for_2G		MACHTTRANSMIT_SETTING_11AX
#else
#define MACHTTRANSMIT_SETTING_for_2G		MACHTTRANSMIT_SETTING_2G
#endif
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
#if defined(RTCONFIG_WLMODULE_MT7663E_AP)
	struct  {	/* refter to src-mtk3.5/linux/linux-3.10.108/driver/net/wireless/mtk/mt7663e/mt_wifi/embedded/include/oid.h: typedef union _HTTRANSMIT_SETTING{}; */
	unsigned short MCS:6;
	unsigned short ldpc:1;
	unsigned short BW:2;
	unsigned short ShortGI:1;
	unsigned short STBC:1;
	unsigned short eTxBF:1;
	unsigned short iTxBF:1;
	unsigned short MODE:3;
	} field;
#else
	struct  {
	unsigned short	MCS:7;	// MCS
	unsigned short	BW:2;	//channel bandwidth 20MHz or 40 MHz
	unsigned short	ShortGI:1;
	unsigned short	STBC:1;	//SPACE
	unsigned short	eTxBF:1;
	unsigned short	iTxBF:1;
	unsigned short	MODE:3;	// Use definition MODE_xxx.
	} field;
#endif
	unsigned short	word;
 } MACHTTRANSMIT_SETTING_11AC, *PMACHTTRANSMIT_SETTING_11AC;

typedef union  _MACHTTRANSMIT_SETTING_11AX {
#ifdef RT_BIG_ENDIAN
	struct {
	unsigned short MODE:3;	/* Use definition MODE_xxx. */
	unsigned short iTxBF:1;
	unsigned short eTxBF:1;
	unsigned short STBC:1;	/* only support in HT/VHT mode with MCS0~7 */
	unsigned short ShortGI:1;	/* TBD: need to extend to 2 bits for HE GI */
	unsigned short BW:2;	/* channel bandwidth 20MHz/40/80 MHz */
	unsigned short ldpc:1;
	unsigned short MCS:6;	/* MCS */
	} field;
#else
	struct {
	unsigned short MCS:6;
	unsigned short ldpc:1;
	unsigned short BW:2;
	unsigned short ShortGI:1;
	unsigned short STBC:1;
	unsigned short eTxBF:1;
	unsigned short iTxBF:1;
	unsigned short MODE:3;
} field;
#endif
	unsigned short	word;
 } MACHTTRANSMIT_SETTING_11AX, *PMACHTTRANSMIT_SETTING_11AX;

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

typedef struct _RT_802_11_MAC_ENTRY_7603E {
    unsigned char  ApIdx;
    unsigned char Addr[ETHER_ADDR_LEN];
    unsigned char Aid;
    unsigned char Psm;              /* 0:PWR_ACTIVE, 1:PWR_SAVE */
    unsigned char MimoPs;           /* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
    char AvgRssi0;
    char AvgRssi1;
    char AvgRssi2;
    unsigned int ConnectedTime;
    MACHTTRANSMIT_SETTING_2G    TxRate;
    unsigned int LastRxRate;
} RT_802_11_MAC_ENTRY_7603E, *PRT_802_11_MAC_ENTRY_7603E;


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

typedef struct _RT_802_11_MAC_ENTRY_11AX {
	unsigned char ApIdx;
	unsigned char Addr[ETHER_ADDR_LEN];
	uint16_t Aid;
	unsigned char Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	char AvgRssi0;
	char AvgRssi1;
	char AvgRssi2;
	uint32_t ConnectedTime;
	MACHTTRANSMIT_SETTING_11AX TxRate;
	uint32_t LastRxRate;
	short StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	short SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
} RT_802_11_MAC_ENTRY_11AX, *PRT_802_11_MAC_ENTRY_11AX;

typedef struct _RT_802_11_MAC_TABLE_5G {
    unsigned long	Num;
    RT_802_11_MAC_ENTRY_for_5G	Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE_5G, *PRT_802_11_MAC_TABLE_5G;

typedef struct _RT_802_11_MAC_TABLE_2G {
    unsigned long	Num;
    RT_802_11_MAC_ENTRY_for_2G	Entry[MAX_NUMBER_OF_MAC];
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
#endif
typedef struct _SITE_SURVEY
{
	char channel[4];
//	unsigned char channel;
//	unsigned char centralchannel;
//	unsigned char unused;
	unsigned char ssid[33];
	char bssid[18];
	char encryption[9];
	char authmode[16];
	char signal[9];
	char wmode[8];
#if 0//defined(RTN14U)
	char wps[4];
	char dpid[5];
#endif
//	char bsstype[3];
//	char centralchannel[3];
} SITE_SURVEY;

typedef struct _SITE_SURVEY_ARRAY
{
	SITE_SURVEY SiteSurvey[64];
} SSA;

struct _SITESURVEY_VSIE {
	char Ssid[33];
	unsigned char Bssid[6];
    unsigned char Channel;
    char Rssi;
    unsigned char vendor_ie[128];
    unsigned char vendor_ie_len;
};

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
	ASUS_SUBCMD_GETSKUTABLE,
    ASUS_SUBCMD_GETSKUTABLE_TXBF,
	ASUS_SUBCMD_CLIQ,
	ASUS_SUBCMD_DRIVERVER,
    ASUS_SUBCMD_CLRSSI,    
    ASUS_SUBCMD_GMONITOR_RSSI,
    ASUS_SUBCMD_MACMODE,
    ASUS_SUBCMD_MACLIST,
	ASUS_SUBCMD_GDFSNOPCHANNEL,
	ASUS_SUBCMD_GCHANNELINFO,
    ASUS_SUBCMD_GETSITESURVEY_VSIE,
    ASUS_SUBCMD_GETAPCLIENABLE,
    ASUS_SUBCMD_GETSITESURVEY_VSIE_COUNT,
    ASUS_SUBCMD_DFS_STATUS,
    ASUS_SUBCMD_RRM_BCN_RESP,
    ASUS_SUBCMD_GET_RCLASS,
    ASUS_SUBCMD_DFS_CH_STATUS,				//24
    ASUS_SUBCMD_GET_CH_BW,				//25
	ASUS_SUBCMD_MAX
};


#if 0
typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B,
	PHY_11A,
	PHY_11ABG_MIXED,
	PHY_11G,
	PHY_11ABGN_MIXED,	// both band		5
	PHY_11N,		//			6
	PHY_11GN_MIXED,		// 2.4G band		7
	PHY_11AN_MIXED,		// 5G  band		8
	PHY_11BGN_MIXED,	// if check 802.11b.	9
	PHY_11AGN_MIXED,	// if check 802.11b.	10
} RT_802_11_PHY_MODE;
#endif

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN 6
#endif

#ifdef RTCONFIG_BTM_11V
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

typedef unsigned char u8;
typedef unsigned short			u16;
typedef unsigned int			u32;

#define cpu2le16(x) ((unsigned short)(x))
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
#define	DAEMON_NEIGHBOR_REPORT_MAX_NUM 128
#define IE_RRM_NEIGHBOR_REP			52
#define IE_MBO_ELEMENT_ID 			221 /* 0xDD */
#define MBO_ATTR_MAX_LEN			252 /* spec 0.0.23 - IE LEN is 256 = OUI 4 + ATTR 252 */


/* MBO Attribute Id List */
#define MBO_ATTR_AP_CAP_INDCATION				1
#define MBO_ATTR_STA_NOT_PREFER_CH_REP			2
#define MBO_ATTR_STA_CDC						3 		/* Cellular Data Capability */
#define MBO_ATTR_AP_ASSOC_DISALLOW				4
#define MBO_ATTR_AP_CDCP						5 		/* Cellular Data Connection Preference */
#define MBO_ATTR_AP_TRANS_REASON				6
#define MBO_ATTR_STA_TRANS_REJ_REASON			7
#define MBO_ATTR_AP_ASSOC_RETRY_DELAY			8
#define MBO_WDEV_ATTR_MAX_NUM					8 		/* Should be updated according to ID list */

typedef enum {
	WAPP_SUCCESS = 0,
	WAPP_INVALID_ARG,
	WAPP_RESOURCE_ALLOC_FAIL,
	WAPP_NOT_INITIALIZED,
	WAPP_LOOKUP_ENTRY_NOT_FOUND,
	WAPP_UNEXP,
} WAPP_ERR_CODE;

struct btm_payload {
	union {
		struct {
			u8 btm_query_reason;
			/*
 			 * Following are BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__ ((packed)) btm_query;

		struct {
			u8 request_mode;
			u16 disassociation_timer;
			u8 validity_interval;
			/*
 			 * Following are BSS Termination Duration, Session Information URL,
 			 * and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__((packed)) btm_req;

		struct {
			u8 status_code;
			u8 bss_termination_delay;
			/*
 			 * Following are Target BSSID, and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__ ((packed)) btm_rsp;
	}u;
} __attribute__ ((packed));

enum btm_req_mode_bit_map {
    CAND_LIST_INCLUDED_BIT_MAP,
	ABIDGED_BIT_MAP,
	DISASSOC_IMNT_BIT_MAP,
	BSS_TERM_INCLUDED_BIT_MAP,
	ESS_DISASSOC_IMNT_BIT_MAP,
};

typedef struct GNU_PACKED _tbtt_info_set {
	u8 NrAPTbttOffset;
	u32 ShortBssid;
} tbtt_info_set;

typedef struct GNU_PACKED _wapp_nr_info
{
	u8 	Bssid[MAC_ADDR_LEN];
	u32 BssidInfo;
	u8  RegulatoryClass;
	u8  ChNum;
	u8  PhyType;
	u8  CandidatePrefSubID;
	u8  CandidatePrefSubLen;
	u8  CandidatePref;
	/* extra sec info */
	u32 akm;
	u32 cipher;
	u8  TbttInfoSetNum;
	tbtt_info_set TbttInfoSet;
	u8  Rssi;
} wapp_nr_info;


/* for NR IE , append Bssid ~ CandidatePref */
#define NEIGHBOR_REPORT_IE_SIZE 	(sizeof(wapp_nr_info) - 15)

typedef struct daemon_nr_list {
	u8 	CurrListNum;
	wapp_nr_info NRInfo[DAEMON_NEIGHBOR_REPORT_MAX_NUM];
} DAEMON_NR_LIST, *P_DAEMON_NR_LIST;

struct mbo_cfg {
	u8  cdcp; 						/* AP's cellular data connection preference */
	u8  assoc_disallow_reason;
	u8  ap_capability;
	u16 assoc_retry_delay;			/* mbo_assoc_retry_delay  unit: 1 second */
	u8  dft_trans_reason; 			/* mbo_default_trans_reason */
#if 0
	/* call back function of mbo events */
	const struct mbo_event_ops *event_ops;

	/* driver interface operation */
	const struct mbo_drv_ops *drv_ops;
#endif
};

typedef struct {
    u8   AttrID;
    u8   AttrLen;
    //CHAR    AttrBody[1];
    char AttrBody[MBO_ATTR_MAX_LEN];
} MBO_ATTR_STRUCT,*P_MBO_ATTR_STRUCT;

struct wifi_app {
	struct mbo_cfg 	*mbo;
	/* neighbor report common pool */
	DAEMON_NR_LIST daemon_nr_list;
};

typedef struct GNU_PACKED btm_req_ie_data_s {
	unsigned int ifindex;
	unsigned char peer_mac_addr[6];
	unsigned char dialog_token;
	unsigned int timeout;
	unsigned int btm_req_len;
	unsigned char btm_req[0];
}btm_req_ie_data_t, *p_btm_req_ie_data_t;

typedef union GNU_PACKED _RRM_BSSID_INFO
{
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		u32 Reserved:18;
		u32 FTM:1;
		u32 VHT:1;
		u32 HT:1;
		u32 MobilityDomain:1;
		u32 ImmediateBA:1;
		u32 DelayBlockAck:1;
		u32 RRM:1;
		u32 APSD:1;
		u32 Qos:1;
		u32 SpectrumMng:1;
		u32 KeyScope:1;
		u32 Security:1;
		u32 APReachAble:2;
#else
		u32 APReachAble:2;
		u32 Security:1;
		u32 KeyScope:1;
		u32 SpectrumMng:1;
		u32 Qos:1;
		u32 APSD:1;
		u32 RRM:1;
		u32 DelayBlockAck:1;
		u32 ImmediateBA:1;
		u32 MobilityDomain:1;
		u32 HT:1;
		u32 VHT:1;
		u32 FTM:1;
		u32 Reserved:18;
#endif
	} field;
	u32 word;
} RRM_BSSID_INFO, *PRRM_BSSID_INFO;

#define OID_GET_SET_TOGGLE		0x8000
#define	OID_GET_SET_FROM_UI		0x4000
#define OID_802_11_WNM_COMMAND  0x094A
#define OID_802_11_WNM_EVENT	0x094B
#define OID_802_11_RRM_COMMAND  0x094C
#define OID_802_11_RRM_EVENT	0x094D

enum wnm_cmd_subid {
	OID_802_11_WNM_CMD_ENABLE = 0x01,
	OID_802_11_WNM_CMD_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ,
	OID_802_11_WNM_CMD_QUERY_BTM_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ_IE,
	OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM,
};

struct GNU_PACKED wnm_command {
	unsigned char command_id;
	unsigned char command_len;
	unsigned char command_body[0];
};
#endif /* RTCONFIG_BTM_11V */

#if defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	0x31000
#define DEFAULT_EEPROM_SIZE_SHIFT	0x20000
#elif defined(RTCONFIG_RALINK_MT7622)
#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	0x21000
#define DEFAULT_EEPROM_SIZE_SHIFT	0x10000
#elif defined(RTCONFIG_MT798X)
#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	(8*124*1024) // 8 UBI LEB
#else
#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	0x11000
#define DEFAULT_EEPROM_SIZE_SHIFT	0x0
#endif
/* Below offset addresses, actually, based on start address of Flash.
 * Thus, there are absolute addresses and only valid on products
 * associated with parallel NOR Flash and SPI Flash.
 */
#define OFFSET_MTD_FACTORY	0x40000
#define OFFSET_EEPROM_VER	0x40002
#if defined(RTCONFIG_MT798X)
#define FTRY_PARM_SHIFT		(0xA0000 + 0x10000) /* EEPROM end + 64KB */
#else /* Legacy */
#define	FTRY_PARM_SHIFT		(0)
#endif


#if defined(RTAC85U) || defined(RTAC85P) || defined(RPAC87) || defined(RTAC1200GU) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750) || defined(RTACRH18)
#define OFFSET_PIN_CODE		0x4ff70	// 8 bytes
#define OFFSET_COUNTRY_CODE	0x4ff78	// 2 bytes
#define OFFSET_BOOT_VER		0x4ff7a	// 4 bytes
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define OFFSET_PIN_CODE		0x6ff70	// 8 bytes
#define OFFSET_COUNTRY_CODE	0x6ff78	// 2 bytes
#define OFFSET_BOOT_VER		0x6ff7a	// 4 bytes
#elif defined(RT4GAC86U)
#define OFFSET_PIN_CODE		0x5ff70	// 8 bytes
#define OFFSET_COUNTRY_CODE	0x5ff78	// 2 bytes
#define OFFSET_BOOT_VER		0x5ff7a	// 4 bytes
#else
#define OFFSET_PIN_CODE		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x180)
#define OFFSET_COUNTRY_CODE	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x188)
#define OFFSET_BOOT_VER		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x18A)
#endif

#if defined(RTN14U) || defined(RTN11P) || defined(RTN300) || defined(RTN11P_B1)
#define OFFSET_MAC_ADDR		0x40004
#define OFFSET_MAC_ADDR_2G	0x40004 //only one MAC
#define OFFSET_MAC_GMAC2	0x4018E
#define OFFSET_MAC_GMAC0	0x40194
#elif defined(RTACRH18)
#define OFFSET_MAC_ADDR		0x48004	//for 5G Virtual MAC
#define OFFSET_MAC_ADDR_2G	0x40004
#define OFFSET_MAC_GMAC0	0x4002A
#define OFFSET_MAC_GMAC2	0x40024
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define OFFSET_MAC_ADDR		0x4000A	//for 5G Virtual MAC
#define OFFSET_MAC_ADDR_2G	0x40004
#define OFFSET_MAC_GMAC0	0x40161
#define OFFSET_MAC_GMAC2	0x40161
#elif defined(RTAC52U) || defined(RTAC51U) || defined(RTN54U) || defined(RTAC54U) || defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC51UP) || defined(RTAC53) || defined(RTAC1200GA1) || defined(RTAC1200GU) || defined(RTAC1200)  || defined(RTAC1200V2) || defined(RPAC87) || defined(RTAC85U) || defined(RTAC85P) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750) || defined(RT4GAC86U)
#if defined(RTN800HP)
#define OFFSET_MAC_ADDR_2G	0x40004
#define OFFSET_MAC_ADDR		0x40004 //only one MAC
#elif defined(RT4GAC86U)
#define OFFSET_MAC_ADDR_2G	0x40004
#define OFFSET_MAC_ADDR		0x45004
#else
#define OFFSET_MAC_ADDR_2G	0x40004
#define OFFSET_MAC_ADDR		0x48004
#endif
#if defined(RTAC85U) || defined(RTAC85P) || defined(RPAC87) || defined(RTAC1200GU) || defined(RTACRH26) || defined(TUFAC1750)
#define OFFSET_MAC_GMAC0	0x4E000
#define OFFSET_MAC_GMAC2	0x4E006
#elif defined(RTN800HP)
#define OFFSET_MAC_GMAC0	0x4E000
#define OFFSET_MAC_GMAC2	0x4E000 //only one Mac
#elif defined(RT4GAC86U)
#define OFFSET_MAC_GMAC0	0x4002A
#define OFFSET_MAC_GMAC2	0x40024
#define OFFSET_GOBIIMEI         0x5ff61 // 15 bytes for gobi IMEI
#else
#define OFFSET_MAC_GMAC0	0x40022
#define OFFSET_MAC_GMAC2	0x40028
#endif
#elif defined(RTCONFIG_MT798X)
#define OFFSET_MAC_ADDR_2G	(OFFSET_MTD_FACTORY + 0x4)	// MTK EEPROM MAC address
#define OFFSET_MAC_ADDR		OFFSET_MAC_ADDR_2G		// 5G MAC is derived from EEPROM MAC
#define OFFSET_MAC_GMAC1	(OFFSET_MTD_FACTORY + 0x24)	// MTK eth1's MAC for WAN
#define OFFSET_MAC_GMAC0	(OFFSET_MTD_FACTORY + 0x2A)	// MTK br-lan's MAC for LAN
#else
#define OFFSET_MAC_ADDR		0x40004
#define OFFSET_MAC_ADDR_2G	0x48004
#define OFFSET_MAC_GMAC2	0x40022
#define OFFSET_MAC_GMAC0	0x40028
#endif
#if defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined(RTAC1200GU)
#define OFFSET_FIX_CHANNEL      0x40170
#elif defined(RTAC85U) || defined(RTAC85P) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750)
#define OFFSET_BR_STP      0x4ff7e	// 1 bytes
#endif

#define OFFSET_TXBF_PARA	0x401A0

#if defined(RTCONFIG_NEW_REGULATION_DOMAIN)
#define	MAX_REGDOMAIN_LEN	10
#define	MAX_REGSPEC_LEN		4
#if defined(RTAC85U) || defined(RTAC85P) || defined(RPAC87) || defined(RTAC1200GU) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750) || defined(RTACRH18)
#define REG2G_EEPROM_ADDR	0x4ff40 //10 bytes
#define REG5G_EEPROM_ADDR	0x4ff4a //10 bytes
#define REGSPEC_ADDR		0x4ff54 // 4 bytes
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define REG2G_EEPROM_ADDR	0x6ff40 //10 bytes
#define REG5G_EEPROM_ADDR	0x6ff4a //10 bytes
#define REGSPEC_ADDR		0x6ff54 // 4 bytes
#elif defined(RT4GAC86U)
#define REG2G_EEPROM_ADDR	0x5ff40 //10 bytes
#define REG5G_EEPROM_ADDR	0x5ff4a //10 bytes
#define REGSPEC_ADDR		0x5ff54 // 4 bytes
#else
#define REG2G_EEPROM_ADDR	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x234) //10 bytes
#define REG5G_EEPROM_ADDR	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x23E) //10 bytes
#define REGSPEC_ADDR		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0x248) // 4 bytes
#endif
#endif /* RTCONFIG_NEW_REGULATION_DOMAIN */

#define OFFSET_FORCE_USB3	0x4ff60	/* 1 bytes */

#if defined(RTAC1200) || defined(RTAC1200V2) || defined(RTN11P_B1) || defined(RTACRH18)
#define OFFSET_PSK		0x4ff80	/* 16 bytes */
#elif defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define OFFSET_PSK		0x6ff80	/* 16 bytes */
#elif defined(RT4GAC86U)
#define OFFSET_PSK		0x5ff80	/* 16 bytes */
#else
#define OFFSET_PSK		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xff80) //15bytes
#endif


#define MAX_PASS_LEN 32

#if defined(RTCONFIG_WLMODULE_MT7915D_AP)
#define OFFSET_PASS		0x6ff50	// 32 bytes (MAX_PASS_LEN)
#define OFFSET_EISN		0x6ff70	// 32 bytes
#define OFFSET_TERRITORY_CODE	0x6ff90	/* 5 bytes, e.g., US/01, US/02, TW/01, etc. */
#define OFFSET_DEV_FLAGS	0x6ffa0 //device dependent flags
#define OFFSET_ODMPID		0x6ffb0 //the shown model name (for Bestbuy and others)
#define OFFSET_FAIL_RET		0x6ffc0
#define OFFSET_FAIL_BOOT_LOG	0x6ffd0	//bit operation for max 100
#define OFFSET_FAIL_DEV_LOG	0x6ffe0	//bit operation for max 100
#define OFFSET_SERIAL_NUMBER	0x6fff0	// 32 bytes
#define OFFSET_IPADDR_LAN	0x6ff30 // force LAN IP for ATE use

#define OFFSET_HWID	0x6FE00	// 4 bytes
#define OFFSET_HW_VERSION	0x6FE04	// 8 bytes
#define OFFSET_HW_BOM	0x6FE0C	// 32 bytes
#define OFFSET_HW_DATE_CODE	0x6FE3E	// 8 bytes
#define OFFSET_HW_COBRAND       0x6FE46 // 1 byte
#ifdef RTCONFIG_32BYTES_ODMPID  
#define OFFSET_32BYTES_ODMPID   0x6FE47 // 32 bytes
#endif
#elif defined(RT4GAC86U)
#define OFFSET_PASS		0x5ff50	// 32 bytes
#define OFFSET_EISN		0x5ff70	// 32 bytes
#define OFFSET_TERRITORY_CODE	0x5ff90	/* 5 bytes, e.g., US/01, US/02, TW/01, etc. */
#define OFFSET_DEV_FLAGS	0x5ffa0 //device dependent flags
#define OFFSET_ODMPID		0x5ffb0 //the shown model name (for Bestbuy and others)
#define OFFSET_FAIL_RET		0x5ffc0
#define OFFSET_FAIL_BOOT_LOG	0x5ffd0	//bit operation for max 100
#define OFFSET_FAIL_DEV_LOG	0x5ffe0	//bit operation for max 100
#define OFFSET_SERIAL_NUMBER	0x5fff0	// 32 bytes
#define OFFSET_IPADDR_LAN	0x5ff30 // force LAN IP for ATE use

#define OFFSET_HWID	0x5FE00	// 4 bytes
#define OFFSET_HW_VERSION	0x5FE04	// 8 bytes
#define OFFSET_HW_BOM	0x5FE0C	// 32 bytes
#define OFFSET_HW_DATE_CODE	0x5FE3E	// 8 bytes
#else
#define OFFSET_PASS		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xff50)	// 32 bytes
#define OFFSET_EISN		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xff70)	// 32 bytes
#define OFFSET_TERRITORY_CODE	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xff90)	/* 5 bytes, e.g., US/01, US/02, TW/01, etc. */
#define OFFSET_DEV_FLAGS	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xffa0) //device dependent flags
#define OFFSET_ODMPID		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xffb0) //the shown model name (for Bestbuy and others)
#define OFFSET_FAIL_RET		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xffc0)
#define OFFSET_FAIL_BOOT_LOG	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xffd0)	//bit operation for max 100
#define OFFSET_FAIL_DEV_LOG	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xffe0)	//bit operation for max 100
#define OFFSET_SERIAL_NUMBER	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xfff0)	// 32 bytes
#define OFFSET_IPADDR_LAN	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xff30) // force LAN IP for ATE use

#define OFFSET_HWID		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFE00)	// 4 bytes
#define OFFSET_HW_VERSION	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFE04)	// 8 bytes
#define OFFSET_HW_BOM		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFE0C)	// 32 bytes
#define OFFSET_HW_DATE_CODE	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFE3E)	// 8 bytes
#define OFFSET_HW_COBRAND	(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFE46)	// 1 bytes
#endif

#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_MT798X)
#define OFFSET_AMAS_BUNDLE_FLAG		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xfd20)
#define OFFSET_AMAS_BUNDLE_KEY		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xfd00)
#else
#define OFFSET_AMAS_BUNDLE_FLAG		0x6fd20
#define OFFSET_AMAS_BUNDLE_KEY		0x6fd00
#endif
#endif // RTCONFIG_AMAS

#ifdef RTCONFIG_ASUSCTRL
#if defined(RTCONFIG_MT798X)
#define OFFSET_ASUSCTRL_FLAGS		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFDF0)  /*  8 bytes */
#define ASUSCTRL_FLAGS_LENGTH		(8)
#define OFFSET_ASUSCTRL_CHG_SKU		(OFFSET_MTD_FACTORY + FTRY_PARM_SHIFT + 0xFDF8)  /*  2 bytes */
#define ASUSCTRL_CHG_SKU_LENGTH		(2)
#else
#define OFFSET_ASUSCTRL_FLAGS		(DEFAULT_EEPROM_SIZE_SHIFT + 0x4FDF0)  /*  8 bytes */
#define ASUSCTRL_FLAGS_LENGTH		(8)
#define OFFSET_ASUSCTRL_CHG_SKU		(DEFAULT_EEPROM_SIZE_SHIFT + 0x4FDF8)  /*  2 bytes */
#define ASUSCTRL_CHG_SKU_LENGTH		(2)
#endif
#endif // RTCONFIG_ASUSCTRL

#define OFFSET_POWER_5G_TX0_36_x6	0x40096
#define OFFSET_POWER_5G_TX1_36_x6	0x400CA
#define OFFSET_POWER_5G_TX2_36_x6	0x400FE
#define OFFSET_POWER_2G			0x480DE

#define RA_LED_ON		0	// low active (all 5xx series)
#define RA_LED_OFF		1

#define	RA_LED_POWER		0
#define	RA_LED_USB		24
#ifdef	RTCONFIG_N56U_SR2
#define	RA_BTN_RESET		25
#else
#define	RA_BTN_RESET		13
#endif
#define	RA_BTN_WPS		26

#ifdef	RTCONFIG_DSL
#define	RA_LED_WAN		25	//javi
#else
#define	RA_LED_WAN		27
#endif

#ifdef	RTCONFIG_N56U_SR2
#define	RA_LED_LAN		31
#else
#define	RA_LED_LAN		19
#endif
/*
#define RTN13U_SW1	9
#define RTN13U_SW2	13
#define RTN13U_SW3	11
*/

#define GPIO_DIR_OUT	1
#define GPIO_DIR_IN	0

#define GPIO0		0x0001
#define GPIO1		0x0002
#define GPIO2		0x0004
#define GPIO3		0x0008
#define GPIO4		0x0010
#define GPIO5		0x0020
#define GPIO6		0x0040
#define GPIO7		0x0080
#define GPIO15		0x8000

unsigned long task_mask;

int switch_init(void);

void switch_fini(void);

int ra3052_reg_read(int offset, int *value);

int ra3052_reg_write(int offset, int value);

int config_3052(int type);

int restore_3052();

void ra_gpio_write_spec(int bit_idx, int flag);

int check_all_tasks();

int ra_gpio_set_dir(int dir);

int ra_gpio_write_int(int value);

int ra_gpio_read_int(int *value);

int ra_gpio_write_bit(int idx, int value);

extern int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq);

//cal the rate from MACHTTRANSMIT_SETTING structure replied from ioctl(CMD_RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT)
extern int getRate(MACHTTRANSMIT_SETTING_for_5G HTSetting);
extern int getRate_2g(MACHTTRANSMIT_SETTING_for_2G HTSetting);

/* for ATE Get_WanLanStatus command */
#if defined(RTCONFIG_RALINK_MT7621)
#define MAX_PORT 6
#elif defined(RTCONFIG_SWITCH_MT7986_MT7531)
#define MAX_PORT 7
#else
#define MAX_PORT 5
#endif
typedef struct {
	unsigned int link[MAX_PORT];
	unsigned int speed[MAX_PORT];
} phyState;

#endif
