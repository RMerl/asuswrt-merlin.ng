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
 *	@file	 wlcsm_dm_tr181.c
 *	@brief	 wlcsm data model tr181 APIs
 *
 * 	this file will handle wlmngr data structure and mapping to tr-181 data mapping
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
#include "wlcsm_linux.h"
#include "cms.h"

#include "cms_util.h"
#include "mdm.h"
#include "mdm_private.h"
#include "odl.h"

#include "cms_core.h"
#include "cms_dal.h"
#include "cms_cli.h"
#include "cms_log.h"
#include "cms.h"
#include "cms_obj.h"
#include "cms_mdm.h"
#include "cms_mem.h"
#include "cms_mgm.h"
#include "cms_util.h"
#include "cms_data_model_selector.h"
#include "cms_msg.h"
#include "mdm.h"     // for mdmLibCxt, but this file should not access this struct.  Delete later.
#include <board.h>
#include <fcntl.h>
#include "wlcsm_dm_tr181.h"
#include "wlcsm_lib_dm.h"
#include <bcmnvram.h>
#define LOCK_WAIT     (60000)

static void *g_MsgHandle=NULL;

static int _wlcsm_dm_prev_obj_num(int idx) {
    int i=0,prev_bssid_num=0;
    /*calculate how many bssid configuratioins before this one*/
    for(i=1; i<idx; i++)
        prev_bssid_num+=wlcsm_dm_get_bssid_num(i);
    return prev_bssid_num;
}

int _wlcsm_dm_tr181_obj_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
{
#ifdef WLCSM_DEBUG
    WLCSM_DM_OBJ_VALUE_SET *data= (WLCSM_DM_OBJ_VALUE_SET *)buf;
    WLCSM_TRACE(WLCSM_TRACE_DBG," wlmngr:oid:%u,offset:%u and change to value:%s \r\n",data->oid,data->offset,data->value );
#endif
    return 0;
}

int _wlcsm_dm_tr181_sta_change_handler(unsigned int radio_idx,unsigned int sub_idx ,void *buf)
{

    WL_STALIST_SUMMARIES *sta_summaries = NULL;
    size_t payload_len =  0;
    WL_STATION_LIST_ENTRY *pStationEntry;
    WL_STA_EVENT_DETAIL *psta_detail=(WL_STA_EVENT_DETAIL *)buf;
    char associated=0,authorized=0;
    CmsMsgHeader *msg = NULL;
    if(psta_detail->event_type == UPDATE_STA_REMOVE_ALL) {
        /* MDM to handle SSID turn from enable to disable */
        return 0;
    }
    payload_len =   sizeof(WL_STALIST_SUMMARIES) + sizeof(WL_STATION_LIST_ENTRY);
    msg = (CmsMsgHeader *) cmsMem_alloc((sizeof(CmsMsgHeader)+ payload_len), ALLOC_ZEROIZE);

    if (!msg) {
        cmsLog_error("ERROR: Fail to allocate message buffer");
        return 1;
    }
    sta_summaries = (WL_STALIST_SUMMARIES*)(msg+1);
    sta_summaries->radioIdx = radio_idx;
    sta_summaries->ssidIdx= sub_idx;
    sta_summaries->num_of_stas= 1;
    sta_summaries->type= psta_detail->event_type;
    pStationEntry = (WL_STATION_LIST_ENTRY *) sta_summaries->stalist_summary;


    if(psta_detail->event_type!=UPDATE_STA_ALL) {
        memcpy(pStationEntry->macAddress,psta_detail->mac,MACSTR_LEN);
    if(sta_summaries->type== STA_CONNECTED  && wlcsm_wl_sta_assoc_auth_status((char *)WL_BSSID_IFNAME(radio_idx,sub_idx),
            (char *)pStationEntry->macAddress,&associated,&authorized))
        fprintf(stderr, "%s:%d:	error get sta :%s status\n",__FUNCTION__,__LINE__,pStationEntry->macAddress);

    pStationEntry->authorized=authorized;
    pStationEntry->associated=associated;
    }

    msg->src = EID_WLMNGR;
    msg->dst = EID_SSK;
    msg->type = CMS_MSG_WIFI_UPDATE_ASSOCIATEDDEVICE;
    msg->flags_request = 1;
    msg->dataLength = payload_len;
    cmsMsg_send(g_MsgHandle, msg);
    CMSMEM_FREE_BUF_AND_NULL_PTR(msg);
    return 0;
}

/*********************************************************************************//*****
 *  TR181 CMS related functions starts
 **************************************************************************************/
#define MDM_STRCPY(x, y)    if ( (y) != NULL ) \
                    CMSMEM_REPLACE_STRING_FLAGS( (x), (y), mdmLibCtx.allocFlags )

void cmsMdm_cleanup_wrapper()
{
#if defined(SUPPORT_DM_LEGACY98) || defined(SUPPORT_DM_HYBRID)

    cmsMdm_cleanup();

#elif defined(SUPPORT_DM_PURE181)

    /*  In Pure TR181 mode, we never initialized the MDM, so no cleanup */

#elif defined(SUPPORT_DM_DETECT)

    if (cmsUtil_isDataModelDevice2() == 0) {
        cmsMdm_cleanup();
    }

#endif
}

