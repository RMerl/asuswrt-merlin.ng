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
 * File Name  : bcmPktDmaEthDqm.c
 *
 * Description: This file contains the Packet DMA Implementation for the
 *              Forward Assit Processor (FAP) Dynamic Queues for use by the
 *              Ethernet Controller.
 *
 *******************************************************************************
 */

#if defined(CONFIG_BCM963268)

#include <linux/string.h>
#include <bcm_intr.h>
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap4ke_local.h"
#include "bcmPktDma.h"
#include "bcmPktDmaHooks.h"
#include "fap.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_dqm.h"
#include "fap4ke_msg.h"
#include "fap4ke_irq.h"
#include "fap4ke_memory.h"
#include "fap4ke_gso.h"

int	bcmPktDma_EthInitRxChan_Dqm(uint32 bufDescrs,
                                 BcmPktDma_LocalEthRxDma *pEthRxDma)
{
    xmit2FapMsg_t fapMsg;


    fapMsg.drvInit.cmd     = FAPMSG_CMD_INIT_RX;
    fapMsg.drvInit.channel = pEthRxDma->channel;
    fapMsg.drvInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvInit.numBds =  bufDescrs;

#if defined(ENET_RX_BDS_IN_PSM)
    /* If BDs in PSM, convert rxBds to FAP-based address - Apr 2010 */
    fapMsg.drvInit.Bds = (uint32)CONVERT_PSM_HOST2FAP(pEthRxDma->rxBds);
#else
    /* Remap to uncached kseg */
    fapMsg.drvInit.Bds = KSEG1ADDR(pEthRxDma->rxBds);
#endif

    /* Dma Ctrl registers require bit 29 to be set as well to read properly */
    fapMsg.drvInit.Dma = (uint32)(VIRT_TO_PHYS(pEthRxDma->rxDma)|0xA0000000);

    bcmPktDma_xmit2Fap(pEthRxDma->fapIdx, FAP_MSG_DRV_ENET_INIT, &fapMsg);

    return 1;

}

int	bcmPktDma_EthInitTxChan_Dqm(uint32 bufDescrs,
                                 BcmPktDma_LocalEthTxDma *pEthTxDma)
{
    xmit2FapMsg_t fapMsg;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(pEthTxDma->txOwnership == HOST_OWNED)
    {
        return( bcmPktDma_EthInitTxChan_Iudma(bufDescrs, pEthTxDma) );
    }
#endif

    fapMsg.drvInit.cmd     = FAPMSG_CMD_INIT_TX;
    fapMsg.drvInit.channel = pEthTxDma->channel;
    fapMsg.drvInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvInit.numBds =  bufDescrs;

#if defined(ENET_TX_BDS_IN_PSM)
    /* If BDs in PSM, convert txBds to FAP-based address - Apr 2010 */
    fapMsg.drvInit.Bds = (uint32)CONVERT_PSM_HOST2FAP(pEthTxDma->txBds);
#else
    /* Remap to uncached kseg */
    fapMsg.drvInit.Bds = KSEG1ADDR(pEthTxDma->txBds);
#endif

    /* Dma Ctrl registers require bit 29 to be set as well to read properly */
    fapMsg.drvInit.Dma = (uint32)(VIRT_TO_PHYS(pEthTxDma->txDma)|0xA0000000);

    bcmPktDma_xmit2Fap(pEthTxDma->fapIdx, FAP_MSG_DRV_ENET_INIT, &fapMsg);

    return 1;

}

#if defined(CONFIG_BCM_GMAC)
int	bcmPktDma_EthUnInitRxChan_Dqm(BcmPktDma_LocalEthRxDma *pEthRxDma)
{
    xmit2FapMsg_t fapMsg;


    fapMsg.drvInit.cmd     = FAPMSG_CMD_UNINIT_RX;
    fapMsg.drvInit.channel = pEthRxDma->channel;
    fapMsg.drvInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvInit.numBds =  0;
    fapMsg.drvInit.Dma = 0;

    bcmPktDma_xmit2Fap(pEthRxDma->fapIdx, FAP_MSG_DRV_ENET_UNINIT, &fapMsg);

    return 1;

}

