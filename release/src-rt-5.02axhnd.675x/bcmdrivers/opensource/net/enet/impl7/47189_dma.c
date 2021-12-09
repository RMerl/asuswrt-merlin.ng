/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
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
 * BCM47189 DMA driver
 */

#include "enet_dbg.h"
#include "port.h"
#include <board.h>
#include "bcm_map_part.h"
#include "bcmenet_common.h"
#include "bcmenet_dma.h"
#include "dma.h"

#include <linux/gbpm.h>
#include "bpm.h"


/********** External functions **********/
extern DmaDesc *bcmPktDma_EthAllocTxBds(struct device *dev, int channel,
                                        int numBds, uint32 *phy_addr);
extern DmaDesc *bcmPktDma_EthAllocRxBds(struct device *dev, int channel,
                                        int numBds, uint32 *phy_addr);
extern int bcmPktDma_EthSetRxChanBpmThresh_Iudma(BcmPktDma_LocalEthRxDma * rxdma,
                                        uint16 allocTrig,
                                        uint16 bulkAlloc);
extern int bcmPktDma_EthBpmSetTxChanThresh_Iudma(BcmPktDma_LocalEthTxDma * txdma,
                                        uint16 *dropThr);
extern int bcmPktDma_EthGetRxBds(BcmPktDma_LocalEthRxDma *rxdma, int channel);
extern int bcmPktDma_EthGetTxBds(BcmPktDma_LocalEthTxDma *txdma, int channel);


/********** Type definitions and macros **********/

/* Recycling context definition */
typedef union {
    struct {
        uint32 reserved     : 30;
        uint32 channel      :  2;
    };
    uint32 u32;
} enet_recycle_context_t;

#define MAX_NUMBER_OF_PORTS 2

#define RECYCLE_CONTEXT(_context)  ( (enet_recycle_context_t *)(&(_context)) )
/*
 * Recycling context definition
 */
#define DELAYED_RECLAIM_ARRAY_LEN 8

#define DO_DELAYED_RECLAIM(arrayIdx, arrayPtr) \
    do { \
        uint32 tmp_idx=0; \
        while (tmp_idx < (arrayIdx)) { \
            nbuff_free((pNBuff_t) (arrayPtr)[tmp_idx]); \
            tmp_idx++; } \
    } while (0)


