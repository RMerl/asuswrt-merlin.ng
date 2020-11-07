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
/**
 *	@file	 wlcsm_dm_tr98.c
 *	@brief	 wlcsm data model for tr98 APIs
 *
 * 	this file will handle wlmngr data structure and mapping to tr98 data mapping
 * 	this file will be mostly like wlmdm in previous implementation.
 *
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
#include "cms_obj.h"
#include "cms_mdm.h"
#include "cms_mgm.h"
#include "cms_util.h"
#include "cms_data_model_selector.h"
#include "cms_msg.h"
#include "mdm.h"
#include "rut_pmap.h"

#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "board.h"
#include "wlioctl.h"


#include <bcmnvram.h>

#include "wlmngr.h"

#include "wllist.h"

#include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <asm/types.h>
#include <sys/types.h>
#include <errno.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_dm.h"
#include "wlcsm_lib_api.h"
#include "wlcsm_dm_tr98.h"
#include <errno.h>
#include "wlcsm_linux.h"
#include "cms.h"
#include "wlmdm.h"

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
#include "wldefs.h"
#include "wldsltr.h"
#define LOCK_WAIT     (60000)
static void *g_MsgHandle = NULL;
int wl_cnt = 0;

/*********************************************************************************//*****
 *  TR98 CMS related functions starts
 **************************************************************************************/

int getShmIdByMsg(void *g_MsgHandle)
{
    int shmId;
    UINT32 timeoutMs = 5000;
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



/*********************************************************************************//*****
 *  TR98 CMS related functions end
 **************************************************************************************/
int _wlcsm_dm_tr98_obj_change_handler(unsigned int radio_idx, unsigned int sub_idx , void *buf)
{
#if 0
    WLCSM_DM_OBJ_VALUE_SET *data = (WLCSM_DM_OBJ_VALUE_SET *)buf;
    if(!strncmp(data->name, "wlAuthMode", 10)) {
        WLCSM_TRACE(WLCSM_TRACE_DBG, " wlmngr:oid:%u,offset:%u and name:%s change to value:%s \r\n", data->oid, data->offset, data->name, data->value );

        strncpy(m_instance_wl[radio_idx].m_wlMssidVar[sub_idx].wlAuthMode, data->value, sizeof(m_instance_wl[radio_idx].m_wlMssidVar[sub_idx].wlAuthMode));
    } else
        strncpy(m_instance_wl[radio_idx].m_wlMssidVar[sub_idx].wlAuthMode, WL_AUTH_OPEN, sizeof(m_instance_wl[radio_idx].m_wlMssidVar[sub_idx].wlAuthMode));
#endif
    return 0;
}

int _wlcsm_dm_tr98_sta_change_handler(unsigned int radio_idx, unsigned int sub_idx , void *buf)
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
        pStationEntry->radioIndex=radio_idx;
        pStationEntry->ssidIndex=sub_idx;
        strncpy(pStationEntry->ssid,WL_BSSID_WLSSID(radio_idx,sub_idx),WL_SSID_SIZE_MAX);
        strncpy(pStationEntry->ifcName,WL_BSSID_IFNAME(radio_idx,sub_idx),WL_SM_SIZE_MAX);
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

struct dm_matching_item {
    char *name;
    int  offset;
    int  type; // 0 is int and 1 is string
};

#define _CHANGE_WIRELESS_VAR_ITEM_INT_(item) { #item,offsetof(WIRELESS_VAR,item),0}
#define _CHANGE_WIRELESS_VAR_ITEM_STR_(item) { #item,offsetof(WIRELESS_VAR,item),1}

struct dm_matching_item m_wlVar_items[] = {
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlChannel),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlNBwCap),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlRegMode),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlNCtrlsb),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlWme),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlWmeNoAck),
    _CHANGE_WIRELESS_VAR_ITEM_INT_(wlWmeApsd),
};
#define _MVAR_ITEMS_LEN_ (sizeof(m_wlVar_items) / sizeof(struct dm_matching_item))
#define _INSTANCE_MVAR_ (m_instance_wl[idx].m_wlVar)
#define _INSTANCE_MVAR_STR_PTR_(offset) ((char *)(((void *)&(m_instance_wl[idx].m_wlVar))+offset))
#define _INSTANCE_MVAR_INT_PTR_(offset) ((int *)(((void *)&(m_instance_wl[idx].m_wlVar))+offset))
#define _INSTANCE_MVAR_INT_(offset) (*_INSTANCE_MVAR_INT_PTR_(offset))
#define _MVAR_NAME_(i) (m_wlVar_items[i].name)
#define _MVAR_OFF_(i) (m_wlVar_items[i].offset)
#define _MVAR_TYPE_(i) (m_wlVar_items[i].type)


