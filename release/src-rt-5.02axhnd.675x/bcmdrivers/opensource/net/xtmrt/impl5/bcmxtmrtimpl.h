/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/**************************************************************************
 * File Name  : bcmxtmrtimpl.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM63268/381/138/148/158 ATM/PTM network device driver.
 ***************************************************************************/

#if !defined(_BCMXTMRTIMPL_H)
#define _BCMXTMRTIMPL_H

#include <bcmtypes.h>
#include "bcmnet.h"
#include "bcmxtmrt.h"
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
#include "bcmPktDma_structs.h"
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
#include <rdpa_api.h>
#endif

#include "bcm_mm.h"


#ifndef CARDNAME
#define CARDNAME        "bcmxtmrt"
#endif
#define XTMRT_VERSION   "2.0"

#define XTM_CACHE_SMARTFLUSH

#define TEQ_DATA_VCID    15

#define MAX_DEV_CTXS                16
#define MAX_MATCH_IDS               128
#define MAX_DEFAULT_MATCH_IDS       16
#define SAR_DMA_MAX_BURST_LENGTH    8
#define ENET_8021Q_SIZE             4
#if defined(CONFIG_BCM963158)
#define PTM_MAX_TX_FRAME_LEN        2048
#else
#define PTM_MAX_TX_FRAME_LEN        1980  /* Per chip limitation, 63268/63381/63138/63148 SAR can only
                                           * transmit max PTM frame size of
                                           * 1980 + 4(FCS) = 1984
                                           */
#endif
#if defined(CONFIG_BCM_JUMBO_FRAME)
/*TODO : instead of using 4 below, move BCM_VLAN_MAX_TAGS 
 * to bcm_pkt_lengths.h and use it
 */ 
#define PTM_MAX_MTU_PAYLOAD_SIZE    (PTM_MAX_TX_FRAME_LEN - ETH_HLEN - (ENET_8021Q_SIZE * 4))
#else
#define PTM_MAX_MTU_PAYLOAD_SIZE    XTM_MAX_MTU_PAYLOAD_SIZE
#endif

#if defined(CONFIG_BCM_USER_DEFINED_DEFAULT_MTU)
    #if (CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > PTM_MAX_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > PTM_MAX_MTU_PAYLOAD_SIZE"
    #endif

    #define BCM_PTM_DEFAULT_MTU_SIZE  CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE

#else /* !CONFIG_BCM_USER_DEFINED_DEFAULT_MTU */

    #define BCM_PTM_DEFAULT_MTU_SIZE  PTM_MAX_MTU_PAYLOAD_SIZE

#endif

#define MAX_BUFMEM_BLOCKS           64
#define RFC1626_MTU                 9180

#define SAR_RX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_0
#define SAR_TX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_4
#define SAR_RX_DMA_BASE_CHAN        0
#define NR_SAR_RX_DMA_CHANS         2
#define SAR_TX_DMA_BASE_CHAN        4
#define NR_SAR_TX_DMA_CHANS         16
#define SAR_TIMEOUT                 (HZ/20)
#define INVALID_VCID                0xff

#define XTMRT_UNINITIALIZED         0
#define XTMRT_INITIALIZED           1
#define XTMRT_RUNNING               2

#define SKB_PROTO_ATM_CELL          0xf000
#define XTM_POLL_DONE               0x80000000

/* Circuit types. */
#define XCT_TRANSPARENT             0x00000001
#define XCT_AAL0_PKT                0x00000002
#define XCT_AAL0_CELL               0x00000003
#define XCT_OAM_F5_SEG              0x00000004
#define XCT_OAM_F5_E2E              0x00000005
#define XCT_RM                      0x00000006
#define XCT_AAL5                    0x00000007
#define XCT_ASM_P0                  0x00000008
#define XCT_ASM_P1                  0x00000009
#define XCT_ASM_P2                  0x0000000a
#define XCT_ASM_P3                  0x0000000b
#define XCT_OAM_F4_SEG              0x0000000c
#define XCT_OAM_F4_E2E              0x0000000d
#define XCT_TEQ                     0x0000000e
#define XCT_PTM                     0x0000000f

#define MAX_CIRCUIT_TYPES           16

