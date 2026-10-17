/*
 * Broadcom Dongle Host Driver (DHD), Generic work queue framework
 * Generic interface to handle dhd deferred work events
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: dhd_linux_wq.c 850500 2025-01-15 10:37:33Z $
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/kfifo.h>

#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>
#include <dhd_linux_wq.h>

struct dhd_deferred_event_t {
	u8	event; /* holds the event */
	void	*event_data; /* Holds event specific data */
	event_handler_t event_handler;
};
#define DEFRD_EVT_SIZE	sizeof(struct dhd_deferred_event_t)

struct dhd_deferred_wq {
	struct work_struct	deferred_work; /* should be the first member */

	/*
	 * work events may occur simultaneously.
	 * Can hold upto 64 low priority events and 4 high priority events
	 */
#define DHD_PRIO_FIFO_SIZE	(4 * sizeof(struct dhd_deferred_event_t))
#define DHD_WORK_FIFO_SIZE	(64 * sizeof(struct dhd_deferred_event_t))
	struct kfifo			*prio_fifo;
	struct kfifo			*work_fifo;
	u8				*prio_fifo_buf;
	u8				*work_fifo_buf;
	spinlock_t			work_lock;
	dhd_pub_t			*dhdp;
	u32				active_mask;
};

static struct kfifo*
dhd_kfifo_alloc(int size, spinlock_t *lock)
{
	struct kfifo *fifo;
	u8 *buf;

	buf = kzalloc(size, GFP_KERNEL);
	if (!buf) {
		return NULL;
	}

	fifo = (struct kfifo *)kzalloc(sizeof(struct kfifo), GFP_KERNEL);
	if (!fifo) {
		kfree(buf);
		return NULL;
	}
	kfifo_init(fifo, buf, size);
	return fifo;
}

static inline void
dhd_kfifo_free(struct kfifo *fifo)
{
	kfifo_free(fifo);
	kfree(fifo);
}

/* deferred work functions */
static void dhd_deferred_work_handler(struct work_struct *data);

struct dhd_deferred_wq*
dhd_deferred_work_init(dhd_pub_t *dhdp)
{
	struct dhd_deferred_wq	*work;

	ASSERT(dhdp);

	work = kzalloc(sizeof(*work), GFP_KERNEL);
	if (!work) {
		DHD_ERROR(("%s: work queue creation failed \n", __FUNCTION__));
		return NULL;
	}

	INIT_WORK(&work->deferred_work, dhd_deferred_work_handler);

	/* initialize event fifo */
	spin_lock_init(&work->work_lock);

	/* allocate prio event fifo */
	work->prio_fifo = dhd_kfifo_alloc(roundup_pow_of_two(DHD_PRIO_FIFO_SIZE), &work->work_lock);
	if (!work->prio_fifo) {
		goto return_null;
	}

	/* allocate normal event fifo */
	work->work_fifo = dhd_kfifo_alloc(roundup_pow_of_two(DHD_WORK_FIFO_SIZE), &work->work_lock);
	if (!work->work_fifo) {
		goto return_null;
	}

	work->dhdp = dhdp;
	work->active_mask = DHD_WQ_EVENTMASK;
	DHD_ERROR(("%s: work queue initialized \n", __FUNCTION__));
	return work;

return_null:
	dhd_deferred_work_deinit(dhdp);
	return NULL;
}

void
dhd_deferred_work_deinit(dhd_pub_t *dhdp)
{
	struct dhd_deferred_wq *work = dhdp->deferred_wq;

	if (!work) {
		DHD_ERROR(("%s: deferred work has been freed already\n", __FUNCTION__));
		return;
	}

	/* clear reference */
	dhdp->deferred_wq = NULL;

	/* cancel the deferred work handling */
	cancel_work_sync(&work->deferred_work);

	/*
	 * free work event fifo.
	 * kfifo_free frees locally allocated fifo buffer
	 */
	if (work->prio_fifo)
		dhd_kfifo_free(work->prio_fifo);

	if (work->work_fifo)
		dhd_kfifo_free(work->work_fifo);

	kfree(work);
}

/*
 *	Prepares event to be queued
 *	Schedules the event
 */
