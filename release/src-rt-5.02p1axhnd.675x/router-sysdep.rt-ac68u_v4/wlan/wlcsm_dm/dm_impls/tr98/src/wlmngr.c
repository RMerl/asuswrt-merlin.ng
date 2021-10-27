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

int  BcmWl_GetMaxMbss(void);
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
#include <wldefs.h>
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

#include "wlutils.h"
#include "wlmngr.h"
#include "wlmdm.h"


#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include "wldsltr.h"
#include <time.h>
#include<bcmconfig.h>
#define MIMO_PHY ((!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_N)) || (!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_AC)))

#ifdef SUPPORT_SES
#ifndef SUPPORT_NVRAM
#error BUILD_SES depends on BUILD_NVRAM
#endif
#endif

#define IFC_BRIDGE_NAME	 "br0"

#define WL_PREFIX "_wl"

WLAN_ADAPTER_STRUCT *m_instance_wl; //[IFC_WLAN_MAX]; /*Need to Optimise to Dynamic buffer allocation*/
extern int wl_cnt;

bool cur_mbss_on = FALSE; /* current mbss status, off by default at init */
int act_wl_cnt = 0; /* total enabled wireless adapters */
int is_smp_system = 0;  /* set to 1 if we are on SMP system */

/* These functions are to be used only within this file */
static void wlcsm_dm_tr98_getVarFromNvram(void *var, const char *name, const char *type);
static void wlcsm_dm_tr98_setVarToNvram(void *var, const char *name, const char *type);

/*Dynamic Allocate adapter buffers*/
int wlcsm_dm_tr98_alloc( int adapter_cnt )
{
    if ( (m_instance_wl = malloc( sizeof(WLAN_ADAPTER_STRUCT) * adapter_cnt )) == NULL ) {
        printf("%s@%d alloc m_instance_wl[size %zu] Err \n", __FUNCTION__, __LINE__,
               sizeof(WLAN_ADAPTER_STRUCT) * adapter_cnt);
        return -1;
    }

    /* Zero the buffers */
    memset( m_instance_wl, 0, sizeof(WLAN_ADAPTER_STRUCT) * adapter_cnt );
    return 0;
}

void wlcsm_dm_tr98_free(void )
{
    if ( m_instance_wl != NULL )
        free(m_instance_wl);
}

void nmode_no_tkip(int idx)
{
    int i;

    WLCSM_TRACE(WLCSM_TRACE_LOG,".........\r\n");
    if ( !WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlNmode, WL_OFF)  ||
            (WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_N) && WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_AC)))
        return;

    WLCSM_TRACE(WLCSM_TRACE_LOG,".........\r\n");
    for (i = 0; i < WL_NUM_SSID; i++) {
        if (WLCSM_STRCMP(m_instance_wl[idx].m_wlMssidVar[i].wlWpa, "tkip") == 0)
            strncpy(m_instance_wl[idx].m_wlMssidVar[i].wlWpa, "tkip+aes", sizeof(m_instance_wl[idx].m_wlMssidVar[i].wlWpa));
    }
    return;
}


#if 0
static unsigned int bits_count(unsigned int n)
{
    unsigned int count = 0;
    while ( n > 0 ) {
        if ( n & 1 )
            count++;
        n >>= 1;
    }
    return count;
}
void wlcsm_dm_tr98_postStart(int idx)
{
    if( wlcsm_dm_tr98_getCoreRev(idx) >= 40 ) {
        char tmp[100], prefix[] = "wlXXXXXXXXXX_";
        int txchain;
        snprintf(prefix, sizeof(prefix), "%s_", m_instance_wl[idx].m_ifcName[0]);
        wlcsm_dm_tr98_getVarFromNvram(&txchain, strcat_r(prefix, "txchain", tmp), "int");
        if( bits_count((unsigned int) txchain) > 1)
            m_instance_wl[idx].m_wlVar.wlTXBFCapable=1;
    }
}
#endif

/***************************************************************************
// Function Name: init.
// Description  : create WlMngr object
// Parameters   : none.
// Returns      : n/a.
****************************************************************************/
void wlcsm_dm_tr98_init(int idx)
{
    char prefix[]="wl";
    int i=0;
    m_instance_wl[idx].m_refresh = FALSE;
    m_instance_wl[idx].wlInitDone = FALSE;
    wlcsm_dm_tr98_initNvram(idx);
    wlcsm_dm_tr98_retrieve(idx, FALSE);
    wlcsm_dm_tr98_initVars(idx);
    m_instance_wl[idx].m_unit = atoi(m_instance_wl[idx].m_ifcName[0]+strlen(prefix));
    m_instance_wl[idx].numStations = 0;
    m_instance_wl[idx].stationList = NULL;
    m_instance_wl[idx].onBridge = FALSE;
    act_wl_cnt = 0;
    for (i=0; i<wl_cnt; i++)
        act_wl_cnt += ((m_instance_wl[i].m_wlVar.wlEnbl == TRUE) ? 1 : 0);
    wlWriteMdmOne(idx);
    m_instance_wl[idx].wlInitDone = TRUE;
    wldsltr_get(idx);
    if ( wlWriteMdmTr69Cfg( idx )  != CMSRET_SUCCESS)
        printf("%s@%d wlWriteTr69Cfg Error\n", __FUNCTION__, __LINE__ );
    wlReadMdmTr69Cfg( idx );
}

