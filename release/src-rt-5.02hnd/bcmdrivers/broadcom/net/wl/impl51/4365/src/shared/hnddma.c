/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: hnddma.c 633585 2016-04-23 06:49:05Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <siutils.h>

#include <sbhnddma.h>
#include <hnddma.h>

/* debug/trace */
#ifdef BCMDBG
#define	DMA_ERROR(args) if (di && (!(*di->msg_level & 1))); else printf args
#define	DMA_TRACE(args) if (di && (!(*di->msg_level & 2))); else printf args
//#elif defined(BCMDBG_ERR)
//#define	DMA_ERROR(args) if (di && (!(*di->msg_level & 1))); else printf args
//#define	DMA_TRACE(args)
#else
#define	DMA_ERROR(args)
#define	DMA_TRACE(args)
#endif /* BCMDBG */


#define	DMA_NONE(args)


#define d32txregs	dregs.d32_u.txregs_32
#define d32rxregs	dregs.d32_u.rxregs_32
#define txd32		dregs.d32_u.txd_32
#define rxd32		dregs.d32_u.rxd_32

#define d64txregs	dregs.d64_u.txregs_64
#define d64rxregs	dregs.d64_u.rxregs_64
#define txd64		dregs.d64_u.txd_64
#define rxd64		dregs.d64_u.rxd_64

/* default dma message level (if input msg_level pointer is null in dma_attach()) */
static uint dma_msg_level =
#ifdef BCMDBG
	1;
#else
	0;
#endif /* BCMDBG */

#define MAXNAMEL	8		/* 8 char names */

#define	DI_INFO(dmah)	((dma_info_t *)(uintptr)dmah)

#define DESCR_DEADBEEF
#if defined(__mips__) || defined(BCM47XX_CA9)
#undef DESCR_DEADBEEF
#endif

/* SplitRX Feature for D11 <TCM,DDR> split pkt reception in Full Dongle Mode */
#if (!defined(__mips__) && !defined(BCM47XX_CA9))
#define D11_SPLIT_RX_FD
#endif

#if ((!defined(__mips__) && !defined(BCM47XX_CA9)) || defined(__NetBSD__))
/* Enable/Disable support for Scatter Gather List in RX direction */
#define SGLIST_RX_SUPPORT
#endif

#if defined(BCM47XX_CA9) && !defined(__NetBSD__)
/* Enable/Disable Bulk Descriptor Flushing optimization */
#define BULK_DESCR_FLUSH
#endif

#if defined(BULK_DESCR_FLUSH)
/* Flush length, given number of descriptors */
#define DMA64_SHIFT (4)  /* multiply by sizeof a dma descriptor  (16 Bytes) */
#define DMA64_FLUSH_LEN(num_desc)   ((num_desc) << DMA64_SHIFT)

#define dma64_rxd64(di, ix) (void *)((uint)(&((dma64dd_t *)((di)->rxd64))[ix]))
#define dma64_txd64(di, ix) (void *)((uint)(&((dma64dd_t *)((di)->txd64))[ix]))
#endif /* BULK_DESCR_FLUSH */


/* Structure for global DMA states and for storing information needed by all
 * DMA channels.
 *
 */
#ifdef BCM_DMA_CT
typedef struct aqm_fifo_stats {
	uint16 next_ld;
} aqm_fifo_stats_t;

typedef struct aqm_stats
{
	int fifo;
	aqm_fifo_stats_t aqm_fifo_stats[32];

} aqm_stats_t;
#endif

struct dma_common {
	void    *osh;           /* os handle */

	uint8 last_qsel;        /* last queue index that was configured in IndQSel */

	/* INDIRECT DMA channel registers for rev>=64 */
	volatile uint32 *indqsel;
	volatile uint32 *suspreq;
	volatile uint32 *flushreq;
	volatile uint32 *chnflushstatus;
#ifdef BCM_DMA_CT
	aqm_stats_t *aqm_stats;
#endif
};

/** dma engine software state */
typedef struct dma_info {
	struct hnddma_pub hnddma;	/* exported structure, don't use hnddma_t,
					 * which could be const
					 */
	uint		*msg_level;	/* message level pointer */
	char		name[MAXNAMEL];	/* callers name for diag msgs */

	void		*osh;		/* os handle */
	si_t		*sih;		/* sb handle */

	bool		dma64;		/* this dma engine is operating in 64-bit mode */
	bool		addrext;	/* this dma engine supports DmaExtendedAddrChanges */

#ifdef BCM_DMA_INDIRECT
	bool            indirect_dma;   /* set if Indirect DMA interface is supported */
	uint8           q_index;        /* index of the queue */
#endif
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
			dma64dd_t	*txd_64;	/* pointer to dma64 tx descriptor ring */
			dma64dd_t	*rxd_64;	/* pointer to dma64 rx descriptor ring */
		} d64_u;
	} dregs;

	uint16		dmadesc_align;	/* alignment requirement for dma descriptors */

	uint16		ntxd;		/* # tx descriptors tunable */
	uint16		txin;		/* index of next descriptor to reclaim */
	uint16		txout;		/* index of next descriptor to post */
	void		**txp;		/* pointer to parallel array of pointers to packets */
	osldma_t 	*tx_dmah;	/* DMA TX descriptor ring handle */
	hnddma_seg_map_t	*txp_dmah;	/* DMA MAP meta-data handle */
	dmaaddr_t	txdpa;		/* Aligned physical address of descriptor ring */
	dmaaddr_t	txdpaorig;	/* Original physical address of descriptor ring */
	uint16		txdalign;	/* #bytes added to alloc'd mem to align txd */
	uint32		txdalloc;	/* #bytes allocated for the ring */
	uint32		xmtptrbase;	/* When using unaligned descriptors, the ptr register
					 * is not just an index, it needs all 13 bits to be
					 * an offset from the addr register.
					 */

	uint16		nrxd;		/* # rx descriptors tunable */
	uint16		rxin;		/* index of next descriptor to reclaim */
	uint16		rxout;		/* index of next descriptor to post */
	void		**rxp;		/* pointer to parallel array of pointers to packets */
	osldma_t 	*rx_dmah;	/* DMA RX descriptor ring handle */
	hnddma_seg_map_t	*rxp_dmah;	/* DMA MAP meta-data handle */
	dmaaddr_t	rxdpa;		/* Aligned physical address of descriptor ring */
	dmaaddr_t	rxdpaorig;	/* Original physical address of descriptor ring */
	uint16		rxdalign;	/* #bytes added to alloc'd mem to align rxd */
	uint32		rxdalloc;	/* #bytes allocated for the ring */
	uint32		rcvptrbase;	/* Base for ptr reg when using unaligned descriptors */

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
	uint		ddoffsethigh;	/*   high 32 bits */
	uint		dataoffsetlow;	/* add to get dma address of data buffer, low 32 bits */
	uint		dataoffsethigh;	/*   high 32 bits */
	bool		aligndesc_4k;	/* descriptor base need to be aligned or not */
	uint8		rxburstlen;	/* burstlen field for rx (for cores supporting burstlen) */
	uint8		txburstlen;	/* burstlen field for tx (for cores supporting burstlen) */
	uint8		txmultioutstdrd; 	/* tx multiple outstanding reads */
	uint8 		txprefetchctl;	/* prefetch control for tx */
	uint8 		txprefetchthresh;	/* prefetch threshold for tx */
	uint8 		rxprefetchctl;	/* prefetch control for rx */
	uint8 		rxprefetchthresh;	/* prefetch threshold for rx */
	pktpool_t	*pktpool;	/* pktpool */
	uint		dma_avoidance_cnt;

	uint32 		d64_xs0_cd_mask; /* tx current descriptor pointer mask */
	uint32 		d64_xs1_ad_mask; /* tx active descriptor mask */
	uint32 		d64_rs0_cd_mask; /* rx current descriptor pointer mask */
	uint16		rs0cd;		/* cached value of rcvstatus0 currdescr */
	uint16		xs0cd;		/* cached value of xmtstatus0 currdescr */
	uint16		xs0cd_snapshot;		/* snapshot of xmtstatus0 currdescr */
#ifdef PCIE_PHANTOM_DEV
	uint		*blwar_nsegs;
	uint		*blwar_size;
	uint8		blwar_d11core;
#endif
	uint8		burstsize_ctrl;
	uint		rxavail;	/* rxavail descriptor */
	uint8		sep_rxhdr;	/* D11_SPLIT_RX_FD: Separate rx header descriptor */
	uint8		split_fifo;	/* to pass down fifo inof to hnddma module */
	struct dma_info	*linked_di;	/* dma link during fifo split  */
	bool		rxfill_suspended;
	/* PLEASE READ THE BELOW before adding new fields
	 * before adding new small fields at the end, please look at the above struct and see
	 * if you could Squeeze the field in the known alignment holes...point is dma_attach is not
	 * an aatachfn, because there are cases where this gets called run time for some compiles
	*/
	bool		dmapad_required;

	dma_common_t    *dmacommon; /* dma_common_t handle */
	uint32		trans_coherent; /* coherent per transaction */
} dma_info_t;

#ifdef BCM_DMA_INDIRECT
#define DMA_INDIRECT(di) ((di)->indirect_dma)
#else
#define DMA_INDIRECT(di) FALSE
#endif


/*
 * If BCMDMA32 is defined, hnddma will support both 32-bit and 64-bit DMA engines.
 * Otherwise it will support only 64-bit.
 *
 * DMA32_ENAB indicates whether hnddma is compiled with support for 32-bit DMA engines.
 * DMA64_ENAB indicates whether hnddma is compiled with support for 64-bit DMA engines.
 *
 * DMA64_MODE indicates whether the current DMA engine is running as 64-bit.
 */
#ifdef BCMDMA32
#define	DMA32_ENAB(di)		1
#define	DMA64_ENAB(di)		1
#define	DMA64_MODE(di)		((di)->dma64)
#else /* !BCMDMA32 */
#define	DMA32_ENAB(di)		0
#define	DMA64_ENAB(di)		1
#define	DMA64_MODE(di)		1
#endif /* !BCMDMA32 */

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

#define	NTXDACTIVE(h, t)	TXD((t) - (h))
#define	NRXDACTIVE(h, t)	RXD((t) - (h))
#define	NRXDACTIVEPERHANDLE(h, t, handler)		(RXDPERHANDLE(((t) - (h)), handler))

/* macros to convert between byte offsets and indexes */
#define	B2I(bytes, type)	((uint16)((bytes) / sizeof(type)))
#define	I2B(index, type)	((index) * sizeof(type))

#define	PCI32ADDR_HIGH		0xc0000000	/* address[31:30] */
#define	PCI32ADDR_HIGH_SHIFT	30		/* address[31:30] */

#define	PCI64ADDR_HIGH		0x80000000	/* address[63] */
#define	PCI64ADDR_HIGH_SHIFT	31		/* address[63] */

#ifdef BCM_DMAPAD
	#ifdef BCMROMBUILD
		#define DMAPADREQUIRED(di)		((di)->dmapad_required)
	#elif defined(BCMSDIODEV_ENABLED)
		#define DMAPADREQUIRED(di)		TRUE
	#else
		#define DMAPADREQUIRED(di)		FALSE
	#endif /* BCMROMBUILD */
#else
	#define DMAPADREQUIRED(di)		FALSE
#endif /* BCM_DMAPAD */

#ifdef BCM_DMA_TRANSCOHERENT
	#ifdef BCMROMBUILD
		#define DMA_TRANSCOHERENT(di)		((di)->trans_coherent)
	#else
		#define DMA_TRANSCOHERENT(di)		1
	#endif /* BCMROMBUILD */
#else
	#define DMA_TRANSCOHERENT(di)			0
#endif /* BCM_DMA_TRANSCOHERENT */

/* Common prototypes */
static bool _dma_isaddrext(dma_info_t *di);
static bool _dma_descriptor_align(dma_info_t *di);
static bool _dma_alloc(dma_info_t *di, uint direction);
static void _dma_detach(dma_info_t *di);
static void _dma_ddtable_init(dma_info_t *di, uint direction, dmaaddr_t pa);
static void _dma_rxinit(dma_info_t *di);
static void *_dma_rx(dma_info_t *di);
static bool _dma_rxfill(dma_info_t *di);
static void _dma_rxreclaim(dma_info_t *di);
static void _dma_rxenable(dma_info_t *di);
static void *_dma_getnextrxp(dma_info_t *di, bool forceall);
static void _dma_rx_param_get(dma_info_t *di, uint16 *rxoffset, uint16 *rxbufsize);

static void _dma_txblock(dma_info_t *di);
static void _dma_txunblock(dma_info_t *di);
static uint _dma_txactive(dma_info_t *di);
static uint _dma_rxactive(dma_info_t *di);
static uint _dma_activerxbuf(dma_info_t *di);
static uint _dma_txpending(dma_info_t *di);
static uint _dma_txcommitted(dma_info_t *di);

static void *_dma_peeknexttxp(dma_info_t *di);
static int _dma_peekntxp(dma_info_t *di, int *len, void *txps[], txd_range_t range);
static void *_dma_peeknextrxp(dma_info_t *di);
static uintptr _dma_getvar(dma_info_t *di, const char *name);
static void _dma_counterreset(dma_info_t *di);
static void _dma_fifoloopbackenable(dma_info_t *di);
static uint _dma_ctrlflags(dma_info_t *di, uint mask, uint flags);
static uint8 dma_align_sizetobits(uint size);
static void *dma_ringalloc(osl_t *osh, uint32 boundary, uint size, uint16 *alignbits, uint* alloced,
	dmaaddr_t *descpa, osldma_t **dmah);
static int _dma_pktpool_set(dma_info_t *di, pktpool_t *pool);
static bool _dma_rxtx_error(dma_info_t *di, bool istx);
static void _dma_burstlen_set(dma_info_t *di, uint8 rxburstlen, uint8 txburstlen);
static uint _dma_avoidancecnt(dma_info_t *di);
static void _dma_param_set(dma_info_t *di, uint16 paramid, uint16 paramval);
static bool _dma_glom_enable(dma_info_t *di, uint32 val);


/* Prototypes for 32-bit routines */
static bool dma32_alloc(dma_info_t *di, uint direction);
static bool dma32_txreset(dma_info_t *di);
static bool dma32_rxreset(dma_info_t *di);
static bool dma32_txsuspendedidle(dma_info_t *di);
static int  dma32_txfast(dma_info_t *di, void *p0, bool commit);
static void *dma32_getnexttxp(dma_info_t *di, txd_range_t range);
static void *dma32_getnextrxp(dma_info_t *di, bool forceall);
static void dma32_txrotate(dma_info_t *di);
static bool dma32_rxidle(dma_info_t *di);
static void dma32_txinit(dma_info_t *di);
static bool dma32_txenabled(dma_info_t *di);
static void dma32_txsuspend(dma_info_t *di);
static void dma32_txresume(dma_info_t *di);
static bool dma32_txsuspended(dma_info_t *di);
#ifdef WL_MULTIQUEUE
static void dma32_txflush(dma_info_t *di);
static void dma32_txflush_clear(dma_info_t *di);
#endif /* WL_MULTIQUEUE */
static void dma32_txreclaim(dma_info_t *di, txd_range_t range);
static bool dma32_txstopped(dma_info_t *di);
static bool dma32_rxstopped(dma_info_t *di);
static bool dma32_rxenabled(dma_info_t *di);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void dma32_dumpring(dma_info_t *di, struct bcmstrbuf *b, dma32dd_t *ring, uint start,
	uint end, uint max_num);
