/*
 * HND generic packet operation primitives
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: $
 */

#ifndef _hnd_pkt_h_
#define _hnd_pkt_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <hnd_lbuf.h>
#include <hnd_pktid.h>

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	(MAXPKTBUFSZ - LBUFSZ)
#define	PKTFRAGSZ	(MAXPKTFRAGSZ - LBUFFRAGSZ)
#define	PKTRXFRAGSZ	(MAXPKTRXFRAGSZ - LBUFFRAGSZ)

/* packet primitives */
#ifdef BCMPKTPOOL
#define PKTGETLF(osh, len, send, lbuf_type)	\
	(void *)hnd_pkt_frag_get((osh), (len), lbuf_type)
void * hnd_pkt_frag_get(osl_t *osh, uint len, enum lbuf_type lbuf_type);
#define PKTGET(osh, len, send)		(void *)hnd_pkt_get(osh, len)
void * hnd_pkt_get(osl_t *osh, uint len);
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#define PKTSWAPD11BUF(osh, p) lfbufpool_swap_d11_buf(osh, p)
int lfbufpool_swap_d11_buf(osl_t *osh, void *p);
#define PKTBUFEARLYFREE(osh, p) lfbufpool_early_free_buf(osh, p)
void lfbufpool_early_free_buf(osl_t *osh, void *p);
#endif /* BCM_DHDHDR && DONGLEBUILD */
#else
#define PKTGETLF(osh, len, send, lbuf_type)	\
	(void *)hnd_pkt_alloc(osh, len, lbuf_type)
#define PKTGET(osh, len, send)		(void *)hnd_pkt_alloc(osh, len, lbuf_basic)
#endif /* BCMPKTPOOL */
#define PKTALLOC(osh, len, lbuf_type) \
	(void *)hnd_pkt_alloc(osh, len, lbuf_type)
#define PKTFREE(osh, p, send)		hnd_pkt_free(osh, p, send)
#define	PKTDATA(osh, lb)		({BCM_REFERENCE(osh); LBP(lb)->data;})
#define	PKTLEN(osh, lb)			({BCM_REFERENCE(osh); LBP(lb)->len;})
#define	PKTHEADROOM(osh, lb) \
	({BCM_REFERENCE(osh); (LBP(lb)->data - lb_head(LBP(lb)));})
#define	PKTTAILROOM(osh, lb) \
	({ \
	BCM_REFERENCE(osh); \
	(lb_end(LBP(lb)) - (LBP(lb)->data + LBP(lb)->len)); \
	})
#define	PKTSETLEN(osh, lb, len)		({BCM_REFERENCE(osh); lb_setlen(LBP(lb), (len));})
#define	PKTPUSH(osh, lb, bytes)		({BCM_REFERENCE(osh); lb_push(LBP(lb), (bytes));})
#define	PKTPULL(osh, lb, bytes)		({BCM_REFERENCE(osh); lb_pull(LBP(lb), (bytes));})
#define PKTDUP(osh, p)			hnd_pkt_dup((osh), (p))
#define	PKTTAG(lb)			((void *)((LBP(lb))->pkttag))
#define	PKTPRIO(lb)			lb_pri(LBP(lb))
#define	PKTSETPRIO(lb, x)		lb_setpri(LBP(lb), (x))
#define PKTSHARED(lb)			(lb_isclone(LBP(lb)) || LBP(lb)->mem.refcnt > 1)
#define PKTALLOCED(osh)	\
	((((osl_t *)osh)->cmn->refcount == 1) ? ((osl_t *)osh)->cmn->pktalloced: 0)

/* checksum primitives */
#define PKTSUMNEEDED(lb)		lb_sumneeded(LBP(lb))
#define PKTSETSUMNEEDED(lb, x)		lb_setsumneeded(LBP(lb), (x))
#define PKTSUMGOOD(lb)			lb_sumgood(LBP(lb))
#define PKTSETSUMGOOD(lb, x)		lb_setsumgood(LBP(lb), (x))

/* message trace primitives */
#define PKTMSGTRACE(lb)			lb_msgtrace(LBP(lb))
#define PKTSETMSGTRACE(lb, x)		lb_setmsgtrace(LBP(lb), (x))

#define PKTDATAOFFSET(lb)		lb_dataoff(LBP(lb))
#define PKTSETDATAOFFSET(lb, dataOff)	lb_setdataoff(LBP(lb), dataOff)

