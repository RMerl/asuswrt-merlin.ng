/*
 * Read-Copy Update mechanism for mutual exclusion
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Copyright IBM Corporation, 2001
 *
 * Authors: Dipankar Sarma <dipankar@in.ibm.com>
 *	    Manfred Spraul <manfred@colorfullife.com>
 *
 * Based on the original work by Paul McKenney <paulmck@us.ibm.com>
 * and inputs from Rusty Russell, Andrea Arcangeli and Andi Kleen.
 * Papers:
 * http://www.rdrop.com/users/paulmck/paper/rclockpdcsproof.pdf
 * http://lse.sourceforge.net/locking/rclock_OLS.2001.05.01c.sc.pdf (OLS2001)
 *
 * For detailed explanation of Read-Copy Update mechanism see -
 *		http://lse.sourceforge.net/locking/rcupdate.html
 *
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/export.h>
#include <linux/hardirq.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/tick.h>

#define CREATE_TRACE_POINTS

#include "rcu.h"

MODULE_ALIAS("rcupdate");
#ifdef MODULE_PARAM_PREFIX
#undef MODULE_PARAM_PREFIX
#endif
#define MODULE_PARAM_PREFIX "rcupdate."

module_param(rcu_expedited, int, 0);

#ifndef CONFIG_TINY_RCU

static atomic_t rcu_expedited_nesting =
	ATOMIC_INIT(IS_ENABLED(CONFIG_RCU_EXPEDITE_BOOT) ? 1 : 0);

/*
 * Should normal grace-period primitives be expedited?  Intended for
 * use within RCU.  Note that this function takes the rcu_expedited
 * sysfs/boot variable into account as well as the rcu_expedite_gp()
 * nesting.  So looping on rcu_unexpedite_gp() until rcu_gp_is_expedited()
 * returns false is a -really- bad idea.
 */
bool rcu_gp_is_expedited(void)
{
	return rcu_expedited || atomic_read(&rcu_expedited_nesting);
}
EXPORT_SYMBOL_GPL(rcu_gp_is_expedited);

/**
 * rcu_expedite_gp - Expedite future RCU grace periods
 *
 * After a call to this function, future calls to synchronize_rcu() and
 * friends act as the corresponding synchronize_rcu_expedited() function
 * had instead been called.
 */
void rcu_expedite_gp(void)
{
	atomic_inc(&rcu_expedited_nesting);
}
EXPORT_SYMBOL_GPL(rcu_expedite_gp);

/**
 * rcu_unexpedite_gp - Cancel prior rcu_expedite_gp() invocation
 *
 * Undo a prior call to rcu_expedite_gp().  If all prior calls to
 * rcu_expedite_gp() are undone by a subsequent call to rcu_unexpedite_gp(),
 * and if the rcu_expedited sysfs/boot parameter is not set, then all
 * subsequent calls to synchronize_rcu() and friends will return to
 * their normal non-expedited behavior.
 */
void rcu_unexpedite_gp(void)
{
	atomic_dec(&rcu_expedited_nesting);
}
EXPORT_SYMBOL_GPL(rcu_unexpedite_gp);

#endif /* #ifndef CONFIG_TINY_RCU */

/*
 * Inform RCU of the end of the in-kernel boot sequence.
 */
void rcu_end_inkernel_boot(void)
{
	if (IS_ENABLED(CONFIG_RCU_EXPEDITE_BOOT))
		rcu_unexpedite_gp();
}

#ifdef CONFIG_PREEMPT_RCU

/*
 * Preemptible RCU implementation for rcu_read_lock().
 * Just increment ->rcu_read_lock_nesting, shared state will be updated
 * if we block.
 */
void __rcu_read_lock(void)
{
	current->rcu_read_lock_nesting++;
	barrier();  /* critical section after entry code. */
}
EXPORT_SYMBOL_GPL(__rcu_read_lock);

/*
 * Preemptible RCU implementation for rcu_read_unlock().
 * Decrement ->rcu_read_lock_nesting.  If the result is zero (outermost
 * rcu_read_unlock()) and ->rcu_read_unlock_special is non-zero, then
 * invoke rcu_read_unlock_special() to clean up after a context switch
 * in an RCU read-side critical section and other special cases.
 */