int	bcmPktDma_EthUnInitTxChan_Dqm(BcmPktDma_LocalEthTxDma *pEthTxDma)
{
    xmit2FapMsg_t fapMsg;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(pEthTxDma->txOwnership == HOST_OWNED)
    {
        return( bcmPktDma_EthUnInitTxChan_Iudma(bufDescrs, pEthTxDma) );
    }
#endif

    fapMsg.drvInit.cmd     = FAPMSG_CMD_UNINIT_TX;
    fapMsg.drvInit.channel = pEthTxDma->channel;
    fapMsg.drvInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvInit.numBds =  0;
    fapMsg.drvInit.Bds = 0;
    fapMsg.drvInit.Dma = 0;

    bcmPktDma_xmit2Fap(pEthTxDma->fapIdx, FAP_MSG_DRV_ENET_UNINIT, &fapMsg);

    return 1;
}
#endif

#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
int	bcmPktDma_EthInitExtSw_Dqm(uint32 extSwConnPort)
{
    uint32 fapIdx = FAP0_IDX;
    xmit2FapMsg_t fapMsg;

    fapMsg.extSwInit.cmd     = FAPMSG_CMD_INIT_EXTSW;
    fapMsg.extSwInit.extSwConnPort = extSwConnPort;

    bcmPktDma_xmit2Fap(fapIdx, FAP_MSG_DRV_ENET_INIT, &fapMsg);
    fapIdx = FAP1_IDX;
    bcmPktDma_xmit2Fap(fapIdx, FAP_MSG_DRV_ENET_INIT, &fapMsg);

    return 1;
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthSelectRxIrq
 Purpose: Return IRQ number to be used for bcmPkt Rx on a specific channel
   Notes: Interrupt ID returned is 0.
          Handling of INTERRUPT_ID_FAP is installed in fapDriver instead
-------------------------------------------------------------------------- */
int	bcmPktDma_EthSelectRxIrq_Dqm(int channel)
{

    return(0);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthClrRxIrq
 Purpose: Clear the Rx interrupt for a specific channel
-------------------------------------------------------------------------- */
void	bcmPktDma_EthClrRxIrq_Dqm(BcmPktDma_LocalEthRxDma *rxdma)
{

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
    uint32 qbits = ((1 << DQM_FAP2HOST_ETH_RX_Q_LOW ) | 
						(1 << DQM_FAP2HOST_ETH_RX_Q_HI ));
#else
    uint32 qbits = (1 << DQM_FAP2HOST_ETH_RX_Q_LOW );  
#endif

    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel: %d\n", rxdma->channel);


    dqmClearNotEmptyIrqStsHost(rxdma->fapIdx, qbits);
    bcmPktDma_dqmEnableNotEmptyIrq(rxdma->fapIdx, qbits);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthTxEnable_Dqm
 Purpose: Coordinate with FAP to enable tx channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_EthTxEnable_Dqm( BcmPktDma_LocalEthTxDma * txdma )
{
    xmit2FapMsg_t fapMsg;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        return( bcmPktDma_EthTxEnable_Iudma(txdma) );
    }
#endif

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_TX_ENABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvCtl.channel = txdma->channel;

    return( bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg) );
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthTxDisable_Dqm
 Purpose: Coordinate with FAP to disable tx channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_EthTxDisable_Dqm( BcmPktDma_LocalEthTxDma *  txdma )
{
    xmit2FapMsg_t fapMsg;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        return( bcmPktDma_EthTxDisable_Iudma(txdma) );
    }
#endif

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_TX_DISABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvCtl.channel = txdma->channel;

    return( bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg) );
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRxEnable_Dqm
 Purpose: Coordinate with FAP to enable rx
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_EthRxEnable_Dqm( BcmPktDma_LocalEthRxDma * rxdma )
{
    xmit2FapMsg_t fapMsg;
    int retVal;

	
    /* Enable rx pkt processing */
#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
    bcmPktDma_dqmHandlerEnableHost(1 << (DQM_FAP2HOST_ETH_RX_Q_HI));
#endif

    bcmPktDma_dqmHandlerEnableHost(1 << (DQM_FAP2HOST_ETH_RX_Q_LOW));

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_RX_ENABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvCtl.channel = rxdma->channel;

    retVal = bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Execute the DMA enable code on Host side */
    //bcmPktDma_EthRxEnable_Iudma(rxdma);

    return( retVal );
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRxDisable_Dqm
 Purpose: Coordinate with FAP to disable rx
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_EthRxDisable_Dqm( BcmPktDma_LocalEthRxDma * rxdma )
{
    xmit2FapMsg_t fapMsg;
    int           retVal;


    fapMsg.drvCtl.cmd     = FAPMSG_CMD_RX_DISABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_ENET;
    fapMsg.drvCtl.channel = rxdma->channel;

    retVal = bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Execute the DMA enable code on Host side */
    //bcmPktDma_EthRxDisable_Iudma(rxdma);

    return( retVal );
}

/* This should be moved to bcmenet.c */
void bcm63xx_enet_dqmhandler(uint32 fapIdx, unsigned long unused)
{
    BCMENET_WAKEUP_RXWORKER(g_pEnetDevCtrl);
}

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int	bcmPktDma_EthSetIqThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh)
{
    xmit2FapMsg_t fapMsg;


    rxdma->iqLoThreshDqm = loThresh;
    rxdma->iqHiThreshDqm = hiThresh;
    rxdma->iqDroppedDqm  = 0;

    fapMsg.threshInit.cmd     = FAPMSG_CMD_SET_IQ_THRESH;
    fapMsg.threshInit.channel = rxdma->channel;
    fapMsg.threshInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.threshInit.loThresh= loThresh;
    fapMsg.threshInit.hiThresh= hiThresh;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_IQ, &fapMsg);

    return 1;
}

