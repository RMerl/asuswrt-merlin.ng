/** \file threadx.c
 *
 * Initialization and support routines for threadX.
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
 * $Id: threadx.c 597803 2015-11-06 08:46:00Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <rte_isr.h>
#include "rte_isr_priv.h"
#include <rte.h>
#include <rte_cons.h>
#include "rte_priv.h"
#include <rte_mem.h>
#include "rte_mem_priv.h"
#include <bcmstdlib_ext.h>

#include <tx_api.h>
#include <tx_low_power.h>
#include <tx_initialize.h>
#include <tx_thread.h>
#include <tx_timer.h>

#include "threadx_priv.h"
#include <threadx_low_power.h>
#include "threadx_low_power_priv.h"

#include <bcm_buzzz.h>

/* wlan timer queue data */
#ifndef THREADX_TIMER_QUEUE_SIZE
#define THREADX_TIMER_QUEUE_SIZE	TX_TIMER_ENTRIES
#endif 	/* THREADX_TIMER_QUEUE_SIZE */

/* expired s/w timer queue */
struct timer_queue
{
	int count;	/* enqueued count */
	int head;	/* enqueue to head */
	int tail;	/* dequeue from tail */
	hnd_timer_t *queue[THREADX_TIMER_QUEUE_SIZE];
};
typedef struct timer_queue timer_queue_t;

/* forward declarations */
static void threadx_timer_process_queue(timer_queue_t *timer_queue);

/* module stats */

/* module testing */

/* s/w timer context */
struct hnd_timer
{
	/* threadx timer */
	osl_ext_timer_t timer;
	/* thread context */
	osl_ext_task_t *thread;
	/* user supplied params */
	uint32 *context;
	void *data;
	hnd_timer_mainfn_t mainfn;
	hnd_timer_auxfn_t auxfn;
	uint msec;
	bool is_periodic;
	bool is_queued;
#ifdef THREADX_TEST
	/* debug/test */
	int count;
	osl_ext_time_ms_t expected_expiry;
	osl_ext_time_ms_t isr_timestamp;
	uint32 start_time;
	uint32 end_time;
#endif	/* THREADX_TEST */
};

/* wlan thread private data */
static osl_ext_event_t wlan_thread_event;
static timer_queue_t wlan_timer_queue;

/* thread handles */
static osl_ext_task_t wlan_thread;
static osl_ext_task_t idle_thread;

static uint wlan_thread_coreid2event(uint coreid);

#ifdef THREADX_STAT
static void print_threadx_stats(void *arg, int argc, char *argv[]);
#endif // endif

/* user defined TX_THREAD extension initial values */
typedef struct {
	TX_THREAD_USER_EXTENSION
} tx_thread_user_ext_t;
static tx_thread_user_ext_t
wlan_thread_user_ext = {
	&wlan_thread_event,
	&wlan_timer_queue,
	wlan_thread_coreid2event,
	0
};

/* thread stacks */
/* same stack size as hndrte */
static CHAR wlan_thread_stack[HND_STACK_SIZE] DECLSPEC_ALIGN(16);
#ifdef ATE_BUILD
static CHAR idle_thread_stack[16384] DECLSPEC_ALIGN(16);
#else
static CHAR idle_thread_stack[512] DECLSPEC_ALIGN(16);
#endif /* ATE_BUILD */

#ifdef THREADX_TEST
static bool test_start = FALSE;

#define NUM_TEST_TIMER	5
static hnd_timer_t *timer[NUM_TEST_TIMER];
static uint timeout[NUM_TEST_TIMER] = {1000, 1000, 1000, 25000, 2000};

static void
test_timer_cb(hnd_timer_t *t)
{
	/* nothing to do */
}

static void
test_timer_verify(hnd_timer_t *t)
{
	osl_ext_time_ms_t now = osl_ext_time_get();
	int i;

	for (i = 0; i < NUM_TEST_TIMER; i++) {
		if (t == timer[i])
			break;
	}
	if (i >= NUM_TEST_TIMER) {
		/* not a test timer */
		return;
	}

	if (t->expected_expiry <= now) {
		printf("timer %d, %d msec, count=%d, time=%d\n",
			i, t->msec, t->count, now);
	} else {
		printf("error timer %d, %d msec, count=%d, time=%d, "
			"expected=%d, isr=%d, "
			"start=%d end=%d elapsed=%d\n",
			i, t->msec, t->count, now,
			t->expected_expiry, t->isr_timestamp,
			t->start_time, t->end_time, t->end_time - t->start_time);
	}

}
#endif	/* THREADX_TEST */

