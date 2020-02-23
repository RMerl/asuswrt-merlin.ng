/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef __WL_DEFS_H__
#define __WL_DEFS_H__

#include <stdio.h>
#include <netinet/in.h>
#include "wl_max_mbssid.h"

#ifndef __WLCSM_DEFS_H__
#define __WLCSM_DEFS_H__

#define WL_NUM_SSID      (BcmWl_GetMaxMbss())
#define INTERFACE_WL0_INDEX       0
#define INTERFACE_WL1_INDEX       1

#define WLAN_PREFIX	"wl"
#define IS_WLAN_IFC(x)	(!strncmp(x, WLAN_PREFIX, strlen(WLAN_PREFIX)))

/* Please sync this WL_MAX_NUM_SSID with private/libs/cms_core/mdm_intwlan.c */
#ifndef WL_MAX_NUM_SSID
#define WL_MAX_NUM_SSID  WL_MAX_NUM_MBSSID
#endif /* WL_MAX_NUM_SSID */
#define WL_SIZE_2_MAX    2
#define WL_SIZE_4_MAX    4
#define WL_SIZE_8_MAX    8
#define WL_SIZE_132_MAX  132
#define WL_SIZE_512_MAX  512
#define WL_SM_SIZE_MAX   16
#define WL_MID_SIZE_MAX  32
#define WL_SSID_SIZE_MAX 48
#define WL_MIN_PSK_LEN	8
#define WL_MAX_PSK_LEN	64
#define WL_WPA_PSK_SIZE_MAX  72  // max 64 hex or 63 char
#define WL_RADIUS_KEY_SIZE_MAX  88 // max is 80, limited by nas_wksp.h.  IAS allow up to 128 char
#define WL_LG_SIZE_MAX   1024
#define WL_KEY_NUM       4
#define WL_WDS_NUM       4
#define WL_MACFLT_NUM    32
#define WL_MACADDR_SIZE      18
#define WL_MIN_FRAGMENTATION_THRESHOLD  256
#define WL_MAX_FRAGMENTATION_THRESHOLD  2346
#define WL_MIN_RTS_THRESHOLD            0
#define WL_MAX_RTS_THRESHOLD            2347
#define WL_MIN_DTIM                     1
#define WL_MAX_DTIM                     255
#define WL_MIN_BEACON_INTERVAL          1
#define WL_MAX_BEACON_INTERVAL          65535
#define WL_KEY64_SIZE_HEX               10
#define WL_KEY64_SIZE_CHAR              5
#define WL_KEY128_SIZE_HEX              26
#define WL_KEY128_SIZE_CHAR             13
#define WL_MAX_ASSOC_STA                128

#define WL_FLT_MAC_OFF   "disabled"
#define WL_FLT_MAC_ALLOW "allow"
#define WL_FLT_MAC_DENY  "deny"

#define AUTO_MODE -1
#define ON         1
#define OFF        0

#define WL_BRIDGE_RESTRICT_ENABLE      0
#define WL_BRIDGE_RESTRICT_DISABLE     1
#define WL_BRIDGE_RESTRICT_ENABLE_SCAN 2

/* authentication mode */
#define WL_AUTH_OPEN     		"open"
#define WL_AUTH_SHARED   		"shared"
#define WL_AUTH_RADIUS   		"radius"
#define WL_AUTH_WPA      		"wpa"
#define WL_AUTH_WPA_PSK  		"psk"
#define WL_AUTH_WPA2     		"wpa2"
#define WL_AUTH_WPA2_PSK 		"psk2"
#define WL_AUTH_WPA2_MIX 		"wpa wpa2"
#define WL_AUTH_WPA2_PSK_MIX 	"psk psk2"

#define WL_WPA_AUTH_DISABLED		0x0000	/* Legacy (i.e., non-WPA) */
#define WL_WPA_AUTH_NONE			0x0001	/* none (IBSS) */
#define WL_WPA_AUTH_UNSPECIFIED		0x0002	/* over 802.1x */
#define WL_WPA_AUTH_PSK				0x0004	/* Pre-shared key */
#define WL_WPA_AUTH_8021X 			0x0020	/* 802.1x, reserved */
#define WL_WPA2_AUTH_UNSPECIFIED	0x0040	/* over 802.1x */
#define WL_WPA2_AUTH_PSK			0x0080	/* Pre-shared key */
#define WL_WSC 0x10

