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

/**  @file wlcsm_defs.h
*    @brief defines used by wlcsm
*/

#ifndef __WLCSM_DEFS_H__
#define __WLCSM_DEFS_H__


#define INTERFACE_WL0_INDEX       0
#define INTERFACE_WL1_INDEX       1

#define WLAN_PREFIX	"wl"
#define IS_WLAN_IFC(x)	(!strncmp(x, WLAN_PREFIX, strlen(WLAN_PREFIX)))

#define WL_VERSION_STR_LEN    (96)
#define WL_CAP_STR_LEN    (1536)

#define WL_SIZE_2_MAX    2
#define WL_SIZE_4_MAX    4
#define WL_SIZE_8_MAX    8
#define WL_SIZE_132_MAX  132
#define WL_SIZE_256_MAX  256
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
#define WL_MACADDR_SIZE  18
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

#define WL_UUID_SIZE_MAX  40
#define WL_NVRAM_VALUE_MAX_LEN 1024
#define WL_DEFAULT_VALUE_SIZE_MAX  255
#define WL_DEFAULT_NAME_SIZE_MAX  36
#define WL_WDS_SIZE_MAX  80
#define WL_MACFLT_NUM 64
#define WL_SINGLEMAC_SIZE 18

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
#ifdef BCMWAPI_WAI
#define WL_AUTH_WAPI          "wapi"
#define WL_AUTH_WAPI_PSK      "wapi_psk"
#define WL_AUTH_WAPI_MIX      "wapi wapi_psk"
#endif

#define WL_WPA_AUTH_DISABLED		0x0000	/* Legacy (i.e., non-WPA) */
#define WL_WPA_AUTH_NONE			0x0001	/* none (IBSS) */
#define WL_WPA_AUTH_UNSPECIFIED		0x0002	/* over 802.1x */
#define WL_WPA_AUTH_PSK				0x0004	/* Pre-shared key */
#define WL_WPA_AUTH_8021X 			0x0020	/* 802.1x, reserved */
#define WL_WPA2_AUTH_UNSPECIFIED	0x0040	/* over 802.1x */
#define WL_WPA2_AUTH_PSK			0x0080	/* Pre-shared key */
#define WL_WSC 0x10
#ifdef BCMWAPI_WAI
#define WL_WAPI_AUTH             0x0400
#define WL_WAPI_AUTH_PSK         0x0800
#endif

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
#define BAND_6G                  4

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


/* mimo */
#define WL_CTL_SB_LOWER         -1
#define WL_CTL_SB_NONE           0
#define WL_CTL_SB_UPPER          1


#define WL_LEGACY_MSSID_NUMBER		2

#define WL_IFC_ENABLED			"wlEnbl_wl0v0"
#define WL_DEFAULT_IFC			"wl0"
typedef enum {
    WL_STS_OK = 0,
    WL_STS_ERR_GENERAL,
    WL_STS_ERR_MEMORY,
    WL_STS_ERR_OBJECT_NOT_FOUND
} WL_STATUS;


#define MAX_WLAN_ADAPTER 4

typedef struct wl_flt_mac_entry {
    struct wl_flt_mac_entry *next;
    char macAddress[WL_MID_SIZE_MAX];
    char ssid[WL_SSID_SIZE_MAX];
    char ifcName[WL_SM_SIZE_MAX];
    int privacy;
    int wps;
    int wpsConfigured;
} WL_FLT_MAC_ENTRY;

typedef struct {
    char macAddress[WL_MID_SIZE_MAX];
    char associated;
    char authorized;
    char radioIndex;
    char ssidIndex;
    char ssid[WL_SSID_SIZE_MAX];
    char ifcName[WL_SM_SIZE_MAX];
} WL_STATION_LIST_ENTRY, *PWL_STATION_LIST_ENTRY;

#define STR_OPEN  "open"
#define STR_RADIUS "radius"
#define STR_NONE  "None"
#define STR_SHARED "shared"
#define STR_ENABLED "enabled"
#define STR_OFF "off"
#define STR_WPA_PERSONAL "WPA-Personal"
#define STR_WPA2_PERSONAL "WPA2-Personal"
#define STR_WPA_WPA2_PERSONAL "WPA-WPA2-Personal"
#define STR_WPA_ENTERPRISE "WPA-Enterprise"
#define STR_WPA2_ENTERPRISE "WPA2-Enterprise"
#define STR_WPA_WPA2_ENTERPRISE "WPA-WPA2-Enterprise"
#define STR_AKM_WPA "wpa"
#define STR_AKM_WPA2 "wpa2"
#define STR_AKM_WPA_WPA2 "wpa wpa2"
#define STR_AKM_PSK "psk"
#define STR_AKM_PSK2 "psk2"
#define STR_AKM_PSK_PSK2 "psk psk2"

#endif