int	bcmPktDma_EthSetIqDqmThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh)
{
    xmit2FapMsg_t fapMsg;


    fapMsg.threshInit.cmd     = FAPMSG_CMD_SET_IQ_DQM_THRESH;
    fapMsg.threshInit.channel = rxdma->channel;
    fapMsg.threshInit.drv     = FAPMSG_DRV_ENET;
    fapMsg.threshInit.loThresh= loThresh;
    fapMsg.threshInit.hiThresh= hiThresh;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_IQ, &fapMsg);

    return 1;
}
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_EthSetRxChanBpmThresh_Dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                         uint16 allocTrig,
                                         uint16 bulkAlloc )
{
    xmit2FapMsg_t fapMsg;


    fapMsg.rxThresh.cmd     = FAPMSG_CMD_SET_RX_BPM_THRESH;
    fapMsg.rxThresh.channel = rxdma->channel;
    fapMsg.rxThresh.drv     = FAPMSG_DRV_ENET;
    fapMsg.rxThresh.allocTrig = allocTrig;
    fapMsg.rxThresh.bulkAlloc = bulkAlloc;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_BPM, &fapMsg);

    return 1;
}


int	bcmPktDma_EthSetTxChanBpmThresh_Dqm( BcmPktDma_LocalEthTxDma * txdma,
                                         uint16 *txDropThr )
{
    xmit2FapMsg_t fapMsg;
    int q;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if (txdma->txOwnership == HOST_OWNED)
    {
        return (bcmPktDma_EthSetTxChanBpmThresh_Iudma(txdma, txDropThr) );
    }
#endif

    fapMsg.txDropThr.cmd     = FAPMSG_CMD_SET_BPM_ETH_TXQ_THRESH;
    fapMsg.txDropThr.channel = txdma->channel;
    fapMsg.txDropThr.drv     = FAPMSG_DRV_ENET;
    /* Q0 and Q1 have the same prio and should have same thresh */
    /* Ignoring Q0 thresh because it can be derived from Q1 */
    for (q=1; q < ENET_TX_EGRESS_QUEUES_MAX; q++)
        fapMsg.txDropThr.thr[q-1] = *(txDropThr+ q);

    bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_BPM, &fapMsg);

    return 1;
}
#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthSetPhyRate_Dqm
 Purpose: Informs the PHY rate of a given Ethernet port to the FAP
  Return: N/A
