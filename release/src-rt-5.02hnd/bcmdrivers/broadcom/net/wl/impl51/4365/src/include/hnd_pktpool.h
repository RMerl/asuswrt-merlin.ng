/*
 * HND generic packet pool operation primitives
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
 * $Id: $
 */

#ifndef _hnd_pktpool_h_
#define _hnd_pktpool_h_

#include <osl_ext.h>

#ifdef __cplusplus
extern "C" {
#endif

/* mutex macros for thread safe */
#ifdef HND_PKTPOOL_THREAD_SAFE
#define HND_PKTPOOL_MUTEX_DECL(mutex)		OSL_EXT_MUTEX_DECL(mutex)
#else
#define HND_PKTPOOL_MUTEX_DECL(mutex)
#endif

#ifdef BCMPKTPOOL
#define POOL_ENAB(pool)		((pool) && (pool)->inited)
#else /* BCMPKTPOOL */
#define POOL_ENAB(bus)		0
#endif /* BCMPKTPOOL */

#ifndef PKTPOOL_LEN_MAX
#define PKTPOOL_LEN_MAX		40
#endif /* PKTPOOL_LEN_MAX */
#define PKTPOOL_CB_MAX		3
#define PKTPOOL_CB_MAX_AVL	4


/* forward declaration */
struct pktpool;

typedef void (*pktpool_cb_t)(struct pktpool *pool, void *arg);
typedef struct {
	pktpool_cb_t cb;
	void *arg;
} pktpool_cbinfo_t;
/* call back fn extension to populate host address in pool pkt */
typedef int (*pktpool_cb_extn_t)(struct pktpool *pool, void *arg1, void* pkt, bool arg2);
typedef struct {
	pktpool_cb_extn_t cb;
	void *arg;
} pktpool_cbextn_info_t;


#ifdef BCMDBG_POOL
/* pkt pool debug states */
#define POOL_IDLE	0
#define POOL_RXFILL	1
#define POOL_RXDH	2
#define POOL_RXD11	3
#define POOL_TXDH	4
#define POOL_TXD11	5
#define POOL_AMPDU	6
#define POOL_TXENQ	7

typedef struct {
	void *p;
	uint32 cycles;
	uint32 dur;
} pktpool_dbg_t;

typedef struct {
	uint8 txdh;	/* tx to host */
	uint8 txd11;	/* tx to d11 */
	uint8 enq;	/* waiting in q */
	uint8 rxdh;	/* rx from host */
	uint8 rxd11;	/* rx from d11 */
	uint8 rxfill;	/* dma_rxfill */
	uint8 idle;	/* avail in pool */
} pktpool_stats_t;
#endif /* BCMDBG_POOL */

typedef struct pktpool {
	bool inited;            /* pktpool_init was successful */
	uint8 type;             /* type of lbuf: basic, frag, etc */
	uint8 id;               /* pktpool ID:  index in registry */
	bool istx;              /* direction: transmit or receive data path */
	HND_PKTPOOL_MUTEX_DECL(mutex)	/* thread-safe mutex */

	void * freelist;        /* free list: see PKTNEXTFREE(), PKTSETNEXTFREE() */
	uint16 avail;           /* number of packets in pool's free list */
	uint16 len;             /* number of packets managed by pool */
	uint16 maxlen;          /* maximum size of pool <= PKTPOOL_LEN_MAX */
	uint16 plen;            /* size of pkt buffer, excluding lbuf|lbuf_frag */

	bool empty;
	uint8 cbtoggle;
	uint8 cbcnt;
	uint8 ecbcnt;
	bool emptycb_disable;
	pktpool_cbinfo_t *availcb_excl;
	pktpool_cbinfo_t cbs[PKTPOOL_CB_MAX_AVL];
	pktpool_cbinfo_t ecbs[PKTPOOL_CB_MAX];
	pktpool_cbextn_info_t cbext;
	pktpool_cbextn_info_t rxcplidfn;
#ifdef BCMDBG_POOL
	uint8 dbg_cbcnt;
	pktpool_cbinfo_t dbg_cbs[PKTPOOL_CB_MAX];
	uint16 dbg_qlen;
	pktpool_dbg_t dbg_q[PKTPOOL_LEN_MAX + 1];
#endif
	pktpool_cbinfo_t dmarxfill;
} pktpool_t;

#ifdef BCM_DHDHDR
typedef struct lfrag_buf_pool {
	bool inited;            /* lfrag_buf_pool_init was successful */
	void *freelist;         /* free list: see PKTNEXTFREE(), PKTSETNEXTFREE() */
	uint16 avail;           /* number of buffers in pool's free list */
	uint16 len;             /* number of buffers managed by pool */
	uint16 maxlen;          /* maximum size of pool <= PKTPOOL_LEN_MAX */
	uint16 buflen;          /* size of buffer, excluding lfrag_buf_t */
	HND_PKTPOOL_MUTEX_DECL(mutex)	/* thread-safe mutex */
} lfrag_buf_pool_t;

typedef struct lfrag_buf {
	union {
		lfrag_buf_pool_t *lfbufp; /* Lfrag buf pool pointer when buffer be allocated */
		void *freelist;	/* Free list link when buffer back to the pool */
	};
} lfrag_buf_t;

#define LFBUFSZ		((int)sizeof(lfrag_buf_t))
#endif /* BCM_DHDHDR */

pktpool_t *get_pktpools_registry(int id);

/* Incarnate a pktpool registry. On success returns total_pools. */
extern int pktpool_attach(osl_t *osh, uint32 total_pools);
extern int pktpool_dettach(osl_t *osh); /* Relinquish registry */

extern int pktpool_init(osl_t *osh, pktpool_t *pktp, int *pktplen, int plen, bool istx, uint8 type);
extern int pktpool_deinit(osl_t *osh, pktpool_t *pktp);
extern int pktpool_fill(osl_t *osh, pktpool_t *pktp, bool minimal);
extern void* _pktpool_get(pktpool_t *pktp, void *bufp);
extern void pktpool_free(pktpool_t *pktp, void *p);
extern int pktpool_add(pktpool_t *pktp, void *p);
extern int pktpool_avail_notify_normal(osl_t *osh, pktpool_t *pktp);
extern int pktpool_avail_notify_exclusive(osl_t *osh, pktpool_t *pktp, pktpool_cb_t cb);
extern int pktpool_avail_register(pktpool_t *pktp, pktpool_cb_t cb, void *arg);
extern int pktpool_empty_register(pktpool_t *pktp, pktpool_cb_t cb, void *arg);
extern int pktpool_setmaxlen(pktpool_t *pktp, uint16 maxlen);
extern int pktpool_setmaxlen_strict(osl_t *osh, pktpool_t *pktp, uint16 maxlen);
extern void pktpool_emptycb_disable(pktpool_t *pktp, bool disable);
extern bool pktpool_emptycb_disabled(pktpool_t *pktp);
extern int pktpool_hostaddr_fill_register(pktpool_t *pktp, pktpool_cb_extn_t cb, void *arg1);
extern int pktpool_rxcplid_fill_register(pktpool_t *pktp, pktpool_cb_extn_t cb, void *arg);
extern void pktpool_invoke_dmarxfill(pktpool_t *pktp);
extern int pkpool_haddr_avail_register_cb(pktpool_t *pktp, pktpool_cb_t cb, void *arg);

#define pktpool_get(pp)			_pktpool_get(pp, NULL)
#define pktpool_lfrag_get(pp, bp)	_pktpool_get(pp, bp)

#ifdef BCM_DHDHDR
extern int lfbufpool_init(osl_t *osh, lfrag_buf_pool_t *pktp, int *pplen, int plen);
extern int lfbufpool_deinit(osl_t *osh, lfrag_buf_pool_t *pktp);
extern int lfbufpool_fill(osl_t *osh, lfrag_buf_pool_t *pktp, bool minimal);
extern void *lfbufpool_get(lfrag_buf_pool_t *pktp);
extern void lfbufpool_free(void *d);
extern int lfbufpool_setmaxlen(lfrag_buf_pool_t *pktp, uint16 maxlen);

#define LFBUFPOOLPTR(pp)	((lfrag_buf_pool_t *)(pp))
#define lfbufpool_avail(pp)	(LFBUFPOOLPTR(pp)->avail)
#define lfbufpool_maxlen(pp)	(LFBUFPOOLPTR(pp)->maxlen)
#endif /* BCM_DHDHDR */

#define POOLPTR(pp)         ((pktpool_t *)(pp))
#define POOLID(pp)          (POOLPTR(pp)->id)

#define POOLSETID(pp, ppid) (POOLPTR(pp)->id = (ppid))

#define pktpool_len(pp)     (POOLPTR(pp)->len)
#define pktpool_avail(pp)   (POOLPTR(pp)->avail)
#define pktpool_plen(pp)    (POOLPTR(pp)->plen)
#define pktpool_maxlen(pp)  (POOLPTR(pp)->maxlen)


/*
 * ----------------------------------------------------------------------------
 * A pool ID is assigned with a pkt pool during pool initialization. This is
 * done by maintaining a registry of all initialized pools, and the registry
 * index at which the pool is registered is used as the pool's unique ID.
 * ID 0 is reserved and is used to signify an invalid pool ID.
 * All packets henceforth allocated from a pool will be tagged with the pool's
 * unique ID. Packets allocated from the heap will use the reserved ID = 0.
 * Packets with non-zero pool id signify that they were allocated from a pool.
 * A maximum of 15 pools are supported, allowing a 4bit pool ID to be used
 * in place of a 32bit pool pointer in each packet.
 * ----------------------------------------------------------------------------
 */
#define PKTPOOL_INVALID_ID          (0)
#define PKTPOOL_MAXIMUM_ID          (15)

/* Registry of pktpool(s) */
/* Pool ID to/from Pool Pointer converters */
#define PKTPOOL_ID2PTR(id)          (get_pktpools_registry(id))
#define PKTPOOL_PTR2ID(pp)          (POOLID(pp))

#ifdef BCMDBG_POOL
extern int pktpool_dbg_register(pktpool_t *pktp, pktpool_cb_t cb, void *arg);
extern int pktpool_start_trigger(pktpool_t *pktp, void *p);
extern int pktpool_dbg_dump(pktpool_t *pktp);
extern int pktpool_dbg_notify(pktpool_t *pktp);
extern int pktpool_stats_dump(pktpool_t *pktp, pktpool_stats_t *stats);
#endif /* BCMDBG_POOL */

#ifdef BCMPKTPOOL
#define SHARED_POOL		(pktpool_shared)
extern pktpool_t *pktpool_shared;
#ifdef BCMFRAGPOOL
#define SHARED_FRAG_POOL	(pktpool_shared_lfrag)
extern pktpool_t *pktpool_shared_lfrag;
#endif
#define SHARED_RXFRAG_POOL	(pktpool_shared_rxlfrag)
extern pktpool_t *pktpool_shared_rxlfrag;

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
#define D3_LFRAG_BUF_POOL	(d3_lfrag_buf_pool)
extern lfrag_buf_pool_t *d3_lfrag_buf_pool;
#define D11_LFRAG_BUF_POOL	(d11_lfrag_buf_pool)
extern lfrag_buf_pool_t *d11_lfrag_buf_pool;
#else
#define D3_LFRAG_BUF_POOL	(NULL)
#define D11_LFRAG_BUF_POOL	(NULL)
#endif /* BCM_DHDHDR */

void hnd_pktpool_init(osl_t *osh);
void hnd_pktpool_fill(pktpool_t *pktpool, bool minimal);
void hnd_pktpool_refill(bool minimal);
#else /* BCMPKTPOOL */
#define SHARED_POOL		((struct pktpool *)NULL)
#endif /* BCMPKTPOOL */

#ifdef __cplusplus
	}
#endif

#endif /* _hnd_pktpool_h_ */
