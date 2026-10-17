/*
   Copyright (c) 2023 Broadcom 
   All Rights Reserved
   <:label-BRCM:2023:DUAL/GPL:standard
   
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

#ifndef _dhd_offload_h_
#define _dhd_offload_h_

#include <bcmmsgbuf.h>

/* Enable miscellaneous acceleration features */
#if defined(BCM_DHD_OFFLOAD)

/**
 * DHD_D2H_SOFT_DOORBELL_SUPPORT enables D2H direct Offload Helper thread wakeup
 */
#define DHD_D2H_SOFT_DOORBELL_SUPPORT

/**
 * DHD offload add-on features
 * Used by upper layers to query and enable
 */
#define DHD_OFFLOAD_LBR_AGGR
#define DHD_OFFLOAD_NO_NONACCPKT_TXSTSOFFL

/**
 * Offload helper need bigger ring sizes as it does not have any back up queues
 * Redefine PCIE ring max item sizes (bcmmsgbuf.h) to meet offload architecture
 */
#if defined(H2DRING_TXPOST_MAX_ITEM)
#undef H2DRING_TXPOST_MAX_ITEM
#endif /* H2DRING_TXPOST_MAX_ITEM */
#define H2DRING_TXPOST_MAX_ITEM       2560

#if defined(H2DRING_RXPOST_MAX_ITEM)
#undef H2DRING_RXPOST_MAX_ITEM
#endif /* H2DRING_RXPOST_MAX_ITEM */
#define H2DRING_RXPOST_MAX_ITEM       1024

#if defined(D2HRING_RXCMPLT_MAX_ITEM)
#undef D2HRING_RXCMPLT_MAX_ITEM
#endif /* H2DRING_RXPOST_MAX_ITEM */
#define D2HRING_RXCMPLT_MAX_ITEM      1024

#endif /* BCM_DHD_OFFLOAD */

/* forward declerations */
struct dhd_offload_obj;

/**
 * Types of DMA buffers shared between DHD and Offload helper
 * Used as sub_ops for dhd_offload_notify ops: D2O_DMA_BUF_NOTIF
 */
typedef enum dhd_offload_dma_buf
{
	DMA_BUF_MASK    = 0xFFFF,

	/** DMA Indices array buffers for: H2D WR and RD, and D2H WR and RD */
	ATTACH_DMA_INDX_BUF    = 1 << 16, /* Alloc uncached and register */
	ATTACH_O2D_WR_BUF      = ATTACH_DMA_INDX_BUF + H2D_DMA_INDX_WR_BUF,
	ATTACH_O2D_RD_BUF      = ATTACH_DMA_INDX_BUF + H2D_DMA_INDX_RD_BUF,
	ATTACH_D2O_WR_BUF      = ATTACH_DMA_INDX_BUF + D2H_DMA_INDX_WR_BUF,
	ATTACH_D2O_RD_BUF      = ATTACH_DMA_INDX_BUF + D2H_DMA_INDX_RD_BUF,

	DETACH_DMA_INDX_BUF    = 1 << 17, /* Free */
	DETACH_O2D_WR_BUF      = DETACH_DMA_INDX_BUF + H2D_DMA_INDX_WR_BUF,
	DETACH_O2D_RD_BUF      = DETACH_DMA_INDX_BUF + H2D_DMA_INDX_RD_BUF,
	DETACH_D2O_WR_BUF      = DETACH_DMA_INDX_BUF + D2H_DMA_INDX_WR_BUF,
	DETACH_D2O_RD_BUF      = DETACH_DMA_INDX_BUF + D2H_DMA_INDX_RD_BUF,


	/** Common Rings and Flowrings DMA buf configuration */
	ATTACH_RING_BUF        = 1 << 18,
	DETACH_RING_BUF        = 1 << 19,
	/* + ring_id to identify the common and flow rings */

} dhd_offload_dma_buf_t;

/**
 * Host(DHD) to Offload requests and Offload to host(DHD) notifications.
 */