static void dma32_dump(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
static void dma32_dumptx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
static void dma32_dumprx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

static bool _dma32_addrext(osl_t *osh, dma32regs_t *dma32regs);

/* Prototypes for 64-bit routines */
static bool dma64_alloc(dma_info_t *di, uint direction);
static bool dma64_txreset(dma_info_t *di);
static bool dma64_rxreset(dma_info_t *di);
static bool dma64_txsuspendedidle(dma_info_t *di);
static int  dma64_txfast(dma_info_t *di, void *p0, bool commit);
static int  dma64_txunframed(dma_info_t *di, void *p0, uint len, bool commit);
static void *dma64_getpos(dma_info_t *di, bool direction);
static void *dma64_getnexttxp(dma_info_t *di, txd_range_t range);
static void *dma64_getnextrxp(dma_info_t *di, bool forceall);
static INLINE void dma64_dd_upd_64_from_params(dma_info_t *di, dma64dd_t *ddring, dma64addr_t pa,
	uint outidx, uint32 *flags, uint32 bufcount);
static INLINE void dma64_dd_upd_64_from_struct(dma_info_t *di, dma64dd_t *ddring, dma64dd_t *dd,
	uint outidx);
static int  dma64_txfast_lfrag(dma_info_t *di, void *p0, bool commit);
static void dma64_txrotate(dma_info_t *di);

static bool dma64_rxidle(dma_info_t *di);
static void dma64_txinit(dma_info_t *di);
static bool dma64_txenabled(dma_info_t *di);
static void dma64_txsuspend(dma_info_t *di);
static void dma64_txresume(dma_info_t *di);
static bool dma64_txsuspended(dma_info_t *di);
#ifdef WL_MULTIQUEUE
static void dma64_txflush(dma_info_t *di);
static void dma64_txflush_clear(dma_info_t *di);
#endif /* WL_MULTIQUEUE */
static void dma64_txreclaim(dma_info_t *di, txd_range_t range);
static bool dma64_txstopped(dma_info_t *di);
static bool dma64_rxstopped(dma_info_t *di);
static bool dma64_rxenabled(dma_info_t *di);
static bool _dma64_addrext(osl_t *osh, dma64regs_t *dma64regs);

STATIC INLINE uint32 parity32(uint32 data);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void dma64_dumpring(dma_info_t *di, struct bcmstrbuf *b, dma64dd_t *ring, uint start,
	uint end, uint max_num);
static void dma64_dump(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
static void dma64_dumptx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
static void dma64_dumprx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
#if defined(D11_SPLIT_RX_FD)
static bool dma_splitrxfill(dma_info_t *di);
#endif


const di_fcn_t dma64proc = {
	(di_detach_t)_dma_detach,
	(di_txinit_t)dma64_txinit,
	(di_txreset_t)dma64_txreset,
	(di_txenabled_t)dma64_txenabled,
	(di_txsuspend_t)dma64_txsuspend,
	(di_txresume_t)dma64_txresume,
	(di_txsuspended_t)dma64_txsuspended,
	(di_txsuspendedidle_t)dma64_txsuspendedidle,
#ifdef WL_MULTIQUEUE
	(di_txflush_t)dma64_txflush,
	(di_txflush_clear_t)dma64_txflush_clear,
#endif /* WL_MULTIQUEUE */
	(di_txfast_t)dma64_txfast,
	(di_txunframed_t)dma64_txunframed,
	(di_getpos_t)dma64_getpos,
	(di_txstopped_t)dma64_txstopped,
	(di_txreclaim_t)dma64_txreclaim,
	(di_getnexttxp_t)dma64_getnexttxp,
	(di_peeknexttxp_t)_dma_peeknexttxp,
	(di_peekntxp_t)_dma_peekntxp,
	(di_txblock_t)_dma_txblock,
	(di_txunblock_t)_dma_txunblock,
	(di_txactive_t)_dma_txactive,
	(di_txrotate_t)dma64_txrotate,

	(di_rxinit_t)_dma_rxinit,
	(di_rxreset_t)dma64_rxreset,
	(di_rxidle_t)dma64_rxidle,
	(di_rxstopped_t)dma64_rxstopped,
	(di_rxenable_t)_dma_rxenable,
	(di_rxenabled_t)dma64_rxenabled,
	(di_rx_t)_dma_rx,
	(di_rxfill_t)_dma_rxfill,
	(di_rxreclaim_t)_dma_rxreclaim,
	(di_getnextrxp_t)_dma_getnextrxp,
	(di_peeknextrxp_t)_dma_peeknextrxp,
	(di_rxparam_get_t)_dma_rx_param_get,

	(di_fifoloopbackenable_t)_dma_fifoloopbackenable,
	(di_getvar_t)_dma_getvar,
	(di_counterreset_t)_dma_counterreset,
	(di_ctrlflags_t)_dma_ctrlflags,

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	(di_dump_t)dma64_dump,
	(di_dumptx_t)dma64_dumptx,
	(di_dumprx_t)dma64_dumprx,
#else
	NULL,
	NULL,
	NULL,
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
	(di_rxactive_t)_dma_rxactive,
	(di_txpending_t)_dma_txpending,
	(di_txcommitted_t)_dma_txcommitted,
	(di_pktpool_set_t)_dma_pktpool_set,
	(di_rxtxerror_t)_dma_rxtx_error,
	(di_burstlen_set_t)_dma_burstlen_set,
	(di_avoidancecnt_t)_dma_avoidancecnt,
	(di_param_set_t)_dma_param_set,
	(dma_glom_enable_t)_dma_glom_enable,
	(dma_active_rxbuf_t)_dma_activerxbuf,
	40
};

static const di_fcn_t dma32proc = {
	(di_detach_t)_dma_detach,
	(di_txinit_t)dma32_txinit,
	(di_txreset_t)dma32_txreset,
	(di_txenabled_t)dma32_txenabled,
	(di_txsuspend_t)dma32_txsuspend,
	(di_txresume_t)dma32_txresume,
	(di_txsuspended_t)dma32_txsuspended,
	(di_txsuspendedidle_t)dma32_txsuspendedidle,
#ifdef WL_MULTIQUEUE
	(di_txflush_t)dma32_txflush,
	(di_txflush_clear_t)dma32_txflush_clear,
#endif /* WL_MULTIQUEUE */
	(di_txfast_t)dma32_txfast,
	NULL,
	NULL,
	(di_txstopped_t)dma32_txstopped,
	(di_txreclaim_t)dma32_txreclaim,
	(di_getnexttxp_t)dma32_getnexttxp,
	(di_peeknexttxp_t)_dma_peeknexttxp,
	(di_peekntxp_t)_dma_peekntxp,
	(di_txblock_t)_dma_txblock,
	(di_txunblock_t)_dma_txunblock,
	(di_txactive_t)_dma_txactive,
	(di_txrotate_t)dma32_txrotate,

	(di_rxinit_t)_dma_rxinit,
	(di_rxreset_t)dma32_rxreset,
	(di_rxidle_t)dma32_rxidle,
	(di_rxstopped_t)dma32_rxstopped,
	(di_rxenable_t)_dma_rxenable,
	(di_rxenabled_t)dma32_rxenabled,
	(di_rx_t)_dma_rx,
	(di_rxfill_t)_dma_rxfill,
	(di_rxreclaim_t)_dma_rxreclaim,
	(di_getnextrxp_t)_dma_getnextrxp,
	(di_peeknextrxp_t)_dma_peeknextrxp,
	(di_rxparam_get_t)_dma_rx_param_get,

	(di_fifoloopbackenable_t)_dma_fifoloopbackenable,
	(di_getvar_t)_dma_getvar,
	(di_counterreset_t)_dma_counterreset,
	(di_ctrlflags_t)_dma_ctrlflags,

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	(di_dump_t)dma32_dump,
	(di_dumptx_t)dma32_dumptx,
	(di_dumprx_t)dma32_dumprx,
#else
	NULL,
	NULL,
	NULL,
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
	(di_rxactive_t)_dma_rxactive,
	(di_txpending_t)_dma_txpending,
	(di_txcommitted_t)_dma_txcommitted,
	(di_pktpool_set_t)_dma_pktpool_set,
	(di_rxtxerror_t)_dma_rxtx_error,
	(di_burstlen_set_t)_dma_burstlen_set,
	(di_avoidancecnt_t)_dma_avoidancecnt,
	(di_param_set_t)_dma_param_set,
	NULL,
	NULL,
	40
};

/* For SDIO and PCIE, dma_attach() (and some of the functions that it calls) are only invoked in
 * attach context; therefore, they can be reclaimed as a memory optimization. For USB, dma_attach()
 * is called in non-attach context and therefore needs to be a persistent function.
 */
#if !defined(BCM_DMA_CT) && (defined(BCMSDIODEV_ENABLED) || \
	defined(BCMPCIEDEV_ENABLED))
	#define HNDDMA_RECLAIM_DMA_ATTACH
#endif

#if defined(HNDDMA_RECLAIM_DMA_ATTACH)
	#define BCMATTACHFN_DMA_ATTACH(_fn)	BCMATTACHFN(_fn)
#else
	#define BCMATTACHFN_DMA_ATTACH(_fn)	(_fn)
#endif


/*
 * This function needs to be called during initialization before calling dma_attach_ext.
 */
dma_common_t *
BCMATTACHFN_DMA_ATTACH(dma_common_attach)(osl_t *osh, volatile uint32 *indqsel,
	volatile uint32 *suspreq, volatile uint32 *flushreq)
{
	dma_common_t *dmac;

	/* allocate private info structure */
	if ((dmac = MALLOCZ(osh, sizeof(*dmac))) == NULL) {
		DMA_NONE(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		return (NULL);
	}

	dmac->osh = osh;

	dmac->indqsel = indqsel;
	dmac->suspreq = suspreq;
	dmac->flushreq = flushreq;

#ifdef BCM_DMA_CT
	/* allocate private info structure */
	if ((dmac->aqm_stats = MALLOCZ(osh, sizeof(aqm_stats_t))) == NULL) {
		DMA_NONE(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		MFREE(dmac->osh, (void *)dmac, sizeof(*dmac));
		return (NULL);
	}
#endif
	return dmac;
}

void
dma_common_detach(dma_common_t *dmacommon)
{
#ifdef BCM_DMA_CT
	if (dmacommon->aqm_stats) {
		MFREE(dmacommon->osh, (void *)dmacommon->aqm_stats, sizeof(aqm_stats_t));
	}
#endif
	/* free our private info structure */
	MFREE(dmacommon->osh, (void *)dmacommon, sizeof(*dmacommon));
}

#ifdef BCM_DMA_INDIRECT
/* Function to configure the q_index into indqsel register
 * to indicate to the DMA engine which queue is being selected
 * for updates or to read back status.
 * ALso updates global dma common state
 * Caller should verify if the di is configured for indirect access.
 * If force is TRUE, the configuration will be done even though
 * global DMA state indicates it is the same as last_qsel
 */
void
dma_set_indqsel(hnddma_t *di, bool override)
{
	dma_info_t *dmainfo = DI_INFO(di);
	dma_common_t *dmacommon = dmainfo->dmacommon;
	ASSERT(dmainfo->indirect_dma == TRUE);

	if ((dmacommon->last_qsel != dmainfo->q_index) || override) {
		/* configure the indqsel register */
		if (si_corerev(dmainfo->sih) == 64) {
			W_REG(dmainfo->osh, dmacommon->indqsel,
				(dmainfo->q_index | DMA_INDQSEL_IA));
		} else {
			W_REG(dmainfo->osh, dmacommon->indqsel, dmainfo->q_index);
		}
		/* also update the global state dmac */
		dmacommon->last_qsel = dmainfo->q_index;
	}
	return;
}

#endif /* BCM_DMA_INDIRECT */

/* This is new dma_attach API to provide the option of using an Indirect DMA interface */
hnddma_t *
BCMATTACHFN_DMA_ATTACH(dma_attach_ext)(dma_common_t *dmac, osl_t *osh, const char *name,
	si_t *sih, volatile void *dmaregstx, volatile void *dmaregsrx, uint32 flags,
	uint8 qnum, uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost,
	uint rxoffset, uint *msg_level)
{
	dma_info_t *di;
	uint size;
	uint32 mask, reg_val;

	/* allocate private info structure */
	if ((di = MALLOC(osh, sizeof (dma_info_t))) == NULL) {
		DMA_ERROR(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		return (NULL);
	}

	bzero(di, sizeof(dma_info_t));

#ifdef BCM_DMA_INDIRECT
	if ((flags & BCM_DMA_IND_INTF_FLAG) && (dmac == NULL)) {
		DMA_ERROR(("%s: Inconsistent flags and dmac params \n", __FUNCTION__));
		ASSERT(0);
		goto fail;
	}
#endif /* ifdef BCM_DMA_INDIRECT */

	/* save dma_common */
	di->dmacommon = dmac;

#ifdef BCM_DMA_INDIRECT
	/* record indirect DMA interface  params */
	di->indirect_dma = (flags & BCM_DMA_IND_INTF_FLAG) ? TRUE : FALSE;
	di->q_index = qnum;

#endif /* BCM_DMA_INDIRECT */

	di->msg_level = msg_level ? msg_level : &dma_msg_level;

	/* old chips w/o sb is no longer supported */
	ASSERT(sih != NULL);

	if (DMA64_ENAB(di))
		di->dma64 = ((si_core_sflags(sih, 0, 0) & SISF_DMA64) == SISF_DMA64);
	else
		di->dma64 = 0;

	/* check arguments */
	ASSERT(ISPOWEROF2(ntxd));
	ASSERT(ISPOWEROF2(nrxd));

	if (nrxd == 0)
		ASSERT(dmaregsrx == NULL);
	if (ntxd == 0)
		ASSERT(dmaregstx == NULL);

	/* init dma reg pointer */
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		di->d64txregs = (dma64regs_t *)dmaregstx;
		di->d64rxregs = (dma64regs_t *)dmaregsrx;
		di->hnddma.di_fn = (const di_fcn_t *)&dma64proc;
	} else if (DMA32_ENAB(di)) {
		ASSERT(ntxd <= D32MAXDD);
		ASSERT(nrxd <= D32MAXDD);
		di->d32txregs = (dma32regs_t *)dmaregstx;
		di->d32rxregs = (dma32regs_t *)dmaregsrx;
		di->hnddma.di_fn = (const di_fcn_t *)&dma32proc;
	} else {
		DMA_ERROR(("%s: driver doesn't support 32-bit DMA\n", __FUNCTION__));
		ASSERT(0);
		goto fail;
	}

	/* Default flags (which can be changed by the driver calling dma_ctrlflags
	 * before enable): For backwards compatibility both Rx Overflow Continue
	 * and Parity are DISABLED.
	 * supports it.
	 */
	di->hnddma.di_fn->ctrlflags(&di->hnddma, DMA_CTRL_ROC | DMA_CTRL_PEN, 0);

	/* check flags for descriptor only DMA */
	if (flags & BCM_DMA_DESC_ONLY_FLAG) {
		di->hnddma.dmactrlflags |= DMA_CTRL_DESC_ONLY_FLAG;
	}

	/* Modern-MACs (d11 corerev 64 and higher) supporting higher MOR need CS set. */
	if (flags & BCM_DMA_CHAN_SWITCH_EN) {
		di->hnddma.dmactrlflags |= DMA_CTRL_CS;
	}

	/* check flags for descriptor only DMA has CD WAR */
	if (flags & BCM_DMA_DESC_CD_WAR_FLAG) {
		di->hnddma.dmactrlflags |= DMA_CTRL_DESC_CD_WAR;
	}

	DMA_TRACE(("%s: %s: %s osh %p flags 0x%x ntxd %d nrxd %d rxbufsize %d "
		"rxextheadroom %d nrxpost %d rxoffset %d dmaregstx %p dmaregsrx %p ",
		name, __FUNCTION__, (DMA64_MODE(di) ? "DMA64" : "DMA32"),
		osh, di->hnddma.dmactrlflags, ntxd, nrxd, rxbufsize, rxextheadroom,
		nrxpost, rxoffset, dmaregstx, dmaregsrx));
#ifdef BCM_DMA_INDIRECT
	DMA_TRACE(("%s: %s: indirect DMA %s, q_index %d \n",
		name, __FUNCTION__, (di->indirect_dma ? "TRUE" : "FALSE"), di->q_index));
#endif
	/* make a private copy of our callers name */
	strncpy(di->name, name, MAXNAMEL);
	di->name[MAXNAMEL-1] = '\0';

	di->osh = osh;
	di->sih = sih;

	/* save tunables */
	di->ntxd = (uint16)ntxd;
	di->nrxd = (uint16)nrxd;

	/* the actual dma size doesn't include the extra headroom */
	di->rxextrahdrroom = (rxextheadroom == -1) ? BCMEXTRAHDROOM : rxextheadroom;
	if (rxbufsize > BCMEXTRAHDROOM)
		di->rxbufsize = (uint16)(rxbufsize - di->rxextrahdrroom);
	else
		di->rxbufsize = (uint16)rxbufsize;

	di->nrxpost = (uint16)nrxpost;
	di->rxoffset = (uint8)rxoffset;

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, TRUE);
	}

	/* Get the default values (POR) of the burstlen. This can be overridden by the modules
	 * if this has to be different. Otherwise this value will be used to program the control
	 * register after the reset or during the init.
	 */
	/* For 4345 PCIE rev 5, writing all 1's to control is broken,
	 * you will read all 1's back
	 */

	if (dmaregsrx) {
		if (DMA64_ENAB(di) && DMA64_MODE(di)) {

			/* first disable the dma if not already done */
			reg_val = R_REG(di->osh, &di->d64rxregs->control);
			if (reg_val & 1) {
				reg_val &= ~1;
				W_REG(di->osh, &di->d64rxregs->control, reg_val);
				W_REG(di->osh, &di->d64rxregs->control, reg_val);
			}
			/* detect the dma descriptor address mask,
			 * should be 0x1fff before 4360B0, 0xffff start from 4360B0
			 */
			W_REG(di->osh, &di->d64rxregs->addrlow, 0xffffffff);
			/* For 4345 PCIE rev 5/8, need one more write to make it work */
			if ((si_coreid(di->sih) == PCIE2_CORE_ID) &&
				((si_corerev(di->sih) == 5) || (si_corerev(di->sih) == 8))) {
				W_REG(di->osh, &di->d64rxregs->addrlow, 0xffffffff);
			}
			mask = R_REG(di->osh, &di->d64rxregs->addrlow);

			if (mask & 0xfff)
				mask = R_REG(di->osh, &di->d64rxregs->ptr) | 0xf;
			else
				mask = 0x1fff;

#ifdef BCMM2MDEV_ENABLED
			if (mask == 0xf)
				mask = 0xffff;
#endif

			DMA_TRACE(("%s: dma_rx_mask: %08x\n", di->name, mask));
			di->d64_rs0_cd_mask = mask;

			if (mask == 0x1fff)
				ASSERT(nrxd <= D64MAXDD);
			else
				ASSERT(nrxd <= D64MAXDD_LARGE);

			di->rxburstlen = (R_REG(di->osh,
				&di->d64rxregs->control) & D64_RC_BL_MASK) >> D64_RC_BL_SHIFT;
			di->rxprefetchctl = (R_REG(di->osh,
				&di->d64rxregs->control) & D64_RC_PC_MASK) >> D64_RC_PC_SHIFT;
			di->rxprefetchthresh = (R_REG(di->osh,
				&di->d64rxregs->control) & D64_RC_PT_MASK) >> D64_RC_PT_SHIFT;
		} else if (DMA32_ENAB(di)) {
			di->rxburstlen = (R_REG(di->osh,
				&di->d32rxregs->control) & RC_BL_MASK) >> RC_BL_SHIFT;
			di->rxprefetchctl = (R_REG(di->osh,
				&di->d32rxregs->control) & RC_PC_MASK) >> RC_PC_SHIFT;
			di->rxprefetchthresh = (R_REG(di->osh,
				&di->d32rxregs->control) & RC_PT_MASK) >> RC_PT_SHIFT;
		}
	}
	if (dmaregstx) {
		if (DMA64_ENAB(di) && DMA64_MODE(di)) {

			/* first disable the dma if not already done */
			reg_val = R_REG(di->osh, &di->d64txregs->control);
			if (reg_val & 1) {
				reg_val &= ~1;
				W_REG(di->osh, &di->d64txregs->control, reg_val);
				W_REG(di->osh, &di->d64txregs->control, reg_val);
			}

			/* detect the dma descriptor address mask,
			 * should be 0x1fff before 4360B0, 0xffff start from 4360B0
			 */
			W_REG(di->osh, &di->d64txregs->addrlow, 0xffffffff);
			/* For 4345 PCIE rev 5/8, need one more write to make it work */
			if ((si_coreid(di->sih) == PCIE2_CORE_ID) &&
				((si_corerev(di->sih) == 5) || (si_corerev(di->sih) == 8))) {
				W_REG(di->osh, &di->d64txregs->addrlow, 0xffffffff);
			}
			mask = R_REG(di->osh, &di->d64txregs->addrlow);

			if (mask & 0xfff)
				mask = R_REG(di->osh, &di->d64txregs->ptr) | 0xf;
			else
				mask = 0x1fff;

#ifdef BCMM2MDEV_ENABLED
			if (mask == 0xf)
				mask = 0xffff;
#endif

			DMA_TRACE(("%s: dma_tx_mask: %08x\n", di->name, mask));
			di->d64_xs0_cd_mask = mask;
			di->d64_xs1_ad_mask = mask;

			/* SWMUMIMO-106: WAR for Descriptor only DMA's CD not being updated
			 * correctly by HW in CT mode.
			 * HW JIRA - CRWLDOT11M-1762, Relevant information in RB 39395.
			 * limit the AD and CD mask to 0x0fff
			 */
			if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_CD_WAR) {
				di->d64_xs0_cd_mask = 0x0fff;
				di->d64_xs1_ad_mask = 0x0fff;
			}

			if (mask == 0x1fff)
				ASSERT(ntxd <= D64MAXDD);
			else
				ASSERT(ntxd <= D64MAXDD_LARGE);

			di->txburstlen = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_BL_MASK) >> D64_XC_BL_SHIFT;
			di->txmultioutstdrd = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_MR_MASK) >> D64_XC_MR_SHIFT;
			di->txprefetchctl = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_PC_MASK) >> D64_XC_PC_SHIFT;
			di->txprefetchthresh = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_PT_MASK) >> D64_XC_PT_SHIFT;
		} else if (DMA32_ENAB(di)) {
			di->txburstlen = (R_REG(di->osh,
				&di->d32txregs->control) & XC_BL_MASK) >> XC_BL_SHIFT;
			di->txmultioutstdrd = (R_REG(di->osh,
				&di->d32txregs->control) & XC_MR_MASK) >> XC_MR_SHIFT;
			di->txprefetchctl = (R_REG(di->osh,
				&di->d32txregs->control) & XC_PC_MASK) >> XC_PC_SHIFT;
			di->txprefetchthresh = (R_REG(di->osh,
				&di->d32txregs->control) & XC_PT_MASK) >> XC_PT_SHIFT;
		}
	}

	/*
	 * figure out the DMA physical address offset for dd and data
	 *     PCI/PCIE: they map silicon backplace address to zero based memory, need offset
	 *     Other bus: use zero
	 *     SI_BUS BIGENDIAN kludge: use sdram swapped region for data buffer, not descriptor
	 */
	di->ddoffsetlow = 0;
	di->dataoffsetlow = 0;
	/* for pci bus, add offset */
	if (sih->bustype == PCI_BUS) {
		if ((sih->buscoretype == PCIE_CORE_ID ||
		     sih->buscoretype == PCIE2_CORE_ID) &&
		    DMA64_MODE(di)) {
			/* pcie with DMA64 */
			di->ddoffsetlow = 0;
			di->ddoffsethigh = SI_PCIE_DMA_H32;
		} else {
			/* pci(DMA32/DMA64) or pcie with DMA32 */
			if ((CHIPID(sih->chip) == BCM4322_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM4342_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM43221_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM43231_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM43111_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM43112_CHIP_ID) ||
			    (CHIPID(sih->chip) == BCM43222_CHIP_ID))
				di->ddoffsetlow = SI_PCI_DMA2;
			else
				di->ddoffsetlow = SI_PCI_DMA;

			di->ddoffsethigh = 0;
		}
		di->dataoffsetlow =  di->ddoffsetlow;
		di->dataoffsethigh =  di->ddoffsethigh;
	}

#ifndef DSLCPE
#if defined(__mips__) && defined(IL_BIGENDIAN)
	/* use sdram swapped region for data buffers but not dma descriptors.
	 * this assumes that we are running on a 47xx mips with a swap window.
	 * But __mips__ is too general, there should be one si_ishndmips() checking
	 * for OUR mips
	 */
	di->dataoffsetlow = di->dataoffsetlow + SI_SDRAM_SWAPPED;
