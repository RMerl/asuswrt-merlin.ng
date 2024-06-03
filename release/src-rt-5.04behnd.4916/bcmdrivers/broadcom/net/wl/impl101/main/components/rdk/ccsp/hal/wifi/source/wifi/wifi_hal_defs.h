/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2015] [Comcast, Corp.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/
#ifndef __WIFI_HAL_DEFS_H__
#define __WIFI_HAL_DEFS_H__

#ifndef XSTR
#define XSTR(s) STR(s)
#define STR(s) #s
#endif

#if (WIFI_HAL_MAJOR_VERSION > 3) || (WIFI_HAL_MAJOR_VERSION == 3 && \
	(WIFI_HAL_MINOR_VERSION  >= 1 || (WIFI_HAL_MINOR_VERSION  >= 0 && \
	WIFI_HAL_MAINTENANCE_VERSION >= 3)))
#define WIFI_HAL_VERSION_GE_3_0_3       1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 3) || (WIFI_HAL_MAJOR_VERSION == 3 && \
	(WIFI_HAL_MINOR_VERSION  >= 1 || (WIFI_HAL_MINOR_VERSION  >= 0 && \
	WIFI_HAL_MAINTENANCE_VERSION >= 2)))
#define WIFI_HAL_VERSION_GE_3_0_2       1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 3) || (WIFI_HAL_MAJOR_VERSION == 3 && \
	(WIFI_HAL_MINOR_VERSION  >= 1 || (WIFI_HAL_MINOR_VERSION  == 0 && \
	WIFI_HAL_MAINTENANCE_VERSION >= 1)))
#define WIFI_HAL_VERSION_GE_3_0_1	1
#pragma message "WIFI_HAL_VERSION_GE_3_0_1 in use"
#endif

#if defined(WIFI_HAL_VERSION_3) || (WIFI_HAL_MAJOR_VERSION > 2)
#define WIFI_HAL_VERSION_GE_3_0		1
#pragma message "WIFI_HAL_VERSION_GE_3_0 in use"
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  > 19)
#define WIFI_HAL_VERSION_GT_2_19	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 19)
#define WIFI_HAL_VERSION_GE_2_19	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 18)
#define WIFI_HAL_VERSION_GE_2_18	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 17)
#define WIFI_HAL_VERSION_GE_2_17	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 16)
#define WIFI_HAL_VERSION_GE_2_16	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 15)
#define WIFI_HAL_VERSION_GE_2_15	1
#endif

#if (WIFI_HAL_MAJOR_VERSION > 2) || (WIFI_HAL_MAJOR_VERSION == 2 && \
	WIFI_HAL_MINOR_VERSION  >= 12)
#define WIFI_HAL_VERSION_GE_2_12	1
#endif

#ifdef MAX_NUM_RADIOS
#undef MAX_NUM_RADIOS
#endif /* MAX_NUM_RADIOS */

#if !defined(_CBR_PRODUCT_REQ_)
#define MAX_NUM_RADIOS  3
#else
#define MAX_NUM_RADIOS	2
#endif

#define HAL_RADIO_NUM_RADIOS	MAX_NUM_RADIOS

/* from wl_cfg80211.h */
#define CHANNEL_IS_2G(channel)  (((channel >= 1) && (channel <= 14)) ? \
        TRUE : FALSE)
#define CHANNEL_IS_5G(channel)  (((channel >= 36) && (channel <= 173)) ? \
        TRUE : FALSE)

#define TIME_FMT_UTC "%a %b %d %H:%M:%S %Y : "	/* e.g Thu Sep 22 12:28:40 2022 : */
#define TIME_FMT_ISO "%Y-%m-%d %H:%M:%S "	/* e.g 2022-09-22 12:28:40 */

#define HAL_WIFI_TMPFILE		"/nvram/wifidbg1"
#define HAL_WIFI_VENDOR_LOG_FILE	"/rdklogs/logs/wifi_vendor_hal.log"
/* Note:
 *  HAL_WIFI_HEALTH_FILE is currently used only by WPS
 *  Unique markers with specific time format are saved there
 *  (e.g) 2022-09-22 10:46:13 RDKB_WPS_ENABLED_1 TRUE
 *  No other info such as function name is included
 */
#define HAL_WIFI_HEALTH_FILE		"/rdklogs/logs/wifihealth.txt"

extern void hal_log_file(const char *logfile, char *time_fmt, const char *fmt, ...);

#define HAL_WIFI_HEALTH(fmt, arg...) hal_log_file(HAL_WIFI_HEALTH_FILE, TIME_FMT_ISO, fmt, ##arg)
#define WPS_LOG HAL_WIFI_HEALTH

