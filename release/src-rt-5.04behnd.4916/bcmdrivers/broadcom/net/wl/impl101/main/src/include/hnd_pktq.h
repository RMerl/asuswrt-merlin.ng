/*
 * HND generic pktq operation primitives
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
 * $Id: hnd_pktq.h 829444 2023-08-30 01:59:56Z $
 */

#ifndef _hnd_pktq_h_
#define _hnd_pktq_h_

#include <osl_ext.h>

#ifdef __cplusplus
extern "C" {
#endif

/* mutex macros for thread safe */
#ifdef HND_PKTQ_THREAD_SAFE
#define HND_PKTQ_MUTEX_DECL(mutex)		OSL_EXT_MUTEX_DECL(mutex)
#define HND_PKTQ_MUTEX_CREATE(name, mutex)	osl_ext_mutex_create(name, mutex)
#define HND_PKTQ_MUTEX_DELETE(mutex)		osl_ext_mutex_delete(mutex)
#define HND_PKTQ_MUTEX_ACQUIRE(mutex, msec)	osl_ext_mutex_acquire(mutex, msec)
#define HND_PKTQ_MUTEX_RELEASE(mutex)		osl_ext_mutex_release(mutex)
#else
#define HND_PKTQ_MUTEX_DECL(mutex)
#define HND_PKTQ_MUTEX_CREATE(name, mutex)	OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_DELETE(mutex)		OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_ACQUIRE(mutex, msec)	OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_RELEASE(mutex)		OSL_EXT_SUCCESS
#endif /* HND_PKTQ_THREAD_SAFE */

/* WL component should static assert the next define agains NUMPRIO (from 802.1d.h). */
#define PKTQ_NUMPRIO		8

/* osl multi-priority packet queue */
#define PKTQ_LEN_MAX		0xFFFF  /* Max uint16 65535 packets */
#ifndef PKTQ_LEN_DEFAULT
#define PKTQ_LEN_DEFAULT	256	/* Max 256 packets */
#endif
#ifndef PKTQ_MAX_PRIO
#define PKTQ_MAX_PRIO		PKTQ_NUMPRIO	/* Maximum precedence/priority levels */
#endif

/**
 * HNDPQP: pktq_prio::stall_count, dequeue_count repurposed
 */
struct pqp_req; /**< HNDPQP: PQP request paired with a PQP managed pktq_prio */

/** Queue for a single priority */
typedef struct pktq_prio {
	void *head;		/**< first packet to dequeue */
	void *tail;		/**< last packet to dequeue */
	uint16 n_pkts;		/**< number of queued packets */
	uint16 max_pkts;	/**< maximum number of queued packets */
	uint16 stall_count;	/**< # seconds since no packets are dequeued  */
	uint16 dequeue_count;	/**< # of packets dequeued in last 1 second */
	struct pqp_req * pqp_req; /**< HNDPQP fields for PS and SupprPS Queues */
} pktq_prec_t;

/** multi-priority packet queue */
struct pktq {
	HND_PKTQ_MUTEX_DECL(mutex)
	uint16 num_prio;	/**< number of prio/idx in use, max PKTQ_MAX_PRIO */
	uint16 max_pkts;	/**< max  packets */
	uint16 n_pkts_tot;	/**< total (cummulative over all priorities) number of packets */
	uint16 v_pkts_tot;
	uint8 *prio_to_idx;	/**< Translates the provided prio in pktq functions to the actual
				 * used idx. This will determine the real precedence. This is a
				 * table to be provided in pktq_config (if desired) and defaults
				 * to a one to one translation prio 0, is idx 0, etc.
				 */
	uint8 *idx_to_prio;	/**< This is the reverse lookup table to be provided when
				 * prio_to_idx is used. This is necesssary because prio_to_idx
				 * allows for N to M mapping where M <= N. When M is smaller then
				 * N then user needs to provide the desired reverse mapping. If
				 * prio 0 and 1 both map to idx 0, then upon for example mdeq the
				 * idx needs to be translated to prio, but it can be 0 or 1. This
				 * table defaults to the same table as used for prio_to_idx.
				 */
	uint8 pktq_type;
	bool is_cfp;
	struct pktq_prio q[PKTQ_MAX_PRIO];
};

typedef enum {
	PKTQ_QUEUE_AMPDU,
	PKTQ_QUEUE_NAR,
	PKTQ_QUEUE_PS,
	PKTQ_QUEUE_MU,
	PKTQ_QUEUE_COMMON,
	PKTQ_QUEUE_MAX = PKTQ_QUEUE_COMMON
} e_pktq_type;

/** simple, non-priority packet queue */
struct spktq {
	HND_PKTQ_MUTEX_DECL(mutex)
	struct pktq_prio q;
};

/* operations on a specific precedence in packet queue */
#define pktqprio_max_pkts(pq, prio)		((pq)->q[(prio)].max_pkts)
#define pktqprio_n_pkts(pq, prio)		((pq)->q[(prio)].n_pkts)
#define pktqprio_empty(pq, prio)		((pq)->q[(prio)].n_pkts == 0)
#define pktqprio_peek(pq, prio)			((pq)->q[(prio)].head)
#define pktqprio_peek_tail(pq, prio)		((pq)->q[(prio)].tail)
#ifdef HND_PKTQ_THREAD_SAFE
extern int pktqprio_avail_pkts(struct pktq *pq, int prio);
extern bool pktqprio_full(struct pktq *pq, int prio);
#else
#define pktqprio_avail_pkts(pq, prio)	MAX(0, ((pq)->q[(prio)].max_pkts - (pq)->q[(prio)].n_pkts))
#define pktqprio_full(pq, prio)		((pq)->q[(prio)].n_pkts >= (pq)->q[(prio)].max_pkts)
#endif	/* HND_PKTQ_THREAD_SAFE */

extern void  pktqprio_append(struct pktq *pq, int prio, struct spktq *list);
extern void  pktqprio_prepend(struct pktq *pq, int prio, struct spktq *list);
extern void *pktqprio_penq(struct pktq *pq, int prio, void *p);
extern void *pktqprio_penq_head(struct pktq *pq, int prio, void *p);
extern void *pktqprio_pdeq(struct pktq *pq, int prio);
extern void *pktqprio_pdeq_prev(struct pktq *pq, int prio, void *prev_p);
extern void *pktqprio_pdeq_tail(struct pktq *pq, int prio);
/** Remove a specified packet from its queue */
extern bool pktqprio_pdel(struct pktq *pq, void *p, int prio);

/* operations on a set of priorities in packet queue */
extern int   pktq_mlen(struct pktq *pq, uint prio_bmp);
extern void *pktq_mdeq(struct pktq *pq, uint prio_bmp, int *prio_out);
extern void *pktq_mpeek(struct pktq *pq, uint prio_bmp, int *prio_out);

/* operations on packet queue as a whole */

#define pktq_n_pkts_tot(pq)	((int)(pq)->n_pkts_tot)
#define pktq_max(pq)		((int)(pq)->max_pkts)
#define pktq_empty(pq)		((pq)->n_pkts_tot == 0)
#ifdef HND_PKTQ_THREAD_SAFE
extern int  pktq_avail(struct pktq *pq);
extern bool pktq_full(struct pktq *pq);
#else
#define pktq_avail(pq)		((int)((pq)->max_pkts - (pq)->n_pkts_tot))
#define pktq_full(pq)		((pq)->n_pkts_tot >= (pq)->max_pkts)
#endif	/* HND_PKTQ_THREAD_SAFE */

/* operations for single precedence queues */
#define pktenq(pq, p)		pktqprio_penq((pq), 0, (p))
#define pktenq_head(pq, p)	pktqprio_penq_head((pq), 0, (p))
#define pktdeq(pq)		pktqprio_pdeq((pq), 0)
#define pktdeq_tail(pq)		pktqprio_pdeq_tail((pq), 0)
#define pktqflush(osh, pq, dir)	pktq_pflush(osh, (pq), 0, (dir))
#define pktqinit(pq, max_pkts)	pktq_init((pq), 1, (max_pkts))
#define pktqdeinit(pq)		pktq_deinit((pq))
#define pktqavail(pq)		pktq_avail((pq))
#define pktqfull(pq)		pktq_full((pq))

extern bool pktq_init(struct pktq *pq, int num_prio, int max_pkts);
extern void pktq_config_prec(e_pktq_type type, struct pktq *pq,
	uint8 *prio_to_idx, uint8 *idx_to_prio);
extern void pktq_set_max_plen(struct pktq *pq, int prio, int max_pkts);

/* prio_out may be NULL if caller is not interested in return value */
extern void *pktq_deq(struct pktq *pq, int *prio_out);
extern void *pktq_deq_tail(struct pktq *pq, int *prio_out);
extern void *pktq_peek(struct pktq *pq, int *prio_out);
extern void *pktq_peek_tail(struct pktq *pq, int *prio_out);

/** flush pktq */
extern void pktq_flush(osl_t *osh, struct pktq *pq, bool dir);
/** Flush the queue with particular priority */
extern void pktq_pflush(osl_t *osh, struct pktq *pq, int prio, bool dir);

/* operations for simple non-priority queues */
extern bool  spktq_init(struct spktq *spq, int max_pkts);
extern void *spktq_enq(struct spktq *spq, void *p);
extern void  spktq_enq_list(struct spktq *spq, struct spktq *list);
extern void *spktq_enq_head(struct spktq *spq, void *p);
extern void  spktq_enq_head_list(struct spktq *spq, struct spktq *list);
extern void *spktq_deq(struct spktq *spq);
extern void *spktq_deq_tail(struct spktq *spq);
extern void  spktq_flush(osl_t *osh, struct spktq *spq, bool dir);
#ifdef HND_PKTQ_THREAD_SAFE
extern int  spktq_avail(struct spktq *spq);
extern bool spktq_full(struct spktq *spq);
extern void spktq_deinit(struct spktq *spq);
extern void pktq_deinit(struct pktq *pq);
#else
#define spktq_avail(spq)	((int)((spq)->q.max_pkts - (spq)->q.n_pkts))
#define spktq_full(spq)		((spq)->q.n_pkts >= (spq)->q.max_pkts)
#define spktq_deinit(spq)	BCM_REFERENCE(spq)
#define pktq_deinit(pq)		BCM_REFERENCE(pq)
#endif	/* HND_PKTQ_THREAD_SAFE */
extern bool spktq_init_list(struct spktq *spq, uint max_pkts,
	void *head, void *tail, uint16 n_pkts);
extern void *spktq_peek(struct spktq *spq);
#define spktq_n_pkts(spq)	((int)(spq)->q.n_pkts)
#define spktq_empty(spq)	((spq)->q.n_pkts == 0)
#define spktq_max(spq)		((int)(spq)->q.max_pkts)
#define spktq_peek_head(pq)	((pq)->q.head)
#define spktq_peek_tail(pq)	((pq)->q.tail)
typedef void (*spktq_cb_t)(void *arg, struct spktq *spq);
extern void spktq_free_register(spktq_cb_t cb, void *arg);
extern void spktq_cb(void *spq);
#define SPKTQFREE	spktq_cb
#ifdef HNDPQP
/* Check for PQP ownership */
extern bool pktq_pqp_owns(struct pktq *pq, int prio);
extern bool pktq_mpqp_owns(struct pktq *pq, uint prio_bmp);

/* PQP packet counts */
#define PKTQ_PKTS_ALL		((uint32) (~0))
extern int pktq_pqp_pkt_cnt(struct pktq *pq, int prio);
extern int pktq_pqp_hbm_cnt(struct pktq *pq, int prio);
extern int pktq_pqp_dbm_cnt(struct pktq *pq, int prio);
extern int pktq_pqp_mlen(struct pktq *pq, uint prio_bmp);

/* #### PQP Page in utilities #### */
/* PQP PGI in priority order */
extern bool pktq_mpqp_pgi(struct pktq *pq, uint prio_bmp, int flags);
/* PQP PGI  from a given prio bmp */
/*   see hnd_pqp.h for flags */
extern void pktq_pqp_pgi(struct pktq *pq, int prio, int cont_pkts, int fill_pkts, int flags);
/* SPKTQ PQP page in */
extern void spktq_pqp_pgi(struct spktq* spq, int cont_pkts, int fill_pkts, int flags);

/* #### PQP Page out utilities #### */
extern void spktq_pqp_pgo(struct spktq* spq, int policy, void* cb_ctx);
extern void pktq_pqp_pgo(struct pktq *pq, uint prio_bmp, int policy, void* cb_ctx);
extern void pktq_prio_pqp_pgo(struct pktq *pq, int prio, int policy, void* cb_ctx);

extern void pktq_prio_pqp_join(struct pktq *pq, int prio, struct pktq_prio *q_B,
	int policy, void* cb_ctx);

#endif /* HNDPQP */

/*
 * pktq filter support
 */

/** filter function return values */
typedef enum {
	PKT_FILTER_NOACTION = 0,    /**< restore the pkt to its position in the queue */
	PKT_FILTER_DELETE = 1,      /**< delete the pkt */
	PKT_FILTER_REMOVE = 2,      /**< do not restore the pkt to the queue,
	                             *   filter fn has taken ownership of the pkt
	                             */
} pktq_filter_result_t;

/**
 * Caller supplied filter function to pktq_filter()/pktq_pfilter().
 * Function filter(ctx, pkt) is called with its ctx pointer on each pkt in the
 * pktq.  When the filter function is called, the supplied pkt will have been
 * unlinked from the pktq.  The filter function returns a pktq_filter_result_t
 * result specifying the action pktq_filter/pfilter() should take for the pkt.
 * Here are the actions taken by pktq_filter/pfilter() based on the supplied
 * filter function's return value:
 *
 * PKT_FILTER_NOACTION - The filter will re-link the pkt at its
 *     previous location.
 *
 * PKT_FILTER_DELETE - The filter will not relink the pkt and will
 *     call the user supplied defer_free_pkt fn on the packet.
 *
 * PKT_FILTER_REMOVE - The filter will not relink the pkt. The supplied
 *     filter fn took ownership (or deleted) the pkt.
 *
 * WARNING: pkts inserted by the user (in pkt_filter and/or flush callbacks
 * and chains) in the prio queue will not be seen by the filter, and the prio
 * queue will be temporarily be removed from the queue hence there're side
 * effects including pktq_n_pkts_tot() on the queue won't reflect the correct number
 * of packets in the queue.
 */
typedef pktq_filter_result_t (*pktq_filter_t)(void* ctx, void* pkt);

/**
 * The defer_free_pkt callback is invoked when the the pktq_filter callback
 * returns PKT_FILTER_DELETE decision, which allows the user to deposite
 * the packet appropriately based on the situation (free the packet or
 * save it in a temporary queue etc.).
 */
typedef void (*defer_free_pkt_fn_t)(void *ctx, void *pkt);

/**
 * The flush_free_pkt callback is invoked when all packets in the pktq
 * are processed.
 */
typedef void (*flush_free_pkt_fn_t)(void *ctx);

extern void pktq_pfilter(struct pktq *pq, int prio, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx);
extern void pktq_filter(struct pktq *pq, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx, flush_free_pkt_fn_t flush, void *flush_ctx);

extern void spktq_filter(struct spktq *spq, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx, flush_free_pkt_fn_t flush, void *flush_ctx);

#ifdef __cplusplus
}
#endif

#endif /* _hnd_pktq_h_ */
