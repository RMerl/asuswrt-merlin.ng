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
/*This file handles the DSL TR Data-model interface*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "cms.h"
#include "board.h"

#include "cms_util.h"
#include "mdm.h"
#include "mdm_private.h"
#include "odl.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"

#include "cms_core.h"

#include <bcmnvram.h>

#include "wlmngr.h"
#include "wlmdm.h"

#include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include "wllist.h"
#include "wldsltr.h"

struct wl_dsl_tr_struct *wl_dsl_tr;
struct wl_dsl_tr_wepkey_struct *wl_dsl_tr_wepkey;

struct wl_dsl_tr_presharedkey_struct *wl_dsl_tr_presharedkey;
int wldsltr_alloc( int adapter_cnt )
{
    if ( (wl_dsl_tr = calloc( adapter_cnt, sizeof(struct wl_dsl_tr_struct) )) == NULL ) {
        printf("%s@%d alloc wl_dsl_tr[size %zu] Err \n", __FUNCTION__, __LINE__,
               sizeof(struct wl_dsl_tr_struct) * adapter_cnt );
        return -1;
    }


    if ( (wl_dsl_tr_wepkey = calloc( adapter_cnt, sizeof(struct wl_dsl_tr_wepkey_struct) )) == NULL ) {
        printf("%s@%d alloc wl_dsl_tr[size %zu] Err \n", __FUNCTION__, __LINE__,
               sizeof(struct wl_dsl_tr_wepkey_struct) *adapter_cnt );
        return -1;
    }

    if ( (wl_dsl_tr_presharedkey = calloc( adapter_cnt, sizeof(struct wl_dsl_tr_presharedkey_struct) )) == NULL ) {
        printf("%s@%d alloc wl_dsl_tr_presharedkey_struct[size %zu] Err \n", __FUNCTION__, __LINE__,
               sizeof(struct wl_dsl_tr_presharedkey_struct) *adapter_cnt );
        return -1;
    }

    return 0;
}

void wldsltr_free(void )
{
    if ( wl_dsl_tr != NULL )
        free(wl_dsl_tr);

    if ( wl_dsl_tr_wepkey != NULL )
        free(wl_dsl_tr_wepkey);

    if ( wl_dsl_tr_presharedkey != NULL ) {
        free(wl_dsl_tr_presharedkey);
    }

}


/*Convert from main structure to tr69 data structure*/
void wldsltr_get(int idx)
{
    WLAN_ADAPTER_STRUCT *wl_instance = &(m_instance_wl[idx]);
    WIRELESS_VAR *m_wlVarPtr = &(wl_instance->m_wlVar);
    WIRELESS_MSSID_VAR *m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[MAIN_BSS_IDX]);
    struct wl_dsl_tr_struct *wl_dsl_trPtr = &wl_dsl_tr[idx];
    struct wl_dsl_tr_wepkey_struct *wl_dsl_tr_wepkeyPtr = wl_dsl_tr_wepkey+idx;
    struct wl_dsl_tr_presharedkey_struct *wl_dsl_tr_presharedkeyPtr = wl_dsl_tr_presharedkey+idx;

    int i = 0;
    int radio=0;


    /*Start Here*/
    wl_dsl_trPtr->enable = m_wlVarPtr->wlEnbl;

    /* status is dynamic */

    strncpy( wl_dsl_trPtr->BSSID, wl_instance->bssMacAddr[MAIN_BSS_IDX], sizeof(wl_dsl_trPtr->BSSID) );

    wl_dsl_tr_get_maxBitRate(m_wlVarPtr->wlBand, m_wlVarPtr->wlRate, m_wlVarPtr->wlgMode,
                             wl_dsl_trPtr->maxBitRate );

    wl_dsl_trPtr->channel = m_wlVarPtr->wlChannel;

    strncpy( wl_dsl_trPtr->SSID, m_wlMssidVarPtr->wlSsid, sizeof(wl_dsl_trPtr->SSID) );

    wl_dsl_tr_get_brcm_beaconType(m_wlMssidVarPtr->wlAuthMode,
                                  wl_dsl_trPtr->beaconType );


    if ( !WLCSM_STRCMP( m_wlMssidVarPtr->wlFltMacMode, WL_FLT_MAC_ALLOW) )
        wl_dsl_trPtr->MACAddressControlEnabled = 1;
    else
        wl_dsl_trPtr->MACAddressControlEnabled = 0;


    wl_dsl_tr_get_standard( m_wlVarPtr->wlBand, m_wlVarPtr->wlgMode,
                            wl_dsl_trPtr->standard);

    if ( m_wlMssidVarPtr->wlKeyBit == WL_BIT_KEY_64 )
        wl_dsl_trPtr->WEPKeyIndex = m_wlMssidVarPtr->wlKeyIndex64;
    else
        wl_dsl_trPtr->WEPKeyIndex = m_wlMssidVarPtr->wlKeyIndex128;

    strncpy(wl_dsl_trPtr->keyPassphrase, "NOS", sizeof(wl_dsl_trPtr->keyPassphrase)); /*Not Support*/

    switch ( m_wlMssidVarPtr->wlKeyBit ) {
    case WL_BIT_KEY_64:
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "40-bit", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));
        break;
    case   WL_BIT_KEY_128:
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "104-bit", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));
        break;
    default:
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "Disabled", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));
        break;
    }


    /* Missed.
    char *    basicEncryptionModes;
    char *    basicAuthenticationMode;
    char *    WPAEncryptionModes;
    char *    WPAAuthenticationMode;
    char *    IEEE11iEncryptionModes;
    char *    IEEE11iAuthenticationMode;

    */
    /*basicEncryptionModes*/
    if ( !WLCSM_STRCMP(m_wlMssidVarPtr->wlWep, WL_DISABLED) ) {
        strncpy(wl_dsl_trPtr->basicEncryptionModes, "None", sizeof(wl_dsl_trPtr->basicEncryptionModes));
    } else {
        strncpy(wl_dsl_trPtr->basicEncryptionModes, "WEPEncryption", sizeof(wl_dsl_trPtr->basicEncryptionModes));
    }

    /*basicAuthenticationMode*/
    if ( !WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_RADIUS) ) {
        strncpy(wl_dsl_trPtr->basicAuthenticationMode, "EAPAuthentication", sizeof(wl_dsl_trPtr->basicAuthenticationMode));
    } else if ( !WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_SHARED) ) {
        strncpy(wl_dsl_trPtr->basicAuthenticationMode, "SharedAuthentication", sizeof(wl_dsl_trPtr->basicAuthenticationMode));
    } else {
        strncpy(wl_dsl_trPtr->basicAuthenticationMode, "None", sizeof(wl_dsl_trPtr->basicAuthenticationMode));
    }

    /* WPAAuthenticationMode*/
    if(WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA_PSK)==0) {
        strncpy(wl_dsl_trPtr->WPAAuthenticationMode, "PSKAuthentication", sizeof(wl_dsl_trPtr->WPAAuthenticationMode));
    } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA)==0) {
        strncpy(wl_dsl_trPtr->WPAAuthenticationMode, "EAPAuthentication", sizeof(wl_dsl_trPtr->WPAAuthenticationMode));
    } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2_PSK)==0) {
        strncpy(wl_dsl_trPtr->IEEE11iAuthenticationMode, "PSKAuthentication", sizeof(wl_dsl_trPtr->IEEE11iAuthenticationMode));
    } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2)==0) {
        strncpy(wl_dsl_trPtr->IEEE11iAuthenticationMode, "EAPAuthentication", sizeof(wl_dsl_trPtr->IEEE11iAuthenticationMode));
    } else {
        strncpy(wl_dsl_trPtr->WPAAuthenticationMode, "PSKAuthentication", sizeof(wl_dsl_trPtr->WPAAuthenticationMode));
        strncpy(wl_dsl_trPtr->IEEE11iAuthenticationMode, "PSKAuthentication", sizeof(wl_dsl_trPtr->IEEE11iAuthenticationMode));
    }
    /* WPAEncryptionModes */
    if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWep, WL_ENABLED)==0) {
        if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, TKIP_ONLY)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "WEPandTKIPEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandTKIPEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, AES_ONLY)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "WEPandAESEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandAESEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, TKIP_AND_AES)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "WEPandTKIPandAESEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandTKIPandAESEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        } else {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "WEPEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        }
    } else {
        if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, TKIP_ONLY)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "TKIPEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "TKIPEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, AES_ONLY)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "AESEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "AESEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        } else if(WLCSM_STRCMP(m_wlMssidVarPtr->wlWpa, TKIP_AND_AES)==0) {
            strncpy(wl_dsl_trPtr->WPAEncryptionModes, "TKIPandAESEncryption", sizeof(wl_dsl_trPtr->WPAEncryptionModes));
            strncpy(wl_dsl_trPtr->IEEE11iEncryptionModes, "TKIPandAESEncryption", sizeof(wl_dsl_trPtr->IEEE11iEncryptionModes));
        }
    }

    wl_dsl_tr_get_possibleChannels( wl_dsl_trPtr->possibleChannels, idx);

    wl_dsl_tr_get_basicDataTransmitRates(m_wlVarPtr->wlBand, m_wlVarPtr->wlgMode, m_wlVarPtr->wlBasicRate,
                                         wl_dsl_trPtr->basicDataTransmitRates);

    strncpy( wl_dsl_trPtr->operationalDataTransmitRates, "NOS", sizeof(wl_dsl_trPtr->operationalDataTransmitRates));	/**No Support  */
    strncpy( wl_dsl_trPtr->possibleDataTransmitRates, "NOS", sizeof(wl_dsl_trPtr->possibleDataTransmitRates) );	       /** No Support */
    wl_dsl_trPtr->insecureOOBAccessEnabled = 0;	/**No Support */
    wl_dsl_trPtr->beaconAdvertisementEnabled=0;	/**No Support  */

    /* radioEnabled is Dynamic */
    wl_dsl_tr_get_radioEnabled( &radio, idx );
    wl_dsl_trPtr->radioEnabled = radio;;

    wl_dsl_trPtr->autoRateFallBackEnabled = 0;	/** No Support*/
    strncpy( wl_dsl_trPtr->locationDescription, "NOS", sizeof(wl_dsl_trPtr->locationDescription));	/**No Support*/

    /* Because cms-data-model.xml has 3 bytes for regulatoryDomain only, here copy two bytes + one terminator to fit it */
    memcpy(wl_dsl_trPtr->regulatoryDomain, m_wlVarPtr->wlCountry, 2);
    wl_dsl_trPtr->regulatoryDomain[2] = 0;

    wl_dsl_trPtr->totalPSKFailures = 0;	/** No Support*/
    wl_dsl_trPtr->totalIntegrityFailures = 0;	/**No Support */
    strncpy(wl_dsl_trPtr->channelsInUse, "NOS", sizeof(wl_dsl_trPtr->channelsInUse));	/** No Support*/


    if ( !WLCSM_STRCMP(m_wlVarPtr->wlMode, WL_OPMODE_AP ) )
        strncpy( wl_dsl_trPtr->deviceOperationMode, "InfrastructureAccessPoint", sizeof(wl_dsl_trPtr->deviceOperationMode));
    if ( !WLCSM_STRCMP(m_wlVarPtr->wlMode, WL_OPMODE_WDS) )
        strncpy( wl_dsl_trPtr->deviceOperationMode, "WireleseeBridge", sizeof(wl_dsl_trPtr->deviceOperationMode));