#endif /* defined(__mips__) && defined(IL_BIGENDIAN) */
#endif /* DSLCPE */
	/* WAR64450 : DMACtl.Addr ext fields are not supported in SDIOD core. */
	if ((si_coreid(sih) == SDIOD_CORE_ID) && ((si_corerev(sih) > 0) && (si_corerev(sih) <= 2)))
		di->addrext = 0;
	else if ((si_coreid(sih) == I2S_CORE_ID) &&
	         ((si_corerev(sih) == 0) || (si_corerev(sih) == 1)))
		di->addrext = 0;
	else
		di->addrext = _dma_isaddrext(di);

	/* does the descriptors need to be aligned and if yes, on 4K/8K or not */
	di->aligndesc_4k = _dma_descriptor_align(di);
	if (di->aligndesc_4k) {
		if (DMA64_MODE(di)) {
			di->dmadesc_align = D64RINGALIGN_BITS;
			if ((ntxd < D64MAXDD / 2) && (nrxd < D64MAXDD / 2)) {
				/* for smaller dd table, HW relax the alignment requirement */
				di->dmadesc_align = D64RINGALIGN_BITS  - 1;
			}
		} else
			di->dmadesc_align = D32RINGALIGN_BITS;
	} else {
		/* The start address of descriptor table should be algined to cache line size,
		 * or other structure may share a cache line with it, which can lead to memory
		 * overlapping due to cache write-back operation. In the case of MIPS 74k, the
		 * cache line size is 32 bytes.
		 */
#ifdef __mips__
		di->dmadesc_align = 5;	/* 32 byte alignment */
#else
		di->dmadesc_align = 4;	/* 16 byte alignment */

		/* Aqm txd table alignment 4KB, limitation comes from the bug of CD,
		 * see DMA_CTRL_DESC_CD_WAR and it will be fixed in C0
		 */
		if ((di->hnddma.dmactrlflags & (DMA_CTRL_DESC_ONLY_FLAG | DMA_CTRL_DESC_CD_WAR)) ==
			(DMA_CTRL_DESC_ONLY_FLAG | DMA_CTRL_DESC_CD_WAR)) {
			di->dmadesc_align = 12;	/* 4K byte alignment */
		}
#endif
	}

	DMA_NONE(("DMA descriptor align_needed %d, align %d\n",
		di->aligndesc_4k, di->dmadesc_align));

	/* allocate tx packet pointer vector
	 * Descriptor only DMAs do not need packet pointer array
	 */
	if ((!(di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)) && (ntxd)) {
		size = ntxd * sizeof(void *);
		if ((di->txp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of tx memory, malloced %d bytes\n",
				di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->txp, size);
	}
	/* allocate rx packet pointer vector */
	if (nrxd) {
		size = nrxd * sizeof(void *);
		if ((di->rxp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of rx memory, malloced %d bytes\n",
			           di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->rxp, size);
	}

	/* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
	if (ntxd) {
		if (!_dma_alloc(di, DMA_TX)) /* this does not allocate buffers */
			goto fail;
	}

	/* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
	if (nrxd) {
		if (!_dma_alloc(di, DMA_RX)) /* this does not allocate buffers */
			goto fail;
	}

	if ((di->ddoffsetlow != 0) && !di->addrext) {
		if (PHYSADDRLO(di->txdpa) > SI_PCI_DMA_SZ) {
			DMA_ERROR(("%s: %s: txdpa 0x%x: addrext not supported\n",
			           di->name, __FUNCTION__, (uint32)PHYSADDRLO(di->txdpa)));
			goto fail;
		}
		if (PHYSADDRLO(di->rxdpa) > SI_PCI_DMA_SZ) {
			DMA_ERROR(("%s: %s: rxdpa 0x%x: addrext not supported\n",
			           di->name, __FUNCTION__, (uint32)PHYSADDRLO(di->rxdpa)));
			goto fail;
		}
	}

	DMA_TRACE(("ddoffsetlow 0x%x ddoffsethigh 0x%x dataoffsetlow 0x%x dataoffsethigh "
	           "0x%x addrext %d\n", di->ddoffsetlow, di->ddoffsethigh, di->dataoffsetlow,
	           di->dataoffsethigh, di->addrext));

	/* allocate DMA mapping vectors */
	if (DMASGLIST_ENAB) {
		if (ntxd) {
			size = ntxd * sizeof(hnddma_seg_map_t);
			if ((di->txp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->txp_dmah, size);
		}

		if (nrxd) {
			size = nrxd * sizeof(hnddma_seg_map_t);
			if ((di->rxp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->rxp_dmah, size);
		}
	}
#ifdef BCM_DMAPAD
	#if defined(BCMSDIODEV_ENABLED)
		di->dmapad_required = TRUE;
	#else
		di->dmapad_required = FALSE;
	#endif /* BCMSDIODEV_ENABLED */
#else
	di->dmapad_required = FALSE;
#endif /* BCM_DMAPAD */

#ifdef BCM_DMA_TRANSCOHERENT
	di->trans_coherent = 1;
#else
	di->trans_coherent = 0;
#endif /* BCM_DMA_TRANSCOHERENT */

	return ((hnddma_t *)di);

fail:
	_dma_detach(di);
	return (NULL);
}


/* This is the legacy dma_attach() API. The interface is kept as-is for
 * legacy code ("et" driver) that does not need the dma_common_t support
 * interface
 */
hnddma_t *
BCMATTACHFN_DMA_ATTACH(dma_attach)(osl_t *osh, const char *name, si_t *sih,
	volatile void *dmaregstx, volatile void *dmaregsrx,
	uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost,
	uint rxoffset, uint *msg_level)
{
	return (hnddma_t *)(dma_attach_ext(NULL, osh, name, sih, dmaregstx, dmaregsrx,
		0, 0, ntxd, nrxd, rxbufsize, rxextheadroom, nrxpost, rxoffset, msg_level));
}


/**
 * Initializes one tx or rx descriptor with the caller provided arguments, notably a buffer.
 *    @param[in] di       Handle
 *    @param[in] ddring   Pointer to properties of the descriptor ring
 *    @param[in] pa       Physical address of rx or tx buffer that the descriptor should point at
 *    @param[in] outidx   Index of the targetted DMA descriptor within the descriptor ring
 *    @param[in] flags    Value that is written into the descriptors 'ctrl1' field
 *    @param[in] bufcount Buffer count value, written into the descriptors 'ctrl2' field
 */
static INLINE void
dma32_dd_upd(dma_info_t *di, dma32dd_t *ddring, dmaaddr_t pa, uint outidx, uint32 *flags,
	uint32 bufcount)
{
	/* dma32 uses 32-bit control to fit both flags and bufcounter */
	*flags = *flags | (bufcount & CTRL_BC_MASK);

	if ((di->dataoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
		W_SM(&ddring[outidx].addr, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].ctrl, BUS_SWAP32(*flags));
	} else {
		/* address extension */
		uint32 ae;
		ASSERT(di->addrext);
		ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
		PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;

		*flags |= (ae << CTRL_AE_SHIFT);
		W_SM(&ddring[outidx].addr, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].ctrl, BUS_SWAP32(*flags));
	}
}

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

/**
 * Initializes one tx or rx descriptor with the caller provided arguments, notably a buffer.
 * Knows how to handle native 64-bit addressing AND bit64 extension.
 *    @param[in] di       Handle
 *    @param[in] ddring   Pointer to properties of the descriptor ring
 *    @param[in] pa       Physical address of rx or tx buffer that the descriptor should point at
 *    @param[in] outidx   Index of the targetted DMA descriptor within the descriptor ring
 *    @param[in] flags    Value that is written into the descriptors 'ctrl1' field
 *    @param[in] bufcount Buffer count value, written into the descriptors 'ctrl2' field
 */
static INLINE void
dma64_dd_upd(dma_info_t *di, dma64dd_t *ddring, dmaaddr_t pa, uint outidx, uint32 *flags,
	uint32 bufcount)
{
	uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

	/* PCI bus with big(>1G) physical address, use address extension */
#if defined(__mips__) && defined(IL_BIGENDIAN)
	if ((di->dataoffsetlow == SI_SDRAM_SWAPPED) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
#else
	if ((di->dataoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
#endif /* defined(__mips__) && defined(IL_BIGENDIAN) */
		/* This is where 64-bit addr ext will come into picture but most likely
		 * nobody will be around by the time we have full 64-bit memory addressing
		 * requirement
		 */
		ASSERT((PHYSADDRHI(pa) & PCI64ADDR_HIGH) == 0);

		W_SM(&ddring[outidx].addrlow, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(PHYSADDRHI(pa) + di->dataoffsethigh));

		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
		W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
	} else {
		/* address extension for 32-bit PCI */
		uint32 ae;
		ASSERT(di->addrext);

		ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
		PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;
		ASSERT(PHYSADDRHI(pa) == 0);

		ctrl2 |= (ae << D64_CTRL2_AE_SHIFT) & D64_CTRL2_AE;
		W_SM(&ddring[outidx].addrlow, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(0 + di->dataoffsethigh));
		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
		W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
	}
	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2 | D64_CTRL2_PARITY));
		}
	}

#if defined(BCM47XX_CA9) && !defined(__NetBSD__) && !defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, (void *)(((uint)(&ddring[outidx])) & ~0x1f), 32, DMA_TX, NULL, NULL);
#endif /* BCM47XX_CA9 && !__NetBSD__ && !BULK_DESCR_FLUSH */

#if defined(__ARM_ARCH_7A__) && defined(CA7)
	/* memory barrier before posting the descriptor */
	DMB();
#endif
} /* dma64_dd_upd */

static bool
_dma32_addrext(osl_t *osh, dma32regs_t *dma32regs)
{
	uint32 w;

	OR_REG(osh, &dma32regs->control, XC_AE);
	w = R_REG(osh, &dma32regs->control);
	AND_REG(osh, &dma32regs->control, ~XC_AE);
	return ((w & XC_AE) == XC_AE);
}

/** Allocates one rx or tx descriptor ring. Does not allocate buffers. */
static bool
BCMATTACHFN_DMA_ATTACH(_dma_alloc)(dma_info_t *di, uint direction)
{
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		return dma64_alloc(di, direction);
	} else if (DMA32_ENAB(di)) {
		return dma32_alloc(di, direction);
	} else
		ASSERT(0);
}

/** !! may be called with core in reset */
static void
_dma_detach(dma_info_t *di)
{

	DMA_TRACE(("%s: dma_detach\n", di->name));

	/* shouldn't be here if descriptors are unreclaimed */
	ASSERT(di->txin == di->txout);
	ASSERT(di->rxin == di->rxout);

	/* free dma descriptor rings */
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		if (di->txd64)
			DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->txd64 - di->txdalign),
			                    di->txdalloc, (di->txdpaorig), di->tx_dmah);
		if (di->rxd64)
			DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->rxd64 - di->rxdalign),
			                    di->rxdalloc, (di->rxdpaorig), di->rx_dmah);
	} else if (DMA32_ENAB(di)) {
		if (di->txd32)
			DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->txd32 - di->txdalign),
			                    di->txdalloc, (di->txdpaorig), di->tx_dmah);
		if (di->rxd32)
			DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->rxd32 - di->rxdalign),
			                    di->rxdalloc, (di->rxdpaorig), di->rx_dmah);
	} else
		ASSERT(0);

	/* free packet pointer vectors */
	if (di->txp)
		MFREE(di->osh, (void *)di->txp, (di->ntxd * sizeof(void *)));
	if (di->rxp)
		MFREE(di->osh, (void *)di->rxp, (di->nrxd * sizeof(void *)));

	/* free tx packet DMA handles */
	if (di->txp_dmah)
		MFREE(di->osh, (void *)di->txp_dmah, di->ntxd * sizeof(hnddma_seg_map_t));

	/* free rx packet DMA handles */
	if (di->rxp_dmah)
		MFREE(di->osh, (void *)di->rxp_dmah, di->nrxd * sizeof(hnddma_seg_map_t));

#ifdef PCIE_PHANTOM_DEV
	/* Free up burstlength war segs/size */
	if (di->blwar_nsegs)
		MFREE(di->osh, (void *)di->blwar_nsegs, di->ntxd * sizeof(uint));

	if (di->blwar_size)
		MFREE(di->osh, (void *)di->blwar_size, di->ntxd * sizeof(uint));
#endif
	/* free our private info structure */
	MFREE(di->osh, (void *)di, sizeof(dma_info_t));

}

static bool
BCMATTACHFN_DMA_ATTACH(_dma_descriptor_align)(dma_info_t *di)
{
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		uint32 addrl;

		/* if this DMA channel is using indirect access, then configure the IndQSel register
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}

		/* Check to see if the descriptors need to be aligned on 4K/8K or not */
		if (di->d64txregs != NULL) {
			W_REG(di->osh, &di->d64txregs->addrlow, 0xff0);
			addrl = R_REG(di->osh, &di->d64txregs->addrlow);
			if (addrl != 0)
				return FALSE;
		} else if (di->d64rxregs != NULL) {
			W_REG(di->osh, &di->d64rxregs->addrlow, 0xff0);
			addrl = R_REG(di->osh, &di->d64rxregs->addrlow);
			if (addrl != 0)
				return FALSE;
		}
	}
	return TRUE;
}

/** return TRUE if this dma engine supports DmaExtendedAddrChanges, otherwise FALSE */
static bool
BCMATTACHFN_DMA_ATTACH(_dma_isaddrext)(dma_info_t *di)
{
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		/* DMA64 supports full 32- or 64-bit operation. AE is always valid */

		/* not all tx or rx channel are available */
		if (di->d64txregs != NULL) {
			/* if using indirect DMA access, then configure IndQSel */
			if (DMA_INDIRECT(di)) {
				dma_set_indqsel((hnddma_t *)di, FALSE);
			}
			if (!_dma64_addrext(di->osh, di->d64txregs)) {
				DMA_ERROR(("%s: _dma_isaddrext: DMA64 tx doesn't have AE set\n",
					di->name));
				ASSERT(0);
			}
			return TRUE;
		} else if (di->d64rxregs != NULL) {
			if (!_dma64_addrext(di->osh, di->d64rxregs)) {
				DMA_ERROR(("%s: _dma_isaddrext: DMA64 rx doesn't have AE set\n",
					di->name));
				ASSERT(0);
			}
			return TRUE;
		}
		return FALSE;
	} else if (DMA32_ENAB(di)) {
		if (di->d32txregs)
			return (_dma32_addrext(di->osh, di->d32txregs));
		else if (di->d32rxregs)
			return (_dma32_addrext(di->osh, di->d32rxregs));
	} else
		ASSERT(0);

	return FALSE;
}

/** initialize descriptor table base address */
static void
_dma_ddtable_init(dma_info_t *di, uint direction, dmaaddr_t pa)
{
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		if (!di->aligndesc_4k) {
			if (direction == DMA_TX)
				di->xmtptrbase = PHYSADDRLO(pa);
			else
				di->rcvptrbase = PHYSADDRLO(pa);
		}
		/* if this DMA channel is using indirect access, then configure the IndQSel register
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}

		if ((di->ddoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
			if (direction == DMA_TX) {
				W_REG(di->osh, &di->d64txregs->addrlow, (PHYSADDRLO(pa) +
				                                         di->ddoffsetlow));
				W_REG(di->osh, &di->d64txregs->addrhigh, (PHYSADDRHI(pa) +
				                                          di->ddoffsethigh));
			} else {
				W_REG(di->osh, &di->d64rxregs->addrlow, (PHYSADDRLO(pa) +
				                                         di->ddoffsetlow));
				W_REG(di->osh, &di->d64rxregs->addrhigh, (PHYSADDRHI(pa) +
				                                          di->ddoffsethigh));
			}
		} else {
			/* DMA64 32bits address extension */
			uint32 ae;
			ASSERT(di->addrext);
			ASSERT(PHYSADDRHI(pa) == 0);

			/* shift the high bit(s) from pa to ae */
			ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
			PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;

			if (direction == DMA_TX) {
				W_REG(di->osh, &di->d64txregs->addrlow, (PHYSADDRLO(pa) +
				                                         di->ddoffsetlow));
				W_REG(di->osh, &di->d64txregs->addrhigh, di->ddoffsethigh);
				SET_REG(di->osh, &di->d64txregs->control, D64_XC_AE,
					(ae << D64_XC_AE_SHIFT));
			} else {
				W_REG(di->osh, &di->d64rxregs->addrlow, (PHYSADDRLO(pa) +
				                                         di->ddoffsetlow));
				W_REG(di->osh, &di->d64rxregs->addrhigh, di->ddoffsethigh);
				SET_REG(di->osh, &di->d64rxregs->control, D64_RC_AE,
					(ae << D64_RC_AE_SHIFT));
			}
		}

	} else if (DMA32_ENAB(di)) {
		ASSERT(PHYSADDRHI(pa) == 0);
		if ((di->ddoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
			if (direction == DMA_TX)
				W_REG(di->osh, &di->d32txregs->addr, (PHYSADDRLO(pa) +
				                                      di->ddoffsetlow));
			else
				W_REG(di->osh, &di->d32rxregs->addr, (PHYSADDRLO(pa) +
				                                      di->ddoffsetlow));
		} else {
			/* dma32 address extension */
			uint32 ae;
			ASSERT(di->addrext);

			/* shift the high bit(s) from pa to ae */
			ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
			PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;

			if (direction == DMA_TX) {
				W_REG(di->osh, &di->d32txregs->addr, (PHYSADDRLO(pa) +
				                                      di->ddoffsetlow));
				SET_REG(di->osh, &di->d32txregs->control, XC_AE, ae <<XC_AE_SHIFT);
			} else {
				W_REG(di->osh, &di->d32rxregs->addr, (PHYSADDRLO(pa) +
				                                      di->ddoffsetlow));
				SET_REG(di->osh, &di->d32rxregs->control, RC_AE, ae <<RC_AE_SHIFT);
			}
		}
	} else
		ASSERT(0);
}

static void
_dma_fifoloopbackenable(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_fifoloopbackenable\n", di->name));

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		/* if this DMA channel is using indirect access, then configure the IndQSel register
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_LE);
	} else if (DMA32_ENAB(di))
		OR_REG(di->osh, &di->d32txregs->control, XC_LE);
	else
		ASSERT(0);
}

static void
_dma_rxinit(dma_info_t *di)
{
#ifdef BCMM2MDEV_ENABLED
	uint32 addrlow;
#endif
	DMA_TRACE(("%s: dma_rxinit\n", di->name));

	if (di->nrxd == 0)
		return;

	/* During the reset procedure, the active rxd may not be zero if pktpool is
	 * enabled, we need to reclaim active rxd to avoid rxd being leaked.
	 */
	if ((POOL_ENAB(di->pktpool)) && (NRXDACTIVE(di->rxin, di->rxout))) {
		_dma_rxreclaim(di);
	}

	ASSERT(di->rxin == di->rxout);
	di->rxin = di->rxout = di->rs0cd = 0;
	di->rxavail = di->nrxd - 1;

	/* To protect cases where driver doesn't post more buffers than the descriptors */
	/* because each buffer really means multiple descriptors */
	if (di->sep_rxhdr) {
		di->nrxpost = MIN(di->nrxpost, di->rxavail/2);
	}
	/* clear rx descriptor ring */
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		BZERO_SM((void *)(uintptr)di->rxd64, (di->nrxd * sizeof(dma64dd_t)));

		/* DMA engine with out alignment requirement requires table to be inited
		 * before enabling the engine
		 */
		if (!di->aligndesc_4k)
			_dma_ddtable_init(di, DMA_RX, di->rxdpa);

#ifdef BCMM2MDEV_ENABLED
	addrlow = R_REG(di->osh, &di->d64rxregs->addrlow);
	addrlow &= 0xffff;
	W_REG(di->osh, &di->d64rxregs->ptr, addrlow);
#endif

		_dma_rxenable(di);

		if (di->aligndesc_4k)
			_dma_ddtable_init(di, DMA_RX, di->rxdpa);

#ifdef BCMM2MDEV_ENABLED
	addrlow = R_REG(di->osh, &di->d64rxregs->addrlow);
	addrlow &= 0xffff;
	W_REG(di->osh, &di->d64rxregs->ptr, addrlow);
#endif

	} else if (DMA32_ENAB(di)) {
		BZERO_SM((void *)(uintptr)di->rxd32, (di->nrxd * sizeof(dma32dd_t)));
		_dma_rxenable(di);
		_dma_ddtable_init(di, DMA_RX, di->rxdpa);
	} else
		ASSERT(0);
}

static void
_dma_rxenable(dma_info_t *di)
{
	uint dmactrlflags = di->hnddma.dmactrlflags;

	DMA_TRACE(("%s: dma_rxenable\n", di->name));

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		uint32 control = (R_REG(di->osh, &di->d64rxregs->control) & D64_RC_AE) | D64_RC_RE;

		if ((dmactrlflags & DMA_CTRL_PEN) == 0)
			control |= D64_RC_PD;

		if (dmactrlflags & DMA_CTRL_ROC)
			control |= D64_RC_OC;

		/* These bits 20:18 (burstLen) of control register can be written but will take
		 * effect only if these bits are valid. So this will not affect previous versions
		 * of the DMA. They will continue to have those bits set to 0.
		 */
		control &= ~D64_RC_BL_MASK;
		control |= (di->rxburstlen << D64_RC_BL_SHIFT);

		control &= ~D64_RC_PC_MASK;
		control |= (di->rxprefetchctl << D64_RC_PC_SHIFT);

		control &= ~D64_RC_PT_MASK;
		control |= (di->rxprefetchthresh << D64_RC_PT_SHIFT);

		if (DMA_TRANSCOHERENT(di)) {
			control &= ~D64_RC_CO_MASK;
			control |= (1 << D64_RC_CO_SHIFT);
		}

#if defined(D11_SPLIT_RX_FD)
		/* Separate rx hdr descriptor */
		if (di->sep_rxhdr)
			control |= (di->sep_rxhdr << D64_RC_SHIFT);
#endif /* D11_SPLIT_RX_FD */

		W_REG(di->osh, &di->d64rxregs->control,
		      ((di->rxoffset << D64_RC_RO_SHIFT) | control));
	} else if (DMA32_ENAB(di)) {
		uint32 control = (R_REG(di->osh, &di->d32rxregs->control) & RC_AE) | RC_RE;

		if ((dmactrlflags & DMA_CTRL_PEN) == 0)
			control |= RC_PD;

		if (dmactrlflags & DMA_CTRL_ROC)
			control |= RC_OC;

		/* These bits 20:18 (burstLen) of control register can be written but will take
		 * effect only if these bits are valid. So this will not affect previous versions
		 * of the DMA. They will continue to have those bits set to 0.
		 */
		control &= ~RC_BL_MASK;
		control |= (di->rxburstlen << RC_BL_SHIFT);

		control &= ~RC_PC_MASK;
		control |= (di->rxprefetchctl << RC_PC_SHIFT);

		control &= ~RC_PT_MASK;
		control |= (di->rxprefetchthresh << RC_PT_SHIFT);

		W_REG(di->osh, &di->d32rxregs->control,
		      ((di->rxoffset << RC_RO_SHIFT) | control));
	} else
		ASSERT(0);
}

static void
_dma_rx_param_get(dma_info_t *di, uint16 *rxoffset, uint16 *rxbufsize)
{
	/* the normal values fit into 16 bits */
	*rxoffset = (uint16)di->rxoffset;
	*rxbufsize = (uint16)di->rxbufsize;
}

/**
 * !! rx entry routine
 * returns a pointer to the next frame received, or NULL if there are no more
 *   if DMA_CTRL_RXMULTI is defined, DMA scattering(multiple buffers) is supported
 *      with pkts chain
 *   otherwise, it's treated as giant pkt and will be tossed.
 *   The DMA scattering starts with normal DMA header, followed by first buffer data.
 *   After it reaches the max size of buffer, the data continues in next DMA descriptor
 *   buffer WITHOUT DMA header
 */