/* TX Buffer sources supported by bcmPktDma Lib */
enum {
    HOST_VIA_LINUX=0,
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

struct enet_xmit_params {
    enet_xmit_params_base;
    uint32 dqm;
    DmaDesc dmaDesc;
    BcmPktDma_EthTxDma *txdma;
    int channel;
    FkBuff_t *pFkb;
    struct sk_buff *skb;
    uint32 reclaim_idx;
    uint32 delayed_reclaim_array[DELAYED_RECLAIM_ARRAY_LEN];
};

/********** External variables **********/
/* From bcmPktDmaBds.c: used to retrieve the number of allocated TX BDs */
extern BcmPktDma_Bds *bcmPktDma_Bds_p;

/********** Private variables **********/
/*
 * TODO [FUTURE OPTIMIZATION]: Driver-managed skb pool cache
 *static int skbcache_init;
 */
static void * eth_alloc_buf_addr[BPM_ENET_BULK_ALLOC_COUNT];

static enetx_port_t *dma_ports[MAX_NUMBER_OF_PORTS];

/********** Public variables **********/
//struct kmem_cache *enetSkbCache;


/********** Function prototypes **********/



/********** Function definitions **********/

/***** BcmPktDma function overrides *****/


/*
 * bcmPktDma_EthFreeRecvBuf
 *
 * Assigns a single RX buffer to an RX DMA
 *
 * PARAMETERS
 * rxdma: RX DMA structure that gets the buffer
 * pBuf: Free buffer to be assigned
 *
 * RETURN
 * 0: If the buffer could be assigned to the RX DMA ring
 * 1: If the DMA ring can't hold any more free buffers (if it was already
 *    completely replenished)
 *
 * NOTES
 * 47189-specific version. Modified from the original in
 * bcmdrivers/opensource/include/bcm963xx/bcmPktDma.h
 */
static inline int bcmPktDma_EthFreeRecvBuf(BcmPktDma_LocalEthRxDma *rxdma,
                                           unsigned char *pBuf)
{
    volatile DmaDesc *rxBd;
    int tail_idx;

    if(rxdma->rxAssignedBds == rxdma->numRxBds) {
        enet_err("rxAssignedBds(%d) == numRxBds(%d)", rxdma->rxAssignedBds,
                 rxdma->numRxBds);
        return -1;
    }

    tail_idx = rxdma->rxTailIndex;
    rxBd = &rxdma->rxBds[tail_idx];

    /* Format Buffer Descriptors */

    /* All BDs point to buffers of BCM_MAX_PKT_LEN */
    rxBd->ctrl2 = BCM_MAX_PKT_LEN;

    /* Assume 1 BD per packet: All BDs have SOF bit enabled */
    if (tail_idx == (rxdma->numRxBds - 1)) {
        /* Last BD in the ring, set the End Of Table bit in it */
        rxBd->ctrl1 = DMA_CTRL1_SOF | DMA_CTRL1_EOT;
        rxdma->rxTailIndex = 0;
    } else {
        rxBd->ctrl1 = DMA_CTRL1_SOF;
        rxdma->rxTailIndex = tail_idx + 1;
    }

    /* Assign the buffer to the BD */
    rxBd->addrhigh = 0;
    rxBd->addrlow = (uint32)VIRT_TO_PHYS(pBuf);

    rxdma->rxAssignedBds++;

    return 0;
}


/*
 * bcmPktDma_EthRecvBufGet
 *
 * Get the buffer pointed to by the head index of an RX DMA ring
 *
 * PARAMETERS
 * rxdma: RX DMA structure (ring) to get the buffer from.
 * pRxAddr (output): Pointer to the data buffer pointed by the head BD
 *
 * NOTES
 * 47189-specific version. Modified from the original in
 * bcmdrivers/opensource/include/bcm963xx/bcmPktDma.h
 */
static inline BOOL bcmPktDma_EthRecvBufGet(BcmPktDma_LocalEthRxDma * rxdma,
                                           uint8 **pRxAddr)
{
    volatile DmaDesc *rxBd;
    int head_idx;

    if (rxdma->rxAssignedBds == 0)
        return FALSE;

    head_idx = rxdma->rxHeadIndex;
    rxBd = &rxdma->rxBds[head_idx];

    rxBd->ctrl2 = BCM_MAX_PKT_LEN;
    rxBd->ctrl1 = DMA_CTRL1_SOF;

    if (head_idx == (rxdma->numRxBds - 1)) {
        rxBd->ctrl1 |= DMA_CTRL1_EOT;
        rxdma->rxHeadIndex = 0;
    } else {
        rxdma->rxHeadIndex = head_idx + 1;
    }
    *pRxAddr = (uint8 *)(phys_to_virt(rxBd->addrlow));
    rxdma->rxAssignedBds--;

    return TRUE;
}


/*
 * bcmPktDma_EthRecv
 *
 * Receive a packet on a specific RX DMA
 *
 * PARAMETERS
 * rxdma: Receiving RX DMA structure
 * pBuf (output): Received data buffer
 * pLen (output): Received data length in bytes
 *
 * RETURN
 * 0: If there was a buffer to receive
 * 1: If there wasn't
 *
 * NOTES
 * 47189-specific version. Modified from the original in
 * bcmdrivers/opensource/include/bcm963xx/bcmPktDma.h
 */
static inline uint32 bcmPktDma_EthRecv(BcmPktDma_LocalEthRxDma * rxdma,
                                       unsigned char **pBuf, int * pLen)
{
    unsigned int bd_offset = (&rxdma->rxBds[rxdma->rxHeadIndex] - rxdma->rxBds)
                                * sizeof(DmaDesc);
    unsigned int status0 = rxdma->rxDma->status0;

    if (rxdma->rxAssignedBds != 0) {
        /*
         * Check if the buffer pointed by the BD was completely received.
         * In the 47189 DMAs the buffer was completely transferred if the
         * DMA_CURRENT_DESCR field of status0 has advanced to the next BD.
         */
        if ((status0 & DMA_CURRENT_DESCR) != bd_offset) {
            *pBuf = (unsigned char *)
                (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].addrlow));

            /* Packet length is stored in the first two bytes */
            *pLen = ((*pBuf)[0] | ((*pBuf)[1] << 8));
            *pBuf += 4;

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

            rxdma->rxAssignedBds--;
        }
        else {
            return -1;
        }
    }
    else {
        /* out of buffers! */
        return -1;
    }
    return 0;
}


/*
 * bcmPktDma_EthXmit
 *
 * Transmit a buffer, checking for available space in the TX ring before.
 *
 * PARAMETERS
 * txdma: TX DMA used for the transmission
 * pBuf: Data buffer to transmit
 * len: Data buffer length in bytes
 *
 * RETURN
 * 0: If the transmission was correctly programmed.
 * 1: If the TX ring doesn't have any free BDs for the transmission.
 *
 * NOTES
 * 47189-specific version. Modified from the original in
 * bcmdrivers/opensource/include/bcm963xx/bcmPktDma.h
 */
static inline int bcmPktDma_EthXmit(BcmPktDma_LocalEthTxDma * txdma,
                                    uint8 *pBuf, uint16 len, uint32 key)
{
    int txIndex = txdma->txTailIndex;
    volatile DmaDesc *txBd;
    BcmPktDma_txRecycle_t *txRecycle_p = &txdma->txRecycle[txIndex];

    if(txdma->txFreeBds) {
        txRecycle_p->key = key;
        /* Decrement total BD count */
        txdma->txFreeBds--;

        /* advance BD pointer to next in the chain. */
        if (txIndex == (txdma->numTxBds - 1))
            txdma->txTailIndex = 0;
        else
            txdma->txTailIndex++;

        /* Format the BD for transmission */
        txBd = &txdma->txBds[txIndex];
        //txBd->ctrl1 |= DMA_CTRL1_SOF | DMA_CTRL1_EOF;
        txBd->ctrl2 = len;
        txBd->addrlow = (uint32)VIRT_TO_PHYS(pBuf);
        txBd->addrhigh = 0;

        /*
         * This memory barrier is crucial for ARM platforms to ensure the BD is
         * correctly formatted before it's posted.
         */
        wmb();

        /*
         * Post the BD to notify the DMA the BD the transfer is ready
         * (advance ptr to the next BD)
         */
        if (txIndex == (txdma->numTxBds - 1))
            txdma->txDma->ptr = (unsigned int)(txdma->txBdsPhysBase);
        else
            txdma->txDma->ptr += sizeof(DmaDesc);

        /* Enable DMA for this channel */
        if(txdma->txEnabled)
            txdma->txDma->control |= DMA_EN;

        return 0;
    }

    return 1;
}


