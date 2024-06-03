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
 * $Id: hnd_pktq.c 829444 2023-08-30 01:59:56Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <hnd_pktq.h>
#ifdef HNDPQP
#include <hnd_pqp.h>
#endif /* HNDPQP */

static uint8 default_prio_to_idx_to_prio[PKTQ_MAX_PRIO] =
{
	0, 1, 2, 3, 4, 5, 6, 7
};

#define PKTQ_IDX_ITER(pq, idx)        for (idx = (pq)->num_prio - 1; idx >= 0; idx--)

/*
 * osl multiple-priority packet queue
 */

/** @param pq   Multi-priority packet queue */
void * BCMFASTPATH
pktqprio_penq(struct pktq *pq, int prio, void *p)
{
	struct pktq_prio *q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prio >= 0 && prio < pq->num_prio);
	ASSERT(PKTLINK(p) == NULL);		/* queueing chains not allowed */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktqprio_full(pq, prio));

	q = &pq->q[prio];

	if (q->head)
		PKTSETLINK(q->tail, p);
	else
		q->head = p;

	q->tail = p;
	q->n_pkts++;

	pq->n_pkts_tot++;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** osl simple, non-priority packet queue */
void * BCMFASTPATH
spktq_enq(struct spktq *spq, void *p)
{
	struct pktq_prio *q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(!spktq_full(spq));

	PKTSETLINK(p, NULL);

	q = &spq->q;

	if (q->head)
		PKTSETLINK(q->tail, p);
	else
		q->head = p;

	q->tail = p;
	q->n_pkts++;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void * BCMFASTPATH
pktqprio_penq_head(struct pktq *pq, int prio, void *p)
{
	struct pktq_prio *q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prio >= 0 && prio < pq->num_prio);
	ASSERT(PKTLINK(p) == NULL);		/* queueing chains not allowed */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktqprio_full(pq, prio));

	q = &pq->q[prio];

	if (q->head == NULL)
		q->tail = p;

	PKTSETLINK(p, q->head);
	q->head = p;
	q->n_pkts++;

	pq->n_pkts_tot++;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param spktq : single priority packet queue */
void * BCMFASTPATH
spktq_enq_head(struct spktq *spq, void *p)
{
	struct pktq_prio *q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(!spktq_full(spq));

	q = &spq->q;

	if (q->head == NULL)
		q->tail = p;

	PKTSETLINK(p, q->head);
	q->head = p;
	q->n_pkts++;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void * BCMFASTPATH
pktqprio_pdeq(struct pktq *pq, int prio)
{
	struct pktq_prio *q;			/**< single priority packet queue */
	void *p;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->n_pkts--;

	pq->n_pkts_tot--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param spktq : single priority packet queue */
void * BCMFASTPATH
spktq_deq(struct spktq *spq)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	q = &spq->q;

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->n_pkts--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void * BCMFASTPATH
pktqprio_pdeq_tail(struct pktq *pq, int prio)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p, *prev;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];

	if ((p = q->head) == NULL)
		goto done;

	for (prev = NULL; p != q->tail; p = PKTLINK(p))
		prev = p;

	if (prev)
		PKTSETLINK(prev, NULL);
	else
		q->head = NULL;

	q->tail = prev;
	q->n_pkts--;

	pq->n_pkts_tot--;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param spktq : single priority packet queue */
void * BCMFASTPATH
spktq_deq_tail(struct spktq *spq)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p, *prev;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	q = &spq->q;

	if ((p = q->head) == NULL)
		goto done;

	for (prev = NULL; p != q->tail; p = PKTLINK(p))
		prev = p;

	if (prev)
		PKTSETLINK(prev, NULL);
	else
		q->head = NULL;

	q->tail = prev;
	q->n_pkts--;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void *
pktq_peek_tail(struct pktq *pq, int *prio_out)
{
	void *p = NULL;
	int idx;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	for (idx = 0; idx < pq->num_prio; idx++)
		if (pq->q[pq->idx_to_prio[idx]].head)
			break;

	if (idx == pq->num_prio)
		goto done;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

	p = pq->q[pq->idx_to_prio[idx]].tail;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/**
 * Append spktq 'list' to the tail of pktq 'pq'
 *
 * @param pq    Multi-priority packet queue
 * @param list  Single priority packet queue
 */
void
pktqprio_append(struct pktq *pq, int prio, struct spktq *list)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	struct pktq_prio *list_q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q;

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prio >= 0 && prio < pq->num_prio);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktqprio_full(pq, prio));

	q = &pq->q[prio];

	if (q->head)
		PKTSETLINK(q->tail, list_q->head);
	else
		q->head = list_q->head;

	q->tail = list_q->tail;
	q->n_pkts += list_q->n_pkts;
	pq->n_pkts_tot += list_q->n_pkts;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->n_pkts = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/*
 * Append spktq 'list' to the tail of spktq 'spq'
 * @param spq : single priority packet queue
 */
