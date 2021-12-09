#ifndef __PKTDMA_H_INCLUDED__
#define __PKTDMA_H_INCLUDED__
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
 * File Name  : bcmPktDma.h
 *
 * Description: This file contains the Packet DMA API definition. It may be
 *              included in Kernel space only.
 *
 *******************************************************************************
 */

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include "bcm_mm.h"
#ifdef FAP_4KE
#include "bcmPktDma_bds.h"
#include "bcmPktDma_structs.h"
#include "bcmenet.h"
#include "bcmPktDmaHooks.h"
#else /* FAP_4KE */
#include <linux/nbuff.h>
#include <linux/netdevice.h>
#include <linux/ppp_channel.h>
#include "bcmPktDma_bds.h"
#include "bcmenet.h"
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include "bcmxtmrtimpl.h"
#endif
#include "bcmPktDma_structs.h"
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#include "fap_hw.h"
#include "fap4ke_memory.h"
#include "bcmPktDmaHooks.h"
#endif
#include <linux/bcm_colors.h>
#endif /* FAP_4KE */

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)

#if defined(CONFIG_BCM963268) && (defined(CONFIG_BCM_PKTDMA_TX_SPLITTING))
#error "RX and TX SPLITTING MAY NOT BE CONFIGURED FOR THE 63268 OR 6828 CHIP"
#endif

enum { HOST_OWNED=0, FAP0_OWNED, FAP1_OWNED };

#define FAP0_IDX 0
#define FAP1_IDX 1
#define FAP_INVALID_IDX 0x0DEADFAB

#if defined(CONFIG_BCM963268)

#if defined(CONFIG_BCM_GMAC)
#define PKTDMA_ETH_RX_OWNERSHIP        FAP0_OWNED, FAP1_OWNED
#define PKTDMA_ETH_TX_OWNERSHIP        FAP1_OWNED, FAP0_OWNED
#define PKTDMA_ETH_US_TX_IUDMA         1
#define PKTDMA_ETH_DS_TX_IUDMA         0
#else
#define PKTDMA_ETH_RX_OWNERSHIP        FAP0_OWNED, FAP1_OWNED
#define PKTDMA_ETH_TX_OWNERSHIP        FAP0_OWNED, FAP1_OWNED
#define PKTDMA_ETH_US_TX_IUDMA         0
#define PKTDMA_ETH_DS_TX_IUDMA         1
#endif

#define PKTDMA_XTM_RX_OWNERSHIP        FAP1_OWNED, FAP1_OWNED
#define PKTDMA_XTM_TX_OWNERSHIP        FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED, FAP0_OWNED
#define PKTDMA_ETH_US_IUDMA            0
#define PKTDMA_ETH_DS_IUDMA            1
#define PKTDMA_XTM_IUDMA               0
#define PKTDMA_DEFAULT_IUDMA           PKTDMA_ETH_US_IUDMA
#define PKTDMA_US_FAP_INDEX            0
#define PKTDMA_DS_FAP_INDEX            1
#define PKTDMA_XTM_TX_DEFAULT_QUEUE    0

#else  

#error "UNSUPPORTED BOARD"

#endif /* CONFIG_BCM963268 */

#define PKTDMA_ETH_PORT_TO_IUDMA       PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA, PKTDMA_ETH_US_IUDMA

//#ifndef FAP_4KE
extern const int g_Eth_rx_iudma_ownership[ENET_RX_CHANNELS_MAX];
extern const int g_Eth_tx_iudma_ownership[ENET_TX_CHANNELS_MAX];
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
extern const int g_Xtm_rx_iudma_ownership[XTM_RX_CHANNELS_MAX];
extern const int g_Xtm_tx_iudma_ownership[XTM_TX_CHANNELS_MAX];
#endif
//#endif /* !FAP_4KE */

/* Constants for Host-FAP communication of rx buffer free info */
#define PKTDMA_RX_SOURCE_MASK          0xFFFF
#define PKTDMA_RX_CHANNEL_MASK         0xFFFF
#define PKTDMA_RX_CHANNEL_SHIFT        16

#else /* if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */
#define PKTDMA_XTM_TX_DEFAULT_QUEUE 0
#endif  /* if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

#define PKTDMA_ETH_NO_BD_THRESHOLD     30

#ifdef FAP_4KE
#define BCM_PKTDMA_LOG_ERROR(fmt, arg...)  fap4kePrt_Error(fmt, ##arg)
#else
#define BCM_PKTDMA_LOG_ERROR(fmt, arg...)  printk(CLRerr "ERROR - %s" fmt CLRnl, __FUNCTION__, ##arg)
#endif

//#define CC_BCM_PKTDMA_DEBUG

/* TX Buffer sources supported by bcmPktDma Lib */
enum { HOST_VIA_LINUX=0,
       HOST_VIA_DQM,
       HOST_VIA_DQM_CSUM,
       HOST_VIA_DQM_GSO,
       HOST_VIA_DQM_GSO_LAST,
       HOST_VIA_DQM_GSO_FRAG,
       HOST_VIA_DQM_GSO_LOOPBACK,
       HOST_VIA_DQM_GSO_LOOPBACK_LAST,
       FAP_XTM_RX,
       FAP_XTM_RX_GSO,
       FAP_XTM_RX_GSO_LAST,
       FAP_ETH_RX,
       FAP_ETH_RX_GSO, //10
       FAP_ETH_RX_GSO_LAST,
       HOST_XTM_RX,
       HOST_XTM_RX_GSO,
       HOST_XTM_RX_GSO_LAST,
       HOST_ETH_RX,
       HOST_ETH_RX_GSO,
       HOST_ETH_RX_GSO_LAST, 
       FAP_MCAST_HDR_MCLOG,
       FAP_MCAST_HDR_POOL,
       FAP_MCAST_ETH_RX,
       FAP_MCAST_XTM_RX
	};

/* TX DMA versions supported by bcmPktDma Lib. This is required in case of
 * BCM6368 running non-bonding/bonding. For non-bonding only HW_DMA is used and
 * for bonding both SW/HW DMA are used.
 */
enum {XTM_HW_DMA=0, XTM_SW_DMA=1} ;

#ifndef FAP_4KE

extern BcmEnet_devctrl *               g_pEnetDevCtrl;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
extern PBCMXTMRT_GLOBAL_INFO           g_pXtmGlobalInfo;
#endif
#endif /* !FAP_4KE */

#define FAP_ENET_CHAN0 (1 << 0)
#define FAP_ENET_CHAN1 (1 << 1)

#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
/* Most chips use the Iudma version of the bcmPkt interface */
/* Eth versions of the interface */
#define bcmPktDma_EthInitRxChan           bcmPktDma_EthInitRxChan_Iudma
#define bcmPktDma_EthInitTxChan           bcmPktDma_EthInitTxChan_Iudma
#if defined(CONFIG_BCM_GMAC)
#define bcmPktDma_EthUnInitRxChan         bcmPktDma_EthUnInitRxChan_Iudma
#define bcmPktDma_EthUnInitTxChan         bcmPktDma_EthUnInitTxChan_Iudma
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define bcmPktDma_EthSetIqThresh          bcmPktDma_EthSetIqThresh_Iudma
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_EthSetRxChanBpmThresh   bcmPktDma_EthSetRxChanBpmThresh_Iudma
#define bcmPktDma_EthSetTxChanBpmThresh   bcmPktDma_EthSetTxChanBpmThresh_Iudma
#define bcmPktDma_EthXmitBufCountGet      bcmPktDma_EthXmitBufCountGet_Iudma
#endif
#define bcmPktDma_EthSelectRxIrq          bcmPktDma_EthSelectRxIrq_Iudma
#define bcmPktDma_EthClrRxIrq             bcmPktDma_EthClrRxIrq_Iudma
#define bcmPktDma_EthRecvAvailable        bcmPktDma_EthRecvAvailable_Iudma
#define bcmPktDma_EthRecv                 bcmPktDma_EthRecv_Iudma
#define bcmPktDma_EthFreeRecvBuf          bcmPktDma_EthFreeRecvBuf_Iudma
#define bcmPktDma_EthXmitAvailable(p1,unused)   \
    bcmPktDma_EthXmitAvailable_Iudma(p1)
#define bcmPktDma_EthXmit(p1,p2,p3,p4,p5,p6,p7,unused1,unused2,unused3,p8,unused4) \
    bcmPktDma_EthXmitNoCheck_Iudma (p1,p2,p3,p4,p5,p6,p7,p8)
#define bcmPktDma_EthTxEnable             bcmPktDma_EthTxEnable_Iudma
#define bcmPktDma_EthTxDisable            bcmPktDma_EthTxDisable_Iudma
#define bcmPktDma_EthRxEnable             bcmPktDma_EthRxEnable_Iudma
#define bcmPktDma_EthRxDisable            bcmPktDma_EthRxDisable_Iudma
#define bcmPktDma_EthFreeXmitBufGet       bcmPktDma_EthFreeXmitBufGet_Iudma
#define bcmPktDma_EthCheckRecvNoBds       bcmPktDma_EthCheckRecvNoBds_Iudma
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_EthRecvBufGet           bcmPktDma_EthRecvBufGet_Iudma
#endif

#define bcmPktDma_EthSetPhyRate(_port, _enable, _kbps, _isWanPort)
#define bcmPktDma_EthSetPauseEn(_port, _enable)
#define bcmPktDma_EthGetStats             bcmPktDma_EthGetStats_Iudma
#define bcmPktDma_EthResetStats           bcmPktDma_EthResetStats_Iudma

/* XTM versions of the interface */
#define bcmPktDma_XtmInitRxChan           bcmPktDma_XtmInitRxChan_Iudma
#define bcmPktDma_XtmUnInitRxChan		  bcmPktDma_XtmUnInitRxChan_Iudma
#define bcmPktDma_XtmInitTxChan           bcmPktDma_XtmInitTxChan_Iudma
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define bcmPktDma_XtmSetIqThresh         bcmPktDma_XtmSetIqThresh_Iudma
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_XtmSetRxChanBpmThresh  bcmPktDma_XtmSetRxChanBpmThresh_Iudma
#define bcmPktDma_XtmSetTxChanBpmThresh  bcmPktDma_XtmSetTxChanBpmThresh_Iudma
#endif
#define bcmPktDma_XtmSetTxChanDropAlg     bcmPktDma_XtmSetTxChanDropAlg_Iudma
#define bcmPktDma_XtmSelectRxIrq          bcmPktDma_XtmSelectRxIrq_Iudma
#define bcmPktDma_XtmClrRxIrq             bcmPktDma_XtmClrRxIrq_Iudma
#define bcmPktDma_XtmRecv_RingSize        bcmPktDma_XtmRecv_RingSize_Iudma
#define bcmPktDma_XtmRecv                 bcmPktDma_XtmRecv_Iudma
#define bcmPktDma_XtmFreeRecvBuf          bcmPktDma_XtmFreeRecvBuf_Iudma
#define bcmPktDma_XtmFreeXmitBuf          bcmPktDma_XtmFreeXmitBuf_Iudma
#define bcmPktDma_XtmXmitAvailable(p1, unused)      \
    bcmPktDma_XtmXmitAvailable_Iudma(p1)
#define bcmPktDma_XtmXmit(p1,p2,p3,p4,p5,p6,p7,p8,p9,unused1,unused2)   \
    bcmPktDma_XtmXmit_Iudma(p1,p2,p3,p4,p5,p6,p7,p8,p9);