int getShmIdByMsg(void *g_MsgHandle)
{
    int shmId;
    UINT32 timeoutMs=5000;
    CmsRet r2;
    CmsMsgHeader msg = EMPTY_MSG_HEADER;
    msg.src = EID_WLMNGR;
    msg.dst = EID_SMD;
    msg.type = CMS_MSG_GET_SHMID;
    msg.flags_request = 1;

    r2 = cmsMsg_sendAndGetReplyWithTimeout(g_MsgHandle, &msg, timeoutMs);
    if (r2 == CMSRET_TIMED_OUT) { /*  assumes shmId is never 9809, which is value of CMSRET_TIMED_OUT */
        cmsLog_error("could not get shmId from smd (r2=%d)", r2);
        return UNINITIALIZED_SHM_ID;
    }

    shmId = (SINT32) r2;
    cmsLog_debug("got smdId=%d", shmId);

    return shmId;
}

CmsRet cmsMdm_init_wrapper(int *shmId  __attribute__((unused)),
                           void *g_MsgHandle  __attribute__((unused)))
{
    CmsRet ret=CMSRET_SUCCESS;

    if (*shmId == UNINITIALIZED_SHM_ID) {
        *shmId = getShmIdByMsg(g_MsgHandle);
        if (*shmId == UNINITIALIZED_SHM_ID) {
            return CMSRET_INTERNAL_ERROR;
        }
    }

    if ((ret = cmsMdm_initWithAcc(EID_WLMNGR, 0, g_MsgHandle, shmId)) != CMSRET_SUCCESS) {
        cmsLog_error("Could not initialize mdm, ret=%d", ret);
    }
    return ret;
}

#define WLCSM_STRCPY(to,from)  do {\
            char *dst_value=NULL;\
            if(from) {\
                dst_value=malloc(strlen(from)+1); \
                if(dst_value) { \
                strncpy(dst_value,from,strlen(from)+1); \
                            if(to) free(to); \
                to=dst_value; \
                } else \
                return 1;\
           }} while(0)

/*********************************************************************************//*****
 *  TR181 CMS related functions end
 **************************************************************************************/

static int _wlcsm_exchange_mapping_value(char b_loading,char *src, char *dst, WLCSM_DM_WLMNGR_MAPPING* mapping,int entry_size)
{
    int i=0,ret=0;
    WLCSM_NAME_OFFSET  *src_set,*dst_set;
    char **srcpptr,**dstpptr;
    char *tempbuf=NULL;
    int buflen=0;
    for ( i=0; i<entry_size; i++ ) {

        if(b_loading==B_DM_LOADING) {
            src_set = &(mapping[i].dm_set);
            dst_set = &(mapping[i].wlmngr_set);
        } else {
            src_set=&(mapping[i].wlmngr_set);
            dst_set=&(mapping[i].dm_set);
            WLCSM_TRACE(WLCSM_TRACE_LOG," name:%s:%s,type:%d \r\n",src_set->name,dst_set->name,mapping[i].type );
        }

        /*when it reach's end,break out because the real size maybe smaller than the entry_size */
        if(!src_set->name) break;
        srcpptr=(char **)(src+src_set->offset);
        dstpptr=(char **)(dst+dst_set->offset);

        switch ( mapping[i].type ) {

        case WLCSM_DT_UINT:
            NUM_SYNC(src,src_set->offset,dst,dst_set->offset,UINT32);
            break;
        case WLCSM_DT_SINT32:
            NUM_SYNC(src,src_set->offset,dst,dst_set->offset,SINT32);
            break;
        case WLCSM_DT_UINT64:
            NUM_SYNC(src,src_set->offset,dst,dst_set->offset,UINT64);
            break;
        case WLCSM_DT_SINT64:
            NUM_SYNC(src,src_set->offset,dst,dst_set->offset,SINT64);
            break;
        case WLCSM_DT_BOOL:
            NUM_SYNC(src,src_set->offset,dst,dst_set->offset,UBOOL8);
            break;
        case WLCSM_DT_BOOLREV:
            BOOL_REV(src,src_set->offset,dst,dst_set->offset,UBOOL8);
            break;
        case WLCSM_DT_STRING:
        case WLCSM_DT_BASE64:
        case WLCSM_DT_DATETIME:
            if(b_loading==B_DM_LOADING) {
                if (*srcpptr && WLCSM_STRCMP(*srcpptr,"(null)")) {
                    WLCSM_STRCPY(*dstpptr,*srcpptr);
                } else {
                    if(*dstpptr) free(*dstpptr);
                    *dstpptr=NULL;
                }
            } else  {
                if (*srcpptr) {
                    MDM_STRCPY(*dstpptr, *srcpptr);
                } else if(*dstpptr)
                    CMSMEM_FREE_BUF_AND_NULL_PTR(*dstpptr);
            }
            break;
        case WLCSM_DT_HEXBINARY:
            if(b_loading==B_DM_LOADING) {
                if (*srcpptr && WLCSM_STRCMP(*srcpptr,"(null)")) {
                    if(!wlcsm_hexStringToBinaryBuf(*srcpptr,&tempbuf,&buflen)) {
                        WLCSM_STRCPY(*dstpptr,tempbuf);
                        free(tempbuf);
                    }
                } else {

                    if(*dstpptr) free(*dstpptr);
                    *dstpptr=NULL;
                }
            } else  {
                if (*srcpptr) {
                    if(!wlcsm_binaryBufToHexString(*srcpptr,strlen(*srcpptr),&tempbuf)) {
                        MDM_STRCPY(*dstpptr, tempbuf);
                        free(tempbuf);
                    }
                } else if(*dstpptr)
                    CMSMEM_FREE_BUF_AND_NULL_PTR(*dstpptr);
            }
            break;
        case WLCSM_DT_STR2INT:
            if(b_loading==B_DM_LOADING) {
                SINT32 value=0;
                SINT32  *dstptr=(SINT32 *)(dst+dst_set->offset);
                if (*srcpptr) {
                    value=wlcsm_dm_get_mapper_int(mapping[i].mapper,*srcpptr,0,&ret);
                    WLCSM_TRACE(WLCSM_TRACE_LOG," get mapper value is:%d \r\n",value );
                }
                *dstptr=value;
            } else {
                char *tmp_str_ptr;
                tmp_str_ptr= wlcsm_dm_get_mapper_str(mapping[i].mapper, NUMVAR_VALUE(src, src_set->offset,SINT32),0,&ret);
                if (tmp_str_ptr && !ret) {
                    MDM_STRCPY(*dstpptr, tmp_str_ptr);
                }
            }
            break;
        case WLCSM_DT_INT2STR:
            if(b_loading==B_DM_LOADING) {
                char *tmp_str_ptr;
                tmp_str_ptr= wlcsm_dm_get_mapper_str(mapping[i].mapper, NUMVAR_VALUE(src, src_set->offset,SINT32),0,&ret);
                if (tmp_str_ptr && !ret) {
                    WLCSM_STRCPY(*dstpptr,tmp_str_ptr);
                }
            } else {
                SINT32 value=0;
                SINT32  *dstptr=(SINT32 *)(dst+dst_set->offset);
                if (*srcpptr) {
                    value=wlcsm_dm_get_mapper_int(mapping[i].mapper,*srcpptr,0,&ret);
                    WLCSM_TRACE(WLCSM_TRACE_LOG," get mapper value is:%d \r\n",value );
                }
                *dstptr=value;
            }
        default:
            WLCSM_TRACE(WLCSM_TRACE_ERR," DO NOT UNDERSTAND THE TYPE:%d and name is:%s:%s    ",src_set->name,dst_set->name ,mapping[i].type );
            break;
        }
    }
    return 0;
}
#define  __wlcsm_exchange_mapping_value(b_loading,src,dst,mapping,entry_size)  _wlcsm_exchange_mapping_value((b_loading),(char *)(src), (char *)(dst), (WLCSM_DM_WLMNGR_MAPPING*)(mapping),(entry_size))

