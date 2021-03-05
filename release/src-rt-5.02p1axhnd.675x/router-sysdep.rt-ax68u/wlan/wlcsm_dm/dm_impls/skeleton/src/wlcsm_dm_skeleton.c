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
 *	@file	 wlcsm_dm_skeleton.c
 *	@brief	 wlcsm data model for skeleton APIs
 *
 * 	this file will handle wlmngr data structure and mapping to skeleton data mapping
 * 	this file will be mostly like wlmdm in previous implementation.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
#include <errno.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_dm.h"
#include "wlcsm_dm_skeleton.h"


int _wlcsm_dm_skeleton_obj_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
{
#ifdef WLCSM_DEBUG
    WLCSM_DM_OBJ_VALUE_SET *data= (WLCSM_DM_OBJ_VALUE_SET *)buf;
    WLCSM_TRACE(WLCSM_TRACE_DBG," wlmngr:oid:%u,offset:%u and change to value:%s \r\n",data->oid,data->offset,data->value );
#endif
    return 0;
}

int _wlcsm_dm_skeleton_sta_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
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

static int _wlcsm_dm_save_wifi()
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: save wifi configuration to DM\r\n");
    return 0;
}

static int _wlcsm_dm_skeleton_save_radio(int idx,int withlock)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: save radio configuration to DM\r\n");
    return 0;
}

static int _wlcsm_dm_skeleton_save_bssid(int idx,int sub_idx,int requestlock)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: save bssid configuration to DM\r\n");
    return 0;
}

static int _wlcsm_dm_skeleton_save_access_point(int idx,int sub_idx,int requestlock)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: save AP configuration to DM\r\n");
    return 0;
}


static int __wlcsm_dm_skeleton_save_config(int idx,int requestlock)
{

    int ret=0,i=0;
    int num_of_bssids= wlcsm_dm_get_bssid_num(idx);

    ret=_wlcsm_dm_skeleton_save_radio(idx,requestlock);
    WLCSM_TRACE(WLCSM_TRACE_LOG," ==>>>>>bssid_num:%d \r\n",gp_adapter_objs[idx-1].radio.wlNumBss );
    if(!ret) {

        for ( i=1; i<=num_of_bssids; i++ ) {
            ret=_wlcsm_dm_skeleton_save_bssid(idx,i,requestlock);
            WLCSM_TRACE(WLCSM_TRACE_LOG,"JXUJXU:%s:%d ----LOADING BSSID DONE----- \r\n",__FUNCTION__,__LINE__ );
            if(ret || (ret=_wlcsm_dm_skeleton_save_access_point(idx,i,requestlock)))
                return ret;
        }
    }
    return ret;

}

static int _wlcsm_dm_skeleton_save_config(int idx,int from)
{
    int start_idx,end_idx,ret=0;

    if(idx) {
        start_idx=idx-1;
        end_idx=start_idx;
    } else {
        start_idx=0;
        end_idx= WL_WIFI_RADIO_NUMBER-1;
    }

    for(idx=start_idx; idx<=end_idx; idx++) {
        ret=__wlcsm_dm_skeleton_save_config(idx,0); /* save without request lock */
        if(ret) break; //if not success, break;
    }
    return ret;

}




static int _wlcsm_dm_skeleton_load_radio(int idx)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO:load radio to wlmngr,default to enable maybe\r\n");
    return 0;
}


static int _wlcsm_dm_skeleton_load_bssid(int idx,int sub_idx)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO:load bssid to wlmngr\r\n");
    return 0;


}

static int _wlcsm_dm_skeleton_load_access_point(int idx,int sub_idx)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO:load AP to wlmngr and set intefaces names, also bridge names\r\n");
    return 0;
}


/*************************************************************//**
 * @brief  load all configuration to manager's structure
 *
 * When idx=0, it will load all Radio's configuration
 *
 * @return  int
 ***************************************************************/
static int _wlcsm_dm_skeleton_load_config(int idx,int from)
{
    int i,start_idx,end_idx,ret=0;

    if(idx) {
        start_idx=idx-1;
        end_idx=idx-1;
    } else {
        start_idx=0;
        end_idx= WL_WIFI_RADIO_NUMBER-1;
    }

    for(idx=start_idx; idx<=end_idx; idx++) {

        ret=_wlcsm_dm_skeleton_load_radio(idx);
        if(!ret) {

            for ( i=1; i<WL_MBSS_NUM(idx); i++ ) {
                ret=_wlcsm_dm_skeleton_load_bssid(idx,i);
                if(ret || (ret=_wlcsm_dm_skeleton_load_access_point(idx,i)))
                    return ret;
            }
        } else
            break;
    }
    return ret;
}

int _wlcsm_dm_skeleton_save_nvram(void)
{
    return   _wlcsm_dm_save_wifi();
}


static int _wlcsm_dm_skeleton_init(void)
{

    /*  This is the FIRST function entry point when this DM is being selected  */
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: allocate resource and necessary init for this DM \r\n");
    return 0;
}


static int _wlcsm_dm_skeleton_deinit(void)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: free all resources being used by this DM\r\n");
    return 0;
}

int _wlcsm_dm_skeleton_oid_mngr_name(char *buf)
{
#if 0
    unsigned int *nums=(unsigned int *)buf;
    unsigned int oid=nums[0];
    unsigned int offset=nums[1];
#endif
    WLCSM_TRACE(WLCSM_TRACE_DBG,"Given DM OID number and offset, to get wlmngr variable's name\r\n");
    return -1;
}


int _wlcsm_dm_skeleton_getbridge_info(char *buf)
{
    int ret=0;
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: reture bridge configuration in format like: 'br0:eth0:eth1:wl0:wl0 br1:eth3:eth4:wl1' \r\n");
    return ret;
}

int _wlcsm_dm_skeleton_query_info(WLCSM_DM_QUERY_CMD cmd,void *buf)
{
    int ret;
    switch ( cmd ) {
    case WLCSM_DM_QUERY_BRIDGE_INFO:
        ret=_wlcsm_dm_skeleton_getbridge_info((char *)buf);
        break;
    case WLCSM_DM_QUERY_MNGR_ENTRY:
        WLCSM_TRACE(WLCSM_TRACE_DBG," skeleton get mngr entry \r\n" );
        ret=_wlcsm_dm_skeleton_oid_mngr_name((char *)buf);
        break;
    case WLCSM_DM_QUERY_SETDBGLEVEL: 
        WLCSM_TRACE(WLCSM_TRACE_LOG,"dbglevel:%s\r\n",buf);
        ret=CMSRET_SUCCESS;
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

DECLARE_WLCSM_DM_ITEM(skeleton);