void __rcu_read_unlock(void)
{
	struct task_struct *t = current;

	if (t->rcu_read_lock_nesting != 1) {
		--t->rcu_read_lock_nesting;
	} else {
		barrier();  /* critical section before exit code. */
		t->rcu_read_lock_nesting = INT_MIN;
		barrier();  /* assign before ->rcu_read_unlock_special load */
		if (unlikely(ACCESS_ONCE(t->rcu_read_unlock_special.s)))
			rcu_read_unlock_special(t);
		barrier();  /* ->rcu_read_unlock_special load before assign */
		t->rcu_read_lock_nesting = 0;
	}
#ifdef CONFIG_PROVE_LOCKING
	{
		int rrln = ACCESS_ONCE(t->rcu_read_lock_nesting);

		WARN_ON_ONCE(rrln < 0 && rrln > INT_MIN / 2);
	}
#endif /* #ifdef CONFIG_PROVE_LOCKING */
}
EXPORT_SYMBOL_GPL(__rcu_read_unlock);

#endif /* #ifdef CONFIG_PREEMPT_RCU */

#ifdef CONFIG_DEBUG_LOCK_ALLOC
static struct lock_class_key rcu_lock_key;
struct lockdep_map rcu_lock_map =
	STATIC_LOCKDEP_MAP_INIT("rcu_read_lock", &rcu_lock_key);
EXPORT_SYMBOL_GPL(rcu_lock_map);

static struct lock_class_key rcu_bh_lock_key;
struct lockdep_map rcu_bh_lock_map =
	STATIC_LOCKDEP_MAP_INIT("rcu_read_lock_bh", &rcu_bh_lock_key);
EXPORT_SYMBOL_GPL(rcu_bh_lock_map);

static struct lock_class_key rcu_sched_lock_key;
struct lockdep_map rcu_sched_lock_map =
	STATIC_LOCKDEP_MAP_INIT("rcu_read_lock_sched", &rcu_sched_lock_key);
EXPORT_SYMBOL_GPL(rcu_sched_lock_map);

static struct lock_class_key rcu_callback_key;
struct lockdep_map rcu_callback_map =
	STATIC_LOCKDEP_MAP_INIT("rcu_callback", &rcu_callback_key);
EXPORT_SYMBOL_GPL(rcu_callback_map);

int notrace debug_lockdep_rcu_enabled(void)
{
	return rcu_scheduler_active && debug_locks &&
	       current->lockdep_recursion == 0;
}
EXPORT_SYMBOL_GPL(debug_lockdep_rcu_enabled);

/**
 * rcu_read_lock_held() - might we be in RCU read-side critical section?
 *
 * If CONFIG_DEBUG_LOCK_ALLOC is selected, returns nonzero iff in an RCU
 * read-side critical section.  In absence of CONFIG_DEBUG_LOCK_ALLOC,
 * this assumes we are in an RCU read-side critical section unless it can
 * prove otherwise.  This is useful for debug checks in functions that
 * require that they be called within an RCU read-side critical section.
 *
 * Checks debug_lockdep_rcu_enabled() to prevent false positives during boot
 * and while lockdep is disabled.
 *
 * Note that rcu_read_lock() and the matching rcu_read_unlock() must
 * occur in the same context, for example, it is illegal to invoke
 * rcu_read_unlock() in process context if the matching rcu_read_lock()
 * was invoked from within an irq handler.
 *
 * Note that rcu_read_lock() is disallowed if the CPU is either idle or
 * offline from an RCU perspective, so check for those as well.
 */
int rcu_read_lock_held(void)
{
	if (!debug_lockdep_rcu_enabled())
		return 1;
	if (!rcu_is_watching())
		return 0;
	if (!rcu_lockdep_current_cpu_online())
		return 0;
	return lock_is_held(&rcu_lock_map);
}
EXPORT_SYMBOL_GPL(rcu_read_lock_held);

/**
 * rcu_read_lock_bh_held() - might we be in RCU-bh read-side critical section?
 *
 * Check for bottom half being disabled, which covers both the
 * CONFIG_PROVE_RCU and not cases.  Note that if someone uses
 * rcu_read_lock_bh(), but then later enables BH, lockdep (if enabled)
 * will show the situation.  This is useful for debug checks in functions
 * that require that they be called within an RCU read-side critical
 * section.
 *
 * Check debug_lockdep_rcu_enabled() to prevent false positives during boot.
 *
 * Note that rcu_read_lock() is disallowed if the CPU is either idle or
 * offline from an RCU perspective, so check for those as well.
 */