static int _wlcsm_dm_save_wifi()
{
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    CmsRet ret;
    _Dev2WifiObject *wifiObj=NULL;
    WLCSM_WLAN_WIFI_STRUCT *wifi=&g_wifi_obj;


    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI, &iidStack, 0, (void *) &wifiObj)) != CMSRET_SUCCESS) {

        if (wifiObj != NULL)
            cmsObj_free((void **) &wifiObj);
        return ret;
    }


    ret=__wlcsm_exchange_mapping_value(B_DM_POPULATING,wifi,wifiObj,g_dm_tr181_mapping_DEV2_WIFI,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI));

    if ((ret = cmsObj_set(wifiObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to wifiobj \r\n" );


    if (wifiObj != NULL) {
        cmsObj_free((void **) &wifiObj);
    }

    WLCSM_TRACE(WLCSM_TRACE_LOG," ====================POPUTLATING WIFI done=================== \r\n" );
    return 0;
}


static int _wlcsm_dm_tr181_save_radio(int idx)
{
    char varValue[8];
    _Dev2WifiRadioObject *wlRadioCfgObj=NULL;
    int ret=0;
    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_RADIO_STRUCT *radio = &(adapter->radio);
    WLCSM_TRACE(WLCSM_TRACE_LOG," ============== starting to save radio================= \r\n" );
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    iidStack.instance[0] = idx;
    iidStack.currentDepth=1;
    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_RADIO, &iidStack, 0, (void **)&wlRadioCfgObj)) != CMSRET_SUCCESS)
        goto release;
    ret=__wlcsm_exchange_mapping_value(B_DM_POPULATING,radio,wlRadioCfgObj,g_dm_tr181_mapping_DEV2_WIFI_RADIO,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_RADIO));
    snprintf(varValue,sizeof(varValue),"%s ",wlRadioCfgObj->X_BROADCOM_COM_WlCountry);
    CMSMEM_REPLACE_STRING_FLAGS(wlRadioCfgObj->regulatoryDomain, varValue, mdmLibCtx.allocFlags);

    if ((ret = cmsObj_set(wlRadioCfgObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to radio \r\n" );
release:
    if (wlRadioCfgObj != NULL)
        cmsObj_free((void **) &wlRadioCfgObj);
    WLCSM_TRACE(WLCSM_TRACE_LOG," ====================saveinging radio done=================== \r\n" );

    return ret;
}

static int _wlcsm_dm_tr181_save_bssid(int idx,int sub_idx)
{
    int ret=0;
    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_BSSID_STRUCT *bssids;

    WLCSM_TRACE(WLCSM_TRACE_LOG," ====== loading adapter:%d === bssid:%d ==== \r\n",idx,sub_idx);

    bssids = adapter->bssids;

    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    _Dev2WifiSsidObject *ssidObj=NULL;

    iidStack.instance[0] = sub_idx+ _wlcsm_dm_prev_obj_num(idx);
    iidStack.currentDepth=1;

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_SSID, &iidStack, 0, (void **) &ssidObj)) != CMSRET_SUCCESS) {
        if (ssidObj != NULL) {
            cmsObj_free((void **) &ssidObj);
        }
        return ret;
    }
    WLCSM_TRACE(WLCSM_TRACE_LOG," DM's ssid is:%s, ifname is:%s \r\n",ssidObj->SSID,ssidObj->name);
    ret= __wlcsm_exchange_mapping_value(B_DM_POPULATING,(char *)&bssids[sub_idx-1],(char *)ssidObj,g_dm_tr181_mapping_DEV2_WIFI_SSID,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_SSID));

    if ((ret = cmsObj_set(ssidObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to bssid \r\n" );

    if (ssidObj != NULL) {
        cmsObj_free((void **) &ssidObj);
    }

    return ret;
}
static int _wlcsm_dm_tr181_save_access_point(int idx,int sub_idx)
{
    int ret=0;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;

    _Dev2WifiAccessPointObject *wlAccessPointCfgObj=NULL;
    _Dev2WifiAccessPointSecurityObject *accessPointSecurityObj=NULL;
    _Dev2WifiAccessPointWpsObject *wpsObj =NULL;
    WLCSM_WLAN_AP_STRUCT *accesspoint;
    WLCSM_WLAN_AP_SECURITY_STRUCT *security;
    WLCSM_WLAN_AP_WPS_STRUCT   *wps;


    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_ACCESSPOINT_STRUCT *ssids;


    ssids=adapter->ssids;
    accesspoint = &(ssids[sub_idx-1].accesspoint);
    security = &(ssids[sub_idx-1].security);
    wps = &(ssids[sub_idx-1].wps);


    iidStack.instance[0] = sub_idx+_wlcsm_dm_prev_obj_num(idx);
    iidStack.currentDepth=1;

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT, &iidStack, 0, (void **)&wlAccessPointCfgObj)) != CMSRET_SUCCESS) {

        if (wlAccessPointCfgObj != NULL) {
            cmsObj_free((void **) &wlAccessPointCfgObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_POPULATING,accesspoint,wlAccessPointCfgObj,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT));

    if ((ret = cmsObj_set(wlAccessPointCfgObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to bssid \r\n" );

    if (wlAccessPointCfgObj != NULL) {
        cmsObj_free((void **) &wlAccessPointCfgObj);
    }

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT_SECURITY, &iidStack, 0, (void **) &accessPointSecurityObj)) != CMSRET_SUCCESS) {


        if (accessPointSecurityObj!=NULL) {
            cmsObj_free((void **) &accessPointSecurityObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_POPULATING,security,accessPointSecurityObj,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_SECURITY,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_SECURITY));


    if ((ret = cmsObj_set(accessPointSecurityObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to security \r\n" );

    if (accessPointSecurityObj!=NULL) {
        cmsObj_free((void **) &accessPointSecurityObj);
    }


    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT_WPS, &iidStack, 0, (void **) &wpsObj)) != CMSRET_SUCCESS) {
        if (wpsObj!=NULL) {
            cmsObj_free((void **) &wpsObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_POPULATING,wps,wpsObj,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_WPS,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_WPS));

    if ((ret = cmsObj_set(wpsObj,&iidStack)) != CMSRET_SUCCESS)
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR to write to security \r\n" );

    if (wpsObj!=NULL) {
        cmsObj_free((void **) &wpsObj);
    }
    return ret;
}


static int __wlcsm_dm_tr181_save_config(int idx)
{

    int ret=0,i=0;
    int num_of_bssids= wlcsm_dm_get_bssid_num(idx);

    ret=_wlcsm_dm_tr181_save_radio(idx);
    WLCSM_TRACE(WLCSM_TRACE_LOG," ==>>>>>bssid_num:%d \r\n",gp_adapter_objs[idx-1].radio.wlNumBss );
    if(!ret) {

        for ( i=1; i<=num_of_bssids; i++ ) {
            ret=_wlcsm_dm_tr181_save_bssid(idx,i);
            if(ret || (ret=_wlcsm_dm_tr181_save_access_point(idx,i)))
                return ret;
        }
    }
    return ret;

}

static int _wlcsm_dm_tr181_save_config(int idx,int from)
{
    int start_idx,end_idx,ret=0;

    if(idx) {
        start_idx=idx;
        end_idx=idx;
    } else {
        start_idx=1;
        end_idx= WL_WIFI_RADIO_NUMBER;
    }


    for(idx=start_idx; idx<=end_idx; idx++) {

        ret=__wlcsm_dm_tr181_save_config(idx);
        if(ret) break; //if not success, break;
    }

    ret=cmsMgm_saveConfigToFlash();

    return ret;

}



static int  _wlcsm_dm_load_wifi()
{
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    CmsRet ret;
    _Dev2WifiObject *wifiObj=NULL;
    WLCSM_WLAN_WIFI_STRUCT *wifi=&g_wifi_obj;

    ret = cmsLck_acquireAllZoneLocksWithBackoff( 0, CMSLCK_MAX_HOLDTIME );
    if ( ret != CMSRET_SUCCESS ) {
        printf("Could not get lock!\n");
        return ret;
    }

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI, &iidStack, 0, (void *) &wifiObj)) != CMSRET_SUCCESS) {

        cmsLck_releaseAllZoneLocks();
        return ret;
    }

    cmsLck_releaseAllZoneLocks();

    ret=__wlcsm_exchange_mapping_value(B_DM_LOADING,wifiObj,wifi,g_dm_tr181_mapping_DEV2_WIFI,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI));

    if (wifiObj != NULL) {
        cmsObj_free((void **) &wifiObj);
    }
    WLCSM_TRACE(WLCSM_TRACE_LOG," ====================Loading WIFI done=================== \r\n" );
    return 0;
}

