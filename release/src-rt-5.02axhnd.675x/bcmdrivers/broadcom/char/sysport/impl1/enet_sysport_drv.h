/*
   <:copyright-BRCM:2017:DUAL/GPL:standard
   
      Copyright (c) 2017 Broadcom 
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

#ifndef _ENET_SYSPORT_DRV_H_
#define _ENET_SYSPORT_DRV_H_

#if defined(CONFIG_BCM963158)
#define PKT_DESC_SYSPORT
#elif defined(CONFIG_BCM94908)
#define PKT_DESC_IUDMA
#endif

#define ENET_NUM_RX_PKT_DESC     (512)
#define ENET_NUM_TX_PKT_DESC     (512)

#define SYSPORT_NUM_RX_PKT_DESC_LOG2  (9)
#define SYSPORT_NUM_TX_PKT_DESC_LOG2  (9)

#if ( ENET_NUM_TX_PKT_DESC != (1<<SYSPORT_NUM_TX_PKT_DESC_LOG2) )
#error "ENET_NUM_TX_PKT_DESC != (1<<SYSPORT_NUM_TX_PKT_DESC_LOG2) "
#endif

#if ( ENET_NUM_RX_PKT_DESC != (1<<SYSPORT_NUM_RX_PKT_DESC_LOG2) )
#error "ENET_NUM_RX_PKT_DESC != (1<<SYSPORT_NUM_RX_PKT_DESC_LOG2) "
#endif

typedef struct BcmEnet_sysport_devctrl {
    /* Kernel network device related stuff */
    struct net_device *dev;             /* ptr to net_device */ 
    struct net_device_stats stats;      /* statistics used by the kernel */ 
    /* Packet RX task related stuff */
    int rx_work_avail;
    wait_queue_head_t rx_thread_wqh;
    struct task_struct *rx_thread;
    /* RX/TX locks */
    spinlock_t ethlock_rx;
    spinlock_t ethlock_tx;
    spinlock_t bcm_extsw_access;
} BcmEnet_sysport_devctrl;

#if defined(PKT_DESC_SYSPORT) 
typedef struct PktDesc {
    uint32_t address    ;
    uint32_t address_hi :8;
    uint32_t status     :10;
/* SYSPORT RX Descriptor Status */
#define PD_STS_RX_EOP       (1<<9)
#define PD_STS_RX_SOP       (1<<8)
#define PD_STS_RX_CSUM      (1<<7)
#define PD_STS_RX_PFAIL     (1<<6)
#define PD_STS_RX_OV        (1<<5)
#define PD_STS_RX_ERR       (1<<4)
#define PD_STS_RX_PKT_TYPE  (3<<2) /* bit:3-2 */
#define PD_STS_RX_RSVD      (3<<0) /* bit:1-0 */

#define PD_STS_TX_EOP       (1<<9)
#define PD_STS_TX_SOP       (1<<8)
#define PD_STS_TX_CSUM      (1<<7)
#define PD_STS_TX_SLP       (1<<6)
#define PD_STS_TX_INSB      (3<<4) /* bit:5-4*/
#define PD_STS_TX_APP_CRC   (1<<3)
#define PD_STS_TX_OVR_CRC   (1<<2)
#define PD_STS_TX_INS_VLAN  (3<<0) /* bit:1-0 */

#if (PD_STS_RX_EOP != PD_STS_TX_EOP) || (PD_STS_RX_SOP != PD_STS_TX_SOP)
#error "ERROR - RX & TX EOP/SOP not same"
#endif

/* Just for convinience to keep it aligned with iuDMA implementation */
#define PD_STS_EOP              PD_STS_RX_EOP  
#define PD_STS_SOP              PD_STS_RX_SOP  
#define PD_STS_APPEND_BRCM_TAG  PD_STS_TX_INSB
#define PD_STS_APPEND_CRC       PD_STS_TX_APP_CRC
    uint32_t length     :14;

#define PD_STS_RX_PKT_ERR  (PD_STS_RX_OV | PD_STS_RX_ERR)
}PktDesc;

#elif defined(PKT_DESC_IUDMA)

#define IUDMA_RX_CHAN (0)
#define IUDMA_TX_CHAN (1)