/*
 * bcmPktDma_EthFreeXmitBufGet
 *
 * Get an already transmitted buffer from a TX DMA ring for recycling. This only
 * retrieves the buffer, the caller is supposed to use this function to locate a
 * buffer and then recycle it.
 *
 * If there is more than one transmitted buffer, this gets the oldest one in the
 * ring. In other words, it looks for the buffer pointed by the head BD.
 *
 * RETURN
 *
 * NOTES
 * 47189-specific version. Modified from the original in
 * bcmdrivers/opensource/include/bcm963xx/bcmPktDma.h
 */
static inline BcmPktDma_txRecycle_t *bcmPktDma_EthFreeXmitBufGet(BcmPktDma_LocalEthTxDma *txdma)
{
    if(txdma->txFreeBds < txdma->numTxBds) {
        int bdIndex = txdma->txHeadIndex;
        int dma_status = txdma->txDma->status0;
        unsigned int bdAddress = txdma->txDma->addrlow + &txdma->txBds[bdIndex]
                                - txdma->txBds;

        /*
         * Check if there are recyclable buffers.
         * For 47189 that means checking that the BD pointed by txHeadIndex is:
         *
         * a) Before the current processed BD (status0 & DMA_CURRENT_DESCR)
         *    if the DMA hasn't wrapped yet.
         * b) After the current processed BD if the DMA has wrapped but the
         *    txHeadIndex didn't.
         */
        if ((bdAddress & DMA_CURRENT_DESCR) != (dma_status & DMA_CURRENT_DESCR)) {
            if (++txdma->txHeadIndex == txdma->numTxBds)
                txdma->txHeadIndex = 0;
            txdma->txFreeBds++;
            return (&txdma->txRecycle[bdIndex]);
        }
    }

    return NULL;
}



/*
 * bcmeapi_free_skb
 *
 * If the SKB_RECYCLE flag is set, it frees the skb into the enetSkbCache. If
 * the skb was already a part of the pool, it'll simply be put in the list of
 * free skbs and the freeing is optimized out.
 *
 * NOTES
 * Currently, the enet skb pool cache is not implemented, so this always returns
 * BCMEAPI_CTRL_FALSE to skip.
 */
static inline int bcmeapi_free_skb(BcmEnet_devctrl *pDevCtrl,
                struct sk_buff *skb, int free_flag, int channel)
{
    return BCMEAPI_CTRL_FALSE;

#if 0
    /*
     * TODO [FUTURE OPTIMIZATION]: Driver-managed skb pool cache
     */

    BcmEnet_RxDma *rxdma;
    /*
    unsigned int is_bulk_rx_lock_active;
    uint32 cpuid;
    */

    if(!(free_flag & SKB_RECYCLE))
        return BCMEAPI_CTRL_FALSE;

    /*
     * Disable preemption so that my cpuid will not change in this func.
     * Not possible for the state of bulk_rx_lock_active to change
     * underneath this function on the same cpu.
     */
    preempt_disable();
    /*
    cpuid =  smp_processor_id();
    is_bulk_rx_lock_active = pDevCtrl->bulk_rx_lock_active[cpuid];
    */

    rxdma = pDevCtrl->rxdma[channel];
    if ((unsigned char *)skb < rxdma->skbs_p || (unsigned char *)skb >= rxdma->end_skbs_p) {
        enet_err("kmem_cache_free\n");
        kmem_cache_free(enetSkbCache, skb);
    }
    else {
        enet_err("next free\n");
        skb->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = skb;
    }

    preempt_enable();
    return BCMEAPI_CTRL_TRUE;
#endif
}



static int init_txdma_structures(enetx_port_t *port)
{
    BcmPktDma_EthTxDma *txdma;
    BcmEnet_devctrl *devctrl = port->priv;

    devctrl->txdma[0] = (BcmPktDma_EthTxDma *)
                        kzalloc(sizeof(BcmPktDma_EthTxDma), GFP_KERNEL);
    if (devctrl->txdma[0] == NULL) {
        enet_err("Unable to allocate memory for TX DMA ring\n");
        return -1;
    }
    txdma = devctrl->txdma[0];

    /* Init the number of Tx BDs in each tx ring */
    txdma->numTxBds = bcmPktDma_EthGetTxBds(txdma, port->p.port_id);

    return 0;
}

static int init_rxdma_structures(enetx_port_t *port)
{
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *devctrl = port->priv;

    /* init rx dma channel structures */
    devctrl->rxdma[0] = (BcmEnet_RxDma *)
                        kzalloc(sizeof(BcmEnet_RxDma), GFP_KERNEL);
    if (devctrl->rxdma[0] == NULL) {
        enet_err("Unable to allocate memory for RX DMA ring\n");
        return -1;
    }
    rxdma = devctrl->rxdma[0];

    /* init number of Rx BDs in each rx ring. */
    rxdma->pktDmaRxInfo.numRxBds = bcmPktDma_EthGetRxBds(&rxdma->pktDmaRxInfo, 0);

    return 0;
}


/*
 * is_rx_empty
 *
 * Returns wether a certain RX ring has pending unprocessed buffers or not.
 *
 * RETURN
 * 0: If there is a pending received buffer to process
 * 1: If there isn't
 */