#define bcmPktDma_XtmXmitSwdmaToIudma     bcmPktDma_XtmXmit_Swdma_to_Iudma
#define bcmPktDma_XtmTxEnable             bcmPktDma_XtmTxEnable_Iudma
#define bcmPktDma_XtmTxDisable            bcmPktDma_XtmTxDisable_Iudma
#define bcmPktDma_XtmRxEnable             bcmPktDma_XtmRxEnable_Iudma
#define bcmPktDma_XtmFreeXmitBufGet       bcmPktDma_XtmFreeXmitBufGet_Iudma
#define bcmPktDma_XtmForceFreeXmitBufGet  bcmPktDma_XtmForceFreeXmitBufGet_Iudma
#define bcmPktDma_XtmRxDisable            bcmPktDma_XtmRxDisable_Iudma
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_XtmRecvBufGet           bcmPktDma_XtmRecvBufGet_Iudma
#endif

#define bcmPktDma_XtmCreateDevice(_devId, _encapType, _headerLen, _trailerLen)
#define bcmPktDma_XtmLinkUp(_devId, _matchId)
#define bcmPktDma_XtmGetStats             bcmPktDma_XtmGetStats_Iudma
#define bcmPktDma_XtmResetStats           bcmPktDma_XtmResetStats_Iudma

#else /* FAP is compiled in */

/* The BCM96362 chip uses the Dqm version of the bcmPkt interface */
/* Eth versions of the interface */
#define bcmPktDma_EthInitRxChan           bcmPktDma_EthInitRxChan_Dqm
#define bcmPktDma_EthInitTxChan           bcmPktDma_EthInitTxChan_Dqm
#if defined(CONFIG_BCM_GMAC)
#define bcmPktDma_EthUnInitRxChan         bcmPktDma_EthUnInitRxChan_Dqm
#define bcmPktDma_EthUnInitTxChan         bcmPktDma_EthUnInitTxChan_Dqm
#endif

#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
#define bcmPktDma_EthInitExtSw            bcmPktDma_EthInitExtSw_Dqm
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define bcmPktDma_EthSetIqThresh          bcmPktDma_EthSetIqThresh_Dqm
#define bcmPktDma_EthSetIqDqmThresh       bcmPktDma_EthSetIqDqmThresh_Dqm
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_EthSetRxChanBpmThresh   bcmPktDma_EthSetRxChanBpmThresh_Dqm
#define bcmPktDma_EthSetTxChanBpmThresh   bcmPktDma_EthSetTxChanBpmThresh_Dqm
#define bcmPktDma_EthXmitBufCountGet      bcmPktDma_EthXmitBufCountGet_Dqm
#endif
#define bcmPktDma_EthSelectRxIrq          bcmPktDma_EthSelectRxIrq_Dqm
#define bcmPktDma_EthClrRxIrq             bcmPktDma_EthClrRxIrq_Dqm
#define bcmPktDma_EthRecvAvailable        bcmPktDma_EthRecvAvailable_Dqm
#define bcmPktDma_EthRecv                 bcmPktDma_EthRecv_Dqm
#define bcmPktDma_EthFreeRecvBuf          bcmPktDma_EthFreeRecvBuf_Dqm
#define bcmPktDma_EthXmitAvailable        bcmPktDma_EthXmitAvailable_Dqm
#define bcmPktDma_EthXmit                 bcmPktDma_EthXmitNoCheck_Dqm
#define bcmPktDma_EthTxEnable             bcmPktDma_EthTxEnable_Dqm
#define bcmPktDma_EthTxDisable            bcmPktDma_EthTxDisable_Dqm
#define bcmPktDma_EthRxEnable             bcmPktDma_EthRxEnable_Dqm
#define bcmPktDma_EthRxDisable            bcmPktDma_EthRxDisable_Dqm
#define bcmPktDma_EthFreeXmitBufGet       bcmPktDma_EthFreeXmitBufGet_Dqm
#define bcmPktDma_EthCheckRecvNoBds       bcmPktDma_EthCheckRecvNoBds_Dqm
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_EthRecvBufGet           bcmPktDma_EthRecvBufGet_Dqm
#endif
#define bcmPktDma_EthSetPhyRate           bcmPktDma_EthSetPhyRate_Dqm
#define bcmPktDma_EthSetPauseEn           bcmPktDma_EthSetPauseEn_Dqm
#define bcmPktDma_EthGetStats             bcmPktDma_EthGetStats_Dqm
#define bcmPktDma_EthResetStats           bcmPktDma_EthResetStats_Dqm

/* XTM versions of the interface */
#define bcmPktDma_XtmInitRxChan           bcmPktDma_XtmInitRxChan_Dqm
#define bcmPktDma_XtmUnInitRxChan		  bcmPktDma_XtmUnInitRxChan_Dqm
#define bcmPktDma_XtmInitTxChan           bcmPktDma_XtmInitTxChan_Dqm
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define bcmPktDma_XtmSetIqThresh          bcmPktDma_XtmSetIqThresh_Dqm
#define bcmPktDma_XtmSetIqDqmThresh       bcmPktDma_XtmSetIqDqmThresh_Dqm
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_XtmSetRxChanBpmThresh   bcmPktDma_XtmSetRxChanBpmThresh_Dqm
#define bcmPktDma_XtmSetTxChanBpmThresh   bcmPktDma_XtmSetTxChanBpmThresh_Dqm
#endif
#define bcmPktDma_XtmSetTxChanDropAlg     bcmPktDma_XtmSetTxChanDropAlg_Dqm
#define bcmPktDma_XtmSelectRxIrq          bcmPktDma_XtmSelectRxIrq_Dqm
#define bcmPktDma_XtmClrRxIrq             bcmPktDma_XtmClrRxIrq_Dqm
#define bcmPktDma_XtmRecvAvailable        bcmPktDma_XtmRecvAvailable_Dqm
#define bcmPktDma_XtmRecv_RingSize        bcmPktDma_XtmRecv_RingSize_Dqm
#define bcmPktDma_XtmRecv                 bcmPktDma_XtmRecv_Dqm
#define bcmPktDma_XtmFreeRecvBuf          bcmPktDma_XtmFreeRecvBuf_Dqm
#define bcmPktDma_XtmFreeXmitBuf          bcmPktDma_XtmFreeXmitBuf_Dqm
#define bcmPktDma_XtmXmitAvailable        bcmPktDma_XtmXmitAvailable_Dqm
#define bcmPktDma_XtmXmit                 bcmPktDma_XtmXmit_Dqm
#define bcmPktDma_XtmTxEnable             bcmPktDma_XtmTxEnable_Dqm
#define bcmPktDma_XtmTxDisable            bcmPktDma_XtmTxDisable_Dqm
#define bcmPktDma_XtmRxEnable             bcmPktDma_XtmRxEnable_Dqm
#define bcmPktDma_XtmRxDisable            bcmPktDma_XtmRxDisable_Dqm
#define bcmPktDma_XtmFreeXmitBufGet       bcmPktDma_XtmFreeXmitBufGet_Dqm
#define bcmPktDma_XtmForceFreeXmitBufGet  bcmPktDma_XtmFreeXmitBufGet_Dqm
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define bcmPktDma_XtmRecvBufGet           bcmPktDma_XtmRecvBufGet_Dqm
#endif

#define bcmPktDma_XtmCreateDevice(_devId, _encapType, _headerLen, _trailerLen)  \
    bcmPktDma_XtmCreateDevice_Dqm(_devId, _encapType, _headerLen, _trailerLen)

#define bcmPktDma_XtmLinkUp(_devId, _matchId)   \
    bcmPktDma_XtmLinkUp_Dqm(_devId, _matchId)

#define bcmPktDma_XtmGetStats             bcmPktDma_XtmGetStats_Dqm
#define bcmPktDma_XtmResetStats           bcmPktDma_XtmResetStats_Dqm
#endif

#define MAX_ETH_RX_CHANNELS               2
#define MAX_ETH_TX_CHANNELS               2

/* enable CPU usage profiling */
//#define FAP4KE_IUDMA_PMON_ENABLE

#if defined(FAP4KE_IUDMA_PMON_ENABLE)
#define FAP4KE_IUDMA_PMON_DECLARE() FAP4KE_PMON_DECLARE()
#define FAP4KE_IUDMA_PMON_BEGIN(_pmonId) FAP4KE_PMON_BEGIN(_pmonId)
#define FAP4KE_IUDMA_PMON_END(_pmonId) FAP4KE_PMON_END(_pmonId)
#else
#define FAP4KE_IUDMA_PMON_DECLARE()
#define FAP4KE_IUDMA_PMON_BEGIN(_pmonId)
#define FAP4KE_IUDMA_PMON_END(_pmonId)
#endif

#define BCM_PKTDMA_SUCCESS 0
#define BCM_PKTDMA_ERROR   -1

#define BCM_PKTDMA_PBUF_FROM_BD(_dmaDesc_p)                     \
    ( (unsigned char *)(phys_to_virt((_dmaDesc_p)->address)) )

#define BCM_PKTDMA_LEN_FROM_BD(_dmaDesc_p)                     \
    ( (unsigned int)((_dmaDesc_p)->length) )

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) || defined (FAP_4KE) )
  /* Do not include brackets around these 4 defines to avoid incorrect address calculations - Aug 2010 */
#define g_pEthRxDma &p4keDspram->global.EthRxDma
#define g_pEthTxDma &p4keDspram->global.EthTxDma
#define g_pXtmRxDma &p4kePsm->global.XtmRxDma
#define g_pXtmTxDma &p4kePsm->global.XtmTxDma
#endif  /* if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) || defined (FAP_4KE) ) */

/*
 * Inline functions
 */

/* NOTE: These only work on the FAP side */
#define BCM_PKTDMA_CHANNEL_TO_RXDMA(_channel) ( g_pEthRxDma[(_channel)] )
#define BCM_PKTDMA_CHANNEL_TO_RXDMA_XTM(_channel) ( g_pXtmRxDma[(_channel)] )


/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecvAvailableGet_Iudma
 Purpose: Return 1 if a packet is available, 0 otherwise
-------------------------------------------------------------------------- */
#if !defined(CONFIG_BCM947189)
static inline int __bcmPktDma_EthRecvAvailableGet_Iudma(BcmPktDma_LocalEthRxDma *rxdma,
                                                        DmaDesc *dmaDesc_p)
{
    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

    dmaDesc_p->word0 = rxdma->rxBds[rxdma->rxHeadIndex].word0;

    {
        if( rxdma->rxAssignedBds && ((dmaDesc_p->status & DMA_OWN) == 0) )
        {
            dmaDesc_p->address = rxdma->rxBds[rxdma->rxHeadIndex].address;
            FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);
            return 1;
        }
        else
        {
            /* FIXME: Added this for compiler warning... */
            dmaDesc_p->address = 0;
        }
    }

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);
    return 0;
}
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecvAvailableGet_Iudma
 Purpose: Return 1 if a packet is available, 0 otherwise
-------------------------------------------------------------------------- */
static inline int __bcmPktDma_XtmRecvAvailableGet_Iudma(BcmPktDma_LocalXtmRxDma *rxdma,
                                                        DmaDesc *dmaDesc_p)
{
    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

    dmaDesc_p->word0 = rxdma->rxBds[rxdma->rxHeadIndex].word0;

    if( rxdma->rxAssignedBds && ((dmaDesc_p->status & DMA_OWN) == 0) )
    {
        dmaDesc_p->address = rxdma->rxBds[rxdma->rxHeadIndex].address;
        //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);
        return 1;
    }
    else
    {
        /* FIXME: Added this for compiler warning... */
        dmaDesc_p->address = 0;
    }

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);
    return 0;
}
#endif /* CONFIG_BCM_XTMCFG */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecvAvailable
 Purpose: Return 1 if a packet is available, 0 otherwise