typedef enum dhd_offload_ops
{
	/* Host DHD to Offload Notifications (D ---> O) */
	D2O_DMA_BUF_NOTIF,                /* Configure a DMA Buf */
	D2O_INIT_NOTIF,                   /* Complete configuration */
	D2O_RXPOST_NOTIF,                 /* Post initial buffers */
	D2O_RXPOST_FREE_NOTIF,            /* Free buffers in RxPost */
	D2O_TXPOST_NOTIF,                 /* Post tx packets */
	D2O_RX_COMPL_NOTIF,               /* Process D2H RxCompl */
	D2O_TX_COMPL_NOTIF,               /* Process D2H TxCompl */

	D2O_FLRING_INIT_NOTIF,            /* Init flowring cache entry */

	D2O_FLRING_ENAB_NOTIF,            /* Enable a flowring */
	D2O_FLRING_DISAB_NOTIF,           /* Disable a flowring */
	D2O_FLRING_FLUSH_NOTIF,           /* Flush a flowring */
	D2O_AGGR_CONFIG_NOTIF,            /* Configure aggregation */
	D2O_TXSTS_CONFIG_NOTIF,           /* Send nonaccpkt TXSTS */
	D2O_MSGRING_FORMAT_NOTIF,         /* Message ring format and size */
	D2O_RXPOST_REINIT_NOTIF,          /* Re-initialize RxPost buffers */
	D2O_PCIE_IPC_CAP_NOTIF,           /* IPC Capabilities1 (mask, *cap) */
	D2O_PCIE_IPC_CAP1_NOTIF = D2O_PCIE_IPC_CAP_NOTIF, /* for compatibility */
	D2O_FLRING_BQSIZE_NOTIF,          /* update quueue depth */
	D2O_PCAP_CFG_NOTIF,               /* Set PCAP config */

	/* Host DHD to Offload Requests (D ---> O) */
	D2O_IDX_BUF_RD_REQUEST,           /* Read DMA index buffer */
	D2O_IDX_BUF_WR_REQUEST,           /* Write DMA index buffer */

	/* Offload to Host DHD Requests (O ---> D) */
	O2D_RX_COMPL_REQUEST,             /* Receive a packet */
	O2D_TX_COMPL_REQUEST,             /* Free a packet */
	O2D_WAKE_DNGL_REQUEST,            /* Wake dongle */

} dhd_offload_ops_t;

/* configuration structure used for D2O_AGGR_CONFIG_NOTIF */
typedef struct dhd_offload_aggr_config {
	uint32 en_mask;                   /* aggregation enable bit mask */
	uint32 len;                       /* aggregation number of packets */
	uint32 timeout;                   /* aggregation release timeout in msec */
} dhd_offload_aggr_config_t;

/**
 * D2O_TXSTS_CONFIG_NOTIF bitmap values
 */
#define DHD_OFFLOAD_TXSTS_CFG_ACCPKT_OFFL       0x1
#define DHD_OFFLOAD_TXSTS_CFG_NONACCPKT_OFFL    0x2
#define DHD_OFFLOAD_TXSTS_CFG_OFFL_MASK         \
	(DHD_OFFLOAD_TXSTS_CFG_ACCPKT_OFFL|DHD_OFFLOAD_TXSTS_CFG_NONACCPKT_OFFL)

/**
 * D2O_PCIE_IPC_CAP_NOTIF bitmap values
 *
 * The values are not guaranteed to have same as PCIE_IPC_DCAPX_XXXX
 */
#define D2O_PCIE_IPC_CAP_HBQD        0x00000002 /* DCAP2_PCIE_HBQD */
#define D2O_PCIE_IPC_CAP_RX_CMPL_V2  0x00000004 /* DCAP2_RX_CMPL_V2 */
#define D2O_PCIE_IPC_CAP_RX_CMPL_RCH 0x00000008 /* DCAP2_RX_CMPL_RCH */
#define D2O_PCIE_IPC_CAP_BA256CFG    0x00000020 /* DCAP2_BA256 */
#define D2O_PCIE_IPC_CAP_IDMA64      0x00000100 /* DCAP2_IDMA64 */
#define D2O_PCIE_IPC_CAP_DUMMYTXC    0x00001000 /* DCAP2_BQ_DRAIN_NOTIF */
#define D2O_PCIE_IPC_CAP_FFRD        0x00010000 /* DCAP1_FAST_DELETE_RING */