static int _wlcsm_dm_tr181_load_radio(int idx)
{
    _Dev2WifiRadioObject *wlRadioCfgObj=NULL;
    int ret=0;
    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_RADIO_STRUCT *radio = &(adapter->radio);
    WLCSM_TRACE(WLCSM_TRACE_LOG," ============== starting to load radio================= \r\n" );


    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;

    iidStack.instance[0] = idx;
    iidStack.currentDepth=1;
    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_RADIO, &iidStack, 0, (void **)&wlRadioCfgObj)) != CMSRET_SUCCESS) {

        return ret;
    }

    ret=__wlcsm_exchange_mapping_value(B_DM_LOADING,wlRadioCfgObj,radio,g_dm_tr181_mapping_DEV2_WIFI_RADIO,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_RADIO));
    if (wlRadioCfgObj != NULL) {
        cmsObj_free((void **) &wlRadioCfgObj);
    }


    WLCSM_TRACE(WLCSM_TRACE_LOG," after mapping: radiowlnumbss:%d,eanbled:%d  \r\n",radio->wlNumBss,radio->wlEnbl );
    WLCSM_TRACE(WLCSM_TRACE_LOG," after mapping: wlBand:%d,phytype:%s  \r\n",radio->wlBand,radio->wlPhyType );
    WLCSM_TRACE(WLCSM_TRACE_LOG," ====================Loading radio done=================== \r\n" );

    return ret;
}