//  strncpy( wl_dsl_trPtr->deviceOperationMode, "NOS", sizeof(wl_dsl_trPtr->deviceOperationMode));	/** No Support*/

    wl_dsl_trPtr->distanceFromRoot =0;	/**No Support*/
    strncpy( wl_dsl_trPtr->peerBSSID, "NOS", sizeof(wl_dsl_trPtr->peerBSSID));	/** */
    strncpy( wl_dsl_trPtr->authenticationServiceMode, "NOS", sizeof(wl_dsl_trPtr->authenticationServiceMode));	/**No Support */


    /* Dynamic
    totalBytesSent;
    totalBytesReceived;
    totalPacketsSent;
    totalPacketsReceived;
    totalAssociations;
    */
#ifdef WL_DSL_TR_DBG
    printf("%s@%d\n", __FUNCTION__, __LINE__ );
    printf("wl_dsl_trPtr->enable=%d\n", wl_dsl_trPtr->enable);

    printf("wl_dsl_trPtr->status=%s\n", wl_dsl_trPtr->status);

    printf("wl_dsl_trPtr->BSSID=%s\n", wl_dsl_trPtr->BSSID );

    printf("wl_dsl_trPtr->maxBitRate=%s\n", wl_dsl_trPtr->maxBitRate );

    printf("wl_dsl_trPtr->channel = %d\n", wl_dsl_trPtr->channel);

    printf("wl_dsl_trPtr->SSID=%s\n", wl_dsl_trPtr->SSID);

    printf("wl_dsl_trPtr->beaconType=%s\n", wl_dsl_trPtr->beaconType);

    printf("wl_dsl_trPtr->MACAddressControlEnabled = %d\n", wl_dsl_trPtr->MACAddressControlEnabled);
    printf(" wl_dsl_trPtr->standard=%s\n",  wl_dsl_trPtr->standard);

    printf("wl_dsl_trPtr->WEPKeyIndex=%d\n",  wl_dsl_trPtr->WEPKeyIndex);
    printf("wl_dsl_trPtr->WEPEncryptionLevel=%s\n", wl_dsl_trPtr->WEPEncryptionLevel );

    printf("wl_dsl_trPtr->basicEncryptionModes=%s\n", wl_dsl_trPtr->basicEncryptionModes);
    printf("wl_dsl_trPtr->basicAuthenticationMode=%s\n", wl_dsl_trPtr->basicAuthenticationMode);
    printf("wl_dsl_trPtr->WPAEncryptionModes=%s\n", wl_dsl_trPtr->WPAEncryptionModes);
    printf("wl_dsl_trPtr->WPAAuthenticationMode=%s\n", wl_dsl_trPtr->WPAAuthenticationMode);
    printf("wl_dsl_trPtr->basicDataTransmitRates=%s\n", wl_dsl_trPtr->basicDataTransmitRates);
    printf("wl_dsl_trPtr->radioEnabled = %d\n", wl_dsl_trPtr->radioEnabled);
    printf("wl_dsl_trPtr->regulatoryDomain=%s\n", wl_dsl_trPtr->regulatoryDomain);
