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
 *	This file is for wlmngr special vars handling
 *
*/

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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

#ifdef SUPPORT_WSC
#include <time.h>
#endif
#include <wlcsm_lib_dm.h>
#include <wlcsm_lib_api.h>
#include <bcmwifi_channels.h>
#include "wlcsm_lib_nvram.h"
#include "wlcsm_lib_wl.h"

#define POSSIBLE_CHANNELS_TOTAL_LEN  (512)
static int _g_wlrefesh=1;

typedef struct {

    char *var;
    mngr_special_var_handler handler;

} SPECIALVAR_HANDLER;

static char wlmngr_get_radio_band_char(unsigned int idx)
{
    switch (WL_RADIO_WLBAND(idx)) {
    case BAND_6G:
        return '6';
    case BAND_A:
        return '5';
    case BAND_B:
    default:
        return '2';
    }
}

int wlmngr_update_possibble_channels(unsigned int idx,SYNC_DIRECTION direction)
{
    int orig_wlnbw= WL_RADIO_WLNBWCAP(idx);
    char  band, bw[8], cmd[128];
    char wlchfile[32];
    struct stat statbuf;

    band = wlmngr_get_radio_band_char(idx);
    snprintf(wlchfile, 31, "/var/wl%dchannels", idx);

    switch(orig_wlnbw) {
    case WLC_BW_CAP_40MHZ:
        snprintf(bw, 7, "40");
        if (WL_RADIO_WLNCTRLSB(idx) == WL_CTL_SB_UPPER)
            snprintf(cmd, 127, "wl -i wl%d chanspecs -b %c -w %s -c %s|grep u > %s", idx, band, bw, WL_RADIO_WLCOUNTRY(idx), wlchfile);
        else
            snprintf(cmd, 127, "wl -i wl%d chanspecs -b %c -w %s -c %s|grep l > %s", idx, band, bw, WL_RADIO_WLCOUNTRY(idx), wlchfile);
        break;
    case  WLC_BW_CAP_80MHZ:
        snprintf(bw, 7, "80");
        break;
    case  WLC_BW_CAP_160MHZ:
        snprintf(bw, 7, "160");
        break;
    case WLC_BW_CAP_20MHZ:
        snprintf(bw, 7, "20");
        break;
    default:
        WLCSM_TRACE(WLCSM_TRACE_ERR," WRONG WLNBWCAP\r\n");
        return -1;
    }

    if (orig_wlnbw != WLC_BW_CAP_40MHZ) {
        snprintf(cmd, 127, "wlctl -i wl%d chanspecs -b %c -w %s -c %s > %s", idx, band, bw, WL_RADIO_WLCOUNTRY(idx), wlchfile);
    }

    BCMWL_WLCTL_CMD(cmd);

    if (!stat(wlchfile, &statbuf)) {
        FILE *fp = fopen(wlchfile, "r");
        if (fp != NULL) {
            char channels[POSSIBLE_CHANNELS_TOTAL_LEN]= {0}, tmp[32], *ptr,control_channel[8];
            ptr = channels;
            while (fscanf(fp, "%s%s\n", control_channel, tmp) != EOF) {
                ptr += sprintf(ptr, "%s,", control_channel);
                if(ptr>(channels+POSSIBLE_CHANNELS_TOTAL_LEN-8)) {
                    fprintf(stderr,"%s:%d SHOULD ALLOCATE MORE MEMORY FOR CHANNELS  \r\n",__FUNCTION__,__LINE__ );
                    break;
                }
            }
            if (ptr != channels && *(ptr-1) == ',')
                *(ptr-1) = '\0';

            wlcsm_strcpy(&WL_RADIO_POSSIBLECHANNELS(idx), channels);
            fclose(fp);
        }
    }
    return 0;
}