/* Transmit Buffer Descriptor frame status word for ATM/PTM. */
#define FSTAT_MASK                  0x00000fff
#define FSTAT_ATM_VCID_MASK         0x0000000f
#define FSTAT_ATM_VCID_SHIFT        0
#define FSTAT_PTM_CRC               0x00000001
#define FSTAT_PTM_ENET_FCS          0x00000002
#define FSTAT_CT_MASK               0x000000f0
#define FSTAT_CT_SHIFT              4
#define FSTAT_CT_TRANSPARENT        (XCT_TRANSPARENT << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_PKT           (XCT_AAL0_PKT    << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_CELL          (XCT_AAL0_CELL   << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_SEG         (XCT_OAM_F5_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_E2E         (XCT_OAM_F5_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_RM                 (XCT_RM          << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL5               (XCT_AAL5        << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P0             (XCT_ASM_P0      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P1             (XCT_ASM_P1      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P2             (XCT_ASM_P2      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P3             (XCT_ASM_P3      << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_SEG         (XCT_OAM_F4_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_E2E         (XCT_OAM_F4_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_TEQ                (XCT_TEQ         << FSTAT_CT_SHIFT)
#define FSTAT_CT_PTM                (XCT_PTM         << FSTAT_CT_SHIFT)
#define FSTAT_COMMON_INS_HDR_EN     0x00000100
#define FSTAT_COMMON_HDR_INDEX_MASK 0x00000600
#define FSTAT_COMMON_HDR_INDEX_SHIFT 9
#define FSTAT_INDEX_CI              0x00000100
#define FSTAT_INDEX_CLP             0x00000200
#define FSTAT_INDEX_USE_ALT_GFC     0x00000400
#define FSTAT_MODE_INDEX            0x00000000
#define FSTAT_MODE_COMMON           0x00000800

/* Receive Buffer Descriptor frame status word for ATM/PTM. */
/* If this definition is changed, please make sure accelerator (FAP/Runner)
** definitions are also in sync */
#if defined(CONFIG_BCM963158)
#define FSTAT_MATCH_ID_MASK         0x0000001f
#define FSTAT_MATCH_ID_SHIFT        1
#define FSTAT_PACKET_CELL_MASK      0x00000040
#define FSTAT_PACKET                0x00000000
#define FSTAT_CELL                  0x00000040
#define FSTAT_ERROR                 0x00000080
#else
#define FSTAT_MATCH_ID_MASK         0x0000007f
#define FSTAT_MATCH_ID_SHIFT        0
#define FSTAT_PACKET_CELL_MASK      0x00000400
#define FSTAT_PACKET                0x00000000
#define FSTAT_CELL                  0x00000400
#define FSTAT_ERROR                 0x00000800
#endif

/* First byte of a received cell. */
#define CHDR_CT_MASK                0x0f
#define CHDR_CT_SHIFT               0
#define CHDR_CT_OAM_F5_SEG          (XCT_OAM_F5_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F5_E2E          (XCT_OAM_F5_E2E  << CHDR_CT_SHIFT)
#define CHDR_CT_RM                  (XCT_RM          << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P0              (XCT_ASM_P0      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P1              (XCT_ASM_P1      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P2              (XCT_ASM_P2      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P3              (XCT_ASM_P3      << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_SEG          (XCT_OAM_F4_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_E2E          (XCT_OAM_F4_E2E  << CHDR_CT_SHIFT)
#define CHDR_PORT_MASK              0x60
#define CHDR_PORT_SHIFT             5
#define CHDR_ERROR                  0x80
#define CHDR_ERROR_MISC             0x01
#define CHDR_ERROR_CRC              0x02
#define CHDR_ERROR_CAM              0x04
#define CHDR_ERROR_HEC              0x08
#define CHDR_ERROR_PORT             0x10

/****************************************************************************
   Logging Defines
****************************************************************************/

/* #define BCM_XTM_LOG      
   #define BCM_XTM_RX_LOG  
   #define BCM_XTM_TX_LOG   
   #define BCM_XTM_LINK_LOG */