void wlcsm_dm_tr98_retrieve(int idx, int isDefault)
{
    int i;
    char lan[32], buf[128];
    int br, cnt;

    for (i = 0; i < 30; i++) {
        if (wlReadMdm() != CMSRET_SUCCESS)
            sleep(1);
        else
            break;
    }
    if (i>=30)
        printf("wlcsm_dm_tr98_retrieve() failed!\n");

    WLCSM_TRACE(WLCSM_TRACE_LOG," ReadMdm done!!!\r\n");

    /* for each adapter */
    for (cnt=0; cnt<wl_cnt; cnt++) {
        /* for each intf */
        for (i = 0 ; i<WL_NUM_SSID; i++) {
            br = m_instance_wl[cnt].m_wlMssidVar[i].wlBrName[2] - 0x30;

            /* Set the OOB state */
            if ( br == 0 )
                snprintf(lan, sizeof(lan), "lan_wps_oob");
            else
                snprintf(lan, sizeof(lan), "lan%d_wps_oob", br);
            strncpy(buf, nvram_safe_get(lan), sizeof(buf));
            if ( buf[0]=='\0')
                continue;
#ifdef SUPPORT_WSC
            if (!WLCSM_STRCMP(buf,"enabled")) {
                strncpy(m_instance_wl[cnt].m_wlMssidVar[i].wsc_config_state, "0", 2);
            } else {
                strncpy(m_instance_wl[cnt].m_wlMssidVar[i].wsc_config_state, "1", 2);
            }
#endif

        }
    }

    WLCSM_TRACE(WLCSM_TRACE_LOG,"............\r\n");
    for (i = 0 ; i<WL_NUM_SSID; i++) {
        if ( i==0 )
            snprintf(m_instance_wl[idx].m_ifcName[i], sizeof(m_instance_wl[idx].m_ifcName[i]), "wl%d", idx);
        else
            snprintf(m_instance_wl[idx].m_ifcName[i], sizeof(m_instance_wl[idx].m_ifcName[i]), "wl%d.%d", idx, i);

    }
    strncpy(m_instance_wl[idx].m_wlVar.wlWlanIfName, m_instance_wl[idx].m_ifcName[0], sizeof(m_instance_wl[idx].m_wlVar.wlWlanIfName));
    strncpy(m_instance_wl[idx].m_wlVar.wlPhyType, wlcsm_dm_tr98_getPhyType(idx), sizeof(m_instance_wl[idx].m_wlVar.wlPhyType));
    m_instance_wl[idx].m_wlVar.wlCoreRev = wlcsm_dm_tr98_getCoreRev(idx);
    m_instance_wl[idx].m_wlVar.wlBand = wlcsm_dm_tr98_getValidBand(idx, m_instance_wl[idx].m_wlVar.wlBand);
    m_instance_wl[idx].m_wlVar.wlRate = wlcsm_dm_tr98_getValidRate(idx, m_instance_wl[idx].m_wlVar.wlRate);
    m_instance_wl[idx].m_wlVar.wlMCastRate = wlcsm_dm_tr98_getValidRate( idx, m_instance_wl[idx].m_wlVar.wlMCastRate);
    wlcsm_dm_tr98_getVer(idx);
    nmode_no_tkip(idx);
    wlcsm_dm_tr98_NvramMapping(idx, MAP_TO_NVRAM);
}



//**************************************************************************
// Function Name: initNvram
// Description  : initialize nvram settings if any.
// Parameters   : None.
// Returns      : None.
//**************************************************************************
void wlcsm_dm_tr98_initNvram(int idx)
{
    char *pos, *name=NULL, *val=NULL;
    char *str = malloc(MAX_NVRAM_SPACE*sizeof(char));
    int len;
    char pair[WL_LG_SIZE_MAX+WL_SIZE_132_MAX];
    int pair_len_byte = DEFAULT_PAIR_LEN_BYTE;
    int flag = 0;
    char format[]="%0?x"; //to store format string, such as %02x or %03x
    *str = '\0';
    wlReadNvram(str, MAX_NVRAM_SPACE);
    if (str[0] != '\0') {
        /* Parse str: format is xxname1=val1xxname2=val2xx...: xx is the len of pair of name&value */
        for (pos=str; *pos; ) {
            if( strlen(pos)< pair_len_byte) {
                printf("nvram date corrupted[%s]..\n", pos);
                free(str);
                return;
            }
            strncpy(pair, pos, pair_len_byte);
            pair[pair_len_byte]='\0';
            sprintf(format, "%s%d%s", "%0", pair_len_byte, "x"); //format set to "%02%d%s" or "%03%d%s"
            if ( sscanf(pair, format, &len) !=1 ) {
                printf("len error [%s]\n", pair);
                free(str);
                return;
            }
            if (len < pair_len_byte || len>= WL_LG_SIZE_MAX+WL_SIZE_132_MAX) {
                printf("corrupted nvam...\n");
                free(str);
                return;
            }
            if (strlen(pos+pair_len_byte) < len ) {
                printf("nvram date corrupted[%s]..\n", pos);
                free(str);
                return;
            }
            strncpy(pair, pos+pair_len_byte, len);
            pair[len]='\0';
            pos += pair_len_byte+len;
            name = pair;
            val = strchr(pair, '=');
            if (val) {
                *val = '\0';
                val++;
                if(flag==0 && !WLCSM_STRCMP(name,"pair_len_byte"))
                    pair_len_byte=atoi(val);
                if(strncmp(val,"*DEL*",5))
                    wlcsm_dm_tr98_nvram_set( name, val);
                flag = 1;
            } else {
                printf("pair not patch.[%s]..\n", pair);
                free(str);
                return;
            }
        }
    }
    free(str);
}

