/*  
<:copyright-BRCM:2016:proprietary:standard 

   Copyright (c) 2016 Broadcom 
   All Rights Reserved

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
*/
#ifndef __WL_DSL_TR_H__
#define __WL_DSL_TR_H__
typedef unsigned char uint8;
typedef enum {
    Wlan_TxRate_Auto     = 0,         // a/b/g
    Wlan_TxRate_1Mbps    = 10000000,    // b/g
    Wlan_TxRate_2Mbps    = 20000000,    // b/g
    Wlan_TxRate_5_5Mbps  = 55000000,    // b/g
    Wlan_TxRate_6Mbps    = 60000000,    // a/g
    Wlan_TxRate_9Mbps    = 90000000,    // a/g
    Wlan_TxRate_11Mbps   = 11000000,    // b/g
    Wlan_TxRate_12Mbps   = 12000000,    // a/g
    Wlan_TxRate_18Mbps   = 18000000,    // a/g
    Wlan_TxRate_24Mbps   = 24000000,    // a/g
    Wlan_TxRate_36Mbps   = 36000000,    // a/g
    Wlan_TxRate_48Mbps   = 48000000,    // a/g
    Wlan_TxRate_54Mbps   = 54000000     // a/g
} Wlan_TxRate;

typedef enum {
    Wlan_BeaconType_None,
    Wlan_BeaconType_Basic,	
    Wlan_BeaconType_WPA,
    Wlan_BeaconType_11i,
    Wlan_BeaconType_BasicandWPA,
    Wlan_BeaconType_Basicand11i,
    Wlan_BeaconType_WPAand11i,
    Wlan_BeaconType_BasicandWPAand11i
} Wlan_BeaconType;

// Wireless 802.1x Authentication
typedef enum {
    WlanAuthMode_None,
    WlanAuthMode_Radius,
} Wlan_AuthMode;

// WepEnable
typedef enum {
    WepDisabled,
    WepEnabled
} WepEnable;

// Wireless Authentication
typedef enum {
    WlanAuth_Open,
    WlanAuth_Shared
} Wlan_Auth;

//Wlan_BasicEncryptionModes
typedef enum {
    Wlan_BasicEncryptionModes_None,
    Wlan_BasicEncryptionModes_WEPEncryption,    
} Wlan_BasicEncryptionModes;


//Wlan_BasicAuthenticationMode
typedef enum {
    Wlan_BasicAuthenticationMode_None,
    Wlan_BasicAuthenticationMode_EAPAuthentication,
    Wlan_BasicAuthenticationMode_SharedAuthentication,
} Wlan_BasicAuthenticationMode;

//Wlan_WPAEncryptionModes
typedef enum {
    Wlan_WPAEncryptionModes_WEPEncryption,
    Wlan_WPAEncryptionModes_TKIPEncryption,
    Wlan_WPAEncryptionModes_WEPandTKIPEncryption,
    Wlan_WPAEncryptionModes_AESEncryption,
    Wlan_WPAEncryptionModes_WEPandAESEncryption,
    Wlan_WPAEncryptionModes_TKIPandAESEncryption,
    Wlan_WPAEncryptionModes_WEPandTKIPandAESEncryption
} Wlan_WPAEncryptionModes;

//Wlan_WPAAuthenticationMode
typedef enum {
    Wlan_WPAAuthenticationMode_PSKAuthentication,
    Wlan_WPAAuthenticationMode_EAPAuthentication
} Wlan_WPAAuthenticationMode;


//Wlan_IEEE11iEncryptionModes
typedef enum {
    Wlan_IEEE11iEncryptionModes_WEPEncryption,
    Wlan_IEEE11iEncryptionModes_TKIPEncryption,
    Wlan_IEEE11iEncryptionModes_WEPandTKIPEncryption,
    Wlan_IEEE11iEncryptionModes_AESEncryption,
    Wlan_IEEE11iEncryptionModes_WEPandAESEncryption,
    Wlan_IEEE11iEncryptionModes_TKIPandAESEncryption,
    Wlan_IEEE11iEncryptionModes_WEPandTKIPandAESEncryption
} Wlan_IEEE11iEncryptionModes;