int is_rx_empty(int rx_channel)
{
    BcmEnet_devctrl *devctrl = dma_ports[rx_channel]->priv;
    BcmPktDma_LocalEthRxDma rxdma = devctrl->rxdma[0]->pktDmaRxInfo;
    unsigned int bd_offset = (&rxdma.rxBds[rxdma.rxHeadIndex] - rxdma.rxBds)
                                * sizeof(DmaDesc);

    if(bd_offset != (rxdma.rxDma->status0 & DMA_CURRENT_DESCR))
        return 0;
    else
        return 1;
}


static int enet_bpm_alloc_buf_ring(enetx_port_t *port, int channel, uint32 num)
{
    unsigned char *pFkBuf, *pData;
    uint32 context = 0;
    uint32 buf_ix;
    BcmEnet_devctrl *devctrl = port->priv;

    RECYCLE_CONTEXT(context)->channel = channel;

    for (buf_ix=0; buf_ix < num; buf_ix++) {
        if ((pData = (uint8_t *) gbpm_alloc_buf()) == NULL)
            return GBPM_ERROR;

        pFkBuf = PDATA_TO_PFKBUFF(pData, BCM_PKT_HEADROOM);

        /* Place a FkBuff_t object at the head of pFkBuf */
        //fkb_preinit(pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle, context);

        cache_flush_region(pData, (uint8_t*)pFkBuf + BCM_PKTBUF_SIZE);
        bcmPktDma_EthFreeRecvBuf(&devctrl->rxdma[channel]->pktDmaRxInfo, pData);
    }

    return GBPM_SUCCESS;
}


static void enet_bpm_free_buf_ring(enetx_port_t *port, int channel)
{
    int i;
    BcmEnet_devctrl *devctrl = port->priv;
    BcmEnet_RxDma *rxdma = devctrl->rxdma[channel];

    uint8 *rxAddr=NULL;

    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++) {
        if (bcmPktDma_EthRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE) {
            if (rxAddr != NULL)
                gbpm_free_buf((void *)rxAddr);
        }
    }

    gbpm_unresv_rx_buf(GBPM_PORT_ETH, port->p.port_id);
}



static void setup_txdma_channel(enetx_port_t *port)
{
    BcmPktDma_EthTxDma *txdma;
    BcmEnet_devctrl *devctrl = port->priv;
    /* txdma[0] because there's only one TX channel */
    txdma = devctrl->txdma[0];

    /*
     * 1 - Keep the default values for burstlen, prefetch_ctrl,
     *     prefetch_trhreshold, and multiple_outstanding_reads.
     * 2 - Configure the TX BD ring base address.
     * 2 - Enable TX channel.
     */
    txdma->txDma->addrhigh = 0;
    txdma->txDma->addrlow = (unsigned int)(txdma->txBdsPhysBase);
    txdma->txDma->control |= DMA_EN;

    /* Configure DMA controller options */
    txdma->txDma->control |= (DMA_PTY_CHK_DISABLE
                              | (1 << DMA_MULTIPLE_OUTSTANDING_READS_SHIFT)
                              | (2 << DMA_BURST_LEN_SHIFT)
                              | (3 << DMA_PREFETCH_CTL_SHIFT)
                              | (3 << DMA_PREFETCH_THRESH_SHIFT));

    /* Start with an empty descriptor table (ptr = addrlow) */
    txdma->txDma->ptr = txdma->txDma->addrlow;
}


static void setup_rxdma_channel(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    BcmEnet_RxDma *rxdma = devctrl->rxdma[0];

    rxdma->pktDmaRxInfo.rxDma->addrhigh = 0;
    rxdma->pktDmaRxInfo.rxDma->addrlow = (unsigned int)(rxdma->pktDmaRxInfo.rxBdsPhysBase);
    rxdma->pktDmaRxInfo.rxDma->control |= DMA_EN;
    rxdma->pktDmaRxInfo.rxDma->control |= (DMA_OVERFLOW_CONTINUE
                                           | DMA_PTY_CHK_DISABLE
                                           | (2 << DMA_PREFETCH_CTL_SHIFT)
                                           | (3 << DMA_BURST_LEN_SHIFT));

    rxdma->pktDmaRxInfo.rxDma->ptr = (unsigned int)(rxdma->pktDmaRxInfo.rxBdsPhysBase) + (sizeof(DmaDesc) * 100);
}



static int init_rx_buffers(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    unsigned long BufsToAlloc;
    BcmEnet_RxDma *rxdma;
    uint32 context = 0;

    RECYCLE_CONTEXT(context)->channel = 0;

    /* allocate recieve buffer pool */
    rxdma = devctrl->rxdma[0];
    /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
    rxdma->pktDmaRxInfo.rxAssignedBds = 0;
    rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;
    BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

    if (enet_bpm_alloc_buf_ring(port, 0, BufsToAlloc) == GBPM_ERROR) {
        enet_err("Low memory\n");
        /* release all allocated receive buffers */
        enet_bpm_free_buf_ring(port, 0);
        return -ENOMEM;
    }

    return 0;
}


