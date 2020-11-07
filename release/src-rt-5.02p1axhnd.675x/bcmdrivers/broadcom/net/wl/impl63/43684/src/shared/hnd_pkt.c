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
 * $Id: hnd_pkt.c 766147 2018-07-25 18:27:32Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <hnd_lbuf.h>
#include <hnd_pktpool.h>
#include <hnd_pkt.h>
#include <bcmutils.h>
#ifdef BCMHWA
#include <hwa_export.h>
#endif // endif

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
#define LB_ALLOC_HEADER(data, len, lbuf_type)	lb_alloc_header(data, len, lbuf_type, CALL_SITE)
#define LB_ALLOC(len, type)			lb_alloc(len, type, CALL_SITE)
#define LB_CLONE(p, offset, len)		lb_clone(p, offset, len, CALL_SITE)
#define LB_DUP(p)				lb_dup(p, CALL_SITE)
#define LB_SHRINK(p)				lb_shrink(p, CALL_SITE)
#else
#define LB_ALLOC_HEADER(data, len, lbuf_type)	lb_alloc_header(data, len, lbuf_type)
#define LB_ALLOC(len, type)			lb_alloc(len, type)
#define LB_CLONE(p, offset, len)		lb_clone(p, offset, len)
#define LB_DUP(p)				lb_dup(p)
#define LB_SHRINK(p)				lb_shrink(p)
#endif // endif

#ifdef BCMPKTPOOL
static uint32 pktget_failed_but_alloced_by_pktpool = 0;
static uint32 pktget_failed_not_alloced_by_pktpool = 0;

/**
 * Allocates a packet from dongle memory, if this fails it attempts to get the packet from the
 * caller specified packet pool.
 *
 * @param osh   OS specific handle
 * @param len   length in [bytes] of packet fragment to allocate in dongle memory
 * @param type  e.g. 'lbuf_frag'
 */
void *
hnd_pkt_frag_get(osl_t *osh, uint len, enum lbuf_type type)
{
	void *pkt = hnd_pkt_alloc(osh, len, type);

#if defined(BCMHWA) && defined(HWA_TXPOST_BUILD)
	printf("XXX: %s(%d): FIXME, why do we come to here?\n", __FUNCTION__, __LINE__);
	ASSERT(0);
#endif // endif

	if (pkt == NULL) {
		switch (type) {
		case lbuf_basic:
			if (POOL_ENAB(SHARED_POOL) && (len <= pktpool_max_pkt_bytes(SHARED_POOL)))
				pkt = pktpool_get(SHARED_POOL);
			break;
		case lbuf_frag: /* support for tx fragments in host memory */
#ifdef BCMFRAGPOOL
			if (POOL_ENAB(SHARED_FRAG_POOL) &&
			    (len <= pktpool_max_pkt_bytes(SHARED_FRAG_POOL)))
				pkt = pktpool_get(SHARED_FRAG_POOL);
#endif // endif
			break;
		case lbuf_rxfrag: /* support for rx fragments in host memory */
#ifdef BCMRXFRAGPOOL
			if (POOL_ENAB(SHARED_RXFRAG_POOL) &&
			    (len <= pktpool_tot_pkts(SHARED_RXFRAG_POOL)))
				pkt = pktpool_get(SHARED_RXFRAG_POOL);
#endif // endif
			break;
		case lbuf_mgmt_tx:
			/* Should not be here */
			ASSERT(0);
			break;
		}

		if (pkt)
			pktget_failed_but_alloced_by_pktpool++;
		else
			pktget_failed_not_alloced_by_pktpool++;
	}

	return pkt;
}

/**
 * Allocates an empty packet in dongle memory. When this fails, it attempts to get a packet from the
 * 'shared' packet pool.
 *
 * @param osh   OS specific handle
 * @param len   length in [bytes] of packet buffer to allocate in dongle memory
 */
void *
hnd_pkt_get_header(osl_t *osh, void *data, uint len, enum lbuf_type lbuf_type)
{
	void *pkt;

	pkt = (void *)LB_ALLOC_HEADER(data, len, lbuf_type);

	if (pkt)
		osh->cmn->pktalloced++;
	return pkt;
}

/**
 * Allocates an empty packet in dongle memory. When this fails, it attempts to get a packet from the
 * 'shared' packet pool.
 *
 * @param osh   OS specific handle
 * @param len   length in [bytes] of packet buffer to allocate in dongle memory
 */
void *
hnd_pkt_get(osl_t *osh, uint len, bool send)
{
	void *pkt;
	enum lbuf_type type = lbuf_basic;

#if defined(BCMHWA) && defined(HWA_TXFIFO_BUILD)
	if (send) {
		type = lbuf_mgmt_tx;
	}
#endif // endif

	pkt = hnd_pkt_alloc(osh, len, type);

	if (pkt == NULL && type == lbuf_basic) {
		if (POOL_ENAB(SHARED_POOL) && (len <= pktpool_max_pkt_bytes(SHARED_POOL))) {
			pkt = pktpool_get(SHARED_POOL);
			if (pkt) {
				PKTSETLEN(osh, pkt, len);
				pktget_failed_but_alloced_by_pktpool++;
			} else {
				pktget_failed_not_alloced_by_pktpool++;
			}
		}
	}

	return pkt;
}
#endif /* BCMPKTPOOL */

/**
 * Allocates an empty packet in dongle memory.
 *
 * @param osh   OS specific handle
 * @param len   length in [bytes] of packet buffer to allocate in dongle memory
 * @param type  e.g. 'lbuf_frag'
 */