void wlmngr_nv_adjust_chanspec(unsigned int idx,SYNC_DIRECTION direction)
{
    char value[20]= {0};
    char name[20]= {0};
    snprintf(name, sizeof(name), "%s_chanspec",WL_BSSID_WLIFNAME(idx,0));

    if (WL_RADIO_WLBAND(idx) == BAND_6G) {
        snprintf(value, sizeof(value), "6g%d",WL_RADIO_WLCHANNEL(idx));
    } else {
        snprintf(value, sizeof(value), "%d", WL_RADIO_WLCHANNEL(idx));
    }

    if (MIMO_PHYY(idx) && WL_RADIO_WLCHANNEL(idx)) {
        if (WL_BW_CAP_160MHZ(WL_RADIO_WLNBWCAP(idx)))
            strcat(value, "/160");
        else if (WL_BW_CAP_80MHZ(WL_RADIO_WLNBWCAP(idx)))
            strcat(value, "/80");
        else if (WL_BW_CAP_40MHZ(WL_RADIO_WLNBWCAP(idx))) {
            WLCSM_TRACE(WLCSM_TRACE_DBG,"WL_RADIO_WLNCTRLSB(idx) :%d\r\n",WL_RADIO_WLNCTRLSB(idx));
            if (WL_RADIO_WLNCTRLSB(idx) == WL_CTL_SB_UPPER)
                strcat(value, "u");
            else if (WL_RADIO_WLNCTRLSB(idx) == WL_CTL_SB_LOWER)
                strcat(value, "l");
        }
    }

    WLCSM_TRACE(WLCSM_TRACE_DBG,"name:%s,value:%s\r\n",name,value);
    if (WL_RADIO_WLBAND(idx) == BAND_A && WL_RADIO_WLCHANNEL(idx)<=CH_MAX_2G_CHANNEL) {
        nvram_set(name,"0");
        wlcsm_dm_mngr_set_all_value(idx,0,"wlChannel","0");
    } else {
        nvram_set(name,value);
    }
}
#define  NUM_OF_SPECIAL_HANDLER    (sizeof(g_mngr_special_handlers)/sizeof(SPECIALVAR_HANDLER))

/*************************************************************//**
 * @brief  wl_refresh handler
 *
 * httpd sometimes to set wlrefresh to control when to show the informaiton,but in new
 * TR181, we hardly need this mechanism, but to be back compatible, we will still provid
 * this handler and most likely always return 0,such as in cgi_wl.c
 * ~~~~
 * 	cgiRefreshPage("/web/wlrefresh.html","wlmacflt.cmd?action=view",fs);
 *~~~~
 * @return int
 ***************************************************************/
static int  _wlmngr_wlrefresh_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    switch (op  ) {
    case WLMNGR_VAR_OPER_SET:
        if(value)
            _g_wlrefesh=atoi(value);
        WLCSM_TRACE(WLCSM_TRACE_LOG,"  G_WLREFRESH SET...........:%d \r\n",_g_wlrefesh );
        break;
    case WLMNGR_VAR_OPER_GET:
    case WLMNGR_VAR_OPER_VALIDATE:
        snprintf(value, WL_MID_SIZE_MAX, "%d",_g_wlrefesh);
        WLCSM_TRACE(WLCSM_TRACE_LOG,"  G_WLREFRESH GET...........:%d \r\n",_g_wlrefesh );
        return strlen(value)+1;
    default:
        value[0]='\0';
        break;
    }
    /* internal var, always return Success */
    return 1;
}

#ifdef SUPPORT_MIMO
static int _wlmngr_wps_wlcurrentsb_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{

    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    if(op==WLMNGR_VAR_OPER_GET) {
        int chanspec = wlmngr_getCurrentChSpec(radio_idx);
        if (((chanspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT) == 3) { /* 40MHz */
            switch ((chanspec & WL_CHANSPEC_CTL_SB_MASK) >> WL_CHANSPEC_CTL_SB_SHIFT)	{
            case (WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT):
                sprintf(varValue, "%s", "Lower");
                break;

            case (WL_CHANSPEC_CTL_SB_UPPER >> WL_CHANSPEC_CTL_SB_SHIFT):
                sprintf(varValue, "%s", "Upper");
                break;

            default:
                sprintf(varValue, "%s", "N/A");
                break;
            }
        } else
            sprintf(varValue, "%s", "N/A"); /* 20 and 80Mhz */
        return strlen(varValue)+1;
    } else  {
        varValue[0]='\0';
    }
    return 1;
}
#endif

static int  _wlmngr_bsd_role_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op) {
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    if(op==WLMNGR_VAR_OPER_SET) {
        int i=0;
        for (i=0; i<WL_WIFI_RADIO_NUMBER;  i++ ) {
            if(i!=radio_idx)
                wlcsm_dm_mngr_set_all_value(i,0,"bsdRole",value);
        }
        wlcsm_nvram_set("bsd_role",value);
    }
    return 0;
}

