/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/

#ifndef _RDPACTL_API_H_
#define _RDPACTL_API_H_

#include "bcmtypes.h"
#include "rdpa_types.h"
#include "rdpa_egress_tm.h"
#include "cms.h"
#include "rdpa_drv.h"
#include "tmctl_api.h"

#define RDPACTL_IFNAME_INVALID_STR "NULL"
#define RDPACTL_IS_IFNAME_VALID(_ifName)                            \
    strncmp((_ifName), RDPACTL_IFNAME_INVALID_STR, IFNAMSIZ)

int rdpaCtl_GetPortTmParms(
         int  devType,
         int  devId,
         int  dir,
         int  *pPortSchedMode,
         int  *pMaxQueues,
         int  *pMaxSpQueues,
         BOOL *pPortShaper,
         BOOL *pQueueShaper,
         int  *pCfgFlags,
         BOOL *pbFound);

int rdpaCtl_GetRootTm(
         int  devType,
         int  devId,
         int  *pRootTmId,
         BOOL *pbFound);

int rdpaCtl_GetRootSpTm(
         int  devType,
         int  portId,
         int  *pRootSpTmId,
         BOOL *pbFound);

int rdpaCtl_GetRootWrrTm(
         int  devType,
         int  portId,
         int  *pRootWrrTmId,
         BOOL *pbFound);

int rdpaCtl_GetOrl(
         int  devType,
         int  portId,
         int  dir,
         int  *pOrlId,
         int  *pShapingRate,
         BOOL *pbOrlLinked,
         BOOL *pbFound);

int rdpaCtl_GetTmByQid(
         int  devType,
         int  devId,
         int  qId,
         int  *pTmId,
         BOOL *pbFound);

int rdpaCtl_GetQueueConfig(
         int  devType,
         int  devId,
         int  qId,
         int  dir,
         int  *pTmId,
         int  *pQsize,
         int  *pWeight,
         int  *pMinRate,
         int  *pShapingRate,
         BOOL *pBestEffort,
         BOOL *pbFound);

int rdpaCtl_SetQueueDropAlg(
         int devType,
         int devId,
         int qId,
         int dir,
         int dropAlg,
         int redMinThrLo,
         int redMaxThrLo,
         int redMinThrHi,
         int redMaxThrHi,
         uint32 priorityMask0,
         uint32 priorityMask1);

int rdpaCtl_GetQueueDropAlg(
         int devType,
         int devId,
         int qId,
         int dir,
         int *dropAlg,
         int *redMinThrLo,
         int *redMaxThrLo,
         int *redDropRateLo,
         int *redMinThrHi,
         int *redMaxThrHi,
         int *redDropRateHi,
         uint32 *priorityMask0,
         uint32 *priorityMask1);

int rdpaCtl_SetQueueSize(
         int devType,
         int devId,
         int qId,
         int dir,
         int size);

int rdpaCtl_SetQueueShaper(
        int devType,
        int devId,
        int qId,
        int dir,
        int cir,
        int pir,
        int bsz);

int rdpaCtl_RootTmConfig(
         int devType,
         int devId,
         int dir,
         int level,
         int arbiterMode,
         int rlMode,
         int cfgFlags,
         int *pRootTmId);

int rdpaCtl_TmConfig(
         int devType,
         int devId,
         int rootTmId,
         int dir,
         int level,
         int arbiterMode,
         int priorityIndex,
         int weight,
         int minRate,
         int shapingRate,
         int burst,
         int *pTmId);

int rdpaCtl_SvcQTmConfig(
         int devType,
         int devId,
         int parentTmId,
         int index,
         int level,
         int arbiterMode,
         int *pTmId);

int rdpaCtl_GetBestEffortTm(
         int devId,
         int dir,
         int *pTmId);

int rdpaCtl_GetTmSubsidiary(
        int dir,
        int tmId,
        int index,
        int *pTmId,
        BOOL* pbFound);

int rdpaCtl_RootTmRemove(
         int tmId,
         int dir,
         int devType,
         int devId);

int rdpaCtl_TmRemove(
         int tmId,
         int dir,
         int devType,
         int devId);

int rdpaCtl_OrlConfig(
         int devType,
         int portId,
         int dir,
         int shapingRate,
         int *pOrlTmId);

int rdpaCtl_OrlRemove(
         int devType,
         int portId,
         int dir);

int rdpaCtl_OrlLink(
         int devType,
         int portId,
         int tmId,
         int dir,
         int shapingRate);

int rdpaCtl_OrlUnlink(
         int devType,
         int portId,
         int tmId,
         int dir);

int rdpaCtl_QueueConfig(
         int devType,
         int devId,
         int tmId,
         int qid,
         int index,
         int dir,
         int qsize,
         int weight,
         int shapingRate,
         BOOL bestEffort,
         BOOL qClean);

int rdpaCtl_QueueRemove(
         int devType,
         int devId,
         int tmId,
         int dir,
         int qid,
         int index);

int rdpaCtl_QueueAllocate(
         int devType,
         int devId,
         int qid);

int rdpaCtl_QueueDislocate(
         int devType,
         int devId,
         int qid);

int rdpaCtl_GetQueueStats(
         int devType,
         int devId,
         int dir,
         int qid,
         rdpa_stat_1way_t *pStats);

uint32 rdpaCtl_getQueuePrioIndex(
         uint32 qid,
         rdpa_traffic_dir dir,
         uint32 maxQueues,
         int prio);

int rdpaCtl_GetTmRlConfig(
         int dir,
         int tmId,
         int* pShapingRate,
         int* pBurst,
         BOOL* pbFound);

int rdpaCtl_IptvLookupMethodSet(
         rdpa_drv_ioctl_iptv_lookup_method_t method);