int
dhd_deferred_schedule_work(dhd_pub_t *dhdp, void *event_data, enum dhd_wq_event event,
	event_handler_t event_handler, u8 priority)
{
	struct dhd_deferred_wq *workq = dhdp->deferred_wq;
	struct dhd_deferred_event_t	deferred_event;
	struct kfifo *fifo;
	u32 event_flag = BIT(event);
	ulong   flags;
	int	status = 0;

	if (!workq) {
		DHD_ERROR(("%s: work queue not initialized \n", __FUNCTION__));
		ASSERT(0);
		return DHD_WQ_STS_UNINITIALIZED;
	}

	if (!(event_flag & DHD_WQ_EVENTMASK)) {
		DHD_ERROR(("%s: Unknown/invalid event: %08X (%08X)\n",
			__FUNCTION__, event_flag, DHD_WQ_EVENTMASK));
		return DHD_WQ_STS_UNKNOWN_EVENT;
	}

	if (!(workq->active_mask & event_flag)) {
		DHD_ERROR(("%s: event 0x%08X disabled (mask: 0x%08X)\n",
			__FUNCTION__, event_flag, workq->active_mask));
		return DHD_WQ_STS_EVENT_SKIPPED;
	}

	/*
	 * default element size is 1, which can be changed
	 * using kfifo_esize(). Older kernel(FC11) doesn't support
	 * changing element size. For compatibility changing
	 * element size is not prefered
	 */
	ASSERT(kfifo_esize(workq->prio_fifo) == 1);
	ASSERT(kfifo_esize(workq->work_fifo) == 1);

	deferred_event.event = event;
	deferred_event.event_data = event_data;
	deferred_event.event_handler = event_handler;

	fifo = priority == DHD_WORK_PRIORITY_HIGH ? workq->prio_fifo : workq->work_fifo;

	spin_lock_irqsave(&workq->work_lock, flags);
	if (kfifo_avail(fifo) >= DEFRD_EVT_SIZE)
		status = kfifo_in(fifo, &deferred_event, DEFRD_EVT_SIZE);
	spin_unlock_irqrestore(&workq->work_lock, flags);

	if (!status) {
		return DHD_WQ_STS_SCHED_FAILED;
	}
	schedule_work(&workq->deferred_work);
	return DHD_WQ_STS_OK;
}

static int
dhd_get_scheduled_work(struct dhd_deferred_wq *deferred_wq, struct dhd_deferred_event_t *event)
{
	int	status = 0;

	if (!deferred_wq) {
		DHD_ERROR(("%s: work queue not initialized \n", __FUNCTION__));
		return DHD_WQ_STS_UNINITIALIZED;
	}

	/*
	 * default element size is 1 byte, which can be changed
	 * using kfifo_esize(). Older kernel(FC11) doesn't support
	 * changing element size. For compatibility changing
	 * element size is not prefered
	 */
	ASSERT(kfifo_esize(deferred_wq->prio_fifo) == 1);
	ASSERT(kfifo_esize(deferred_wq->work_fifo) == 1);

	/* first read  priorit event fifo */
	status = kfifo_out_spinlocked(deferred_wq->prio_fifo, event,
		DEFRD_EVT_SIZE, &deferred_wq->work_lock);

	if (!status) {
		/* priority fifo is empty. Now read low prio work fifo */
		status = kfifo_out_spinlocked(deferred_wq->work_fifo, event,
			DEFRD_EVT_SIZE, &deferred_wq->work_lock);
	}

	return status;
}

/*
 *	Called when work is scheduled
 */
static void
dhd_deferred_work_handler(struct work_struct *work)
{
	struct dhd_deferred_wq		*wq;
	struct dhd_deferred_event_t	work_event;
	int				status;

	wq = container_of(work, struct dhd_deferred_wq, deferred_work);

	do {
		status = dhd_get_scheduled_work(wq, &work_event);
		DHD_TRACE(("%s: event to handle %d \n", __FUNCTION__, status));
		if (!status) {
			DHD_TRACE(("%s: No event to handle %d \n", __FUNCTION__, status));
			break;
		}

		if (work_event.event_handler) {
			work_event.event_handler(wq->dhdp, work_event.event_data, work_event.event);
		} else {
			DHD_ERROR(("%s: event not handled %d\n", __FUNCTION__, work_event.event));
		}
	} while (1);
	return;
}

void
dhd_deferred_work_enable(dhd_pub_t *dhdp, u32 event_mask)
{
	struct dhd_deferred_wq *work = dhdp->deferred_wq;

	if (!work || !event_mask || (event_mask & DHD_WQ_EVENTMASK) != event_mask) {
		DHD_ERROR(("%s: Invalid!!\n", __FUNCTION__));
		return;
	}

	work->active_mask |= event_mask;
}

bool
dhd_deferred_work_enabled(dhd_pub_t *dhdp, u32 event_mask)
{
	struct dhd_deferred_wq *work = dhdp->deferred_wq;

	if (!work || !event_mask || (event_mask & DHD_WQ_EVENTMASK) != event_mask) {
		DHD_ERROR(("%s: Invalid!!\n", __FUNCTION__));
		return FALSE;
	}

	if (work->active_mask & event_mask) {
		return TRUE;
	} else {
		return FALSE;
	}

}

void
dhd_deferred_work_disable(dhd_pub_t *dhdp, u32 event_mask)
{
	struct dhd_deferred_wq *work = dhdp->deferred_wq;

	if (!work || !event_mask || (event_mask & DHD_WQ_EVENTMASK) != event_mask) {
		DHD_ERROR(("%s: Invalid!!\n", __FUNCTION__));
		return;
	}

	work->active_mask &= ~event_mask;
}

void
dhd_cancel_deferred_wq(dhd_pub_t *dhdp)
{
	cancel_work_sync(&dhdp->deferred_wq->deferred_work);
}

uint
dhd_deferred_wq_scheduled(dhd_pub_t *dhdp)
{
	return work_busy(&dhdp->deferred_wq->deferred_work);
}

bool
dhd_wait_for_deferred_wq(dhd_pub_t *dhdp)
{
	if (dhdp->deferred_wq) {
		return flush_work(&dhdp->deferred_wq->deferred_work);
	}
	/* no valid wq for flushing. */
	return FALSE;
}