int rcu_read_lock_bh_held(void)
{
	if (!debug_lockdep_rcu_enabled())
		return 1;
	if (!rcu_is_watching())
		return 0;
	if (!rcu_lockdep_current_cpu_online())
		return 0;
	return in_softirq() || irqs_disabled();
}
EXPORT_SYMBOL_GPL(rcu_read_lock_bh_held);

#endif /* #ifdef CONFIG_DEBUG_LOCK_ALLOC */

/**
 * wakeme_after_rcu() - Callback function to awaken a task after grace period
 * @head: Pointer to rcu_head member within rcu_synchronize structure
 *
 * Awaken the corresponding task now that a grace period has elapsed.
 */
void wakeme_after_rcu(struct rcu_head *head)
{
	struct rcu_synchronize *rcu;

	rcu = container_of(head, struct rcu_synchronize, head);
	complete(&rcu->completion);
}

void wait_rcu_gp(call_rcu_func_t crf)
{
	struct rcu_synchronize rcu;

	init_rcu_head_on_stack(&rcu.head);
	init_completion(&rcu.completion);
	/* Will wake me after RCU finished. */
	crf(&rcu.head, wakeme_after_rcu);
	/* Wait for it. */
	wait_for_completion(&rcu.completion);
	destroy_rcu_head_on_stack(&rcu.head);
}
EXPORT_SYMBOL_GPL(wait_rcu_gp);

#ifdef CONFIG_DEBUG_OBJECTS_RCU_HEAD
void init_rcu_head(struct rcu_head *head)
{
	debug_object_init(head, &rcuhead_debug_descr);
}

void destroy_rcu_head(struct rcu_head *head)
{
	debug_object_free(head, &rcuhead_debug_descr);
}

/*
 * fixup_activate is called when:
 * - an active object is activated
 * - an unknown object is activated (might be a statically initialized object)
 * Activation is performed internally by call_rcu().
 */
static int rcuhead_fixup_activate(void *addr, enum debug_obj_state state)
{
	struct rcu_head *head = addr;

	switch (state) {

	case ODEBUG_STATE_NOTAVAILABLE:
		/*
		 * This is not really a fixup. We just make sure that it is
		 * tracked in the object tracker.
		 */
		debug_object_init(head, &rcuhead_debug_descr);
		debug_object_activate(head, &rcuhead_debug_descr);
		return 0;
	default:
		return 1;
	}
}

/**
 * init_rcu_head_on_stack() - initialize on-stack rcu_head for debugobjects
 * @head: pointer to rcu_head structure to be initialized
 *
 * This function informs debugobjects of a new rcu_head structure that
 * has been allocated as an auto variable on the stack.  This function
 * is not required for rcu_head structures that are statically defined or
 * that are dynamically allocated on the heap.  This function has no
 * effect for !CONFIG_DEBUG_OBJECTS_RCU_HEAD kernel builds.
 */
void init_rcu_head_on_stack(struct rcu_head *head)
{
	debug_object_init_on_stack(head, &rcuhead_debug_descr);
}
EXPORT_SYMBOL_GPL(init_rcu_head_on_stack);

/**
 * destroy_rcu_head_on_stack() - destroy on-stack rcu_head for debugobjects
 * @head: pointer to rcu_head structure to be initialized
 *
 * This function informs debugobjects that an on-stack rcu_head structure
 * is about to go out of scope.  As with init_rcu_head_on_stack(), this
 * function is not required for rcu_head structures that are statically
 * defined or that are dynamically allocated on the heap.  Also as with
 * init_rcu_head_on_stack(), this function has no effect for
 * !CONFIG_DEBUG_OBJECTS_RCU_HEAD kernel builds.
 */
void destroy_rcu_head_on_stack(struct rcu_head *head)
{
	debug_object_free(head, &rcuhead_debug_descr);
}
EXPORT_SYMBOL_GPL(destroy_rcu_head_on_stack);

struct debug_obj_descr rcuhead_debug_descr = {
	.name = "rcu_head",
	.fixup_activate = rcuhead_fixup_activate,
};
EXPORT_SYMBOL_GPL(rcuhead_debug_descr);
#endif /* #ifdef CONFIG_DEBUG_OBJECTS_RCU_HEAD */

