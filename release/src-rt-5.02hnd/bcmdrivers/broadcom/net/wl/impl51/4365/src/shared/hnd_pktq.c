/*
 * HND generic pktq operation primitives
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

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <hnd_pktq.h>

/* mutex macros for thread safe */
#ifdef HND_PKTQ_THREAD_SAFE
#define HND_PKTQ_MUTEX_CREATE(name, mutex)	osl_ext_mutex_create(name, mutex)
#define HND_PKTQ_MUTEX_DELETE(mutex)		osl_ext_mutex_delete(mutex)
#define HND_PKTQ_MUTEX_ACQUIRE(mutex, msec)	osl_ext_mutex_acquire(mutex, msec)
#define HND_PKTQ_MUTEX_RELEASE(mutex)		osl_ext_mutex_release(mutex)
#else
#define HND_PKTQ_MUTEX_CREATE(name, mutex)	OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_DELETE(mutex)		OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_ACQUIRE(mutex, msec)	OSL_EXT_SUCCESS
#define HND_PKTQ_MUTEX_RELEASE(mutex)		OSL_EXT_SUCCESS
#endif

/*
 * osl multiple-precedence packet queue
 * hi_prec is always >= the number of the highest non-empty precedence
 */
void * BCMFASTPATH
pktq_penq(struct pktq *pq, int prec, void *p)
{
	struct pktq_prec *q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(p) == NULL);         /* queueing chains not allowed */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktq_pfull(pq, prec));

	q = &pq->q[prec];

	if (q->head)
		PKTSETLINK(q->tail, p);
	else
		q->head = p;

	q->tail = p;
	q->len++;

	pq->len++;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_penq_head(struct pktq *pq, int prec, void *p)
{
	struct pktq_prec *q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(p) == NULL);         /* queueing chains not allowed */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktq_pfull(pq, prec));

	q = &pq->q[prec];

	if (q->head == NULL)
		q->tail = p;

	PKTSETLINK(p, q->head);
	q->head = p;
	q->len++;

	pq->len++;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

/*
 * Append spktq 'list' to the tail of pktq 'pq'
 */
void BCMFASTPATH
pktq_append(struct pktq *pq, int prec, struct spktq *list)
{
	struct pktq_prec *q;
	struct pktq_prec *list_q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q[0];

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktq_pfull(pq, prec));

	q = &pq->q[prec];

	if (q->head)
		PKTSETLINK(q->tail, list_q->head);
	else
		q->head = list_q->head;

	q->tail = list_q->tail;
	q->len += list_q->len;
	pq->len += list_q->len;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->len = 0;
	list->len = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/*
 * Prepend spktq 'list' to the head of pktq 'pq'
 */
void BCMFASTPATH
pktq_prepend(struct pktq *pq, int prec, struct spktq *list)
{
	struct pktq_prec *q;
	struct pktq_prec *list_q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q[0];

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_full(pq));
	ASSERT(!pktq_pfull(pq, prec));

	q = &pq->q[prec];

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

	q->len += list_q->len;
	pq->len += list_q->len;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->len = 0;
	list->len = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