/***************************************************************************
// Function Name: initVars.
// Description  : initialize vars based on run-time info
// Parameters   : none.
// Returns      : n/a.
****************************************************************************/
void wlcsm_dm_tr98_initVars(int idx)
{
    char buf[WL_LG_SIZE_MAX];
    char cmd[WL_LG_SIZE_MAX];
    int ret;

    m_instance_wl[idx].numBss = m_instance_wl[idx].maxMbss = WL_LEGACY_MSSID_NUMBER;
    m_instance_wl[idx].mbssSupported = FALSE;
    m_instance_wl[idx].aburnSupported=TRUE;
    m_instance_wl[idx].amsduSupported=FALSE;

    //retrive wlan driver info
    snprintf(cmd, sizeof(cmd), "wl%d", idx);
    ret = wl_iovar_get(cmd, "cap", (void *)buf, WL_LG_SIZE_MAX);
    if (ret) {
        fprintf(stderr, "%s, get wl%d cap failed, ret=%d\n", __FUNCTION__, idx, ret);
    } else {
        if(strstr(buf, "1ssid")) {
            m_instance_wl[idx].numBss=1;
            /* for PSI comptibility */
            m_instance_wl[idx].mbssSupported=FALSE;
        }
        if(strstr(buf, "mbss4")) {
            m_instance_wl[idx].numBss=m_instance_wl[idx].maxMbss = 4;
            m_instance_wl[idx].mbssSupported=TRUE;
        }
        // we are limiting it to 4 for now.
        if(strstr(buf, "mbss8")) {
            m_instance_wl[idx].numBss=m_instance_wl[idx].maxMbss = WL_MAX_NUM_SSID>8?8:WL_MAX_NUM_SSID;
            m_instance_wl[idx].mbssSupported=TRUE;
        }
        if(strstr(buf, "mbss16")) {
            m_instance_wl[idx].numBss=m_instance_wl[idx].maxMbss = WL_MAX_NUM_SSID>16?16:WL_MAX_NUM_SSID;
            m_instance_wl[idx].mbssSupported=TRUE;
        }
        if(strstr(buf, "afterburner")) {
            m_instance_wl[idx].aburnSupported=TRUE;
        }
        if(strstr(buf, "ampdu")) {
            m_instance_wl[idx].ampduSupported=TRUE;
        } else {
            /* A-MSDU doesn't work with AMPDU */
            if(strstr(buf, "amsdu")) {
                m_instance_wl[idx].amsduSupported=TRUE;
            }
        }
        if(strstr(buf, "wme")) {
            m_instance_wl[idx].wmeSupported=TRUE;
        }
        if(strstr(buf, "wmf")) {
            m_instance_wl[idx].wmfSupported=TRUE;
        }

        m_instance_wl[idx].wmfSupported=TRUE; /* temporarly make wmf eanbled unconditionally, 43602 cap forgot to inlcude wmf? */
#ifdef DUCATI
        if(strstr(buf, "vec")) {
            m_instance_wl[idx].wlVecSupported=TRUE;
        }
#endif
        if(strstr(buf, "sta")) {
            m_instance_wl[idx].apstaSupported=TRUE;
            snprintf(cmd, sizeof(cmd), "wl%d_apsta", idx);
            wlcsm_dm_tr98_nvram_set(cmd, "1");
        }
        if(strstr(buf, "media")) {
            m_instance_wl[idx].wlMediaSupported=TRUE;
        }
    }

    wlcsm_dm_tr98_nvram_set("router_disable", "0");
}

//**************************************************************************
// Function Name: wlcsm_dm_tr98_getVarFromNvram
// Description  : retrieve var from nvram by name
// Parameters   : var, name, and type
// Returns      : none
//**************************************************************************
void wlcsm_dm_tr98_getVarFromNvram(void *var, const char *name, const char *type)
{
    char temp_s[256] = {0};
    int len;

    strncpy(temp_s, nvram_safe_get(name), sizeof(temp_s));

    if(!WLCSM_STRCMP(type,"int")) {
        if(*temp_s)
            *(int*)var =(int)_strtol_(temp_s);
        else
            *(int*)var = 0;
    } else if(!WLCSM_STRCMP(type,"string")) {
        if(*temp_s) {
            len = strlen(temp_s);

            /* Don't truncate tail-space if existed for SSID string */
            if (strstr(name, "ssid") == NULL) {
                if ((strstr(name, "wpa_psk") == NULL) && (len > 0) && (temp_s[len - 1] == ' '))
                    temp_s[len - 1] = '\0';
            }
            strcpy((char*)var,temp_s);
        } else {
            *(char*)var = 0;
        }
    } else {
        printf("wlcsm_dm_tr98_getVarFromNvram:type not found\n");
    }
}

//**************************************************************************
// Function Name: setVarFromNvram
// Description  : set var to nvram by name
// Parameters   : var, name, and type
// Returns      : none
//**************************************************************************
void wlcsm_dm_tr98_setVarToNvram(void *var, const char *name, const char *type)
{
    char temp_s[100] = {0};
    int len;

    if(!WLCSM_STRCMP(type,"int")) {
        snprintf(temp_s, sizeof(temp_s), "%d",*(int*)var);
        wlcsm_dm_tr98_nvram_set(name, temp_s);
    } else if(!WLCSM_STRCMP(type,"string") && ((!var) ||
              ((!WLCSM_STRCMP((char*)var,"")) || (!strncmp((char*)var,"*DEL*",5))))) {
        nvram_unset(name);
    } else if(!WLCSM_STRCMP(type,"string")) {
        if (var != NULL)
            len = strlen(var);
        else
            len = 0;

        /* Don't truncate tail-space if existed for SSID string */
        if (strstr(name, "ssid") == NULL) {
            if ((strstr(name, "wpa_psk") == NULL) && (len > 0) && (((char*)var)[len - 1] == ' '))
                ((char *)var)[len - 1] = '\0';
        }
        wlcsm_dm_tr98_nvram_set(name, (char*)var);
    } else {
        printf("wlcsm_dm_tr98_setVarToNvram:type not found\n");
    }
}