static int alloc_txdma_bds(enetx_port_t *port)
{
    BcmPktDma_EthTxDma *txdma;
    BcmEnet_devctrl *devctrl = port->priv;
    int nr_tx_bds;
    uint32_t phy_addr;
    int i;

    txdma = devctrl->txdma[0];
    nr_tx_bds = txdma->numTxBds;

    /* BDs allocated in bcmPktDma lib in PSM or in DDR */
    /* Second parameter is not used inside the function for 47189 */
    txdma->txBdsBase = (volatile DmaDesc *)
            bcmPktDma_EthAllocTxBds(&port->dev->dev, port->p.port_id, nr_tx_bds, &phy_addr);
    txdma->txBdsPhysBase = (volatile DmaDesc *)(uintptr_t)phy_addr;
    if (txdma->txBdsBase == NULL) {
        enet_err("Unable to allocate memory for Tx Descriptors\n");
        return -ENOMEM;
    }
    /* Assumption : allocated BDs are 16 Byte aligned */
    txdma->txRecycleBase = kmalloc(nr_tx_bds * sizeof(BcmPktDma_txRecycle_t) +
                                   BCM_DCACHE_LINE_LEN, GFP_ATOMIC);
    if (txdma->txRecycleBase !=NULL) {
        memset(txdma->txRecycleBase, 0,
               nr_tx_bds * sizeof(BcmPktDma_txRecycle_t) + BCM_DCACHE_LINE_LEN);
    }

    txdma->txRecycle = (BcmPktDma_txRecycle_t*)
                            (((uintptr_t)txdma->txRecycleBase + BCM_DCACHE_ALIGN_LEN)
                             & ~BCM_DCACHE_ALIGN_LEN);

    txdma->txBds = txdma->txBdsBase;

    for (i = 0; i < nr_tx_bds; i++)
        txdma->txBds[i].ctrl1 |= DMA_CTRL1_SOF | DMA_CTRL1_EOF;
    txdma->txBds[nr_tx_bds - 1].ctrl1 |= DMA_CTRL1_EOT;

    txdma->txFreeBds = nr_tx_bds;
    txdma->txHeadIndex = txdma->txTailIndex = 0;
    nr_tx_bds = txdma->numTxBds;

    return 0;
}


static int alloc_rxdma_bds(enetx_port_t *port)
{
    BcmEnet_RxDma *rxdma;
    uint32 phy_addr;
    BcmEnet_devctrl *devctrl = port->priv;

    rxdma = devctrl->rxdma[0];

    rxdma->pktDmaRxInfo.rxBdsBase = (volatile DmaDesc *)
                        bcmPktDma_EthAllocRxBds(&port->dev->dev, 0,
                        rxdma->pktDmaRxInfo.numRxBds, &phy_addr);

    if (rxdma->pktDmaRxInfo.rxBdsBase == NULL) {
        enet_err("Unable to allocate memory for Rx Descriptors\n");
        return -ENOMEM;
    }
    rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)
                        (((uintptr_t)rxdma->pktDmaRxInfo.rxBdsBase
                        + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN);
    rxdma->pktDmaRxInfo.rxBdsPhysBase = (volatile DmaDesc *)(uintptr_t)phy_addr;

    /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
    rxdma->pktDmaRxInfo.rxAssignedBds = 0;
    rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

    return 0;
}



static int init_tx_channel(enetx_port_t *port)
{
    BcmPktDma_EthTxDma *txdma;
    BcmEnet_devctrl *devctrl = port->priv;
    volatile DmaRegs *dmaCtrl = devctrl->dmaCtrl;

    txdma = devctrl->txdma[0];
    txdma->txDma = &dmaCtrl->dmaxmt;

    /* Reset the DMA channel */
    txdma->txDma->control &= ~DMA_EN;

    /* allocate and assign tx buffer descriptors */
    if (alloc_txdma_bds(port) < 0) {
        enet_err("Allocation of TX BDs Failed\n");
        return -1;
    }
    setup_txdma_channel(port);
    devctrl->default_txq = 0;

    return 0;
}


static void enet_rx_set_bpm_alloc_trig(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    BcmPktDma_EthRxDma *rxdma = &devctrl->rxdma[0]->pktDmaRxInfo;
    uint32 allocTrig = rxdma->numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100;

    bcmPktDma_EthSetRxChanBpmThresh_Iudma(rxdma, allocTrig,
                                          BPM_ENET_BULK_ALLOC_COUNT);
}


static int init_rx_channel(enetx_port_t *port)
{
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *devctrl = port->priv;
    volatile DmaRegs *dmaCtrl = devctrl->dmaCtrl;
    //int phy_chan = 0;

    /* setup the RX DMA channel */
    rxdma = devctrl->rxdma[0];

    /* init rxdma structures */
    rxdma->pktDmaRxInfo.rxDma = &dmaCtrl->dmarcv;

    /* Reset the DMA channel */
    rxdma->pktDmaRxInfo.rxDma->control &= ~DMA_EN;

    /* allocate RX BDs */
    if (alloc_rxdma_bds(port) < 0)
        return -1;

    enet_rx_set_bpm_alloc_trig(port);

    /* initialize the receive buffers */
    if (init_rx_buffers(port)) {
        enet_err("Low memory.\n");
        enet_bpm_free_buf_ring(port, 0);
        return -ENOMEM;
    }
    gbpm_resv_rx_buf(GBPM_PORT_ETH, port->p.port_id, rxdma->pktDmaRxInfo.numRxBds,
                (rxdma->pktDmaRxInfo.numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100));

    setup_rxdma_channel(port);

    return 0;
}



/*
 * bcmeapi_pkt_xmt_dispatch
 *
 * Minimum configuration before and after sending a buffer. The actual sending
 * is programmed in bcmPktDma_EthXmit
 */
