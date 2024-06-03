/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: hnddma_priv.h 828974 2023-08-21 06:00:29Z $
 */

#ifndef _HNDDMA_PRIV_H_
#define _HNDDMA_PRIV_H_

#include <bcmpcie.h>	// For haddr64_t

/* debug/trace */
#ifdef BCMDBG
#define	DMA_ERROR(args) if (di && (!(*di->msg_level & 1))); else printf args
#define	DMA_TRACE(args) if (di && (!(*di->msg_level & 2))); else printf args
#elif defined(BCMDBG_ERR) || defined(BCMDBG_TXSTALL)
#define	DMA_ERROR(args) if (di && (!(*di->msg_level & 1))); else printf args
#define	DMA_TRACE(args)
#else
#define	DMA_ERROR(args)
#define	DMA_TRACE(args)
#endif /* BCMDBG */

#define DMA_PRINT(args) printf args
#define	DMA_NONE(args)

#define d32txregs	dregs.d32_u.txregs_32
#define d32rxregs	dregs.d32_u.rxregs_32
#define txd32		dregs.d32_u.txd_32
#define rxd32		dregs.d32_u.rxd_32

#define d64txregs	dregs.d64_u.txregs_64
#define d64rxregs	dregs.d64_u.rxregs_64
#define txd64		dregs.d64_u.txd_64
#define txd64short	dregs.d64_u.txd_64_short
#define rxd64		dregs.d64_u.rxd_64

#ifdef BCM_DMAPAD
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define DMAPADREQUIRED(di)		((di)->dmapad_required)
	#elif defined(BCMSDIODEV_ENABLED)
		#define DMAPADREQUIRED(di)		TRUE
	#else
		#define DMAPADREQUIRED(di)		FALSE
	#endif /* ROM_ENAB_RUNTIME_CHECK */
#else
	#define DMAPADREQUIRED(di)		FALSE
#endif /* BCM_DMAPAD */

/* BCM_DMA_TRANSCOHERENT */
#define DMA_TRANSCOHERENT(di)		((di)->trans_coherent)

#if defined(HNDDMA_RECLAIM_DMA_ATTACH)
	#define BCMATTACHFN_DMA_ATTACH(_fn)	BCMATTACHFN(_fn)
#else
	#define BCMATTACHFN_DMA_ATTACH(_fn)	(_fn)
#endif

#define	MAXNAMEL	14		/* 14 char names */

#define	DI_INFO(dmah)	((dma_info_t *)(uintptr)dmah)

#define DESCR_DEADBEEF
#if defined(__mips__) || defined(BCM47XX_CA9) || defined(STB) || defined(BCM_ROUTER) || \
	defined(BULK_PKTLIST)
#undef DESCR_DEADBEEF
#endif

/* SplitRX Feature for D11 <TCM,DDR> split pkt reception in Full Dongle Mode */
#if !defined(__mips__) && !defined(BCM47XX_CA9) && defined(DONGLEBUILD)
#define D11_SPLIT_RX_FD
#endif /* !__mips__ && !BCM47XX_CA9 && DONGLEBUILD */

#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCM_ROUTER))) || defined(BCM_SECURE_DMA)
/* Enable/Disable support for Scatter Gather List in RX direction */
#define SGLIST_RX_SUPPORT
#endif

#if defined(BULK_DESCR_FLUSH)
/* Flush length, given number of descriptors */
#define DMA64_FLUSH_LEN(di, num_desc)   ((di)->dd64_size * (num_desc))
#define dma64_rxd64(di, ix) (void *)((unsigned long)(&((dma64dd_t *)((di)->rxd64))[ix]))
#define dma64_txd64(di, ix) (void *)((unsigned long)(&((dma64dd_t *)((di)->txd64))[ix]))
#define dma64_txd64_short(di, ix) (void *)((unsigned long) \
	(&((dma64dd_short_t *)((di)->txd64short))[ix]))