#if defined(BCMPKTIDMAP)
#define PKTPTR(id)			(hnd_pktptr_map[id])
#define PKTID(lb)			lb_getpktid(LBP(lb))
#define PKTSETID(lb, id)		lb_setpktid(LBP(lb), (id))
#define PKTNEXT(osh, lb)		({BCM_REFERENCE(osh); PKTPTR(LBP(lb)->nextid);})
#define PKTSETNEXT(osh, lb, x) \
	({ \
	BCM_REFERENCE(osh); \
	(LBP(lb)->nextid = ((x) == NULL) ? PKT_NULL_ID : PKTID(LBP(x))); \
	})
#define PKTLINK(lb)			(PKTPTR(LBP(lb)->linkid))
#define PKTSETLINK(lb, x) \
	(LBP(lb)->linkid = ((x) == NULL) ? PKT_NULL_ID : PKTID(LBP(x)))
#define PKTFREELIST(lb)			(LBP(lb)->freelist)
#define PKTSETFREELIST(lb, x)		(LBP(lb)->freelist = LBP(x))
#else  /* ! BCMPKTIDMAP */
#define PKTPTR(lb)			(lb)
#define PKTID(lb)			({BCM_REFERENCE(lb); 0;})
#define PKTSETID(lb, id)		({BCM_REFERENCE(lb); BCM_REFERENCE(id);})
#define	PKTNEXT(osh, lb)		({BCM_REFERENCE(osh); (LBP(lb)->next);})
#define	PKTSETNEXT(osh, lb, x)		({BCM_REFERENCE(osh); (LBP(lb)->next = LBP(x));})
#define	PKTLINK(lb)			(LBP(lb)->link)
#define	PKTSETLINK(lb, x)		(LBP(lb)->link = LBP(x))
#define PKTFREELIST(lb)			PKTLINK(lb)
#define PKTSETFREELIST(lb, x)		PKTSETLINK((lb), (x))
#endif /* ! BCMPKTIDMAP */

/* packet pool */
#define PKTSETPOOL(osh, lb, x, y) \
	({BCM_REFERENCE(osh); lb_setpool(LBP(lb), (x), (y));})
#define PKTPOOL(osh, lb)		({BCM_REFERENCE(osh); lb_pool(LBP(lb));})
#ifdef BCMDBG_POOL
#define PKTPOOLSTATE(lb)		lb_poolstate(LBP(lb))
#define PKTPOOLSETSTATE(lb, s)		lb_setpoolstate(LBP(lb), (s))
#endif // endif

#define PKTALTINTF(lb)			lb_altinterface(LBP(lb))
#define PKTSETALTINTF(lb, x)		lb_setaltinterface(LBP(lb), (x))

#define PKTSHRINK(osh, m)		hnd_pkt_shrink((osh), (m))

/* BCM_DMAPAD not supported if DMA bulk processing is enabled. */
#if defined(DMA_BULK_PKTLIST) && defined(BCM_DMAPAD)
#error "DMA BULK Processing (DMA_BULK_PKTLIST) incompatible with (BCM_DMAPAD) feature"
#endif // endif

#define PKTDMAPAD(osh, lb)		({BCM_REFERENCE(osh); (LBP(lb)->dmapad);})
#define PKTSETDMAPAD(osh, lb, pad)	({BCM_REFERENCE(osh); (LBP(lb)->dmapad = (pad));})

#define PKTDMAIDX(osh, lb)		({BCM_REFERENCE(osh); (LBP(lb)->dma_index);})
#define PKTSETDMAIDX(osh, lb, idx)	({BCM_REFERENCE(osh); (LBP(lb)->dma_index = (idx));})

#define PKTRXCPLID(osh, lb)		({BCM_REFERENCE(osh); (LBP(lb)->rxcpl_id);})
#define PKTSETRXCPLID(osh, lb, id)	({BCM_REFERENCE(osh); (LBP(lb)->rxcpl_id = (id));})
#define PKTRESETRXCPLID(osh, lb)	PKTSETRXCPLID(osh, lb, 0)

#define PKTSETNODROP(osh, lb)		({BCM_REFERENCE(osh); lb_setnodrop(LBP(lb));})
#define PKTNODROP(osh, lb)		({BCM_REFERENCE(osh); lb_nodrop(LBP(lb));})