#define TKIP_ONLY 	"tkip"
#define AES_ONLY  	"aes"
#define TKIP_AND_AES 	"tkip+aes"

#define WL_BIT_KEY_128   0
#define WL_BIT_KEY_64    1

#define WL_PHYTYPE_A      0
#define WL_PHYTYPE_B      1
#define WL_PHYTYPE_G      2
#define WL_PHYTYPE_N      4
#define WL_PHYTYPE_LP     5
#define WL_PHYTYPE_SSN    6
#define WL_PHYTYPE_HT     7
#define WL_PHYTYPE_LCN    8
#define WL_PHYTYPE_LCN40  10
#define WL_PHYTYPE_AC     11

#define WL_PHY_TYPE_A    "a"
#define WL_PHY_TYPE_B    "b"
#define WL_PHY_TYPE_G    "g"
#define WL_PHY_TYPE_N    "n"
#define WL_PHY_TYPE_LP   "l"
#define WL_PHY_TYPE_H    "h"
#define WL_PHY_TYPE_AC   "v"

#define WL_WIDE_BW_CHAN_WIDTH_20  0
#define WL_WIDE_BW_CHAN_WIDTH_40  1
#define WL_WIDE_BW_CHAN_WIDTH_80  2
#define WL_WIDE_BW_CHAN_WIDTH_160  3
#define WL_WIDE_BW_CHAN_WIDTH_80_80  4

#define WL_BASIC_RATE_DEFAULT    "default"
#define WL_BASIC_RATE_ALL        "all"
#define WL_BASIC_RATE_1_2        "12"
#define WL_BASIC_RATE_WIFI_2     "wifi2"

#define WL_MODE_G_AUTO           1
#define WL_MODE_G_PERFORMANCE    4
#define WL_MODE_G_LRS            5
#define WL_MODE_B_ONLY           0

#define WL_AUTO                  "auto"
#define WL_OFF                   "off"
#define WL_ON                    "on"
#define WL_DISABLED              "disabled"
#define WL_ENABLED               "enabled"

#define WL_WME_ON		 		1
#define WL_WME_OFF		 		0

#define WL_PREAUTH_ON			1
#define WL_PREAUTH_OFF			0

#define BAND_A                   1
#define BAND_B                   2

#define WL_CHANSPEC_2G		2
#define WL_CHANSPEC_5G		5

#define WIRELESS_APP_FMT       "Wireless%dCfg"

#define WL_VARS_ID              1
#define WL_MSSID_VARS_NUM_ID    2
#define WL_MSSID_VARS_TBL_ID    3
#define WL_FLT_MAC_NUM_ID       4
#define WL_FLT_MAC_TBL_ID       5
#define WL_WDS_MAC_NUM_ID       6
#define WL_WDS_MAC_TBL_ID       7
#define WL_SCAN_WDS_MAC_NUM_ID  8
#define WL_SCAN_WDS_MAC_TBL_ID  9

#define RESET_WLAN	 "/var/reset.wlan"
#define RELOAD_VARS_WLAN "/var/reloadvars.wlan"

#define MAIN_ADPT_IDX   0

#define MAIN_BSS_IDX	0
#define GUEST_BSS_IDX	1
#define GUEST1_BSS_IDX	2
#define GUEST2_BSS_IDX	3

#define WL0_MAIN_BSS_IDX	       0
#define WL0_GUEST_BSS_IDX	       1
#define WL0_GUEST1_BSS_IDX	2
#define WL0_GUEST2_BSS_IDX	3

#define WL1_MAIN_BSS_IDX	       4
#define WL1_GUEST_BSS_IDX	       5
#define WL1_GUEST1_BSS_IDX	6
#define WL1_GUEST2_BSS_IDX	7