-------------------------------------------------------------------------- */
#if !defined(CONFIG_BCM947189)
static inline int __bcmPktDma_EthRecvAvailable_Iudma(BcmPktDma_LocalEthRxDma *rxdma)
{
    if( rxdma->rxAssignedBds &&
        !(rxdma->rxBds[rxdma->rxHeadIndex].status & DMA_OWN) )
        return 1;

    return 0;

}
static inline int bcmPktDma_EthRecvAvailable_Iudma(BcmPktDma_LocalEthRxDma *rxdma)
{
    return __bcmPktDma_EthRecvAvailable_Iudma(rxdma);
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecvAvailable
 Purpose: Return 1 if a packet is available, 0 otherwise
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline int bcmPktDma_EthRecvAvailable_Dqm(BcmPktDma_LocalEthRxDma *rxdma)
{
    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel: %d", rxdma->channel);
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    if((bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx, DQM_FAP2HOST_ETH_RX_Q_LOW)) 
		||(bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx, DQM_FAP2HOST_ETH_RX_Q_HI)))
		return 1;
	else
		return 0;
#else
    return (bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx, DQM_FAP2HOST_ETH_RX_Q_LOW));
#endif
}
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: __bcmPktDma_XtmRecvAvailable
 Purpose: Return 1 if a packet is available, 0 otherwise
-------------------------------------------------------------------------- */
static inline int __bcmPktDma_XtmRecvAvailable_Iudma(BcmPktDma_LocalXtmRxDma *rxdma)
{

    if( rxdma->rxAssignedBds &&
        !(rxdma->rxBds[rxdma->rxHeadIndex].status & DMA_OWN) )
        return 1;

    return 0;

    //return ((rxdma->pBdHead->status & DMA_OWN) ? 0 : 1);

}
#endif /* CONFIG_BCM_XTMCFG */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecvNoCheck_Iudma
 Purpose: Receive a packet on a specific channel, without checking if
          packets are available. Must be used in conjunction with
          bcmPktDma_EthRecvAvailableGet_Iudma().
-------------------------------------------------------------------------- */
static inline int __bcmPktDma_EthRecvNoCheck_Iudma(BcmPktDma_LocalEthRxDma *rxdma)
{
    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

#if defined(CC_BCM_PKTDMA_DEBUG)
    if(rxdma->rxAssignedBds == 0)
    {
        return BCM_PKTDMA_ERROR;
    }
#endif

    /* If no more rx packets, we are done for this channel */
    {
        /* Wrap around the rxHeadIndex */
        if (++rxdma->rxHeadIndex == rxdma->numRxBds)
        {
            rxdma->rxHeadIndex = 0;
        }
        rxdma->rxAssignedBds--;
    }

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);

    return BCM_PKTDMA_SUCCESS;
}


/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecvNoCheck_Iudma
 Purpose: Receive a packet on a specific channel, without checking if
          packets are available. Must be used in conjunction with
          bcmPktDma_XtmRecvAvailableGet_Iudma().
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
static inline int __bcmPktDma_XtmRecvNoCheck_Iudma(BcmPktDma_LocalXtmRxDma *rxdma)
{
    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

#if defined(CC_BCM_PKTDMA_DEBUG)
    if(rxdma->rxAssignedBds == 0)
    {
        return BCM_PKTDMA_ERROR;
    }
#endif

    /* Wrap around the rxHeadIndex */
    if (++rxdma->rxHeadIndex == rxdma->numRxBds)
        rxdma->rxHeadIndex = 0;

    rxdma->rxAssignedBds--;

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);

    return BCM_PKTDMA_SUCCESS;
}
#endif /* CONFIG_BCM_XTMCFG */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthClrRxIrq
 Purpose: Clear the Rx interrupt for a specific channel
-------------------------------------------------------------------------- */
static inline void __bcmPktDma_EthClrRxIrq_Iudma(BcmPktDma_LocalEthRxDma *rxdma)
{
    /* Move rxDma ptr to local context */
#if defined(CONFIG_BCM947189)
    rxdma->miscReg->intmask = 0; // disable interrupt
    rxdma->miscReg->intstatus = I_RI; // clr interrupt
#else 
    rxdma->rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;  // clr interrupts
#endif
}
static inline void bcmPktDma_EthClrRxIrq_Iudma(BcmPktDma_LocalEthRxDma *rxdma)
{
    __bcmPktDma_EthClrRxIrq_Iudma(rxdma);
}

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmClrRxIrq
 Purpose: Clear the Rx interrupt for a specific channel
-------------------------------------------------------------------------- */
static inline void __bcmPktDma_XtmClrRxIrq_Iudma(BcmPktDma_LocalXtmRxDma *rxdma)
{
    rxdma->rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
}

static inline void bcmPktDma_XtmClrRxIrq_Iudma(BcmPktDma_LocalXtmRxDma *rxdma)
{
    __bcmPktDma_XtmClrRxIrq_Iudma(rxdma);
}

#endif /* CONFIG_BCM_XTMCFG */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufGet
 Purpose: Gets a TX buffer to free by caller