/**
 * D2O_FLRING_BQSIZE_NOTIF bitmap values
 *
 * __________________________________________________________
 * |           SIZE                 |         FLAGS         |
 * ----------------------------------------------------------
 * |15|14|13|12|11|10| 9 | 8 | 7 | 6| 5 | 4 | 3 | 2 | 1 | 0 |
 * ----------------------------------------------------------
 * |                                |               | D | G |
 * ----------------------------------------------------------
 */
/* Size should be in multiples of 64 */
#define FLRING_BQSIZE_NOTIF_SIZE_MASK  (0xFFC0)
#define FLRING_BQSIZE_NOTIF_FLAGS_MASK (0x003F)
#define FLRING_BQSIZE_NOTIF_GROUP      (1ul << 0)
#define FLRING_BQSIZE_NOTIF_DIRECT     (1ul << 1)

/**
 * Configuration key id's
 */
typedef enum dhd_offload_iovar {
	DHD_OFFLOAD_IOV_PROFILE,          /* flowring profile configuration */
	DHD_OFFLOAD_IOV_POLICY,           /* flowring profile configuration */
	DHD_OFFLOAD_IOV_RXOFFL,           /* rx offload configuration */
	DHD_OFFLOAD_IOV_STATS,            /* dhd offload intf stats */
	DHD_OFFLOAD_IOV_DUMP,             /* dhd offload dump */
	DHD_OFFLOAD_IOV_STATUS,           /* dhd offload flowrings */
	DHD_OFFLOAD_IOV_MAX               /* delimiter */
} dhd_offload_iov_t;

/* buffer structure used for DHD_OFFLOAD_IOV_STATS */
typedef struct dhd_offload_stats {
	uint16 ifidx;                     /* interface index */

	uint32 mcast_pkts;                /* TX multicast packets */
	uint32 mcast_bytes;               /* TX multicast bytes */
	uint32 dropped_pkts;              /* dropped TX packets */
} dhd_offload_stats_t;

/* Offload Helper featurs list */
typedef struct dhd_offload_feat {
	uint32 txoffl:1;                  /* Tx post offload */
	uint32 rxoffl:1;                  /* Rx post,complete offload */
	uint32 txcmpl2host:1;             /* Tx complete to host for nonaccpkts */
	uint32 dhdhdr:1;                  /* add llcsnap header */
	uint32 lbraggr:1;                 /* lbr aggregation */
	uint32 msgringformat:3;           /* msg ring formats */
	uint32 bkupq:1;                   /* Offload backup queues */
	uint32 hwawkup:1;                 /* ARM-HWA format to wake up dongle */
	uint32 ffrd:1;                    /* fast flow ring delete support */
	uint32 nplusm:1;                  /* Partial Tx offload mode (N+M) */
	uint32 hbqd:1;                    /* Host backup queue depth */
	uint32 dynbkupq:1;                /* Dynamic flow-ring backup queue size */
	uint32 codel:1;                   /* Offlaod Codel feature */
	uint32 cmnrngsz:1;                /* Configurable Common Ring Sizes */
	uint32 rxcmplv2:1;                /* RxComplete V2 (TID) */
	uint32 rxcmplrch:1;               /* RxComplete RCH (RxC in Headroom) */
	uint32 ba256cfg:1;                /* Rings configuration for ba256 */
	uint32 idma64:1;                  /* iDMA64 groups support */
	uint32 idmagrpcfg:1;              /* iDMA group configuration */
	uint32 pcap:1;                    /* PCAP support */
	uint32 dummytxc:1;                /* Dummy TxComplete WI */
	uint32 txcpl_ewi:1;               /* TxComplete Ext WI */
} dhd_offload_feat_t;