static void * BCMFASTPATH
_dma_rx(dma_info_t *di)
{
	void *p, *head, *tail;
	uint len;
	uint pkt_len;
	int resid = 0;
#if defined(D11_SPLIT_RX_FD)
	uint tcm_maxsize = 0;		/* max size of tcm descriptor */
#endif
	void *data;

next_frame:
	head = _dma_getnextrxp(di, FALSE);
	if (head == NULL)
		return (NULL);

	if (di->split_fifo == SPLIT_FIFO_0) {
		/* Fifo-0 handles all host address */
		/* Below PKT ops are not valid for host pkts */
		return head;
	}

	data = PKTDATA(di->osh, head);

#if (!defined(__mips__) && !defined(BCM47XX_CA9) && !defined(__NetBSD__))
	if (di->hnddma.dmactrlflags & DMA_CTRL_SDIO_RXGLOM) {
		/* In case of glommed pkt get length from hwheader */
		len = ltoh16(*((uint16 *)(data) + di->rxoffset/2 + 2)) + 4;

		*(uint16 *)(data) = (uint16)len;
	} else {
		len = ltoh16(*(uint16 *)(data));
	}
#else  /* !__mips__ && !BCM47XX_CA9 && !__NetBSD__ */
	{
	int read_count = 0;
	for (read_count = 200; read_count; read_count--) {
		len = ltoh16(*(uint16 *)(data));
		if (len != 0)
			break;
#if defined(BCM_GMAC3)
		if (PKTISFWDERBUF(di->osh, head)) {
			OSL_CACHE_INV_LINE(data);
		} else
#endif /* BCM_GMAC3 */
			DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
		OSL_DELAY(1);
	}

	if (!len) {
		DMA_ERROR(("%s: dma_rx: frame length (%d)\n", di->name, len));
		PKTFREE(di->osh, head, FALSE);
		goto next_frame;
	}

	}
#endif /* !__mips__ && !BCM47XX_CA9 && !__NetBSD__ */
	DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

	/* set actual length */
	pkt_len = MIN((di->rxoffset + len), di->rxbufsize);

#if defined(D11_SPLIT_RX_FD)
	if (di->sep_rxhdr) {
		/* If separate desc feature is enabled, set length for lcl & host pkt */
		tcm_maxsize = PKTLEN(di->osh, head);
		/* Dont support multi buffer pkt for now */
		if (pkt_len <= tcm_maxsize) {
			/* Full pkt sitting in TCM */
			PKTSETLEN(di->osh, head, pkt_len);	/* LCL pkt length */
			PKTSETFRAGUSEDLEN(di->osh, head, 0);	/* Host segment length */
		} else {
			/* Pkt got split between TCM and host */
			PKTSETLEN(di->osh, head, tcm_maxsize);	/* LCL pkt length */
			/* use PKTFRAGUSEDLEN to indicate actual length dma ed  by d11 */
			/* Cant use PKTFRAGLEN since if need to reclaim this */
			/* we need fraglen intact */
			PKTSETFRAGUSEDLEN(di->osh, head,
				(pkt_len - tcm_maxsize)); /* Host segment length */
		}
	} else
#endif /* D11_SPLIT_RX_FD */
	{
		PKTSETLEN(di->osh, head, pkt_len);
	}

	resid = len - (di->rxbufsize - di->rxoffset);

	if (resid <= 0) {
		/* Single frame, all good */
	} else if (di->hnddma.dmactrlflags & DMA_CTRL_RXSINGLE) {
		DMA_TRACE(("%s: dma_rx: corrupted length (%d)\n", di->name, len));
		PKTFREE(di->osh, head, FALSE);
		di->hnddma.rxgiants++;
		goto next_frame;
	} else {
		/* multi-buffer rx */
#ifdef BCMDBG
		/* get rid of compiler warning */
		p = NULL;
#endif /* BCMDBG */
		tail = head;
		while ((resid > 0) && (p = _dma_getnextrxp(di, FALSE))) {
			PKTSETNEXT(di->osh, tail, p);
			pkt_len = MIN(resid, (int)di->rxbufsize);
#if defined(D11_SPLIT_RX_FD)
			if (di->sep_rxhdr)
				PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
			else
#endif /* D11_SPLIT_RX_FD */
			PKTSETLEN(di->osh, p, pkt_len);

			tail = p;
			resid -= di->rxbufsize;
		}

#ifdef BCMDBG
		if (resid > 0) {
			uint16 cur;
			ASSERT(p == NULL);
			cur = (DMA64_ENAB(di) && DMA64_MODE(di)) ?
				B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
				di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t) :
				B2I(R_REG(di->osh, &di->d32rxregs->status) & RS_CD_MASK,
				dma32dd_t);
			DMA_ERROR(("_dma_rx, rxin %d rxout %d, hw_curr %d\n",
				di->rxin, di->rxout, cur));
		}
#endif /* BCMDBG */

		if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
			DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));
			PKTFREE(di->osh, head, FALSE);
			di->hnddma.rxgiants++;
			goto next_frame;
		}
	}

	return (head);
}

/*
 * there are cases where teh host mem can't be used when the dongle is in certian
 * state, so this covers that case
 * used only for RX FIFO0 with the split rx builds
*/

int
dma_rxfill_suspend(hnddma_t *dmah, bool suspended)
{
	dma_info_t * di = DI_INFO(dmah);

	di->rxfill_suspended = suspended;
	return 0;
}

/**
 * A 'receive' DMA engine must be fed with buffers to write received data into. This function
 * 'posts' receive buffers. If the 'packet pool' feature is enabled, the buffers are drawn from the
 * packet pool. Otherwise, the buffers are retrieved using the OSL 'PKTGET' macro.
 *
 *  return FALSE is refill failed completely and ring is empty
 *  this will stall the rx dma and user might want to call rxfill again asap
 *  This unlikely happens on memory-rich NIC, but often on memory-constrained dongle
 */
static bool BCMFASTPATH
_dma_rxfill(dma_info_t *di)
{
	void *p;
	uint16 rxin, rxout;
	uint32 flags = 0;
	uint n;
	uint i;
	dmaaddr_t pa;
	uint extra_offset = 0, extra_pad;
	bool ring_empty;
	uint alignment_req = (di->hnddma.dmactrlflags & DMA_CTRL_USB_BOUNDRY4KB_WAR) ?
				16 : 1;	/* MUST BE POWER of 2 */
#if defined(D11_SPLIT_RX_FD)
	uint pktlen;
	dma64addr_t pa64 = {0, 0};

	if (di->split_fifo) {
		/* SPLIFIFO rxfill is handled separately */
		return dma_splitrxfill(di);
	}
#endif /* D11_SPLIT_RX_FD */

	ring_empty = FALSE;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;

#if defined(D11_SPLIT_RX_FD)
	/* if sep_rxhdr is enabled, for every pkt, two descriptors are programmed */
	/* NRXDACTIVE(rxin, rxout) would show 2 times no of actual full pkts */
	if (di->sep_rxhdr) {
		n = di->nrxpost - (NRXDACTIVE(rxin, rxout) / 2);
	} else
#endif /* D11_SPLIT_RX_FD */
	{
		n = di->nrxpost - NRXDACTIVE(rxin, rxout);
	}

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;

	DMA_TRACE(("%s: dma_rxfill: post %d\n", di->name, n));

	for (i = 0; i < n; i++) {
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
		if (POOL_ENAB(di->pktpool)) {
			ASSERT(di->pktpool);
			p = pktpool_get(di->pktpool);
#ifdef BCMDBG_POOL
			if (p)
				PKTPOOLSETSTATE(p, POOL_RXFILL);
#endif /* BCMDBG_POOL */
		}
		else {
			p = PKTGET(di->osh, (di->rxbufsize + extra_offset +  alignment_req - 1),
				FALSE);
		}
		if (p == NULL) {
			DMA_TRACE(("%s: dma_rxfill: out of rxbufs\n", di->name));
			if (i == 0) {
				if (DMA64_ENAB(di) && DMA64_MODE(di)) {
					if (dma64_rxidle(di)) {
						DMA_TRACE(("%s: rxfill64: ring is empty !\n",
							di->name));
						ring_empty = TRUE;
					}
				} else if (DMA32_ENAB(di)) {
					if (dma32_rxidle(di)) {
						DMA_TRACE(("%s: rxfill32: ring is empty !\n",
							di->name));
						ring_empty = TRUE;
					}
				} else
					ASSERT(0);
			}
			di->hnddma.rxnobuf++;
			break;
		}
		/* reserve an extra headroom, if applicable */
		if (di->hnddma.dmactrlflags & DMA_CTRL_USB_BOUNDRY4KB_WAR) {
			extra_pad = ((alignment_req - (uint)(((unsigned long)PKTDATA(di->osh, p) -
				(unsigned long)(uchar *)0))) & (alignment_req - 1));
		} else
			extra_pad = 0;

		if (extra_offset + extra_pad)
			PKTPULL(di->osh, p, extra_offset + extra_pad);

#ifdef CTFMAP
		/* mark as ctf buffer for fast mapping */
		if (CTF_ENAB(kcih)) {
			ASSERT((((uint32)PKTDATA(di->osh, p)) & 31) == 0);
			PKTSETCTF(di->osh, p);
		}
#endif /* CTFMAP */

		/* Do a cached write instead of uncached write since DMA_MAP
		 * will flush the cache.
		*/
		*(uint32 *)(PKTDATA(di->osh, p)) = 0;

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__)) && defined(BCM_GMAC3)
		/* Packets tagged as FWDER_BUF are ensured to only have FWDER_PKTMAPSZ
		 * (at most) in the cache, with the first 2Bytes dirty in cache.
		 * FWDER_BUF tagged packets are placed in NON ACP address space. Non
		 * FWDER_BUF packets are placed into ACP region and avail of HW cache
		 * coherency via ACP.
		 */
		if (PKTISFWDERBUF(di->osh, p)) {
			void *va = PKTDATA(di->osh, p);
			pa = virt_to_phys_noacp(va); /* map to non acp address space */
			OSL_CACHE_FLUSH_LINE(va); /* 1 cache line */
		} else {
			/* cache flush first 4Byte length */
			DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
			/* cache invalidate entire packet buffer */
			pa = DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX, p, NULL);
		}
#else /* !BCM_GMAC3 based FWDER_BUF optimization */

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
#endif
#if defined(SGLIST_RX_SUPPORT)
		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout], sizeof(hnddma_seg_map_t));

		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
		              di->rxbufsize, DMA_RX, p,
		              &di->rxp_dmah[rxout]);
#else  /* !SGLIST_RX_SUPPORT */
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
		             di->rxbufsize, DMA_RX, p, NULL);
#endif /* !SGLIST_RX_SUPPORT */

#endif /* !(linux && (BCM47XX_CA9 || __mips__) && BCM_GMAC3) */

		ASSERT(ISALIGNED(PHYSADDRLO(pa), 4));

		/* save the free packet pointer */
		ASSERT(di->rxp[rxout] == NULL);
		di->rxp[rxout] = p;

#if defined(D11_SPLIT_RX_FD)
		if (!di->sep_rxhdr)
#endif
		{
			/* reset flags for each descriptor */
			flags = 0;
			if (DMA64_ENAB(di) && DMA64_MODE(di)) {
				if (rxout == (di->nrxd - 1))
					flags = D64_CTRL1_EOT;
				/* for bufs in local mem */
				if (DMA_TRANSCOHERENT(di))
					flags |= D64_CTRL1_COHERENT;
				dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, di->rxbufsize);
			} else if (DMA32_ENAB(di)) {
				if (rxout == (di->nrxd - 1))
					flags = CTRL_EOT;

				ASSERT(PHYSADDRHI(pa) == 0);
				dma32_dd_upd(di, di->rxd32, pa, rxout, &flags, di->rxbufsize);
			} else
				ASSERT(0);
			rxout = NEXTRXD(rxout);

		}
#if defined(D11_SPLIT_RX_FD)
		else {
			/* TCM Descriptor */
			flags = 0;
			pktlen = PKTLEN(di->osh, p);
			if (rxout == (di->nrxd - 1))
				flags = D64_CTRL1_EOT;

			/* MAR SOF, so start frame will go to this descriptor */
			flags |= D64_CTRL1_SOF;
			/* rx hdr split case where the buf is from local
			 * memory.
			 */
			if (DMA_TRANSCOHERENT(di))
				flags |= D64_CTRL1_COHERENT;
			dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, pktlen);
			rxout = NEXTRXD(rxout);		/* update rxout */

			/* Now program host descriptor */
			/* Mark up this descriptor that its a host descriptor */
			/* store 0x80000000 so that dma_rx need'nt process this descriptor */
			di->rxp[rxout] = (void*)PCI64ADDR_HIGH;
			flags = 0;	/* Reset the flags */

			if (rxout == (di->nrxd - 1))
				flags = D64_CTRL1_EOT;

			/* Extract out host length */
			pktlen = PKTFRAGLEN(di->osh, p, 1);

			/* Extract out host addresses */
			pa64.loaddr = PKTFRAGDATA_LO(di->osh, p, 1);
			pa64.hiaddr = PKTFRAGDATA_HI(di->osh, p, 1);

			/* load the descriptor */
			dma64_dd_upd_64_from_params(di, di->rxd64, pa64, rxout, &flags, pktlen);
			rxout = NEXTRXD(rxout);	/* update rxout */
		}
#endif /* D11_SPLIT_RX_FD */
	}

#if !defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif

	/* update the chip lastdscr pointer */
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
#if defined(BULK_DESCR_FLUSH)
		uint32 flush_cnt = NRXDACTIVE(di->rxout, rxout);
		if (rxout < di->rxout) {
			DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(rxout),
			        DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(flush_cnt),
		        DMA_TX, NULL, NULL);
#endif /* BULK_DESCR_FLUSH */
		W_REG(di->osh, &di->d64rxregs->ptr, di->rcvptrbase + I2B(rxout, dma64dd_t));
	} else if (DMA32_ENAB(di)) {
		W_REG(di->osh, &di->d32rxregs->ptr, I2B(rxout, dma32dd_t));
	} else
		ASSERT(0);

#if defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif

	return ring_empty;
} /* _dma_rxfill */

/** like getnexttxp but no reclaim */
static void *
_dma_peeknexttxp(dma_info_t *di)
{
	uint16 end, i;

	if (di->ntxd == 0)
		return (NULL);

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		/* if this DMA channel is using indirect access, then configure the IndQSel register
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}

		end = (uint16)B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
		           di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
		di->xs0cd = end;
	} else if (DMA32_ENAB(di)) {
		end = (uint16)B2I(R_REG(di->osh, &di->d32txregs->status) & XS_CD_MASK, dma32dd_t);
		di->xs0cd = end;
	} else
		ASSERT(0);

	for (i = di->txin; i != end; i = NEXTTXD(i))
		if (di->txp[i])
			return (di->txp[i]);

	return (NULL);
}

int
_dma_peekntxp(dma_info_t *di, int *len, void *txps[], txd_range_t range)
{
	uint16 start, end, i;
	uint act;
	void *txp = NULL;
	int k, len_max;

	DMA_TRACE(("%s: dma_peekntxp\n", di->name));

	ASSERT(len);
	ASSERT(txps);
	ASSERT(di);
	if (di->ntxd == 0) {
		*len = 0;
		return BCME_ERROR;
	}

	len_max = *len;
	*len = 0;

	start = di->xs0cd;
	if (range == HNDDMA_RANGE_ALL)
		end = di->txout;
	else {
		if (DMA64_ENAB(di)) {
			/* if this DMA channel is using indirect access, then configure the
			 * IndQSel register for this DMA channel
			 */
			if (DMA_INDIRECT(di)) {
				dma_set_indqsel((hnddma_t *)di, FALSE);
			}

			end = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
				di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);

			act = (uint)(R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK);
			act = (act - di->xmtptrbase) & D64_XS0_CD_MASK;
			act = (uint)B2I(act, dma64dd_t);
		} else {
			end = B2I(R_REG(di->osh, &di->d32txregs->status) & XS_CD_MASK, dma32dd_t);

			act = (uint)((R_REG(di->osh, &di->d32txregs->status) & XS_AD_MASK) >>
				XS_AD_SHIFT);
			act = (uint)B2I(act, dma32dd_t);
		}

		di->xs0cd = end;
		if (end != act)
			end = PREVTXD(act);
	}

	if ((start == 0) && (end > di->txout))
		return BCME_ERROR;

	k = 0;
	for (i = start; i != end; i = NEXTTXD(i)) {
		txp = di->txp[i];
		if (txp != NULL) {
			if (k < len_max)
				txps[k++] = txp;
			else
				break;
		}
	}
	*len = k;

	return BCME_OK;
}

/** like getnextrxp but not take off the ring */
static void *
_dma_peeknextrxp(dma_info_t *di)
{
	uint16 end, i;

	if (di->nrxd == 0)
		return (NULL);

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		end = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
			di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
		di->rs0cd = end;
	} else if (DMA32_ENAB(di)) {
		end = (uint16)B2I(R_REG(di->osh, &di->d32rxregs->status) & RS_CD_MASK, dma32dd_t);
		di->rs0cd = end;
	} else
		ASSERT(0);

	for (i = di->rxin; i != end; i = NEXTRXD(i))
		if (di->rxp[i])
			return (di->rxp[i]);

	return (NULL);
}

static void
_dma_rxreclaim(dma_info_t *di)
{
	void *p;
	bool origcb = TRUE;

	DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

	if (POOL_ENAB(di->pktpool) &&
	    ((origcb = pktpool_emptycb_disabled(di->pktpool)) == FALSE))
		pktpool_emptycb_disable(di->pktpool, TRUE);

	/* For split fifo, this function expects fifo-1 di to proceed further. */
	/* fifo-0 requests can be discarded since fifo-1 will reclaim both fifos */
	if (di->split_fifo == SPLIT_FIFO_0) {
		return;
	}

	while ((p = _dma_getnextrxp(di, TRUE))) {
		/* For unframed data, we don't have any packets to free */
#if (defined(__mips__) || defined(BCM47XX_CA9)) && defined(linux)
		if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED))
#endif /* (__mips__ || BCM47XX_CA9) && linux */
			PKTFREE(di->osh, p, FALSE);
	}

	if (origcb == FALSE)
		pktpool_emptycb_disable(di->pktpool, FALSE);
}

/** returns entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_getnextrxp(dma_info_t *di, bool forceall)
{
	if (di->nrxd == 0)
		return (NULL);

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		return dma64_getnextrxp(di, forceall);
	} else if (DMA32_ENAB(di)) {
		return dma32_getnextrxp(di, forceall);
	} else
		ASSERT(0);
}

static void
_dma_txblock(dma_info_t *di)
{
	di->hnddma.txavail = 0;
}

static void
_dma_txunblock(dma_info_t *di)
{
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

static uint
_dma_txactive(dma_info_t *di)
{
	return NTXDACTIVE(di->txin, di->txout);
}

/* count the number of tx packets that are queued to the dma ring */
uint
dma_txp(hnddma_t *dmah)
{
	uint count = 0;
	uint16 i;
	dma_info_t * di = DI_INFO(dmah);

	/* count the number of txp pointers that are non-null */
	for (i = 0; i < di->ntxd; i++) {
		if (di->txp[i] != NULL) {
			count++;
		}
	}

	return count;
}

static uint
_dma_txpending(dma_info_t *di)
{
	uint16 curr;

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		/* if this DMA channel is using indirect access, then configure the IndQSel register
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}

		curr = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
		           di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
		di->xs0cd = curr;
	} else if (DMA32_ENAB(di)) {
		curr = B2I(R_REG(di->osh, &di->d32txregs->status) & XS_CD_MASK, dma32dd_t);
		di->xs0cd = curr;
	} else
		ASSERT(0);

	return NTXDACTIVE(curr, di->txout);
}

static uint
_dma_txcommitted(dma_info_t *di)
{
	uint16 ptr;
	uint txin = di->txin;

	if (txin == di->txout)
		return 0;

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		/* if this DMA channel is using indirect access, then configure the IndQSel
		 * for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}
		ptr = B2I(R_REG(di->osh, &di->d64txregs->ptr), dma64dd_t);
	} else if (DMA32_ENAB(di)) {
		ptr = B2I(R_REG(di->osh, &di->d32txregs->ptr), dma32dd_t);
	} else
		ASSERT(0);

	return NTXDACTIVE(di->txin, ptr);
}

static uint
_dma_rxactive(dma_info_t *di)
{
	return NRXDACTIVE(di->rxin, di->rxout);
}

static uint
_dma_activerxbuf(dma_info_t *di)
{
	uint16 curr, ptr;
	curr = B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
	ptr =  B2I(((R_REG(di->osh, &di->d64rxregs->ptr) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
	return NRXDACTIVE(curr, ptr);
}


static void
_dma_counterreset(dma_info_t *di)
{
	/* reset all software counter */
	di->hnddma.rxgiants = 0;
	di->hnddma.rxnobuf = 0;
	di->hnddma.txnobuf = 0;
}

static uint
_dma_ctrlflags(dma_info_t *di, uint mask, uint flags)
{
	uint dmactrlflags;

	if (!di) {
		DMA_ERROR(("_dma_ctrlflags: NULL dma handle\n"));
		return (0);
	}

	dmactrlflags = di->hnddma.dmactrlflags;
	ASSERT((flags & ~mask) == 0);

	dmactrlflags &= ~mask;
	dmactrlflags |= flags;

	/* If trying to enable parity, check if parity is actually supported */
	if (dmactrlflags & DMA_CTRL_PEN) {
		uint32 control;

		if (DMA64_ENAB(di) && DMA64_MODE(di)) {
			/* if using indirect DMA access, then configure IndQSel */
			if (DMA_INDIRECT(di)) {
				dma_set_indqsel((hnddma_t *)di, FALSE);
			}

			control = R_REG(di->osh, &di->d64txregs->control);
			W_REG(di->osh, &di->d64txregs->control, control | D64_XC_PD);
			if (R_REG(di->osh, &di->d64txregs->control) & D64_XC_PD) {
				/* We *can* disable it so it is supported,
				 * restore control register
				 */
				W_REG(di->osh, &di->d64txregs->control, control);
			} else {
				/* Not supported, don't allow it to be enabled */
				dmactrlflags &= ~DMA_CTRL_PEN;
			}
		} else if (DMA32_ENAB(di)) {
			control = R_REG(di->osh, &di->d32txregs->control);
			W_REG(di->osh, &di->d32txregs->control, control | XC_PD);
			if (R_REG(di->osh, &di->d32txregs->control) & XC_PD) {
				W_REG(di->osh, &di->d32txregs->control, control);
			} else {
				/* Not supported, don't allow it to be enabled */
				dmactrlflags &= ~DMA_CTRL_PEN;
			}
		} else
			ASSERT(0);
	}

	di->hnddma.dmactrlflags = dmactrlflags;

	return (dmactrlflags);
}

/** get the address of the var in order to change later */
static uintptr
_dma_getvar(dma_info_t *di, const char *name)
{
	if (!strcmp(name, "&txavail"))
		return ((uintptr) &(di->hnddma.txavail));
	else if (!strcmp(name, "&rxavail"))
		return ((uintptr) &(di->rxavail));
	else {
		ASSERT(0);
	}
	return (0);
}

static uint
_dma_avoidancecnt(dma_info_t *di)
{
	return (di->dma_avoidance_cnt);
}