-------------------------------------------------------------------------- */
static inline BcmPktDma_txRecycle_t *bcmPktDma_EthFreeXmitBufGet_Iudma(BcmPktDma_LocalEthTxDma *txdma,
                                                                       BcmPktDma_txRecycle_t *unused_p)
{
    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    /* Reclaim transmitted buffers */
    if(txdma->txFreeBds < txdma->numTxBds)
    {
        int bdIndex = txdma->txHeadIndex;

#if defined(CONFIG_BCM947189)
        uint32 dma_status = txdma->txDma->status0;
        uint32 bd_offset = ((txdma->txHeadIndex * sizeof(DmaDesc)) + 
                              (uint32)(txdma->txBdsPhysBase)) & DMA_CURRENT_DESCR;
        /*
         * Check if there are recyclable buffers.
         * For 47189 that means checking that the BD pointed by txHeadIndex is:
         *
         * a) Before the current processed BD (status0 & DMA_CURRENT_DESCR)
         *    if the DMA hasn't wrapped yet.
         * b) After the current processed BD if the DMA has wrapped but the
         *    txHeadIndex didn't.
         */
        if (bd_offset != (dma_status & DMA_CURRENT_DESCR)) 
#else
        if(!(txdma->txBds[bdIndex].status & DMA_OWN))
#endif
        {
            if (++txdma->txHeadIndex == txdma->numTxBds)
            {
                txdma->txHeadIndex = 0;
            }

            txdma->txFreeBds++;

            return (&txdma->txRecycle[bdIndex]);
        }
    }

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    return NULL;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufGetRecycle_Iudma
 Purpose: Gets a TX buffer to free by caller, but does not remove it from Ring
-------------------------------------------------------------------------- */
#if !defined(CONFIG_BCM947189)
static inline BcmPktDma_txRecycle_t *bcmPktDma_EthFreeXmitBufGetRecycle_Iudma(BcmPktDma_LocalEthTxDma *txdma)
{
    if(txdma->txFreeBds < txdma->numTxBds)
    {
        int bdIndex = txdma->txHeadIndex;

        if(!(txdma->txBds[bdIndex].status & DMA_OWN))
        {
            return (&txdma->txRecycle[bdIndex]);
        }
    }

    return NULL;
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufNoCheck_Iudma
 Purpose: Frees the next TX buffer
-------------------------------------------------------------------------- */
static inline void bcmPktDma_EthFreeXmitBufNoCheck_Iudma(BcmPktDma_LocalEthTxDma *txdma)
{
    if (++txdma->txHeadIndex == txdma->numTxBds)
    {
        txdma->txHeadIndex = 0;
    }

    txdma->txFreeBds++;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufGet_Dqm
 Purpose: Returns a pointer to the transmit recycle information if free BDs
          are available in the specified BD Ring. Otherwise NULL is returned
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline BcmPktDma_txRecycle_t *bcmPktDma_EthFreeXmitBufGet_Dqm(BcmPktDma_LocalEthTxDma *txdma,
                                                                     BcmPktDma_txRecycle_t *txRecycle_p)
{
#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        return bcmPktDma_EthFreeXmitBufGet_Iudma(txdma, NULL);
    }
#endif

    /* Reclaim transmitted buffers for any queue */
    if(bcmPktDma_isDqmRecvAvailableHost(txdma->fapIdx, DQM_FAP2HOST_ETH_FREE_TXBUF_Q))
    {
        DQMQueueDataReg_S msg;

        bcmPktDma_dqmRecvMsgHost(txdma->fapIdx, DQM_FAP2HOST_ETH_FREE_TXBUF_Q,
                                 DQM_FAP2HOST_ETH_FREE_TXBUF_Q_SIZE, &msg);

        txRecycle_p->key = (uint32) msg.word0;

        return txRecycle_p;
    }

    return NULL;
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufGet
    Purpose: Same as bcmPktDma_EthFreeXmitBufGet_Iudma, with the exeption
             that it does not check wether the Tx buffer has been assigned
             to the CPU. It assumes that the caller has verified this before
             calling it.
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline BcmPktDma_txRecycle_t *bcmPktDma_EthFreeXmitBufGetNoCheck_Iudma(BcmPktDma_LocalEthTxDma *txdma)
{
    int bdIndex;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    bdIndex = txdma->txHeadIndex;

    if (++txdma->txHeadIndex == txdma->numTxBds)
    {
        txdma->txHeadIndex = 0;
    }

    txdma->txFreeBds++;

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    return (&txdma->txRecycle[bdIndex]);
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeXmitBufCount_Iudma
    Purpose: Returns the total number of Tx Buffers currently assigned to
             the CPU, i.e., ready to be recycled.
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline uint32 bcmPktDma_EthFreeXmitBufCount_Iudma(BcmPktDma_LocalEthTxDma *txdma)
{
#if 0
    uint32 txHeadIndex;
    uint32 ringOffset;
    uint32 txQueueDepth;

//    if(!txdma->txEnabled) return 0;

    txHeadIndex = txdma->txHeadIndex;

#if defined(CONFIG_BCM_GMAC)
#define GMAC_TXDMA_BASE (GMAC_DMA_BASE + 0x210)
    if (txdma->txDma == (volatile DmaChannelCfg *) GMAC_TXDMA_BASE)
        ringOffset = GMAC_DMA->stram.s[(GMAC_PHY_CHAN * 2) + 1].state_data;
    else
#endif
        ringOffset = SW_DMA->stram.s[(txdma->channel * 2) + 1].state_data;

    ringOffset &= 0x1FFF;

    if(ringOffset == txHeadIndex)
    {
        if(txdma->txFreeBds == 0)
        {
            txQueueDepth = txdma->numTxBds - 1;
        }
        else
        {
            txQueueDepth = 0;
        }
    }
    else if(ringOffset > txHeadIndex)
    {
        txQueueDepth = ringOffset - txHeadIndex - 1;
    }
    else /* (ringOffset < txHeadIndex) */
    {
        /* wraparound */
        txQueueDepth = txdma->numTxBds - txHeadIndex + ringOffset - 1;
    }
    return txQueueDepth;
#else
    uint32 txHeadIndex;
    uint32 txTailIndex;
    uint32 txQueueDepth;

    txHeadIndex = txdma->txHeadIndex;
    txTailIndex = txdma->txTailIndex;

    if (txTailIndex >= txHeadIndex) 
        txQueueDepth = txTailIndex - txHeadIndex; 
    else
        txQueueDepth = txdma->numTxBds + txTailIndex - txHeadIndex; 

    return txQueueDepth;
#endif
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmFreeXmitBufGet
 Purpose: Gets a TX buffer to free by caller
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
static inline BOOL bcmPktDma_XtmFreeXmitBufGet_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint32 *pKey,
                                                     uint32 *pTxSource, uint32 *pTxAddr,
                                                     uint32 *pRxChannel,
                                                     uint32 dmaType,
                                                     uint32 noGlobalBufAccount)
{
    BOOL ret = FALSE;
    int  bdIndex;

    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    bdIndex = txdma->txHeadIndex;
    *pKey = 0;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* TxSource & TxAddr not required in non-FAP applications */
    *pTxSource = 0;
    *pTxAddr   = 0;
    *pRxChannel = 0;
#endif

    /* Reclaim transmitted buffers */
    if (txdma->txFreeBds < txdma->ulQueueSize)
    {
        if (!(txdma->txBds[bdIndex].status & DMA_OWN))
        {
           *pKey = txdma->txRecycle[bdIndex].key;
           txdma->txBds[bdIndex].status &=  ~DMA_OWN ; /* Clear the DMA_OWN flag, as this is force free */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
           *pTxSource = txdma->txRecycle[bdIndex].source;
           *pTxAddr = txdma->txRecycle[bdIndex].address;
           *pRxChannel = txdma->txRecycle[bdIndex].rxChannel;
#endif

           if (++txdma->txHeadIndex == txdma->ulQueueSize)
               txdma->txHeadIndex = 0;

           txdma->txFreeBds++;
           txdma->ulNumTxBufsQdOne--;
#if !defined(CONFIG_BCM963268)
           // FIXME - Which chip uses more then one TX queue?
           if (!noGlobalBufAccount)
           g_pXtmGlobalInfo->ulNumTxBufsQdAll--;
#endif
           ret = TRUE;
        }
    }

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET);

    return ret;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmForceFreeXmitBufGet
 Purpose: Gets a TX buffer to free by caller, ignoring DMA_OWN status
-------------------------------------------------------------------------- */
static inline BOOL bcmPktDma_XtmForceFreeXmitBufGet_Iudma(BcmPktDma_LocalXtmTxDma * txdma, uint32 *pKey,
                                            uint32 *pTxSource, uint32 *pTxAddr,
                                            uint32 *pRxChannel, uint32 dmaType,
                                            uint32 noGlobalBufAccount)
{
    BOOL ret = FALSE;
    int  bdIndex;

    bdIndex = txdma->txHeadIndex;
    *pKey   = 0;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* TxSource & TxAddr not required in non-FAP applications */
    *pTxSource  = 0;
    *pTxAddr    = 0;
    *pRxChannel = 0;
#endif

    /* Reclaim transmitted buffers */
    if (txdma->txFreeBds < txdma->ulQueueSize)
    {
#if 0 //for debug
        if (txdma->txBds[bdIndex].status & DMA_OWN)
        {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
            fap4kePrt_Error("Force free bdIndex %d\n", bdIndex);
#else
            printk("Force free bdIndex %d\n", bdIndex);            
#endif
        }
#endif        
        txdma->txBds[bdIndex].status &=  ~DMA_OWN ; /* Clear the DMA_OWN flag, as this is force free */
        *pKey = txdma->txRecycle[bdIndex].key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        *pTxSource = txdma->txRecycle[bdIndex].source;
        *pTxAddr = txdma->txRecycle[bdIndex].address;
        *pRxChannel = txdma->txRecycle[bdIndex].rxChannel;
#endif

        if (++txdma->txHeadIndex == txdma->ulQueueSize)
            txdma->txHeadIndex = 0;

        txdma->txFreeBds++;
        txdma->ulNumTxBufsQdOne--;
#if !defined(CONFIG_BCM963268)
        // FIXME - Which chip uses more then one TX queue?
        if (!noGlobalBufAccount)
            g_pXtmGlobalInfo->ulNumTxBufsQdAll--;
#endif

        ret = TRUE;
    }
    else {
       txdma->txBds[bdIndex].status &=  ~DMA_OWN ; /* Clear the DMA_OWN flag, as this is force free */
       if (dmaType == XTM_HW_DMA)
          txdma->txHeadIndex = (txdma->txStateRam->state_data & 0x1fff) ;
       txdma->txTailIndex = txdma->txHeadIndex ;
    }

    return ret;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmFreeXmitBufGetRecycle_Iudma
 Purpose: Gets a TX buffer to free by caller, but does not remove it from Ring
-------------------------------------------------------------------------- */
static inline BOOL bcmPktDma_XtmFreeXmitBufGetRecycle_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint32 *pKey,
                                                     uint32 *pTxSource, uint32 *pTxAddr,
                                                     uint32 *pRxChannel,
                                                     uint32 dmaType,
                                                     uint32 noGlobalBufAccount)
{
    BOOL ret = FALSE;
    int  bdIndex;

    bdIndex = txdma->txHeadIndex;
    *pKey = 0;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* TxSource & TxAddr not required in non-FAP applications */
    *pTxSource = 0;
    *pTxAddr   = 0;
    *pRxChannel = 0;
#endif

    if(txdma->txFreeBds < txdma->ulQueueSize)
    {
        if(!(txdma->txBds[bdIndex].status & DMA_OWN))
        {
           *pKey = txdma->txRecycle[bdIndex].key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
           *pTxSource = txdma->txRecycle[bdIndex].source;
           *pTxAddr = txdma->txRecycle[bdIndex].address;
           *pRxChannel = txdma->txRecycle[bdIndex].rxChannel;
#endif
            ret = TRUE ;
        }
    }

    return ret;
}
#endif /* CONFIG_BCM_XTMCFG */


/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeRecvBuf
 Purpose: Free a single RX buffer
-------------------------------------------------------------------------- */
static inline int bcmPktDma_EthFreeRecvBuf_Iudma(BcmPktDma_LocalEthRxDma * rxdma, unsigned char * pBuf)
{
    volatile DmaDesc * rxBd;
#if !defined(CONFIG_BCM947189)
    DmaDesc            dmaDesc;
#endif
    int                tail_idx;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    if(rxdma->rxAssignedBds == rxdma->numRxBds)
    {
        BCM_PKTDMA_LOG_ERROR("rxAssignedBds(%d) == numRxBds(%d)", rxdma->rxAssignedBds, rxdma->numRxBds);
        return BCM_PKTDMA_ERROR;
    }

    tail_idx = rxdma->rxTailIndex;

#if defined(CONFIG_BCM947189)
    if (rxdma->channel_init == 0)
    {
        rxBd = &rxdma->rxBds[tail_idx];
        /* All BDs point to buffers of BCM_MAX_PKT_LEN */
        rxBd->ctrl2 = BCM_MAX_PKT_LEN;
        if (tail_idx == (rxdma->numRxBds - 1)) {
            /* Last BD in the ring, set the End Of Table bit in it */
            rxBd->ctrl1 = DMA_CTRL1_SOF | DMA_CTRL1_EOT;
            rxdma->rxTailIndex = 0;
        } else {
            rxBd->ctrl1 = DMA_CTRL1_SOF;
            rxdma->rxTailIndex = tail_idx + 1;
        }
    }
    else
    {
        if (tail_idx == (rxdma->numRxBds - 1)) {
            tail_idx = 0;
            rxdma->rxDma->ptr = (unsigned int)(rxdma->rxBdsPhysBase);
        } else {
            tail_idx++;
            rxdma->rxDma->ptr += sizeof(DmaDesc);
        }
        rxBd = &rxdma->rxBds[tail_idx];
        rxdma->rxTailIndex = tail_idx;
    }
    rxdma->rxAssignedBds++;

    /* Assign the buffer to the BD */
    rxBd->addrhigh = 0;
    rxBd->addrlow = (uint32)VIRT_TO_PHYS(pBuf - DMA_DATA_OFFSET);
#else
    rxBd = &rxdma->rxBds[tail_idx];

    /* assign packet, prepare descriptor, and advance pointer */
    dmaDesc.length = BCM_MAX_PKT_LEN;

    if (tail_idx == (rxdma->numRxBds - 1)) {
        dmaDesc.status = DMA_OWN | DMA_WRAP;
        rxdma->rxTailIndex = 0;
    } else {
        dmaDesc.status = DMA_OWN;
        rxdma->rxTailIndex = tail_idx + 1;
    }

    rxBd->address = (uint32)VIRT_TO_PHYS(pBuf);
    rxBd->word0 = dmaDesc.word0;

    rxdma->rxAssignedBds++;
    if(rxdma->rxEnabled)
    {   /* Do not reenable if Host MIPs has shut the interface down */
        rxdma->rxDma->cfg = DMA_ENABLE;
    }
#endif

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    return BCM_PKTDMA_SUCCESS;
}

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_RecycleBufToFap
 Purpose: Send a single RX buffer to the FAP to be freed
   Notes: In DQM msg, word0 = channel; word1 = pBuf
-------------------------------------------------------------------------- */
static inline void bcmPktDma_RecycleBufToFap(uint32 fapIdx, 
                                             int channel, 
                                             unsigned char *pBuf,
                                             uint32 dqmId,
                                             uint32 dqmSize)
{
    DQMQueueDataReg_S msg;

    /* Wait until there is space in the ETH_FREE_RXBUF_Q,
       then send the free request to the FAP */

    while(!bcmPktDma_isDqmXmitAvailableHost(fapIdx, DQM_HOST2FAP_ETH_FREE_RXBUF_Q));

    msg.word0 = (uint32) channel;
    msg.word1 = (uint32) pBuf;
#ifndef FAP_4KE
//printk("channel %d fapIdx %d pBuf %p\n", channel, (int)fapIdx, pBuf);
#endif
    bcmPktDma_dqmXmitMsgHost(fapIdx, dqmId, dqmSize, &msg);
}


/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeRecvBuf
 Purpose: Send a single RX buffer to the FAP to be freed
   Notes: In DQM msg, word0 = channel; word1 = pBuf
-------------------------------------------------------------------------- */
static inline void bcmPktDma_EthFreeRecvBuf_Dqm(BcmPktDma_LocalEthRxDma * rxdma,
                                                unsigned char * pBuf)
{
    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel: %d", rxdma->channel);


    bcmPktDma_RecycleBufToFap(rxdma->fapIdx, rxdma->channel, pBuf,
                              DQM_HOST2FAP_ETH_FREE_RXBUF_Q,
                              DQM_HOST2FAP_ETH_FREE_RXBUF_Q_SIZE);
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmFreeRecvBuf
 Purpose: Free a single RX buffer
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
static inline int bcmPktDma_XtmFreeRecvBuf_Iudma(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char *pBuf)
{
    volatile DmaDesc        *rxBd;
    DmaDesc                  dmaDesc;
    int                      tail_idx;

    //printk("bcmPktDma_XtmFreeRecvBuf_Iudma ch: %d pBuf: %p\n", rxdma->channel, pBuf);

    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    tail_idx = rxdma->rxTailIndex;
    rxBd = &rxdma->rxBds[tail_idx];

    /* assign packet, prepare descriptor, and advance pointer */
    dmaDesc.length = BCM_MAX_PKT_LEN;

    if (tail_idx == (rxdma->numRxBds - 1)) {
        dmaDesc.status = DMA_OWN | DMA_WRAP;
        rxdma->rxTailIndex = 0;
    } else {
        dmaDesc.status = DMA_OWN;
        rxdma->rxTailIndex = tail_idx + 1;
    }

    rxBd->address = (uint32)VIRT_TO_PHYS(pBuf);
    rxBd->word0 = dmaDesc.word0;

    rxdma->rxAssignedBds++;

    if(rxdma->rxEnabled)
    {   /* Do not reenable if Host MIPs has shut the interface down */
        rxdma->rxDma->cfg = DMA_ENABLE;
    }

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    return BCM_PKTDMA_SUCCESS;
}
#endif /* CONFIG_BCM_XTMCFG */

#ifdef FAP_4KE
#include "fap4ke_printer.h"
//#define ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
#define BCM_PKTDMA_TX_DEBUG fap4kePrt_Print
#else
#define BCM_PKTDMA_TX_DEBUG BCM_ENET_TX_DEBUG
#endif
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmitNoCheck
 Purpose: Xmit an NBuff; not necessary to check for space available.
        : Already done by call to bcmPktDma_EthXmitAvailable.
   Notes: param1 = gemid;   param2 = port_id; param3 = egress_queue
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

static inline void bcmPktDma_EthXmitNoCheck_Iudma(BcmPktDma_LocalEthTxDma *txdma, uint8 *pBuf, uint16 len,
#if defined(CONFIG_BCM947189)
                                                  int bufSource, uint32 dmaStatus, unsigned long key,
#else
                                                  int bufSource, uint16 dmaStatus, unsigned long key,
#endif
                                                  int param1, int param2)
{
    int txIndex;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_XMIT);

    txIndex = txdma->txTailIndex;


    {
        BcmPktDma_txRecycle_t *txRecycle_p = &txdma->txRecycle[txIndex];
        txRecycle_p->key = key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        txRecycle_p->source = bufSource;
        txRecycle_p->address = (uint32)pBuf;
        txRecycle_p->rxChannel = param1;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_PKTDMA_TX_DEBUG("Tx BD: rxChannel: 0%d \n", param1);
#endif
#endif
    }

    /* Decrement total BD count */
    txdma->txFreeBds--;

    /* advance BD pointer to next in the chain. */
    if (txIndex == (txdma->numTxBds - 1))
    {
#if defined(CONFIG_BCM947189)
        dmaStatus |= DMA_CTRL1_EOT;
#else
        dmaStatus |= DMA_WRAP;
#endif
        txdma->txTailIndex = 0;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_PKTDMA_TX_DEBUG("Tx BD: dma_status: 0x%04x \n", dmaStatus);
#endif
    }
    else
    {
        txdma->txTailIndex++;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_PKTDMA_TX_DEBUG("Tx BD: dma_status: 0x%04x \n", dmaStatus);
#endif
    }

    {
        volatile DmaDesc *txBd;
#if defined(CONFIG_BCM947189)
        txBd = &txdma->txBds[txIndex];
        txBd->addrlow = (uint32)VIRT_TO_PHYS(pBuf);
        txBd->addrhigh = 0;
        txBd->ctrl2 = len;
        txBd->ctrl1 = dmaStatus;
#else
        DmaDesc dmaDesc;

        /* Note that the Tx BD has only 2 bits for priority. This means only one
           the first 4 egress queues can be selected using this priority. */
        dmaDesc.length = len;
        dmaDesc.status = dmaStatus;

#if defined(CONFIG_BCM960333) && !defined(MEMCPY_UNALIGNED_WORKAROUND)
        /* BCM960333 workaround: If the buffer is unaligned, set TS bit in the
         * descriptor. This workaround is temporary and shouldn't be needed
         * for B0.
         */
        if (param2)
        {
            dmaDesc.status |= DMA_DESC_TS;
        }
#endif

        txBd = &txdma->txBds[txIndex];
        txBd->address = (uint32)VIRT_TO_PHYS(pBuf);
        txBd->word0 = dmaDesc.word0;
#endif

#if defined(CONFIG_BCM947189)
        /*
         * Post the BD to notify the DMA the BD the transfer is ready
         * (advance ptr to the next BD)
         */
        if (txIndex == (txdma->numTxBds - 1))
            txdma->txDma->ptr = (unsigned int)(txdma->txBdsPhysBase);
        else
            txdma->txDma->ptr += sizeof(DmaDesc);
#endif /* 947189 */
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) 
        wmb();
#endif /* ARM */
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_PKTDMA_TX_DEBUG("bdIdx: %d bdAddr: 0x%08x\n", txIndex, (int)txBd);
        BCM_PKTDMA_TX_DEBUG("key: 0x%08x\n", (int)key);
        BCM_PKTDMA_TX_DEBUG("TX BD: address=0x%08x\n", (int)VIRT_TO_PHYS(pBuf) );
        BCM_PKTDMA_TX_DEBUG("Tx BD: length=%u\n", len);
        BCM_PKTDMA_TX_DEBUG("Tx BD: word0=0x%04x \n", (unsigned int) dmaDesc.word0);

        BCM_PKTDMA_TX_DEBUG("Enabling Tx DMA \n");
#endif
    }

    /* Enable DMA for this channel */
    if(txdma->txEnabled)
        /* Do not reenable if Host MIPs has shut the interface down */
#if defined(CONFIG_BCM947189)
        txdma->txDma->control |= DMA_EN;
#else
        txdma->txDma->cfg = DMA_ENABLE;
#endif

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_XMIT);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmitBufCountGet
 Purpose: Determine the number of buffer queued for xmit
-------------------------------------------------------------------------- */
static inline int bcmPktDma_EthXmitBufCountGet_Iudma( BcmPktDma_LocalEthTxDma *txdma)
{
    return (txdma->numTxBds - txdma->txFreeBds);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmitBufCountGet_Dqm
 Purpose: Determine the number of buffer queued for xmit
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline int bcmPktDma_EthXmitBufCountGet_Dqm(BcmPktDma_LocalEthTxDma *txdma, uint32 dqm)
{
	int dqmDepth = DQM_HOST2FAP_ETH_XMIT_DEPTH_LOW;
    int avail = bcmPktDma_isDqmXmitAvailableHost(txdma->fapIdx, dqm);
   
	if(dqm == DQM_HOST2FAP_ETH_XMIT_Q_HI)
		dqmDepth = DQM_HOST2FAP_ETH_XMIT_DEPTH_HI;
		
    return (dqmDepth- avail);
}
#endif


/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmitAvailable
 Purpose: Determine if there are free resources for the xmit
-------------------------------------------------------------------------- */
static inline int bcmPktDma_EthXmitAvailable_Iudma( BcmPktDma_LocalEthTxDma *txdma )
{
#if defined(CONFIG_BCM947189)
    return (txdma->txFreeBds - 1);
#else
    return (txdma->txFreeBds);
#endif
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmitAvailable_Dqm
 Purpose: Determine if there are free resources for the xmit
   Notes: Intended to be called on Host MIPs only
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline int bcmPktDma_EthXmitAvailable_Dqm(BcmPktDma_LocalEthTxDma *txdma, uint32 dqm)
{
    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "chanel %d", txdma->channel);

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        return( bcmPktDma_EthXmitAvailable_Iudma(txdma) );
    }
#endif

    return (bcmPktDma_isDqmXmitAvailableHost(txdma->fapIdx, dqm));
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmit
 Purpose: Xmit an NBuff
   Notes: param1 = gemid;   param2 = port_id; param3 = egress_queue
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
#if !defined(CONFIG_BCM947189)
static inline int bcmPktDma_EthXmit_Iudma(BcmPktDma_LocalEthTxDma * txdma, uint8 *pBuf, uint16 len,
                                          int bufSource, uint16 dmaStatus, uint32 key,
                                          int param1, int param2)
{
    if(bcmPktDma_EthXmitAvailable_Iudma(txdma))
    {
        bcmPktDma_EthXmitNoCheck_Iudma(txdma, pBuf, len, bufSource, dmaStatus, key, param1, param2);

        return 1;
    }

    return 0;
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthXmit
 Purpose: Xmit an skb without space check. Must be used in conjunction with
          bcmPktDma_EthXmitAvailable_Dqm().
   Notes: param1 = gemid; param2 = port_id; param3 = egress_queue
          In DQM msg, word0 = pBuf; word1 = channel & len;
                      word2 = gemid & port_id; word3 = egress_queue
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline void bcmPktDma_EthXmitNoCheck_Dqm(BcmPktDma_LocalEthTxDma *txdma,
                                                uint8 *pBuf, uint16 len, int bufSource, uint16 dmaStatus,
                                                uint32 key, int param1, uint32 dqm, uint32 virtDestPort,
                                                uint32 destQueue, int param2, uint32 is_spdsvc_setup_packet)
{
    fapDqm_EthTx_t tx;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        bcmPktDma_EthXmitNoCheck_Iudma(txdma, pBuf, len, bufSource, dmaStatus, 
                                       key, param1, param2);
        return;
    }
#endif

#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
    BCM_PKTDMA_TX_DEBUG("Tx BD: param1: 0x%08x \n", (uint32_t)param1);
    BCM_PKTDMA_TX_DEBUG("Tx BD: param2: 0x%08x \n", (uint32_t)param2);
#endif

    
    tx.pBuf = pBuf;
    tx.is_spdsvc_setup_packet = is_spdsvc_setup_packet;
    tx.source = bufSource;
    tx.virtDestPort = virtDestPort;
    tx.destQueue = destQueue;
    tx.len = len;
    tx.key = key;
    tx.dmaStatus = dmaStatus;
    tx.param1 = param1;
    bcmPktDma_dqmXmitMsgHost(txdma->fapIdx, dqm,
                             DQM_HOST2FAP_ETH_XMIT_Q_SIZE, (DQMQueueDataReg_S *)(&tx));
}
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmit
 Purpose: Xmit an NBuff
   Notes: param1 = gemid;   param2 = port_id; param3 = egress_queue
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
static inline int bcmPktDma_XtmXmit_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint8 *pBuf, uint16 len,
                                          int bufSource, uint16 dmaStatus, uint32 key,
                                          int param1, uint32 dmaType,
                                          uint32 noGlobalBufAccount)
{
    DmaDesc                  dmaDesc;
    volatile DmaDesc        *txBd;
    int                      txIndex;

    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_XMIT);

    /* Per chip limitation,
     * max PTM frame size of 1980 + 4(FCS) = 1984 (63268)
     */
    if (((dmaStatus & FSTAT_CT_MASK) == FSTAT_CT_PTM) && (len > PTM_MAX_TX_FRAME_LEN))
    {
//        fap4kePrt_Error("PTM frame len=%u exceeds %u (max)", len, PTM_MAX_TX_FRAME_LEN);
        return 0;   /* drop */
    }        

    if (txdma->txFreeBds == 0) return 0;   /* No free tx BDs. Return Fail */
    if (txdma->txEnabled == 0) return 0;   /* Channel is down */

    txIndex = txdma->txTailIndex;

    dmaDesc.length = len;

    /* Decrement total BD count */
    txdma->txFreeBds--;
    txdma->ulNumTxBufsQdOne++;
#if !defined(CONFIG_BCM963268)
       // FIXME - Which chip uses more then one TX queue?
    if (!noGlobalBufAccount)
    g_pXtmGlobalInfo->ulNumTxBufsQdAll++;
#endif

    txdma->txRecycle[txIndex].key = key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    txdma->txRecycle[txIndex].source = bufSource;
    txdma->txRecycle[txIndex].address = (uint32)pBuf;
    txdma->txRecycle[txIndex].rxChannel = param1;
#endif

    /* Note that the Tx BD has only 2 bits for priority. This means only one
       the first 4 egress queues can be selected using this priority. */
    dmaDesc.status = dmaStatus;

    txBd = &txdma->txBds[txIndex];

    /* advance BD pointer to next in the chain. */
    if (txIndex == (txdma->ulQueueSize - 1))
    {
        dmaDesc.status = dmaDesc.status | DMA_WRAP;
        txdma->txTailIndex = 0;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_XTM_TX_DEBUG("Tx BD: dma_status: %x\n", dmaDesc.status);
#endif
    }
    else
    {
        txdma->txTailIndex++;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_XTM_TX_DEBUG("Tx BD: dma_status: %x idx: %d\n", dmaDesc.status, txdma->txTailIndex);
#endif
    }

    txBd->address = (uint32)VIRT_TO_PHYS(pBuf);
    txBd->word0 = dmaDesc.word0;

#if defined(CONFIG_ARM)
    wmb();
#endif

#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
    BCM_XTM_TX_DEBUG("key: 0x%08x\n", (int)key);
    BCM_XTM_TX_DEBUG("TX BD: address=0x%08x\n", (int)VIRT_TO_PHYS(pBuf) );
    BCM_XTM_TX_DEBUG("Tx BD: length=%u\n", len);
    BCM_XTM_TX_DEBUG("Tx BD: word0=%ld \n", dmaDesc.word0);

    BCM_XTM_TX_DEBUG("Enabling Tx DMA \n\n");
#endif


    /* Enable DMA for this channel.
     * Software DMA are controlled using this "cfg" field more at the whole DMA
     * enable/disable operation level as opposed to being at every packet
     */
    if (dmaType == XTM_HW_DMA)
    txdma->txDma->cfg = DMA_ENABLE;

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_XMIT);

    return 1;
}
#endif /* CONFIG_BCM_XTMCFG */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecvBufGet_Iudma
 Purpose: Get a buffer starting from head index on a specific channel
-------------------------------------------------------------------------- */
static inline BOOL bcmPktDma_EthRecvBufGet_Iudma(BcmPktDma_LocalEthRxDma * rxdma, uint8 **pRxAddr)
{
#if defined(CONFIG_BCM947189)
    uint32 bd_offset;
    uint32 status0;
#else
    volatile DmaDesc * rxBd;
    DmaDesc            dmaDesc;
    int                head_idx;
#endif

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    if(rxdma->rxAssignedBds == 0)
    {
        return FALSE;
    }


#if defined(CONFIG_BCM947189)
    bd_offset = ((rxdma->rxHeadIndex * sizeof(DmaDesc)) + 
                  (uint32)(rxdma->rxBdsPhysBase)) & DMA_CURRENT_DESCR;
    status0 = rxdma->rxDma->status0;
    if ((status0 & DMA_CURRENT_DESCR) != bd_offset) 
    {
        *pRxAddr = (unsigned char *)
            (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].addrlow));

        /* Packet length is stored in the first two bytes */
        *pRxAddr += 4;

        /*
         * Advance ptr to the next BD. Point it to the beginning of the ring if
         * rxHeadindex is right before the EOT BD to allow the DMA to use the
         * EOT BD for reception and automatically wrap around.
         */
        if (rxdma->rxDma->ptr == ((unsigned int)(&rxdma->rxBds[rxdma->numRxBds - 1]) & 0xFFFF))
            rxdma->rxDma->ptr = (unsigned int)(rxdma->rxBdsPhysBase);
        else
            rxdma->rxDma->ptr += sizeof(DmaDesc);

        /* Advance rxHeadIndex. Wrap around if it reached the end of the ring */
        if (rxdma->rxHeadIndex == (rxdma->numRxBds - 1)) {
            rxdma->rxHeadIndex = 0;
        }
        else
            rxdma->rxHeadIndex++;
    }