#define _CHANGE_WIRELESS_BSS_ITEM_INT_(item) { #item,offsetof(WIRELESS_MSSID_VAR,item),0}
#define _CHANGE_WIRELESS_BSS_ITEM_STR_(item) { #item,offsetof(WIRELESS_MSSID_VAR,item),1}
struct dm_matching_item m_wlVar_bss_items[] = {
    _CHANGE_WIRELESS_BSS_ITEM_INT_(wlKeyBit),
    _CHANGE_WIRELESS_BSS_ITEM_STR_(wlAuthMode),
    _CHANGE_WIRELESS_BSS_ITEM_STR_(wlBrName),
};
#define _MVARBSS_ITEMS_LEN_ (sizeof(m_wlVar_bss_items) / sizeof(struct dm_matching_item))
#define _INSTANCE_MVARBSS_(bss) (m_instance_wl[idx].m_wlMssidVar[bss])
#define _INSTANCE_MVARBSS_STR_PTR_(bss,offset)  ((char *)((void *)&(m_instance_wl[idx].m_wlMssidVar[bss])+offset))
#define _INSTANCE_MVARBSS_INT_PTR_(bss,offset)  ((int *)((void *)&(m_instance_wl[idx].m_wlMssidVar[bss])+offset))
#define _INSTANCE_MVARBSS_INT_(bss,offset)  (*_INSTANCE_MVARBSS_INT_PTR_(bss,offset))
#define _MVARBSS_NAME_(i) (m_wlVar_bss_items[i].name)
#define _MVARBSS_OFF_(i) (m_wlVar_bss_items[i].offset)
#define _MVARBSS_TYPE_(i) (m_wlVar_bss_items[i].type)