#define DMA_BULK_DESCR_TX_START_INVALID -1
#define DMA_BULK_DESCR_TX_SET_INVALID(di)	(((di)->bulk_descr_tx_start_txout) = \
				DMA_BULK_DESCR_TX_START_INVALID)
#define DMA_BULK_DESCR_TX_IS_VALID(di)		(((di)->bulk_descr_tx_start_txout) != \
				DMA_BULK_DESCR_TX_START_INVALID)

#endif /* BULK_DESCR_FLUSH */

#define D11RX_WAR_MAX_BUF_SIZE 492
#define MAXRXBUFSZ 2048

#ifdef BULK_PKTLIST
#define DMA_BULK_PATH(di) ((di)->hnddma.dmactrlflags & DMA_CTRL_BULK_PROCESSING)
#define DMA_GET_NEXTPKT(p) PKTLINK((p))
#define DMA_APPEND_LIST(p, nextp) PKTSETLINK((p), (nextp))
#define DMA_GET_INDEX(osh, p) PKTDMAIDX((osh), (p))
#define DMA_SET_INDEX(osh, p, idx) PKTSETDMAIDX((osh), (p), (idx))
#define DMA_SET_LISTHEAD(di, pkt) ((di)->dma_pkt_list.head_pkt = (pkt))
#define DMA_SET_LISTTAIL(di, pkt) ((di)->dma_pkt_list.tail_pkt = (pkt))
#define DMA_GET_LISTHEAD(di) ((di)->dma_pkt_list.head_pkt)
#define DMA_GET_LISTTAIL(di) ((di)->dma_pkt_list.tail_pkt)
#else
#define DMA_BULK_PATH(di) (0)
#endif

#ifdef BULKRX_PKTLIST
#define DMA_BULKRX_PATH(di) ((di)->hnddma.dmactrlflags & DMA_CTRL_BULKRX_PROCESSING)
#else
#define DMA_BULKRX_PATH(di) (0)
#endif

typedef struct dma_hme_sgmt {
	dma64addr_t	hmeaddr;
	uint		hmelen;
	uint32		hmecurr;
} dma_hme_sgmt_t;

/* Structure for global DMA states and for storing information needed by all
 * DMA channels.
 *
 */
struct dma_common {
	void		*osh;		/* os handle */

	uint8		last_qsel;	/* last queue index that was configured in IndQSel */

	/* INDIRECT DMA channel registers for rev>=64 */
	uint		reg_count;	/* number of valid suspreq/flushreq entries */
	volatile uint32 *indqsel;
	volatile uint32 *suspreq[BCM_DMA_REQ_REG_COUNT];
	volatile uint32 *flushreq[BCM_DMA_REQ_REG_COUNT];
	volatile uint32 *puqaqmringdepth;
	volatile uint32 *suspflush_grpsel;

	/* HostMemoryExtension info for Hybrid DMA descriptor table */
	uint16		sgmt_avail;
	dma_hme_sgmt_t  *sgmt;
#ifdef BCM_PCAP
	void		*pcap_hdl;
#endif /* BCM_PCAP */
};

/**< for bulk DMA processing */
typedef struct dma_pktlist {
	void *head_pkt;
	void *tail_pkt;
#ifdef BULK_PKTLIST_DEBUG
	void *last_pkt;
	void *last_bulk_pkt;
	uint16 last_txin;
	uint16 last_bulk_txin;
	uint16 last_bulk_ncons;
	uint16 last_bulk_npkt;
	uint16 npkt;
#endif
} dma_pktlist_t;

#ifdef BCM_HWALITE
struct dma_mlo_aux_info {
	uint16		nrxd;		/* # rx descriptors tunable */
	uint16		rxin;		/* index of next descriptor to reclaim */
	uint16		rxout;		/* index of next descriptor to post */
	uint16		rs0cd;		/* cached value of rcvstatus0 currdescr */
	dma_pktlist_t	dma_rx_pkt_list;
};
#endif /* BCM_HWALITE */

