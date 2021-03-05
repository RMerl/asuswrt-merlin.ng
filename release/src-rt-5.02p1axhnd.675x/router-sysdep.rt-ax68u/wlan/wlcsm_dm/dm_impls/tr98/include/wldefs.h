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
#ifndef __WL_DEFS_H__
#define __WL_DEFS_H__


#include <stdio.h>
#include <netinet/in.h>
#include "wlcsm_lib_api.h"
#define WL_NUM_SSID      (wlcsm_dm_tr98_getMaxMbss(idx))


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
#endif
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
#endif
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
    char wlOperMode[WL_SM_SIZE_MAX];
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
#endif
#ifdef SUPPORT_WSC
    char wsc_mode[12]; // enabled or disabled
    char wsc_config_state[4]; /*0: unconfig 1:Configed*/
#endif
#ifdef WMF
    int wlEnableWmf;
#endif
    WL_FLT_MAC_ENTRY     *m_tblFltMac;
    int wlEnableHspot; /* 0:disabled, 1:enabled , 2:hspot is not supported*/
    int wlMFP;         /* 0:disabled, 1:enabled, 2:required */
    int wlSsdType;
} WIRELESS_MSSID_VAR, *PWIRELESS_MSSID_VAR;

#define MSSID_VAROFF(vv) (offsetof(WIRELESS_MSSID_VAR,vv))

static inline char * MSSID_STRVARVALUE(char *p,int off)
{
    return (char *) (((char *)p)+off);
}
static inline int  MSSID_INTVARVALUE(char *p,int off)
{
    return *((int *) (((char *)p)+off));
}

typedef struct {
    char *varName;
    char *varValue;
} WIRELESS_ITEM, *PWIRELESS_ITEM;



#ifdef BCMWAPI_WAI
// Make sure (WAPI_CERT_BUFF_SIZE) > (max_cert_num) * sizeof(struct cert_index_item_t).
// (max_cert_num) defined @ "wapi/as/conf/AS.conf".
// (cert_index_item_t) defined @ "wapi/as/include/as_common.h".
#define WAPI_CERT_BUFF_SIZE (4096)
#endif
#define WL_N_BW_20ALL               0
#define WL_N_BW_40ALL               1
#define WL_N_BW_20IN2G_40IN5G       2
#define WL_V_BW_80IN5G              3
#define WL_MACFLT_NUM    32
#endif