static inline int bcmeapi_pkt_xmt_dispatch(EnetXmitParams *pParam)
{
    uint32 key;
    BcmPktDma_EthTxDma *txdma = pParam->txdma;

    key = (uint32)pParam->pNBuff;
    nbuff_flush(pParam->pNBuff, pParam->data, pParam->len);

    bcmPktDma_EthXmit(txdma, pParam->data, pParam->len, key);

#ifdef DEBUG_COUNTERS
    txdma->localstats.tx_pkts++;
#endif
    return BCMEAPI_CTRL_CONTINUE;
}


/*
 * bcmeapi_buf_reclaim
 *
 * Bulk-reclaim already sent buffers. Reclaiming is only done at this point if
 * there are more than DELAYED_RECLAIM_ARRAY_LEN unreclaimed buffers.
 * Otherwise, the buffers are reclaimed after the transmission (out of this
 * function).
 */
static inline void bcmeapi_buf_reclaim(EnetXmitParams *pParam)
{
    BcmPktDma_txRecycle_t *txRecycle_p;

    //ENET_TX_LOCK();

    while((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(pParam->txdma)) != NULL) {
        pParam->delayed_reclaim_array[pParam->reclaim_idx] = txRecycle_p->key;

        pParam->reclaim_idx++;
        /*
         * only unlock and do reclaim if we have collected many free
         * buffers, otherwise, wait until end of transmission when we have
         * already released the tx lock to do reclaim.
         */
        if (pParam->reclaim_idx >= DELAYED_RECLAIM_ARRAY_LEN) {
            //ENET_TX_UNLOCK();
            DO_DELAYED_RECLAIM(pParam->reclaim_idx, pParam->delayed_reclaim_array);
            pParam->reclaim_idx = 0;
            //ENET_TX_LOCK();
        }
    }
}


/*
 * dma_tx
 *
 * Transmits a network buffer through a port.
 *
 * Looks for reclaimable buffers before sending and reclaims them before sending
 * (if there are at least a certain amount of reclaimable buffers) or after the
 * transfer has been programmed (if there aren't).
 *
 * Delegates the transmission programming to bcmeapi_pkt_xmt_dispatch
 */
int dma_tx(enetx_port_t *port, pNBuff_t pNBuff, int channel)
{
    BcmEnet_devctrl *devctrl = port->priv;
    EnetXmitParams params;

    params.pDevPriv = devctrl;
    params.vstats = &params.pDevPriv->stats;
    params.pNBuff = pNBuff;
    params.txdma = devctrl->txdma[0];
    params.channel = port->p.port_id;

    //enet_err("Channel %d\n", channel);

    if (nbuff_get_params_ext(pNBuff, &params.data, &params.len,
                             &params.mark, &params.priority,
                             &params.r_flags) == NULL) {
        return 0;
    }

    params.skb = PNBUFF_2_SKBUFF(params.pNBuff);
    if (IS_FKBUFF_PTR(params.pNBuff))
        params.pFkb = PNBUFF_2_FKBUFF(params.pNBuff);

    /********** START of TX critical section **********/
    /* bcmeapi_buf_reclaim takes the TX lock */
    bcmeapi_buf_reclaim(&params);
    bcmeapi_pkt_xmt_dispatch(&params);

    //ENET_TX_UNLOCK();
    DO_DELAYED_RECLAIM(params.reclaim_idx, params.delayed_reclaim_array);

    /********** END of TX critical section **********/

    return 0;
}



int dma_init(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    int retval = 0;

    if (port->p.port_id == 0) {
        devctrl->dmaCtrl = (DmaRegs *)ENET_CORE0_DMA;
    } else if (port->p.port_id == 1) {
        devctrl->dmaCtrl = (DmaRegs *)ENET_CORE1_DMA;
    } else {
        enet_err("Invalid Ethernet core number in port_id (%d)\n",
                 port->p.port_id);
        return 1;
    }

    /*
     * Save the port in a global place so it can be accessible from the RX path
     * functions, which don't have any information about the rx "port" in their
     * context, but they know the receiving "channel". So what we do is to
     * establish a one-to-one relation between port and channel:
     * dma_ports[channel] = port
     */
    dma_ports[port->p.port_id] = port;

    dma_set_coherent_mask(&port->dev->dev, DMA_BIT_MASK(32));
    /* Allocate the TX and RX DMA data structures */
    retval = init_txdma_structures(port);
    if (retval < 0) {
        enet_err("Error initializing TX DMA data structures\n");
        return retval;
    }
    retval = init_rxdma_structures(port);
    if (retval < 0) {
        enet_err("Error initializing RX DMA data structures\n");
        return retval;
    }

    /* Allocate and format the TX buffer descriptors */
    retval = init_tx_channel(port);
    if (retval < 0) {
        enet_err("Error initializing the TX DMA buffer descriptors\n");
        return retval;
    }

    /* Allocate the RX buffers, allocate and format RX buffer descriptors */
    retval = init_rx_channel(port);
    if (retval < 0) {
        enet_err("Error initializing the RX DMA buffer descriptors\n");
        return retval;
    }

#if 0
    /* Initialize only one enetSkbCache for the driver */
    if (skbcache_init == 0) {
        /* create a slab cache for device descriptors */
        enetSkbCache = kmem_cache_create("bcm_EnetSkbCache",
                        BCM_SKB_ALIGNED_SIZE, 0, SLAB_HWCACHE_ALIGN, NULL);

        if(enetSkbCache == NULL) {
            printk(KERN_NOTICE "Eth: Unable to create skb cache\n");
            return -1;
        }
        skbcache_init = 1;
    }
#endif

    return 0;
}