#define hal_wifi_log_arg(fmt, arg...)   hal_log_file(HAL_WIFI_VENDOR_LOG_FILE, TIME_FMT_UTC, fmt, ##arg)
#define HAL_WIFI_DBG(x) do {if (access(HAL_WIFI_TMPFILE, 4) != -1) hal_wifi_log_arg x;} \
			while (0);
#define HAL_WIFI_LOG(x) do {hal_wifi_log_arg x;} while (0);
#define HAL_WIFI_ERR(x) HAL_WIFI_LOG(x)

#ifndef MAX_NUM_VAP_PER_RADIO
#define MAX_NUM_VAP_PER_RADIO	8
#endif

#define HAL_WIFI_TOTAL_NO_OF_APS MAX_NUM_RADIOS * MAX_NUM_VAP_PER_RADIO

#pragma message "MAX_WLAN_ADAPTER is " XSTR(MAX_WLAN_ADAPTER)
#pragma message "MAX_NUM_RADIOS is " XSTR(MAX_NUM_RADIOS)
#pragma message "MAX_NUM_VAP_PER_RADIO is " XSTR(MAX_NUM_VAP_PER_RADIO)
#pragma message "HAL_WIFI_TOTAL_NO_OF_APS is " XSTR(HAL_WIFI_TOTAL_NO_OF_APS)

#define MACF				"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_TO_MACF(addr)		addr[0], \
					addr[1], \
					addr[2], \
					addr[3], \
					addr[4], \
					addr[5]

#define MACF_TO_MAC(macstr, mac) \
	sscanf(macstr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", \
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])

#define WIFI_LANBRIDGE_UP          0x1003             /* interface is up              */
#define WIFI_LANBRIDGE_DOWN        0x1002               /* interface is down          */

/* Should not change the port number as this port is used by 'wlevt' to send the event */
#define AP_ASSOC_DISASSOC_PORT 52300

/* two minutes */
#define WPS_PAIRING_TIMEOUT 58
#define HAL_WPS_INIT 0
#define HAL_WPS_OK 2

#define WLAN_CM_RDK_RRM_LOG_FILE_NVRAM "rrm_rdk_log_file"
#define WLAN_CM_RDK_RRM_LOG_FILE "/tmp/rdk_log_rrm_11k.txt"
#define RDKB_RRM_STRING_LENGTH 64

/* From 802.11.h */
/* Management Frame Information Element IDs */
#define DOT11_MNG_MEASURE_REQUEST_ID   38      /* 11H MeasurementRequest */
#define DOT11_MNG_MEASURE_REPORT_ID    39      /* 11H MeasurementReport */
#define DOT11_MEASURE_TYPE_BEACON       5   /* d11 measurement Beacon type */
/* Sub-element IDs for Beacon Report */
#define DOT11_RMREP_BCN_FRM_BODY        1
#define DOT11_RMREP_BCN_FRM_BODY_LEN_MAX        224 /* 802.11k-2008 7.3.2.22.6 */

#define DOT11_NEIGHBOR_REP_IE_FIXED_LEN 13
#define DOT11_MNG_NEIGHBOR_REP_ID       52
#define TLV_HDR_LEN             2       /* header length */

#define OUTPUT_STRING_LENGTH_16     16
#define OUTPUT_STRING_LENGTH_18     18
#define OUTPUT_STRING_LENGTH_32     32
#define OUTPUT_STRING_LENGTH_64     64
#define OUTPUT_STRING_LENGTH_128    128
#define OUTPUT_STRING_LENGTH_256    256
#define OUTPUT_STRING_LENGTH_512    512
#define OUTPUT_STRING_LENGTH_1024   1024
#define OUTPUT_STRING_LENGTH_2048   2048
#define IP_ADDR_LEN                 45

#define HAL_RADIO_STARTING_INDEX    0
#define HAL_AP_STARTING_INDEX       0 /* ?? */
#define HAL_AP_NUM_APS_PER_RADIO    8

#define CSI_DELAY_PERIOD           100	/* CSI collection period: 100ms */

#define BUF_LEN_26 26
#define NEIGHBOR_SECURITY_MODE_MAX 8
#define MAX_OPERATING_BANDWIDTH 5

#define WIFI_MAC_ADDRESS_LENGTH 6
#define HAL_AP_IDX_TO_HAL_RADIO(apIdx)  ((apIdx < (2 * HAL_AP_NUM_APS_PER_RADIO)) ? \
        (apIdx % 2) : (apIdx / HAL_AP_NUM_APS_PER_RADIO))

#ifdef RDKB_ONE_WIFI
#define HAL_AP_IDX_TO_SSID_IDX(apIdx)  ((apIdx < (2 * HAL_AP_NUM_APS_PER_RADIO)) ? \
	(((apIdx / 2) == 7) ? 0 : ((apIdx / 2) + 1)) : ((apIdx + 1) % HAL_AP_NUM_APS_PER_RADIO))