static int _wlcsm_dm_tr181_load_bssid(int idx,int sub_idx)
{
    int len=0,ret=0;
    int num_of_bssids=wlcsm_dm_get_bssid_num(idx);
    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_BSSID_STRUCT *bssids;
    len= sizeof(WLCSM_WLAN_BSSID_STRUCT) * num_of_bssids;

    WLCSM_TRACE(WLCSM_TRACE_LOG," bssids: num of bssids:%d \r\n",num_of_bssids );
    WLCSM_TRACE(WLCSM_TRACE_LOG," ====== loading adapter:%d === bssid:%d ==== \r\n",idx,sub_idx);

    if(!adapter->bssids) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," try to allocate for idx:%d,sub_idx:%d \r\n",idx,sub_idx );
        if(!(adapter->bssids=malloc(len))) {
            WLCSM_TRACE(WLCSM_TRACE_ERR," could not allocate memory for bssids \r\n");
        } else
            memset(adapter->bssids,0,len);
    } else
        WLCSM_TRACE(WLCSM_TRACE_LOG," bssids is not null, allocated already???:sub_idx:%d \r\n",sub_idx );

    bssids = adapter->bssids;


    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    _Dev2WifiSsidObject *ssidObj=NULL;

    iidStack.instance[0] = sub_idx+_wlcsm_dm_prev_obj_num(idx);
    iidStack.currentDepth=1;

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_SSID, &iidStack, 0, (void **) &ssidObj)) != CMSRET_SUCCESS) {
        if (ssidObj != NULL) {
            cmsObj_free((void **) &ssidObj);
        }
        return ret;
    }
    WLCSM_TRACE(WLCSM_TRACE_LOG," DM's ssid is:%s, ifname is:%s \r\n",ssidObj->SSID,ssidObj->name);
    ret= __wlcsm_exchange_mapping_value(B_DM_LOADING,(char *)ssidObj,(char *)&bssids[sub_idx-1],g_dm_tr181_mapping_DEV2_WIFI_SSID,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_SSID));
    WLCSM_TRACE(WLCSM_TRACE_LOG," sub_idx:%d,WLMNG's ssid is:%s, ifname is :%s \r\n",sub_idx,bssids[sub_idx-1].wlSsid,bssids[sub_idx-1].wlifname );
    WLCSM_TRACE(WLCSM_TRACE_LOG," ifname:%s \r\n",WL_BSSID_IFNAME(idx-1,sub_idx-1));
    WLCSM_TRACE(WLCSM_TRACE_LOG," =================================================++++++++++++++++++++++++++++++++++++++++ \r\n" );

    if (ssidObj != NULL) {
        cmsObj_free((void **) &ssidObj);
    }

    return ret;
}