void * BCMFASTPATH
pktq_pdeq(struct pktq *pq, int prec)
{
	struct pktq_prec *q;
	void *p;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->len--;

	pq->len--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_pdeq_prev(struct pktq *pq, int prec, void *prev_p)
{
	struct pktq_prec *q;
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];

	if (prev_p == NULL)
		goto done;

	if ((p = PKTLINK(prev_p)) == NULL)
		goto done;

	q->len--;

	pq->len--;

	PKTSETLINK(prev_p, PKTLINK(p));
	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_pdeq_with_fn(struct pktq *pq, int prec, ifpkt_cb_t fn, int arg)
{
	struct pktq_prec *q;
	void *p, *prev = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];
	p = q->head;

	while (p) {
		if (fn == NULL || (*fn)(p, arg)) {
			break;
		} else {
			prev = p;
			p = PKTLINK(p);
		}
	}
	if (p == NULL)
		goto done;

	if (prev == NULL) {
		if ((q->head = PKTLINK(p)) == NULL) {
			q->tail = NULL;
		}
	} else {
		PKTSETLINK(prev, PKTLINK(p));
		if (q->tail == p) {
			q->tail = prev;
		}
	}

	q->len--;

	pq->len--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_pdeq_tail(struct pktq *pq, int prec)
{
	struct pktq_prec *q;
	void *p, *prev;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	for (prev = NULL; p != q->tail; p = PKTLINK(p))
		prev = p;

	if (prev)
		PKTSETLINK(prev, NULL);
	else
		q->head = NULL;

	q->tail = prev;
	q->len--;

	pq->len--;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void
pktq_pflush(osl_t *osh, struct pktq *pq, int prec, bool dir, ifpkt_cb_t fn, int arg)
{
	struct pktq_prec *q;
	void *p, *prev = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	q = &pq->q[prec];
	p = q->head;
	while (p) {
		if (fn == NULL || (*fn)(p, arg)) {
			bool head = (p == q->head);
			if (head)
				q->head = PKTLINK(p);
			else
				PKTSETLINK(prev, PKTLINK(p));
			PKTSETLINK(p, NULL);
			PKTFREE(osh, p, dir);
			q->len--;
			pq->len--;
			p = (head ? q->head : PKTLINK(prev));
		} else {
			prev = p;
			p = PKTLINK(p);
		}
	}

	if (q->head == NULL) {
		ASSERT(q->len == 0);
		q->tail = NULL;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

bool BCMFASTPATH
pktq_pdel(struct pktq *pq, void *pktbuf, int prec)
{
	bool ret = FALSE;
	struct pktq_prec *q;
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	/* Should this just assert pktbuf? */
	if (!pktbuf)
		goto done;

	q = &pq->q[prec];

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

	q->len--;
	pq->len--;
	PKTSETLINK(pktbuf, NULL);
	ret = TRUE;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}


static void
_pktq_pfilter(struct pktq *pq, int prec, pktq_filter_t fltr, void* fltr_ctx,
              defer_free_pkt_fn_t defer, void *defer_ctx)
{
	struct pktq_prec wq;
	struct pktq_prec *q;
	void *p;
	uint enq_len = 0;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* move the prec queue aside to a work queue */
	q = &pq->q[prec];

	wq = *q;

	q->head = NULL;
	q->tail = NULL;
	q->len = 0;

#ifdef WL_TXQ_STALL
	q->dequeue_count += wq.len;
#endif

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;

	/* start with the head of the work queue */
	while ((p = wq.head) != NULL) {
		/* unlink the current packet from the list */
		wq.head = PKTLINK(p);
		PKTSETLINK(p, NULL);
		wq.len--;

#ifdef WL_TXQ_STALL
		wq.dequeue_count++;
#endif

		/* call the filter function on current packet */
		ASSERT(fltr != NULL);
		switch ((*fltr)(fltr_ctx, p)) {
		case PKT_FILTER_NOACTION:
			/* put this packet back */
			pktq_penq(pq, prec, p);
			break;

		case PKT_FILTER_DELETE:
			/* delete this packet */
			ASSERT(defer != NULL);
			(*defer)(defer_ctx, p);
			enq_len++;
			break;

		case PKT_FILTER_REMOVE:
			/* pkt already removed from list */
			break;

		default:
			ASSERT(0);
			break;
		}
	}

	ASSERT(wq.len == 0);
}


void
pktq_pfilter(struct pktq *pq, int prec, pktq_filter_t fltr, void* fltr_ctx,
	defer_free_pkt_fn_t defer, void *defer_ctx, flush_free_pkt_fn_t flush, void *flush_ctx)
{
	_pktq_pfilter(pq, prec, fltr, fltr_ctx, defer, defer_ctx);

	ASSERT(flush != NULL);
	(*flush)(flush_ctx);
}


bool
pktq_init(struct pktq *pq, int num_prec, int max_len)
{
	int prec;

	if (HND_PKTQ_MUTEX_CREATE("pktq", &pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(num_prec > 0 && num_prec <= PKTQ_MAX_PREC);

	/* pq is variable size; only zero out what's requested */
	bzero(pq, OFFSETOF(struct pktq, q) + (sizeof(struct pktq_prec) * num_prec));

	pq->num_prec = (uint16)num_prec;

	pq->max = (uint16)max_len;

	for (prec = 0; prec < num_prec; prec++)
		pq->q[prec].max = pq->max;

	return TRUE;
}

bool
pktq_deinit(struct pktq *pq)
{
	if (HND_PKTQ_MUTEX_DELETE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return TRUE;
}

void
pktq_set_max_plen(struct pktq *pq, int prec, int max_len)
{
	ASSERT(prec >= 0 && prec < pq->num_prec);

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	if (prec < pq->num_prec)
		pq->q[prec].max = (uint16)max_len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

void * BCMFASTPATH
pktq_deq(struct pktq *pq, int *prec_out)
{
	struct pktq_prec *q;
	void *p = NULL;
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	while ((prec = pq->hi_prec) > 0 && pq->q[prec].head == NULL)
		pq->hi_prec--;

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->len--;

	pq->len--;

	if (prec_out)
		*prec_out = prec;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_deq_tail(struct pktq *pq, int *prec_out)
{
	struct pktq_prec *q;
	void *p = NULL, *prev;
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	for (prec = 0; prec < pq->hi_prec; prec++)
		if (pq->q[prec].head)
			break;

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	for (prev = NULL; p != q->tail; p = PKTLINK(p))
		prev = p;

	if (prev)
		PKTSETLINK(prev, NULL);
	else
		q->head = NULL;

	q->tail = prev;
	q->len--;

	pq->len--;

	if (prec_out)
		*prec_out = prec;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void *
pktq_peek(struct pktq *pq, int *prec_out)
{
	int prec;
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	while ((prec = pq->hi_prec) > 0 && pq->q[prec].head == NULL)
		pq->hi_prec--;

	if (prec_out)
		*prec_out = prec;

	p = pq->q[prec].head;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void *
pktq_peek_tail(struct pktq *pq, int *prec_out)
{
	int prec;
	void *p = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	for (prec = 0; prec < pq->hi_prec; prec++)
		if (pq->q[prec].head)
			break;

	if (prec_out)
		*prec_out = prec;

	p = pq->q[prec].tail;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void
pktq_flush(osl_t *osh, struct pktq *pq, bool dir, ifpkt_cb_t fn, int arg)
{
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	/* Optimize flush, if pktq len = 0, just return.
	 * pktq len of 0 means pktq's prec q's are all empty.
	 */
	if (pq->len == 0)
		goto done;

	for (prec = 0; prec < pq->num_prec; prec++)
		pktq_pflush(osh, pq, prec, dir, fn, arg);
	if (fn == NULL)
		ASSERT(pq->len == 0);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/* Return sum of lengths of a specific set of precedences */
int
pktq_mlen(struct pktq *pq, uint prec_bmp)
{
	int prec, len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	len = 0;

	for (prec = 0; prec <= pq->hi_prec; prec++)
		if (prec_bmp & (1 << prec))
			len += pq->q[prec].len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return len;
}

/* Priority peek from a specific set of precedences */
void * BCMFASTPATH
pktq_mpeek(struct pktq *pq, uint prec_bmp, int *prec_out)
{
	struct pktq_prec *q;
	void *p = NULL;
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	while ((prec = pq->hi_prec) > 0 && pq->q[prec].head == NULL)
		pq->hi_prec--;

	while ((prec_bmp & (1 << prec)) == 0 || pq->q[prec].head == NULL)
		if (prec-- == 0)
			goto done;

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if (prec_out)
		*prec_out = prec;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}
/* Priority dequeue from a specific set of precedences */
void * BCMFASTPATH
pktq_mdeq(struct pktq *pq, uint prec_bmp, int *prec_out)
{
	struct pktq_prec *q;
	void *p = NULL;
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	while ((prec = pq->hi_prec) > 0 && pq->q[prec].head == NULL)
		pq->hi_prec--;

	while ((pq->q[prec].head == NULL) || ((prec_bmp & (1 << prec)) == 0))
		if (prec-- == 0)
			goto done;

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->len--;

	if (prec_out)
		*prec_out = prec;

	pq->len--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

#ifdef HND_PKTQ_THREAD_SAFE
int
pktq_pavail(struct pktq *pq, int prec)
{
	int ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	ret = pq->q[prec].max - pq->q[prec].len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return ret;
}

bool
pktq_pfull(struct pktq *pq, int prec)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	ret = pq->q[prec].len >= pq->q[prec].max;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

int
pktq_avail(struct pktq *pq)
{
	int ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	ret = pq->max - pq->len;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return 0;

	return ret;
}

bool
pktq_full(struct pktq *pq)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ret = pq->len >= pq->max;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

bool
pktq_swfull(struct swpktq *pq, int prec)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	ret = pq->q[prec].len >= pq->q[prec].max;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}

bool
pktq_sw_full(struct swpktq *pq)
{
	bool ret;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return FALSE;

	ret = pq->len >= pq->max;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	return ret;
}
#endif	/* HND_PKTQ_THREAD_SAFE */

void * BCMFASTPATH
pktq_swdeq(struct swpktq *pq, int prec)
{
	struct pktq_prec *q;
	void *p;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->len--;

	pq->len--;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_swenq(struct swpktq *pq, int prec, void *p)
{
	struct pktq_prec *q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(p) == NULL);         /* queueing chains not allowed */

	ASSERT(!pktq_sw_full(pq));
	ASSERT(!pktq_swfull(pq, prec));

	q = &pq->q[prec];

	if (q->head)
		PKTSETLINK(q->tail, p);
	else
		q->head = p;

	q->tail = p;
	q->len++;

	pq->len++;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void * BCMFASTPATH
pktq_swenq_head(struct swpktq *pq, int prec, void *p)
{
	struct pktq_prec *q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(p) == NULL);         /* queueing chains not allowed */

	ASSERT(!pktq_sw_full(pq));
	ASSERT(!pktq_swfull(pq, prec));

	q = &pq->q[prec];

	if (q->head == NULL)
		q->tail = p;

	PKTSETLINK(p, q->head);
	q->head = p;
	q->len++;

	pq->len++;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}

void
pktq_swflush(osl_t *osh, struct swpktq *pq, int prec, bool dir, ifpkt_cb_t fn, int arg)
{
	struct pktq_prec *q;
	void *p, *prev = NULL;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	q = &pq->q[prec];
	p = q->head;
	while (p) {
		if (fn == NULL || (*fn)(p, arg)) {
			bool head = (p == q->head);
			if (head)
				q->head = PKTLINK(p);
			else
				PKTSETLINK(prev, PKTLINK(p));
			PKTSETLINK(p, NULL);
			PKTFREE(osh, p, dir);
			q->len--;
			pq->len--;
			p = (head ? q->head : PKTLINK(prev));
		} else {
			prev = p;
			p = PKTLINK(p);
		}
	}

	if (q->head == NULL) {
		ASSERT(q->len == 0);
		q->tail = NULL;
	}

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

/*
 * Append spktq 'list' to the tail of pktq 'pq'
 */
void BCMFASTPATH
pktq_sw_append(struct swpktq *pq, int prec, struct spktq *list)
{
	struct pktq_prec *q;
	struct pktq_prec *list_q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q[0];

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_sw_full(pq));
	ASSERT(!pktq_swfull(pq, prec));

	q = &pq->q[prec];

	if (q->head)
		PKTSETLINK(q->tail, list_q->head);
	else
		q->head = list_q->head;

	q->tail = list_q->tail;
	q->len += list_q->len;
	pq->len += list_q->len;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->len = 0;
	list->len = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}

void * BCMFASTPATH
pktq_sw_deq(struct swpktq *pq, int *prec_out)
{
	struct pktq_prec *q;
	void *p = NULL;
	int prec;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return NULL;

	if (pq->len == 0)
		goto done;

	while ((prec = pq->hi_prec) > 0 && pq->q[prec].head == NULL)
		pq->hi_prec--;

	q = &pq->q[prec];

	if ((p = q->head) == NULL)
		goto done;

	if ((q->head = PKTLINK(p)) == NULL)
		q->tail = NULL;

	q->len--;

	pq->len--;

	if (prec_out)
		*prec_out = prec;

	PKTSETLINK(p, NULL);

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return NULL;

	return p;
}


bool
pktq_sw_init(struct swpktq *pq, int num_prec, int max_len)
{
	int prec;

	if (HND_PKTQ_MUTEX_CREATE("pktq_sw", &pq->mutex) != OSL_EXT_SUCCESS)
		return FALSE;

	ASSERT(num_prec > 0 && num_prec <= PKTQ_MAX_SW);

	/* pq is variable size; only zero out what's requested */
	bzero(pq, OFFSETOF(struct pktq, q) + (sizeof(struct pktq_prec) * num_prec));

	pq->num_prec = (uint16)num_prec;

	pq->max = (uint16)max_len;

	for (prec = 0; prec < num_prec; prec++)
		pq->q[prec].max = pq->max;

	return TRUE;
}

/*
 * Prepend spktq 'list' to the head of pktq 'pq'
 */
void BCMFASTPATH
pktq_sw_prepend(struct swpktq *pq, int prec, struct spktq *list)
{
	struct pktq_prec *q;
	struct pktq_prec *list_q;

	/* protect shared resource */
	if (HND_PKTQ_MUTEX_ACQUIRE(&pq->mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	list_q = &list->q[0];

	/* empty list check */
	if (list_q->head == NULL)
		goto done;

	ASSERT(prec >= 0 && prec < pq->num_prec);
	ASSERT(PKTLINK(list_q->tail) == NULL);         /* terminated list */

	ASSERT(!pktq_sw_full(pq));
	ASSERT(!pktq_swfull(pq, prec));

	q = &pq->q[prec];

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

	q->len += list_q->len;
	pq->len += list_q->len;

	if (pq->hi_prec < prec)
		pq->hi_prec = (uint8)prec;

	list_q->head = NULL;
	list_q->tail = NULL;
	list_q->len = 0;
	list->len = 0;

done:
	/* protect shared resource */
	if (HND_PKTQ_MUTEX_RELEASE(&pq->mutex) != OSL_EXT_SUCCESS)
		return;
}