static int  _wlmngr_wlwds_stsc_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    if(op==WLMNGR_VAR_OPER_SET) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," reset scan result list \r\n" );
        list_del_all( (WL_RADIO(radio_idx).m_tblScanWdsMac), struct wl_flt_mac_entry);
        WLCSM_TRACE(WLCSM_TRACE_LOG," reset scan result list done\r\n" );
    } else  {
        value[0]='\0';
        WLCSM_TRACE(WLCSM_TRACE_LOG," TODO... stsc..:op:%d \r\n",op );
    }
    return 1;
}

static int  _wlmngr_wlwds_addmac_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    if(op==WLMNGR_VAR_OPER_SET) {

        char * wlwds=WL_RADIO_WLWDS(radio_idx);
        if(!wlwds || strlen(wlwds)>((WL_WDS_NUM-1)*WL_MACADDR_SIZE))
            return wlcsm_dm_mngr_set_all_value(radio_idx,0,"wlWds",value);
        else if(!strstr(wlwds,value)) {
            int str_len=strlen(wlwds)+WL_MACADDR_SIZE+1;
            char *newwds=malloc(str_len);
            if (newwds) {
                snprintf(newwds, str_len, "%s %s",wlwds,value);
                wlcsm_dm_mngr_set_all_value(radio_idx,0,"wlWds",newwds);
                free(newwds);
            } else return  WLMNGR_VAR_HANDLE_FAILURE;
        }
    } else  {
        value[0]='\0';
        WLCSM_TRACE(WLCSM_TRACE_LOG," TODO... wlWds..:op:%d \r\n",op );
    }
    return 1;
}

#ifdef __CONFIG_VISUALIZATION__
static int  _wlmngr_wlvisaulization_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *value, t_WLMNGR_VAR_OPER_ENUM op)
{
    if(op==WLMNGR_VAR_OPER_GET) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," get VISUALIZATION \r\n" );
        sprintf(value,"%d",1);
        return strlen(value)+1;
    } else {
        /* internal var, alway return 1 and has only GET operation */
        value[0]='\0';
        return 1;
    }
}
#endif

static int  _wlmngr_countryrev_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op) {

    unsigned int idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    if(op==WLMNGR_VAR_OPER_SET && varValue) {
        char ccode[4]= {0};
        char  s_regrv[4]= {0};
        char  regrev=-1;
        if(!wlcsm_wl_parse_countryrev(varValue,ccode,&regrev)) {
            sprintf(s_regrv,"%d",regrev);
            //wlcsm_dm_mngr_set_all_value(idx, 0, "wlRegRev", s_regrv);
            wlcsm_dm_mngr_set_all_value(idx, 0, "wlCountry", ccode);
            WLCSM_TRACE(WLCSM_TRACE_LOG," country:%s,reg:%d \r\n",WL_RADIO_COUNTRY(idx),WL_RADIO_REGREV(idx) );
        } else {
            WLCSM_TRACE(WLCSM_TRACE_ERR," !!!!ADJUST COUNTRY FAILURE!!!!  \r\n" );
            WLCSM_TRACE(WLCSM_TRACE_ERR," country:%s,reg:%d \r\n",WL_RADIO_COUNTRY(idx),WL_RADIO_REGREV(idx) );
        }
        return 1;
    }
    return 0;
}

static int  _wlmngr_dmname_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    if(op==WLMNGR_VAR_OPER_GET) {
        char *dmname=wlcsm_dm_get_dmname();
        WLCSM_TRACE(WLCSM_TRACE_LOG," get DMNAME\r\n" );
        if(dmname) {
            sprintf(varValue, "%s", dmname);
            return strlen(varValue)+1;
        } else return  WLMNGR_VAR_HANDLE_FAILURE;
    } else {
        /* internal var, alway return 1 and has only GET operation */
        varValue[0]='\0';
        return 1;
    }
}

static int  _wlmngr_wlsyncnvram_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    /* in TR181, nvram and wlmngr variable always synched,provide the handler only to work with existing HTTP*/
    if(op==WLMNGR_VAR_OPER_GET) {
        sprintf(varValue, "%d", 1);
        return 2;
    } else {
        /* internal var, alway return 1 and has only GET operation */
        varValue[0]='\0';
        return 1;
    }
}


