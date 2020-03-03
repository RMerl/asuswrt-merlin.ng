/*
 * Read-Copy Update mechanism for mutual exclusion, the Bloatwatch edition.
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
 * Copyright IBM Corporation, 2008
 *
 * Author: Paul E. McKenney <paulmck@linux.vnet.ibm.com>
 *
 * For detailed explanation of Read-Copy Update mechanism see -
 *		Documentation/RCU
 */
#ifndef __LINUX_TINY_H
#define __LINUX_TINY_H

#include <linux/cache.h>

static inline unsigned long get_state_synchronize_rcu(void)
{
	return 0;
}

static inline void cond_synchronize_rcu(unsigned long oldstate)
{
	might_sleep();
}

static inline void rcu_barrier_bh(void)
{
	wait_rcu_gp(call_rcu_bh);
}

static inline void rcu_barrier_sched(void)
{
	wait_rcu_gp(call_rcu_sched);
}

static inline void synchronize_rcu_expedited(void)
{
	synchronize_sched();	/* Only one CPU, so pretty fast anyway!!! */
}

static inline void rcu_barrier(void)
{
	rcu_barrier_sched();  /* Only one CPU, so only one list of callbacks! */
}

static inline void synchronize_rcu_bh(void)
{
	synchronize_sched();
}

static inline void synchronize_rcu_bh_expedited(void)
{
	synchronize_sched();
}

static inline void synchronize_sched_expedited(void)
{
	synchronize_sched();
}

static inline void kfree_call_rcu(struct rcu_head *head,
				  void (*func)(struct rcu_head *rcu))
{
	call_rcu(head, func);
}

static inline void rcu_note_context_switch(void)
{
	rcu_sched_qs();
}

/*
 * Take advantage of the fact that there is only one CPU, which
 * allows us to ignore virtualization-based context switches.
 */
static inline void rcu_virt_note_context_switch(int cpu)
{
}

/*
 * Return the number of grace periods started.
 */
static inline unsigned long rcu_batches_started(void)
{
	return 0;
}

/*
 * Return the number of bottom-half grace periods started.
 */
static inline unsigned long rcu_batches_started_bh(void)
{
	return 0;
}

/*
 * Return the number of sched grace periods started.
 */
static inline unsigned long rcu_batches_started_sched(void)
{
	return 0;
}

/*
 * Return the number of grace periods completed.
 */
static inline unsigned long rcu_batches_completed(void)
{
	return 0;
}

/*
 * Return the number of bottom-half grace periods completed.
 */
static inline unsigned long rcu_batches_completed_bh(void)
{
	return 0;
}

/*
 * Return the number of sched grace periods completed.
 */
static inline unsigned long rcu_batches_completed_sched(void)
{
	return 0;
}

static inline void rcu_force_quiescent_state(void)
{
}

static inline void rcu_bh_force_quiescent_state(void)
{
}

static inline void rcu_sched_force_quiescent_state(void)
{
}

static inline void show_rcu_gp_kthreads(void)
{
}

static inline void rcu_cpu_stall_reset(void)
{
}

static inline void exit_rcu(void)
{
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC
extern int rcu_scheduler_active __read_mostly;
void rcu_scheduler_starting(void);
#else /* #ifdef CONFIG_DEBUG_LOCK_ALLOC */
static inline void rcu_scheduler_starting(void)
{
}
#endif /* #else #ifdef CONFIG_DEBUG_LOCK_ALLOC */

#if defined(CONFIG_DEBUG_LOCK_ALLOC) || defined(CONFIG_RCU_TRACE)

static inline bool rcu_is_watching(void)
{
	return __rcu_is_watching();
}

#else /* defined(CONFIG_DEBUG_LOCK_ALLOC) || defined(CONFIG_RCU_TRACE) */

static inline bool rcu_is_watching(void)
{
	return true;
}

#endif /* #else defined(CONFIG_DEBUG_LOCK_ALLOC) || defined(CONFIG_RCU_TRACE) */

static inline void rcu_all_qs(void)
{
}

#endif /* __LINUX_RCUTINY_H */