-------------------------------------------------------------------------- */
void bcmPktDma_EthSetPhyRate_Dqm(uint8 port, uint8 enable, int kbps, int isWanPort)
{
#if defined(CC_FAP4KE_TM)
    bcmPktDma_tmPortType(port, (isWanPort) ? FAP_TM_PORT_TYPE_WAN : FAP_TM_PORT_TYPE_LAN);

    bcmPktDma_tmPortConfig(port, FAP_TM_MODE_AUTO, kbps, 2000, FAP_TM_SHAPING_TYPE_DISABLED);

    bcmPktDma_tmPortEnable(port, FAP_TM_MODE_AUTO, enable);

    bcmPktDma_tmApply(port);
#endif
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthSetPauseEn_Dqm
 Purpose: Enables pause for a given port 
  Return: N/A
-------------------------------------------------------------------------- */
void bcmPktDma_EthSetPauseEn_Dqm(uint8 port, uint8 enable)
{
#if defined(CC_FAP4KE_TM)
    bcmPktDma_tmPauseEnable(port, enable);
    bcmPktDma_tmApply(port);
#endif
}

void bcmPktDma_EthGetStats_Dqm(uint8 port, uint32 *rxDrop_p, uint32 *txDrop_p)
{
    *rxDrop_p = 0;
    *txDrop_p = 0;

    *rxDrop_p += pHostQsmGbl(FAP0_IDX)->enet[port].rxDropped;
    *txDrop_p += pHostQsmGbl(FAP0_IDX)->enet[port].txDropped;

#if NUM_FAPS > 1
    *rxDrop_p += pHostQsmGbl(FAP1_IDX)->enet[port].rxDropped;
    *txDrop_p += pHostQsmGbl(FAP1_IDX)->enet[port].txDropped;
#endif
}

void bcmPktDma_EthResetStats_Dqm(uint8 port)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.stats.cmd     = FAPMSG_CMD_DRV_RESET_STATS;
    fapMsg.stats.port    = port;
    fapMsg.stats.drv     = FAPMSG_DRV_ENET;

    bcmPktDma_xmit2Fap(FAP0_IDX, FAP_MSG_STATS, &fapMsg);
#if NUM_FAPS > 1
    bcmPktDma_xmit2Fap(FAP1_IDX, FAP_MSG_STATS, &fapMsg);
#endif
}


EXPORT_SYMBOL(bcm63xx_enet_dqmhandler);

EXPORT_SYMBOL(bcmPktDma_EthInitRxChan_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthInitTxChan_Dqm);
#if defined(CONFIG_BCM_GMAC)
EXPORT_SYMBOL(bcmPktDma_EthUnInitRxChan_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthUnInitTxChan_Dqm);
#endif
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
EXPORT_SYMBOL(bcmPktDma_EthInitExtSw_Dqm);
#endif
EXPORT_SYMBOL(bcmPktDma_EthSelectRxIrq_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthClrRxIrq_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthTxEnable_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthTxDisable_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthRxEnable_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthRxDisable_Dqm);
#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
EXPORT_SYMBOL(bcmPktDma_EthSetIqThresh_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthSetIqDqmThresh_Dqm);
#endif
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
EXPORT_SYMBOL(bcmPktDma_EthSetRxChanBpmThresh_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthSetTxChanBpmThresh_Dqm);
#endif
EXPORT_SYMBOL(bcmPktDma_EthSetPhyRate_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthSetPauseEn_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthGetStats_Dqm);
EXPORT_SYMBOL(bcmPktDma_EthResetStats_Dqm);

#endif /* CONFIG_BCM963268 */