void BCMFASTPATH
spktq_enq_list(struct spktq *spq, struct spktq *list)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	struct pktq_prio *list_q;			/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q;

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!spktq_full(spq));

	q = &spq->q;

	if (q->head)
		PKTSETLINK(q->tail, list_q->head);
	else
		q->head = list_q->head;

	q->tail = list_q->tail;
	q->n_pkts += list_q->n_pkts;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->n_pkts = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/**
 * Prepend spktq 'list' to the head of pktq 'pq'
 * @param pq     Multi-priority packet queue
 * @param list   Single priority packet queue
 */
void
pktqprio_prepend(struct pktq *pq, int prio, struct spktq *list)
{
	struct pktq_prio *q;			/**< single priority packet queue */
	struct pktq_prio *list_q;		/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q;

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prio >= 0 && prio < pq->num_prio);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktqprio_full(pq, prio));

	q = &pq->q[prio];

	/* set the tail packet of list to point at the former pq head */
	PKTSETLINK(list_q->tail, q->head);
	/* the new q head is the head of list */
	q->head = list_q->head;

	/* If the q tail was non-null, then it stays as is.
	 * If the q tail was null, it is now the tail of list
	 */
	if (q->tail == NULL) {
		q->tail = list_q->tail;
	}

	q->n_pkts += list_q->n_pkts;
	pq->n_pkts_tot += list_q->n_pkts;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->n_pkts = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/*
 * Prepend spktq 'list' to the head of spktq 'spq'
 * @param spq   Single priority packet queue
 * @param list  Single priority packet queue
 */
void BCMFASTPATH
spktq_enq_head_list(struct spktq *spq, struct spktq *list)
{
	struct pktq_prio *q;			/**< single priority packet queue */
	struct pktq_prio *list_q;		/**< single priority packet queue */

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q;

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!spktq_full(spq));

	q = &spq->q;

	/* set the tail packet of list to point at the former pq head */
	PKTSETLINK(list_q->tail, q->head);
	/* the new q head is the head of list */
	q->head = list_q->head;

	/* If the q tail was non-null, then it stays as is.
	 * If the q tail was null, it is now the tail of list
	 */
	if (q->tail == NULL) {
		q->tail = list_q->tail;
	}

	q->n_pkts += list_q->n_pkts;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->n_pkts = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/** @param pq   Multi-priority packet queue */
void *
pktqprio_pdeq_prev(struct pktq *pq, int prio, void *prev_p)
{
	struct pktq_prio *q;			/**< single priority packet queue */
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];

	if (prev_p == NULL)
		goto done;

	if ((p = PKTLINK(prev_p)) == NULL)
		goto done;

	q->n_pkts--;

	pq->n_pkts_tot--;

	PKTSETLINK(prev_p, PKTLINK(p));
	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
