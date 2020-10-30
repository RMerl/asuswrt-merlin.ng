/*
 * HND generic packet pool operation primitives
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hnd_pktpool.c 787796 2020-06-11 23:04:24Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <hnd_pktpool.h>
#ifdef BCMRESVFRAGPOOL
#include <hnd_resvpool.h>
#endif /* BCMRESVFRAGPOOL */
#ifdef BCMFRWDPOOLREORG
#include <hnd_poolreorg.h>
#endif /* BCMFRWDPOOLREORG */

#if defined(DONGLEBUILD) && defined(SRMEM)
#include <hndsrmem.h>
#endif /* DONGLEBUILD && SRMEM */
#if defined(DONGLEBUILD)
#include <d11_cfg.h>
#endif // endif

#if defined(DONGLEBUILD) && defined(BCMHWA)
#include <hwa_export.h>
#endif // endif

/* mutex macros for thread safe */
#ifdef HND_PKTPOOL_THREAD_SAFE
#define HND_PKTPOOL_MUTEX_CREATE(name, mutex)	osl_ext_mutex_create(name, mutex)
#define HND_PKTPOOL_MUTEX_DELETE(mutex)		osl_ext_mutex_delete(mutex)
#define HND_PKTPOOL_MUTEX_ACQUIRE(mutex, msec)	osl_ext_mutex_acquire(mutex, msec)
#define HND_PKTPOOL_MUTEX_RELEASE(mutex)	osl_ext_mutex_release(mutex)
#else
#define HND_PKTPOOL_MUTEX_CREATE(name, mutex)	OSL_EXT_SUCCESS
#define HND_PKTPOOL_MUTEX_DELETE(mutex)		OSL_EXT_SUCCESS
#define HND_PKTPOOL_MUTEX_ACQUIRE(mutex, msec)	OSL_EXT_SUCCESS
#define HND_PKTPOOL_MUTEX_RELEASE(mutex)	OSL_EXT_SUCCESS
#endif // endif

/* Registry size is one larger than max pools, as slot #0 is reserved */
#define PKTPOOLREG_RSVD_ID				(0U)
#define PKTPOOLREG_RSVD_PTR				(POOLPTR(0xdeaddead))
#define PKTPOOLREG_FREE_PTR				(POOLPTR(NULL))

#define PKTPOOL_REGISTRY_SET(id, pp)	(pktpool_registry_set((id), (pp)))
#define PKTPOOL_REGISTRY_CMP(id, pp)	(pktpool_registry_cmp((id), (pp)))

/* Tag a registry entry as free for use */
#define PKTPOOL_REGISTRY_CLR(id)		\
		PKTPOOL_REGISTRY_SET((id), PKTPOOLREG_FREE_PTR)
#define PKTPOOL_REGISTRY_ISCLR(id)		\
		(PKTPOOL_REGISTRY_CMP((id), PKTPOOLREG_FREE_PTR))

/* Tag registry entry 0 as reserved */
#define PKTPOOL_REGISTRY_RSV()			\
		PKTPOOL_REGISTRY_SET(PKTPOOLREG_RSVD_ID, PKTPOOLREG_RSVD_PTR)
#define PKTPOOL_REGISTRY_ISRSVD()		\
		(PKTPOOL_REGISTRY_CMP(PKTPOOLREG_RSVD_ID, PKTPOOLREG_RSVD_PTR))

/* Walk all un-reserved entries in registry */
#define PKTPOOL_REGISTRY_FOREACH(id)	\
		for ((id) = 1U; (id) <= pktpools_max; (id)++)

enum pktpool_empty_cb_state {
	EMPTYCB_ENABLED = 0,	/* Enable callback when new packets are added to pool */
	EMPTYCB_DISABLED,	/* Disable callback when new packets are added to pool */
	EMPTYCB_SKIPPED		/* Packet was added to pool when callback was disabled */
};

uint32 pktpools_max = 0U; /* maximum number of pools that may be initialized */
pktpool_t *pktpools_registry[PKTPOOL_MAXIMUM_ID + 1]; /* Pktpool registry */

/* Register/Deregister a pktpool with registry during pktpool_init/deinit */
static int pktpool_register(pktpool_t * poolptr);
static int pktpool_deregister(pktpool_t * poolptr);

/** add declaration */
static void pktpool_avail_notify(pktpool_t *pktp);

/** accessor functions required when ROMming this file, forced into RAM */

pktpool_t *
BCMRAMFN(get_pktpools_registry)(int id)
{
	return pktpools_registry[id];
}

static void
BCMRAMFN(pktpool_registry_set)(int id, pktpool_t *pp)
{
	pktpools_registry[id] = pp;
}

static bool
BCMRAMFN(pktpool_registry_cmp)(int id, pktpool_t *pp)
{
	return pktpools_registry[id] == pp;
}

/** Constructs a pool registry to serve a maximum of total_pools */
int
BCMATTACHFN(pktpool_attach)(osl_t *osh, uint32 total_pools)
{
	uint32 poolid;
	BCM_REFERENCE(osh);

	if (pktpools_max != 0U) {
		return BCME_ERROR;
	}

	ASSERT(total_pools <= PKTPOOL_MAXIMUM_ID);

	/* Initialize registry: reserve slot#0 and tag others as free */
	PKTPOOL_REGISTRY_RSV();		/* reserve slot#0 */

	PKTPOOL_REGISTRY_FOREACH(poolid) {	/* tag all unreserved entries as free */
		PKTPOOL_REGISTRY_CLR(poolid);
	}

	pktpools_max = total_pools;

	return (int)pktpools_max;
}

/** Destructs the pool registry. Ascertain all pools were first de-inited */
int
BCMATTACHFN(pktpool_dettach)(osl_t *osh)
{
	uint32 poolid;
	BCM_REFERENCE(osh);

	if (pktpools_max == 0U) {
		return BCME_OK;
	}

	/* Ascertain that no pools are still registered */
	ASSERT(PKTPOOL_REGISTRY_ISRSVD()); /* assert reserved slot */

	PKTPOOL_REGISTRY_FOREACH(poolid) {	/* ascertain all others are free */
		ASSERT(PKTPOOL_REGISTRY_ISCLR(poolid));
	}

	pktpools_max = 0U; /* restore boot state */

	return BCME_OK;
}

/** Registers a pool in a free slot; returns the registry slot index */
static int
BCMATTACHFN(pktpool_register)(pktpool_t * poolptr)
{
	uint32 poolid;

	if (pktpools_max == 0U) {
		return PKTPOOL_INVALID_ID; /* registry has not yet been constructed */
	}

	ASSERT(pktpools_max != 0U);

	/* find an empty slot in pktpools_registry */
	PKTPOOL_REGISTRY_FOREACH(poolid) {
		if (PKTPOOL_REGISTRY_ISCLR(poolid)) {
			PKTPOOL_REGISTRY_SET(poolid, POOLPTR(poolptr)); /* register pool */
			return (int)poolid; /* return pool ID */
		}
	} /* FOREACH */

	return PKTPOOL_INVALID_ID;	/* error: registry is full */
}

/** Deregisters a pktpool, given the pool pointer; tag slot as free */
static int
BCMATTACHFN(pktpool_deregister)(pktpool_t * poolptr)
{
	uint32 poolid;

	ASSERT(POOLPTR(poolptr) != POOLPTR(NULL));

	poolid = POOLID(poolptr);
	ASSERT(poolid <= pktpools_max);

	/* Asertain that a previously registered poolptr is being de-registered */
	if (PKTPOOL_REGISTRY_CMP(poolid, POOLPTR(poolptr))) {
		PKTPOOL_REGISTRY_CLR(poolid); /* mark as free */
	} else {
		ASSERT(0);
		return BCME_ERROR; /* mismatch in registry */
	}

	return BCME_OK;
}