void
dma_txpioloopback(osl_t *osh, dma32regs_t *regs)
{
	OR_REG(osh, &regs->control, XC_LE);
}

static
uint8 BCMATTACHFN_DMA_ATTACH(dma_align_sizetobits)(uint size)
{
	uint8 bitpos = 0;
	ASSERT(size);
	ASSERT(!(size & (size-1)));
	while (size >>= 1) {
		bitpos ++;
	}
	return (bitpos);
}

/**
 * Allocates one rx or tx descriptor ring. Does not allocate buffers.
 * This function ensures that the DMA descriptor ring will not get allocated
 * across Page boundary. If the allocation is done across the page boundary
 * at the first time, then it is freed and the allocation is done at
 * descriptor ring size aligned location. This will ensure that the ring will
 * not cross page boundary
 */
static void *
BCMATTACHFN_DMA_ATTACH(dma_ringalloc)(osl_t *osh, uint32 boundary, uint size, uint16 *alignbits,
	uint* alloced, dmaaddr_t *descpa, osldma_t **dmah)
{
	void * va;
	uint32 desc_strtaddr;
	uint32 alignbytes = 1 << *alignbits;

	if ((va = DMA_ALLOC_CONSISTENT(osh, size, *alignbits, alloced, descpa, dmah)) == NULL)
		return NULL;

	desc_strtaddr = (uint32)ROUNDUP((uint)PHYSADDRLO(*descpa), alignbytes);
	if (((desc_strtaddr + size - 1) & boundary) !=
	    (desc_strtaddr & boundary)) {
		*alignbits = dma_align_sizetobits(size);
		DMA_FREE_CONSISTENT(osh, va,
		                    size, *descpa, *dmah);
		va = DMA_ALLOC_CONSISTENT(osh, size, *alignbits, alloced, descpa, dmah);
	}
	return va;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
dma32_dumpring(dma_info_t *di, struct bcmstrbuf *b, dma32dd_t *ring, uint start, uint end,
	uint max_num)
{
	uint i;

	for (i = start; i != end; i = XXD((i + 1), max_num)) {
		/* in the format of high->low 8 bytes */
		bcm_bprintf(b, "ring index %d: 0x%x %x\n",
			i, R_SM(&ring[i].addr), R_SM(&ring[i].ctrl));
	}
}

static void
dma32_dumptx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	if (di->ntxd == 0)
		return;

	bcm_bprintf(b, "DMA32: txd32 %p txdpa 0x%lx txp %p txin %d txout %d "
	            "txavail %d txnodesc %d\n", di->txd32, PHYSADDRLO(di->txdpa), di->txp, di->txin,
	            di->txout, di->hnddma.txavail, di->hnddma.txnodesc);

	bcm_bprintf(b, "xmtcontrol 0x%x xmtaddr 0x%x xmtptr 0x%x xmtstatus 0x%x\n",
		R_REG(di->osh, &di->d32txregs->control),
		R_REG(di->osh, &di->d32txregs->addr),
		R_REG(di->osh, &di->d32txregs->ptr),
		R_REG(di->osh, &di->d32txregs->status));

	if (dumpring && di->txd32)
		dma32_dumpring(di, b, di->txd32, di->txin, di->txout, di->ntxd);
}

static void
dma32_dumprx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	if (di->nrxd == 0)
		return;

	bcm_bprintf(b, "DMA32: rxd32 %p rxdpa 0x%lx rxp %p rxin %d rxout %d\n",
	            di->rxd32, PHYSADDRLO(di->rxdpa), di->rxp, di->rxin, di->rxout);

	bcm_bprintf(b, "rcvcontrol 0x%x rcvaddr 0x%x rcvptr 0x%x rcvstatus 0x%x\n",
		R_REG(di->osh, &di->d32rxregs->control),
		R_REG(di->osh, &di->d32rxregs->addr),
		R_REG(di->osh, &di->d32rxregs->ptr),
		R_REG(di->osh, &di->d32rxregs->status));
	if (di->rxd32 && dumpring)
		dma32_dumpring(di, b, di->rxd32, di->rxin, di->rxout, di->nrxd);
}

static void
dma32_dump(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	dma32_dumptx(di, b, dumpring);
	dma32_dumprx(di, b, dumpring);
}

static void
dma64_dumpring(dma_info_t *di, struct bcmstrbuf *b, dma64dd_t *ring, uint start, uint end,
	uint max_num)
{
	uint i;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	for (i = start; i != end; i = XXD((i + 1), max_num)) {
		/* in the format of high->low 16 bytes */
		if (b) {
			bcm_bprintf(b, "ring index %d: 0x%x %x %x %x\n",
			i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
			R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1));
		}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
		else {
			DMA_ERROR(("ring index %d: 0x%x %x %x %x\n",
			i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
			R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1)));
		}
#endif /* BCMDBG || BCMDBG_DUMP */
	}
}

static void
dma64_dumptx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	if (di->ntxd == 0)
		return;
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	if (b) {
		bcm_bprintf(b, "DMA64: txd64 %p txdpa 0x%lx txdpahi 0x%lx txp %p txin %d txout %d "
			"txavail %d txnodesc %d\n", di->txd64, PHYSADDRLO(di->txdpa),
			PHYSADDRHI(di->txdpaorig), di->txp, di->txin, di->txout, di->hnddma.txavail,
			di->hnddma.txnodesc);

		bcm_bprintf(b, "xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
			    "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
			    R_REG(di->osh, &di->d64txregs->control),
			    R_REG(di->osh, &di->d64txregs->addrlow),
			    R_REG(di->osh, &di->d64txregs->addrhigh),
			    R_REG(di->osh, &di->d64txregs->ptr),
			    R_REG(di->osh, &di->d64txregs->status0),
			    R_REG(di->osh, &di->d64txregs->status1));

		bcm_bprintf(b, "DMA64: DMA avoidance applied %d\n", di->dma_avoidance_cnt);
	}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	else {
		DMA_ERROR(("DMA64: txd64 %p txdpa 0x%lx txdpahi 0x%lx txp %p txin %d txout %d "
		       "txavail %d txnodesc %d\n", di->txd64, (unsigned long)PHYSADDRLO(di->txdpa),
		       (unsigned long)PHYSADDRHI(di->txdpaorig), di->txp, di->txin, di->txout,
		       di->hnddma.txavail, di->hnddma.txnodesc));

		DMA_ERROR(("xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
		       "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
		       R_REG(di->osh, &di->d64txregs->control),
		       R_REG(di->osh, &di->d64txregs->addrlow),
		       R_REG(di->osh, &di->d64txregs->addrhigh),
		       R_REG(di->osh, &di->d64txregs->ptr),
		       R_REG(di->osh, &di->d64txregs->status0),
		       R_REG(di->osh, &di->d64txregs->status1)));
	}
#endif /* BCMDBG || BCMDBG_DUMP */
	if (dumpring && di->txd64) {
		dma64_dumpring(di, b, di->txd64, di->txin, di->txout, di->ntxd);
	}
}

static void
dma64_dumprx(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	if (di->nrxd == 0)
		return;

	bcm_bprintf(b, "DMA64: rxd64 %p rxdpa 0x%lx rxdpahi 0x%lx rxp %p rxin %d rxout %d\n",
	            di->rxd64, PHYSADDRLO(di->rxdpa), PHYSADDRHI(di->rxdpaorig), di->rxp,
	            di->rxin, di->rxout);

	bcm_bprintf(b, "rcvcontrol 0x%x rcvaddrlow 0x%x rcvaddrhigh 0x%x rcvptr "
		       "0x%x rcvstatus0 0x%x rcvstatus1 0x%x\n",
		       R_REG(di->osh, &di->d64rxregs->control),
		       R_REG(di->osh, &di->d64rxregs->addrlow),
		       R_REG(di->osh, &di->d64rxregs->addrhigh),
		       R_REG(di->osh, &di->d64rxregs->ptr),
		       R_REG(di->osh, &di->d64rxregs->status0),
		       R_REG(di->osh, &di->d64rxregs->status1));
	if (di->rxd64 && dumpring) {
		dma64_dumpring(di, b, di->rxd64, di->rxin, di->rxout, di->nrxd);
	}
}

static void
dma64_dump(dma_info_t *di, struct bcmstrbuf *b, bool dumpring)
{
	dma64_dumptx(di, b, dumpring);
	dma64_dumprx(di, b, dumpring);
}

#endif	/* BCMDBG || BCMDBG_DUMP */


/* 32-bit DMA functions */

static void
dma32_txinit(dma_info_t *di)
{
	uint32 control = XC_XE;

	DMA_TRACE(("%s: dma_txinit\n", di->name));

	if (di->ntxd == 0)
		return;

	di->txin = di->txout = di->xs0cd = 0;
	di->hnddma.txavail = di->ntxd - 1;

	/* clear tx descriptor ring */
	BZERO_SM(DISCARD_QUAL(di->txd32, void), (di->ntxd * sizeof(dma32dd_t)));

	/* These bits 20:18 (burstLen) of control register can be written but will take
	 * effect only if these bits are valid. So this will not affect previous versions
	 * of the DMA. They will continue to have those bits set to 0.
	 */
	control |= (di->txburstlen << XC_BL_SHIFT);
	control |= (di->txmultioutstdrd << XC_MR_SHIFT);
	control |= (di->txprefetchctl << XC_PC_SHIFT);
	control |= (di->txprefetchthresh << XC_PT_SHIFT);

	if ((di->hnddma.dmactrlflags & DMA_CTRL_PEN) == 0)
		control |= XC_PD;
	W_REG(di->osh, &di->d32txregs->control, control);
	_dma_ddtable_init(di, DMA_TX, di->txdpa);
}

static bool
dma32_txenabled(dma_info_t *di)
{
	uint32 xc;

	/* If the chip is dead, it is not enabled :-) */
	xc = R_REG(di->osh, &di->d32txregs->control);
	return ((xc != 0xffffffff) && (xc & XC_XE));
}

static void
dma32_txsuspend(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txsuspend\n", di->name));

	if (di->ntxd == 0)
		return;

	OR_REG(di->osh, &di->d32txregs->control, XC_SE);
}

static void
dma32_txresume(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txresume\n", di->name));

	if (di->ntxd == 0)
		return;

	AND_REG(di->osh, &di->d32txregs->control, ~XC_SE);
}

static bool
dma32_txsuspended(dma_info_t *di)
{
	return (di->ntxd == 0) || ((R_REG(di->osh, &di->d32txregs->control) & XC_SE) == XC_SE);
}

#ifdef WL_MULTIQUEUE
static void
dma32_txflush(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txflush\n", di->name));

	if (di->ntxd == 0)
		return;

	OR_REG(di->osh, &di->d32txregs->control, XC_SE | XC_FL);
}

static void
dma32_txflush_clear(dma_info_t *di)
{
	uint32 status;

	DMA_TRACE(("%s: dma_txflush_clear\n", di->name));

	if (di->ntxd == 0)
		return;

	SPINWAIT(((status = (R_REG(di->osh, &di->d32txregs->status) & XS_XS_MASK))
		 != XS_XS_DISABLED) &&
		 (status != XS_XS_IDLE) &&
		 (status != XS_XS_STOPPED),
		 (10000));
	AND_REG(di->osh, &di->d32txregs->control, ~XC_FL);
}
#endif /* WL_MULTIQUEUE */

static void
dma32_txreclaim(dma_info_t *di, txd_range_t range)
{
	void *p;

	DMA_TRACE(("%s: dma_txreclaim %s\n", di->name,
	           (range == HNDDMA_RANGE_ALL) ? "all" :
	           ((range == HNDDMA_RANGE_TRANSMITTED) ? "transmitted" : "transfered")));

	if (di->txin == di->txout)
		return;

	while ((p = dma32_getnexttxp(di, range)))
		PKTFREE(di->osh, p, TRUE);
}

static bool
dma32_txstopped(dma_info_t *di)
{
	return ((R_REG(di->osh, &di->d32txregs->status) & XS_XS_MASK) == XS_XS_STOPPED);
}

static bool
dma32_rxstopped(dma_info_t *di)
{
	return ((R_REG(di->osh, &di->d32rxregs->status) & RS_RS_MASK) == RS_RS_STOPPED);
}

/** Allocates one rx or tx descriptor ring. Does not allocate buffers. */
static bool
BCMATTACHFN_DMA_ATTACH(dma32_alloc)(dma_info_t *di, uint direction)
{
	uint size;	/**< nr of bytes that one descriptor ring will occupy */
	uint ddlen;
	void *va;
	uint alloced;
	uint16 align;
	uint16 align_bits;

	ddlen = sizeof(dma32dd_t);

	size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

	alloced = 0;
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);

	if (direction == DMA_TX) {
		if ((va = dma_ringalloc(di->osh, D32RINGALIGN, size, &align_bits, &alloced,
			&di->txdpaorig, &di->tx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n",
			           di->name));
			return FALSE;
		}

		PHYSADDRHISET(di->txdpa, 0);
		ASSERT(PHYSADDRHI(di->txdpaorig) == 0);
		di->txd32 = (dma32dd_t *)ROUNDUP((uintptr)va, align);
		di->txdalign = (uint)((int8 *)(uintptr)di->txd32 - (int8 *)va);

		PHYSADDRLOSET(di->txdpa, PHYSADDRLO(di->txdpaorig) + di->txdalign);
		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->txdpa) >= PHYSADDRLO(di->txdpaorig));

		di->txdalloc = alloced;
		ASSERT(ISALIGNED(di->txd32, align));
	} else {
		if ((va = dma_ringalloc(di->osh, D32RINGALIGN, size, &align_bits, &alloced,
			&di->rxdpaorig, &di->rx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n",
			           di->name));
			return FALSE;
		}

		PHYSADDRHISET(di->rxdpa, 0);
		ASSERT(PHYSADDRHI(di->rxdpaorig) == 0);
		di->rxd32 = (dma32dd_t *)ROUNDUP((uintptr)va, align);
		di->rxdalign = (uint)((int8 *)(uintptr)di->rxd32 - (int8 *)va);

		PHYSADDRLOSET(di->rxdpa, PHYSADDRLO(di->rxdpaorig) + di->rxdalign);
		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->rxdpa) >= PHYSADDRLO(di->rxdpaorig));
		di->rxdalloc = alloced;
		ASSERT(ISALIGNED(di->rxd32, align));
	}

	return TRUE;
}

static bool
dma32_txreset(dma_info_t *di)
{
	uint32 status;

	if (di->ntxd == 0)
		return TRUE;

	/* suspend tx DMA first */
	W_REG(di->osh, &di->d32txregs->control, XC_SE);
	SPINWAIT(((status = (R_REG(di->osh, &di->d32txregs->status) & XS_XS_MASK))
		 != XS_XS_DISABLED) &&
		 (status != XS_XS_IDLE) &&
		 (status != XS_XS_STOPPED),
		 (10000));

	W_REG(di->osh, &di->d32txregs->control, 0);
	SPINWAIT(((status = (R_REG(di->osh,
	         &di->d32txregs->status) & XS_XS_MASK)) != XS_XS_DISABLED),
	         10000);

	/* We should be disabled at this point */
	if (status != XS_XS_DISABLED) {
		DMA_ERROR(("%s: status != D64_XS0_XS_DISABLED 0x%x\n", __FUNCTION__, status));
		ASSERT(status == XS_XS_DISABLED);
		OSL_DELAY(300);
	}

	return (status == XS_XS_DISABLED);
}

static bool
dma32_rxidle(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxidle\n", di->name));

	if (di->nrxd == 0)
		return TRUE;

	return ((R_REG(di->osh, &di->d32rxregs->status) & RS_CD_MASK) ==
	        R_REG(di->osh, &di->d32rxregs->ptr));
}

static bool
dma32_rxreset(dma_info_t *di)
{
	uint32 status;

	if (di->nrxd == 0)
		return TRUE;

	W_REG(di->osh, &di->d32rxregs->control, 0);
	SPINWAIT(((status = (R_REG(di->osh,
	         &di->d32rxregs->status) & RS_RS_MASK)) != RS_RS_DISABLED),
	         10000);

	return (status == RS_RS_DISABLED);
}

static bool
dma32_rxenabled(dma_info_t *di)
{
	uint32 rc;

	rc = R_REG(di->osh, &di->d32rxregs->control);
	return ((rc != 0xffffffff) && (rc & RC_RE));
}

static bool
dma32_txsuspendedidle(dma_info_t *di)
{
	if (di->ntxd == 0)
		return TRUE;

	if (!(R_REG(di->osh, &di->d32txregs->control) & XC_SE))
		return 0;

	if ((R_REG(di->osh, &di->d32txregs->status) & XS_XS_MASK) != XS_XS_IDLE)
		return 0;

	OSL_DELAY(2);
	return ((R_REG(di->osh, &di->d32txregs->status) & XS_XS_MASK) == XS_XS_IDLE);
}

/**
 * !! tx entry routine
 * supports full 32bit dma engine buffer addressing so
 * dma buffers can cross 4 Kbyte page boundaries.
 *
 * WARNING: call must check the return value for error.
 *   the error(toss frames) could be fatal and cause many subsequent hard to debug problems
 */
static int
dma32_txfast(dma_info_t *di, void *p0, bool commit)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint16 txout;
	uint32 flags = 0;
	dmaaddr_t pa;

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	txout = di->txout;

	/*
	 * Walk the chain of packet buffers
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		uint nsegs, j;
		hnddma_seg_map_t *map;

		data = PKTDATA(di->osh, p);
		len = PKTLEN(di->osh, p);
#ifdef BCM_DMAPAD
		if (DMAPADREQUIRED(di)) {
			len += PKTDMAPAD(di->osh, p);
		}
#endif
		next = PKTNEXT(di->osh, p);

		/* return nonzero if out of tx descriptors */
		if (NEXTTXD(txout) == di->txin)
			goto outoftxd;

		if (len == 0)
			continue;

		if (DMASGLIST_ENAB)
			bzero(&di->txp_dmah[txout], sizeof(hnddma_seg_map_t));

		/* get physical address of buffer start */
		pa = DMA_MAP(di->osh, data, len, DMA_TX, p, &di->txp_dmah[txout]);

		if (DMASGLIST_ENAB) {
			map = &di->txp_dmah[txout];

			/* See if all the segments can be accounted for */
			if (map->nsegs > (uint)(di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1))
				goto outoftxd;

			nsegs = map->nsegs;
		} else
			nsegs = 1;

		for (j = 1; j <= nsegs; j++) {
			flags = 0;
			if (p == p0 && j == 1)
				flags |= CTRL_SOF;

			/* With a DMA segment list, Descriptor table is filled
			 * using the segment list instead of looping over
			 * buffers in multi-chain DMA. Therefore, EOF for SGLIST is when
			 * end of segment list is reached.
			 */
			if ((!DMASGLIST_ENAB && next == NULL) ||
			    (DMASGLIST_ENAB && j == nsegs))
				flags |= (CTRL_IOC | CTRL_EOF);
			if (txout == (di->ntxd - 1))
				flags |= CTRL_EOT;

			if (DMASGLIST_ENAB) {
				len = map->segs[j - 1].length;
				pa = map->segs[j - 1].addr;
			}
			ASSERT(PHYSADDRHI(pa) == 0);

			dma32_dd_upd(di, di->txd32, pa, txout, &flags, len);
			ASSERT(di->txp[txout] == NULL);

			txout = NEXTTXD(txout);
		}

		/* See above. No need to loop over individual buffers */
		if (DMASGLIST_ENAB)
			break;
	}

	/* if last txd eof not set, fix it */
	if (!(flags & CTRL_EOF)) {
		W_SM(&di->txd32[PREVTXD(txout)].ctrl, BUS_SWAP32(flags | CTRL_IOC | CTRL_EOF));
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit)
		W_REG(di->osh, &di->d32txregs->ptr, I2B(txout, dma32dd_t));

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	di->hnddma.txnodesc++;
	return (-1);
}

/**
 * Reclaim next completed txd (txds if using chained buffers) in the range
 * specified and return associated packet.
 * If range is HNDDMA_RANGE_TRANSMITTED, reclaim descriptors that have be
 * transmitted as noted by the hardware "CurrDescr" pointer.
 * If range is HNDDMA_RANGE_TRANSFERED, reclaim descriptors that have be
 * transfered by the DMA as noted by the hardware "ActiveDescr" pointer.
 * If range is HNDDMA_RANGE_ALL, reclaim all txd(s) posted to the ring and
 * return associated packet regardless of the value of hardware pointers.
 */
static void *
dma32_getnexttxp(dma_info_t *di, txd_range_t range)
{
	uint16 start, end, i;
	uint16 active_desc;
	void *txp;

	DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name,
	           (range == HNDDMA_RANGE_ALL) ? "all" :
	           ((range == HNDDMA_RANGE_TRANSMITTED) ? "transmitted" : "transfered")));

	if (di->ntxd == 0)
		return (NULL);

	txp = NULL;

	start = di->txin;
	if (range == HNDDMA_RANGE_ALL)
		end = di->txout;
	else {
		dma32regs_t *dregs = di->d32txregs;

		if (di->txin == di->xs0cd) {
		end = (uint16)B2I(R_REG(di->osh, &dregs->status) & XS_CD_MASK, dma32dd_t);
			di->xs0cd = end;
		} else
			end = di->xs0cd;

		if (range == HNDDMA_RANGE_TRANSFERED) {
			active_desc = (uint16)((R_REG(di->osh, &dregs->status) & XS_AD_MASK) >>
			                       XS_AD_SHIFT);
			active_desc = (uint16)B2I(active_desc, dma32dd_t);
			if (end != active_desc)
				end = PREVTXD(active_desc);
		}
	}

	if ((start == 0) && (end > di->txout))
		goto bogus;

	for (i = start; i != end && !txp; i = NEXTTXD(i)) {
		dmaaddr_t pa;
		hnddma_seg_map_t *map = NULL;
		uint size, j, nsegs;

		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->txd32[i].addr)) - di->dataoffsetlow));
		PHYSADDRHISET(pa, 0);

		if (DMASGLIST_ENAB) {
			map = &di->txp_dmah[i];
			size = map->origsize;
			nsegs = map->nsegs;
		} else {
			size = (BUS_SWAP32(R_SM(&di->txd32[i].ctrl)) & CTRL_BC_MASK);
			nsegs = 1;
		}

		for (j = nsegs; j > 0; j--) {
#if defined(DESCR_DEADBEEF)
			W_SM(&di->txd32[i].addr, 0xdeadbeef);
#endif

			txp = di->txp[i];
			di->txp[i] = NULL;
			if (j > 1)
				i = NEXTTXD(i);
		}

		DMA_UNMAP(di->osh, pa, size, DMA_TX, txp, map);
	}

	di->txin = i;

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (txp);

