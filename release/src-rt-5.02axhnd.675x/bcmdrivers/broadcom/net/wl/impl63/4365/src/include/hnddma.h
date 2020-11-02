/*
 * Generic Broadcom Home Networking Division (HND) DMA engine SW interface
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: hnddma.h 778298 2019-08-29 00:43:36Z $
 */

#ifndef	_hnddma_h_
#define	_hnddma_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <siutils.h>

#ifndef _hnddma_pub_
#define _hnddma_pub_
/* for pktpool_t */
#include <bcmutils.h>
typedef const struct hnddma_pub hnddma_t;
#endif /* _hnddma_pub_ */

/* range param for dma_getnexttxp() and dma_txreclaim */
typedef enum txd_range {
	HNDDMA_RANGE_ALL		= 1,
	HNDDMA_RANGE_TRANSMITTED,
	HNDDMA_RANGE_TRANSFERED
} txd_range_t;

/* dma parameters id */
enum dma_param_id {
	HNDDMA_PID_TX_MULTI_OUTSTD_RD	= 0,
	HNDDMA_PID_TX_PREFETCH_CTL,
	HNDDMA_PID_TX_PREFETCH_THRESH,
	HNDDMA_PID_TX_BURSTLEN,

	HNDDMA_PID_RX_PREFETCH_CTL	= 0x100,
	HNDDMA_PID_RX_PREFETCH_THRESH,
	HNDDMA_PID_RX_BURSTLEN,
	HNDDMA_PID_BURSTLEN_CAP,
	HNDDMA_PID_BURSTLEN_WAR,
	HNDDMA_SEP_RX_HDR,
	HNDDMA_SPLIT_FIFO
};
#define SPLIT_FIFO_0	1
#define SPLIT_FIFO_1	2

extern void dma_detach(hnddma_t *dmah);
extern void dma_txinit(hnddma_t *dmah);
extern bool dma_txreset(hnddma_t *dmah);
extern bool dma_txenabled(hnddma_t *dmah);
extern void dma_txsuspend(hnddma_t *dmah);
extern void dma_txresume(hnddma_t *dmah);
extern bool dma_txsuspended(hnddma_t *dmah);
extern bool dma_txsuspendedidle(hnddma_t *dmah);
#ifdef WL_MULTIQUEUE
extern void dma_txflush(hnddma_t *dmah);
extern void dma_txflush_clear(hnddma_t *dmah);
#endif /* WL_MULTIQUEUE */
extern int  dma_txfast(hnddma_t *dmah, void *p, bool commit);
extern int  dma_txunframed(hnddma_t *dmah, void *p, uint len, bool commit);
extern void *dma_getpos(hnddma_t *di, bool direction);
extern bool dma_txstopped(hnddma_t *dmah);
extern int  dma_txreclaim(hnddma_t *dmah, txd_range_t range);
extern void *dma_getnexttxp(hnddma_t *dmah, txd_range_t range);
extern void *dma_peeknexttxp(hnddma_t *dmah);
extern int  dma_peekntxp(hnddma_t *dmah, int *len, void *txps[], txd_range_t range);
extern void dma_txblock(hnddma_t *dmah);
extern void dma_txunblock(hnddma_t *dmah);
extern uint dma_txactive(hnddma_t *dmah);
extern void dma_txrotate(hnddma_t *dmah);
extern void dma_rxinit(hnddma_t *dmah);
extern bool dma_rxreset(hnddma_t *dmah);
extern bool dma_rxidle(hnddma_t *dmah);
extern bool dma_rxstopped(hnddma_t *dmah);
extern void dma_rxenable(hnddma_t *dmah);
extern bool dma_rxenabled(hnddma_t *dmah);
extern void *dma_rx(hnddma_t *dmah);
extern bool dma_rxfill(hnddma_t *dmah);
extern void dma_rxreclaim(hnddma_t *dmah);
extern void *dma_getnextrxp(hnddma_t *dmah, bool forceall);
extern void *dma_peeknextrxp(hnddma_t *dmah);
extern void dma_rxparam_get(hnddma_t *dmah, uint16 *rxoffset, uint16 *rxbufsize);
extern void dma_fifoloopbackenable(hnddma_t *dmah);
extern uintptr dma_getvar(hnddma_t *dmah, const char *name);
extern void dma_counterreset(hnddma_t *dmah);
extern uint dma_ctrlflags(hnddma_t *dmah, uint mask, uint flags);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void dma_dump(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring);
extern void dma_dumptx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring);
extern void dma_dumprx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring);
#endif // endif
extern uint dma_rxactive(hnddma_t *dmah);
extern uint dma_txpending(hnddma_t *dmah);
extern uint dma_txcommitted(hnddma_t *dmah);
extern int  dma_pktpool_set(hnddma_t *dmah, pktpool_t *pool);
extern bool dma_rxtxerror(hnddma_t *dmah, bool istx);
extern void dma_burstlen_set(hnddma_t *dmah, uint8 rxburstlen, uint8 txburstlen);
extern uint dma_avoidance_cnt(hnddma_t *dmah);
extern void dma_param_set(hnddma_t *dmah, uint16 paramid, uint16 paramval);
extern bool dma_glom_enable(hnddma_t *dmah, uint32 val);
extern uint dma_activerxbuf(hnddma_t *dmah);
#ifdef RXDMA_STUCK_WAR
extern bool dma_get_curr_rxin(hnddma_t * dmah, uint16 *curr_rxin);
#endif // endif
/*
 * Exported data structure (read-only)
 */
