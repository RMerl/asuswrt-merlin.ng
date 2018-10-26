#ifndef __PKTDMASTRUCTS_H_INCLUDED__
#define __PKTDMASTRUCTS_H_INCLUDED__

#include <bcm_map_part.h>
#include "bcmPktDma_defines.h"

#if defined(FAP_4KE)
#define HOST_VOLATILE
#else
#define HOST_VOLATILE volatile
#endif

#define RxChanTo0BasedPhyChan(chan) ((chan)*2)
#define TxChanTo0BasedPhyChan(chan) ((chan)*2+1)

typedef struct BcmPktDma_LocalEthRxDma
{
   int      enetrxchannel_isr_enable;
   volatile DmaChannelCfg *rxDma;
   volatile DmaDesc *rxBdsBase;   /* save non-aligned address for free */
   volatile DmaDesc *rxBds;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
   volatile DmaDesc *rxBdsPhysBase;
#endif
   int      rxAssignedBds;
   int      numRxBds;
   int      rxHeadIndex;
   int      rxTailIndex;
   int      channel;
   uint32   fapIdx;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   int      rxUnassignedBdsTrig;
   uint32   allocTrig;
   uint32   bulkAlloc;
   uint32   bufAllocWait;
   uint32   alloc;
   uint32   free;
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   uint32   iqLoThresh;
   uint32   iqHiThresh;
   uint32   iqDropped;
   uint32   iqDepth;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   uint32   iqLoThreshDqm;
   uint32   iqHiThreshDqm;
   uint32   iqDroppedDqm;
   uint32   iqDepthDqm;
#endif
#endif
   volatile int  rxEnabled;
   int           rxOwnership;
#if defined(CONFIG_BCM947189)
   volatile EnetCoreMisc *miscReg;
   int      channel_init;
#endif

} BcmPktDma_LocalEthRxDma;

typedef struct {
    unsigned long key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    uint32 address;
    uint16 source;
    uint16 rsvd;
    uint32 rxChannel;
#endif
} BcmPktDma_txRecycle_t;

typedef struct BcmPktDma_LocalEthTxDma
{
   HOST_VOLATILE int txFreeBds;      /* # of free transmit bds */
   HOST_VOLATILE int txHeadIndex;
   HOST_VOLATILE int txTailIndex;
   int               numTxBds;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   int          bdsAllocated;
#endif
   volatile DmaChannelCfg *txDma;    /* location of transmit DMA register set */
    volatile DmaDesc *txBdsBase;   /* save non-aligned address for free */
    volatile DmaDesc *txBds;       /* Memory location of tx Dma BD ring */
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    volatile DmaDesc *txBdsPhysBase;   /* save non-aligned address for free */
    BcmPktDma_txRecycle_t *txRecycleBase;
#endif
   BcmPktDma_txRecycle_t *txRecycle;
   volatile int txEnabled;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    UINT32 ulLoThresh;
    UINT32 ulHiThresh;
    UINT32 ulDropped;
    uint32      txDropThr[ENET_TX_EGRESS_QUEUES_MAX];    /* Tx Threshold when crossed pkt is dropped */
    uint32      txDropThrPkts[ENET_TX_EGRESS_QUEUES_MAX];/* Number of pkts dropped because of thresh */
#endif
   int          txOwnership;
   int          channel;
   uint32       fapIdx;
#if defined(CONFIG_BCM947189)
   uint32       coreIndex;
#endif
} BcmPktDma_LocalEthTxDma;

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* NOTE: Keep this in sync with BcmPktDma_XtmRxDma in bcmxtmrtimpl.h */
typedef struct BcmPktDma_LocalXtmRxDma
{
   int                     xtmrxchannel_isr_enable;
   volatile DmaChannelCfg *rxDma;
   volatile DmaDesc       *rxBdsBase;   /* save non-aligned address for free */
   volatile DmaDesc       *rxBds;
#if defined(CONFIG_ARM)
   volatile DmaDesc       *rxBdsPhysBase;
#endif
   int                     numRxBds;
   int                     rxAssignedBds;
   int                     rxHeadIndex;
   int                     rxTailIndex;
   int                     channel;
   uint32                  fapIdx;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   int                     rxUnassignedBdsTrig;
   uint32                  allocTrig;
   uint32                  bulkAlloc;
   uint32                  bufAllocWait;
   uint32                  alloc;
   uint32                  free;
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   uint32                  iqHiThresh;
   uint32                  iqLoThresh;
   uint32                  iqDropped;
   uint32                  iqDepth;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   uint32                  iqLoThreshDqm;
   uint32                  iqHiThreshDqm;
   uint32                  iqDroppedDqm;
   uint32                  iqDepthDqm;
#endif
#endif

   volatile int            rxEnabled;
} BcmPktDma_LocalXtmRxDma;

