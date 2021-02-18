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
#ifndef __WL_MNGR_H__
#define __WL_MNGR_H__

#include <wlcsm_linux.h>
#include <wlcsm_lib_wl.h>
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>
#include <wldefs.h>
//#include <bcmconfig.h>
#define WL_SYNC_TO_MDM	0
#define WL_SYNC_TO_MDM_NVRAM    1
#define WL_SYNC_FROM_MDM	2
#define WL_SYNC_FROM_MDM_TR69C  3
#define WL_SYNC_FROM_MDM_HTTPD  4
#define WL_SYNC_NONE		(WL_SYNC_FROM_MDM_HTTPD +1)
#define DEFAULT_PAIR_LEN_BYTE 2
#define PAIR_LEN_BYTE 3

typedef unsigned char bool;

typedef struct {
    WIRELESS_VAR m_wlVar;
    WIRELESS_MSSID_VAR m_wlMssidVar[WL_MAX_NUM_SSID];
    char m_ifcName[WL_MAX_NUM_SSID][WL_SM_SIZE_MAX];
    int numStations;
    int numStationsBSS[WL_MAX_NUM_SSID];
    char bssMacAddr[WL_MAX_NUM_SSID][WL_MID_SIZE_MAX];
    int wlCurrentChannel;
    int m_bands;
    int maxMbss;
    int mbssSupported;
    int numBss;
    WL_STATION_LIST_ENTRY *stationList;
    int qosInitDone;
    char wlVer[WL_MID_SIZE_MAX];

    int aburnSupported;
    int ampduSupported;
    int amsduSupported;
    int wmeSupported;
    bool wlInitDone;
    bool onBridge;
    int m_unit;
    int m_refresh;

    WL_FLT_MAC_ENTRY     *m_tblWdsMac;
    WL_FLT_MAC_ENTRY     *m_tblScanWdsMac;
#ifdef WMF
    int wmfSupported;
#endif
#ifdef DUCATI
    int wlVecSupported;
    int wlIperf;
    int wlVec;
#endif
    int apstaSupported;
    int wlMediaSupported;
} WLAN_ADAPTER_STRUCT;


extern int wl_index;
extern WLAN_ADAPTER_STRUCT *m_instance_wl;
extern int is_smp_system;


/******************************************
Function Declaration
*******************************************/
void wlcsm_dm_tr98_initCfg ( unsigned char dataSync, int idx);
int wlcsm_dm_tr98_alloc( int adapter_cnt );
void wlcsm_dm_tr98_free(void );

void wlcsm_dm_tr98_initNvram(int idx);
void wlcsm_dm_tr98_write_nvram(void);

void wlcsm_dm_tr98_initVars(int idx);
void wlcsm_dm_tr98_init(int idx);
void wlcsm_dm_tr98_unInit(int idx);
void wlcsm_dm_tr98_update_assoc_list(int idx);
void wlcsm_dm_tr98_setup(int idx, WL_SETUP_TYPE type);
void wlcsm_dm_tr98_store(int idx);
void wlcsm_dm_tr98_retrieve(int idx, int isDefault);
void wlcsm_dm_tr98_getVar(int idx, char *varName, char *varValue);
void wlcsm_dm_tr98_getVarEx(int idx, int argc, char **argv, char *varValue);
void wlcsm_dm_tr98_setVar(int idx, char *varName, char *varValue);
void wlcsm_dm_tr98_stopServices(int idx);
void wlcsm_dm_tr98_startServices(int idx);
void wlcsm_dm_tr98_startNas(int idx);
void wlcsm_dm_tr98_startNasOne(int idx, int ssid_idx);
void wlcsm_dm_tr98_stopNas(int idx);
#ifdef SUPPORT_WSC
void wlcsm_dm_tr98_startWsc(int idx);
void wlcsm_dm_tr98_stopWsc(int idx);
#endif



char *strcat_r( const char *s1, const char *s2, char *buf);

void wlcsm_dm_tr98_wlIfcUp(int idx);
void wlcsm_dm_tr98_wlIfcDown(int idx);


void wlcsm_dm_tr98_getHwAddr(int idx, char *ifname, char *hwaddr);
int wlcsm_dm_tr98_getBands(int idx);
int wlcsm_dm_tr98_checkAfterburner(int idx);
int wlcsm_dm_tr98_checkWme(int idx);
int wlcsm_dm_tr98_getNumStations(int idx);
void wlcsm_dm_tr98_getStation(int idx, int i, char *macAddress, char *associated,  char *authorized, char *ssid, char *ifcName);
void wlcsm_dm_tr98_getCountryList(int idx,int argc, char **argv, char *list);
void wlcsm_dm_tr98_getChannelList(int idx,int argc, char **argv, char *list);
void wlcsm_dm_tr98_getVer(int idx );
int wlcsm_dm_tr98_getMaxMbss(int idx);

void wlcsm_dm_tr98_doWlConf(int idx);
void wlcsm_dm_tr98_WlConfDown(int idx);
void wlcsm_dm_tr98_getFltMacList(int idx,int bssidx, char *buf, int size);
void wlcsm_dm_tr98_doSecurity(int idx);
void wlcsm_dm_tr98_doWdsSec(int idx);
void wlcsm_dm_tr98_doQoS(int idx);
int  wlcsm_dm_tr98_getValidChannel(int idx, int channel);
long wlcsm_dm_tr98_getValidRate(int idx, long rate);
int  wlcsm_dm_tr98_getCoreRev(int idx);
char *wlcsm_dm_tr98_getPhyType(int idx);
int wlcsm_dm_tr98_getValidBand(int idx, int band);
void wlcsm_dm_tr98_getWdsMacList(int idx, char *buf, int size);
void wlcsm_dm_tr98_setWdsMacList(int idx, char *buf, int size);

#ifdef SUPPORT_WSC
void wlcsm_dm_tr98_genWscRandSssidPsk(int idx); //This function is used to generate Random SSID and PSK Key
int wlcsm_dm_tr98_wscPincheck(char *pin_string);
int  wl_wscPinGet( void );
#endif

void wlcsm_dm_tr98_setupBridge(int idx);
void wlcsm_dm_tr98_stopBridge(int idx);
void wlcsm_dm_tr98_setupRegulatory(int idx);
void wlcsm_dm_tr98_togglePowerSave(char *anyassoc);
void wlcsm_dm_tr98_restore_config_default(char *tftpip);
#ifdef DMP_X_BROADCOM_COM_WIFIWAN_1
void wlcsm_dm_tr98_initUre(void);
void wlcsm_dm_tr98_initWanWifi(void);
#endif
#define bcmSystem(cmd)		bcmSystemEx (cmd,1)
#define bcmSystemMute(cmd)	bcmSystemEx (cmd,0)
void wlcsm_dm_tr98_nvram_set(const char *name,const char *value);
extern void wlctl_cmd(char *cmd);
#ifdef DSLCPE_SHLIB
#define BCMWL_WLCTL_CMD(x)    wlctl_cmd(x)
#define BCMWL_STR_CONVERT(in, out) bcmConvertStrToShellStr(in, out)
#else
#define BCMWL_WLCTL_CMD(x) bcmSystem(x)
#define BCMWL_STR_CONVERT(in, out) bcmConvertStrToShellStr(in, out)
#endif
void wlcsm_dm_tr98_NvramMapping(int idx, const int dir);
extern WL_STATUS BcmWl_addFilterMac2(char *mac, char *ssid, char *ifcName, int wl_idx, int idx);
extern WL_STATUS BcmWl_removeAllFilterMac(int wl_idx, int idx);
#endif
