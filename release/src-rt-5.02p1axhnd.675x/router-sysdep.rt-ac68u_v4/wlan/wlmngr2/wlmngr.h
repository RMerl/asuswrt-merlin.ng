/*
* <:copyright-BRCM:2011:proprietary:standard
*
*    Copyright (c) 2011 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#ifndef __WL_MNGR_H__
#define __WL_MNGR_H__
#include <wlcsm_lib_dm.h>
#include <wlcsm_linux.h>
#include <pthread.h>
#include <bcmendian.h>
#include <wlsyscall.h>



typedef enum {
    WLMNGR_VAR_OPER_GET,
    WLMNGR_VAR_OPER_SET,
    WLMNGR_VAR_OPER_VALIDATE
} t_WLMNGR_VAR_OPER_ENUM;

#define  WLMNGR_VAR_HANDLE_FAILURE (-1)
#define  WLMNGR_VAR_NOT_HANDLED (0)
extern int wl_index;
extern int is_smp_system;


/******************************************
Function Declaration
*******************************************/
void wlmngr_initCfg ( unsigned char dataSync, int idx);
int wlmngr_alloc( int adapter_cnt );
void wlmngr_free(void );

int wlmngr_save_nvram(void);

void wlmngr_initVars(const unsigned int idx);
void wlmngr_init(const unsigned int idx);
void wlmngr_update_assoc_list(const unsigned int idx);

void wlmngr_setup(const unsigned int idx);
void wlmngr_store(const unsigned int idx);
void wlmngr_setVar(const unsigned int idx, char *varName, char *varValue);
void wlmngr_stopServices(void);
void wlmngr_startServices(void);
void wlmngr_startNas(const unsigned int idx);
void wlmngr_startNasOne(const unsigned int idx, int ssid_idx);
void wlmngr_stopNas(const unsigned int idx);
#ifdef SUPPORT_WSC
void wlmngr_startWsc(void);
void wlmngr_stopWsc(const unsigned int idx);
#endif
#ifdef SUPPORT_SES
void wlmngr_startSes(const unsigned int idx);
void wlmngr_stopSes(const unsigned int idx);
void wlmngr_startSesCl(const unsigned int idx);
void wlmngr_stopSesCl(const unsigned int idx);
void wlmngr_clearSesLed(const unsigned int idx);
#endif



char *strcat_r( const char *s1, const char *s2, char *buf);

void wlmngr_wlIfcUp(const unsigned int idx);
void wlmngr_wlIfcDown(const unsigned int idx);


void wlmngr_scanWdsResult(const unsigned int idx);
void wlmngr_scanResult(const unsigned int idx, int chose);

void wlmngr_getHwAddr(const unsigned int idx, char *ifname, char *hwaddr);
int wlmngr_getBands(const unsigned int idx);
int wlmngr_checkAfterburner(const unsigned int idx);
int wlmngr_checkWme(const unsigned int idx);
char wlmngr_scanForAddr(char *line, int isize, char **start, int *size);
void wlmngr_aquireStationList(const unsigned int idx);
int wlmngr_scanFileForMAC(char *fname, char *mac);
#ifdef DSLCPE_WLCSM_EXT
char *wlmngr_getVer(const unsigned int idx,char *version );
#else
void wlmngr_getVer(const unsigned int idx );
#endif
void wlmngr_getCurrentChannel(const unsigned int idx);
int wlmngr_getChannelImState(const unsigned int idx);
int wlmngr_getCurrentChSpec(const unsigned int idx);
int wlmngr_getMaxMbss(const unsigned int idx);
void wlmngr_setup_if_mac(const unsigned int idx);
void wlmngr_setupMbssMacAddr(const unsigned int idx);
WL_STATUS wlmngr_enumInterfaces(const unsigned int idx, char *devname);

void wlmngr_doWlConf(const unsigned int idx);
void wlmngr_WlConfDown(const unsigned int idx);
void wlmngr_WlConfStart(const unsigned int idx);
void wlmngr_getFltMacList(const unsigned int idx,int bssidx, char *buf, int size);
void wlmngr_doSecurity(const unsigned int idx);
void wlmngr_doWdsSec(const unsigned int idx);
long wlmngr_getValidRate(const unsigned int idx, long rate);
int  wlmngr_getCoreRev(const unsigned int idx);
char *wlmngr_getPhyType(const unsigned int idx);
int wlmngr_getValidBand(const unsigned int idx, int band);
void wlmngr_getWdsMacList(const unsigned int idx, char *buf, int size);
void wlmngr_setWdsMacList(const unsigned int idx, char *buf, int size);

#ifdef SUPPORT_WSC
void wlmngr_genWscRandSssidPsk(const unsigned int idx); //This function is used to generate Random SSID and PSK Key
int wlmngr_wscPincheck(char *pin_string);
int  wl_wscPinGen( void );
#endif

void wlmngr_setupBridge(const unsigned int idx);
void wlmngr_stopBridge(const unsigned int idx);
void wlmngr_setupAntenna(const unsigned int idx);
void wlmngr_autoChannel(const unsigned int idx);
void wlmngr_setupRegulatory(const unsigned int idx);
void wlmngr_printSsidList(const unsigned int idx, char *text);
void wlmngr_printMbssTbl(const unsigned int idx, char *text);
bool wlmngr_getWdsWsec(const unsigned int idx, int unit, int which, char *mac, char *role, char *crypto, char *auth, ...);
void wlmngr_getWlInfo(const unsigned int idx, char *buf, char *id);
void wlmngr_getPwrSaveStatus(const unsigned int idx, char *varValue);
#ifdef IDLE_PWRSAVE
void wlmngr_togglePowerSave(void);
#endif
void wlmngr_printScanResult(const unsigned int idx, char *text);
void wlmngr_saveApInfoFromScanResult(int apListIdx);
bool wlmngr_isEnrSta(const unsigned int idx);
bool wlmngr_isEnrAP(const unsigned int idx);
void wlmngr_doWpsApScan(void);
int wlmngr_ReadChipInfoStbc(void);
void wlmngr_crash_log_backup_init(void);

