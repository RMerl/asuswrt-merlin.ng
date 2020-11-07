/*
* <:copyright-BRCM:2017:proprietary:standard
* 
*    Copyright (c) 2017 Broadcom 
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
#ifndef __TMCTL_BDMF_RDPA_H_
#define __TMCTL_BDMF_RDPA_H_

#include "user_api.h"
#include "tmctl_api.h"
#include "rdpa_drv.h"

#define MAX_Q_PER_TM_MULTI_LEVEL 32
#define TMCTL_ORL_TM_id          -1
#define TMCTL_ALL_TCONT_ID       -1

/* high priorty number get lowset idx */
#define HIGH_P_LOW_IDX 1 

#if !defined(BCM_PON_XRDP) && !defined(BCM_DSL_XRDP)
#define MAX_Q_PER_SID 4
#else
#define MAX_Q_PER_SID 8
#endif /* !defined(BCM_PON_XRDP) && !defined(BCM_DSL_XRDP */

#ifndef G9991
#define MAX_Q_PER_LAN_TM 8 
#else
#define MAX_Q_PER_LAN_TM MAX_Q_PER_SID 
#endif /* G9991 */

#ifndef CHIP_6846
#define MAX_Q_PER_WAN_TM 32
#else
#define MAX_Q_PER_WAN_TM 8
#endif /* CHIP_6846 */

#define BDMF_NULL 0

#ifdef BCM_RDP
#define getDefaultQueueSize(devType, dir) \
    ((devType == RDPA_IOCTL_DEV_TCONT) ? TMCTL_DEF_TCONT_Q_SZ :\
    ((devType == RDPA_IOCTL_DEV_LLID) ? TMCTL_DEF_LLID_Q_SZ :\
    ((devType == RDPA_IOCTL_DEV_PORT && dir == rdpa_dir_us) ? \
    TMCTL_DEF_ETH_Q_SZ_US: TMCTL_DEF_ETH_Q_SZ_DS)))
#else
#define getDefaultQueueSize(devType, dir) \
    ((dir == rdpa_dir_us) ? TMCTL_DEF_ETH_Q_SZ_US: TMCTL_DEF_ETH_Q_SZ_DS)
#endif /* BCM_RDP */

#define PACKET_SIZE 1536 /* this is value is used only to convert 'tmctl packet based api' into XRDP api which is in bytes */

#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
/* On XRDP platforms queue size is in bytes*/
#define getDeviceQueueSize(usrSize) (usrSize * PACKET_SIZE)
#define getUsrQueueSize(devSize) (devSize / PACKET_SIZE)
#else
/* On RDP platforms queue size is in packets*/
#define getDeviceQueueSize(usrSize) (usrSize)
#define getUsrQueueSize(devSize) (devSize)
#endif /* BCM_PON_XRDP || defined(BCM_DSL_XRDP */

#define getDir(devType, rdpaIf) \
    ((((devType == RDPA_IOCTL_DEV_PORT) && (!rdpa_if_is_wan(rdpaIf))) ||\
    (devType == RDPA_IOCTL_DEV_NONE))? rdpa_dir_ds : rdpa_dir_us)

#define convertDropAlg(tmDropAlg) \
    ((tmDropAlg == TMCTL_DROP_DT) ? rdpa_tm_drop_alg_dt :\
    ((tmDropAlg == TMCTL_DROP_RED) ? rdpa_tm_drop_alg_red :\
    ((tmDropAlg == TMCTL_DROP_WRED) ? rdpa_tm_drop_alg_wred :\
    rdpa_tm_drop_alg_dt)))

#define IS_ACTIV_Q(q_cfg) \
    (q_cfg.queue_id != BDMF_INDEX_UNASSIGNED && q_cfg.drop_threshold)

#define IS_SINGLE_LEVEL(level) \
    (level == rdpa_tm_level_queue)

#define KBPS_TO_BPS(kbps) (kbps) * 1000ULL
#define BPS_TO_KBPS(kbps) (kbps) / 1000ULL

