#ifndef __PKTDMA_DEFINES_H_INCLUDED__
#define __PKTDMA_DEFINES_H_INCLUDED__

#define XTMFREE_FORCE_FREE    1
#define XTMFREE_NO_FORCE_FREE 0

#define ENET_TX_EGRESS_QUEUES_MAX  4

#if defined(CONFIG_BCM963268) ||      defined(CONFIG_BCM_FAP_MODULE)
/* Increase these from 1 to 2 to support rx & tx splitting - Oct 2010 */
#define ENET_RX_CHANNELS_MAX  2
#define ENET_TX_CHANNELS_MAX  2

#elif defined(CONFIG_BCM963268) && defined(CONFIG_BCM_FAP_PWRSAVE)
#define ENET_RX_CHANNELS_MAX  3   // 1 iuDMA needed for Host when FAP is powered off
#define ENET_TX_CHANNELS_MAX  3   // 1 iuDMA needed for Host when FAP is powered off
#elif defined(CONFIG_BCM947189)
#define ENET_RX_CHANNELS_MAX  2
#define ENET_TX_CHANNELS_MAX  2
#else
#define ENET_RX_CHANNELS_MAX  4
#define ENET_TX_CHANNELS_MAX  4
#endif

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
 * 5. FAP has fixed number of BDs because of limited DSPRAM/PSRAM
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
#if defined(CONFIG_BCM947189)
#define HOST_ENET_NR_TXBDS              800
#else
#define HOST_ENET_NR_TXBDS              200
#endif

#if defined(CONFIG_BCM_DSL_GINP_RTX) || defined(SUPPORT_DSL_GINP_RTX)
																/* 20 ms RTX buffering */
#define HOST_XTM_NR_TXBDS               4700
#else
#define HOST_XTM_NR_TXBDS               400
#endif

#define MOCA_TXQ_DEPTH_MAX              3000

#if defined(CONFIG_BCM963268)
/* FAP: # of buffers assigned to RXBDs */
#define FAP_ENET_NR_RXBDS               600     /* FAP chnl */
#define FAP_XTM_NR_RXBDS                200
#define HOST_ENET_NR_RXBDS_MIN          800     /* Host chnl, WANoE case */
#define HOST_XTM_NR_RXBDS_MIN           512

/* FAP: # of RXBDs for non-default channels */
#define FAP_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FAP_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FAP: # of TXBDs for IuDMA managed by FAP */
#define FAP_ENET_NR_TXBDS               HOST_ENET_NR_TXBDS
#define FAP_XTM_NR_TXBDS                HOST_XTM_NR_TXBDS
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

#define MOCA_TXQ_DEPTH_MAX              3000

#if defined(CONFIG_BCM963268)
/* FAP: # of buffers assigned to RXBDs */
#define FAP_ENET_NR_RXBDS               HOST_ENET_NR_RXBDS
#define FAP_XTM_NR_RXBDS                HOST_XTM_NR_RXBDS

/* FAP: # of RXBDs for non-default channels */
#define FAP_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FAP_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FAP: # of TXBDs for IuDMA managed by FAP */
#define FAP_ENET_NR_TXBDS               HOST_ENET_NR_TXBDS
#define FAP_XTM_NR_TXBDS                HOST_XTM_NR_TXBDS
#endif

#endif // (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/* Note: these #defines are also the default values for various allocations
   when not using BPM hence they are not protected by CONFIG_BCM_BPM */
#define FAP_BPM_ENET_BULK_ALLOC_MAX     128
#define FAP_BPM_XTM_BULK_ALLOC_MAX      128

#define FAP_BPM_BUF_CACHE_SIZE           1024
#define FAP_BPM_BUF_CACHE_ALLOC_THRESH   512
#define FAP_BPM_BUF_CACHE_BULK_ALLOC_MAX 512

#define FAP_BPM_FBL_ENT_SIZE       512   
#if defined(CONFIG_BCM_DSL_GINP_RTX) || defined(SUPPORT_DSL_GINP_RTX)
#define FAP_BPM_FBL_MAX_REQ        64
#else
#define FAP_BPM_FBL_MAX_REQ        16
#endif

/* other chips have 2 FAPs */
#define FAP_BPM_BUF_RESV    ((FAP_BPM_BUF_CACHE_SIZE+FAP_BPM_FBL_ENT_SIZE)*2)
#else
#define FAP_BPM_BUF_RESV    0
#endif

#endif /* __PKTDMA_DEFINES_H_INCLUDED__ */