/** dma engine software state */
typedef struct dma_info {
	struct hnddma_pub hnddma;	/* exported structure, don't use hnddma_t,
					 * which could be const
					 */
	uint		*msg_level;	/* message level pointer */

	void		*osh;		/* os handle */
	si_t		*sih;		/* sb handle */

	uint32		q_index;	/* index of the queue */
	uint32		q_index_mask;	/* index of the queue as a register bitmap */
	volatile uint32	*suspreq;	/* SuspReq[x] register pointer */
	volatile uint32	*flushreq;	/* FlushReq[x] register pointer */

	union {
		struct {
			dma32regs_t	*txregs_32;	/* 32-bit dma tx engine registers */
			dma32regs_t	*rxregs_32;	/* 32-bit dma rx engine registers */
			dma32dd_t	*txd_32;	/* pointer to dma32 tx descriptor ring */
			dma32dd_t	*rxd_32;	/* pointer to dma32 rx descriptor ring */
		} d32_u;
		struct {
			dma64regs_t	*txregs_64;	/* 64-bit dma tx engine registers */
			dma64regs_t	*rxregs_64;	/* 64-bit dma rx engine registers */
			union {
			dma64dd_t	*txd_64;	/* pointer to dma64 tx descriptor ring */
			dma64dd_short_t	*txd_64_short;	/* pointer to short aqm descriptor ring */
			};
			dma64dd_t	*rxd_64;	/* pointer to dma64 rx descriptor ring */
		} d64_u;
	} dregs;

	/** the number of lsb address bits that need to be zero for start of dma descriptor ring */
	uint16		dmadesc_align;

	uint16		ntxd;		/* # tx descriptors tunable */
	uint16		txin;		/* index of next descriptor to reclaim */
	uint16		txout;		/* index of next descriptor to post */
	void		**txp;		/* pointer to parallel array of pointers to packets */
	osldma_t	*tx_dmah;	/* DMA TX descriptor ring handle */
	hnddma_seg_map_t	*txp_dmah;	/* DMA MAP meta-data handle */
	dmaaddr_t	txdpa;		/* Aligned physical address of descriptor ring */
	dmaaddr_t	txdpaorig;	/* Original physical address of descriptor ring */
	uint32		txdalloc;	/* #bytes allocated for the ring */
	uint32		xmtptrbase;	/* When using unaligned descriptors, the ptr register
					 * is not just an index, it needs all 13 bits to be
					 * an offset from the addr register.
					 */
	uint32		xmtcontrol;	/* xmit control value used when enable PUQ */
	uint32		txdpahi;	/* High address of txdpa for dongle uses BCMHME */
	uint16		txdalign;	/* #bytes added to alloc'd mem to align txd */

	uint16		nrxd;		/* # rx descriptors tunable */
	/**
	 * @rxin  Index of first posted descriptor to be reclaimed by firmware. The buffer that the
	 * descriptor points at may currently be either empty or filled by DMA.
	 */
	uint16		rxin;
	uint16		rxout;		/* index of next descriptor to post */
	void		**rxp;		/* pointer to parallel array of pointers to packets */
	osldma_t	*rx_dmah;	/* DMA RX descriptor ring handle */
	hnddma_seg_map_t	*rxp_dmah;	/* DMA MAP meta-data handle */
	dmaaddr_t	rxdpa;		/* Aligned physical address of descriptor ring */
	dmaaddr_t	rxdpaorig;	/* Original physical address of descriptor ring */
	uint32		rxdalloc;	/* #bytes allocated for the ring */
	uint32		rcvptrbase;	/* Base for ptr reg when using unaligned descriptors */
	uint32		rxdpahi;	/* High address of rxdpa for dongle uses BCMHME */
	uint16		rxdalign;	/* #bytes added to alloc'd mem to align rxd */

	/* tunables */
	uint16		rxbufsize;	/* rx buffer size in bytes,
					 * not including the extra headroom
					 */
	uint		rxextrahdrroom;	/* extra rx headroom, reverseved to assist upper stack
					 *  e.g. some rx pkt buffers will be bridged to tx side
					 *  without byte copying. The extra headroom needs to be
					 *  large enough to fit txheader needs.
					 *  Some dongle driver may not need it.
					 */
	uint		nrxpost;	/* # rx buffers to keep posted */
	uint		rxoffset;	/* rxcontrol offset */
	uint		ddoffsetlow;	/* add to get dma address of descriptor ring, low 32 bits */
	uint		rxddoffsethigh;	/*   high 32 bits for rx direction */
	uint		txddoffsethigh;	/*   high 32 bits for tx direction */
	uint		dataoffsetlow;	/* add to get dma address of data buffer, low 32 bits */
	uint		dataoffsethigh;	/*   high 32 bits */
	uint8		rxburstlen;	/* burstlen field for rx (for cores supporting burstlen) */
	uint8		txburstlen;	/* burstlen field for tx (for cores supporting burstlen) */
	uint8		txmultioutstdrd;	/* tx multiple outstanding reads */
	uint8		txprefetchctl;	/* prefetch control for tx */
	uint8		txprefetchthresh;	/* prefetch threshold for tx */
	uint8		rxprefetchctl;	/* prefetch control for rx */
	uint8		rxprefetchthresh;	/* prefetch threshold for rx */
	uint8		txchanswitch;	/* channel switch for rx */
	pktpool_t	*pktpool;	/* pktpool */
	uint		dma_avoidance_cnt;

	uint32		d64_xs0_cd_mask; /* tx current descriptor pointer mask */
	uint32		d64_xs1_ad_mask; /* tx active descriptor mask */
	uint32		d64_rs0_cd_mask; /* rx current descriptor pointer mask */
	uint16		rs0cd;		/* cached value of rcvstatus0 currdescr */
	uint16		xs0cd;		/* cached value of xmtstatus0 currdescr */
	uint16		xs0cd_snapshot;	/* snapshot of xmtstatus0 currdescr */
	uint8		burstsize_ctrl;
	uint8		sep_rxhdr;	/* D11_SPLIT_RX_FD: Separate rx header descriptor */
	uint		rxavail;	/* rxavail descriptor */
	uint8		split_fifo;	/* to pass down fifo inof to hnddma module */
	uint8		bcmrx_pcn_fifo;	/* to pass classified fifo info to hnddma module */

	/* bools combined into as few bytes as possible */
	bool		addrext:1;	/* this dma engine supports DmaExtendedAddrChanges */
	bool		indirect_dma:1;	/* set if Indirect DMA interface is supported */
	bool		aligndesc_4k:1;	/* descriptor base need to be aligned or not */
	bool		rx_waitcomplete_enabled:1; /* DMA with D64_RC_WC enabled */
	bool		phyrxs_dma_flags_war:1; /* 6717-SW WAR to fix d11rxhdr_t::dma_flags */
	bool		sts_xfer_phyrxs:1;	/** PhyRx Status xfer is enabled for this FIFO */
	bool		rxfill_suspended:1;
	bool		dmapad_required:1;
	bool		d11rx_war:1;
#ifdef WL_EAP_REKEY_WAR
	bool		dis_msgs:1;	/* suppress debug print */
#endif /* WL_EAP_REKEY_WAR */
#ifdef BCM_HWALITE
	bool		is_aqm:1;
#endif /* BCM_HWALITE */
#ifdef BCM_PCAP
	bool		pcap_en:1;
#endif
	uint8		rxwaitforcomplt;
	char		name[MAXNAMEL];	/* callers name for diag msgs */

	struct dma_info	*linked_di;	/* dma link during fifo split  */
	union {
		dmaaddr_t	phyrxsts_dmaaddr;	/**< PhyRx Status ring memory */
#if defined(STS_XFER_PHYRXS_MBX)
		haddr64_t	phyrxsts_haddr64;	/**< PhyRx Status ring host mem (HNDMBX) */
#endif /* STS_XFER_PHYRXS_MBX */
	};
	/* PLEASE READ THE BELOW before adding new fields
	 * before adding new small fields at the end, please look at the above struct and see
	 * if you could Squeeze the field in the known alignment holes...point is dma_attach is not
	 * an aatachfn, because there are cases where this gets called run time for some compiles
	*/
	uint32		d64_rs1_ad_mask; /* rx active descriptor pointer mask */
#ifdef BCM_SECURE_DMA
	struct sec_cma_info sec_cma_info_rx;
	struct sec_cma_info sec_cma_info_tx;
#endif /* BCM_SECURE_DMA */

	dma_common_t    *dmacommon; /* dma_common_t handle */
	uint32          trans_coherent; /* coherent per transaction */
	void		*ctx;		/* context for dma users */
	setup_context_t	fn;		/* callback function for dma users */
	dma_pktlist_t dma_pkt_list;
	dma_pktlist_t dma_rx_pkt_list;
	uint32		bulk_descr_tx_start_txout;	/* txout before bulk dma start;
							 * -1=no bulk dma in progress
							 */
	uint32		dd64_size;	/* size of dma64 descriptor */
#ifdef BCM_HWALITE
	void *mlc_dev;
	struct dma_mlo_aux_info aux_info[2]; /* Max 2 Aux AP supported */
	uint8 fifo;
#endif
	union {
		uint8 desc_wrap;
		struct {
			uint8 reserved:4;
			uint8 desc_wrap_ind:4;	/* AQM descriptor wraparound indication for PUQ */
		};
	};
} dma_info_t;

