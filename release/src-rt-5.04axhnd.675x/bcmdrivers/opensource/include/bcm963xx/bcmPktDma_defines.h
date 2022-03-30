#ifndef __PKTDMA_DEFINES_H_INCLUDED__
#define __PKTDMA_DEFINES_H_INCLUDED__

#define XTMFREE_FORCE_FREE    1
#define XTMFREE_NO_FORCE_FREE 0

#define ENET_TX_EGRESS_QUEUES_MAX  4

#define ENET_RX_CHANNELS_MAX  4
#define ENET_TX_CHANNELS_MAX  4

#define XTM_RX_CHANNELS_MAX   2
#define XTM_TX_CHANNELS_MAX   16

/*
 * -----------------------------------------------------------------------------
 * 1. Added % memory used by BPM to menuconfig. Set default to 15% of memory.
 * 2. For 64MB  #of RXBDs and #of buffers doubles to that of 32MB
 * 3. Mini-jumbo packets (size of 2K) support on 6816 only.
 * 4. Number of RXBDs will be computed as % of total buffers. i.e. Ethernet
 *    host BDs on 40% of total buffers. Unless CMF is compiled in and then
 *    it is reduced to 20% of total
 *    Eth Chnl: # of RXBDs = 600
 *    XTM Chnl: # of RXBDs = 200
 *    Eth (WoE) Chnl: # of RXBDs = 800 (min)
 *
 * -----------------------------------------------------------------------------
 */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/* Channel-0 is default */
/* % of number of buffers assigned to RXBDs */
#define ENET_DEF_RXBDS_BUF_PRCNT        40
#define XTM_DEF_RXBDS_BUF_PRCNT         10

/* Fixed # of RXBDs for non-default channels */
#define HOST_ENET_NON_DEF_CHNL_NR_RXBDS 100
#define HOST_XTM_NON_DEF_CHNL_NR_RXBDS  16

#define HOST_ENET_NR_RXBDS              600

#if defined(CHIP_6338)
/* 38 needs a bigger cell queue for soft sar (64 bytes each) */
#define HOST_XTM_NR_RXBDS               500
#else
#define HOST_XTM_NR_RXBDS               HOST_ENET_NR_RXBDS
#endif


/* Host/MIPS: # of TXBDs for IuDMA managed by host */
#define HOST_ENET_NR_TXBDS              200

#if defined(CONFIG_BCM_DSL_GINP_RTX) || defined(SUPPORT_DSL_GINP_RTX)
																/* 20 ms RTX buffering */
#define HOST_XTM_NR_TXBDS               4700
#else
#define HOST_XTM_NR_TXBDS               400
#endif




#else // (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))


/* Channel-0 is default */
/* Fixed # of RXBDs for non-default channels */
#define HOST_ENET_NON_DEF_CHNL_NR_RXBDS 100
#define HOST_XTM_NON_DEF_CHNL_NR_RXBDS  16
#define HOST_ENET_NR_RXBDS              400

#if defined(CHIP_6338)
/* 38 needs a bigger cell queue for soft sar (64 bytes each) */
#define HOST_XTM_NR_RXBDS               500
#else
#define HOST_XTM_NR_RXBDS               HOST_ENET_NR_RXBDS
#endif


/* Host/MIPS: # of TXBDs for IuDMA managed by host */
#define HOST_ENET_NR_TXBDS              200
#define HOST_XTM_NR_TXBDS               HOST_ENET_NR_RXBDS


#endif // (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

#endif /* __PKTDMA_DEFINES_H_INCLUDED__ */