#define PKTSETTYPEEVENT(osh, lb)	({BCM_REFERENCE(osh); lb_settypeevent(LBP(lb));})
#define PKTTYPEEVENT(osh, lb)		({BCM_REFERENCE(osh); lb_typeevent(LBP(lb));})

#define PKT_SET_DOT3(osh, lb)		({BCM_REFERENCE(osh); lb_set_dot3_pkt(LBP(lb));})
#define PKT_DOT3(osh, lb)		({BCM_REFERENCE(osh); lb_dot3_pkt(LBP(lb));})

#ifdef BCMDBG_PKT /* pkt logging for debugging */
#define PKTLIST_DUMP(osh, buf)		({BCM_REFERENCE(osh); (void)buf;})
#else /* BCMDBG_PKT */
#define PKTLIST_DUMP(osh, buf)		BCM_REFERENCE(osh)
#endif /* BCMDBG_PKT */

#define PKTFRMNATIVE(osh, lb)		((void *)(hnd_pkt_frmnative((osh), (lb))))
#define PKTTONATIVE(osh, p)		((struct lbuf *)(hnd_pkt_tonative((osh), (p))))

#define PKTSET80211(lb)			lb_set80211pkt(LBP(lb))
#define PKT80211(lb)			lb_80211pkt(LBP(lb))

#if defined(WL_MONITOR)
#define PKTSETMON(lb)                   lb_setmonpkt(LBP(lb))
#define PKTMON(lb)                      lb_monpkt(LBP(lb))
#endif /* WL_MONITOR */

#define PKTIFINDEX(osh, lb)		(LBP(lb)->ifidx)
#define PKTSETIFINDEX(osh, lb, idx)	(LBP(lb)->ifidx = (idx))

/* These macros used to set/get a 32-bit value in the pkttag */
#define PKTTAG_SET_VALUE(lb, val)	(*((uint32*)PKTTAG(lb)) = (uint32)val)
#define PKTTAG_GET_VALUE(lb)		(*((uint32*)PKTTAG(lb)))

/* Lbuf with fraglist */
#define PKTFRAGPKTID(osh, lb)		(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.pktid)
#define PKTSETFRAGPKTID(osh, lb, id)	(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.pktid = (id))
#define PKTFRAGTOTNUM(osh, lb)		LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.fnum
#define PKTSETFRAGTOTNUM(osh, lb, tot) \
	(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.fnum = (tot))
#define PKTFRAGTOTLEN(osh, lb)		LBFP(lb)->flist.flen[LB_FRAG_CTX]
#define PKTSETFRAGTOTLEN(osh, lb, len) \
	(LBFP(lb)->flist.flen[LB_FRAG_CTX] = (len))
#define PKTFRAGFLOWRINGID(osh, lb)	LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.flowid
#define PKTSETFRAGFLOWRINGID(osh, lb, ring) \
	(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.flowid = (ring))

/* Some of the builds use only one frag and in those cases,
 * would like to use the last one for metadata
*/
/* Lbuf with metadata inside */
#define PKTFRAGMETADATALEN(osh, lb)	LBFP(lb)->flist.flen[LB_FRAG_MAX]
#define PKTFRAGMETADATA_LO(osh, lb)	LBFP(lb)->flist.finfo[LB_FRAG_MAX].frag.data_lo
#define PKTFRAGMETADATA_HI(osh, lb)	LBFP(lb)->flist.finfo[LB_FRAG_MAX].frag.data_hi
#define PKTSETFRAGMETADATALEN(osh, lb, len) \
	(PKTFRAGMETADATALEN(osh, lb) = (len))
#define PKTSETFRAGMETADATA_LO(osh, lb, addr) \
	(PKTFRAGMETADATA_LO(osh, lb) = (addr))
#define PKTSETFRAGMETADATA_HI(osh, lb, addr) \
	(PKTFRAGMETADATA_HI(osh, lb) = (addr))

/* RX Lfrag with Rx completion ID */
#define PKTFRAGRXCPLID(osh, lb)		(LBFP(lb)->flist.rxcpl_id)
#define PKTFRAGSETRXCPLID(osh, lb, id) \
	(PKTFRAGRXCPLID(osh, lb) = (id))

/* in rx path, reuse totlen as used len */
#define PKTFRAGUSEDLEN(osh, lb)	\
	(PKTISRXFRAG(osh, lb) ?	LBFP(lb)->flist.flen[LB_FRAG_CTX] : 0)