#define MAP_FROM_NVRAM	0
#define MAP_TO_NVRAM	1
#define WL_SES_ENTRY	99
#define MAX_NVPARSE 100
#define SES_WDS_MODE_DISABLED           0 /* disabled */
#define SES_WDS_MODE_AUTO               1 /* dynamic cf/cl selection */
#define SES_WDS_MODE_ENABLED_ALWAYS     2 /* enabled always */
#define SES_WDS_MODE_ENABLED_EXCL       3 /* WDS-only i.e no regular STAs */
#define SES_WDS_MODE_CLIENT             4 /* always wds client mode */
#define REG_MODE_OFF                    0 /* disabled 11h/d mode */
#define REG_MODE_H                      1 /* use 11h mode */
#define REG_MODE_D                      2 /* use 11d mode */

#define WL_OPMODE_AP                    "ap"
#define WL_OPMODE_WDS                   "wds"
#define WL_OPMODE_WET                   "wet"
#define WL_OPMODE_WET_MACSPOOF          "wetmacspoof"
#define WL_PREAMBLE_LONG                "long"
#define WL_PREAMBLE_SHORT               "short"

// TR69 overrides
#define WL_TR69_BEACONTYPE_NONE                             0
#define WL_TR69_BEACONTYPE_BASIC                            1
#define WL_TR69_BEACONTYPE_WPA                              2
#define WL_TR69_BEACONTYPE_11I                              3
#define WL_TR69_BEACONTYPE_BASIC_AND_WPA                    4
#define WL_TR69_BEACONTYPE_BASIC_AND_11I                    5
#define WL_TR69_BEACONTYPE_WPA_AND_11I                      6
#define WL_TR69_BEACONTYPE_BASIC_AND_WPA_AND_11I            7

#define WL_TR69_BASIC_ENCRYPTIONMODE_NONE                   0
#define WL_TR69_BASIC_ENCRYPTIONMODE_WEP                    1

#define WL_TR69_BASIC_AUTHENTICATION_NONE                   0
#define WL_TR69_BASIC_AUTHENTICATION_EAP                    1

#define WL_TR69_WPA_ENCRYPTION_MODES_WEP                    0
#define WL_TR69_WPA_ENCRYPTION_MODES_TKIP                   1
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_TKIP           2
#define WL_TR69_WPA_ENCRYPTION_MODES_AES                    3
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_AES            4
#define WL_TR69_WPA_ENCRYPTION_MODES_TKIP_AND_AES           5
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_TKIP_AES       6

#define WL_TR69_WPA_AUTHENTICATION_MODE_PSK                 0
#define WL_TR69_WPA_AUTHENTICATION_MODE_EAP                 1

#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP                0
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_TKIP               1
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_TKIP       2
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_AES                3
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_AES        4
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_TKIP_AND_AES       5
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_TKIP_AES   6

#define WL_TR69_IEEE11I_AUTHENTICATION_MODE_PSK             0
#define WL_TR69_IEEE11I_AUTHENTICATION_MODE_EAP             1

/* mimo */
#define WL_CTL_SB_LOWER         -1
#define WL_CTL_SB_NONE           0
#define WL_CTL_SB_UPPER          1

/* notify the average dma xfer rate (in kbps) to the driver */
#define AVG_DMA_XFER_RATE 100000

#define WL_N_BW_20ALL               0
#define WL_N_BW_40ALL               1
#define WL_N_BW_20IN2G_40IN5G       2
#define WL_V_BW_80IN5G              3

#define WL_LEGACY_MSSID_NUMBER		2

#define WL_IFC_ENABLED			"wlEnbl_wl0v0"
#define WL_DEFAULT_IFC			"wl0"

typedef enum {
   WL_STS_OK = 0,
   WL_STS_ERR_GENERAL,
   WL_STS_ERR_MEMORY,
   WL_STS_ERR_OBJECT_NOT_FOUND
} WL_STATUS;

typedef struct {
   char macAddress[WL_MID_SIZE_MAX];
   char associated;
   char authorized;
   char ssid[WL_SSID_SIZE_MAX];
   char ifcName[WL_SM_SIZE_MAX];
} WL_STATION_LIST_ENTRY, *PWL_STATION_LIST_ENTRY;

typedef struct wl_flt_mac_entry {
   struct wl_flt_mac_entry *next;
   char macAddress[WL_MID_SIZE_MAX];
   char ssid[WL_SSID_SIZE_MAX];
   char ifcName[WL_SM_SIZE_MAX];
   int privacy;
   int wps;
   int wpsConfigured;
} WL_FLT_MAC_ENTRY;
#endif // endif