#else
    head_idx = rxdma->rxHeadIndex;
    rxBd = &rxdma->rxBds[head_idx];

    /* take the ownership of DMA, and advance pointer */
    dmaDesc.length = BCM_MAX_PKT_LEN;
    dmaDesc.status &= ~DMA_OWN;
    if (head_idx == (rxdma->numRxBds - 1)) {
        dmaDesc.status |= DMA_WRAP;
        rxdma->rxHeadIndex = 0;
    } else {
        rxdma->rxHeadIndex = head_idx + 1;
    }

    *pRxAddr = BCM_PKTDMA_PBUF_FROM_BD(rxBd);
    rxBd->word0 = dmaDesc.word0;
#endif
    rxdma->rxAssignedBds--;

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    return TRUE;
}

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecvBufGet
 Purpose: Free a single RX buffer
-------------------------------------------------------------------------- */
static inline BOOL bcmPktDma_XtmRecvBufGet_Iudma(BcmPktDma_LocalXtmRxDma *rxdma, uint32 *pRxAddr)
{
    volatile DmaDesc        *rxBd;
    DmaDesc                  dmaDesc;
    int                      head_idx;

    //printk("bcmPktDma_XtmRecvBufGet_Iudma ch: %d pBuf: %p\n", rxdma->channel, ppBuf);

    //FAP4KE_IUDMA_PMON_DECLARE();
    //FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    if(rxdma->rxAssignedBds == 0)
    {
        return FALSE;
    }

    head_idx = rxdma->rxHeadIndex;
    rxBd = &rxdma->rxBds[head_idx];

    /* take the ownership of DMA, and advance pointer */
    dmaDesc.length = BCM_MAX_PKT_LEN;
    dmaDesc.status &= ~DMA_OWN;
    if (head_idx == (rxdma->numRxBds - 1)) {
        dmaDesc.status |= DMA_WRAP;
        rxdma->rxHeadIndex = 0;
    } else {
        rxdma->rxHeadIndex = head_idx + 1;
    }

    *pRxAddr = (uint32) BCM_PKTDMA_PBUF_FROM_BD(rxBd);
    rxBd->word0 = dmaDesc.word0;

    rxdma->rxAssignedBds--;

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_FREERECVBUF);

    return TRUE;
}
#endif /* CONFIG_BCM_XTMCFG */
#endif