bogus:
	DMA_NONE(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
	          start, end, di->txout, forceall));
	return (NULL);
}

static void *
dma32_getnextrxp(dma_info_t *di, bool forceall)
{
	uint16 i, curr;
	void *rxp;
	dmaaddr_t pa;
	/* if forcing, dma engine must be disabled */
	ASSERT(!forceall || !dma32_rxenabled(di));

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	if (di->rxin == di->rs0cd) {
		curr = (uint16)B2I(R_REG(di->osh, &di->d32rxregs->status) & RS_CD_MASK, dma32dd_t);
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	/* ignore curr if forceall */
	if (!forceall && (i == curr))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);
	di->rxp[i] = NULL;

	PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd32[i].addr)) - di->dataoffsetlow));
	PHYSADDRHISET(pa, 0);

	/* clear this packet from the descriptor ring */
	DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, rxp, &di->rxp_dmah[i]);
#if defined(DESCR_DEADBEEF)
	W_SM(&di->rxd32[i].addr, 0xdeadbeef);
#endif

	di->rxin = NEXTRXD(i);

	return (rxp);
}

/**
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
static void
dma32_txrotate(dma_info_t *di)
{
	uint16 ad;
	uint nactive;
	uint rot;
	uint16 old, new;
	uint32 w;
	uint16 first, last;

	ASSERT(dma32_txsuspendedidle(di));

	nactive = _dma_txactive(di);
	ad = B2I(((R_REG(di->osh, &di->d32txregs->status) & XS_AD_MASK) >> XS_AD_SHIFT), dma32dd_t);
	rot = TXD(ad - di->txin);

	ASSERT(rot < di->ntxd);

	/* full-ring case is a lot harder - don't worry about this */
	if (rot >= (di->ntxd - nactive)) {
		DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
		return;
	}

	first = di->txin;
	last = PREVTXD(di->txout);

	/* move entries starting at last and moving backwards to first */
	for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
		new = TXD(old + rot);

		/*
		 * Move the tx dma descriptor.
		 * EOT is set only in the last entry in the ring.
		 */
		w = BUS_SWAP32(R_SM(&di->txd32[old].ctrl)) & ~CTRL_EOT;
		if (new == (di->ntxd - 1))
			w |= CTRL_EOT;
		W_SM(&di->txd32[new].ctrl, BUS_SWAP32(w));
		W_SM(&di->txd32[new].addr, R_SM(&di->txd32[old].addr));

#if defined(DESCR_DEADBEEF)
		/* zap the old tx dma descriptor address field */
		W_SM(&di->txd32[old].addr, BUS_SWAP32(0xdeadbeef));
#endif /* DESCR_DEADBEEF */

		/* move the corresponding txp[] entry */
		ASSERT(di->txp[new] == NULL);
		di->txp[new] = di->txp[old];

		/* Move the segment map as well */
		if (DMASGLIST_ENAB) {
			bcopy(&di->txp_dmah[old], &di->txp_dmah[new], sizeof(hnddma_seg_map_t));
			bzero(&di->txp_dmah[old], sizeof(hnddma_seg_map_t));
		}

		di->txp[old] = NULL;
	}

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	/* kick the chip */
	W_REG(di->osh, &di->d32txregs->ptr, I2B(di->txout, dma32dd_t));
}

/* 64-bit DMA functions */

static void
dma64_txinit(dma_info_t *di)
{
	uint32 control;
	uint32 addrlow, act;
	int i;
	si_t *sih;
	sih = di->sih;

	DMA_TRACE(("%s: dma_txinit\n", di->name));

	if (di->ntxd == 0)
		return;
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	di->txin = di->txout = di->xs0cd = di->xs0cd_snapshot = 0;
	di->hnddma.txavail = di->ntxd - 1;

	/* clear tx descriptor ring */
	BZERO_SM((void *)(uintptr)di->txd64, (di->ntxd * sizeof(dma64dd_t)));

	/* These bits 20:18 (burstLen) of control register can be written but will take
	 * effect only if these bits are valid. So this will not affect previous versions
	 * of the DMA. They will continue to have those bits set to 0.
	 */
	control = R_REG(di->osh, &di->d64txregs->control);
	control = (control & ~D64_XC_BL_MASK) | (di->txburstlen << D64_XC_BL_SHIFT);
	control = (control & ~D64_XC_MR_MASK) | (di->txmultioutstdrd << D64_XC_MR_SHIFT);
	control = (control & ~D64_XC_PC_MASK) | (di->txprefetchctl << D64_XC_PC_SHIFT);
	control = (control & ~D64_XC_PT_MASK) | (di->txprefetchthresh << D64_XC_PT_SHIFT);
	if (DMA_TRANSCOHERENT(di))
		control = (control & ~D64_XC_CO_MASK) | (1 << D64_XC_CO_SHIFT);
	W_REG(di->osh, &di->d64txregs->control, control);

	control = D64_XC_XE;
	/* DMA engine with out alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (!di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);
#ifdef BCMM2MDEV_ENABLED
	addrlow = R_REG(di->osh, &di->d64txregs->addrlow);
	addrlow &= 0xffff;
	W_REG(di->osh, &di->d64txregs->ptr, addrlow);
#endif

	if (DMA_INDIRECT(di) &&
		(di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG) &&
		((si_coreid(sih) == D11_CORE_ID) &&
		(si_corerev(sih) == 64 || si_corerev(sih) == 65))) {
		addrlow = (uint)(R_REG(di->osh, &di->d64txregs->addrlow) & 0xffff);
		if (addrlow != 0)
			W_REG(di->osh, &di->d64txregs->ptr, addrlow);
		for (i = 0; i < 20; i++) {
			act = (uint)(R_REG(di->osh, &di->d64txregs->status1) & 0xffff);
			if (addrlow == act) {
				break;
			}
			OSL_DELAY(1);
		}
		if (addrlow != act) {
			DMA_ERROR(("%s %s: dma txdesc AD %#x != addrlow %#x\n", di->name,
				__FUNCTION__, act, addrlow));
		}
		ASSERT(addrlow == act);
	}

	if (di->hnddma.dmactrlflags & DMA_CTRL_CS)
		control |= D64_XC_CS;

	if ((di->hnddma.dmactrlflags & DMA_CTRL_PEN) == 0)
		control |= D64_XC_PD;
	OR_REG(di->osh, &di->d64txregs->control, control);

	/* DMA engine with alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);

#ifdef BCMM2MDEV_ENABLED
	addrlow = R_REG(di->osh, &di->d64txregs->addrlow);
	addrlow &= 0xffff;
	W_REG(di->osh, &di->d64txregs->ptr, addrlow);
#endif

}

static bool
dma64_txenabled(dma_info_t *di)
{
	uint32 xc;
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	/* If the chip is dead, it is not enabled :-) */
	xc = R_REG(di->osh, &di->d64txregs->control);
	return ((xc != 0xffffffff) && (xc & D64_XC_XE));
}

static void
dma64_txsuspend(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txsuspend\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common register, SuspReq */
	if (DMA_INDIRECT(di)) {
		OR_REG(di->osh, di->dmacommon->suspreq, (1 << di->q_index));
	}
	else
#endif
	{
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
	}
}

static void
dma64_txresume(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txresume\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common register, SuspReq */
	if (DMA_INDIRECT(di)) {
		AND_REG(di->osh, di->dmacommon->suspreq, ~(1 << di->q_index));
	}
	else
#endif
	{
		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_SE);
	}
}

static bool
dma64_txsuspended(dma_info_t *di)
{
	if (di->ntxd == 0)
		return TRUE;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common register, SuspReq */
	if (DMA_INDIRECT(di)) {
		return ((R_REG(di->osh, di->dmacommon->suspreq) & (1 << di->q_index)) != 0);
	}
	else
#endif
	{
		return ((R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE) == D64_XC_SE);
	}
}

#ifdef WL_MULTIQUEUE
static void
dma64_txflush(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txflush\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common register, FlushReq */
	if (DMA_INDIRECT(di)) {
		OR_REG(di->osh, di->dmacommon->flushreq, (1 << di->q_index));
	}
	else
#endif
	{
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE | D64_XC_FL);
	}
}

static void
dma64_txflush_clear(dma_info_t *di)
{
	uint32 status;

	DMA_TRACE(("%s: dma_txflush_clear\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common register, FlushReq */
	if (DMA_INDIRECT(di)) {
		AND_REG(di->osh, di->dmacommon->flushreq, ~(1 << di->q_index));
	}
	else
#endif
	{
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED) &&
		         (status != D64_XS0_XS_IDLE) &&
		         (status != D64_XS0_XS_STOPPED),
		         10000);
		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_FL);
	}
}

void
dma_txrewind(hnddma_t *dmah)
{
	uint16 start, end, i;
	uint act;
	uint32 flags;
	dma64dd_t *ring;

	dma_info_t * di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma64_txrewind\n", di->name));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	ASSERT(dma64_txsuspended(di));

	/* select to read index of already fetched desc */
	AND_REG(di->osh, &di->d64txregs->control, ~D64_RC_SA);

	act = (uint)(R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK);
	act = (act - di->xmtptrbase) & D64_XS0_CD_MASK;
	start = (uint16)B2I(act, dma64dd_t);

	end = di->txout;

	ring = di->txd64;
	for (i = start; i != end; i = NEXTTXD(i)) {
		/* find the first one having eof set */
		flags = R_SM(&ring[i].ctrl1);
		if (flags & CTRL_EOF) {
			/* rewind end to (i+1) */
			W_REG(di->osh,
			      &di->d64txregs->ptr, di->xmtptrbase + I2B(NEXTTXD(i), dma64dd_t));
			DMA_TRACE(("ActiveIdx %d EndIdx was %d now %d\n", start, end, NEXTTXD(i)));
			break;
		}
	}
}
#endif /* WL_MULTIQUEUE */

static void BCMFASTPATH
dma64_txreclaim(dma_info_t *di, txd_range_t range)
{
	void *p;

	DMA_TRACE(("%s: dma_txreclaim %s\n", di->name,
	           (range == HNDDMA_RANGE_ALL) ? "all" :
	           ((range == HNDDMA_RANGE_TRANSMITTED) ? "transmitted" : "transfered")));

	if (di->txin == di->txout)
		return;

	/* if this is decriptor only DMA then just reset the txin. No data packets to free */
	if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG) {
		DMA_TRACE(("%s: DESC only DMA. Seting txin=txout=%d \n", di->name, di->txout));
		di->txin = di->txout;
		di->hnddma.txavail = di->ntxd - 1;
		return;
	}

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	while ((p = dma64_getnexttxp(di, range))) {
		/* For unframed data, we don't have any packets to free */
		if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED))
			PKTFREE(di->osh, p, TRUE);
	}
}

static bool
dma64_txstopped(dma_info_t *di)
{
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	return ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED);
}

static bool
dma64_rxstopped(dma_info_t *di)
{
	return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK) == D64_RS0_RS_STOPPED);
}

static bool
BCMATTACHFN_DMA_ATTACH(dma64_alloc)(dma_info_t *di, uint direction)
{
	uint32 size;
	uint ddlen;
	void *va;
	uint alloced = 0;
	uint32 align;
	uint16 align_bits;

	ddlen = sizeof(dma64dd_t);

	size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);

	if (direction == DMA_TX) {
		if ((va = dma_ringalloc(di->osh,
			(di->d64_xs0_cd_mask == 0x1fff) ? D64RINGBOUNDARY : D64RINGBOUNDARY_LARGE,
			size, &align_bits, &alloced,
			&di->txdpaorig, &di->tx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n",
			           di->name));
			return FALSE;
		}
		align = (1 << align_bits);

		/* adjust the pa by rounding up to the alignment */
		PHYSADDRLOSET(di->txdpa, ROUNDUP(PHYSADDRLO(di->txdpaorig), align));
		PHYSADDRHISET(di->txdpa, PHYSADDRHI(di->txdpaorig));

		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->txdpa) >= PHYSADDRLO(di->txdpaorig));

		/* find the alignment offset that was used */
		di->txdalign = (uint)(PHYSADDRLO(di->txdpa) - PHYSADDRLO(di->txdpaorig));

		/* adjust the va by the same offset */
		di->txd64 = (dma64dd_t *)((uintptr)va + di->txdalign);

		di->txdalloc = alloced;
		ASSERT(ISALIGNED(PHYSADDRLO(di->txdpa), align));
	} else {
		if ((va = dma_ringalloc(di->osh,
			(di->d64_rs0_cd_mask == 0x1fff) ? D64RINGBOUNDARY : D64RINGBOUNDARY_LARGE,
			size, &align_bits, &alloced,
			&di->rxdpaorig, &di->rx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n",
			           di->name));
			return FALSE;
		}
		align = (1 << align_bits);

		/* adjust the pa by rounding up to the alignment */
		PHYSADDRLOSET(di->rxdpa, ROUNDUP(PHYSADDRLO(di->rxdpaorig), align));
		PHYSADDRHISET(di->rxdpa, PHYSADDRHI(di->rxdpaorig));

		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->rxdpa) >= PHYSADDRLO(di->rxdpaorig));

		/* find the alignment offset that was used */
		di->rxdalign = (uint)(PHYSADDRLO(di->rxdpa) - PHYSADDRLO(di->rxdpaorig));

		/* adjust the va by the same offset */
		di->rxd64 = (dma64dd_t *)((uintptr)va + di->rxdalign);

		di->rxdalloc = alloced;
		ASSERT(ISALIGNED(PHYSADDRLO(di->rxdpa), align));
	}

	return TRUE;
}

static bool
dma64_txreset(dma_info_t *di)
{
	uint32 status = D64_XS0_XS_DISABLED;

	if (di->ntxd == 0)
		return TRUE;

	/* if using indirect DMA access, then configure IndQSel */
	/* If the DMA core has resetted, then the default IndQSel = 0 */
	/* So force the configuration */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, TRUE);
	}

	if (!DMA_INDIRECT(di)) {
		/* If DMA is already in reset, do not reset. */
		if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) ==
			D64_XS0_XS_DISABLED)
			return TRUE;

		/* suspend tx DMA first */
		W_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED) &&
		         (status != D64_XS0_XS_IDLE) &&
		         (status != D64_XS0_XS_STOPPED),
		         10000);
	}

	/* For IndDMA, the channel status is ignored. */
	W_REG(di->osh, &di->d64txregs->control, 0);

	if (!DMA_INDIRECT(di) || (di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)) {
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED),
		         10000);

		/* We should be disabled at this point */
		if (status != D64_XS0_XS_DISABLED) {
			DMA_ERROR(("%s: status != D64_XS0_XS_DISABLED 0x%x\n",
					__FUNCTION__, status));
			ASSERT(status == D64_XS0_XS_DISABLED);
			OSL_DELAY(300);
		}
	}

	return (status == D64_XS0_XS_DISABLED);
}

static bool
dma64_rxidle(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxidle\n", di->name));

	if (di->nrxd == 0)
		return TRUE;

	return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) ==
		(R_REG(di->osh, &di->d64rxregs->ptr) & D64_RS0_CD_MASK));
}

static bool
dma64_rxreset(dma_info_t *di)
{
	uint32 status;

	if (di->nrxd == 0)
		return TRUE;

	W_REG(di->osh, &di->d64rxregs->control, 0);
	SPINWAIT(((status = (R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK)) !=
	          D64_RS0_RS_DISABLED), 10000);

	return (status == D64_RS0_RS_DISABLED);
}

static bool
dma64_rxenabled(dma_info_t *di)
{
	uint32 rc;

	rc = R_REG(di->osh, &di->d64rxregs->control);
	return ((rc != 0xffffffff) && (rc & D64_RC_RE));
}

static bool
dma64_txsuspendedidle(dma_info_t *di)
{

	if (di->ntxd == 0)
		return TRUE;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	if (!(R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE))
		return 0;

	if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_IDLE)
		return 1;

	return 0;
}

/**
 * Useful when sending unframed data.  This allows us to get a progress report from the DMA.
 * We return a pointer to the beginning of the data buffer of the current descriptor.
 * If DMA is idle, we return NULL.
 */
/* Might be nice if DMA HW could tell us current position rather than current descriptor */
static void*
dma64_getpos(dma_info_t *di, bool direction)
{
	void *va;
	bool idle;
	uint16 cur_idx;

	if (direction == DMA_TX) {
		/* if using indirect DMA access, then configure IndQSel */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}

		cur_idx = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
		               di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
		idle = !NTXDACTIVE(di->txin, di->txout);
		va = di->txp[cur_idx];
	} else {
		cur_idx = B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		               di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
		idle = !NRXDACTIVE(di->rxin, di->rxout);
		va = di->rxp[cur_idx];
	}

	/* If DMA is IDLE, return NULL */
	if (idle) {
		DMA_TRACE(("%s: DMA idle, return NULL\n", __FUNCTION__));
		va = NULL;
	}

	return va;
}

/**
 * TX of unframed data
 *
 * Adds a DMA ring descriptor for the data pointed to by "buf".
 * This is for DMA of a buffer of data and is unlike other hnddma TX functions
 * that take a pointer to a "packet"
 * Each call to this is results in a single descriptor being added for "len" bytes of
 * data starting at "buf", it doesn't handle chained buffers.
 */
static int
dma64_txunframed(dma_info_t *di, void *buf, uint len, bool commit)
{
	uint16 txout;
	uint32 flags = 0;
	dmaaddr_t pa; /* phys addr */

	txout = di->txout;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	if (len == 0)
		return 0;

	pa = DMA_MAP(di->osh, buf, len, DMA_TX, NULL, &di->txp_dmah[txout]);

	flags = (D64_CTRL1_SOF | D64_CTRL1_IOC | D64_CTRL1_EOF);

	if (txout == (di->ntxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);
	ASSERT(di->txp[txout] == NULL);

#if defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh,  dma64_txd64(di, di->txout), DMA64_FLUSH_LEN(1),
		DMA_TX, NULL, NULL);
#endif /* BULK_DESCR_FLUSH */

	/* save the buffer pointer - used by dma_getpos */
	di->txp[txout] = buf;

	txout = NEXTTXD(txout);
	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit) {
		W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(txout, dma64dd_t));
	}

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: %s: out of txds !!!\n", di->name, __FUNCTION__));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}


/**
 * RX of unframed data
 *
 * Adds a DMA ring descriptor for the data pointed to by "buf".
 * This is for DMA of a buffer of data and is unlike other hnddma RX functions
 * that take a pointer to a "packet"
 * Each call to this is result in a single descriptor being added for "len" bytes of
 * data starting at "buf", it doesn't handle chained buffers.
 */
int
dma_rxfill_unframed(hnddma_t *dmah, void *buf, uint len, bool commit)
{
	uint16 rxout;
	uint32 flags = 0;
	dmaaddr_t pa; /* phys addr */

	dma_info_t *di = DI_INFO(dmah);

	rxout = di->rxout;

	/* return nonzero if out of rx descriptors */
	if (NEXTRXD(rxout) == di->rxin)
		goto outofrxd;

	ASSERT(len <= di->rxbufsize);

	/* cache invalidate maximum buffer length */
	pa = DMA_MAP(di->osh, buf, di->rxbufsize, DMA_RX, NULL, NULL);

	if (rxout == (di->nrxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, len);
	ASSERT(di->rxp[rxout] == NULL);

#if defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(1),
		DMA_TX, NULL, NULL);
#endif /* BULK_DESCR_FLUSH */

	/* save the buffer pointer - used by dma_getpos */
	di->rxp[rxout] = buf;

	rxout = NEXTRXD(rxout);

	/* kick the chip */
	if (commit) {
		W_REG(di->osh, &di->d64rxregs->ptr, di->rcvptrbase + I2B(rxout, dma64dd_t));
	}

	/* bump the rx descriptor index */
	di->rxout = rxout;

	/* rx flow control */
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

	return (0);

outofrxd:
	DMA_ERROR(("%s: %s: out of rxds !!!\n", di->name, __FUNCTION__));
	di->rxavail = 0;
	di->hnddma.rxnobuf++;
	return (-1);
}


/**
 * !! tx entry routine
 * WARNING: call must check the return value for error.
 *   the error(toss frames) could be fatal and cause many subsequent hard to debug problems
 */