typedef DmaDesc PktDesc;

/* DmaDesc to PktDesc mapping */
#define          PD_STS_OWN                DMA_OWN  
#define          PD_STS_EOP                DMA_SOP  
#define          PD_STS_SOP                DMA_EOP  
#define          PD_STS_WRAP               DMA_WRAP 
#define          PD_STS_PRIO               DMA_PRIO 
#define          PD_STS_APPEND_BRCM_TAG    DMA_APPEND_BRCM_TAG
#define          PD_STS_APPEND_CRC         DMA_APPEND_CRC
#define          PD_STS_USB_ZERO_PKT       USB_ZERO_PKT

#define PD_STS_RX_PKT_ERR  (0) /* Defined just to match the other struct */

#define          PD_STS_DESC_USEFPM    DMA_DESC_USEFPM
#define          PD_STS_DESC_MULTICAST DMA_DESC_MULTICAST
#define          PD_STS_DESC_BUFLENGTH DMA_DESC_BUFLENGTH

extern int gmac_hw_stats(struct net_device_stats *stats);
extern int gmac_dump_mib(int type);

#endif

#define PKT_DESC_ASSIGN_BUF_ADDR(pd, pData) (pd)->address = (uint32)VIRT_TO_PHYS((pData))
#define PKT_DESC_ASSIGN_BUF_LEN(pd, len) (pd)->length = (len)

#define RX_PKT_DESC_ASSIGN_BUF_ADDR(pd, pData) PKT_DESC_ASSIGN_BUF_ADDR(pd, pData)
#define TX_PKT_DESC_ASSIGN_BUF_ADDR(pd, pData) PKT_DESC_ASSIGN_BUF_ADDR(pd, pData)

#define TX_PKT_DESC_ASSIGN_BUF_LEN(pd, len) PKT_DESC_ASSIGN_BUF_LEN(pd, len)

#define TX_PKT_DESC_ASSIGN_STATUS(pd, status) (pd)->status = status

#if defined(PKT_DESC_IUDMA)
#define RX_PKT_DESC_ASSIGN_BUF_LEN(pd, len) PKT_DESC_ASSIGN_BUF_LEN(pd, len)
#else
#define RX_PKT_DESC_ASSIGN_BUF_LEN(pd, len) 
#endif

/* Structure to hold different type of addresses to PktDesc Ring */
typedef struct pkt_desc_ptrs {
    volatile PktDesc *p_desc_unaligned_addr; /* Unalligned host address */
    volatile PktDesc *p_desc_host_addr;      /* Aligned host addres */
    volatile PktDesc *p_desc_physical_addr;  /* Aligned physical address */
}pkt_desc_ptrs;

typedef struct pkt_desc_info {
    pkt_desc_ptrs           desc_ptr;           /* PktDesc Pointers */
    uint32_t                total_pkt_desc;     /* Total number of PktDesc allocated to this ring */
    uint32_t                assigned_pkt_desc;  /* Number of PktDesc with assigned buffer */
    uint32_t                tail_idx;           /* Driver's view of PktDesc ring */
    uint32_t                head_idx;           /* Driver's view of PktDesc ring */ 
#if defined(PKT_DESC_IUDMA)
    volatile DmaChannelCfg  *pDmaChCfg;         /* */
#else
    uint16_t                rx_c_index;         /* Must be 16 bit - RX Consumer Index 
                                                 * rx_c_index tries to catch up to HW-producer-index;
                                                 * Only used to check if HW has produced a packet
                                                 * Once SW consumes the packet -- increment */
    uint16_t                tx_c_index;         /* Must be 16 bit - TX Consumer Index 
                                                 * tx_c_index tries to catch up to HW-consumer-index;
                                                 * Only used to reclaim the transmitted packets
                                                 * Once HW consumes the packet -- increment */
#endif
}pkt_desc_info;

typedef void* enet_tx_recycle_ctxt;

/* SKB Management */
typedef struct enet_sysport_skb_pool {
    struct sk_buff *freeSkbList;
    unsigned char *start_skbs_p;
    unsigned char *end_skbs_p;
    struct kmem_cache *enetSkbCache;
}enet_sysport_skb_pool;

#endif /* _ENET_SYSPORT_DRV_H_*/
