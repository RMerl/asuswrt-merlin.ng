/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
*
*    Copyright (c) 2013 Broadcom
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
 *	@file	 wlcsm_dm_nocms.c
 *	@brief	 wlcsm data model for nocms APIs
 *
 * 	this file will handle wlmngr data structure and mapping to nocms data mapping
 * 	this file will be mostly like wlmdm in previous implementation.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
//#include <bcmnvram.h>
#include <errno.h>
#include "wlcsm_lib_api.h"
#include "wlcsm_dm_nocms.h"
#include <bcmnvram.h>

#include <wlcsm_lib_wl.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_nvram.h"

int _wlcsm_dm_nocms_obj_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
{
    return 0;
}


int _wlcsm_dm_nocms_sta_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
{
    size_t ret_size = 0;
    WL_STALIST_SUMMARIES *sta_summaries = NULL;
    if (buf) {
        sta_summaries = (WL_STALIST_SUMMARIES *)buf;
        ret_size = (sta_summaries->num_of_stas) * sizeof(WL_STATION_LIST_ENTRY);
        ret_size += sizeof(WL_STALIST_SUMMARIES);
    }

    return 0;
}


static int _wlcsm_dm_nocms_save_config(int idx,int from)
{

    char *name;
    FILE *fp;
    fp=fopen("/data/nvram_cfg.txt","w+");
    if(fp) {
        char *buf = malloc(MAX_NVRAM_SPACE);
        if(!buf) {
            fprintf(stderr,"could not allocate memory for buf\n");
            return -1;
        }
        wlcsm_nvram_getall(buf, MAX_NVRAM_SPACE*sizeof(char));
        for (name = buf; *name; name += strlen(name) + 1) {
            if(!strncmp(name,"wps_force_restart", strlen("wps_force_restart"))
               || !strncmp(name, "pair_len_byte=", strlen("pair_len_byte="))
               || !strncmp(name, "acs_ifnames", strlen("acs_ifnames"))
               || !strncmp(name, "wl_unit", strlen("wl_unit"))
               || strstr(name, "_dpd") || strstr(name, "pcie_"))
                continue;
            WLCSM_TRACE(WLCSM_TRACE_DBG,"write :%s to file\r\n",name);
            fprintf(fp,"%s\n",name);
        }
        free(buf);
        fclose(fp);
    }
    return 0;
}


/*************************************************************//**
 * @brief  load all configuration to manager's structure
 *
 * @return  int
 ***************************************************************/
static int _wlcsm_dm_nocms_load_config(int idx,int from)
{
    int ret=0;
    unsigned int	start_idx,end_idx;
    FILE *fp;
    char line[WL_LG_SIZE_MAX],*name,*value,*newline;

    if(idx>WL_WIFI_RADIO_NUMBER) return -1;

    fp=fopen("/data/nvram_cfg.txt","r");
    if(fp) {
        while(fgets(line,sizeof(line),fp)!=NULL) {
            value=line;
            if((newline=strchr(value,'\n'))!=NULL) newline[0]=0;
            name=strsep(&value,"=");
            wlcsm_nvram_set(name,value);
            WLCSM_TRACE(WLCSM_TRACE_DM," name:%s,value:%s\r\n",name,value);
            if(idx>0 && wlcsm_nvram_name_parser(name,&start_idx,&end_idx) && (start_idx!=(idx-1)))
                continue;
            else
                wlcsm_nvram_update_runtime_mngr(name,value);
        }
        fclose(fp);
    }
    return ret;
}

int _wlcsm_dm_nocms_save_nvram(void)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"nocms save nvram does nothing\r\n");
    return 0;
}


/*************************************************************//**
* @brief  default setting to data model
*
* 	init data model with default value, in mdm case,
* 	it should init mdm if the configuration is not there
* 	,something like hardwareAdjusting. after this function
* 	,data model should have at least the default configruation
* 	 or loaded saved configuration.
*
* @return  int
***************************************************************/
static int _wlcsm_dm_nocms_init(void)
{
    WLCSM_TRACE(WLCSM_TRACE_LOG,"nocms init do not need to do anything for now\r\n");
    return 0;
}


static int _wlcsm_dm_nocms_deinit(void)
{
    return 0;
}

int _wlcsm_dm_nocms_oid_mngr_name(char *buf)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"NOT APPLICABLE\r\n");
    return -1;
}


int _wlcsm_dm_nocms_getbridge_info(char *buf)
{
    int ret=0;
    strcpy(buf,"br0:eth0:eth1:wl0");
    WLCSM_TRACE(WLCSM_TRACE_LOG,"TODO: reture bridge configuration \r\n");
    return ret;
}

int _wlcsm_dm_nocms_query_info(WLCSM_DM_QUERY_CMD cmd,void *buf)
{
    int ret=0;
    switch ( cmd ) {
    case WLCSM_DM_QUERY_BRIDGE_INFO:
        ret=_wlcsm_dm_nocms_getbridge_info((char *)buf);
        break;
    case WLCSM_DM_QUERY_MNGR_ENTRY:
        WLCSM_TRACE(WLCSM_TRACE_LOG," nocms get mngr entry \r\n" );
        ret=_wlcsm_dm_nocms_oid_mngr_name((char *)buf);
        break;
    case WLCSM_DM_QUERY_SETDBGLEVEL: 
        WLCSM_TRACE(WLCSM_TRACE_LOG,"dbglevel:%s\r\n",buf);
        break;
    case WLCSM_DM_QUERY_LOCK: {
        WLCSM_TRACE(WLCSM_TRACE_DBG," DM LOCK implemantion request\r\n");
    }
    break;
    default:
        ret=-1;
        break;
    }
    return ret;

}

DECLARE_WLCSM_DM_ITEM(nocms);