#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmitSwdmaToIudma
 Purpose: Xmit the DmaDesc contents from SW DMA to HW DMA. Done by
          BCM6368 SW Scheduler.
   Notes: param1 = gemid;   param2 = port_id; param3 = egress_queue
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
static inline int bcmPktDma_XtmXmit_Swdma_to_Iudma(BcmPktDma_LocalXtmTxDma *txdma, DmaDesc dmaDesc,
                                                   uint32 key)
{
    volatile DmaDesc        *txBd;
    int                      txIndex;

    if (txdma->txFreeBds == 0) return 0;   /* No free tx BDs. Return Fail */
    if (txdma->txEnabled == 0) return 0;   /* Channel is down */

    txIndex = txdma->txTailIndex;

    /* Decrement total BD count */
    txdma->txFreeBds--;
    txdma->ulNumTxBufsQdOne++;

    txdma->txRecycle[txIndex].key = key;

    txBd = &txdma->txBds[txIndex];

    dmaDesc.status &= ~DMA_WRAP ;

    /* advance BD pointer to next in the chain. */
    if (txIndex == (txdma->ulQueueSize - 1))
    {
        dmaDesc.status = dmaDesc.status | DMA_WRAP;
        txdma->txTailIndex = 0;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_XTM_TX_DEBUG("Tx BD: dma_status: %x\n", dmaDesc.status);
#endif
    }
    else
    {
        txdma->txTailIndex++;
#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
        BCM_XTM_TX_DEBUG("Tx BD: dma_status: %x idx: %d\n", dmaDesc.status, txdma->txTailIndex);
#endif
    }

    txBd->address = dmaDesc.address ;
    txBd->word0 = dmaDesc.word0;

#if defined(ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING)
    BCM_XTM_TX_DEBUG("key: 0x%08x\n", (int)key);
    BCM_XTM_TX_DEBUG("TX BD: address=0x%08x\n", (int)VIRT_TO_PHYS(pBuf) );
    BCM_XTM_TX_DEBUG("Tx BD: length=%u\n", len);
    BCM_XTM_TX_DEBUG("Tx BD: word0=%ld \n", dmaDesc.word0);

    BCM_XTM_TX_DEBUG("Enabling Tx DMA \n\n");
#endif


    /* Enable DMA for this channel.
     * Software DMA are controlled using this "cfg" field more at the whole DMA
     * enable/disable operation level as opposed to being at every packet
     */
    txdma->txDma->cfg = DMA_ENABLE;

    //FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_XMIT);

    return 1;
}
#endif /* CONFIG_BCM_XTMCFG */

static inline int bcmPktDma_EthXmitFreeCount_Iudma(BcmPktDma_LocalEthTxDma *txdma)
{
    return txdma->txFreeBds;
}

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
static inline int bcmPktDma_XtmXmitFreeCount_Iudma(BcmPktDma_LocalXtmTxDma *txdma)
{
    return txdma->txFreeBds;
}
#endif /* CONFIG_BCM_XTMCFG */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthFreeRecvBufCount
 Purpose: Calculate the number of rx BDs available for the h/w.
 Return: Value >= PKTDMA_ETH_NO_BD_THRESHOLD if h/w has lots of BDs available.
         Value < PKTDMA_ETH_NO_BD_THRESHOLD if h/w is about to run out of BDs.
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline uint32 bcmPktDma_EthFreeRecvBufCount_Iudma(BcmPktDma_LocalEthRxDma * rxdma)
{
#if 0
    /* H/W version of function. Do not use h/w counter (ringOffset) that causes crashes - Sep 2010 */
    uint32 rxHeadIndex;
    uint32 ringOffset;
    uint32 rxQueueDepth;

    rxHeadIndex = rxdma->rxHeadIndex;

    ringOffset = SW_DMA->stram.s[rxdma->channel * 2].state_data;
    ringOffset &= 0x1FFF;

    if(ringOffset == rxHeadIndex)
    {
        if(rxdma->rxAssignedBds >= rxdma->numRxBds)
        {
            rxQueueDepth = 0;
        }
        else
{
            rxQueueDepth = rxdma->numRxBds;
        }
    }
    else if(ringOffset > rxHeadIndex)
    {
        rxQueueDepth = ringOffset - rxHeadIndex - 1;
    }
    else /* (ringOffset < rxHeadIndex) */
    {
        /* wraparound */
        rxQueueDepth = rxdma->numRxBds - rxHeadIndex + ringOffset - 1;
    }

    return(rxdma->numRxBds - rxQueueDepth);

#else
    /* S/W version of function. Do not use h/w counter (ringOffset) that causes crashes - Sep 2010 */
    volatile DmaDesc * rxBd;
    DmaDesc            dmaDesc;
    uint32             rxIndex;

	if(!rxdma->rxEnabled) return PKTDMA_ETH_NO_BD_THRESHOLD;

    if(rxdma->rxAssignedBds < PKTDMA_ETH_NO_BD_THRESHOLD)
        /* Can't tell if h/w about to run out of BDs */
        return(PKTDMA_ETH_NO_BD_THRESHOLD);

    rxIndex = rxdma->rxTailIndex;
    if(rxIndex >= PKTDMA_ETH_NO_BD_THRESHOLD)
        rxIndex -= PKTDMA_ETH_NO_BD_THRESHOLD;
    else
        rxIndex = rxdma->numRxBds - (PKTDMA_ETH_NO_BD_THRESHOLD - rxIndex);

    rxBd = &rxdma->rxBds[rxIndex];
    dmaDesc.word0 = rxBd->word0;
    if (dmaDesc.status & DMA_OWN)
        /* h/w is not getting close to running out of BDs yet */
        return(PKTDMA_ETH_NO_BD_THRESHOLD);

	return(0);
#endif
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthCheckRecvNoBds
 Purpose: Check if iuDMA h/w is about to run out of BDs for receive.
          Force a packet drop to avoid the no BD condition.
 Return: 1 if packets were dropped. 0 otherwise.
-------------------------------------------------------------------------- */
static inline int bcmPktDma_EthCheckRecvNoBds_Iudma(BcmPktDma_LocalEthRxDma * rxdma)
{
    volatile DmaDesc * rxBd;
    uint32             pBuf;
    DmaDesc            dmaDesc;
    int                tail_idx;
    int                i;

    if(bcmPktDma_EthFreeRecvBufCount_Iudma(rxdma) < PKTDMA_ETH_NO_BD_THRESHOLD)
    {   /* hardware about to run out of BDs */
        for(i = 0; i < PKTDMA_ETH_NO_BD_THRESHOLD; i++)
        {
            /* Forced receive of oldest packets  */
            rxBd = &rxdma->rxBds[rxdma->rxHeadIndex];
            pBuf = rxBd->address;

            dmaDesc.word0 = rxBd->word0;
            if (dmaDesc.status & DMA_OWN)
            {
                break;
            }

            if (++rxdma->rxHeadIndex == rxdma->numRxBds)
            {
                rxdma->rxHeadIndex = 0;
            }

            /* Immediate free to hardware */
            tail_idx = rxdma->rxTailIndex;
            rxBd = &rxdma->rxBds[tail_idx];

            dmaDesc.length = BCM_MAX_PKT_LEN;
            if (tail_idx == (rxdma->numRxBds - 1)) {
                dmaDesc.status = DMA_OWN | DMA_WRAP;
                rxdma->rxTailIndex = 0;
            } else {
                dmaDesc.status = DMA_OWN;
                rxdma->rxTailIndex = tail_idx + 1;
            }

            rxBd->address = pBuf;
            rxBd->word0 = dmaDesc.word0;
        }  /* end for */

        return(1);
    }

    return(0);
}

static inline int bcmPktDma_EthCheckRecvNoBds_Dqm(BcmPktDma_LocalEthRxDma * rxdma)
{

    return(0);
}
#endif  /* #if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecv
 Purpose: Receive a packet on a specific channel,
          returning the associated DMA flag
-------------------------------------------------------------------------- */
static inline uint32 bcmPktDma_EthRecv_Iudma(BcmPktDma_LocalEthRxDma * rxdma,
                                             unsigned char **pBuf, int * pLen)
{
#if defined(CONFIG_BCM947189)
    DmaRxStatus  rx_status;

    if (rxdma->rxAssignedBds > 1)
    {
        unsigned int bd_offset = ((rxdma->rxHeadIndex * sizeof(DmaDesc)) + 
                                   (unsigned int)(rxdma->rxBdsPhysBase)) & DMA_CURRENT_DESCR;
        unsigned int status0 = rxdma->rxDma->status0;

        /*
         * Check if the buffer pointed by the BD was completely received.
         * In the 47189 DMAs the buffer was completely transferred if the
         * DMA_CURRENT_DESCR field of status0 has advanced to the next BD.
         */
        if ((status0 & DMA_CURRENT_DESCR) != bd_offset) 
        {
            *pBuf = (unsigned char *)
                (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].addrlow));

            cache_invalidate_region(*pBuf - 60, *pBuf + 68); //fixme, why need to invalidate cache
            /* Packet length is stored in the first two bytes */
            rx_status.word = *((uint32 *)*pBuf);
            *pLen = rx_status.length;
            *pBuf += DMA_DATA_OFFSET;
            /* Advance rxHeadIndex. Wrap around if it reached the end of the ring */
            if (rxdma->rxHeadIndex == (rxdma->numRxBds - 1)) {
                rxdma->rxHeadIndex = 0;
            }
            else
                rxdma->rxHeadIndex++;

            rxdma->rxAssignedBds--;
            return rx_status.word;
        }
        else
            return (uint32)0xFFFF;
    }
    else
        return (uint32)0xFFFF;