/* invoked by tx_initilize_kernel_enter */
void _tx_initialize_low_level(void)
{
	/* nothing to do */
}

#ifdef BRCM_ENABLE_THREAD_PROFILE
static VOID log_thread_times(TX_THREAD *thread_to_exec);

VOID (*brcm_thread_switch_callback)(TX_THREAD *) = log_thread_times;

static TX_THREAD *current_thread = NULL;
static ULONG thread_sched_time = 0;

static void
log_thread_times(TX_THREAD* thread_to_exec)
{
	if (current_thread) {
		current_thread->tx_thread_exec_time +=
			(hnd_time() - thread_sched_time);
	}

	if (thread_to_exec) {
		thread_sched_time = hnd_time();
		current_thread = thread_to_exec;
	}
}

static void
print_thread_stats(void *arg, int argc, char *argv[])
{
	TX_THREAD *th = _tx_thread_created_ptr;

	if (th) {
		printf("Total threads count:    %ld\n\n", _tx_thread_created_count);
		do {
			printf("%s:\n", th->tx_thread_name);
			printf("Thread ptr:	%p\n", th);
			printf("Priority:   %u\n", th->tx_thread_priority);
			printf("Run count:  %lu\n", th->tx_thread_run_count);
			printf("Run time:   %lu\n", th->tx_thread_exec_time);
			printf("Stack size: %lu\n", th->tx_thread_stack_size);
			printf("Stack ptr:  %p\n", th->tx_thread_stack_ptr);
			printf("Stack start addr:	%p\n", th->tx_thread_stack_start);
			printf("Stack end addr:		%p\n", th->tx_thread_stack_end);
			printf("\n");

			th = th->tx_thread_created_next;
		} while (th != _tx_thread_created_ptr);
	}
}
#endif /* BRCM_ENABLE_THREAD_PROFILE */

/* return enqueued timer count */
static int
timer_queue_count(timer_queue_t *queue)
{
	return queue->count;
}

/* enqueue a timer */
static bool
timer_queue_enqueue(timer_queue_t *queue, hnd_timer_t *timer)
{
	bool rc = FALSE;

	if (queue->count < THREADX_TIMER_QUEUE_SIZE) {
		queue->queue[queue->head] = timer;
		queue->head = (queue->head + 1) % THREADX_TIMER_QUEUE_SIZE;
		queue->count++;
		rc = TRUE;
	}
	return rc;
}

/* dequeue a timer */
static hnd_timer_t *
timer_queue_dequeue(timer_queue_t *queue)
{
	hnd_timer_t *timer = NULL;

	if (queue->count > 0) {
		timer = queue->queue[queue->tail];
		queue->tail = (queue->tail + 1) % THREADX_TIMER_QUEUE_SIZE;
		queue->count--;
	}
	return timer;
}

/* delete an enqueued timer */
static bool
timer_queue_delete(timer_queue_t *queue, hnd_timer_t *timer)
{
	bool rc = FALSE;
	int i;
	int tail = queue->tail;

	for (i = 0; i < queue->count; i++) {
		hnd_timer_t **t = &queue->queue[(tail + i) % THREADX_TIMER_QUEUE_SIZE];
		if (*t == timer) {
			/* set to NULL to delete, a NULL timer dequeued is ignored */
			*t = NULL;
			rc = TRUE;
			break;
		}
	}
	return rc;
}