#endif

    /*Setup Wepkey*/
    for (i = 0; i < 4; i++) {
        if ( m_wlMssidVarPtr->wlKeyBit == WL_BIT_KEY_64 )
            strncpy( wl_dsl_tr_wepkeyPtr->WEPKey[i], m_wlMssidVarPtr->wlKeys64[i], sizeof(wl_dsl_tr_wepkeyPtr->WEPKey[i]));
        else
            strncpy( wl_dsl_tr_wepkeyPtr->WEPKey[i], m_wlMssidVarPtr->wlKeys128[i], sizeof(wl_dsl_tr_wepkeyPtr->WEPKey[i]));
    }

    for (i = 0; i < 1; i++) {
        strncpy( wl_dsl_tr_presharedkeyPtr->PreSharedKey[i], m_wlMssidVarPtr->wlWpaPsk, sizeof(wl_dsl_tr_presharedkeyPtr->PreSharedKey[i]));
    }


    /*******************************
    Private fileds X_BROADCOM_COM
    ********************************/
    /* Dynamic statistical Data
        UINT32   X_BROADCOM_COM_RxErrors;
        UINT32   X_BROADCOM_COM_RxDrops;
        UINT32   X_BROADCOM_COM_TxErrors;
        UINT32   X_BROADCOM_COM_TxDrops;
    */

    m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[MAIN_BSS_IDX]);
    strncpy(wl_dsl_trPtr->X_BROADCOM_COM_IfName,  wl_instance->m_ifcName[0], sizeof(wl_dsl_trPtr->X_BROADCOM_COM_IfName));
    wl_dsl_trPtr->X_BROADCOM_COM_HideSSID = m_wlMssidVarPtr->wlHide;
    wl_dsl_trPtr->X_BROADCOM_COM_TxPowerPercent = m_wlVarPtr->wlTxPwrPcnt;


    for (i=0; i<WL_NUM_SSID-1; i++) {
        m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[i+GUEST_BSS_IDX]);
        struct wl_dsl_tr_mbss_struct *wl_dsl_tr_mbssPtr = &wl_dsl_trPtr->GuestMbss[i];

        strncpy(wl_dsl_tr_mbssPtr->GuestSSID,  m_wlMssidVarPtr->wlSsid, sizeof(wl_dsl_tr_mbssPtr->GuestSSID));
        strncpy( wl_dsl_tr_mbssPtr->GuestBSSID, wl_instance->bssMacAddr[i+GUEST_BSS_IDX], sizeof(wl_dsl_tr_mbssPtr->GuestBSSID) );
        wl_dsl_tr_mbssPtr->GuestEnable = m_wlMssidVarPtr->wlEnblSsid;
        wl_dsl_tr_mbssPtr->GuestHiden = m_wlMssidVarPtr->wlHide;
        wl_dsl_tr_mbssPtr->GuestIsolateClients = m_wlMssidVarPtr->wlAPIsolation;
        wl_dsl_tr_mbssPtr->GuestDisableWMMAdvertise = m_wlVarPtr->wlWme;
        wl_dsl_tr_mbssPtr->GuestMaxClients = m_wlMssidVarPtr->wlMaxAssoc;
    }
}