static int _wlcsm_dm_tr181_load_access_point(int idx,int sub_idx)
{
    int ret=0;
    int num_of_bssids= wlcsm_dm_get_bssid_num(idx);
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;

    WLCSM_TRACE(WLCSM_TRACE_LOG," ACCESSPOINT: num of bssids:%d \r\n",num_of_bssids );
    _Dev2WifiAccessPointObject *wlAccessPointCfgObj=NULL;
    _Dev2WifiAccessPointSecurityObject *accessPointSecurityObj=NULL;
    _Dev2WifiAccessPointWpsObject *wpsObj =NULL;
    WLCSM_WLAN_AP_STRUCT *accesspoint;
    WLCSM_WLAN_AP_SECURITY_STRUCT *security;
    WLCSM_WLAN_AP_WPS_STRUCT   *wps;


    WLCSM_WLAN_ADAPTER_STRUCT *adapter = (WLCSM_WLAN_ADAPTER_STRUCT *)(&gp_adapter_objs[idx-1]);
    WLCSM_WLAN_ACCESSPOINT_STRUCT *ssids;

    if(!adapter->ssids) {
        adapter->ssids = malloc( sizeof(WLCSM_WLAN_ACCESSPOINT_STRUCT) * num_of_bssids);
        if(!adapter->ssids) {
            WLCSM_TRACE(WLCSM_TRACE_ERR," allocate mem for AP failed \r\n");
            return -1;
        } else {
            memset(adapter->ssids,0,sizeof(WLCSM_WLAN_ACCESSPOINT_STRUCT)*num_of_bssids);
        }
    }

    ssids=adapter->ssids;
    accesspoint = &(ssids[sub_idx-1].accesspoint);
    security = &(ssids[sub_idx-1].security);
    wps = &(ssids[sub_idx-1].wps);



    iidStack.instance[0] = sub_idx+_wlcsm_dm_prev_obj_num(idx);
    iidStack.currentDepth=1;

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT, &iidStack, 0, (void **)&wlAccessPointCfgObj)) != CMSRET_SUCCESS) {

        if (wlAccessPointCfgObj != NULL) {
            cmsObj_free((void **) &wlAccessPointCfgObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_LOADING,(char *)wlAccessPointCfgObj,accesspoint,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT));

    if (wlAccessPointCfgObj != NULL) {
        cmsObj_free((void **) &wlAccessPointCfgObj);
    }

    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT_SECURITY, &iidStack, 0, (void **) &accessPointSecurityObj)) != CMSRET_SUCCESS) {


        if (accessPointSecurityObj!=NULL) {
            cmsObj_free((void **) &accessPointSecurityObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_LOADING,(char *)accessPointSecurityObj,security,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_SECURITY,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_SECURITY));


    if (accessPointSecurityObj!=NULL) {
        cmsObj_free((void **) &accessPointSecurityObj);
    }

    iidStack.instance[0] = sub_idx+_wlcsm_dm_prev_obj_num(idx);

    iidStack.currentDepth=1;
    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT, &iidStack, 0, (void **)&wlAccessPointCfgObj)) != CMSRET_SUCCESS) {

        if (wlAccessPointCfgObj != NULL) {
            cmsObj_free((void **) &wlAccessPointCfgObj);
        }
        return ret;
    }
    if (wlAccessPointCfgObj != NULL) {
        cmsObj_free((void **) &wlAccessPointCfgObj);
    }


    if ((ret = cmsObj_get(MDMOID_DEV2_WIFI_ACCESS_POINT_WPS, &iidStack, 0, (void **) &wpsObj)) != CMSRET_SUCCESS) {
        if (wpsObj!=NULL) {
            cmsObj_free((void **) &wpsObj);
        }
        return ret;
    }

    ret= __wlcsm_exchange_mapping_value(B_DM_LOADING,(char *)wpsObj,wps,g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_WPS,MAPPING_ENTRY_SIZE(g_dm_tr181_mapping_DEV2_WIFI_ACCESS_POINT_WPS));

    if (wpsObj!=NULL) {
        cmsObj_free((void **) &wpsObj);
    }
    return ret;
}

/*************************************************************//**
 * @brief  load all configuration to manager's structure
 *
 * @return  int
 ***************************************************************/
static int _wlcsm_dm_tr181_load_config(int idx,int from)
{
    int i, start_idx,end_idx,ret=0;

    if(idx) {
        start_idx=idx;
        end_idx=idx;
    } else {
        start_idx=1;
        end_idx= WL_WIFI_RADIO_NUMBER;
    }
    for(idx=start_idx; idx<=end_idx; idx++) {
        ret=_wlcsm_dm_tr181_load_radio(idx);
        WLCSM_TRACE(WLCSM_TRACE_LOG," ==>>>>>bssid_num:%d \r\n",WL_RADIO_WLNUMBSS(idx-1));
        if(!ret) {
            for ( i=1; i<=WL_RADIO_WLNUMBSS(idx-1); i++ ) {
                ret=_wlcsm_dm_tr181_load_bssid(idx,i);
                if(ret || (ret=_wlcsm_dm_tr181_load_access_point(idx,i)))
                    return ret;
            }
        } else
            break;
    }
    return ret;
}