int rdpaCtl_IptvLookupMethodGet(
         rdpa_drv_ioctl_iptv_lookup_method_t *method);

int rdpaCtl_IptvPrefixFilterMethodSet(
         rdpa_drv_ioctl_iptv_filter_method_t method);

int rdpaCtl_IptvPrefixFilterMethodGet(
         rdpa_drv_ioctl_iptv_filter_method_t *method);

int rdpaCtl_IptvEntryAdd(
        rdpa_drv_ioctl_iptv_egress_port_t egress_port,
        rdpa_drv_ioctl_iptv_key_t *key,
        rdpa_drv_ioctl_iptv_vlan_action_t vlan_action,
        uint16_t action_vid,
        uint32_t *index);

int rdpaCtl_IptvEntryRemove(
        uint32_t index,
        rdpa_drv_ioctl_iptv_egress_port_t egress_port);

int rdpaCtl_IptvEntryFlush(void);

int rdpaCtl_add_classification_rule(rdpactl_classification_rule_t *rule, uint8_t *prty);
int rdpaCtl_del_classification_rule(rdpactl_classification_rule_t *rule);
int rdpaCtl_add_classification(rdpactl_classification_rule_t *rule);
int rdpaCtl_del_classification(rdpactl_classification_rule_t *rule);
int rdpaCtl_find_classification(rdpactl_classification_rule_t *rule);

int rdpaCtl_get_wan_type(rdpa_if wan_if, rdpa_wan_type *wanType);
int rdpaCtl_get_in_tpid(uint16_t *inTpid);
int rdpaCtl_set_in_tpid(uint16_t inTpid);
int rdpaCtl_get_out_tpid(uint16_t *outTpid);
int rdpaCtl_set_out_tpid(uint16_t outTpid);
int rdpaCtl_set_epon_mode(rdpa_epon_mode eponMode);
int rdpaCtl_get_epon_mode(rdpa_epon_mode *eponMode);
int rdpaCtl_get_epon_status(BOOL *enable);
int rdpaCtl_set_always_tpid(uint16_t alwaysTpid);
int rdpaCtl_set_detect_tpid(uint16_t tpid, BOOL is_inner);
int rdpaCtl_get_force_dscp(uint16_t dir, BOOL *enable);
int rdpaCtl_set_force_dscp(uint16_t dir, BOOL enable);
int rdpaCtl_set_sys_car_mode(BOOL enable);


int rdpaCtl_set_port_sa_limit(uint16_t port_idx, BOOL is_wan_side, uint16_t max_limit);
int rdpaCtl_get_port_sa_limit(uint16_t port_idx, BOOL is_wan_side, uint16_t *max_limit);
int rdpaCtl_set_port_sal_miss_action(uint16_t port_idx, BOOL is_wan_side, rdpa_forward_action act);
int rdpaCtl_get_port_sal_miss_action(uint16_t port_idx, BOOL is_wan_side, rdpa_forward_action *act);
int rdpaCtl_set_port_dal_miss_action(uint16_t port_idx, BOOL is_wan_side, rdpa_forward_action act);
int rdpaCtl_get_port_dal_miss_action(uint16_t port_idx, BOOL is_wan_side, rdpa_forward_action *act);

int rdpaCtl_BrExist(uint8_t brId, BOOL *found);
int rdpaCtl_BrLocalSwitchSet(uint8_t brId, BOOL local_switch);


int rdpaCtl_TmRlConfig(int dir, int tmId, int shapingRate, int burst);

int rdpaCtl_LlidCreate(uint8_t llid_index);

int rdpaCtl_DsWanUdpFilterAdd(rdpactl_ds_wan_udp_filter_t *filter_p);
int rdpaCtl_DsWanUdpFilterDelete(long index);
int rdpaCtl_DsWanUdpFilterGet(rdpactl_ds_wan_udp_filter_t *filter_p);

int rdpaCtl_RdpaMwMCastSet(int dscp_val);
int rdpaCtl_RdpaMwWanConf(void);
int rdpaCtl_time_sync_init(void);

int rdpaCtl_filter_entry_create(rdpa_filter_key_t *key, rdpa_filter_ctrl_t *ctrl);
int rdpaCtl_filter_entry_modify(rdpa_filter_key_t *key, rdpa_filter_ctrl_t *ctrl);
int rdpaCtl_filter_set_global_cfg(rdpa_filter_global_cfg_t *global_cfg);
int rdpaCtl_filter_etyp_udef_cfg(uint32_t udef_inx, uint32_t udef_val);
int rdpaCtl_filter_tpid_vals_cfg(rdpa_filter_tpid_vals_t *tpid_vals, BOOL direction);
int rdpaCtl_filter_oui_cfg(rdpa_filter_oui_val_key_t *oui_val_key, uint32_t oui_val);
int rdpaCtl_filter_stats_get(rdpa_filter_stats_key_t *stats_params, int64_t *stats_val);

int rdpaCtl_DscpToPitGet(BOOL *found, tmctl_dscpToPbitCfg_t *map);

int rdpaCtl_DscpToPitSet(tmctl_dscpToPbitCfg_t map);

int rdpaCtl_PitToQueueGet(int devType, 
                        int devId, 
                        BOOL *found,
                        tmctl_pbitToQCfg_t *pbit_q_map);

int rdpaCtl_PitToQueueSet(int devType, 
                        int devId, 
                        tmctl_pbitToQCfg_t pbit_q_map);

int rdpaCtl_PktBasedQosGet(int dir, int type, BOOL *enable);

int rdpaCtl_PktBasedQosSet(int dir, int type, BOOL enable);

int rdpaCtl_get_svc_q_mode(int *enable);
int rdpaCtl_set_svc_q_mode(int enable);

#endif /* _RDPACTL_API_H_ */