/* NOTE: Keep this in sync with BcmPktDma_XtmTxDma in bcmxtmrtimpl.h */
typedef struct BcmPktDma_LocalXtmTxDma
{
    UINT32 ulPort;
    UINT32 ulPtmPriority;
    UINT32 ulSubPriority;
    UINT32 ulQueueSize;
    UINT32 ulDmaIndex;
    HOST_VOLATILE UINT32    ulNumTxBufsQdOne;

    HOST_VOLATILE int       txFreeBds;
    HOST_VOLATILE int       txHeadIndex;
    HOST_VOLATILE int       txTailIndex;
    volatile DmaChannelCfg *txDma;
    volatile DmaDesc       *txBdsBase;   /* save non-aligned address for free */
    volatile DmaDesc       *txBds;
#if defined(CONFIG_ARM)
    volatile DmaDesc       *txBdsPhysBase;   /* save non-aligned address for free */
#endif
    volatile DmaStateRam   *txStateRam;
    BcmPktDma_txRecycle_t  *txRecycle;
    volatile int            txEnabled;

    /* Field added for xtmrt dmaStatus field generation for xtm flows - Apr 2010 */
    uint32                  baseDmaStatus;
    // int                  channel;    Not required for xtm. Same as ulDmaIndex
    uint32                  fapIdx;

    /* For SW WFQ implementation */
    UINT16 ulAlg;
    UINT16 ulWeightValue;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    UINT32 ulLoThresh;
    UINT32 ulHiThresh;
    UINT32 ulDropped;
#endif
} BcmPktDma_LocalXtmTxDma;
#endif /* CONFIG_BCM_XTMCFG */

typedef struct BcmPktDma_HostBds {
    int eth_rxbds[ENET_RX_CHANNELS_MAX];
    int eth_txbds[ENET_TX_CHANNELS_MAX];
#if (defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE))
    int xtm_rxbds[XTM_RX_CHANNELS_MAX];
    int xtm_txbds[XTM_TX_CHANNELS_MAX];
#endif
#if defined(CONFIG_BCM963268)
    int eth_rxdqm[ENET_RX_CHANNELS_MAX];
    int eth_txdqm[ENET_TX_CHANNELS_MAX];
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int xtm_rxdqm[XTM_RX_CHANNELS_MAX];
    int xtm_txdqm[XTM_TX_CHANNELS_MAX];
#endif
#endif
} BcmPktDma_HostBds;


#if defined(CONFIG_BCM963268)
typedef struct BcmPktDma_FapBds {
    int eth_rxbds[ENET_RX_CHANNELS_MAX];
    int eth_txbds[ENET_TX_CHANNELS_MAX];
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int xtm_rxbds[XTM_RX_CHANNELS_MAX];
    int xtm_txbds[XTM_TX_CHANNELS_MAX];
#endif
} BcmPktDma_FapBds;
#endif




typedef struct BcmPktDma_Bds{
    BcmPktDma_HostBds host;
#if defined(CONFIG_BCM963268)
    BcmPktDma_FapBds  fap;
#endif
} BcmPktDma_Bds;


#endif
