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

/**
 *	@file	 wlmngr_specialvar.c
 *	@brief
 *
 *	<+detail description+>
 *
 *	@date  	01/22/2015 10:44:15 AM
*/

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

#ifdef DSLCPE_1905
#include <wps_1905.h>
#endif
#include "cms_util.h"
#include "mdm.h"
#include "mdm_private.h"
#include "odl.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "cms_core.h"
#include <pthread.h>
#include <bcmnvram.h>
#include<bcmconfig.h>

#include "wlioctl.h"
#include "wlutils.h"
#include "wlmngr.h"
#include "wllist.h"

#include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <wlcsm_lib_dm.h>
//#include "wlmngr_http.h"

#ifdef SUPPORT_WSC
#include <time.h>
#endif
#ifdef DSLCPE_WLCSM_EXT
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>
#include <wlcsm_linux.h>
#endif


#ifdef SUPPORT_WSC
static int  _wlmngr_wps_nvramvar_handler(t_WLCSM_MNGR_VARHDR *hdr,char * name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{

    switch (op) {
    case WLMNGR_VAR_OPER_SET:

    case WLMNGR_VAR_OPER_GET: {
        char *tmp = nvram_safe_get("wps_device_pin");
        sprintf(varValue, "%s", tmp);
        return strlen(varValue)+1;
    }
    case WLMNGR_VAR_OPER_VALIDATE:
    default:
        break;
    }
    //return 1;
    if(!varValue) {
        nvram_set(name,varValue);
    }
    return 0;
}

static int  _wlmngr_wps_doApScan_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG," TODO: do wps apscan \r\n" );
    return 0;
}

/***************************************************************************
// Function Name: _wlmngr_getScanParam
// Returns      : WdsMacNode or NULL if not found
****************************************************************************/

static void *_wlmngr_getScanParam(int idx,void *pVoid, char *mac, char *ssid, int *privacy, int *wpsConfigured)
{
    WL_FLT_MAC_ENTRY *entry = NULL;

    list_get_next( ((WL_FLT_MAC_ENTRY *)pVoid), WL_RADIO(idx).m_tblScanWdsMac,  entry);
    if ( entry != NULL ) {
        strncpy( mac, entry->macAddress, WL_MID_SIZE_MAX );
        strncpy(ssid, entry->ssid, WL_SSID_SIZE_MAX);
        *privacy = entry->privacy;
        *wpsConfigured = entry->wpsConfigured;
    }
    return (void *)entry;
}

static int  _wlmngr_wps_saveapinfo_fromscanresult_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    switch (op) {
    case WLMNGR_VAR_OPER_SET: {
        int i=0;
        char mac[WL_MID_SIZE_MAX];
        char ssid[WL_SSID_SIZE_MAX];
        void *node = NULL;
        int privacy=0;
        int wpsConfigured=0;
        unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
        int apListIdx= atoi(value);

        node = _wlmngr_getScanParam(radio_idx,node, mac, ssid, &privacy, &wpsConfigured);

        while ( node != NULL ) {
            if (apListIdx== i)	{
                nvram_set("wps_enr_ssid", (char *)ssid);
                nvram_set("wps_enr_bssid", (char *)mac);

                if(privacy == 0)
                    nvram_set("wps_enr_wsec", "0");
                else
                    nvram_set("wps_enr_wsec", "16");

                if(wpsConfigured == 1)
                    nvram_set("wps_ap_configured", "configured");
                else
                    nvram_set("wps_ap_configured", "unconfigured");

                break;
            }
            i++;

            node = _wlmngr_getScanParam(radio_idx,node, mac, ssid, &privacy, &wpsConfigured);
        }
    }
    break;
    default:
        WLCSM_TRACE(WLCSM_TRACE_DBG," TODO: do wps saveapinfo \r\n" );
        break;
    }
    return 0;
}

static int  _wlmngr_wps_configure_change_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    switch (op) {
    case WLMNGR_VAR_OPER_SET: {
        bool cfg_changed = FALSE;
        char *methodValue;
        unsigned short config_method;

        if ((varValue != NULL) && (strlen(varValue) > 0))  {
            nvram_set("_wps_config_method", varValue);
            methodValue = nvram_safe_get("wps_config_method");
            config_method =  strtoul(methodValue, NULL, 16);
            if (!WLCSM_STRCMP(varValue, "ap-pin")) {
                /*  WPS_CONFMET_PBC = 0x0080 */
                if ((config_method & 0x80) == 0x80) {
                    nvram_set("wps_config_method", "0x4");
                    cfg_changed = TRUE;
                }
            } else {
                if ((config_method & 0x80) != 0x80) {
                    set_wps_config_method_default();
                    cfg_changed = TRUE;
                }
            }
        }

        if (cfg_changed == TRUE) { /*  need to restart wps_monitor to take effect */
            WLCSM_TRACE(WLCSM_TRACE_DBG,"  WPS confgiure changed and to restart WPS \r\n" );
            bcmSystem("killall -q -15 wps_monitor 2>/dev/null");
            sleep(1);
            bcmSystem("/bin/wps_monitor&");
        }
        return 1;
    }
    case WLMNGR_VAR_OPER_GET: {
        char *temp=nvram_safe_get("_wps_config_method");
        sprintf(varValue, "%s", temp);
        return strlen(varValue)+1;

    }
    case WLMNGR_VAR_OPER_VALIDATE:
    default:
        break;
    }
    return 1;
}