void *
hnd_pkt_alloc(osl_t *osh, uint len, enum lbuf_type type)
{
	void *pkt;

#if defined(MEM_LOWATLIMIT)
	if ((OSL_MEM_AVAIL() < MEM_LOWATLIMIT) && len >= PKTBUFSZ) {
		return (void *)NULL;
	}
#endif // endif
	pkt = (void *)LB_ALLOC(len, type);

	if (pkt)
		osh->cmn->pktalloced++;
	return pkt;
}

void
hnd_pkt_free(osl_t *osh, void* p, bool send)
{
	struct lbuf *nlbuf;

	if (send) {
		if (osh->tx_fn) /* set using PKTFREESETCB() */
			osh->tx_fn(osh->tx_ctx, p, 0);
	} else {
		if (osh->rx_fn) /* set using PKTFREESETRXCB() */ {
			for (nlbuf = (struct lbuf *)p; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
				if (lb_pool(nlbuf)) {
					osh->rx_fn(osh->rx_ctx, nlbuf);
				}
			}
		}

	}

	for (nlbuf = (struct lbuf *)p; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
#ifdef STS_FIFO_RXEN
		if (!PKTISTXFRAG(osh, nlbuf) && (stsbuf_free_cb_fn != NULL)) {
			stsbuf_free_cb_fn(stsbuf_free_cb_ctx, nlbuf);
		}
#endif /* STS_FIFO_RXEN */
		if (!lb_pool(nlbuf) &&
#if defined(BCMHWA) && defined(HWA_PKT_MACRO)
			(!PKTISHWAPKT(osh, nlbuf) || PKTISMGMTTXPKT(osh, nlbuf)) &&
#endif // endif
			TRUE) {
			ASSERT(osh->cmn->pktalloced > 0);
			osh->cmn->pktalloced--;
		}
	}

	lb_free((struct lbuf *)p);
}

void *
hnd_pkt_clone(osl_t *osh, void *p, int offset, int len)
{
	void *pkt;

	pkt = (void *)LB_CLONE(p, offset, len);

	if (pkt)
		osh->cmn->pktalloced++;

	return pkt;
}

void *
hnd_pkt_dup(osl_t *osh, void *p)
{
	void *pkt;

	if ((pkt = (void *)LB_DUP((struct lbuf *)p)))
		osh->cmn->pktalloced++;

	return pkt;
}

void *
hnd_pkt_frmnative(osl_t *osh, struct lbuf *lbuf)
{
	struct lbuf *nlbuf;

	for (nlbuf = lbuf; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
		if (!lb_pool(nlbuf) &&
#if defined(BCMHWA) && defined(HWA_PKT_MACRO)
			!PKTISHWAPKT(osh, nlbuf) &&
#endif // endif
			TRUE) {
			osh->cmn->pktalloced++;
		}
	}

	return ((void *)lbuf);
}

struct lbuf *
hnd_pkt_tonative(osl_t *osh, void *p)
{
	struct lbuf *nlbuf;

	for (nlbuf = (struct lbuf *)p; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
		if (!lb_pool(nlbuf) &&
#if defined(BCMHWA) && defined(HWA_PKT_MACRO)
			!PKTISHWAPKT(osh, nlbuf) &&
#endif // endif
			TRUE) {
			ASSERT(osh->cmn->pktalloced > 0);
			osh->cmn->pktalloced--;
		}
	}

	return ((struct lbuf *)p);
}

/**
 * XXX:
 * Should be used at the dma_rx_getnextp, useful for cases where
 * DMA is given a bigger buffer but the actual packet data
 * len is small. if this is used with in the driver, there are lots of error cases one need to
 * deal with..(pktcallbacks, stored packetcallback arguments, pkttag info etc)
*/
void *
hnd_pkt_shrink(osl_t *osh, void *p)
{
	void *pkt;

	pkt = (void *)LB_SHRINK((struct lbuf *)p);
	if (pkt != p)
		osh->cmn->pktalloced++;

	return pkt;
}

/**
 * Util function for lfrags and legacy lbufs to return individual fragment pointers and length.
 * Takes in pkt pointer and frag idx as inputs. Fragidx will be updated by this function itself.
 */
uint8 *
hnd_pkt_params(osl_t * osh, void **pkt, uint32 *len, uint32 *fragix, uint16 *fragtot, uint8* fbuf,
	uint32 *lo_data, uint32 *hi_data)
{
	uint8 * data;
	struct lbuf * lbuf = *(struct lbuf **)pkt;

	*fbuf = 0;
	*hi_data = 0;

	if (lbuf != NULL) {
		if (lb_is_frag(lbuf)) {	/* Lbuf with fraglist walk */
			uint32 ix = *fragix;

			*fragtot = PKTFRAGTOTNUM(osh, lbuf);

			if (ix != 0) {
				*lo_data = PKTFRAGDATA_LO(osh, lbuf, ix);
				*hi_data = PKTFRAGDATA_HI(osh, lbuf, ix);
				data = (uint8 *)lo_data;
				*fbuf = 1;
				*len = PKTFRAGLEN(osh, lbuf, ix);
				if (ix == PKTFRAGTOTNUM(osh, lbuf)) {	/* last frag */
					*fragix = 1U;		/* skip inlined data in next */
					*pkt = PKTNEXT(osh, lbuf);	/* next lbuf */
				} else {
					*fragix = ix + 1;
				}
			} else { /* ix == 0 */
				data = PKTDATA(osh, lbuf);
				*len = PKTLEN(osh, lbuf);
				*fragix = 1U;
			}
		} else {
			*fragtot = 0;
			data = PKTDATA(osh, lbuf);
			*len = PKTLEN(osh, lbuf);
			*pkt = PKTNEXT(osh, lbuf);
		}
		return data;
	}
	return NULL;
}