typedef enum {
   WL_SETUP_ALL = 0,
   WL_SETUP_BASIC,
   WL_SETUP_SECURITY,
   WL_SETUP_MAC_FILTER,
   WL_SETUP_WDS,
   WL_SETUP_ADVANCED,
   WL_SETUP_SES /*SUPPORT_SES*/
} WL_SETUP_TYPE;

typedef struct {
   int wlSsidIdx;
   char wlMode[WL_SM_SIZE_MAX];
   char wlCountry[WL_SM_SIZE_MAX];
   int wlRegRev;
   char wlFltMacMode[WL_SM_SIZE_MAX];
   char wlPhyType[WL_SIZE_2_MAX];
   char wlBasicRate[WL_SIZE_8_MAX];
   char wlProtection[WL_SIZE_8_MAX];
   char wlPreambleType[WL_SM_SIZE_MAX];
   char wlAfterBurnerEn[WL_SIZE_8_MAX];
   char wlFrameBurst[WL_SIZE_4_MAX];
   char wlWlanIfName[WL_SIZE_4_MAX];
   char wlWds[WL_WDS_NUM][WL_MID_SIZE_MAX];
   int  wlWdsSec;
   char wlWdsKey[WL_SM_SIZE_MAX];
   int  wlWdsSecEnable;
   int  wlCoreRev;
   int  wlEnbl;
   int  wlChannel;
   int  wlFrgThrshld;
   int  wlRtsThrshld;
   int  wlDtmIntvl;
   int  wlBcnIntvl;
   long wlRate;
   int  wlgMode;
   int  wlLazyWds;
   int  wlBand;
   int  wlMCastRate;
   int  ezc_version;
   int  ezc_enable;
   int  wlInfra;
   int  wlAntDiv;
   int	wlWme;
   int  wlWmeNoAck;
   int  wlWmeApsd;
   int  wlTxPwrPcnt;
   int  wlRegMode;
   int  wlDfsPreIsm;
   int  wlDfsPostIsm;
   int  wlTpcDb;
   int  wlCsScanTimer;
   int  wlGlobalMaxAssoc;
#ifdef SUPPORT_SES
   int	wlSesEnable;
   int  wlSesEvent;
   char wlSesStates[WL_SIZE_8_MAX];
   char wlSesSsid[WL_SSID_SIZE_MAX];
   char wlSesWpaPsk[WL_WPA_PSK_SIZE_MAX];
   int  wlSesHide;
   int  wlSesAuth;
   char wlSesAuthMode[WL_SIZE_8_MAX];
   char wlSesWep[WL_SM_SIZE_MAX];
   char wlSesWpa[WL_SM_SIZE_MAX];
   int  wlSesWdsMode;
   int	wlSesClEnable;
   int  wlSesClEvent;
   char wlWdsWsec[WL_SIZE_132_MAX];
#endif // endif
#ifdef SUPPORT_MIMO
   int wlNBwCap;
   int wlNCtrlsb;
   int wlNBand;
   int wlNMcsidx;
   char wlNProtection[WL_SIZE_8_MAX];
   char wlRifs[WL_SIZE_8_MAX];
   char wlAmpdu[WL_SIZE_8_MAX];
   char wlAmsdu[WL_SIZE_8_MAX];
   char wlNmode[WL_SIZE_8_MAX];
   int wlNReqd;
   int wlStbcTx;
   int wlStbcRx;
#endif // endif
   int wlRifsAdvert;
   int wlChanImEnab;
   int wlObssCoex;
   int wlRxChainPwrSaveEnable;
   int wlRxChainPwrSaveQuietTime;
   int wlRxChainPwrSavePps;
   int wlRadioPwrSaveEnable;
   int wlRadioPwrSaveQuietTime;
   int wlRadioPwrSavePps;
   int wlRadioPwrSaveLevel;
   int wlEnableUre; /* 0: disabled, 1: bridge (Range Extender), 2: routed (Travel Router) */
   int wlStaRetryTime;
   int wlTXBFCapable;
   int wlEnableBFR;
   int wlEnableBFE;
   int bsdRole;
   int bsdHport;
   int bsdPport;
   char bsdHelper[WL_MID_SIZE_MAX];
   char bsdPrimary[WL_MID_SIZE_MAX];
   int ssdEnable;
   int wlTafEnable;
   int wlAtf;
   int wlPspretendThreshold;
   int wlPspretendRetryLimit;
   int wlAcsFcsMode;
   int wlAcsDfs;
   int wlAcsCsScanTimer;
   int wlAcsCiScanTimer;
   int wlAcsCiScanTimeout;
   int wlAcsScanEntryExpire;
   int wlAcsTxIdleCnt;
   int wlAcsChanDwellTime;
   int wlAcsChanFlopPeriod;
   int wlIntferPeriod;
   int wlIntferCnt;
   int wlIntferTxfail;
   int wlIntferTcptxfail;
   char wlAcsDfsrImmediate[WL_MID_SIZE_MAX];
   char wlAcsDfsrDeferred[WL_MID_SIZE_MAX];
   char wlAcsDfsrActivity[WL_MID_SIZE_MAX];
   unsigned long GPIOOverlays;
} WIRELESS_VAR, *PWIRELESS_VAR;