#define WL_DRIVER_TO_AP_IDX(idx, subidx) ((idx < 2) ? \
	((subidx == 0) ? (2 * (HAL_AP_NUM_APS_PER_RADIO - 1) + idx + 1) : (idx  + (subidx - 1) * 2 + 1)) : \
	((idx * HAL_AP_NUM_APS_PER_RADIO) + (subidx ? subidx : HAL_AP_NUM_APS_PER_RADIO)))
#define HAL_RADIO_IDX_TO_HAL_AP(radioIdx) (WL_DRIVER_TO_AP_IDX(radioIdx, 0) - 1)
#else
#define HAL_AP_IDX_TO_SSID_IDX(apIdx)  ((apIdx < (2 * HAL_AP_NUM_APS_PER_RADIO)) ? \
	(apIdx / 2) : (apIdx % HAL_AP_NUM_APS_PER_RADIO))
#define WL_DRIVER_TO_AP_IDX(idx, subidx) ((idx < 2) ? (idx + subidx * 2 + 1) : \
	((idx * HAL_AP_NUM_APS_PER_RADIO) + subidx + 1))
#define HAL_RADIO_IDX_TO_HAL_AP(radioIdx) ((radioIdx < 2) ? radioIdx : \
	(radioIdx * HAL_AP_NUM_APS_PER_RADIO))
#endif /* RDKB_ONE_WIFI */

#define WIFIACCESSPOINTSECURITY_MODEENABLED_MAX					64
#define WIFISYSTEM_NUMSSIDSPERRADIO						8
#define VOID void

#ifndef WIFI_HAL_INVALID_ARGUMENTS
#define WIFI_HAL_INVALID_ARGUMENTS -4
#endif
//typedef unsigned int ULONG;
//typedef unsigned int UINT;
typedef void *PVOID;
typedef ULONG *PULONG;
//typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char BYTE;
typedef BYTE *PBYTE;
//typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef unsigned char BOOLEAN;
typedef BOOLEAN *PBOOLEAN;
//typedef int INT;
typedef INT *PINT;
typedef unsigned int UINT32;
typedef UINT32 *PUINT32;
typedef char INT8;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned short uint16;
typedef short INT16;
typedef int INT32;
typedef INT32 STATUS;
typedef char * PCHAR;
//typedef char CHAR;
typedef const char * PCONSTCHAR;
typedef const char * const PCONSTCHARCONST;

#define MAX_REGISTERED_CB_NUM  48

#ifndef AP_PREFIX
#define AP_PREFIX   "ath"
#endif

#ifndef RADIO_PREFIX
#define RADIO_PREFIX    "wifi"
#endif

#ifndef BOOLEAN
typedef BOOLEAN       bool;
#endif

#ifndef ETHER_ISNULLADDR
#define ETHER_ISNULLADDR(ea)    ((((const uint8 *)(ea))[0] |            \
                                  ((const uint8 *)(ea))[1] |            \
                                  ((const uint8 *)(ea))[2] |            \
                                  ((const uint8 *)(ea))[3] |            \
                                  ((const uint8 *)(ea))[4] |            \
                                  ((const uint8 *)(ea))[5]) == 0)

#endif

typedef enum
{
   WIFI_LED_OFF_CONTINUOUS = 0,
   WIFI_LED_ON_CONTINUOUS,
   WIFI_LED_SLOW_CONTINUOUS_BLINK,
   WIFI_LED_MEDIUM_CONTINUOUS_BLINK,
   WIFI_LED_FAST_CONTINUOUS_BLINK,
   WIFI_LED_OFF_AND_FAST_BLINK,
   WIFI_LED_OFF_AND_SLOW_BLINK,
   WIFI_LED_MAX_BLINK
} wifi_led_blink_state_t;

#ifndef WIFI_HAL_VERSION_3  /* Redefinition */
typedef struct wl_event_rx_frame_data {
        uint16_t  version;
        uint16_t  channel;        /* Matches chanspec_t format from bcmwifi_channels.h */
        int32_t   rssi;
        uint32_t  mactime;
        uint32_t  rate;
} __attribute__((__packed__)) wl_event_rx_frame_data_t;
#endif /* End of Redefinition */

#ifndef WL_CHANSPEC_CHAN_MASK
#define WL_CHANSPEC_CHAN_MASK		0x00ffu
#endif