#ifdef BCM_PCAP
#define DMA_PCAP_EN(di)	((di)->pcap_en)
#else
#define	DMA_PCAP_EN(di)	0
#endif /* BCM_PCAP */

#ifdef BCM_DMA_INDIRECT
#define DMA_INDIRECT(di) ((di)->indirect_dma)
#else
#define DMA_INDIRECT(di) FALSE
#endif

#define DMA_AQM_DESC(di)        ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)
#define DMA_AQM_DESC_SHORT(di)  ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_8B_FLAG)

#if defined(BCMHWA) && defined(HWA_PKT_MACRO)
#define DMA_HWA_RX(di)          ((di)->hnddma.dmactrlflags & DMA_CTRL_HWA_RX)
#define DMA_HWA_TX(di)          ((di)->hnddma.dmactrlflags & DMA_CTRL_HWA_TX)
#ifdef BCMHME
#define DMA_HWA_HME_RX(di)      ((di)->hnddma.dmactrlflags & DMA_CTRL_HWA_HME_RX)
#define DMA_HWA_HME_TX(di)      ((di)->hnddma.dmactrlflags & DMA_CTRL_HWA_HME_TX)
#else
#define DMA_HWA_HME_RX(di)      FALSE
#define DMA_HWA_HME_TX(di)      FALSE
#endif /* BCMHME */
#else
#define DMA_HWA_RX(di)          FALSE
#define DMA_HWA_TX(di)          FALSE
#define DMA_HWA_HME_RX(di)      FALSE
#define DMA_HWA_HME_TX(di)      FALSE
#endif /* BCMHWA && HWA_PKT_MACRO */