#else
    DmaDesc         dmaDesc;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

    dmaDesc.word0 = 0;
    if (rxdma->rxAssignedBds != 0)
    {
        /* Get the status from Rx BD */
        dmaDesc.word0 = rxdma->rxBds[rxdma->rxHeadIndex].word0;

        /* If no more rx packets, we are done for this channel */
        if ((dmaDesc.status & DMA_OWN) == 0)
        {
            *pBuf = (unsigned char *)
                   (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].address));
            *pLen = (int) dmaDesc.length;

            /* Wrap around the rxHeadIndex */
            if (++rxdma->rxHeadIndex == rxdma->numRxBds)
            {
                rxdma->rxHeadIndex = 0;
            }
            rxdma->rxAssignedBds--;
        }
    }
    else   /* out of buffers! */
       return (uint32)0xFFFF;

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);

    return dmaDesc.word0;
#endif
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRecv
 Purpose: Receive a packet on a specific channel, returning the associated DMA desc
   Notes: In DQM msg, word0 = pBuf; word1 = length; word2 = dmaFlag
-------------------------------------------------------------------------- */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline uint32 bcmPktDma_EthRecv_Dqm(BcmPktDma_LocalEthRxDma * rxdma,
                                           unsigned char **pBuf,
                                           int            *pLen)
{
    fapDqm_EthRx_t    rx;

    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel: %d", rxdma->channel);


	/* we use strict priority scheduling among HI & LOW priority DQM's*/
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    if (bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx, 
										DQM_FAP2HOST_ETH_RX_Q_HI))
    {
        bcmPktDma_dqmRecvMsgHost(rxdma->fapIdx, DQM_FAP2HOST_ETH_RX_Q_HI,
						 DQM_FAP2HOST_ETH_RX_Q_SIZE, (DQMQueueDataReg_S *) &rx);

        *pBuf          = (unsigned char *) rx.pBuf;
        *pLen          = rx.rxParam1.length;

#ifndef FAP_4KE
      //  printk("HI pbuf: 0x%08lX len: %d rx.rxParam1.data: 0x%08lX\n",
        //          (long unsigned int)*pBuf, *pLen, (rx.rxParam1.data));
#endif
    }
    else if (bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx, 
										DQM_FAP2HOST_ETH_RX_Q_LOW))
#else
    if (bcmPktDma_isDqmRecvAvailableHost(rxdma->fapIdx,
										 DQM_FAP2HOST_ETH_RX_Q_LOW))
#endif
    {
        bcmPktDma_dqmRecvMsgHost(rxdma->fapIdx, DQM_FAP2HOST_ETH_RX_Q_LOW,
					 	DQM_FAP2HOST_ETH_RX_Q_SIZE, (DQMQueueDataReg_S *) &rx);

        *pBuf         = (unsigned char *) rx.pBuf;
        *pLen          = rx.rxParam1.length;
#ifndef FAP_4KE
      //   printk("LOW pbuf: 0x%08lX len: %d rx.rxParam1.data: 0x%08lX\n",
         //          (long unsigned int)*pBuf, *pLen, (rx.rxParam1.data));
#endif
    }
    else
    {
        *pBuf   = (unsigned char *) NULL;
        *pLen   = 0;
        rx.rxParam1.data = 0;
        rx.rxParam1.msgId = FAP2HOST_ETH_RX_MSGID_MAX;
        rx.rxParam1.status = DMA_OWN;
    }

    return rx.rxParam1.data;
}
#endif

/* Prototypes for interface */
/* IUDMA versions */
int    bcmPktDma_EthInitRxChan_Dqm( uint32 bufDescrs,
                                 BcmPktDma_LocalEthRxDma *pEthRxDma);

int    bcmPktDma_EthInitTxChan_Dqm( uint32 bufDescrs,
                                 BcmPktDma_LocalEthTxDma *pEthTxDma);

#if defined(CONFIG_BCM_GMAC)
int    bcmPktDma_EthUnInitRxChan_Dqm(BcmPktDma_LocalEthRxDma *pEthRxDma);
int    bcmPktDma_EthUnInitTxChan_Dqm(BcmPktDma_LocalEthTxDma *pEthTxDma);
#endif

#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
int bcmPktDma_EthInitExtSw_Dqm( uint32 extSwConnPort);
#endif

int bcmPktDma_EthInitRxChan_Iudma( uint32 bufDescrs,
                                   BcmPktDma_LocalEthRxDma *pEthRxDma);

int bcmPktDma_EthInitTxChan_Iudma( uint32 bufDescrs,
                                   BcmPktDma_LocalEthTxDma *pEthTxDma);
#if defined(CONFIG_BCM_GMAC)
int bcmPktDma_EthUnInitRxChan_Iudma(BcmPktDma_LocalEthRxDma *pEthRxDma);
int bcmPktDma_EthUnInitTxChan_Iudma(BcmPktDma_LocalEthTxDma *pEthTxDma);
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmInitRxChan_Iudma( uint32 bufDescrs,
                                   BcmPktDma_LocalXtmRxDma *pXtmRxDma);

int bcmPktDma_XtmUnInitRxChan_Iudma(BcmPktDma_LocalXtmRxDma *pXtmRxDma);

int bcmPktDma_XtmInitRxChan_Dqm( uint32 bufDescrs,
                                 BcmPktDma_LocalXtmRxDma *pXtmRxDma);

int bcmPktDma_XtmUnInitRxChan_Dqm( BcmPktDma_LocalXtmRxDma * pXtmRxDma);

int bcmPktDma_XtmInitTxChan_Iudma( uint32 bufDescrs,
                                   BcmPktDma_LocalXtmTxDma *pXtmTxDma,
                                   uint32 dmaType);

int bcmPktDma_XtmInitTxChan_Dqm( uint32 bufDescrs,
                                   BcmPktDma_LocalXtmTxDma *pXtmTxDma,
                                   uint32 dmaTypeUnused);

int bcmPktDma_XtmCreateDevice_Dqm(uint32 devId, uint32 encapType, uint32 headerLen, uint32 trailerLen);
int bcmPktDma_XtmLinkUp_Dqm(uint32 devId, uint32 matchId);
#endif /* CONFIG_BCM_XTMCFG */

int bcmPktDma_EthInit_Dqm( BcmPktDma_LocalEthRxDma **pEthRxDma,
                             BcmPktDma_LocalEthTxDma **pEthTxDma,
                             uint32 rxChannels,
                             uint32 txChannels,
                             uint32 rxBufDescs,
                             uint32 txBufDescs );
int	bcmPktDma_EthInit_Iudma( BcmPktDma_LocalEthRxDma **pEthRxDma,
                             BcmPktDma_LocalEthTxDma **pEthTxDma,
                             uint32 rxChannels,
                             uint32 txChannels,
                             uint32 rxBufDescs,
                             uint32 txBufDescs );

int     bcmPktDma_EthSelectRxIrq_Iudma(int channel);
int     bcmPktDma_XtmSelectRxIrq_Iudma(int channel);

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
void    bcmPktDma_XtmClrRxIrq_Iudma(BcmPktDma_LocalXtmRxDma * rxdma);

uint32  bcmPktDma_XtmRecv_Iudma(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char **pBuf, int * pLen);

uint32  bcmPktDma_XtmRecv_RingSize_Iudma(BcmPktDma_LocalXtmRxDma * rxdma);

void    bcmPktDma_XtmFreeXmitBuf_Iudma(int channel, int index);

int     bcmPktDma_XtmXmitAvailable_Iudma(BcmPktDma_LocalXtmTxDma *txdma);

int     bcmPktDma_XtmXmit_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint8 *pBuf, uint16 len, int bufSource, uint16 dmaStatus, uint32 key, int param1,
                                uint32 dmaType, uint32 noGlobalBufAccount);

int     bcmPktDma_XtmXmit_Swdma_to_Iudma(BcmPktDma_LocalXtmTxDma *txdma, DmaDesc dmaDesc, uint32 key) ;
#endif /* CONFIG_BCM_XTMCFG */

int     bcmPktDma_EthTxEnable_Iudma(BcmPktDma_LocalEthTxDma * txdma);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int     bcmPktDma_XtmTxEnable_Iudma(BcmPktDma_XtmTxDma * txdma, PDEV_PARAMS pDevParams, uint32 dmaType);
#endif

int     bcmPktDma_EthTxDisable_Iudma(BcmPktDma_LocalEthTxDma * txdma);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int     bcmPktDma_XtmTxDisable_Iudma(BcmPktDma_LocalXtmTxDma * txdma, uint32 dmaType, void (*func) (uint32 param1, BcmPktDma_XtmTxDma *txdma), uint32 param1);
#endif

int     bcmPktDma_EthRxEnable_Iudma(BcmPktDma_LocalEthRxDma * rxdma);
int     bcmPktDma_EthRxDisable_Iudma(BcmPktDma_LocalEthRxDma * rxdma);

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int     bcmPktDma_XtmRxEnable_Iudma(BcmPktDma_LocalXtmRxDma * rxdma);
int     bcmPktDma_XtmRxDisable_Iudma(BcmPktDma_LocalXtmRxDma * rxdma);

static inline BOOL bcmPktDma_XtmForceFreeXmitBufGet_Iudma(BcmPktDma_LocalXtmTxDma * txdma, uint32 *pKey,
                                            uint32 *pTxSource, uint32 *pTxAddr,
                                            uint32 *pRxChannel, uint32 dmaType,
                                            uint32 noGlobalBufAccount);
static inline BOOL bcmPktDma_XtmFreeXmitBufGetRecycle_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint32 *pKey,
                                                     uint32 *pTxSource, uint32 *pTxAddr,
                                                     uint32 *pRxChannel,
                                                     uint32 dmaType,
                                                     uint32 noGlobalBufAccount) ;
#endif /* CONFIG_BCM_XTMCFG */