static int  _wlmngr_wps_configure_apmode_handler(t_WLCSM_MNGR_VARHDR *hdr, char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    unsigned int sub_idx=hdr->sub_idx;
    switch(op) {
    case WLMNGR_VAR_OPER_VALIDATE:
        break;
    case WLMNGR_VAR_OPER_SET: {
        if (varValue != NULL && (!wlcsm_dm_mngr_set_all_value(radio_idx,sub_idx,name,varValue))) {
            char lan[32];
            int i,cnt,br;
            int br_tmp = WL_BSSID_WLBRNAME(radio_idx,sub_idx)[2] - 0x30;
            if ( br_tmp == 0 )
                snprintf(lan, sizeof(lan), "lan_wps_oob");
            else
                snprintf(lan, sizeof(lan), "lan%d_wps_oob", br_tmp);
            if ( !WLCSM_STRCMP(WL_APWPS_WLWSCAPMODE(radio_idx,sub_idx),"0"))
                nvram_set(lan, "enabled");
            else
                nvram_set(lan, "disabled");

            for (cnt=0; cnt<WL_WIFI_RADIO_NUMBER; cnt++) {
                for (i = 0 ; i<WL_RADIO_WLMAXMBSS(cnt); i++) {
                    br = WL_BSSID_WLBRNAME(cnt,i)[2]  - 0x30;
                    if ( br == br_tmp)
                        wlcsm_dm_mngr_set_all_value(cnt,i,name,varValue);
                }
            }
        } else {
            WLCSM_TRACE(WLCSM_TRACE_ERR," wps apmode set error \r\n" );
            return -1;
        }
    }
    case WLMNGR_VAR_OPER_GET:
        return WLMNGR_VAR_NOT_HANDLED;
    default:
        break;
    }
    varValue[0]='\0';
    return 1;
}

/* handle setting "wsc_config_command" var setting from anywhere */
static int  _wlmngr_wps_configure_command_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    if(varValue && *varValue=='1') {
        nvram_set(name,"2");
        usleep(500000);
    }
    nvram_set(name,varValue);
    return 0;
}

#define WPSDEV_PIN_LEN (8)

/* handle setting "wlWscDevPin" var*/
static int  _wlmngr_wps_wlWscDevPin_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    switch (op) {
    case WLMNGR_VAR_OPER_VALIDATE:
    case WLMNGR_VAR_OPER_SET:
        if(!varValue || strlen(varValue)!=WPSDEV_PIN_LEN)  return WLMNGR_VAR_HANDLE_FAILURE;
        if(op==WLMNGR_VAR_OPER_SET) nvram_set("wps_device_pin",varValue);
        varValue[0]='\0';
        break;
    case WLMNGR_VAR_OPER_GET: {
        char *tmp = nvram_safe_get("wps_device_pin");
        sprintf(varValue, "%s", tmp);
        return strlen(varValue)+1;
    }
    default:
        break;
    }
    return 1;
}