#ifdef WL_PUQ
#define DMA_PUQ_QREMAP(di)      ((di)->hnddma.dmactrlflags & DMA_CTRL_PUQ_QREMAP)
#define DMA_PUQ_WAR(di)         ((di)->hnddma.dmactrlflags & DMA_CTRL_PUQ_WAR)
#else
#define DMA_PUQ_QREMAP(di)      FALSE
#define DMA_PUQ_WAR(di)         FALSE
#endif

/* DMA Scatter-gather list is supported. Note this is limited to TX direction only */
#ifdef BCMDMASGLISTOSL
#define DMASGLIST_ENAB TRUE
#else
#define DMASGLIST_ENAB FALSE
#endif /* BCMDMASGLISTOSL */

/* descriptor bumping macros */
#define	XXD(x, n)	((x) & ((n) - 1))	/* faster than %, but n must be power of 2 */
#define	TXD(x)		XXD((x), di->ntxd)
#define	RXD(x)		XXD((x), di->nrxd)
#define	RXDPERHANDLE(x, handler)		(XXD((x), handler->nrxd))

#define	NEXTTXD(i)	TXD((i) + 1)
#define	PREVTXD(i)	TXD((i) - 1)
#define	NEXTRXD(i)	RXD((i) + 1)
#define	PREVRXD(i)	RXD((i) - 1)
#define	PREVNRXD(i, n)	((i < n) ? (di->nrxd - (n - i)) : (i - n))