/**
 * pktpool_init:
 * User provides a pktpool_t structure and specifies the number of packets to
 * be pre-filled into the pool (n_pkts).
 * pktpool_init first attempts to register the pool and fetch a unique poolid.
 * If registration fails, it is considered an BCME_ERR, caused by either the
 * registry was not pre-created (pktpool_attach) or the registry is full.
 * If registration succeeds, then the requested number of packets will be filled
 * into the pool as part of initialization. In the event that there is no
 * available memory to service the request, then BCME_NOMEM will be returned
 * along with the count of how many packets were successfully allocated.
 * In dongle builds, prior to memory reclaimation, one should limit the number
 * of packets to be allocated during pktpool_init and fill the pool up after
 * reclaim stage.
 *
 * @param n_pkts           Number of packets to be pre-filled into the pool
 * @param max_pkt_bytes   The size of all packets in a pool must be the same. E.g. PKTBUFSZ.
 * @param type            e.g. 'lbuf_frag'
 */
int
BCMATTACHFN(pktpool_init)(osl_t *osh, pktpool_t *pktp, int *n_pkts, int max_pkt_bytes, bool istx,
	uint8 type)
{
	int i, err = BCME_OK;
	int pktplen;
	uint8 pktp_id;

	ASSERT(pktp != NULL);
	ASSERT(osh != NULL);
	ASSERT(n_pkts != NULL);

	pktplen = *n_pkts;

	bzero(pktp, sizeof(pktpool_t));

	/* assign a unique pktpool id */
	if ((pktp_id = (uint8) pktpool_register(pktp)) == PKTPOOL_INVALID_ID) {
		return BCME_ERROR;
	}
	POOLSETID(pktp, pktp_id);

	pktp->inited = TRUE;
	pktp->istx = istx ? TRUE : FALSE;
	pktp->max_pkt_bytes = (uint16)max_pkt_bytes;
	pktp->type = type;
	pktp->freelist_tail = NULL;

	if (HND_PKTPOOL_MUTEX_CREATE("pktpool", &pktp->mutex) != OSL_EXT_SUCCESS) {
		return BCME_ERROR;
	}

	pktp->maxlen = PKTPOOL_LEN_MAX;
	pktplen = LIMIT_TO_MAX(pktplen, pktp->maxlen);

	for (i = 0; i < pktplen; i++) {
		void *p;
#ifdef _RTE_
		/* For rte builds, use PKTALLOC rather than PKTGET
		 * Avoid same pkts being dequed and enqued to pool
		 * when allocation fails.
		 */
		p = PKTALLOC(osh, max_pkt_bytes, type);
#else
		p = PKTGET(osh, max_pkt_bytes, TRUE);
#endif // endif

		if (p == NULL) {
			/* Not able to allocate all requested pkts
			 * so just return what was actually allocated
			 * We can add to the pool later
			 */
			if (pktp->freelist == NULL) /* pktpool free list is empty */
				err = BCME_NOMEM;

			goto exit;
		}

		PKTSETPOOL(osh, p, TRUE, pktp); /* Tag packet with pool ID */

		PKTSETFREELIST(p, pktp->freelist); /* insert p at head of free list */
		pktp->freelist = p;
		if (pktp->freelist_tail == NULL) {
			pktp->freelist_tail = p;
		}

		pktp->avail++;

#ifdef BCMDBG_POOL
		pktp->dbg_q[pktp->dbg_qlen++].p = p;
#endif // endif
	}

exit:
	pktp->n_pkts = pktp->avail;

	*n_pkts = pktp->n_pkts; /* number of packets managed by pool */
	return err;
} /* pktpool_init */

/**
 * pktpool_deinit:
 * Prior to freeing a pktpool, all packets must be first freed into the pktpool.
 * Upon pktpool_deinit, all packets in the free pool will be freed to the heap.
 * An assert is in place to ensure that there are no packets still lingering
 * around. Packets freed to a pool after the deinit will cause a memory
 * corruption as the pktpool_t structure no longer exists.
 */
int
BCMATTACHFN(pktpool_deinit)(osl_t *osh, pktpool_t *pktp)
{
	uint16 freed = 0;

	ASSERT(osh != NULL);
	ASSERT(pktp != NULL);

#ifdef BCMDBG_POOL
	{
		int i;
		for (i = 0; i <= pktp->n_pkts; i++) {
			pktp->dbg_q[i].p = NULL;
		}
	}
#endif // endif

	while (pktp->freelist != NULL) {
		void * p = pktp->freelist;

		pktp->freelist = PKTFREELIST(p); /* unlink head packet from free list */
		PKTSETFREELIST(p, NULL);

		PKTSETPOOL(osh, p, FALSE, NULL); /* clear pool ID tag in pkt */

		PKTFREE(osh, p, pktp->istx); /* free the packet */

		freed++;
		ASSERT(freed <= pktp->n_pkts);
	}

	pktp->freelist_tail = NULL;
	pktp->avail -= freed;
	ASSERT(pktp->avail == 0);

	pktp->n_pkts -= freed;

	pktpool_deregister(pktp); /* release previously acquired unique pool id */
	POOLSETID(pktp, PKTPOOL_INVALID_ID);

	if (HND_PKTPOOL_MUTEX_DELETE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	pktp->inited = FALSE;

	/* Are there still pending pkts? */
	ASSERT(pktp->n_pkts == 0);

	return 0;
}

int
pktpool_fill(osl_t *osh, pktpool_t *pktp, bool minimal)
{
	void *p;
	int err = 0;
	int n_pkts, psize, maxlen;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

#ifndef BCM_DHDHDR
	/* In BCM_DHDHDR shared lfrag pool is inited with zero byte length */
	ASSERT(pktp->max_pkt_bytes != 0);
#endif // endif

	maxlen = pktp->maxlen;
	psize = minimal ? (maxlen >> 2) : maxlen;
	for (n_pkts = (int)pktp->n_pkts; n_pkts < psize; n_pkts++) {

#ifdef _RTE_
		/* For rte builds, use PKTALLOC rather than PKTGET
		 * Avoid same pkts being dequed and enqued to pool when allocation fails.
		 * All pkts in pool have same length.
		 */
		p = PKTALLOC(osh, pktp->max_pkt_bytes, pktp->type);
#else
		p = PKTGET(osh, pktp->n_pkts, TRUE);
#endif // endif

		if (p == NULL) {
			err = BCME_NOMEM;
			break;
		}

		if (pktpool_add(pktp, p) != BCME_OK) {
			PKTFREE(osh, p, FALSE);
			err = BCME_ERROR;
			break;
		}
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	if (pktp->cbcnt) {
		if (pktp->empty == FALSE)
			pktpool_avail_notify(pktp);
	}

	return err;
}

#ifdef BCMPOOLRECLAIM
/* New API to decrease the pkts from pool, but not deinit
*/
uint16
pktpool_reclaim(osl_t *osh, pktpool_t *pktp, uint16 free_cnt)
{
	uint16 freed = 0;

	pktpool_cb_extn_t cb = NULL;
	void *arg = NULL;

	ASSERT(osh != NULL);
	ASSERT(pktp != NULL);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS) {
		return freed;
	}

	if (pktp->avail < free_cnt) {
		free_cnt = pktp->avail;
	}

	if (BCMSPLITRX_ENAB() && (pktp->type == lbuf_rxfrag)) {
		/* If pool is shared rx frag pool, use call back fn to reclaim host address
		 * and Rx cpl ID associated with the pkt.
		 */
		ASSERT(pktp->cbext.cb != NULL);

		cb = pktp->cbext.cb;
		arg = pktp->cbext.arg;

	} else if ((pktp->type == lbuf_basic) && (pktp->rxcplidfn.cb != NULL)) {
		/* If pool is shared rx pool, use call back fn to freeup Rx cpl ID
		 * associated with the pkt.
		 */
		cb = pktp->rxcplidfn.cb;
		arg = pktp->rxcplidfn.arg;
	}

	while ((pktp->freelist != NULL) && (free_cnt)) {
		void * p = pktp->freelist;

		pktp->freelist = PKTFREELIST(p); /* unlink head packet from free list */
		PKTSETFREELIST(p, NULL);

		if (cb != NULL) {
			if (cb(pktp, arg, p, REMOVE_RXCPLID)) {
				PKTSETFREELIST(p, pktp->freelist);
				pktp->freelist = p;
				if (pktp->freelist_tail == NULL) {
					pktp->freelist_tail = p;
				}
				break;
			}
		}

		PKTSETPOOL(osh, p, FALSE, NULL); /* clear pool ID tag in pkt */

		PKTFREE(osh, p, pktp->istx); /* free the packet */

		freed++;
		free_cnt--;
	}

	if (pktp->freelist == NULL) {
		pktp->freelist_tail = NULL;
	}
	pktp->avail -= freed;

	pktp->n_pkts -= freed;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS) {
		return freed;
	}

	return freed;
}
#endif /* #ifdef BCMPOOLRECLAIM */