#define PKTSETFRAGUSEDLEN(osh, lb, len) \
	(LBFP(lb)->flist.flen[LB_FRAG_CTX] = len)

#define PKTFRAGLEN(osh, lb, ix)		LBFP(lb)->flist.flen[ix]
#define PKTSETFRAGLEN(osh, lb, ix, len) \
	(LBFP(lb)->flist.flen[ix] = (len))
#define PKTFRAGDATA_LO(osh, lb, ix)	LBFP(lb)->flist.finfo[ix].frag.data_lo
#define PKTSETFRAGDATA_LO(osh, lb, ix, addr) \
	(LBFP(lb)->flist.finfo[ix].frag.data_lo = (addr))
#define PKTFRAGDATA_HI(osh, lb, ix)	LBFP(lb)->flist.finfo[ix].frag.data_hi
#define PKTSETFRAGDATA_HI(osh, lb, ix, addr) \
	(LBFP(lb)->flist.finfo[ix].frag.data_hi = (addr))

#define PKTCOPYFLAGS(osh, lb1, lb2)      ({BCM_REFERENCE(osh);LBP(lb1)->flags \
						= LBP(lb2)->flags;})

/* RX FRAG */
#define PKTISRXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_is_rxfrag(LBP(lb));})
#define PKTSETRXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_set_rxfrag(LBP(lb));})
#define PKTRESETRXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_reset_rxfrag(LBP(lb));})
#define PKTISPKTFETCHED(osh, lb)        ({BCM_REFERENCE(osh); lb_is_pktfetched(LBP(lb));})
#define PKTSETPKTFETCHED(osh, lb)       ({BCM_REFERENCE(osh); lb_set_pktfetched(LBP(lb));})
#define PKTRESETPKTFETCHED(osh, lb)     ({BCM_REFERENCE(osh); lb_reset_pktfetched(LBP(lb));})
#define PKTSETFIFO0INT(osh, lb)		({BCM_REFERENCE(osh); LBFP(lb)->flist.finfo[LB_FRAG_CTX]. \
					ctx.lfrag_flags |= LB_FIFO0_INT;})
#define PKTSETFIFO1INT(osh, lb)		({BCM_REFERENCE(osh); LBFP(lb)->flist.finfo[LB_FRAG_CTX]. \
					ctx.lfrag_flags |= LB_FIFO1_INT;})

#define PKTRESETFIFO0INT(osh, lb)		({BCM_REFERENCE(osh); (LBFP(lb)->flist.finfo \
				[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_FIFO0_INT);})
#define PKTRESETFIFO1INT(osh, lb)		({BCM_REFERENCE(osh); (LBFP(lb)->flist.finfo \
					[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_FIFO1_INT);})

#define PKTISFIFO0INT(osh, lb)		({BCM_REFERENCE(osh); ((LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags & LB_FIFO0_INT) ? 1 : 0);})
#define PKTISFIFO1INT(osh, lb)		({BCM_REFERENCE(osh); ((LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags & LB_FIFO1_INT) ? 1 : 0);})

#define PKTISHDRCONVTD(osh, lb)		({BCM_REFERENCE(osh); ((LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags & LB_HDR_CONVERTED) ? 1 : 0);})

#define PKTRESETHDRCONVTD(osh, lb)		({BCM_REFERENCE(osh); (LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_HDR_CONVERTED);})
#define PKTSETHDRCONVTD(osh, lb)	({BCM_REFERENCE(osh); LBFP(lb)->flist.finfo[LB_FRAG_CTX]. \
			ctx.lfrag_flags |= LB_HDR_CONVERTED;})

#define PKTISTXSPROCESSED(osh, lb)	({BCM_REFERENCE(osh); ((LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags & LB_TXS_PROCESSED) ? 1 : 0);})

#define PKTRESETTXSPROCESSED(osh, lb)	({BCM_REFERENCE(osh); LBFP(lb)->flist.finfo \
			[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_TXS_PROCESSED;})

#define PKTSETTXSPROCESSED(osh, lb)	({BCM_REFERENCE(osh); LBFP(lb)->flist.finfo \
				[LB_FRAG_CTX].ctx.lfrag_flags |= LB_TXS_PROCESSED;})