/* Convert from tr69 data structure to main structure*/
void wldsltr_set(int idx)
{
    WLAN_ADAPTER_STRUCT *wl_instance = &(m_instance_wl[idx]);
    WIRELESS_VAR *m_wlVarPtr = &(wl_instance->m_wlVar);
    WIRELESS_MSSID_VAR *m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[MAIN_BSS_IDX]);
    struct wl_dsl_tr_struct *wl_dsl_trPtr = &wl_dsl_tr[idx];
    struct wl_dsl_tr_wepkey_struct *wl_dsl_tr_wepkeyPtr = wl_dsl_tr_wepkey+idx;
    struct wl_dsl_tr_presharedkey_struct *wl_dsl_tr_presharedkeyPtr = wl_dsl_tr_presharedkey+idx;


    WepEnable wepEnabled = -1; //WepDisabled;
    Wlan_AuthMode authMode = -1; // WlanAuthMode_None;
    Wlan_Auth auth = -1; //WlanAuth_Open;
    Wlan_WpaAKm	WpaAkm;
    Wlan_Crypto crypto = -1; // Wlan_Crypto_Tkip;

    int  brcm_BeaconType =-1;
    int  brcm_BasicEncryptionModes=-1;
    int  brcm_BasicAuthenticationMode=-1;
    int  brcm_WPAEncryptionModes=-1;
    int  brcm_WPAAuthenticationMode=-1;
    int  brcm_IEEE11iEncryptionModes=-1;
    int  brcm_IEEE11iAuthenticationMode=-1;
    int keylen;

    int i = 0;



    WpaAkm.u.val = 0;


    /*Start Here */

    m_wlVarPtr->wlEnbl = wl_dsl_trPtr->enable;
    m_wlMssidVarPtr->wlEnblSsid = wl_dsl_trPtr->enable;

    /* status is ReadOnly dynamic */
    /* BSSID is ReadOnly dynamic */

    wl_dsl_tr_set_maxBitRate(wl_dsl_trPtr->maxBitRate, m_wlVarPtr->wlBand, m_wlVarPtr->wlgMode,
                             &(m_wlVarPtr->wlRate) );

    m_wlVarPtr->wlChannel = wl_dsl_trPtr->channel;

    strncpy( m_wlMssidVarPtr->wlSsid, wl_dsl_trPtr->SSID, sizeof(m_wlMssidVarPtr->wlSsid) );

    /*Set beaconType, next Security Part*/

    if ( wl_dsl_trPtr->MACAddressControlEnabled != 0 )
        strncpy(m_wlMssidVarPtr->wlFltMacMode, WL_FLT_MAC_ALLOW, sizeof(m_wlMssidVarPtr->wlFltMacMode));
    else
        strncpy(m_wlMssidVarPtr->wlFltMacMode, WL_FLT_MAC_DENY, sizeof(m_wlMssidVarPtr->wlFltMacMode));


    keylen = strlen((wl_dsl_tr_wepkey->WEPKey[wl_dsl_trPtr->WEPKeyIndex-1]));

    if(keylen == 5 || keylen == 10)
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "40-bit", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));
    else if(keylen == 13 || keylen == 26)
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "104-bit", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));
    else
        strncpy( wl_dsl_trPtr->WEPEncryptionLevel, "Disabled", sizeof(wl_dsl_trPtr->WEPEncryptionLevel));

    /*Standard is readOnly*/

    if ( !WLCSM_STRCMP( wl_dsl_trPtr->WEPEncryptionLevel, "40-bit") )
        m_wlMssidVarPtr->wlKeyBit  =WL_BIT_KEY_64;
    if ( !WLCSM_STRCMP( wl_dsl_trPtr->WEPEncryptionLevel, "104-bit") )
        m_wlMssidVarPtr->wlKeyBit  =WL_BIT_KEY_128;
    if ( !WLCSM_STRCMP( wl_dsl_trPtr->WEPEncryptionLevel, "Disabled") )
        m_wlMssidVarPtr->wlKeyBit  =WL_BIT_KEY_128;

    if ( m_wlMssidVarPtr->wlKeyBit == WL_BIT_KEY_64 )
        m_wlMssidVarPtr->wlKeyIndex64 = wl_dsl_trPtr->WEPKeyIndex;
    else
        m_wlMssidVarPtr->wlKeyIndex128 = wl_dsl_trPtr->WEPKeyIndex;

    /*keyPassphrase is not supported*/

    /* Next Security Part
    char *    basicEncryptionModes;
    char *    basicAuthenticationMode;
    char *    WPAEncryptionModes;
    char *    WPAAuthenticationMode;
    char *    IEEE11iEncryptionModes;
    char *    IEEE11iAuthenticationMode;
    */

    /*possibleChannels is ReadOnly*/

    wl_dsl_tr_set_basicDataTransmitRates(wl_dsl_trPtr->basicDataTransmitRates, m_wlVarPtr->wlBand, m_wlVarPtr->wlgMode,
                                         m_wlVarPtr->wlBasicRate );

    /*operationalDataTransmitRates RO */

    /* Not Support
       insecureOOBAccessEnabled
       beaconAdvertisementEnabled
    */

    wl_dsl_tr_set_radioEnabled( wl_dsl_trPtr->radioEnabled, idx);

    /* NOS
        autoRateFallBackEnabled
        locationDescription
    */
    strncpy(m_wlVarPtr->wlCountry, wl_dsl_trPtr->regulatoryDomain, sizeof(m_wlVarPtr->wlCountry));

    /* RO
        totalPSKFailures
        totalIntegrityFailures
        channelsInUse
    */

    if ( !WLCSM_STRCMP( wl_dsl_trPtr->deviceOperationMode, "InfrastructureAccessPoint") )
        strncpy(m_wlVarPtr->wlMode, WL_OPMODE_AP, sizeof(m_wlVarPtr->wlMode) );
    if ( !WLCSM_STRCMP(wl_dsl_trPtr->deviceOperationMode, "WireleseeBridge") )
        strncpy(m_wlVarPtr->wlMode, WL_OPMODE_WDS, sizeof(m_wlVarPtr->wlMode) );


    /* Not Support
         distanceFromRoot
         peerBSSID
         authenticationServiceMode
    */

    /* RO
    totalBytesSent;
    totalBytesReceived;
    totalPacketsSent;
    totalPacketsReceived;
    totalAssociations;
    */


    /********************
    Security Part
    *********************/

    /*Setup Wepkey*/
    for (i = 0; i < 4; i++) {
        if ( m_wlMssidVarPtr->wlKeyBit == WL_BIT_KEY_64 )
            strncpy( m_wlMssidVarPtr->wlKeys64[i], wl_dsl_tr_wepkeyPtr->WEPKey[i], sizeof(m_wlMssidVarPtr->wlKeys64[i]) );
        else
            strncpy( m_wlMssidVarPtr->wlKeys128[i], wl_dsl_tr_wepkeyPtr->WEPKey[i], sizeof(m_wlMssidVarPtr->wlKeys128[i]));
    }

    /*Setup PreSharedKey*/
    for (i = 0; i < 1; i++) {
        strncpy( m_wlMssidVarPtr->wlWpaPsk, wl_dsl_tr_presharedkeyPtr->PreSharedKey[i], sizeof(m_wlMssidVarPtr->wlWpaPsk) );
    }

#ifdef WL_DSL_TR_DBG
    printf("%s@%d\n", __FUNCTION__, __LINE__ );
    printf("wl_dsl_trPtr->enable=%d\n", wl_dsl_trPtr->enable);

    printf("wl_dsl_trPtr->status=%s\n", wl_dsl_trPtr->status);

    printf("wl_dsl_trPtr->BSSID=%s\n", wl_dsl_trPtr->BSSID );

    printf("wl_dsl_trPtr->maxBitRate=%s\n", wl_dsl_trPtr->maxBitRate );

    printf("wl_dsl_trPtr->channel = %d\n", wl_dsl_trPtr->channel);

    printf("wl_dsl_trPtr->SSID=%s\n", wl_dsl_trPtr->SSID);

    printf("wl_dsl_trPtr->beaconType=%s\n", wl_dsl_trPtr->beaconType);

    printf("wl_dsl_trPtr->MACAddressControlEnabled = %d\n", wl_dsl_trPtr->MACAddressControlEnabled);
    printf(" wl_dsl_trPtr->standard=%s\n",  wl_dsl_trPtr->standard);

    printf("wl_dsl_trPtr->WEPKeyIndex=%d\n",  wl_dsl_trPtr->WEPKeyIndex);
    printf("wl_dsl_trPtr->WEPEncryptionLevel=%s\n", wl_dsl_trPtr->WEPEncryptionLevel );

    printf("wl_dsl_trPtr->basicEncryptionModes=%s\n", wl_dsl_trPtr->basicEncryptionModes);
    printf("wl_dsl_trPtr->basicAuthenticationMode=%s\n", wl_dsl_trPtr->basicAuthenticationMode);
    printf("wl_dsl_trPtr->WPAEncryptionModes=%s\n", wl_dsl_trPtr->WPAEncryptionModes);
    printf("wl_dsl_trPtr->WPAAuthenticationMode=%s\n", wl_dsl_trPtr->WPAAuthenticationMode);
    printf("wl_dsl_trPtr->basicDataTransmitRates=%s\n", wl_dsl_trPtr->basicDataTransmitRates);
    printf("wl_dsl_trPtr->radioEnabled = %d\n", wl_dsl_trPtr->radioEnabled);
    printf("wl_dsl_trPtr->regulatoryDomain=%s\n", wl_dsl_trPtr->regulatoryDomain);