#if defined(BCM_XTM_LOG)
#define BCM_XTM_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_DEBUG(fmt, arg...)
#define BCM_XTM_INFO(fmt, arg...)
#define BCM_XTM_NOTICE(fmt, arg...)
#define BCM_XTM_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_RX_LOG)
#define BCM_XTM_RX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_RX_DEBUG(fmt, arg...)
#define BCM_XTM_RX_INFO(fmt, arg...)
#define BCM_XTM_RX_NOTICE(fmt, arg...)
#define BCM_XTM_RX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_TX_LOG)
#define BCM_XTM_TX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_TX_DEBUG(fmt, arg...)
#define BCM_XTM_TX_INFO(fmt, arg...)
#define BCM_XTM_TX_NOTICE(fmt, arg...)
#define BCM_XTM_TX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_LINK_LOG)
#define BCM_XTM_LINK_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_INFO(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_LINK_DEBUG(fmt, arg...)
#define BCM_XTM_LINK_INFO(fmt, arg...)
#define BCM_XTM_LINK_NOTICE(fmt, arg...)
#define BCM_XTM_LINK_ERROR(fmt, arg...)
#endif

/* Information about a DMA transmit channel. A device instance may use more
 * than one transmit DMA channel. A DMA channel corresponds to a transmit queue.
 */
 
struct bcmxtmrt_dev_context;   /* forward reference */

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)

#define BcmPktDma_XtmRxDma BcmPktDma_LocalXtmRxDma
#define RXBDINFO BcmPktDma_XtmRxDma
#define PRXBDINFO BcmPktDma_XtmRxDma *

typedef struct BcmXtm_RxDma {

    BcmPktDma_XtmRxDma pktDmaRxInfo;
    struct sk_buff  *freeSkbList;
    unsigned char   *buf_pool[MAX_BUFMEM_BLOCKS];
    /* SKB Pool now dynamically allocated - Apr 2010 */
    unsigned char    *skbs_p;
    unsigned char    *end_skbs_p;
    int              rxIrq; 
    int              channel; 
} BcmXtm_RxDma;
              
#define BcmPktDma_XtmTxDma BcmPktDma_LocalXtmTxDma
#endif

#if defined(CONFIG_BCM963178)
#define DmaChannelCfg SAR_DmaChannelCfg
#define DmaStateRam   SAR_DmaStateRam
#define DmaDesc       IUDmaDesc

typedef struct {
    unsigned long key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    uint32 address;
    uint16 source;
    uint16 rsvd;
    uint32 rxChannel;
#endif
} BcmPktDma_txRecycle_t;

typedef struct BcmPktDma_LocalXtmTxDma
{
    UINT32 ulPort;
    UINT32 ulPtmPriority;
    UINT32 ulSubPriority;
    UINT32 ulQueueSize;
    UINT32 ulDmaIndex;
    volatile UINT32    ulNumTxBufsQdOne;

    volatile int       txFreeBds;
    volatile int       txHeadIndex;
    volatile int       txTailIndex;
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
}BcmPktDma_LocalXtmTxDma;
#define BcmPktDma_XtmTxDma BcmPktDma_LocalXtmTxDma
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)

typedef struct BcmXtm_TxQueue
{
    volatile int txEnabled;
    UINT32 ulDmaIndex;
    UINT32 ulPort;
    UINT32 ulPtmPriority;
    UINT32 ulSubPriority;
    UINT32 ulQueueSize;
    UINT8  ucDropAlg;            /* DT or RED or WRED */
    UINT8  ucLoMinThresh;        /* RED/WRED Low Class min threshold in % of queue size */
    UINT8  ucLoMaxThresh;        /* RED/WRED Low Class max threshold in % of queue size */
    UINT8  ucHiMinThresh;        /* WRED High Class min threshold in % of queue size */
    UINT8  ucHiMaxThresh;        /* WRED High Class max threshold in % of queue size */
} BcmXtm_TxQueue;

#define BcmPktDma_XtmTxDma BcmXtm_TxQueue

typedef struct BcmXtm_TxBdmfObj {
   bdmf_object_handle   xtmchannel; //tcont;
   bdmf_object_handle   egress_tm;
   bdmf_object_handle   xtmflow[MAX_CIRCUIT_TYPES];  //gem[MAX_CIRCUIT_TYPES];
} BcmXtm_TxBdmfObj;