#ifdef THREADX_STAT
/* statistics */
typedef enum {
	THREADX_STAT_ERROR,			/* threadx error */
	THREADX_STAT_THREAD_ENTRY,		/* thread entry */
	THREADX_STAT_EVENT_FLAGS_SET,		/* event set/get */
	THREADX_STAT_EVENT_FLAGS_GET,
	THREADX_STAT_QUEUE_SEND,		/* queue send/receive/remove */
	THREADX_STAT_QUEUE_RECEIVE,
	THREADX_STAT_QUEUE_REMOVE,
	THREADX_STAT_MUTEX_GET,			/* mutex get/put */
	THREADX_STAT_MUTEX_PUT,
	THREADX_STAT_SEMAPHORE_GET,		/* semaphore get/put */
	THREADX_STAT_SEMAPHORE_PUT,
	THREADX_STAT_CPU_ISR,			/* cpu ISR */
	THREADX_STAT_CC_CORE_ISR,		/* chip common ISR/DPC */
	THREADX_STAT_CC_CORE_DPC,
	THREADX_STAT_D11_CORE_ISR,		/* d11 ISR/DPC */
	THREADX_STAT_D11_CORE_DPC,
	THREADX_STAT_HOST_BUS_ISR,		/* host bus ISR/DPC */
	THREADX_STAT_HOST_BUS_DPC,
	THREADX_STAT_TIMER_STARTED,		/* timer started */
	THREADX_STAT_TIMER_STOPPED,		/* timer stopped */
	THREADX_STAT_TIMER_EXPIRED,		/* timer expired */
	THREADX_STAT_IDLE,			/* idle thread */
	THREADX_STAT_ISR_MAX = 23,
	THREADX_STAT_DPC_MAX = 24,
	THREADX_STAT_LAST
} threadx_stat_t;

uint32 threadx_stat[THREADX_STAT_LAST];

/* increment specified stat */
#define THREADX_STAT_UPDATE(stat)	threadx_stat[stat]++
#else
#define THREADX_STAT_UPDATE(stat)
#endif	/* THREADX_STAT */

/* entering idle */
static void
threadx_idle_enter(void)
{
	BUZZZ_LVL1(THREADX_IDLE, 0);
	THREADX_STAT_UPDATE(THREADX_STAT_IDLE);

#ifdef THREADX_TEST
	test_start = TRUE;
#endif	/* THREADX_TEST */

	/* update PMU before sleeping */
	threadx_low_power_enter();
}

void
hnd_idle_loop(si_t *sih)
{
	while (TRUE) {
		threadx_idle_enter();
		hnd_poll(sih);
	}
}

/* wlan thread event flag bits - least significant bit has higher priority */
/* bus devices/drivers share the same event flag bit assuming one at a time */
#define WLAN_EVENT_D11_DPC		(1 << 0)	/* D11 */
#define WLAN_EVENT_PCIE_DPC		(1 << 1)	/* PCIe */
#define WLAN_EVENT_SDIO_DPC		(1 << 1)	/* SDIO */
#define WLAN_EVENT_USB_DPC		(1 << 1)	/* USB */
#define WLAN_EVENT_CC_DPC		(1 << 2)	/* PMU, UART, GPIO, etc */
#define WLAN_EVENT_PMU_DPC		(1 << 3)	/* PMU */

#define WLAN_EVENT_NUM_BITS		4		/* number of event bits used */

/* all core event bits */
#define WLAN_EVENT_CORE_BITS		((1 << WLAN_EVENT_NUM_BITS) - 1)

/* MSB used for threadx events */
#define WLAN_EVENT_TIMER		(1 << 24)

/* convert coreid to event */
static uint32
wlan_thread_coreid2event(uint coreid)
{
	ULONG event = 0;

	switch (coreid)
	{
	case D11_CORE_ID:
		event = WLAN_EVENT_D11_DPC;
		break;
	case PCIE2_CORE_ID:
		event = WLAN_EVENT_PCIE_DPC;
		break;
	case SDIOD_CORE_ID:
		event = WLAN_EVENT_SDIO_DPC;
		break;
	case USB30D_CORE_ID:
	case USB20D_CORE_ID:
		event = WLAN_EVENT_USB_DPC;
		break;
	case CC_CORE_ID:
		event = WLAN_EVENT_CC_DPC;
		break;
	case PMU_CORE_ID:
		event = WLAN_EVENT_PMU_DPC;
		break;
	default:
		/* Unknown to this function */
		OSL_SYS_HALT();
		break;
	}

	return event;
}