#endif

    /*Beacon Type*/
    if ( WLCSM_STRCMP (wl_dsl_trPtr->beaconType, "Basic") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_Basic;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "WPA") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_WPA;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "11i") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_11i;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "BasicandWPA") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_BasicandWPA;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "Basicand11i") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_Basicand11i;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "WPAand11i") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_WPAand11i;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->beaconType, "BasicandWPAand11i") == 0 ) {
        brcm_BeaconType = Wlan_BeaconType_BasicandWPAand11i;
    } else {
        brcm_BeaconType = Wlan_BeaconType_None;
    }

    /******************************
    char *    basicEncryptionModes;
    char *    basicAuthenticationMode;
    char *    WPAEncryptionModes;
    char *    WPAAuthenticationMode;
    char *    IEEE11iEncryptionModes;
    char *    IEEE11iAuthenticationMode;
    *******************************/
    if ( WLCSM_STRCMP(wl_dsl_trPtr->basicEncryptionModes, "WEPEncryption") == 0 )
        brcm_BasicEncryptionModes = Wlan_BasicEncryptionModes_WEPEncryption;
    else if ( WLCSM_STRCMP(wl_dsl_trPtr->basicEncryptionModes, "None") == 0 )
        brcm_BasicEncryptionModes = Wlan_BasicEncryptionModes_None;

    if ( WLCSM_STRCMP(wl_dsl_trPtr->basicAuthenticationMode, "None") == 0 ) {
        brcm_BasicAuthenticationMode = Wlan_BasicAuthenticationMode_None;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->basicAuthenticationMode, "EAPAuthentication") == 0 )  {
        brcm_BasicAuthenticationMode = Wlan_BasicAuthenticationMode_EAPAuthentication;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->basicAuthenticationMode, "SharedAuthentication") == 0 )  {
        brcm_BasicAuthenticationMode = Wlan_BasicAuthenticationMode_SharedAuthentication;
    }


    if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "WEPEncryption") == 0 ) {
        brcm_WPAEncryptionModes =Wlan_WPAEncryptionModes_WEPEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "TKIPEncryption") == 0 ) {
        brcm_WPAEncryptionModes =Wlan_WPAEncryptionModes_TKIPEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "WEPandTKIPEncryption") == 0 ) {
        brcm_WPAEncryptionModes = Wlan_WPAEncryptionModes_WEPandTKIPEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "AESEncryption") == 0 ) {
        brcm_WPAEncryptionModes = Wlan_WPAEncryptionModes_AESEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "WEPandAESEncryption") == 0 ) {
        brcm_WPAEncryptionModes = Wlan_WPAEncryptionModes_WEPandAESEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "TKIPandAESEncryption") == 0 ) {
        brcm_WPAEncryptionModes = Wlan_WPAEncryptionModes_TKIPandAESEncryption;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAEncryptionModes, "WEPandTKIPandAESEncryption") == 0 ) {
        brcm_WPAEncryptionModes = Wlan_WPAEncryptionModes_WEPandTKIPandAESEncryption;
    }

    if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAAuthenticationMode, "PSKAuthentication") == 0 ) {
        brcm_WPAAuthenticationMode = Wlan_WPAAuthenticationMode_PSKAuthentication;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->WPAAuthenticationMode, "EAPAuthentication") == 0 ) {
        brcm_WPAAuthenticationMode = Wlan_WPAAuthenticationMode_EAPAuthentication;
    }

    if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_WEPEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "TKIPEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_TKIPEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandTKIPEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_WEPandTKIPEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "AESEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_AESEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandAESEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_WEPandAESEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "TKIPandAESEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_TKIPandAESEncryption;
    } else  if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iEncryptionModes, "WEPandTKIPandAESEncryption") == 0 ) {
        brcm_IEEE11iEncryptionModes = Wlan_IEEE11iEncryptionModes_WEPandTKIPandAESEncryption;
    }

    if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iAuthenticationMode, "PSKAuthentication") == 0 ) {
        brcm_IEEE11iAuthenticationMode = Wlan_IEEE11iAuthenticationMode_PSKAuthentication;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iAuthenticationMode, "EAPAuthentication") == 0 ) {
        brcm_IEEE11iAuthenticationMode = Wlan_IEEE11iAuthenticationMode_EAPAuthentication;
    } else if ( WLCSM_STRCMP(wl_dsl_trPtr->IEEE11iAuthenticationMode, "EAPandPSKAuthentication") == 0 ) {
        brcm_IEEE11iAuthenticationMode = Wlan_IEEE11iAuthenticationMode_EAPandPSKAuthentication;
    }

    /*Setup Security params*/
    if(    brcm_BeaconType != Wlan_BeaconType_None &&
           brcm_BeaconType != Wlan_BeaconType_BasicandWPA &&
           brcm_BeaconType != Wlan_BeaconType_Basicand11i &&
           brcm_BeaconType != Wlan_BeaconType_BasicandWPAand11i ) {

        authMode = WlanAuthMode_None;
        wepEnabled = WepDisabled;
        WpaAkm.u.val = 0;
        auth = WlanAuth_Open;
    }

    // change WpaAkm and authMode accordingly
    switch ( brcm_BeaconType ) {
    case Wlan_BeaconType_None:
        // Infra sta mode, not supported
        break;
    case Wlan_BeaconType_Basic:
        switch ( brcm_BasicEncryptionModes ) {
        case Wlan_BasicEncryptionModes_None:
            wepEnabled = WepDisabled;
            break;
        case Wlan_BasicEncryptionModes_WEPEncryption:
            wepEnabled = WepEnabled;
            break;
        }

        switch ( brcm_BasicAuthenticationMode ) {
        case Wlan_BasicAuthenticationMode_None:
            authMode = WlanAuthMode_None;
            break;
        case Wlan_BasicAuthenticationMode_EAPAuthentication:
            authMode = WlanAuthMode_Radius;
            break;
        case Wlan_BasicAuthenticationMode_SharedAuthentication:
            auth = WlanAuth_Shared;
            break;
        }
        break;
    case Wlan_BeaconType_WPA:

        switch ( brcm_WPAEncryptionModes ) {
        case Wlan_WPAEncryptionModes_WEPEncryption:
            wepEnabled = WepEnabled;
            break;
        case Wlan_WPAEncryptionModes_TKIPEncryption:
            crypto = Wlan_Crypto_Tkip;
            break;
        case Wlan_WPAEncryptionModes_WEPandTKIPEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Tkip;
            break;
        case Wlan_WPAEncryptionModes_AESEncryption:
            crypto = Wlan_Crypto_Aes;
            break;
        case Wlan_WPAEncryptionModes_WEPandAESEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Aes;
            break;
        case Wlan_WPAEncryptionModes_TKIPandAESEncryption:
            crypto = Wlan_Crypto_Tkip_Aes;
            break;
        case Wlan_WPAEncryptionModes_WEPandTKIPandAESEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Tkip_Aes;
            break;
        }

        switch ( brcm_WPAAuthenticationMode ) {
        case Wlan_WPAAuthenticationMode_PSKAuthentication:
            WpaAkm.u.bitVal.Wlan_WpaAKm_Psk = 1;
            break;
        case Wlan_WPAAuthenticationMode_EAPAuthentication:
            WpaAkm.u.bitVal.Wlan_WpaAKm_Unspecified = 1;
            break;
        }
        break;

    case Wlan_BeaconType_11i:

        switch ( brcm_IEEE11iEncryptionModes ) {
        case Wlan_IEEE11iEncryptionModes_WEPEncryption:
            wepEnabled = WepEnabled;
            break;
        case Wlan_IEEE11iEncryptionModes_TKIPEncryption:
            crypto = Wlan_Crypto_Tkip;
            break;
        case Wlan_IEEE11iEncryptionModes_WEPandTKIPEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Tkip;
            break;
        case Wlan_IEEE11iEncryptionModes_AESEncryption:
            crypto = Wlan_Crypto_Aes;
            break;
        case Wlan_IEEE11iEncryptionModes_WEPandAESEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Aes;
            break;
        case Wlan_IEEE11iEncryptionModes_TKIPandAESEncryption:
            crypto = Wlan_Crypto_Tkip_Aes;
            break;
        case Wlan_IEEE11iEncryptionModes_WEPandTKIPandAESEncryption:
            wepEnabled = WepEnabled;
            crypto = Wlan_Crypto_Tkip_Aes;
            break;
        }

        switch ( brcm_IEEE11iAuthenticationMode ) {
        case Wlan_IEEE11iAuthenticationMode_PSKAuthentication:
            WpaAkm.u.bitVal.Wlan_Wpa2AKm_Psk = 1;
            break;
        case Wlan_IEEE11iAuthenticationMode_EAPAuthentication:
            WpaAkm.u.bitVal.Wlan_Wpa2AKm_Unspecified = 1;
            break;
        case Wlan_IEEE11iAuthenticationMode_EAPandPSKAuthentication:
            //not supported now
            break;
        }
        break;
    case Wlan_BeaconType_WPAand11i:
        switch ( brcm_WPAEncryptionModes ) {
        case Wlan_WPAEncryptionModes_WEPEncryption:
            wepEnabled = WepEnabled;
            break;
        case Wlan_WPAEncryptionModes_TKIPEncryption:
            crypto = Wlan_Crypto_Tkip;
            break;

        case Wlan_WPAEncryptionModes_AESEncryption:
            crypto = Wlan_Crypto_Aes;
            break;

        case Wlan_WPAEncryptionModes_TKIPandAESEncryption:
            crypto = Wlan_Crypto_Tkip_Aes;
            break;

        case Wlan_WPAEncryptionModes_WEPandTKIPandAESEncryption:
        case Wlan_WPAEncryptionModes_WEPandTKIPEncryption:
        case Wlan_WPAEncryptionModes_WEPandAESEncryption:
            break;
        }

        switch ( brcm_WPAAuthenticationMode ) {
        case Wlan_WPAAuthenticationMode_PSKAuthentication:
            WpaAkm.u.bitVal.Wlan_WpaAKm_Psk = 1;
            break;
        case Wlan_WPAAuthenticationMode_EAPAuthentication:
            WpaAkm.u.bitVal.Wlan_WpaAKm_Unspecified = 1;
            break;
        }

        switch ( brcm_IEEE11iEncryptionModes ) {
        case Wlan_IEEE11iEncryptionModes_WEPEncryption:
            wepEnabled = WepEnabled;
            break;
        case Wlan_IEEE11iEncryptionModes_TKIPEncryption:
            crypto = Wlan_Crypto_Tkip;
            break;


        case Wlan_IEEE11iEncryptionModes_AESEncryption:
            crypto = Wlan_Crypto_Aes;
            break;

        case Wlan_IEEE11iEncryptionModes_TKIPandAESEncryption:
            crypto = Wlan_Crypto_Tkip_Aes;
            break;

        case Wlan_IEEE11iEncryptionModes_WEPandAESEncryption:
        case Wlan_IEEE11iEncryptionModes_WEPandTKIPEncryption:
        case Wlan_IEEE11iEncryptionModes_WEPandTKIPandAESEncryption:
            break;
        }

        switch ( brcm_IEEE11iAuthenticationMode ) {
        case Wlan_IEEE11iAuthenticationMode_PSKAuthentication:
            WpaAkm.u.bitVal.Wlan_Wpa2AKm_Psk = 1;
            break;
        case Wlan_IEEE11iAuthenticationMode_EAPAuthentication:
            WpaAkm.u.bitVal.Wlan_Wpa2AKm_Unspecified = 1;
            break;
        case Wlan_IEEE11iAuthenticationMode_EAPandPSKAuthentication:
            //not supported now
            break;
        }

        break;
    case Wlan_BeaconType_BasicandWPA:
    case Wlan_BeaconType_Basicand11i:
    case Wlan_BeaconType_BasicandWPAand11i:
        // not supported now
        break;
    }


    //wepEnabled
    switch ( wepEnabled ) {
    case WepDisabled:
        strncpy(m_wlMssidVarPtr->wlWep, WL_DISABLED, sizeof(m_wlMssidVarPtr->wlWep));
        break;
    case WepEnabled:
        strncpy(m_wlMssidVarPtr->wlWep, WL_ENABLED, sizeof(m_wlMssidVarPtr->wlWep));
        break;
    }

    //auth
    m_wlMssidVarPtr->wlAuth = auth;


    // 802.1x Auth
    if( authMode == WlanAuthMode_Radius )
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_RADIUS, sizeof(m_wlMssidVarPtr->wlAuthMode));

    //WpaAkm
    if( WpaAkm.u.bitVal.Wlan_WpaAKm_Unspecified && WpaAkm.u.bitVal.Wlan_Wpa2AKm_Unspecified ) {
        // WPA and WPA2
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2_MIX, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else if( WpaAkm.u.bitVal.Wlan_WpaAKm_Unspecified ) {
        // WPA
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else if( WpaAkm.u.bitVal.Wlan_Wpa2AKm_Unspecified ) {
        // WPA2
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else if (WpaAkm.u.bitVal.Wlan_WpaAKm_Psk && WpaAkm.u.bitVal.Wlan_Wpa2AKm_Psk ) {
        // WPA-PSK and WPA2-PSK
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2_PSK_MIX, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else if (WpaAkm.u.bitVal.Wlan_WpaAKm_Psk ) {
        // WPA-PSK
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA_PSK, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else if (WpaAkm.u.bitVal.Wlan_Wpa2AKm_Psk ) {
        // WPA2-PSK
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_WPA2_PSK, sizeof(m_wlMssidVarPtr->wlAuthMode));
    } else
        //none and open
        strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_OPEN, sizeof(m_wlMssidVarPtr->wlAuthMode));

    //wep + shared
    if( wepEnabled == WepEnabled) {
        if( auth == WlanAuth_Shared ) {
            strncpy(m_wlMssidVarPtr->wlAuthMode, WL_AUTH_SHARED, sizeof(m_wlMssidVarPtr->wlAuthMode));
        }
    }


    //crypto
    switch ( crypto ) {
    case Wlan_Crypto_Aes:
        strncpy(m_wlMssidVarPtr->wlWpa, AES_ONLY, sizeof(m_wlMssidVarPtr->wlWpa));
        break;
    case Wlan_Crypto_Tkip_Aes:
        strncpy(m_wlMssidVarPtr->wlWpa, TKIP_AND_AES, sizeof(m_wlMssidVarPtr->wlWpa));
        break;
    case Wlan_Crypto_Tkip:
    default:
        strncpy(m_wlMssidVarPtr->wlWpa, TKIP_ONLY, sizeof(m_wlMssidVarPtr->wlWpa));
        break;
    }


    /*******************************
    Private fileds X_BROADCOM_COM
    ********************************/

    /* RO
        UINT32   X_BROADCOM_COM_RxErrors;
        UINT32   X_BROADCOM_COM_RxDrops;
        UINT32   X_BROADCOM_COM_TxErrors;
        UINT32   X_BROADCOM_COM_TxDrops;
    */

//RO    strncpy(wl_instance->m_ifcName[0], wl_dsl_trPtr->X_BROADCOM_COM_IfName, sizeof(wl_instance->m_ifcName[0]));
    m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[MAIN_BSS_IDX]);
    m_wlMssidVarPtr->wlHide = wl_dsl_trPtr->X_BROADCOM_COM_HideSSID;
    m_wlVarPtr->wlTxPwrPcnt = wl_dsl_trPtr->X_BROADCOM_COM_TxPowerPercent;


    for (i=0; i<WL_NUM_SSID-1; i++) {
        m_wlMssidVarPtr =  &(wl_instance->m_wlMssidVar[i+GUEST_BSS_IDX]);
        struct wl_dsl_tr_mbss_struct *wl_dsl_tr_mbssPtr = &wl_dsl_trPtr->GuestMbss[i];

        strncpy( m_wlMssidVarPtr->wlSsid, wl_dsl_tr_mbssPtr->GuestSSID, sizeof(m_wlMssidVarPtr->wlSsid));
        m_wlMssidVarPtr->wlEnblSsid = wl_dsl_tr_mbssPtr->GuestEnable;
        m_wlMssidVarPtr->wlHide = wl_dsl_tr_mbssPtr->GuestHiden;
        m_wlMssidVarPtr->wlAPIsolation = wl_dsl_tr_mbssPtr->GuestIsolateClients;
        m_wlVarPtr->wlWme = wl_dsl_tr_mbssPtr->GuestDisableWMMAdvertise;
        m_wlMssidVarPtr->wlMaxAssoc = wl_dsl_tr_mbssPtr->GuestMaxClients;
    }
}


/*Get Beacon Type*/
void wl_dsl_tr_get_brcm_beaconType( char *wlAuthMode, char *value )
{

    strcpy(value, "None");

    if (  !WLCSM_STRCMP(wlAuthMode, WL_AUTH_OPEN) ||
          !WLCSM_STRCMP(wlAuthMode, WL_AUTH_SHARED) ||
          !WLCSM_STRCMP(wlAuthMode, WL_AUTH_RADIUS)  )
        strcpy(value, "Basic");

    if ( !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA) ||
         !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA_PSK) )
        strcpy(value, "WPA");

    if ( !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA2) ||
         !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA2_PSK) )
        strcpy(value, "11i");

    if ( !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA2_MIX) ||
         !WLCSM_STRCMP(wlAuthMode, WL_AUTH_WPA2_PSK_MIX) )
        strcpy(value, "WPAand11i");
}


