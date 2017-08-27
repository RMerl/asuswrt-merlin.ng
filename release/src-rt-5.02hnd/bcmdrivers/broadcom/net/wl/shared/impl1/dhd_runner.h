/*
 * DHD Runner Helper Interface header file
 *
 * Provides type definitions and function prototypes used to link the
 * DHD to the Runner network processor.
 *
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_runner.h 634379 2016-04-27 20:21:50Z syarra $
 */
#ifndef _dhd_runner_h_
#define _dhd_runner_h_

#include <bcmmsgbuf.h>

/* Enable miscellaneous acceleration features */
#if defined(BCM_DHD_RUNNER)

/* DHD_D2H_SOFT_DOORBELL_SUPPORT enables D2H direct Runner thread wakeup */
#define DHD_D2H_SOFT_DOORBELL_SUPPORT

/**
 * DHD runner add-on features
 * Used by upper layers to query and enable
 */
#define DHD_RNR_LBR_AGGR
#define DHD_RNR_NO_NONACCPKT_TXSTSOFFL

#endif /* BCM_DHD_RUNNER */

/* forward declerations */
struct dhd_runner_hlp;

/**
 * Types of DMA buffers shared between DHD and Runner
 * Used as sub_ops for dhd_runner_notify ops: H2R_DMA_BUF_NOTIF
 */
typedef enum dhd_runner_dma_buf
{
	DMA_BUF_MASK    = 0xFFFF,

	/** DMA Indices array buffers for: H2D WR and RD, and D2H WR and RD */
	ATTACH_DMA_INDX_BUF    = 1 << 16, /* Alloc uncached and register */
	ATTACH_R2D_WR_BUF      = ATTACH_DMA_INDX_BUF + H2D_DMA_INDX_WR_BUF,
	ATTACH_R2D_RD_BUF      = ATTACH_DMA_INDX_BUF + H2D_DMA_INDX_RD_BUF,
	ATTACH_D2R_WR_BUF      = ATTACH_DMA_INDX_BUF + D2H_DMA_INDX_WR_BUF,
	ATTACH_D2R_RD_BUF      = ATTACH_DMA_INDX_BUF + D2H_DMA_INDX_RD_BUF,

	DETACH_DMA_INDX_BUF    = 1 << 17, /* Free */
	DETACH_R2D_WR_BUF      = DETACH_DMA_INDX_BUF + H2D_DMA_INDX_WR_BUF,
	DETACH_R2D_RD_BUF      = DETACH_DMA_INDX_BUF + H2D_DMA_INDX_RD_BUF,
	DETACH_D2R_WR_BUF      = DETACH_DMA_INDX_BUF + D2H_DMA_INDX_WR_BUF,
	DETACH_D2R_RD_BUF      = DETACH_DMA_INDX_BUF + D2H_DMA_INDX_RD_BUF,


	/** Common Rings and Flowrings DMA buf configuration */
	ATTACH_RING_BUF        = 1 << 18,
	DETACH_RING_BUF        = 1 << 19,
	/* + ring_id to identify the common and flow rings */

} dhd_runner_dma_buf_t;


/**
 * Host(DHD) to Runner requests and Runner to host(DHD) notifications.
 */
typedef enum dhd_runner_ops
{
	/* Host DHD to Runner Notifications (H ---> R) */
	H2R_DMA_BUF_NOTIF,      /* Host notifies Runner to configure a DMA Buf */
	H2R_INIT_NOTIF,         /* Host notifies Runner to complete configuration */
	H2R_RXPOST_NOTIF,       /* Host notifies Runner to post initial buffers */
	H2R_RXPOST_FREE_NOTIF,  /* Host notifies Runner to free buffers in RxPost */
	H2R_TXPOST_NOTIF,       /* Host notifies Runner to post tx packets */
	H2R_RX_COMPL_NOTIF,     /* Host notifies Runner to process D2H RxCompl */
	H2R_TX_COMPL_NOTIF,     /* Host notifies Runner to process D2H TxCompl */

	H2R_FLRING_INIT_NOTIF,  /* Host notifies Runner to init flowring cache entry */

	H2R_FLRING_ENAB_NOTIF,  /* Host notifies Runner to enable a flowring */
	H2R_FLRING_DISAB_NOTIF, /* Host notifies Runner to disable a flowring */
	H2R_FLRING_FLUSH_NOTIF, /* Host notifies Runner to flush a flowring */
	H2R_AGGR_CONFIG_NOTIF,  /* Host notifies Runner to configure aggregation */
	H2R_TXSTS_CONFIG_NOTIF, /* Host notifies Runner to send nonaccpkt TXSTS */

	/* Host DHD to Runner Requests (H ---> R) */
	H2R_IDX_BUF_RD_REQUEST, /* Host requests Runner to read DMA index buffer */
	H2R_IDX_BUF_WR_REQUEST, /* Host requests Runner to write DMA index buffer */

	/* Runner to Host DHD Requests (R ---> H) */
	R2H_RX_COMPL_REQUEST,   /* Runner requests Host to receive a packet */
	R2H_TX_COMPL_REQUEST,   /* Runner requests Host to free a packet */
	R2H_WAKE_DNGL_REQUEST,  /* Runner requests Host to wake dongle */

} dhd_runner_ops_t;

/* configuration structure used for H2R_AGGR_CONFIG_NOTIF */
typedef struct dhd_runner_aggr_config {
	uint32 en_mask;       /* aggregation enable bit mask */
	uint32 len;           /* aggregation number of packets */
	uint32 timeout;       /* aggregation release timeout in msec */
} dhd_runner_aggr_config_t;