/* host TX frag Macros */
#ifdef HOST_HDR_FETCH
#define PKTREUSED11BUF(osh, orig, new) \
	({BCM_REFERENCE(osh); lbuf_reuse_d11_buf((struct lbuf *)orig, (struct lbuf *)new);})

#define PKTFRAGSETRA(osh, lb, RA) \
	({BCM_REFERENCE(osh); lb_frag_cache_RA(LBFP(lb), RA);})
#define PKTFRAGRA(osh, lb) \
	({BCM_REFERENCE(osh); lb_frag_RA(LBFP(lb));})

#define PKTFRAGSETBAND(osh, lb, id) \
	({BCM_REFERENCE(osh); \
	lb_frag_set_bandidx(LBFP(lb), (id));})
#define PKTFRAGBAND(osh, lb) \
	({BCM_REFERENCE(osh); \
	lb_frag_bandidx(LBFP(lb));})

#define PKTSETFRAGEPOCH(osh, lb, epoch) ({BCM_REFERENCE(osh); \
	(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_TXHDR_EPOCH_MASK); \
	(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.lfrag_flags |= \
	((epoch) << LB_TXHDR_EPOCH_SHIFT));})
#define PKTFRAGEPOCH(osh, lb) ({BCM_REFERENCE(osh); \
	((LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.lfrag_flags & LB_TXHDR_EPOCH_MASK) >> \
	LB_TXHDR_EPOCH_SHIFT); })

#define PKTISHDRINHOST(osh, lb)	({BCM_REFERENCE(osh); ((LBFP(lb)->flist.finfo \
	[LB_FRAG_CTX].ctx.lfrag_flags & LB_TXHDR_IN_HOST) ? 1 : 0);})

#define __PKTISHDRINHOST(osh, lb)	\
	(lb && PKTISTXFRAG(osh, lb) && PKTISHDRINHOST(osh, lb))
#define PKTRESETHDRINHOST(osh, lb) \
	({ \
		BCM_REFERENCE(osh); \
		if (__PKTISHDRINHOST(osh, lb)) { \
			(LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.lfrag_flags &= ~LB_TXHDR_IN_HOST); \
		} else {			\
			BCM_REFERENCE(lb);	\
		}				\
	})
#define PKTSETHDRINHOST(osh, lb) \
	({ \
		BCM_REFERENCE(osh);		\
		if (PKTISTXFRAG(osh, lb)) { 	\
			LBFP(lb)->flist.finfo[LB_FRAG_CTX]. ctx.lfrag_flags |= LB_TXHDR_IN_HOST; \
		} else { 			\
			BCM_REFERENCE(lb); 	\
		}				\
	})
#define PKTSETHOSTREMAP(osh, lb) \
	({\
		void* host_addr; \
		if (__PKTISHDRINHOST(osh, lb)) { \
			lbuf_dhdhdr_memory_remap_extension(lb, &host_addr); \
			PKTSETBUF(osh, lb, host_addr, PKTLEN(osh, lb)); \
			PKTRESETHDRINHOST(osh, lb); \
		} \
	})

#else /* HOST_HDR_FETCH */
#define PKTISHDRINHOST(osh, lb)		(0)
#define __PKTISHDRINHOST(osh, lb) 	(0)
#endif /* HOST_HDR_FETCH */

#ifdef AMSDU_FRAG_OPT
#define PKTSETMFRAGPKTID(osh, lb, idx, pktid) \
	({BCM_REFERENCE(osh); \
	 lb_set_multi_frag_pktid(LBFP(lb), (idx), (pktid));});
#define PKTMFRAGPKTID(osh, lb, idx) \
	({BCM_REFERENCE(osh); \
	lb_multi_frag_pktid(LBFP(lb), (idx));})

#define PKTSETMFRAGFETCHIDX(osh, lb, idx, fetchidx) \
	({BCM_REFERENCE(osh); \
	 lb_set_multi_frag_fetchidx(LBFP(lb), (idx), (fetchidx));})

#define PKTMFRAGFETCHIDX(osh, lb, idx) \
	({BCM_REFERENCE(osh); \
	lb_multi_frag_fetchidx(LBFP(lb), (idx));})
#endif /* AMSDU_FRAG_OPT */

/* TX FRAG */
#define PKTISTXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_is_txfrag(LBP(lb));})
#define PKTSETTXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_set_txfrag(LBP(lb));})
#define PKTRESETTXFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_reset_txfrag(LBP(lb));})