#endif

#define TXQINFO  BcmPktDma_XtmTxDma
#define PTXQINFO BcmPktDma_XtmTxDma *


/* Struct added for xtmrt dmaStatus field generation for xtm flows - Apr 2010 */
typedef struct dev_params
{
    UINT32 ulFlags;
    UINT32 ulHdrType;
    UINT8  ucTxVcid;
} DEV_PARAMS, *PDEV_PARAMS;


#define PACKET_BLOG        0
#define PACKET_NORMAL      1

#include "bcmxtmrtbond.h"

#ifndef FAP_4KE

/* The definition of the driver control structure */
typedef struct bcmxtmrt_dev_context
{
    /* Linux structures. */
    struct net_device *pDev;        
    struct rtnl_link_stats64 DevStats;
    IOCTL_MIB_INFO MibInfo;
    struct ppp_channel Chan;

    /* ATM/PTM fields. */
    XTM_ADDR Addr;
    UINT32 ulLinkState;
    UINT32 ulLinkUsRate[MAX_BOND_PORTS] ;
    UINT32 ulLinkDsRate ;
    UINT32 ulTrafficType ;
    UINT32 ulTxPafEnabled ;
    UINT32 ulPortDataMask ;
    UINT32 ulOpenState;
    UINT32 ulAdminStatus;
    UINT32 ulHdrType;
    UINT32 ulEncapType; /* IPoA, PPPoA, or EoA[bridge,MER,PPPoE] */
    UINT32 ulFlags;

    /* Transmit fields. */
    UINT8 ucTxVcid;
    UINT32 ulTxQInfosSize;
    BcmPktDma_XtmTxDma *txdma[MAX_TRANSMIT_QUEUES];
    BcmPktDma_XtmTxDma *pTxQids[MAX_TRANSMIT_QUEUES];
    BcmPktDma_XtmTxDma *pHighestPrio;
    BcmPktDma_XtmTxDma *pTxPriorities[MAX_PTM_PRIORITIES][MAX_PHY_PORTS][MAX_SUB_PRIORITIES];

    /* DmaKeys, DmaSources, DmaAddresses now allocated with txBds - Apr 2010 */

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Device params to be passed to the FAP on tx enable - Apr 2010 */
    DEV_PARAMS devParams;
#endif

    /*Port Mirroring fields*/
    char szMirrorIntfIn[MIRROR_INTF_SIZE];
    char szMirrorIntfOut[MIRROR_INTF_SIZE];

} BCMXTMRT_DEV_CONTEXT, *PBCMXTMRT_DEV_CONTEXT;

/* Information that is global to all network device instances. */
#define TXDMACTRL(pDevCtx)       g_GlobalInfo.dmaCtrl
#define TXDMATYPE(pDevCtx)       XTM_HW_DMA

/* ATM Cell header definitions - Ref G.998.1 */
#define ATM_CELL_HDR_VPI_MASK                  0x0FF00000
#define ATM_CELL_HDR_VPI_SHIFT                 20

#define ATM_NON_BONDED_CELL_HDR_VCI_MASK       0x000FFFF0
#define ATM_BONDED_CELL_HDR_VCI_MASK           0x00000FF0
#define ATM_CELL_HDR_VCI_SHIFT                 4