static void __wlcsm_dm_tr98_exchange_value(int idx, int direction)
{

    char *temp_s, temp[64] = {0};
    int i = 0,j=0;
    if(direction == B_DM_LOADING) {

        for (i = 0; i <_MVAR_ITEMS_LEN_; ++i) {
            if(_MVAR_TYPE_(i)==0) {
                sprintf(temp, "%d", _INSTANCE_MVAR_INT_(_MVAR_OFF_(i)));
                wlcsm_dm_mngr_set_all_value(idx, 0, _MVAR_NAME_(i), temp);
            } else {
                wlcsm_dm_mngr_set_all_value(idx, 0, _MVAR_NAME_(i),_INSTANCE_MVAR_STR_PTR_(_MVAR_OFF_(i)));
            }
        }

        for (i = 0; i < _MVARBSS_ITEMS_LEN_ ; ++i) {
            for (j = 0; j < WL_NUM_SSID; j++) {
                WLCSM_TRACE(WLCSM_TRACE_DBG,"...%d.......\r\n",WL_NUM_SSID);
                if(_MVARBSS_TYPE_(i)==0) {
                    sprintf(temp, "%d",_INSTANCE_MVARBSS_INT_(j,_MVARBSS_OFF_(i)));
                    wlcsm_dm_mngr_set_all_value(idx, j, _MVARBSS_NAME_(i), temp);
                } else {
                    wlcsm_dm_mngr_set_all_value(idx, j, _MVARBSS_NAME_(i),_INSTANCE_MVARBSS_STR_PTR_(j, _MVARBSS_OFF_(i)));
                }
            }
        }

    } else {

        for (i = 0; i <_MVAR_ITEMS_LEN_; ++i) {
            temp_s = wlcsm_dm_mngr_get_all_value(idx, 0, _MVAR_NAME_(i), temp);
            if(temp_s) {
                if(_MVAR_TYPE_(i)==0)
                    sscanf(temp, "%d",_INSTANCE_MVAR_INT_PTR_(_MVAR_OFF_(i)));
                else
                    strncpy(_INSTANCE_MVAR_STR_PTR_(_MVAR_OFF_(i)),temp_s,strlen(temp_s)+1);
            }
        }

        for (i = 0; i <_MVARBSS_ITEMS_LEN_; ++i) {
            for (j = 0; j < WL_NUM_SSID; j++) {
                WLCSM_TRACE(WLCSM_TRACE_DBG,"...%d.......\r\n",WL_NUM_SSID);
                temp_s = wlcsm_dm_mngr_get_all_value(idx, j, _MVARBSS_NAME_(i), temp);
                if(temp_s) {
                    if(_MVARBSS_TYPE_(i)==0)
                        sscanf(temp, "%d", _INSTANCE_MVARBSS_INT_PTR_(j,  _MVARBSS_OFF_(i)));
                    else
                        strncpy(_INSTANCE_MVARBSS_STR_PTR_(j,_MVARBSS_OFF_(i)),temp_s,strlen(temp_s)+1);
                }
            }
        }

    }
}

static int __wlcsm_dm_tr98_save_config(int idx, int requestlock)
{

    int ret = 0;
    __wlcsm_dm_tr98_exchange_value(idx, B_DM_POPULATING);
    wlcsm_dm_tr98_NvramMapping(idx, MAP_FROM_NVRAM);
    wldsltr_get(idx);
    return ret =  wlWriteMdmOne( idx );

}

static int _wlcsm_dm_tr98_save_config(int idx, int from)
{
    int start_idx, end_idx, ret = 0;

    if(idx) {
        start_idx = idx - 1;
        end_idx = start_idx;
    } else {
        start_idx = 0;
        end_idx = WL_WIFI_RADIO_NUMBER - 1;
    }

    for(idx = start_idx; idx <= end_idx; idx++) {
        ret = __wlcsm_dm_tr98_save_config(idx, 0);
        if(ret) break; //if not success, break;
    }
    ret = cmsMgm_saveConfigToFlash();
    if(ret != CMSRET_SUCCESS) {
        WLCSM_TRACE(WLCSM_TRACE_ERR, " Save to MDM error\r\n");
    } else {
        WLCSM_TRACE(WLCSM_TRACE_DBG, "Save MDM to FLash ok\r\n");
    }

    return ret;

}

/*************************************************************//**
 * @brief  load all configuration to manager's structure
 *
 * When idx=0, it will load all Radio's configuration
 *
 * @return  int
 ***************************************************************/
static int _wlcsm_dm_tr98_load_config(int idx, int from)
{
    int start_idx, end_idx, ret = 0;

    if(idx) {
        start_idx = idx - 1;
        end_idx = idx - 1;
    } else {
        start_idx = 0;
        end_idx = WL_WIFI_RADIO_NUMBER - 1;
    }
    for(idx = start_idx; idx <= end_idx; idx++) {
        if(from == WLCSM_MNGR_RESTART_TR69C)  {
            wlReadMdmOne(idx);
            wlReadMdmTr69Cfg(idx);
            wldsltr_set(idx);
            wlWriteMdmOne(idx);
            wlcsm_dm_tr98_NvramMapping(idx, MAP_TO_NVRAM);
            wlcsm_dm_tr98_write_nvram();
            WLCSM_TRACE(WLCSM_TRACE_DBG,"............\r\n");
            cmsMgm_saveConfigToFlash();
        } else
            memset ( (void *)m_instance_wl + idx, 0, sizeof(WLAN_ADAPTER_STRUCT) );
        wlcsm_dm_tr98_init(idx);
        __wlcsm_dm_tr98_exchange_value(idx, B_DM_LOADING);
    }
    return ret;
}