typedef struct {
    int lan_wps_oob;
    int wlAuth;
    char wlSsid[WL_SSID_SIZE_MAX];
    char wlWpaPsk[WL_WPA_PSK_SIZE_MAX];
    char wlWep[WL_SM_SIZE_MAX];
    char wlWpa[WL_SM_SIZE_MAX];
    char wlAuthMode[WL_SM_SIZE_MAX];

} WLMNGR_1905_CREDENTIAL;
#define CREDENTIAL_VAROFF(vv) (offsetof(WLMNGR_1905_CREDENTIAL,vv))
#define CREDENTIAL_STRVARVALUE(vv, off)  ((char *)(((char *)vv)+off))
#define CREDENTIAL_INTVARPTR(vv, off)  ((int * )(((char *)vv)+off))

#define bcmSystem(cmd)		bcmSystemEx (cmd,1)
#define bcmSystemMute(cmd)	bcmSystemEx (cmd,0)
extern int g_wlmngr_ready_for_event;
void wlmngr_thread_lock(int lock);
void wlmngr_get_thread_lock(void);
void wlmngr_release_thread_lock(void);
void wlan_restart_all(int wait,SYNC_DIRECTION direction);
int wlmngr_start_wl_restart(t_WLCSM_MNGR_VARHDR *hdr);
#ifdef DSLCPE_SHLIB
extern void wlctl_cmd(char *cmd);
#define BCMWL_WLCTL_CMD(x)    wlctl_cmd(x)
#define BCMWL_STR_CONVERT(in, out) bcmConvertStrToShellStr(in, out)
#else
#define BCMWL_WLCTL_CMD(x)   bcmSystem(x)
#define BCMWL_STR_CONVERT(in, out) bcmConvertStrToShellStr(in, out)
#endif

extern pthread_mutex_t g_WLMNGR_REBOOT_MUTEX;
#define  WLMNGR_RESTART_LOCK() pthread_mutex_lock(&g_WLMNGR_REBOOT_MUTEX)
#define  WLMNGR_RESTART_UNLOCK() pthread_mutex_unlock(&g_WLMNGR_REBOOT_MUTEX)

/****** handling speicial vars,return 0 if handled, else return 1  ********/
typedef int (*mngr_special_var_handler)(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value,t_WLMNGR_VAR_OPER_ENUM op);
int wlmngr_handle_special_var(t_WLCSM_MNGR_VARHDR *hdr, char *name,char *value,t_WLMNGR_VAR_OPER_ENUM op);
int wlmngr_update_possibble_channels(unsigned int idx,SYNC_DIRECTION direction);
void set_wps_config_method_default(void);
void wlmngr_nv_adjust_chanspec(unsigned int idx,SYNC_DIRECTION direction);
int wlmngr_special_nvram_handler(char *name,char *value);
int  wlmngr_wps_wsc_handler(t_WLCSM_MNGR_VARHDR *hdr,char * name,char *value, t_WLMNGR_VAR_OPER_ENUM op);
int  wlmngr_wps_wlwsc_handler(t_WLCSM_MNGR_VARHDR *hdr,char * name,char *value, t_WLMNGR_VAR_OPER_ENUM op);
int wlmngr_set_var(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue);
int wlmngr_get_var(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue);
int wlmngr_getPidByName(const char *name);
void *wlmngr_get_runtime_obj_pointer(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,WLCSM_DM_MNGR_CMD cmd,void *par);
void wlmngr_sign_chan_by_chanspec( uint idx,uint ssid_idx,char *value);
int  wlmngr_handle_wlcsm_cmd_restart(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue);
extern void brcm_get_lock(char *name, int timeout);
extern void brcm_release_lock(char* name);
extern int dhd_probe(char *name);
WLCSM_DM_DECLARE(nocms);
WLCSM_DM_DECLARE(tr181);
WLCSM_DM_DECLARE(tr98);
int restarting_thread_init(void);
extern int g_dm_loaded;
extern char g_ifbuf[];
extern int act_wl_cnt;
extern char g_wifi_autorestart_enable;
void wlmngr_nvram_adjust(const unsigned int idx,int direction);
void wlmngr_write_wl_nvram(const unsigned int idx);
void wlmmgr_vars_adjust(const unsigned int idx,SYNC_DIRECTION direction);
#ifdef HSPOT_SUPPORT
void wlmngr_hspot_default(void);
#endif
int wlmngr_handle_wlcsm_cmd_resume_restart(t_WLCSM_MNGR_VARHDR  *hdr, char *interface,char *action);
int wlmngr_handle_wlcsm_cmd_halt_restart(t_WLCSM_MNGR_VARHDR  *hdr, char *interface,char *action); 
int wlmngr_get_restarting_status(void);
void wlmngr_handle_bridge_setup(void);
void _wlmngr_bridge_ipalias(void);
void wlmngr_post_setup(unsigned int idx);
void wlmngr_update_stalist(int radio_idx);
void wlmngr_postStart_Service(void);
void wlmngr_start_wbd(void);
void wlcsm_dm_get_bridge_info_adjust(SYNC_DIRECTION direction,char *buf);
void wlmngr_pre_setup(const unsigned int idx);
#endif