typedef struct bcmxtmrt_global_info
{
    /* Linux structures. */
    PBCMXTMRT_DEV_CONTEXT pDevCtxs[MAX_DEV_CTXS];
    PBCMXTMRT_DEV_CONTEXT pDevCtxsByMatchId[MAX_MATCH_IDS];
    UINT32                ulDevCtxMask;
    UINT32                ulIntEnableMask;
    struct atm_dev        *pAtmDev;
    struct net_device     *pTeqNetDev;  
    struct timer_list     Timer;

    /* Callback functions. */
    XTMRT_CALLBACK pfnOamHandler;
    void *pOamContext;
    XTMRT_CALLBACK pfnAsmHandler;
    void *pAsmContext;

    /* Bonding information */
    UINT32                 atmBondSidMode;
    XtmBondConfig          bondConfig;
    XtmRtPtmTxBondHeader   ptmBondHdr[XTMRT_PTM_BOND_MAX_FRAG_PER_PKT];
    XtmRtPtmBondInfo       ptmBondInfo;

    /* MIB counter registers. */
    UINT32 ulMibRxClrOnRead;
    volatile UINT32 *pulMibTxOctetCountBase;
    volatile UINT32 *pulMibRxCtrl;
    volatile UINT32 *pulMibRxMatch;
    volatile UINT32 *pulMibRxOctetCount;
    volatile UINT32 *pulMibRxPacketCount;
    volatile UINT32 *pulRxCamBase;
    
	 /* Temporary storage for stats collection */
	 struct rtnl_link_stats64 dummy_stats;
   
    /* Everything else. */
    UINT32 ulChipRev;
    UINT32 ulDrvState;
    
    spinlock_t xtmlock_tx;
    spinlock_t xtmlock_rx;
    spinlock_t xtmlock_rx_regs;

    /* RX thread support */
    struct task_struct *rx_thread;
    wait_queue_head_t   rx_thread_wqh;
    volatile unsigned long rx_work_avail;
#define XTMRT_BUDGET   32

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963178)
#if !defined(CONFIG_BCM963178)    
    /* DMA, BD and buffer fields. */
    BcmXtm_RxDma       *rxdma[MAX_RECEIVE_QUEUES];
    volatile DmaRegs   *dmaCtrl;
#endif

    /* Global transmit queue fields. */
    UINT32 ulNumExtBufs;
    UINT32 ulNumExtBufsRsrvd;
    UINT32 ulNumExtBufs90Pct;
    UINT32 ulNumExtBufs50Pct;
    UINT32 ulNumTxQs;
    UINT32 ulNumTxBufsQdAll;
    UINT32 ulDbgQ1;
    UINT32 ulDbgQ2;
    UINT32 ulDbgQ3;
    UINT32 ulDbgD1;
    UINT32 ulDbgD2;
    UINT32 ulDbgD3;

    struct kmem_cache *xtmSkbCache;
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    bdmf_object_handle  bdmfXtm;
    bdmf_object_handle  bdmfWan;
    bdmf_object_handle  bdmfXtm_Orl_Tm;
    BcmXtm_TxBdmfObj    txBdmfObjs[MAX_TRANSMIT_QUEUES];
#endif
    
} BCMXTMRT_GLOBAL_INFO, *PBCMXTMRT_GLOBAL_INFO;

extern BCMXTMRT_GLOBAL_INFO g_GlobalInfo;

/**** Function Prototypes ****/
int bcmxtmrt_xmit(pNBuff_t pNBuf, struct net_device *dev);
UINT32 bcmxtmrt_processRxPkt(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *rxdma,
                             UINT8  *pData, UINT16 bufStatus, UINT32 len, UINT32 flow_key);
void bcmxtmrt_processRxCell(UINT8 *pData);
                             
                             
int bcmxtmrt_ptmbond_add_hdr(UINT32 ulTxPafEnabled, pNBuff_t *ppNBuff, UINT32 ulPtmPrioIdx);
int ProcTxBondInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset);


#endif /* FAP_4KE */

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#define XTMRT_PTM_FIXED_OVERHEAD           8   /* 4 (FCS) + 2 (CRC) + 1 (SOF) + 1 (Last CW byte count, which is Ck byte) */
#define XTMRT_PTM_BOND_FIXED_OVERHEAD      4   /* 4 (FCS) */
#define XTMRT_PTM_BOND_FRAG_FIXED_OVERHEAD 6   /* 2 (CRC) + 2 (FragHdr) + 1 (SOF) + 1 (Last Cw byte count, Ck) */
#define XTMRT_GFAST_DTU_OVERHEAD           2
#define XTMRT_GFAST_PKT_OVERHEAD           1
#define XTMRT_GFAST_AVG_PKTSIZE            512
#define XTMRT_GFAST_TOTAL_OVERHEAD         (XTMRT_GFAST_PKT_OVERHEAD+XTMRT_GFAST_DTU_OVERHEAD+4)  /* 4 (FCS) */
#define XTMRT_PTM_CODEWORD_LEN             64
#define XTMRT_PTM_CODEWORD_SHIFT           6
#define XTMRT_PTM_CODEWORD_SYNC_LEN        1
#define XTMRT_ATM_AAL5_TRAILER_LEN         8
#define XTMRT_ATM_CELL_HEADER_LEN          5
#define XTMRT_ATM_NITRO_CELL_HEADER_LEN    1
#define XTMRT_ATM_CELL_PAYLOAD_LEN         48