#if defined(CONFIG_TREE_RCU) || defined(CONFIG_PREEMPT_RCU) || defined(CONFIG_RCU_TRACE)
void do_trace_rcu_torture_read(const char *rcutorturename, struct rcu_head *rhp,
			       unsigned long secs,
			       unsigned long c_old, unsigned long c)
{
	trace_rcu_torture_read(rcutorturename, rhp, secs, c_old, c);
}
EXPORT_SYMBOL_GPL(do_trace_rcu_torture_read);
#else
#define do_trace_rcu_torture_read(rcutorturename, rhp, secs, c_old, c) \
	do { } while (0)
#endif

#ifdef CONFIG_RCU_STALL_COMMON

#ifdef CONFIG_PROVE_RCU
#define RCU_STALL_DELAY_DELTA	       (5 * HZ)
#else
#define RCU_STALL_DELAY_DELTA	       0
#endif

int rcu_cpu_stall_suppress __read_mostly; /* 1 = suppress stall warnings. */
static int rcu_cpu_stall_timeout __read_mostly = CONFIG_RCU_CPU_STALL_TIMEOUT;

module_param(rcu_cpu_stall_suppress, int, 0644);
module_param(rcu_cpu_stall_timeout, int, 0644);

int rcu_jiffies_till_stall_check(void)
{
	int till_stall_check = ACCESS_ONCE(rcu_cpu_stall_timeout);

	/*
	 * Limit check must be consistent with the Kconfig limits
	 * for CONFIG_RCU_CPU_STALL_TIMEOUT.
	 */
	if (till_stall_check < 3) {
		ACCESS_ONCE(rcu_cpu_stall_timeout) = 3;
		till_stall_check = 3;
	} else if (till_stall_check > 300) {
		ACCESS_ONCE(rcu_cpu_stall_timeout) = 300;
		till_stall_check = 300;
	}
	return till_stall_check * HZ + RCU_STALL_DELAY_DELTA;
}

void rcu_sysrq_start(void)
{
	if (!rcu_cpu_stall_suppress)
		rcu_cpu_stall_suppress = 2;
}

void rcu_sysrq_end(void)
{
	if (rcu_cpu_stall_suppress == 2)
		rcu_cpu_stall_suppress = 0;
}

static int rcu_panic(struct notifier_block *this, unsigned long ev, void *ptr)
{
	rcu_cpu_stall_suppress = 1;
	return NOTIFY_DONE;
}

static struct notifier_block rcu_panic_block = {
	.notifier_call = rcu_panic,
};

static int __init check_cpu_stall_init(void)
{
	atomic_notifier_chain_register(&panic_notifier_list, &rcu_panic_block);
	return 0;
}
early_initcall(check_cpu_stall_init);

#endif /* #ifdef CONFIG_RCU_STALL_COMMON */

#ifdef CONFIG_TASKS_RCU

/*
 * Simple variant of RCU whose quiescent states are voluntary context switch,
 * user-space execution, and idle.  As such, grace periods can take one good
 * long time.  There are no read-side primitives similar to rcu_read_lock()
 * and rcu_read_unlock() because this implementation is intended to get
 * the system into a safe state for some of the manipulations involved in
 * tracing and the like.  Finally, this implementation does not support
 * high call_rcu_tasks() rates from multiple CPUs.  If this is required,
 * per-CPU callback lists will be needed.
 */

/* Global list of callbacks and associated lock. */
static struct rcu_head *rcu_tasks_cbs_head;
static struct rcu_head **rcu_tasks_cbs_tail = &rcu_tasks_cbs_head;
static DECLARE_WAIT_QUEUE_HEAD(rcu_tasks_cbs_wq);
static DEFINE_RAW_SPINLOCK(rcu_tasks_cbs_lock);

/* Track exiting tasks in order to allow them to be waited for. */
DEFINE_SRCU(tasks_rcu_exit_srcu);

/* Control stall timeouts.  Disable with <= 0, otherwise jiffies till stall. */
static int rcu_task_stall_timeout __read_mostly = HZ * 60 * 10;
module_param(rcu_task_stall_timeout, int, 0644);

static void rcu_spawn_tasks_kthread(void);

/*
 * Post an RCU-tasks callback.  First call must be from process context
 * after the scheduler if fully operational.
 */