/*Get Max Tx Rate*/
void wl_dsl_tr_get_maxBitRate(int wlBand, long wlRate, int wlgMode, char *value)
{
    switch ( (Wlan_TxRate)wlRate ) {
    case Wlan_TxRate_Auto:
        strcpy(value, "Auto");
        break;
    case Wlan_TxRate_1Mbps:
        strcpy(value, "1");
        break;
    case Wlan_TxRate_2Mbps:
        strcpy(value, "2");
        break;
    case Wlan_TxRate_5_5Mbps:
        strcpy(value, "5.5");
        break;
    case Wlan_TxRate_6Mbps:
        strcpy(value, "6");
        break;
    case Wlan_TxRate_9Mbps:
        strcpy(value, "9");
        break;
    case Wlan_TxRate_11Mbps:
        strcpy(value, "11");
        break;
    case Wlan_TxRate_12Mbps:
        strcpy(value, "12");
        break;
    case Wlan_TxRate_18Mbps:
        strcpy(value, "18");
        break;
    case Wlan_TxRate_24Mbps:
        strcpy(value, "24");
        break;
    case Wlan_TxRate_36Mbps:
        strcpy(value, "36");
        break;
    case Wlan_TxRate_48Mbps:
        strcpy(value, "48");
        break;
    case Wlan_TxRate_54Mbps:
        strcpy(value, "54");
        break;
    default:
        strcpy( value, "");
        break;
    }
}