/* buffer structure used for DHD_OFFLOAD_IOV_STATUS */
typedef struct dhd_offload_status {
	dhd_offload_feat_t sup_features;  /* supported offload features */
	dhd_offload_feat_t en_features;   /* enabled offload features */
	uint16 hw_flowrings;              /* Total HW flowrings */
	uint16 sw_flowrings;              /* Total SW flowrings */
} dhd_offload_status_t;

/* structure used for sending non-accelerated TXSTS messages to prot layer */
typedef struct dhd_offload_txsts {
	host_txbuf_cmpl_t dngl_txsts;     /* txsts message from dongle */
	void *pkt;                        /* tx packet */
} dhd_offload_txsts_t;

/* structure used for sending non-accelerated RXCMPL messages to prot layer */
typedef struct dhd_offload_rxcmplt {
	host_rxbuf_cmpl_t dngl_rxcmplt;   /* rx complete message from dongle */
	void *pkt;                        /* rx packet */
} dhd_offload_rxcmplt_t;

/**
 * dhd_offload_attach():
 * Offload allocates a helper object and initializes it.
 * Invoked on insmod (pcie probe), corresponds to dhd_prot_attach().
 *
 * During dhd_prot_init(), a dhd_offload_notify(D2O_INIT_NOTIF) will be invoked
 * to stage the completion of offload initialization.
 *
 * At this point in time, the DHD does not know the number of TxPost flowrings
 * that are advertized by the dongle (dhd_prot_init phase)
 */
extern struct dhd_offload_obj *dhd_offload_attach(dhd_pub_t *dhd,
	bcmpcie_soft_doorbell_t *soft_doobells);

/**
 * dhd_offload_detach():
 * Offload de-initializes itself.
 * Invoked as part of rmmod, dhd_dettach -> dhd_prot_dettach
 */
extern void dhd_offload_detach(dhd_pub_t * dhd, struct dhd_offload_obj *dhdol);

/**
 * dhd_offload_notify():
 * Host DHD notifies Offload to perform an operation.
 */
extern int dhd_offload_notify(struct dhd_offload_obj *dhdol,
	dhd_offload_ops_t ops, unsigned long arg1, unsigned long arg2);

/**
 * dhd_offload_request():
 * Offload requests Host DHD to handle an operation.
 */
extern int dhd_offload_request(struct dhd_offload_obj *dhdol,
	dhd_offload_ops_t ops, unsigned long arg1, unsigned long arg2);

/**
 * dhd_offload_flowmgr_init():
 * Initializes a flowid allocator for Offload.
 */
extern void *dhd_offload_flowmgr_init(struct dhd_offload_obj *dhdol,
	int max_h2d_rings, int max_bss);

/**
 * dhd_offload_flowmgr_fini():
 * Deallocates a flowid allocator for Offload.
 */
extern void *dhd_offload_flowmgr_fini(struct dhd_offload_obj *dhdol,
	void *flowmgr);

/**
 * dhd_offload_flowmgr_alloc():
 * Allocate a flowid, given the sta, wme_ac
 */
extern uint16 dhd_offload_flowmgr_alloc(void *flowmgr, int ifidx, int prio,
	uint8 *mac, bool d11ac, bool *is_hw_ring);

/**
 * dhd_offload_flowmgr_free():
 * Free a previously allocated flowid
 */
extern int dhd_offload_flowmgr_free(void *flowmgr, uint16 flow_id);

/**
 * dhd_offload_do_iov():
 * Fill the iovar buffer with the profiles information
 */
extern int dhd_offload_do_iov(struct dhd_offload_obj *dhdol,
	dhd_offload_iov_t key, bool set, char *buf, int buflen);

/**
 * dhd_offload_process_cpu_queue():
 * Process the Offload helper-> Host CPU Queue for TxComplete and RxComplete
 * messages
 */
extern bool dhd_offload_process_cpu_queue(struct dhd_offload_obj *dhdol,
	bool resched);

#endif /* _dhd_offload_h_ */