int bcmPktDma_EthGetRxBds( BcmPktDma_LocalEthRxDma *rxdma, int channel );
int bcmPktDma_EthGetTxBds( BcmPktDma_LocalEthTxDma *txdma, int channel );
int bcmPktDma_XtmGetRxBds( int channel );
int bcmPktDma_XtmGetTxBds( int channel );

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int bcmPktDma_EthSetIqThresh_Iudma( BcmPktDma_LocalEthRxDma * rxdma,
                                    uint16 loThresh,
                                    uint16 hiThresh);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmSetIqThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                    uint16 loThresh,
                                    uint16 hiThresh);
#endif /* CONFIG_BCM_XTMCFG */
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_EthSetRxChanBpmThresh_Iudma( BcmPktDma_LocalEthRxDma * rxdma,
                                           uint16 allocTrig,
                                           uint16 bulkAlloc);
int bcmPktDma_EthBpmSetTxChanThresh_Iudma( BcmPktDma_LocalEthTxDma * txdma,
                                           uint16 *dropThr );
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmSetRxChanBpmThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                           uint16 allocTrig,
                                           uint16 bulkAlloc );
int bcmPktDma_EthSetTxChanBpmThresh_Iudma( BcmPktDma_LocalEthTxDma * txdma,
                                           uint16 *dropThr );
int bcmPktDma_XtmSetTxChanBpmThresh_Iudma( BcmPktDma_LocalXtmTxDma * txdma,
                                           uint16 loThresh,
                                           uint16 hiThresh,
                                           uint32 dmaType);
#endif /* CONFIG_BCM_XTMCFG */
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmSetTxChanDropAlg_Iudma( BcmPktDma_LocalXtmTxDma * txdma, int dropAlgorithm,
                                         int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                         int dropProbabilityHi, int minThresholdHi, int maxThresholdHi);
#endif

void bcmPktDma_EthGetStats_Iudma(uint8 vport, uint32 *rxDropped, 
    uint32 *txDropped);
void bcmPktDma_EthResetStats_Iudma(uint8 vport);

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
void bcmPktDma_XtmGetStats_Iudma(uint8 vport, uint32 *rxDropped, 
    uint32 *txDropped);
void bcmPktDma_XtmResetStats_Iudma(uint8 vport);
#endif /* CONFIG_BCM_XTMCFG */

/* DQM versions */
int     bcmPktDma_EthSelectRxIrq_Dqm(int channel);
void    bcmPktDma_EthClrRxIrq_Dqm(BcmPktDma_LocalEthRxDma *rxdma);
void    bcmPktDma_EthFreeXmitBuf_Dqm(int channel, BcmPktDma_LocalEthTxDma * txdma, int index);
int     bcmPktDma_EthTxEnable_Dqm(BcmPktDma_LocalEthTxDma * txdma);
int     bcmPktDma_EthTxDisable_Dqm(BcmPktDma_LocalEthTxDma *  txdma);
int     bcmPktDma_EthRxEnable_Dqm(BcmPktDma_LocalEthRxDma * rxdma);
int     bcmPktDma_EthRxDisable_Dqm(BcmPktDma_LocalEthRxDma * rxdma);
int     bcmPktDma_EthRxReEnable_Dqm(int channel);

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int     bcmPktDma_XtmSelectRxIrq_Dqm(int channel);
void    bcmPktDma_XtmClrRxIrq_Dqm(BcmPktDma_LocalXtmRxDma *rxdma);
uint32  bcmPktDma_XtmRecv_RingSize_Dqm(BcmPktDma_LocalXtmRxDma * rxdma);
uint32  bcmPktDma_XtmRecv_Dqm(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char **pBuf, int * pLen);
void    bcmPktDma_XtmFreeRecvBuf_Dqm(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char * pBuf);
void    bcmPktDma_XtmFreeXmitBuf_Dqm(int channel, int index);
int     bcmPktDma_XtmXmitAvailable_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint32 dqm) ;
int     bcmPktDma_XtmXmit_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint8 *pBuf, uint16 len, int bufSource,
                              uint16 dmaStatus, uint32 key,int param1,
                              uint32 dmaType, uint32 noGlobalBufAccount, uint32 dqm,
                              uint32 is_spdsvc_setup_packet);
int     bcmPktDma_XtmTxEnable_Dqm(BcmPktDma_XtmTxDma * txdma, PDEV_PARAMS pDevParams, uint32 dmaTypeUnused);
int     bcmPktDma_XtmTxDisable_Dqm(BcmPktDma_LocalXtmTxDma * txdma, uint32 dmaTypeUnused, void (*func)(uint32 param1, BcmPktDma_XtmTxDma *txdma), uint32 param1);

int     bcmPktDma_XtmRxEnable_Dqm(BcmPktDma_LocalXtmRxDma * rxdma);
int     bcmPktDma_XtmRxDisable_Dqm(BcmPktDma_LocalXtmRxDma * rxdma);
BOOL    bcmPktDma_XtmFreeXmitBufGet_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint32 *pKey, uint32 *pTxSource,
                                        uint32 *pTxAddr, uint32 *rxChannel,
                                        uint32 dmaType, uint32 noGlobalBufAccount);

#endif /* CONFIG_BCM_XTMCFG */

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int	bcmPktDma_EthSetIqThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh);
int	bcmPktDma_EthSetIqDqmThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh);
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int	bcmPktDma_XtmSetIqThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh);
int	bcmPktDma_XtmSetIqDqmThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh);
#endif /* CONFIG_BCM_XTMCFG */
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_EthSetRxChanBpmThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                         uint16 allocTrig,
                                         uint16 bulkAlloc );
int	bcmPktDma_EthSetTxChanBpmThresh_Dqm( BcmPktDma_LocalEthTxDma * txdma,
                                         uint16 *dropThr );
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmSetRxChanBpmThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                         uint16 allocTrig,
                                         uint16 bulkAlloc );
int	bcmPktDma_XtmSetTxChanBpmThresh_Dqm( BcmPktDma_LocalXtmTxDma * txdma,
                                         uint16 loThresh,
                                         uint16 hiThresh,
                                         uint32 dmaType);
#endif /* CONFIG_BCM_XTMCFG */
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
int bcmPktDma_XtmSetTxChanDropAlg_Dqm( BcmPktDma_LocalXtmTxDma * txdma, int dropAlgorithm,
                                       int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                       int dropProbabilityHi, int minThresholdHi, int maxThresholdHi);
#endif

void bcmPktDma_EthSetPhyRate_Dqm(uint8 port, uint8 enable, int kbps, int isWanPort);
void bcmPktDma_EthSetPauseEn_Dqm(uint8 port, uint8 enable);
void bcmPktDma_EthGetStats_Dqm(uint8 vport, uint32 *rxDropped, 
    uint32 *txDropped);
void bcmPktDma_EthResetStats_Dqm(uint8 vport);

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
void bcmPktDma_XtmGetStats_Dqm(uint8 vport, uint32 *rxDropped, 
    uint32 *txDropped);
void bcmPktDma_XtmResetStats_Dqm(uint8 vport);
#endif /* CONFIG_BCM_XTMCFG */

void bcm63xx_enet_dqmhandler(uint32 fapIdx, unsigned long channel);
void bcm63xx_enet_xmit_free_handler(unsigned long channel);

void bcm63xx_xtm_dqmhandler(uint32 fapIdx, unsigned long channel);
void bcm63xx_xtm_xmit_free_handler(unsigned long channel);

/* Used by both iudma and dqm versions */
#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
DmaDesc * bcmPktDma_EthAllocTxBds(int channel, int numBds);
#else
DmaDesc * bcmPktDma_EthAllocTxBds(struct device *dev, int channel, int numBds, uint32 *phy_addr);
#endif

#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
DmaDesc * bcmPktDma_EthAllocRxBds(int channel, int numBds);
#else
DmaDesc * bcmPktDma_EthAllocRxBds(struct device *dev, int channel, int numBds, uint32 *phy_addr);
#endif

DmaDesc * bcmPktDma_XtmAllocTxBds(int channel, int numBds, uint32* phy_addr);
void bcmPktDma_XtmFreeTxBds(DmaDesc *mem, DmaDesc * phy_addr, int numBds) ;
DmaDesc * bcmPktDma_XtmAllocRxBds(int channel, int numBds, uint32* phy_addr);

#ifndef FAP_4KE
/* Add code for buffer quick free between enet and xtm - June 2010 */
/* Calls directed through bcmPktDma due to linking restrictions between enet and xtm loaded drivers */
void bcmPktDma_set_enet_recycle(RecycleFuncP enetRecycle);
RecycleFuncP bcmPktDma_get_enet_recycle(void);
void bcmPktDma_set_xtm_recycle(RecycleFuncP fkbRecycleFunc, RecycleFuncP skbRecycleFunc);
RecycleFuncP bcmPktDma_get_xtm_fkb_recycle(void);
RecycleFuncP bcmPktDma_get_xtm_skb_recycle(void);
#endif /* !FAP_4KE */

/* Added to support CONFIG_BCM_PKTDMA_RX_SPLITTING - Aug 2010 */
void bcmPktDma_BcmHalInterruptEnable(int channel, int irq);
void bcmPktDma_BcmHalInterruptDisable(int channel, int irq);

//#ifndef FAP_4KE
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)

void mapEthPortToRxIudma(uint8 port, uint8 iudmaChannel);
int getEthRxIudmaFromPort(int port);

static __inline uint32 getFapIdxFromEthRxIudma(int iudma)
{
    switch(g_Eth_rx_iudma_ownership[iudma])
    {
        case FAP0_OWNED:
            return FAP0_IDX;
        case FAP1_OWNED:
            return FAP1_IDX;
        case HOST_OWNED:
        default:
            return FAP_INVALID_IDX;
    }
}

static __inline uint32 getFapIdxFromEthTxIudma(int iudma)
{
    switch(g_Eth_tx_iudma_ownership[iudma])
    {
        case FAP0_OWNED:
            return FAP0_IDX;
        case FAP1_OWNED:
            return FAP1_IDX;
        case HOST_OWNED:
        default:
            return FAP_INVALID_IDX;
    }
}

static __inline uint32 getFapIdxForGPONPort(void)
{
    return( getFapIdxFromEthRxIudma(getEthRxIudmaFromPort(GPON_PORT_ID)) );
}

static __inline uint32 getFapIdxFromEthRxPort(int port)
{
    return( getFapIdxFromEthRxIudma(getEthRxIudmaFromPort(port)) );
}

static __inline uint32 getFapIdxFromXtmRxIudma(int iudma)
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    switch(g_Xtm_rx_iudma_ownership[iudma])
    {
        case FAP0_OWNED:
            return FAP0_IDX;
        case FAP1_OWNED:
            return FAP1_IDX;
        case HOST_OWNED:
        default:
            return FAP_INVALID_IDX;
    }
#else
    return FAP_INVALID_IDX;    
#endif
}

static __inline uint32 getFapIdxFromXtmTxIudma(int iudma)
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    switch(g_Xtm_tx_iudma_ownership[iudma])
    {
        case FAP0_OWNED:
            return FAP0_IDX;
        case FAP1_OWNED:
            return FAP1_IDX;
        case HOST_OWNED:
        default:
            return FAP_INVALID_IDX;
    }
#else
    return FAP_INVALID_IDX;    
#endif
}

static __inline uint32 getFapIdxFromXtmRxPort(int port)
{
    return( getFapIdxFromXtmRxIudma(PKTDMA_XTM_IUDMA) );
}

static __inline uint32 getFapIdxFromXtmTxPort(int port)
{
    return( getFapIdxFromXtmTxIudma(PKTDMA_XTM_IUDMA) );
}

#else
#define mapEthPortToRxIudma(_port, _iudmaChannel)
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */
//#endif /* !FAP_4KE */

#endif  /* defined(__PKTDMA_H_INCLUDED__) */