/*
 * enet_bpm_free_buf
 *
 * Returns a buffer to BPM
 */
static inline int enet_bpm_free_buf(BcmEnet_devctrl *pDevCtrl, int channel,
                                    uint8 *pData)
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;
    gbpm_free_buf((void *)pData);
    rxdma->free++;

    return GBPM_SUCCESS;
}


/*
 * assign_rx_buffer
 *
 * Assigns a buffer to a DMA RX ring if it's not full. Otherwise it returns the
 * buffer back to BPM.
 *
 * PARAMETERS
 * pDevCtrl: BcmEnet_devctrl of the receiving port
 * channel: (Always 0)
 * pData: Buffer to assign
 */
static inline void assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                    uint8 * pData)
{
    BcmPktDma_LocalEthRxDma *pktDmaRxInfo_p = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;
    int buf_freed = 0;

    //ENET_RX_LOCK();

    /*
     * If the ring is completely replenished, give the buffer back to BPM. If
     * not, use it to fill the ring.
     */
    if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds) {
        buf_freed = 1;
        enet_bpm_free_buf(pDevCtrl, channel, pData);
    }
    if (buf_freed == 0) {
        bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
    }

    //ENET_RX_UNLOCK();
}


/*
 * flush_assign_rx_buffer
 *
 * Cache-invalidates and assign a buffer to an RX ring
 *
 * PARAMETERS
 * pDevCtrl: BcmEnet_devctrl of the receiving port
 * channel: (Always 0)
 * pData: Buffer to assign
 * pEnd: Points to the end of the buffer
 */
static inline void flush_assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                          uint8 * pData, uint8 * pEnd)
{
    cache_flush_region(pData, pEnd);
    assign_rx_buffer(pDevCtrl, channel, pData);
}


static inline int enet_bpm_alloc_buf(BcmEnet_devctrl *pDevCtrl, int channel)
{
    unsigned char *pFkBuf, *pData;
    int buf_ix;
    void **pDataBufs = eth_alloc_buf_addr;
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

    if((rxdma->numRxBds - rxdma->rxAssignedBds) >= rxdma->allocTrig){
        /* number of used buffers has crossed the trigger threshold */
        if ((gbpm_alloc_mult_buf(rxdma->bulkAlloc, (void **)pDataBufs)) == GBPM_ERROR) {
            /* may be temporarily global buffer pool is depleted.
             * Later try again */
            return GBPM_ERROR;
        }

        rxdma->alloc += rxdma->bulkAlloc;

        for (buf_ix=0; buf_ix < rxdma->bulkAlloc; buf_ix++) {

            pData = (uint8_t *) pDataBufs[buf_ix];
            pFkBuf = PDATA_TO_PFKBUFF(pData, BCM_PKT_HEADROOM);

            flush_assign_rx_buffer(pDevCtrl, channel, pData,
                                   (uint8_t*)pFkBuf + BCM_PKTBUF_SIZE);
        }
    }
    return GBPM_SUCCESS;
}


int dma_rx(int rx_channel, unsigned char **pBuf, int *len)
{
    BcmEnet_devctrl *devctrl = dma_ports[rx_channel]->priv;
    BcmEnet_RxDma *rxdma = devctrl->rxdma[0];

    if (rxdma->pktDmaRxInfo.rxAssignedBds == 0) {
        printk(KERN_EMERG "rxAssignedBds = 0\n");
        return -1;
        //return BCMEAPI_CTRL_SKIP;
    }

    /* Read <status,length> from Rx BD at head of ring */
    if (bcmPktDma_EthRecv(&rxdma->pktDmaRxInfo, pBuf, len))
        return -1;

    /*
     * Explicitly allocate more RX buffers and assign them to the RX ring in
     * case the previous buffers were cut-through forwarded and not recycled.
     */
    enet_bpm_alloc_buf(devctrl, 0);

    cache_invalidate_len(pBuf, BCM_MAX_PKT_LEN);
    return 0;
}


/*
 * fkb_databuf_recycle
 *
 * Recycles FKBs
 *
 * PARAMETERS
 * fkb: FKB to recycle
 * context: Ethernet core number for the receiving interface
 *
 * NOTES
 * The context must be used to get the appropriate BcmEnet_devctrl pointer. We
 * use the dma_ports array for that.
 */
void fkb_databuf_recycle(FkBuff_t *fkb, void *context)
{
    BcmEnet_devctrl *pDevCtrl;
    uint8_t *pData = PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM);
    uint32_t ethcore_num = (int)context;

    if (ethcore_num < MAX_NUMBER_OF_PORTS)
        pDevCtrl = dma_ports[ethcore_num]->priv;
    else
        pDevCtrl = (BcmEnet_devctrl *)context;
    assign_rx_buffer(pDevCtrl, 0, pData);
}