/**
 * H2R_TXSTS_CONFIG_NOTIF bitmap values
 */
#define DHD_RNR_TXSTS_CFG_ACCPKT_OFFL       0x1
#define DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL    0x2
#define DHD_RNR_TXSTS_CFG_OFFL_MASK         \
	(DHD_RNR_TXSTS_CFG_ACCPKT_OFFL|DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL)

/**
 * Configuration key id's
 */
typedef enum dhd_runner_iovar {
	DHD_RNR_IOVAR_PROFILE,    /* id for flowring profile configuration */
	DHD_RNR_IOVAR_POLICY,     /* id for flowring profile configuration */
	DHD_RNR_IOVAR_RXOFFL,     /* id for rx offload configuration */
	DHD_RNR_IOVAR_RNR_STATS,  /* id for dhd runner intf stats */
	DHD_RNR_IOVAR_DUMP,       /* id for dhd runner dump */
	DHD_RNR_IOVAR_STATUS,     /* id for dhd runner flowrings */
	DHD_RNR_MAX_IOVARS        /* delimiter */
} dhd_runner_iovar_t;

/* buffer structure used for DHD_RNR_IOVAR_RNR_STATS */
typedef struct dhd_helper_rnr_stats {
	uint16 ifidx;            /* interface index */

	uint32 mcast_pkts;       /* TX multicast packets */
	uint32 mcast_bytes;      /* TX multicast bytes */
	uint32 dropped_pkts;     /* dropped TX packets */
} dhd_helper_rnr_stats_t;

typedef struct dhd_helper_feat {
	uint32 txoffl:1;                  /* Tx post offload */
	uint32 rxoffl:1;                  /* Rx post,complete offload */
	uint32 txcmpl2host:1;             /* Tx complete to host for nonaccpkts */
	uint32 dhdhdr:1;                  /* add llcsnap header */
	uint32 lbraggr:1;                 /* lbr aggregation */
} dhd_helper_feat_t;

/* buffer structure used for DHD_RNR_IOVAR_RNR_STATUS */
typedef struct dhd_helper_status {
	dhd_helper_feat_t sup_features;   /* supported runner features */
	dhd_helper_feat_t en_features;    /* enabled runner features */
	uint16 hw_flowrings;              /* Total HW flowrings */
	uint16 sw_flowrings;              /* Total SW flowrings */
} dhd_helper_status_t;

/* structure used for sending non-accelerated TXSTS messages to prot layer */
typedef struct dhd_runner_txsts {
	host_txbuf_cmpl_t dngl_txsts; /* txsts message from dongle */
	void *pkt;                    /* tx packet */
} dhd_runner_txsts_t;

/* structure used for sending non-accelerated RXCMPL messages to prot layer */
typedef struct dhd_runner_rxcmplt {
	host_rxbuf_cmpl_t dngl_rxcmplt; /* rx complete message from dongle */
	void *pkt;                      /* rx packet */
} dhd_runner_rxcmplt_t;

/**
 * dhd_runner_attach():
 * Runner allocates a helper object and initializes it.
 * Invoked on insmod (pcie probe), corresponds to dhd_prot_attach().
 *
 * During dhd_prot_init(), a dhd_runner_notify(H2R_INIT_NOTIF) will be invoked
 * to stage the completion of runner initialization.
 *
 * At this point in time, the DHD does not know the number of TxPost flowrings
 * that are advertized by the dongle (dhd_prot_init phase)
 */
extern struct dhd_runner_hlp *dhd_runner_attach(dhd_pub_t *dhd,
	bcmpcie_soft_doorbell_t *soft_doobells);

/**
 * dhd_runner_detach():
 * Runner de-initializes itself.
 * Invoked as part of rmmod, dhd_dettach -> dhd_prot_dettach
 */
extern void dhd_runner_detach(dhd_pub_t * dhd, struct dhd_runner_hlp *dhd_hlp);


/**
 * dhd_runner_notify():
 * Host DHD notifies Runner to perform an operation.
 */
extern int dhd_runner_notify(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2);

/**
 * dhd_runner_request():
 * Runner requests Host DHD to handle an operation.
 */
extern int dhd_runner_request(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2);

/**
 * dhd_runner_flowmgr_init():
 * Initializes a flowid allocator for Runner.
 */
extern void *dhd_runner_flowmgr_init(struct dhd_runner_hlp *dhd_hlp,
	int max_h2d_rings, int max_bss);

/**
 * dhd_runner_flowmgr_fini():
 * Deallocates a flowid allocator for Runner.
 */
extern void *dhd_runner_flowmgr_fini(struct dhd_runner_hlp *dhd_hlp,
	void *flowmgr);

/**
 * dhd_runner_flowmgr_alloc():
 * Allocate a flowid, given the sta, wme_ac
 */
extern uint16 dhd_runner_flowmgr_alloc(void *flowmgr,
	int ifidx, int prio, uint8 *mac, bool d11ac, bool *is_hw_ring);

/**
 * dhd_runner_flowmgr_free():
 * Free a previously allocated flowid
 */
extern int dhd_runner_flowmgr_free(void *flowmgr, uint16 flow_id);

/**
 * dhd_runner_do_iovar():
 * Fill the iovar buffer with the profiles information
 */
extern int dhd_runner_do_iovar(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_iovar_t key, bool set, char *buf, int buflen);

#endif /* _dhd_runner_h_ */