/*Set Max Tx Rate*/
void wl_dsl_tr_set_maxBitRate(char *maxBitRate, int wlBand, int wlgMode, long *wlRate)
{
    if ( WLCSM_STRCMP(maxBitRate, "Auto") == 0 ) {
        *wlRate = Wlan_TxRate_Auto;
    } else {
        switch ( atoi(maxBitRate) ) {
        case 1:
            *wlRate = Wlan_TxRate_1Mbps;
            break;
        case 2:
            *wlRate = Wlan_TxRate_2Mbps;
            break;
        case 6:
            *wlRate = Wlan_TxRate_6Mbps;
            break;
        case 9:
            *wlRate = Wlan_TxRate_9Mbps;
            break;
        case 11:
            *wlRate = Wlan_TxRate_11Mbps;
            break;
        case 12:
            *wlRate = Wlan_TxRate_12Mbps;
            break;
        case 18:
            *wlRate = Wlan_TxRate_18Mbps;
            break;
        case 24:
            *wlRate = Wlan_TxRate_24Mbps;
            break;
        case 36:
            *wlRate = Wlan_TxRate_36Mbps;
            break;
        case 48:
            *wlRate = Wlan_TxRate_48Mbps;
            break;
        case 54:
            *wlRate = Wlan_TxRate_54Mbps;
            break;
        default:
            *wlRate = Wlan_TxRate_5_5Mbps;
            break;
        }
    }
}