#ifdef SUPPORT_WSC
static int _wlmngr_wps_wlforecewps_handler(t_WLCSM_MNGR_VARHDR *hdr,char *name,char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    unsigned int sub_idx=hdr->sub_idx;
    int i=0;
    if(op==WLMNGR_VAR_OPER_GET) {
        int wpsDisable=0;
        if (nvram_match("wps_version2", "enabled")) {
            /* Checke Hidden Access point: if there is any of Hiden(include virtual), wpsDisable=1  */
            for (i=MAIN_BSS_IDX; i<WL_RADIO_WLMAXMBSS(radio_idx); i++) {
                if ((WL_BSSID_WLENBLSSID(radio_idx,i) == 1) &&
                        (WL_AP_WLHIDE(radio_idx,i) == 1)) {
                    wpsDisable = 1;
                    break;
                }
            }
            /* Checke Mac filter: if 'Allow' is choosed and no any MAC address is in filter list, wpsDisable=1 */
            if (!wpsDisable) {
                if ((WLCSM_STRCMP(WL_AP_WLFLTMACMODE(radio_idx,sub_idx), "allow") == 0) && WL_AP_WLFLTMACLIST(radio_idx,sub_idx)==NULL)
                    wpsDisable = 1;
            }
            snprintf(varValue, WL_MID_SIZE_MAX, "%d", wpsDisable);
        } else
            varValue[0]='\0';

    } else
        varValue[0]='\0';

    return strlen(varValue)+1;
}

#endif

static int _wlmngr_channel_handler(t_WLCSM_MNGR_VARHDR *hdr, char *name, char *varValue, t_WLMNGR_VAR_OPER_ENUM op)
{
    unsigned int idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    switch( op ) {
    case WLMNGR_VAR_OPER_GET:
        if (!WLCSM_STRCMP(name, "channelsInUse") || !WLCSM_STRCMP(name, "wlCurrentChannel")) {
            wlmngr_getCurrentChannel(idx);
            snprintf(varValue, WL_MID_SIZE_MAX, "%d", WL_RADIO_WLCURRENTCHANNEL(idx));
            return strlen(varValue)+1;
        }
        break;
    case WLMNGR_VAR_OPER_SET:
        if (!WLCSM_STRCMP(name, "wlChannel") || !WLCSM_STRCMP(name, "wlCurrentChannel")) {
            wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "wlChannel", varValue);
            if (!WLCSM_STRCMP(varValue, "0")) {   //Auto Channel Enabled
                char buf[4] = "1";
                wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "autoChannelEnable", buf);
            }

        } else if (!WLCSM_STRCMP(name, "autoChannelEnable")) {
            char buf[WL_MID_SIZE_MAX]= {0};
            if (varValue[0] == '0') {  // disable auto channel selection
                wlmngr_getCurrentChannel(idx);
                snprintf(buf, WL_MID_SIZE_MAX, "%d", WL_RADIO_WLCURRENTCHANNEL(idx));
                wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "wlChannel", buf);
            } else {
                buf[0] = '0';
                wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "wlChannel", buf);
                wlcsm_dm_mngr_set_all_value(idx, hdr->sub_idx, "autoChannelEnable", varValue);
                return 1;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}


SPECIALVAR_HANDLER g_mngr_special_handlers[] = {
    {"*channel",    _wlmngr_channel_handler},
    {"wlRefresh", 	_wlmngr_wlrefresh_handler}, /* used for SES before, not used any more */
    {"wdsStSc", 	_wlmngr_wlwds_stsc_handler}, /* used for start wds start scan */
    {"bsdRole", 	_wlmngr_bsd_role_handler}, /* used for sync bsd_roles */

#ifdef SUPPORT_WSC
    {"wsc_",		wlmngr_wps_wsc_handler},
    {"wlWsc",		wlmngr_wps_wlwsc_handler},
    {"wlIsForceWpsDisable",	_wlmngr_wps_wlforecewps_handler},

#endif
#ifdef SUPPORT_MIMO
    {"wlCurrentSb",		_wlmngr_wps_wlcurrentsb_handler},
#endif

    {"wlSyncNvram", 	_wlmngr_wlsyncnvram_handler},
    {"sessionKey", 	NULL},
    {"wdsAddMac", 	_wlmngr_wlwds_addmac_handler}, /* used for add wds mac to the list*/
#ifdef __CONFIG_VISUALIZATION__
    {"wlVisualization", _wlmngr_wlvisaulization_handler},
#endif
    {"_dmname_", 	_wlmngr_dmname_handler},
    {"wlCountryRev", 	_wlmngr_countryrev_handler},
};