//**************************************************************************
// Function Name: wlcsm_dm_tr98_NvramMapping
// Description  : retrieve/set nvram parameters for applications IPC
// Parameters   : dir (direction)
// Returns      : None
//**************************************************************************
void wlcsm_dm_tr98_NvramMapping(int idx, const int dir)
{
    char tmp[100], tmp_s[256],prefix[] = "wlXXXXXXXXXX_";
    char name[32];
    int  maxMacFilterBlockSize= WL_MACFLT_NUM * WL_MACADDR_SIZE + 2; //! boundary issue
    char *nvram_val,*tmp_s2= malloc(maxMacFilterBlockSize);
    int i=0,set,val;
    //int set,i=0,br, cnt, found, br_num,val;
    memset(tmp_s2, 0, maxMacFilterBlockSize);
    set = (dir==MAP_TO_NVRAM)? 1:0;
    snprintf(prefix, sizeof(prefix), "%s_", m_instance_wl[idx].m_ifcName[0]);
    if(set)
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlPhyType, strcat_r(prefix, "phytype", tmp), "string");

    if(set) {
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSaveEnable, strcat_r(prefix, "rxchain_pwrsave_enable", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSaveQuietTime, strcat_r(prefix, "rxchain_pwrsave_quiet_time", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSavePps, strcat_r(prefix, "rxchain_pwrsave_pps", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlEnbl,strcat_r(prefix, "radio", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlStbcRx,strcat_r(prefix, "stbc_rx", tmp), "int");
        switch(m_instance_wl[idx].m_wlVar.wlStbcTx) {
        case 0:
            snprintf(name,sizeof(name),"off");
            break;
        case 1:
            snprintf(name,sizeof(name),"on");
            break;
        default:
            snprintf(name,sizeof(name),"auto");
            break;
        }
        wlcsm_dm_tr98_setVarToNvram(&name, strcat_r(prefix, "stbc_tx", tmp), "string");

    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlEnbl,strcat_r(prefix, "radio", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSaveEnable, strcat_r(prefix,"rxchain_pwrsave_enable", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSaveQuietTime, strcat_r(prefix, "rxchain_pwrsave_quiet_time", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRxChainPwrSavePps, strcat_r(prefix, "rxchain_pwrsave_pps", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlStbcRx, strcat_r(prefix, "stbc_rx", tmp), "int");
        nvram_val=wlcsm_nvram_get(strcat_r(prefix, "stbc_tx", tmp));
        if(nvram_val) {
            if(!strncmp(nvram_val,"on",2))
                m_instance_wl[idx].m_wlVar.wlStbcTx=1;
            else if(!strncmp(nvram_val,"off",3))
                m_instance_wl[idx].m_wlVar.wlStbcTx=0;
            else
                m_instance_wl[idx].m_wlVar.wlStbcTx=-1;
        }
    }

    if(set) {
        for (i = 0; i < WL_NUM_SSID; i++) {
            snprintf(name, sizeof(name), "%s_", m_instance_wl[idx].m_ifcName[i]);
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].bssMacAddr[i],                         strcat_r(name, "hwaddr", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlOperMode,            strcat_r(name, "mode", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnblSsid,            strcat_r(name, "bss_enabled", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlSsid,                strcat_r(name, "ssid", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpa,                 strcat_r(name, "crypto", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpaPsk,              strcat_r(name, "wpa_psk", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlRadiusKey,           strcat_r(name, "radius_key", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpaGTKRekey,         strcat_r(name, "wpa_gtk_rekey", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(inet_ntoa(m_instance_wl[idx].m_wlMssidVar[i].wlRadiusServerIP), strcat_r(name, "radius_ipaddr", tmp),"string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlRadiusPort,          strcat_r(name, "radius_port", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlNetReauth,           strcat_r(name, "net_reauth", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlPreauth,             strcat_r(name, "preauth", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlMFP,                 strcat_r(name, "mfp", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlSsdType,             strcat_r(name, "ssd_type", tmp), "int");
#ifdef SUPPORT_WSC
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wsc_mode,              strcat_r(name, "wps_mode", tmp), "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wsc_config_state,      strcat_r(name, "wps_config_state", tmp), "string");
#endif
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlMaxAssoc,            strcat_r(name, "bss_maxassoc", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAPIsolation,         strcat_r(name, "ap_isolate", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlHide,                strcat_r(name, "closed", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnableWmf,           strcat_r(name, "wmf_bss_enable", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlDisableWme,          strcat_r(name, "wme_bss_disable", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlFltMacMode,          strcat_r(name, "macmode", tmp), "string");
            wlcsm_dm_tr98_getFltMacList(idx,i, tmp_s2, maxMacFilterBlockSize);
            wlcsm_dm_tr98_setVarToNvram(tmp_s2, strcat_r(name, "maclist", tmp), "string");

            /*auth_mode*//* Network auth mode (radius|none) */
            /*akm*//* WPA akm list (wpa|wpa2|psk|psk2) */
            if(!strcmp(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode, WL_AUTH_OPEN))
            {
                nvram_unset(strcat_r(name, "auth_mode", tmp));
                nvram_unset(strcat_r(name, "akm", tmp));
            } 
            else if(!strcmp(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode, WL_AUTH_RADIUS))
            {
                strncpy(tmp_s, WL_AUTH_RADIUS, sizeof(tmp_s));
                wlcsm_dm_tr98_setVarToNvram(tmp_s, strcat_r(name, "auth_mode", tmp), "string");  
                nvram_unset(strcat_r(name, "akm", tmp));
            }
            else
            {
                strncpy(tmp_s, "none", sizeof(tmp_s));
                wlcsm_dm_tr98_setVarToNvram(tmp_s, strcat_r(name, "auth_mode", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode, strcat_r(name, "akm", tmp), "string");
            }
        }
    } else {
        for (i = 0; i < WL_NUM_SSID; i++) {
            snprintf(name, sizeof(name), "%s_", m_instance_wl[idx].m_ifcName[i]);
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].bssMacAddr[i],                       strcat_r(name, "hwaddr", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnblSsid,          strcat_r(name, "bss_enabled", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlOperMode,          strcat_r(name, "mode", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlSsid,              strcat_r(name, "ssid", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpa,               strcat_r(name, "crypto", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpaPsk,            strcat_r(name, "wpa_psk", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlRadiusKey,         strcat_r(name, "radius_key", tmp),  "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWpaGTKRekey,       strcat_r(name, "wpa_gtk_rekey", tmp),  "int");
            wlcsm_dm_tr98_getVarFromNvram(tmp, strcat_r(name, "radius_ipaddr", tmp), "string");
            m_instance_wl[idx].m_wlMssidVar[i].wlRadiusServerIP.s_addr = inet_addr(tmp);
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlRadiusPort,        strcat_r(name, "radius_port", tmp),  "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlNetReauth,         strcat_r(name, "net_reauth", tmp),  "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlPreauth,           strcat_r(name, "preauth", tmp),  "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlMFP,               strcat_r(name, "mfp", tmp),  "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlSsdType,           strcat_r(name, "ssd_type", tmp),  "int");
#ifdef SUPPORT_WSC
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wsc_config_state,            strcat_r(name, "wps_config_state", tmp), "string");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wsc_mode,            strcat_r(name, "wps_mode", tmp), "string");
#endif
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlMaxAssoc,          strcat_r(name, "bss_maxassoc", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAPIsolation,       strcat_r(name, "ap_isolate", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlHide,              strcat_r(name, "closed", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnableWmf,         strcat_r(name, "wmf_bss_enable", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlDisableWme,        strcat_r(name, "wme_bss_disable", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlFltMacMode,        strcat_r(name, "macmode", tmp), "string");
            /* Mac Filter */
            tmp_s2[0] = '\0';
            strncpy(tmp_s2, nvram_safe_get(strcat_r(name, "maclist", tmp)), maxMacFilterBlockSize);
            BcmWl_removeAllFilterMac(idx,i);
            if(tmp_s2[0]!='\0') {
                char* tokens;
                int count=0;
                tokens=strtok(tmp_s2, " ");
                while(tokens!=NULL && count < WL_MACFLT_NUM) {
                    BcmWl_addFilterMac2(tokens, m_instance_wl[idx].m_wlMssidVar[i].wlSsid, m_instance_wl[idx].m_ifcName[i], idx, i);
                    tokens=strtok(NULL, " ");
                    count++;
                }
                fprintf(stderr, "More than count=%d %d mac addresses will be dropped\n", count, WL_MACFLT_NUM);
                memset(tmp_s2, 0, maxMacFilterBlockSize);
                wlcsm_dm_tr98_getFltMacList(idx,i, tmp_s2, maxMacFilterBlockSize);
                wlcsm_dm_tr98_setVarToNvram(tmp_s2, strcat_r(name, "maclist", tmp), "string");
            }
            /*auth_mode*//* Network auth mode (radius|none) */
            /*akm*//* WPA akm list (wpa|wpa2|psk|psk2) */
            wlcsm_dm_tr98_getVarFromNvram(tmp_s, strcat_r(name, "auth_mode", tmp), "string");           
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode, strcat_r(name, "akm", tmp), "string");
            if (!strcmp(tmp_s, WL_AUTH_RADIUS))
                strncpy(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode, WL_AUTH_RADIUS, sizeof(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode));

            if (m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode[0] == '\0')
                strncpy(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode,WL_AUTH_OPEN, sizeof(m_instance_wl[idx].m_wlMssidVar[i].wlAuthMode));
        }
    }

    if(set) {
#if 0
        offsetof(,wlObssCoex),"obss_coex","int"
        offsetof(,wlNMcsidx),"nmcsidx","int"
        offsetof(,wlDfsPreIsm),"dfs_preism","int"
        offsetof(,wlDfsPostIsm),"dfs_postism","int"
        offsetof(,wlTpcDb),"tpc_db","int"
        offsetof(,wlChannel),"channel","int"
        offsetof(,wlRate, strcat_r(prefix),"rate","int"
                 offsetof(,wlMCastRate),"mrate","int"
                 offsetof(,wlBasicRate),"rateset","string"
                 offsetof(,wlPreambleType),"plcphdr","string"
                 offsetof(,wlCountry),"country_code","string"
                 offsetof(,wlRegRev),"country_rev","int"
                 offsetof(,wlgMode),"gmode","int"
                 offsetof(,wlProtection),"gmode_protection","string"
                 offsetof(,wlBcnIntvl),"bcn","int"
                 offsetof(,wlDtmIntvl),"dtim","int"
                 offsetof(,wlRtsThrshld),"rts","int"
                 offsetof(,wlFrgThrshld),"frag","int"
                 offsetof(,wlFrameBurst),"frameburst","string"
                 offsetof(,wlGlobalMaxAssoc),"maxassoc","int"
                 offsetof(,wlHide), "closed", "int");
        offsetof(,wlEnableBFR),"txbf_bfr_cap","int"
        offsetof(,wlEnableBFE),"txbf_bfe_cap","int"
        offsetof(,bsdRole),"bsd_role","int"
        offsetof(,bsdHport),"bsd_hport","int"
        offsetof(,bsdPport),"bsd_pport","int"
        offsetof(,bsdHelper),"bsd_helper","string"
        offsetof(,bsdPrimary),"bsd_primary","string"
        offsetof(,ssdEnable),"ssd_enable","int"
        offsetof(,wlTafEnable),"taf_enable","int"
        offsetof(,wlAtf, strcat_r(prefix),"atf","int"
                 offsetof(,wlPspretendThreshold),"pspretend_threshold","int"
                 offsetof(,wlPspretendRetryLimit),"pspretend_retry_limit","int"
                 offsetof(,wlAcsFcsMode),"acs_fcs_mode","int"
                 offsetof(,wlAcsDfs),"acs_dfs","int"
                 offsetof(,wlAcsCsScanTimer),"acs_cs_scan_timer","int"
                 offsetof(,wlAcsCiScanTimer),"acs_ci_scan_timer","int"
                 offsetof(,wlAcsCiScanTimeout),"acs_ci_scan_timeout","int"
                 offsetof(,wlAcsScanEntryExpire),"acs_scan_entry_expire","int"
                 offsetof(,wlAcsTxIdleCnt),"acs_tx_idle_cnt","int"
                 offsetof(,wlAcsChanDwellTime),"acs_chan_dwell_time","int"
                 offsetof(,wlAcsChanFlopPeriod),"acs_chan_flop_period","int"
                 offsetof(,wlIntferPeriod),"intfer_period","int"
                 offsetof(,wlIntferCnt),"intfer_cnt","int"
                 offsetof(,wlIntferTxfail),"intfer_txfail","int"
                 offsetof(,wlIntferTcptxfail),"intfer_tcptxfail","int"
                 offsetof(,wlAcsDfsrImmediate),"acs_dfsr_immediate","string"
                 offsetof(,wlAcsDfsrDeferred),"acs_dfsr_deferred","string"
                 offsetof(,wlAcsDfsrActivity),"acs_dfsr_activity","string"
#else
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlObssCoex, strcat_r(prefix, "obss_coex", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlNMcsidx, strcat_r(prefix, "nmcsidx", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlDfsPreIsm, strcat_r(prefix, "dfs_preism", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlDfsPostIsm, strcat_r(prefix, "dfs_postism", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlTpcDb, strcat_r(prefix, "tpc_db", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlChannel, strcat_r(prefix, "channel", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRate, strcat_r(prefix, "rate", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlMCastRate, strcat_r(prefix, "mrate", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlBasicRate, strcat_r(prefix, "rateset", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlPreambleType, strcat_r(prefix, "plcphdr", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlCountry, strcat_r(prefix, "country_code", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRegRev, strcat_r(prefix, "country_rev", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlgMode, strcat_r(prefix, "gmode", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlProtection, strcat_r(prefix, "gmode_protection", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlBcnIntvl, strcat_r(prefix, "bcn", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlDtmIntvl, strcat_r(prefix, "dtim", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlRtsThrshld, strcat_r(prefix, "rts", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlFrgThrshld, strcat_r(prefix, "frag", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlFrameBurst, strcat_r(prefix, "frameburst", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlGlobalMaxAssoc, strcat_r(prefix, "maxassoc", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[MAIN_BSS_IDX].wlHide, strcat_r(prefix, "closed", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlEnableBFR, strcat_r(prefix, "txbf_bfr_cap", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlEnableBFE, strcat_r(prefix, "txbf_bfe_cap", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.bsdRole, strcat_r(prefix, "bsd_role", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.bsdHport, strcat_r(prefix, "bsd_hport", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.bsdPport, strcat_r(prefix, "bsd_pport", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.bsdHelper, strcat_r(prefix, "bsd_helper", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.bsdPrimary, strcat_r(prefix, "bsd_primary", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.ssdEnable, strcat_r(prefix, "ssd_enable", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlTafEnable, strcat_r(prefix, "taf_enable", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAtf, strcat_r(prefix, "atf", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlPspretendThreshold, strcat_r(prefix, "pspretend_threshold", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlPspretendRetryLimit, strcat_r(prefix, "pspretend_retry_limit", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsFcsMode, strcat_r(prefix, "acs_fcs_mode", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfs, strcat_r(prefix, "acs_dfs", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsCsScanTimer, strcat_r(prefix, "acs_cs_scan_timer", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsCiScanTimer, strcat_r(prefix, "acs_ci_scan_timer", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsCiScanTimeout, strcat_r(prefix, "acs_ci_scan_timeout", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsScanEntryExpire, strcat_r(prefix, "acs_scan_entry_expire", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsTxIdleCnt, strcat_r(prefix, "acs_tx_idle_cnt", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsChanDwellTime, strcat_r(prefix, "acs_chan_dwell_time", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsChanFlopPeriod, strcat_r(prefix, "acs_chan_flop_period", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlIntferPeriod, strcat_r(prefix, "intfer_period", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlIntferCnt, strcat_r(prefix, "intfer_cnt", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlIntferTxfail, strcat_r(prefix, "intfer_txfail", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlIntferTcptxfail, strcat_r(prefix, "intfer_tcptxfail", tmp), "int");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrImmediate, strcat_r(prefix, "acs_dfsr_immediate", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrDeferred, strcat_r(prefix, "acs_dfsr_deferred", tmp), "string");
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrActivity, strcat_r(prefix, "acs_dfsr_activity", tmp), "string");
#endif
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlObssCoex, strcat_r(prefix, "obss_coex", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlNMcsidx, strcat_r(prefix, "nmcsidx", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlDfsPreIsm, strcat_r(prefix, "dfs_preism", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlDfsPostIsm, strcat_r(prefix, "dfs_postism", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlTpcDb, strcat_r(prefix, "tpc_db", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlChannel, strcat_r(prefix, "channel", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRate, strcat_r(prefix, "rate", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlMCastRate, strcat_r(prefix, "mrate", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlBasicRate, strcat_r(prefix, "rateset", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlPreambleType, strcat_r(prefix, "plcphdr", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlCountry, strcat_r(prefix, "country_code", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRegRev, strcat_r(prefix, "country_rev", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlgMode, strcat_r(prefix, "gmode", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlProtection, strcat_r(prefix, "gmode_protection", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlBcnIntvl, strcat_r(prefix, "bcn", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlDtmIntvl, strcat_r(prefix, "dtim", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlRtsThrshld, strcat_r(prefix, "rts", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlFrgThrshld, strcat_r(prefix, "frag", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlFrameBurst, strcat_r(prefix, "frameburst", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlGlobalMaxAssoc, strcat_r(prefix, "maxassoc", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[MAIN_BSS_IDX].wlHide, strcat_r(prefix, "closed", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlEnableBFR, strcat_r(prefix, "txbf_bfr_cap", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlEnableBFE, strcat_r(prefix, "txbf_bfe_cap", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.bsdRole, strcat_r(prefix, "bsd_role", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.bsdHport, strcat_r(prefix, "bsd_hport", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.bsdPport, strcat_r(prefix, "bsd_pport", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.bsdHelper, strcat_r(prefix, "bsd_helper", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.bsdPrimary, strcat_r(prefix, "bsd_primary", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.ssdEnable, strcat_r(prefix, "ssd_enable", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlTafEnable, strcat_r(prefix, "taf_enable", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAtf, strcat_r(prefix, "atf", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlPspretendThreshold, strcat_r(prefix, "pspretend_threshold", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlPspretendRetryLimit, strcat_r(prefix, "pspretend_retry_limit", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsFcsMode, strcat_r(prefix, "acs_fcs_mode", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfs, strcat_r(prefix, "acs_dfs", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsCsScanTimer, strcat_r(prefix, "acs_cs_scan_timer", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsCiScanTimer, strcat_r(prefix, "acs_ci_scan_timer", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsCiScanTimeout, strcat_r(prefix, "acs_ci_scan_timeout", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsScanEntryExpire, strcat_r(prefix, "acs_scan_entry_expire", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsTxIdleCnt, strcat_r(prefix, "acs_tx_idle_cnt", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsChanDwellTime, strcat_r(prefix, "acs_chan_dwell_time", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsChanFlopPeriod, strcat_r(prefix, "acs_chan_flop_period", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlIntferPeriod, strcat_r(prefix, "intfer_period", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlIntferCnt, strcat_r(prefix, "intfer_cnt", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlIntferTxfail, strcat_r(prefix, "intfer_txfail", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlIntferTcptxfail, strcat_r(prefix, "intfer_tcptxfail", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrImmediate, strcat_r(prefix, "acs_dfsr_immediate", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrDeferred, strcat_r(prefix, "acs_dfsr_deferred", tmp), "string");
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlAcsDfsrActivity, strcat_r(prefix, "acs_dfsr_activity", tmp), "string");
    }

    /*Save WEP key and wep Index (Only works with Idx=1 now */
    if(set) {
#if 0
        int ssid_idx = m_instance_wl[idx].m_wlVar.wlSsidIdx;
        if (m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeyBit == WL_BIT_KEY_64 )
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeyIndex64,  "wl_key", "int");
        else
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeyIndex128, "wl_key", "int");

        if (m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeyBit == WL_BIT_KEY_64 ) {
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys64[0],  "wl_key1", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys64[1],  "wl_key2", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys64[2],  "wl_key3", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys64[3],  "wl_key4", "string");
        } else {
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys128[0],  "wl_key1", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys128[1],  "wl_key2", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys128[2],  "wl_key3", "string");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[ssid_idx].wlKeys128[3],  "wl_key4", "string");
        }

#endif
        for (i = 0; i < WL_NUM_SSID; i++) {
            snprintf(tmp_s, sizeof(tmp_s), "%s_", m_instance_wl[idx].m_ifcName[i]);

            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnableHspot,         strcat_r(tmp_s, "hspot", tmp), "int");

            if (m_instance_wl[idx].m_wlMssidVar[i].wlKeyBit == WL_BIT_KEY_64 )
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeyIndex64,  strcat_r(tmp_s, "key", tmp), "int");
            else
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeyIndex128,  strcat_r(tmp_s, "key", tmp), "int");

            if (m_instance_wl[idx].m_wlMssidVar[i].wlKeyBit == WL_BIT_KEY_64 ) {
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[0],  strcat_r(tmp_s, "key1", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[1],  strcat_r(tmp_s, "key2", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[2],  strcat_r(tmp_s, "key3", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[3],  strcat_r(tmp_s, "key4", tmp), "string");
            } else {
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[0],  strcat_r(tmp_s, "key1", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[1],  strcat_r(tmp_s, "key2", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[2],  strcat_r(tmp_s, "key3", tmp), "string");
                wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[3],  strcat_r(tmp_s, "key4", tmp), "string");
            }
        }
    } else {
        for (i = 0; i < WL_NUM_SSID; i++) {
            snprintf(tmp_s, sizeof(tmp_s), "%s_", m_instance_wl[idx].m_ifcName[i]);

            if (m_instance_wl[idx].m_wlMssidVar[i].wlKeyBit == WL_BIT_KEY_64 )
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeyIndex64,  strcat_r(tmp_s, "key", tmp), "int");
            else
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeyIndex128,  strcat_r(tmp_s, "key", tmp), "int");

            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlEnableHspot,       strcat_r(tmp_s, "hspot", tmp), "int");

            if (m_instance_wl[idx].m_wlMssidVar[i].wlKeyBit == WL_BIT_KEY_64) {
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[0], strcat_r(tmp_s, "key1", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[1], strcat_r(tmp_s, "key2", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[2], strcat_r(tmp_s, "key3", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys64[3], strcat_r(tmp_s, "key4", tmp), "string");
            } else {
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[0], strcat_r(tmp_s, "key1", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[1], strcat_r(tmp_s, "key2", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[2], strcat_r(tmp_s, "key3", tmp), "string");
                wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlKeys128[3], strcat_r(tmp_s, "key4", tmp), "string");
            }
        }
    }

    for (i = 0; i < WL_NUM_SSID; i++) {
        snprintf(tmp_s, sizeof(tmp_s), "%s_", m_instance_wl[idx].m_ifcName[i]);
        if(set) {
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAuth, strcat_r(tmp_s, "auth", tmp), "int");
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWep,  strcat_r(tmp_s, "wep", tmp), "string");
        } else {
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlAuth, strcat_r(tmp_s, "auth", tmp), "int");
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlMssidVar[i].wlWep, strcat_r(tmp_s, "wep", tmp), "string");
        }
    }

    if(set) {
        wlcsm_dm_tr98_getWdsMacList(idx, tmp_s, sizeof(tmp_s));
        wlcsm_dm_tr98_setVarToNvram(&tmp_s, strcat_r(prefix, "wds", tmp), "string");

        switch (m_instance_wl[idx].m_wlVar.wlLazyWds) {
        case WL_BRIDGE_RESTRICT_ENABLE:
            val = 0;
            break;
        case WL_BRIDGE_RESTRICT_ENABLE_SCAN:
            val = 0;
            break;
        case WL_BRIDGE_RESTRICT_DISABLE:
        default:
            val = 1;
            break;
        }
        wlcsm_dm_tr98_setVarToNvram(&val, strcat_r(prefix, "lazywds", tmp), "int");
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlLazyWds, strcat_r(prefix, "lazywds", tmp), "int");
        wlcsm_dm_tr98_getVarFromNvram(&tmp_s, strcat_r(prefix, "wds", tmp), "string");
        wlcsm_dm_tr98_setWdsMacList(idx, tmp_s, sizeof(tmp_s));
    }
    /* WEP over WDS */
    if (set) {
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlWdsSecEnable, strcat_r(prefix, "wdssec_enable", tmp), "int");
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlWdsSecEnable, strcat_r(prefix, "wdssec_enable", tmp), "int");
    }

    /*URE*/
    if(set) {
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].apstaSupported, strcat_r(prefix, "apsta", tmp), "int");
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].apstaSupported, strcat_r(prefix, "apsta", tmp), "int");
    }

    /* nband */
    if(set) {
        m_instance_wl[idx].m_wlVar.wlNBand = m_instance_wl[idx].m_wlVar.wlBand;
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlNBand, strcat_r(prefix, "nband", tmp), "int");
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlNBand, strcat_r(prefix, "nband", tmp), "int");
        m_instance_wl[idx].m_wlVar.wlBand = m_instance_wl[idx].m_wlVar.wlNBand;
    }

    if(set) {
        wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlNBwCap, strcat_r(prefix, "bw_cap", tmp), "int");
    } else {
        wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlNBwCap, strcat_r(prefix, "bw_cap", tmp), "int");
    }

    /* nctlsb */
    if(set) {
        WLCSM_TRACE(WLCSM_TRACE_LOG,"mdm  get wlNCtrlsb:%d\r\n",m_instance_wl[idx].m_wlVar.wlNCtrlsb);
        char *tmp_str_ptr=wlcsm_dm_get_mapper_str(_WLCSM_MNGR_STRMAPPER_WLCTRLSB,m_instance_wl[idx].m_wlVar.wlNCtrlsb,1,&i);
        if(tmp_str_ptr && !i) {
            WLCSM_TRACE(WLCSM_TRACE_LOG,"nctrlsb nvram value:%s\r\n",tmp_str_ptr);
            nvram_set(strcat_r(prefix, "nctrlsb", tmp),tmp_str_ptr);
        }
    } else {
        char *tmp_str_ptr=nvram_get(strcat_r(prefix, "nctrlsb",tmp));
        if(tmp_str_ptr) {
            val=wlcsm_dm_get_mapper_int(_WLCSM_MNGR_STRMAPPER_WLCTRLSB ,tmp_str_ptr,1,&i);
            if(!i)
                m_instance_wl[idx].m_wlVar.wlNCtrlsb=val;
            else
                fprintf(stderr, "%s:%d WRONG NCTRLSB string value:%s\n",__FUNCTION__,__LINE__,tmp_str_ptr);
            WLCSM_TRACE(WLCSM_TRACE_LOG,"mdm set wlNCtrlsb :%s:%d\r\n",tmp_str_ptr,m_instance_wl[idx].m_wlVar.wlNCtrlsb);
        }
    }

    if(MIMO_PHY) {
        /* nmode */
        if(set) {
            val = AUTO_MODE;
            if (!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlNmode, WL_OFF))
                val = OFF;
            else if (!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlNmode, WL_ON))
                val = ON;
            wlcsm_dm_tr98_setVarToNvram(&val, strcat_r(prefix, "nmode", tmp), "int");
        } else {
            val = 0;
            wlcsm_dm_tr98_getVarFromNvram(&val, strcat_r(prefix, "nmode", tmp), "int");
            if (val == ON)
                snprintf(m_instance_wl[idx].m_wlVar.wlNmode, sizeof(m_instance_wl[idx].m_wlVar.wlNmode), WL_ON);
            else if (val == OFF)
                snprintf(m_instance_wl[idx].m_wlVar.wlNmode, sizeof(m_instance_wl[idx].m_wlVar.wlNmode), WL_OFF);
            else
                snprintf(m_instance_wl[idx].m_wlVar.wlNmode, sizeof(m_instance_wl[idx].m_wlVar.wlNmode), WL_AUTO);
        }

        /* nmode_protection */
        if(set) {
            wlcsm_dm_tr98_setVarToNvram(&m_instance_wl[idx].m_wlVar.wlNProtection, strcat_r(prefix, "nmode_protection", tmp), "string");
        } else {
            wlcsm_dm_tr98_getVarFromNvram(&m_instance_wl[idx].m_wlVar.wlNProtection, strcat_r(prefix, "nmode_protection", tmp), "string");
        }

        /* rifs_advert */
        memset(name, 0, sizeof(name));
        if(set) {
            if (m_instance_wl[idx].m_wlVar.wlRifsAdvert != 0)
                snprintf(name, sizeof(name), WL_AUTO);
            else
                snprintf(name, sizeof(name), WL_OFF);
            wlcsm_dm_tr98_setVarToNvram(&name, strcat_r(prefix, "rifs_advert", tmp), "string");
        } else {
            wlcsm_dm_tr98_getVarFromNvram(&name, strcat_r(prefix, "rifs_advert", tmp), "string");
            if (!WLCSM_STRCMP(name, WL_AUTO))
                m_instance_wl[idx].m_wlVar.wlRifsAdvert = AUTO_MODE;
            else
                m_instance_wl[idx].m_wlVar.wlRifsAdvert = OFF;
        }

        /* obss_coex */
    }

    free(tmp_s2);
}