/*Get Band*/
void wl_dsl_tr_get_standard(int wlBand, int wlgMode, char *value)
{
    switch ( wlBand) {
    case BAND_A:
        strcpy(value, "a" );
        break;
    case BAND_B:
        if ( wlgMode == 0 )
            strcpy(value, "b" );
        else
            strcpy(value, "g" );
        break;
    default:
        strcpy( value, "");
        break;
    }
}


/*Get TX Rate**/
void wl_dsl_tr_get_basicDataTransmitRates(int wlBand, int wlgMode, char *wlBasicRate, char *value)
{
    switch ( wlBand) {
    case BAND_A:
        if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_DEFAULT) )
            strcpy(value, "54" );
        if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_ALL) )
            strcpy(value, "1,2,5.5,6,9,11,12,18,24,36,48,54");
        if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_WIFI_2) )
            strcpy(value, "6,12,24");
        if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_1_2) )
            strcpy(value, "6,12");
        break;
    case BAND_B:
        if ( wlgMode == 0 ) {
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_DEFAULT) )
                strcpy(value, "11" );
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_ALL) )
                strcpy(value, "1,2,5.5,11");
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_WIFI_2) )
                strcpy(value, "1,2,5.5,6,11,12,24");
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_1_2) )
                strcpy(value, "1,2");
        } else {
            // .11g
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_DEFAULT) )
                strcpy(value, "54" );
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_ALL) )
                strcpy(value, "1,2,5.5,6,9,11,12,18,24,36,48,54");
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_WIFI_2) )
                strcpy(value, "6,12,24");
            if ( !WLCSM_STRCMP(wlBasicRate, WL_BASIC_RATE_1_2) )
                strcpy(value, "6,12");
        }
        break;
    default:
        strcpy( value, "");
        break;
    }
}

/*Set Tx Rate*/
void wl_dsl_tr_set_basicDataTransmitRates(char *basicDataTransmitRates, int wlBand, int wlgMode, char *wlBasicRate)
{
    char *ptr = basicDataTransmitRates;

    // Skip all preceeding white space if any
    while ( *ptr == ' ' )
        ptr++;

    if ( wlBand == BAND_A ) {
        if ( WLCSM_STRCMP(ptr, "54") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_DEFAULT);
        } else if ( WLCSM_STRCMP(ptr, "1,2,5.5,6,9,11,12,18,24,36,48,54") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_ALL);
        } else if ( WLCSM_STRCMP(ptr, "6,12,24") == 0 ) {
            strcpy(wlBasicRate,  WL_BASIC_RATE_WIFI_2);
        } else if ( WLCSM_STRCMP(ptr, "6,12") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_1_2);
        }
    } else if ( wlgMode == 0) { //b mode rate????
        if ( WLCSM_STRCMP(ptr, "11") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_DEFAULT);
        } else if ( WLCSM_STRCMP(ptr, "1,2,5.5,6,9,11") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_ALL);
        } else if ( WLCSM_STRCMP(ptr, "1,2,5.5,6,11,12,24") == 0 ) {
            strcpy(wlBasicRate,  WL_BASIC_RATE_WIFI_2);
        } else if ( WLCSM_STRCMP(ptr, "1,2") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_1_2);
        }
    } else { //G Mode
        if ( WLCSM_STRCMP(ptr, "54") == 0 ) {
            strcpy(wlBasicRate , WL_BASIC_RATE_DEFAULT);
        } else if ( WLCSM_STRCMP(ptr, "1,2,5.5,6,9,11,12,18,24,36,48,54") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_ALL);
        } else if ( WLCSM_STRCMP(ptr, "6,12,24") == 0 ) {
            strcpy(wlBasicRate,  WL_BASIC_RATE_WIFI_2);
        } else if ( WLCSM_STRCMP(ptr, "6,12") == 0 ) {
            strcpy(wlBasicRate, WL_BASIC_RATE_1_2);
        }
    }
}



/*get Channel*/
void wl_dsl_tr_get_possibleChannels( char *value, int idx )
{
#define IFC_GIANT_LEN           1024


    char buf[IFC_GIANT_LEN] = {0};
    FILE *fp = NULL;
    char *pBuf= buf;
    char *pRes = value;

    *value = '\0';

    snprintf(buf, sizeof(buf), "wlctl -i wl%d channels > /var/wlchannels", idx);
    bcmSystemEx(buf, 1);
    fp = fopen("/var/wlchannels", "r");
    if ( fp != NULL ) {
        fgets(buf, IFC_GIANT_LEN, fp);
        // Skip all the white space
        while ( *pBuf == ' ' ) pBuf++;

        // process the string
        while ( *pBuf != '\0' ) {
            if ( *pBuf == ' ' ) {
                *pRes = ',';
                pRes++;
                // Skip all the white space till the next non space
                while ( *pBuf == ' ' )
                    pBuf++;
            } else  {
                if ( *pBuf >='0' && *pBuf <='9' ) {
                    *pRes = *pBuf;
                    pRes++;
                }
                pBuf++;
            }
        }
        // Close the file and remove it.
        fclose(fp);
        bcmSystemEx("rm /var/wlchannels", 1);
        *pRes = '\0';
    }
}


/*Set radio Enable/Disable*/
void wl_dsl_tr_set_radioEnabled( int value, int idx )
{
    char buf[128];
    if ( value )
        snprintf(buf, sizeof(buf), "wlctl -i wl%d radio on", idx);
    else
        snprintf(buf, sizeof(buf), "wlctl -i wl%d radio off", idx);
    bcmSystemEx(buf, 1);
}

/*Get Radio State*/
void wl_dsl_tr_get_radioEnabled( int  *value, int idx )
{
    char buf[128] = {0};
    FILE *fp = NULL;

    *value = 1;
    snprintf(buf, sizeof(buf), "wlctl -i wl%d radio > /var/radiostatus", idx);
    bcmSystemEx(buf, 1);
    fp = fopen("/var/radiostatus", "r");
    if ( fp != NULL ) {
        fgets(buf, 32, fp);
        if ( strstr(buf, "1") != NULL ) {
            *value = 0;
        } else {
            *value = 1;
        }
        fclose(fp);
        bcmSystemEx("rm /var/radiostatus", 1);
    }
}