/* handle setting "wlWscAuthoStaMac" var setting from anywhere */
static int _wlmngr_wps_wlWscAuthoStaMac_handler(t_WLCSM_MNGR_VARHDR *hdr, char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{

    if(op==WLMNGR_VAR_OPER_SET) {
        if(nvram_match("wps_version2","enabled")) {
            nvram_set("wps_autho_sta_mac",varValue);
        }
    } else {
        return 1;
    }
    return 0;
}

static int _wlmngr_wps_wlWscStaPin_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{

    switch (op) {
    case WLMNGR_VAR_OPER_SET:
        if ((varValue != NULL) && (((strlen(varValue) == 8) || (strlen(varValue) == 4)) ||
                                   (nvram_match("wps_version2", "enabled") && (strlen(varValue) == 9) && (varValue[4] < '0' || varValue[4] > '9')))) {
            if (strlen(varValue) == 9) {
                char pin[8];
                memcpy(pin, varValue+5, 5);
                memcpy(varValue+4, pin, 5);
            }
            if ( wlmngr_wscPincheck(varValue))
                nvram_set("wps_sta_pin", "ErrorPin");
            else
                nvram_set("wps_sta_pin", varValue);
        } else {
            if ( varValue && strlen(varValue) == 0 )
                nvram_set("wps_sta_pin", "00000000");
            else
                nvram_set("wps_sta_pin", "");
        }
        break;
    case WLMNGR_VAR_OPER_GET: {
        char *tmp = nvram_safe_get("wps_sta_pin");
        if (WLCSM_STRCMP(tmp, "00000000") == 0)
            *varValue = 0;
        else
            sprintf(varValue, "%s", tmp);
        return strlen(varValue)+1;
    }
    case WLMNGR_VAR_OPER_VALIDATE:
    default:
        break;
    }
    return 1;
}

int  wlmngr_wps_wsc_handler(t_WLCSM_MNGR_VARHDR *hdr,char * varName,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG," varName:%s ,op:%d\r\n",varName,op );
    /*  {"wsc_config_command"}, */
    if(!WLCSM_STRCMP(varName,"wsc_config_command"))
        return _wlmngr_wps_configure_command_handler(hdr,varName,varValue,op);
    else
        /*  {"wsc_event","wsc_force_restart","wsc_addER","wsc_proc_status"} */
        return _wlmngr_wps_nvramvar_handler(hdr,varName,varValue,op);

}

int  wlmngr_wps_wlwsc_handler(t_WLCSM_MNGR_VARHDR *hdr,char * varName,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{

    unsigned int idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);

    WLCSM_TRACE(WLCSM_TRACE_DBG," varName:%s ,op:%d\r\n",varName,op );

    if(!WLCSM_STRCMP(varName,"wlWscAvail")) { /* checking if WSC is enabled  */
        /* only get for this */
        if(op==WLMNGR_VAR_OPER_GET)  {
            sprintf(varValue,"%s","yes");
        } else
            varValue[0]='\0';
    } else if(WLCSM_STRCMP(varName, "wlWscMode") == 0 ) {
        if (op==WLMNGR_VAR_OPER_SET) {
            char buf[2]= {0};
            buf[0] = WLCSM_STRCMP(varValue, "enabled") ? '0' : '1';
            wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "wpsEnable", buf);
            return WLMNGR_VAR_NOT_HANDLED;
        }
        return WLMNGR_VAR_NOT_HANDLED;
    } else {
        /* the following has nvram to name mapping */
        char *wlWsc_match[]= {
            "wlWscIRMode","wl_wps_reg",
            "wlWscAddER","wps_addER",
            "wlWscConfig","wps_config",
            "wlWscVer2","wps_version2",
            "wlWscApPinGetCfg","wps_ApPinGetCfg",
            NULL
        };
        char **pvar=wlWsc_match;
        while(*pvar) {
            if(!WLCSM_STRCMP(varName,*pvar)) {
                pvar++;
                switch (op) {
                case WLMNGR_VAR_OPER_SET:
                    if(varValue)
                        wlcsm_nvram_set(*pvar,varValue);
                    else
                        wlcsm_nvram_unset(*pvar);
                    break;
                case WLMNGR_VAR_OPER_GET: {
                    char *temp=nvram_get(*pvar);
                    if(temp) {
                        WLCSM_TRACE(WLCSM_TRACE_DBG," %s=%s \r\n", varName, temp );
                        sprintf(varValue, "%s", temp);
                        return strlen(varValue)+1;
                    }
                }
                break;
                case WLMNGR_VAR_OPER_VALIDATE:
                default:
                    break;
                }
                if(varValue) varValue[0]='\0';
                return 1;
            }
            pvar+=2;
        }
        /* it is not nvram type wps variables,has to handle special */
        if(!WLCSM_STRCMP(varName,"wlWpsApScan"))
            return _wlmngr_wps_doApScan_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscApListIdx"))
            return _wlmngr_wps_saveapinfo_fromscanresult_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscCfgMethod"))
            return _wlmngr_wps_configure_change_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscAPMode"))
            return _wlmngr_wps_configure_apmode_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscDevPin"))
            return _wlmngr_wps_wlWscDevPin_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscAuthoStaMac"))
            return _wlmngr_wps_wlWscAuthoStaMac_handler(hdr,varName,varValue,op);
        else if(!WLCSM_STRCMP(varName,"wlWscStaPin"))
            return _wlmngr_wps_wlWscStaPin_handler(hdr,varName,varValue,op);
        else {
            fprintf(stderr,"JXUJXU:%s:%d  !!!!Variable :%s \r\n",__FUNCTION__,__LINE__,varName );
            return WLMNGR_VAR_NOT_HANDLED;
        }
    }
    return strlen(varValue)+1;
}


#endif