/* New API to empty the pkts from pool, but not deinit
* NOTE: caller is responsible to ensure,
*	all pkts are available in pool for free; else LEAK !
*/
int
pktpool_empty(osl_t *osh, pktpool_t *pktp)
{
	uint16 freed = 0;

	ASSERT(osh != NULL);
	ASSERT(pktp != NULL);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

#ifdef BCMDBG_POOL
	{
		int i;
		for (i = 0; i <= pktp->n_pkts; i++) {
			pktp->dbg_q[i].p = NULL;
		}
	}
#endif // endif

	while (pktp->freelist != NULL) {
		void * p = pktp->freelist;

		pktp->freelist = PKTFREELIST(p); /* unlink head packet from free list */
		PKTSETFREELIST(p, NULL);

		PKTSETPOOL(osh, p, FALSE, NULL); /* clear pool ID tag in pkt */

		PKTFREE(osh, p, pktp->istx); /* free the packet */

		freed++;
		ASSERT(freed <= pktp->n_pkts);
	}

	pktp->freelist_tail = NULL;
	pktp->avail -= freed;
	ASSERT(pktp->avail == 0);

	pktp->n_pkts -= freed;

	ASSERT(pktp->n_pkts == 0);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

static void *
pktpool_deq(pktpool_t *pktp)
{
	void *p = NULL;

	if (pktp->avail == 0)
		return NULL;

	ASSERT(pktp->freelist != NULL);

	p = pktp->freelist;  /* dequeue packet from head of pktpool free list */
	pktp->freelist = PKTFREELIST(p); /* free list points to next packet */

	if (pktp->freelist == NULL) {
		pktp->freelist_tail = NULL;
	}

#if defined(DONGLEBUILD) && defined(SRMEM)
	if (SRMEM_ENAB()) {
		PKTSRMEM_INC_INUSE(p);
	}
#endif /* DONGLEBUILD && SRMEM */

	PKTSETFREELIST(p, NULL);

	pktp->avail--;

	return p;
}

static void
pktpool_enq(pktpool_t *pktp, void *p)
{
	ASSERT(p != NULL);

	PKTSETFREELIST(p, pktp->freelist); /* insert at head of pktpool free list */
	pktp->freelist = p; /* free list points to newly inserted packet */
	if (pktp->freelist_tail == NULL) {
		pktp->freelist_tail = p;
	}

#if defined(DONGLEBUILD) && defined(SRMEM)
	if (SRMEM_ENAB()) {
		PKTSRMEM_DEC_INUSE(p);
	}
#endif /* DONGLEBUILD && SRMEM */

	pktp->avail++;
	ASSERT(pktp->avail <= pktp->n_pkts);
}

#ifdef DONGLEBUILD
static void
pktpool_enqchain_tail(pktpool_t *pktp, void *head, void *tail, int cnt)
{
	ASSERT((head != NULL) && (tail != NULL));

	/* Insert chain of packets at tail of pktpool free list */
	if (pktp->freelist == NULL) {
		pktp->freelist = head;
	} else {
		PKTSETFREELIST(pktp->freelist_tail, head);
	}
	pktp->freelist_tail = tail;
	PKTSETFREELIST(pktp->freelist_tail, NULL);
	pktp->avail += cnt;

	ASSERT(pktp->avail <= pktp->n_pkts);
}
#endif /* DONGLEBUILD */

/* BCM_DHDHDR cannot be compiled into the ROM */
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)

#if defined(HNDLBUFCOMPACT)
#error "BCM_DHDHDR is exclusive of HNDLBUFCOMPACT."
#endif // endif
#if !defined(WLFC_CONTROL_TO_HOST_DISABLED)
#error "BCM_DHDHDR is exclusive of WLFC_CONTROL_TO_HOST_DISABLED."
#endif // endif

#ifndef D3_BUFFER_LEN
#define D3_BUFFER_LEN	(SHARED_FRAG_POOL_LEN/2)
#endif // endif
#ifndef D11_BUFFER_LEN
#define D11_BUFFER_LEN	(SHARED_FRAG_POOL_LEN/2)
#endif // endif

/* Here are D3_BUFFER (small) and D11_BUFFER (big) pools for split packet context and packet buffer
 * design in order to reduce packet store memory in dongle
 */
lfrag_buf_pool_t *d3_lfrag_buf_pool = NULL;
lfrag_buf_pool_t *d11_lfrag_buf_pool = NULL;

#define LFBUFFREELIST(buf)		(((lfrag_buf_t *)(buf))->freelist)
#define LFBUFSETFREELIST(buf, x)	(((lfrag_buf_t *)(buf))->freelist = ((lfrag_buf_t *)(x)))
#define LFBUFHOMEPOOL(buf)		(((lfrag_buf_t *)(buf))->lfbufp)
#define LFBUFSETHOMEPOOL(buf, p)	(((lfrag_buf_t *)(buf))->lfbufp = (p))
#define D3LFBUF_ETHERHDRSZ	(14)
#define D3LFBUF_SZ		(D3LFBUF_ETHERHDRSZ)

static void *
lfbufpool_deq(lfrag_buf_pool_t *lfbufp)
{
	void *buf = NULL;

	if (lfbufp->avail == 0)
		return NULL;

	ASSERT(lfbufp->freelist != NULL);

	buf = lfbufp->freelist; /* dequeue buffer from head of lfrag_buf_pool_t free list */
	lfbufp->freelist = LFBUFFREELIST(buf); /* free list points to next buffer */
	LFBUFSETHOMEPOOL(buf, lfbufp); /* set home pool */

	lfbufp->avail--;

	return buf;
}

static void
lfbufpool_enq(lfrag_buf_pool_t *lfbufp, void *buf)
{
	ASSERT(buf != NULL);

	LFBUFSETFREELIST(buf, lfbufp->freelist); /* insert at head of lfrag_buf_pool_t free list */
	lfbufp->freelist = buf; /* free list points to newly inserted buffer */

	lfbufp->avail++;
	ASSERT(lfbufp->avail <= lfbufp->n_bufs);
}