static int BCMFASTPATH
dma64_txfast(dma_info_t *di, void *p0, bool commit)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint16 txout;
	uint32 flags = 0;
	dmaaddr_t pa;
	bool war;

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	/* new DMA routine for LFRAGS */
	if (BCMLFRAG_ENAB()) {
		if (PKTISTXFRAG(di->osh, p0))
			return dma64_txfast_lfrag(di, p0, commit);
#ifdef PCIE_PHANTOM_DEV
		/* changes to accomodate segment split up is also handled here */
		if (di->blwar_d11core)
			return dma64_txfast_lfrag(di, p0, commit);
#endif /* PCIE_PHANTOM_DEV */
	}

	txout = di->txout;
	war = (di->hnddma.dmactrlflags & DMA_CTRL_DMA_AVOIDANCE_WAR) ? TRUE : FALSE;

	/*
	 * Walk the chain of packet buffers
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		uint nsegs, j, segsadd;
		hnddma_seg_map_t *map = NULL;

		data = PKTDATA(di->osh, p);
		len = PKTLEN(di->osh, p);
#ifdef BCM_DMAPAD
		if (DMAPADREQUIRED(di)) {
			len += PKTDMAPAD(di->osh, p);
		}
#endif /* BCM_DMAPAD */
		next = PKTNEXT(di->osh, p);

		/* return nonzero if out of tx descriptors */
		if (NEXTTXD(txout) == di->txin)
			goto outoftxd;

		if (len == 0)
			continue;

		/* get physical address of buffer start */
		if (DMASGLIST_ENAB)
			bzero(&di->txp_dmah[txout], sizeof(hnddma_seg_map_t));

		pa = DMA_MAP(di->osh, data, len, DMA_TX, p, &di->txp_dmah[txout]);

		if (DMASGLIST_ENAB) {
			map = &di->txp_dmah[txout];

			/* See if all the segments can be accounted for */
			if (map->nsegs > (uint)(di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1))
				goto outoftxd;

			nsegs = map->nsegs;
		} else
			nsegs = 1;

		segsadd = 0;
		for (j = 1; j <= nsegs; j++) {
			flags = 0;
			if (p == p0 && j == 1)
				flags |= D64_CTRL1_SOF;

			/* With a DMA segment list, Descriptor table is filled
			 * using the segment list instead of looping over
			 * buffers in multi-chain DMA. Therefore, EOF for SGLIST is when
			 * end of segment list is reached.
			 */
			if ((!DMASGLIST_ENAB && next == NULL) ||
			    (DMASGLIST_ENAB && j == nsegs)) {
				/* Set "interrupt on completion" bit only on last commit packet
				 * to reduce the Tx completion event
				 */
				flags |= D64_CTRL1_EOF;
				if (commit)
					flags |= D64_CTRL1_IOC;
			}

			if (txout == (di->ntxd - 1))
				flags |= D64_CTRL1_EOT;

			if (di->burstsize_ctrl)
				flags |= D64_CTRL1_NOTPCIE;

			/* for lbufs of type lbuf_basic which come from local
			 * memory.
			 */
			if (DMA_TRANSCOHERENT(di))
				flags |= D64_CTRL1_COHERENT;

			if (DMASGLIST_ENAB) {
				len = map->segs[j - 1].length;
				pa = map->segs[j - 1].addr;
				if (len > 128 && war) {
					uint remain, new_len, align64;
					/* check for 64B aligned of pa */
					align64 = (uint)(PHYSADDRLO(pa) & 0x3f);
					align64 = (64 - align64) & 0x3f;
					new_len = len - align64;
					remain = new_len % 128;
					if (remain > 0 && remain <= 4) {
						uint32 buf_addr_lo;
						uint32 tmp_flags =
							flags & (~(D64_CTRL1_EOF | D64_CTRL1_IOC));
						flags &= ~(D64_CTRL1_SOF | D64_CTRL1_EOT);
						remain += 64;
						dma64_dd_upd(di, di->txd64, pa, txout,
							&tmp_flags, len-remain);
						ASSERT(di->txp[txout] == NULL);
						txout = NEXTTXD(txout);
						/* return nonzero if out of tx descriptors */
						if (txout == di->txin) {
							DMA_ERROR(("%s: dma_txfast: Out-of-DMA"
								" descriptors (txin %d txout %d"
								" nsegs %d)\n", __FUNCTION__,
								di->txin, di->txout, nsegs));
							goto outoftxd;
						}
						if (txout == (di->ntxd - 1))
							flags |= D64_CTRL1_EOT;
						buf_addr_lo = PHYSADDRLO(pa);
						PHYSADDRLOSET(pa, (PHYSADDRLO(pa) + (len-remain)));
						if (PHYSADDRLO(pa) < buf_addr_lo) {
							PHYSADDRHISET(pa, (PHYSADDRHI(pa) + 1));
						}
						len = remain;
						segsadd++;
						di->dma_avoidance_cnt++;
					}
				}
			}
			dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);
			ASSERT(di->txp[txout] == NULL);

			txout = NEXTTXD(txout);
			/* return nonzero if out of tx descriptors */
			if (txout == di->txin) {
				DMA_ERROR(("%s: dma_txfast: Out-of-DMA descriptors"
					   " (txin %d txout %d nsegs %d)\n", __FUNCTION__,
					   di->txin, di->txout, nsegs));
				goto outoftxd;
			}
		}
		if (segsadd && DMASGLIST_ENAB)
			map->nsegs += segsadd;

		/* See above. No need to loop over individual buffers */
		if (DMASGLIST_ENAB)
			break;
	}

	/* if last txd eof not set, fix it */
	if (!(flags & D64_CTRL1_EOF)) {
		W_SM(&di->txd64[PREVTXD(txout)].ctrl1,
		     BUS_SWAP32(flags | D64_CTRL1_IOC | D64_CTRL1_EOF));
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = p0;

#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NTXDACTIVE(di->txout, txout);
		if (txout < di->txout) {
			DMA_MAP(di->osh, dma64_txd64(di, 0), DMA64_FLUSH_LEN(txout),
			        DMA_TX, NULL, NULL);
			flush_cnt -= txout;
		}
		DMA_MAP(di->osh, dma64_txd64(di, di->txout), DMA64_FLUSH_LEN(flush_cnt),
		        DMA_TX, NULL, NULL);
	}
#endif  /* BULK_DESCR_FLUSH */

	/* bump the tx descriptor index */
	di->txout = txout;

#if defined(__ARM_ARCH_7A__) && defined(CA7)
	/* memory barrier before posting the descriptor */
	DMB();
#endif

	/* kick the chip */
	if (commit)
		W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(txout, dma64dd_t));

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

int BCMFASTPATH
dma_commit(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(di->txout, dma64dd_t));
	return (0);
}

#if defined(BCM_DMA_CT)
void
aqm_dma_init_fifo_stats(hnddma_t *dmah, int fifo)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmac = di->dmacommon;
	aqm_stats_t *aqm_stats = dmac->aqm_stats;
	uint16 ad;

	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	ad = B2I((((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK) -
			di->xmtptrbase) & D64_XS1_AD_MASK), dma64dd_t);
	aqm_stats->aqm_fifo_stats[fifo].next_ld = ad;
}

int BCMFASTPATH
aqm_dma_get_current_fifo(dma_common_t *dmac)
{
	aqm_stats_t *aqm_stats = dmac->aqm_stats;
	return aqm_stats->fifo;
}

bool BCMFASTPATH
aqm_dma_sched_fifo(hnddma_t *dmah, uint fifo)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmac = di->dmacommon;
	aqm_stats_t *aqm_stats = dmac->aqm_stats;
	uint16 next_ld;

	next_ld = aqm_stats->aqm_fifo_stats[fifo].next_ld;

	if (NTXDACTIVE(next_ld, di->txout)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool BCMFASTPATH
aqm_dma_prefetch_active(hnddma_t *dmah, uint fifo)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmac = di->dmacommon;
	aqm_stats_t *aqm_stats = dmac->aqm_stats;
	uint16 ad, next_ld;

	next_ld = aqm_stats->aqm_fifo_stats[fifo].next_ld;

	/* if this DMA channel is using indirect access, then configure the
	 * IndQSel register for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}
	ad = B2I((((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK) -
			di->xmtptrbase) & D64_XS1_AD_MASK), dma64dd_t);
	if (ad != next_ld) {
		return TRUE;
	}
	return FALSE;
}

#define AQM_MAX_PKTS 64
void BCMFASTPATH
aqm_dma_commit(hnddma_t *dmah, uint fifo)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmac = di->dmacommon;
	aqm_stats_t *aqm_stats = dmac->aqm_stats;
	uint16 ad, next_ld;

	/* if this DMA channel is using indirect access, then configure the
	 * IndQSel register for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	ad = B2I((((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK) -
			di->xmtptrbase) & D64_XS1_AD_MASK), dma64dd_t);

	/* Get the number of descriptors between current ad and txout */
	if ((NTXDACTIVE(ad, di->txout)) > AQM_MAX_PKTS) {
		next_ld = XXD(ad + AQM_MAX_PKTS, di->ntxd);

	} else {
		next_ld = di->txout;
	}

	/* Update lastptr */
	W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(next_ld, dma64dd_t));
	aqm_stats->aqm_fifo_stats[fifo].next_ld = next_ld;
	aqm_stats->fifo = fifo;
}
#endif /* BCM_DMA_CT */

/* Routine to post a descriptor. It requires the caller to send in partially formatted
 * information for the descriptor.
 */
int BCMFASTPATH
dma_txdesc(hnddma_t *dmah, dma64dd_t *dd, bool commit)
{
	uint16 txout;
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txdesc\n", di->name));
	txout = di->txout;

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	/* fill in remaining bits in ctrl1 */
	if (di->burstsize_ctrl)
		dd->ctrl1 |= D64_CTRL1_NOTPCIE;
	/* dongle aqm desc need to have CO as the SOFD buffer comes from local
	 * memory.
	 */
	if (DMA_TRANSCOHERENT(di))
		dd->ctrl1 |= D64_CTRL1_COHERENT;
	if (txout == (di->ntxd - 1))
		dd->ctrl1 |= D64_CTRL1_EOT;

	DMA_TRACE(("%s: dma_txdesc Descriptor index = %d, dd->ctrl1 = 0x%x, dd->ctrl2 = 0x%x,"
		"dd->addrlow = 0x%x, dd->addrhigh = 0x%x \n", di->name, txout, dd->ctrl1,
		dd->ctrl2, dd->addrlow, dd->addrhigh));

	/* load the descriptor */
	dma64_dd_upd_64_from_struct(di, di->txd64, dd, txout);

	txout = NEXTTXD(txout);
	/* bump the descriptor index */
	di->txout = txout;

	/* If commit is set, write the DMA register to inform the DMA of the new descriptor */
	if (commit) {
		W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(txout, dma64dd_t));
	}

	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txdesc: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

/**
 * Reclaim next completed txd (txds if using chained buffers) in the range
 * specified and return associated packet.
 * If range is HNDDMA_RANGE_TRANSMITTED, reclaim descriptors that have be
 * transmitted as noted by the hardware "CurrDescr" pointer.
 * If range is HNDDMA_RANGE_TRANSFERED, reclaim descriptors that have be
 * transfered by the DMA as noted by the hardware "ActiveDescr" pointer.
 * If range is HNDDMA_RANGE_ALL, reclaim all txd(s) posted to the ring and
 * return associated packet regardless of the value of hardware pointers.
 */
static void * BCMFASTPATH
dma64_getnexttxp(dma_info_t *di, txd_range_t range)
{
	uint16 start, end, i;
	uint16 active_desc;
	void *txp;

	DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name,
	           (range == HNDDMA_RANGE_ALL) ? "all" :
	           ((range == HNDDMA_RANGE_TRANSMITTED) ? "transmitted" : "transfered")));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	if (di->ntxd == 0)
		return (NULL);

	txp = NULL;

	start = di->txin;
	if (range == HNDDMA_RANGE_ALL)
		end = di->txout;
	else {
		dma64regs_t *dregs = di->d64txregs;

		if (di->txin == di->xs0cd) {
			end = (uint16)(B2I(((R_REG(di->osh, &dregs->status0) & D64_XS0_CD_MASK) -
			      di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t));
			di->xs0cd = end;
		} else
			end = di->xs0cd;

		if (range == HNDDMA_RANGE_TRANSFERED) {
			active_desc = (uint16)(R_REG(di->osh, &dregs->status1) & D64_XS1_AD_MASK);
			active_desc = (active_desc - di->xmtptrbase) & D64_XS1_AD_MASK;
			active_desc = B2I(active_desc, dma64dd_t);
			if (end != active_desc)
				end = PREVTXD(active_desc);
		}
	}

	if ((start == 0) && (end > di->txout))
		goto bogus;

	for (i = start; i != end && !txp; i = NEXTTXD(i)) {
		/* dma 64-bit */
		hnddma_seg_map_t *map = NULL;
		uint size, j, nsegs;

#if ((!defined(__mips__) && !defined(BCM47XX_CA9)) || defined(__NetBSD__))
		dmaaddr_t pa;
		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->txd64[i].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->txd64[i].addrhigh)) - di->dataoffsethigh));
#endif

		if (DMASGLIST_ENAB) {
			map = &di->txp_dmah[i];
			size = map->origsize;
			nsegs = map->nsegs;
			if (nsegs > (uint)NTXDACTIVE(i, end)) {
				di->xs0cd = i;
				break;
			}
		} else {
#if ((!defined(__mips__) && !defined(BCM47XX_CA9)) || defined(__NetBSD__))
			size = (BUS_SWAP32(R_SM(&di->txd64[i].ctrl2)) & D64_CTRL2_BC_MASK);
#endif
			nsegs = 1;
		}

#ifdef PCIE_PHANTOM_DEV
		if (di->blwar_d11core) {
			size = di->blwar_size[i];
			nsegs = di->blwar_nsegs[i];
		}
#endif /* PCIE_PHANTOM_DEV */

		for (j = nsegs; j > 0; j--) {
#if defined(DESCR_DEADBEEF)
			W_SM(&di->txd64[i].addrlow, 0xdeadbeef);
			W_SM(&di->txd64[i].addrhigh, 0xdeadbeef);
#endif

			txp = di->txp[i];
			di->txp[i] = NULL;
			if (j > 1)
				i = NEXTTXD(i);
		}

#if ((!defined(__mips__) && !defined(BCM47XX_CA9)) || defined(__NetBSD__))
		DMA_UNMAP(di->osh, pa, size, DMA_TX, txp, map);
#endif
	}

	di->txin = i;

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (txp);

bogus:
	DMA_NONE(("dma_getnexttxp: bogus curr: start %d end %d txout %d range %d\n",
	          start, end, di->txout, range));
	return (NULL);
}

/* Function to reclaim the next completed descriptors for DESCRIPTOR only DMA */
int BCMFASTPATH
dma_getnexttxdd(hnddma_t *dmah, txd_range_t range, uint32 *flags)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16 end = 0;
	uint16 prev_txin = di->txin;

	DMA_TRACE(("  %s: dma_getnexttxdd %s\n", di->name,
	           (range == HNDDMA_RANGE_ALL) ? "all" :
	           ((range == HNDDMA_RANGE_TRANSMITTED) ? "transmitted" : "transfered")));

	/* The check below can be removed when/if new chips implement a DMA that does support
	 * use of the AD for Descriptor only DMAs, and that implementation to support the AD
	 * checking needs to be added below where the range is being set.
	 */
	if (range == HNDDMA_RANGE_TRANSFERED) {
		DMA_ERROR(("%s: dma_getnexttxdd: HNDDMA_RANGE_TRANSFERED is not valid range \n",
			di->name));
		ASSERT(range != HNDDMA_RANGE_TRANSFERED);
		return BCME_RANGE;
	}

	if (di->ntxd == 0) {
		DMA_ERROR(("%s: dma_getnexttxdd ntxd=0 \n", di->name));
		return BCME_ERROR;
	}

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	if (range == HNDDMA_RANGE_ALL)
		end = di->txout;
	else {
		dma64regs_t *dregs = di->d64txregs;
		if (di->txin == di->xs0cd) {
			end = (uint16)(B2I(((R_REG(di->osh, &dregs->status0) & D64_XS0_CD_MASK) -
			      di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t));
			di->xs0cd = end;
		} else
			end = di->xs0cd;
	}

	if (prev_txin == end)
		return BCME_NOTFOUND;

	*flags = BUS_SWAP32(R_SM(&di->txd64[prev_txin].ctrl1));

	di->txin = NEXTTXD(prev_txin);
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	DMA_TRACE(("%s: dma_getnexttxdd pre-txin=%d post-txin =%d, txavail = %d \n",
		di->name, prev_txin, di->txin, di->hnddma.txavail));
	return BCME_OK;
}

/** returns entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
dma64_getnextrxp(dma_info_t *di, bool forceall)
{
	uint16 i, curr;
	void *rxp;
#if ((!defined(__mips__) && !defined(BCM47XX_CA9)) || defined(__NetBSD__))
	dmaaddr_t pa;
#endif

	/* if forcing, dma engine must be disabled */
#ifdef ATE_BUILD
	if (dma64_rxenabled(di)) {
		dma64_rxreset(di);
	}
#endif
	ASSERT(!forceall || !dma64_rxenabled(di));

#if defined(D11_SPLIT_RX_FD)
nextframe:
#endif
	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	if (di->rxin == di->rs0cd) {
		curr = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	/* ignore curr if forceall */
	if (!forceall && (i == curr))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);

#if (defined(__mips__) || defined(BCM47XX_CA9)) && defined(linux)
	if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED)) {
		/* Processor prefetch of 1 x 32B cacheline carrying HWRXOFF */
		uint8 * addr = PKTDATA(di->osh, rxp);
#if defined(BCM_GMAC3)
	if (PKTISFWDERBUF(di->osh, rxp)) {
		OSL_CACHE_INV_LINE(addr);
	}
#endif
		bcm_prefetch_32B(addr, 1);
	}
#endif /* (__mips__ || BCM47XX_CA9) && linux */

	di->rxp[i] = NULL;

#if defined(SGLIST_RX_SUPPORT)
	PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow));
	PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) - di->dataoffsethigh));

	/* clear this packet from the descriptor ring */
	DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, rxp, &di->rxp_dmah[i]);

#if defined(DESCR_DEADBEEF)
	W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
	W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);
#endif /* DESCR_DEADBEEF */

#endif /* SGLIST_RX_SUPPORT */

	di->rxin = NEXTRXD(i);

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

#if defined(D11_SPLIT_RX_FD)
	/* We had marked up host descriptors as 0x80000000 */
	/* if we find that address, just skip that and go to next frame */
	if (di->sep_rxhdr) {
		if ((uint32)(uintptr)rxp == PCI64ADDR_HIGH)
			goto nextframe;
	}
#endif /* D11_SPLIT_RX_FD */

	return (rxp);
}

void BCMFASTPATH
dma_clearrxp(hnddma_t *di_pub)
{
	uint16 i, curr;
	dma_info_t *di = (dma_info_t *)di_pub;

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return;

	if (di->rxin == di->rs0cd) {
		curr = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	while (i != curr) {

		ASSERT(di->rxp[i]);
		di->rxp[i] = NULL;

		W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
		W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);
		i = NEXTRXD(i);
	}

	di->rxin = i;

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
}

/* If using indirect DMA interface, then before calling this function the IndQSel must be
 * configured for the DMA channel for which this call is being made.
 */
static bool
BCMATTACHFN_DMA_ATTACH(_dma64_addrext)(osl_t *osh, dma64regs_t *dma64regs)
{
	uint32 w;
	OR_REG(osh, &dma64regs->control, D64_XC_AE);
	w = R_REG(osh, &dma64regs->control);
	AND_REG(osh, &dma64regs->control, ~D64_XC_AE);
	return ((w & D64_XC_AE) == D64_XC_AE);
}

/**
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
static void
dma64_txrotate(dma_info_t *di)
{
	uint16 ad;
	uint nactive;
	uint rot;
	uint16 old, new;
	uint32 w;
	uint16 first, last;

	ASSERT(dma64_txsuspendedidle(di));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	nactive = _dma_txactive(di);
	ad = B2I((((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK)
		- di->xmtptrbase) & D64_XS1_AD_MASK), dma64dd_t);
	rot = TXD(ad - di->txin);

	ASSERT(rot < di->ntxd);

	/* full-ring case is a lot harder - don't worry about this */
	if (rot >= (di->ntxd - nactive)) {
		DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
		return;
	}

	first = di->txin;
	last = PREVTXD(di->txout);

	/* move entries starting at last and moving backwards to first */
	for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
		new = TXD(old + rot);

		/*
		 * Move the tx dma descriptor.
		 * EOT is set only in the last entry in the ring.
		 */
		w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl1)) & ~D64_CTRL1_EOT;
		if (new == (di->ntxd - 1))
			w |= D64_CTRL1_EOT;
		W_SM(&di->txd64[new].ctrl1, BUS_SWAP32(w));

		w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl2));
		W_SM(&di->txd64[new].ctrl2, BUS_SWAP32(w));

		W_SM(&di->txd64[new].addrlow, R_SM(&di->txd64[old].addrlow));
		W_SM(&di->txd64[new].addrhigh, R_SM(&di->txd64[old].addrhigh));

#if defined(DESCR_DEADBEEF)
		/* zap the old tx dma descriptor address field */
		W_SM(&di->txd64[old].addrlow, BUS_SWAP32(0xdeadbeef));
		W_SM(&di->txd64[old].addrhigh, BUS_SWAP32(0xdeadbeef));
#endif /* DESCR_DEADBEEF */

		/* move the corresponding txp[] entry */
		ASSERT(di->txp[new] == NULL);
		di->txp[new] = di->txp[old];

		/* Move the map */
		if (DMASGLIST_ENAB) {
			bcopy(&di->txp_dmah[old], &di->txp_dmah[new], sizeof(hnddma_seg_map_t));
			bzero(&di->txp_dmah[old], sizeof(hnddma_seg_map_t));
		}

		di->txp[old] = NULL;
	}

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

#if defined(__ARM_ARCH_7A__) && defined(CA7)
	/* memory barrier before posting the descriptor */
	DMB();
#endif

	/* kick the chip */
	W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(di->txout, dma64dd_t));
}