/*---------------------------------------------------------------------------
 * UINT32 bcmxtmrt_xtmOverhead (UINT16 bufStatus, UINT32 hdrType, UINT32 len, UINT32
 *                              rfc2684Type, PBCMXTMRT_DEV_CONTEXT pDevCtx)
 *
 * Description:
 *    Calculates the overhead for ATM and PTM single/bonded modes based on link
 *    type ( adsl, vdsl, gfast)
 * Returns:
 *    The overhead length
 *---------------------------------------------------------------------------
 */
static inline UINT32 bcmxtmrt_xtmOverhead(UINT32 hdrType, UINT32 len, UINT32 rfc2684Type, PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   UINT32                  overhead ;
   UINT32                  totalBytes ;
   UINT32                  devFlags = pDevCtx->ulFlags ;
   PBCMXTMRT_GLOBAL_INFO   pGi     = &g_GlobalInfo ;

   switch (pDevCtx->Addr.ulTrafficType) {

      case   TRAFFIC_TYPE_ATM           :
      case   TRAFFIC_TYPE_ATM_BONDED    : {

        /* RFC header is added by HW but SW can add it in case of issues
        ** with HW, which is not the case with recent HW. Recent HW has been
        ** stable and we do not enable the SW addition of the RFC header at all.
        **/
        UINT32 packetLen = (rfc2684Type == RFC2684_NONE) ? (len) : (len - HT_LEN(hdrType)) ;
        UINT32 atmFrame = packetLen + HT_LEN(hdrType) + XTMRT_ATM_AAL5_TRAILER_LEN ;
        UINT32 cells = atmFrame / XTMRT_ATM_CELL_PAYLOAD_LEN ;

        cells = (atmFrame > (cells * XTMRT_ATM_CELL_PAYLOAD_LEN)) ? (cells+1) : cells ;

        if (devFlags & CNI_NITRO)
           totalBytes = cells * (XTMRT_ATM_NITRO_CELL_HEADER_LEN + XTMRT_ATM_CELL_PAYLOAD_LEN) ;
        else
           totalBytes = cells * (XTMRT_ATM_CELL_HEADER_LEN + XTMRT_ATM_CELL_PAYLOAD_LEN) ;

        overhead = totalBytes - packetLen ;

        //printk ("XtmRt-ATM/ATMBnd: len %d, packetLen %d, atmFrame %d, cells %d, Nitro %d, overhead %d rfc2684 %d \n", len,
                //packetLen, atmFrame, cells, ((devFlags & CNI_NITRO) ? 1 : 0), overhead, rfc2684Type) ;
        break ;
      }

      case   TRAFFIC_TYPE_PTM           : {
        if (devFlags & CNI_GFAST) {
           UINT32 gfastFrame = len + (len/XTMRT_GFAST_AVG_PKTSIZE) + XTMRT_GFAST_PKT_OVERHEAD + XTMRT_GFAST_DTU_OVERHEAD + ETH_FCS_LEN ; 

           overhead = gfastFrame - len ;
           //printk("XtmRt-GFAST: len %d, gfastFrame %d, overhead %d\n", len, gfastFrame, overhead);
        }
        else {

           UINT32 ptmFrame = len + XTMRT_PTM_FIXED_OVERHEAD ;
           UINT32 codewords = (ptmFrame + XTMRT_PTM_CODEWORD_LEN - 1) >> XTMRT_PTM_CODEWORD_SHIFT;
           totalBytes = ptmFrame + (codewords * XTMRT_PTM_CODEWORD_SYNC_LEN) ;

           overhead = totalBytes - len;
           //printk("XtmRt-PTM: len %d, ptmFrame %d, codewords %d, overhead %d\n", len, ptmFrame, codewords, overhead);
        }
        break ;
      }

      case   TRAFFIC_TYPE_PTM_BONDED  : {

         XtmRtPtmTxBondHeader  *pPtmHeader ;
         UINT32 i, numFrags ;

         numFrags = bcmxtmrt_ptmbond_get_hdr ((UINT8 *) &pGi->ptmBondHdr[0], pDevCtx->ulTxPafEnabled, 0, len) ; /* Priority can be 0 here as the purpose
                                                                                                       * is to get just the no.of frags.
                                                                                                       */
         overhead = 0 ;
         for (i = 0; i < numFrags; i++) {

            UINT32 ptmFrame, codewords ;

            pPtmHeader = &pGi->ptmBondHdr[i] ;

            ptmFrame =  pPtmHeader->sVal.FragSize ; /* Included overhead of fragments */

            if (devFlags & CNI_GFAST) {
               overhead += (ptmFrame/XTMRT_GFAST_AVG_PKTSIZE) + XTMRT_GFAST_PKT_OVERHEAD + XTMRT_GFAST_DTU_OVERHEAD ;
            }
            else {
               codewords = (ptmFrame + XTMRT_PTM_CODEWORD_LEN - 1) >> XTMRT_PTM_CODEWORD_SHIFT ;
               totalBytes = ptmFrame + (codewords * XTMRT_PTM_CODEWORD_SYNC_LEN) ;

               overhead += totalBytes - ptmFrame + XTMRT_PTM_BOND_FRAG_FIXED_OVERHEAD ;
            }

            if (pPtmHeader->sVal.PktEop)
               overhead += XTMRT_PTM_BOND_FIXED_OVERHEAD ;
         }
         //printk ("XtmRt-PTMBnd: len %d, numFrags %d, GFAST %d, overhead %d\n", len, numFrags, ((devFlags & CNI_GFAST) ? 1 : 0), overhead) ;
         break ;
      }
      
      default                         :
         overhead = 0 ;
         //printk("XtmRt-Unknown: len %d, overhead %d\n", len, overhead);
         break ;

   } /* End of switch (pDevCtx->Addr.ulTrafficType) */

   return overhead;
}