void call_rcu_tasks(struct rcu_head *rhp, void (*func)(struct rcu_head *rhp))
{
	unsigned long flags;
	bool needwake;

	rhp->next = NULL;
	rhp->func = func;
	raw_spin_lock_irqsave(&rcu_tasks_cbs_lock, flags);
	needwake = !rcu_tasks_cbs_head;
	*rcu_tasks_cbs_tail = rhp;
	rcu_tasks_cbs_tail = &rhp->next;
	raw_spin_unlock_irqrestore(&rcu_tasks_cbs_lock, flags);
	if (needwake) {
		rcu_spawn_tasks_kthread();
		wake_up(&rcu_tasks_cbs_wq);
	}
}
EXPORT_SYMBOL_GPL(call_rcu_tasks);

/**
 * synchronize_rcu_tasks - wait until an rcu-tasks grace period has elapsed.
 *
 * Control will return to the caller some time after a full rcu-tasks
 * grace period has elapsed, in other words after all currently
 * executing rcu-tasks read-side critical sections have elapsed.  These
 * read-side critical sections are delimited by calls to schedule(),
 * cond_resched_rcu_qs(), idle execution, userspace execution, calls
 * to synchronize_rcu_tasks(), and (in theory, anyway) cond_resched().
 *
 * This is a very specialized primitive, intended only for a few uses in
 * tracing and other situations requiring manipulation of function
 * preambles and profiling hooks.  The synchronize_rcu_tasks() function
 * is not (yet) intended for heavy use from multiple CPUs.
 *
 * Note that this guarantee implies further memory-ordering guarantees.
 * On systems with more than one CPU, when synchronize_rcu_tasks() returns,
 * each CPU is guaranteed to have executed a full memory barrier since the
 * end of its last RCU-tasks read-side critical section whose beginning
 * preceded the call to synchronize_rcu_tasks().  In addition, each CPU
 * having an RCU-tasks read-side critical section that extends beyond
 * the return from synchronize_rcu_tasks() is guaranteed to have executed
 * a full memory barrier after the beginning of synchronize_rcu_tasks()
 * and before the beginning of that RCU-tasks read-side critical section.
 * Note that these guarantees include CPUs that are offline, idle, or
 * executing in user mode, as well as CPUs that are executing in the kernel.
 *
 * Furthermore, if CPU A invoked synchronize_rcu_tasks(), which returned
 * to its caller on CPU B, then both CPU A and CPU B are guaranteed
 * to have executed a full memory barrier during the execution of
 * synchronize_rcu_tasks() -- even if CPU A and CPU B are the same CPU
 * (but again only if the system has more than one CPU).
 */
void synchronize_rcu_tasks(void)
{
	/* Complain if the scheduler has not started.  */
	rcu_lockdep_assert(!rcu_scheduler_active,
			   "synchronize_rcu_tasks called too soon");

	/* Wait for the grace period. */
	wait_rcu_gp(call_rcu_tasks);
}
EXPORT_SYMBOL_GPL(synchronize_rcu_tasks);

/**
 * rcu_barrier_tasks - Wait for in-flight call_rcu_tasks() callbacks.
 *
 * Although the current implementation is guaranteed to wait, it is not
 * obligated to, for example, if there are no pending callbacks.
 */
void rcu_barrier_tasks(void)
{
	/* There is only one callback queue, so this is easy.  ;-) */
	synchronize_rcu_tasks();
}
EXPORT_SYMBOL_GPL(rcu_barrier_tasks);

/* See if tasks are still holding out, complain if so. */
static void check_holdout_task(struct task_struct *t,
			       bool needreport, bool *firstreport)
{
	int cpu;

	if (!ACCESS_ONCE(t->rcu_tasks_holdout) ||
	    t->rcu_tasks_nvcsw != ACCESS_ONCE(t->nvcsw) ||
	    !ACCESS_ONCE(t->on_rq) ||
	    (IS_ENABLED(CONFIG_NO_HZ_FULL) &&
	     !is_idle_task(t) && t->rcu_tasks_idle_cpu >= 0)) {
		ACCESS_ONCE(t->rcu_tasks_holdout) = false;
		list_del_init(&t->rcu_tasks_holdout_list);
		put_task_struct(t);
		return;
	}
	if (!needreport)
		return;
	if (*firstreport) {
		pr_err("INFO: rcu_tasks detected stalls on tasks:\n");
		*firstreport = false;
	}
	cpu = task_cpu(t);
	pr_alert("%p: %c%c nvcsw: %lu/%lu holdout: %d idle_cpu: %d/%d\n",
		 t, ".I"[is_idle_task(t)],
		 "N."[cpu < 0 || !tick_nohz_full_cpu(cpu)],
		 t->rcu_tasks_nvcsw, t->nvcsw, t->rcu_tasks_holdout,
		 t->rcu_tasks_idle_cpu, cpu);
	sched_show_task(t);
}

