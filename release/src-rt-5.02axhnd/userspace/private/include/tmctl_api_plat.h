/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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


#ifndef _TMCTL_API_PLAT_H_
#define _TMCTL_API_PLAT_H_

/*!\file tmctl_api_plat.h
 * \brief This file contains declarations for QoS related functions.
 *
 */

tmctl_ret_e tmctl_portTmInit_plat(tmctl_devType_e devType,
                                  tmctl_if_t*     if_p,
                                  uint32_t        cfgFlags,
                                  int             numQueues);


tmctl_ret_e tmctl_portTmUninit_plat(tmctl_devType_e devType,
                                    tmctl_if_t*     if_p);


tmctl_ret_e tmctl_getQueueCfg_plat(tmctl_devType_e   devType,
                                   tmctl_if_t*       if_p,
                                   int               queueId,
                                   tmctl_queueCfg_t* qcfg_p);


tmctl_ret_e tmctl_setQueueCfg_plat(tmctl_devType_e   devType,
                                   tmctl_if_t*       if_p,
                                   tmctl_queueCfg_t* qcfg_p);


tmctl_ret_e tmctl_delQueueCfg_plat(tmctl_devType_e devType,
                                   tmctl_if_t*     if_p,
                                   int             queueId);


tmctl_ret_e tmctl_getPortShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p);


tmctl_ret_e tmctl_setPortShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p);


tmctl_ret_e tmctl_allocQueueProfileId_plat(int* queueProfileId_p);


tmctl_ret_e tmctl_freeQueueProfileId_plat(int queueProfileId);


tmctl_ret_e tmctl_getQueueProfile_plat(int                   queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p);


tmctl_ret_e tmctl_setQueueProfile_plat(int                   queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p);


tmctl_ret_e tmctl_getQueueDropAlg_plat(tmctl_devType_e       devType,
                                       tmctl_if_t*           if_p,
                                       int                   queueId,
                                       tmctl_queueDropAlg_t* dropAlg_p);


tmctl_ret_e tmctl_setQueueDropAlg_plat(tmctl_devType_e       devType,
                                       tmctl_if_t*           if_p,
                                       int                   queueId,
                                       tmctl_queueDropAlg_t* dropAlg_p);

tmctl_ret_e tmctl_setQueueDropAlgExt_plat(tmctl_devType_e          devType,
                                          tmctl_if_t*              if_p,
                                          int                      queueId,
                                          tmctl_queueDropAlg_t* dropAlg_p);


tmctl_ret_e tmctl_getXtmChannelDropAlg_plat(tmctl_devType_e       devType,
                                            int                   channelId,
                                            tmctl_queueDropAlg_t* dropAlg_p);


tmctl_ret_e tmctl_setXtmChannelDropAlg_plat(tmctl_devType_e       devType,
                                            int                   channelId,
                                            tmctl_queueDropAlg_t* dropAlg_p);


tmctl_ret_e tmctl_getQueueStats_plat(tmctl_devType_e     devType,
                                     tmctl_if_t*         if_p,
                                     int                 queueId,
                                     tmctl_queueStats_t* stats_p);


tmctl_ret_e tmctl_getPortTmParms_plat(tmctl_devType_e      devType,
                                      tmctl_if_t*          if_p,
                                      tmctl_portTmParms_t* tmParms_p);


tmctl_ret_e tmctl_getDscpToPbit_plat(tmctl_dscpToPbitCfg_t* cfg_p);


tmctl_ret_e tmctl_setDscpToPbit_plat(tmctl_dscpToPbitCfg_t* cfg_p);


tmctl_ret_e tmctl_getPbitToQ_plat(tmctl_devType_e devType, 
                                  tmctl_if_t* if_p, 
                                  tmctl_pbitToQCfg_t* cfg_p);


tmctl_ret_e tmctl_setPbitToQ_plat(tmctl_devType_e devType, 
                                  tmctl_if_t* if_p, 
                                  tmctl_pbitToQCfg_t* cfg_p);


tmctl_ret_e tmctl_getForceDscpToPbit_plat(tmctl_dir_e dir, BOOL* enable_p);


tmctl_ret_e tmctl_setForceDscpToPbit_plat(tmctl_dir_e dir, BOOL* enable_p);


tmctl_ret_e tmctl_getPktBasedQos_plat(tmctl_dir_e dir, 
                                      tmctl_qosType_e type, 
                                      BOOL* enable_p);


tmctl_ret_e tmctl_setPktBasedQos_plat(tmctl_dir_e dir, 
                                      tmctl_qosType_e type, 
                                      BOOL* enable_p);

tmctl_ret_e tmctl_setQueueSize_plat(tmctl_devType_e devType,
                                    tmctl_if_t* if_p,
                                    int queueId,
                                    int size);

int tmctl_getDefQSize_plat(tmctl_devType_e devType, tmctl_dir_e dir);

tmctl_ret_e tmctl_setQueueShaper_plat(tmctl_devType_e          devType,
                                      tmctl_if_t*        if_p,
                                      int                queueId,
                                      tmctl_shaper_t     *shaper_p);


#endif /* _TMCTL_API_PLAT_H_ */