//Wlan_IEEE11iAuthenticationMode
typedef enum {
    Wlan_IEEE11iAuthenticationMode_PSKAuthentication,
    Wlan_IEEE11iAuthenticationMode_EAPAuthentication,
    Wlan_IEEE11iAuthenticationMode_EAPandPSKAuthentication
} Wlan_IEEE11iAuthenticationMode;


// Wireless Authentication Key Management
typedef struct {
    uint8
    Wlan_WpaAKm_None:1,
    Wlan_WpaAKm_Unspecified:1, 
    Wlan_WpaAKm_Psk:1, 
    reserved:3,
    Wlan_Wpa2AKm_Unspecified:1,
    Wlan_Wpa2AKm_Psk:1;
} Wlan_WpaAKmBit;

typedef struct {
    union {
        uint8 val;
        Wlan_WpaAKmBit bitVal;
    } u;	
} Wlan_WpaAKm;

// Wireless Encryption
typedef enum {
    Wlan_Crypto_Tkip,
    Wlan_Crypto_Aes,
    Wlan_Crypto_Tkip_Aes
} Wlan_Crypto;

struct wl_dsl_tr_mbss_struct {
    char        GuestSSID[WL_SSID_SIZE_MAX];/* JXUJXU-#503567 changed 32 to WL_SSID_SIZE_MAX which is defined as 48 */	/**< X_BROADCOM_COM_GuestSSID */
    char        GuestBSSID[32];	/**< X_BROADCOM_COM_GuestBSSID */
    bool        GuestEnable;	/**< X_BROADCOM_COM_GuestEnable */
    bool        GuestHiden;	/**< X_BROADCOM_COM_GuestHiden */
    bool        GuestIsolateClients;	/**< X_BROADCOM_COM_GuestIsolateClients */
    bool        GuestDisableWMMAdvertise;	/**< X_BROADCOM_COM_GuestDisableWMMAdvertise */
    SINT32      GuestMaxClients;	/**< X_BROADCOM_COM_GuestMaxClients */
};
/* This is tr69 data structure, matching to the data-model definition*/
struct  wl_dsl_tr_struct {
    bool       enable;	/**< Enable */
    char       status[16];	/**< Status */
    char       BSSID[20];	/**< BSSID */
    char       maxBitRate[8];	/**< MaxBitRate */
    UINT32   channel;	/**< Channel */
    char       SSID[WL_SSID_SIZE_MAX];	/**< SSID */
    char       beaconType[64];	/**< BeaconType */
    bool       MACAddressControlEnabled;	/**< MACAddressControlEnabled */
    char       standard[8];	/**< Standard */
    UINT32   WEPKeyIndex;	/**< WEPKeyIndex */
    char       keyPassphrase[64];	/**< KeyPassphrase */
    char       WEPEncryptionLevel[64];	/**< WEPEncryptionLevel */
    char       basicEncryptionModes[32];	/**< BasicEncryptionModes */
    char       basicAuthenticationMode[32];	/**< BasicAuthenticationMode */
    char       WPAEncryptionModes[32];	/**< WPAEncryptionModes */
    char       WPAAuthenticationMode[32];	/**< WPAAuthenticationMode */
    char       IEEE11iEncryptionModes[32];	/**< IEEE11iEncryptionModes */
    char       IEEE11iAuthenticationMode[32];	/**< IEEE11iAuthenticationMode */
    char       possibleChannels[1024];	/**< PossibleChannels */
    char       basicDataTransmitRates[256];	/**< BasicDataTransmitRates */
    char       operationalDataTransmitRates[256];	/**< OperationalDataTransmitRates */
    char       possibleDataTransmitRates[256];	/**< PossibleDataTransmitRates */
    bool       insecureOOBAccessEnabled;	/**< InsecureOOBAccessEnabled */
    bool       beaconAdvertisementEnabled;	/**< BeaconAdvertisementEnabled */
    bool       radioEnabled;	/**< RadioEnabled */
    bool       autoRateFallBackEnabled;	/**< AutoRateFallBackEnabled */
    char       locationDescription[4096];	/**< LocationDescription */
    char       regulatoryDomain[4];	/**< RegulatoryDomain */
    UINT32   totalPSKFailures;	/**< TotalPSKFailures */
    UINT32   totalIntegrityFailures;	/**< TotalIntegrityFailures */
    char       channelsInUse[1024];	/**< ChannelsInUse */
    char       deviceOperationMode[32];	/**< DeviceOperationMode */
    UINT32    distanceFromRoot;	/**< DistanceFromRoot */
    char        peerBSSID[20];	/**< PeerBSSID */
    char        authenticationServiceMode[32];	/**< AuthenticationServiceMode */
    UINT32    totalBytesSent;	/**< TotalBytesSent */
    UINT32    totalBytesReceived;	/**< TotalBytesReceived */
    UINT32    totalPacketsSent;	/**< TotalPacketsSent */
    UINT32    totalPacketsReceived;	/**< TotalPacketsReceived */
    UINT32    totalAssociations;	/**< TotalAssociations */

/*Private fields X_BROADCOM_COM: 
It would be nice to have all these private fields to be struct array and Data-Model to be refined*/
    UINT32   X_BROADCOM_COM_RxErrors;
    UINT32   X_BROADCOM_COM_RxDrops;
    UINT32   X_BROADCOM_COM_TxErrors;
    UINT32   X_BROADCOM_COM_TxDrops;