uint
BCMATTACHFN_DMA_ATTACH(dma_addrwidth)(si_t *sih, void *dmaregs)
{
	dma32regs_t *dma32regs;
	osl_t *osh;

	osh = si_osh(sih);

	/* Perform 64-bit checks only if we want to advertise 64-bit (> 32bit) capability) */
	/* DMA engine is 64-bit capable */
	if ((si_core_sflags(sih, 0, 0) & SISF_DMA64) == SISF_DMA64) {
		/* backplane are 64-bit capable */
		if (si_backplane64(sih))
			/* If bus is System Backplane or PCIE then we can access 64-bits */
			if ((BUSTYPE(sih->bustype) == SI_BUS) ||
			    ((BUSTYPE(sih->bustype) == PCI_BUS) &&
			     ((sih->buscoretype == PCIE_CORE_ID) ||
			      (sih->buscoretype == PCIE2_CORE_ID))))
				return (DMADDRWIDTH_64);

		/* DMA64 is always 32-bit capable, AE is always TRUE */
		ASSERT(_dma64_addrext(osh, (dma64regs_t *)dmaregs));

		return (DMADDRWIDTH_32);
	}

	/* Start checking for 32-bit / 30-bit addressing */
	dma32regs = (dma32regs_t *)dmaregs;

	/* For System Backplane, PCIE bus or addrext feature, 32-bits ok */
	if ((BUSTYPE(sih->bustype) == SI_BUS) ||
	    ((BUSTYPE(sih->bustype) == PCI_BUS) &&
	     ((sih->buscoretype == PCIE_CORE_ID) ||
	      (sih->buscoretype == PCIE2_CORE_ID))) ||
	    (_dma32_addrext(osh, dma32regs)))
		return (DMADDRWIDTH_32);

	/* Fallthru */
	return (DMADDRWIDTH_30);
}

static int
_dma_pktpool_set(dma_info_t *di, pktpool_t *pool)
{
	ASSERT(di);
	ASSERT(di->pktpool == NULL);
	di->pktpool = pool;
	return 0;
}

static bool
_dma_rxtx_error(dma_info_t *di, bool istx)
{
	uint32 status1 = 0;
	uint16 curr;

	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		if (istx) {

			/* if using indirect DMA access, then configure IndQSel */
			if (DMA_INDIRECT(di)) {
				dma_set_indqsel((hnddma_t *)di, FALSE);
			}

			status1 = R_REG(di->osh, &di->d64txregs->status1);

			if ((status1 & D64_XS1_XE_MASK) != D64_XS1_XE_NOERR)
				return TRUE;
			else if (si_coreid(di->sih) == GMAC_CORE_ID && si_corerev(di->sih) >= 4) {
				curr = (uint16)(B2I(((R_REG(di->osh, &di->d64txregs->status0) &
					D64_XS0_CD_MASK) - di->xmtptrbase) &
					D64_XS0_CD_MASK, dma64dd_t));

				if (NTXDACTIVE(di->txin, di->txout) != 0 &&
					curr == di->xs0cd_snapshot) {

					/* suspicious */
					return TRUE;
				}
				di->xs0cd_snapshot = di->xs0cd = curr;

				return FALSE;
			}
			else
				return FALSE;
		}
		else {

			status1 = R_REG(di->osh, &di->d64rxregs->status1);

			if ((status1 & D64_RS1_RE_MASK) != D64_RS1_RE_NOERR)
				return TRUE;
			else
				return FALSE;
		}

	} else if (DMA32_ENAB(di)) {
		return FALSE;

	} else {
		ASSERT(0);
		return FALSE;
	}

}

void
_dma_burstlen_set(dma_info_t *di, uint8 rxburstlen, uint8 txburstlen)
{
	di->rxburstlen = rxburstlen;
	di->txburstlen = txburstlen;
}
#ifdef PCIE_PHANTOM_DEV
/* Book keeping arrays of nsegs and size while splitting up TCM packets into chunk of 64 bytes */
int
dma_blwar_alloc(hnddma_t *dmah)
{
	uint size;
	dma_info_t * di = (dma_info_t *)dmah;
	if ((di->ntxd) && di->blwar_d11core) {
		size  = di->ntxd * sizeof(uint);
		if ((di->blwar_nsegs = (uint*)MALLOC(di->osh, size)) == NULL)
			goto fail;
		if ((di->blwar_size = (uint*)MALLOC(di->osh, size)) == NULL)
			goto fail;

		bzero(di->blwar_nsegs, size);
		bzero(di->blwar_size, size);
	}
	return 0;

fail:
	return -1;
}
#endif /* PCIE_PHANTOM_DEV */
void
_dma_param_set(dma_info_t *di, uint16 paramid, uint16 paramval)
{
	switch (paramid) {
	case HNDDMA_PID_TX_MULTI_OUTSTD_RD:
		di->txmultioutstdrd = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_PREFETCH_CTL:
		di->txprefetchctl = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_PREFETCH_THRESH:
		di->txprefetchthresh = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_BURSTLEN:
		di->txburstlen = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_PREFETCH_CTL:
		di->rxprefetchctl = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_PREFETCH_THRESH:
		di->rxprefetchthresh = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_BURSTLEN:
		di->rxburstlen = (uint8)paramval;
		break;

	case HNDDMA_PID_BURSTLEN_CAP:
		di->burstsize_ctrl = (uint8)paramval;
		break;

#ifdef PCIE_PHANTOM_DEV
	case HNDDMA_PID_BURSTLEN_WAR:
		di->blwar_d11core = (uint8)paramval;
		break;
#endif
#if defined(D11_SPLIT_RX_FD)
	case HNDDMA_SEP_RX_HDR:
		di->sep_rxhdr = (uint8)paramval;	/* indicate sep hdr descriptor is used */
		break;
#endif /* D11_SPLIT_RX_FD */
	case HNDDMA_SPLIT_FIFO :
		di->split_fifo = (uint8)paramval;
		break;

	default:
		break;
	}
}

static bool
_dma_glom_enable(dma_info_t *di, uint32 val)
{
	dma64regs_t *dregs = di->d64rxregs;
	bool ret = TRUE;

	di->hnddma.dmactrlflags &= ~DMA_CTRL_SDIO_RXGLOM;
	if (val) {
		OR_REG(di->osh, &dregs->control, D64_RC_GE);
		if (!(R_REG(di->osh, &dregs->control) & D64_RC_GE))
			ret = FALSE;
		else
			di->hnddma.dmactrlflags |= DMA_CTRL_SDIO_RXGLOM;
	} else {
		AND_REG(di->osh, &dregs->control, ~D64_RC_GE);
	}
	return ret;
}
static INLINE void
dma64_dd_upd_64_from_params(dma_info_t *di, dma64dd_t *ddring, dma64addr_t pa, uint outidx,
	uint32 *flags, uint32 bufcount)
{
	uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

	/* bit 63 is arleady set for host addresses by the caller */
	W_SM(&ddring[outidx].addrlow, BUS_SWAP32(pa.loaddr + di->dataoffsetlow));
	W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(pa.hiaddr + di->dataoffsethigh));

	W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
	W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));

	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2 | D64_CTRL2_PARITY));
		}
	}
#if defined(__ARM_ARCH_7A__) && defined(CA7)
	/* memory barrier before posting the descriptor */
	DMB();
#endif
}
static INLINE void
dma64_dd_upd_64_from_struct(dma_info_t *di, dma64dd_t *ddring, dma64dd_t *dd, uint outidx)
{
	/* bit 63 is arleady set for host addresses by the caller */
	W_SM(&ddring[outidx].addrlow, BUS_SWAP32(dd->addrlow));
	W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(dd->addrhigh));
	W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(dd->ctrl1));
	W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(dd->ctrl2));

	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(dd->ctrl2 | D64_CTRL2_PARITY));
		}
	}
#if defined(__ARM_ARCH_7A__) && defined(CA7)
	/* memory barrier before posting the descriptor */
	DMB();
#endif
}
int BCMFASTPATH
dma_rxfast(hnddma_t *dmah, dma64addr_t p, uint32 len)
{
	uint16 rxout;
	uint32 flags = 0;

	dma_info_t * di = DI_INFO(dmah);
	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxout = di->rxout;

	if ((di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1) < 1)
		goto outofrxd;

	/* reset flags for each descriptor */
	if (rxout == (di->nrxd - 1))
		flags = D64_CTRL1_EOT;

	/* Update descriptor */
	dma64_dd_upd_64_from_params(di, di->rxd64, p, rxout, &flags, len);

	di->rxp[rxout] = (void *)(uintptr)(p.loaddr);

	rxout = NEXTRXD(rxout);

	di->rxout = rxout;

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
	/* update the chip lastdscr pointer */
	W_REG(di->osh, &di->d64rxregs->ptr, di->rcvptrbase + I2B(rxout, dma64dd_t));
	return 0;
outofrxd:
	di->rxavail = 0;
	DMA_ERROR(("%s: dma_rxfast: out of rxds\n", di->name));
	return -1;
}
int BCMFASTPATH
dma_msgbuf_txfast(hnddma_t *dmah, dma64addr_t p0, bool commit, uint32 len, bool first, bool last)
{
	uint16 txout;
	uint32 flags = 0;
	dma_info_t * di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	txout = di->txout;

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	if (len == 0)
		return 0;

	if (first)
		flags |= D64_CTRL1_SOF;
	if (last)
		flags |= D64_CTRL1_EOF | D64_CTRL1_IOC;

	if (txout == (di->ntxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd_64_from_params(di, di->txd64, p0, txout, &flags, len);

	ASSERT(di->txp[txout] == NULL);

	txout = NEXTTXD(txout);

	/* return nonzero if out of tx descriptors */
	if (txout == di->txin) {
		DMA_ERROR(("%s: dma_txfast: Out-of-DMA descriptors"
			   " (txin %d txout %d)\n", __FUNCTION__,
			   di->txin, di->txout));
		goto outoftxd;
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = (void *)(uintptr)(p0.loaddr);

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit)
		W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(txout, dma64dd_t));

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}


/*
 * Sequentially program the pktdata(lfrag) - from TCM, followed by the
 * individual fragments from the HOST.
 */
static int BCMFASTPATH
dma64_txfast_lfrag(dma_info_t *di, void *p0, bool commit)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint16 txout;
	uint32 flags = 0;
	dmaaddr_t pa;
	dma64addr_t pa64 = {0, 0};
	uint8 i = 0, j = 0;
#ifdef PCIE_PHANTOM_DEV
	uint16 thresh = 64;
	uint16 templen;
	uint8* panew;
#endif

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	txout = di->txout;

	/*
	 * Lfrag - Program the descriptor for Lfrag data first before
	 * considering the individual fragments
	 */
	for (p = p0; p; p = next) {
		uint ftot = 0;
		uint nsegs = 1;

		next = PKTNEXT(di->osh, p);
		data = PKTDATA(di->osh, p);
		len  = PKTLEN(di->osh, p);

		if (PKTISTXFRAG(di->osh, p))
			ftot = PKTFRAGTOTNUM(di->osh, p);

		if (len == 0) {
			/* Should not happen ideally unless this is a chained lfrag */
#ifdef PCIE_PHANTOM_DEV
			if (di->blwar_d11core) {
				di->blwar_nsegs[txout] = 0 + ftot;
				di->blwar_size[txout] = 0;
			}
#endif
			goto program_frags;
		}

		pa = DMA_MAP(di->osh, data, len, DMA_TX, p, &di->txp_dmah[txout]);
#ifdef PCIE_PHANTOM_DEV
		panew = (uint8*) pa;
		if (di->blwar_d11core) {
			if (ISALIGNED((uint8 *) pa, 4))
				templen = len;
			else
				templen = len + ((uint8 *) pa - (uint8 *) ALIGN_ADDR(pa, 4));

			nsegs = (templen % thresh) ? ((templen / thresh) + 1) : (templen / thresh);
			if ((nsegs+ftot) > (uint)(di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1))
				goto outoftxd;

			di->blwar_nsegs[txout] = nsegs + ftot;
			di->blwar_size[txout] = len;
		} else
#endif /* PCIE_PHANTOM_DEV */
		{
			if ((nsegs+ftot) > (uint)(di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1))
				goto outoftxd;
		}

		for (j = 1; j <= nsegs; j++) {
			flags = 0;
			if ((p == p0) && (j == 1))
				flags |= D64_CTRL1_SOF;
			if (txout == (di->ntxd - 1))
				flags |= D64_CTRL1_EOT;
			if (di->burstsize_ctrl)
				flags |= D64_CTRL1_NOTPCIE;
			/* txsplit and buffer is from local memory. */
			if (DMA_TRANSCOHERENT(di))
				flags |= D64_CTRL1_COHERENT;

			if ((j == nsegs) && (ftot == 0) && (next == NULL))
				flags |= (D64_CTRL1_IOC | D64_CTRL1_EOF);
#ifdef PCIE_PHANTOM_DEV
			uint16 dma_len;
			if (di->blwar_d11core) {
				/* split into chunks of thresh bytes */
				if (len < thresh)
					dma_len = len;
				else
					dma_len = thresh - ((uintptr)panew % 4);

				len -= dma_len;
				dma64_dd_upd(di, di->txd64, (dmaaddr_t)panew,
					txout, &flags, dma_len);
				panew = panew + dma_len;
			} else
#endif
			{
				dma64_dd_upd(di, di->txd64, pa, txout,
					&flags, len);
			}

			ASSERT(di->txp[txout] == NULL);
			txout = NEXTTXD(txout);
		}

program_frags:
		/*
		 * Now, walk the chain of fragments in this lfrag allocating
		 * and initializing transmit descriptor entries.
		 */
		for (i = 1, j = 1; j <= ftot; i++, j++) {
			flags = 0;
			if (PKTFRAGISCHAINED(di->osh, i)) {
				 i = 1;
				 p = PKTNEXT(di->osh, p);
				 ASSERT(p != NULL);
				 next = PKTNEXT(di->osh, p);
			}

			len = PKTFRAGLEN(di->osh, p, i);

#ifdef BCM_DMAPAD
			if (DMAPADREQUIRED(di)) {
				len += PKTDMAPAD(di->osh, p);
			}
#endif /* BCM_DMAPAD */

			pa64.loaddr = PKTFRAGDATA_LO(di->osh, p, i);
			pa64.hiaddr = PKTFRAGDATA_HI(di->osh, p, i);

			if ((j == ftot) && (next == NULL))
				flags |= (D64_CTRL1_IOC | D64_CTRL1_EOF);
			if (txout == (di->ntxd - 1))
				flags |= D64_CTRL1_EOT;

			/* War to handle 64 bit dma address for now */
			dma64_dd_upd_64_from_params(di, di->txd64, pa64, txout, &flags, len);

			ASSERT(di->txp[txout] == NULL);
			txout = NEXTTXD(txout);
		}
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit)
		W_REG(di->osh, &di->d64txregs->ptr, di->xmtptrbase + I2B(txout, dma64dd_t));

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

#if defined(D11_SPLIT_RX_FD)
static bool BCMFASTPATH
dma_splitrxfill(dma_info_t *di)
{
	void *p = NULL;
	uint16 rxin1, rxout1;
	uint16 rxin2, rxout2;
	uint32 flags = 0;
	uint n;
	uint i;
	dmaaddr_t pa;
	uint extra_offset = 0;
	bool ring_empty;
	uint pktlen;
	dma64addr_t pa64 = {0, 0};


	ring_empty = FALSE;
	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 * Both DMA engines may not have same no. of lacking descrs
	 * Take MIN of both
	 */
	if (di->split_fifo == SPLIT_FIFO_0) {
		/* For split fifo fill, this function expects fifo-1 di to proceed further. */
		/* fifo-0 requests can be discarded since fifo-1 request will fill up both fifos */
		return FALSE;
	}
	rxin1 = di->rxin;
	rxout1 = di->rxout;
	rxin2 = di->linked_di->rxin;
	rxout2 = di->linked_di->rxout;

	/* Always post same no of descriptors to fifo-0 & fifo-1 */
	/* While reposting , consider the fifo which has consumed least no of descriptors */
	n = MIN(di->nrxpost - NRXDACTIVE(rxin1, rxout1),
		di->linked_di->nrxpost - NRXDACTIVEPERHANDLE(rxin2, rxout2, di->linked_di));

	/* Assert that both DMA engines have same pktpools */
	ASSERT(di->pktpool == di->linked_di->pktpool);

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;

	for (i = 0; i < n; i++) {
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
		if (POOL_ENAB(di->pktpool)) {
			ASSERT(di->pktpool);
			p = pktpool_get(di->pktpool);
#ifdef BCMDBG_POOL
			if (p)
				PKTPOOLSETSTATE(p, POOL_RXFILL);
#endif /* BCMDBG_POOL */
		}

		if (p == NULL) {
			DMA_TRACE(("%s: dma_rxfill: out of rxbufs\n", di->name));
			if (i == 0) {
				if (DMA64_ENAB(di) && DMA64_MODE(di)) {
					if (dma64_rxidle(di)) {
						DMA_TRACE(("%s: rxfill64: ring is empty !\n",
							di->name));
						ring_empty = TRUE;
					}
				} else if (DMA32_ENAB(di)) {
					if (dma32_rxidle(di)) {
						DMA_TRACE(("%s: rxfill32: ring is empty !\n",
							di->name));
						ring_empty = TRUE;
					}
				} else
					ASSERT(0);
			}
			di->hnddma.rxnobuf++;
			break;
		}
		if (extra_offset)
			PKTPULL(di->osh, p, extra_offset);

		/* Do a cached write instead of uncached write since DMA_MAP
		 * will flush the cache.
		*/

		*(uint16 *)(PKTDATA(di->osh, p)) = 0;
#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint16), DMA_TX, NULL, NULL);
#endif
		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout1], sizeof(hnddma_seg_map_t));

		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout1]);

		ASSERT(ISALIGNED(PHYSADDRLO(pa), 4));

		/* save the free packet pointer */
		ASSERT(di->rxp[rxout1] == NULL);
		di->rxp[rxout1] = p;

		/* TCM Descriptor */
		flags = 0;
		pktlen = PKTLEN(di->osh, p);
		if (rxout1 == (di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		/* buffers that are going to be entirely in local mem should
		 * have SOF=0 and CO=1.
		 */
		if (DMA_TRANSCOHERENT(di))
			flags |= D64_CTRL1_COHERENT;

		dma64_dd_upd(di, di->rxd64, pa, rxout1, &flags, pktlen);
		rxout1 = NEXTRXD(rxout1);		/* update rxout */

		/* Now program host descriptor */
		/* Store pointer to packet */
		di->linked_di->rxp[rxout2] = p;
		flags = 0;	/* Reset the flags */
		if (rxout2 == (di->linked_di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		/* Extract out host length */
		pktlen = PKTFRAGLEN(di->linked_di->osh, p, 1);

		/* Extract out host addresses */
		pa64.loaddr = PKTFRAGDATA_LO(di->linked_di->osh, p, 1);
		pa64.hiaddr = PKTFRAGDATA_HI(di->linked_di->osh, p, 1);
		/* load the descriptor */
		dma64_dd_upd_64_from_params(di->linked_di, di->linked_di->rxd64, pa64, rxout2,
			&flags, pktlen);
		rxout2 = NEXTRXD(rxout2);	/* update rxout */
	}

	di->rxout = rxout1;
	di->linked_di->rxout = rxout2;

	/* update the chip lastdscr pointer */
	if (DMA64_ENAB(di) && DMA64_MODE(di)) {
		W_REG(di->osh, &di->d64rxregs->ptr, di->rcvptrbase + I2B(rxout1, dma64dd_t));
	} else if (DMA32_ENAB(di)) {
		W_REG(di->osh, &di->d32rxregs->ptr, I2B(rxout1, dma32dd_t));
	} else
		ASSERT(0);

	if (DMA64_ENAB(di->linked_di) && DMA64_MODE(di->linked_di)) {
		W_REG(di->linked_di->osh, &di->linked_di->d64rxregs->ptr,
			di->linked_di->rcvptrbase + I2B(rxout2, dma64dd_t));
	} else if (DMA32_ENAB(di->linked_di)) {
		W_REG(di->linked_di->osh, &di->linked_di->d32rxregs->ptr, I2B(rxout2, dma32dd_t));
	} else
		ASSERT(0);

	return ring_empty;
}
#endif /* D11_SPLIT_RX_FD */

void
dma_link_handle(hnddma_t *dmah1, hnddma_t *dmah2)
{
	dma_info_t *di1 = DI_INFO(dmah1);
	dma_info_t *di2 = DI_INFO(dmah2);
	di1->linked_di = di2;
	di2->linked_di = di1;
}

/* Function to get the next descriptor index to post */
uint16
dma_get_next_txd_idx(hnddma_t *di, bool txout)
{
	if (txout)
		return (DI_INFO(di))->txout;
	else
		return (DI_INFO(di))->txin;
}

/* Function to get the number of descriptors between start and txout/txin */
uint16
dma_get_txd_count(hnddma_t *dmah, uint16 start, bool txout)
{
	dma_info_t *di = DI_INFO(dmah);

	if (txout)
		return (NTXDACTIVE(start, di->txout));
	else
		return (NTXDACTIVE(start, di->txin));
}

/* Function to return the address of the descriptor with the given index */
uintptr
dma_get_txd_addr(hnddma_t *di, uint16 idx)
{
	dma_info_t *ddi = DI_INFO(di);
	return ((uintptr)(ddi->txd64 + idx));
}

/* Function to get the memory address of the buffer pointed to by the
 * descriptor #idx
 */
void
dma_get_txd_memaddr(hnddma_t *dmah, uint32 *addrlo, uint32 *addrhi, uint idx)
{
	dma_info_t *di = DI_INFO(dmah);
	/* get the memory address of the data buffer pointed by descriptor */
	*addrlo = BUS_SWAP32(R_SM(&di->txd64[idx].addrlow));
	*addrhi = BUS_SWAP32(R_SM(&di->txd64[idx].addrhigh));
}

int
dmatx_map_pkts(hnddma_t *dmah, map_pkts_cb_fn cb, void *ctx)
{
	uint16 start, end, i;
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dmatx_map_pkts\n", di->name));

	if (di->ntxd == 0) {
		return BCME_ERROR;
	}

	start = di->txin;
	end = di->txout;

	for (i = start; i != end; i = NEXTTXD(i)) {
		if (di->txp[i] != NULL) {
			/* ignoring the return 'delete' bool since hnddma
			 * does not allow deleting pkts on the ring.
			 */
			(void)cb(ctx, di->txp[i]);
		}
	}
	return BCME_OK;
}