/* RCU-tasks kthread that detects grace periods and invokes callbacks. */
static int __noreturn rcu_tasks_kthread(void *arg)
{
	unsigned long flags;
	struct task_struct *g, *t;
	unsigned long lastreport;
	struct rcu_head *list;
	struct rcu_head *next;
	LIST_HEAD(rcu_tasks_holdouts);

	/* Run on housekeeping CPUs by default.  Sysadm can move if desired. */
	housekeeping_affine(current);

	/*
	 * Each pass through the following loop makes one check for
	 * newly arrived callbacks, and, if there are some, waits for
	 * one RCU-tasks grace period and then invokes the callbacks.
	 * This loop is terminated by the system going down.  ;-)
	 */
	for (;;) {

		/* Pick up any new callbacks. */
		raw_spin_lock_irqsave(&rcu_tasks_cbs_lock, flags);
		list = rcu_tasks_cbs_head;
		rcu_tasks_cbs_head = NULL;
		rcu_tasks_cbs_tail = &rcu_tasks_cbs_head;
		raw_spin_unlock_irqrestore(&rcu_tasks_cbs_lock, flags);

		/* If there were none, wait a bit and start over. */
		if (!list) {
			wait_event_interruptible(rcu_tasks_cbs_wq,
						 rcu_tasks_cbs_head);
			if (!rcu_tasks_cbs_head) {
				WARN_ON(signal_pending(current));
				schedule_timeout_interruptible(HZ/10);
			}
			continue;
		}

		/*
		 * Wait for all pre-existing t->on_rq and t->nvcsw
		 * transitions to complete.  Invoking synchronize_sched()
		 * suffices because all these transitions occur with
		 * interrupts disabled.  Without this synchronize_sched(),
		 * a read-side critical section that started before the
		 * grace period might be incorrectly seen as having started
		 * after the grace period.
		 *
		 * This synchronize_sched() also dispenses with the
		 * need for a memory barrier on the first store to
		 * ->rcu_tasks_holdout, as it forces the store to happen
		 * after the beginning of the grace period.
		 */
		synchronize_sched();

		/*
		 * There were callbacks, so we need to wait for an
		 * RCU-tasks grace period.  Start off by scanning
		 * the task list for tasks that are not already
		 * voluntarily blocked.  Mark these tasks and make
		 * a list of them in rcu_tasks_holdouts.
		 */
		rcu_read_lock();
		for_each_process_thread(g, t) {
			if (t != current && ACCESS_ONCE(t->on_rq) &&
			    !is_idle_task(t)) {
				get_task_struct(t);
				t->rcu_tasks_nvcsw = ACCESS_ONCE(t->nvcsw);
				ACCESS_ONCE(t->rcu_tasks_holdout) = true;
				list_add(&t->rcu_tasks_holdout_list,
					 &rcu_tasks_holdouts);
			}
		}
		rcu_read_unlock();

		/*
		 * Wait for tasks that are in the process of exiting.
		 * This does only part of the job, ensuring that all
		 * tasks that were previously exiting reach the point
		 * where they have disabled preemption, allowing the
		 * later synchronize_sched() to finish the job.
		 */
		synchronize_srcu(&tasks_rcu_exit_srcu);

		/*
		 * Each pass through the following loop scans the list
		 * of holdout tasks, removing any that are no longer
		 * holdouts.  When the list is empty, we are done.
		 */
		lastreport = jiffies;
		while (!list_empty(&rcu_tasks_holdouts)) {
			bool firstreport;
			bool needreport;
			int rtst;
			struct task_struct *t1;

			schedule_timeout_interruptible(HZ);
			rtst = ACCESS_ONCE(rcu_task_stall_timeout);
			needreport = rtst > 0 &&
				     time_after(jiffies, lastreport + rtst);
			if (needreport)
				lastreport = jiffies;
			firstreport = true;
			WARN_ON(signal_pending(current));
			list_for_each_entry_safe(t, t1, &rcu_tasks_holdouts,
						rcu_tasks_holdout_list) {
				check_holdout_task(t, needreport, &firstreport);
				cond_resched();
			}
		}

		/*
		 * Because ->on_rq and ->nvcsw are not guaranteed
		 * to have a full memory barriers prior to them in the
		 * schedule() path, memory reordering on other CPUs could
		 * cause their RCU-tasks read-side critical sections to
		 * extend past the end of the grace period.  However,
		 * because these ->nvcsw updates are carried out with
		 * interrupts disabled, we can use synchronize_sched()
		 * to force the needed ordering on all such CPUs.
		 *
		 * This synchronize_sched() also confines all
		 * ->rcu_tasks_holdout accesses to be within the grace
		 * period, avoiding the need for memory barriers for
		 * ->rcu_tasks_holdout accesses.
		 *
		 * In addition, this synchronize_sched() waits for exiting
		 * tasks to complete their final preempt_disable() region
		 * of execution, cleaning up after the synchronize_srcu()
		 * above.
		 */
		synchronize_sched();

		/* Invoke the callbacks. */
		while (list) {
			next = list->next;
			local_bh_disable();
			list->func(list);
			local_bh_enable();
			list = next;
			cond_resched();
		}
		schedule_timeout_uninterruptible(HZ/10);
	}
}