bool
pktqprio_pdel(struct pktq *pq, void *pktbuf, int prio)
{
	bool ret = FALSE;
	struct pktq_prio *q;			/**< single priority packet queue */
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(prio >= 0 && prio < pq->num_prio);

	/* Should this just assert pktbuf? */
	if (!pktbuf)
		goto done;

	q = &pq->q[prio];

	if (q->head == pktbuf) {
		if ((q->head = PKTLINK(pktbuf)) == NULL)
			q->tail = NULL;
	} else {
		for (p = q->head; p && PKTLINK(p) != pktbuf; p = PKTLINK(p))
			;
		if (p == NULL)
			goto done;

		PKTSETLINK(p, PKTLINK(pktbuf));
		if (q->tail == pktbuf)
			q->tail = p;
	}

	q->n_pkts--;
	pq->n_pkts_tot--;

	PKTSETLINK(pktbuf, NULL);
	ret = TRUE;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

/** @param pq   Multi-priority packet queue */
bool
pktq_init(struct pktq *pq, int num_prio, int max_pkts)
{
	int idx;

	ASSERT(num_prio > 0 && num_prio <= PKTQ_MAX_PRIO);

	/* pq is variable size; only zero out what's requested */
	bzero(pq, OFFSETOF(struct pktq, q) + (sizeof(struct pktq_prio) * num_prio));

	if (HND_PKTQ_MUTEX_CREATE("pktq", &pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	pq->num_prio = (uint16)num_prio;

	pq->max_pkts = (uint16)max_pkts;

	pq->prio_to_idx = default_prio_to_idx_to_prio;
	pq->idx_to_prio = default_prio_to_idx_to_prio;

	for (idx = 0; idx < num_prio; idx++)
		pq->q[idx].max_pkts = pq->max_pkts;

	return TRUE;
}

void
pktq_config_prec(e_pktq_type type, struct pktq *pq, uint8 *prio_to_idx, uint8 *idx_to_prio)
{
	pq->pktq_type = type;
	pq->prio_to_idx = prio_to_idx;
	pq->idx_to_prio = idx_to_prio;
}

/** @param spq : single priority packet queue */
bool
spktq_init(struct spktq *spq, int max_pkts)
{
	bzero(spq, sizeof(struct spktq));

	if (HND_PKTQ_MUTEX_CREATE("spktq", &spq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	spq->q.max_pkts = (uint16)max_pkts;

	return TRUE;
}

/** @param spq : single priority packet queue */
bool
spktq_init_list(struct spktq *spq, uint max_pkts, void *head, void *tail, uint16 n_pkts)
{
	if (HND_PKTQ_MUTEX_CREATE("spktq", &spq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(PKTLINK(tail) == NULL);
	PKTSETLINK(tail, NULL);
	spq->q.head = head;
	spq->q.tail = tail;
	spq->q.max_pkts = (uint16)max_pkts;
	spq->q.n_pkts = n_pkts;
	spq->q.stall_count = 0;
	spq->q.dequeue_count = 0;

	return TRUE;
}

#ifdef HND_PKTQ_THREAD_SAFE
/** @param pq   Multi-priority packet queue */
void
pktq_deinit(struct pktq *pq)
{
	BCM_REFERENCE(pq);
	HND_PKTQ_MUTEX_DELETE(&pq->mutex);
}

/** @param spq : single priority packet queue */
void
spktq_deinit(struct spktq *spq)
{
	BCM_REFERENCE(spq);
	HND_PKTQ_MUTEX_DELETE(&spq->mutex);
}
#endif /* HND_PKTQ_THREAD_SAFE */

/** @param pq   Multi-priority packet queue */
void
pktq_set_max_plen(struct pktq *pq, int prio, int max_pkts)
{
	if (max_pkts > PKTQ_LEN_MAX) {
		max_pkts = PKTQ_LEN_MAX;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	ASSERT(prio >= 0 && prio < pq->num_prio);
	pq->q[prio].max_pkts = (uint16)max_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/** @param pq   Multi-priority packet queue */
void * BCMFASTPATH
pktq_deq(struct pktq *pq, int *prio_out)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p = NULL;
	int idx;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	idx = pq->num_prio - 1;
	while (idx && pq->q[pq->idx_to_prio[idx]].head == NULL)
		idx--;

	q = &pq->q[pq->idx_to_prio[idx]];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->n_pkts--;

	pq->n_pkts_tot--;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void *
pktq_deq_tail(struct pktq *pq, int *prio_out)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p = NULL, *prev;
	int idx;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	for (idx = 0; idx < pq->num_prio; idx++)
		if (pq->q[pq->idx_to_prio[idx]].head)
			break;

	if (idx == pq->num_prio)
		goto done;

	q = &pq->q[pq->idx_to_prio[idx]];

	p = q->head;

	for (prev = NULL; p != q->tail; p = PKTLINK(p))
		prev = p;

	if (prev)
		PKTSETLINK(prev, NULL);
	else
		q->head = NULL;

	q->tail = prev;
	q->n_pkts--;

	pq->n_pkts_tot--;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void *
pktq_peek(struct pktq *pq, int *prio_out)
{
	int idx;
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	idx = pq->num_prio - 1;
	while (idx && pq->q[pq->idx_to_prio[idx]].head == NULL)
		idx--;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

	p = pq->q[pq->idx_to_prio[idx]].head;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param spq : single priority packet queue */
void *
spktq_peek(struct spktq *spq)
{
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (spq->q.n_pkts == 0)
		goto done;

	p = spq->q.head;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/** @param pq   Multi-priority packet queue */
void
pktq_pflush(osl_t *osh, struct pktq *pq, int prio, bool dir)
{
	void *p;

	/* no need for a mutex protection! */

	/* start with the head of the list */
	while ((p = pktqprio_pdeq(pq, prio)) != NULL) {

		/* delete this packet */
		PKTFREE(osh, p, dir);
	}
}

/** @param spq : single priority packet queue */
void
spktq_flush(osl_t *osh, struct spktq *spq, bool dir)
{
	void *p;

	/* no need for a mutex protection! */

	/* start with the head of the list */
	while ((p = spktq_deq(spq)) != NULL) {

		/* delete this packet */
		PKTFREE(osh, p, dir);
	}
}

/** @param pq   Multi-priority packet queue */
void
pktq_flush(osl_t *osh, struct pktq *pq, bool dir)
{
	bool flush = FALSE;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* Optimize flush, if pktq n_pkts_tot = 0, just return.
	 * pktq len of 0 means pktq's prio q's are all empty.
	 */
	if (pq->n_pkts_tot > 0) {
		flush = TRUE;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;

	if (flush) {
		int idx;

		PKTQ_IDX_ITER(pq, idx) {
			pktq_pflush(osh, pq, pq->idx_to_prio[idx], dir);
		}
	}
}

/**
 * Return sum of lengths of a specific set of priorities
 *
 * @param pq   Multi-priority packet queue
 */
int
pktq_mlen(struct pktq *pq, uint prio_bmp)
{
	int prio, len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	len = 0;

	for (prio = 0; prio < pq->num_prio; prio++)
		if (prio_bmp & (1 << prio))
			len += pq->q[prio].n_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return len;
}

/**
 * Priority peek from a specific set of priorities
 *
 * @param pq   Multi-priority packet queue
 */
void *
pktq_mpeek(struct pktq *pq, uint prio_bmp, int *prio_out)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p = NULL;
	int idx;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	idx = pq->num_prio - 1;
	while ((prio_bmp & (1 << pq->idx_to_prio[idx])) == 0 ||
		pq->q[pq->idx_to_prio[idx]].head == NULL) {
		if (idx == 0)
			goto done;
		idx--;
	}
	q = &pq->q[pq->idx_to_prio[idx]];

	if ((p = q->head) == NULL)
		goto done;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/**
 * Priority dequeue from a specific set of priorities
 *
 * @param pq   Multi-priority packet queue
 */
void * BCMFASTPATH
pktq_mdeq(struct pktq *pq, uint prio_bmp, int *prio_out)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	void *p = NULL;
	int idx;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->n_pkts_tot == 0)
		goto done;

	idx = pq->num_prio - 1;
	while (((prio_bmp & (1 << pq->idx_to_prio[idx])) == 0) ||
		(pq->q[pq->idx_to_prio[idx]].head == NULL)) {
		if (idx == 0)
			goto done;
		idx--;
	}

	q = &pq->q[pq->idx_to_prio[idx]];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->n_pkts--;

	if (prio_out)
		*prio_out = pq->idx_to_prio[idx];

	pq->n_pkts_tot--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

#ifdef HND_PKTQ_THREAD_SAFE
/** @param pq   Multi-priority packet queue */
int
pktqprio_avail_pkts(struct pktq *pq, int prio)
{
	int ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	ASSERT(idx >= 0 && idx < pq->num_prio);

	ret = pq->q[prio].max_pkts - pq->q[prio].n_pkts;
	if (ret < 0)
		ret = 0;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return ret;
}

/** @param pq   Multi-priority packet queue */
bool
pktqprio_full(struct pktq *pq, int prio)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(prio >= 0 && prio < pq->num_prio);

	ret = pq->q[prio].n_pkts >= pq->q[prio].max_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

/** @param pq   Multi-priority packet queue */
int
pktq_avail(struct pktq *pq)
{
	int ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	ret = pq->max_pkts - pq->n_pkts_tot;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return ret;
}

/** @param spq : single priority packet queue */
int
spktq_avail(struct spktq *spq)
{
	int ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	ret = spq->q.max_pkts - spq->q.n_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return ret;
}

/** @param pq   Multi-priority packet queue */
bool
pktq_full(struct pktq *pq)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ret = pq->n_pkts_tot >= pq->max_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

/** @param spq : single priority packet queue */
bool
spktq_full(struct spktq *spq)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ret = spq->q.n_pkts >= spq->q.max_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

#endif	/* HND_PKTQ_THREAD_SAFE */

/** @param pq   Multi-priority packet queue */
void
pktq_pfilter(struct pktq *pq, int prio, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx)
{
	struct pktq_prio wq;			/**< single priority packet queue */
	struct pktq_prio *q;			/**< single priority packet queue */
	void *p;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* move the prio queue aside to a work queue */
	q = &pq->q[prio];

	wq = *q;

	q->head = NULL;
	q->tail = NULL;
	q->n_pkts = 0;
	pq->n_pkts_tot -= wq.n_pkts;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;

	/* start with the head of the work queue */
	while ((p = wq.head) != NULL) {
		/* unlink the current packet from the list */
		wq.head = PKTLINK(p);
		PKTSETLINK(p, NULL);
		wq.n_pkts--;

		/* call the filter function on current packet */
		ASSERT(fltr != NULL);
		switch ((*fltr)(fltr_ctx, p)) {
		case PKT_FILTER_NOACTION:
			/* put this packet back */
			pktqprio_penq(pq, prio, p);
			break;

		case PKT_FILTER_DELETE:
			/* delete this packet */
			ASSERT(defer != NULL);
			(*defer)(defer_ctx, p);
			break;

		case PKT_FILTER_REMOVE:
			/* pkt already removed from list */
			break;

		default:
			ASSERT(0);
			break;
		}
	}

	ASSERT(wq.n_pkts == 0);
}

/** filter a pktq, using the caller supplied filter/deposition/flush functions
 * @param pq   Multi-priority packet queue
 */
void
pktq_filter(struct pktq *pq, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx, flush_free_pkt_fn_t flush, void *flush_ctx)
{
	bool filter = FALSE;
	int prio;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* Optimize if pktq n_pkts = 0, just return.
	 * pktq len of 0 means pktq's prio q's are all empty.
	 */
	if (pq->n_pkts_tot > 0) {
		filter = TRUE;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;

	if (filter) {
		for (prio = 0; prio < pq->num_prio; prio++) {
			pktq_pfilter(pq, prio, fltr, fltr_ctx, defer, defer_ctx);
		}

		ASSERT(flush != NULL);
		(*flush)(flush_ctx);
	}
}

/** filter a pktq, using the caller supplied filter/deposition/flush functions
 * @param spq : single priority packet queue
 */
void
spktq_filter(struct spktq *spq, pktq_filter_t fltr, void *fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx, flush_free_pkt_fn_t flush, void *flush_ctx)
{
	bool filter = FALSE;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* Optimize if pktq n_pkts = 0, just return.
	 * pktq len of 0 means pktq's prio q's are all empty.
	 */
	if (spktq_n_pkts(spq) > 0) {
		filter = TRUE;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
		return;

	if (filter) {
		struct pktq_prio wq;			/**< single priority packet queue */
		struct pktq_prio *q;			/**< single priority packet queue */
		void *p;

		/* protect shared resource */
		if (HND_PKTQ_MUTEX_ACQUIRE(&spq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
			return;

		/* move the prio queue aside to a work queue */
		q = &spq->q;

		wq = *q;

		q->head = NULL;
		q->tail = NULL;
		q->n_pkts = 0;

		/* protect shared resource */
		if (HND_PKTQ_MUTEX_RELEASE(&spq->mutex) != OSL_EXT_SUCCESS)
			return;

		/* start with the head of the work queue */
		while ((p = wq.head) != NULL) {
			/* unlink the current packet from the list */
			wq.head = PKTLINK(p);
			PKTSETLINK(p, NULL);
			wq.n_pkts--;

			/* call the filter function on current packet */
			ASSERT(fltr != NULL);
			switch ((*fltr)(fltr_ctx, p)) {
			case PKT_FILTER_NOACTION:
				/* put this packet back */
				spktq_enq(spq, p);
				break;

			case PKT_FILTER_DELETE:
				/* delete this packet */
				ASSERT(defer != NULL);
				(*defer)(defer_ctx, p);
				break;

			case PKT_FILTER_REMOVE:
				/* pkt already removed from list */
				break;

			default:
				ASSERT(0);
				break;
			}
		}

		ASSERT(wq.n_pkts == 0);

		ASSERT(flush != NULL);
		(*flush)(flush_ctx);
	}
}

typedef struct {
	spktq_cb_t cb;
	void *arg;
} spktq_cbinfo_t;
static spktq_cbinfo_t spktq_cbinfo = {NULL, NULL};
static spktq_cbinfo_t *spktq_cbinfo_get(void);

static spktq_cbinfo_t*
BCMRAMFN(spktq_cbinfo_get)(void)
{
	return (&spktq_cbinfo);
}

void
BCMATTACHFN(spktq_free_register)(spktq_cb_t cb, void *arg)
{
	spktq_cbinfo_t *cbinfop = spktq_cbinfo_get();
	cbinfop->cb = cb;
	cbinfop->arg = arg;
}

void
spktq_cb(void *spq)
{
	spktq_cbinfo_t *cbinfop = spktq_cbinfo_get();
	if (cbinfop->cb) {
		cbinfop->cb(cbinfop->arg, spq);
	}
}

#ifdef HNDPQP
/* Total packets in the queue including host and dongle packets */
int BCMFASTPATH
pktq_pqp_pkt_cnt(struct pktq *pq, int prio)
{
	struct pktq_prio *q;

	ASSERT(pq);
	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];
	return pqp_pkt_cnt(q);
}

/* Host resident packet count */
int
pktq_pqp_hbm_cnt(struct pktq *pq, int prio)
{
	struct pktq_prio *q;

	ASSERT(pq);
	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];
	return pqp_hbm_cnt(q);
}

/* Dongle resident packet count */
int
pktq_pqp_dbm_cnt(struct pktq *pq, int prio)
{
	struct pktq_prio *q;

	ASSERT(pq);
	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];
	return pqp_dbm_cnt(q);
}

/**
 * Return sum of lengths of a specific set of priorities
 *
 * @param pq   Multi-priority packet queue
 */
int
pktq_pqp_mlen(struct pktq *pq, uint prio_bmp)
{
	int idx, len;

	len = 0;

	for (idx = 0; idx < pq->num_prio; idx++) {
		if (prio_bmp & (1 << idx)) {
			len += pqp_pkt_cnt(&pq->q[idx]);
		}
	}

	return len;
}

/* Returns if queue owned by PQP for a given prio */
bool BCMFASTPATH
pktq_pqp_owns(struct pktq *pq, int prio)
{
	struct pktq_prio *q;

	ASSERT(pq);
	ASSERT(prio >= 0 && prio < pq->num_prio);

	q = &pq->q[prio];
	return pqp_owns(q);
}

/* Check if any of the prio is owned by PQP */
bool BCMFASTPATH
pktq_mpqp_owns(struct pktq *pq, uint prio_bmp)
{
	int idx;

	ASSERT(pq);

	PKTQ_IDX_ITER(pq, idx) {
		if ((prio_bmp & (1 << pq->idx_to_prio[idx])) == 0)
			continue;

		/* Check if owned by PQP */
		if (pqp_owns(&pq->q[pq->idx_to_prio[idx]]))
			return TRUE;
	}
	return FALSE;
}

/* #### PQP Page in utilities #### */

/**
 * Priority PQP Page IN from a specific set of priorities.
 *
 * Given a multi prio pktq and a prio bitmap, find the highest prio with
 * packets queued either in host or dongle.
 * For the highest prio with available packet, if head packet is in dongle
 * return immediately without PGI.
 * If packets available but head packet not in dongle, initiate a PQP PGI for a single packet.
 */
bool BCMFASTPATH
pktq_mpqp_pgi(struct pktq *pq, uint prio_bmp, int flags)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	int idx;

	ASSERT(pq);

	/* Check if PQP owns any of the prio queues */
	if (!pktq_mpqp_owns(pq, prio_bmp))
		goto done;

	/* Starting from pq->num_prio - 1, find a prio with available packets */
	idx = pq->num_prio - 1;
	while (idx && (pktq_pqp_pkt_cnt(pq, pq->idx_to_prio[idx]) == 0))
		idx--;

	/* Loop through the prio_bit map to find a prio with available packets */
	while ((pktq_pqp_pkt_cnt(pq, pq->idx_to_prio[idx]) == 0) ||
		((prio_bmp & (1 << pq->idx_to_prio[idx])) == 0))
		if (idx-- == 0)
			goto done;

	/* Atleast 1 packet avaialble for the given prio */
	ASSERT(pktq_pqp_pkt_cnt(pq, pq->idx_to_prio[idx]));

	q = &pq->q[pq->idx_to_prio[idx]];

	/* Head packet in Dongle. Return without PGI */
	if ((q->head) != NULL)
		goto done;

	/* Packet in the host. Trigger a Page IN for one packet */
	pqp_pgi(q, 1, 1, flags);

	/* PGI will be resumed through callbacks when resource is available. */
	if ((q->head) == NULL)
		return FALSE;
done:
	return TRUE;
}

/* PQP page in for a given prio */
void BCMFASTPATH
pktq_pqp_pgi(struct pktq *pq, int prio, int cont_pkts, int fill_pkts, int flags)
{
	int __cont_pkts, __fill_pkts, pqp_pkt_cnt;
	struct pktq_prio *q;				/**< single priority packet queue */

	ASSERT(pq);

	q = &(pq->q[prio]);

	/* Check for PQP ownership */
	if (!pqp_owns(q))
		return;

	pqp_pkt_cnt = pktq_pqp_pkt_cnt(pq, prio);

	/* Check if we need to drain the whole queue */
	__fill_pkts = (fill_pkts == PKTQ_PKTS_ALL) ? pqp_pkt_cnt : fill_pkts;

	__cont_pkts = (cont_pkts == PKTQ_PKTS_ALL) ? pqp_pkt_cnt : cont_pkts;

	ASSERT(__fill_pkts <= pqp_pkt_cnt);
	ASSERT(__cont_pkts <= pqp_pkt_cnt);

	/* Page in requested count */
	pqp_pgi(q, __cont_pkts, __fill_pkts, flags);
}

/* SPKTQ PQP page in */
void
spktq_pqp_pgi(struct spktq* spq, int cont_pkts, int fill_pkts, int flags)
{
	int __cont_pkts, __fill_pkts, pkt_cnt;
	struct pktq_prio *q;				/**< single priority packet queue */
	ASSERT(spq);

	q = &spq->q;

	/* Check for PQP ownership */
	if (!pqp_owns(q))
		return;

	pkt_cnt = pqp_pkt_cnt(q);

	/* Check if we need to drain the whole queue */
	__fill_pkts = (fill_pkts == PKTQ_PKTS_ALL) ? pkt_cnt : fill_pkts;

	__cont_pkts = (cont_pkts == PKTQ_PKTS_ALL) ? pkt_cnt : cont_pkts;

	ASSERT(__fill_pkts <= pkt_cnt);
	ASSERT(__cont_pkts <= pkt_cnt);

	/* Page in requested count */
	pqp_pgi(q, __cont_pkts, __fill_pkts, flags);
}

/* #### PQP Page out utilities #### */

/* SPKTQ PQP page out */
void
spktq_pqp_pgo(struct spktq* spq, int policy, void* cb_ctx)
{
	struct pktq_prio *q;				/**< single priority packet queue */
	ASSERT(spq);

	q = &spq->q;

	/* Page Out the single prio queue */
	if (q->n_pkts) {
		pqp_pgo(q, policy, cb_ctx);
	}
}

/* PQP page out for a given prio bmp */
void BCMFASTPATH
pktq_pqp_pgo(struct pktq *pq, uint prio_bmp, int policy, void* cb_ctx)
{
	int idx;
	struct pktq_prio *q;				/**< single priority packet queue */

	ASSERT(pq);

	PKTQ_IDX_ITER(pq, idx) {
		if ((prio_bmp & (1 << pq->idx_to_prio[idx])) == 0)
			continue;

		q = &(pq->q[pq->idx_to_prio[idx]]);

		/* Page Out the single prio queue */
		if (q->n_pkts) {
			pqp_pgo(q, policy, cb_ctx);
		}
	}
}

/* PQP page out for a given specific prio */
void BCMFASTPATH
pktq_prio_pqp_pgo(struct pktq *pq, int prio, int policy, void* cb_ctx)
{
	struct pktq_prio *q;				/**< single priority packet queue */

	ASSERT(pq);

	q = &(pq->q[prio]);

	/* Page Out the single prio queue */
	if (q->n_pkts) {
		pqp_pgo(q, policy, cb_ctx);
	}
}

/* PQP, join a spktq with a pktq (by prio) */
void BCMFASTPATH
pktq_prio_pqp_join(struct pktq *pq, int prio, struct pktq_prio *q_B, int policy, void* cb_ctx)
{
	struct pktq_prio *q_A;				/**< single priority packet queue */

	ASSERT(pq);

	q_A = &(pq->q[prio]);

	pq->n_pkts_tot += pqp_pkt_cnt(q_B);
	pqp_join(q_A, q_B, policy, cb_ctx);
}

#endif /* HNDPQP */
