#ifndef __FAP_TM_H_INCLUDED__
#define __FAP_TM_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/

/*
 *******************************************************************************
 * File Name  : fap_tm.h
 *
 * Description: This file contains the FAP Traffic Manager API.
 *
 *******************************************************************************
 */

#if defined(CC_FAP4KE_TM)

#define FAP_TM_BUCKET_SIZE_MAX   65535
#define FAP_TM_KBPS_MAX          1000000

/* This MUST be kept in sync with fapIoctl_tmMode_t */
typedef enum {
    FAP_TM_MODE_AUTO=0,
    FAP_TM_MODE_MANUAL,
    FAP_TM_MODE_MAX
} fapTm_mode_t;

/* This MUST be kept in sync with fapIoctl_tmPortType_t */
typedef enum {
    FAP_TM_PORT_TYPE_LAN=0,
    FAP_TM_PORT_TYPE_WAN,
    FAP_TM_PORT_TYPE_MAX
} fapTm_portType_t;

typedef enum {
    FAP_TM_SHAPING_TYPE_DISABLED=0,
    FAP_TM_SHAPING_TYPE_RATE,
    FAP_TM_SHAPING_TYPE_RATIO,
    FAP_TM_SHAPING_TYPE_MAX
} fapTm_shapingType_t;

/* This MUST be kept in sync with fapIoctl_tmDropAlg_t */
typedef enum
{
    FAP_TM_DROP_ALG_DT=0,
    FAP_TM_DROP_ALG_RED,
    FAP_TM_DROP_ALG_WRED,
    FAP_TM_DROP_ALG_MAX
} fapTm_dropAlg_t;


/* Management API */
void fapTm_masterConfig(int enable);
int fapTm_portConfig(uint8 port, fapTm_mode_t mode, int kbps, int mbs,
                     fapTm_shapingType_t shapingType);
int fapTm_getPortConfig(uint8 port, fapTm_mode_t mode, int *kbps_p, int *mbs_p,
                        fapTm_shapingType_t *shapingType_p);
int fapTm_getPortCapability(uint8 port, uint32_t *schedType_p, int *maxQueues_p,
                            int *maxSpQueues_p, uint8 *portShaper_p, uint8 *queueShaper_p);                       
int fapTm_setPortMode(uint8 port, fapTm_mode_t mode);
fapTm_mode_t fapTm_getPortMode(uint8 port);
int fapTm_modeReset(uint8 port, fapTm_mode_t mode);
int fapTm_portType(uint8 port, fapTm_portType_t portType);
int fapTm_portEnable(uint8 port, fapTm_mode_t mode, int enable);
int fapTm_pauseEnable(uint8 port, int enable);
int fapTm_apply(uint8 port);
int fapTm_queueConfig(uint8 port, fapTm_mode_t mode, uint8 queue,
                      uint8 shaperType, int kbps, int mbs);
int fapTm_queueUnconfig(uint8 port, fapTm_mode_t mode, uint8 queue);
int fapTm_getQueueConfig(uint8 port, fapTm_mode_t mode, uint8 queue,
                         int *kbps_p, int *minKbps_p, int *mbs_p, int *weight_p, int *qsize_p);
int fapTm_allocQueueProfileId(int *queueProfileId_p);
int fapTm_freeQueueProfileId(int queueProfileId);
int fapTm_findQueueProfileId(int dropProbability, int minThreshold, int maxThreshold, int* queueProfileId_p);
int fapTm_queueProfileConfig(int queueProfileId, int dropProbability, int minThreshold,
                             int maxThreshold);
int fapTm_getQueueProfileConfig(int queueProfileId, int *dropProbability_p,
                             int *minThreshold_p, int *maxThreshold_p);
int fapTm_queueDropAlgConfig(uint8 port, uint8 queue, fapTm_dropAlg_t dropAlgorithm,
                            int queueProfileIdLo, int queueProfileIdHi,
                            uint32_t priorityMask0, uint32_t priorityMask1);
int fapTm_getQueueDropAlgConfig(uint8 port, uint8 queue, fapTm_dropAlg_t *dropAlgorithm_p,
                            int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                            uint32_t *priorityMask0_p, uint32_t *priorityMask1_p);
int fapTm_queueDropAlgConfigExt(uint8 port, uint8 queue, fapTm_dropAlg_t dropAlgorithm,
                                int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                int dropProbabilityHi, int minThresholdHi, int maxThresholdHi,
                                uint32_t priorityMask0, uint32_t priorityMask1);
int fapTm_checkHighPrio(uint8 phy, uint8 port, uint8 queue, uint8 chnl, uint32 tc);
int fapTm_checkSetHighPrio(uint8 port, uint8 queue, uint32 tc, uint32 *destQueue_p);
int fapTm_xtmCheckHighPrio(uint8 chnl, uint32 tc);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int fapTm_XtmQueueDropAlgConfig(uint8_t chnl, fapTm_dropAlg_t dropAlgorithm,
                                int queueProfileIdLo, int queueProfileIdHi,
                                uint32_t priorityMask0, uint32_t priorityMask1);
int fapTm_getXtmQueueDropAlgConfig(uint8_t chnl, fapTm_dropAlg_t *dropAlgorithm_p,
                                   int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                                   uint32_t *priorityMask0_p, uint32_t *priorityMask1_p);
int fapTm_XtmQueueDropAlgConfigExt(uint8_t chnl, fapTm_dropAlg_t dropAlgorithm,
                                   int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                   int dropProbabilityHi, int minThresholdHi, int maxThresholdHi);
#endif
int fapTm_getQueueStats(uint8 port, fapTm_mode_t mode, uint8 queue,
                        uint32_t *txPackets_p, uint32_t *txBytes_p, uint32_t *droppedPackets_p, uint32_t *droppedBytes_p);
int fapTm_arbiterConfig(uint8 port, fapTm_mode_t mode, uint8 arbiterType, uint8 arbiterArg);
int fapTm_getArbiterConfig(uint8 port, fapTm_mode_t mode, fap4keTm_arbiterType_t *arbiterType_p, int *arbiterArg_p);
int fapTm_setQueueWeight(uint8 port, fapTm_mode_t mode, uint8 queue, uint8 weight);
int fapTm_getQueueRate(uint8 port, uint8 queue);
int fapTm_mapTmQueueToSwQueue(uint8 port, uint8 queue, uint8 swQueue);

/* Stats/Debuggging */
void fapTm_status(void);
void fapTm_stats(int port);
void fapTm_dumpMaps(void);

/* Others */
void fapTm_setFlowInfo(fap4kePkt_flowInfo_t *flowInfo_p, uint32 virtDestPortMask);
int fapTm_ioctl(unsigned long arg);
void fapTm_init(void);

#endif /* CC_FAP4KE_TM */

#endif /* __FAP_TM_H_INCLUDED__ */