/* export structure */
struct hnddma_pub {
	void		*di_fn;		/* Place holder for ROM compatible */
	uint		txavail;	/* # free tx descriptors */
	uint		dmactrlflags;	/* dma control flags */

	/* rx error counters */
	uint		rxgiants;	/* rx giant frames */
	uint		rxnobuf;	/* rx out of dma descriptors */
	/* tx error counters */
	uint		txnobuf;	/* tx out of dma descriptors */
	uint		txnodesc;	/* tx out of dma descriptors running count */
};
#ifdef PCIE_PHANTOM_DEV
extern int dma_blwar_alloc(hnddma_t *di);
#endif // endif

typedef struct dma_common dma_common_t;

/* Flags for dma_attach_ext function */
#define BCM_DMA_IND_INTF_FLAG		0x00000001	/* set for using INDIRECT DMA INTERFACE */
#define BCM_DMA_DESC_ONLY_FLAG		0x00000002	/* For DMA that posts descriptors only and
							 * no packets
							 */
#define BCM_DMA_DESC_CD_WAR_FLAG	0x00000004	/* WAR for descriptor only DMA's CD not
							 * being updated correctly by HW in cut
							 * through DMA mode.
							 */
#define BCM_DMA_CHAN_SWITCH_EN		0x00000008	/* for d11 corerev 64+ to help arbitrate
							 * btw dma channels.
							 */
#define BCM_DMA_BULK_PROCESSING		0x00000080	/* Datapath bulk DMA processing */

extern dma_common_t * dma_common_attach(osl_t *osh, volatile uint32 *indqsel,
	volatile uint32 *suspreq, volatile uint32 *flushreq);
extern void dma_common_detach(dma_common_t *dmacommon);

#ifdef BCM_DMA_INDIRECT
/* Use indirect registers for non-ctmode */
#define DMA_INDQSEL_IA	(1 << 31)
extern void dma_set_indqsel(hnddma_t *di, bool force);
#else
#define dma_set_indqsel(a, b)
#endif /* #ifdef BCM_DMA_INDIRECT */

extern hnddma_t * dma_attach_ext(dma_common_t *dmac, osl_t *osh, const char *name, si_t *sih,
	volatile void *dmaregstx, volatile void *dmaregsrx, uint32 flags, uint8 qnum,
	uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost, uint rxoffset,
	uint *msg_level);
extern hnddma_t * dma_attach(osl_t *osh, const char *name, si_t *sih,
	volatile void *dmaregstx, volatile void *dmaregsrx,
	uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost,
	uint rxoffset, uint *msg_level);

/* return addresswidth allowed
 * This needs to be done after SB attach but before dma attach.
 * SB attach provides ability to probe backplane and dma core capabilities
 * This info is needed by DMA_ALLOC_CONSISTENT in dma attach
 */
extern uint dma_addrwidth(si_t *sih, void *dmaregs);

/* count the number of tx packets that are queued to the dma ring */
extern uint dma_txp(hnddma_t *di);

#ifdef WL_MULTIQUEUE
extern void dma_txrewind(hnddma_t *di);
#endif // endif
/* pio helpers */
extern void dma_txpioloopback(osl_t *osh, dma32regs_t *);
extern int dma_msgbuf_txfast(hnddma_t *di, dma64addr_t p0, bool com, uint32 ln, bool fst, bool lst);

extern int dma_rxfast(hnddma_t *di, dma64addr_t p, uint32 len);
extern int dma_rxfill_suspend(hnddma_t *dmah, bool suspended);
extern void dma_link_handle(hnddma_t *dmah1, hnddma_t *dmah2);
extern int dma_rxfill_unframed(hnddma_t *di, void *buf, uint len, bool commit);

extern uint16 dma_get_next_txd_idx(hnddma_t *di, bool txout);
extern uint16 dma_get_txd_count(hnddma_t *dmah, uint16 start, bool txout);
extern uintptr dma_get_txd_addr(hnddma_t *di, uint16 idx);

/* returns the memory address (hi and low) of the buffer associated with the dma descriptor
 * having index idx.
 */
extern void dma_get_txd_memaddr(hnddma_t *dmah, uint32 *addrlo, uint32 *addrhi, uint idx);

extern int dma_txdesc(hnddma_t *dmah, dma64dd_t *dd, bool commit);
extern int dma_getnexttxdd(hnddma_t *dmah, txd_range_t range, uint32 *flags);
extern void dma_clearrxp(hnddma_t *di);
extern int dmatx_map_pkts(hnddma_t *di, map_pkts_cb_fn cb, void *ctx);
extern int dma_commit(hnddma_t *dmah);
extern void aqm_dma_commit(hnddma_t *dmah, uint queue);
extern bool aqm_dma_sched_fifo(hnddma_t *dmah, uint fifo);
extern bool aqm_dma_prefetch_active(hnddma_t *dmah, uint fifo);
extern int aqm_dma_get_current_fifo(dma_common_t *dmac);
extern void aqm_dma_init_fifo_stats(hnddma_t *dmah, int fifo);

#ifdef BULK_PKTLIST
extern int dma64_bulk_tx(hnddma_t *dmah, void *tx_list, bool do_commit);
extern int dma64_bulk_txcomplete(hnddma_t *dmah, uint16 ncons, uint16 *nproc,
		void **list_head, void **list_tail, txd_range_t range);
#endif /* BULK_PKTLIST */

#endif	/* _hnddma_h_ */