/* Need Rx completion used for AMPDU reordering */
#define PKTNEEDRXCPL(osh, lb)		({BCM_REFERENCE(osh); lb_need_rxcpl(LBP(lb));})
#define PKTSETNORXCPL(osh, lb)		({BCM_REFERENCE(osh); lb_set_norxcpl(LBP(lb));})
#define PKTRESETNORXCPL(osh, lb)	({BCM_REFERENCE(osh); lb_clr_norxcpl(LBP(lb));})

/* flow read index storage access */
#define PKTFRAGRINGINDEX(osh, lb)	LBFP(lb)->flist.ring_idx
#define PKTFRAGSETRINGINDEX(osh, lb, ix) \
	(LBFP(lb)->flist.ring_idx = (ix))

#define PKTISFRAG(osh, lb)		({BCM_REFERENCE(osh); lb_is_frag(LBP(lb));})
#define PKTPARAMS(osh, lb, len, ix, ft, fbuf, lo, hi) \
	hnd_pkt_params(osh, &(lb), &(len), &(ix), &(ft), &(fbuf), &(lo), &(hi))

#define PKTFRAGISCHAINED(osh, i)	({BCM_REFERENCE(osh); (i > LB_FRAG_MAX);})
/* TRIM Tail bytes fomr lfrag */
#define PKTFRAG_TRIM_TAILBYTES(osh, p, len, type) \
	hnd_pkt_lfrag_trim_tailbytes(osh, p, len, type)

/* packet has metadata */
#define PKTHASMETADATA(osh, lb) \
	({BCM_REFERENCE(osh); lb_has_metadata((struct lbuf *)lb);})
#define PKTSETHASMETADATA(osh, lb) \
	({BCM_REFERENCE(osh); lb_set_has_metadata((struct lbuf *)lb);})
#define PKTRESETHASMETADATA(osh, lb)\
	({BCM_REFERENCE(osh); lb_reset_has_metadata((struct lbuf *)lb);})

#ifdef BCM_DHDHDR
#define PKTSETBUF(osh, lb, buf, n) \
	({BCM_REFERENCE(osh); lb_set_buf((struct lbuf *)lb, buf, n);})
#define PKTHEAD(osh, lb) \
	({BCM_REFERENCE(osh); lb_head(LBP(lb));})
#define PKTFRAGSETTXSTATUS(osh, lb, txs) \
	({BCM_REFERENCE(osh); \
	lb_frag_set_txstatus(LBFP(lb), txs); })
#define PKTFRAGTXSTATUS(osh, lb)	(lb_frag_txstatus(LBFP(lb)))
#define PKTSETWLFCSEQ(osh, lb, seq) ({BCM_REFERENCE(osh); ((LBP(lb)->frameid_fcseq) = (seq));})
#define PKTWLFCSEQ(osh, lb)  ((LBP(lb)->frameid_fcseq))
#define PKTFRAGFCTLV(osh, lb) \
	({BCM_REFERENCE(osh); lb_frag_fc_tlv(LBFP(lb));})
#endif /* BCM_DHDHDR */
/*
 * Given two lfrag lb0, lb1 which are AMSDU linked with next pointers
 * Copy host info from lb1 to multi frag section of lb0.
 * End goal is to retain just single lfrag with two MSDU info
 */
#define PKTFRAGCOPY(osh, lb0, lb1) \
	({ \
		BCM_REFERENCE(osh); \
		PKTSETFRAGDATA_LO(osh, (lb0), LB_FRAG2, (PKTFRAGDATA_LO(osh, (lb1), LB_FRAG1))); \
		PKTSETFRAGDATA_HI(osh, (lb0), LB_FRAG2, (PKTFRAGDATA_HI(osh, (lb1), LB_FRAG1))); \
		PKTSETFRAGLEN(osh, (lb0), LB_FRAG2, (PKTFRAGLEN(osh, (lb1), LB_FRAG1))); \
		PKTSETFRAGTOTNUM(osh, (lb0), LB_FRAG2); \
		PKTSETFRAGTOTLEN(osh, (lb0), (PKTFRAGLEN(osh, (lb0), LB_FRAG1) + \
			PKTFRAGLEN(osh, (lb1), LB_FRAG1))); \
		PKTSETMFRAGPKTID(osh, (lb0), LB_FRAG2, PKTFRAGPKTID(osh, (lb1))); \
		PKTSETMFRAGFETCHIDX(osh, (lb0), LB_FRAG2, PKTFRAGRINGINDEX(osh, (lb1))); \
	})