int _wlcsm_dm_tr98_save_nvram(void)
{
    wlcsm_dm_tr98_write_nvram();
    cmsMgm_saveConfigToFlash();
    return 0;
}


static int _wlcsm_dm_tr98_init(void)
{

    int ret = 0;
    SINT32 shmId = UNINITIALIZED_SHM_ID;
    CmsLogLevel logLevel = DEFAULT_LOG_LEVEL;
    /*
     *  * Do CMS initialization after wlevt is spawned so that wlevt does
     *   * not inherit any file descriptors opened by CMS init code.
     *    */
    cmsLog_initWithName(EID_WLMNGR, "wlmngr");
    cmsLog_setLevel(logLevel);

    if ((ret = cmsMsg_initWithFlags(EID_WLMNGR, 0, &g_MsgHandle)) != CMSRET_SUCCESS) {
        cmsLog_error("could not initialize msg, ret=%d", ret);
        cmsLog_cleanup();
        return -1;
    }

    /*  The wrapper has logic to request the shmId from smd if wlmngr was
     *   * started on the command line.  It also contains logic to skip MDM
     *    * initialization if we are in Pure181 mode.
     *     */
    if ((ret = cmsMdm_init_wrapper(&shmId, g_MsgHandle)) != CMSRET_SUCCESS) {
        cmsMsg_cleanup(&g_MsgHandle);
        cmsLog_cleanup();
        return -1;
    }
    wl_cnt = wlcsm_wl_get_adapter_num();
    if(wlcsm_dm_tr98_alloc(wl_cnt) != 0 || wldsltr_alloc(wl_cnt) != 0) {
        WLCSM_TRACE(WLCSM_TRACE_DBG, "alloc wlmngr space problem\r\n");
        return -1;
    }

    WLCSM_TRACE(WLCSM_TRACE_DBG, "TR98 init successful\r\n");
    return ret;
}


static int _wlcsm_dm_tr98_deinit(void)
{
    cmsMdm_cleanup();
    cmsMsg_cleanup(&g_MsgHandle);
    cmsLog_cleanup();
    return 0;

}

int _wlcsm_dm_tr98_oid_mngr_name(char *buf)
{
#if 0
    unsigned int *nums = (unsigned int *)buf;
    unsigned int oid = nums[0];
    unsigned int offset = nums[1];
#endif
    WLCSM_TRACE(WLCSM_TRACE_DBG, "Given DM OID number and offset, to get wlmngr variable's name\r\n");
    return -1;
}