#else
#define bcmxtmrt_xtmOverhead(_hdrType, _len, _rfc2684Type, _pDevCtx)  0
#endif

/* Macro to wake up the RX thread */
#define BCMXTMRT_WAKEUP_RXWORKER(x) do {                    \
               set_bit(0, &(x)->rx_work_avail);             \
               wake_up_interruptible(&((x)->rx_thread_wqh)); } while (0)

#ifndef FAP_4KE
/* FOR ATMCell, initialize pBufStatus value to its starting value based on skb->protocol.
 * Everything else, initialize pBufStatus value to 0 */
static inline int bcmxtmrt_get_bufStatus(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                                         UINT32 isAtmCell, UINT16 *pBufStatus)
{

   /* Calculate bufStatus for the packet */
   if ((pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
   {
      *pBufStatus |= pDevCtx->ucTxVcid;
      
      if (isAtmCell)
      {
         if (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT)
         {
            *pBufStatus |= FSTAT_MODE_COMMON;
            *pBufStatus &= ~(FSTAT_COMMON_INS_HDR_EN |
                               FSTAT_COMMON_HDR_INDEX_MASK);
         }
      }
      else
      {
         *pBufStatus |= FSTAT_CT_AAL5;
         if (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT)
         {
            *pBufStatus |= FSTAT_MODE_COMMON;
            if (HT_LEN(pDevCtx->ulHdrType) && (pDevCtx->ulFlags & CNI_HW_ADD_HEADER))
            {
               *pBufStatus |= FSTAT_COMMON_INS_HDR_EN |
                              ((HT_TYPE(pDevCtx->ulHdrType) - 1) <<
                               FSTAT_COMMON_HDR_INDEX_SHIFT);
            }
            else
            {
               *pBufStatus &= ~(FSTAT_COMMON_INS_HDR_EN |
                                FSTAT_COMMON_HDR_INDEX_MASK);
            }
         }
      }
   }
   else
   {
      *pBufStatus = FSTAT_CT_PTM | FSTAT_PTM_ENET_FCS | FSTAT_PTM_CRC;
   }
   return 0;
}
#endif /* FAP_4KE */

#endif /* _BCMXTMRTIMPL_H */