#define PKTISMULTIFRAG(osh, lb)  (PKTFRAGTOTNUM(osh, (lb)) >= (LB_FRAG2))

#define PKTSETFRAMEID(lb, id)		((LBP(lb)->frameid_fcseq) = (id))
#define PKTGETFRAMEID(lb)		((LBP(lb)->frameid_fcseq))

#ifdef PKTC_DONGLE
#define	PKTCSETATTR(s, f, p, b)	BCM_REFERENCE(s)
#define	PKTCCLRATTR(s)		BCM_REFERENCE(s)
#define	PKTCGETATTR(s)		({BCM_REFERENCE(s); 0;})
#define	PKTCCNT(lb)		({BCM_REFERENCE(lb); 0;})
#define	PKTCLEN(lb)		({BCM_REFERENCE(lb); 0;})
#define	PKTCGETFLAGS(lb)	({BCM_REFERENCE(lb); 0;})
#define	PKTCSETFLAGS(lb, f)	BCM_REFERENCE(lb)
#define	PKTCCLRFLAGS(lb)	BCM_REFERENCE(lb)
#define	PKTCFLAGS(lb)		({BCM_REFERENCE(lb); 0;})
#define	PKTCSETCNT(lb, c)	BCM_REFERENCE(lb)
#define	PKTCINCRCNT(lb)		BCM_REFERENCE(lb)
#define	PKTCADDCNT(lb, c)	BCM_REFERENCE(lb)
#define	PKTCSETLEN(lb, l)	BCM_REFERENCE(lb)
#define	PKTCADDLEN(lb, l)	BCM_REFERENCE(lb)
#define	PKTCSETFLAG(lb, fb)	BCM_REFERENCE(lb)
#define	PKTCCLRFLAG(lb, fb)	BCM_REFERENCE(lb)
#define	PKTCLINK(lb)		PKTLINK(lb)
#define	PKTSETCLINK(lb, x)	PKTSETLINK(lb, x)
#define	FOREACH_CHAINED_PKT(lb, nlb) \
	for (; (lb) != NULL; (lb) = (nlb)) \
		if ((nlb) = PKTCLINK(lb), PKTSETCLINK((lb), NULL), 1)
#define	PKTCFREE(osh, lb, send) \
	do {			 \
		void *nlb;	 \
		ASSERT((lb) != NULL);	   \
		FOREACH_CHAINED_PKT((lb), nlb) {	\
			PKTFREE((osh), (struct lbuf *)(lb), (send));	\
		}					\
	} while (0)
#define PKTCENQTAIL(h, t, p) \
	do {		     \
		if ((t) == NULL) {		\
			(h) = (t) = (p);	\
		} else {			\
			PKTSETCLINK((t), (p));	\
			(t) = (p);		\
		}				\
	} while (0)
#endif /* PKTC_DONGLE */

#ifdef PKTC_TX_DONGLE
#define	PKTSETCHAINED(o, lb)	({BCM_REFERENCE(o); lb_setchained(LBP(lb));})
#define	PKTISCHAINED(lb)	lb_ischained(LBP(lb))
#define	PKTCLRCHAINED(o, lb)	({BCM_REFERENCE(o); lb_clearchained(LBP(lb));})
#endif /* PKTC_TX_DONGLE */

extern void * hnd_pkt_frmnative(osl_t *osh, struct lbuf *lb);
extern struct lbuf * hnd_pkt_tonative(osl_t *osh, void *p);
extern void * hnd_pkt_alloc(osl_t *osh, uint len, enum lbuf_type lbuf_type);
extern void hnd_pkt_free(osl_t *osh, void *p, bool send);
extern void * hnd_pkt_dup(osl_t *osh, void *p);
extern void * hnd_pkt_clone(osl_t *osh, void *p, int offset, int len);
extern void * hnd_pkt_shrink(osl_t *osh, void *p);
extern uchar * hnd_pkt_params(osl_t *osh, void **p, uint32 *len, uint32 *fragix,
	uint16 *ftot, uint8* fbuf, uint32 *lo_addr, uint32 *hi_addr);
extern void hnd_pkt_lfrag_trim_tailbytes(osl_t * osh, void* p, uint16 len, uint8 type);

#endif /* _hnd_pkt_h_ */