/* notifying DPC based on action isr */
static void
notify_dpc(hnd_isr_action_t *action)
{
	ULONG event;
	osl_ext_task_t *ctx;

	event = action->event;
	if (event == 0)
		return;

	ctx = action->thread;
	if (ctx == NULL)
		return;

	/* post event and update stat */
	if (osl_ext_event_set(ctx->event, event) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
	}
	THREADX_STAT_UPDATE(THREADX_STAT_EVENT_FLAGS_SET);

#ifdef THREADX_STAT
	{
	threadx_stat_t stat;

	if (event == WLAN_EVENT_D11_DPC)
		stat = THREADX_STAT_D11_CORE_ISR;
	else if (event == WLAN_EVENT_CC_DPC)
		stat = THREADX_STAT_CC_CORE_ISR;
	else
		stat = THREADX_STAT_HOST_BUS_ISR;

	THREADX_STAT_UPDATE(stat);
	}
#endif	/* THREADX_STAT */
}

/* run wlan thread DPCs */
static void
wlan_thread_dpc(ULONG event)
{
#ifdef THREADX_STAT
	uint32 cycles;
	uint32 delta;

	OSL_GETCYCLES(cycles);
#endif // endif

	BUZZZ_LVL1(HND_DPC_ENT, 1, event);
	if (!hnd_dpc_proc_event(hnd_dpc_get_inst(), event)) {
		BUZZZ_LVL1(HND_DPC_RTN_ERR, 0);
		return;
	}
	BUZZZ_LVL1(HND_DPC_RTN, 0);

#ifdef THREADX_STAT
	OSL_GETCYCLES(delta);
	delta -= cycles;
	if (delta > threadx_stat[THREADX_STAT_DPC_MAX]) {
		threadx_stat[THREADX_STAT_DPC_MAX] = delta;
	}

	{
	threadx_stat_t stat;

	if (event == WLAN_EVENT_D11_DPC)
		stat = THREADX_STAT_D11_CORE_DPC;
	else if (event == WLAN_EVENT_CC_DPC)
		stat = THREADX_STAT_CC_CORE_DPC;
	else
		stat = THREADX_STAT_HOST_BUS_DPC;

	THREADX_STAT_UPDATE(stat);
	}
#endif	/* THREADX_STAT */
}

/* interrupt handler */
void
threadx_isr(threadx_trap_t *tr)
{
	uint32 sbflagst;
#ifdef THREADX_STAT
	uint32 cycles;
	uint32 delta;

	OSL_GETCYCLES(cycles);
#endif // endif

	BUZZZ_LVL5(THREADX_CPU_ISR_ENT, 1, (uint32)tr->lr);

#ifdef HND_PRINTF_THREAD_SAFE
	in_isr_handler ++;
#endif	/* HND_PRINTF_THREAD_SAFE */

	/* increment count */
	THREADX_STAT_UPDATE(THREADX_STAT_CPU_ISR);

	/* read interrupt flag */
	sbflagst = si_intflag(hnd_sih);

	/* exit to update timers with elapsed time */
	threadx_low_power_exit();

	/* run isr */
	hnd_isr_proc_sbflagst(hnd_isr_get_inst(),
		sbflagst, NULL, notify_dpc);

#ifdef HND_PRINTF_THREAD_SAFE
	in_isr_handler --;
#endif	/* HND_PRINTF_THREAD_SAFE */

#ifdef THREADX_STAT
	OSL_GETCYCLES(delta);
	delta -= cycles;
	if (delta > threadx_stat[THREADX_STAT_ISR_MAX]) {
		threadx_stat[THREADX_STAT_ISR_MAX] = delta;
	}
#endif // endif

	BUZZZ_LVL5(THREADX_CPU_ISR_RTN, 0);
}

void
threadx_fiq_isr(void)
{
	/* Note: we should go back for fiq handlers registration */
	hnd_cpu_gtimer_fiq_hdl();
}

/* convert coreid to event */
uint
hnd_isr_get_dpc_event(osl_ext_task_t *ctx, uint32 coreid)
{
	if (ctx != NULL &&
	    ctx->coreid2event != NULL)
		return (ctx->coreid2event)(coreid);
	return 0;
}

/* wlan idle thread */
static void
idle_thread_entry(osl_ext_task_arg_t arg)
{
	THREADX_STAT_UPDATE(THREADX_STAT_THREAD_ENTRY);

	hnd_idle_loop(hnd_sih);
}

/* wlan thread */
si_t *_c_main(void);