/* Spawn rcu_tasks_kthread() at first call to call_rcu_tasks(). */
static void rcu_spawn_tasks_kthread(void)
{
	static DEFINE_MUTEX(rcu_tasks_kthread_mutex);
	static struct task_struct *rcu_tasks_kthread_ptr;
	struct task_struct *t;

	if (ACCESS_ONCE(rcu_tasks_kthread_ptr)) {
		smp_mb(); /* Ensure caller sees full kthread. */
		return;
	}
	mutex_lock(&rcu_tasks_kthread_mutex);
	if (rcu_tasks_kthread_ptr) {
		mutex_unlock(&rcu_tasks_kthread_mutex);
		return;
	}
	t = kthread_run(rcu_tasks_kthread, NULL, "rcu_tasks_kthread");
	BUG_ON(IS_ERR(t));
	smp_mb(); /* Ensure others see full kthread. */
	ACCESS_ONCE(rcu_tasks_kthread_ptr) = t;
	mutex_unlock(&rcu_tasks_kthread_mutex);
}

#endif /* #ifdef CONFIG_TASKS_RCU */

#ifdef CONFIG_PROVE_RCU

/*
 * Early boot self test parameters, one for each flavor
 */
static bool rcu_self_test;
static bool rcu_self_test_bh;
static bool rcu_self_test_sched;

module_param(rcu_self_test, bool, 0444);
module_param(rcu_self_test_bh, bool, 0444);
module_param(rcu_self_test_sched, bool, 0444);

static int rcu_self_test_counter;

static void test_callback(struct rcu_head *r)
{
	rcu_self_test_counter++;
	pr_info("RCU test callback executed %d\n", rcu_self_test_counter);
}

static void early_boot_test_call_rcu(void)
{
	static struct rcu_head head;

	call_rcu(&head, test_callback);
}

static void early_boot_test_call_rcu_bh(void)
{
	static struct rcu_head head;

	call_rcu_bh(&head, test_callback);
}

static void early_boot_test_call_rcu_sched(void)
{
	static struct rcu_head head;

	call_rcu_sched(&head, test_callback);
}

void rcu_early_boot_tests(void)
{
	pr_info("Running RCU self tests\n");

	if (rcu_self_test)
		early_boot_test_call_rcu();
	if (rcu_self_test_bh)
		early_boot_test_call_rcu_bh();
	if (rcu_self_test_sched)
		early_boot_test_call_rcu_sched();
}

static int rcu_verify_early_boot_tests(void)
{
	int ret = 0;
	int early_boot_test_counter = 0;

	if (rcu_self_test) {
		early_boot_test_counter++;
		rcu_barrier();
	}
	if (rcu_self_test_bh) {
		early_boot_test_counter++;
		rcu_barrier_bh();
	}
	if (rcu_self_test_sched) {
		early_boot_test_counter++;
		rcu_barrier_sched();
	}

	if (rcu_self_test_counter != early_boot_test_counter) {
		WARN_ON(1);
		ret = -1;
	}

	return ret;
}
late_initcall(rcu_verify_early_boot_tests);
#else
void rcu_early_boot_tests(void) {}
#endif /* CONFIG_PROVE_RCU */