extern char *wlcsm_prefix_match(char *name);
static int _wlcsm_dm_tobe_saved_nvram(char *name)
{

    char *prefix=NULL;
    char tempc;
    int index=0;
    /*kernel calibration data and runtime var, no need to save it */
    if(sscanf(name,"%d:%c",&index,&tempc)==2
            ||(sscanf(name,"wl_%c",&tempc)==1))
        return 0;
    prefix=wlcsm_prefix_match(name);
    if(prefix) {
        int entries_num=sizeof(g_wlcsm_nvram_mngr_mapping)/sizeof(WLCSM_NVRAM_MNGR_MAPPING);
        char *var_name=name+strlen(prefix);
        char *nvram_var;
        for ( index=0; index<entries_num; index++) {
            nvram_var= g_wlcsm_nvram_mngr_mapping[index].nvram_var;
            if(!strncmp(var_name,nvram_var,strlen(nvram_var))) return 0;
        }
    }
    return 1;
}

static int  _wlcsm_dm_get_nvram(void)
{
    char *name;
    char *pair,*buf = malloc(MAX_NVRAM_SPACE);
    int pair_len_max=WL_LG_SIZE_MAX+WL_SIZE_132_MAX;

    if(!buf) {
        fprintf(stderr,"could not allocate memory for buf\n");
        return -1;
    } else if(!(pair=malloc(WL_LG_SIZE_MAX+WL_SIZE_132_MAX))) {
        fprintf(stderr,"could not allocate memory for pair\n");
        free(buf);
        return -1;
    }

    if(g_wifi_obj.nvram) {
        free(g_wifi_obj.nvram);
        g_wifi_obj.nvram=NULL;
    }

    if(!(g_wifi_obj.nvram=malloc(MAX_NVRAM_SPACE)) ) {
        fprintf(stderr,"could not allocate memory for buf\n");
        free(buf);
        free(pair);
        return -1;
    }

    wlcsm_nvram_getall(buf, MAX_NVRAM_SPACE*sizeof(char));
    memset(g_wifi_obj.nvram,0,MAX_NVRAM_SPACE);
    strcat(g_wifi_obj.nvram,"FFFF");
    for (name = buf; *name && (strlen(g_wifi_obj.nvram)<MAX_NVRAM_SPACE); name += strlen(name) + 1) {
        if(wlcsm_nvram_from_kernel(name,0)) {
            continue;
        }
        if(!strncmp(name,"wps_force_restart", strlen("wps_force_restart"))
                || !strncmp(name, "pair_len_byte=", strlen("pair_len_byte="))
                || !strncmp(name, "acs_ifnames", strlen("acs_ifnames"))
                || !strncmp(name, "wl_unit", strlen("wl_unit"))
                || strstr(name, "_dpd") || strstr(name, "pcie_"))
            continue;
        if(_wlcsm_dm_tobe_saved_nvram(name)) {

            if(! wlcsm_hspot_var_isdefault(name)) {
                snprintf(pair, pair_len_max, "%03X%s", (int)strlen(name), name);
                strcat(g_wifi_obj.nvram, pair);
            }
        }
    }

    wlcsm_nvram_from_kernel(NULL,1);
    free(buf);
    free(pair);
    return 0;
}