static void
wlan_thread_entry(osl_ext_task_arg_t arg)
{
	tx_thread_user_ext_t *ext = (tx_thread_user_ext_t *)arg;
	osl_ext_task_t *ctx = osl_ext_task_current();
	si_t *sih;

	THREADX_STAT_UPDATE(THREADX_STAT_THREAD_ENTRY);

	/* save user defined TX_THREAD extension values */
	*(tx_thread_user_ext_t *)&ctx->event = *ext;

	/* invoke c_main for initialization */
	sih = _c_main();

	/* initialize idle loop */
	hnd_idle_init(sih);

	/* create idle thread at lowest priority */
	if (osl_ext_task_create("idle_thread",
		idle_thread_stack, sizeof(idle_thread_stack),
		OSL_EXT_TASK_IDLE_PRIORITY, idle_thread_entry,
		0, &idle_thread) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
		return;
	}

	printf("ThreadX v%d.%d initialized\n",
		__THREADX_MAJOR_VERSION, __THREADX_MINOR_VERSION);

	/* wait to process DPCs */
	while (TRUE) {
		osl_ext_event_bits_t event_bits;

		/* waits forever waiting for an event */
		if (osl_ext_event_get(ctx->event,
			WLAN_EVENT_CORE_BITS | WLAN_EVENT_TIMER,
			OSL_EXT_TIME_FOREVER, &event_bits) != OSL_EXT_SUCCESS) {
			THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
			continue;
		}
		THREADX_STAT_UPDATE(THREADX_STAT_EVENT_FLAGS_GET);

#ifdef THREADX_TEST
		{
		static bool is_test_started = FALSE;

		/* start test after initialization has completed */
		if (!is_test_started && test_start) {
			int i;
			for (i = 0; i < NUM_TEST_TIMER; i++) {
				timer[i] = hnd_timer_create(0, 0, test_timer_cb, 0);
				hnd_timer_start(timer[i], timeout[i], TRUE);
			}
			is_test_started = TRUE;
		}
		}
#endif	/* THREADX_TEST */

		/* core event */
		if (event_bits & WLAN_EVENT_CORE_BITS) {
			int i;
			for (i = 0; i < WLAN_EVENT_NUM_BITS; i++) {
				ULONG event;
				event = event_bits & (1 << i);
				if (event != 0) {
					wlan_thread_dpc(event);
				}
			}
		}

		/* timer event */
		threadx_timer_process_queue(ctx->timerq);
	}
}

/* threadx application initialization */
void
tx_application_define(void *first_unused_memory)
{
	/* create event flags */
	if (osl_ext_event_create("wlan_thread_event", &wlan_thread_event)
		!= OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
		return;
	}

	/* create wlan thread */
	if (osl_ext_task_create("wlan_thread",
		wlan_thread_stack, sizeof(wlan_thread_stack),
		OSL_EXT_TASK_NORMAL_PRIORITY,
		wlan_thread_entry, (osl_ext_task_arg_t)&wlan_thread_user_ext,
		&wlan_thread) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
		return;
	}

	/* enable task stack checking */
	if (osl_ext_task_enable_stack_check() != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
		return;
	}
}

/* timer callback occurs in interrupt context */
static void
threadx_timer_callback(osl_ext_timer_arg_t arg)
{
	hnd_timer_t *t = (hnd_timer_t *)arg;
	osl_ext_task_t *ctx = t->thread;

#ifdef THREADX_TEST
	t->isr_timestamp = osl_ext_time_get();
	t->end_time = hnd_time();
#endif	/* THREADX_TEST */

	/* queue timer data */
	if (!timer_queue_enqueue(ctx->timerq, t))
		return;

	THREADX_STAT_UPDATE(THREADX_STAT_QUEUE_SEND);

	/* mark timer as queued */
	t->is_queued = TRUE;

	/* notify timer event */
	if (osl_ext_event_set(ctx->event, WLAN_EVENT_TIMER) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
		return;
	}
	THREADX_STAT_UPDATE(THREADX_STAT_EVENT_FLAGS_SET);
}

/* schedule zero length timer */
static void
schedule_zero_timer(hnd_timer_t *t)
{
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
	/* invoke callback to post timer event and timer callback
	 * will occur when timer event is processed
	 */
	threadx_timer_callback((osl_ext_timer_arg_t)t);
	osl_ext_interrupt_restore(state);
}