/*
 * enet_dma_recycle
 *
 * Callback function to recycle SKBs or FKBs.
 *
 * PARAMETERS
 * pNBuff: Network buffer to recycle
 * context: Recycling context (either a BcmEnet_devctrl or an eth core number
 * flags: Recycling flags
 *
 * The context may be either the BcmEnet_devctrl of the receiving port (private
 * port data) or the Ethernet core number, depending on whether the nbuff is an
 * skb or an fkb. But ultimately, all underlying functions need a pointer to the
 * BcmEnet_devctrl struct in order to refill the appropriate RX ring, so the
 * context must be converted to a BcmEnet_devctrl pointer in all cases. We use
 * the dma_ports array to get a BcmEnet_devctrl from a core number.
 */
void enet_dma_recycle(pNBuff_t pNBuff, uint32_t context, uint32_t flags)
{
    BcmEnet_devctrl *pDevCtrl;

    if (context < MAX_NUMBER_OF_PORTS)
        pDevCtrl = dma_ports[context]->priv;
    else
        pDevCtrl = (BcmEnet_devctrl *)context;

    if (IS_FKBUFF_PTR(pNBuff)) {
        /* FKB */
        FkBuff_t *pFkb = PNBUFF_2_FKBUFF(pNBuff);
        uint8_t *pData = PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM);

        cache_invalidate_len(pFkb->data, pFkb->len);
        assign_rx_buffer(pDevCtrl, 0, pData);
    }
    else {
        /* SKB */
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if (bcmeapi_free_skb(pDevCtrl, skb, flags, 0) != BCMEAPI_CTRL_TRUE) {
            /* Recycle data buffer and replenish the RX ring */
            uint8_t *pData = skb->head + BCM_PKT_HEADROOM;
            uint8_t *pEnd;

            uint8_t *dirty_p = skb_shinfo(skb)->dirty_p;
            uint8_t *shinfoBegin = (uint8_t *)skb_shinfo(skb);
            uint8_t *shinfoEnd;
            if (skb_shinfo(skb)->nr_frags == 0) {
                /*
                 * No frags was used on this skb, so can shorten amount of data
                 * flushed on the skb_shared_info structure.
                 */
                shinfoEnd = shinfoBegin + offsetof(struct skb_shared_info, frags);
            }
            else {
                shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
            }
            cache_flush_region(shinfoBegin, shinfoEnd);

            /*
             * If driver returned this buffer to us with a valid dirty_p,
             * then we can shorten the flush length.
             */
            if (dirty_p) {
                if ((dirty_p < skb->head) || (dirty_p > shinfoBegin)) {
                    printk("invalid dirty_p detected: %px valid=[%px %px]\n",
                           dirty_p, skb->head, shinfoBegin);
                    pEnd = shinfoBegin;
                } else {
                    pEnd = (dirty_p < pData) ? pData : dirty_p;
                }
            } else {
                pEnd = shinfoBegin;
            }
            flush_assign_rx_buffer(pDevCtrl, 0, pData, pEnd);
        }
    }
}


/*
 *  uninit_buffers: un-initialize driver's pools of receive buffers
 */
static void uninit_rx_buffers(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    int i;
    BcmEnet_RxDma *rxdma = devctrl->rxdma[0];
    uint8 *rxAddr = NULL;

    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++) {
        if (bcmPktDma_EthRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE) {
            if (rxAddr != NULL)
                gbpm_free_buf((void *)rxAddr);
        }
    }

    gbpm_unresv_rx_buf(GBPM_PORT_ETH, port->p.port_id);
}


/*
 *  uninit_buffers: un-initialize driver's pools of transmit buffers
 */
static void uninit_txdma_structures(enetx_port_t *port)
{
    BcmPktDma_EthTxDma *txdma;
    BcmEnet_devctrl *devctrl = port->priv;
    int num_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[port->p.port_id];
    int size = num_tx_bds * sizeof(DmaDesc);

    txdma = devctrl->txdma[0];

    /* Disable the DMA channel */
    txdma->txDma->control &= ~DMA_EN;

    /* if any, free the tx skbs */
    while (txdma->txFreeBds < (num_tx_bds - 1)) {
        txdma->txFreeBds++;
        nbuff_free((void *)txdma->txRecycle[txdma->txHeadIndex++].key);
        if (txdma->txHeadIndex == num_tx_bds)
            txdma->txHeadIndex = 0;
    }

    /* Free the tx BD ring */
    if (txdma->txBdsBase)
        debug_dma_free_coherent(&port->dev->dev, size, (void *)txdma->txBdsBase, GFP_KERNEL);

    /* Free the txdma channel structures */
    if (devctrl->txdma[0])
        kfree((void *)(devctrl->txdma[0]));
}

static void uninit_rxdma_structures(enetx_port_t *port)
{
    BcmEnet_devctrl *devctrl = port->priv;
    BcmEnet_RxDma *rxdma = devctrl->rxdma[0];
    int size = (uint32_t)(rxdma->pktDmaRxInfo.numRxBds * sizeof(DmaDesc));

    uninit_rx_buffers(port);

    if (rxdma->pktDmaRxInfo.rxBdsBase)
        debug_dma_free_coherent(&port->dev->dev, size, (void *)rxdma->pktDmaRxInfo.rxBdsBase, GFP_KERNEL);

    /* Free the rxdma channel structures */
    if (devctrl->rxdma[0])
        kfree((void *)(devctrl->rxdma[0]));
}

void dma_uninit(enetx_port_t *port)
{
    uninit_txdma_structures(port);
    uninit_rxdma_structures(port);

    gbpm_unresv_rx_buf(GBPM_PORT_ETH, port->p.port_id);
}