tmctl_ret_e tmctl_RdpaTmInit(rdpa_drv_ioctl_dev_type rdpa_dev,
                             tmctl_devType_e dev_Type,
                             rdpa_if if_id,
                             uint32_t cfg_flags,
                             int num_queues);

tmctl_ret_e tmctl_RdpaTmUninit(rdpa_drv_ioctl_dev_type rdpa_dev,
                               tmctl_devType_e dev_type, 
                               rdpa_if if_id);

tmctl_ret_e tmctl_RdpaQueueCfgGet(rdpa_drv_ioctl_dev_type rdpa_dev,
                                  tmctl_devType_e dev_type,
                                  rdpa_if if_id,
                                  int queue_id,
                                  tmctl_queueCfg_t* qcfg_p);

tmctl_ret_e tmctl_RdpaTmQueueSet(rdpa_drv_ioctl_dev_type rdpa_dev,
                                 tmctl_devType_e dev_type,
                                 rdpa_if if_id,
                                 tmctl_queueCfg_t* qcfg_p);

tmctl_ret_e tmctl_RdpaTmQueueDel(rdpa_drv_ioctl_dev_type rdpa_dev,
                                 tmctl_devType_e dev_type,
                                 rdpa_if if_id,
                                 int queue_id);

tmctl_ret_e tmctl_RdpaGetPortShaper(rdpa_drv_ioctl_dev_type rdpa_dev,
                                    tmctl_devType_e dev_type,
                                    rdpa_if if_id,
                                    tmctl_shaper_t* shaper_p);

tmctl_ret_e tmctl_RdpaSetPortShaper(rdpa_drv_ioctl_dev_type rdpa_dev,
                                   tmctl_devType_e dev_type,
                                   rdpa_if if_id,
                                   tmctl_shaper_t* shaper_p);

tmctl_ret_e tmctl_RdpaGetQueueDropAlg(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          rdpa_if if_id,
                          int queue_id,
                          tmctl_queueDropAlg_t *dropAlg_p);

tmctl_ret_e tmctl_RdpaSetQueueDropAlg(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          rdpa_if if_id,
                          int queue_id,
                          tmctl_queueDropAlg_t* dropAlg_p);

tmctl_ret_e tmctl_RdpaGetQueueStats(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          rdpa_if if_id,
                          int queue_id, 
                          tmctl_queueStats_t* stats_p);

tmctl_ret_e tmctl_RdpaSetQueueSize(rdpa_drv_ioctl_dev_type rdpa_dev,        
                                   tmctl_devType_e dev_type,
                                   int if_id,
                                   int queue_id,
                                   int size);

tmctl_ret_e tmctl_RdpaSetQueueShaper(rdpa_drv_ioctl_dev_type rdpa_dev,        
                                   tmctl_devType_e dev_type,
                                   int if_id,
                                   int queue_id,
                                   tmctl_shaper_t *shaper_p);

#if defined(BCM_PON)
tmctl_ret_e tmctl_RdpaGetDscpToPbit(rdpa_drv_ioctl_dev_type rdpa_dev,
                                    int if_id, 
                                    tmctl_dscpToPbitCfg_t* cfg_p);

tmctl_ret_e tmctl_RdpaSetDscpToPbit(rdpa_drv_ioctl_dev_type rdpa_dev,
                                    int if_id, 
                                    tmctl_dscpToPbitCfg_t* cfg_p);

tmctl_ret_e tmctl_RdpaCreatePolicer(tmctl_policer_t *policer_p);
tmctl_ret_e tmctl_RdpaModifyPolicer(tmctl_policer_t *policer_p);
tmctl_ret_e tmctl_RdpaDeletePolicer(tmctl_dir_e dir, int policerId);
tmctl_ret_e tmctl_RdpaSetOverAllShaper(tmctl_shaper_t *shaper_p);
tmctl_ret_e tmctl_RdpaGetOverAllShaper(tmctl_shaper_t *shaper_p, uint32_t *tcont_map);
tmctl_ret_e tmctl_RdpaLinkOverAllShaper(int tcont_id, BOOL do_link);
#endif

#endif /* __TMCTL_BDMF_RDPA_H_ */