/* process timer queue */
static void
threadx_timer_process_queue(timer_queue_t *timer_queue)
{
	int enqueued;
	int i;

	/* get number of queued timers */
	enqueued = timer_queue_count(timer_queue);

	/* invoke callback for queued timers */
	/* only process currently enqueued timers as timer callback
	 * may queue more timers resulting in an infinite loop
	 */

	for (i = 0; i < enqueued; i++) {
		hnd_timer_t *t;

		/* dequeue a timer */
		{
		/* critical section */
		osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
		t = timer_queue_dequeue(timer_queue);
		/* critical section */
		osl_ext_interrupt_restore(state);
		}
		THREADX_STAT_UPDATE(THREADX_STAT_QUEUE_RECEIVE);

		if (t == NULL)
			continue;

		/* mark timer as not queued */
		t->is_queued = FALSE;

#ifdef THREADX_TEST
		test_timer_verify(t);
#endif	/* THREADX_TEST */

		/* restart timer if periodic */
		if (t->is_periodic) {
			hnd_timer_start(t, t->msec, TRUE);
		}

		/* invoke timer callback */
		if (t->mainfn != NULL) {
			THREADX_STAT_UPDATE(THREADX_STAT_TIMER_EXPIRED);

			BUZZZ_LVL1(HND_TMR_ENT, 1, (uint32)t->mainfn);
			(*t->mainfn)(t);
			BUZZZ_LVL1(HND_TMR_RTN, 0);
		}
	}
}

#ifdef RTE_DBG_TIMER
static void
threadx_print_timers(void *arg, int argc, char *argv[])
{
	TX_TIMER *this = _tx_timer_created_ptr;
	ULONG i = 0;

	if (this == NULL) {
		printf("No timers\n");
		return;
	}

	while (i < _tx_timer_created_count) {
		/* assuming TX_TIMER is the first member of the struct */
		hnd_timer_t *t = (hnd_timer_t *)this;
		ULONG remaining_ticks;
		ULONG reschedule_ticks;

		tx_timer_info_get(this, NULL, NULL, &remaining_ticks,
		                  &reschedule_ticks, &this);

		printf("timer %p left %lu resched %lu "
		       "(main %p, aux %p, ctx %p, data %p, %d ms, prd %d, exp %d)\n",
		       t, remaining_ticks, reschedule_ticks,
		       t->mainfn, t->auxfn, t->context, t->data, t->msec,
		       t->is_periodic, t->is_queued);

#ifdef THREADX_TEST
		printf("count %d exp %u isr ts %u start %u end %u\n",
		       t->count, t->expected_expiry, t->isr_timestamp,
		       t->start_time, t->end_time);
#endif	/* THREADX_TEST */

		i ++;
	}
}
#endif /* RTE_DBG_TIMER */

hnd_timer_t *
hnd_timer_create(void *context, void *data,
	hnd_timer_mainfn_t mainfn, hnd_timer_auxfn_t auxfn)
{
	hnd_timer_t *t;

	BUZZZ_LVL5(HND_TMR_CRT, 1, (uint32)__builtin_return_address(0));

	t = (hnd_timer_t *)MALLOCZ(hnd_osh, sizeof(hnd_timer_t));
	if (t != NULL) {
		/* current running task */
		t->thread = osl_ext_task_current();
		t->context = context;
		t->data = data;
		t->mainfn = mainfn;
		t->auxfn = auxfn;

		if (osl_ext_timer_create("timer",
			OSL_EXT_TIME_FOREVER, OSL_EXT_TIMER_MODE_ONCE,
			threadx_timer_callback, t, &t->timer) != OSL_EXT_SUCCESS) {
			THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
			MFREE(hnd_osh, t, sizeof(hnd_timer_t));
			t = NULL;
		}
	}
	return t;
}

/* dequeue timer if queued */
static void
dequeue_timer(hnd_timer_t *t)
{
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
	ASSERT(t != NULL);

	if (t->is_queued) {
		osl_ext_task_t *ctx = t->thread;

		if (timer_queue_delete(ctx->timerq, t)) {
			THREADX_STAT_UPDATE(THREADX_STAT_QUEUE_REMOVE);
		}
		t->is_queued = FALSE;
	}

	osl_ext_interrupt_restore(state);
}