/**  @brief: function to handle special var
 *
 * 	there are some wlmngr values to be hanled diffently other than simple set/get,for specialvar
 * 	handler, it either return WLMNGR_VAR_HANDLE_FAILURE or return positive number
 *
 *
 *   @return
 *
 *   	0  - means there is no such sepcial var to handle
 *   	>0 - menas handled correctly and the varValue is correctly assigned value with the
 *   	     return as the len of the content filled in the varValue.
 *   	<0 - means errror happend when handle this value
 *
 * */
int wlmngr_handle_special_var(t_WLCSM_MNGR_VARHDR *hdr, char *name,char *value,t_WLMNGR_VAR_OPER_ENUM op)
{
    int i=0;
    SPECIALVAR_HANDLER *special_handler;
    unsigned int radio_idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);

    if(radio_idx>=WL_WIFI_RADIO_NUMBER) return WLMNGR_VAR_HANDLE_FAILURE;
    for (i = 0; i < NUM_OF_SPECIAL_HANDLER; i++) {
        special_handler= &(g_mngr_special_handlers[i]);
        if(special_handler->var[0] == '*' && strcasestr(name, &(special_handler->var[1])) != NULL) {
            if(special_handler->handler) {
                return  special_handler->handler(hdr,name,value,op);
            }
        } else if(!strncmp(name,special_handler->var,strlen(special_handler->var))) {
            if(special_handler->handler)
                return  special_handler->handler(hdr,name,value,op);
        }
    }
    /* return 0 indicates this var is not handled */
    return WLMNGR_VAR_NOT_HANDLED;
}
/**  @brief: function to handle special nvram variable
 *
 * 	there are some nvram values to be hanled diffently other than simple set/get
 *
 * */

int wlmngr_special_nvram_handler(char *name,char *value)
{
    unsigned int idx=0,ssid_idx;
    int i=0,br=0;
    char *nv_name;
    char tmp_str[WL_SIZE_132_MAX]={0};

    if(!strncmp(name,"lan_wps_oob",11)||((sscanf(name,"lan%d_%s",&idx,tmp_str) == 2 ) && (!strcmp(tmp_str,"wps_oob"))) ) {
        for (i=0; i<WL_WIFI_RADIO_NUMBER;  i++ ) {
            for(ssid_idx=0; ssid_idx<WL_RADIO_WLNUMBSS(i); ssid_idx++)  {
                br = WL_BSSID_WLBRNAME(i,ssid_idx)[2] - 0x30;
                if(br==idx) {
                    if (!WLCSM_STRCMP(value,"enabled"))
                        wlcsm_dm_mngr_set_all_value(i,ssid_idx,"wlWscAPMode","0");
                    else
                        wlcsm_dm_mngr_set_all_value(i,ssid_idx,"wlWscAPMode","1");
                }
            }
        }
    } else if( (!strncmp(name,"lan_ifnames",11)) || ( (sscanf(name,"lan%d_%s",&idx,tmp_str) == 2 ) && (!strcmp(tmp_str,"ifnames"))) ) {
        char br_name[WL_MID_SIZE_MAX]={0};
        char lan_ifname[WL_SIZE_132_MAX]={0};
        char *lnext=NULL;

        sprintf(br_name,"br%d",idx);

        for_each(lan_ifname,value,lnext) {
            for (i=0; i<WL_WIFI_RADIO_NUMBER;  i++ ) {
                for(ssid_idx=0; ssid_idx<WL_RADIO_WLNUMBSS(i); ssid_idx++)  {
                    if(!strcmp(lan_ifname,WL_BSSID_IFNAME(i,ssid_idx)))
                    {// Update bridge name
                        wlcsm_dm_mngr_set_all_value(i,ssid_idx,"wlBrName",br_name);
                    }
                }
            }
        }

    } else if(!strncmp(name,"bsd_role",8)) {
        for (i=0; i<WL_WIFI_RADIO_NUMBER;  i++ ) {
            wlcsm_dm_mngr_set_all_value(i,0,"bsdRole",value);
        }
    } else {
        nv_name=wlcsm_nvram_name_parser(name,&idx,&ssid_idx);
        /* the idx has to be in the range */
        if((idx<WL_WIFI_RADIO_NUMBER) && (ssid_idx<WL_RADIO_WLNUMBSS(idx)) && (nv_name)) {
            if(!WLCSM_STRCMP(nv_name,"chanspec")) {
                /* when set wlx.x_chanspec, we will change the other field as we depends on the other to
                 * adjust it at runtime*/
                wlmngr_sign_chan_by_chanspec(idx,ssid_idx,value);
            } else if((!strncmp(nv_name,"key",3)) && (sscanf(nv_name,"key%d",&idx)==1)) {
                if( !value || !strlen(value)) {
                    fprintf(stderr, ":%s:%d:	invalid key setting, ignore\n",__FUNCTION__,__LINE__);
                    return -1;
                }
            } else if(!WLCSM_STRCMP(nv_name,"bw_cap") &&
                      (WL_RADIO_WLCHANNEL(idx) && value && !WLCSM_STRCMP(value,"-1"))) {
                /*if radio chanenel number is not 0(auto channel), then ignore bw_cap -1*/
                return -1;

            }
        }

    }
    return 0;
}

