/**
 * @file definition of host message ring functionality
 * Provides type definitions and function prototypes used to link the
 * DHD OS, bus, and protocol modules.
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_msgbuf.c 807970 2022-02-07 06:44:58Z $
 */

#include <typedefs.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmmsgbuf.h>
#include <bcmendian.h>
#ifdef WL_MONITOR
#include <bcmwifi_radiotap.h>
#endif /* WL_MONITOR */

#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_proto.h>

#ifdef BCMDBUS
#include <dbus.h>
#else
#include <dhd_bus.h>
#endif /* BCMDBUS */

#include <dhd_dbg.h>
#include <siutils.h>

#include <dhd_flowring.h>

#include <pcie_core.h>
#include <bcmpcie.h>

#include <dhd_pcie.h>
#include <bcm_ring.h>
#include <linux/netlink.h>

#if defined(DHD_LB)
#if !defined(LINUX) && !defined(linux) && !defined(OEM_ANDROID)
#error "DHD Loadbalancing only supported on LINUX | OEM_ANDROID"
#endif /* !LINUX && !OEM_ANDROID */
#include <linux/cpu.h>
#include <bcm_ring.h>
#define DHD_LB_WORKQ_SZ                            (8192)
#define DHD_LB_WORKQ_SYNC           (16)
#define DHD_LB_WORK_SCHED           (DHD_LB_WORKQ_SYNC * 2)
#endif /* DHD_LB */

#ifdef BCM_NBUFF
#include <dhd_nbuff.h>
#endif

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#if defined(BCM_PKTFWD)
#include <dhd_pktfwd.h>
#endif /* BCM_PKTFWD */

#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */

#define LOCKER_SZ			(sizeof(dhd_pktid_item_t))
#define MAX_LOCKERS			(MAX_PKTID_ITEMS + 1)
#define LOCKER_BLOCKS			((MAX_LOCKERS + LOCKERS_PER_BLOCK - 1) / LOCKERS_PER_BLOCK)
#define LOCKERS_PER_BLOCK		(1024)
#define LOCKER_BLOCK_SZ			(LOCKERS_PER_BLOCK * LOCKER_SZ)
#define GET_LOCKER_BLOCK(key_idx)	((key_idx)/LOCKERS_PER_BLOCK)
#define GET_LOCKER_IN_BLOCK(key_idx)	((key_idx)%LOCKERS_PER_BLOCK)
#define GET_LOCKER(map, key_idx)	((dhd_pktid_item_t*)((map)-> \
					locker_blocks[GET_LOCKER_BLOCK((key_idx))]) \
					+ GET_LOCKER_IN_BLOCK((key_idx)))

#include <dhd_macdbg.h>

#if defined(BCM_CPE_PKTC)
#include <wl_pktc.h>
#endif /* BCM_CPE_PKTC */

#ifdef IOCTLRESP_USE_CONSTMEM
#error "IOCTLRESP_USE_CONSTMEM is deprecated."
#endif

/**
 * Host configures a soft doorbell for d2h rings, by specifying a 32bit host
 * address where a value must be written. Host may also interrupt coalescing
 * on this soft doorbell.
 * Use Case: Hosts with network processors, may register with the dongle the
 * network processor's thread wakeup register and a value corresponding to the
 * core/thread context. Dongle will issue a write transaction <address,value>
 * to the PCIE RC which will need to be routed to the mapped register space, by
 * the host.
 */
/* #define DHD_D2H_SOFT_DOORBELL_SUPPORT */

#define RETRIES 2		/* # of retries to retrieve matching ioctl response */

#define DEFAULT_RX_BUFFERS_TO_POST	256
#define RXBUFPOST_THRESHOLD			64
#define RX_BUF_BURST				32 /* Number of Rx buffers posted at a time */

#define DHD_STOP_QUEUE_THRESHOLD	200
#define DHD_START_QUEUE_THRESHOLD	100

#define RX_DMA_OFFSET		8 /* Mem2mem DMA inserts an extra 8 */
#define IOCTL_DMA_BUF_LEN	(RX_DMA_OFFSET + WLC_IOCTL_MAXLEN)

/* flags for ioctl pending status */
#define MSGBUF_IOCTL_ACK_PENDING	(1<<0)
#define MSGBUF_IOCTL_RESP_PENDING	(1<<1)

#define DHD_IOCTL_REQ_PKTBUFSZ(pcie_ipc_rev) ((pcie_ipc_rev) >= 0x85 ? 2560 : 2048)
#define MSGBUF_IOCTL_MAX_RQSTLEN(pcie_ipc_rev) \
	(DHD_IOCTL_REQ_PKTBUFSZ(pcie_ipc_rev) - H2DRING_CTRL_SUB_ITEMSIZE)

#define DMA_D2H_SCRATCH_BUF_LEN	8
#define DHD_DMAXFER_LEN_LIMIT	0x400000

#if defined(BCM_DHDHDR)
#define BCM_DHDHDR_DMA_BUF_LEN (BCM_DHDHDR_MAXIMUM * BCM_DHDHDR_SIZE)
#endif

#ifdef BCM_HOST_MEM_SCB
#ifndef DMA_HOST_BUFFER_LEN
#define DMA_HOST_BUFFER_LEN	0x80000 /* Scratch Host Memory Length */
#endif
#endif /* BCM_HOST_MEM_SCB */

#define DHD_FLOWRING_IOCTL_BUFPOST_PKTSZ		16384

#define DHD_FLOWRING_MAX_EVENTBUF_POST			8
#define DHD_FLOWRING_MAX_IOCTLRESPBUF_POST		8

#define DHD_PROT_FUNCS	37

/* Length of buffer in host for bus throughput measurement */
#define DHD_BUS_TPUT_BUF_LEN 2048

#define TXP_FLUSH_NITEMS

/* optimization to write "n" tx items at a time to ring */
#define TXP_FLUSH_MAX_ITEMS_FLUSH_CNT	48

#define RING_NAME_MAX_LENGTH		24

#if defined(BCM_NBUFF) && defined(BCM_WFD)
#define BCM_WFD_CACHE_FLUSH_LEN  16		/* ethernet header (14) + IP/TOS (2) */
#endif /* BCM_NBUFF && BCM_WFD */

/* Unused item_misc field in a msgbuf_ring */
#define DHD_MSGBUF_RING_ITEM_MISC   (~0)
struct msgbuf_ring; /* ring context for common and flow rings */

/**
 * +--------------------------------------------------------------------------+
 * CSI Monitor handling
 * +--------------------------------------------------------------------------+
 */
#include <bcm_csimon.h>

typedef struct dhd_csimon
{
	csimon_ipc_hme_t      * ipc_hme;
	csimon_ring_elem_t    * table;
	uint32                * wr_ptr;
	uint32                * rd_ptr;
	bcm_ring_t              ring;
	struct sock           * nl_sock;
	uint32                  num_records;
	uint32                  drops; /* example: CSIMON_FILESIZE ovfl */
	uint32                  fopen_fails; /* Failures to open CSIMON file */

} dhd_csimon_t;

/**
 * RxPost, TxPost, RxCmpl and TxCmpl datapath ring handlers
 *
 * Two type of ring message handlers exist:
 * a. Legacy messaging with support of LoadBalancing (pre IPC rev7)
 * b. Legacy and new message formats without LoadBalancing (post IPC rev7)
 *
 * In dhd_prot_preinit time the appropriate message handlers to be used will
 * be attached to the prot layer, and invoked.
 * In a system without LoadBalancing requirement, the post IPC Rev7 handlers
 * may be directly invoked.
 */

/* H2D RxPost ring message handler */
typedef int (*dhd_prot_rxpost_fn_t)(dhd_pub_t *dhd, bool use_rsv_pktid);

/* D2H (Tx and Rx) Completion ring message handler */
typedef int (* dhd_prot_cpln_fn_t)(dhd_pub_t *dhd,
	struct msgbuf_ring *ring, uint8 *buf, uint32 len);

/**
 * PCIE D2H DMA Complete Sync Modes
 *
 * Firmware may interrupt the host, prior to the D2H Mem2Mem DMA completes into
 * Host system memory. A WAR using one of 3 approaches is needed:
 * 1. Dongle places a modulo-253 seqnum in last word of each D2H message
 * 2. XOR Checksum, with epoch# in each work item. Dongle builds an XOR checksum
 *    writes in the last word of each work item. Each work item has a seqnum
 *    number = sequence num % 253.
 *
 * 3. Read Barrier: Dongle does a host memory read access prior to posting an
 *    interrupt, ensuring that D2H data transfer indeed completed.
 * 4. Dongle DMA's all indices after producing items in the D2H ring, flushing
 *    ring contents before the indices.
 *
 * Host does not sync for DMA to complete with option #3 or #4, and a noop sync
 * callback (see dhd_prot_d2h_sync_none) may be bound.
 *
 * Dongle advertizes host side sync mechanism requirements.
 */
#define PCIE_D2H_SYNC

#if defined(PCIE_D2H_SYNC)
#define PCIE_D2H_SYNC_WAIT_TRIES    512

/**
 * Custom callback attached based upon D2H DMA Sync mode advertized by dongle.
 *
 * On success: return cmn_msg_hdr_t::msg_type
 * On failure: return 0 (invalid msg_type)
 */
typedef uint8 (* d2h_sync_cb_t)(dhd_pub_t *dhd, struct msgbuf_ring *ring,
                                cmn_msg_hdr_t *msg, int msglen);
#endif /* PCIE_D2H_SYNC */

/*
 * +----------------------------------------------------------------------------
 *
 * RingIds and FlowId are not equivalent as ringids include D2H rings whereas
 * flowids do not.
 *
 * Dongle advertizes the max H2D rings, as max_sub_queues = 'N' which includes
 * the H2D common rings as well as the (N-BCMPCIE_H2D_COMMON_MSGRINGS) flowrings
 *
 * Here is a sample mapping for (based on PCIE Full Dongle Rev5) where,
 *  BCMPCIE_H2D_COMMON_MSGRINGS = 2, i.e. 2 H2D common rings,
 *  BCMPCIE_COMMON_MSGRINGS     = 5, i.e. include 3 D2H common rings.
 *
 *  H2D Control  Submit   RingId = 0        FlowId = 0 reserved never allocated
 *  H2D RxPost   Submit   RingId = 1        FlowId = 1 reserved never allocated
 *
 *  D2H Control  Complete RingId = 2
 *  D2H Transmit Complete RingId = 3
 *  D2H Receive  Complete RingId = 4
 *
 *  H2D TxPost   FLOWRING RingId = 5         FlowId = 2     (1st flowring)
 *  H2D TxPost   FLOWRING RingId = 6         FlowId = 3     (2nd flowring)
 *  H2D TxPost   FLOWRING RingId = 5 + (N-1) FlowId = (N-1) (Nth flowring)
 *
 * When TxPost FlowId(s) are allocated, the FlowIds [0..FLOWID_RESERVED) are
 * unused, where FLOWID_RESERVED is BCMPCIE_H2D_COMMON_MSGRINGS.
 *
 * Example: when a system supports 4 bc/mc and 128 uc flowrings, with
 * BCMPCIE_H2D_COMMON_MSGRINGS = 2, and BCMPCIE_H2D_COMMON_MSGRINGS = 5, and the
 * FlowId values would be in the range [2..133] and the corresponding
 * RingId values would be in the range [5..136].
 *
 * The flowId allocator, may chose to, allocate Flowids:
 *   bc/mc (per virtual interface) in one consecutive range [2..(2+VIFS))
 *   X# of uc flowids in consecutive ranges (per station Id), where X is the
 *   packet's access category (e.g. 4 uc flowids per station).
 *
 * CAUTION:
 * When DMA indices array feature is used, RingId=5, corresponding to the 0th
 * FLOWRING, will actually use the FlowId as index into the H2D DMA index,
 * since the FlowId truly represents the index in the H2D DMA indices array.
 *
 * Likewise, in the D2H direction, the RingId - BCMPCIE_H2D_COMMON_MSGRINGS,
 * will represent the index in the D2H DMA indices array.
 *
 * +----------------------------------------------------------------------------
 */

/* First TxPost Flowring Id */
#define DHD_FLOWRING_START_FLOWID   BCMPCIE_H2D_COMMON_MSGRINGS

/* Determine whether a ringid belongs to a TxPost flowring */
#define DHD_IS_FLOWRING(ringid) \
	((ringid) >= BCMPCIE_COMMON_MSGRINGS)

/* Convert a H2D TxPost FlowId to a MsgBuf RingId */
#define DHD_FLOWID_TO_RINGID(flowid) \
	(BCMPCIE_COMMON_MSGRINGS + ((flowid) - BCMPCIE_H2D_COMMON_MSGRINGS))

/* Convert a MsgBuf RingId to a H2D TxPost FlowId */
#define DHD_RINGID_TO_FLOWID(ringid) \
	(BCMPCIE_H2D_COMMON_MSGRINGS + ((ringid) - BCMPCIE_COMMON_MSGRINGS))

/* Convert a H2D MsgBuf RingId to an offset index into the H2D DMA indices array
 * This may be used for the H2D DMA WR index array or H2D DMA RD index array or
 * any array of H2D rings.
 */
#define DHD_H2D_RING_OFFSET(ringid) \
	((DHD_IS_FLOWRING(ringid)) ? DHD_RINGID_TO_FLOWID(ringid) : (ringid))

/* Convert a D2H MsgBuf RingId to an offset index into the D2H DMA indices array
 * This may be used for the D2H DMA WR index array or D2H DMA RD index array or
 * any array of D2H rings.
 */
#define DHD_D2H_RING_OFFSET(ringid) \
	((ringid) - BCMPCIE_H2D_COMMON_MSGRINGS)

/* Convert a D2H DMA Indices Offset to a RingId */
#define DHD_D2H_RINGID(offset) \
	((offset) + BCMPCIE_H2D_COMMON_MSGRINGS)

/* XXX: The ringid and flowid and dma indices array index idiosyncracy is error
 * prone. While a simplification is possible, the backward compatability
 * requirement (DHD should operate with any PCIE rev version of firmware),
 * limits what may be accomplished.
 *
 * At the minimum, implementation should use macros for any conversions
 * facilitating introduction of future PCIE FD revs that need more "common" or
 * other dynamic rings.
 */

#define DHD_DMAH_NULL      ((void*)NULL)

/*
 * Pad a DMA-able buffer by an additional cachline. If the end of the DMA-able
 * buffer does not occupy the entire cacheline, and another object is placed
 * following the DMA-able buffer, data corruption may occur if the DMA-able
 * buffer is used to DMAing into (e.g. D2H direction), when HW cache coherency
 * is not available.
 */
#if defined(L1_CACHE_BYTES)
#define DHD_DMA_PAD        (L1_CACHE_BYTES)
#else
#define DHD_DMA_PAD        (128)
#endif

/* Used in loopback tests */
typedef struct dhd_dmaxfer {
	dhd_dma_buf_t src_dma_buf;
	dhd_dma_buf_t dst_dma_buf;
	uint32        srcdelay;
	uint32        destdelay;
	uint32        len;
	bool          in_progress;
} dhd_dmaxfer_t;

/**
 * msgbuf_ring : This object manages the host side ring that includes a DMA-able
 * buffer, the WR and RD indices, ring parameters such as max number of items
 * an length of each items, and other miscellaneous runtime state.
 * A msgbuf_ring may be used to represent a H2D or D2H common ring or a
 * H2D TxPost ring as specified in the PCIE FullDongle Spec.
 * Ring parameters are conveyed to the dongle, which maintains its own peer end
 * ring state. Depending on whether the DMA Indices feature is supported, the
 * host will update the WR/RD index in the DMA indices array in host memory or
 * directly in dongle memory.
 */
typedef struct msgbuf_ring {
	uint8          inited;
	uint8          flag;
	uint16         id;        /* ring id */
	uint16         rd;        /* read index */
	uint16         wr;        /* write index */
	uint16         item_type; /* legacy, cwi32, cwi64, acwi32, acwi64 */
	uint16         item_size; /* length of each item in the ring */
	uint16         max_items; /* maximum number of items in ring */
	uint16         item_misc; /* for miscellaneous use, per ring field */
	haddr64_t      base_addr; /* LITTLE ENDIAN formatted: base address */
	dhd_dma_buf_t  dma_buf;   /* DMA-able buffer: pa, va, len, dmah, secdma */
	uint32         seqnum;    /* next expected item's sequence number */
#ifdef TXP_FLUSH_NITEMS
	void           *start_addr;
	/* # of messages on ring not yet announced to dongle */
	uint16         pend_items_count;
#endif /* TXP_FLUSH_NITEMS */
	uint16         idma_index; /* idma index 0 ~ 15 */
	uint16         qd;         /* posted queue depth */
	uchar		name[RING_NAME_MAX_LENGTH];
} msgbuf_ring_t;

#define DHD_RING_BGN_VA(ring)           ((ring)->dma_buf.va)
#define DHD_RING_END_VA(ring) \
	((uint8 *)(DHD_RING_BGN_VA((ring))) + \
	 (((ring)->max_items - 1) * (ring)->item_size))

#if defined(BCM_ROUTER_DHD) && defined(BCM_CPE_PKTC)
/** D2H WLAN Rx Packet Chaining context */
typedef struct rxchain_info {
	uint		pkt_count;
	uint		ifidx;
	void		*pkthead;
	void		*pkttail;
	uint8		*h_da;	/* pointer to da of chain head */
	uint8		*h_sa;	/* pointer to sa of chain head */
	uint8		h_prio; /* prio of chain head */
} rxchain_info_t;
#endif /* BCM_ROUTER_DHD && BCM_CPE_PKTC */

/**
 * Host Memory Extensions (HME) service support in PCIE IPC Revision 0x82
 */
typedef struct dhd_hme {
	uint32          users;              /* total HME users advertised         */
	uint32          pages;              /* total pages of all regions         */
	uint32          bytes;              /* total bytes of all regions         */
	dhd_dma_buf_t  *user_dma_buf;       /* ptr to table of user dma_buf       */
	dhd_dma_buf_t  user_haddr64_dma_buf; /* dma_buf for table of user haddr64 */
} dhd_hme_t;

/** DHD protocol handle. Is an opaque type to other DHD software layers. */
typedef struct dhd_prot {
	osl_t *osh;		/* OSL handle */

	dhd_prot_rxpost_fn_t rxpost_fn; /* H2D RxPost ring producer handler */

	uint16 rxbufpost; /* count of posted rx buffers. see RXBUFPOST_THRESHOLD */
	uint16 max_rxbufpost; /* max number of rx buffers that may be posted */
	uint16 inprogress_rxbufpost; /* avoid re-entrancy to rx buffer posting */

	uint16 max_eventbufpost;
	uint16 max_ioctlrespbufpost;

	uint16 cur_event_bufs_posted;
	uint16 cur_ioctlresp_bufs_posted;

	/* Flow control mechanism based on active transmits pending */
	uint16 active_tx_count; /* increments on every packet tx, and decrements on tx_status */
	uint16 txp_threshold;  /* optimization to write "n" tx items at a time to ring */

	uint8  txpost_hdr_offset; /* offset of ethernet header in work item */
	uint16 txpost_item_type; /* legacy, cwi32, cwi64 */
	uint16 txpost_item_size; /* len of the work item */
	uint16 txpost_max_items; /* max items in a txpost ring */
	uint16 txpost_item_misc; /* 0 as no acwi format is supported */

	bool   use_haddr64;
	uint32 host_physaddrhi;

	dhd_prot_cpln_fn_t txcpln_fn; /* D2H Tx Completion consumer handler */
	dhd_prot_cpln_fn_t rxcpln_fn; /* D2H Rx Completion consumer handler */

	/* MsgBuf Ring info: has a dhd_dma_buf that is dynamically allocated */
	msgbuf_ring_t h2dring_ctrl_subn; /* H2D ctrl message submission ring */
	msgbuf_ring_t h2dring_rxp_subn; /* H2D RxBuf post ring */
	msgbuf_ring_t d2hring_ctrl_cpln; /* D2H ctrl completion ring */
	msgbuf_ring_t d2hring_tx_cpln; /* D2H Tx complete message ring */
	msgbuf_ring_t *d2hring_rx_cpln; /* D2H Rx complete message ring */

	msgbuf_ring_t *h2d_flowrings_pool; /* Pool of preallocated flowings */
	dhd_dma_buf_t flowrings_dma_buf; /* Contiguous DMA buffer for flowrings */
	uint16        max_h2d_rings; /* total H2D (common rings + flowrings) */

	uint32        dma_rxoffset;

	/* H2D Doorbell to dongle CPU or HWA */
	dhd_mb_ring_t   mb_ring_fn;
	dhd_mb_ring_2_t mb_2_ring_fn;
	dhd_mb_ring_2_t db1_2_ring_fn;  /* HWA iDMA */

	/* ioctl related resources */
	uint8 ioctl_state;
	int16 ioctl_status;     /* status returned from dongle */
	uint16 ioctl_resp_len;
	uint ioctl_received;

	dhd_dma_buf_t   ioctl_resp_dma_buf; /* For holding ioctl response */
	dhd_dma_buf_t   ioctl_rqst_dma_buf; /* For holding ioctl request */

	/* Extension of dongle memory in host - legacy scratch buffer carving */
	dhd_dma_buf_t   host_mem_dma_buf;

	/* DMA-able arrays for holding WR and RD indices */
	uint32          rw_index_sz; /* Size of a RD or WR index in dongle */
	dhd_dma_buf_t   h2d_dma_indx_wr_buf;    /* Array of H2D WR indices */
	dhd_dma_buf_t   h2d_dma_indx_rd_buf;    /* Array of H2D RD indices */
	dhd_dma_buf_t   d2h_dma_indx_wr_buf;    /* Array of D2H WR indices */
	dhd_dma_buf_t   d2h_dma_indx_rd_buf;    /* Array of D2H RD indices */

	dhd_dma_buf_t   bus_tput_dma_buf; /* pcie_bus_tput measure buffer */

	dhd_dma_buf_t   *flowring_buf;    /* pool of flow ring buf */
	uint32          flowring_num;
#if defined(BCM_DHD_RUNNER)
	void             *flring_cache; /* Cache of Runner flowring contexts */
#endif /* BCM_DHD_RUNNER */

#if defined(PCIE_D2H_SYNC)
	d2h_sync_cb_t d2h_sync_cb; /* Sync on D2H DMA done: SEQNUM or XORCSUM */
	ulong d2h_sync_wait_max; /* max number of wait loops to receive one msg */
	ulong d2h_sync_wait_tot; /* total wait loops */
#endif  /* PCIE_D2H_SYNC */

	dhd_dmaxfer_t   dmaxfer; /* for test/DMA loopback */

	/* Host Memory Extension service in PCIE IPC Rev 0x82 */
	dhd_hme_t       hme;
	dhd_csimon_t    csimon;

	uint16		ioctl_seq_no;
	uint16		data_seq_no;
	uint16		ioctl_trans_id;
	void		*pktid_map_handle; /* a pktid maps to a packet and its metadata */
	void		*pktid_map_handle_ioctl;

	/* Applications/utilities can read tx and rx metadata using IOVARs */
	uint16		tx_metadata_offset;

#if defined(BCM_ROUTER_DHD) && defined(BCM_CPE_PKTC)
	rxchain_info_t	rxchain;	/* chain of rx packets */
#endif

#if defined(DHD_D2H_SOFT_DOORBELL_SUPPORT)
	/* Host's soft doorbell configuration */
	bcmpcie_soft_doorbell_t soft_doorbell[BCMPCIE_D2H_COMMON_MSGRINGS];
#endif /* DHD_D2H_SOFT_DOORBELL_SUPPORT */
#if defined(DHD_LB)
	/* Work Queues to be used by the producer and the consumer, and threshold
	 * when the WRITE index must be synced to consumer's workq
	 */
#if defined(DHD_LB_TXC)
	uint32 tx_compl_prod_sync ____cacheline_aligned;
	bcm_workq_t tx_compl_prod, tx_compl_cons;
#endif /* DHD_LB_TXC */
#if defined(DHD_LB_RXC)
	uint32 rx_compl_prod_sync ____cacheline_aligned;
	bcm_workq_t rx_compl_prod, rx_compl_cons;
#endif /* DHD_LB_RXC */
#endif /* DHD_LB */
#ifdef BCMHWA
	uint32		hwa_caps;
	bool		rxpost_stop;
#endif
	uint8		max_rxcpln_rings; /* max number of rxcpl ring */
	/* re-post rxbuffer when intermittent running out of BPM buffer */
	bool		rxpost_fail;
} dhd_prot_t;

#ifdef BCMHWA
/* uint8 flag in msgbuf_ring */
#define DHD_RING_FLAG_HWA_RXPOST	(1 << 0)
#define DHD_RING_FLAG_HWA_TXCPL		(1 << 1)
#define DHD_RING_FLAG_HWA_RXCPL		(1 << 2)
#define DHD_RXCPL_DRAIN_WAIT_TIME   100      /* msec */

#define HWA_RXPOST_ACTIVE(r) ((r)->flag & DHD_RING_FLAG_HWA_RXPOST)
#define HWA_TXCPL_ACTIVE(r) ((r)->flag & DHD_RING_FLAG_HWA_TXCPL)
#define HWA_RXCPL_ACTIVE(r) ((r)->flag & DHD_RING_FLAG_HWA_RXCPL)
#else
#define HWA_RXPOST_ACTIVE(r)		FALSE
#define HWA_TXCPL_ACTIVE(r)		FALSE
#define HWA_RXCPL_ACTIVE(r)		FALSE
#endif /* BCMHWA */

/* Fetch the msgbuf_ring_t from the d2hring_rx_cpln given a index */
#define DHD_RING_IN_D2HRING_RX_CPLN(prot, idx) \
	(msgbuf_ring_t*)((prot)->d2hring_rx_cpln) + idx

/* Traverse each rx_cpln in the d2hring_rx_cpln, assigning ring and index */
#define FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, idx) \
	for ((idx) = 0, \
		 (ring) = DHD_RING_IN_D2HRING_RX_CPLN(prot, idx); \
		 (idx) < (prot)->max_rxcpln_rings; \
		 (idx)++, (ring)++)

/* Convert a dmaaddr_t to a base_addr with htol operations */
static INLINE void dhd_base_addr_htolpa(haddr64_t *base_addr, dmaaddr_t pa);

/* Legacy Scratch Host memory DMA-able buffer (pre-HME Service) */
static int  dhd_prot_host_mem_alloc(dhd_pub_t *dhd);
static void dhd_prot_host_mem_free(dhd_pub_t *dhd);

/* msgbuf ring management */
static int dhd_prot_ring_alloc(dhd_pub_t *dhd,
	msgbuf_ring_t *ring, const char *name, uint16 ringid,
	uint16 item_type, uint16 item_size, uint16 max_items, uint16 item_misc);
static void dhd_prot_ring_init(dhd_pub_t *dhd, msgbuf_ring_t *ring);
static void dhd_prot_ring_reset(dhd_pub_t *dhd, msgbuf_ring_t *ring);
static void dhd_prot_ring_free(dhd_pub_t *dhd, msgbuf_ring_t *ring);

/* Pool of pre-allocated msgbuf_ring_t with DMA-able buffers for Flowrings */
static int  dhd_prot_flowrings_pool_alloc(dhd_pub_t *dhd);
static void dhd_prot_flowrings_pool_reset(dhd_pub_t *dhd);
static void dhd_prot_flowrings_pool_free(dhd_pub_t *dhd);

/* Fetch and Release a flowring msgbuf_ring from flowring  pool */
static msgbuf_ring_t *dhd_prot_flowrings_pool_fetch(dhd_pub_t *dhd,
	uint16 flowid);
/* see also dhd_prot_flowrings_pool_release() in dhd_prot.h */

/* Producer: Allocate space in a msgbuf ring */
static void* dhd_prot_alloc_ring_space(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	uint16 nitems, uint16 *alloced, bool exactly_nitems);
static void* dhd_prot_get_ring_space(msgbuf_ring_t *ring, uint16 nitems,
	uint16 *alloced, bool exactly_nitems);

/* Consumer: Determine the location where the next message may be consumed */
static uint8* dhd_prot_get_read_addr(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	uint32 *available_len);

/* Producer (WR index update) or Consumer (RD index update) indication */
static void dhd_prot_ring_write_complete(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	void *p, uint16 len);
static void dhd_prot_flow_ring_write_complete(dhd_pub_t *dhd,
	flow_ring_node_t *flow_ring_node, void *p, uint16 len);
static void dhd_prot_upd_read_idx(dhd_pub_t *dhd, msgbuf_ring_t *ring);

/* Allocate DMA-able memory for saving H2D/D2H WR/RD indices */
static INLINE int dhd_prot_dma_indx_alloc(dhd_pub_t *dhd, uint8 type,
	dhd_dma_buf_t *dma_buf, const char *str, uint32 bufsz);

/* Set/Get a RD or WR index in the array of indices */
/* See also: dhd_prot_dma_indx_init() */
static void dhd_prot_dma_indx_set(dhd_pub_t *dhd, uint16 new_index, uint8 type,
	uint16 ringid);
static uint16 dhd_prot_dma_indx_get(dhd_pub_t *dhd, uint8 type, uint16 ringid);

/* Set/Get RD/WR split dma index in the array of indices */
static void dhd_prot_pcie_hbqd_indx_set(dhd_pub_t *dhd, uint32 new_index,
	uint8 type, uint16 ringid);
static uint32 dhd_prot_pcie_hbqd_indx_get(dhd_pub_t *dhd, uint8 type,
	uint16 ringid);

/* Locate a packet given a pktid */
static INLINE void *dhd_prot_packet_get(dhd_pub_t *dhd, uint32 pktid, uint8 pkttype,
	bool free_pktid);
/* Locate a packet given a PktId and free it. */
static INLINE void dhd_prot_packet_free(dhd_pub_t *dhd, uint32 pktid, uint8 pkttype);

static int dhd_msgbuf_query_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd,
	void *buf, uint len, uint8 action);
static int dhd_msgbuf_set_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd,
	void *buf, uint len, uint8 action);
static int dhd_msgbuf_wait_ioctl_cmplt(dhd_pub_t *dhd, uint32 len, void *buf);
static int dhd_fillup_ioct_reqst(dhd_pub_t *dhd, uint16 len, uint cmd,
	void *buf, int ifidx);

/* Post buffers for control ioctl response and events */
static uint16 dhd_msgbuf_rxbuf_post_ctrlpath(dhd_pub_t *dhd, bool event_buf, uint32 max_to_post);
static void dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd_pub_t *pub);
static void dhd_msgbuf_rxbuf_post_event_bufs(dhd_pub_t *pub);

/* Post buffers for 802.3 data packets reception via RxPost ring */
static INLINE void dhd_msgbuf_rxbuf_post_refresh(dhd_pub_t *dhd);
static void dhd_msgbuf_rxbuf_post(dhd_pub_t *dhd, bool use_rsv_pktid);

/* Common RxBuf posting with support for all message formats:
 * Legacy, CWI32, CWI64, ACWI32, ACWI64
 */
static int dhd_prot_rxbuf_post(dhd_pub_t *dhd, bool use_rsv_pktid);

#if defined(DHD_LB)
/* RxPost ring message handler */
static int dhd_prot_rxbuf_post_lb(dhd_pub_t *dhd, bool use_rsv_pktid);
#endif /* DHD_LB_RXC */

static void dhd_prot_return_rxbuf(dhd_pub_t *dhd, uint32 pktid, uint32 rxcnt);

/* Generic D2H Message handling, with LoadBalancing and D2H DMA Sync support */
static int dhd_prot_process_msgtype(dhd_pub_t *dhd, /* Legacy */
	msgbuf_ring_t *ring, uint8 *buf, uint32 len);

/** Support for Legacy, Compact and Aggregated Formats (32b and 64b) */
/* Custom bulk D2H completion processing handlers (Legacy + CWI formats) */
static int dhd_prot_process_txcpln(dhd_pub_t *dhd,
	msgbuf_ring_t *ring, uint8 *buf, uint32 len); // dhd_prot_txstatus_process
static int dhd_prot_process_rxcpln(dhd_pub_t *dhd,
	msgbuf_ring_t *ring, uint8 *buf, uint32 len); // dhd_prot_rxcmplt_process

/* D2H Message handlers */
static void dhd_prot_noop(dhd_pub_t *dhd, void *msg);
static void dhd_prot_ioctcmplt_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_ioctack_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_ringstatus_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_genstatus_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_event_process(dhd_pub_t *dhd, void *msg);

static void dhd_prot_txstatus_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_rxcmplt_process(dhd_pub_t *dhd, void *msg);

/* Loopback test with dongle */
static void dmaxfer_free_dmaaddr(dhd_pub_t *dhd, dhd_dmaxfer_t *dma);
static int dmaxfer_prepare_dmaaddr(dhd_pub_t *dhd, uint len, uint srcdelay,
	uint destdelay, dhd_dmaxfer_t *dma);
static void dhd_msgbuf_dmaxfer_process(dhd_pub_t *dhd, void *msg);

#ifdef WL_MONITOR
/* Monitor Mode */
extern bool dhd_monitor_enabled(dhd_pub_t *dhd, int ifidx);
extern void dhd_rx_mon_pkt(dhd_pub_t *dhdp, host_rxbuf_cmpl_t* msg, void *pkt, int ifidx);
#endif /* WL_MONITOR */

/* Flowring management communication with dongle */
static void dhd_prot_flow_ring_create_response_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_flow_ring_delete_response_process(dhd_pub_t *dhd, void *msg);
static void dhd_prot_flow_ring_flush_response_process(dhd_pub_t *dhd, void *msg);

/* Configure a soft doorbell per D2H ring */
static void dhd_msgbuf_ring_config_d2h_soft_doorbell(dhd_pub_t *dhd);
static void dhd_prot_d2h_ring_config_cmplt_process(dhd_pub_t *dhd, void *msg);

#if defined(BCM_DHD_RUNNER)
static int dhd_prot_runner_preinit(dhd_pub_t *dhdp);
#endif /* BCM_DHD_RUNNER */
typedef void (*dhd_msgbuf_func_t)(dhd_pub_t *dhd, void *msg);

/** callback functions for messages generated by the dongle */
#define MSG_TYPE_INVALID 0

static dhd_msgbuf_func_t table_lookup[DHD_PROT_FUNCS] = {
	dhd_prot_noop, /* 0 is MSG_TYPE_INVALID */
	dhd_prot_genstatus_process, /* MSG_TYPE_GEN_STATUS */
	dhd_prot_ringstatus_process, /* MSG_TYPE_RING_STATUS */
	NULL,
	dhd_prot_flow_ring_create_response_process, /* MSG_TYPE_FLOW_RING_CREATE_CMPLT */
	NULL,
	dhd_prot_flow_ring_delete_response_process, /* MSG_TYPE_FLOW_RING_DELETE_CMPLT */
	NULL,
	dhd_prot_flow_ring_flush_response_process, /* MSG_TYPE_FLOW_RING_FLUSH_CMPLT */
	NULL,
	dhd_prot_ioctack_process, /* MSG_TYPE_IOCTLPTR_REQ_ACK */
	NULL,
	dhd_prot_ioctcmplt_process, /* MSG_TYPE_IOCTL_CMPLT */
	NULL,
	dhd_prot_event_process, /* MSG_TYPE_WL_EVENT */
	NULL,
	dhd_prot_txstatus_process, /* MSG_TYPE_TX_STATUS */
	NULL,
	dhd_prot_rxcmplt_process, /* MSG_TYPE_RX_CMPLT */
	NULL,
	dhd_msgbuf_dmaxfer_process, /* MSG_TYPE_LPBK_DMAXFER_CMPLT */
	NULL, /* MSG_TYPE_FLOW_RING_RESUME */
	NULL, /* MSG_TYPE_FLOW_RING_RESUME_CMPLT */
	NULL, /* MSG_TYPE_FLOW_RING_SUSPEND */
	NULL, /* MSG_TYPE_FLOW_RING_SUSPEND_CMPLT */
	NULL, /* MSG_TYPE_INFO_BUF_POST */
	NULL, /* MSG_TYPE_INFO_BUF_CMPLT */
	NULL, /* MSG_TYPE_H2D_RING_CREATE */
	NULL, /* MSG_TYPE_D2H_RING_CREATE */
	NULL, /* MSG_TYPE_H2D_RING_CREATE_CMPLT */
	NULL, /* MSG_TYPE_D2H_RING_CREATE_CMPLT */
	NULL, /* MSG_TYPE_H2D_RING_CONFIG */
	NULL, /* MSG_TYPE_D2H_RING_CONFIG */
	NULL, /* MSG_TYPE_H2D_RING_CONFIG_CMPLT */
	dhd_prot_d2h_ring_config_cmplt_process, /* MSG_TYPE_D2H_RING_CONFIG_CMPLT */
	NULL, /* MSG_TYPE_H2D_MAILBOX_DATA */
	NULL, /* MSG_TYPE_D2H_MAILBOX_DATA */
};

#if (defined(BCM_ROUTER_DHD) && defined(BCM_CPE_PKTC)) && !defined(BCM_DHD_RUNNER)
/* Related to router CPU mapping per radio core */
#define DHD_RX_CHAINING
#endif /* ((BCM_ROUTER_DHD && BCM_CPE_PKTC) && !BCM_DHD_RUNNER) */

#ifdef DHD_RX_CHAINING

#if defined(BCM_CPE_PKTC)
#define PKTC_TBL_CHAINABLE(dhd, ifidx, evh, prio, h_sa, h_da, h_prio) \
	(dhd_rx_pktc_tbl_chainable((dhd), (ifidx)) && \
	!ETHER_ISNULLDEST(((struct ether_header *)(evh))->ether_dhost) && \
	!ETHER_ISMULTI(((struct ether_header *)(evh))->ether_dhost) && \
	!eacmp((h_da), ((struct ether_header *)(evh))->ether_dhost) && \
	!eacmp((h_sa), ((struct ether_header *)(evh))->ether_shost) && \
	((h_prio) == (prio)) && (dhd_pktc_tbl_check((dhd), (evh), (ifidx))) && \
	((((struct ether_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IP)) || \
	(((struct ether_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IPV6))))
#else
#define PKT_CTF_CHAINABLE(dhd, ifidx, evh, prio, h_sa, h_da, h_prio) \
	(dhd_rx_pkt_chainable((dhd), (ifidx)) && \
	!ETHER_ISNULLDEST(((struct ether_header *)(evh))->ether_dhost) && \
	!ETHER_ISMULTI(((struct ether_header *)(evh))->ether_dhost) && \
	!eacmp((h_da), ((struct ether_header *)(evh))->ether_dhost) && \
	!eacmp((h_sa), ((struct ether_header *)(evh))->ether_shost) && \
	((h_prio) == (prio)) && (dhd_ctf_hotbrc_check((dhd), (evh), (ifidx))) && \
	((((struct ether_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IP)) || \
	(((struct ether_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IPV6))))
#endif /* BCM_CPE_PKTC */

static INLINE void BCMFASTPATH dhd_rxchain_reset(rxchain_info_t *rxchain);
static void BCMFASTPATH dhd_rxchain_frame(dhd_pub_t *dhd, void *pkt, uint ifidx);
static void BCMFASTPATH dhd_rxchain_commit(dhd_pub_t *dhd);

#define DHD_PKT_CTF_MAX_CHAIN_LEN	64

#endif /* DHD_RX_CHAINING */

static void dhd_prot_h2d_sync_init(dhd_pub_t *dhd);

#if defined(PCIE_D2H_SYNC) /* avoids problems related to host CPU cache */

/**
 * D2H DMA to completion callback handlers. Based on the mode advertised by the
 * dongle through the PCIE shared region, the appropriate callback will be
 * registered in the proto layer to be invoked prior to precessing any message
 * from a D2H DMA ring. If the dongle uses a read barrier or another mode that
 * does not require host participation, then a noop callback handler will be
 * bound that simply returns the msg_type.
 */
static void dhd_prot_d2h_sync_livelock(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                                       uint32 tries, uchar *msg, int msglen);
static uint8 dhd_prot_d2h_sync_seqnum(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                                      cmn_msg_hdr_t *msg, int msglen);
static uint8 dhd_prot_d2h_sync_xorcsum(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                                       cmn_msg_hdr_t *msg, int msglen);
static uint8 dhd_prot_d2h_sync_none(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                                    cmn_msg_hdr_t *msg, int msglen);
static void dhd_prot_d2h_sync_init(dhd_pub_t *dhd);

/**
 * dhd_prot_d2h_sync_livelock - when the host determines that a DMA transfer has
 * not completed, a livelock condition occurs. Host will avert this livelock by
 * dropping this message and moving to the next. This dropped message can lead
 * to a packet leak, or even something disastrous in the case the dropped
 * message happens to be a control response.
 * Here we will log this condition. One may choose to reboot the dongle.
 *
 */
static void
dhd_prot_d2h_sync_livelock(dhd_pub_t *dhd, msgbuf_ring_t *ring, uint32 tries,
                           uchar *msg, int msglen)
{
	uint32 seqnum = ring->seqnum;

	DHD_ERROR(("LIVELOCK DHD<%p> seqnum<%u:%u> tries<%u> max<%lu> tot<%lu>"
		"dma_buf va<%p> msg<%p>\n",
		dhd, seqnum, seqnum% D2H_EPOCH_MODULO, tries,
		dhd->prot->d2h_sync_wait_max, dhd->prot->d2h_sync_wait_tot,
		ring->dma_buf.va, msg));
	prhex("D2H MsgBuf Failure", (uchar *)msg, msglen);

#ifdef OEM_ANDROID
#if defined(SUPPORT_LINKDOWN_RECOVERY) && defined(CONFIG_ARCH_MSM)
	dhd->bus->islinkdown = 1;
	dhd_os_check_hang(dhd, 0, -ETIMEDOUT);
#endif /* SUPPORT_LINKDOWN_RECOVERY && CONFIG_ARCH_MSM */
#endif /* OEM_ANDROID */
}

/**
 * dhd_prot_d2h_sync_seqnum - Sync on a D2H DMA completion using the SEQNUM
 * mode. Sequence number is always in the last word of a message.
 */
static uint8 BCMFASTPATH
dhd_prot_d2h_sync_seqnum(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                         cmn_msg_hdr_t *msg, int msglen)
{
	uint32 tries;
	uint32 ring_seqnum = ring->seqnum % D2H_EPOCH_MODULO;
	int num_words = msglen / sizeof(uint32); /* num of 32bit words */
	volatile uint32 *marker = (uint32 *)msg + (num_words - 1); /* last word */
	dhd_prot_t *prot = dhd->prot;

	ASSERT(msglen == ring->item_size);

	for (tries = 0; tries < PCIE_D2H_SYNC_WAIT_TRIES; tries++) {
		uint32 msg_seqnum = *marker;
		if (ltoh32(msg_seqnum) == ring_seqnum) { /* dma upto last word done */
			ring->seqnum++; /* next expected sequence number */
			goto dma_completed;
		}

		if (tries > prot->d2h_sync_wait_max)
			prot->d2h_sync_wait_max = tries;

		OSL_CACHE_INV(msg, msglen); /* invalidate and try again */
#if !(defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
		OSL_CPU_RELAX(); /* CPU relax for msg_seqnum  value to update */
#endif /* !(defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)) */

	} /* for PCIE_D2H_SYNC_WAIT_TRIES */

	dhd_prot_d2h_sync_livelock(dhd, ring, tries, (uchar *)msg, msglen);

	ring->seqnum++; /* skip this message ... leak of a pktid */
	return MSG_TYPE_INVALID; /* invalid msg_type 0 -> noop callback */

dma_completed:
	prot->d2h_sync_wait_tot += tries;

	return msg->msg_type;
}

/**
 * dhd_prot_d2h_sync_xorcsum - Sync on a D2H DMA completion using the XORCSUM
 * mode. The xorcsum is placed in the last word of a message. Dongle will also
 * place a seqnum in the epoch field of the cmn_msg_hdr.
 */
static uint8 BCMFASTPATH
dhd_prot_d2h_sync_xorcsum(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                          cmn_msg_hdr_t *msg, int msglen)
{
	uint32 tries;
	uint32 prot_checksum = 0; /* computed checksum */
	int num_words = msglen / sizeof(uint32); /* num of 32bit words */
	uint8 ring_seqnum = ring->seqnum % D2H_EPOCH_MODULO;
	dhd_prot_t *prot = dhd->prot;

	ASSERT(msglen == ring->item_size);

	for (tries = 0; tries < PCIE_D2H_SYNC_WAIT_TRIES; tries++) {
		prot_checksum = bcm_compute_xor32((volatile uint32 *)msg, num_words);
		if (prot_checksum == 0U) { /* checksum is OK */
			if (msg->epoch == ring_seqnum) {
				ring->seqnum++; /* next expected sequence number */
				goto dma_completed;
			}
		}

		if (tries > prot->d2h_sync_wait_max)
			prot->d2h_sync_wait_max = tries;

		OSL_CACHE_INV(msg, msglen); /* invalidate and try again */
#if !(defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
		OSL_CPU_RELAX(); /* CPU relax for msg_seqnum  value to update */
#endif /* !(defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)) */

	} /* for PCIE_D2H_SYNC_WAIT_TRIES */

	dhd_prot_d2h_sync_livelock(dhd, ring, tries, (uchar *)msg, msglen);

	ring->seqnum++; /* skip this message ... leak of a pktid */
	return MSG_TYPE_INVALID; /* invalid msg_type 0 -> noop callback */

dma_completed:
	prot->d2h_sync_wait_tot += tries;

	return msg->msg_type;
}

/**
 * dhd_prot_d2h_sync_none - Dongle ensure that the DMA will complete and host
 * need to try to sync. This noop sync handler will be bound when the dongle
 * advertises that neither the SEQNUM nor XORCSUM mode of DMA sync is required.
 */
static uint8 BCMFASTPATH
dhd_prot_d2h_sync_none(dhd_pub_t *dhd, msgbuf_ring_t *ring,
                       cmn_msg_hdr_t *msg, int msglen)
{
	return msg->msg_type;
}

/**
 * dhd_prot_d2h_sync_init - Setup the host side DMA sync mode based on what
 * dongle advertizes.
 */
static void
dhd_prot_d2h_sync_init(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring;
	uint16 i;

	prot->d2h_sync_wait_max = 0UL;
	prot->d2h_sync_wait_tot = 0UL;

	prot->d2hring_ctrl_cpln.seqnum = D2H_EPOCH_INIT_VAL;
	prot->d2hring_tx_cpln.seqnum = D2H_EPOCH_INIT_VAL;

	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
		ring->seqnum = D2H_EPOCH_INIT_VAL;
	}

	if (dhd->d2h_sync_mode & PCIE_IPC_FLAGS_D2H_SYNC_SEQNUM) {
		prot->d2h_sync_cb = dhd_prot_d2h_sync_seqnum;
	} else if (dhd->d2h_sync_mode & PCIE_IPC_FLAGS_D2H_SYNC_XORCSUM) {
		prot->d2h_sync_cb = dhd_prot_d2h_sync_xorcsum;
	} else {
		prot->d2h_sync_cb = dhd_prot_d2h_sync_none;
	}
}

#endif /* PCIE_D2H_SYNC */

/**
 * dhd_prot_h2d_sync_init - Per H2D common ring, setup the msgbuf ring seqnum
 */
static void
dhd_prot_h2d_sync_init(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	prot->h2dring_rxp_subn.seqnum = H2D_EPOCH_INIT_VAL;
	prot->h2dring_ctrl_subn.seqnum = H2D_EPOCH_INIT_VAL;
}

/* +-----------------  End of PCIE DHD H2D DMA SYNC ------------------------+ */

/*
 * +---------------------------------------------------------------------------+
 *
 *  DHD support for Host Memory Extension (HME) service (PCIE IPC Rev 0x82)
 *
 *  PCIE IPC specifies upto N Users that may request memory in host DDR, to
 *  serve as extensions of Dongle memory. HME regions flagged as 'PRIVATE' may
 *  use un-cacheable memory, or even carved out of a (CFE)-allocated region.
 *
 *  Memory is requested in units of HME pages (4 KBytes). An extension is
 *  bounded to (4 MBytes - 4 KBytes), i.e.  one page less of 4MB. A table of
 *  physical addresses is maintained in the prot layer in LittleEndian form,
 *  accessible to dongle via the pcie_ipc::host_mem_haddr64.
 *
 *  See dhd_prot_init() where the pcie_ipc::host_mem_len and host_mem_haddr64
 *  get populated.
 *
 *  During Host-Dongle PCIE IPC training stage, DHD bus layer explicitly resets
 *  HME (to free any previously allocated DMA bufs, from a previous incarnation
 *  of a dongle firmware download/handshake.
 *
 *
 * Bus layer invocation sequence per dongle firmware instantiation:
 * - dhd_prot_preinit()  : Specifically for msgbuf_rings preinit
 * - dhd_prot_hme_reset(): Free up all prior HME regions
 * - dhd_prot_hme_init() : Invoked if dongle advertizes HME
 * - dhd_prot_init()     : (Re)Initialization of the DHD protocol layer
 *
 *   Each HME user is required to re-initialize itself and adopt the new
 *   HME regions assigned to it.
 *
 * +---------------------------------------------------------------------------+
 */

int
dhd_prot_hme_init(dhd_pub_t *dhd, uint32 users)
{
	uint32 user_id, bytes;

	dhd_hme_t * hme = &dhd->prot->hme;
	haddr64_t * hme_user_haddr64;

	ASSERT(users > 0);

	/* Free the legacy scratch host mem, if present */
	dhd_prot_host_mem_free(dhd);

	hme->users = users;

	/* Allocate an array of dhd_dma_buf_t */
	bytes = (uint32)(hme->users * sizeof(dhd_dma_buf_t));
	hme->user_dma_buf = (dhd_dma_buf_t*) MALLOCZ(dhd->osh, bytes);
	if (hme->user_dma_buf == (dhd_dma_buf_t*)NULL)
	{
		DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
			"HME DMA Buf Table users %u bytes %u allocation\n",
			__FUNCTION__, hme->users, bytes));
		goto hme_init_failure;
	}

	/* Allocate a dhd_dma_buf_t to hold the table of per user HME haddr64 */
	bytes = (uint32)(hme->users * sizeof(haddr64_t));
	if (dhd_dma_buf_alloc(dhd, &hme->user_haddr64_dma_buf, "hme_user_haddr64",
	                      bytes, DHD_MEM_ALIGN_BITS_MIN))
	{
		DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
			"HME Haddr64 Table users %u bytes %u align %u dma_buf allocation\n",
			__FUNCTION__, hme->users, bytes, DHD_MEM_ALIGNMENT_MIN));
		goto hme_init_failure;
	}
	DHD_PCIE_IPC(("\t Protocol: HME Haddr64 Table users %u" DHD_DMA_BUF_FMT "\n",
		hme->users, DHD_DMA_BUF_VAL(hme->user_haddr64_dma_buf)));

	hme_user_haddr64 = (haddr64_t*)hme->user_haddr64_dma_buf.va;

	/* Allocate HME regions for each user */
	for (user_id = 0U; user_id < hme->users; ++user_id)
	{
		int pages;
		dhd_dma_buf_t * user_dma_buf = hme->user_dma_buf + user_id;

		/* Request bus layer to alloc using PCIe IPC User HME attributes */
		pages = dhd_hme_buf_alloc(dhd->bus, user_dma_buf, user_id);

		if (pages == 0) continue;

		if (pages < 0) { // BCME_NOMEM
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"HME user %u dma_buf allocation\n", __FUNCTION__, user_id));
			ASSERT(pages >= 0);
			goto hme_init_failure;
		}

		DHD_PCIE_IPC(("\t Protocol: HME user %u" DHD_DMA_BUF_FMT "\n",
			user_id, DHD_DMA_BUF_VAL(*user_dma_buf)));

		/* HTOL copy Physical Address into HME address table */
		dhd_base_addr_htolpa(hme_user_haddr64 + user_id,
		                     hme->user_dma_buf[user_id].pa);

		hme->pages += pages;
		hme->bytes += PCIE_IPC_HME_BYTES(pages);
	}

	/* Successfully allocated all user's requested HME extensions */

	if (hme->bytes == 0U) { /* Oddly no request ... but yet dcap1 set? */
		goto hme_init_failure;
	}

	OSL_CACHE_FLUSH((void *)hme->user_haddr64_dma_buf.va,
	                hme->user_haddr64_dma_buf.len);

	return BCME_OK;

hme_init_failure:

	dhd_prot_hme_reset(dhd); /* including the HME user HMOSWP (HYBRIDFW) */
	return BCME_NOMEM;

}   /* dhd_prot_hme_init() */

/**
 * Free any/all HME dma_buf(s) that may have been allocated in a prior dongle
 * instantiation. As new dongle firmware can be uploaded dynamically, the HME
 * users and their specifications can change.
 */
int
dhd_prot_hme_reset(dhd_pub_t *dhd)
{
	uint32 user_id, size;
	dhd_hme_t * hme = &dhd->prot->hme;

	if (hme->user_dma_buf == (dhd_dma_buf_t*)NULL) return BCME_OK;

	/* Free all HME User regions */
	for (user_id = 0U; user_id < hme->users; ++user_id) {
		dhd_dma_buf_t * user_dma_buf = hme->user_dma_buf + user_id;
		dhd_dma_buf_free(dhd, user_dma_buf, "hme_user");
	}

	/* Free the DMA-able buffer carrying the table of per user's haddr64 */
	dhd_dma_buf_free(dhd, &hme->user_haddr64_dma_buf, "hme_user_haddr64");

	size = (uint32)(hme->users * sizeof(dhd_dma_buf_t));
	MFREE(dhd->osh, hme->user_dma_buf, size);
	hme->user_dma_buf = (dhd_dma_buf_t*)NULL;

	hme->users = 0U;
	hme->pages = 0U;
	hme->bytes = 0U;

	return BCME_OK;
}	/* dhd_prot_hme_reset() */

/**
 * +--------------------------------------------------------------------------+
 * CSI Monitor handling
 * +--------------------------------------------------------------------------+
 */

/**
 * Theory of operation:
 * ====================
 *
 * The DHD side of the Channel State Information Monitor (CSIMON) includes
 * reading or consuming the CSI records/structures from the circular ring in
 * the host memory and making them available to the user. DHD
 * CSIMON watchdog is invoked periodically to consume the record
 * and send over a netlink socket or write to a file.
 *
 * The circular ring of CSI records consists of a preamble in addition to the
 * records. The preamble mantains the read index, the write index, and pointer
 * to the start of the records besides the version and the ring item size.
 *
 * In case of a netlink socket, DHD multicasts each record over
 * to a multicast group. The user application needs to become a
 * member of that group to receive the CSI record.
 *
 * In case of a file output, DHD writes the CSI records in a
 * file /var/csimon sequentially as they are read/consumed from
 * the circular ring. The file has a size limit. Once the limit
 * is reached, the next records are not copied and they are
 * essentially dropped. So it is user's responsibility to copy
 * the file out of the AP and remove it. DHD will then create a
 * new file of the same name and start populating the next CSI
 * records.
 *
 * CSI record generation:
 * The dongle sends a null QoS frame to each client in the CSIMON client list
 * periodically. When a client responds with an ACK, the PHY layer computes
 * the CSI based on the PHY preamble. It is stored in SVMP memory. During
 * TxStatus processing, the dongle recognizes the Null frame's ACK and copies
 * the CSI report from the SVMP memory to the host memory in the HME (Host
 * Memory Extension) region in the circular ring of the CSI records. In addition
 * to the CSI report, it also copies additional information like RSSI, MAC
 * address, as a part of the overall CSI record.
 *
 * DHD side statistics:
 * num_records	: Total records read from the circular ring
 * drops	: DHD drops a record if multicasting over netlink
 * fails or the destination file is full
 *
 */

/** Following 3 functions assume csimon has been successfully initialized */
static inline uint32 // Fetch dongle posted WR index
__dhd_csimon_pull_wr(dhd_pub_t *dhd, dhd_csimon_t *csimon)
{
	OSL_CACHE_INV((void*)(csimon->wr_ptr), sizeof(uint32));

	return *(csimon->wr_ptr);
} // __dhd_csimon_pull_wr()

static inline uint32 // Fetch rd index in preamble shared with dongle
__dhd_csimon_pull_rd(dhd_pub_t *dhd, dhd_csimon_t *csimon)
{
	OSL_CACHE_INV((void*)(csimon->rd_ptr), sizeof(uint32));

	return *(csimon->rd_ptr);
} // __dhd_csimon_pull_wr()

static inline void // Commit rd pointer and push dongle via preamble
__dhd_csimon_push_rd(dhd_pub_t *dhd, dhd_csimon_t *csimon)
{
	*(csimon->rd_ptr) = (uint32)csimon->ring.read;
	OSL_CACHE_FLUSH((void*)(csimon->rd_ptr), sizeof(uint32));
} // __dhd_csimon_push_rd()

#ifdef CSIMON_FILE_BUILD
#include <linux/fs.h>

static int
dhd_csimon_file_write(dhd_csimon_t *csimon, void * csi_buf, ssize_t csi_buf_len)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	ssize_t         ret;
	mm_segment_t    old_fs;
	struct file     *file;
	loff_t          offset;
	int             flags       = O_RDWR | O_CREAT | O_LARGEFILE | O_SYNC;
	const char      *filename   = CSIMON_FILENAME;

	ASSERT(csimon != NULL);
	ASSERT(csi_buf != NULL);

	old_fs = get_fs(); /* current addr_limit: user|kernel space */
	set_fs(KERNEL_DS); /* change to "kernel data segment" address limit */

	file = filp_open(filename, flags, 0600);
	if (IS_ERR(file)) {
		DHD_ERROR(("%s filp_open(%s) failed\n", __FUNCTION__, filename));
		ret = PTR_ERR(file);
		csimon->fopen_fails++;
		goto exit2;
	}

	offset = default_llseek(file, 0, SEEK_END); /* seek to EOF */
	if (offset < 0) {
		ret = (ssize_t)offset;
		goto exit1;
	}

	if (offset >= CSIMON_FILESIZE) {    /* csimon file is bounded, drop */
		ret = BCME_NORESOURCE;
		goto exit1;
	}

	ret = kernel_write(file, csi_buf, csi_buf_len, &offset);
	if (ret != csi_buf_len) {
		DHD_ERROR(("%s kernel_write %s failed: %zd\n",
			__FUNCTION__, filename, ret));
		if (ret > 0)
			ret = -EIO;
		goto exit1;
	}

	ret = BCME_OK;

exit1:
	filp_close(file, NULL);

exit2:
	set_fs(old_fs); // restore saved address limit

	return ret;

#else   /* LINUX_VERSION_CODE < 4 */
	return BCME_OK;
#endif	/* LINUX_VERSION_CODE < 4 */

} // dhd_csimon_file_write()

#else /* ! CSIMON_FILE_BUILD */

static int /* send the CSI record over netlink socket to the user application */
dhd_csimon_netlink_send(dhd_csimon_t *csimon, void * csi_buf, ssize_t csi_buf_len)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	struct sk_buff *skb;
	struct nlmsghdr *nlmh;
	int ret;

	if (csimon->nl_sock == (struct sock *)NULL) {
		DHD_ERROR(("%s: nl mcast not done as nl_sock is NULL\n", __FUNCTION__));
		// ASSERT(0);
		return BCME_NORESOURCE;
	}

	/* Allocate an SKB and its data area */
	skb = alloc_skb(NLMSG_SPACE(csi_buf_len), GFP_KERNEL);
	if (skb == NULL) {
		DHD_ERROR(("Allocation failure!\n"));
		return BCME_NORESOURCE;
	}
	skb_put(skb, NLMSG_SPACE(csi_buf_len));

	/* Fill in the netlink msg header that is at skb->data */
	nlmh = (struct nlmsghdr *)skb->data;
	nlmh->nlmsg_len = NLMSG_SPACE(csi_buf_len);
	nlmh->nlmsg_pid = 0; /* kernel */
	nlmh->nlmsg_flags = 0;

	/* Copy the CSI record into the netlink SKB structure */
	/* XXX Can we avoid this copy inside kernel space? Apparently mmap netlink
	 * could do but not sure if we can use it in this case where we have the
	 * CSI records in HME. Also the mmap netlink support has been removed from
	 * the latest Linux kernels.
	 */
	memcpy(NLMSG_DATA(nlmh), csi_buf, csi_buf_len);
	{
		struct timespec64 ts;
		ktime_get_real_ts64(&ts);
		DHD_INFO(("%s: nl multicast at %lu.%lu for skb %p nlmh %p \n",
		          __FUNCTION__, (ulong)ts.tv_sec, ts.tv_nsec/1000, skb, nlmh));
	}

	/* Send the CSI record over netlink multicast */
	ret = netlink_broadcast(csimon->nl_sock, skb, 0, CSIMON_GRP_BIT, GFP_KERNEL);
	if (ret < 0) {
		DHD_ERROR(("%s: nl mcast failed %d sock %p skb %p\n", __FUNCTION__,
		           ret, csimon->nl_sock, skb));
		return ret;
	}
	else {
		DHD_INFO(("%s: nl mcast successful ret %d socket %p skb %p\n",
		           __FUNCTION__, ret, csimon->nl_sock, skb));
	}

	return BCME_OK;

#else   /* LINUX_VERSION_CODE < 4 */
	return BCME_OK;
#endif	/* LINUX_VERSION_CODE < 4 */
}

#endif /* ! CSIMON_FILE_BUILD */

static inline void
dhd_csimon_console_print(uint16 *csi_rec)
{
#ifdef CSIMON_PRINT_BUILD
	uint16 mem_len = 256 + 32; // 256 words = 512B for report and 64B for header
	uint num_col = 16;
	int i, j;

	DHD_ERROR(("CSI record for elem idx %d at %p:\n", elem_idx, elem));
	/* Dump the CSI record to the console */
	for (i = 0; i < (mem_len / num_col); i++) {
		for (j = 0; j < num_col; j++) {
			DHD_ERROR(("0x%04x\t", csi_rec[i * num_col + j]));
		}
		DHD_ERROR(("\n"));
	}
#endif /* CSIMON_PRINT_BUILD */
}

int
dhd_csimon_watchdog(dhd_pub_t *dhd)
{
	dhd_prot_t   * prot     = dhd->prot;
	dhd_csimon_t * csimon   = &prot->csimon;
	csimon_ring_elem_t * elem;
	int elem_idx;

	/* coverity[dead_error_line] */
	if ((dhd->tickcnt % CSIMON_POLL_10MSEC_TICKS) != 0) return BCME_OK;

	if ((dhd->csi_monitor == FALSE) || (csimon->ipc_hme == NULL))
		return BCME_UNSUPPORTED;

#ifdef CSIMON_FILE_BUILD
	if (csimon->fopen_fails >= CSIMON_FILE_OPEN_FAIL_LIMIT)
		return BCME_NODEVICE;
#endif

	/* Check whether dongle has completed the configuration of the preamble */
	if (csimon->ipc_hme->preamble.table_daddr32 == 0U) {
		OSL_CACHE_INV(&(csimon->ipc_hme->preamble), sizeof(csimon_preamble_t));
		if (csimon->ipc_hme->preamble.table_daddr32 == 0U) return BCME_OK;
	}

	/* Refresh the ring's write index */
	csimon->ring.write = (int)__dhd_csimon_pull_wr(dhd, csimon);

	/* Transfer the CSI records present in the host memory ring to a file */
	while ((elem_idx = bcm_ring_cons(&csimon->ring, CSIMON_RING_ITEMS_MAX))
	            != BCM_RING_EMPTY)
	{
		elem = CSIMON_TABLE_IDX2ELEM(csimon->table, elem_idx);
		OSL_CACHE_INV((void*)(elem), CSIMON_RING_ITEM_SIZE);

		/* print on console if enabled */
		dhd_csimon_console_print((uint16 *)(elem->data));

#ifdef CSIMON_FILE_BUILD
		if (dhd_csimon_file_write(csimon, elem->data, CSIMON_RING_ITEM_SIZE)) {
			csimon->drops++;
			DHD_ERROR(("%s: CSIMON file write %u\n", __FUNCTION__, csimon->drops));
		}
#else /* ! CSIMON_FILE_BUILD */
		/* transfer the CSI records over netlink socket */
		if (dhd_csimon_netlink_send(csimon, elem->data, CSIMON_RING_ITEM_SIZE)) {
			csimon->drops++;
			DHD_ERROR(("%s: CSIMON drops with nl send %u\n", __FUNCTION__,
			           csimon->drops));
		}
#endif /* ! CSIMON_FILE_BUILD */

		csimon->num_records++;
	}

	__dhd_csimon_push_rd(dhd, csimon);

	return BCME_OK;
}  // dhd_csimon_watchdog()

int
dhd_csimon_dump(dhd_pub_t *dhd)
{
	dhd_prot_t * prot = dhd->prot;
	dhd_csimon_t * csimon = &prot->csimon;

	if (dhd->csi_monitor == FALSE) {
		DHD_ERROR(("CSIMON not enabled\n"));
		return BCME_UNSUPPORTED;
	}
	if (csimon->ipc_hme == (csimon_ipc_hme_t*)NULL) {
		DHD_ERROR(("CSIMON not initialized\n"));
		return BCME_UNSUPPORTED;
	}

	OSL_CACHE_INV(&(csimon->ipc_hme->preamble), sizeof(csimon_preamble_t));
	DHD_ERROR(("CSIMON: HOST" CSIMON_VRP_FMT " DNGL" CSIMON_VRP_FMT "\n",
		CSIMON_VRP_VAL(CSIMON_VERSIONCODE),
		CSIMON_VRP_VAL(csimon->ipc_hme->preamble.version_code)));

#ifndef CSIMON_FILE_BUILD
	DHD_ERROR(("Netlink subsystem for CSIMON: %d\n",
	           dhd->unit == 0 ? NETLINK_CSIMON0 : (dhd->unit == 1 ?
	           NETLINK_CSIMON1 : NETLINK_CSIMON2)));
#endif

	DHD_ERROR(("\ttable %p == daddr32 %08x, elem sz %u == %u\n",
		csimon->table, csimon->ipc_hme->preamble.table_daddr32,
		CSIMON_RING_ITEM_SIZE, csimon->ipc_hme->preamble.elem_size));
	DHD_ERROR(("\t<wr,rd> ring<%02u,%02u> preamble<%02u,%02u> "
		"#record xfers %u drops %u\n",
		csimon->ring.write, csimon->ring.read,
		__dhd_csimon_pull_wr(dhd, csimon), __dhd_csimon_pull_rd(dhd, csimon),
		csimon->num_records, csimon->drops));

	return BCME_OK;
} // dhd_csimon_dump()

int
dhd_csimon_init(dhd_pub_t *dhd)
{
	dhd_prot_t   *prot   = dhd->prot;
	dhd_csimon_t *csimon = &prot->csimon;
	csimon_ipc_hme_t *ipc_hme;

	if (dhd->csi_monitor == FALSE) {
		return BCME_UNSUPPORTED;
	}

	memset(csimon, 0, sizeof(dhd_csimon_t));

	ASSERT(prot->hme.user_dma_buf != (dhd_dma_buf_t*)NULL);
	ipc_hme = (csimon_ipc_hme_t *)
	          ((prot->hme.user_dma_buf + PCIE_IPC_HME_USER_CSIMON)->va);

	if (ipc_hme == NULL) {
		DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
			"CSIMON DMA buffer not allocated.\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
	csimon->ipc_hme = ipc_hme;
	csimon->wr_ptr  = &ipc_hme->preamble.write_idx.u32;
	csimon->rd_ptr  = &ipc_hme->preamble.read_idx.u32;
	csimon->table   = &ipc_hme->table[0];
	bcm_ring_init(&csimon->ring);

#ifndef CSIMON_FILE_BUILD
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	{
		int nl_id = 0;

		/* There is a netlink socket per radio */
		if (dhd->unit == 0) {
			nl_id = NETLINK_CSIMON0;
		} else if (dhd->unit == 1) {
			nl_id = NETLINK_CSIMON1;
		} else { /* Third radio */
			nl_id = NETLINK_CSIMON2;
		}
		csimon->nl_sock = netlink_kernel_create(&init_net, nl_id, NULL);
		if (csimon->nl_sock == NULL) {
			DHD_ERROR(("Failed creating netlink: radio unit idx %d, nl_id %d\n",
			           dhd->unit, nl_id));
		} else {
			DHD_ERROR(("Created netlink: radio unit idx %d, nl_id %d\n",
			           dhd->unit, nl_id));
		}
	}
#endif	/* LINUX_VERSION_CODE < 4 */
#endif /* ! CSIMON_FILE_BUILD */
	DHD_ERROR(("CSIMON: HOST" CSIMON_VRP_FMT "\n",
		CSIMON_VRP_VAL(CSIMON_VERSIONCODE)));

	return BCME_OK;
}  // dhd_csimon_init()

int
dhd_csimon_fini(dhd_csimon_t *csimon)
{
	if (csimon->ipc_hme == (csimon_ipc_hme_t*)NULL)
		return BCME_UNSUPPORTED;

#ifndef CSIMON_FILE_BUILD
	netlink_kernel_release(csimon->nl_sock);
	csimon->nl_sock = (struct sock *)NULL;
#endif
	return BCME_OK;
} // dhd_csimon_fini()

/*
 * +---------------------------------------------------------------------------+
 * PCIE DMA-able buffer. Sets up a dhd_dma_buf_t object, which includes the
 * virtual and physical address, the buffer lenght and the DMA handler.
 * A secdma handler is also included in the dhd_dma_buf object.
 * +---------------------------------------------------------------------------+
 */

static INLINE void
dhd_base_addr_htolpa(haddr64_t *base_addr, dmaaddr_t pa)
{
	HADDR64_LO_SET_HTOL((*base_addr), PHYSADDRLO(pa));
	HADDR64_HI_SET_HTOL((*base_addr), PHYSADDRHI(pa));
}

/**
 * +---------------------------------------------------------------------------+
 * DHD DMA-able Buffer Management Abstract Data Type
 * +---------------------------------------------------------------------------+
 */
#define DHD_DMA_BUF_DO_AUDIT        ((bool)(TRUE))
#define DHD_DMA_BUF_NO_AUDIT        ((bool)(FALSE))
static void dhd_dma_buf_reset_audit(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf,
                const char *str, bool with_audit);
static void dhd_dma_buf_free_audit(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf,
                const char *str, bool with_audit);

/**
 * +---------------------------------------------------------------------------+
 * dhd_dma_buf_audit - DHD DMA Buffer Audits:
 *
 *  1. A DMA-able memory buffer is present.
 *  2. Minimum (8 Byte) alignment is observed.
 *  3. Memory does not wrap around a 0xFFFFFFFF boundary.
 *     Dongle may perform a 32 bit pointer arithmetic (i.e. no carryover)
 *  4. Alignment adjustment of virtual address given to user does not cause an
 *     overflow of the memory allocated by the OS.
 * +---------------------------------------------------------------------------+
 */
int
dhd_dma_buf_audit(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf, const char *str)
{
	/* dongle may use physical addresses in 32b arithmetic */
	uint32  pa_base, pa_end;

	/* Audit 1. A DMA-able memory buffer is present */
	ASSERT(dma_buf);
	ASSERT(dma_buf->va   != (void*)NULL);
	ASSERT(dma_buf->len  != 0U);

	/* Audit 2. Minimum (8 Byte) alignment is observed. */
	pa_base = PHYSADDRLO(dma_buf->pa);
	// ASSERT(pa_base != 0U);
	ASSERT(ISALIGNED(pa_base, DHD_MEM_ALIGNMENT_MIN)); /* 8 Byte alignment */

	/* Audit 3. Memory does not wrap around a 0xFFFFFFFF boundary. */
	pa_end = (pa_base + dma_buf->len); /* end of physical memory */
	if (pa_end < pa_base) { /* 32 bit arithmetic excludes carryover */
		DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
			"%s" DHD_DMA_BUF_OS_FMT " 32bit arithmetic wrap\n",
			__FUNCTION__, str, DHD_DMA_BUF_OS_VAL(*dma_buf)));
		ASSERT(0);
		return BCME_ERROR;
	}

	/* Audit 4. Buffer overflow check for dhd_dma_buf_alloc() memory */
	if (dma_buf->_mem) {
		uintptr va_base, mem_base;
		va_base  = (uintptr)(dma_buf->va);   /* adjusted base with alignment */
		mem_base = (uintptr)(dma_buf->_mem); /* OS allocated memory base */
		if ((va_base + dma_buf->len) > (mem_base + dma_buf->_alloced)) {
			DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
				"%s" DHD_DMA_BUF_OS_FMT " overflow\n",
				__FUNCTION__, str, DHD_DMA_BUF_OS_VAL(*dma_buf)));
			ASSERT(0);
			return BCME_ERROR;
		}
	}

	return BCME_OK;

}   /* dhd_dma_buf_audit() */

/**
 * +---------------------------------------------------------------------------+
 * dhd_dma_buf_reset - Reset a cache coherent DMA-able buffer.
 * - zero out the memory buffer
 * - invalidate the cache. No-op on systems with Coherency in HW.
 * +---------------------------------------------------------------------------+
 */
void
dhd_dma_buf_reset_audit(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf,
	const char *str, bool with_audit)
{
	if ((dma_buf == NULL) || (dma_buf->_mem == (uintptr)NULL)) {
		return;
	}

	if (with_audit == DHD_DMA_BUF_DO_AUDIT) {
		(void)dhd_dma_buf_audit(dhd, dma_buf, str);
	}

	/* Zero out the entire "OS allocated" memory and cache flush */
	memset((void*)dma_buf->_mem, 0, dma_buf->_alloced);
	OSL_CACHE_FLUSH((void *)dma_buf->_mem, dma_buf->_alloced);

}   /* dhd_dma_buf_reset_audit() */

void
dhd_dma_buf_reset(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf, const char *str)
{
	dhd_dma_buf_reset_audit(dhd, dma_buf, str, DHD_DMA_BUF_DO_AUDIT);
}   /* dhd_dma_buf_reset() */

/**
 * +---------------------------------------------------------------------------+
 * dhd_dma_buf_free - Free a DMA-able buffer that was previously allocated using
 * dhd_dma_buf_alloc().
 * - OS allocate memory is freed to the OS.
 * - dhd_dma_buf context is zeroed.
 * +---------------------------------------------------------------------------+
 */
void
dhd_dma_buf_free_audit(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf,
	const char *str, bool with_audit)
{
	osl_t *osh = dhd->osh;

	ASSERT(dma_buf);

	if (dma_buf->_mem == (uintptr)NULL) {
		memset(dma_buf, 0, sizeof(*dma_buf));
		return; /* Allow for free invocation, when alloc failed */
	}

	if (with_audit == DHD_DMA_BUF_DO_AUDIT) {
		(void)dhd_dma_buf_audit(dhd, dma_buf, str);
	}
	/* DEBUG: dhd_dma_buf_reset_audit(dhd, dma_buf, DHD_DMA_BUF_NO_AUDIT) */

	DMA_FREE_CONSISTENT(osh, /* Free "OS allocated" memory */
		(void*)dma_buf->_mem, dma_buf->_alloced, dma_buf->pa, dma_buf->dmah);

	memset(dma_buf, 0, sizeof(*dma_buf));

}   /* dhd_dma_buf_free_audit() */

void
dhd_dma_buf_free(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf, const char *str)
{
	dhd_dma_buf_free_audit(dhd, dma_buf, str, DHD_DMA_BUF_DO_AUDIT);
}   /* dhd_dma_buf_free() */

/**
 * +---------------------------------------------------------------------------+
 * dhd_dma_buf_alloc - Allocate an aligned coherent DMA-able buffer.
 *
 * Alignment: The underlying Kernel OS memory manager need only serve an 8 Byte
 *      aligned memory (GNU, sizeof unsigned long long: DHD_MEM_ALIGNMENT_MIN).
 *      When an alignment (specified in align_bits) of larger than the minimum
 *      alignment (1 << DHD_MEM_ALIGN_BITS_MIN) is requested, the size is
 *      readjusted (increased by the alignment number of bytes). The readjusted
 *      size is requested from the OS memory manager. An aligned physical memory
 *      is carved from the OS allocated memory.
 *      Actual memory allocated by the OS is saved in private dhd_dma_buf::_mem
 *      virtual address, and dhd_dma_buf::_alloced length fields.
 *      Aligned memory allocated to user is in dhd_dma_buf::<va, pa, len>
 * Padding: User specified memory length is increased by DHD_DMA_BUF_PAD_LENGTH,
 *      to ensure that a DMA-able buffer is fully contained within a cacheline.
 *      i.e. the adjacent memory that may be used by a different application
 *      does not share a cacheline with the DMA-able buffer. This requirement is
 *      imposed on platforms where Cache Coherency is via explicit Software
 *      invoked Cache-Ops, (i.e. SW coherency).
 *
 * Return: BCME_OK on success. non-zero negative BCME error code on failure.
 * +---------------------------------------------------------------------------+
 */
int /* BCME_OK or BCME error code */
dhd_dma_buf_alloc(dhd_pub_t *dhd, dhd_dma_buf_t *dma_buf, const char *str,
                  uint32 buf_len, uint32 align_bits)
{
	int ret = BCME_OK;
	uint32 padding, alignment; /* bytes to accommodate padding and alignment */

	ASSERT(dma_buf != (dhd_dma_buf_t*)NULL);
	ASSERT(dma_buf->va == (void*)NULL);
	ASSERT(dma_buf->len == 0U);

	alignment = (align_bits > DHD_MEM_ALIGN_BITS_MIN) ? (1 << align_bits) : 0U;
	padding = (buf_len % DHD_DMA_BUF_PAD_LENGTH) ? DHD_DMA_BUF_PAD_LENGTH : 0U;

	if (align_bits <= DHD_DMA_BUF_4KB_ALIGN_BITS) {
		/* Do allocation without alignment to avoid memory waste,
		 * expect linux return at 4KB alignment already
		 */
		/* dhd_dma_buf::_mem and dhd_dma_buf::_alloced used in dhd_dma_buf_free */
		dma_buf->_mem = (uintptr) DMA_ALLOC_CONSISTENT(dhd->osh,
				buf_len + padding, /* OS may not perform alignment */
				0, &dma_buf->_alloced, &dma_buf->pa, &dma_buf->dmah);
		if (dma_buf->_mem == (uintptr)NULL) {
			ret = BCME_NOMEM;
			goto dma_buf_alloc_failure;
		}

		/* Check alignment */
		if (ISALIGNED(PHYSADDRLO(dma_buf->pa), (1 << align_bits))) {
			dma_buf->len = buf_len; /* not including alignment or padded bytes */
			dma_buf->va  = (void*)dma_buf->_mem;
			goto dma_buf_audit;
		} else {
			/* Free it and do allocation with SW alignment */
			dhd_dma_buf_free_audit(dhd, dma_buf, str, DHD_DMA_BUF_NO_AUDIT);
		}
	}

	/* dhd_dma_buf::_mem and dhd_dma_buf::_alloced used in dhd_dma_buf_free */
	dma_buf->_mem = (uintptr) DMA_ALLOC_CONSISTENT(dhd->osh,
			alignment + buf_len + padding, /* OS may not perform alignment */
			align_bits, &dma_buf->_alloced, &dma_buf->pa, &dma_buf->dmah);
	if (dma_buf->_mem == (uintptr)NULL) {
		ret = BCME_NOMEM;
		goto dma_buf_alloc_failure;
	}

	dma_buf->len = buf_len; /* not including alignment or padded bytes */

	/* Perform alignment by rounding up to alignment boundary */
	if (alignment) {
		/* Compute bytes to skip from front to align physical memory */
		uint32 pa_align = ROUNDUP(PHYSADDRLO(dma_buf->pa), alignment);
		uint32 readjust = pa_align - PHYSADDRLO(dma_buf->pa);
		ASSERT(pa_align >= PHYSADDRLO(dma_buf->pa));
		dma_buf->va     = (void*)(dma_buf->_mem + readjust);
		PHYSADDRLOSET(dma_buf->pa, pa_align);
	} else {
		dma_buf->va     = (void*)dma_buf->_mem;
	}

dma_buf_audit:
	if ((ret = dhd_dma_buf_audit(dhd, dma_buf, str)) != BCME_OK) {
		dhd_dma_buf_free_audit(dhd, dma_buf, str, DHD_DMA_BUF_NO_AUDIT);
		goto dma_buf_alloc_failure;
	}

	/* Zero out and cache flush */
	dhd_dma_buf_reset_audit(dhd, dma_buf, str, DHD_DMA_BUF_NO_AUDIT);

	return ret;

dma_buf_alloc_failure:

#if defined(__GNUC__) // GNU GCC C Extension __builtin_return_address
	DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
		"%s dma_buf len %u caller %px\n", __FUNCTION__, str, buf_len,
		(void*)(uintptr)__builtin_return_address(0)));
#else
	DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
		"%s dma_buf len %u\n", __FUNCTION__, str, buf_len));
#endif /* ! __GNUC__ */

	return ret;

}   /* dhd_dma_buf_alloc() */

/* +------------------  End of PCIE DHD DMA BUF ADT ------------------------+ */

/*
 * +---------------------------------------------------------------------------+
 * PktId Map: Provides a native packet pointer to unique 32bit PktId mapping.
 * Main purpose is to save memory on the dongle, has other purposes as well.
 * The packet id map, also includes storage for some packet parameters that
 * may be saved. A native packet pointer along with the parameters may be saved
 * and a unique 32bit pkt id will be returned. Later, the saved packet pointer
 * and the metadata may be retrieved using the previously allocated packet id.
 * +---------------------------------------------------------------------------+
 */
#define DHD_PCIE_PKTID
#define MAX_PKTID_ITEMS     ((36 * 1024) -1) /* Maximum number of pktids supported */

/* On Router, the pktptr serves as a pktid. */
#if defined(BCM_ROUTER_DHD) && !defined(CONFIG_ARM_LPAE)
#undef DHD_PCIE_PKTID		/* Comment this undef, to reenable PKTIDMAP */
#endif /* BCM_ROUTER_DHD && !BCA_HNDROUTER */

/* XXX: PROP_TXSTATUS: WLFS defines a private pkttag layout.
 * Hence cannot store the dma parameters in the pkttag and the pktidmap locker
 * is required.
 */
#if defined(PROP_TXSTATUS) && !defined(DHD_PCIE_PKTID)
#error "PKTIDMAP must be supported with PROP_TXSTATUS/WLFC"
#endif

/* Enum for marking the buffer color based on usage */
typedef enum dhd_pkttype {
	PKTTYPE_DATA_TX = 0,
	PKTTYPE_DATA_RX,
	PKTTYPE_IOCTL_RX,
	PKTTYPE_EVENT_RX,
	/* dhd_prot_pkt_free no check, if pktid reserved and no space avail case */
	PKTTYPE_NO_CHECK
} dhd_pkttype_t;

#define DHD_PKTID_INVALID               (0U)
#if defined(BCM_DHD_RUNNER)
/* Make sure last 2bits are 00'b or 11'b so it won't clash with Runner PKTID */
#define	DHD_RUNNER_PKTID_SHIFT          (4)
#define DHD_RUNNER_PKTID_TO_REQID(pktid)  ((pktid) << DHD_RUNNER_PKTID_SHIFT)
#define DHD_RUNNER_REQID_TO_PKTID(reqid)  ((reqid) >> DHD_RUNNER_PKTID_SHIFT)
#define DHD_IOCTL_REQ_PKTID             (0xFFFE << DHD_RUNNER_PKTID_SHIFT)
#define DHD_FAKE_PKTID                  (0xFACE << DHD_RUNNER_PKTID_SHIFT)
#else  /* !BCM_DHD_RUNNER */
#define DHD_IOCTL_REQ_PKTID             (0xFFFE)
#define DHD_FAKE_PKTID                  (0xFACE)
#endif /* !BCM_DHD_RUNNER */
#define DHD_PKTID_FREE_LOCKER           (FALSE)
#define DHD_PKTID_RSV_LOCKER            (TRUE)

typedef void * dhd_pktid_map_handle_t; /* opaque handle to a pktid map */

/* Construct a packet id mapping table, returning an opaque map handle */
static dhd_pktid_map_handle_t *dhd_pktid_map_init(dhd_pub_t *dhd, uint32 num_items, uint32 index);

/* Destroy a packet id mapping table, freeing all packets active in the table */
static void dhd_pktid_map_fini(dhd_pub_t *dhd, dhd_pktid_map_handle_t *map);

#define PKTID_MAP_HANDLE	(0)
#define PKTID_MAP_HANDLE_IOCTL	(1)

#define DHD_NATIVE_TO_PKTID_INIT(dhd, items, index) dhd_pktid_map_init((dhd), (items), (index))
#define DHD_NATIVE_TO_PKTID_FINI(dhd, map)   dhd_pktid_map_fini((dhd), (map))

#if defined(DHD_PCIE_PKTID)

/* Determine number of pktids that are available */
static INLINE uint32 dhd_pktid_map_avail_cnt(dhd_pktid_map_handle_t *handle);

/* Allocate a unique pktid against which a pkt and some metadata is saved */
static INLINE uint32 dhd_pktid_map_reserve(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle,
	void *pkt);
static INLINE void dhd_pktid_map_save(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle,
	void *pkt, uint32 nkey, dmaaddr_t pa, uint32 len, uint8 dma,
	void *dmah, void *secdma, dhd_pkttype_t pkttype);
static uint32 dhd_pktid_map_alloc(dhd_pub_t *dhd, dhd_pktid_map_handle_t *map,
	void *pkt, dmaaddr_t pa, uint32 len, uint8 dma,
	void *dmah, void *secdma, dhd_pkttype_t pkttype);

/* Return an allocated pktid, retrieving previously saved pkt and metadata */
static void *dhd_pktid_map_free(dhd_pub_t *dhd, dhd_pktid_map_handle_t *map,
	uint32 id, dmaaddr_t *pa, uint32 *len, void **dmah,
	void **secdma, dhd_pkttype_t pkttype, bool rsv_locker);

/*
 * DHD_PKTID_AUDIT_ENABLED: Audit of PktIds in DHD for duplicate alloc and frees
 *
 * DHD_PKTID_AUDIT_MAP: Audit the LIFO or FIFO PktIdMap allocator
 * DHD_PKTID_AUDIT_RING: Audit the pktid during producer/consumer ring operation
 *
 * CAUTION: When DHD_PKTID_AUDIT_ENABLED is defined,
 *    either DHD_PKTID_AUDIT_MAP or DHD_PKTID_AUDIT_RING may be selected.
 */
#ifndef DHD_PKTID_AUDIT_ENABLED
#define DHD_PKTID_AUDIT_ENABLED 1
#endif /* DHD_PKTID_AUDIT_ENABLED */

#if defined(BCM_ROUTER_DHD) || defined(STBLINUX)
#undef DHD_PKTID_AUDIT_ENABLED
/* NO PACKET AUDIT FOR ROUTERS */
#endif /* BCM_ROUTER_DHD */

#if defined(DHD_PKTID_AUDIT_ENABLED)
#define USE_DHD_PKTID_AUDIT_LOCK 1
/* Audit the pktidmap allocator */
/* #define DHD_PKTID_AUDIT_MAP */

/* Audit the pktid during production/consumption of workitems */
#define DHD_PKTID_AUDIT_RING

#if defined(DHD_PKTID_AUDIT_MAP) && defined(DHD_PKTID_AUDIT_RING)
#error "May only enabled audit of MAP or RING, at a time."
#endif /* DHD_PKTID_AUDIT_MAP && DHD_PKTID_AUDIT_RING */

#define DHD_DUPLICATE_ALLOC     1
#define DHD_DUPLICATE_FREE      2
#define DHD_TEST_IS_ALLOC       3
#define DHD_TEST_IS_FREE        4

#ifdef USE_DHD_PKTID_AUDIT_LOCK
#define DHD_PKTID_AUDIT_LOCK_INIT(osh)          dhd_os_spin_lock_init(osh)
#define DHD_PKTID_AUDIT_LOCK_DEINIT(osh, lock)  dhd_os_spin_lock_deinit(osh, lock)
#define DHD_PKTID_AUDIT_LOCK(lock)              dhd_os_spin_lock(lock)
#define DHD_PKTID_AUDIT_UNLOCK(lock, flags)     dhd_os_spin_unlock(lock, flags)
#else
#define DHD_PKTID_AUDIT_LOCK_INIT(osh)          (void *)(1)
#define DHD_PKTID_AUDIT_LOCK_DEINIT(osh, lock)  do { /* noop */ } while (0)
#define DHD_PKTID_AUDIT_LOCK(lock)              0
#define DHD_PKTID_AUDIT_UNLOCK(lock, flags)     do { /* noop */ } while (0)
#endif /* !USE_DHD_PKTID_AUDIT_LOCK */

#endif /* DHD_PKTID_AUDIT_ENABLED */

/* #define USE_DHD_PKTID_LOCK   1 */

#ifdef USE_DHD_PKTID_LOCK
#define DHD_PKTID_LOCK_INIT(osh)                dhd_os_spin_lock_init(osh)
#define DHD_PKTID_LOCK_DEINIT(osh, lock)        dhd_os_spin_lock_deinit(osh, lock)
#define DHD_PKTID_LOCK(lock)                    dhd_os_spin_lock(lock)
#define DHD_PKTID_UNLOCK(lock, flags)           dhd_os_spin_unlock(lock, flags)
#else
#define DHD_PKTID_LOCK_INIT(osh)                (void *)(1)
#define DHD_PKTID_LOCK_DEINIT(osh, lock)	\
	do { \
		BCM_REFERENCE(osh); \
		BCM_REFERENCE(lock); \
	} while (0)
#define DHD_PKTID_LOCK(lock)                    0
#define DHD_PKTID_UNLOCK(lock, flags)           \
	do { \
		BCM_REFERENCE(lock); \
		BCM_REFERENCE(flags); \
	} while (0)
#endif /* !USE_DHD_PKTID_LOCK */

/* Packet metadata saved in packet id mapper */

/* The Locker can be 3 states
 * LOCKER_IS_FREE - Locker is free and can be allocated
 * LOCKER_IS_BUSY - Locker is assigned and is being used, values in the
 *                  locker (buffer address, len, phy addr etc) are populated
 *		    with valid values
 * LOCKER_IS_RSVD - The locker is reserved for future use, but the values
 *                  in the locker are not valid. Especially pkt should be
 *                  NULL in this state. When the user wants to re-use the
 *                  locker dhd_pktid_map_free can be called with a flag
 *                  to reserve the pktid for future use, which will clear
 *                  the contents of the locker. When the user calls
 *                  dhd_pktid_map_save the locker would move to LOCKER_IS_BUSY
 */
typedef enum dhd_locker_state {
	LOCKER_IS_FREE,
	LOCKER_IS_BUSY,
	LOCKER_IS_RSVD
} dhd_locker_state_t;

typedef struct dhd_pktid_item {
	uint8	      state;	/* tag a locker to be free, busy or reserved */
	uint8         dir;      /* dma map direction (Tx=flush or Rx=invalidate) */
	uint8	      pkttype; /* pktlists are maintained based on pkttype */
	uint16        len;      /* length of mapped packet's buffer */
	void          *pkt;     /* opaque native pointer to a packet */
	dmaaddr_t     pa;       /* physical address of mapped packet's buffer */
	void          *dmah;    /* handle to OS specific DMA map */
	void          *secdma;
} dhd_pktid_item_t;

typedef struct dhd_pktid_map {
	uint32      items;    /* total items in map */
	uint32      avail;    /* total available items */
	int         failures; /* lockers unavailable count */
	/* Spinlock to protect dhd_pktid_map in process/tasklet context */
	void        *pktid_lock; /* Used when USE_DHD_PKTID_LOCK is defined */

#if defined(DHD_PKTID_AUDIT_ENABLED)
	void	    *pktid_audit_lock;
	struct bcm_mwbmap *pktid_audit; /* multi word bitmap based audit */
#endif /* DHD_PKTID_AUDIT_ENABLED */

	uint32      keys[MAX_PKTID_ITEMS + 1]; /* stack of unique pkt ids */
	void*	locker_blocks[LOCKER_BLOCKS]; /* metadata storage */
} dhd_pktid_map_t;

/*
 * PktId (Locker) #0 is never allocated and is considered invalid.
 *
 * On request for a pktid, a value DHD_PKTID_INVALID must be treated as a
 * depleted pktid pool and must not be used by the caller.
 *
 * Likewise, a caller must never free a pktid of value DHD_PKTID_INVALID.
 */

#define DHD_NATIVE_TO_PKTID_FINI_IOCTL(dhd, map)  dhd_pktid_map_fini_ioctl((dhd), (map))

#if defined(BCM_DHD_RUNNER)
/* Convert a packet to a pktid, and save pkt pointer in busy locker */
#define DHD_NATIVE_TO_PKTID_RSV(dhd, map, pkt)    \
	DHD_RUNNER_PKTID_TO_REQID(dhd_pktid_map_reserve((dhd), (map), (pkt)))

/* Reuse a previously reserved locker to save packet params */
#define DHD_NATIVE_TO_PKTID_SAVE(dhd, map, pkt, nkey, pa, len, dir, dmah, secdma, pkttype) \
	dhd_pktid_map_save((dhd), (map), (void *)(pkt), DHD_RUNNER_REQID_TO_PKTID(nkey),   \
	                   (pa), (uint32)(len), (uint8)(dir), (void *)(dmah),              \
	                   (void *)(secdma), (dhd_pkttype_t)(pkttype))

/* Convert a packet to a pktid, and save packet params in locker */
#define DHD_NATIVE_TO_PKTID(dhd, map, pkt, pa, len, dir, dmah, secdma, pkttype)            \
	DHD_RUNNER_PKTID_TO_REQID(dhd_pktid_map_alloc((dhd), (map), (void *)(pkt), (pa),   \
	                    (uint32)(len), (uint8)(dir), (void *)(dmah), (void *)(secdma), \
			    (dhd_pkttype_t)(pkttype)))

/* Convert pktid to a packet, and free the locker */
#define DHD_PKTID_TO_NATIVE(dhd, map, pktid, pa, len, dmah, secdma, pkttype)               \
	dhd_pktid_map_free((dhd), (map), (uint32)(DHD_RUNNER_REQID_TO_PKTID(pktid)),       \
	                   (dmaaddr_t *)&(pa), (uint32 *)&(len), (void **)&(dmah),         \
	                   (void **) &secdma, (dhd_pkttype_t)(pkttype), DHD_PKTID_FREE_LOCKER)

/* Convert the pktid to a packet, empty locker, but keep it reserved */
#define DHD_PKTID_TO_NATIVE_RSV(dhd, map, pktid, pa, len, dmah, secdma, pkttype)           \
	dhd_pktid_map_free((dhd), (map), (uint32)(DHD_RUNNER_REQID_TO_PKTID(pktid)),       \
	                   (dmaaddr_t *)&(pa), (uint32 *)&(len), (void **)&(dmah),         \
	                   (void **) &secdma, (dhd_pkttype_t)(pkttype), DHD_PKTID_RSV_LOCKER)

#define DHD_PKTID_AVAIL(map)                 dhd_pktid_map_avail_cnt(map)
#else  /* !BCM_DHD_RUNNER */
/* Convert a packet to a pktid, and save pkt pointer in busy locker */
#define DHD_NATIVE_TO_PKTID_RSV(dhd, map, pkt)    dhd_pktid_map_reserve((dhd), (map), (pkt))

/* Reuse a previously reserved locker to save packet params */
#define DHD_NATIVE_TO_PKTID_SAVE(dhd, map, pkt, nkey, pa, len, dir, dmah, secdma, pkttype) \
	dhd_pktid_map_save((dhd), (map), (void *)(pkt), (nkey), (pa), (uint32)(len), \
	                   (uint8)(dir), (void *)(dmah), (void *)(secdma), \
			   (dhd_pkttype_t)(pkttype))

/* Convert a packet to a pktid, and save packet params in locker */
#define DHD_NATIVE_TO_PKTID(dhd, map, pkt, pa, len, dir, dmah, secdma, pkttype) \
	dhd_pktid_map_alloc((dhd), (map), (void *)(pkt), (pa), (uint32)(len), \
	                    (uint8)(dir), (void *)(dmah), (void *)(secdma), \
			    (dhd_pkttype_t)(pkttype))

/* Convert pktid to a packet, and free the locker */
#define DHD_PKTID_TO_NATIVE(dhd, map, pktid, pa, len, dmah, secdma, pkttype) \
	dhd_pktid_map_free((dhd), (map), (uint32)(pktid), \
	(dmaaddr_t *)&(pa), (uint32 *)&(len), (void **)&(dmah), \
	(void **) &secdma, (dhd_pkttype_t)(pkttype), DHD_PKTID_FREE_LOCKER)

/* Convert the pktid to a packet, empty locker, but keep it reserved */
#define DHD_PKTID_TO_NATIVE_RSV(dhd, map, pktid, pa, len, dmah, secdma, pkttype) \
	dhd_pktid_map_free((dhd), (map), (uint32)(pktid), \
	(dmaaddr_t *)&(pa), (uint32 *)&(len), (void **)&(dmah), \
	(void **) &secdma, (dhd_pkttype_t)(pkttype), DHD_PKTID_RSV_LOCKER)

#define DHD_PKTID_AVAIL(map)                 dhd_pktid_map_avail_cnt(map)
#endif /* !BCM_DHD_RUNNER */

#if defined(DHD_PKTID_AUDIT_ENABLED)

static int dhd_pktid_audit(dhd_pub_t *dhd, dhd_pktid_map_t *pktid_map, uint32 pktid,
	const int test_for, const char *errmsg);

/* Call back into OS layer to take the dongle dump and panic */
#ifdef DHD_DEBUG_PAGEALLOC
extern void dhd_pktid_audit_fail_cb(dhd_pub_t *dhdp);
#endif /* DHD_DEBUG_PAGEALLOC */

/**
* dhd_pktid_audit - Use the mwbmap to audit validity of a pktid.
*/
static int
dhd_pktid_audit(dhd_pub_t *dhd, dhd_pktid_map_t *pktid_map, uint32 pktid,
	const int test_for, const char *errmsg)
{
#define DHD_PKT_AUDIT_STR "ERROR: %16s Host PktId Audit: "

	const uint32 max_pktid_items = (MAX_PKTID_ITEMS);
	struct bcm_mwbmap *handle;
	uint32	flags;
	bool ignore_audit;

	if (pktid_map == (dhd_pktid_map_t *)NULL) {
		DHD_ERROR((DHD_PKT_AUDIT_STR "Pkt id map NULL\n", errmsg));
		return BCME_OK;
	}

	flags = DHD_PKTID_AUDIT_LOCK(pktid_map->pktid_audit_lock);

	handle = pktid_map->pktid_audit;
	if (handle == (struct bcm_mwbmap *)NULL) {
		DHD_ERROR((DHD_PKT_AUDIT_STR "Handle NULL\n", errmsg));
		DHD_PKTID_AUDIT_UNLOCK(pktid_map->pktid_audit_lock, flags);
		return BCME_OK;
	}

	/* Exclude special pktids from audit */
	ignore_audit = (pktid == DHD_IOCTL_REQ_PKTID) | (pktid == DHD_FAKE_PKTID);
	if (ignore_audit) {
		DHD_PKTID_AUDIT_UNLOCK(pktid_map->pktid_audit_lock, flags);
		return BCME_OK;
	}

	if ((pktid == DHD_PKTID_INVALID) || (pktid > max_pktid_items)) {
		DHD_ERROR((DHD_PKT_AUDIT_STR "PktId<%d> invalid\n", errmsg, pktid));
		/* lock is released in "error" */
		goto error;
	}

	/* Perform audit */
	switch (test_for) {
		case DHD_DUPLICATE_ALLOC:
			if (!bcm_mwbmap_isfree(handle, pktid)) {
				DHD_ERROR((DHD_PKT_AUDIT_STR "PktId<%d> alloc duplicate\n",
				           errmsg, pktid));
				goto error;
			}
			bcm_mwbmap_force(handle, pktid);
			break;

		case DHD_DUPLICATE_FREE:
			if (bcm_mwbmap_isfree(handle, pktid)) {
				DHD_ERROR((DHD_PKT_AUDIT_STR "PktId<%d> free duplicate\n",
				           errmsg, pktid));
				goto error;
			}
			bcm_mwbmap_free(handle, pktid);
			break;

		case DHD_TEST_IS_ALLOC:
			if (bcm_mwbmap_isfree(handle, pktid)) {
				DHD_ERROR((DHD_PKT_AUDIT_STR "PktId<%d> is not allocated\n",
				           errmsg, pktid));
				goto error;
			}
			break;

		case DHD_TEST_IS_FREE:
			if (!bcm_mwbmap_isfree(handle, pktid)) {
				DHD_ERROR((DHD_PKT_AUDIT_STR "PktId<%d> is not free",
				           errmsg, pktid));
				goto error;
			}
			break;

		default:
			goto error;
	}

	DHD_PKTID_AUDIT_UNLOCK(pktid_map->pktid_audit_lock, flags);
	return BCME_OK;

error:

	DHD_PKTID_AUDIT_UNLOCK(pktid_map->pktid_audit_lock, flags);
	/* May insert any trap mechanism here ! */
#ifdef DHD_DEBUG_PAGEALLOC
	dhd_pktid_audit_fail_cb(dhd);
#else
	ASSERT(0);
#endif /* DHD_DEBUG_PAGEALLOC */

	return BCME_ERROR;
}

#define DHD_PKTID_AUDIT(dhdp, map, pktid, test_for) \
	dhd_pktid_audit((dhdp), (dhd_pktid_map_t *)(map), (pktid), (test_for), __FUNCTION__)

#endif /* DHD_PKTID_AUDIT_ENABLED */

/* +------------------  End of PCIE DHD PKTID AUDIT ------------------------+ */

/**
 * +---------------------------------------------------------------------------+
 * Packet to Packet Id mapper using a <numbered_key, locker> paradigm.
 *
 * dhd_pktid_map manages a set of unique Packet Ids range[1..MAX_PKTID_ITEMS].
 *
 * dhd_pktid_map_alloc() may be used to save some packet metadata, and a unique
 * packet id is returned. This unique packet id may be used to retrieve the
 * previously saved packet metadata, using dhd_pktid_map_free(). On invocation
 * of dhd_pktid_map_free(), the unique packet id is essentially freed. A
 * subsequent call to dhd_pktid_map_alloc() may reuse this packet id.
 *
 * Implementation Note:
 * Convert this into a <key,locker> abstraction and place into bcmutils !
 * Locker abstraction should treat contents as opaque storage, and a
 * callback should be registered to handle busy lockers on destructor.
 *
 * +---------------------------------------------------------------------------+
 */

/** Allocate and initialize a mapper of num_items <numbered_key, locker> */

static dhd_pktid_map_handle_t *
dhd_pktid_map_init(dhd_pub_t *dhd, uint32 num_items, uint32 index)
{
	void *osh;
	uint32 nkey;
	int i;

	dhd_pktid_map_t *map;
	dhd_pktid_item_t *locker;

	uint32 dhd_pktid_map_sz;
	uint32 map_items;
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_PKTIDMAP)
	uint32 section;
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_PKTIDMAP */
	osh = dhd->osh;

	ASSERT((num_items >= 1) && (num_items <= MAX_PKTID_ITEMS));
	dhd_pktid_map_sz = sizeof(dhd_pktid_map_t);

#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_PKTIDMAP)
	if (index == PKTID_MAP_HANDLE) {
		section = DHD_PREALLOC_PKTID_MAP;
	} else {
		section = DHD_PREALLOC_PKTID_MAP_IOCTL;
	}

	map = (dhd_pktid_map_t *)DHD_OS_PREALLOC(dhd, section, dhd_pktid_map_sz);
#else
	map = (dhd_pktid_map_t *)MALLOC(osh, dhd_pktid_map_sz);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_PKTIDMAP */

	if (map == NULL) {
		DHD_ERROR(("%s:%d: MALLOC failed for size %d\n",
			__FUNCTION__, __LINE__, dhd_pktid_map_sz));
		goto error;
	}

	bzero(map, dhd_pktid_map_sz);

	for (i = 0; i < LOCKER_BLOCKS; i++)
	{
		map->locker_blocks[i] = MALLOC(osh, LOCKER_BLOCK_SZ);
		if (map->locker_blocks[i] == NULL)
		{
			DHD_ERROR(("%s:%d: MALLOC failed for lockerblock %d,lockerblock size %zu\n",
				__FUNCTION__, __LINE__, i, LOCKER_BLOCK_SZ));
			goto error;
		}
	}

	/* Initialize the lock that protects this structure */
	map->pktid_lock = DHD_PKTID_LOCK_INIT(osh);
	if (map->pktid_lock == NULL) {
		DHD_ERROR(("%s:%d: Lock init failed \r\n", __FUNCTION__, __LINE__));
		goto error;
	}

	map->items = num_items;
	map->avail = num_items;

	map_items = map->items;

#if defined(DHD_PKTID_AUDIT_ENABLED)
	/* Incarnate a hierarchical multiword bitmap for auditing pktid allocator */
	map->pktid_audit = bcm_mwbmap_init(osh, map_items + 1);
	if (map->pktid_audit == (struct bcm_mwbmap *)NULL) {
		DHD_ERROR(("%s:%d: pktid_audit init failed\r\n", __FUNCTION__, __LINE__));
		goto error;
	} else {
		DHD_ERROR(("%s:%d: pktid_audit init succeeded %d\n",
			__FUNCTION__, __LINE__, map_items + 1));
	}

	map->pktid_audit_lock = DHD_PKTID_AUDIT_LOCK_INIT(osh);

#endif /* DHD_PKTID_AUDIT_ENABLED */

	for (nkey = 1; nkey <= map_items; nkey++) { /* locker #0 is reserved */
		map->keys[nkey] = nkey; /* populate with unique keys */
		locker = GET_LOCKER(map, nkey);
		locker->state = LOCKER_IS_FREE;
		locker->pkt   = NULL; /* bzero: redundant */
		locker->len   = 0;
	}

	/* Reserve pktid #0, i.e. DHD_PKTID_INVALID to be busy */
	locker = GET_LOCKER(map, DHD_PKTID_INVALID);
	locker->state = LOCKER_IS_BUSY;
	locker->pkt   = NULL; /* bzero: redundant */
	locker->len   = 0;

#if defined(DHD_PKTID_AUDIT_ENABLED)
	/* do not use dhd_pktid_audit() here, use bcm_mwbmap_force directly */
	bcm_mwbmap_force(map->pktid_audit, DHD_PKTID_INVALID);
#endif /* DHD_PKTID_AUDIT_ENABLED */

	return (dhd_pktid_map_handle_t *)map; /* opaque handle */

error:

	if (map) {

#if defined(DHD_PKTID_AUDIT_ENABLED)
		if (map->pktid_audit != (struct bcm_mwbmap *)NULL) {
			bcm_mwbmap_fini(osh, map->pktid_audit); /* Destruct pktid_audit */
			map->pktid_audit = (struct bcm_mwbmap *)NULL;
			if (map->pktid_audit_lock)
				DHD_PKTID_AUDIT_LOCK_DEINIT(osh, map->pktid_audit_lock);
		}
#endif /* DHD_PKTID_AUDIT_ENABLED */

		if (map->pktid_lock)
			DHD_PKTID_LOCK_DEINIT(osh, map->pktid_lock);

		for (i = 0; i < LOCKER_BLOCKS; i++)
		{
			if (map->locker_blocks[i] != NULL) {
				MFREE(osh, map->locker_blocks[i], LOCKER_BLOCK_SZ);
			}
		}

		MFREE(osh, map, dhd_pktid_map_sz);
	}

	return (dhd_pktid_map_handle_t *)NULL;
}

/**
 * Retrieve all allocated keys and free all <numbered_key, locker>.
 * Freeing implies: unmapping the buffers and freeing the native packet
 * This could have been a callback registered with the pktid mapper.
 */

static void
dhd_pktid_map_fini(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle)
{
	void *osh;
	uint32 nkey;
	dhd_pktid_map_t *map;
	uint32 dhd_pktid_map_sz;
	dhd_pktid_item_t *locker;
	uint32 map_items;
	uint32 flags;
	int i;

	if (handle == NULL) {
		return;
	}

	map = (dhd_pktid_map_t *)handle;
	flags =  DHD_PKTID_LOCK(map->pktid_lock);
	osh = dhd->osh;

	dhd_pktid_map_sz = sizeof(dhd_pktid_map_t);

	nkey = 1; /* skip reserved KEY #0, and start from 1 */
	locker = GET_LOCKER(map, nkey);

	map_items = map->items;

	for (; nkey <= map_items; nkey++, locker = GET_LOCKER(map, nkey)) {

		if (locker->state == LOCKER_IS_BUSY) { /* numbered key still in use */

			locker->state = LOCKER_IS_FREE; /* force open the locker */

#if defined(DHD_PKTID_AUDIT_ENABLED)
			DHD_PKTID_AUDIT(dhd, map, nkey, DHD_DUPLICATE_FREE); /* duplicate frees */
#endif /* DHD_PKTID_AUDIT_ENABLED */

			{   /* This could be a callback registered with dhd_pktid_map */
				DMA_UNMAP(osh, locker->pa, locker->len,
				    locker->dir, 0, DHD_DMAH_NULL);
				PKTFREE(osh, (ulong*)locker->pkt, TRUE);
			}
		}
#if defined(DHD_PKTID_AUDIT_ENABLED)
		else {
			DHD_PKTID_AUDIT(dhd, map, nkey, DHD_TEST_IS_FREE);
		}
#endif /* DHD_PKTID_AUDIT_ENABLED */

		locker->pkt = NULL; /* clear saved pkt */
		locker->len = 0;
	}

#if defined(DHD_PKTID_AUDIT_ENABLED)
	if (map->pktid_audit != (struct bcm_mwbmap *)NULL) {
		bcm_mwbmap_fini(osh, map->pktid_audit); /* Destruct pktid_audit */
		map->pktid_audit = (struct bcm_mwbmap *)NULL;
		if (map->pktid_audit_lock) {
			DHD_PKTID_AUDIT_LOCK_DEINIT(osh, map->pktid_audit_lock);
		}
	}
#endif /* DHD_PKTID_AUDIT_ENABLED */

	for (i = 0; i < LOCKER_BLOCKS; i++)
	{
		if (map->locker_blocks[i] != NULL) {
			MFREE(osh, map->locker_blocks[i], LOCKER_BLOCK_SZ);
		}
	}

	DHD_PKTID_UNLOCK(map->pktid_lock, flags);
	DHD_PKTID_LOCK_DEINIT(osh, map->pktid_lock);

#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_PKTIDMAP)
	DHD_OS_PREFREE(dhd, handle, dhd_pktid_map_sz);
#else
	MFREE(osh, handle, dhd_pktid_map_sz);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_PKTIDMAP */
}

#ifdef IOCTLRESP_USE_CONSTMEM
/** Called in detach scenario. Releasing IOCTL buffers. */
static void
dhd_pktid_map_fini_ioctl(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle)
{
	uint32 nkey;
	dhd_pktid_map_t *map;
	uint32 dhd_pktid_map_sz;
	dhd_pktid_item_t *locker;
	uint32 map_items;
	uint32 flags;
	osl_t *osh = dhd->osh;

	if (handle == NULL) {
		return;
	}

	map = (dhd_pktid_map_t *)handle;
	flags = DHD_PKTID_LOCK(map->pktid_lock);

	dhd_pktid_map_sz = (sizeof(dhd_pktid_map_t));

	nkey = 1; /* skip reserved KEY #0, and start from 1 */
	locker = &map->lockers[nkey];

	map_items = map->items;

	for (; nkey <= map_items; nkey++, locker = GET_LOCKER(map, nkey)) {

		if (locker->state == LOCKER_IS_BUSY) { /* numbered key still in use */

			locker->state = LOCKER_IS_FREE; /* force open the locker */

#if defined(DHD_PKTID_AUDIT_ENABLED)
			DHD_PKTID_AUDIT(dhd, map, nkey, DHD_DUPLICATE_FREE); /* duplicate frees */
#endif /* DHD_PKTID_AUDIT_ENABLED */

			{
				dhd_dma_buf_t ioctl_resp_dma_buf;
				ioctl_resp_dma_buf.va = locker->pkt;
				ioctl_resp_dma_buf.len = locker->len;
				ioctl_resp_dma_buf.pa = locker->pa;
				ioctl_resp_dma_buf.dmah = locker->dmah;
				ioctl_resp_dma_buf.secdma = locker->secdma;

				/* This could be a callback registered with dhd_pktid_map */
				DHD_PKTID_UNLOCK(map->pktid_lock, flags);
				free_ioctl_return_buffer(dhd, &ioctl_resp_dma_buf);
				flags = DHD_PKTID_LOCK(map->pktid_lock);
			}
		}
#if defined(DHD_PKTID_AUDIT_ENABLED)
		else {
			DHD_PKTID_AUDIT(dhd, map, nkey, DHD_TEST_IS_FREE);
		}
#endif /* DHD_PKTID_AUDIT_ENABLED */

		locker->pkt = NULL; /* clear saved pkt */
		locker->len = 0;
	}

#if defined(DHD_PKTID_AUDIT_ENABLED)
	if (map->pktid_audit != (struct bcm_mwbmap *)NULL) {
		bcm_mwbmap_fini(osh, map->pktid_audit); /* Destruct pktid_audit */
		map->pktid_audit = (struct bcm_mwbmap *)NULL;
		if (map->pktid_audit_lock) {
			DHD_PKTID_AUDIT_LOCK_DEINIT(osh, map->pktid_audit_lock);
		}
	}
#endif /* DHD_PKTID_AUDIT_ENABLED */

	DHD_PKTID_UNLOCK(map->pktid_lock, flags);
	DHD_PKTID_LOCK_DEINIT(osh, map->pktid_lock);

#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_PKTIDMAP)
	DHD_OS_PREFREE(dhd, handle, dhd_pktid_map_sz);
#else
	MFREE(osh, handle, dhd_pktid_map_sz);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_PKTIDMAP */
}
#endif /* IOCTLRESP_USE_CONSTMEM */

/** Get the pktid free count */
static INLINE uint32 BCMFASTPATH
dhd_pktid_map_avail_cnt(dhd_pktid_map_handle_t *handle)
{
	dhd_pktid_map_t *map;
	uint32	flags;
	uint32	avail;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;

	flags = DHD_PKTID_LOCK(map->pktid_lock);
	avail = map->avail;
	DHD_PKTID_UNLOCK(map->pktid_lock, flags);

	return avail;
}

/**
 * Allocate locker, save pkt contents, and return the locker's numbered key.
 * dhd_pktid_map_alloc() is not reentrant, and is the caller's responsibility.
 * Caller must treat a returned value DHD_PKTID_INVALID as a failure case,
 * implying a depleted pool of pktids.
 */

static INLINE uint32
__dhd_pktid_map_reserve(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, void *pkt)
{
	uint32 nkey;
	dhd_pktid_map_t *map;
	dhd_pktid_item_t *locker;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;

	if (map->avail <= 0) { /* no more pktids to allocate */
		map->failures++;
		DHD_INFO(("%s:%d: failed, no free keys\n", __FUNCTION__, __LINE__));
		return DHD_PKTID_INVALID; /* failed alloc request */
	}

	ASSERT(map->avail <= map->items);
	nkey = map->keys[map->avail]; /* fetch a free locker, pop stack */
	locker = GET_LOCKER(map, nkey);
	map->avail--;
	locker->pkt = pkt; /* pkt is saved, other params not yet saved. */
	locker->len = 0;
	locker->state = LOCKER_IS_BUSY; /* reserve this locker */

#if defined(DHD_PKTID_AUDIT_MAP)
	DHD_PKTID_AUDIT(dhd, map, nkey, DHD_DUPLICATE_ALLOC); /* Audit duplicate alloc */
#endif /* DHD_PKTID_AUDIT_MAP */

	ASSERT(nkey != DHD_PKTID_INVALID);

	return nkey; /* return locker's numbered key */
}

/**
 * dhd_pktid_map_reserve - reserve a unique numbered key. Reserved locker is not
 * yet populated. Invoke the pktid save api to populate the packet parameters
 * into the locker.
 * Wrapper that takes the required lock when called directly.
 */
static INLINE uint32
dhd_pktid_map_reserve(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, void *pkt)
{
	dhd_pktid_map_t *map;
	uint32 flags;
	uint32 ret;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;
	flags = DHD_PKTID_LOCK(map->pktid_lock);
	ret = __dhd_pktid_map_reserve(dhd, handle, pkt);
	DHD_PKTID_UNLOCK(map->pktid_lock, flags);

	return ret;
}

static INLINE void
__dhd_pktid_map_save(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, void *pkt,
	uint32 nkey, dmaaddr_t pa, uint32 len, uint8 dir, void *dmah, void *secdma,
	dhd_pkttype_t pkttype)
{
	dhd_pktid_map_t *map;
	dhd_pktid_item_t *locker;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;

	ASSERT((nkey != DHD_PKTID_INVALID) && (nkey <= map->items));

	locker = GET_LOCKER(map, nkey);

	ASSERT(((locker->state == LOCKER_IS_BUSY) && (locker->pkt == pkt)) ||
		((locker->state == LOCKER_IS_RSVD) && (locker->pkt == NULL)));

#if defined(DHD_PKTID_AUDIT_MAP)
	DHD_PKTID_AUDIT(dhd, map, nkey, DHD_TEST_IS_ALLOC); /* apriori, reservation */
#endif /* DHD_PKTID_AUDIT_MAP */

	/* store contents in locker */
	locker->dir = dir;
	locker->pa = pa;
	locker->len = (uint16)len; /* 16bit len */
	locker->dmah = dmah; /* 16bit len */
	locker->secdma = secdma;
	locker->pkttype = pkttype;
	locker->pkt = pkt;
	locker->state = LOCKER_IS_BUSY; /* make this locker busy */
}

/**
 * dhd_pktid_map_save - Save a packet's parameters into a locker corresponding
 * to a previously reserved unique numbered key.
 * Wrapper that takes the required lock when called directly.
 */
static INLINE void
dhd_pktid_map_save(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, void *pkt,
	uint32 nkey, dmaaddr_t pa, uint32 len, uint8 dir, void *dmah, void *secdma,
	dhd_pkttype_t pkttype)
{
	dhd_pktid_map_t *map;
	uint32 flags;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;
	flags = DHD_PKTID_LOCK(map->pktid_lock);
	__dhd_pktid_map_save(dhd, handle, pkt, nkey, pa, len,
		dir, dmah, secdma, pkttype);
	DHD_PKTID_UNLOCK(map->pktid_lock, flags);
}

/**
 * dhd_pktid_map_alloc - Allocate a unique numbered key and save the packet
 * contents into the corresponding locker. Return the numbered key.
 */
static uint32 BCMFASTPATH
dhd_pktid_map_alloc(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, void *pkt,
	dmaaddr_t pa, uint32 len, uint8 dir, void *dmah, void *secdma,
	dhd_pkttype_t pkttype)
{
	uint32 nkey;
	uint32 flags;
	dhd_pktid_map_t *map;

	ASSERT(handle != NULL);
	map = (dhd_pktid_map_t *)handle;

	flags = DHD_PKTID_LOCK(map->pktid_lock);

	nkey = __dhd_pktid_map_reserve(dhd, handle, pkt);
	if (nkey != DHD_PKTID_INVALID) {
		__dhd_pktid_map_save(dhd, handle, pkt, nkey, pa,
			len, dir, dmah, secdma, pkttype);
#if defined(DHD_PKTID_AUDIT_MAP)
		DHD_PKTID_AUDIT(dhd, map, nkey, DHD_TEST_IS_ALLOC); /* apriori, reservation */
#endif /* DHD_PKTID_AUDIT_MAP */
	}

	DHD_PKTID_UNLOCK(map->pktid_lock, flags);

	return nkey;
}

/**
 * dhd_pktid_map_free - Given a numbered key, return the locker contents.
 * dhd_pktid_map_free() is not reentrant, and is the caller's responsibility.
 * Caller may not free a pktid value DHD_PKTID_INVALID or an arbitrary pktid
 * value. Only a previously allocated pktid may be freed.
 */
static void * BCMFASTPATH
dhd_pktid_map_free(dhd_pub_t *dhd, dhd_pktid_map_handle_t *handle, uint32 nkey,
	dmaaddr_t *pa, uint32 *len, void **dmah, void **secdma,
	dhd_pkttype_t pkttype, bool rsv_locker)
{
	dhd_pktid_map_t *map;
	dhd_pktid_item_t *locker;
	void * pkt;
	uint32 flags;

	ASSERT(handle != NULL);

	map = (dhd_pktid_map_t *)handle;

	flags = DHD_PKTID_LOCK(map->pktid_lock);

	ASSERT((nkey != DHD_PKTID_INVALID) && (nkey <= map->items));

	locker = GET_LOCKER(map, nkey);

#if defined(DHD_PKTID_AUDIT_MAP)
	DHD_PKTID_AUDIT(dhd, map, nkey, DHD_DUPLICATE_FREE); /* Audit duplicate FREE */
#endif /* DHD_PKTID_AUDIT_MAP */

	if (locker->state == LOCKER_IS_FREE) { /* Debug check for cloned numbered key */
		DHD_ERROR(("%s:%d: Error! freeing invalid pktid<%u>\n",
			__FUNCTION__, __LINE__, nkey));
		ASSERT(locker->state != LOCKER_IS_FREE);

		DHD_PKTID_UNLOCK(map->pktid_lock, flags);
		return NULL;
	}

	/* Check for the colour of the buffer i.e The buffer posted for TX,
	 * should be freed for TX completion. Similarly the buffer posted for
	 * IOCTL should be freed for IOCT completion etc.
	 */
	if ((pkttype != PKTTYPE_NO_CHECK) && (locker->pkttype != pkttype)) {

		DHD_PKTID_UNLOCK(map->pktid_lock, flags);

		DHD_ERROR(("%s:%d: Error! Invalid Buffer Free for pktid<%u> \n",
			__FUNCTION__, __LINE__, nkey));
		ASSERT(locker->pkttype == pkttype);

		return NULL;
	}

	if (rsv_locker == DHD_PKTID_FREE_LOCKER) {
		map->avail++;
		map->keys[map->avail] = nkey; /* make this numbered key available */
		locker->state = LOCKER_IS_FREE; /* open and free Locker */
	} else {
		/* pktid will be reused, but the locker does not have a valid pkt */
		locker->state = LOCKER_IS_RSVD;
	}

#if defined(DHD_PKTID_AUDIT_MAP)
	DHD_PKTID_AUDIT(dhd, map, nkey, DHD_TEST_IS_FREE);
#endif /* DHD_PKTID_AUDIT_MAP */

	*pa = locker->pa; /* return contents of locker */
	*len = (uint32)locker->len;
	*dmah = locker->dmah;
	*secdma = locker->secdma;

	pkt = locker->pkt;
	locker->pkt = NULL; /* Clear pkt */
	locker->len = 0;

	DHD_PKTID_UNLOCK(map->pktid_lock, flags);

	return pkt;
}

#else /* ! DHD_PCIE_PKTID */

#ifndef linux
#error "DHD_PCIE_PKTID has to be defined for non-linux/android platforms"
#endif

typedef struct pktlists {
	PKT_LIST *tx_pkt_list;		/* list for tx packets */
	PKT_LIST *rx_pkt_list;		/* list for rx packets */
	PKT_LIST *ctrl_pkt_list;	/* list for ioctl/event buf post */
#if defined(CONFIG_ARM64)
	uint64 pktidmaphigh;		/* fixed map value for higher 32bit */
#endif /* CONFIG_ARM64 */
} pktlists_t;

/*
 * Given that each workitem only uses a 32bit pktid, only 32bit hosts may avail
 * of a one to one mapping 32bit pktptr and a 32bit pktid.
 *
 * - When PKTIDMAP is not used, DHD_NATIVE_TO_PKTID variants will never fail.
 * - Neither DHD_NATIVE_TO_PKTID nor DHD_PKTID_TO_NATIVE need to be protected by
 *   a lock.
 * - Hence DHD_PKTID_INVALID is not defined when DHD_PCIE_PKTID is undefined.
 */
#if defined(CONFIG_ARM64)
#define DHD_PKTID32(pktptr)	((uint32)((uint64)pktptr))
#define DHD_PKTPTR32(pktid32)	((void *)((uint64)pktid32))
#else /* !CONFIG_ARM64 */
#define DHD_PKTID32(pktptr32)	((uint32)(pktptr32))
#define DHD_PKTPTR32(pktid32)	((void *)(pktid32))
#endif /* !CONFIG_ARM64 */

static INLINE uint32 dhd_native_to_pktid(dhd_pktid_map_handle_t *map, void *pktptr,
	dmaaddr_t pa, uint32 dma_len, void *dmah, void *secdma,
	dhd_pkttype_t pkttype);
static INLINE void * dhd_pktid_to_native(dhd_pktid_map_handle_t *map, uint32 pktid32,
	dmaaddr_t *pa, uint32 *dma_len, void **dmah, void **secdma,
	dhd_pkttype_t pkttype);

static dhd_pktid_map_handle_t *
dhd_pktid_map_init(dhd_pub_t *dhd, uint32 num_items, uint32 index)
{
	osl_t *osh = dhd->osh;
	pktlists_t *handle = NULL;

	if ((handle = (pktlists_t *) MALLOCZ(osh, sizeof(pktlists_t))) == NULL) {
		DHD_ERROR(("%s:%d: MALLOC failed for lists allocation, size=%d\n",
		           __FUNCTION__, __LINE__, (int)sizeof(pktlists_t)));
		goto error_done;
	}

	if ((handle->tx_pkt_list = (PKT_LIST *) MALLOC(osh, sizeof(PKT_LIST))) == NULL) {
		DHD_ERROR(("%s:%d: MALLOC failed for list allocation, size=%d\n",
		           __FUNCTION__, __LINE__, (int)sizeof(PKT_LIST)));
		goto error;
	}

	if ((handle->rx_pkt_list = (PKT_LIST *) MALLOC(osh, sizeof(PKT_LIST))) == NULL) {
		DHD_ERROR(("%s:%d: MALLOC failed for list allocation, size=%d\n",
		           __FUNCTION__, __LINE__, (int)sizeof(PKT_LIST)));
		goto error;
	}

	if ((handle->ctrl_pkt_list = (PKT_LIST *) MALLOC(osh, sizeof(PKT_LIST))) == NULL) {
		DHD_ERROR(("%s:%d: MALLOC failed for list allocation, size=%d\n",
		           __FUNCTION__, __LINE__, (int)sizeof(PKT_LIST)));
		goto error;
	}

#if defined(CONFIG_ARM64)
	{
	    void *pkt;

	    /* Allocate a dummy packet to get the higher 32bits of a packet */
	    if ((pkt = PKTGET(dhd->osh, DHD_RX_PKT_BUFSZ, FALSE)) == NULL) {
	        DHD_ERROR(("%s:%d: PKTGET failed for dummy allocation, size=%d\n",
	            __FUNCTION__, __LINE__, DHD_RX_PKT_BUFSZ));
	        goto error;
	    }
	    handle->pktidmaphigh = (uint64)pkt & 0xFFFFFFFF00000000;

	    DHD_INFO(("%s:%d: pktidmaphigh set to [0x%llx]\r\n",
	        __FUNCTION__, __LINE__, handle->pktidmaphigh));

	    PKTFREE(dhd->osh, pkt, FALSE);
	}
#endif /* CONFIG_ARM64 */

	PKTLIST_INIT(handle->tx_pkt_list);
	PKTLIST_INIT(handle->rx_pkt_list);
	PKTLIST_INIT(handle->ctrl_pkt_list);

	return (dhd_pktid_map_handle_t *) handle;

error:
	if (handle->ctrl_pkt_list) {
		MFREE(osh, handle->ctrl_pkt_list, sizeof(PKT_LIST));
	}

	if (handle->rx_pkt_list) {
		MFREE(osh, handle->rx_pkt_list, sizeof(PKT_LIST));
	}

	if (handle->tx_pkt_list) {
		MFREE(osh, handle->tx_pkt_list, sizeof(PKT_LIST));
	}

	if (handle) {
		MFREE(osh, handle, sizeof(pktlists_t));
	}

error_done:
	return (dhd_pktid_map_handle_t *)NULL;
}

static void
dhd_pktid_map_fini(dhd_pub_t *dhd, dhd_pktid_map_handle_t *map)
{
	osl_t *osh = dhd->osh;
	pktlists_t *handle = (pktlists_t *) map;

	ASSERT(handle != NULL);
	if (handle == (pktlists_t *)NULL) {
		return;
	}

	if (handle->ctrl_pkt_list) {
		PKTLIST_FINI(osh, handle->ctrl_pkt_list);
		MFREE(osh, handle->ctrl_pkt_list, sizeof(PKT_LIST));
	}

	if (handle->rx_pkt_list) {
		PKTLIST_FINI(osh, handle->rx_pkt_list);
		MFREE(osh, handle->rx_pkt_list, sizeof(PKT_LIST));
	}

	if (handle->tx_pkt_list) {
		PKTLIST_FINI(osh, handle->tx_pkt_list);
		MFREE(osh, handle->tx_pkt_list, sizeof(PKT_LIST));
	}

	if (handle) {
		MFREE(osh, handle, sizeof(pktlists_t));
	}
}

/** Save dma parameters into the packet's pkttag and convert a pktptr to pktid */
static INLINE uint32
dhd_native_to_pktid(dhd_pktid_map_handle_t *map, void *pktptr,
	dmaaddr_t pa, uint32 dma_len, void *dmah, void *secdma,
	dhd_pkttype_t pkttype)
{
	pktlists_t *handle = (pktlists_t *) map;
	ASSERT(pktptr != NULL);
	DHD_PKT_SET_DMA_LEN(pktptr, dma_len);
	DHD_PKT_SET_DMAH(pktptr, dmah);
	DHD_PKT_SET_PA(pktptr, pa);
	DHD_PKT_SET_SECDMA(pktptr, secdma);

	if (pkttype == PKTTYPE_DATA_TX) {
		PKTLIST_ENQ(handle->tx_pkt_list,  pktptr);
	} else if (pkttype == PKTTYPE_DATA_RX) {
		PKTLIST_ENQ(handle->rx_pkt_list,  pktptr);
	} else {
		PKTLIST_ENQ(handle->ctrl_pkt_list,  pktptr);
	}

#if defined(CONFIG_ARM64)
	if (handle->pktidmaphigh != ((uint64)pktptr & 0xFFFFFFFF00000000)) {
	    DHD_ERROR(("Unexpected pktidmaphigh change [0x%llx] -> [0x%llx] for pkt [0x%p]\r\n",
	        handle->pktidmaphigh, (uint64)pktptr & 0xFFFFFFFF00000000, pktptr));
	    DHD_ERROR(("Compile DHD with DHD_PCIE_PKTID flag\r\n"));
	    ASSERT(0);
	}
#endif /* CONFIG_ARM64 */

	return DHD_PKTID32(pktptr);
}

/** Convert a pktid to pktptr and retrieve saved dma parameters from packet */
static INLINE void *
dhd_pktid_to_native(dhd_pktid_map_handle_t *map, uint32 pktid32,
	dmaaddr_t *pa, uint32 *dma_len, void **dmah, void **secdma,
	dhd_pkttype_t pkttype)
{
	pktlists_t *handle = (pktlists_t *) map;
	void *pktptr;

	ASSERT(pktid32 != 0U);
	pktptr = DHD_PKTPTR32(pktid32);

#if defined(CONFIG_ARM64)
	/* Add the higher 32bit of PKTPTR */
	pktptr = (void*) ((uint64)pktptr | handle->pktidmaphigh);
#endif /* CONFIG_ARM64 */

	*dma_len = DHD_PKT_GET_DMA_LEN(pktptr);
	*dmah = DHD_PKT_GET_DMAH(pktptr);
	*pa = DHD_PKT_GET_PA(pktptr);
	*secdma = DHD_PKT_GET_SECDMA(pktptr);

	if (pkttype == PKTTYPE_DATA_TX) {
		PKTLIST_UNLINK(handle->tx_pkt_list,  pktptr);
	} else if (pkttype == PKTTYPE_DATA_RX) {
		PKTLIST_UNLINK(handle->rx_pkt_list,  pktptr);
	} else {
		PKTLIST_UNLINK(handle->ctrl_pkt_list,  pktptr);
	}

	return pktptr;
}

#define DHD_NATIVE_TO_PKTID_RSV(dhd, map, pkt)  DHD_PKTID32(pkt)

#define DHD_NATIVE_TO_PKTID_SAVE(dhd, map, pkt, nkey, pa, len, dma_dir, dmah, secdma, pkttype) \
	({ BCM_REFERENCE(dhd); BCM_REFERENCE(nkey); BCM_REFERENCE(dma_dir); \
	   dhd_native_to_pktid((dhd_pktid_map_handle_t *) map, (pkt), (pa), (len), \
			   (dmah), (secdma), (dhd_pkttype_t)(pkttype)); \
	})

#define DHD_NATIVE_TO_PKTID(dhd, map, pkt, pa, len, dma_dir, dmah, secdma, pkttype) \
	({ BCM_REFERENCE(dhd); BCM_REFERENCE(dma_dir); \
	   dhd_native_to_pktid((dhd_pktid_map_handle_t *) map, (pkt), (pa), (len), \
			   (dmah), (secdma), (dhd_pkttype_t)(pkttype)); \
	})

#define DHD_PKTID_TO_NATIVE(dhd, map, pktid, pa, len, dmah, secdma, pkttype) \
	({ BCM_REFERENCE(dhd); BCM_REFERENCE(pkttype);	\
		dhd_pktid_to_native((dhd_pktid_map_handle_t *) map, (uint32)(pktid), \
				(dmaaddr_t *)&(pa), (uint32 *)&(len), (void **)&(dmah), \
				(void **)&secdma, (dhd_pkttype_t)(pkttype)); \
	})

#define DHD_PKTID_AVAIL(map)  (~0)

#endif /* ! DHD_PCIE_PKTID */

/* +------------------ End of PCIE DHD PKTID MAPPER  -----------------------+ */

#if defined(BCM_BUZZZ_STREAMING_BUILD)

static int  dhd_bcm_buzzz_attach(dhd_pub_t *dhd);
static void dhd_bcm_buzzz_detach(dhd_pub_t *dhd);
static void dhd_bcm_buzzz_init(dhd_pub_t *dhd);

int // Allocate DMAable memory of 32 MBytes total in 8 segments of 4 MBytes each
dhd_bcm_buzzz_attach(dhd_pub_t *dhd)
{
	int seg;
	uint32 dma_buf_len = BCM_BUZZZ_HOSTMEM_SEGSZ;

	/* BCM_BUZZZ_STREAMING_BUILD configuration : see bcm_buzzz.h */
#if defined(BCM_BUZZZ_FUNC)
	ASSERT(sizeof(bcm_buzzz_log_t) == 8);
#else
	ASSERT(sizeof(bcm_buzzz_log_t) == 16);
#endif

	ASSERT(BCM_BUZZZ_SEGMENTS == 8);
	ASSERT(BCM_BUZZZ_HOSTMEM_SEGSZ == (4  * 1024 * 1024));
	ASSERT(BCM_BUZZZ_HOSTMEM_TOTSZ == (32 * 1024 * 1024));

	DHD_ERROR(("bcm_buzzz_attach:\n"));

	if (dhd->bcm_buzzz_dma_buf != NULL)
		return BCME_OK;

	/* Allocate a table of dhd_dma_buf_t, one entry per host memory segment */
	dhd->bcm_buzzz_dma_buf =
		MALLOCZ(dhd->osh, sizeof(dhd_dma_buf_t) * BCM_BUZZZ_SEGMENTS);

	if (dhd->bcm_buzzz_dma_buf == NULL)
		return BCME_NOMEM;

	/* Populate the table with DMA-able host memory */
	for (seg = 0; seg < BCM_BUZZZ_SEGMENTS; seg++)
	{
		if (dhd_dma_buf_alloc(dhd, dhd->bcm_buzzz_dma_buf + seg, "buzzz",
				dma_buf_len, DHD_MEM_ALIGN_BITS_MIN)) {
			DHD_ERROR(("\t Failure allocation segment %u\n", seg));
				return BCME_NOMEM;
		}

		ASSERT(PHYSADDRHI((dhd->bcm_buzzz_dma_buf + seg)->pa) == 0U);
		dhd->bcm_buzzz_pa[seg] = PHYSADDRLO((dhd->bcm_buzzz_dma_buf + seg)->pa);
		dhd->bcm_buzzz_va[seg] = (void*)(dhd->bcm_buzzz_dma_buf + seg)->va;

		DHD_ERROR(("\t segment[%d] = va<%p> pa<0x%08x>\n",
			seg, dhd->bcm_buzzz_va[seg], dhd->bcm_buzzz_pa[seg]));
	}

	DHD_ERROR(("\t %u segments of %u size = total %u BYTES\n",
		BCM_BUZZZ_SEGMENTS, BCM_BUZZZ_HOSTMEM_SEGSZ, BCM_BUZZZ_HOSTMEM_TOTSZ));

	return BCME_OK;
} /* dhd_bcm_buzzz_attach */

void // Deallocate all DMAable memory for BUZZZ Streaming to host
dhd_bcm_buzzz_detach(dhd_pub_t *dhd)
{
	int seg;

	DHD_ERROR(("bcm_buzzz_detach\n"));
	if (dhd->bcm_buzzz_dma_buf == NULL)
		return;

	/* Free each buzzz host memory segment */
	for (seg = 0; seg < BCM_BUZZZ_SEGMENTS; seg++) {
		dhd->bcm_buzzz_va[seg] = NULL;
		dhd->bcm_buzzz_pa[seg] = 0U;
		dhd_dma_buf_free(dhd, dhd->bcm_buzzz_dma_buf + seg, "buzzz");
	}

	MFREE(dhd->osh, dhd->bcm_buzzz_dma_buf,
		sizeof(dhd_dma_buf_t) * BCM_BUZZZ_SEGMENTS);

	dhd->bcm_buzzz_dma_buf = NULL;

} /* dhd_bcm_buzzz_detach */

void // Register stream buffers into dongle
dhd_bcm_buzzz_init(dhd_pub_t *dhd)
{
	int seg;
	uint32 bcm_buzzz_pa[BCM_BUZZZ_SEGMENTS];

	DHD_ERROR(("dhd_bcm_buzzz_init:\n"));

	/* Prepare the list of 32bit addresses of the host memory segments */
	for (seg = 0; seg < BCM_BUZZZ_SEGMENTS; seg++) {
		bcm_buzzz_pa[seg] = htol32(dhd->bcm_buzzz_pa[seg]);
	}

	/* Post addresses of all buzzz host memory segments to dongle */
	dhd_bus_cmn_writeshared(dhd->bus, bcm_buzzz_pa, BUZZZ_HOSTMEM_SEGMENTS, 0);

} /* dhd_bcm_buzzz_init */

#endif /* BCM_BUZZZ_STREAMING_BUILD */

/**
 * Legacy (pre PCIe IPC rev 0x82) Host Memory scratch buffer is allocated for
 * all possible dongle features (even if they are not enabled). This maximum
 * sized memory is pre-allocated in dhd_prot_attach() (pcie probe time). Dongle
 * features carve their memories from this large maximum sized host memory.
 *
 * With the introduction of HME service over the PCIe IPC, DHD will allocate
 * DMA-able memory buffers per advertized dongle HME user.
 * When HME service is advertized, the legacy scratch host memory is freed.
 */
/* XXX: In lab firmware-only download scheme, a rollback to a legacy PCIe IPC
 * revision that does not support HME service requires the scratch host memory
 * to be re-allocated and is susceptible to host memory fragmentation.
 * See: dhd_prot_init()
 */
int
dhd_prot_host_mem_alloc(dhd_pub_t *dhd)
{
	/* Sections are padded to 4096 Bytes - a DHD page unit */
	const uint32 rounding = (1 << DHD_DMA_BUF_4KB_ALIGN_BITS);
	uint32 dma_buf_len;
	dma_buf_len = ROUNDUP(DMA_D2H_SCRATCH_BUF_LEN, rounding);
#if defined(BCM_DHDHDR)
	dma_buf_len += ROUNDUP(BCM_DHDHDR_DMA_BUF_LEN, rounding);
#endif
#ifdef BCM_HOST_MEM_SCB
	dma_buf_len += DMA_HOST_BUFFER_LEN;
#endif
	DHD_INFO(("%s: Alloc Legacy Host Memory DMA Buffer len %u rounding %u\n",
		__FUNCTION__, dma_buf_len, rounding));
	return dhd_dma_buf_alloc(dhd, &dhd->prot->host_mem_dma_buf, "host_mem",
	                         dma_buf_len, DHD_DMA_BUF_4KB_ALIGN_BITS);

}   /* dhd_prot_host_mem_alloc() */

void
dhd_prot_host_mem_free(dhd_pub_t *dhd)
{
	DHD_INFO(("%s: Free Legacy Host Memory DMA Buffer\n", __FUNCTION__));
	dhd_dma_buf_free(dhd, &dhd->prot->host_mem_dma_buf, "host_mem");
}   /* dhd_prot_host_mem_free() */

/**
 * The PCIE FD protocol layer is constructed in two phases:
 *    Phase 1. dhd_prot_attach()
 *    Phase 1. dhd_prot_preinit()
 *             dhd_prot_hme_reset()
 *             dhd_prot_hme_init()
 *    Phase 2. dhd_prot_init()
 *
 * dhd_prot_attach() - Allocates a dhd_prot_t object and resets all its fields.
 * Some Common rings may be attached (msgbuf_ring_t objects are allocated
 * with DMA-able buffers). Dongle may request support for ACWI formatted common
 * rings, hence, such rings may be allocated during dhd_prot_preinit
 *
 * All dhd_dma_buf_t objects are also allocated here.
 *
 * As dhd_prot_attach is invoked prior to the pcie_ipc object is read, any
 * initialization of objects that requires information advertized by the dongle
 * may not be performed here.
 * E.g. the number of TxPost flowrings is not known at this point, neither do
 * we know which form of D2H DMA sync mechanism is advertized by the dongle, or
 * whether the dongle supports DMA-ing of WR/RD indices for the H2D and/or D2H
 * rings (common + flow).
 *
 * dhd_prot_init() is invoked after the bus layer has fetched the information
 * advertized by the dongle in the pcie_ipc_t.
 */
int
dhd_prot_attach(dhd_pub_t *dhd)
{
	osl_t *osh = dhd->osh;
	dhd_prot_t *prot;
#if defined(BCM_DHD_RUNNER)
	bcmpcie_soft_doorbell_t *soft_doobells = NULL;
#endif  /* BCM_DHD_RUNNER */

	/* Allocate prot structure */
	if (!(prot = (dhd_prot_t *)DHD_OS_PREALLOC(dhd, DHD_PREALLOC_PROT,
		sizeof(dhd_prot_t)))) {
		DHD_ERROR(("%s: kmalloc failed\n", __FUNCTION__));
		goto fail;
	}
	memset(prot, 0, sizeof(*prot));

	prot->osh = osh;
	dhd->prot = prot;

	/* DMAing ring completes supported? FALSE by default  */
	dhd->dma_d2h_ring_upd_support = FALSE;
	dhd->dma_h2d_ring_upd_support = FALSE;

#if defined(BCM_DHD_RUNNER)
#if defined(DHD_D2H_SOFT_DOORBELL_SUPPORT)
	soft_doobells = &prot->soft_doorbell[0];
#endif /* DHD_D2H_SOFT_DOORBELL_SUPPORT */

	DHD_RNR_CLR_OFFL(dhd);

	dhd->runner_hlp = dhd_runner_attach(dhd, soft_doobells);
	if (dhd->runner_hlp == NULL) {
		DHD_ERROR(("%s dhd_runner_attach failure\n", __FUNCTION__));
		goto fail;
	}

	DHD_RNR_SET_OFFL(dhd);
#endif /* BCM_DHD_RUNNER */

	dhd->idma_inited = 0;

	/* Common Ring Allocations */

	/* Control Submission and Completion Common ring, uses legacy format. */
	prot->use_haddr64 = TRUE; /* Full 64bit host address. */
	prot->host_physaddrhi = 0xFFFFFFFF; /* No check on physaddrhi */

	/* Ring  0: H2D Control Submission */
	if (dhd_prot_ring_alloc(dhd, &prot->h2dring_ctrl_subn, "h2dctrl",
	        BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT, MSGBUF_WI_WI64,
	        H2DRING_CTRL_SUB_ITEMSIZE, H2DRING_CTRL_SUB_MAX_ITEM,
	        DHD_MSGBUF_RING_ITEM_MISC) != BCME_OK)
	{
		DHD_ERROR(("H2D Ctrl Submission Ring Alloc failed\n")); ASSERT(0);
		goto fail;
	}

	/* Ring  2: D2H Control Completion */
	if (dhd_prot_ring_alloc(dhd, &prot->d2hring_ctrl_cpln, "d2hctrl",
	        BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE, MSGBUF_WI_WI64,
	        D2HRING_CTRL_CMPLT_ITEMSIZE, D2HRING_CTRL_CMPLT_MAX_ITEM,
	        DHD_MSGBUF_RING_ITEM_MISC) != BCME_OK)
	{
		DHD_ERROR(("D2H Ctrl Completion Ring Alloc failed\n")); ASSERT(0);
		goto fail;
	}

	/*
	 * Max number of flowrings is not yet known. msgbuf_ring_t with DMA-able
	 * buffers for flowrings will be instantiated, in dhd_prot_init() .
	 * See dhd_prot_flowrings_pool_alloc()
	 */
	/* ioctl response buffer */
	if (dhd_dma_buf_alloc(dhd, &prot->ioctl_resp_dma_buf, "ioctl_resp",
	                      IOCTL_DMA_BUF_LEN, DHD_DMA_BUF_ALIGN_BITS)) {
		goto fail;
	}

	/* IOCTL request buffer */
	if (dhd_dma_buf_alloc(dhd, &prot->ioctl_rqst_dma_buf, "ioctl_rqst",
	                      IOCTL_DMA_BUF_LEN, DHD_DMA_BUF_ALIGN_BITS)) {
		goto fail;
	}

	/* Legacy Host Scratch Memory used as an extension by dongle */
	if (dhd_prot_host_mem_alloc(dhd)) {
		goto fail;
	}

	/* scratch buffer bus throughput measurement, DHD_DMA_BUF_ALIGN_BITS */
	if (dhd_dma_buf_alloc(dhd, &prot->bus_tput_dma_buf, "bus_tput",
	                      DHD_BUS_TPUT_BUF_LEN, DHD_DMA_BUF_ALIGN_BITS)) {
		goto fail;
	}

#ifdef DHD_RX_CHAINING
	dhd_rxchain_reset(&prot->rxchain);
#endif

#if defined(DHD_LB)

	   /* Initialize the work queues to be used by the Load Balancing logic */
#if defined(DHD_LB_TXC)
	{
		void *buffer;
		buffer = MALLOC(dhd->osh, sizeof(void*) * DHD_LB_WORKQ_SZ);
		bcm_workq_init(&prot->tx_compl_prod, &prot->tx_compl_cons,
			buffer, DHD_LB_WORKQ_SZ);
		prot->tx_compl_prod_sync = 0;
		DHD_INFO(("%s: created tx_compl_workq <%p,%d>\n",
			__FUNCTION__, buffer, DHD_LB_WORKQ_SZ));
	}
#endif /* DHD_LB_TXC */

#if defined(DHD_LB_RXC)
	{
		void *buffer;
		buffer = MALLOC(dhd->osh, sizeof(uint32) * DHD_LB_WORKQ_SZ);
		bcm_workq_init(&prot->rx_compl_prod, &prot->rx_compl_cons,
			buffer, DHD_LB_WORKQ_SZ);
		prot->rx_compl_prod_sync = 0;
		DHD_INFO(("%s: created rx_compl_workq <%p,%d>\n",
			__FUNCTION__, buffer, DHD_LB_WORKQ_SZ));
	}
#endif /* DHD_LB_RXC */

#endif /* DHD_LB */

#if defined(BCM_BUZZZ_STREAMING_BUILD)
	if (dhd_bcm_buzzz_attach(dhd)) {
		DHD_ERROR(("dhd_bcm_buzzz_attach failed\n"));
		goto fail;
	}
#endif /* BCM_BUZZZ_STREAMING_BUILD */

	return BCME_OK;

fail:

#ifndef CONFIG_DHD_USE_STATIC_BUF
	if (prot != NULL) {
		dhd_prot_detach(dhd);
	}
#endif /* CONFIG_DHD_USE_STATIC_BUF */

	return BCME_NOMEM;
} /* dhd_prot_attach */

/**
 * dhd_prot_preinit - Complete the allocation of all other common rings using
 * the messaging formats requested by dongle and host.
 */
int
dhd_prot_preinit(dhd_pub_t *dhdp, bool use_haddr64, uint32 host_physaddrhi,
	uint16 rxcpln_dataoffet, uint8 max_rxcpln_rings,
	uint8 txpost_format, uint8 rxpost_format,
	uint8 txcpln_format, uint8 rxcpln_format,
	uint32 flags)
{
	dhd_prot_t *prot = dhdp->prot;
	uint16 ringid, i;
	uint16 type = 0, misc = 0, max = 0, size = 0; /* work item configuration */
#if defined(STB) && !defined(STBAP)
	/* Dont call preinit again(suspend-resume) */
	if (prot->txpost_max_items)
		return BCME_OK;
#endif /* STB && !STBAP */

	/* Common configuration for RxPost, TxPost, TxCpln and RxCpln */
	prot->use_haddr64 = use_haddr64;
	prot->host_physaddrhi = host_physaddrhi;

	/** TxPost Flowring configuration */
	switch (txpost_format)
	{
		case MSGBUF_WI_LEGACY:
			ASSERT(sizeof(host_txbuf_post_t) == H2DRING_TXPOST_ITEMSIZE);
			type = MSGBUF_WI_WI64; size = sizeof(host_txbuf_post_t);
			misc = OFFSETOF(host_txbuf_post_t, txhdr);
			break;

		case MSGBUF_WI_COMPACT:
			if (prot->use_haddr64 == TRUE ||
				(flags & PCIE_IPC_FLAGS_NO_TXPOST_CWI32)) {
				ASSERT(sizeof(hwa_txpost_cwi64_t) == HWA_TXPOST_CWI64_BYTES);
				type = MSGBUF_WI_CWI64; size = sizeof(hwa_txpost_cwi64_t);
				misc = OFFSETOF(hwa_txpost_cwi64_t, eth_sada);
			} else {
				ASSERT(sizeof(hwa_txpost_cwi32_t) == HWA_TXPOST_CWI32_BYTES);
				type = MSGBUF_WI_CWI32; size = sizeof(hwa_txpost_cwi32_t);
				misc = OFFSETOF(hwa_txpost_cwi32_t, eth_sada);
			}
			break;

		case MSGBUF_WI_AGGREGATE: /* not supported for TxPost Flowrings */
		default:
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"Invalid txpost_format %u\n", __FUNCTION__, txpost_format));
			ASSERT(0);
			return BCME_UNSUPPORTED;
	}

	max = H2DRING_TXPOST_MAX_ITEM; /* No aggregation for TxPost */

	/* Configure the prot layer for the dynamically allocated Txpost flowring
	 * When TxPost Flowrings are constructed, the following fields saved in the
	 * prot layer will be applied.
	 */
	prot->txpost_item_type = type; /* TxPost: legacy, cwi32, cwi64 */
	prot->txpost_item_size = size; /* Size of a TxPost work item */
	prot->txpost_max_items = max;  /* Max number of TxPost work items in ring */
	prot->txpost_item_misc = misc; /* Offset of eth header in work item */

	DHD_PCIE_IPC(("\t Protocol: H2D TxPost  : "
		"format %u [type %u size %2u max %4u hdr_off %u]\n",
		txpost_format, type, size, max, misc));

#if defined(BCM_DHD_RUNNER)
	/* dhd_runner protocol pre-init Start */
	if (dhd_prot_runner_preinit(dhdp) != BCME_OK) {
	    DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
			"Runner preinit\n", __FUNCTION__));
		ASSERT(0);
	    return BCME_ERROR;
	}
	/* dhd_runner protocol pre-init Complete */
#endif /* BCM_DHD_RUNNER */

	/**
	 * Common Ring Construction: Configuration and Allocation
	 * Later in dhd_prot_init, the common ring info will be provided to dongle.
	 */

	/** Ring  1: H2D Rx Post Common Ring Construction */
	ringid = BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT;

	prot->rxpost_fn = dhd_prot_rxbuf_post; /* no Rx load balancing, yet */

#if defined(DHD_LB)
	/* Handler Override: with LoadBalancing support for legacy formats */
	ASSERT(rxpost_format == MSGBUF_WI_LEGACY);
	prot->rxpost_fn = dhd_prot_rxbuf_post_lb;
#endif /* DHD_LB_RXC */

	misc = 0; /* default, single item per message, shift of 0 */

	switch (rxpost_format)
	{
		case MSGBUF_WI_LEGACY:
			ASSERT(sizeof(host_rxbuf_post_t) == H2DRING_RXPOST_ITEMSIZE);
			type = MSGBUF_WI_WI64; size = sizeof(host_rxbuf_post_t);
			break;

		case MSGBUF_WI_COMPACT:
			if (prot->use_haddr64 == TRUE) {
				ASSERT(sizeof(hwa_rxpost_cwi64_t) == HWA_RXPOST_CWI64_BYTES);
				type = MSGBUF_WI_CWI64; size = sizeof(hwa_rxpost_cwi64_t);
			} else {
				ASSERT(sizeof(hwa_rxpost_cwi32_t) == HWA_RXPOST_CWI32_BYTES);
				type = MSGBUF_WI_CWI32; size = sizeof(hwa_rxpost_cwi32_t);
			}
			break;

		case MSGBUF_WI_AGGREGATE:
			misc = 2; /* shift-2 for multiplaction/division by HWA_AGGR_MAX */
			if (prot->use_haddr64 == TRUE) {
				ASSERT(sizeof(hwa_rxpost_acwi64_t) == HWA_RXPOST_ACWI64_BYTES);
				type = MSGBUF_WI_ACWI64; size = sizeof(hwa_rxpost_acwi64_t);
			} else {
				ASSERT(sizeof(hwa_rxpost_acwi32_t) == HWA_RXPOST_ACWI32_BYTES);
				type = MSGBUF_WI_ACWI32; size = sizeof(hwa_rxpost_acwi32_t);
			}
			break;

		default:
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"RxPost format %u invalid\n", __FUNCTION__, rxpost_format));
			ASSERT(0);
			return BCME_BADARG;
	}

	max = H2DRING_RXPOST_MAX_ITEM >> misc;

	DHD_PCIE_IPC(("\t Protocol: H2D RxPost %u: "
		"format %u [type %u size %2u max %4u shift %u]\n",
		ringid, rxpost_format, type, size, max, misc));

	if (dhd_prot_ring_alloc(dhdp, &prot->h2dring_rxp_subn, "h2drxp", ringid,
	        type, size, max, misc) != BCME_OK) {
		DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
			"H2D RxPost ring allocation\n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}
	/* H2D Rx Post Common Ring Construction Completed */

	/** Ring  3: D2H Transmit Complete Common Ring Construction */
	ringid = BCMPCIE_D2H_MSGRING_TX_COMPLETE;
	prot->txcpln_fn = dhd_prot_process_txcpln; /* optimized consumer */

	misc = 0; /* default, single item per message, shift of 0 */

	switch (txcpln_format)
	{
		case MSGBUF_WI_LEGACY:
			ASSERT(sizeof(host_txbuf_cmpl_t) == D2HRING_TXCMPLT_ITEMSIZE);
			type = MSGBUF_WI_WI64; size = sizeof(host_txbuf_cmpl_t);
#if defined(DHD_LB)
			/* Generic Handler includes D2H DMA Sync and LoadBalancing */
			prot->txcpln_fn = dhd_prot_process_msgtype; /* generic consumer */
#endif /* DHD_LB_RXC */
			break;

		case MSGBUF_WI_COMPACT:
			ASSERT(sizeof(hwa_txcple_cwi_t) == HWA_TXCPLE_CWI_BYTES);
			type = MSGBUF_WI_CWI; size = sizeof(hwa_txcple_cwi_t);
			break;

		case MSGBUF_WI_AGGREGATE:
			ASSERT(sizeof(hwa_txcple_acwi_t) == HWA_TXCPLE_ACWI_BYTES);
			type = MSGBUF_WI_ACWI; size = sizeof(hwa_txcple_acwi_t);
			misc = 2; /* shift-2 for multiplaction/division by HWA_AGGR_MAX */
			break;

		default:
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"D2H TxCpln format %u invalid\n", __FUNCTION__, txcpln_format));
			ASSERT(0);
			return BCME_BADARG;
	}

	max = D2HRING_TXCMPLT_MAX_ITEM;

	DHD_PCIE_IPC(("\t Protocol: D2H TxCpln %u: "
		"format %u [type %u size %2u max %4u]\n",
		ringid, txcpln_format, type, size, max));

	if (dhd_prot_ring_alloc(dhdp, &prot->d2hring_tx_cpln, "d2htxcpl", ringid,
	        type, size, max, misc) != BCME_OK) {
		DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
			"D2H TxCpln ring allocation\n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}
	/* D2H Tx Completion Common Ring Construction Completed */

	/* Ring  4: D2H Rx Completion Common Ring Construction */
	ringid = BCMPCIE_D2H_MSGRING_RX_COMPLETE;
	prot->rxcpln_fn = dhd_prot_process_rxcpln; /* optimized consumer */
	prot->max_rxcpln_rings = max_rxcpln_rings;

	misc = 0; /* default, single item per message, shift of 0 */

	switch (rxcpln_format)
	{
		case MSGBUF_WI_LEGACY:
			ASSERT(sizeof(host_rxbuf_cmpl_t) == D2HRING_RXCMPLT_ITEMSIZE);
			type = MSGBUF_WI_WI64; size = sizeof(host_rxbuf_cmpl_t);
#if defined(DHD_LB)
			/* Generic Handler includes D2H DMA Sync and LoadBalancing */
			prot->rxcpln_fn = dhd_prot_process_msgtype; /* generic consumer */
#endif /* DHD_LB_RXC */
			break;

		case MSGBUF_WI_COMPACT:
			ASSERT(sizeof(hwa_rxcple_cwi_t) == HWA_RXCPLE_CWI_BYTES);
			type = MSGBUF_WI_CWI; size = sizeof(hwa_rxcple_cwi_t);
			break;

		case MSGBUF_WI_AGGREGATE:
			ASSERT(sizeof(hwa_rxcple_acwi_t) == HWA_RXCPLE_ACWI_BYTES);
			type = MSGBUF_WI_ACWI; size = sizeof(hwa_rxcple_acwi_t);
			misc = 2; /* shift-2 for multiplaction/division by HWA_AGGR_MAX */
			break;

		default:
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"RxCpln format %u invalid\n", __FUNCTION__, rxcpln_format));
			ASSERT(0);
			return BCME_BADARG;
	}

	max = D2HRING_RXCMPLT_MAX_ITEM;

	DHD_PCIE_IPC(("\t Protocol: H2D RxCpln %u: "
		"format %u [type %u size %2u max %4u offset %u]\n",
		ringid, rxcpln_format, type, size, max, misc));

	/* if needed allocate pool of msgbuf_ring_t objects for d2hring_rx_cpln */
	if (prot->d2hring_rx_cpln == NULL)
	{
		prot->d2hring_rx_cpln = (msgbuf_ring_t *)MALLOCZ(prot->osh,
			(max_rxcpln_rings * sizeof(msgbuf_ring_t)));
		if (prot->d2hring_rx_cpln == NULL) {
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"D2H RxCpln ring %u memory allocation\n",
				__FUNCTION__, max_rxcpln_rings));
			ASSERT(0);
			return BCME_NOMEM;
		}
	}

	for (i = 0; i < max_rxcpln_rings; i++)
	{
		if (dhd_prot_ring_alloc(dhdp, DHD_RING_IN_D2HRING_RX_CPLN(prot, i),
			"d2hrxcpl", (ringid + i), type, size, max, misc) != BCME_OK)
		{
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"D2H RxCpln ring %u allocation\n", __FUNCTION__, i));
			ASSERT(0);
			return BCME_NOMEM;
		}
	}

	/* D2H Rx Completion Common Ring Construction Completed */
	return BCME_OK;

} /* dhd_prot_preinit */

/**
 * dhd_prot_init - second stage of dhd_prot_attach. Now that the dongle has
 * completed it's initialization of the pcie_ipc structure, we may now fetch
 * the dongle advertized features and adjust the protocol layer accordingly.
 *
 * dhd_prot_init() may be invoked again after a dhd_prot_reset().
 */
int
dhd_prot_init(dhd_pub_t *dhd)
{
	haddr64_t base_addr;
	dhd_prot_t *prot = dhd->prot;
	uint32 idmacontrol;
	uint32 waitcount = 0;
	msgbuf_ring_t *rx_cpln;
	uint32 i;

	/* XXX: The section on pktid map initialization should be moved into
	 * dhd_prot_attach, and remove the de-initialization in dhd_prot_reset.
	 * There is no guarantee that the memory will not be fragmented on a
	 * soft reset of the dongle for a re-init of the pktid maps with lockers.
	 */
	/* PKTID handle INIT */
	if (prot->pktid_map_handle != NULL) {
		DHD_ERROR(("%s: pktid_map_handle already set!\n", __FUNCTION__));
		ASSERT(0);
		return BCME_ERROR;
	}

#ifdef IOCTLRESP_USE_CONSTMEM
	if (prot->pktid_map_handle_ioctl != NULL) {
		DHD_ERROR(("%s: pktid_map_handle_ioctl already set!\n", __FUNCTION__));
		ASSERT(0);
		return BCME_ERROR;
	}
#endif /* IOCTLRESP_USE_CONSTMEM */

	prot->pktid_map_handle = DHD_NATIVE_TO_PKTID_INIT(dhd, MAX_PKTID_ITEMS, PKTID_MAP_HANDLE);
	if (prot->pktid_map_handle == NULL) {
		DHD_ERROR(("%s: Unable to map packet id's\n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}

#ifdef IOCTLRESP_USE_CONSTMEM
	prot->pktid_map_handle_ioctl = DHD_NATIVE_TO_PKTID_INIT(dhd,
		DHD_FLOWRING_MAX_IOCTLRESPBUF_POST, PKTID_MAP_HANDLE_IOCTL);
	if (prot->pktid_map_handle_ioctl == NULL) {
		DHD_ERROR(("%s: Unable to map ioctl response buffers\n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}
#endif /* IOCTLRESP_USE_CONSTMEM */

	/* Read max rx packets supported by dongle */
	dhd_bus_cmn_readshared(dhd->bus, &prot->max_rxbufpost, MAX_RX_PKTS, 0);
	if (prot->max_rxbufpost == 0) {
		/* This would happen if the dongle firmware is not */
		/* using the latest shared structure template */
		prot->max_rxbufpost = DEFAULT_RX_BUFFERS_TO_POST;
	}
	DHD_INFO(("%s:%d: MAX_RXBUFPOST = %d\n", __FUNCTION__, __LINE__, prot->max_rxbufpost));

	/* Initialize.  bzero() would blow away the dma pointers. */
	prot->max_eventbufpost = DHD_FLOWRING_MAX_EVENTBUF_POST;
	prot->max_ioctlrespbufpost = DHD_FLOWRING_MAX_IOCTLRESPBUF_POST;

	prot->cur_ioctlresp_bufs_posted = 0;
	prot->data_seq_no = 0;
	prot->ioctl_seq_no = 0;
	prot->rxbufpost = 0;
	prot->cur_event_bufs_posted = 0;
	prot->active_tx_count = 0;

	prot->dmaxfer.src_dma_buf.va = NULL;
	prot->dmaxfer.dst_dma_buf.va = NULL;
	prot->dmaxfer.in_progress = FALSE;

	prot->tx_metadata_offset = 0;
	prot->txp_threshold = TXP_FLUSH_MAX_ITEMS_FLUSH_CNT;

	prot->ioctl_trans_id = 0;

	/* Register the interrupt function upfront */
	/* remove corerev checks in data path */
	prot->mb_ring_fn = dhd_bus_get_mbintr_fn(dhd->bus);

	prot->mb_2_ring_fn = dhd_bus_get_mbintr_2_fn(dhd->bus);
	prot->db1_2_ring_fn = dhd_bus_get_db1intr_2_fn(dhd->bus);

	/* Initialize Common MsgBuf Rings */
	dhd_prot_ring_init(dhd, &prot->h2dring_ctrl_subn);
	dhd_prot_ring_init(dhd, &prot->h2dring_rxp_subn);
	dhd_prot_ring_init(dhd, &prot->d2hring_ctrl_cpln);
	dhd_prot_ring_init(dhd, &prot->d2hring_tx_cpln);

	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, rx_cpln, i) {
		dhd_prot_ring_init(dhd, rx_cpln);
	}

#if defined(PCIE_D2H_SYNC)
	dhd_prot_d2h_sync_init(dhd);
#endif /* PCIE_D2H_SYNC */

	dhd_prot_h2d_sync_init(dhd);

	/* init the scratch buffer */
	if (prot->hme.bytes == 0U) { // Legacy host_mem_dma_buf scheme
		if (prot->host_mem_dma_buf.va == NULL) {
			if (dhd_prot_host_mem_alloc(dhd)) {
				return BCME_ERROR; // Fragmented host memory
			}
		}
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3) && defined(BCM47XX_CA9)
		/* Alias to NONACP region */
		PHYSADDRLOSET(prot->host_mem_dma_buf.pa,
			PHYSADDRLO(prot->host_mem_dma_buf.pa) + ACP_WIN_SIZE);
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
		/* Legacy host mem (aka scratch mem) region */
		dhd_base_addr_htolpa(&base_addr, prot->host_mem_dma_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, HOST_MEM_BUF_ADDR, 0);
		dhd_bus_cmn_writeshared(dhd->bus, &prot->host_mem_dma_buf.len, HOST_MEM_BUF_LEN, 0);
	} else {
		/*
		 * DHD Host Memory Extension service:
		 * + pcie_ipc::host_mem_len is total bytes of all HME user regions.
		 * + pcie_ipc::host_mem_haddr64 is address of table of user region's pa
		 */
		dhd_base_addr_htolpa(&base_addr,  prot->hme.user_haddr64_dma_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, HOST_MEM_BUF_ADDR, 0);
		dhd_bus_cmn_writeshared(dhd->bus, &prot->hme.bytes, HOST_MEM_BUF_LEN, 0);
	}

	/* If supported by the host, indicate the memory block
	 * for completion writes / submission reads to shared space
	 */
	if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
		dhd_base_addr_htolpa(&base_addr, prot->d2h_dma_indx_wr_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, D2H_DMA_INDX_WR_BUF, 0);
		dhd_base_addr_htolpa(&base_addr, prot->h2d_dma_indx_rd_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, H2D_DMA_INDX_RD_BUF, 0);
	}

	if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support) || IDMA_ENAB(dhd)) {
		dhd_base_addr_htolpa(&base_addr, prot->h2d_dma_indx_wr_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, H2D_DMA_INDX_WR_BUF, 0);
		dhd_base_addr_htolpa(&base_addr, prot->d2h_dma_indx_rd_buf.pa);
		dhd_bus_cmn_writeshared(dhd->bus, &base_addr, D2H_DMA_INDX_RD_BUF, 0);
	}

	/* Signal to the dongle that common ring init is complete */
	dhd_bus_hostready(dhd->bus);

	/*
	 * If the DMA-able buffers for flowring needs to come from a specific
	 * contiguous memory region, then setup prot->flowrings_dma_buf here.
	 * dhd_prot_flowrings_pool_alloc() will carve out DMA-able buffers from
	 * this contiguous memory region, for each of the flowrings.
	 */

	/* Pre-allocate pool of msgbuf_ring for flowrings */
	if (dhd_prot_flowrings_pool_alloc(dhd) != BCME_OK) {
		return BCME_ERROR;
	}

	/* If IDMA is enabled and initied, wait for FW to setup the IDMA descriptors
	 * Waiting just before configuring doorbell
	 */
#ifdef BCMQT
#define	IDMA_ENABLE_WAIT  100
#else
#define	IDMA_ENABLE_WAIT  5
#endif
	if (IDMA_ACTIVE(dhd)) {
		/* wait for idma_en bit in IDMAcontrol register to be set */
		/* Loop till idma_en is not set */
		idmacontrol = si_corereg(dhd->bus->sih, dhd->bus->sih->buscoreidx,
			IDMAControl, 0, 0);
		while (!(idmacontrol & PCIE_IDMA_MODE_EN) &&
			(waitcount++ < IDMA_ENABLE_WAIT)) {

			DHD_ERROR(("iDMA not enabled yet,waiting 1 ms c=%d IDMAControl = %08x\n",
				waitcount, idmacontrol));
#ifdef BCMQT
			OSL_DELAY(200000); /* 200msec for BCMQT  */
#else
			OSL_DELAY(1000); /* 1ms as its onetime only */
#endif
			idmacontrol = si_corereg(dhd->bus->sih, dhd->bus->sih->buscoreidx,
				IDMAControl, 0, 0);
		}

		if (waitcount < IDMA_ENABLE_WAIT) {
			DHD_ERROR(("iDMA enabled PCIEControl = %08x\n", idmacontrol));
		} else {
			DHD_ERROR(("Error: wait for iDMA timed out wait=%d IDMAControl = %08x\n",
				waitcount, idmacontrol));
			return BCME_ERROR;
		}
	}

	/* Host should configure soft doorbells if needed ... here */

	/* Post to dongle host configured soft doorbells */
	dhd_msgbuf_ring_config_d2h_soft_doorbell(dhd);

	/* Post buffers for packet reception and ioctl/event responses */
#if defined(BCM_DHD_RUNNER)
	dhd_runner_notify(dhd->runner_hlp, H2R_INIT_NOTIF, (uintptr)dhd->bus->dev, 0);

	if (DHD_RNR_OFFL_RXCMPL(dhd)) {
	    dhd_runner_notify(dhd->runner_hlp, H2R_RXPOST_NOTIF, 0, 0);
	} else
#endif  /* BCM_DHD_RUNNER */
	dhd_msgbuf_rxbuf_post(dhd, FALSE); /* alloc pkt ids */
	dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd);
	dhd_msgbuf_rxbuf_post_event_bufs(dhd);

#if defined(BCM_BUZZZ_STREAMING_BUILD)
	dhd_bcm_buzzz_init(dhd);
#endif /* BCM_BUZZZ_STREAMING_BUILD */

	dhd_csimon_init(dhd);

	return BCME_OK;
} /* dhd_prot_init */

/**
 * dhd_prot_detach - PCIE FD protocol layer destructor.
 * Unlink, frees allocated protocol memory (including dhd_prot)
 */
void
dhd_prot_detach(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *rx_cpln;
	uint32 i;

	/* Stop the protocol module */
	if (prot) {

		/* free up all DMA-able buffers allocated during prot attach/init */

#if defined(BCM_BUZZZ_STREAMING_BUILD)
		dhd_bcm_buzzz_detach(dhd);
#endif /* BCM_BUZZZ_STREAMING_BUILD */

		dhd_dma_buf_free(dhd, &prot->host_mem_dma_buf,   "host_mem");
		dhd_dma_buf_free(dhd, &prot->ioctl_resp_dma_buf, "ioctl_resp");
		dhd_dma_buf_free(dhd, &prot->ioctl_rqst_dma_buf, "ioctl_rqst");
		dhd_dma_buf_free(dhd, &prot->bus_tput_dma_buf,   "bus_tput");

		/* free all HME user dma_buf, including user pa table */
		dhd_prot_hme_reset(dhd);

#if defined(BCM_DHD_RUNNER)
		/* free up the buffers allocated for h2dring_rxp_subm */
		dhd_runner_notify(dhd->runner_hlp, H2R_RXPOST_FREE_NOTIF, 0, 0);

		/* Platform specific non-cached memory free */
		dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		                   (DETACH_DMA_INDX_BUF + H2D_DMA_INDX_WR_BUF),
		                   (uintptr)(&prot->h2d_dma_indx_wr_buf));
		dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		                   (DETACH_DMA_INDX_BUF + H2D_DMA_INDX_RD_BUF),
		                   (uintptr)(&prot->h2d_dma_indx_rd_buf));
		dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		                   (DETACH_DMA_INDX_BUF + D2H_DMA_INDX_WR_BUF),
		                   (uintptr)(&prot->d2h_dma_indx_wr_buf));
		dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		                   (DETACH_DMA_INDX_BUF + D2H_DMA_INDX_RD_BUF),
		                   (uintptr)(&prot->d2h_dma_indx_rd_buf));
#else /* !BCM_DHD_RUNNER */
		/* DMA-able buffers for DMAing H2D/D2H WR/RD indices */
		dhd_dma_buf_free(dhd, &prot->h2d_dma_indx_wr_buf, "h2d_wr");
		dhd_dma_buf_free(dhd, &prot->h2d_dma_indx_rd_buf, "h2d_rd");
		dhd_dma_buf_free(dhd, &prot->d2h_dma_indx_wr_buf, "d2h_wr");
		dhd_dma_buf_free(dhd, &prot->d2h_dma_indx_rd_buf, "d2h_rd");
#endif /* !BCM_DHD_RUNNER */

		/* Common MsgBuf Rings */
		dhd_prot_ring_free(dhd, &prot->h2dring_ctrl_subn);
		dhd_prot_ring_free(dhd, &prot->h2dring_rxp_subn);
		dhd_prot_ring_free(dhd, &prot->d2hring_ctrl_cpln);
		dhd_prot_ring_free(dhd, &prot->d2hring_tx_cpln);

		if (prot->d2hring_rx_cpln) {
			FOREACH_RING_IN_D2HRING_RX_CPLN(prot, rx_cpln, i) {
				dhd_prot_ring_free(dhd, rx_cpln);
			}

			MFREE(prot->osh, prot->d2hring_rx_cpln,
				(prot->max_rxcpln_rings * sizeof(msgbuf_ring_t)));

			prot->d2hring_rx_cpln = (msgbuf_ring_t*)NULL;
			prot->max_rxcpln_rings = 0;
		}

		/* Detach each DMA-able buffer and free the pool of msgbuf_ring_t */
		dhd_prot_flowrings_pool_free(dhd);

#if defined(BCM_DHD_RUNNER)
		dhd_runner_detach(dhd, dhd->runner_hlp);
		dhd->runner_hlp = NULL;
#endif /* BCM_DHD_RUNNER */

		if (prot->pktid_map_handle) {
			DHD_NATIVE_TO_PKTID_FINI(dhd, prot->pktid_map_handle);
		}

#ifndef CONFIG_DHD_USE_STATIC_BUF
		MFREE(dhd->osh, prot, sizeof(dhd_prot_t));
#endif /* CONFIG_DHD_USE_STATIC_BUF */

#if defined(DHD_LB)
#if defined(DHD_LB_TXC)
		if (prot->tx_compl_prod.buffer) {
			MFREE(dhd->osh, prot->tx_compl_prod.buffer,
				sizeof(void*) * DHD_LB_WORKQ_SZ);
		}
#endif /* DHD_LB_TXC */
#if defined(DHD_LB_RXC)
		if (prot->rx_compl_prod.buffer) {
			MFREE(dhd->osh, prot->rx_compl_prod.buffer,
				sizeof(void*) * DHD_LB_WORKQ_SZ);
		}
#endif /* DHD_LB_RXC */
#endif /* DHD_LB */

		dhd->prot = NULL;
	}
} /* dhd_prot_detach */

/**
 * dhd_prot_reset - Reset the protocol layer without freeing all objects. This
 * may be invoked to soft reboot the dongle, without having to detach and attach
 * the entire protocol layer.
 *
 * After dhd_prot_reset(), dhd_prot_init() may be invoked without going through
 * a dhd_prot_attach() phase.
 */
void
dhd_prot_reset(dhd_pub_t *dhd)
{
	struct dhd_prot *prot = dhd->prot;
	msgbuf_ring_t *rx_cpln;
	uint32 i;

	DHD_TRACE(("%s\n", __FUNCTION__));

	if (prot == NULL) {
		return;
	}

	dhd_prot_flowrings_pool_reset(dhd);

	dhd_prot_ring_reset(dhd, &prot->h2dring_ctrl_subn);
	dhd_prot_ring_reset(dhd, &prot->h2dring_rxp_subn);
	dhd_prot_ring_reset(dhd, &prot->d2hring_ctrl_cpln);
	dhd_prot_ring_reset(dhd, &prot->d2hring_tx_cpln);

	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, rx_cpln, i) {
		dhd_prot_ring_reset(dhd, rx_cpln);
	}

	dhd_prot_hme_reset(dhd);

	dhd_dma_buf_reset(dhd, &prot->ioctl_resp_dma_buf,  "ioctl_resp");
	dhd_dma_buf_reset(dhd, &prot->ioctl_rqst_dma_buf,  "ioctl_rqst");
	dhd_dma_buf_reset(dhd, &prot->host_mem_dma_buf,    "host_mem");
	dhd_dma_buf_reset(dhd, &prot->h2d_dma_indx_wr_buf, "h2d_wr");
	dhd_dma_buf_reset(dhd, &prot->h2d_dma_indx_rd_buf, "h2d_rd");
	dhd_dma_buf_reset(dhd, &prot->d2h_dma_indx_wr_buf, "d2h_wr");
	dhd_dma_buf_reset(dhd, &prot->d2h_dma_indx_rd_buf, "d2h_rd");

	dmaxfer_free_dmaaddr(dhd, &prot->dmaxfer);

	prot->tx_metadata_offset = 0;

	prot->rxbufpost = 0;
	prot->cur_event_bufs_posted = 0;
	prot->cur_ioctlresp_bufs_posted = 0;

	prot->active_tx_count = 0;
	prot->data_seq_no = 0;
	prot->ioctl_seq_no = 0;

	prot->ioctl_trans_id = 0;

	/* dhd_flow_rings_init is located at dhd_bus_start,
	 * so when stopping bus, flowrings shall be deleted
	 */
	if (dhd->flow_rings_inited) {
		dhd_flow_rings_deinit(dhd);
	}

	/* XXX: The section on pktid map initialization should be moved into
	 * dhd_prot_attach, and remove this de-initialization in dhd_prot_reset.
	 * There is no guarantee that the memory will not be fragmented on a
	 * soft reset of the dongle for a re-init of the pktid maps with lockers.
	 */
	if (prot->pktid_map_handle) {
		DHD_NATIVE_TO_PKTID_FINI(dhd, prot->pktid_map_handle);
		prot->pktid_map_handle = NULL;
	}

#ifdef IOCTLRESP_USE_CONSTMEM
	if (prot->pktid_map_handle_ioctl) {
		DHD_NATIVE_TO_PKTID_FINI_IOCTL(dhd, prot->pktid_map_handle_ioctl);
		prot->pktid_map_handle_ioctl = NULL;
	}
#endif /* IOCTLRESP_USE_CONSTMEM */

	dhd_csimon_fini(&prot->csimon);

} /* dhd_prot_reset */

/* Watchdog timer function */
void dhd_msgbuf_watchdog(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;

	DHD_LOCK(dhd);
	if ((prot->max_eventbufpost) && (prot->cur_event_bufs_posted == 0)) {
		dhd_msgbuf_rxbuf_post_event_bufs(dhd);
	}

	if ((prot->max_ioctlrespbufpost) && (prot->cur_ioctlresp_bufs_posted == 0)) {
		dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd);
	}

	/* re-post rxbuffer when intermittent running out of BPM buffer  */
	if (prot->rxpost_fail &&
#if defined(BCM_DHD_RUNNER)
		!DHD_RNR_OFFL_RXCMPL(dhd) &&
#endif
		TRUE) {
			dhd_msgbuf_rxbuf_post(dhd, FALSE);
	}
	DHD_UNLOCK(dhd);
}

void
dhd_prot_dma_rxoffset(dhd_pub_t *dhd, uint32 dma_rxoffset)
{
	dhd_prot_t *prot = dhd->prot;
	prot->dma_rxoffset = dma_rxoffset;
}

/**
 * Initialize protocol: sync w/dongle state.
 * Sets dongle media info (iswl, drv_version, mac address).
 */
int
dhd_sync_with_dongle(dhd_pub_t *dhd)
{
	int ret = 0;
	wlc_rev_info_t revinfo;

#ifndef OEM_ANDROID
	char buf[128];
#endif /* OEM_ANDROID */

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	dhd_os_set_ioctl_resp_timeout(IOCTL_RESP_TIMEOUT);

#ifndef OEM_ANDROID
	/* Get the device MAC address */
	memset(buf, 0, sizeof(buf));
	strncpy(buf, "cur_etheraddr", sizeof(buf) - 1);
	ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, buf, sizeof(buf), FALSE, 0);
	if (ret < 0) {
		DHD_ERROR(("%s: GET iovar cur_etheraddr FAILED\n", __FUNCTION__));
		goto done;
	}
	memcpy(dhd->mac.octet, buf, ETHER_ADDR_LEN);
	bcm_print_bytes("CUR_ETHERADDR ", buf, ETHER_ADDR_LEN);
#endif /* OEM_ANDROID */

#ifdef DHD_FW_COREDUMP
#ifdef CUSTOMER_HW4
	/* Check the memdump capability */
	dhd_get_memdump_info(dhd);
#elif defined(OEM_ANDROID) && defined(DHD_DEBUG)
	/* This is required only for Brix Android platform, after collecting memdump call BUGON */
	dhd->memdump_enabled = DUMP_MEMFILE_BUGON;
#else
	dhd->memdump_enabled = DUMP_MEMFILE;
#endif /* CUSTOMER_HW4 */
#endif /* DHD_FW_COREDUMP */

	/* Get the device rev info */
	memset(&revinfo, 0, sizeof(revinfo));
	ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_REVINFO, &revinfo, sizeof(revinfo), FALSE, 0);
	if (ret < 0) {
		DHD_ERROR(("%s: GET revinfo FAILED\n", __FUNCTION__));
		goto done;
	}
	DHD_ERROR(("%s: GET_REVINFO device 0x%x, vendor 0x%x, chipnum 0x%x, corerev %d\n",
		__FUNCTION__, revinfo.deviceid, revinfo.vendorid,
		revinfo.chipnum, revinfo.corerev));
	dhd_macdbg_upd_revinfo(dhd, &revinfo);

#ifdef OEM_ANDROID
	dhd_process_cid_mac(dhd, TRUE);
#endif /* OEM_ANDROID */

#if !defined(TARGETOS_nucleus)
	ret = dhd_preinit_ioctls(dhd);
#endif /* !defined(TARGETOS_nucleus) */

#ifdef OEM_ANDROID
	if (!ret) {
		dhd_process_cid_mac(dhd, FALSE);
	}
#endif /* OEM_ANDROID */

	/* Always assumes wl for now */
	dhd->iswl = TRUE;

done:
	return ret;
} /* dhd_sync_with_dongle */

#if defined(DHD_LB)

/* DHD load balancing: deferral of work to another online CPU */

/* DHD_LB_TXC DHD_LB_RXC DHD_LB_RXP dispatchers, in dhd_linux.c */
extern void dhd_lb_tx_compl_dispatch(dhd_pub_t *dhdp);
extern void dhd_lb_rx_compl_dispatch(dhd_pub_t *dhdp);
extern void dhd_lb_rx_napi_dispatch(dhd_pub_t *dhdp);

extern void dhd_lb_rx_pkt_enqueue(dhd_pub_t *dhdp, void *pkt, int ifidx);

/**
 * dhd_lb_dispatch - load balance by dispatch work to other CPU cores
 * Note: rx_compl_tasklet is dispatched explicitly.
 */
static INLINE void
dhd_lb_dispatch(dhd_pub_t *dhdp, uint16 ring_idx)
{
	switch (ring_idx) {

#if defined(DHD_LB_TXC)
		case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
			bcm_workq_prod_sync(&dhdp->prot->tx_compl_prod); /* flush WR index */
			dhd_lb_tx_compl_dispatch(dhdp); /* dispatch tx_compl_tasklet */
			break;
#endif /* DHD_LB_TXC */

		case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
		{
#if defined(DHD_LB_RXC)
			dhd_prot_t *prot = dhdp->prot;
			/* Schedule the takslet only if we have to */
			if (prot->rxbufpost <= (prot->max_rxbufpost - RXBUFPOST_THRESHOLD)) {
				/* flush WR index */
				bcm_workq_prod_sync(&dhdp->prot->rx_compl_prod);
				dhd_lb_rx_compl_dispatch(dhdp); /* dispatch rx_compl_tasklet */
			}
#endif /* DHD_LB_RXC */
#if defined(DHD_LB_RXP)
			dhd_lb_rx_napi_dispatch(dhdp); /* dispatch rx_process_napi */
#endif /* DHD_LB_RXP */
			break;
		}
		default:
			break;
	}
}

#if defined(DHD_LB_TXC)
/**
 * DHD load balanced tx completion tasklet handler, that will perform the
 * freeing of packets on the selected CPU. Packet pointers are delivered to
 * this tasklet via the tx complete workq.
 */
void
dhd_lb_tx_compl_handler(unsigned long data)
{
	int elem_ix;
	void *pkt, **elem;
	dmaaddr_t pa;
	uint32 pa_len;
	dhd_pub_t *dhd = (dhd_pub_t *)data;
	dhd_prot_t *prot = dhd->prot;
	bcm_workq_t *workq = &prot->tx_compl_cons;
	uint32 count = 0;

	DHD_LB_STATS_TXC_PERCPU_CNT_INCR(dhd);

	while (1) {
		elem_ix = bcm_ring_cons(WORKQ_RING(workq), DHD_LB_WORKQ_SZ);

		if (elem_ix == BCM_RING_EMPTY) {
			break;
		}

		elem = WORKQ_ELEMENT(void *, workq, elem_ix);
		pkt = *elem;

		DHD_INFO(("%s: tx_compl_cons pkt<%p>\n", __FUNCTION__, pkt));

		OSL_PREFETCH(PKTTAG(pkt));
		OSL_PREFETCH(pkt);

		pa = DHD_PKTTAG_PA((dhd_pkttag_fr_t *)PKTTAG(pkt));
		pa_len = DHD_PKTTAG_PA_LEN((dhd_pkttag_fr_t *)PKTTAG(pkt));

		DMA_UNMAP(dhd->osh, pa, pa_len, DMA_RX, 0, 0);

#if defined(BCMPCIE) && (defined(LINUX) || defined(OEM_ANDROID))
		dhd_txcomplete(dhd, pkt, true);
#endif /* BCMPCIE && (defined(LINUX) || defined(OEM_ANDROID)) */

		PKTFREE(dhd->osh, pkt, TRUE);
		count++;
	}

	/* smp_wmb(); */
	bcm_workq_cons_sync(workq);
	DHD_LB_STATS_UPDATE_TXC_HISTO(dhd, count);
}
#endif /* DHD_LB_TXC */

#if defined(DHD_LB_RXC)
void
dhd_lb_rx_compl_handler(unsigned long data)
{
	dhd_pub_t *dhd = (dhd_pub_t *)data;
	bcm_workq_t *workq = &dhd->prot->rx_compl_cons;

	DHD_LB_STATS_RXC_PERCPU_CNT_INCR(dhd);

	dhd_msgbuf_rxbuf_post(dhd, TRUE); /* re-use pktids */
	bcm_workq_cons_sync(workq);
}
#endif /* DHD_LB_RXC */

#endif /* DHD_LB */

static INLINE void BCMFASTPATH
dhd_prot_packet_free(dhd_pub_t *dhd, uint32 pktid, uint8 pkttype)
{
	void *PKTBUF;
	dmaaddr_t pa;
	uint32 len;
	void *dmah;
	void *secdma;

	PKTBUF = DHD_PKTID_TO_NATIVE(dhd, dhd->prot->pktid_map_handle, pktid,
		pa, len, dmah, secdma, pkttype);
	if (PKTBUF) {
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
		if (!PKTISFWDERBUF(dhd->osh, PKTBUF) && !DHD_PKT_GET_WFD_BUF(PKTBUF))
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
		{
#ifdef BCM_SECURE_DMA
			SECURE_DMA_UNMAP(dhd->osh, pa, (uint) len, DMA_TX, 0, dmah,
				secdma, 0);
#else
			DMA_UNMAP(dhd->osh, pa, (uint) len, DMA_TX, 0, dmah);
#endif
		}
#ifdef BCM_WFD
		if (DHD_PKT_GET_WFD_BUF(PKTBUF))
			DHD_PKT_CLR_WFD_BUF(PKTBUF); /* reset flag */
#endif /* BCM_WFD */
		PKTFREE(dhd->osh, PKTBUF, FALSE);
	}
}

static INLINE void * BCMFASTPATH
dhd_prot_packet_get(dhd_pub_t *dhd, uint32 pktid, uint8 pkttype, bool free_pktid)
{
	void *PKTBUF;
	dmaaddr_t pa;
	uint32 len;
	void *dmah;
	void *secdma;

#ifdef DHD_PCIE_PKTID
	if (free_pktid) {
		PKTBUF = DHD_PKTID_TO_NATIVE(dhd, dhd->prot->pktid_map_handle,
			pktid, pa, len, dmah, secdma, pkttype);
	} else {
		PKTBUF = DHD_PKTID_TO_NATIVE_RSV(dhd, dhd->prot->pktid_map_handle,
			pktid, pa, len, dmah, secdma, pkttype);
	}
#else
	PKTBUF = DHD_PKTID_TO_NATIVE(dhd, dhd->prot->pktid_map_handle, pktid, pa,
		len, dmah, secdma, pkttype);
#endif /* DHD_PCIE_PKTID */

	if (PKTBUF) {
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
		if (!PKTISFWDERBUF(dhd->osh, PKTBUF) && !DHD_PKT_GET_WFD_BUF(PKTBUF))
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
		{
#ifdef BCM_SECURE_DMA
			SECURE_DMA_UNMAP(dhd->osh, pa, (uint) len, DMA_RX, 0, dmah,
				secdma, 0);
#else
			DMA_UNMAP(dhd->osh, pa, (uint) len, DMA_RX, 0, dmah);
#endif
		}
#ifdef BCM_WFD
		if (DHD_PKT_GET_WFD_BUF(PKTBUF))
			DHD_PKT_CLR_WFD_BUF(PKTBUF); /* reset flag */
#endif
	}

	return PKTBUF;
}

#ifdef IOCTLRESP_USE_CONSTMEM
static INLINE void BCMFASTPATH
dhd_prot_ioctl_resp_dma_buf_get(dhd_pub_t *dhd, uint32 pktid,
                              dhd_dma_buf_t *ioctl_resp_dma_buf)
{
	memset(ioctl_resp_dma_buf, 0, sizeof(dhd_dma_buf_t));
	ioctl_resp_dma_buf->va =
		DHD_PKTID_TO_NATIVE(dhd, dhd->prot->pktid_map_handle_ioctl, pktid,
			ioctl_resp_dma_buf->pa, ioctl_resp_dma_buf->len,
			ioctl_resp_dma_buf->dmah, ioctl_resp_dma_buf->secdma,
			PKTTYPE_IOCTL_RX);

	return;
}
#endif /* IOCTLRESP_USE_CONSTMEM */

static INLINE void BCMFASTPATH
dhd_msgbuf_rxbuf_post_refresh(dhd_pub_t *dhd)
{
	msgbuf_ring_t *ring;
	ring = &dhd->prot->h2dring_rxp_subn;

	if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
		ring->rd = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_RD_UPD, ring->id);
	} else {
		dhd_bus_cmn_readshared(dhd->bus, &(ring->rd), RING_RD_UPD, ring->id);
	}
}

static void BCMFASTPATH
dhd_msgbuf_rxbuf_post(dhd_pub_t *dhd, bool use_rsv_pktid)
{
	dhd_prot_t *prot = dhd->prot;
	int16 budget_rxbufpost;
	int retcount;

#ifdef BCMHWA
	if (prot->rxpost_stop) {
		return;
	}
#endif

	/* Refresh the rxpost submission common ring's rd index */
	dhd_msgbuf_rxbuf_post_refresh(dhd);

	budget_rxbufpost = prot->max_rxbufpost - prot->rxbufpost;

	ASSERT(budget_rxbufpost >= (int16)0);

	while (budget_rxbufpost >= RX_BUF_BURST) {

		/* Post in a burst of RX_BUF_BURST number of buffers at a time */

		/* Callback could be legacy path with DHD_LB or ACWI */
		retcount = prot->rxpost_fn(dhd, use_rsv_pktid);

		if (retcount > 0) {
			prot->rxbufpost += (uint16)retcount;
#ifdef DHD_LB_RXC
			/* retcount is the number of buffers posted */
			DHD_LB_STATS_UPDATE_RXC_HISTO(dhd, retcount);
#endif /* DHD_LB_RXC */
			budget_rxbufpost = prot->max_rxbufpost - prot->rxbufpost;
		} else {
			/* Make sure we don't run loop any further */
			break;
		}
	}
}

/**
 * RxPost using the following helper to perform a staged alloc, len setting,
 * DMA mapping and PktId allocation.
 */
typedef struct dhd_rxp_pkt
{
	uint32        pktid;  /* pktid associated with pkt, for posting */
	void          *ptr;   /* ptr to pkt returned by PKTGET */
	dmaaddr_t     buf_pa; /* PHYSADDR of the pkt data buffer, for posting */
} dhd_rxp_pkt_t;

/* Common handler to post legacy, cwi32, cwi64, acwi32 or acwi64 formats */
static int BCMFASTPATH
dhd_prot_rxbuf_post(dhd_pub_t *dhd, bool use_rsv_pktid)
{
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *rxp_ring = &prot->h2dring_rxp_subn;

	void *rxp_wi_start; /* start address for space allocated in RxPost ring */
	uint16 avail_count, pkt_count;

	uint8 *pkt_buf_va; /* virtual address of data buffer, for posting */
	/* RX_BUF_BURST = 32. rxp_pktpool is 640-Byte on stack */
	dhd_rxp_pkt_t rxp_pktpool[RX_BUF_BURST]; /* pkt pool, for posting */

	uint32 pkt_tot_len = DHD_RX_PKT_BUFSZ; /* len of packet data buffer */
	uint32 extraheadroom = 0;
#if defined(BCM_NBUFF_PKT_BPM)
	bool use_bpm_pool;
	FkBuff_t * fkb = NULL;

#if defined(BCM_CPE_PKTC)
	use_bpm_pool = FALSE;
#else
	use_bpm_pool = TRUE;
#endif /* BCM_CPE_PKTC */
#endif /* BCM_NBUFF_PKT_BPM */

#ifdef BCM_ROUTER_DHD
	/* Reserve extra headroom for router builds */
	extraheadroom = BCMEXTRAHDROOM;
#endif /* BCM_ROUTER_DHD */

#ifdef WL_MONITOR
	if (dhd_monitor_enabled(dhd, 0)) {
		pkt_tot_len += sizeof(wl_radiotap_vht_t) + DHD_RX_PKT_BUFSZ;
		extraheadroom += sizeof(wl_radiotap_vht_t);
#if defined(BCM_NBUFF_PKT_BPM)
		use_bpm_pool = FALSE;
#endif /* BCM_NBUFF_PKT_BPM */
	}
#endif /* WL_MONITOR */

	{   /* Stage 1. Speculatively allocate space in ring */

		/* Items requested (taking into consideration Aggregation, if any) */
		/* Use divide-by-1 (item_misc = 0) or divide-by-4 (item_misc = 2) */
		int req_items = RX_BUF_BURST >> rxp_ring->item_misc;

		DHD_GENERAL_LOCK(dhd, flags);

		if (prot->inprogress_rxbufpost) {
			DHD_GENERAL_UNLOCK(dhd, flags);
			return -1;
		}

		/* Avoid re-entrancy: dhd_prot_alloc_ring_space is not re-entrant */
		prot->inprogress_rxbufpost = TRUE;

		rxp_wi_start = dhd_prot_alloc_ring_space(dhd, rxp_ring,
		                   req_items, &avail_count, TRUE);

		if (rxp_wi_start == NULL) {
			prot->inprogress_rxbufpost = FALSE; /* End re-entrancy */
			DHD_GENERAL_UNLOCK(dhd, flags);

			DHD_INFO(("%s: RxPost Ring space not available\n", __FUNCTION__));
			return -1;
		}

		DHD_GENERAL_UNLOCK(dhd, flags);

		/* Number of pkts that can fit into avail_count number of items */
		/* Use multiply-by-1 (item_misc = 0) or multiply-by-4 (item_misc = 2) */
		pkt_count = avail_count << rxp_ring->item_misc;

		/* audit number of work items available to store pkts */
		ASSERT((avail_count > 0) && (pkt_count <= RX_BUF_BURST));
	}

	{
		int i;
		dhd_rxp_pkt_t *curpkt;
		uint32 pkt_buf_len;
		for (i = 0, curpkt = &rxp_pktpool[0]; i < pkt_count; i++, curpkt++)
		{
			{
#if defined(BCM_NBUFF_PKT_BPM)
				if (use_bpm_pool) {
					void * databuf = dhd_databuf_alloc(dhd);

					if (databuf == NULL) {
						DHD_ERROR(("%s: dhd_databuf_alloc failed\n",
							__FUNCTION__));
						curpkt->ptr = NULL;
						dhd->rx_pktgetfail++;
						goto error_free_pktpool;
					}

					/* Convert BPM buffer to FkBuff */
					fkb = fkb_init(databuf, BCM_PKT_HEADROOM,
							databuf, pkt_tot_len);

#ifdef BCM_NBUFF_DHD_RECYCLE_HOOK
					fkb->recycle_hook = dhd_nbuff_recycle;
#else
					fkb->recycle_hook = gbpm_recycle_pNBuff;
#endif
					fkb->recycle_context = 0;
					curpkt->ptr = FKBUFF_2_PNBUFF(fkb);
				}
				else
#endif /* BCM_NBUFF_PKT_BPM */
				{
#if defined(BCM_NBUFF_PKT_BPM_SKB)
					curpkt->ptr = dhd_nbuff_bpm_skb_get(dhd, pkt_tot_len);
#else /* !BCM_NBUFF_PKT_BPM_SKB */
					/* 2KBytes packet total buffer len is inclusive
					 * of any headroom
					 */
					curpkt->ptr = PKTGET(dhd->osh, pkt_tot_len, FALSE);
#endif /* !BCM_NBUFF_PKT_BPM_SKB */
				}
			}

			if (curpkt->ptr == NULL) {
				DHD_ERROR(("%s: PKTGET failed\n", __FUNCTION__));
				dhd->rx_pktgetfail++;
				goto error_free_pktpool;
			}

			/* TODO: Use BPM buffer headromm */
			/* Align the data buffer with required headroom */
			PKTPULL(dhd->osh, curpkt->ptr, extraheadroom);

			/* Configure data buffer to be 8-byte aligned */
			pkt_buf_va = PKTDATA(dhd->osh, curpkt->ptr);
			if (!ISALIGNED((uintptr)pkt_buf_va, 8)) {
				uint32 headroom_align = ROUNDUP((uintptr)pkt_buf_va, 8)
					- (uintptr)pkt_buf_va; // additional headroom for 8B align
				PKTPULL(dhd->osh, curpkt->ptr, headroom_align);
				pkt_buf_va = PKTDATA(dhd->osh, curpkt->ptr);
			}
			pkt_buf_len = PKTLEN(dhd->osh, curpkt->ptr);

			/* Prepare the Rx packet's data buffer for DMA  */
#ifdef BCM_SECURE_DMA
			DHD_GENERAL_LOCK(dhd, flags);
			curpkt->buf_pa =
			SECURE_DMA_MAP(dhd->osh, pkt_buf_va, pkt_buf_len, DMA_RX,
				curpkt->ptr, 0, rxp_ring->dma_buf.secdma, 0,
				SECDMA_RXBUF_POST);
			DHD_GENERAL_UNLOCK(dhd, flags);
#else
#if defined(BCM_NBUFF_PKT_BPM)
			if (use_bpm_pool)
			{
				ULONGTOPHYSADDR(
					(unsigned long)VIRT_TO_PHYS(PKTDATA(dhd->osh, curpkt->ptr)),
					curpkt->buf_pa);

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
				fkb->dirty_p = _to_dptr_from_kptr_(
					(uint8*)PKTDATA(dhd->osh, curpkt->ptr) +
						BCM_DCACHE_LINE_LEN);
#endif /* CC_NBUFF_FLUSH_OPTIMIZATION */
			} else
#endif /* BCM_NBUFF_PKT_BPM */
			{
				curpkt->buf_pa =
					DMA_MAP(dhd->osh, pkt_buf_va, pkt_buf_len,
					DMA_RX, curpkt->ptr, 0);
			}
#endif /* BCM_SECURE_DMA */

			if (PHYSADDRISZERO(curpkt->buf_pa)) {
				DHD_ERROR(("Invalid phyaddr 0\n"));
				ASSERT(0);
				curpkt->pktid = DHD_PKTID_INVALID; /* in case of failures */
				goto error_free_pktpool;
			}

#ifdef DHD_PCIE_PKTID
			DHD_GENERAL_LOCK(dhd, flags);
#endif
			curpkt->pktid =
				DHD_NATIVE_TO_PKTID(dhd, prot->pktid_map_handle,
					curpkt->ptr, curpkt->buf_pa, pkt_buf_len,
					DMA_RX, NULL, rxp_ring->dma_buf.secdma, PKTTYPE_DATA_RX);
#ifdef DHD_PCIE_PKTID
			DHD_GENERAL_UNLOCK(dhd, flags);

			if (curpkt->pktid == DHD_PKTID_INVALID) {
				dhd->rx_pktidfail++;
				DHD_ERROR(("Rx PktId alloc failed\n")); /* resize pktid pool? */
				goto error_free_pktpool;
			}
#endif /* DHD_PCIE_PKTID */

#if defined(DHD_PKTID_AUDIT_RING)
			DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle,
				curpkt->pktid, DHD_DUPLICATE_ALLOC);
#endif /* DHD_PKTID_AUDIT_RING */

		} /* for pkt_count < RX_BUF_BURST */

		/* Allocated and prepared RX_BUF_BURST number of packets */
		ASSERT(i == pkt_count);

	}   /* Stage 2. Alloc and prepare RX_BUF_BURST number of pkts */

	{   /* Stage 3. Bulk post prepared packets using legacy/cwi/acwi formats */
		int i;
		dhd_rxp_pkt_t *curpkt;
		uint8 *rxp_wi = (uint8 *)rxp_wi_start;

		/* Post the prepared pkts from the pktpool into the RxPost Ring */
		for (i = 0, curpkt = &rxp_pktpool[0]; i < pkt_count; )
		{

			ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));

			/* Suport for various RxPost Work Item Formats */
			switch (rxp_ring->item_type)
			{

				case MSGBUF_WI_WI64: /* Legacy Work Item */
				{
					host_rxbuf_post_t *wi64 = (host_rxbuf_post_t *)rxp_wi;
					uint16 data_buf_len = PKTLEN(dhd->osh, curpkt->ptr);
					wi64->cmn_hdr.msg_type = MSG_TYPE_RXBUF_POST;
					wi64->cmn_hdr.if_id = 0;
					wi64->cmn_hdr.epoch = rxp_ring->seqnum++ % H2D_EPOCH_MODULO;
					wi64->cmn_hdr.request_id = htol32(curpkt->pktid);
					wi64->data_buf_len = htol16(data_buf_len);
					HADDR64_LO_SET_HTOL(wi64->data_buf_haddr64,
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(wi64->data_buf_haddr64,
						PHYSADDRHI(curpkt->buf_pa));
					// wi64->metadata_buf_len = 0;
					// HADDR64_LO_SET(wi64->metadata_buf_haddr64, 0);
					// HADDR64_HI_SET(wi64->metadata_buf_haddr64, 0);
					i += 1;
					curpkt++;
					break;
				}

				case MSGBUF_WI_CWI32: /* Compact Work Item with 32b haddr */
				{
					hwa_rxpost_cwi32_t *cwi32 = (hwa_rxpost_cwi32_t *)rxp_wi;
					cwi32->host_pktid = htol32(curpkt->pktid);
					cwi32->data_buf_haddr32 = /* low address only */
						htol32(PHYSADDRLO(curpkt->buf_pa));
					ASSERT(PHYSADDRHI(curpkt->buf_pa) == prot->host_physaddrhi);
					i += 1;
					curpkt++;
					break;
				}

				case MSGBUF_WI_CWI64: /* Compact Work Item with 64b haddr */
				{
					hwa_rxpost_cwi64_t *cwi64 = (hwa_rxpost_cwi64_t *)rxp_wi;
					cwi64->host_pktid = htol32(curpkt->pktid);
					HADDR64_LO_SET_HTOL(cwi64->data_buf_haddr64,
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(cwi64->data_buf_haddr64,
						PHYSADDRHI(curpkt->buf_pa));
					i += 1;
					curpkt++;
					break;
				}

				case MSGBUF_WI_ACWI32: /* Aggregated Compact with 32b haddr */
				{
					hwa_rxpost_acwi32_t *acwi32 = (hwa_rxpost_acwi32_t *)rxp_wi;

					/* unroll loop of 4 */
					acwi32->host_pktid[0] = htol32(curpkt->pktid);
					acwi32->data_buf_haddr32[0] =
						htol32(PHYSADDRLO(curpkt->buf_pa));
					ASSERT(PHYSADDRHI(curpkt->buf_pa) == prot->host_physaddrhi);

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi32->host_pktid[1] = htol32(curpkt->pktid);
					acwi32->data_buf_haddr32[1] =
						htol32(PHYSADDRLO(curpkt->buf_pa));
					ASSERT(PHYSADDRHI(curpkt->buf_pa) == prot->host_physaddrhi);

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi32->host_pktid[2] = htol32(curpkt->pktid);
					acwi32->data_buf_haddr32[2] =
						htol32(PHYSADDRLO(curpkt->buf_pa));
					ASSERT(PHYSADDRHI(curpkt->buf_pa) == prot->host_physaddrhi);

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi32->host_pktid[3] = htol32(curpkt->pktid);
					acwi32->data_buf_haddr32[3] =
						htol32(PHYSADDRLO(curpkt->buf_pa));
					ASSERT(PHYSADDRHI(curpkt->buf_pa) == prot->host_physaddrhi);

					i += 4;
					curpkt++;
					break;
				}

				case MSGBUF_WI_ACWI64: /* Aggregated Compact with 64b haddr */
				{
					hwa_rxpost_acwi64_t *acwi64 = (hwa_rxpost_acwi64_t *)rxp_wi;

					/* unroll loop of 4 */
					acwi64->host_pktid[0] = htol32(curpkt->pktid);
					HADDR64_LO_SET_HTOL(acwi64->data_buf_haddr64[0],
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(acwi64->data_buf_haddr64[0],
						PHYSADDRHI(curpkt->buf_pa));

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi64->host_pktid[1] = htol32(curpkt->pktid);
					HADDR64_LO_SET_HTOL(acwi64->data_buf_haddr64[1],
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(acwi64->data_buf_haddr64[1],
						PHYSADDRHI(curpkt->buf_pa));

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi64->host_pktid[2] = htol32(curpkt->pktid);
					HADDR64_LO_SET_HTOL(acwi64->data_buf_haddr64[2],
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(acwi64->data_buf_haddr64[2],
						PHYSADDRHI(curpkt->buf_pa));

					curpkt++;
					ASSERT(curpkt->ptr && (curpkt->pktid != DHD_PKTID_INVALID));
					acwi64->host_pktid[3] = htol32(curpkt->pktid);
					HADDR64_LO_SET_HTOL(acwi64->data_buf_haddr64[3],
						PHYSADDRLO(curpkt->buf_pa));
					HADDR64_HI_SET_HTOL(acwi64->data_buf_haddr64[3],
						PHYSADDRHI(curpkt->buf_pa));

					i += 4;
					curpkt++;
					break;
				}

				default: ASSERT(0); break;
			} /* switch item_type */

			rxp_wi = (uint8 *)rxp_wi + rxp_ring->item_size;

		}   /* for loop: i < pkt_count */

		ASSERT(i == pkt_count);

	}   /* Stage 3. Bulk post prepared packets using legacy/cwi/acwi formats */

	dhd_prot_ring_write_complete(dhd, rxp_ring, rxp_wi_start, avail_count);

	prot->inprogress_rxbufpost = FALSE;

	/* re-post rxbuffer when intermittent running out of BPM buffer  */
	prot->rxpost_fail = FALSE;

	return pkt_count;

error_free_pktpool:

	{    /* Stage Error: Restore ring's WR index without needing to flush */
		int i;
		dhd_rxp_pkt_t *curpkt;

		if (rxp_ring->wr == 0)
			rxp_ring->wr = rxp_ring->max_items - avail_count;
		else
			rxp_ring->wr -= avail_count;

		/* Release all packets */
		for (i = 0, curpkt = &rxp_pktpool[0]; i < pkt_count; i++, curpkt++)
		{
			if (curpkt->ptr == NULL)
				break;

			if (curpkt->pktid != DHD_PKTID_INVALID) {
				void *curpkt_ptr;
				DHD_GENERAL_LOCK(dhd, flags);
				curpkt_ptr = dhd_prot_packet_get(dhd, curpkt->pktid,
				                 PKTTYPE_DATA_RX, TRUE);
				DHD_GENERAL_UNLOCK(dhd, flags);
				ASSERT(curpkt_ptr == curpkt->ptr);
			}

			if (PHYSADDRISZERO(curpkt->buf_pa)) {
				uint32 pktlen = PKTLEN(dhd->osh, curpkt->ptr);
#ifdef BCM_SECURE_DMA
				DHD_GENERAL_LOCK(dhd, flags);
				SECURE_DMA_UNMAP(dhd->osh, curpkt->buf_pa, pktlen, DMA_RX,
					0, DHD_DMAH_NULL, rxp_ring->dma_buf.secdma, 0);
				DHD_GENERAL_UNLOCK(dhd, flags);
#else
				DMA_UNMAP(dhd->osh, curpkt->buf_pa, pktlen, DMA_RX,
					0, DHD_DMAH_NULL);
#endif /* BCM_SECURE_DMA */
			}

			PKTFREE(dhd->osh, curpkt->ptr, FALSE);
		}
	}

	prot->inprogress_rxbufpost = FALSE; /* End re-entrancy */

	/* re-post rxbuffer when intermittent running out of BPM buffer  */
	prot->rxpost_fail = TRUE;

	return -1;
} /* dhd_prot_rxbuf_post */

/** Post RX_BUF_BURST no of rx buffers to dongle */
#if defined(DHD_LB)
static int BCMFASTPATH
dhd_prot_rxbuf_post_lb(dhd_pub_t *dhd, bool use_rsv_pktid)
{
	void *p;
	uint16 count = RX_BUF_BURST;
	uint16 pktsz = DHD_RX_PKT_BUFSZ;
	uint8 *rxbuf_post_tmp;
	host_rxbuf_post_t *rxbuf_post;
	void *msg_start;
	dmaaddr_t pa;
	uint32 pktlen;
	uint8 i = 0;
	uint16 alloced = 0;
	unsigned long flags;
	uint32 pktid;
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring = &prot->h2dring_rxp_subn;

#ifdef BCM_SECURE_DMA
	if (!SECURE_DMA_RX_BUFFS_IS_AVAIL(dhd->osh)) {
			DHD_INFO(("%s:%d: SECDMA Buffers Not available \n",
				__FUNCTION__, __LINE__));
			return -1;
		}
#endif /* BCM_SECURE_DMA */

	DHD_GENERAL_LOCK(dhd, flags);

	/* Claim space for exactly 'count' no of messages, for mitigation purpose */
	msg_start = (void *)
		dhd_prot_alloc_ring_space(dhd, ring, count, &alloced, TRUE);

	DHD_GENERAL_UNLOCK(dhd, flags);

	if (msg_start == NULL) {
		DHD_INFO(("%s:%d: Rxbufpost Msgbuf Not available\n", __FUNCTION__, __LINE__));
		return -1;
	}
	/* if msg_start !=  NULL, we should have alloced space for atleast 1 item */
	ASSERT(alloced > 0);

	rxbuf_post_tmp = (uint8*)msg_start;
#ifdef WL_MONITOR
	if (dhd_monitor_enabled(dhd, 0)) {
		pktsz += sizeof(wl_radiotap_vht_t) + DHD_RX_PKT_BUFSZ;
	}
#endif /* WL_MONITOR */

	/* loop through each allocated message in the rxbuf post msgbuf_ring */
	for (i = 0; i < alloced; i++) {
		rxbuf_post = (host_rxbuf_post_t *)rxbuf_post_tmp;
		/* Create a rx buffer */

		if ((p = PKTGET(dhd->osh, pktsz, FALSE)) == NULL) {
			DHD_ERROR(("%s:%d: PKTGET for rxbuf failed\n", __FUNCTION__, __LINE__));
			dhd->rx_pktgetfail++;
			break;
		}

#ifdef BCM_ROUTER_DHD
		/* Reserve extra headroom for router builds */
		PKTPULL(dhd->osh, p, BCMEXTRAHDROOM);
#endif /* BCM_ROUTER_DHD */

		/* Configure data buffer to be 8-byte aligned */
		if (!ISALIGNED((uintptr)PKTDATA(dhd->osh, p), 8)) {
			uint32 headroom = ROUNDUP((uintptr)PKTDATA(dhd->osh, p), 8) -
				(uintptr)PKTDATA(dhd->osh, p);
			PKTPULL(dhd->osh, p, headroom);
		}

		pktlen = PKTLEN(dhd->osh, p);
#ifdef BCM_SECURE_DMA
		DHD_GENERAL_LOCK(dhd, flags);
		pa = SECURE_DMA_MAP(dhd->osh, PKTDATA(dhd->osh, p), pktlen,
			DMA_RX, p, 0, ring->dma_buf.secdma, 0, SECDMA_RXBUF_POST);
		DHD_GENERAL_UNLOCK(dhd, flags);
#else
		pa = DMA_MAP(dhd->osh, PKTDATA(dhd->osh, p), pktlen, DMA_RX, p, 0);
#endif /* BCM_SECURE_DMA */

		if (PHYSADDRISZERO(pa)) {
#ifndef BCM_SECURE_DMA
			DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, p, DHD_DMAH_NULL);
#endif /* BCM_SECURE_DMA */
			PKTFREE(dhd->osh, p, FALSE);
#ifndef BCM_SECURE_DMA
			DHD_ERROR(("Invalid phyaddr 0 \n"));
			ASSERT(0);
#else
			/* For SECDMA phyaddr zero is possible when SECDMA buffers are full */
			DHD_INFO(("%s:%d: SECDMA Buffers Not available \n",
				__FUNCTION__, __LINE__));
#endif /* BCM_SECURE_DMA */
			break;
		}

		/* Common msg header */
		rxbuf_post->cmn_hdr.msg_type = MSG_TYPE_RXBUF_POST;
		rxbuf_post->cmn_hdr.if_id = 0;
		rxbuf_post->cmn_hdr.epoch = ring->seqnum % H2D_EPOCH_MODULO;
		ring->seqnum++;

#if defined(DHD_LB_RXC)
		if (use_rsv_pktid == TRUE) {
			bcm_workq_t *workq = &prot->rx_compl_cons;
			int elem_ix = bcm_ring_cons(WORKQ_RING(workq), DHD_LB_WORKQ_SZ);
			if (elem_ix == BCM_RING_EMPTY) {
				DHD_ERROR(("%s rx_compl_cons ring is empty\n", __FUNCTION__));
				pktid = DHD_PKTID_INVALID;
				goto alloc_pkt_id;
			} else {
				uint32 *elem = WORKQ_ELEMENT(uint32, workq, elem_ix);
				pktid = *elem;
			}

			/* Now populate the previous locker with valid information */
			if (pktid != DHD_PKTID_INVALID) {
				rxbuf_post->cmn_hdr.request_id = htol32(pktid);
				DHD_NATIVE_TO_PKTID_SAVE(dhd, prot->pktid_map_handle, p, pktid,
					pa, pktlen, DMA_RX, NULL, ring->dma_buf.secdma,
					PKTTYPE_DATA_RX);
			}
		} else
#endif /* DHD_LB_RXC */
		{
#if defined(DHD_LB_RXC)
alloc_pkt_id:
#endif
#if defined(DHD_PCIE_PKTID)
		/* get the lock before calling DHD_NATIVE_TO_PKTID */
		DHD_GENERAL_LOCK(dhd, flags);
#endif
		pktid = DHD_NATIVE_TO_PKTID(dhd, prot->pktid_map_handle, p, pa,
			pktlen, DMA_RX, NULL, ring->dma_buf.secdma, PKTTYPE_DATA_RX);

#if defined(DHD_PCIE_PKTID)
		/* free lock */
		DHD_GENERAL_UNLOCK(dhd, flags);

		if (pktid == DHD_PKTID_INVALID) {
#ifdef BCM_SECURE_DMA
			DHD_GENERAL_LOCK(dhd, flags);
			SECURE_DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL,
				ring->dma_buf.secdma, 0);
			DHD_GENERAL_UNLOCK(dhd, flags);
#else
			DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL);
#endif /* BCM_SECURE_DMA */
			PKTFREE(dhd->osh, p, FALSE);
			DHD_ERROR(("%s:%d Pktid pool depleted.\n", __FUNCTION__, __LINE__));
			break;
		}
#endif /* DHD_PCIE_PKTID */
		}

		rxbuf_post->data_buf_len = htol16((uint16)pktlen);
		HADDR64_LO_SET_HTOL(rxbuf_post->data_buf_haddr64, PHYSADDRLO(pa));
		HADDR64_HI_SET_HTOL(rxbuf_post->data_buf_haddr64, PHYSADDRHI(pa));

		/* Check 8-byte alignment */
		ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));

		rxbuf_post->metadata_buf_len = 0;
		HADDR64_LO_SET(rxbuf_post->metadata_buf_haddr64, 0U); // htol32(0) = 0
		HADDR64_HI_SET(rxbuf_post->metadata_buf_haddr64, 0U); // htol32(0) = 0

#if defined(DHD_PKTID_AUDIT_RING)
		DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid, DHD_DUPLICATE_ALLOC);
#endif /* DHD_PKTID_AUDIT_RING */

		rxbuf_post->cmn_hdr.request_id = htol32(pktid);

		/* Move rxbuf_post_tmp to next item */
		rxbuf_post_tmp = rxbuf_post_tmp + ring->item_size;
	}

	if (i < alloced) {
		if (ring->wr < (alloced - i)) {
			ring->wr = ring->max_items - (alloced - i);
		} else {
			ring->wr -= (alloced - i);
		}

		alloced = i;
	}

	/* Update ring's WR index and ring doorbell to dongle */
	if (alloced > 0) {
		dhd_prot_ring_write_complete(dhd, ring, msg_start, alloced);
	}

	return alloced;
} /* dhd_prot_rxbuf_post_lb */
#endif /* DHD_LB_RXC */

#ifdef IOCTLRESP_USE_CONSTMEM
static int
alloc_ioctl_return_buffer(dhd_pub_t *dhd, dhd_dma_buf_t * ioctl_resp_dma_buf)
{
	memset(ioctl_resp_dma_buf, 0, sizeof(dhd_dma_buf_t));

	if (dhd_dma_buf_alloc(dhd, ioctl_resp_dma_buf, "ioctl_resp",
	                      IOCTL_DMA_BUF_LEN, DHD_MEM_ALIGN_BITS_MIN)) {
		DHD_ERROR(("%s: dhd_dma_buf_alloc\n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}

	return BCME_OK;
}

static void
free_ioctl_return_buffer(dhd_pub_t *dhd, dhd_dma_buf_t * ioctl_resp_dma_buf)
{
	if (ioctl_resp_dma_buf->va) {
		uint32 dma_pad;
		dma_pad = (IOCTL_DMA_BUF_LEN % DHD_DMA_PAD) ? DHD_DMA_PAD : 0;
		ioctl_resp_dma_buf->len = IOCTL_DMA_BUF_LEN;
		ioctl_resp_dma_buf->_alloced = ioctl_resp_dma_buf->len + dma_pad;
		ioctl_resp_dma_buf->_mem = (uintptr)ioctl_resp_dma_buf->va;
		/* JIRA:SWWLAN-70021 The pa value would be overwritten by the dongle.
		 * Need to reassign before free to pass the check in dhd_dma_buf_audit().
		 */
		ioctl_resp_dma_buf->pa =
			DMA_MAP(dhd->osh, ioctl_resp_dma_buf->va,
				ioctl_resp_dma_buf->len, DMA_RX, NULL, NULL);
	}

	dhd_dma_buf_free(dhd, ioctl_resp_dma_buf, "ioctl_resp");
}
#endif /* IOCTLRESP_USE_CONSTMEM */

static int
dhd_prot_rxbufpost_ctrl(dhd_pub_t *dhd, bool event_buf)
{
	void *p;
	uint16 pktsz;
	ioctl_resp_evt_buf_post_msg_t *rxbuf_post;
	dmaaddr_t pa;
	uint32 pktlen;
	dhd_prot_t *prot = dhd->prot;
	uint16 alloced = 0;
	unsigned long flags;
	dhd_dma_buf_t ioctl_resp_dma_buf;
	void *dmah = NULL;
	uint32 pktid;
	void *map_handle;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	if (dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: bus is already down.\n", __FUNCTION__));
		return -1;
	}

#ifdef BCM_SECURE_DMA
	if (!SECURE_DMA_RXCTL_BUFFS_IS_AVAIL(dhd->osh)) {
		DHD_INFO(("%s:%d: SECDMA Buffers Not available \n",
			__FUNCTION__, __LINE__));
		return -1;
	}
#endif /* BCM_SECURE_DMA */

	memset(&ioctl_resp_dma_buf, 0, sizeof(dhd_dma_buf_t));

	if (event_buf) {
		/* Allocate packet for event buffer post */
		pktsz = DHD_RX_EVENT_BUFSZ;
	} else {
		/* Allocate packet for ctrl/ioctl buffer post */
		pktsz = DHD_FLOWRING_IOCTL_BUFPOST_PKTSZ;
	}

#ifdef IOCTLRESP_USE_CONSTMEM
	if (!event_buf) {
		if (alloc_ioctl_return_buffer(dhd, &ioctl_resp_dma_buf) != BCME_OK) {
			DHD_ERROR(("Could not allocate IOCTL response buffer\n"));
			return -1;
		}
		ASSERT(ioctl_resp_dma_buf.len == IOCTL_DMA_BUF_LEN);
		p = ioctl_resp_dma_buf.va;
		pktlen = ioctl_resp_dma_buf.len;
		pa = ioctl_resp_dma_buf.pa;
		dmah = ioctl_resp_dma_buf.dmah;
	} else
#endif /* IOCTLRESP_USE_CONSTMEM */
	{
		if ((p = PKTGET(dhd->osh, pktsz, FALSE)) == NULL) {
			DHD_ERROR(("%s:%d: PKTGET for ctrl rxbuf failed\n",
				__FUNCTION__, __LINE__));
			dhd->rx_pktgetfail++;
			return -1;
		}

		pktlen = PKTLEN(dhd->osh, p);

#ifdef BCM_SECURE_DMA
			DHD_GENERAL_LOCK(dhd, flags);
			pa = SECURE_DMA_MAP(dhd->osh, PKTDATA(dhd->osh, p), pktlen,
				DMA_RX, p, 0, ring->dma_buf.secdma, 0, SECDMA_RXCTR_BUF_POST);
			DHD_GENERAL_UNLOCK(dhd, flags);
#else
			pa = DMA_MAP(dhd->osh, PKTDATA(dhd->osh, p), pktlen, DMA_RX, p, 0);
#endif /* BCM_SECURE_DMA */
		if (PHYSADDRISZERO(pa)) {
#ifndef BCM_SECURE_DMA
			DHD_ERROR(("Invalid phyaddr 0 \n"));
#endif /* BCM_SECURE_DMA */
			ASSERT(0);
			goto free_pkt_return;
		}
	}

	DHD_GENERAL_LOCK(dhd, flags);

	rxbuf_post = (ioctl_resp_evt_buf_post_msg_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);

	if (rxbuf_post == NULL) {
		DHD_GENERAL_UNLOCK(dhd, flags);
		DHD_ERROR(("%s:%d: Ctrl submit Msgbuf Not available to post buffer \n",
			__FUNCTION__, __LINE__));

#ifdef IOCTLRESP_USE_CONSTMEM
		if (event_buf)
#endif /* IOCTLRESP_USE_CONSTMEM */
		{
#ifdef BCM_SECURE_DMA
			DHD_GENERAL_LOCK(dhd, flags);
			SECURE_DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL,
				ring->dma_buf.secdma, 0);
			DHD_GENERAL_UNLOCK(dhd, flags);
#else
			DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL);
#endif
		}
		goto free_pkt_return;
	}

	/* CMN msg header */
	if (event_buf) {
		rxbuf_post->cmn_hdr.msg_type = MSG_TYPE_EVENT_BUF_POST;
	} else {
		rxbuf_post->cmn_hdr.msg_type = MSG_TYPE_IOCTLRESP_BUF_POST;
	}

#ifdef IOCTLRESP_USE_CONSTMEM
	if (!event_buf) {
		map_handle = prot->pktid_map_handle_ioctl;
		pktid =	DHD_NATIVE_TO_PKTID(dhd, map_handle, p, pa, pktlen,
			DMA_RX, dmah, ring->dma_buf.secdma, PKTTYPE_IOCTL_RX);
	} else
#endif /* IOCTLRESP_USE_CONSTMEM */
	{
		map_handle = prot->pktid_map_handle;
		pktid =	DHD_NATIVE_TO_PKTID(dhd, map_handle,
			p, pa, pktlen, DMA_RX, dmah, ring->dma_buf.secdma,
			event_buf ? PKTTYPE_EVENT_RX : PKTTYPE_IOCTL_RX);
	}

	if (pktid == DHD_PKTID_INVALID) {
		if (ring->wr == 0) {
			ring->wr = ring->max_items - 1;
		} else {
			ring->wr--;
		}
		DHD_GENERAL_UNLOCK(dhd, flags);
		DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL);
		goto free_pkt_return;
	}

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, map_handle, pktid, DHD_DUPLICATE_ALLOC);
#endif /* DHD_PKTID_AUDIT_RING */

	rxbuf_post->cmn_hdr.request_id = htol32(pktid);
	rxbuf_post->cmn_hdr.if_id = 0;
	rxbuf_post->cmn_hdr.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

#if defined(DHD_PCIE_PKTID)
	if (rxbuf_post->cmn_hdr.request_id == DHD_PKTID_INVALID) {
		if (ring->wr == 0) {
			ring->wr = ring->max_items - 1;
		} else {
			ring->wr--;
		}
		DHD_GENERAL_UNLOCK(dhd, flags);
#ifdef IOCTLRESP_USE_CONSTMEM
		if (event_buf)
#endif /* IOCTLRESP_USE_CONSTMEM */
		{
#ifdef BCM_SECURE_DMA
			DHD_GENERAL_LOCK(dhd, flags);
			SECURE_DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL,
				ring->dma_buf.secdma, 0);
			DHD_GENERAL_UNLOCK(dhd, flags);
#else
			DMA_UNMAP(dhd->osh, pa, pktlen, DMA_RX, 0, DHD_DMAH_NULL);
#endif /* #ifdef BCM_SECURE_DMA */
		}
		goto free_pkt_return;
	}
#endif /* DHD_PCIE_PKTID */

	rxbuf_post->cmn_hdr.flags = 0;
#ifndef IOCTLRESP_USE_CONSTMEM
	rxbuf_post->host_buf_len = htol16((uint16)PKTLEN(dhd->osh, p));
#else
	rxbuf_post->host_buf_len = htol16((uint16)pktlen);
#endif /* IOCTLRESP_USE_CONSTMEM */
	rxbuf_post->host_buf_addr.high_addr = htol32(PHYSADDRHI(pa));
	rxbuf_post->host_buf_addr.low_addr  = htol32(PHYSADDRLO(pa));

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, rxbuf_post, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return 1;

free_pkt_return:
#ifdef IOCTLRESP_USE_CONSTMEM
	if (!event_buf) {
		free_ioctl_return_buffer(dhd, &ioctl_resp_dma_buf);
	} else
#endif /* IOCTLRESP_USE_CONSTMEM */
	{
		PKTFREE(dhd->osh, p, FALSE);
	}

	return -1;
} /* dhd_prot_rxbufpost_ctrl */

static uint16
dhd_msgbuf_rxbuf_post_ctrlpath(dhd_pub_t *dhd, bool event_buf, uint32 max_to_post)
{
	uint32 i = 0;
	int32 ret_val;

	DHD_INFO(("max to post %d, event %d \n", max_to_post, event_buf));

	if (dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: bus is already down.\n", __FUNCTION__));
		return 0;
	}

	while (i < max_to_post) {
		ret_val  = dhd_prot_rxbufpost_ctrl(dhd, event_buf);
		if (ret_val < 0) {
			break;
		}
		i++;
	}

	DHD_CTL(("posted %d buffers to event_pool/ioctl_resp_pool %d\n", i, event_buf));
	return (uint16)i;
}

static void
dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	int max_to_post;

	DHD_INFO(("ioctl resp buf post\n"));
	max_to_post = prot->max_ioctlrespbufpost - prot->cur_ioctlresp_bufs_posted;
	if (max_to_post <= 0) {
		DHD_INFO(("%s: Cannot post more than max IOCTL resp buffers\n",
			__FUNCTION__));
		return;
	}
	prot->cur_ioctlresp_bufs_posted += dhd_msgbuf_rxbuf_post_ctrlpath(dhd,
		FALSE, max_to_post);
}

static void
dhd_msgbuf_rxbuf_post_event_bufs(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	int max_to_post;

	max_to_post = prot->max_eventbufpost - prot->cur_event_bufs_posted;
	if (max_to_post <= 0) {
		DHD_INFO(("%s: Cannot post more than max event buffers\n",
			__FUNCTION__));
		return;
	}
	prot->cur_event_bufs_posted += dhd_msgbuf_rxbuf_post_ctrlpath(dhd,
		TRUE, max_to_post);
}

/** called when DHD needs to check for 'receive complete' messages from the dongle */
bool BCMFASTPATH
dhd_prot_process_msgbuf_rxcpl(dhd_pub_t *dhd, uint bound)
{
	uint n = 0;
	uint32 i;
	bool more = TRUE;
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring;

	/* Process all the messages - DTOH direction */
	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
		while (!dhd_is_device_removed(dhd)) {
			uint8 *msg_addr;
			uint32 msg_len;

			if (dhd->hang_was_sent) {
				more = FALSE;
				break;
			}

			/* Get the address of the next message to be read from ring */
			msg_addr = dhd_prot_get_read_addr(dhd, ring, &msg_len);
			if (msg_addr == NULL) {
				more = FALSE;
				break;
			}

			/* Prefetch data to populate the cache */
			OSL_PREFETCH(msg_addr);

			if (prot->rxcpln_fn(dhd, ring, msg_addr, msg_len) != BCME_OK) {
				DHD_ERROR(("%s: process %s msg addr %p len %d\n",
					__FUNCTION__, ring->name, msg_addr, msg_len));
			}

			/* Update read pointer */
			dhd_prot_upd_read_idx(dhd, ring);

			/* After batch processing, check RX bound */
			n += msg_len / ring->item_size;
			if (n >= bound) {
				break;
			}
		}
	}

#if defined(BCM_PKTFWD)
	dhd_pktfwd_flush_pktqueues(dhd);
#endif /* BCM_PKTFWD */

	return more;
}

/**
 * Hands transmit packets (with a caller provided flow_id) over to dongle territory (the flow ring)
 */
void
dhd_prot_update_txflowring(dhd_pub_t *dhd, uint16 flowid, void *msgring)
{
	msgbuf_ring_t *ring = (msgbuf_ring_t *)msgring;

	/* Update read pointer */
	if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
		unsigned long flags;

		DHD_GENERAL_LOCK(dhd, flags);

		ring->rd = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_RD_UPD, ring->id);

		DHD_GENERAL_UNLOCK(dhd, flags);
	}

	DHD_TRACE(("ringid %d flowid %d write %d read %d \n\n",
		ring->id, flowid, ring->wr, ring->rd));

	/* Need more logic here, but for now use it directly */
	dhd_bus_schedule_queue(dhd->bus, flowid, TRUE); /* from queue to flowring */
}

/** called when DHD needs to check for 'transmit complete' messages from the dongle */
bool BCMFASTPATH
dhd_prot_process_msgbuf_txcpl(dhd_pub_t *dhd, uint bound)
{
	uint n = 0;
	bool more = TRUE;
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring = &prot->d2hring_tx_cpln;

	/* Process all the messages - DTOH direction */
	while (!dhd_is_device_removed(dhd)) {
		uint8 *msg_addr;
		uint32 msg_len;

		if (dhd->hang_was_sent) {
			more = FALSE;
			break;
		}

		/* Get the address of the next message to be read from ring */
		msg_addr = dhd_prot_get_read_addr(dhd, ring, &msg_len);
		if (msg_addr == NULL) {
			more = FALSE;
			break;
		}

		/* Prefetch data to populate the cache */
		OSL_PREFETCH(msg_addr);

		if (prot->txcpln_fn(dhd, ring, msg_addr, msg_len) != BCME_OK) {
			DHD_ERROR(("%s: process %s msg addr %p len %d\n",
				__FUNCTION__, ring->name, msg_addr, msg_len));
		}

		/* Write to dngl rd ptr */
		dhd_prot_upd_read_idx(dhd, ring);

		/* After batch processing, check bound */
		n += msg_len / ring->item_size;
		if (n >= bound) {
			break;
		}
	}

	return more;
}

/** called when DHD needs to check for 'ioctl complete' messages from the dongle */
int BCMFASTPATH
dhd_prot_process_ctrlbuf(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring = &prot->d2hring_ctrl_cpln;

	/* Process all the messages - DTOH direction */
	while (!dhd_is_device_removed(dhd)) {
		uint8 *msg_addr;
		uint32 msg_len;

		if (dhd->hang_was_sent) {
			break;
		}

		/* Get the address of the next message to be read from ring */
		msg_addr = dhd_prot_get_read_addr(dhd, ring, &msg_len);
		if (msg_addr == NULL) {
			break;
		}

		/* Prefetch data to populate the cache */
		OSL_PREFETCH(msg_addr);
		if (dhd_prot_process_msgtype(dhd, ring, msg_addr, msg_len) != BCME_OK) {
			DHD_ERROR(("%s: process %s msg addr %p len %d\n",
				__FUNCTION__, ring->name, msg_addr, msg_len));
		}

		/* Write to dngl rd ptr */
		dhd_prot_upd_read_idx(dhd, ring);
	}

	return 0;
}

/**
 * Consume messages out of the D2H ring. Ensure that the message's DMA to host
 * memory has completed, before invoking the message handler via a table lookup
 * of the cmn_msg_hdr::msg_type.
 */
static int BCMFASTPATH
dhd_prot_process_msgtype(dhd_pub_t *dhd, msgbuf_ring_t *ring, uint8 *buf, uint32 len)
{
	int buf_len = len;
	uint16 item_len;
	uint8 msg_type;
	cmn_msg_hdr_t *msg = NULL;
	int ret = BCME_OK;

	ASSERT(ring);
	item_len = ring->item_size;
	if (item_len == 0) {
		DHD_ERROR(("%s: ringidx %d item_len %d buf_len %d\n",
			__FUNCTION__, ring->id, item_len, buf_len));
		return BCME_ERROR;
	}

	while (buf_len > 0) {
		if (dhd->hang_was_sent) {
			ret = BCME_ERROR;
			goto done;
		}

		msg = (cmn_msg_hdr_t *)buf;

#if defined(PCIE_D2H_SYNC)
		/* Wait until DMA completes, then fetch msg_type */
		msg_type = dhd->prot->d2h_sync_cb(dhd, ring, msg, item_len);
#else
		msg_type = msg->msg_type;
#endif /* !PCIE_D2H_SYNC */

		/* Prefetch data to populate the cache */
		OSL_PREFETCH(buf + item_len);

		DHD_INFO(("msg_type %d item_len %d buf_len %d\n",
			msg_type, item_len, buf_len));

		if (msg_type == MSG_TYPE_LOOPBACK) {
			bcm_print_bytes("LPBK RESP: ", (uint8 *)msg, item_len);
			DHD_ERROR((" MSG_TYPE_LOOPBACK, len %d\n", item_len));
		}

		ASSERT(msg_type < DHD_PROT_FUNCS);
		if (msg_type >= DHD_PROT_FUNCS) {
			DHD_ERROR(("%s: msg_type %d item_len %d buf_len %d\n",
				__FUNCTION__, msg_type, item_len, buf_len));
			ret = BCME_ERROR;
			goto done;
		}

		if (table_lookup[msg_type]) {
			table_lookup[msg_type](dhd, buf);
		}

		if (buf_len < item_len) {
			ret = BCME_ERROR;
			goto done;
		}
		buf_len = buf_len - item_len;
		buf = buf + item_len;
	}

done:

#if defined(DHD_RX_CHAINING) || (defined(BCM_AWL) && defined(DHD_AWL_RX))
	dhd_rxchain_commit(dhd);
#endif /* DHD_RX_CHAINING || (BCM_AWL && DHD_AWL_RX) */

#if defined(DHD_LB)
	dhd_lb_dispatch(dhd, ring->id);
#endif

#if defined(BCM_PKTFWD)
	dhd_pktfwd_flush_pktqueues(dhd);
#endif /* BCM_PKTFWD */

	return ret;
} /* dhd_prot_process_msgtype */

static void
dhd_prot_noop(dhd_pub_t *dhd, void *msg)
{
}

/** called on MSG_TYPE_RING_STATUS message received from dongle */
static void
dhd_prot_ringstatus_process(dhd_pub_t *dhd, void *msg)
{
	pcie_ring_status_t *ring_status = (pcie_ring_status_t *)msg;
	DHD_ERROR(("ring status: request_id %d, status 0x%04x, flow ring %d, write_idx %d \n",
		ring_status->cmn_hdr.request_id, ring_status->compl_hdr.status,
		ring_status->compl_hdr.flow_ring_id, ring_status->write_idx));

	/* How do we track this to pair it with ??? */
}

/** called on MSG_TYPE_GEN_STATUS ('general status') message received from dongle */
static void
dhd_prot_genstatus_process(dhd_pub_t *dhd, void *msg)
{
	pcie_gen_status_t *gen_status = (pcie_gen_status_t *)msg;

	DHD_ERROR(("ERROR: gen status: request_id %d, STATUS 0x%04x, flow ring %d \n",
		gen_status->cmn_hdr.request_id, gen_status->compl_hdr.status,
		gen_status->compl_hdr.flow_ring_id));

	/* How do we track this to pair it with ??? */
	if (gen_status->cmn_hdr.msg_type == MSG_TYPE_GEN_STATUS) {

		switch (ltoh16(gen_status->compl_hdr.status)) {
			case BCMPCIE_NO_EVENT_BUF:
				dhd_msgbuf_rxbuf_post_event_bufs(dhd);
				break;

			case BCMPCIE_NO_IOCTLRESP_BUF:
				dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd);
				break;

			default:
				break;
		}

	}
}

/**
 * Called on MSG_TYPE_IOCTLPTR_REQ_ACK ('ioctl ack') message received from dongle, meaning that the
 * dongle received the ioctl message in dongle memory.
 */
static void
dhd_prot_ioctack_process(dhd_pub_t *dhd, void *msg)
{
	uint32 pktid;
	ioctl_req_ack_msg_t *ioct_ack = (ioctl_req_ack_msg_t *)msg;
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;

	pktid = ltoh32(ioct_ack->cmn_hdr.request_id);

#if defined(DHD_PKTID_AUDIT_RING)
	/* Skip DHD_IOCTL_REQ_PKTID = 0xFFFE */
	if (pktid != DHD_IOCTL_REQ_PKTID) {
		DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid,
			DHD_TEST_IS_ALLOC);
	}
#endif /* DHD_PKTID_AUDIT_RING */

	DHD_GENERAL_LOCK(dhd, flags);
	if ((prot->ioctl_state & MSGBUF_IOCTL_ACK_PENDING) &&
		(prot->ioctl_state & MSGBUF_IOCTL_RESP_PENDING)) {
		prot->ioctl_state &= ~MSGBUF_IOCTL_ACK_PENDING;
	} else {
		DHD_ERROR(("%s: received ioctl ACK with state %02x\n",
			__FUNCTION__, prot->ioctl_state));
	}
	DHD_GENERAL_UNLOCK(dhd, flags);

	DHD_CTL(("ioctl req ack: request_id %d, status 0x%04x, flow ring %d \n",
		ioct_ack->cmn_hdr.request_id, ioct_ack->compl_hdr.status,
		ioct_ack->compl_hdr.flow_ring_id));
	if (ioct_ack->compl_hdr.status != 0)  {
		DHD_ERROR(("got an error status for the ioctl request...need to handle that\n"));
	}
}

/** called on MSG_TYPE_IOCTL_CMPLT message received from dongle */
static void
dhd_prot_ioctcmplt_process(dhd_pub_t *dhd, void *msg)
{
	dhd_prot_t *prot = dhd->prot;
	uint32 pkt_id, xt_id;
	ioctl_comp_resp_msg_t *ioct_resp = (ioctl_comp_resp_msg_t *)msg;
	void *pkt;
	unsigned long flags;
	dhd_dma_buf_t ioctl_resp_dma_buf;

	memset(&ioctl_resp_dma_buf, 0, sizeof(dhd_dma_buf_t));

	pkt_id = ltoh32(ioct_resp->cmn_hdr.request_id);

#if defined(DHD_PKTID_AUDIT_RING)
#ifndef IOCTLRESP_USE_CONSTMEM
	DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pkt_id, DHD_DUPLICATE_FREE);
#else
	DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle_ioctl, pkt_id, DHD_DUPLICATE_FREE);
#endif /* !IOCTLRESP_USE_CONSTMEM */
#endif /* DHD_PKTID_AUDIT_RING */

	DHD_GENERAL_LOCK(dhd, flags);
#ifndef IOCTLRESP_USE_CONSTMEM
	pkt = dhd_prot_packet_get(dhd, pkt_id, PKTTYPE_IOCTL_RX, TRUE);
#else
	dhd_prot_ioctl_resp_dma_buf_get(dhd, pkt_id, &ioctl_resp_dma_buf);
	pkt = ioctl_resp_dma_buf.va;
#endif /* !IOCTLRESP_USE_CONSTMEM */
	if (!pkt) {
		prot->ioctl_state = 0;
		DHD_GENERAL_UNLOCK(dhd, flags);
		DHD_ERROR(("%s: received ioctl response with NULL pkt\n", __FUNCTION__));
		return;
	}
	if ((prot->ioctl_state & MSGBUF_IOCTL_ACK_PENDING) ||
		!(prot->ioctl_state & MSGBUF_IOCTL_RESP_PENDING)) {
		/* reset ioctl state */
		prot->ioctl_state = 0;
		DHD_GENERAL_UNLOCK(dhd, flags);
		DHD_ERROR(("%s: received ioctl response with state %02x\n",
			__FUNCTION__, prot->ioctl_state));
		goto exit;
	}
	DHD_GENERAL_UNLOCK(dhd, flags);

	prot->ioctl_resp_len = ltoh16(ioct_resp->resp_len);
	prot->ioctl_status = ltoh16(ioct_resp->compl_hdr.status);
	xt_id = ltoh16(ioct_resp->trans_id);
	if (xt_id != prot->ioctl_trans_id) {
		DHD_ERROR(("%s: cmd 0x%x pkt_id 0x%x xt_id %d trans_id %d "
			"status 0x%x resp_len %d\n",
			__FUNCTION__, ltoh32(ioct_resp->cmd), pkt_id, xt_id, prot->ioctl_trans_id,
			prot->ioctl_status, prot->ioctl_resp_len));

		ASSERT(0);
		goto exit;
	}

	DHD_CTL(("%s: cmd 0x%x pkt_id 0x%x xt_id %d trans_id %d status 0x%x resp_len %d\n",
		__FUNCTION__, ltoh32(ioct_resp->cmd), pkt_id, xt_id, prot->ioctl_trans_id,
		prot->ioctl_status, prot->ioctl_resp_len));

	if (prot->ioctl_resp_len > 0) {
#ifndef IOCTLRESP_USE_CONSTMEM
		bcopy(PKTDATA(dhd->osh, pkt),
		      prot->ioctl_resp_dma_buf.va, prot->ioctl_resp_len);
#else
		bcopy(pkt, prot->ioctl_resp_dma_buf.va, prot->ioctl_resp_len);
#endif /* !IOCTLRESP_USE_CONSTMEM */
	}
	prot->ioctl_received = 1;

	/* wake up any dhd_os_ioctl_resp_wait() */
	dhd_os_ioctl_resp_wake(dhd);

exit:
#ifndef IOCTLRESP_USE_CONSTMEM
	PKTFREE(dhd->osh, pkt, FALSE);
#else
	free_ioctl_return_buffer(dhd, &ioctl_resp_dma_buf);
#endif /* !IOCTLRESP_USE_CONSTMEM */
}

/**
 * Consumer handler for TxCompletion Ring.
 * Does not have support for D2H DMA Sync nor Load-balancing support.
 * dhd_prot_process_msgbuf_txcpl() will invoke the txcpln_fn hook. All messages
 * in the TxCpln ring will be processed here, given the address of the first
 * message and the total length of all messages.
 */
static int BCMFASTPATH
dhd_prot_process_txcpln(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	uint8 *buf, uint32 buf_len)
{
	dhd_prot_t *prot;
	unsigned long flags;
	uint32 pktid, pkts_to_free_count;
	uint16 item_size;
	void *pkt, *pkts_to_free_llist;

	const uint8 * buf_end = buf + buf_len;

	/* Stage 0. Preparation stage */
	// OSL_PREFETCH(buf) would be invoked in caller

	pkts_to_free_count = 0U;
	pkts_to_free_llist = NULL;
	item_size = ring->item_size >> ring->item_misc;

	prot = dhd->prot;

	DHD_GENERAL_LOCK(dhd, flags);

	while (buf < buf_end) /* Process all TxCompletion workitems */
	{
		/* Stage 1. Fetch and audit the pktid from current workitem */
		switch (ring->item_type) {
			case MSGBUF_WI_WI64: {
				host_txbuf_cmpl_t *wi = (host_txbuf_cmpl_t *) buf;
				pktid = ltoh32(wi->cmn_hdr.request_id);
				break;
			}
			case MSGBUF_WI_CWI: {
				hwa_txcple_cwi_t *cwi = (hwa_txcple_cwi_t *) buf;
				pktid = ltoh32(cwi->host_pktid);
				break;
			}
			case MSGBUF_WI_ACWI: {
				hwa_txcple_cwi_t *cwi = (hwa_txcple_cwi_t *) buf;
				pktid = ltoh32(cwi->host_pktid);
				if (pktid == HWA_HOST_PKTID_NULL) {
					/* Shift to next aggregate workitem */
					buf = ALIGN_ADDR(buf, ring->item_size);
					OSL_PREFETCH(buf);
					continue;
				}
				break;
			}
			default:
				pktid = 0U;
				ASSERT(0);
		} /* switch item_type */

		DHD_INFO(("%s pktid 0x%08x\n", __FUNCTION__, pktid));
		ASSERT(pktid != 0);

#if defined(DHD_PKTID_AUDIT_RING)
		DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid, DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

		/* Prepare to process next work item */
		buf += item_size;

		/* speculatively prefetch next message (without checking for buf_end) */
		OSL_PREFETCH(buf);

		{   /* Stage 2. Handle the TxCompletion for current pktid */
			dmaaddr_t pa;
			uint32 len;
			void *dmah, *secdma;
			if (pktid == 0xdeadbeaf) {
				DHD_ERROR(("%s pktid 0x%08x\n", __FUNCTION__, pktid));
				pkt = NULL;
			} else {
				pkt = DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle, pktid,
					pa, len, dmah, secdma, PKTTYPE_DATA_TX);
			}

			if (pkt) {
				OSL_PREFETCH(pkt);
#ifdef BCM_WFD
				if (!DHD_PKT_GET_WFD_BUF(pkt)) {
#endif
#ifdef BCM_SECURE_DMA
					SECURE_DMA_UNMAP(dhd->osh, pa,
						0, DMA_RX, 0, dmah, secdma, 0);
#else
					DMA_UNMAP(dhd->osh, pa, (uint)len, DMA_RX, 0, dmah);
#endif
#ifdef BCM_WFD
				}
#endif

#if defined(BCMPCIE) && (defined(LINUX) || defined(OEM_ANDROID))
				dhd_txcomplete(dhd, pkt, true); /* pass up to upper OS layer */
#endif
				/* Upper DHD OS layer may not touch this packet, now */

				/* Link this packet for freeing */
				pkts_to_free_count++;
				PKTSETLINK(pkt, pkts_to_free_llist);
				pkts_to_free_llist = pkt;

			} else { /* BAD: pkt == NULL */
				DHD_ERROR(("%s pktid<%u> mapped to NULL pkt\n",
				           __FUNCTION__, pktid));
			}
		}

	} /* while more TxCompletions to process */

	OSL_PREFETCH(pkts_to_free_llist);

	/* Stage 3. Perform protocol layer accounting */
	if (prot->active_tx_count >= pkts_to_free_count) {
		prot->active_tx_count -= pkts_to_free_count;
	} else {
		DHD_ERROR(("%s: %u extra TxCompletions received\n",
			__FUNCTION__, pkts_to_free_count - prot->active_tx_count));
		prot->active_tx_count = 0;
	}

	DHD_GENERAL_UNLOCK(dhd, flags);

	/* Stage 4. Free all accumulated packets */

	pkt = pkts_to_free_llist;

#ifdef BCM_BLOG
	DHD_UNLOCK(dhd);
#endif

	while (pkt != NULL)
	{
		pkts_to_free_llist = PKTLINK(pkt);
		OSL_PREFETCH(pkts_to_free_llist);

		PKTSETLINK(pkt, NULL); /* redundant */
		PKTFREE(dhd->osh, pkt, TRUE);

		pkt = pkts_to_free_llist;
#ifdef BCMDBG
		pkts_to_free_count--;
#endif
	}

#ifdef BCM_BLOG
	DHD_LOCK(dhd);
#endif

#ifdef BCMDBG
	ASSERT(pkts_to_free_count == 0U);
#endif

	return BCME_OK;
} /* dhd_prot_process_txcpln */

/** called on MSG_TYPE_TX_STATUS message received from dongle */
static void BCMFASTPATH
dhd_prot_txstatus_process(dhd_pub_t *dhd, void *msg)
{
	dhd_prot_t *prot = dhd->prot;
	host_txbuf_cmpl_t * txstatus;
	unsigned long flags;
	uint32 pktid;
	void *pkt = NULL;
	dmaaddr_t pa;
	uint32 len;
	void *dmah;
	void *secdma;

	/* locks required to protect circular buffer accesses */
	DHD_GENERAL_LOCK(dhd, flags);

	txstatus = (host_txbuf_cmpl_t *)msg;
	pktid = ltoh32(txstatus->cmn_hdr.request_id);

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid, DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

	DHD_INFO(("txstatus for pktid 0x%04x\n", pktid));
	if (prot->active_tx_count) {
		prot->active_tx_count--;
	} else {
		DHD_ERROR(("Extra packets are freed\n"));
	}

	ASSERT(pktid != 0);

#if defined(DHD_LB_TXC) && !defined(BCM_SECURE_DMA)
	{
		int elem_ix;
		void **elem;
		bcm_workq_t *workq;

		pkt = DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle,
			pktid, pa, len, dmah, secdma, PKTTYPE_DATA_TX);

		workq = &prot->tx_compl_prod;
		/*
		 * Produce the packet into the tx_compl workq for the tx compl tasklet
		 * to consume.
		 */
		OSL_PREFETCH(PKTTAG(pkt));

		/* fetch next available slot in workq */
		elem_ix = bcm_ring_prod(WORKQ_RING(workq), DHD_LB_WORKQ_SZ);

		DHD_PKTTAG_SET_PA((dhd_pkttag_fr_t *)PKTTAG(pkt), pa);
		DHD_PKTTAG_SET_PA_LEN((dhd_pkttag_fr_t *)PKTTAG(pkt), len);

		if (elem_ix == BCM_RING_FULL) {
			DHD_ERROR(("tx_compl_prod BCM_RING_FULL\n"));
			goto workq_ring_full;
		}

		elem = WORKQ_ELEMENT(void *, &prot->tx_compl_prod, elem_ix);
		*elem = pkt;

		smp_wmb();

		/* Sync WR index to consumer if the SYNC threshold has been reached */
		if (++prot->tx_compl_prod_sync >= DHD_LB_WORKQ_SYNC) {
			bcm_workq_prod_sync(workq);
			prot->tx_compl_prod_sync = 0;
		}

		DHD_INFO(("%s: tx_compl_prod pkt<%p> sync<%d>\n",
		__FUNCTION__, pkt, prot->tx_compl_prod_sync));

		DHD_GENERAL_UNLOCK(dhd, flags);
		return;
	   }

workq_ring_full:

#endif /* !DHD_LB_TXC */

	/*
	 * We can come here if no DHD_LB_TXC is enabled and in case where DHD_LB_TXC is
	 * defined but the tx_compl queue is full.
	 */
	if (pkt == NULL) {
		pkt = DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle,
			pktid, pa, len, dmah, secdma, PKTTYPE_DATA_TX);
	}

	if (pkt) {
#ifdef BCM_WFD
		if (!DHD_PKT_GET_WFD_BUF(pkt)) {
#endif

#ifdef BCM_SECURE_DMA
{
			int offset = 0;
			BCM_REFERENCE(offset);

			if (prot->tx_metadata_offset)
				offset = prot->tx_metadata_offset + ETHER_HDR_LEN;
			SECURE_DMA_UNMAP(dhd->osh, pa,
				(uint) prot->tx_metadata_offset, DMA_RX, 0, dmah,
				secdma, offset);
}
#else
			DMA_UNMAP(dhd->osh, pa, (uint) len, DMA_RX, 0, dmah);
#endif /* BCM_SECURE_DMA */
#ifdef BCM_WFD
		}
#endif
#if defined(BCMPCIE) && (defined(LINUX) || defined(OEM_ANDROID))
		dhd_txcomplete(dhd, pkt, true);
#endif /* BCMPCIE && (defined(LINUX) || defined(OEM_ANDROID)) */

#ifdef BCM_BLOG
		DHD_UNLOCK(dhd);
#endif
		DHD_GENERAL_UNLOCK(dhd, flags);
		PKTFREE(dhd->osh, pkt, TRUE);
		DHD_GENERAL_LOCK(dhd, flags);
		DHD_FLOWRING_TXSTATUS_CNT_UPDATE(dhd->bus, txstatus->compl_hdr.flow_ring_id,
		txstatus->tx_status);
#ifdef BCM_BLOG
		DHD_LOCK(dhd);
#endif
	}

	DHD_GENERAL_UNLOCK(dhd, flags);
} /* dhd_prot_txstatus_process */

/** called on MSG_TYPE_WL_EVENT message received from dongle */
static void
dhd_prot_event_process(dhd_pub_t *dhd, void *msg)
{
	wlevent_req_msg_t *evnt;
	uint32 bufid;
	uint16 buflen;
	int ifidx = 0;
	void* pkt;
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;

	/* Event complete header */
	evnt = (wlevent_req_msg_t *)msg;
	bufid = ltoh32(evnt->cmn_hdr.request_id);

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, bufid, DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

	buflen = ltoh16(evnt->event_data_len);

	ifidx = BCMMSGBUF_API_IFIDX(&evnt->cmn_hdr);

	/* Post another rxbuf to the device */
	if (prot->cur_event_bufs_posted) {
		prot->cur_event_bufs_posted--;
	}
	dhd_msgbuf_rxbuf_post_event_bufs(dhd);

	/* locks required to protect pktid_map */
	DHD_GENERAL_LOCK(dhd, flags);
	pkt = dhd_prot_packet_get(dhd, bufid, PKTTYPE_EVENT_RX, TRUE);
	DHD_GENERAL_UNLOCK(dhd, flags);

	if (!pkt) {
		return;
	}

	if (buflen > DHD_RX_EVENT_BUFSZ) {
		DHD_ERROR(("%s: %s: event size too big<%u>\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__, buflen));
	}

#if !defined(BCM_ROUTER_DHD)
	/* DMA RX offset updated through shared area */
	if (prot->dma_rxoffset) {
		PKTPULL(dhd->osh, pkt, prot->dma_rxoffset);
	}
#endif /* !BCM_ROUTER_DHD */

	PKTSETLEN(dhd->osh, pkt, buflen);

	dhd_bus_rx_frame(dhd->bus, pkt, ifidx, 1);
}

/**
 * Consumer handler for RxCompletion Ring.
 * Does not have support for D2H DMA Sync nor Load-balancing support.
 * dhd_prot_process_msgbuf_rxcpl() will invoke the rxcpln_fn hook. All messages
 * in the RxCpln ring will be processed here, given the address of the first
 * message and the total length of all messages.
 */
static int BCMFASTPATH
dhd_prot_process_rxcpln(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	uint8 *buf, uint32 buf_len)
{
	dhd_prot_t *prot;
	unsigned long flags;

	uint8 ifidx, pktflags;
	uint16 data_offset, data_len;
	uint16 rx_compl_count, item_size;
	uint32 pktid;
	void *pkt;

	const uint8 * buf_end = buf + buf_len;

#if defined(BCM_DHD_RUNNER)
	if (DHD_RNR_OFFL_RXCMPL(dhd)) {
		DHD_ERROR(("%s: Unexpected direct RX_CMPLT when Offloaded\r\n",
			__FUNCTION__));
		ASSERT(FALSE);
		return BCME_ERROR;
	}
#endif /* BCM_DHD_RUNNER */

	/* Stage 0. Preparation stage */

	// OSL_PREFETCH(buf) would have been invoked in caller

	prot = dhd->prot;
	rx_compl_count = 0;
	item_size = ring->item_size >> ring->item_misc;

	while (buf < buf_end) /* Process all RxCompletion workitems */
	{
		/* Stage 1. Parse the RxCompletion workitem */
		switch (ring->item_type) {
			case MSGBUF_WI_WI64: {
				host_rxbuf_cmpl_t *wi = (host_rxbuf_cmpl_t *) buf;
				ifidx = wi->cmn_hdr.if_id;
				pktid = ltoh32(wi->cmn_hdr.request_id);
				pktflags = (uint8)wi->flags;
				data_offset = ltoh16(wi->data_offset);
				data_len = ltoh16(wi->data_len);
				break;
			}
			case MSGBUF_WI_CWI: {
				hwa_rxcple_cwi_t *cwi = (hwa_rxcple_cwi_t *) buf;
				ifidx = BCM_GBF(cwi->u8[0], HWA_RXCPLE_IFID);
				pktid = ltoh32(cwi->host_pktid);
				pktflags = BCM_GBF(cwi->u8[0], HWA_RXCPLE_FLAGS);
				data_offset = ltoh16(cwi->data_offset);
				data_len = ltoh16(cwi->data_len);
				break;
			}
			case MSGBUF_WI_ACWI: {
				hwa_rxcple_cwi_t *cwi = (hwa_rxcple_cwi_t *) buf;
				pktid = ltoh32(cwi->host_pktid);
				if (pktid == HWA_HOST_PKTID_NULL) {
					/* Shift to next aggregate workitem */
					buf = ALIGN_ADDR(buf, ring->item_size);
					OSL_PREFETCH(buf);
					continue;
				}
				ifidx = BCM_GBF(cwi->u8[0], HWA_RXCPLE_IFID);
				pktflags = BCM_GBF(cwi->u8[0], HWA_RXCPLE_FLAGS);
				data_offset = ltoh16(cwi->data_offset);
				data_len = ltoh16(cwi->data_len);

				break;
			}
			default:
				pktid = 0; ifidx = pktflags = 0; data_offset = data_len = 0;
				ASSERT(0);
		} /* switch item_type */

		DHD_INFO(("%s pktid 0x%08x\n", __FUNCTION__, pktid));
		ASSERT(pktid != 0);

#if defined(DHD_PKTID_AUDIT_RING)
		DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid, DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

		/* Stage 2.  Replenish RxBuffer in bulk of RX_BUF_BURST */
		rx_compl_count++;

		if (rx_compl_count >= RX_BUF_BURST) {

			if (prot->rxbufpost >= rx_compl_count) {
				prot->rxbufpost -= rx_compl_count;
				rx_compl_count = 0;
			} else {
				DHD_ERROR(("%s: %u extra RxCompletions received\n",
					__FUNCTION__, rx_compl_count - prot->rxbufpost));
				prot->rxbufpost = 0;
			}

			if (prot->rxbufpost <= (prot->max_rxbufpost - RXBUFPOST_THRESHOLD)) {
				dhd_msgbuf_rxbuf_post(dhd, FALSE); /* alloc pkt ids */
			}
		}

		/* prepare to process next work item */
		buf += item_size;
		OSL_PREFETCH(buf);

		/* Stage 3. Convert pktid to a pkt and pass up to the OS layer */

		DHD_GENERAL_LOCK(dhd, flags);

		pkt = dhd_prot_packet_get(dhd, pktid, PKTTYPE_DATA_RX, TRUE);
		OSL_PREFETCH(pkt);

		DHD_GENERAL_UNLOCK(dhd, flags);

		if (!pkt) {
			DHD_ERROR(("%s pktid<%u> mapped to NULL pkt\n",
			          __FUNCTION__, pktid));
			continue;
		}

#ifdef BCMHWA
		if (data_len == 0) {
			/* HWA Rx block may send zero length packet
			 * to free up pktids when reset 1a and 1b block.
			 * Or free up pktids for dropped rx packets.
			 */
			PKTFREE(dhd->osh, pkt, FALSE);
			continue;
		}
#endif

		if (pktflags & BCMPCIE_PKT_FLAGS_FRAME_802_11) {
			DHD_INFO(("D11 frame rxed \n"));
		}

		PKTPULL(dhd->osh, pkt, data_offset);
		PKTSETLEN(dhd->osh, pkt, data_len);

#ifdef WL_MONITOR
		if (dhd_monitor_enabled(dhd, ifidx)) {
			if (pktflags & BCMPCIE_PKT_FLAGS_FRAME_802_11) {
				dhd_rx_mon_pkt(dhd, (host_rxbuf_cmpl_t *)buf, pkt, ifidx);
				continue;
			}
		}
#endif /* WL_MONITOR */

#if defined(DHD_RX_CHAINING) || (defined(BCM_AWL) && defined(DHD_AWL_RX))
#ifdef BCM_CPE_PKTC
	if (dhd->pktc)
#else
	if (1)
#endif /* BCM_CPE_PKTC */
		dhd_rxchain_frame(dhd, pkt, ifidx); /* chain the packets */
	else
		dhd_bus_rx_frame(dhd->bus, pkt, ifidx, 1);
#else
		dhd_bus_rx_frame(dhd->bus, pkt, ifidx, 1); /* Send up */
#endif /* ! DHD_RX_CHAINING && !(BCM_AWL && DHD_AWL_RX) */

	} /*  while more RxCompletions to process */

	/* Stage 4. Final accounting and replenish if necessary */

	if (prot->rxbufpost >= rx_compl_count) {
		prot->rxbufpost -= rx_compl_count;
	} else {
		DHD_ERROR(("%s: %u extra RxCompletions received\n",
		           __FUNCTION__, rx_compl_count - prot->rxbufpost));
		prot->rxbufpost = 0;
	}

	if (prot->rxbufpost <= (prot->max_rxbufpost - RXBUFPOST_THRESHOLD)) {
		dhd_msgbuf_rxbuf_post(dhd, FALSE); /* alloc pkt ids */
	}

#if defined(DHD_RX_CHAINING) || (defined(BCM_AWL) && defined(DHD_AWL_RX))
	dhd_rxchain_commit(dhd); /* send up */
#endif /* DHD_RX_CHAINING || (BCM_AWL && DHD_AWL_RX) */

	return BCME_OK;
} /* dhd_prot_process_rxcpln */

/** called on MSG_TYPE_RX_CMPLT message received from dongle */
static void BCMFASTPATH
dhd_prot_rxcmplt_process(dhd_pub_t *dhd, void *msg)
{
	host_rxbuf_cmpl_t *rxcmplt_h;
	uint16 data_offset;             /* offset at which data starts */
	void *pkt;
	unsigned long flags;
	uint ifidx;
	uint32 pktid;
#if defined(DHD_LB_RXC)
	const bool free_pktid = FALSE;
#else
	const bool free_pktid = TRUE;
#endif /* DHD_LB_RXC */

#if defined(BCM_DHD_RUNNER)
	if (DHD_RNR_OFFL_RXCMPL(dhd)) {
	    /*
	     * RX_CMPLT message should not directly come from dongle when
	     * Rx is offloaded to Runner
	     */
	    DHD_ERROR(("%s: Unexpected direct RX_CMPLT when Offloaded\r\n",
	        __FUNCTION__));
	    ASSERT(FALSE);
	    return;
	}
#endif /* BCM_DHD_RUNNER */

	/* RXCMPLT HDR */
	rxcmplt_h = (host_rxbuf_cmpl_t *)msg;

	/* offset from which data starts is populated in rxstatus0 */
	data_offset = ltoh16(rxcmplt_h->data_offset);

	pktid = ltoh32(rxcmplt_h->cmn_hdr.request_id);

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, dhd->prot->pktid_map_handle, pktid,
		DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

	DHD_GENERAL_LOCK(dhd, flags);
	pkt = dhd_prot_packet_get(dhd, pktid, PKTTYPE_DATA_RX, free_pktid);
	DHD_GENERAL_UNLOCK(dhd, flags);

	if (!pkt) {
		return;
	}

	/* Post another set of rxbufs to the device */
	dhd_prot_return_rxbuf(dhd, pktid, 1);

	DHD_INFO(("id 0x%04x, offset %d, len %d, ifid %d, phase 0x%02x, pktdata %p, metalen %d\n",
		ltoh32(rxcmplt_h->cmn_hdr.request_id), data_offset, ltoh16(rxcmplt_h->data_len),
		rxcmplt_h->cmn_hdr.if_id, rxcmplt_h->cmn_hdr.flags, PKTDATA(dhd->osh, pkt),
		ltoh16(rxcmplt_h->metadata_len)));

	if (rxcmplt_h->flags & BCMPCIE_PKT_FLAGS_FRAME_802_11) {
		DHD_INFO(("D11 frame rxed \n"));
	}

	/* data_offset from buf start */
	if (data_offset) {
		/* data offset given from dongle after split rx */
		PKTPULL(dhd->osh, pkt, data_offset); /* data offset */
	} else {
		/* DMA RX offset updated through shared area */
		if (dhd->prot->dma_rxoffset) {
			PKTPULL(dhd->osh, pkt, dhd->prot->dma_rxoffset);
		}
	}
	/* Actual length of the packet */
	PKTSETLEN(dhd->osh, pkt, ltoh16(rxcmplt_h->data_len));

	ifidx = rxcmplt_h->cmn_hdr.if_id;
#ifdef WL_MONITOR
	if (dhd_monitor_enabled(dhd, ifidx) &&
		(rxcmplt_h->flags & BCMPCIE_PKT_FLAGS_FRAME_802_11)) {
		dhd_rx_mon_pkt(dhd, rxcmplt_h, pkt, ifidx);
		return;
	}
#endif /* WL_MONITOR */

#if defined(DHD_LB_RXP)
	dhd_lb_rx_pkt_enqueue(dhd, pkt, ifidx);
#else  /* ! DHD_LB_RXP */
#ifdef DHD_RX_CHAINING
#ifdef BCM_CPE_PKTC
	if (dhd->pktc)
#else
	if (0)
#endif /* BCM_CPE_PKTC */
		dhd_rxchain_frame(dhd, pkt, ifidx);
	else
		dhd_bus_rx_frame(dhd->bus, pkt, ifidx, 1);
#else /* ! DHD_RX_CHAINING */
	/* offset from which data starts is populated in rxstatus0 */
	dhd_bus_rx_frame(dhd->bus, pkt, ifidx, 1);
#endif /* ! DHD_RX_CHAINING */
#endif /* ! DHD_LB_RXP */
} /* dhd_prot_rxcmplt_process */

/** Stop protocol: sync w/dongle state. */
void
dhd_prot_stop(dhd_pub_t *dhd)
{
	ASSERT(dhd);
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));
}

/* Add any protocol-specific data header.
 * Caller must reserve prot_hdrlen prepend space.
 */
void BCMFASTPATH
dhd_prot_hdrpush(dhd_pub_t *dhd, int ifidx, void *PKTBUF)
{
}

uint
dhd_prot_hdrlen(dhd_pub_t *dhd, void *PKTBUF)
{
	return 0;
}

/** LLCSNAP Insertion support: Used in 32bit copy with 32bit endian */
#include <802.3.h>

#define SNAP_HDR_LEN 6
static const union {
	uint32 u32;
	uint16 u16[2];
	char u8[4];
} _ctl_oui3 = { .u8 = {0x00, 0x00, 0x00, 0x03} };
static const uint8 _llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

/** LLCSNAP: OUI[2] setting for Bridge Tunnel (Apple ARP and Novell IPX) */
#define ETHER_TYPE_APPLE_ARP    0x80f3 /* Apple Address Resolution Protocol */
#define ETHER_TYPE_NOVELL_IPX   0x8137 /* Novel IPX Protocol */

#define BRIDGE_TUNNEL_OUI2      0xf8 /* OUI[2] value for Bridge Tunnel */

#define IS_BRIDGE_TUNNEL(et) \
	(((et) == ETHER_TYPE_APPLE_ARP) || ((et) == ETHER_TYPE_NOVELL_IPX))

/* Copy 14B ethernet header: 32bit aligned source and destination. */
#define edasacopy32(s, d) \
do { \
	((uint32 *)(d))[0] = ((const uint32 *)(s))[0]; \
	((uint32 *)(d))[1] = ((const uint32 *)(s))[1]; \
	((uint32 *)(d))[2] = ((const uint32 *)(s))[2]; \
} while (0)

/*
 * Copy Ethernet header alongwith LLCSNAP.
 * Handcoded construction of dot3_mac_llc_snap_header.
 *    6B 802.3 MacDA
 *    6B 802.3 MacSA
 *    2B Ethernet Data Length (includes 8B LLCSNAP header length)
 *    2B LLC DSAP SSAP
 *    1B LLC CTL
 *    3B SNAP OUI
 *    2B EtherType
 */
static INLINE BCMFASTPATH  void
dhd_prot_insert_llcsnap(uint8 *s, uint8 *d, uint16 eth_len, uint16 eth_type,
	const uint align32);

static INLINE BCMFASTPATH  void
dhd_prot_insert_llcsnap(uint8 *s, uint8 *d, uint16 eth_len, uint16 eth_type,
	const uint align32)
{
	/* 802.3 MacDA MacSA: 12B copy, using 3 x 32b aligned 4B copy */
	if (align32 == TRUE) {
		edasacopy32(s, d);
		/* LLC ctl = 0x03, out[3] = { 0x00 0x00 0x00}: 32b aligned 4B copy */
		((uint32 *)(d))[4] = HTON32(_ctl_oui3.u32);
	} else {
		eacopy(s, d);
		eacopy(s+ETHER_ADDR_LEN, d+ETHER_ADDR_LEN);
		((uint16 *)(d))[8] = HTON16(_ctl_oui3.u16[0]);
		((uint16 *)(d))[9] = HTON16(_ctl_oui3.u16[1]);
	}
	/* ethernet payload length: 2B */
	((uint16 *)(d))[6] = HTON16(eth_len);

	/* Reference bcm_proto.h: static const uint8 _llc_snap_hdr[] */
	/* dsap = 0xaa ssap = 0xaa: 2B copy */
	((uint16 *)(d))[7] = (uint16)0xAAAA; /* no need for htons16 */

	/* Set OUI[2] for Bridge Tunnel */
	if (IS_BRIDGE_TUNNEL(eth_type)) {
		((struct dot3_mac_llc_snap_header*)(d))->oui[2] = BRIDGE_TUNNEL_OUI2;
	}
}

#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
/**
 * Given a flow ring, the minimum of the queue length and the available pktid
 * is determined. The flowring is then queried for space (reserving the space in
 * the flowring). The flowring may have less than the requested space, and
 * returns (NULL) or the address where 'alloced' number of packets may be
 * written into. After these alloced number of packets are written, the flowring
 * WR is updated and doorbell posted. A retry for space availability is done.
 *
 * dhd_prot_txdata is not aware of PKTFWDERBUF type packets in a flowring's
 * queue, and hence dhd_prot_txqueue_flush() must be invoked to flush packets
 * out of a queue into a flowring, when PKTFWDERBUF typed packets may be placed
 * into a queue (BCM_GMAC3).
 */
int BCMFASTPATH
dhd_prot_txqueue_flush(dhd_pub_t *dhd, void *flow_ring)
{
	dhd_prot_t *prot = dhd->prot;
	flow_ring_node_t *flow_ring_node;
	flow_queue_t * flow_queue;
	msgbuf_ring_t *ring;
	int nitems;
	uint16 queue_avail, ring_avail;
	uint16 alloced = 0, try_once = 0;
	uint16 processed = 0;
	uint16 flowid = 0;
	uint32 txpost_hdr_offset;
	void   *txp_wi_start; /* Start address of allocated space in TxPost ring */
	uint8  *txp_wi; /* current work item being filled */
	uint32 ifid;

	flow_ring_node = (flow_ring_node_t *)flow_ring;
	flowid = flow_ring_node->flowid;
	flow_queue = &flow_ring_node->queue;
	ring = (msgbuf_ring_t*)(flow_ring_node->prot_info);

	ASSERT(prot->tx_metadata_offset == 0);

	if ((queue_avail = (uint16)DHD_FLOW_QUEUE_LEN(flow_queue)) == 0) {
		return BCME_OK;
	}

#if defined(DHD_PCIE_PKTID)
	{
		uint16 pktid_avail = (uint16)DHD_PKTID_AVAIL(prot->pktid_map_handle);
		if ((pktid_avail + queue_avail) < 2) {
			if (pktid_avail == 0) {
				DHD_INFO(("%s:%d Pktid pool depleted.\n", __FUNCTION__, __LINE__));
			}
			return BCME_NORESOURCE;
		}

		/* request ring_avail space - min of queue_avail, pktid_avail, depth */
		ring_avail = MIN(queue_avail, pktid_avail);
	}
	ring_avail = MIN(ring_avail, ring->max_items);
#else  /* ! DHD_PCIE_PKTID */
	ring_avail = MIN(queue_avail, ring->max_items);
#endif /* ! DHD_PCIE_PKTID */

retry_ring_space:
	txp_wi_start =
		dhd_prot_alloc_ring_space(dhd, ring, ring_avail, &alloced, FALSE);
	if (txp_wi_start == NULL) {
		DHD_INFO(("%s: HTOD Msgbuf Not available TxCount = %d\n",
			__FUNCTION__, prot->active_tx_count));
		goto done_txpost;
	}

	ring_avail -= alloced; /* may not get requested space or wrapped */
	txp_wi = (uint8 *)txp_wi_start; /* start address in flowring */
	txpost_hdr_offset = ring->item_misc; /* EthHdr offset in work item */
	ifid = flow_ring_node->flow_info.ifindex;

	/* submit nitems of TxPost by dequeueing packets from flowring's queue */
	for (nitems = 0; nitems < alloced; nitems++)
	{
		void   *pkt; /* current packet */
		uint8  *pktdata; /* start of data ... may include rxStatus LAN */
		uint8  *pkthdr; /* 802.3 header in packet */
		uint8  *txp_wi_pkthdr; /* pointer to 802.3 hdr in current work item */
		uint8  txp_flags;
		uint16 pktlen;
		uint8  prio;
		uint32 pktid;
		dmaaddr_t pa;

		/* location in the work item, where the ether header will be placed */
		txp_wi_pkthdr = txp_wi + txpost_hdr_offset;

		/* fetch a packet from the flowring's backup queue */
		pkt = dhd_flow_queue_dequeue(dhd, flow_queue);

		if (pkt == NULL) { /* flow_ring's queue is empty .... hmmm */
			DHD_ERROR(("%s: %s: NULL pkt, queue_avail<%u> alloced<%u>\n",
			           dhd_ifname(dhd, ifid), __FUNCTION__, queue_avail, alloced));
			ASSERT(pkt != NULL);
			try_once = ~0;
			goto done_queue_processing;
		}

		dhd_shortpktpad(dhd, pkt);

		pktdata = PKTDATA(dhd->osh, pkt);
		pktlen = (uint16)PKTLEN(dhd->osh, pkt);

		{
			pkthdr = pktdata;
#ifdef BCM_NBUFF_WLMCAST
			if (DHDHDR_SUPPORT(dhd) && FKB_HAS_DHDHDR(pkt)) {
				pkthdr -= DOT11_LLC_SNAP_HDR_LEN;
			}
			/* if it is WMF handled mutlicast packet, its MAC is in pkttag */
			if (FKB_IS_WMF_UCAST(pkt)) {

				eacopy(DHD_PKT_GET_MAC(pkt),txp_wi_pkthdr); /* copy da address */
				if (FKB_HAS_DHDHDR(pkt)) {
					/* cpy sr address */
					eacopy(pkthdr+ETHER_ADDR_LEN, txp_wi_pkthdr+ETHER_ADDR_LEN);
					/* if has to handle vlan type,here need to adjust
					accordingly, cpy type  here
					*/
					bcopy(pkthdr+2*ETHER_ADDR_LEN+DOT11_LLC_SNAP_HDR_LEN,
						txp_wi_pkthdr+2*ETHER_ADDR_LEN, sizeof(uint16));
				} else {
					bcopy(pkthdr+ETHER_ADDR_LEN, txp_wi_pkthdr+ETHER_ADDR_LEN,
						ETHER_HDR_LEN-ETHER_ADDR_LEN);
				}
			} else if (FKB_HAS_DHDHDR(pkt)) {
				eacopy(pkthdr, txp_wi_pkthdr);
				eacopy(pkthdr+ETHER_ADDR_LEN, txp_wi_pkthdr+ETHER_ADDR_LEN);
				bcopy(pkthdr+2*ETHER_ADDR_LEN+DOT11_LLC_SNAP_HDR_LEN,
						txp_wi_pkthdr+2*ETHER_ADDR_LEN, sizeof(uint16));
			} else
#endif /* BCM_NBUFF_WLMCAST */
			bcopy(pkthdr, txp_wi_pkthdr, ETHER_HDR_LEN);

			pktlen -= (uint16) ETHER_HDR_LEN;
			pktdata += ETHER_HDR_LEN;

#ifdef BCM_NBUFF_WLMCAST
			if (DHDHDR_SUPPORT(dhd) && !FKB_HAS_DHDHDR(pkt)) {
#else
			if (DHDHDR_SUPPORT(dhd)) {
#endif
				uint8 *s = pkthdr;
				uint8 *d = pkthdr - DOT11_LLC_SNAP_HDR_LEN;
				uint16 ethlen = pktlen + DOT11_LLC_SNAP_HDR_LEN;
				uint16 ether_type = ((struct ether_header*)s)->ether_type;

				/* Ensure space for LLCSNAP */
				ASSERT(PKTHEADROOM(dhd->osh, pkt) >= DOT11_LLC_SNAP_HDR_LEN);

				eacopy(s, d);
				eacopy(s + ETHER_ADDR_LEN, d + ETHER_ADDR_LEN);

				((uint16 *)(d))[6] = HTON16(ethlen);

				bcopy(_llc_snap_hdr, d + ETHER_HDR_LEN, SNAP_HDR_LEN);

				/* Set OUI[2] for Bridge Tunnel */
				if (IS_BRIDGE_TUNNEL(ether_type)) {
					((struct dot3_mac_llc_snap_header*)(d))->oui[2] =
						BRIDGE_TUNNEL_OUI2;
				}
#ifdef BCM_NBUFF_WLMCAST
				/* mark to tell theat this FKB is DHDHDR moved */
				DHD_PKT_SET_DATA_DHDHDR(pkt);
#endif

				/* Cache flush */
				DMA_MAP(dhd->osh, d, ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN,
					DMA_TX, NULL, NULL);
			}

#if defined(BCM_NBUFF) && defined(BCM_WFD)
			if (DHD_PKT_GET_WFD_BUF(pkt))
				nbuff_flush((pNBuff_t)pkt, pktdata, BCM_WFD_CACHE_FLUSH_LEN);
			else
				nbuff_flush((pNBuff_t)pkt, pktdata, pktlen);
			ULONGTOPHYSADDR((ulong)virt_to_phys(pktdata), pa);
#else
			pa = DMA_MAP(dhd->osh, pktdata, pktlen, DMA_TX, pkt, 0);
#endif /* BCM_NBUFF && BCM_WFD */
		}
		if (PHYSADDRISZERO(pa)) {
			DHD_ERROR(("%s: Invalid physaddr 0\n", dhd_ifname(dhd, ifid)));
			dhd_flow_queue_reinsert(dhd, flow_queue, pkt);
			break;
		}

		pktid = DHD_NATIVE_TO_PKTID(dhd, prot->pktid_map_handle, pkt,
			pa, pktlen, DMA_TX, NULL, ring->dma_buf.secdma, PKTTYPE_DATA_TX);
#if defined(DHD_PCIE_PKTID)
		ASSERT(pktid != DHD_PKTID_INVALID);
#endif /* DHD_PCIE_PKTID */

#if defined(DHD_PKTID_AUDIT_RING)
		DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid,
			DHD_DUPLICATE_ALLOC);
#endif /* DHD_PKTID_AUDIT_RING */

		DHD_TRACE(("txpost: data_len %d, pktid 0x%04x\n", pktlen, pktid));

		/* Some common fields in legacy, cwi32 or cwi64 format */
		// ifid common to all packets ...

		txp_flags = BCMPCIE_PKT_FLAGS_FRAME_802_3;
		prio = (uint8)PKTPRIO(pkt);

		pktid = htol32(pktid);
		pktlen = htol16(pktlen);

		/* Compose the rest of the TxPost work item (etherhdr done) */
		switch (ring->item_type)
		{
			case MSGBUF_WI_WI64: /* Legacy Work item */
			{
				host_txbuf_post_t *wi64 = (host_txbuf_post_t *)txp_wi;
				wi64->cmn_hdr.msg_type = MSG_TYPE_TX_POST;
				wi64->cmn_hdr.request_id = pktid;
				wi64->cmn_hdr.if_id = ifid;
				wi64->flags = (uint8)txp_flags |
				              ((prio & 0x7) << BCMPCIE_PKT_FLAGS_PRIO_SHIFT);
				wi64->seg_cnt = 1;
				wi64->data_len = pktlen;
				HADDR64_LO_SET_HTOL(wi64->data_buf_haddr64, PHYSADDRLO(pa));
				HADDR64_HI_SET_HTOL(wi64->data_buf_haddr64, PHYSADDRHI(pa));
				// wi64->metadata_buf_len = htol16(0);
				// wi64->metadata_buf_addr.high_addr = 0;
				// wi64->metadata_buf_addr.low_addr = 0;
				break;
			}

			case MSGBUF_WI_CWI32:
			{
				hwa_txpost_cwi32_t *cwi32 = (hwa_txpost_cwi32_t *)txp_wi;
				cwi32->host_pktid = pktid;
				cwi32->u32[1] =
					BCM_SBF(ifid, HWA_TXPOST_IFID)
					| BCM_SBF(prio, HWA_TXPOST_PRIO)
					// | BCM_SBF(0, HWA_TXPOST_COPY)
					| BCM_SBF(txp_flags, HWA_TXPOST_FLAGS)
					| BCM_SBF(pktlen, HWA_TXPOST_DATA_BUF_HLEN);
				cwi32->data_buf_haddr32 = htol32(PHYSADDRLO(pa));
				ASSERT(PHYSADDRHI(pa) == prot->host_physaddrhi);
				break;
			}

			case MSGBUF_WI_CWI64:
			{
				hwa_txpost_cwi64_t *cwi64 = (hwa_txpost_cwi64_t *)txp_wi;
				cwi64->host_pktid = pktid;
				cwi64->u32[1] =
					BCM_SBF(ifid, HWA_TXPOST_IFID)
					| BCM_SBF(prio, HWA_TXPOST_PRIO)
					// | BCM_SBF(0, HWA_TXPOST_COPY)
					| BCM_SBF(txp_flags, HWA_TXPOST_FLAGS)
					| BCM_SBF(pktlen, HWA_TXPOST_DATA_BUF_HLEN);
				HADDR64_LO_SET_HTOL(cwi64->data_buf_haddr64, PHYSADDRLO(pa));
				HADDR64_HI_SET_HTOL(cwi64->data_buf_haddr64, PHYSADDRHI(pa));
				break;
			}

			default:
				DHD_ERROR(("%s: %s Invalid item_type %u\n",
				          dhd_ifname(dhd, ifid), __FUNCTION__, ring->item_type));
				ASSERT(0);

		} /* switch item_type */

		txp_wi = (uint8 *)txp_wi + ring->item_size;

	} /* for nitems */

done_queue_processing:

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_flow_ring_write_complete(dhd, flow_ring_node, (void*)txp_wi_start, nitems);
	prot->active_tx_count += nitems;
	processed += nitems;

	/* Check if more space is available, try only once or we will loop here */
	if (ring_avail > try_once) {
		try_once = ~0; /* do not try again even if ring_avail is non zero */
		goto retry_ring_space;
	}

done_txpost:
	return (processed > 0) ? BCME_OK : BCME_NORESOURCE;
} /* dhd_prot_txqueue_flush */

#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

#ifdef __FreeBSD__
#define PKTBUF *pktbuf
#else
#define PKTBUF pktbuf
#endif

/**
 * Called when a tx ethernet packet has been dequeued from a flow queue, and has to be inserted in
 * the corresponding flow ring.
 */
int BCMFASTPATH
dhd_prot_txdata(dhd_pub_t *dhd, void *PKTBUF, uint8 ifidx)
{
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;
	uint8  *txp_wi_pkthdr; /* pointer to 802.3 hdr in current work item */
	void   *txp_wi_start; /* Start address of allocated space in TxPost ring */
	uint8  txp_flags;
	dmaaddr_t pa;
	uint8 *pktdata;
	uint32 pktlen;
	uint32 pktid;
	uint8  prio;
	uint16 flowid = 0;
	uint16 alloced = 0;
	msgbuf_ring_t *ring;
	flow_ring_table_t *flow_ring_table;
	flow_ring_node_t *flow_ring_node;

#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */

#if defined(DHD_PCIE_PKTID)
	void *dmah;
	void *secdma;
#endif /* DHD_PCIE_PKTID */

	ASSERT(prot->tx_metadata_offset == 0);

	if (dhd->flow_ring_table == NULL) {
		return BCME_NORESOURCE;
	}

	flowid = DHD_PKT_GET_FLOWID(PKTBUF);

	flow_ring_table = (flow_ring_table_t *)dhd->flow_ring_table;
	flow_ring_node = (flow_ring_node_t *)&flow_ring_table[flowid];

	ring = (msgbuf_ring_t *)flow_ring_node->prot_info;

#if defined(BCM_DHDHDR)
	if (DHDHDR_SUPPORT(dhd))
#ifdef BCM_NBUFF
	    if (IS_SKBUFF_PTR(PKTBUF))
#endif /* BCM_NBUFF */
		ASSERT(skb_headroom(PKTBUF) >= DOT11_LLC_SNAP_HDR_LEN);
#endif /* BCM_DHDHDR && PCIE_FULL_DONGLE */

	/*
	 * XXX:
	 * JIRA SW4349-436:
	 * Copying the TX Buffer to an SKB that lives in the DMA Zone
	 * is done here. Previously this was done from dhd_stat_xmit
	 * On conditions where the Host is pumping heavy traffic to
	 * the dongle, we see that the Queue that is backing up the
	 * flow rings is getting full and holds the precious memory
	 * from DMA Zone, leading the host to run out of memory in DMA
	 * Zone. So after this change the back up queue would continue to
	 * hold the pointers from Network Stack, just before putting
	 * the PHY ADDR in the flow rings, we'll do the copy.
	 */

	DHD_GENERAL_LOCK(dhd, flags);

	/* Create a unique 32-bit packet id */
	pktid = DHD_NATIVE_TO_PKTID_RSV(dhd, prot->pktid_map_handle, PKTBUF);
#if defined(DHD_PCIE_PKTID)
	if (pktid == DHD_PKTID_INVALID) {
		DHD_ERROR(("%s: %s:%d Pktid pool depleted.\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__, __LINE__));
		/*
		 * If we return error here, the caller would queue the packet
		 * again. So we'll just free the skb allocated in DMA Zone.
		 * Since we have not freed the original SKB yet the caller would
		 * requeue the same.
		 */
		goto err_no_res_pktfree;
	}
#endif /* DHD_PCIE_PKTID */

#ifdef BCM_SECURE_DMA
	if (!SECURE_DMA_BUFFS_IS_AVAIL(dhd->osh)) {
#if defined(DHD_PCIE_PKTID)
		/* Free up the PKTID. physaddr and pktlen will be garbage. */
		DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle, pktid,
			pa, pktlen, dmah, secdma, PKTTYPE_NO_CHECK);
#endif /* DHD_PCIE_PKTID */
		DHD_INFO(("%s:%d: SECDMA Buffers Not available \n",
			__FUNCTION__, __LINE__));
		goto err_no_res_pktfree;
	}
#endif /* BCM_SECURE_DMA */

	/* Reserve space in the circular buffer */
	txp_wi_start =
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);
	if (txp_wi_start == NULL) {
#if defined(DHD_PCIE_PKTID)
		/* Free up the PKTID. physaddr and pktlen will be garbage. */
		DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle, pktid,
			pa, pktlen, dmah, secdma, PKTTYPE_NO_CHECK);
#endif /* DHD_PCIE_PKTID */
		DHD_INFO(("%s:%d: HTOD Msgbuf Not available TxCount = %d\n",
			__FUNCTION__, __LINE__, prot->active_tx_count));
		goto err_no_res_pktfree;
	}

	/* Extract the data pointer and length information */
	pktdata = PKTDATA(dhd->osh, PKTBUF);
	pktlen  = PKTLEN(dhd->osh, PKTBUF);

	/* offset of ethderhdr in txpost is passed through flowring's flag field */
	txp_wi_pkthdr = (uint8 *)txp_wi_start + ring->item_misc;

	/* Ethernet header: Copy before we cache flush packet using DMA_MAP */
	bcopy(pktdata, txp_wi_pkthdr, ETHER_HDR_LEN);

	/* Construct SubFrame Header by inserting LLCSNAP */
	if (DHDHDR_SUPPORT(dhd)) {
		uint8 *s = pktdata;
		uint8 *d = pktdata - DOT11_LLC_SNAP_HDR_LEN;
		uint16 ethlen = pktlen - ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN;
		uint16 ether_type = ((struct ether_header*)s)->ether_type;

		eacopy(s, d);
		eacopy(s + ETHER_ADDR_LEN, d + ETHER_ADDR_LEN);

		((uint16 *)(d))[6] = HTON16(ethlen);

		bcopy(_llc_snap_hdr, d + ETHER_HDR_LEN, SNAP_HDR_LEN);

		if (IS_BRIDGE_TUNNEL(ether_type)) {
			((struct dot3_mac_llc_snap_header*)(d))->oui[2] =
				BRIDGE_TUNNEL_OUI2;
		}

		/* Include the LLC_SNAP into skb data payload from headroom, for dma_map */
		pktdata -= DOT11_LLC_SNAP_HDR_LEN;
		pktlen += DOT11_LLC_SNAP_HDR_LEN;
	}
	else {
		/* Extract the ethernet header and adjust the data pointer and length */
		pktdata += ETHER_HDR_LEN;
		pktlen -= ETHER_HDR_LEN;
	}

	/* Map the data pointer to a DMA-able address */
	/* Also, cache Flush including SubFrame Header */
#ifdef BCM_SECURE_DMA
		pa = SECURE_DMA_MAP(dhd->osh, pktdata, pktlen,
			DMA_TX, PKTBUF, 0, ring->dma_buf.secdma, 0, SECDMA_TXBUF_POST);
#else
		pa = DMA_MAP(dhd->osh, pktdata, pktlen, DMA_TX, PKTBUF, 0);
#endif /* BCM_SECURE_DMA */

#ifndef BCM_SECURE_DMA
	if ((PHYSADDRHI(pa) == 0) && (PHYSADDRLO(pa) == 0)) {
		DHD_ERROR(("%s: Something really bad, unless 0 is a valid phyaddr\n",
			dhd_ifname(dhd, ifidx)));
		ASSERT(0);
	}
#endif /* BCM_SECURE_DMA */

	/* No need to lock. Save the rest of the packet's metadata */
	DHD_NATIVE_TO_PKTID_SAVE(dhd, prot->pktid_map_handle, PKTBUF, pktid,
	    pa, pktlen, DMA_TX, NULL, ring->dma_buf.secdma, PKTTYPE_DATA_TX);

#ifdef TXP_FLUSH_NITEMS
	if (ring->pend_items_count == 0) {
		ring->start_addr = txp_wi_start;
	}
	ring->pend_items_count++;
#endif /* TXP_FLUSH_NITEMS */

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, prot->pktid_map_handle, pktid,
		DHD_DUPLICATE_ALLOC);
#endif /* DHD_PKTID_AUDIT_RING */

	/* Form the Tx descriptor message buffer */
	DHD_TRACE(("txpost: data_len %d, pktid 0x%04x\n", pktlen, pktid));

	/* Some common fields in legacy, cwi32 or cwi64 format */

	txp_flags  = BCMPCIE_PKT_FLAGS_FRAME_802_3;

	prio = (uint8)PKTPRIO(PKTBUF);

	/* Pass the actual eth data payload to the firmware */
	if (DHDHDR_SUPPORT(dhd)) {
#ifdef BCMDMA64OSL
		PHYSADDRTOULONG(pa, paddr);
		paddr += (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);
		ULONGTOPHYSADDR(paddr, pa);
#else
		pa += (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);
#endif /* BCMDMA64OSL */
		pktlen -= (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);
	}

	pktid = htol32(pktid);
	pktlen = htol16((uint16) pktlen);

	/* Compose the rest of the TxPost work item (etherhdr done) */
	switch (ring->item_type)
	{
		case MSGBUF_WI_WI64: /* Legacy Work item */
		{
			host_txbuf_post_t *wi64 = (host_txbuf_post_t *)txp_wi_start;
			wi64->cmn_hdr.msg_type = MSG_TYPE_TX_POST;
			wi64->cmn_hdr.request_id = pktid;
			wi64->cmn_hdr.if_id = ifidx;
			wi64->flags = txp_flags |
				(prio & 0x7) << BCMPCIE_PKT_FLAGS_PRIO_SHIFT;
			wi64->seg_cnt = 1;
			wi64->data_len = pktlen;
			HADDR64_LO_SET_HTOL(wi64->data_buf_haddr64, PHYSADDRLO(pa));
			HADDR64_HI_SET_HTOL(wi64->data_buf_haddr64, PHYSADDRHI(pa));
			// wi64->metadata_buf_len = htol16(0);
			// HADDR64_LO_SET_HTOL(wi64->metadata_buf_haddr64, 0);
			// HADDR64_HI_SET_HTOL(wi64->metadata_buf_haddr64, 0);
			break;
		}

		case MSGBUF_WI_CWI32:
		{
			hwa_txpost_cwi32_t *cwi32 = (hwa_txpost_cwi32_t *)txp_wi_start;
			cwi32->host_pktid = pktid;
			cwi32->u32[1] =
				BCM_SBF(ifidx, HWA_TXPOST_IFID)
				| BCM_SBF(prio, HWA_TXPOST_PRIO)
				// | BCM_SBF(0, HWA_TXPOST_COPY)
				| BCM_SBF(txp_flags, HWA_TXPOST_FLAGS)
				| BCM_SBF(pktlen, HWA_TXPOST_DATA_BUF_HLEN);
			cwi32->data_buf_haddr32 = htol32(PHYSADDRLO(pa));
			ASSERT(PHYSADDRHI(pa) == prot->host_physaddrhi);
			break;
		}

		case MSGBUF_WI_CWI64:
		{
			hwa_txpost_cwi64_t *cwi64 = (hwa_txpost_cwi64_t *)txp_wi_start;
			cwi64->host_pktid = pktid;
			cwi64->u32[1] =
				BCM_SBF(ifidx, HWA_TXPOST_IFID)
				| BCM_SBF(prio, HWA_TXPOST_PRIO)
				// | BCM_SBF(0, HWA_TXPOST_COPY)
				| BCM_SBF(txp_flags, HWA_TXPOST_FLAGS)
				| BCM_SBF(pktlen, HWA_TXPOST_DATA_BUF_HLEN);
			HADDR64_LO_SET_HTOL(cwi64->data_buf_haddr64, PHYSADDRLO(pa));
			HADDR64_HI_SET_HTOL(cwi64->data_buf_haddr64, PHYSADDRHI(pa));
			break;
		}

		default:
			DHD_ERROR(("%s: %s Invalid item_type %u\n",
				dhd_ifname(dhd, ifidx), __FUNCTION__,
				ring->item_type));
			ASSERT(0);

	} /* switch item_type */

	/* Update the write pointer in TCM & ring bell */
#ifdef TXP_FLUSH_NITEMS
	/* Flush if we have either hit the txp_threshold or if this msg is */
	/* occupying the last slot in the flow_ring - before wrap around.  */
	if ((ring->pend_items_count == prot->txp_threshold) ||
		((uint8 *) txp_wi_start == (uint8 *) DHD_RING_END_VA(ring))) {
		dhd_prot_txdata_write_flush(dhd, flowid, TRUE);
	}
#else
	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_flow_ring_write_complete(dhd, flow_ring_node, txp_wi_start, 1);

#endif

	prot->active_tx_count++;

	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_OK;

err_no_res_pktfree:

	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_NORESOURCE;

} /* dhd_prot_txdata */

/* called with a lock */
/** optimization to write "n" tx items at a time to ring */
void BCMFASTPATH
dhd_prot_txdata_write_flush(dhd_pub_t *dhd, uint16 flowid, bool in_lock)
{
#ifdef TXP_FLUSH_NITEMS
	unsigned long flags = 0;
	flow_ring_table_t *flow_ring_table;
	flow_ring_node_t *flow_ring_node;
	msgbuf_ring_t *ring;

	if (dhd->flow_ring_table == NULL) {
		return;
	}

	if (!in_lock) {
		DHD_GENERAL_LOCK(dhd, flags);
	}

	flow_ring_table = (flow_ring_table_t *)dhd->flow_ring_table;
	flow_ring_node = (flow_ring_node_t *)&flow_ring_table[flowid];
	ring = (msgbuf_ring_t *)flow_ring_node->prot_info;

	if (ring->pend_items_count) {
		/* update ring's WR index and ring doorbell to dongle */
		dhd_prot_flow_ring_write_complete(dhd, flow_ring_node, ring->start_addr,
			ring->pend_items_count);
		ring->pend_items_count = 0;
		ring->start_addr = NULL;
	}

	if (!in_lock) {
		DHD_GENERAL_UNLOCK(dhd, flags);
	}
#endif /* TXP_FLUSH_NITEMS */
}

#undef PKTBUF	/* Only defined in the above routine */

int BCMFASTPATH
dhd_prot_hdrpull(dhd_pub_t *dhd, int *ifidx, void *pkt, uchar *buf, uint *len)
{
	return 0;
}

/** post a set of receive buffers to the dongle */
static void BCMFASTPATH
dhd_prot_return_rxbuf(dhd_pub_t *dhd, uint32 pktid, uint32 rxcnt)
{
	dhd_prot_t *prot = dhd->prot;
#if defined(DHD_LB_RXC)
	int elem_ix;
	uint32 *elem;
	bcm_workq_t *workq;

	workq = &prot->rx_compl_prod;

	/* Produce the work item */
	elem_ix = bcm_ring_prod(WORKQ_RING(workq), DHD_LB_WORKQ_SZ);
	if (elem_ix == BCM_RING_FULL) {
		DHD_ERROR(("%s LB RxCompl workQ is full\n", __FUNCTION__));
		ASSERT(0);
		return;
	}

	elem = WORKQ_ELEMENT(uint32, workq, elem_ix);
	*elem = pktid;

	smp_wmb();

	/* Sync WR index to consumer if the SYNC threshold has been reached */
	if (++prot->rx_compl_prod_sync >= DHD_LB_WORKQ_SYNC) {
		bcm_workq_prod_sync(workq);
		prot->rx_compl_prod_sync = 0;
	}

	DHD_INFO(("%s: rx_compl_prod pktid<%u> sync<%d>\n",
		__FUNCTION__, pktid, prot->rx_compl_prod_sync));

#endif /* DHD_LB_RXC */

	if (prot->rxbufpost >= rxcnt) {
		prot->rxbufpost -= rxcnt;
	} else {
		/* XXX: I have seen this assert hitting.
		 * Will be removed once rootcaused.
		 */
		/* ASSERT(0); */
		prot->rxbufpost = 0;
	}

#if !defined(DHD_LB_RXC)
	if (prot->rxbufpost <= (prot->max_rxbufpost - RXBUFPOST_THRESHOLD)) {
		dhd_msgbuf_rxbuf_post(dhd, FALSE); /* alloc pkt ids */
	}
#endif /* !DHD_LB_RXC */
}

/* called before an ioctl is sent to the dongle */
static void
dhd_prot_wlioctl_intercept(dhd_pub_t *dhd, wl_ioctl_t * ioc, void * buf)
{
	dhd_prot_t *prot = dhd->prot;

	if (ioc->cmd == WLC_SET_VAR && buf != NULL && !strcmp(buf, "pcie_bus_tput")) {
		int slen = 0;
		pcie_bus_tput_params_t *tput_params;

		slen = strlen("pcie_bus_tput") + 1;
		tput_params = (pcie_bus_tput_params_t*)((char *)buf + slen);
		bcopy(&prot->bus_tput_dma_buf.pa, &tput_params->host_buf_addr,
			sizeof(tput_params->host_buf_addr));
		tput_params->host_buf_len = DHD_BUS_TPUT_BUF_LEN;
	}
}

#if defined(CUSTOMER_HW4) && defined(CONFIG_CONTROL_PM)
extern bool g_pm_control;
#endif /* CUSTOMER_HW4 & CONFIG_CONTROL_PM */

/**
 * Use protocol to issue ioctl to dongle. Only one ioctl may be in transit.
 *
 * @param[out] buf  Contains output from dongle on function exit, is completely overwritten.
 */
int
dhd_prot_ioctl(dhd_pub_t *dhd, int ifidx, wl_ioctl_t *ioc, void *buf, int len)
{
	int ret = -1;
	uint8 action;

	if ((dhd->busstate == DHD_BUS_DOWN) || dhd->hang_was_sent) {
		DHD_ERROR(("%s: %s : bus is down. we have nothing to do\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__));
		goto done;
	}

	if (dhd->busstate == DHD_BUS_SUSPENDING || dhd->busstate == DHD_BUS_SUSPENDED) {
		DHD_ERROR(("%s: %s : bus is suspended\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__));
		goto done;
	}

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if( !getintvar(NULL, "debug_wl") && ioc->cmd==WLC_CURRENT_PWR) {
		DHD_ERROR(("%s: unexpected wl cmd\n", __FUNCTION__));
		return BCME_ERROR;
	}

#ifdef CUSTOMER_HW4
	if (ioc->cmd == WLC_SET_PM) {
#ifdef CONFIG_CONTROL_PM
		if (g_pm_control == TRUE) {
			DHD_ERROR(("%s: %s: SET PM ignored!(Requested:%d)\n",
				dhd_ifname(dhd, ifidx), __FUNCTION__, *(char *)buf));
			goto done;
		}
#endif /* CONFIG_CONTROL_PM */
		DHD_ERROR(("%s: %s: SET PM to %d\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__, *(char *)buf));
	}
#endif /* CUSTOMER_HW4 */

	ASSERT(len <= WLC_IOCTL_MAXLEN);

	if (len > WLC_IOCTL_MAXLEN) {
		goto done;
	}

	action = ioc->set;

	dhd_prot_wlioctl_intercept(dhd, ioc, buf);

	if (action & WL_IOCTL_ACTION_SET) {
		ret = dhd_msgbuf_set_ioctl(dhd, ifidx, ioc->cmd, buf, len, action);
	} else {
		ret = dhd_msgbuf_query_ioctl(dhd, ifidx, ioc->cmd, buf, len, action);
		if (ret > 0) {
			ioc->used = ret;
		}
	}

	/* Too many programs assume ioctl() returns 0 on success */
	if (ret >= 0) {
		ret = 0;
	} else {
#ifndef DETAIL_DEBUG_LOG_FOR_IOCTL
		if (ioc->cmd == WLC_GET_VAR && buf != NULL) {
			DHD_ERROR(("%s: %s: status ret value is %d iov=%s\n",
				dhd_ifname(dhd, ifidx), __FUNCTION__, ret, (char*)buf));
		} else {
			DHD_ERROR(("%s: %s: status ret value is %d cmd=%d\n",
				dhd_ifname(dhd, ifidx), __FUNCTION__, ret, ioc->cmd));
		}
#endif /* !DETAIL_DEBUG_LOG_FOR_IOCTL */
		dhd->dongle_error = ret;
	}

	if (!ret && ioc->cmd == WLC_SET_VAR && buf != NULL) {
		/* Intercept the wme_dp ioctl here */
		if (!strcmp(buf, "wme_dp")) {
			int slen, val = 0;

			slen = strlen("wme_dp") + 1;
			if (len >= (int)(slen + sizeof(int))) {
				bcopy(((char *)buf + slen), &val, sizeof(int));
			}
			dhd->wme_dp = (uint8) ltoh32(val);
		}
	}

done:
	return ret;
} /* dhd_prot_ioctl */

/** test / loopback */

/*
 * XXX: This will fail with new PCIe Split header Full Dongle using fixed
 * sized messages in control submission ring. We seem to be sending the lpbk
 * data via the control message, wherein the lpbk data may be larger than 1
 * control message that is being committed.
 */
int
dhdmsgbuf_lpbk_req(dhd_pub_t *dhd, uint len)
{
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;
	uint16 alloced = 0;

	ioct_reqst_hdr_t *ioct_rqst;

	uint16 hdrlen = sizeof(ioct_reqst_hdr_t);
	uint16 msglen = len + hdrlen;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	msglen = ALIGN_SIZE(msglen, DHD_MEM_ALIGNMENT_MIN);
	msglen = LIMIT_TO_MAX(msglen, MSGBUF_MAX_MSG_SIZE);

	DHD_GENERAL_LOCK(dhd, flags);

	ioct_rqst = (ioct_reqst_hdr_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);

	if (ioct_rqst == NULL) {
		DHD_GENERAL_UNLOCK(dhd, flags);
		return 0;
	}

	{
		uint8 *ptr;
		uint16 i;

		ptr = (uint8 *)ioct_rqst;
		for (i = 0; i < msglen; i++) {
			ptr[i] = i % 256;
		}
	}

	/* Common msg buf hdr */
	ioct_rqst->msg.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

	ioct_rqst->msg.msg_type = MSG_TYPE_LOOPBACK;
	ioct_rqst->msg.if_id = 0;

	bcm_print_bytes("LPBK REQ: ", (uint8 *)ioct_rqst, msglen);

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, ioct_rqst, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return 0;
}

/** test / loopback */
void
dmaxfer_free_dmaaddr(dhd_pub_t *dhd, dhd_dmaxfer_t *dmaxfer)
{
	dhd_dma_buf_free(dhd, &dmaxfer->src_dma_buf, "dmaxfer_src");
	dhd_dma_buf_free(dhd, &dmaxfer->dst_dma_buf, "dmaxfer_dst");
}

/** test / loopback */
int
dmaxfer_prepare_dmaaddr(dhd_pub_t *dhd, uint len,
	uint srcdelay, uint destdelay, dhd_dmaxfer_t *dmaxfer)
{
	uint i;

	/* First free up existing buffers */
	dmaxfer_free_dmaaddr(dhd, dmaxfer);

	if (dhd_dma_buf_alloc(dhd, &dmaxfer->src_dma_buf, "dmaxfer_src",
	                      len, DHD_MEM_ALIGN_BITS_MIN)) {
		return BCME_NOMEM;
	}

	if (dhd_dma_buf_alloc(dhd, &dmaxfer->dst_dma_buf, "dmaxfer_dst",
	                      len + 8, DHD_MEM_ALIGN_BITS_MIN)) {
		dhd_dma_buf_free(dhd, &dmaxfer->src_dma_buf, "dmaxfer_src");
		return BCME_NOMEM;
	}

	dmaxfer->len = len;

	/* Populate source with a pattern */
	for (i = 0; i < dmaxfer->len; i++) {
		((uint8*)dmaxfer->src_dma_buf.va)[i] = i % 256;
	}
	OSL_CACHE_FLUSH(dmaxfer->src_dma_buf.va, dmaxfer->len);

	dmaxfer->srcdelay = srcdelay;
	dmaxfer->destdelay = destdelay;

	return BCME_OK;
} /* dmaxfer_prepare_dmaaddr */

static void
dhd_msgbuf_dmaxfer_process(dhd_pub_t *dhd, void *msg)
{
	dhd_dmaxfer_t *dmaxfer = &dhd->prot->dmaxfer;

	OSL_CACHE_INV(dmaxfer->dst_dma_buf.va, dmaxfer->len);

	if (dmaxfer->src_dma_buf.va && dmaxfer->dst_dma_buf.va) {
		if (memcmp(dmaxfer->src_dma_buf.va, dmaxfer->dst_dma_buf.va, dmaxfer->len)) {
			bcm_print_bytes("XFER SRC: ", dmaxfer->src_dma_buf.va, dmaxfer->len);
			bcm_print_bytes("XFER DST: ", dmaxfer->dst_dma_buf.va, dmaxfer->len);
		} else {
			DHD_INFO(("DMA successful\n"));
		}
	}
	dmaxfer_free_dmaaddr(dhd, dmaxfer);
	dmaxfer->in_progress = FALSE;
}

/** Test functionality.
 * Transfers bytes from host to dongle and to host again using DMA
 * This function is not reentrant, as prot->dmaxfer.in_progress is not protected
 * by a spinlock.
 */
int
dhdmsgbuf_dmaxfer_req(dhd_pub_t *dhd, uint len, uint srcdelay, uint destdelay)
{
	unsigned long flags;
	int ret = BCME_OK;
	pcie_dma_xfer_params_t *dmap;
	uint32 dmaxfer_len = LIMIT_TO_MAX(len, DHD_DMAXFER_LEN_LIMIT);
	uint16 alloced = 0;
	msgbuf_ring_t *ring = &dhd->prot->h2dring_ctrl_subn;
	dhd_dmaxfer_t *dmaxfer = &dhd->prot->dmaxfer;

	if (dmaxfer->in_progress) {
		DHD_ERROR(("DMA is in progress...\n"));
		return ret;
	}

	dmaxfer->in_progress = TRUE;
	if ((ret = dmaxfer_prepare_dmaaddr(dhd, dmaxfer_len, srcdelay, destdelay,
	        dmaxfer)) != BCME_OK) {
		dmaxfer->in_progress = FALSE;
		return ret;
	}

	DHD_GENERAL_LOCK(dhd, flags);

	dmap = (pcie_dma_xfer_params_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);

	if (dmap == NULL) {
		dmaxfer_free_dmaaddr(dhd, dmaxfer);
		dmaxfer->in_progress = FALSE;
		DHD_GENERAL_UNLOCK(dhd, flags);
		return BCME_NOMEM;
	}

	/* Common msg buf hdr */
	dmap->cmn_hdr.msg_type = MSG_TYPE_LPBK_DMAXFER;
	dmap->cmn_hdr.request_id = htol32(DHD_FAKE_PKTID);
	dmap->cmn_hdr.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

	dmap->host_input_buf_addr.high = htol32(PHYSADDRHI(dmaxfer->src_dma_buf.pa));
	dmap->host_input_buf_addr.low  = htol32(PHYSADDRLO(dmaxfer->src_dma_buf.pa));
	dmap->host_ouput_buf_addr.high = htol32(PHYSADDRHI(dmaxfer->dst_dma_buf.pa));
	dmap->host_ouput_buf_addr.low  = htol32(PHYSADDRLO(dmaxfer->dst_dma_buf.pa));
	dmap->xfer_len = htol32(dmaxfer->len);
	dmap->srcdelay = htol32(dmaxfer->srcdelay);
	dmap->destdelay = htol32(dmaxfer->destdelay);

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, dmap, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	DHD_ERROR(("DMA Started...\n"));

	return BCME_OK;
} /* dhdmsgbuf_dmaxfer_req */

/**
 * Called in the process of 'getting' an iovar or ioctl from the dongle
 *
 * @param buf[inout] Carries e.g. iovar name on function entry
 */
static int
dhd_msgbuf_query_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd, void *buf, uint len, uint8 action)
{
	int ret = 0;
	dhd_prot_t *prot = dhd->prot;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* Respond "bcmerror" and "bcmerrorstr" with local cache */
	if (cmd == WLC_GET_VAR && buf)
	{
		if (!strcmp((char *)buf, "bcmerrorstr"))
		{
			strncpy((char *)buf, bcmerrorstr(dhd->dongle_error), BCME_STRLEN);
			goto done;
		}
		else if (!strcmp((char *)buf, "bcmerror"))
		{
			*(int *)buf = dhd->dongle_error;
			goto done;
		}
	}

	if (prot->cur_ioctlresp_bufs_posted) {
		ret = dhd_fillup_ioct_reqst(dhd, (uint16)len, cmd, buf, ifidx);

		DHD_CTL(("query_ioctl: ACTION %d ifdix %d cmd %d len %d\n",
			action, ifidx, cmd, len));

		if (ret == BCME_OK) {
			prot->cur_ioctlresp_bufs_posted--;
			/* wait for IOCTL completion message from dongle and get first fragment */
			ret = dhd_msgbuf_wait_ioctl_cmplt(dhd, len, buf);
		}
	} else {
		ret = -EIO;
	}

	dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd);

done:
	return ret;
}

/**
 * Waits for IOCTL completion message from the dongle and copies this into caller provided parameter
 * 'buf'. In case of a WLC_GET_VAR, the variable name in 'buf' on function entry is not preserved.
 *
 * @param[out]  buf  Overwritten from the start with dongle output.
 */
static int
dhd_msgbuf_wait_ioctl_cmplt(dhd_pub_t *dhd, uint32 len, void *buf)
{
	dhd_prot_t *prot = dhd->prot;
	int timeleft;
	unsigned long flags;
	int ret = 0;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhd->dongle_reset) {
		ret = -EIO;
		goto out;
	}

	timeleft = dhd_os_ioctl_resp_wait(dhd, &prot->ioctl_received);
	if (timeleft == 0) {
		dhd->rxcnt_timeout++;
		dhd->rx_ctlerrs++;
		DHD_ERROR(("%s: resumed on timeout rxcnt_timeout %d\n",
			__FUNCTION__, dhd->rxcnt_timeout));
#if defined(DHD_FW_COREDUMP) && defined(CUSTOMER_HW4)
		if (dhd->memdump_enabled) {
			/* write core dump to file */
			dhd_bus_mem_dump(dhd);
		}
#endif /* DHD_FW_COREDUMP && CUSTOMER_HW4 */
		if (dhd->rxcnt_timeout > MAX_CNTL_RX_TIMEOUT) {
#ifdef SUPPORT_LINKDOWN_RECOVERY
#ifdef CONFIG_ARCH_MSM
			dhd->bus->islinkdown = 1;
#endif /* CONFIG_ARCH_MSM */
#endif /* SUPPORT_LINKDOWN_RECOVERY */
			DHD_ERROR(("%s: timeout > MAX_CNTL_RX_TIMEOUT\n", __FUNCTION__));
			dhd->busstate = DHD_BUS_DOWN;
			dhd_schedule_trap_log_dump(dhd, false);
		}
		ret = -ETIMEDOUT;
		goto out;
	} else {
		dhd->rxcnt_timeout = 0;
		dhd->rx_ctlpkts++;
		DHD_CTL(("%s: ioctl resp resumed, got %d\n",
			__FUNCTION__, prot->ioctl_resp_len));
	}

	if (dhd->dongle_trap_occured) {
#ifdef SUPPORT_LINKDOWN_RECOVERY
#ifdef CONFIG_ARCH_MSM
		dhd->bus->islinkdown = 1;
#endif /* CONFIG_ARCH_MSM */
#endif /* SUPPORT_LINKDOWN_RECOVERY */
		DHD_ERROR(("%s: TRAP occurred!!\n", __FUNCTION__));
		ret = -EREMOTEIO;
		goto out;
	}

	if (prot->ioctl_resp_len > len) {
		prot->ioctl_resp_len = (uint16)len;
	}
	if (buf) {
		bcopy(prot->ioctl_resp_dma_buf.va, buf, prot->ioctl_resp_len);
	}

	ret = (int)(prot->ioctl_status);
out:
	DHD_GENERAL_LOCK(dhd, flags);
	prot->ioctl_state = 0;
	prot->ioctl_resp_len = 0;
	prot->ioctl_received = 0;
	DHD_GENERAL_UNLOCK(dhd, flags);

	return ret;
} /* dhd_msgbuf_wait_ioctl_cmplt */

/** Sends an ioctl/iovar to the dongle */
static int
dhd_msgbuf_set_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd, void *buf, uint len, uint8 action)
{
	int ret = 0;
	dhd_prot_t *prot = dhd->prot;

	DHD_TRACE(("%s: Enter \n", __FUNCTION__));

	if (dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: %s : bus is down. we have nothing to do\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__));
		return -EIO;
	}

	/* don't talk to the dongle if fw is about to be reloaded */
	if (dhd->hang_was_sent) {
		DHD_ERROR(("%s: %s: HANG was sent up earlier. Not talking to the chip\n",
			dhd_ifname(dhd, ifidx), __FUNCTION__));
		return -EIO;
	}

	/* Fill up msgbuf for ioctl req */
	if (prot->cur_ioctlresp_bufs_posted) {
		ret = dhd_fillup_ioct_reqst(dhd, (uint16)len, cmd, buf, ifidx);

		DHD_CTL(("ACTION %d ifdix %d cmd %d len %d \n",
			action, ifidx, cmd, len));

		if (ret == BCME_OK) {
			prot->cur_ioctlresp_bufs_posted--;
			ret = dhd_msgbuf_wait_ioctl_cmplt(dhd, len, buf);
		}
	} else {
		ret = -EIO;
	}

	dhd_msgbuf_rxbuf_post_ioctlresp_bufs(dhd);

	return ret;
}

/** Called by upper DHD layer. Handles a protocol control response asynchronously. */
int
dhd_prot_ctl_complete(dhd_pub_t *dhd)
{
	return 0;
}

/** Called by upper DHD layer. Check for and handle local prot-specific iovar commands */
int
dhd_prot_iovar_op(dhd_pub_t *dhd, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	return BCME_UNSUPPORTED;
}

/** Add prot dump output to a buffer */
void
dhd_prot_dump(dhd_pub_t *dhd, struct bcmstrbuf *b)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *rx_cpln;
	uint32 i;

#if defined(BCM_ROUTER_DHD)
#if defined(BCM_GMAC3)
	bcm_bprintf(b, "DHD Router: 3GMAC forwarding mode\n");
#else
	bcm_bprintf(b, "DHD Router: 1GMAC HotBRC forwarding mode\n");
#endif /* BCM_GMAC3 */
#endif /* BCM_ROUTER_DHD */

#if defined(BCM_DHDHDR) && defined(PCIE_FULL_DONGLE)
	if (DHDHDR_SUPPORT(dhd))
		bcm_bprintf(b, "DHDHDR supported\n");
#endif
	bcm_bprintf(b,
		"WI Format: haddr64=%u physaddrhi=0x%08x"
		" TxP=%u,%u RxP=%u,%u TxC=%u,%u",
		prot->use_haddr64, prot->host_physaddrhi,
		prot->txpost_item_type, prot->txpost_item_size,
		prot->h2dring_rxp_subn.item_type, prot->h2dring_rxp_subn.item_size,
		prot->d2hring_tx_cpln.item_type, prot->d2hring_tx_cpln.item_size);

	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, rx_cpln, i) {
		bcm_bprintf(b, " RxC%d=%u,%u", i, rx_cpln->item_type, rx_cpln->item_size);
	}
	bcm_bprintf(b, "\n");

#if defined(PCIE_D2H_SYNC)
	if (dhd->d2h_sync_mode & PCIE_IPC_FLAGS_D2H_SYNC_SEQNUM)
		bcm_bprintf(b, "D2H SYNC: SEQNUM:");
	else if (dhd->d2h_sync_mode & PCIE_IPC_FLAGS_D2H_SYNC_XORCSUM)
		bcm_bprintf(b, "D2H SYNC: XORCSUM:");
	else
		bcm_bprintf(b, "D2H SYNC: NONE:");
	bcm_bprintf(b, " d2h_sync_wait max<%lu> tot<%lu>\n",
		prot->d2h_sync_wait_max, prot->d2h_sync_wait_tot);
#endif  /* PCIE_D2H_SYNC */

	bcm_bprintf(b, "Dongle DMA Indices: h2d %d  d2h %d index size %d bytes\n",
		DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support),
		DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support),
		prot->rw_index_sz);

#if defined(BCM_DHD_RUNNER)
	dhd_runner_do_iovar(dhd->runner_hlp, DHD_RNR_IOVAR_DUMP, 0, (char*)b, b->size);
#endif
}

/* Update local copy of dongle statistics */
void
dhd_prot_dstats(dhd_pub_t *dhd)
{
	return;
}

/** Called by upper DHD layer */
int
dhd_process_pkt_reorder_info(dhd_pub_t *dhd, uchar *reorder_info_buf,
	uint reorder_info_len, void **pkt, uint32 *free_buf_count)
{
	return 0;
}

/** Debug related, post a dummy message to interrupt dongle. Used to process cons commands. */
int
dhd_post_dummy_msg(dhd_pub_t *dhd)
{
	unsigned long flags;
	hostevent_hdr_t *hevent = NULL;
	uint16 alloced = 0;

	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	DHD_GENERAL_LOCK(dhd, flags);

	hevent = (hostevent_hdr_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);

	if (hevent == NULL) {
		DHD_GENERAL_UNLOCK(dhd, flags);
		return -1;
	}

	/* CMN msg header */
	hevent->msg.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;
	hevent->msg.msg_type = MSG_TYPE_HOST_EVNT;
	hevent->msg.if_id = 0;

	/* Event payload */
	hevent->evnt_pyld = htol32(HOST_EVENT_CONS_CMD);

	/* Since, we are filling the data directly into the bufptr obtained
	 * from the msgbuf, we can directly call the write_complete
	 */
	dhd_prot_ring_write_complete(dhd, ring, hevent, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return 0;
}

/**
 * If exactly_nitems is true, this function will allocate space for nitems or fail
 * If exactly_nitems is false, this function will allocate space for nitems or less
 */
static void * BCMFASTPATH
dhd_prot_alloc_ring_space(dhd_pub_t *dhd, msgbuf_ring_t *ring,
	uint16 nitems, uint16 * alloced, bool exactly_nitems)
{
	void * ret_buf;

	/* Alloc space for nitems in the ring */
	ret_buf = dhd_prot_get_ring_space(ring, nitems, alloced, exactly_nitems);

	if (ret_buf == NULL) {
		/* if alloc failed , invalidate cached read ptr */
		if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
			ring->rd = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_RD_UPD, ring->id);
		} else {
			dhd_bus_cmn_readshared(dhd->bus, &(ring->rd), RING_RD_UPD, ring->id);
		}

		/* Try allocating once more */
		ret_buf = dhd_prot_get_ring_space(ring, nitems, alloced, exactly_nitems);

		if (ret_buf == NULL) {
			DHD_INFO(("%s: Ring space not available  \n", ring->name));
			return NULL;
		}
	}

	/* Return alloced space */
	return ret_buf;
}

/**
 * Non inline ioct request.
 * Form a ioctl request first as per ioctptr_reqst_hdr_t header in the circular buffer
 * Form a separate request buffer where a 4 byte cmn header is added in the front
 * buf contents from parent function is copied to remaining section of this buffer
 *
 * @param[in] buf  buffer
 */
static int
dhd_fillup_ioct_reqst(dhd_pub_t *dhd, uint16 len, uint cmd, void* buf, int ifidx)
{
	dhd_prot_t *prot = dhd->prot;
	ioctl_req_msg_t *ioctl_req_msg;
	void * ioct_buf;	/* For ioctl payload */
	uint16  rqstlen, resplen;
	unsigned long flags;
	uint16 alloced = 0;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	rqstlen = len;
	resplen = len;

	/* Limit ioct request to MSGBUF_MAX_MSG_SIZE bytes including hdrs */
	/* 8K allocation of dongle buffer fails */
	/* dhd doesnt give separate input & output buf lens */
	/* so making the assumption that input length can never be more than 2.5k */
	rqstlen = MIN(rqstlen, MSGBUF_IOCTL_MAX_RQSTLEN(dhd->bus->pcie_ipc_revision));

	DHD_GENERAL_LOCK(dhd, flags);

	if (prot->ioctl_state) {
		DHD_CTL(("pending ioctl %02x\n", prot->ioctl_state));
		DHD_GENERAL_UNLOCK(dhd, flags);
		return BCME_BUSY;
	} else {
		prot->ioctl_state = MSGBUF_IOCTL_ACK_PENDING | MSGBUF_IOCTL_RESP_PENDING;
	}

	/* Request for cbuf space */
	ioctl_req_msg = (ioctl_req_msg_t*)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);
	if (ioctl_req_msg == NULL) {
		DHD_ERROR(("%s: couldn't allocate space on msgring to send ioctl request\n",
			dhd_ifname(dhd, ifidx)));
		prot->ioctl_state = 0;
		DHD_GENERAL_UNLOCK(dhd, flags);
		return -1;
	}

	/* Common msg buf hdr */
	ioctl_req_msg->cmn_hdr.msg_type = MSG_TYPE_IOCTLPTR_REQ;
	ioctl_req_msg->cmn_hdr.if_id = (uint8)ifidx;
	ioctl_req_msg->cmn_hdr.flags = 0;
	ioctl_req_msg->cmn_hdr.request_id = htol32(DHD_IOCTL_REQ_PKTID);
	ioctl_req_msg->cmn_hdr.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

	ioctl_req_msg->cmd = htol32(cmd);
	ioctl_req_msg->output_buf_len = htol16(resplen);
	prot->ioctl_trans_id++;
	ioctl_req_msg->trans_id = prot->ioctl_trans_id;

	/* populate ioctl buffer info */
	ioctl_req_msg->input_buf_len = htol16(rqstlen);
	ioctl_req_msg->host_input_buf_addr.high
		= htol32(PHYSADDRHI(prot->ioctl_rqst_dma_buf.pa));
	ioctl_req_msg->host_input_buf_addr.low
		= htol32(PHYSADDRLO(prot->ioctl_rqst_dma_buf.pa));
	/* copy ioctl payload */
	ioct_buf = (void *) prot->ioctl_rqst_dma_buf.va;

	if (buf) {
		memcpy(ioct_buf, buf, len); /* ioct_buf <- cpy <- buf */
	}

	OSL_CACHE_FLUSH((void *) prot->ioctl_rqst_dma_buf.va, len);

	if (!ISALIGNED(ioct_buf, DHD_MEM_ALIGNMENT_MIN)) {
		DHD_ERROR(("host ioct address unaligned !!!!! \n"));
	}

	DHD_CTL(("submitted IOCTL request request_id %d, cmd %d, output_buf_len %d, tx_id %d\n",
		ioctl_req_msg->cmn_hdr.request_id, cmd, ioctl_req_msg->output_buf_len,
		ioctl_req_msg->trans_id));

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, ioctl_req_msg, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_OK;
} /* dhd_fillup_ioct_reqst */

/**
 * dhd_prot_ring_alloc - Initialize the msgbuf_ring object and attach a
 * DMA-able buffer to it. The ring is NOT tagged as inited until all the ring
 * information is posted to the dongle.
 *
 * returns BCME_OK=0 on success
 * returns non-zero negative error value on failure.
 */
static int
dhd_prot_ring_alloc(dhd_pub_t *dhd, msgbuf_ring_t *ring, const char *name,
	uint16 ringid, uint16 item_type, uint16 item_size,
	uint16 max_items, uint16 item_misc)
{
	int dma_buf_alloced = BCME_NOMEM;
	uint32 dma_buf_len = max_items * item_size;
	dhd_prot_t *prot = dhd->prot;

	ASSERT(ring);
	ASSERT(name);
	ASSERT(item_type <= MSGBUF_WI_ACWI64);
	if (DHD_IS_FLOWRING(ringid)) {
		ASSERT((item_misc != 0) && (item_misc < item_size)); // offset of ethhdr
	} else if (ringid == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) {
		ASSERT((item_misc == 0) || (item_misc == 2)); // lr_shift by 0 or 2
	}

	/* Init name */
	strncpy(ring->name, name, RING_NAME_MAX_LENGTH);
	ring->name[RING_NAME_MAX_LENGTH - 1] = '\0';

	ring->id = ringid;

	ring->item_type = item_type;
	ring->item_size = item_size;
	ring->max_items = max_items;
	ring->item_misc = item_misc;

	/* A contiguous space may be reserved for all flowrings */
	if (DHD_IS_FLOWRING(ringid) && (prot->flowrings_dma_buf.va)) {
		/* Carve out from the contiguous DMA-able flowring buffer */
		uint16 flowid;
		uint32 base_offset;

		dhd_dma_buf_t *dma_buf = &ring->dma_buf;
		dhd_dma_buf_t *rsv_buf = &prot->flowrings_dma_buf;

		flowid = DHD_RINGID_TO_FLOWID(ringid);
		base_offset = (flowid - BCMPCIE_H2D_COMMON_MSGRINGS) * dma_buf_len;

		ASSERT(base_offset + dma_buf_len <= rsv_buf->len);

		dma_buf->len = dma_buf_len;
		dma_buf->va = (void *)((uintptr)rsv_buf->va + base_offset);
		PHYSADDRHISET(dma_buf->pa, PHYSADDRHI(rsv_buf->pa));
		PHYSADDRLOSET(dma_buf->pa, PHYSADDRLO(rsv_buf->pa) + base_offset);

		/* On 64bit, contiguous space may not span across 0x00000000FFFFFFFF */
		ASSERT(PHYSADDRLO(dma_buf->pa) >= PHYSADDRLO(rsv_buf->pa));

		dma_buf->dmah   = rsv_buf->dmah;
		dma_buf->secdma = rsv_buf->secdma;

		/* This dhd_dma_buf is not allocated by dhd_dma_buf_alloc()
		 * and, dhd_dma_buf::_alloced = 0, dhd_dma_buf::_mem = (uintptr)NULL
		 */
		(void)dhd_dma_buf_audit(dhd, &ring->dma_buf, ring->name);

	} else {

		/* Runner allocate a dhd_dma_buf */
#if defined(BCM_DHD_RUNNER)
		ring->dma_buf.len = dma_buf_len;
		dma_buf_alloced = dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		    (uintptr)ATTACH_RING_BUF + ring->id, (uintptr)&ring->dma_buf);

		/* This dhd_dma_buf is not allocated by dhd_dma_buf_alloc()
		 * and, dhd_dma_buf::_alloced = 0, dhd_dma_buf::_mem = (uintptr)NULL
		 */
		(void)dhd_dma_buf_audit(dhd, &ring->dma_buf, ring->name);

		if (DHD_IS_FLOWRING(ringid)) {
		    ring->max_items = ring->dma_buf.len / ring->item_size;
		} else if ((dma_buf_alloced == BCME_OK) &&
		           ((ring->id == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ||
		           (ring->id == BCMPCIE_D2H_MSGRING_TX_COMPLETE) ||
		           (ring->id == BCMPCIE_D2H_MSGRING_RX_COMPLETE)))  {
	            /* TXPOST ring configuration is done in dhd_prot_runner_preinit() */
	            dma_buf_alloced = dhd_runner_notify(dhd->runner_hlp,
	                H2R_MSGRING_FORMAT_NOTIF, ring->id,
	                ((ring->item_size << 16) | ring->item_type));
	        }
#else /* !BCM_DHD_RUNNER */
		dma_buf_alloced = dhd_dma_buf_alloc(dhd, &ring->dma_buf, ring->name,
		                      dma_buf_len, DHD_DMA_BUF_ALIGN_BITS);
#endif /* !BCM_DHD_RUNNER */
		if (dma_buf_alloced != BCME_OK) {
			return BCME_NOMEM;
		}
	}

	/* CAUTION: Save ring::base_addr in little endian format! */
	dhd_base_addr_htolpa(&ring->base_addr, ring->dma_buf.pa);

#ifdef BCM_SECURE_DMA
		ring->dma_buf.secdma = MALLOCZ(prot->osh, sizeof(sec_cma_info_t));
		if (ring->dma_buf.secdma == NULL) {
			goto free_dma_buf;
		}
#endif /* BCM_SECURE_DMA */

	DHD_INFO(("RING_ATTACH : %s Items: type %u size %u max %u misc %u, "
		"Ring: size %u start %p buf phys addr" HADDR64_FMT "\n", ring->name,
		ring->item_type, ring->item_size, ring->max_items, ring->item_misc,
		dma_buf_len, ring->dma_buf.va, HADDR64_VAL_LTOH(ring->base_addr)));

	return BCME_OK;

#ifdef BCM_SECURE_DMA
free_dma_buf:
	if (dma_buf_alloced == BCME_OK) {
		dhd_dma_buf_free(dhd, &ring->dma_buf, name);
	}
#endif /* BCM_SECURE_DMA */

	return BCME_NOMEM;
} /* dhd_prot_ring_alloc */

/**
 * dhd_prot_ring_init - Post the common ring information to dongle.
 *
 * Used only for common rings.
 *
 * The flowrings information is passed via the create flowring control message
 * (tx_flowring_create_request_t) sent over the H2D control submission common
 * ring.
 */
static void
dhd_prot_ring_init(dhd_pub_t *dhd, msgbuf_ring_t *ring)
{
	uint16 ringid;
	ring->wr = 0;
	ring->rd = 0;
	ring->qd = 0;

	ringid = ring->id;

	dhd_bus_cmn_writeshared(dhd->bus, &ring->id, RING_MEM_ID, ringid);
	dhd_bus_cmn_writeshared(dhd->bus, &ring->item_type, RING_ITEM_TYPE, ringid);
	dhd_bus_cmn_writeshared(dhd->bus, &ring->item_size, RING_ITEM_SIZE, ringid);
	dhd_bus_cmn_writeshared(dhd->bus, &ring->max_items, RING_MAX_ITEMS, ringid);

	/* CAUTION: ring::base_addr already in Little Endian */
	dhd_bus_cmn_writeshared(dhd->bus, &ring->base_addr, RING_BASE_ADDR, ringid);

	dhd_bus_cmn_writeshared(dhd->bus, &ring->wr, RING_WR_UPD, ringid);
	dhd_bus_cmn_writeshared(dhd->bus, &ring->rd, RING_RD_UPD, ringid);

	if (IDMA_ACTIVE(dhd)) {
		ring->idma_index = dhd_idma_flowmgr_ringid_2_index(
			dhd, ring->id,
			DHD_H2D_RING_OFFSET(ring->id));
		DHD_INFO(("%s: Ring \"%s\" ringid %d flowid %d idma_index %d\n", __FUNCTION__,
			ring->name, ring->id, DHD_H2D_RING_OFFSET(ring->id), ring->idma_index));
	}

	/* ring inited */
	ring->inited = TRUE;

} /* dhd_prot_ring_init */

/**
 * dhd_prot_ring_reset - bzero a ring's DMA-ble buffer and cache flush
 * Reset WR and RD indices to 0.
 */
static void
dhd_prot_ring_reset(dhd_pub_t *dhd, msgbuf_ring_t *ring)
{
	DHD_TRACE(("%s\n", __FUNCTION__));

	dhd_dma_buf_reset(dhd, &ring->dma_buf, ring->name);

	ring->rd = ring->wr = 0;
}

/**
 * dhd_prot_ring_free - Detach the DMA-able buffer and any other objects
 * hanging off the msgbuf_ring.
 */
static void
dhd_prot_ring_free(dhd_pub_t *dhd, msgbuf_ring_t *ring)
{
	dhd_prot_t *prot = dhd->prot;
	ASSERT(ring);

	ring->inited = FALSE;
	/* rd = ~0, wr = ring->rd - 1, max_items = 0, len_item = ~0 */

#ifdef BCM_SECURE_DMA
	if (ring->dma_buf.secdma) {
		SECURE_DMA_UNMAP_ALL(prot->osh, ring->dma_buf.secdma);
		MFREE(prot->osh, ring->dma_buf.secdma, sizeof(sec_cma_info_t));
		ring->dma_buf.secdma = NULL;
	}
#endif /* BCM_SECURE_DMA */

	/* If the DMA-able buffer was carved out of a pre-reserved contiguous
	 * memory, then simply stop using it.
	 */
	if (DHD_IS_FLOWRING(ring->id) && (prot->flowrings_dma_buf.va)) {
		(void)dhd_dma_buf_audit(dhd, &ring->dma_buf, ring->name);
		memset(&ring->dma_buf, 0, sizeof(dhd_dma_buf_t));
	} else {
#if defined(BCM_DHD_RUNNER)
		dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		    DETACH_RING_BUF + ring->id, (uintptr)&ring->dma_buf);
#else /* !BCM_DHD_RUNNER */
		dhd_dma_buf_free(dhd, &ring->dma_buf, ring->name);
#endif /* !BCM_DHD_RUNNER */
	}

} /* dhd_prot_ring_free */

/*
 * +----------------------------------------------------------------------------
 * Flowring Pool
 *
 * Unlike common rings, which are attached very early on (dhd_prot_attach),
 * flowrings are dynamically instantiated. Moreover, flowrings may require a
 * larger DMA-able buffer. To avoid issues with fragmented cache coherent
 * DMA-able memory, a pre-allocated pool of msgbuf_ring_t is allocated once.
 * The DMA-able buffers are attached to these pre-allocated msgbuf_ring.
 *
 * Each DMA-able buffer may be allocated independently, or may be carved out
 * of a single large contiguous region that is registered with the protocol
 * layer into flowrings_dma_buf. On a 64bit platform, this contiguous region
 * may not span 0x00000000FFFFFFFF (avoid dongle side 64bit ptr arithmetic).
 *
 * No flowring pool action is performed in dhd_prot_attach(), as the number
 * of h2d rings is not yet known.
 *
 * In dhd_prot_init(), the dongle advertized number of h2d rings is used to
 * determine the number of flowrings required, and a pool of msgbuf_rings are
 * allocated and a DMA-able buffer (carved or allocated) is attached.
 * See: dhd_prot_flowrings_pool_alloc()
 *
 * A flowring msgbuf_ring object may be fetched from this pool during flowring
 * creation, using the flowid. Likewise, flowrings may be freed back into the
 * pool on flowring deletion.
 * See: dhd_prot_flowrings_pool_fetch(), dhd_prot_flowrings_pool_release()
 *
 * In dhd_prot_detach(), the flowring pool is detached. The DMA-able buffers
 * are detached (returned back to the carved region or freed), and the pool of
 * msgbuf_ring and any objects allocated against it are freed.
 * See: dhd_prot_flowrings_pool_free()
 *
 * In dhd_prot_reset(), the flowring pool is simply reset by returning it to a
 * state as-if upon an attach. All DMA-able buffers are retained.
 * Following a dhd_prot_reset(), in a subsequent dhd_prot_init(), the flowring
 * pool attach will notice that the pool persists and continue to use it. This
 * will avoid the case of a fragmented DMA-able region.
 *
 * +----------------------------------------------------------------------------
 */

/* Fetch number of H2D flowrings given the total number of h2d rings */
#define DHD_FLOWRINGS_POOL_TOTAL(max_h2d_rings) \
	((max_h2d_rings) - BCMPCIE_H2D_COMMON_MSGRINGS)

/* Conversion of a flowid to a flowring pool index */
#define DHD_FLOWRINGS_POOL_OFFSET(flowid) \
	((flowid) - BCMPCIE_H2D_COMMON_MSGRINGS)

/* Fetch the msgbuf_ring_t from the flowring pool given a flowid */
#define DHD_RING_IN_FLOWRINGS_POOL(prot, flowid) \
	(msgbuf_ring_t*)((prot)->h2d_flowrings_pool) + DHD_FLOWRINGS_POOL_OFFSET(flowid)

/* Traverse each flowring in the flowring pool, assigning ring and flowid */
#define FOREACH_RING_IN_FLOWRINGS_POOL(prot, ring, flowid) \
	for ((flowid) = DHD_FLOWRING_START_FLOWID, \
		 (ring) = DHD_RING_IN_FLOWRINGS_POOL(prot, flowid); \
		 (flowid) < (prot)->max_h2d_rings; \
		 (flowid)++, (ring)++)

/**
 * dhd_prot_flowrings_pool_alloc - Initialize a pool of flowring msgbuf_ring_t.
 *
 * Allocate a pool of msgbuf_ring along with DMA-able buffers for flowrings.
 * Dongle includes common rings when it advertizes the number of H2D rings.
 * Allocates a pool of msgbuf_ring_t and invokes dhd_prot_ring_alloc to
 * allocate the DMA-able buffer and initialize each msgbuf_ring_t object.
 *
 * dhd_prot_ring_alloc is invoked to perform the actual initialization and
 * attaching the DMA-able buffer.
 *
 * Later dhd_prot_flowrings_pool_fetch() may be used to fetch a preallocated and
 * initialized msgbuf_ring_t object.
 *
 * returns BCME_OK=0 on success
 * returns non-zero negative error value on failure.
 */
static int
dhd_prot_flowrings_pool_alloc(dhd_pub_t *dhd)
{
	uint16 flowid;
	msgbuf_ring_t *ring;
	uint16 ringid, h2d_flowrings_total; /* exclude H2D common rings */
	dhd_prot_t *prot = dhd->prot;
	char ring_name[RING_NAME_MAX_LENGTH];

	if (prot->h2d_flowrings_pool != NULL) {
		return BCME_OK; /* dhd_prot_init rentry after a dhd_prot_reset */
	}

	ASSERT(prot->max_h2d_rings == 0);

	/* max_h2d_rings includes H2D common rings: ctrl and rxbuf subn */
	prot->max_h2d_rings = (uint16)dhd_bus_max_h2d_rings(dhd->bus);

	if (prot->max_h2d_rings < BCMPCIE_H2D_COMMON_MSGRINGS) {
		DHD_ERROR(("%s: max_h2d_rings advertized as %u\n",
			__FUNCTION__, prot->max_h2d_rings));
		return BCME_ERROR;
	}

	/* Subtract number of H2D common rings, to determine number of flowrings */
	h2d_flowrings_total = DHD_FLOWRINGS_POOL_TOTAL(prot->max_h2d_rings);

	DHD_ERROR(("Attach flowrings pool for %d rings\n", h2d_flowrings_total));

	/* Allocate pool of msgbuf_ring_t objects for all flowrings */
	prot->h2d_flowrings_pool = (msgbuf_ring_t *)MALLOCZ(prot->osh,
		(h2d_flowrings_total * sizeof(msgbuf_ring_t)));

	if (prot->h2d_flowrings_pool == NULL) {
		DHD_ERROR(("%s: flowrings pool for %d flowrings, alloc failure\n",
			__FUNCTION__, h2d_flowrings_total));
		goto fail;
	}

#if defined(BCM_DHD_RUNNER)
	dhd_runner_notify(dhd->runner_hlp, H2R_FLRING_INIT_NOTIF,
	    prot->max_h2d_rings, (unsigned long)&prot->flring_cache);
#endif /* BCM_DHD_RUNNER */

	/* Setup & Attach a DMA-able buffer to each flowring in the flowring pool */
	FOREACH_RING_IN_FLOWRINGS_POOL(prot, ring, flowid) {
		ringid = DHD_FLOWID_TO_RINGID(flowid);
		snprintf(ring_name, sizeof(ring_name), "h2dflr_%03u", flowid);
		ring_name[RING_NAME_MAX_LENGTH - 1] = '\0';
		if (dhd_prot_ring_alloc(dhd, ring, ring_name, ringid,
		        prot->txpost_item_type, prot->txpost_item_size,
		        prot->txpost_max_items, prot->txpost_item_misc) != BCME_OK)
		{
			goto attach_fail;
		}

		if (IDMA_ACTIVE(dhd)) {
			ring->idma_index = dhd_idma_flowmgr_ringid_2_index(
				dhd, ring->id, flowid);
			DHD_INFO(("%s: Ring \"%s\" ringid %d flowid %d idma_index %d\n",
				__FUNCTION__, ring->name, ring->id, flowid, ring->idma_index));
		}
	}

	return BCME_OK;

attach_fail:
	/* XXX: On a per project basis, one may decide whether to continue with
	 * "fewer" flowrings, and what value of fewer suffices.
	 */
	dhd_prot_flowrings_pool_free(dhd); /* Free entire pool of flowrings */

fail:
	prot->max_h2d_rings = 0;

	return BCME_NOMEM;
} /* dhd_prot_flowrings_pool_alloc */

/**
 * dhd_prot_flowrings_pool_reset - Reset all msgbuf_ring_t objects in the pool.
 * Invokes dhd_prot_ring_reset to perform the actual reset.
 *
 * The DMA-able buffer is not freed during reset and neither is the flowring
 * pool freed.
 *
 * dhd_prot_flowrings_pool_reset will be invoked in dhd_prot_reset. Following
 * the dhd_prot_reset, dhd_prot_init will be re-invoked, and the flowring pool
 * from a previous flowring pool instantiation will be reused.
 *
 * This will avoid a fragmented DMA-able memory condition, if multiple
 * dhd_prot_reset were invoked to reboot the dongle without a full detach/attach
 * cycle.
 */
static void
dhd_prot_flowrings_pool_reset(dhd_pub_t *dhd)
{
	uint16 flowid;
	msgbuf_ring_t *ring;
	dhd_prot_t *prot = dhd->prot;

	if (prot->h2d_flowrings_pool == NULL) {
		ASSERT(prot->max_h2d_rings == 0);
		return;
	}

	/* Reset each flowring in the flowring pool */
	FOREACH_RING_IN_FLOWRINGS_POOL(prot, ring, flowid) {
		dhd_prot_ring_reset(dhd, ring);
		ring->inited = FALSE;
	}

	/* Flowring pool state must be as-if dhd_prot_flowrings_pool_alloc */
}

/**
 * dhd_prot_flowrings_pool_free - Free pool of msgbuf_ring along with
 * DMA-able buffers for flowrings.
 * dhd_prot_ring_free is invoked to free the DMA-able buffer and perform any
 * de-initialization of each msgbuf_ring_t.
 */
static void
dhd_prot_flowrings_pool_free(dhd_pub_t *dhd)
{
	int flowid;
	msgbuf_ring_t *ring;
	int h2d_flowrings_total; /* exclude H2D common rings */
	dhd_prot_t *prot = dhd->prot;

	if (prot->h2d_flowrings_pool == NULL) {
		ASSERT(prot->max_h2d_rings == 0);
		return;
	}

	/* Detach the DMA-able buffer for each flowring in the flowring pool */
	FOREACH_RING_IN_FLOWRINGS_POOL(prot, ring, flowid) {
		dhd_prot_ring_free(dhd, ring);
	}

	h2d_flowrings_total = DHD_FLOWRINGS_POOL_TOTAL(prot->max_h2d_rings);

	MFREE(prot->osh, prot->h2d_flowrings_pool,
		(h2d_flowrings_total * sizeof(msgbuf_ring_t)));

	prot->h2d_flowrings_pool = (msgbuf_ring_t*)NULL;
	prot->max_h2d_rings = 0;

} /* dhd_prot_flowrings_pool_free */

/**
 * dhd_prot_flowrings_pool_fetch - Fetch a preallocated and initialized
 * msgbuf_ring from the flowring pool, and assign it.
 *
 * Unlike common rings, which uses a dhd_prot_ring_init() to pass the common
 * ring information to the dongle, a flowring's information is passed via a
 * flowring create control message.
 *
 * Only the ring state (WR, RD) index are initialized.
 */
static msgbuf_ring_t *
dhd_prot_flowrings_pool_fetch(dhd_pub_t *dhd, uint16 flowid)
{
	msgbuf_ring_t *ring;
	dhd_prot_t *prot = dhd->prot;

	ASSERT(flowid >= DHD_FLOWRING_START_FLOWID);
	ASSERT(flowid < prot->max_h2d_rings);
	ASSERT(prot->h2d_flowrings_pool != NULL);

	ring = DHD_RING_IN_FLOWRINGS_POOL(prot, flowid);

	/* ASSERT flow_ring->inited == FALSE */

	ring->wr = 0;
	ring->rd = 0;
	ring->qd = 0;
	ring->inited = TRUE;

	return ring;
}

/**
 * dhd_prot_flowrings_pool_release - release a previously fetched flowring's
 * msgbuf_ring back to the flow_ring pool.
 */
void
dhd_prot_flowrings_pool_release(dhd_pub_t *dhd, uint16 flowid, void *flow_ring)
{
	msgbuf_ring_t *ring;
	dhd_prot_t *prot = dhd->prot;

	ASSERT(flowid >= DHD_FLOWRING_START_FLOWID);
	ASSERT(flowid < prot->max_h2d_rings);
	ASSERT(prot->h2d_flowrings_pool != NULL);

	ring = DHD_RING_IN_FLOWRINGS_POOL(prot, flowid);

	ASSERT(ring == (msgbuf_ring_t*)flow_ring);
	/* ASSERT flow_ring->inited == TRUE */

	(void)dhd_dma_buf_audit(dhd, &ring->dma_buf, ring->name);

	ring->wr = 0;
	ring->rd = 0;
	ring->qd = 0;
	ring->inited = FALSE;
}

/* Assumes only one index is updated at a time */
/* If exactly_nitems is true, this function will allocate space for nitems or fail */
/*    Exception: when wrap around is encountered, to prevent hangup (last nitems of ring buffer) */
/* If exactly_nitems is false, this function will allocate space for nitems or less */
static void *BCMFASTPATH
dhd_prot_get_ring_space(msgbuf_ring_t *ring, uint16 nitems, uint16 * alloced,
	bool exactly_nitems)
{
	void *ret_ptr = NULL;
	uint16 ring_avail_cnt;

	ASSERT(nitems <= ring->max_items);

	ring_avail_cnt = CHECK_WRITE_SPACE(ring->rd, ring->wr, ring->max_items);

	if ((ring_avail_cnt == 0) ||
	       (exactly_nitems && (ring_avail_cnt < nitems) &&
	       ((ring->max_items - ring->wr) >= nitems))) {
		DHD_INFO(("Space not available: ring %s items %d write %d read %d\n",
			ring->name, nitems, ring->wr, ring->rd));
		return NULL;
	}
	*alloced = MIN(nitems, ring_avail_cnt);

	/* Return next available space */
	ret_ptr = (char *)DHD_RING_BGN_VA(ring) + (ring->wr * ring->item_size);

	/* Update write index */
	if ((ring->wr + *alloced) == ring->max_items) {
		ring->wr = 0;
	} else if ((ring->wr + *alloced) < ring->max_items) {
		ring->wr += *alloced;
	} else {
		/* Should never hit this */
		ASSERT(0);
		return NULL;
	}

	return ret_ptr;
} /* dhd_prot_get_ring_space */

/**
 * dhd_prot_ring_write_complete - Host updates the new WR index on producing
 * new messages in a H2D ring. The messages are flushed from cache prior to
 * posting the new WR index. The new WR index will be updated in the DMA index
 * array or directly in the dongle's ring state memory.
 * A PCIE doorbell will be generated to wake up the dongle.
 */
static void BCMFASTPATH
dhd_prot_ring_write_complete(dhd_pub_t *dhd, msgbuf_ring_t * ring, void* p,
	uint16 nitems)
{
	uint32 db_index;
	dhd_prot_t *prot = dhd->prot;

	/* cache flush */
	OSL_CACHE_FLUSH(p, ring->item_size * nitems);

	/* Update write index
	 * Except case: The HWA_RXPOST is using iDMA to update WR to HWA
	 * internal RxPost ring context.  So they don't need update to host memory nor TCM.
	 */
	if (!HWA_RXPOST_ACTIVE(ring)) {
		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support) || IDMA_ACTIVE(dhd)) {
			dhd_prot_dma_indx_set(dhd, ring->wr, H2D_DMA_INDX_WR_UPD, ring->id);
		} else {
			dhd_bus_cmn_writeshared(dhd->bus, &(ring->wr), RING_WR_UPD, ring->id);
		}
	}

	/* raise h2d interrupt */
	if (HWA_RXPOST_ACTIVE(ring)) {
		db_index = ((ring->wr << INDEX_VAL_SHIFT) |
			(DMA_TYPE_HWA_RXPOST << DMA_TYPE_SHIFT));
		prot->db1_2_ring_fn(dhd->bus, db_index, TRUE);
	} else if (IDMA_ACTIVE(dhd)) {
		db_index = ((DMA_TYPE_IDMA << DMA_TYPE_SHIFT) | ring->idma_index);
		prot->mb_2_ring_fn(dhd->bus, db_index, TRUE);
	} else {
		prot->mb_ring_fn(dhd->bus, ring->wr);
	}
}

/**
 * dhd_prot_upd_read_idx - Host updates the new RD index on consuming messages
 * from a D2H ring. The new RD index will be updated in the DMA Index array or
 * directly in dongle's ring state memory.
 */
static void
dhd_prot_upd_read_idx(dhd_pub_t *dhd, msgbuf_ring_t * ring)
{
	uint32 db_index;
	dhd_prot_t *prot = dhd->prot;

	/* Update read index
	 * If DMA'ing H2D indices supported then update r -indices in the host memory o/w in TCM.
	 * Except case: Both HWA_TXCPL, HWA_RXCPL are using iDMA to update RD to HWA
	 * internal CPL ring context.  So they don't need to update to host memory nor TCM.
	 */
	if (!HWA_TXCPL_ACTIVE(ring) && !HWA_RXCPL_ACTIVE(ring)) {
		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support) || IDMA_ACTIVE(dhd)) {
			dhd_prot_dma_indx_set(dhd, ring->rd, D2H_DMA_INDX_RD_UPD, ring->id);
		} else {
			dhd_bus_cmn_writeshared(dhd->bus, &(ring->rd), RING_RD_UPD, ring->id);
		}
	}

	/* raise h2d interrupt */
	if (HWA_TXCPL_ACTIVE(ring)) {
		db_index = ((ring->rd << INDEX_VAL_SHIFT) |
			(DMA_TYPE_HWA_TXCPL << DMA_TYPE_SHIFT));
		prot->db1_2_ring_fn(dhd->bus, db_index, TRUE);
	} else if (HWA_RXCPL_ACTIVE(ring)) {
		db_index = ((ring->rd << INDEX_VAL_SHIFT) |
			((ring->id - BCMPCIE_D2H_MSGRING_TX_COMPLETE) << INDEX_NUM_SHIFT) |
			(DMA_TYPE_HWA_RXCPL << DMA_TYPE_SHIFT));
		prot->db1_2_ring_fn(dhd->bus, db_index, TRUE);
	} else if (IDMA_ACTIVE(dhd)) {
		db_index = ((DMA_TYPE_IDMA << DMA_TYPE_SHIFT) | ring->idma_index);
		prot->mb_2_ring_fn(dhd->bus, db_index, TRUE);
	}
}

/**
 * dhd_prot_flow_ring_write_complete - Host updates the flow-ring new WR index
 * and QD on arrival of new messages. WR index is updated on producing
 * new messages in a H2D flow-ring ring. QD is updated on change in the queue
 * length. The messages are flushed from cache prior to posting the new WR index
 * The Queue Depth will be updated in the DMA index array. The new WR index will
 * be updated in the DMA index array or directly in the dongle's ring state
 * memory. A PCIE doorbell will be generated to wake up the dongle.
 */
static void BCMFASTPATH
dhd_prot_flow_ring_write_complete(dhd_pub_t *dhd,
	flow_ring_node_t *flow_ring_node, void* p, uint16 nitems)
{
	uint32 db_index;
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t * ring = (msgbuf_ring_t*)(flow_ring_node->prot_info);
	uint32 qd_wr;

	/* cache flush  the flow-ring descriptors */
	OSL_CACHE_FLUSH(p, ring->item_size * nitems);

	/* Update write index and queue length (if needed) */
	if (PCIE_HBQD(dhd)) {
		flow_queue_t  * flow_queue = &flow_ring_node->queue;

		ring->qd = DHD_FLOW_QUEUE_LEN(flow_queue);
		qd_wr = PCIE_HBQD_VAL(ring->qd, ring->wr);
		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support) || IDMA_ACTIVE(dhd))
			dhd_prot_pcie_hbqd_indx_set(dhd, qd_wr, H2D_DMA_INDX_WR_UPD,
				ring->id);
		else
			dhd_bus_cmn_writeshared(dhd->bus, &(qd_wr), RING_QDWR_UPD, ring->id);
	} else {
		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support) || IDMA_ACTIVE(dhd))
			dhd_prot_dma_indx_set(dhd, ring->wr, H2D_DMA_INDX_WR_UPD, ring->id);
		else
			dhd_bus_cmn_writeshared(dhd->bus, &(ring->wr), RING_WR_UPD,
				ring->id);
	}

	/* raise h2d interrupt */
	if (IDMA_ACTIVE(dhd)) {
		db_index = ((DMA_TYPE_IDMA << DMA_TYPE_SHIFT) | ring->idma_index);
		prot->mb_2_ring_fn(dhd->bus, db_index, TRUE);
	} else {
		prot->mb_ring_fn(dhd->bus, ring->wr);
	}
}

/**
 * dhd_prot_dma_indx_set - set a new WR or RD index in the DMA index array.
 * Dongle will DMA the entire array (if DMA_INDX feature is enabled).
 * See dhd_prot_dma_indx_init()
 */
static void
dhd_prot_dma_indx_set(dhd_pub_t *dhd, uint16 new_index, uint8 type, uint16 ringid)
{
	uint8 *ptr;
	uint16 offset;
	dhd_prot_t *prot = dhd->prot;

	switch (type) {
		case H2D_DMA_INDX_WR_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_wr_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;
#ifdef BCMHWA
		case H2D_DMA_INDX_RD_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_rd_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;
#endif /* BCMHWA */
		case D2H_DMA_INDX_RD_UPD:
			ptr = (uint8 *)(prot->d2h_dma_indx_rd_buf.va);
			offset = DHD_D2H_RING_OFFSET(ringid);
			break;

		default:
			DHD_ERROR(("%s: Invalid option for DMAing read/write index\n",
				__FUNCTION__));
			return;
	}

	ASSERT(prot->rw_index_sz != 0);
	ptr += offset * prot->rw_index_sz;

#if defined(BCM_DHD_RUNNER)
	dhd_runner_notify(dhd->runner_hlp, H2R_IDX_BUF_WR_REQUEST,
	    (unsigned long)ptr, (unsigned long)new_index);
#else /* ! BCM_DHD_RUNNER */

	*(uint16*)ptr = htol16(new_index);

	OSL_CACHE_FLUSH((void *)ptr, prot->rw_index_sz);
#endif /* ! BCM_DHD_RUNNER */

	DHD_TRACE(("%s: data %d type %d ringid %d ptr 0x%p offset %d\n",
		__FUNCTION__, new_index, type, ringid, ptr, offset));

} /* dhd_prot_dma_indx_set */

/**
 * dhd_prot_dma_indx_get - Fetch a WR or RD index from the dongle DMA-ed index
 * array.
 * Dongle DMAes an entire array to host memory (if the feature is enabled).
 * See dhd_prot_dma_indx_init()
 */
static uint16
dhd_prot_dma_indx_get(dhd_pub_t *dhd, uint8 type, uint16 ringid)
{
	uint8 *ptr;
	uint16 data;
	uint16 offset;
	dhd_prot_t *prot = dhd->prot;

	switch (type) {
		case H2D_DMA_INDX_WR_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_wr_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;

		case H2D_DMA_INDX_RD_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_rd_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;

		case D2H_DMA_INDX_WR_UPD:
			ptr = (uint8 *)(prot->d2h_dma_indx_wr_buf.va);
			offset = DHD_D2H_RING_OFFSET(ringid);
			break;

		case D2H_DMA_INDX_RD_UPD:
			ptr = (uint8 *)(prot->d2h_dma_indx_rd_buf.va);
			offset = DHD_D2H_RING_OFFSET(ringid);
			break;

		default:
			DHD_ERROR(("%s: Invalid option for DMAing read/write index\n",
				__FUNCTION__));
			return 0;
	}

	ASSERT(prot->rw_index_sz != 0);
	ptr += offset * prot->rw_index_sz;

#if defined(BCM_DHD_RUNNER)
	{
		uint32 u32_data;

		/*
		 * dhd_runner needs u32 pointer if hbqd is enabled even though
		 * 16bit data is needed
		 */
		dhd_runner_notify(dhd->runner_hlp, H2R_IDX_BUF_RD_REQUEST,
			(unsigned long)ptr, (unsigned long)&u32_data);
		data = (uint16)(u32_data & 0xFFFF);
	}
#else  /* ! BCM_DHD_RUNNER */
	OSL_CACHE_INV((void *)ptr, prot->rw_index_sz);

	data = LTOH16(*((uint16*)ptr));
#endif /* ! BCM_DHD_RUNNER */

	DHD_TRACE(("%s: data %d type %d ringid %d ptr 0x%p offset %d\n",
		__FUNCTION__, data, type, ringid, ptr, offset));

	return (data);
} /* dhd_prot_dma_indx_get */

/**
 * dhd_prot_pcie_hbqd_indx_set - set a new WR or RD 16bit index and 16bit value
 * in the DMA index array. Dongle will DMA the entire array (if DMA_INDX
 * feature is enabled). See dhd_prot_dma_indx_init()
 */
static void
dhd_prot_pcie_hbqd_indx_set(dhd_pub_t *dhd, uint32 new_index, uint8 type,
	uint16 ringid)
{
	uint8 *ptr;
	uint16 offset;
	dhd_prot_t *prot = dhd->prot;

	switch (type) {
		case H2D_DMA_INDX_WR_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_wr_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;
		default:
			DHD_ERROR(("%s: Invalid option for DMAing read/write index\n",
				__FUNCTION__));
			return;
	}

	ASSERT(prot->rw_index_sz != 0);
	ptr += offset * prot->rw_index_sz;

#if defined(BCM_DHD_RUNNER)
	dhd_runner_notify(dhd->runner_hlp, H2R_IDX_BUF_WR_REQUEST,
	    (unsigned long)ptr, (unsigned long)new_index);
#else /* ! BCM_DHD_RUNNER */

	*(uint32*)ptr = htol32(new_index);

	OSL_CACHE_FLUSH((void *)ptr, prot->rw_index_sz);
#endif /* ! BCM_DHD_RUNNER */

	DHD_TRACE(("%s: data %d type %d ringid %d ptr 0x%p offset %d\n",
		__FUNCTION__, new_index, type, ringid, ptr, offset));

} /* dhd_prot_pcie_hbqd_indx_set */

/**
 * dhd_prot_pcie_hbqd_indx_get - Fetch a split WR or RD index from the dongle
 * DMA-ed index array.
 * Dongle DMAes an entire array to host memory (if the feature is enabled).
 * See dhd_prot_dma_indx_init()
 */
static uint32
dhd_prot_pcie_hbqd_indx_get(dhd_pub_t *dhd, uint8 type, uint16 ringid)
{
	uint8 *ptr;
	uint32 data;
	uint16 offset;
	dhd_prot_t *prot = dhd->prot;

	switch (type) {
		case H2D_DMA_INDX_WR_UPD:
			ptr = (uint8 *)(prot->h2d_dma_indx_wr_buf.va);
			offset = DHD_H2D_RING_OFFSET(ringid);
			break;

		default:
			DHD_ERROR(("%s: Invalid option for DMAing read/write index\n",
				__FUNCTION__));
			return 0;
	}

	ASSERT(prot->rw_index_sz != 0);
	ptr += offset * prot->rw_index_sz;

#if defined(BCM_DHD_RUNNER)
	dhd_runner_notify(dhd->runner_hlp, H2R_IDX_BUF_RD_REQUEST,
	    (unsigned long)ptr, (unsigned long)&data);
#else  /* ! BCM_DHD_RUNNER */
	OSL_CACHE_INV((void *)ptr, prot->rw_index_sz);

	data = ltoh32(*((uint32*)ptr));
#endif /* ! BCM_DHD_RUNNER */

	DHD_TRACE(("%s: data %d type %d ringid %d ptr 0x%p offset %d\n",
		__FUNCTION__, data, type, ringid, ptr, offset));

	return (data);
} /* dhd_prot_pcie_hbqd_indx_get */

/**
 * An array of DMA read/write indices, containing information about host rings, can be maintained
 * either in host memory or in device memory, dependent on preprocessor options. This function is,
 * dependent on these options, called during driver initialization. It reserves and initializes
 * blocks of DMA'able host memory containing an array of DMA read or DMA write indices. The physical
 * address of these host memory blocks are communicated to the dongle later on. By reading this host
 * memory, the dongle learns about the state of the host rings.
 */

static INLINE int
dhd_prot_dma_indx_alloc(dhd_pub_t *dhd, uint8 type,
	dhd_dma_buf_t *dma_buf, const char * str, uint32 bufsz)
{
	int rc;

	if ((dma_buf->len == bufsz) || (dma_buf->va != NULL))
		return BCME_OK;

#if defined(BCM_DHD_RUNNER)
	/* Platform specific non-cached memory allocation */
	dma_buf->len = bufsz;
	rc = dhd_runner_notify(dhd->runner_hlp, H2R_DMA_BUF_NOTIF,
		ATTACH_DMA_INDX_BUF + type, (uintptr)dma_buf);
#else /* !BCM_DHD_RUNNER */
	rc = dhd_dma_buf_alloc(dhd, dma_buf, str, bufsz, DHD_DMA_BUF_ALIGN_BITS);
#endif /* !BCM_DHD_RUNNER */

	return rc;
}

int
dhd_prot_dma_indx_init(dhd_pub_t *dhd, uint32 rw_index_sz, uint8 type, uint32 length)
{
	uint32 bufsz;
	dhd_prot_t *prot = dhd->prot;
	dhd_dma_buf_t *dma_buf;

	if (prot == NULL) {
		DHD_ERROR(("%s: prot is not inited\n", __FUNCTION__));
		return BCME_ERROR;
	}

	/* Dongle advertizes 2B or 4B RW index size */
	ASSERT(rw_index_sz != 0);
	prot->rw_index_sz = rw_index_sz;

	bufsz = rw_index_sz * length;

	switch (type) {
		case H2D_DMA_INDX_WR_BUF:
			dma_buf = &prot->h2d_dma_indx_wr_buf;
			if (dhd_prot_dma_indx_alloc(dhd, type, dma_buf, "h2d_wr", bufsz)) {
				goto ret_no_mem;
			}
			DHD_PCIE_IPC(("\t Protocol: "
				"H2D WR Indices Array:" DHD_DMA_BUF_FMT " = %u * %4u\n",
				DHD_DMA_BUF_VAL(*dma_buf), rw_index_sz, length));
			break;

		case H2D_DMA_INDX_RD_BUF:
			dma_buf = &prot->h2d_dma_indx_rd_buf;
			if (dhd_prot_dma_indx_alloc(dhd, type, dma_buf, "h2d_rd", bufsz)) {
				goto ret_no_mem;
			}
			DHD_PCIE_IPC(("\t Protocol: "
				"H2D RD Indices Array:" DHD_DMA_BUF_FMT " = %u * %4u\n",
				DHD_DMA_BUF_VAL(*dma_buf), rw_index_sz, length));
			break;

		case D2H_DMA_INDX_WR_BUF:
			dma_buf = &prot->d2h_dma_indx_wr_buf;
			if (dhd_prot_dma_indx_alloc(dhd, type, dma_buf, "d2h_wr", bufsz)) {
				goto ret_no_mem;
			}
			DHD_PCIE_IPC(("\t Protocol: "
				"D2H WR Indices Array:" DHD_DMA_BUF_FMT " = %u * %4u\n",
				DHD_DMA_BUF_VAL(*dma_buf), rw_index_sz, length));
			break;

		case D2H_DMA_INDX_RD_BUF:
			dma_buf = &prot->d2h_dma_indx_rd_buf;
			if (dhd_prot_dma_indx_alloc(dhd, type, dma_buf, "d2h_rd", bufsz)) {
				goto ret_no_mem;
			}
			DHD_PCIE_IPC(("\t Protocol: "
				"D2H RD Indices Array:" DHD_DMA_BUF_FMT" = %u * %4u\n",
				DHD_DMA_BUF_VAL(*dma_buf), rw_index_sz, length));
			break;

		default:
			DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
				"Indices Array type %u\n", __FUNCTION__, type));
			return BCME_BADOPTION;
	}

	return BCME_OK;

ret_no_mem:
	DHD_ERROR(("%s: PCIe IPC PROTOCOL FAILURE: "
		"Indices Array type %u size %u bufsz %u\n",
		__FUNCTION__, type, length, bufsz));

	return BCME_NOMEM;
} /* dhd_prot_dma_indx_init */

/**
 * Called on checking for 'completion' messages from the dongle. Returns next host buffer to read
 * from, or NULL if there are no more messages to read.
 */
static uint8*
dhd_prot_get_read_addr(dhd_pub_t *dhd, msgbuf_ring_t *ring, uint32 *available_len)
{
	uint16 wr;
	uint16 rd;
	uint16 depth;
	uint16 items;
	void  *read_addr = NULL; /* address of next msg to be read in ring */
	uint16 d2h_wr = 0;

	DHD_TRACE(("%s: d2h_dma_indx_rd_buf %p, d2h_dma_indx_wr_buf %p\n",
		__FUNCTION__, (uint32 *)(dhd->prot->d2h_dma_indx_rd_buf.va),
		(uint32 *)(dhd->prot->d2h_dma_indx_wr_buf.va)));

	/* update write pointer */
	if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
		/* DMAing write/read indices supported */
		d2h_wr = dhd_prot_dma_indx_get(dhd, D2H_DMA_INDX_WR_UPD, ring->id);
		ring->wr = d2h_wr;
	} else {
		dhd_bus_cmn_readshared(dhd->bus, &(ring->wr), RING_WR_UPD, ring->id);
	}

	wr = ring->wr;
	rd = ring->rd;
	depth = ring->max_items;

	/* check for avail space, in number of ring items */
	items = READ_AVAIL_SPACE(wr, rd, depth);
	if (items == 0) {
		return NULL;
	}

	ASSERT(items < ring->max_items);

	/* if space is available, calculate address to be read */
	read_addr = (char*)ring->dma_buf.va + (rd * ring->item_size);

	/* update read pointer */
	if ((ring->rd + items) >= ring->max_items) {
		ring->rd = 0;
	} else {
		ring->rd += items;
	}

	ASSERT(ring->rd < ring->max_items);

	/* convert items to bytes : available_len must be 32bits */
	*available_len = (uint32)(items * ring->item_size);

	OSL_CACHE_INV(read_addr, *available_len);

	/* return read address */
	return read_addr;
} /* dhd_prot_get_read_addr */

#ifdef DHD_IFE
/* Read current RD and WR index */
void
dhd_msgbuf_get_curr_idx(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node, uint16 *rd, uint16 *wr)
{
	msgbuf_ring_t *flow_ring = (msgbuf_ring_t*)(flow_ring_node->prot_info);

	if (DMA_INDX_ENAB(dhdp->dma_d2h_ring_upd_support))
		*rd = dhd_prot_dma_indx_get(dhdp, H2D_DMA_INDX_RD_UPD, flow_ring->id);
	else
		*rd = flow_ring->rd;

	if (DMA_INDX_ENAB(dhdp->dma_h2d_ring_upd_support))
		*wr = dhd_prot_dma_indx_get(dhdp, H2D_DMA_INDX_WR_UPD, flow_ring->id);
	else
		*wr = flow_ring->wr;
}
#endif /* DHD_IFE */

/** Creates a flow ring and informs dongle of this event */
int
dhd_prot_flow_ring_create(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node)
{
	tx_flowring_create_request_t *flow_create_rqst;
	msgbuf_ring_t *flow_ring;
	dhd_prot_t *prot = dhd->prot;
	unsigned long flags;
	uint16 alloced = 0;
	msgbuf_ring_t *ctrl_ring = &prot->h2dring_ctrl_subn;

	/* Fetch a pre-initialized msgbuf_ring from the flowring pool */
	flow_ring = dhd_prot_flowrings_pool_fetch(dhd, flow_ring_node->flowid);
	if (flow_ring == NULL) {
		DHD_ERROR(("%s: dhd_prot_flowrings_pool_fetch TX Flowid %d failed\n",
			__FUNCTION__, flow_ring_node->flowid));
		return BCME_NOMEM;
	}

	DHD_GENERAL_LOCK(dhd, flags);

	/* Request for ctrl_ring buffer space */
	flow_create_rqst = (tx_flowring_create_request_t *)
		dhd_prot_alloc_ring_space(dhd, ctrl_ring, 1, &alloced, FALSE);

	if (flow_create_rqst == NULL) {
		dhd_prot_flowrings_pool_release(dhd, flow_ring_node->flowid, flow_ring);
		DHD_ERROR(("%s: Flow Create Req flowid %d - failure ring space\n",
			__FUNCTION__, flow_ring_node->flowid));
		DHD_GENERAL_UNLOCK(dhd, flags);
		return BCME_NOMEM;
	}

	flow_ring_node->prot_info = (void *)flow_ring;

	/* Common msg buf hdr */
	flow_create_rqst->msg.msg_type = MSG_TYPE_FLOW_RING_CREATE;
	flow_create_rqst->msg.if_id = (uint8)flow_ring_node->flow_info.ifindex;
	flow_create_rqst->msg.request_id = htol32(0); /* TBD */

	flow_create_rqst->msg.epoch = ctrl_ring->seqnum % H2D_EPOCH_MODULO;
	ctrl_ring->seqnum++;

	/* Update flow create message */
	flow_create_rqst->tid = flow_ring_node->flow_info.tid;
	flow_create_rqst->item_type = flow_ring->item_type;
	flow_create_rqst->flow_ring_id = htol16((uint16)flow_ring_node->flowid);
	memcpy(flow_create_rqst->sa, flow_ring_node->flow_info.sa, sizeof(flow_create_rqst->sa));
	memcpy(flow_create_rqst->da, flow_ring_node->flow_info.da, sizeof(flow_create_rqst->da));
	/* CAUTION: ring::base_addr already in Little Endian */
	HADDR64_SET(flow_create_rqst->haddr64, flow_ring->base_addr);

	flow_create_rqst->max_items = htol16(flow_ring->max_items);
	flow_create_rqst->len_item = htol16(flow_ring->item_size);

	DHD_INFO(("%s: %s: Send Flow Create Req flow ID %d for peer " MACDBG
		" prio %d ifindex %d\n", dhd_ifname(dhd, flow_ring_node->flow_info.ifindex),
		__FUNCTION__, flow_ring_node->flowid, MAC2STRDBG(flow_ring_node->flow_info.da),
		flow_ring_node->flow_info.tid, flow_ring_node->flow_info.ifindex));

	/* Update the flow_ring's WRITE index */
	dhd_prot_flow_ring_write_complete(dhd, flow_ring_node, (void*)NULL, 0);

	/* update control subn ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ctrl_ring, flow_create_rqst, 1);

#if defined(BCM_DHD_RUNNER)
	if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
		dhd_runner_notify(dhd->runner_hlp, H2R_FLRING_ENAB_NOTIF,
		flow_ring_node->flowid, 0);
	}
#endif /* BCM_DHD_RUNNER */

	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_OK;
} /* dhd_prot_flow_ring_create */

/** called on receiving MSG_TYPE_FLOW_RING_CREATE_CMPLT message from dongle */
static void
dhd_prot_flow_ring_create_response_process(dhd_pub_t *dhd, void *msg)
{
	tx_flowring_create_response_t *flow_create_resp = (tx_flowring_create_response_t *)msg;

	DHD_INFO(("%s: Flow Create Response status = %d Flow %d\n", __FUNCTION__,
		ltoh16(flow_create_resp->cmplt.status),
		ltoh16(flow_create_resp->cmplt.flow_ring_id)));

	dhd_bus_flow_ring_create_response(dhd->bus,
		ltoh16(flow_create_resp->cmplt.flow_ring_id),
		ltoh16(flow_create_resp->cmplt.status));
}

/** called on e.g. flow ring delete */
void
dhd_prot_clean_flow_ring(dhd_pub_t *dhd, void *msgbuf_flow_info)
{
	msgbuf_ring_t *flow_ring = (msgbuf_ring_t *)msgbuf_flow_info;
	dhd_prot_ring_free(dhd, flow_ring);
	DHD_INFO(("%s Cleaning up Flow \n", __FUNCTION__));

#if defined(BCM_DHD_RUNNER)
	if (DHD_IS_FLOWRING(flow_ring->id)) {
	    dhd_runner_notify(dhd->runner_hlp, H2R_FLRING_FLUSH_NOTIF,
	        DHD_RINGID_TO_FLOWID(flow_ring->id), 0);
	}
#endif /* BCM_DHD_RUNNER */

}

void
dhd_prot_print_flow_ring(dhd_pub_t *dhd, void *msgbuf_flow_info,
	struct bcmstrbuf *strbuf, const char * fmt)
{
	msgbuf_ring_t *flow_ring = (msgbuf_ring_t *)msgbuf_flow_info;
	uint16 rd, wr;
	uint16 host_rd, host_wr;

	/* Get write/read indices from dongle's memory */
	dhd_bus_cmn_readshared(dhd->bus, &rd, RING_RD_UPD, flow_ring->id);
	dhd_bus_cmn_readshared(dhd->bus, &wr, RING_WR_UPD, flow_ring->id);

	/* H2D COMMONRING */
	if (flow_ring->id < BCMPCIE_H2D_COMMON_MSGRINGS) {
		if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support))
			host_rd = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_RD_UPD, flow_ring->id);
		else
			host_rd = flow_ring->rd;

		/* Read RxPost WR from DMA index array when Runner RxCpl offload is enabled.
		 * Because the correct WR value is in DMA index array updated by Runner.
		 */
		if ((HWA_RXPOST_ACTIVE(flow_ring) &&
#if defined(BCM_DHD_RUNNER)
			!DHD_RNR_OFFL_RXCMPL(dhd) &&
#endif
			TRUE) || !DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support)) {
			host_wr = flow_ring->wr;
		}
		else
			host_wr = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_WR_UPD, flow_ring->id);
	/* H2D FLOWRING  */
	} else if (DHD_IS_FLOWRING(flow_ring->id)) {
		if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support))
			host_rd = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_RD_UPD, flow_ring->id);
		else
			host_rd = flow_ring->rd;

		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support))
			host_wr = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_WR_UPD, flow_ring->id);
		else
			host_wr = flow_ring->wr;
	/* D2H RING */
	} else {
		/* Although _dhd_prot_schedule_runner() will update TXCPL and RXCPL RD index
		 * from DMA index array to flow_ring->rd, let's just print host_rd from DMA index
		 * array.  It's more accurately.
		 */
		if ((HWA_TXCPL_ACTIVE(flow_ring) &&
#if defined(BCM_DHD_RUNNER)
			!DHD_RNR_OFFL_TXSTS(dhd) &&
#endif
			TRUE) ||
			(HWA_RXCPL_ACTIVE(flow_ring) &&
#if defined(BCM_DHD_RUNNER)
			!DHD_RNR_OFFL_RXCMPL(dhd) &&
#endif
			TRUE) ||
			!DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support))
			host_rd = flow_ring->rd;
		else
			host_rd = dhd_prot_dma_indx_get(dhd, D2H_DMA_INDX_RD_UPD, flow_ring->id);

		if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support))
			host_wr = dhd_prot_dma_indx_get(dhd, D2H_DMA_INDX_WR_UPD, flow_ring->id);
		else
			host_wr = flow_ring->wr;
	}

	if (PCIE_HBQD(dhd) && (DHD_IS_FLOWRING(flow_ring->id))) {
		uint32 qd;
		uint32 host_qd;

		dhd_bus_cmn_readshared(dhd->bus, &qd, RING_QDWR_UPD, flow_ring->id);
		qd = PCIE_HBQD_QD(qd);
		if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support))
			host_qd = PCIE_HBQD_QD(dhd_prot_pcie_hbqd_indx_get(dhd,
				H2D_DMA_INDX_WR_UPD, flow_ring->id));
		else
			host_qd = flow_ring->qd;

		if (fmt == NULL)
			fmt = "RD %d:%d WR %d,%d:%d,%d\n";
		bcm_bprintf(strbuf, fmt, rd, host_rd, qd, wr, host_qd, host_wr);
	} else {
		if (fmt == NULL)
			fmt = "RD %d:%d WR %d:%d\n";

		bcm_bprintf(strbuf, fmt, rd, host_rd, wr, host_wr);
	}
}

void
dhd_prot_print_info(dhd_pub_t *dhd, struct bcmstrbuf *strbuf)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring;
	uint32 i;

	bcm_bprintf(strbuf, "CtrlPost: ");
	dhd_prot_print_flow_ring(dhd, &prot->h2dring_ctrl_subn, strbuf, NULL);

	bcm_bprintf(strbuf, "CtrlCpl: ");
	dhd_prot_print_flow_ring(dhd, &prot->d2hring_ctrl_cpln, strbuf, NULL);

	bcm_bprintf(strbuf, "RxPost: ");
	bcm_bprintf(strbuf, "RBP %d ", prot->rxbufpost);
	dhd_prot_print_flow_ring(dhd, &prot->h2dring_rxp_subn, strbuf, NULL);

	FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
		bcm_bprintf(strbuf, "RxCpl[%d]: ", i);
		dhd_prot_print_flow_ring(dhd, ring, strbuf, NULL);
	}

	bcm_bprintf(strbuf, "TxCpl: ");
	dhd_prot_print_flow_ring(dhd, &prot->d2hring_tx_cpln, strbuf, NULL);

	bcm_bprintf(strbuf, "active_tx_count %d	 pktidmap_avail %d\n",
		prot->active_tx_count, DHD_PKTID_AVAIL(prot->pktid_map_handle));
}

int
dhd_prot_flow_ring_delete(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node)
{
	tx_flowring_delete_request_t *flow_delete_rqst;
	dhd_prot_t *prot = dhd->prot;
	unsigned long flags;
	uint16 alloced = 0;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	DHD_GENERAL_LOCK(dhd, flags);

#if defined(BCM_DHD_RUNNER)
	if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
	    dhd_runner_notify(dhd->runner_hlp, H2R_FLRING_DISAB_NOTIF,
	        flow_ring_node->flowid, 0);
	}
#endif /* BCM_DHD_RUNNER */

	/* Request for ring buffer space */
	flow_delete_rqst = (tx_flowring_delete_request_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);

	if (flow_delete_rqst == NULL) {
		DHD_GENERAL_UNLOCK(dhd, flags);
		DHD_ERROR(("%s: Flow Delete Req - failure ring space\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	/* Common msg buf hdr */
	flow_delete_rqst->msg.msg_type = MSG_TYPE_FLOW_RING_DELETE;
	flow_delete_rqst->msg.if_id = (uint8)flow_ring_node->flow_info.ifindex;
	flow_delete_rqst->msg.request_id = htol32(0); /* TBD */

	flow_delete_rqst->msg.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

	/* Update Delete info */
	flow_delete_rqst->flow_ring_id = htol16((uint16)flow_ring_node->flowid);
	flow_delete_rqst->reason = htol16(BCME_OK);

	DHD_INFO(("%s: %s: Send Flow Delete Req RING ID %d for peer " MACDBG
		" prio %d ifindex %d\n", dhd_ifname(dhd, flow_ring_node->flow_info.ifindex),
		__FUNCTION__, flow_ring_node->flowid, MAC2STRDBG(flow_ring_node->flow_info.da),
		flow_ring_node->flow_info.tid, flow_ring_node->flow_info.ifindex));

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, flow_delete_rqst, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_OK;
}

static void BCMFASTPATH
dhd_prot_flow_ring_fastdelete(dhd_pub_t *dhd, uint16 flowid, uint16 rd_idx)
{
	flow_ring_node_t *flow_ring_node = DHD_FLOW_RING(dhd, flowid);
	msgbuf_ring_t *ring = (msgbuf_ring_t *)flow_ring_node->prot_info;
	host_txbuf_cmpl_t txstatus;
	uint16 wr_idx;
	void *txp_wi_start;

	DHD_INFO(("%s: FAST delete ring, flowid=%d, rd_idx=%d, wr_idx=%d\n",
		__FUNCTION__, flowid, rd_idx, ring->wr));

#if defined(BCM_DHD_RUNNER)
	if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
	    dhd_runner_notify(dhd->runner_hlp, H2R_FLRING_FLUSH_NOTIF, flowid, rd_idx);
	    return;
	}
#endif /* BCM_DHD_RUNNER */

	memset(&txstatus, 0, sizeof(txstatus));
	txstatus.compl_hdr.flow_ring_id = flowid;
	txstatus.cmn_hdr.if_id = flow_ring_node->flow_info.ifindex;
	wr_idx = ring->wr;

	while (wr_idx != rd_idx) {
		if (wr_idx) {
			wr_idx--;
		} else {
			wr_idx = ring->max_items - 1;
		}
		txp_wi_start = ((char *)DHD_RING_BGN_VA(ring) + (wr_idx * ring->item_size));

		switch (ring->item_type) {
			case MSGBUF_WI_WI64: /* Legacy Work item */
			{
				host_txbuf_post_t *wi64 = (host_txbuf_post_t *)txp_wi_start;
				txstatus.cmn_hdr.request_id = wi64->cmn_hdr.request_id;
				break;
			}
			case MSGBUF_WI_CWI32:
			{
				hwa_txpost_cwi32_t *cwi32 = (hwa_txpost_cwi32_t *)txp_wi_start;
				txstatus.cmn_hdr.request_id = cwi32->host_pktid;
				break;
			}
			case MSGBUF_WI_CWI64:
			{
				hwa_txpost_cwi64_t *cwi64 = (hwa_txpost_cwi64_t *)txp_wi_start;
				txstatus.cmn_hdr.request_id = cwi64->host_pktid;
				break;
			}
			default:
				DHD_ERROR(("%s Invalid item_type %u\n", __FUNCTION__,
					ring->item_type));
				ASSERT(0);
		}
		dhd_prot_txstatus_process(dhd, &txstatus);
	}
}

static void
dhd_prot_flow_ring_delete_response_process(dhd_pub_t *dhd, void *msg)
{
	tx_flowring_delete_response_t *flow_delete_resp = (tx_flowring_delete_response_t *)msg;

	DHD_INFO(("%s: Flow Delete Response status = %d \n", __FUNCTION__,
		flow_delete_resp->cmplt.status));

	if (dhd->fast_delete_ring_support) {
		dhd_prot_flow_ring_fastdelete(dhd, flow_delete_resp->cmplt.flow_ring_id,
			flow_delete_resp->read_idx);
	}
	dhd_bus_flow_ring_delete_response(dhd->bus, flow_delete_resp->cmplt.flow_ring_id,
		flow_delete_resp->cmplt.status);
}

int
dhd_prot_flow_ring_flush(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node)
{
	tx_flowring_flush_request_t *flow_flush_rqst;
	dhd_prot_t *prot = dhd->prot;
	unsigned long flags;
	uint16 alloced = 0;
	msgbuf_ring_t *ring = &prot->h2dring_ctrl_subn;

	DHD_GENERAL_LOCK(dhd, flags);

	/* Request for ring buffer space */
	flow_flush_rqst = (tx_flowring_flush_request_t *)
		dhd_prot_alloc_ring_space(dhd, ring, 1, &alloced, FALSE);
	if (flow_flush_rqst == NULL) {
		DHD_GENERAL_UNLOCK(dhd, flags);
		DHD_ERROR(("%s: Flow Flush Req - failure ring space\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	/* Common msg buf hdr */
	flow_flush_rqst->msg.msg_type = MSG_TYPE_FLOW_RING_FLUSH;
	flow_flush_rqst->msg.if_id = (uint8)flow_ring_node->flow_info.ifindex;
	flow_flush_rqst->msg.request_id = htol32(0); /* TBD */

	flow_flush_rqst->msg.epoch = ring->seqnum % H2D_EPOCH_MODULO;
	ring->seqnum++;

	flow_flush_rqst->flow_ring_id = htol16((uint16)flow_ring_node->flowid);
	flow_flush_rqst->reason = htol16(BCME_OK);

	DHD_INFO(("%s: Send Flow Flush Req\n", __FUNCTION__));

	/* update ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ring, flow_flush_rqst, 1);
	DHD_GENERAL_UNLOCK(dhd, flags);

	return BCME_OK;
} /* dhd_prot_flow_ring_flush */

static void
dhd_prot_flow_ring_flush_response_process(dhd_pub_t *dhd, void *msg)
{
	tx_flowring_flush_response_t *flow_flush_resp = (tx_flowring_flush_response_t *)msg;

	DHD_INFO(("%s: Flow Flush Response status = %d\n", __FUNCTION__,
		flow_flush_resp->cmplt.status));

	dhd_bus_flow_ring_flush_response(dhd->bus, flow_flush_resp->cmplt.flow_ring_id,
		flow_flush_resp->cmplt.status);
}

/**
 * Request dongle to configure soft doorbells for D2H rings. Host populated soft
 * doorbell information is transferred to dongle via the d2h ring config control
 * message.
 */
void
dhd_msgbuf_ring_config_d2h_soft_doorbell(dhd_pub_t *dhd)
{
#if defined(DHD_D2H_SOFT_DOORBELL_SUPPORT)
	uint16 ring_idx;
	uint8 *msg_next;
	void *msg_start;
	uint16 alloced = 0;
	unsigned long flags;
	dhd_prot_t *prot = dhd->prot;
	ring_config_req_t *ring_config_req;
	bcmpcie_soft_doorbell_t *soft_doorbell;
	msgbuf_ring_t *ctrl_ring = &prot->h2dring_ctrl_subn;
	uint16 d2h_rings = BCMPCIE_D2H_COMMON_MSGRINGS;

	/*
	 * Legacy IPC Rev or older dongle image only has 1 RxCpl, 3 D2H common rings.
	 * So here only dongle maximum D2H rings need to be configured.
	*/
	if (prot->max_rxcpln_rings == 1) {
		/* Remove 3 RxCpl rings */
		ASSERT(BCMPCIE_D2H_COMMON_MSGRINGS == 6);
		d2h_rings -= 3;
	}
	DHD_ERROR(("Config doorbell for d2h_rings <%u>\n", d2h_rings));

	/* Claim space for d2h_ring number of d2h_ring_config_req_t messages */
	DHD_GENERAL_LOCK(dhd, flags);
	msg_start = dhd_prot_alloc_ring_space(dhd, ctrl_ring, d2h_rings, &alloced, TRUE);

	if (msg_start == NULL) {
		DHD_ERROR(("%s Msgbuf no space for %d D2H ring config soft doorbells\n",
			__FUNCTION__, d2h_rings));
		DHD_GENERAL_UNLOCK(dhd, flags);
		return;
	}

	msg_next = (uint8*)msg_start;

	for (ring_idx = 0; ring_idx < d2h_rings; ring_idx++) {

		/* position the ring_config_req into the ctrl subm ring */
		ring_config_req = (ring_config_req_t *)msg_next;

		/* Common msg header */
		ring_config_req->msg.msg_type = MSG_TYPE_D2H_RING_CONFIG;
		ring_config_req->msg.if_id = 0;
		ring_config_req->msg.flags = 0;

		ring_config_req->msg.epoch = ctrl_ring->seqnum % H2D_EPOCH_MODULO;
		ctrl_ring->seqnum++;

		ring_config_req->msg.request_id = htol32(DHD_FAKE_PKTID); /* unused */

		/* Ring Config subtype and d2h ring_id */
		ring_config_req->subtype = htol16(D2H_RING_CONFIG_SUBTYPE_SOFT_DOORBELL);
		ring_config_req->ring_id = htol16(DHD_D2H_RINGID(ring_idx));

		/* Host soft doorbell configuration */
		soft_doorbell = &prot->soft_doorbell[ring_idx];

		ring_config_req->soft_doorbell.value = htol32(soft_doorbell->value);
		ring_config_req->soft_doorbell.haddr.high =
			htol32(soft_doorbell->haddr.high);
		ring_config_req->soft_doorbell.haddr.low =
			htol32(soft_doorbell->haddr.low);
		ring_config_req->soft_doorbell.items = htol16(soft_doorbell->items);
		ring_config_req->soft_doorbell.msecs = htol16(soft_doorbell->msecs);

		DHD_INFO(("%s: Soft doorbell haddr 0x%08x 0x%08x value 0x%08x\n",
			__FUNCTION__, ring_config_req->soft_doorbell.haddr.high,
			ring_config_req->soft_doorbell.haddr.low,
			ring_config_req->soft_doorbell.value));

		msg_next = msg_next + ctrl_ring->item_size;
	}

	/* update control subn ring's WR index and ring doorbell to dongle */
	dhd_prot_ring_write_complete(dhd, ctrl_ring, msg_start, d2h_rings);
	DHD_GENERAL_UNLOCK(dhd, flags);
#endif /* DHD_D2H_SOFT_DOORBELL_SUPPORT */
}

static void
dhd_prot_d2h_ring_config_cmplt_process(dhd_pub_t *dhd, void *msg)
{
	DHD_INFO(("%s: Ring Config Response - status %d ringid %d\n",
		__FUNCTION__, ltoh16(((ring_config_resp_t *)msg)->compl_hdr.status),
		ltoh16(((ring_config_resp_t *)msg)->compl_hdr.flow_ring_id)));
}

int
dhd_prot_ringupd_dump(dhd_pub_t *dhd, struct bcmstrbuf *b)
{
	uint32 *ptr;
	uint32 value;
	uint32 i;
	uint32 max_h2d_rings = dhd_bus_max_h2d_rings(dhd->bus);
	dhd_prot_t *prot = dhd->prot;

	OSL_CACHE_INV((void *)prot->d2h_dma_indx_wr_buf.va,
		prot->d2h_dma_indx_wr_buf.len);

	ptr = (uint32 *)(prot->d2h_dma_indx_wr_buf.va);

	bcm_bprintf(b, "\n max_tx_rings %u\n", max_h2d_rings);

	bcm_bprintf(b, "\nRPTR block H2D common rings, 0x%04x\n", ptr);
	value = ltoh32(*ptr);
	bcm_bprintf(b, "\tH2D CTRL: value 0x%04x\n", value);
	ptr++;
	value = ltoh32(*ptr);
	bcm_bprintf(b, "\tH2D RXPOST: value 0x%04x\n", value);

	ptr++;
	bcm_bprintf(b, "RPTR block Flow rings , 0x%04x\n", ptr);
	for (i = BCMPCIE_H2D_COMMON_MSGRINGS; i < max_h2d_rings; i++) {
		value = ltoh32(*ptr);
		bcm_bprintf(b, "\tflowring ID %u: value 0x%04x\n", i, value);
		ptr++;
	}

	OSL_CACHE_INV((void *)prot->h2d_dma_indx_rd_buf.va,
		prot->h2d_dma_indx_rd_buf.len);

	ptr = (uint32 *)(prot->h2d_dma_indx_rd_buf.va);

	bcm_bprintf(b, "\nWPTR block D2H common rings, 0x%04x\n", ptr);
	value = ltoh32(*ptr);
	bcm_bprintf(b, "\tD2H CTRLCPLT: value 0x%04x\n", value);
	ptr++;
	value = ltoh32(*ptr);
	bcm_bprintf(b, "\tD2H TXCPLT: value 0x%04x\n", value);
	ptr++;
	value = ltoh32(*ptr);
	bcm_bprintf(b, "\tD2H RXCPLT: value 0x%04x\n", value);

	return 0;
}

uint32
dhd_prot_metadata_dbg_set(dhd_pub_t *dhd, bool val)
{
	ASSERT(0);

	return 0U;
}

uint32
dhd_prot_metadata_dbg_get(dhd_pub_t *dhd)
{
	ASSERT(0);

	return 0U;
}

uint32
dhd_prot_metadatalen_set(dhd_pub_t *dhd, uint32 val, bool rx)
{
	ASSERT(0);

	return 0U;
}

uint32
dhd_prot_metadatalen_get(dhd_pub_t *dhd, bool rx)
{
	ASSERT(0);

	return 0U;
}

/** optimization to write "n" tx items at a time to ring */
uint32
dhd_prot_txp_threshold(dhd_pub_t *dhd, bool set, uint32 val)
{
	dhd_prot_t *prot = dhd->prot;

	if (set)
		prot->txp_threshold = (uint16)val;
	val = prot->txp_threshold;

	return val;
}

#ifdef DHD_RX_CHAINING

static INLINE void BCMFASTPATH
dhd_rxchain_reset(rxchain_info_t *rxchain)
{
	rxchain->pkt_count = 0;
}

static void BCMFASTPATH
dhd_rxchain_frame(dhd_pub_t *dhd, void *pkt, uint ifidx)
{
	uint8 *eh;
	uint8 prio;
	dhd_prot_t *prot = dhd->prot;
	rxchain_info_t *rxchain = &prot->rxchain;

	ASSERT(!PKTISCHAINED(pkt));
	ASSERT(PKTCLINK(pkt) == NULL);
	ASSERT(PKTCGETATTR(pkt) == 0);

	eh = PKTDATA(dhd->osh, pkt);
	prio = IP_TOS46(eh + ETHER_HDR_LEN) >> IPV4_TOS_PREC_SHIFT;

#if defined(BCM_CPE_PKTC)
	if (rxchain->pkt_count && !(PKTC_TBL_CHAINABLE(dhd, ifidx, eh, prio, rxchain->h_sa,
		rxchain->h_da, rxchain->h_prio)))
#else
	if (rxchain->pkt_count && !(PKT_CTF_CHAINABLE(dhd, ifidx, eh, prio, rxchain->h_sa,
		rxchain->h_da, rxchain->h_prio)))
#endif /* BCM_CPE_PKTC */
	{
		/* Different flow - First release the existing chain */
		dhd_rxchain_commit(dhd);
	}

	/* For routers, with HNDCTF, link the packets using PKTSETCLINK, */
	/* so that the chain can be handed off to CTF bridge as is. */
	if (rxchain->pkt_count == 0) {
		/* First packet in chain */
		rxchain->pkthead = rxchain->pkttail = pkt;

		/* Keep a copy of ptr to ether_da, ether_sa and prio */
		rxchain->h_da = ((struct ether_header *)eh)->ether_dhost;
		rxchain->h_sa = ((struct ether_header *)eh)->ether_shost;
		rxchain->h_prio = prio;
		rxchain->ifidx = ifidx;
		rxchain->pkt_count++;
#if defined(BCM_CPE_PKTC)
		if (!dhd_pktc_tbl_check((dhd), (eh), (ifidx)))
		{
		   dhd_rxchain_commit(dhd);
		   return;
		}
#endif /* BCM_CPE_PKTC */
	} else {
		/* Same flow - keep chaining */
		PKTSETCLINK(rxchain->pkttail, pkt);
		rxchain->pkttail = pkt;
		rxchain->pkt_count++;
	}

#if defined(BCM_CPE_PKTC)
	if ((dhd_rx_pktc_tbl_chainable(dhd, ifidx)) && (!ETHER_ISMULTI(rxchain->h_da)) &&
#else
	if ((dhd_rx_pkt_chainable(dhd, ifidx)) && (!ETHER_ISMULTI(rxchain->h_da)) &&
#endif /* BCM_CPE_PKTC */
		((((struct ether_header *)eh)->ether_type == HTON16(ETHER_TYPE_IP)) ||
		(((struct ether_header *)eh)->ether_type == HTON16(ETHER_TYPE_IPV6)))) {
		PKTSETCHAINED(dhd->osh, pkt);
		PKTCINCRCNT(rxchain->pkthead);
		PKTCADDLEN(rxchain->pkthead, PKTLEN(dhd->osh, pkt));
	} else {
		dhd_rxchain_commit(dhd);
		return;
	}

#if defined(BCM_CPE_PKTC)
	dhd->cur_pktccnt = rxchain->pkt_count;
	if (dhd->cur_pktccnt > dhd->max_pktccnt)
		dhd->max_pktccnt = dhd->cur_pktccnt;
#endif /* BCM_CPE_PKTC */

	/* If we have hit the max chain length, dispatch the chain and reset */
#if defined(BCM_CPE_PKTC)
	if (rxchain->pkt_count >= dhd->pktcbnd) {
#else
	if (rxchain->pkt_count >= DHD_PKT_CTF_MAX_CHAIN_LEN) {
#endif /* BCM_CPE_PKTC */
		dhd_rxchain_commit(dhd);
	}
}

static void BCMFASTPATH
dhd_rxchain_commit(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	rxchain_info_t *rxchain = &prot->rxchain;

	if (rxchain->pkt_count == 0)
		return;

	/* Release the packets to dhd_linux */
	dhd_bus_rx_frame(dhd->bus, rxchain->pkthead, rxchain->ifidx, rxchain->pkt_count);

	/* Reset the chain */
	dhd_rxchain_reset(rxchain);
}

#endif /* DHD_RX_CHAINING */

#if defined(BCM_DHD_RUNNER)
static INLINE void _dhd_prot_schedule_runner(dhd_pub_t *dhd,
	msgbuf_ring_t *ring, int is_tx_complete);

static INLINE void
_dhd_prot_schedule_runner(dhd_pub_t *dhd, msgbuf_ring_t *ring, int is_tx_complete)
{
	uint8 *msg_addr;
	uint32 msg_len;

	msg_addr = dhd_prot_get_read_addr(dhd, ring, &msg_len);
	if (msg_addr == NULL)
		return;

	if (is_tx_complete) {
		dhd_runner_notify(dhd->runner_hlp, H2R_TX_COMPL_NOTIF, 0, 0);
	} else {
		dhd_runner_notify(dhd->runner_hlp, H2R_RX_COMPL_NOTIF, 0, 0);
	}

}

void
dhd_prot_schedule_runner(dhd_pub_t *dhd)
{
	if (DHD_RNR_OFFL_TXSTS(dhd)) {
		_dhd_prot_schedule_runner(dhd, &dhd->prot->d2hring_tx_cpln, 1);
	}

	if (DHD_RNR_OFFL_RXCMPL(dhd)) {
		/* XXX, use first RxCpl ring for now.
		 * Revisit it when we need runner to support 4 RxCpl
		 */
		_dhd_prot_schedule_runner(dhd, dhd->prot->d2hring_rx_cpln, 0);
	}
}

/** called on MSG_TYPE_TX_STATUS message received from dongle for non-accelerated packets */
void
dhd_prot_runner_txstatus_process(dhd_pub_t *dhd, void* msg)
{
	dhd_prot_t *prot = dhd->prot;
	DHD_LOCK(dhd);

	if (!DHD_RNR_OFFL_TXSTS(dhd)) {
	    /* Runner should never send this message if txsts is not offloaded */
	    DHD_ERROR(("%s: Unexpected TX_STATUS from Runner\r\n",
	        __FUNCTION__));
	    ASSERT(FALSE);
	    DHD_UNLOCK(dhd);
	    return;
	}

	if (prot->txpost_item_type == MSGBUF_WI_WI64) {
	    flow_ring_node_t *flow_ring_node;
	    host_txbuf_cmpl_t * txstatus;
	    int	ifidx;
	    txstatus = (host_txbuf_cmpl_t *)msg;
	    flow_ring_node = DHD_FLOW_RING(dhd, ltoh16(txstatus->compl_hdr.flow_ring_id));
	    ifidx = flow_ring_node->flow_info.ifindex;
	    ASSERT(flow_ring_node->flowid == ltoh16(txstatus->compl_hdr.flow_ring_id));

	    if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
#if defined(DHD_RNR_NO_NONACCPKT_TXSTSOFFL)
	        if (DHD_RNR_OFFL_NONACCPKT_TXSTS(dhd)) {
	            /* Runner should never send this message on offloaded ring */
	            DHD_ERROR(("%s: %s: Unexpected TX_STATUS from Runner on flowring <%d>\r\n",
	               dhd_ifname(dhd, ifidx), __FUNCTION__, flow_ring_node->flowid));
	            ASSERT(FALSE);
	            DHD_UNLOCK(dhd);
	            return;
	        } else {
	            dmaaddr_t pa;
	            msgbuf_ring_t *ring = (msgbuf_ring_t*)(flow_ring_node->prot_info);
	            void *pkt = NULL;
	            uint32 pktid;

	            pkt = ((dhd_runner_txsts_t*)msg)->pkt;
	            if (pkt != NULL) {
	                unsigned long flags;

	                /* If pkt pointer is specified, need to convert pkt to request_id */
	                ULONGTOPHYSADDR((ulong)virt_to_phys(PKTDATA(dhd->osh, pkt)), pa);

	                /* locks required to protect circular buffer accesses */
	                DHD_GENERAL_LOCK(dhd, flags);

	                /* Create a unique 32-bit packet id */
	                pktid = DHD_NATIVE_TO_PKTID(dhd, prot->pktid_map_handle, pkt,
	                    pa, PKTLEN(dhd->osh, pkt), DMA_TX, NULL,
	                    ring->dma_buf.secdma, PKTTYPE_DATA_TX);

#if defined(DHD_PCIE_PKTID)
	                if (pktid == DHD_PKTID_INVALID) {
				DHD_ERROR(("%s: %s:%d Pktid pool depleted.\n",
					dhd_ifname(dhd, ifidx), __FUNCTION__, __LINE__));
	                    ASSERT(FALSE);
	                    /* Free the packet */
	                    PKTFREE(dhd->osh, pkt, TRUE);
	                    DHD_GENERAL_UNLOCK(dhd, flags);
	                    DHD_UNLOCK(dhd);
	                    return;
	                }
#endif /* DHD_PCIE_PKTID */

	                DHD_INFO(("Got pktid[0x%x] for NON accelerated TXPKT[0x%p]\r\n",
	                    pktid, pkt));
	                txstatus->cmn_hdr.request_id = htol32(pktid);
	                prot->active_tx_count++;

	                DHD_GENERAL_UNLOCK(dhd, flags);
	            }
	    }
#else /* !DHD_RNR_NO_NONACCPKT_TXSTSOFFL */
	        /* Runner should never send this message on offloaded ring */
	        DHD_ERROR(("%s: %s: Unexpected TX_STATUS from Runner on flowring <%d>\r\n",
	            dhd_ifname(dhd, ifidx), __FUNCTION__, flow_ring_node->flowid));
	        DHD_UNLOCK(dhd);
	        ASSERT(FALSE);
	        return;
#endif /* !DHD_RNR_NO_NONACCPKT_TXSTSOFFL */
	    }
	    /*
	     * interface index is not available from Runner
	     * Fill the interface index from ring id info
	    */
	    txstatus->cmn_hdr.if_id = ifidx;

	    dhd_prot_txstatus_process(dhd, msg);
	} else {
	    void *pkt = ((dhd_runner_txsts_t*)msg)->pkt;

	    /*
	     * pkt is valid for offloaded flow ring.
	     * for non-offloaded flow-rings, convert request_id to pkt
	     */
	    if (!pkt) {
			ulong pa;
			uint32 len;
			void *dmah, *secdma;

			pkt = DHD_PKTID_TO_NATIVE(dhd, prot->pktid_map_handle,
				ltoh32(((dhd_runner_txsts_t*)msg)->dngl_txsts.cmn_hdr.request_id),
				pa, len, dmah, secdma, PKTTYPE_DATA_TX);
	    }

	    if (pkt) {
#if defined(BCMPCIE)
	        OSL_PREFETCH(pkt);
	        dhd_txcomplete(dhd, pkt, true); /* pass up to upper OS layer */
#endif /* BCMPCIE */
	        PKTFREE(dhd->osh, pkt, TRUE);
	    } else { /* BAD: pkt == NULL */
	        DHD_ERROR(("%s pkt received from runner is NULL pkt\n",
	            __FUNCTION__));
	    }
	}

	DHD_UNLOCK(dhd);
} /* dhd_prot_runner_txstatus_process */

/**
 * dhd_prot_runner_preinit
 * - Configure msgring format in the runner
 * - Update tx and rx offload status
 */
int
dhd_prot_runner_preinit(dhd_pub_t *dhdp)
{
	dhd_prot_t *prot = dhdp->prot;
	dhd_helper_status_t rnr_status;
	int rc;

	/*
	 * Configure Ring format (item_type and item_size) in the runner
	 *  00:15  ring type (MSGBUF_WI_XXX)
	 *  16:31  ring item size
	 */

	/** TxPost Flowring configuration */
	rc = dhd_runner_notify(dhdp->runner_hlp, H2R_MSGRING_FORMAT_NOTIF,
	    ID16_INVALID,
	    ((prot->txpost_item_size << 16) | prot->txpost_item_type));
	if (rc != BCME_OK) {
	    DHD_ERROR(("Failed to configure H2D TxPost ring size %u type %2u in runner\r\n",
	        prot->txpost_item_size, prot->txpost_item_type));
	    return rc;
	}

	/* Get current tx and rx offload status from dhd_runner */
	rc = dhd_runner_do_iovar(dhdp->runner_hlp, DHD_RNR_IOVAR_STATUS,
	    FALSE, (char*)&rnr_status, sizeof(rnr_status));
	if (rc != BCME_OK) {
	    DHD_ERROR(("Failed to get status from dhd_runner\r\n"));
	    return rc;
	}

#if defined(DHD_D2H_SOFT_DOORBELL_SUPPORT)
	/* Update the TX and RX offload status after pre-init stage */
	/* Tx Complete and Rx Complete by default processed by DHD */
	if (rnr_status.en_features.txoffl) {
	    DHD_RNR_SET_TXSTS_OFFL(dhdp);
#if defined(DHD_RNR_NO_NONACCPKT_TXSTSOFFL)
	    /*
	     * Disable TXSTS offload for dhd originated txpost packets
	     * and enable only for accelerated packets.
	     * DHD needs to process TXSTS for non-accelerated packets
	     * (e.g 802.1x) that are origniated from DHD
	     */
	    if (dhd_runner_notify(dhdp->runner_hlp, H2R_TXSTS_CONFIG_NOTIF,
	        DHD_RNR_TXSTS_CFG_ACCPKT_OFFL, 0) != BCME_OK)
#endif /* DHD_RNR_NO_NONACCPKT_TXSTSOFFL */
	    DHD_RNR_SET_NONACCPKT_TXSTS_OFFL(dhdp);
	} else {
	    /* Clear doorbell information */
	    memset(&prot->soft_doorbell[BCMPCIE_D2H_MSGRING_TX_COMPLETE_IDX],
	        0, sizeof(bcmpcie_soft_doorbell_t));
	}

	if (rnr_status.en_features.rxoffl) {
	    DHD_RNR_SET_RXCMPL_OFFL(dhdp);
	} else {
	    /* Clear doorbell information */
	    memset(&prot->soft_doorbell[BCMPCIE_D2H_MSGRING_RX_COMPLETE_IDX],
	        0, sizeof(bcmpcie_soft_doorbell_t));
	}
#endif /* DHD_D2H_SOFT_DOORBELL_SUPPORT */

	return rc;
} /* dhd_prot_runner_preinit */
#endif /* BCM_DHD_RUNNER */

#ifdef BCMHWA

void
dhd_prot_set_hwa_caps(dhd_pub_t *dhd, uint32 caps)
{
	dhd_prot_t *prot = dhd->prot;

	prot->hwa_caps = caps;
}

/* Setup HWA attribute for RxPost, TxCpl and RxCpl rings */
void
dhd_prot_set_hwa_attributes(dhd_pub_t *dhd)
{
	uint32 i;
	msgbuf_ring_t *ring;
	dhd_prot_t *prot = dhd->prot;

	/* RxPost */
	ring = &prot->h2dring_rxp_subn;
	if (prot->hwa_caps & PCIE_IPC_HCAP1_HWA_RXPOST_IDMA)
		ring->flag |= DHD_RING_FLAG_HWA_RXPOST;
	else
		ring->flag &= ~DHD_RING_FLAG_HWA_RXPOST;

	/* TxCpl */
	ring = &prot->d2hring_tx_cpln;
	if (prot->hwa_caps & PCIE_IPC_HCAP1_HWA_TXCPL_IDMA)
		ring->flag |= DHD_RING_FLAG_HWA_TXCPL;
	else
		ring->flag &= ~DHD_RING_FLAG_HWA_TXCPL;

	/* RxCpl */
	if (prot->hwa_caps & PCIE_IPC_HCAP1_HWA_RXCPL_IDMA) {
		FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
			ring->flag |= DHD_RING_FLAG_HWA_RXCPL;
		}
	}
	else {
		FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
			ring->flag &= ~DHD_RING_FLAG_HWA_RXCPL;
		}
	}
}

/* Update RxPost WR manually inorder to trigger iDMA again */
void
dhd_prot_rxpost_upd(dhd_pub_t *dhd)
{
	dhd_prot_t *prot = dhd->prot;
	msgbuf_ring_t *ring = &prot->h2dring_rxp_subn;
	uint32 u32_db_index;
	uint16 wr;

	wr = ring->wr;
#if defined(BCM_DHD_RUNNER)
	if (DHD_RNR_OFFL_RXCMPL(dhd)) {
		/* Get the wr index from DMA index array */
		wr = dhd_prot_dma_indx_get(dhd, H2D_DMA_INDX_WR_UPD, ring->id);
	}
#endif  /* BCM_DHD_RUNNER */
	if (prot->hwa_caps & PCIE_IPC_HCAP1_HWA_RXPOST_IDMA) {
		/* raise h2d interrupt */
		u32_db_index = (wr << INDEX_VAL_SHIFT) |
			(DMA_TYPE_HWA_RXPOST << DMA_TYPE_SHIFT);
		prot->db1_2_ring_fn(dhd->bus, u32_db_index, TRUE);
	}
}

static INLINE void
dhd_msgbuf_rxpost_reclaim(dhd_pub_t *dhd, msgbuf_ring_t *ring)
{
	dhd_prot_ring_reset(dhd, ring);

	if (DMA_INDX_ENAB(dhd->dma_h2d_ring_upd_support)) {
		dhd_prot_dma_indx_set(dhd, ring->wr, H2D_DMA_INDX_WR_UPD, ring->id);
	} else {
		dhd_bus_cmn_writeshared(dhd->bus, &(ring->wr), RING_WR_UPD, ring->id);
	}

	if (DMA_INDX_ENAB(dhd->dma_d2h_ring_upd_support)) {
		dhd_prot_dma_indx_set(dhd, ring->rd, H2D_DMA_INDX_RD_UPD, ring->id);
	} else {
		dhd_bus_cmn_writeshared(dhd->bus, &(ring->rd), RING_RD_UPD, ring->id);
	}
}

static void
dhd_prot_rxpost_reclaim_pkt(dhd_pub_t *dhd, uint32 pktid)
{
	unsigned long flags;
	void *pkt;

	DHD_INFO(("%s pktid 0x%08x\n", __FUNCTION__, pktid));
	ASSERT(pktid != 0);

#if defined(DHD_PKTID_AUDIT_RING)
	DHD_PKTID_AUDIT(dhd, dhd->prot->pktid_map_handle, pktid, DHD_DUPLICATE_FREE);
#endif /* DHD_PKTID_AUDIT_RING */

	DHD_GENERAL_LOCK(dhd, flags);

	pkt = dhd_prot_packet_get(dhd, pktid, PKTTYPE_DATA_RX, TRUE);
	OSL_PREFETCH(pkt);

	DHD_GENERAL_UNLOCK(dhd, flags);

	if (!pkt) {
		DHD_ERROR(("%s pktid<%u> mapped to NULL pkt\n",
			__FUNCTION__, pktid));
		return;
	}

	PKTFREE(dhd->osh, pkt, FALSE);
}

/* Reclaim all RxPost workitems and free packets. */
static void
dhd_prot_rxpost_reclaim(dhd_pub_t *dhd)
{
	dhd_prot_t *prot;
	msgbuf_ring_t *ring;
	uint16 wr;
	uint16 rd;
	uint16 depth;
	uint16 items;
	uint16 i;
	uint32 available_len;
	uint32 pktid;
	void  *rxp_wi_start, *rxp_wi_end, *rxp_wi;

	prot = dhd->prot;
	ring = &dhd->prot->h2dring_rxp_subn;

	if (!(prot->hwa_caps & PCIE_IPC_HCAP1_HWA_RXPOST_IDMA)) {
		return;
	}

	/* Refresh the rxpost submission common ring's rd index */
	dhd_msgbuf_rxbuf_post_refresh(dhd);

	wr = ring->wr;
	rd = ring->rd;
	depth = ring->max_items;

	while (rd != wr) {
		/* check for avail space, in number of ring items */
		items = READ_AVAIL_SPACE(wr, rd, depth);
		if (items == 0) {
			break;
		}

		ASSERT(items < ring->max_items);

		prot->rxbufpost -= items;

		/* if space is available, calculate address to be read */
		rxp_wi_start = (char*)ring->dma_buf.va + (rd * ring->item_size);

		/* update read pointer */
		if ((rd + items) >= depth) {
			rd = 0;
		} else {
			rd += items;
		}

		ASSERT(ring->rd < depth);

		/* convert items to bytes : available_len must be 32bits */
		available_len = (uint32)(items * ring->item_size);

		OSL_CACHE_INV(rxp_wi_start, available_len);

		/* Prefetch data to populate the cache */
		OSL_PREFETCH(rxp_wi_start);

		rxp_wi = rxp_wi_start;
		rxp_wi_end = rxp_wi_start + available_len;

		while (rxp_wi < rxp_wi_end) {
			/* Suport for various RxPost Work Item Formats */
			switch (ring->item_type)
			{
				case MSGBUF_WI_CWI32: /* Compact Work Item with 32b haddr */
				{
					hwa_rxpost_cwi32_t *cwi32 = (hwa_rxpost_cwi32_t *)rxp_wi;
					pktid = cwi32->host_pktid;
					if (pktid != DHD_PKTID_INVALID) {
						dhd_prot_rxpost_reclaim_pkt(dhd, pktid);
					}
					break;
				}

				case MSGBUF_WI_CWI64: /* Compact Work Item with 64b haddr */
				{
					hwa_rxpost_cwi64_t *cwi64 = (hwa_rxpost_cwi64_t *)rxp_wi;
					pktid = cwi64->host_pktid;
					if (pktid != DHD_PKTID_INVALID) {
						dhd_prot_rxpost_reclaim_pkt(dhd, pktid);
					}
					break;
				}

				case MSGBUF_WI_ACWI32: /* Aggregated Compact with 32b haddr */
				{
					hwa_rxpost_acwi32_t *acwi32 = (hwa_rxpost_acwi32_t *)rxp_wi;
					for (i = 0; i < 4; i++) {
						pktid = acwi32->host_pktid[i];
						if (pktid != DHD_PKTID_INVALID) {
							dhd_prot_rxpost_reclaim_pkt(dhd, pktid);
						}
					}
					break;
				}

				case MSGBUF_WI_ACWI64: /* Aggregated Compact with 64b haddr */
				{
					hwa_rxpost_acwi64_t *acwi64 = (hwa_rxpost_acwi64_t *)rxp_wi;
					for (i = 0; i < 4; i++) {
						pktid = acwi64->host_pktid[i];
						if (pktid != DHD_PKTID_INVALID) {
							dhd_prot_rxpost_reclaim_pkt(dhd, pktid);
						}
					}
					break;
				}

				default: ASSERT(0); break;
			} /* switch item_type */

			/* prepare to process next work item */
			rxp_wi = (uint8 *)rxp_wi + ring->item_size;
			OSL_PREFETCH(rxp_wi);
		}
	}

	/* Set the rxpost submission common ring's rd index and wr index to 0 */
	dhd_msgbuf_rxpost_reclaim(dhd, ring);
}

void
dhd_prot_process_hwa_event(dhd_pub_t *dhd, uint32 reason)
{
	dhd_prot_t *prot;

	prot = dhd->prot;

	DHD_INFO(("%s reason 0x%08x\n", __FUNCTION__, reason));

	switch (reason) {
		case WLC_E_HWA_RX_STOP_REFILL:
			prot->rxpost_stop = TRUE;
			break;

		case WLC_E_HWA_RX_STOP:
			prot->rxpost_stop = TRUE;
			/* Reclaim all RxPost workitems and free packets. */
#if defined(BCM_DHD_RUNNER)
			if (DHD_RNR_OFFL_RXCMPL(dhd)) {
				msgbuf_ring_t *ring;
				uint16 i;

				/*
				 * Make sure all RX Complete rings are processed by Runner
				 * before issuing RXPOST_REINIT
				 */
				FOREACH_RING_IN_D2HRING_RX_CPLN(prot, ring, i) {
					uint16 wr, rd;
					int timeout = DHD_RXCPL_DRAIN_WAIT_TIME;

					do {
						/* Get the rd/wr index from DMA index array */
						rd = dhd_prot_dma_indx_get(dhd, D2H_DMA_INDX_RD_UPD,
								ring->id);
						wr = dhd_prot_dma_indx_get(dhd, D2H_DMA_INDX_WR_UPD,
								ring->id);

						if (rd == wr) break;

						OSL_DELAY(1000);
					} while (--timeout);

					if (timeout == 0) {
						DHD_ERROR(("%s dhd%d RXCple[%d] Ring drain timedout"
							" rd [%d] wr [%d]\n",
							__FUNCTION__, dhd->unit, i, rd, wr));
					}
				}

				/* Reclaim and re-init the RxPost WI */
				dhd_runner_notify(dhd->runner_hlp, H2R_RXPOST_REINIT_NOTIF, 0, 0);
			} else
#endif /* BCM_DHD_RUNNER */
			{
				dhd_prot_rxpost_reclaim(dhd);
			}
			dhd_prot_rxpost_upd(dhd);
			break;

		case WLC_E_HWA_RX_POST:
			prot->rxpost_stop = FALSE;
			/* Redo rxbuf_post to dongle */
#if defined(BCM_DHD_RUNNER)
			/* Nothing to do for Rx DoR, handled during RX_POST_STOP */
			if (!DHD_RNR_OFFL_RXCMPL(dhd))
#endif /* BCM_DHD_RUNNER */
			{
				dhd_msgbuf_rxbuf_post(dhd, FALSE);
			}
			/* Update WR manually, we need it in STA mode */
			dhd_prot_rxpost_upd(dhd);
			dhd_hwa_event(dhd);
			break;

		case WLC_E_HWA_RX_REINIT:
			prot->rxpost_stop = FALSE;
			/* Redo rxbuf_post to dongle */
#if defined(BCM_DHD_RUNNER)
			/* Nothing to do for Rx DoR, handled during RX_POST_STOP */
			if (!DHD_RNR_OFFL_RXCMPL(dhd))
#endif /* BCM_DHD_RUNNER */
			{
				dhd_msgbuf_rxbuf_post(dhd, FALSE);
			}
			dhd_hwa_event(dhd);
			break;

		case WLC_E_HWA_DHD_DUMP:
			dhd_dump_to_kernelog(dhd);
			break;

		default:
			DHD_ERROR(("Unknown HWA event <%u>\n", reason));
			break;
	}
}
#endif /* BCMHWA */