int _wlcsm_dm_tr98_getbridge_info(char *buf)
{
    InstanceIdStack BrStk=EMPTY_INSTANCE_ID_STACK;
    InstanceIdStack FtrStk=EMPTY_INSTANCE_ID_STACK;
    InstanceIdStack IfStk = EMPTY_INSTANCE_ID_STACK;

    L2BridgingEntryObject *pBr=NULL;
    L2BridgingFilterObject *pFtr=NULL;
    L2BridgingIntfObject *pIf = NULL;

    char lan_ifname[32];
    UINT32 key;
    int ret=CMSRET_SUCCESS;
    int first=1;
    buf[0]='\0';

    ret = cmsLck_acquireAllZoneLocksWithBackoff( 0, CMSLCK_MAX_HOLDTIME );
    if ( ret != CMSRET_SUCCESS ) {
        printf("Could not get lock!\n");
        return ret;
    }

    while ((cmsObj_getNext(MDMOID_L2_BRIDGING_ENTRY, &BrStk, (void **)&pBr)) == CMSRET_SUCCESS) {

        if (pBr == NULL) {
            cmsLck_releaseAllZoneLocks();
            return -1;
        }

        if(!first) strcat(buf," ");
        sprintf(lan_ifname,"br%d",pBr->bridgeKey);
        strcat(buf,lan_ifname);
        WLCSM_TRACE(WLCSM_TRACE_DBG,"buf:%s\r\n",buf);
        first=0;

        INIT_INSTANCE_ID_STACK(&FtrStk);
        while((cmsObj_getNext(MDMOID_L2_BRIDGING_FILTER, &FtrStk, (void **)&pFtr)) == CMSRET_SUCCESS) {

            if (pFtr == NULL) {
                cmsObj_free((void **)&pBr);
                cmsLck_releaseAllZoneLocks();
                return -1;
            }

            if ((pFtr->filterBridgeReference == (SINT32)pBr->bridgeKey) && (cmsUtl_strcmp(pFtr->filterInterface, MDMVS_LANINTERFACES))) {
                cmsUtl_strtoul(pFtr->filterInterface, NULL, 0, &key);
                INIT_INSTANCE_ID_STACK(&IfStk);
                if(rutPMap_getAvailableInterfaceByKey(key, &IfStk, &pIf) == CMSRET_SUCCESS) {
                    if (pIf == NULL) {
                        cmsObj_free((void **)&pFtr);
                        cmsObj_free((void **)&pBr);
                        cmsLck_releaseAllZoneLocks();
                        return -1;
                    }
                    if (!cmsUtl_strcmp(pIf->interfaceType, MDMVS_LANINTERFACE)) {
                        if (rutPMap_availableInterfaceReferenceToIfName(pIf->interfaceReference, lan_ifname) == CMSRET_SUCCESS) {
                            strcat(buf,":");
                            strcat(buf,lan_ifname);
#ifdef SUPPORT_LANVLAN
                            if(!strstr(lan_ifname,".") && strstr(lan_ifname,ETH_IFC_STR)) {
                                char lanIfName2[BUFLEN_8]={0};
                                snprintf(lanIfName2, sizeof(lanIfName2), ".%d", pFtr->X_BROADCOM_COM_VLANIDFilter);
                                strcat(buf,lanIfName2);
                            }
#endif
                        }
                    }
                    cmsObj_free((void **) &pIf);
                }
            }
            cmsObj_free((void **)&pFtr);
        }
        cmsObj_free((void **)&pBr);
    }
    cmsLck_releaseAllZoneLocks();
    return ret;
}

int _wlcsm_dm_tr98_query_info(WLCSM_DM_QUERY_CMD cmd, void *buf)
{
    int ret=0;
    switch ( cmd ) {
    case WLCSM_DM_QUERY_BRIDGE_INFO:
        ret = _wlcsm_dm_tr98_getbridge_info((char *)buf);
        break;
    case WLCSM_DM_QUERY_MNGR_ENTRY:
        WLCSM_TRACE(WLCSM_TRACE_DBG, " tr98 get mngr entry \r\n" );
        ret = _wlcsm_dm_tr98_oid_mngr_name((char *)buf);
        break;
    case WLCSM_DM_QUERY_SETDBGLEVEL: {
        int dbglevel=0;
        sscanf(buf,"%d",&dbglevel);
        cmsLog_setLevel(dbglevel);
    }
    break;
    case WLCSM_DM_QUERY_LOCK:
        if(buf) {
            ret = cmsLck_acquireAllZoneLocksWithBackoff( 0, CMSLCK_MAX_HOLDTIME );
            if ( ret != CMSRET_SUCCESS ) {
                printf("Could not get lock!\n");
                ret=-1;
            } else ret=0;
        } else
            cmsLck_releaseAllZoneLocks();
        break;
    default:
        ret = -1;
        break;
    }
    return ret;

}

DECLARE_WLCSM_DM_ITEM(tr98);