#define	NTXDACTIVE(h, t)	TXD((t) - (h))
/**
 * @NRXDACTIVE  From the firmware perspective: the number of yet empty descriptors that are
 *              available for hardware to be filled. Note that since the last time firmware
 *              updated its status, it could well be that dma hardware already filled (part of)
 *              these descriptors. In that case, NRXDACTIVE() would return a stale status, unless
 *              the firmware re-reads hardware registers and updated its di->rxin and di->rxout
 *              pointers accordingly.
 */
#define	NRXDACTIVE(h, t)	RXD((t) - (h))
#define	NRXDACTIVEPERHANDLE(h, t, handler)		(RXDPERHANDLE(((t) - (h)), handler))

/* macros to convert between byte offsets and indexes */
#define	B2I(bytes, size)	((uint16)((bytes) / (size)))
#define	I2B(index, size)	((index) * (size))

#define	PCI32ADDR_HIGH		0xc0000000	/* address[31:30] */
#define	PCI32ADDR_HIGH_SHIFT	30		/* address[31:30] */

#define	PCI64ADDR_HIGH		0x80000000	/* address[63] */
#define	PCI64ADDR_HIGH_SHIFT	31		/* address[63] */

void dma64_dd_upd(dma_info_t *di, dma64dd_t *ddring, dmaaddr_t pa, uint outidx, uint32 *flags,
	uint32 bufcount);
void dma64_dd_upd_64_from_params(dma_info_t *di, dma64dd_t *ddring, dma64addr_t pa,
	uint outidx, uint32 *flags, uint32 bufcount);

/** Check for odd number of 1's */
STATIC INLINE uint32 parity32(uint32 data)
{
	data ^= data >> 16;
	data ^= data >> 8;
	data ^= data >> 4;
	data ^= data >> 2;
	data ^= data >> 1;

	return (data & 1);
}

#define DMA64_DD_PARITY(dd)  parity32((dd)->addrlow ^ (dd)->addrhigh ^ (dd)->ctrl1 ^ (dd)->ctrl2)
#define DMA64_DD_SHORT_PARITY(dd)  parity32((dd)->ctrl1 ^ (dd)->ctrl2)

#define DMA64_LD_UPD(di, new_ld, tx) \
({ \
	dma64regs_t *_d64regs = (tx) ? (di)->d64txregs : (di)->d64rxregs; \
	uint32 _ptrbase = (tx) ? (di)->xmtptrbase : (di)->rcvptrbase; \
	if ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) { \
		if ((tx) && DMA_PUQ_QREMAP((di))) \
			W_REG((di)->osh, &_d64regs->ptr, (uint32)(((B2I(_ptrbase, \
				(di)->dd64_size) + (new_ld)) & (di)->d64_xs0_cd_mask) | \
				((di)->desc_wrap_ind << 12))); \
		else \
			W_REG((di)->osh, &_d64regs->ptr, (uint32)(B2I(_ptrbase, \
				(di)->dd64_size) + (new_ld))); \
	} \
	else \
		W_REG((di)->osh, &_d64regs->ptr, \
		    (uint32)(_ptrbase + I2B((new_ld), (di)->dd64_size))); \
})
#define DMA64_TX_LD_UPD(di, new_ld) DMA64_LD_UPD((di), (new_ld), 1)
#define DMA64_RX_LD_UPD(di, new_ld) DMA64_LD_UPD((di), (new_ld), 0)