   char        X_BROADCOM_COM_IfName[16];
    bool        X_BROADCOM_COM_HideSSID;	/**< X_BROADCOM_COM_HideSSID */
    UINT32    X_BROADCOM_COM_TxPowerPercent;	/**< X_BROADCOM_COM_TxPowerPercent */

    struct wl_dsl_tr_mbss_struct GuestMbss[WL_MAX_NUM_SSID-1]; 
};

struct wl_dsl_tr_wepkey_struct {
    char    WEPKey[4][128];
};


struct wl_dsl_tr_presharedkey_struct {
    char    PreSharedKey[1][128];
};

/* For legacy structure casting */
struct mdm_object_mbss_struct {
    char *    X_BROADCOM_COM_GuestSSID;	
    char *    X_BROADCOM_COM_GuestBSSID;   
    UBOOL8    X_BROADCOM_COM_GuestEnable;
    UBOOL8    X_BROADCOM_COM_GuestHiden;	
    UBOOL8    X_BROADCOM_COM_GuestIsolateClients;	
    UBOOL8    X_BROADCOM_COM_GuestDisableWMMAdvertise;	
    SINT32    X_BROADCOM_COM_GuestMaxClients;	
};

extern struct wl_dsl_tr_presharedkey_struct *wl_dsl_tr_presharedkey;
extern struct wl_dsl_tr_struct *wl_dsl_tr;
extern struct wl_dsl_tr_wepkey_struct *wl_dsl_tr_wepkey;


void wl_dsl_tr_get_brcm_beaconType( char *wlAuthMode, char *value );
void wl_dsl_tr_get_maxBitRate(int wlBand, long wlRate, int wlgMode, char *value);
void wl_dsl_tr_set_maxBitRate(char *maxBitRate, int wlBand, int wlgMode, long *wlRate);
void wl_dsl_tr_get_standard(int wlBand, int wlgMode, char *value);
void wl_dsl_tr_get_basicDataTransmitRates(int wlBand, int wlgMode, char *wlBasicRate, char *value);
void wl_dsl_tr_set_basicDataTransmitRates(char *basicDataTransmitRates, int wlBand, int wlgMode, char *wlBasicRate);
void wl_dsl_tr_get_possibleChannels( char *value, int idx );
void wl_dsl_tr_set_radioEnabled( int value, int idx );
void wl_dsl_tr_get_radioEnabled( int  *value, int idx );
void wldsltr_set(int idx);
void wldsltr_get(int idx);

int wldsltr_alloc( int adapter_cnt );
void wldsltr_free(void );
#endif