/* Initial a lfrag buffer pool and add pplen number of buffer to pool */
int
BCMATTACHFN(lfbufpool_init)(osl_t *osh, lfrag_buf_pool_t *lfbufp, int *n_pkts, int max_buf_bytes)
{
	int i, err = BCME_OK;
	int plen;

	ASSERT(osh != NULL);
	ASSERT(lfbufp != NULL);
	ASSERT(n_pkts != NULL);

	plen = *n_pkts;

	bzero(lfbufp, sizeof(lfrag_buf_pool_t));

	lfbufp->inited = TRUE;
	lfbufp->max_buf_bytes = (uint16)max_buf_bytes;

	if (HND_PKTPOOL_MUTEX_CREATE("lfbufpool", &lfbufp->mutex) != OSL_EXT_SUCCESS) {
		return BCME_ERROR;
	}

	lfbufp->maxlen = PKTPOOL_LEN_MAX;
	plen = LIMIT_TO_MAX(plen, lfbufp->maxlen);

	for (i = 0; i < plen; i++) {
		lfrag_buf_t *buf;

		buf = (lfrag_buf_t *)MALLOC(OSH_NULL, max_buf_bytes + LFBUFSZ);
		if (buf == NULL) {
			/* Not able to allocate all requested pkts
			 * so just return what was actually allocated
			 * We can add to the pool later
			 */
			if (lfbufp->freelist == NULL) /* pktpool free list is empty */
				err = BCME_NOMEM;

			goto exit;
		}
		buf->lfbufp = lfbufp;
		LFBUFSETFREELIST(buf, lfbufp->freelist); /* insert p at head of free list */
		lfbufp->freelist = buf;
		lfbufp->avail++;
	}

exit:
	lfbufp->n_bufs = lfbufp->avail;

	*n_pkts = lfbufp->n_bufs;
	return err;
}

/* De-initial a lfrag buffer pool, free the data in this pool */
int
BCMATTACHFN(lfbufpool_deinit)(osl_t *osh, lfrag_buf_pool_t *lfbufp)
{
	uint16 freed = 0;
	void *buf = NULL;

	ASSERT(osh != NULL);
	ASSERT(lfbufp != NULL);

	if (lfbufp->inited == FALSE)
		return BCME_OK;

	while (lfbufp->freelist != NULL) {
		buf = lfbufp->freelist;
		lfbufp->freelist = LFBUFFREELIST(buf); /* unlink head buffer from free list */
		LFBUFSETFREELIST(buf, NULL);

		MFREE(OSH_NULL, lfbufp, lfbufp->max_buf_bytes);

		freed++;
		ASSERT(freed <= lfbufp->n_bufs);
	}

	lfbufp->avail -= freed;
	ASSERT(lfbufp->avail == 0);

	lfbufp->n_bufs -= freed;

	if (HND_PKTPOOL_MUTEX_DELETE(&lfbufp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	lfbufp->inited = FALSE;

	/* Are there still pending buffer? */
	ASSERT(lfbufp->n_bufs == 0);

	return BCME_OK;
}

/* fill more buffer to the pool */
int
lfbufpool_fill(osl_t *osh, lfrag_buf_pool_t *lfbufp, bool minimal)
{
	lfrag_buf_t *lfbuf;
	int err = BCME_OK;
	int len, psize, maxlen;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&lfbufp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(lfbufp->max_buf_bytes != 0);

	maxlen = lfbufp->maxlen;
	psize = minimal ? (maxlen >> 2) : maxlen;
	for (len = (int)lfbufp->n_bufs; len < psize; len++) {
		lfbuf = (lfrag_buf_t *)MALLOC(osh, lfbufp->max_buf_bytes + LFBUFSZ);
		if (lfbuf == NULL) {
			err = BCME_NOMEM;
			break;
		}
		lfbuf->lfbufp = lfbufp;

		lfbufp->n_bufs++;
		lfbufpool_enq(lfbufp, (void *)lfbuf);
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&lfbufp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

void *
lfbufpool_get(lfrag_buf_pool_t *lfbufp)
{
	uchar *buf;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&lfbufp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	buf = (uchar *)lfbufpool_deq(lfbufp);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&lfbufp->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	/* Shift lfrag_buf header */
	return (void *)(buf ? (buf + LFBUFSZ) : NULL);
}

void
lfbufpool_free(void *buf)
{
	lfrag_buf_t *lfbuf;

	ASSERT(buf != NULL);

	lfbuf = (lfrag_buf_t*)((uchar*)buf - LFBUFSZ);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&LFBUFHOMEPOOL(lfbuf)->mutex, OSL_EXT_TIME_FOREVER) !=
		OSL_EXT_SUCCESS)
		return;

	lfbufpool_enq(LFBUFHOMEPOOL(lfbuf), lfbuf);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&LFBUFHOMEPOOL(lfbuf)->mutex) != OSL_EXT_SUCCESS)
		return;
}

/* Swap D3_BUFFER to use D11_BUFFER */
int
lfbufpool_swap_d11_buf(osl_t *osh, void *p)
{
	void *d3buf_head, *d3buf_data, *d11buf;
	uint16 len, extra_push = 0;
	bool d3buf_alloc = TRUE;

	ASSERT(!PKTHASHEAPBUF(osh, p));

	if (lfbufpool_avail(d11_lfrag_buf_pool) == 0)
		return BCME_ERROR;

	/* The length must be 14B */
	len = PKTLEN(osh, p);
	ASSERT(len == D3LFBUF_ETHERHDRSZ);

	if (len > D3LFBUF_ETHERHDRSZ)
		extra_push = len - D3LFBUF_ETHERHDRSZ;

	if ((d11buf = lfbufpool_get(d11_lfrag_buf_pool)) == NULL)
		return BCME_ERROR;

	/* save d3 buffer head and data pointers */
	d3buf_head = PKTHEAD(osh, p);
	d3buf_data = PKTDATA(osh, p);
	d3buf_alloc = PKTISBUFALLOC(osh, p);

	/* set d11 buffer to lbuf */
	PKTSETBUF(osh, p, d11buf, d11_lfrag_buf_pool->max_buf_bytes);
	PKTSETBUFALLOC(osh, p);

	/* pull for headroom */
	PKTPULL(osh, p, (PKTFRAGSZ - D3LFBUF_ETHERHDRSZ - extra_push));
	PKTSETLEN(osh, p, len);

	/* copy all d3buf data to d11buf */
	bcopy(d3buf_data, PKTDATA(osh, p), len);

	/* free d3 buffer */
	if (d3buf_alloc) {
		lfbufpool_free(d3buf_head);
	}

	return BCME_OK;
}

void
lfbufpool_early_free_buf(osl_t *osh, void *p)
{
	void *buf_head;

	/* save buffer head and data pointers */
	buf_head = PKTHEAD(osh, p);

	/* set buffer to end of lbuffrag and length to 0 */
	PKTSETBUF(osh, p, ((uchar *)p + LBUFFRAGSZ), 0);

	/* free buffer */
	if (PKTISBUFALLOC(osh, p)) {
		if (PKTHASHEAPBUF(OSH_NULL, p)) {
			MFREE(OSH_NULL, PKTHEAD(OSH_NULL, p), MAXPKTDATABUFSZ);
			PKTRESETHEAPBUF(OSH_NULL, p);
		} else {
			lfbufpool_free(buf_head);
		}
		PKTRESETBUFALLOC(osh, p);
	}
}

/* Bind D11_BUFFER */
int
lfbufpool_bind_d11_buf(osl_t *osh, void *p)
{
	void *d11buf;

	ASSERT(PKTDATA(osh, p) == NULL);

	if (lfbufpool_avail(d11_lfrag_buf_pool) == 0)
		return BCME_ERROR;

	if ((d11buf = lfbufpool_get(d11_lfrag_buf_pool)) == NULL)
		return BCME_ERROR;

	/* set d11 buffer to lbuf */
	PKTSETBUF(osh, p, d11buf, d11_lfrag_buf_pool->max_buf_bytes);
	PKTSETBUFALLOC(osh, p);

	return BCME_OK;
}

/* Force lfbufpool_setmaxlen () into RAM as it uses a constant
 * (PKTPOOL_LEN_MAX) that may be changed post tapeout for ROM-based chips.
 */
int
BCMRAMFN(lfbufpool_setmaxlen)(lfrag_buf_pool_t *lfbufp, uint16 maxlen)
{
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&lfbufp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	if (maxlen > PKTPOOL_LEN_MAX)
		maxlen = PKTPOOL_LEN_MAX;

	/* if pool is already beyond maxlen, then just cap it
	 * since we currently do not reduce the pool len
	 * already allocated
	 */
	lfbufp->maxlen = (lfbufp->n_bufs > maxlen) ? lfbufp->n_bufs : maxlen;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&lfbufp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return lfbufp->maxlen;
}
#endif /* BCM_DHDHDR && DONGLEBUILD */