void wlmngr_sign_chan_by_chanspec( uint idx,uint ssid_idx,char *value)
{
    chanspec_t chanspec;
    uint band,nbw,nctrlsb,channel = 0;
    char buf[32];
    if(value) {
        /* first change string chanspec to binary format, then we got
          band | BW | CTRL SB | Channel */
        chanspec = wf_chspec_aton(value);
        channel = wf_chspec_ctlchan(chanspec);
        if(channel) {
            nbw = CHSPEC_BW(chanspec);
            nctrlsb = CHSPEC_CTL_SB(chanspec);
            band= CHSPEC_BAND(chanspec);
            switch(nbw) {
            case WL_CHANSPEC_BW_160:
                nbw=WLC_BW_CAP_160MHZ;
                snprintf(buf,sizeof(buf),"%d",(nctrlsb>>WL_CHANSPEC_CTL_SB_SHIFT)+6);
                wlcsm_dm_mngr_set_all_value(idx,0,"wlNCtrlsb",buf);
                break;
            case WL_CHANSPEC_BW_80:
                nbw=WLC_BW_CAP_80MHZ;
                snprintf(buf,sizeof(buf),"%d",(nctrlsb>>WL_CHANSPEC_CTL_SB_SHIFT)+2);
                wlcsm_dm_mngr_set_all_value(idx,0,"wlNCtrlsb",buf);
                break;
            case  WL_CHANSPEC_BW_40:
                nbw=WLC_BW_CAP_40MHZ;
                switch (nctrlsb) {
                case WL_CHANSPEC_CTL_SB_LOWER:
                    wlcsm_dm_mngr_set_all_value(idx,0,"wlNCtrlsb", "-1");
                    WL_RADIO_WLNCTRLSB(idx) = WL_CTL_SB_LOWER;
                    break;
                case WL_CHANSPEC_CTL_SB_UPPER:
                    wlcsm_dm_mngr_set_all_value(idx,0,"wlNCtrlsb", "1");
                    WL_RADIO_WLNCTRLSB(idx) = WL_CTL_SB_UPPER;
                    break;
                default:
                    wlcsm_dm_mngr_set_all_value(idx,0,"wlNCtrlsb", "0");
                    WL_RADIO_WLNCTRLSB(idx) = WL_CTL_SB_NONE;
                    break;
                }
                break;
            case  WL_CHANSPEC_BW_20:
                nbw=WLC_BW_CAP_20MHZ;
                break;
            }

            switch (band) {
            default:
            case WL_CHANSPEC_BAND_2G:
                sprintf(buf,"%d",BAND_B);
                break;
            case WL_CHANSPEC_BAND_5G:
                sprintf(buf,"%d",BAND_A);
                break;
#ifdef WL_CHANSPEC_BAND_6G
            case WL_CHANSPEC_BAND_6G:
                sprintf(buf,"%d",BAND_6G);
                break;
#endif
            }
            wlcsm_dm_mngr_set_all_value(idx,0,"wlBand",buf);

            sprintf(buf,"%d",nbw);
            wlcsm_dm_mngr_set_all_value(idx,0,"wlNBwCap",buf);
            WLCSM_TRACE(WLCSM_TRACE_DBG, " band:%d,nctrlsb:%d,channel:%d,nbw:%d  \r\n",
                        band,nctrlsb,channel,nbw);
            WL_RADIO_AUTOCHANNELENABLE(idx)=0;
        } else {
            WL_RADIO_AUTOCHANNELENABLE(idx)=1;
        }
        sprintf(buf,"%d",channel);
        wlcsm_dm_mngr_set_all_value(idx,0,"wlChannel",buf);
    }
}