void
hnd_timer_free(hnd_timer_t *t)
{
	BUZZZ_LVL5(HND_TMR_DEL, 1, (uint32)__builtin_return_address(0));

	if (t == NULL)
		return;

	/* dequeue timer if queued */
	dequeue_timer(t);

	if (osl_ext_timer_delete(&t->timer) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
	}
	MFREE(hnd_osh, t, sizeof(hnd_timer_t));
}

bool
hnd_timer_start(hnd_timer_t *t, uint ms, bool periodic)
{
	BUZZZ_LVL5(HND_TMR_BGN, 1, (uint32)__builtin_return_address(0));

	if (t == NULL)
		return FALSE;

	t->msec = ms;
	t->is_periodic = periodic ? TRUE : FALSE;
	t->is_queued = FALSE;

	if (ms == 0) {
		/* dequeue timer if queued */
		dequeue_timer(t);

		/* schedule zero length timer */
		schedule_zero_timer(t);
	} else {
		/* timer may be active */
		hnd_timer_stop(t);

		/* periodic is not used and timer is restarted
		 * after timer callback to ensure periodic timers are started
		 * after timer callback and to avoid mutiple timer expiries
		 * being queued
		 */
		if (osl_ext_timer_start(&t->timer, ms,
			OSL_EXT_TIMER_MODE_ONCE) != OSL_EXT_SUCCESS) {
			THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
			return FALSE;
		}
	}

#ifdef THREADX_TEST
	t->count++;
	t->expected_expiry = osl_ext_time_get() + OSL_MSEC_TO_TICKS(ms);
	t->start_time = hnd_time();
#endif	/* THREADX_TEST */

	THREADX_STAT_UPDATE(THREADX_STAT_TIMER_STARTED);
	return TRUE;
}

bool
hnd_timer_stop(hnd_timer_t *t)
{
	BUZZZ_LVL5(HND_TMR_END, 1, (uint32)__builtin_return_address(0));

	if (t == NULL)
		return FALSE;

	if (osl_ext_timer_stop(&t->timer) != OSL_EXT_SUCCESS) {
		THREADX_STAT_UPDATE(THREADX_STAT_ERROR);
	}

	/* dequeue timer if queued */
	dequeue_timer(t);

	THREADX_STAT_UPDATE(THREADX_STAT_TIMER_STOPPED);
	return TRUE;
}

int
BCMATTACHFN(hnd_timer_init)(si_t *sih)
{
	/* initialize ticks */
	hnd_update_now();

	/* initialize low power mode */
	threadx_low_power_init();

	return BCME_OK;
}

/* Must be called after hnd_cons_init() */
void
BCMATTACHFN(hnd_timer_cli_init)(void)
{
#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
#ifdef RTE_DBG_TIMER
	hnd_cons_add_cmd("tim", threadx_print_timers, 0);
#endif // endif
#ifdef BRCM_ENABLE_THREAD_PROFILE
	hnd_cons_add_cmd("ths", print_thread_stats, 0);
#endif // endif
#ifdef THREADX_STAT
	hnd_cons_add_cmd("txs", print_threadx_stats, 0);
#endif // endif
#endif /* RTE_CONS  && ! BCM_BOOTLOADER */
}

void *
hnd_timer_get_ctx(hnd_timer_t *t)
{
	return t->context;
}

void *
hnd_timer_get_data(hnd_timer_t *t)
{
	return t->data;
}

hnd_timer_auxfn_t
hnd_timer_get_auxfn(hnd_timer_t *t)
{
	return t->auxfn;
}

/** Cancel the h/w timer if it is already armed and ignore any further h/w timer requests */
void
hnd_suspend_timer(void)
{
	hnd_ack_irq_timer();
}

/** Resume the timer activities */
void
hnd_resume_timer(void)
{
	hnd_set_irq_timer(0);
}

void
hnd_run_timeouts(void)
{
}

#ifdef THREADX_STAT
static void
print_threadx_stats(void *arg, int argc, char *argv[])
{
	int i;

	for (i = 0; i < THREADX_STAT_LAST; i ++)
		printf("%d: %u\n", i, threadx_stat[i]);
}
#endif /* THREADX_STAT */