/** utility for registering host addr fill function called from pciedev */
int
/* BCMATTACHFN */
(pktpool_hostaddr_fill_register)(pktpool_t *pktp, pktpool_cb_extn_t cb, void *arg)
{

	ASSERT(cb != NULL);

	ASSERT(pktp->cbext.cb == NULL);
	pktp->cbext.cb = cb;
	pktp->cbext.arg = arg;
	return 0;
}

int
BCMATTACHFN(pktpool_rxcplid_fill_register)(pktpool_t *pktp, pktpool_cb_extn_t cb, void *arg)
{

	ASSERT(cb != NULL);

	if (pktp == NULL)
		return BCME_ERROR;
	ASSERT(pktp->rxcplidfn.cb == NULL);
	pktp->rxcplidfn.cb = cb;
	pktp->rxcplidfn.arg = arg;
	return 0;
}

/** whenever host posts rxbuffer, invoke dma_rxfill from pciedev layer */
void
pktpool_invoke_dmarxfill(pktpool_t *pktp)
{
	ASSERT(pktp->dmarxfill.cb);
	ASSERT(pktp->dmarxfill.arg);

	if (pktp->dmarxfill.cb)
		pktp->dmarxfill.cb(pktp, pktp->dmarxfill.arg);
}

/** Registers callback functions for split rx mode */
int
pkpool_haddr_avail_register_cb(pktpool_t *pktp, pktpool_cb_t cb, void *arg)
{

	ASSERT(cb != NULL);

	pktp->dmarxfill.cb = cb;
	pktp->dmarxfill.arg = arg;

	return 0;
}

/**
 * Registers callback functions.
 * No BCMATTACHFN as it is used in xdc_enable_ep which is not an attach function
 */
int
pktpool_avail_register(pktpool_t *pktp, pktpool_cb_t cb, void *arg)
{
	int err = 0;
	int i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(cb != NULL);

	for (i = 0; i < pktp->cbcnt; i++) {
		ASSERT(pktp->cbs[i].cb != NULL);
		if ((cb == pktp->cbs[i].cb) && (arg == pktp->cbs[i].arg)) {
			pktp->cbs[i].refcnt++;
			goto done;
		}
	}

	i = pktp->cbcnt;
	if (i == PKTPOOL_CB_MAX_AVL) {
		err = BCME_ERROR;
		goto done;
	}

	ASSERT(pktp->cbs[i].cb == NULL);
	pktp->cbs[i].cb = cb;
	pktp->cbs[i].arg = arg;
	pktp->cbs[i].refcnt++;
	pktp->cbcnt++;

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

/* No BCMATTACHFN as it is used in a non-attach function */
int
pktpool_avail_deregister(pktpool_t *pktp, pktpool_cb_t cb, void *arg)
{
	int err = 0;
	int i, k;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS) {
		return BCME_ERROR;
	}

	ASSERT(cb != NULL);

	for (i = 0; i < pktp->cbcnt; i++) {
		ASSERT(pktp->cbs[i].cb != NULL);
		if ((cb == pktp->cbs[i].cb) && (arg == pktp->cbs[i].arg)) {
			pktp->cbs[i].refcnt--;
			if (pktp->cbs[i].refcnt) {
				/* Still there are references to this callback */
				goto done;
			}
			/* Moving any more callbacks to fill the hole */
			for (k = i+1; k < pktp->cbcnt; i++, k++) {
				pktp->cbs[i].cb = pktp->cbs[k].cb;
				pktp->cbs[i].arg = pktp->cbs[k].arg;
				pktp->cbs[i].refcnt = pktp->cbs[k].refcnt;
			}

			/* reset the last callback */
			pktp->cbs[i].cb = NULL;
			pktp->cbs[i].arg = NULL;
			pktp->cbs[i].refcnt = 0;

			pktp->cbcnt--;
			goto done;
		}
	}

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS) {
		return BCME_ERROR;
	}

	return err;
}