/* Two wepkey and index could be merged later
    char wlKeys128[WL_KEY_NUM][WL_MID_SIZE_MAX];
    char wlKeys64[WL_KEY_NUM][WL_SM_SIZE_MAX];
*/
typedef struct {
   char wlSsid[WL_SSID_SIZE_MAX];
   char wlBrName[WL_MID_SIZE_MAX];
   char wlKeys128[WL_KEY_NUM][WL_MID_SIZE_MAX];
   char wlKeys64[WL_KEY_NUM][WL_SM_SIZE_MAX];
   char wlWpaPsk[WL_WPA_PSK_SIZE_MAX];
   char wlRadiusKey[WL_RADIUS_KEY_SIZE_MAX];
   char wlWep[WL_SM_SIZE_MAX];
   char wlWpa[WL_SM_SIZE_MAX];
   struct in_addr wlRadiusServerIP;
   char wlAuthMode[WL_SM_SIZE_MAX];
   int  wlWpaGTKRekey;
   int  wlRadiusPort;
   int  wlAuth;
   int  wlEnblSsid;
   int  wlKeyIndex128;
   int  wlKeyIndex64;
   int  wlKeyBit;
   int  wlPreauth;
   int  wlNetReauth;
   int  wlNasWillrun; /*runtime*/
   int  wlHide;
   int  wlAPIsolation;
   int  wlMaxAssoc;
   int  wlDisableWme; /* this is per ssid advertisement in beacon/probe_resp */
   char wlFltMacMode[WL_SM_SIZE_MAX];
#ifdef SUPPORT_TR69C
   int  tr69cBeaconType;
   int  tr69cBasicEncryptionModes;
   int  tr69cBasicAuthenticationMode;
   int  tr69cWPAEncryptionModes;
   int  tr69cWPAAuthenticationMode;
   int  tr69cIEEE11iEncryptionModes;
   int  tr69cIEEE11iAuthenticationMode;
#endif // endif
#ifdef SUPPORT_WSC
   char wsc_mode[12]; // enabled or disabled
   char wsc_config_state[4]; /*0: unconfig 1:Configed*/
#endif // endif
#ifdef WMF
   int wlEnableWmf;
#endif // endif
   WL_FLT_MAC_ENTRY     *m_tblFltMac;
   int wlEnableHspot; /* 0:disabled, 1:enabled , 2:hspot is not supported*/
   int wlMFP;         /* 0:disabled, 1:enabled, 2:required */
   int wlSsdType;
} WIRELESS_MSSID_VAR, *PWIRELESS_MSSID_VAR;

#define MSSID_VAROFF(vv) (offsetof(WIRELESS_MSSID_VAR,vv))

static inline char * MSSID_STRVARVALUE(char *p,int off)  { return (char *) (((char *)p)+off); }
static inline int  MSSID_INTVARVALUE(char *p,int off) { return *((int *) (((char *)p)+off)); }

typedef struct {
   char *varName;
   char *varValue;
} WIRELESS_ITEM, *PWIRELESS_ITEM;

#endif // endif
