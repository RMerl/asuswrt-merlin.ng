/***********************************************************
 *
 * <:copyright-BRCM:2009:DUAL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 ***********************************************************/

#ifndef __FAP4KE_XTMRT_H_INCLUDED__
#define __FAP4KE_XTMRT_H_INCLUDED__


 /*****************************************************************************
 * File Name  : fap4ke_xtmrt.h
 *
 * Description: This file contains the constants and prototypes needed for the
 *              XTMRT Driver running on the 4ke.
 *
 *****************************************************************************/

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)

#include "bcmPktDma_defines.h"

#define FAP4KE_XTM_MAX_DEV_CTXS   16    /* up to 256 */
#define FAP4KE_XTM_MAX_MATCH_IDS  128

#define XTM_TX_VCID_MASK    0xf

#define pXtmCtrl ( &p4keDspramGbl->xtm )

typedef struct {
    uint16 encapType;
    uint8  headerLen;
    uint8  trailerLen;
} fap4keXtm_devContext_t;

typedef struct {
    fap4keXtm_devContext_t devContext[FAP4KE_XTM_MAX_DEV_CTXS];
    uint8 matchIdToDevId[FAP4KE_XTM_MAX_MATCH_IDS];
} fap4keXtm_devMap_t;

typedef struct {
    uint32 dropAlg          : 4,
           queueProfileIdLo : 14,
           queueProfileIdHi : 14;
} fap4keXtm_queueDropAlg_t;

typedef struct {
    int numTxQueues;
    int numTxBufsQdAll;
    uint16 numTxBufsRsrvdPerQueue[XTM_TX_CHANNELS_MAX];
    fap4keXtm_queueDropAlg_t queueDropAlg[XTM_TX_CHANNELS_MAX];
} fap4keXtm_qos_t;

/* XTMRT Driver Prototypes */
fapRet xtmInit(int isFirstTime);
fapRet xtmRxChannelInit(uint32 channel, uint32 numBds, uint32 Bds, uint32 Dma);
fapRet xtmRxChannelUnInit(uint32 channel);
fapRet xtmTxChannelInit(uint32 channel, uint32 numBds, uint32 Bds, uint32 Dma);
fapRet xtmTxChannelInitState(uint32 channel, uint32 DmaStateRam);
fapRet xtmCreateDevice(uint32 devId, uint32 encapType, uint32 headerLen, uint32 trailerLen);
fapRet xtmLinkUp(uint32 devId, uint32 matchId);
fapRet xtmQueueDropAlgConfig(uint8 channel, uint8 dropAlg, uint16 queueProfileIdLo, uint16 queueProfileIdHi);
fapRet xtmDrvResetStats(uint32 port);
void xtmDmaStatus(int channel);
void xtmRxDmaStatus(int channel);
void xtmTxDmaStatus(int channel);

/* For use by fap4ke_ffe.c fap4ke_enet.c and fap4ke_hostIf.c */
void xtmFreeTxBuffers(uint32 channel, int forceFree);
void xtmFreeRecvBuf(int channel, unsigned char *pBuf);

void xtmTxCleanup(void);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
void xtmBpmResetAllocBufResp_wait(uint32 channel);
int  xtmBpmAllocBufResp( uint32 channel, uint32 seqId, uint32 numBufs);
void xtmBpmFreeBuf(int channel, uint8 *pData);
void xtmBpmFreeBufResp( uint32 channel, uint32 seqId );
int  xtmFlushIudmaRings( void );
int  xtmFlushIudmaRxRing( uint32 channel );
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define fap4keXtm_freeRecvBuf(_rxChannel, _pBuf) \
xtmBpmFreeBuf(_rxChannel, _pBuf)
#else
#define fap4keXtm_freeRecvBuf(_rxChannel, _pBuf) \
xtmFreeRecvBuf(_rxChannel, _pBuf)
#endif

#endif /* CONFIG_BCM_XTMCFG */
#endif  /* defined(__FAP4KE_XTMRT_H_INCLUDED__) */