int _wlcsm_dm_tr181_save_nvram(void)
{
    if(!_wlcsm_dm_get_nvram() && !_wlcsm_dm_save_wifi()) {
        cmsMgm_saveConfigToFlash ();
        return 0;
    } else 
        return -1; 
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
static int _wlcsm_dm_tr181_init(void)
{
    int ret=0;

    SINT32 shmId=UNINITIALIZED_SHM_ID;
    CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;
    /*
     * Do CMS initialization after wlevt is spawned so that wlevt does
     * not inherit any file descriptors opened by CMS init code.
     */
    cmsLog_initWithName(EID_WLMNGR, "wlmngr");
    cmsLog_setLevel(logLevel);

    if ((ret = cmsMsg_initWithFlags(EID_WLMNGR, 0, &g_MsgHandle)) != CMSRET_SUCCESS) {
        cmsLog_error("could not initialize msg, ret=%d", ret);
        cmsLog_cleanup();
        return -1;
    }

    /* The wrapper has logic to request the shmId from smd if wlmngr was
     * started on the command line.  It also contains logic to skip MDM
     * initialization if we are in Pure181 mode.
     */
    if ((ret = cmsMdm_init_wrapper(&shmId, g_MsgHandle)) != CMSRET_SUCCESS) {
        cmsMsg_cleanup(&g_MsgHandle);
        cmsLog_cleanup();
        return -1;
    }
    /* this function to init the global variable  g_wifi_obj */
    ret=_wlcsm_dm_load_wifi();
    return ret;
}


static int _wlcsm_dm_tr181_deinit(void)
{
    cmsMdm_cleanup_wrapper();
    cmsMsg_cleanup(&g_MsgHandle);
    cmsLog_cleanup();
    return 0;
}

CmsRet _wlcsm_dm_tr181_oid_mngr_name(char *buf)
{
    unsigned int *nums=(unsigned int *)buf;
    unsigned int oid=nums[0];
    unsigned int offset=nums[1];
    int size=WLCSM_DM_WLMNGR_OID_MAPPING_ENTRY_SIZE(g_wlcsm_tr181_oid_mapping);
    int i=0;
    WLCSM_DM_WLMNGR_OID_MAPPING *oidmapping=NULL;
    WLCSM_DM_WLMNGR_MAPPING *mapping=NULL;
    WLCSM_NAME_OFFSET **mngr_set_pointer=NULL;
    for ( i=0; i<size; i++ ) {
        if(g_wlcsm_tr181_oid_mapping[i].oid==oid) {
            oidmapping=&(g_wlcsm_tr181_oid_mapping[i]);
            mapping=(g_wlcsm_tr181_oid_mapping[i].mapper);
            size=g_wlcsm_tr181_oid_mapping[i].size;
            for ( i=0; i<size; i++ ) {
                WLCSM_TRACE(WLCSM_TRACE_DBG,"i:%d  offset:%d,name:%s \r\n",i,mapping[i].dm_set.offset, mapping[i].dm_set.name );
                if(!mapping[i].dm_set.name) return -1;
                if(mapping[i].dm_set.offset==offset) {
                    if(strlen(mapping[i].wlmngr_set.name)< WLCSM_MNGR_VARNAME_MAX) {
                        sprintf(buf,"%s",mapping[i].wlmngr_set.name);
                        mngr_set_pointer=(WLCSM_NAME_OFFSET **)buf;
                        *mngr_set_pointer= &(mapping[i].wlmngr_set);
                        return oidmapping->mngr_oid;
                    }
                }
            }
        }
    }
    return -1;
}


CmsRet _wlcsm_dm_tr181_getbridge_info(char *buf)
{
    Dev2BridgeObject *brObj=NULL;
    Dev2BridgePortObject *brPortObj=NULL;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    InstanceIdStack sub_iidStack = EMPTY_INSTANCE_ID_STACK;
    CmsRet ret;
    int first=1;
    buf[0]='\0';

    ret = cmsLck_acquireAllZoneLocksWithBackoff( 0, CMSLCK_MAX_HOLDTIME );
    if ( ret != CMSRET_SUCCESS ) {
        printf("Could not get lock!\n");
        return ret;
    }

    while ((ret = cmsObj_getNextFlags(MDMOID_DEV2_BRIDGE, &iidStack,
                                      OGF_NO_VALUE_UPDATE,
                                      (void **) &brObj)) == CMSRET_SUCCESS) {
        UINT32 bridgeNumber;
        bridgeNumber = atoi(&(brObj->X_BROADCOM_COM_IfName[2]));
        cmsAst_assert(bridgeNumber < MAX_LAYER2_BRIDGES);
        INIT_INSTANCE_ID_STACK(&sub_iidStack);
        if(!first) strcat(buf," ");
        strcat(buf,brObj->X_BROADCOM_COM_IfName);
        first=0;
        while (cmsObj_getNextInSubTree(MDMOID_DEV2_BRIDGE_PORT, &iidStack,
                                       &sub_iidStack,(void **) &brPortObj) == CMSRET_SUCCESS) {
            if (!brPortObj->managementPort) {
                strcat(buf,":");
                strcat(buf,brPortObj->name);
            }
            cmsObj_free((void **)&brPortObj);
        }
        cmsObj_free((void **) &brObj);
    }

    cmsLck_releaseAllZoneLocks();
    return ret;
}


int _wlcsm_dm_tr181_query_info(WLCSM_DM_QUERY_CMD cmd,void *buf)
{
    CmsRet ret;
    switch ( cmd ) {
    case WLCSM_DM_QUERY_BRIDGE_INFO:
        ret=_wlcsm_dm_tr181_getbridge_info((char *)buf);
        break;
    case WLCSM_DM_QUERY_MNGR_ENTRY:
        WLCSM_TRACE(WLCSM_TRACE_DBG," tr181 get mngr entry \r\n" );
        ret=_wlcsm_dm_tr181_oid_mngr_name((char *)buf);
        break;
    case WLCSM_DM_QUERY_SETDBGLEVEL: {
        int dbglevel=0;
        sscanf(buf,"%d",&dbglevel);
        cmsLog_setLevel(dbglevel);
        ret=CMSRET_SUCCESS;
    }
    break;
    case WLCSM_DM_QUERY_LOCK: {
        if(buf) {
            ret = cmsLck_acquireAllZoneLocksWithBackoff( 0, CMSLCK_MAX_HOLDTIME ); 
            if ( ret != CMSRET_SUCCESS ) {
                printf("Could not get lock!\n");
                return -1;
            } else
                return 0;
        } else {
            cmsLck_releaseAllZoneLocks();
            return 0;
        }
    }
    break;
    default:
        cmsLog_error("unknow cmd:%d\n",cmd);
        ret=-1;
        break;
    }
    return ret;

}

DECLARE_WLCSM_DM_ITEM(tr181);