#define DMA64_LD(di, tx) \
({ \
	dma64regs_t *_d64regs = (tx) ? (di)->d64txregs : (di)->d64rxregs; \
	uint32 _desc_mask = (tx) ? (di)->d64_xs0_cd_mask : (di)->d64_rs0_cd_mask; \
	uint32 _ptrbase = (tx) ? (di)->xmtptrbase : (di)->rcvptrbase; \
	uint16 _ld; \
	if ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) { \
		_ld = (uint16)((R_REG((di)->osh, &_d64regs->ptr) & _desc_mask) \
			- (B2I(_ptrbase, (di)->dd64_size) & _desc_mask)); \
	} else { \
		_ld = (uint16)(B2I(((R_REG((di)->osh, &_d64regs->ptr) & _desc_mask) \
			- _ptrbase) & _desc_mask, (di)->dd64_size)); \
	} \
	_ld; \
})
#define DMA64_TX_LD(di) DMA64_LD((di), 1)
#define DMA64_RX_LD(di) DMA64_LD((di), 0)

#define DMA64_CD(di, tx) \
({ \
	dma64regs_t *_d64regs = (tx) ? (di)->d64txregs : (di)->d64rxregs; \
	uint32 _desc_mask = (tx) ? (di)->d64_xs0_cd_mask : (di)->d64_rs0_cd_mask; \
	uint32 _ptrbase = (tx) ? (di)->xmtptrbase : (di)->rcvptrbase; \
	uint16 _cd; \
	if ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) { \
		_cd = (uint16)((R_REG((di)->osh, &_d64regs->status0) & _desc_mask) \
			- (B2I(_ptrbase, (di)->dd64_size) & _desc_mask)); \
	} else { \
		_cd = (uint16)(B2I(((R_REG((di)->osh, &_d64regs->status0) & _desc_mask) \
			- _ptrbase) & _desc_mask, (di)->dd64_size)); \
	} \
	_cd; \
})
#define DMA64_TX_CD(di) DMA64_CD((di), 1)
#define DMA64_RX_CD(di) DMA64_CD((di), 0)

#define DMA64_AD(di, tx) \
({ \
	dma64regs_t *_d64regs = (tx) ? (di)->d64txregs : (di)->d64rxregs; \
	uint32 _desc_mask = (tx) ? (di)->d64_xs1_ad_mask : (di)->d64_rs1_ad_mask; \
	uint32 _ptrbase = (tx) ? (di)->xmtptrbase : (di)->rcvptrbase; \
	uint16 _ad; \
	if ((di)->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) { \
		_ad = (uint16)((R_REG((di)->osh, &_d64regs->status1) & _desc_mask) \
			- (B2I(_ptrbase, (di)->dd64_size) & _desc_mask)); \
	} else { \
		_ad = (uint16)(B2I(((R_REG((di)->osh, &_d64regs->status1) & _desc_mask) \
			- _ptrbase) & _desc_mask, (di)->dd64_size)); \
	} \
	_ad; \
})
#define DMA64_TX_AD(di) DMA64_AD((di), 1)
#define DMA64_RX_AD(di) DMA64_AD((di), 0)

/**
 * Helper routine to do commit only operation on descriptors
 */

/* Local inline copy, the macro causes inlining to happen in the dongle case */
#ifdef BCM_HWALITE
#define dma64_txcommit_local(di, aqm_fifo) \
({ \
	if (((di)->mlc_dev) && mlc_hwalite_is_enable((di)->mlc_dev)) \
		mlc_hwalite_txfifo_ld_upd((di)->mlc_dev, (di)->fifo, \
			(uint16)(((B2I((di)->xmtptrbase, (di)->dd64_size) + (di)->txout) & \
			(di)->d64_xs0_cd_mask) | ((di)->desc_wrap_ind << 12)), (aqm_fifo)); \
	else \
		DMA64_TX_LD_UPD((di), (di)->txout); \
})
#else
#define dma64_txcommit_local(di, aqm_fifo) DMA64_TX_LD_UPD((di), (di)->txout)
#endif /* BCM_HWALITE */
#endif /* _HNDDMA_PRIV_H_ */