#ifndef WIFI_HAL_VERSION_3 /* Redefinition */
typedef enum {
	WL_CHAN_REASON_CSA = 0,
	WL_CHAN_REASON_DFS_AP_MOVE_START = 1,
	WL_CHAN_REASON_DFS_AP_MOVE_RADAR_FOUND = 2,
	WL_CHAN_REASON_DFS_AP_MOVE_ABORTED = 3,
	WL_CHAN_REASON_DFS_AP_MOVE_SUCCESS = 4,
	WL_CHAN_REASON_DFS_AP_MOVE_STUNT = 5,
	WL_CHAN_REASON_DFS_AP_MOVE_STUNT_SUCCESS = 6,
	WL_CHAN_REASON_CSA_TO_DFS_CHAN_FOR_CAC_ONLY = 7, /* Support Co-ordinated CAC for MAP R2 */
	WL_CHAN_REASON_ANY = 8
} wl_chan_change_reason_t;

typedef struct wl_event_change_chan {
	uint16_t version;
	uint16_t length;		/* excluding pad field */
	wl_chan_change_reason_t reason;	/* CSA or DFS_AP_MOVE */
	uint16_t target_chanspec;
	uint16_t pad;			/* 4-byte alignment */
} wl_event_change_chan_t;
#endif /* End of Redefinition */

typedef struct wl_radar_detected_info {
	uint8_t radar_type;		/* one of RADAR_TYPE_XXX */
	uint16_t min_pw;		/* minimum pulse-width (usec * 20) */
	uint16_t max_pw;		/* maximum pulse-width (usec * 20) */
	uint16_t min_pri;		/* minimum pulse repetition interval (usec) */
	uint16_t max_pri;		/* maximum pulse repetition interval (usec) */
	uint16_t subband;		/* subband/frequency */

} wl_radar_detected_info_t;

typedef struct wl_event_radar_detect {
	uint32_t version;
	uint16_t current_chanspec;	/* chanspec on which the radar is recieved */
	uint16_t target_chanspec;	/*  Target chspec after detecting radar on current_chspec */
	wl_radar_detected_info_t radar_info[2];
} wl_event_radar_detect_t;

typedef enum
{
    BEACON_TYPE_NONE = 0,
    BEACON_TYPE_BASIC,
    BEACON_TYPE_WPA,
    BEACON_TYPE_11I,
    BEACON_TYPE_WPA_11I,
    BEACON_TYPE_TOTAL
} BEACON_TYPE;

typedef enum
{
    BASIC_ENCRYPTION_MODE_NONE = 0,
    BASIC_ENCRYPTION_MODE_WEP,
    BASIC_ENCRYPTION_MODE_TOTAL
} BASIC_ENCRYPTION_MODE;

typedef enum
{
    BASIC_AUTHENTICATION_MODE_NONE = 0,
    BASIC_AUTHENTICATION_MODE_EAP,
    BASIC_AUTHENTICATION_MODE_SHARED,
    BASIC_AUTHENTICATION_MODE_PSK,
    BASIC_AUTHENTICATION_MODE_TOTAL
} BASIC_AUTHENTICATION_MODE;

typedef enum
{
    WPA_ENCRYPTION_MODE_TKIP = 0,
    WPA_ENCRYPTION_MODE_AES,
    WPA_ENCRYPTION_MODE_TKIP_AES,
    WPA_ENCRYPTION_MODE_TOTAL
} WPA_ENCRYPTION_MODE;

typedef struct {
    char essid[OUTPUT_STRING_LENGTH_32+1];
    wifi_scanFilterMode_t mode;
} scanfilter_t;

// 802.11-2016 section 9.4.2.22
typedef struct _wifi_MeasurementReportElement_t
{   // rep stands for report for lack of a better abbreviation
    UCHAR rep_Id;
    UCHAR rep_Len;
    UCHAR rep_Token;	   // unique among the elements in a frame
    UCHAR rep_ReportMode;  // bit 0 late, bit 1 incapable, bit 2 refused
    UCHAR rep_Type;	   // 5 for Beacon
    // Only one report per element; 802.11-2016 section 9.4.2.22.1.
    UCHAR rep_Report[0];
} wifi_MeasurementReportElement_t;

typedef struct _wifi_BeaconMeasurement_packed_t {
	UCHAR	opClass;
	UCHAR	channel;
	ULLONG	startTime;
	USHORT	duration;
	UCHAR	frameInfo;
	UCHAR	rcpi;
	UCHAR	rsni;
	bssid_t	bssid;
	UCHAR	antenna;
	UINT	tsf;
} __attribute__((packed)) wifi_BeaconMeasurement_packed_t;

typedef struct wifi_enum_to_str_map
{
	unsigned int enum_val;
	const char *str_val;
} wifi_enum_to_str_map_t;

/* type defined in wlc_types.h */
enum wlc_bandunit {
	BAND_2G_INDEX = 0,
	BAND_5G_INDEX,
	BAND_6G_INDEX,
	MAXBANDS
};
#endif /*__WIFI_HAL_DEFS_H__*/
