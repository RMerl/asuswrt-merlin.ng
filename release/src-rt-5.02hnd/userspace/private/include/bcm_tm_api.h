/***********************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2015:proprietary:standard

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

#ifndef _BCM_TM_API_H_
#define _BCM_TM_API_H_

#include "bcm_tm_defs.h"

#define BCM_TM_API_IFNAME_INVALID_STR "NULL"
#define BCM_TM_API_IS_IFNAME_VALID(_ifName)                     \
    strncmp((_ifName), BCM_TM_API_IFNAME_INVALID_STR, IFNAMSIZ)

int bcmTmApi_masterConfig(int enable);
int bcmTmApi_portConfig(char *ifName, bcmTmDrv_mode_t mode, int kbps,
                        int mbs, bcmTmDrv_shapingType_t shapingType);
int bcmTmApi_getPortConfig(char *ifName, bcmTmDrv_mode_t mode, int *kbps_p,
                           int *mbs_p, bcmTmDrv_shapingType_t *shapingType_p);
int bcmTmApi_getPortCapability(char *ifName, uint32_t *schedType_p, int *maxQueues_p,
                               int *maxSpQueues_p, uint8_t *portShaper_p, uint8_t *queueShaper_p);
int bcmTmApi_queueConfig(char *ifName, bcmTmDrv_mode_t mode, int queue,
                         bcmTmDrv_shaperType_t shaperType, int kbps, int mbs);
int bcmTmApi_queueUnconfig(char *ifName, bcmTmDrv_mode_t mode, int queue);
int bcmTmApi_getQueueConfig(char *ifName, bcmTmDrv_mode_t mode, int queue,
                            int *maxRateKbps_p, int *minRateKbps_p, int *mbs_p,
                            int *weight_p, int *qsize_p);
int bcmTmApi_setQueueCap(char *ifName, bcmTmDrv_mode_t mode, int queue,
                         int nbrOfEntriesCap);
int bcmTmApi_allocQueueProfileId(int *queueProfileId_p);
int bcmTmApi_freeQueueProfileId(int queueProfileId);
int bcmTmApi_queueProfileConfig(int queueProfileId, int dropProbability,
                                int minThreshold, int maxThreshold);
int bcmTmApi_getQueueProfileConfig(int queueProfileId, int *dropProbability_p,
                                   int *minThreshold_p, int *maxThreshold_p);
int bcmTmApi_queueDropAlgConfig(char *ifName, int queue, bcmTmDrv_dropAlg_t dropAlgorithm,
                                int queueProfileIdLo, int queueProfileIdHi,
				uint32_t priorityMask0, uint32_t priorityMask1);
int bcmTmApi_getQueueDropAlgConfig(char *ifName, int queue, bcmTmDrv_dropAlg_t *dropAlgorithm_p,
                                   int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                                   uint32_t *priorityMask0_p, uint32_t *priorityMask1_p);
int bcmTmApi_xtmChannelDropAlgConfig(int channel, bcmTmDrv_dropAlg_t dropAlgorithm,
                                     int queueProfileIdLo, int queueProfileIdHi,
                                     uint32_t priorityMask0, uint32_t priorityMask1);
int bcmTmApi_getXtmChannelDropAlgConfig(int channel, bcmTmDrv_dropAlg_t *dropAlgorithm_p,
                                        int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                                        uint32_t *priorityMask0_p, uint32_t *priorityMask1_p);
int bcmTmApi_getQueueStats(char *ifName, bcmTmDrv_mode_t mode, int queue,
                           uint32_t *txPackets_p, uint32_t *txBytes_p,
                           uint32_t *droppedPackets_p, uint32_t *droppedBytes_p,
                           uint32_t *bps_p);
int bcmTmApi_setQueueWeight(char *ifName, bcmTmDrv_mode_t mode, int queue, int weight);
int bcmTmApi_arbiterConfig(char *ifName, bcmTmDrv_mode_t mode,
                           bcmTmDrv_arbiterType_t arbiterType, int arbiterArg);
int bcmTmApi_getArbiterConfig(char *ifName, bcmTmDrv_mode_t mode,
                              bcmTmDrv_arbiterType_t *arbiterType_p, int *arbiterArg_p);
int bcmTmApi_setPortMode(char *ifName, bcmTmDrv_mode_t mode);
int bcmTmApi_modeReset(char *ifName, bcmTmDrv_mode_t mode);
int bcmTmApi_portEnable(char *ifName, bcmTmDrv_mode_t mode, int enable);
int bcmTmApi_apply(char *ifName);
int bcmTmApi_status(void);
int bcmTmApi_stats(char *ifName);

#endif /* _BCM_TM_API_H_ */