/** Registers callback functions */
int
BCMATTACHFN(pktpool_empty_register)(pktpool_t *pktp, pktpool_cb_t cb, void *arg)
{
	int err = 0;
	int i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(cb != NULL);

	i = pktp->ecbcnt;
	if (i == PKTPOOL_CB_MAX) {
		err = BCME_ERROR;
		goto done;
	}

	ASSERT(pktp->ecbs[i].cb == NULL);
	pktp->ecbs[i].cb = cb;
	pktp->ecbs[i].arg = arg;
	pktp->ecbcnt++;

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

/** Calls registered callback functions */
static int
pktpool_empty_notify(pktpool_t *pktp)
{
	int i;

	pktp->empty = TRUE;
	for (i = 0; i < pktp->ecbcnt; i++) {
		ASSERT(pktp->ecbs[i].cb != NULL);
		pktp->ecbs[i].cb(pktp, pktp->ecbs[i].arg);
	}
	pktp->empty = FALSE;

	return 0;
}

#ifdef BCMDBG_POOL
int
pktpool_dbg_register(pktpool_t *pktp, pktpool_cb_t cb, void *arg)
{
	int err = 0;
	int i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(cb);

	i = pktp->dbg_cbcnt;
	if (i == PKTPOOL_CB_MAX) {
		err = BCME_ERROR;
		goto done;
	}

	ASSERT(pktp->dbg_cbs[i].cb == NULL);
	pktp->dbg_cbs[i].cb = cb;
	pktp->dbg_cbs[i].arg = arg;
	pktp->dbg_cbcnt++;

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

int pktpool_dbg_notify(pktpool_t *pktp);

int
pktpool_dbg_notify(pktpool_t *pktp)
{
	int i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	for (i = 0; i < pktp->dbg_cbcnt; i++) {
		ASSERT(pktp->dbg_cbs[i].cb);
		pktp->dbg_cbs[i].cb(pktp, pktp->dbg_cbs[i].arg);
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

int
pktpool_dbg_dump(pktpool_t *pktp)
{
	int i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	printf("pool len=%d maxlen=%d\n",  pktp->dbg_qlen, pktp->maxlen);
	for (i = 0; i < pktp->dbg_qlen; i++) {
		ASSERT(pktp->dbg_q[i].p);
		printf("%d, p: 0x%x dur:%lu us state:%d\n", i,
			pktp->dbg_q[i].p, pktp->dbg_q[i].dur/100, PKTPOOLSTATE(pktp->dbg_q[i].p));
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

int
pktpool_stats_dump(pktpool_t *pktp, pktpool_stats_t *stats)
{
	int i;
	int state;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	bzero(stats, sizeof(pktpool_stats_t));
	for (i = 0; i < pktp->dbg_qlen; i++) {
		ASSERT(pktp->dbg_q[i].p != NULL);

		state = PKTPOOLSTATE(pktp->dbg_q[i].p);
		switch (state) {
			case POOL_TXENQ:
				stats->enq++; break;
			case POOL_TXDH:
				stats->txdh++; break;
			case POOL_TXD11:
				stats->txd11++; break;
			case POOL_RXDH:
				stats->rxdh++; break;
			case POOL_RXD11:
				stats->rxd11++; break;
			case POOL_RXFILL:
				stats->rxfill++; break;
			case POOL_IDLE:
				stats->idle++; break;
		}
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

int
pktpool_start_trigger(pktpool_t *pktp, void *p)
{
	uint32 cycles, i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	if (!PKTPOOL(OSH_NULL, p))
		goto done;

	OSL_GETCYCLES(cycles);

	for (i = 0; i < pktp->dbg_qlen; i++) {
		ASSERT(pktp->dbg_q[i].p != NULL);

		if (pktp->dbg_q[i].p == p) {
			pktp->dbg_q[i].cycles = cycles;
			break;
		}
	}

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

int pktpool_stop_trigger(pktpool_t *pktp, void *p);

int
pktpool_stop_trigger(pktpool_t *pktp, void *p)
{
	uint32 cycles, i;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	if (!PKTPOOL(OSH_NULL, p))
		goto done;

	OSL_GETCYCLES(cycles);

	for (i = 0; i < pktp->dbg_qlen; i++) {
		ASSERT(pktp->dbg_q[i].p != NULL);

		if (pktp->dbg_q[i].p == p) {
			if (pktp->dbg_q[i].cycles == 0)
				break;

			if (cycles >= pktp->dbg_q[i].cycles)
				pktp->dbg_q[i].dur = cycles - pktp->dbg_q[i].cycles;
			else
				pktp->dbg_q[i].dur =
					(((uint32)-1) - pktp->dbg_q[i].cycles) + cycles + 1;

			pktp->dbg_q[i].cycles = 0;
			break;
		}
	}

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}
#endif /* BCMDBG_POOL */

int
pktpool_avail_notify_normal(osl_t *osh, pktpool_t *pktp)
{
	BCM_REFERENCE(osh);
	ASSERT(pktp);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	pktp->availcb_excl = NULL;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return 0;
}

int
pktpool_avail_notify_exclusive(osl_t *osh, pktpool_t *pktp, pktpool_cb_t cb)
{
	int i;
	int err;
	BCM_REFERENCE(osh);

	ASSERT(pktp);

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(pktp->availcb_excl == NULL);
	for (i = 0; i < pktp->cbcnt; i++) {
		if (cb == pktp->cbs[i].cb) {
			pktp->availcb_excl = &pktp->cbs[i];
			break;
		}
	}

	if (pktp->availcb_excl == NULL)
		err = BCME_ERROR;
	else
		err = 0;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

static void
pktpool_avail_notify(pktpool_t *pktp)
{
	int i, k, idx;
	int avail;

	ASSERT(pktp);
	if (pktp->availcb_excl != NULL) {
		pktp->availcb_excl->cb(pktp, pktp->availcb_excl->arg);
		return;
	}

	k = pktp->cbcnt - 1;
	for (i = 0; i < pktp->cbcnt; i++) {
		avail = pktp->avail;

		if (avail) {
			if (pktp->cbtoggle)
				idx = i;
			else
				idx = k--;

			ASSERT(pktp->cbs[idx].cb != NULL);
			pktp->cbs[idx].cb(pktp, pktp->cbs[idx].arg);
		}
	}

	/* Alternate between filling from head or tail
	 */
	pktp->cbtoggle ^= 1;

	return;
}

/** Gets an empty packet from the caller provided pool */
void *
pktpool_get_ext(pktpool_t *pktp, uint8 type, void *bufp)
{
	void *p;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

#if defined(DONGLEBUILD)
	bool rxcpl = (pktp->rxcplidfn.cb != NULL) ? TRUE : FALSE;
#endif /* DONGLEBUILD */

	p = pktpool_deq(pktp);

	if (p == NULL) {
		/* Notify and try to reclaim tx pkts */
		if (pktp->ecbcnt)
			pktpool_empty_notify(pktp);

		p = pktpool_deq(pktp);
		if (p == NULL)
			goto done;
	}

#if defined(DONGLEBUILD)
	if (BCMSPLITRX_ENAB() && (type == lbuf_rxfrag)) {
		void *head = NULL, *tail = NULL;
		int cnt = 0;

#if defined(BCMHWA) && defined(HWA_RXFILL_BUILD)
		/* Nobody should get pkt from lbuf_rxfrag pool, as it is exlusive to HWA RX. */
		ASSERT(0);
		printf("HWA: Nobody should get pkt from lbuf_rxfrag pool, "
			"as it is exlusive to HWA RX.\n");
#endif // endif

		/* If pool is shared rx pool, use call back fn to populate host address */
		ASSERT(pktp->cbext.cb != NULL);

		do {
			/* If host address fill fails, get a pkt with host address populated */
			if (pktp->cbext.cb(pktp, pktp->cbext.arg, p, rxcpl)) {
				if (head == NULL) {
					head = tail = p;
				} else {
					PKTSETFREELIST(tail, p);
					tail = p;
				}

				cnt++;
				p = pktpool_deq(pktp);
			} else {
				/* Host address fill success, break and return pkt */
				break;
			}
		} while (p != NULL);

		if (head != NULL) {
			pktpool_enqchain_tail(pktp, head, tail, cnt);
		}
	} else if ((type == lbuf_basic) && rxcpl) {
		/* If pool is shared rx pool, use call back fn to populate Rx cpl ID */
		ASSERT(pktp->rxcplidfn.cb != NULL);
		/* If rxcplblock is allocated */
		if (pktp->rxcplidfn.cb(pktp, pktp->rxcplidfn.arg, p, TRUE)) {
			pktpool_enq(pktp, p);
			p = NULL;
		}
	}
#if defined(BCM_DHDHDR) && defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
	else if (BCMLFRAG_ENAB() && (type == lbuf_frag) && bufp) {
		void *buf;

		/* head pointer into data buffer got from the pool */
		buf = lfbufpool_get(bufp);
		if (buf == NULL) {
			pktpool_enq(pktp, p);
			p = NULL;
			goto done;
		}

		PKTSETBUF(OSH_NULL, p, buf, ((lfrag_buf_pool_t *)bufp)->max_buf_bytes);
		PKTSETBUFALLOC(OSH_NULL, p);
	}
#endif /* BCM_DHDHDR &&  BCMFRAGPOOL && !BCMFRAGPOOL_DISABLED */
#endif /* _DONGLEBUILD_ */

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void
pktpool_free(pktpool_t *pktp, void *p)
{
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	ASSERT(p != NULL);
#ifdef BCMDBG_POOL
	/* pktpool_stop_trigger(pktp, p); */
#endif // endif

	pktpool_enq(pktp, p);

	/**
	 * Feed critical DMA with freshly freed packets, to avoid DMA starvation.
	 * If any avail callback functions are registered, send a notification
	 * that a new packet is available in the pool.
	 */
	if (pktp->cbcnt) {
		/* To more efficiently use the cpu cycles, callbacks can be temporarily disabled.
		 * This allows to feed on burst basis as opposed to inefficient per-packet basis.
		 */
		if (pktp->emptycb_disable == EMPTYCB_ENABLED) {
			/**
			 * If the call originated from pktpool_empty_notify, the just freed packet
			 * is needed in pktpool_get.
			 * Therefore don't call pktpool_avail_notify.
			 */
			if (pktp->empty == FALSE)
				pktpool_avail_notify(pktp);
		} else {
			/**
			 * The callback is temporarily disabled, log that a packet has been freed.
			 */
			pktp->emptycb_disable = EMPTYCB_SKIPPED;
		}
	}

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return;
}

/** Adds a caller provided (empty) packet to the caller provided pool */
int
pktpool_add(pktpool_t *pktp, void *p)
{
	int err = 0;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	ASSERT(p != NULL);

	if (pktp->n_pkts == pktp->maxlen) {
		err = BCME_RANGE;
		goto done;
	}

	/* pkts in pool have same length */
	ASSERT(pktp->max_pkt_bytes == PKTLEN(OSH_NULL, p));
	PKTSETPOOL(OSH_NULL, p, TRUE, pktp);

	pktp->n_pkts++;
	pktpool_enq(pktp, p);

#ifdef BCMDBG_POOL
	pktp->dbg_q[pktp->dbg_qlen++].p = p;
#endif // endif

done:
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return err;
}

/**
 * Force pktpool_setmaxlen () into RAM as it uses a constant
 * (PKTPOOL_LEN_MAX) that may be changed post tapeout for ROM-based chips.
 */
int
BCMRAMFN(pktpool_setmaxlen)(pktpool_t *pktp, uint16 maxlen)
{
	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_ACQUIRE(&pktp->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	if (maxlen > PKTPOOL_LEN_MAX)
		maxlen = PKTPOOL_LEN_MAX;

	/* if pool is already beyond maxlen, then just cap it
	 * since we currently do not reduce the pool len
	 * already allocated
	 */
	pktp->maxlen = (pktp->n_pkts > maxlen) ? pktp->n_pkts : maxlen;

	/* protect shared resource */
	if (HND_PKTPOOL_MUTEX_RELEASE(&pktp->mutex) != OSL_EXT_SUCCESS)
		return BCME_ERROR;

	return pktp->maxlen;
}

void
pktpool_emptycb_disable(pktpool_t *pktp, bool disable)
{
	ASSERT(pktp);

	/**
	 * To more efficiently use the cpu cycles, callbacks can be temporarily disabled.
	 * If callback is going to be re-enabled, check if any packet got
	 * freed and added back to the pool while callback was disabled.
	 * When this is the case do the callback now, provided that callback functions
	 * are registered and this call did not originate from pktpool_empty_notify.
	 */
	if ((!disable) && (pktp->cbcnt) && (pktp->empty == FALSE) &&
		(pktp->emptycb_disable == EMPTYCB_SKIPPED)) {
			pktpool_avail_notify(pktp);
	}

	/* Enable or temporarily disable callback when packet becomes available. */
	pktp->emptycb_disable = disable ? EMPTYCB_DISABLED : EMPTYCB_ENABLED;
}

bool
pktpool_emptycb_disabled(pktpool_t *pktp)
{
	ASSERT(pktp);
	return pktp->emptycb_disable != EMPTYCB_ENABLED;
}

#ifdef BCMPKTPOOL
#include <hnd_lbuf.h>

pktpool_t *pktpool_shared = NULL;

#ifdef BCMFRAGPOOL
pktpool_t *pktpool_shared_lfrag = NULL;
#ifdef BCMRESVFRAGPOOL
pktpool_t *pktpool_resv_lfrag = NULL;
struct resv_info *resv_pool_info = NULL;
#endif /* BCMRESVFRAGPOOL */
#endif /* BCMFRAGPOOL */

pktpool_t *pktpool_shared_rxlfrag = NULL;

#ifdef UTXD_POOL
pktpool_t *pktpool_shared_utxd = NULL;
#endif // endif

static osl_t *pktpool_osh = NULL;

/**
 * Initializes several packet pools and allocates packets within those pools.
 */
int
BCMATTACHFN(hnd_pktpool_init)(osl_t *osh)
{
	int err = BCME_OK;
	int n;

	/* Construct a packet pool registry before initializing packet pools */
	n = pktpool_attach(osh, PKTPOOL_MAXIMUM_ID);
	if (n != PKTPOOL_MAXIMUM_ID) {
		ASSERT(0);
		err = BCME_ERROR;
		goto error0;
	}

	pktpool_shared = MALLOCZ(osh, sizeof(pktpool_t));
	if (pktpool_shared == NULL) {
		ASSERT(0);
		err = BCME_NOMEM;
		goto error1;
	}

#if defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
	pktpool_shared_lfrag = MALLOCZ(osh, sizeof(pktpool_t));
	if (pktpool_shared_lfrag == NULL) {
		ASSERT(0);
		err = BCME_NOMEM;
		goto error2;
	}
#if defined(BCMRESVFRAGPOOL) && !defined(BCMRESVFRAGPOOL_DISABLED)
	resv_pool_info = hnd_resv_pool_alloc(osh);
	if (resv_pool_info == NULL) {
		ASSERT(0);
		goto error2;
	}
	pktpool_resv_lfrag = resv_pool_info->pktp;
	if (pktpool_resv_lfrag == NULL) {
		ASSERT(0);
		goto error2;
	}
#endif	/* RESVFRAGPOOL */
#endif /* FRAGPOOL */

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	d3_lfrag_buf_pool = MALLOCZ(osh, sizeof(lfrag_buf_pool_t));
	if (d3_lfrag_buf_pool == NULL) {
		ASSERT(0);
		goto error3;
	}
#endif /* !BCM_DHDHDR_D3_DISABLE */
	d11_lfrag_buf_pool = MALLOCZ(osh, sizeof(lfrag_buf_pool_t));
	if (d11_lfrag_buf_pool == NULL) {
		ASSERT(0);
		goto error3;
	}
#endif /* BCM_DHDHDR && DONGLEBUILD */

#if defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)
	pktpool_shared_rxlfrag = MALLOCZ(osh, sizeof(pktpool_t));
	if (pktpool_shared_rxlfrag == NULL) {
		ASSERT(0);
		err = BCME_NOMEM;
		goto error3;
	}
#endif /* BCMRXFRAGPOOL */

#ifdef UTXD_POOL
		pktpool_shared_utxd = MALLOCZ(osh, sizeof(pktpool_t));
		if (pktpool_shared_utxd == NULL) {
			ASSERT(0);
			err = BCME_NOMEM;
			goto error3a;
		}
#endif /* BCMRXFRAGPOOL */

	/*
	 * At this early stage, there's not enough memory to allocate all
	 * requested pkts in the shared pool.  Need to add to the pool
	 * after reclaim
	 *
	 * n = NRXBUFPOST + SDPCMD_RXBUFS;
	 *
	 * Initialization of packet pools may fail (BCME_ERROR), if the packet pool
	 * registry is not initialized or the registry is depleted.
	 *
	 * A BCME_NOMEM error only indicates that the requested number of packets
	 * were not filled into the pool.
	 */
	n = 1;
	MALLOC_SET_NOPERSIST(osh); /* Ensure subsequent allocations are non-persist */
	if ((err = pktpool_init(osh, pktpool_shared,
	                        &n, PKTBUFSZ, FALSE, lbuf_basic)) != BCME_OK) {
		ASSERT(0);
		goto error4;
	}
	pktpool_setmaxlen(pktpool_shared, SHARED_POOL_LEN);

#if defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
#if defined(BCMRESVFRAGPOOL) && !defined(BCMRESVFRAGPOOL_DISABLED)
	n = 0; /* IMPORTANT: DO NOT allocate any packets in resv pool */
	if (pktpool_init(osh, pktpool_resv_lfrag,
	                 &n, PKTFRAGSZ, TRUE, lbuf_frag) == BCME_ERROR) {
		ASSERT(0);
		goto error5;
	}
	pktpool_setmaxlen(pktpool_resv_lfrag, RESV_FRAG_POOL_LEN);
#endif /* RESVFRAGPOOL */

	/* tx_lfrag header initialization */
	n = 1;
	if (pktpool_init(osh, pktpool_shared_lfrag, &n,
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
		0,
#else
		PKTFRAGSZ,
#endif // endif
		TRUE, lbuf_frag) == BCME_ERROR) {
		ASSERT(0);
		goto error5;
	}
	pktpool_setmaxlen(pktpool_shared_lfrag, SHARED_FRAG_POOL_LEN);
#endif /* BCMFRAGPOOL && !BCMFRAGPOOL_DISABLED */

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	/* D3_BUFFER pool initialization */
	n = 1;
	if (lfbufpool_init(osh, d3_lfrag_buf_pool, &n, D3LFBUF_SZ) == BCME_ERROR) {
		ASSERT(0);
		goto error6;
	}
	lfbufpool_setmaxlen(d3_lfrag_buf_pool, D3_BUFFER_LEN);
#endif /* !BCM_DHDHDR_D3_DISABLE */
	/* D11_BUFFER pool initialization */
	n = 1;
	if (lfbufpool_init(osh, d11_lfrag_buf_pool, &n, PKTFRAGSZ) == BCME_ERROR) {
		ASSERT(0);
		goto error6;
	}
	lfbufpool_setmaxlen(d11_lfrag_buf_pool, D11_BUFFER_LEN);
#endif /* BCM_DHDHDR && DONGLEBUILD */

#if defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)
	n = 1;
	if ((err = pktpool_init(osh, pktpool_shared_rxlfrag, &n,
#if defined(BCMHWA) && defined(HWA_RXFILL_BUILD)
				/* Rx buffer is from RXBM when HWA is enabled */
				0,
#else
				PKTRXFRAGSZ,
#endif // endif
				TRUE, lbuf_rxfrag)) != BCME_OK) {
		ASSERT(0);
		goto error6;
	}
	pktpool_setmaxlen(pktpool_shared_rxlfrag, SHARED_RXFRAG_POOL_LEN);
#endif /* BCMRXFRAGPOOL */

#ifdef UTXD_POOL
	n = 1;
	if ((err = pktpool_init(osh, pktpool_shared_utxd, &n,
		PKTUTXDSZ,
		TRUE, lbuf_basic)) != BCME_OK) {
		ASSERT(0);
		goto error6a;
	}
	pktpool_setmaxlen(pktpool_shared_utxd, PKTUTXDLEN);
#endif /* BCMRXFRAGPOOL */

#if defined(BCMFRWDPOOLREORG) && !defined(BCMFRWDPOOLREORG_DISABLED)
	/* Attach poolreorg module */
	if ((frwd_poolreorg_info = poolreorg_attach(osh,
#if defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
			pktpool_shared_lfrag,
#else
			NULL,
#endif // endif
#if defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)
			pktpool_shared_rxlfrag,
#else
			NULL,
#endif // endif
#ifdef UTXD_POOL
			pktpool_shared_utxd,
#else
			NULL,
#endif // endif
			pktpool_shared)) == NULL) {
		ASSERT(0);
		goto error7;
	}
#endif /* defined(BCMFRWDPOOLREORG) && !defined(BCMFRWDPOOLREORG_DISABLED) */

	pktpool_osh = osh;
	MALLOC_CLEAR_NOPERSIST(osh);

	return BCME_OK;

#if defined(BCMFRWDPOOLREORG) && !defined(BCMFRWDPOOLREORG_DISABLED)
	/* detach poolreorg module */
	poolreorg_detach(frwd_poolreorg_info);
error7:
#endif /* defined(BCMFRWDPOOLREORG) && !defined(BCMFRWDPOOLREORG_DISABLED) */

#ifdef UTXD_POOL
	pktpool_deinit(osh, pktpool_shared_utxd);
error6a:
#endif // endif

#if defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)
	pktpool_deinit(osh, pktpool_shared_rxlfrag);
error6:
#endif // endif

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	if (d3_lfrag_buf_pool)
		lfbufpool_deinit(osh, d3_lfrag_buf_pool);
#endif /* !BCM_DHDHDR_D3_DISABLE */
	if (d11_lfrag_buf_pool)
		lfbufpool_deinit(osh, d11_lfrag_buf_pool);
#endif /* BCM_DHDHDR && DONGLEBUILD */

#if defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
	pktpool_deinit(osh, pktpool_shared_lfrag);
error5:
#endif /* BCMFRAGPOOL && !BCMFRAGPOOL_DISABLED */

#if (defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)) || \
	(defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED))
	pktpool_deinit(osh, pktpool_shared);
#endif // endif

error4:
#ifdef UTXD_POOL
		hnd_free(pktpool_shared_utxd);
		pktpool_shared_utxd = (pktpool_t *)NULL;
	error3a:
#endif /* UTXD_POOL */

#if defined(BCMRXFRAGPOOL) && !defined(BCMRXFRAGPOOL_DISABLED)
	hnd_free(pktpool_shared_rxlfrag);
	pktpool_shared_rxlfrag = (pktpool_t *)NULL;
error3:
#endif /* BCMRXFRAGPOOL */

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	if (d3_lfrag_buf_pool) {
		hnd_free(d3_lfrag_buf_pool);
		d3_lfrag_buf_pool = (lfrag_buf_pool_t *)NULL;
	}
#endif /* */
	if (d11_lfrag_buf_pool) {
		hnd_free(d11_lfrag_buf_pool);
		d11_lfrag_buf_pool = (lfrag_buf_pool_t *)NULL;
	}
#endif /* BCM_DHDHDR && DONGLEBUILD */

#if defined(BCMFRAGPOOL) && !defined(BCMFRAGPOOL_DISABLED)
	hnd_free(pktpool_shared_lfrag);
	pktpool_shared_lfrag = (pktpool_t *)NULL;
error2:
#endif /* BCMFRAGPOOL */

	hnd_free(pktpool_shared);
	pktpool_shared = (pktpool_t *)NULL;

error1:
	pktpool_dettach(osh);
error0:
	MALLOC_CLEAR_NOPERSIST(osh);
	return err;
} /* hnd_pktpool_init */

/** is called at each 'wl up' */
int
hnd_pktpool_fill(pktpool_t *pktpool, bool minimal)
{
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	lfbufpool_fill(pktpool_osh, d3_lfrag_buf_pool, minimal);
#endif // endif
	lfbufpool_fill(pktpool_osh, d11_lfrag_buf_pool, minimal);
#endif /* BCM_DHDHDR && DONGLEBUILD */

	return (pktpool_fill(pktpool_osh, pktpool, minimal));
}

/** refills pktpools after reclaim, is called once */
void
hnd_pktpool_refill(bool minimal)
{
	if (POOL_ENAB(pktpool_shared)) {
#if defined(SRMEM)
		if (SRMEM_ENAB()) {
			int maxlen = pktpool_max_pkts(pktpool_shared);
			int n_pkts = pktpool_tot_pkts(pktpool_shared);

			for (; n_pkts < maxlen; n_pkts++) {
				void *p;
				if ((p = PKTSRGET(pktpool_max_pkt_bytes(pktpool_shared))) == NULL)
					break;
				pktpool_add(pktpool_shared, p);
			}
		}
#endif /* SRMEM */
		pktpool_fill(pktpool_osh, pktpool_shared, minimal);
	}
/* fragpool reclaim */
#ifdef BCMFRAGPOOL
	if (POOL_ENAB(pktpool_shared_lfrag)) {
		pktpool_fill(pktpool_osh, pktpool_shared_lfrag, minimal);
	}
#endif /* BCMFRAGPOOL */

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#if !defined(BCM_DHDHDR_D3_DISABLE)
	if (POOL_ENAB(d3_lfrag_buf_pool)) {
		lfbufpool_fill(pktpool_osh, d3_lfrag_buf_pool, minimal);
	}
#endif // endif
	if (POOL_ENAB(d11_lfrag_buf_pool)) {
		lfbufpool_fill(pktpool_osh, d11_lfrag_buf_pool, minimal);
	}
#endif /* BCM_DHDHDR && DONGLEBUILD */

/* rx fragpool reclaim */
#ifdef BCMRXFRAGPOOL
	if (POOL_ENAB(pktpool_shared_rxlfrag)) {
		pktpool_fill(pktpool_osh, pktpool_shared_rxlfrag, minimal);
	}
#endif // endif

#ifdef UTXD_POOL
	if (POOL_ENAB(pktpool_shared_utxd)) {
		pktpool_fill(pktpool_osh, pktpool_shared_utxd, minimal);
	}
#endif // endif

#if defined(BCMFRAGPOOL) && defined(BCMRESVFRAGPOOL)
	if (POOL_ENAB(pktpool_resv_lfrag)) {
		int resv_size = (PKTFRAGSZ + LBUFFRAGSZ)*RESV_FRAG_POOL_LEN;
		hnd_resv_pool_init(resv_pool_info, resv_size);
		hnd_resv_pool_enable(resv_pool_info);
	}
#endif /* BCMRESVFRAGPOOL */
}
#endif /* BCMPKTPOOL */
