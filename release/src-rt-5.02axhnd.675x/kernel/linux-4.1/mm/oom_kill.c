/*
 *  linux/mm/oom_kill.c
 * 
 *  Copyright (C)  1998,2000  Rik van Riel
 *	Thanks go out to Claus Fischer for some serious inspiration and
 *	for goading me into coding this file...
 *  Copyright (C)  2010  Google, Inc.
 *	Rewritten by David Rientjes
 *
 *  The routines in this file are used to kill a process when
 *  we're seriously out of memory. This gets called from __alloc_pages()
 *  in mm/page_alloc.c when we really run out of memory.
 *
 *  Since we won't call these routines often (on a well-configured
 *  machine) this file will double as a 'coding guide' and a signpost
 *  for newbie kernel hackers. It features several pointers to major
 *  kernel subsystems and hints as to where to find out what things do.
 */

#include <linux/oom.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/timex.h>
#include <linux/jiffies.h>
#include <linux/cpuset.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/security.h>
#include <linux/ptrace.h>
#include <linux/freezer.h>
#include <linux/ftrace.h>
#include <linux/ratelimit.h>

#define CREATE_TRACE_POINTS
#include <trace/events/oom.h>

int sysctl_panic_on_oom;
int sysctl_oom_kill_allocating_task;
int sysctl_oom_dump_tasks = 1;
static DEFINE_SPINLOCK(zone_scan_lock);

#ifdef CONFIG_NUMA
/**
 * has_intersects_mems_allowed() - check task eligiblity for kill
 * @start: task struct of which task to consider
 * @mask: nodemask passed to page allocator for mempolicy ooms
 *
 * Task eligibility is determined by whether or not a candidate task, @tsk,
 * shares the same mempolicy nodes as current if it is bound by such a policy
 * and whether or not it has the same set of allowed cpuset nodes.
 */
static bool has_intersects_mems_allowed(struct task_struct *start,
					const nodemask_t *mask)
{
	struct task_struct *tsk;
	bool ret = false;

	rcu_read_lock();
	for_each_thread(start, tsk) {
		if (mask) {
			/*
			 * If this is a mempolicy constrained oom, tsk's
			 * cpuset is irrelevant.  Only return true if its
			 * mempolicy intersects current, otherwise it may be
			 * needlessly killed.
			 */
			ret = mempolicy_nodemask_intersects(tsk, mask);
		} else {
			/*
			 * This is not a mempolicy constrained oom, so only
			 * check the mems of tsk's cpuset.
			 */
			ret = cpuset_mems_allowed_intersects(current, tsk);
		}
		if (ret)
			break;
	}
	rcu_read_unlock();

	return ret;
}
#else
static bool has_intersects_mems_allowed(struct task_struct *tsk,
					const nodemask_t *mask)
{
	return true;
}
#endif /* CONFIG_NUMA */

/*
 * The process p may have detached its own ->mm while exiting or through
 * use_mm(), but one or more of its subthreads may still have a valid
 * pointer.  Return p, or any of its subthreads with a valid ->mm, with
 * task_lock() held.
 */
struct task_struct *find_lock_task_mm(struct task_struct *p)
{
	struct task_struct *t;

	rcu_read_lock();

	for_each_thread(p, t) {
		task_lock(t);
		if (likely(t->mm))
			goto found;
		task_unlock(t);
	}
	t = NULL;
found:
	rcu_read_unlock();

	return t;
}

/* return true if the task is not adequate as candidate victim task. */
static bool oom_unkillable_task(struct task_struct *p,
		struct mem_cgroup *memcg, const nodemask_t *nodemask)
{
	if (is_global_init(p))
		return true;
	if (p->flags & PF_KTHREAD)
		return true;

	/* When mem_cgroup_out_of_memory() and p is not member of the group */
	if (memcg && !task_in_mem_cgroup(p, memcg))
		return true;

	/* p may not have freeable memory in nodemask */
	if (!has_intersects_mems_allowed(p, nodemask))
		return true;

	return false;
}

/**
 * oom_badness - heuristic function to determine which candidate task to kill
 * @p: task struct of which task we should calculate
 * @totalpages: total present RAM allowed for page allocation
 *
 * The heuristic for determining which task to kill is made to be as simple and
 * predictable as possible.  The goal is to return the highest value for the
 * task consuming the most memory to avoid subsequent oom failures.
 */
unsigned long oom_badness(struct task_struct *p, struct mem_cgroup *memcg,
			  const nodemask_t *nodemask, unsigned long totalpages)
{
	long points;
	long adj;

	if (oom_unkillable_task(p, memcg, nodemask))
		return 0;

	p = find_lock_task_mm(p);
	if (!p)
		return 0;

	adj = (long)p->signal->oom_score_adj;
	if (adj == OOM_SCORE_ADJ_MIN) {
		task_unlock(p);
		return 0;
	}

	/*
	 * The baseline for the badness score is the proportion of RAM that each
	 * task's rss, pagetable and swap space use.
	 */
	points = get_mm_rss(p->mm) + get_mm_counter(p->mm, MM_SWAPENTS) +
		atomic_long_read(&p->mm->nr_ptes) + mm_nr_pmds(p->mm);
	task_unlock(p);

	/*
	 * Root processes get 3% bonus, just like the __vm_enough_memory()
	 * implementation used by LSMs.
	 */
	if (has_capability_noaudit(p, CAP_SYS_ADMIN))
		points -= (points * 3) / 100;

	/* Normalize to oom_score_adj units */
	adj *= totalpages / 1000;
	points += adj;

	/*
	 * Never return 0 for an eligible task regardless of the root bonus and
	 * oom_score_adj (oom_score_adj can't be OOM_SCORE_ADJ_MIN here).
	 */
	return points > 0 ? points : 1;
}

/*
 * Determine the type of allocation constraint.
 */
#if !defined(CONFIG_BCM_KF_OOM_REBOOT)

#ifdef CONFIG_NUMA
static enum oom_constraint constrained_alloc(struct zonelist *zonelist,
				gfp_t gfp_mask, nodemask_t *nodemask,
				unsigned long *totalpages)
{
	struct zone *zone;
	struct zoneref *z;
	enum zone_type high_zoneidx = gfp_zone(gfp_mask);
	bool cpuset_limited = false;
	int nid;

	/* Default to all available memory */
	*totalpages = totalram_pages + total_swap_pages;

	if (!zonelist)
		return CONSTRAINT_NONE;
	/*
	 * Reach here only when __GFP_NOFAIL is used. So, we should avoid
	 * to kill current.We have to random task kill in this case.
	 * Hopefully, CONSTRAINT_THISNODE...but no way to handle it, now.
	 */
	if (gfp_mask & __GFP_THISNODE)
		return CONSTRAINT_NONE;

	/*
	 * This is not a __GFP_THISNODE allocation, so a truncated nodemask in
	 * the page allocator means a mempolicy is in effect.  Cpuset policy
	 * is enforced in get_page_from_freelist().
	 */
	if (nodemask && !nodes_subset(node_states[N_MEMORY], *nodemask)) {
		*totalpages = total_swap_pages;
		for_each_node_mask(nid, *nodemask)
			*totalpages += node_spanned_pages(nid);
		return CONSTRAINT_MEMORY_POLICY;
	}

	/* Check this allocation failure is caused by cpuset's wall function */
	for_each_zone_zonelist_nodemask(zone, z, zonelist,
			high_zoneidx, nodemask)
		if (!cpuset_zone_allowed(zone, gfp_mask))
			cpuset_limited = true;

	if (cpuset_limited) {
		*totalpages = total_swap_pages;
		for_each_node_mask(nid, cpuset_current_mems_allowed)
			*totalpages += node_spanned_pages(nid);
		return CONSTRAINT_CPUSET;
	}
	return CONSTRAINT_NONE;
}
#else
static enum oom_constraint constrained_alloc(struct zonelist *zonelist,
				gfp_t gfp_mask, nodemask_t *nodemask,
				unsigned long *totalpages)
{
	*totalpages = totalram_pages + total_swap_pages;
	return CONSTRAINT_NONE;
}
#endif
#endif   /* CONFIG_BCM_KF_OOM_REBOOT */

enum oom_scan_t oom_scan_process_thread(struct task_struct *task,
		unsigned long totalpages, const nodemask_t *nodemask,
		bool force_kill)
{
	if (oom_unkillable_task(task, NULL, nodemask))
		return OOM_SCAN_CONTINUE;

	/*
	 * This task already has access to memory reserves and is being killed.
	 * Don't allow any other task to have access to the reserves.
	 */
	if (test_tsk_thread_flag(task, TIF_MEMDIE)) {
		if (!force_kill)
			return OOM_SCAN_ABORT;
	}
	if (!task->mm)
		return OOM_SCAN_CONTINUE;

	/*
	 * If task is allocating a lot of memory and has been marked to be
	 * killed first if it triggers an oom, then select it.
	 */
	if (oom_task_origin(task))
		return OOM_SCAN_SELECT;

	if (task_will_free_mem(task) && !force_kill)
		return OOM_SCAN_ABORT;

	return OOM_SCAN_OK;
}

#if !defined(CONFIG_BCM_KF_OOM_REBOOT)
/*
 * Simple selection loop. We chose the process with the highest
 * number of 'points'.  Returns -1 on scan abort.
 *
 * (not docbooked, we don't want this one cluttering up the manual)
 */
static struct task_struct *select_bad_process(unsigned int *ppoints,
		unsigned long totalpages, const nodemask_t *nodemask,
		bool force_kill)
{
	struct task_struct *g, *p;
	struct task_struct *chosen = NULL;
	unsigned long chosen_points = 0;

	rcu_read_lock();
	for_each_process_thread(g, p) {
		unsigned int points;

		switch (oom_scan_process_thread(p, totalpages, nodemask,
						force_kill)) {
		case OOM_SCAN_SELECT:
			chosen = p;
			chosen_points = ULONG_MAX;
			/* fall through */
		case OOM_SCAN_CONTINUE:
			continue;
		case OOM_SCAN_ABORT:
			rcu_read_unlock();
			return (struct task_struct *)(-1UL);
		case OOM_SCAN_OK:
			break;
		};
		points = oom_badness(p, NULL, nodemask, totalpages);
		if (!points || points < chosen_points)
			continue;
		/* Prefer thread group leaders for display purposes */
		if (points == chosen_points && thread_group_leader(chosen))
			continue;

		chosen = p;
		chosen_points = points;
	}
	if (chosen)
		get_task_struct(chosen);
	rcu_read_unlock();

	*ppoints = chosen_points * 1000 / totalpages;
	return chosen;
}

#endif
/**
 * dump_tasks - dump current memory state of all system tasks
 * @memcg: current's memory controller, if constrained
 * @nodemask: nodemask passed to page allocator for mempolicy ooms
 *
 * Dumps the current memory state of all eligible tasks.  Tasks not in the same
 * memcg, not in the same cpuset, or bound to a disjoint set of mempolicy nodes
 * are not shown.
 * State information includes task's pid, uid, tgid, vm size, rss, nr_ptes,
 * swapents, oom_score_adj value, and name.
 */
static void dump_tasks(struct mem_cgroup *memcg, const nodemask_t *nodemask)
{
	struct task_struct *p;
	struct task_struct *task;

	pr_warning("[ pid ]   uid  tgid total_vm      rss nr_ptes nr_pmds swapents oom_score_adj name\n");
	rcu_read_lock();
	for_each_process(p) {
		if (oom_unkillable_task(p, memcg, nodemask))
			continue;

		task = find_lock_task_mm(p);
		if (!task) {
			/*
			 * This is a kthread or all of p's threads have already
			 * detached their mm's.  There's no need to report
			 * them; they can't be oom killed anyway.
			 */
			continue;
		}

		pr_warning("[%5d] %5d %5d %8lu %8lu %7ld %7ld %8lu         %5hd %s\n",
			task->pid, from_kuid(&init_user_ns, task_uid(task)),
			task->tgid, task->mm->total_vm, get_mm_rss(task->mm),
			atomic_long_read(&task->mm->nr_ptes),
			mm_nr_pmds(task->mm),
			get_mm_counter(task->mm, MM_SWAPENTS),
			task->signal->oom_score_adj, task->comm);
		task_unlock(task);
	}
	rcu_read_unlock();
}

static void dump_header(struct task_struct *p, gfp_t gfp_mask, int order,
			struct mem_cgroup *memcg, const nodemask_t *nodemask)
{
	task_lock(current);
	pr_warning("%s invoked oom-killer: gfp_mask=0x%x, order=%d, "
		"oom_score_adj=%hd\n",
		current->comm, gfp_mask, order,
		current->signal->oom_score_adj);
	cpuset_print_task_mems_allowed(current);
	task_unlock(current);
	dump_stack();
	if (memcg)
		mem_cgroup_print_oom_info(memcg, p);
	else
		show_mem(SHOW_MEM_FILTER_NODES);
	if (sysctl_oom_dump_tasks)
		dump_tasks(memcg, nodemask);
}

/*
 * Number of OOM victims in flight
 */
static atomic_t oom_victims = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(oom_victims_wait);

bool oom_killer_disabled __read_mostly;
static DECLARE_RWSEM(oom_sem);


/**
 * mark_tsk_oom_victim - marks the given task as OOM victim.
 * @tsk: task to mark
 *
 * Has to be called with oom_sem taken for read and never after
 * oom has been disabled already.
 */
void mark_tsk_oom_victim(struct task_struct *tsk)
{
	WARN_ON(oom_killer_disabled);
	/* OOM killer might race with memcg OOM */
	if (test_and_set_tsk_thread_flag(tsk, TIF_MEMDIE))
		return;
	/*
	 * Make sure that the task is woken up from uninterruptible sleep
	 * if it is frozen because OOM killer wouldn't be able to free
	 * any memory and livelock. freezing_slow_path will tell the freezer
	 * that TIF_MEMDIE tasks should be ignored.
	 */
	__thaw_task(tsk);
	atomic_inc(&oom_victims);
}

/**
 * unmark_oom_victim - unmarks the current task as OOM victim.
 *
 * Wakes up all waiters in oom_killer_disable()
 */
void unmark_oom_victim(void)
{
	if (!test_and_clear_thread_flag(TIF_MEMDIE))
		return;

	down_read(&oom_sem);
	/*
	 * There is no need to signal the lasst oom_victim if there
	 * is nobody who cares.
	 */
	if (!atomic_dec_return(&oom_victims) && oom_killer_disabled)
		wake_up_all(&oom_victims_wait);
	up_read(&oom_sem);
}

/**
 * oom_killer_disable - disable OOM killer
 *
 * Forces all page allocations to fail rather than trigger OOM killer.
 * Will block and wait until all OOM victims are killed.
 *
 * The function cannot be called when there are runnable user tasks because
 * the userspace would see unexpected allocation failures as a result. Any
 * new usage of this function should be consulted with MM people.
 *
 * Returns true if successful and false if the OOM killer cannot be
 * disabled.
 */
bool oom_killer_disable(void)
{
	/*
	 * Make sure to not race with an ongoing OOM killer
	 * and that the current is not the victim.
	 */
	down_write(&oom_sem);
	if (test_thread_flag(TIF_MEMDIE)) {
		up_write(&oom_sem);
		return false;
	}

	oom_killer_disabled = true;
	up_write(&oom_sem);

	wait_event(oom_victims_wait, !atomic_read(&oom_victims));

	return true;
}

/**
 * oom_killer_enable - enable OOM killer
 */
void oom_killer_enable(void)
{
	down_write(&oom_sem);
	oom_killer_disabled = false;
	up_write(&oom_sem);
}



#define K(x) ((x) << (PAGE_SHIFT-10))
/*
 * Must be called while holding a reference to p, which will be released upon
 * returning.
 */
void oom_kill_process(struct task_struct *p, gfp_t gfp_mask, int order,
		      unsigned int points, unsigned long totalpages,
		      struct mem_cgroup *memcg, nodemask_t *nodemask,
		      const char *message)
{
	struct task_struct *victim = p;
	struct task_struct *child;
	struct task_struct *t;
	struct mm_struct *mm;
	unsigned int victim_points = 0;
	static DEFINE_RATELIMIT_STATE(oom_rs, DEFAULT_RATELIMIT_INTERVAL,
					      DEFAULT_RATELIMIT_BURST);

	/*
	 * If the task is already exiting, don't alarm the sysadmin or kill
	 * its children or threads, just set TIF_MEMDIE so it can die quickly
	 */
	task_lock(p);
	if (p->mm && task_will_free_mem(p)) {
		mark_tsk_oom_victim(p);
		task_unlock(p);
		put_task_struct(p);
		return;
	}
	task_unlock(p);

	if (__ratelimit(&oom_rs))
		dump_header(p, gfp_mask, order, memcg, nodemask);

	task_lock(p);
	pr_err("%s: Kill process %d (%s) score %d or sacrifice child\n",
		message, task_pid_nr(p), p->comm, points);
	task_unlock(p);

	/*
	 * If any of p's children has a different mm and is eligible for kill,
	 * the one with the highest oom_badness() score is sacrificed for its
	 * parent.  This attempts to lose the minimal amount of work done while
	 * still freeing memory.
	 */
	read_lock(&tasklist_lock);
	for_each_thread(p, t) {
		list_for_each_entry(child, &t->children, sibling) {
			unsigned int child_points;

			if (child->mm == p->mm)
				continue;
			/*
			 * oom_badness() returns 0 if the thread is unkillable
			 */
			child_points = oom_badness(child, memcg, nodemask,
								totalpages);
			if (child_points > victim_points) {
				put_task_struct(victim);
				victim = child;
				victim_points = child_points;
				get_task_struct(victim);
			}
		}
	}
	read_unlock(&tasklist_lock);

	p = find_lock_task_mm(victim);
	if (!p) {
		put_task_struct(victim);
		return;
	} else if (victim != p) {
		get_task_struct(p);
		put_task_struct(victim);
		victim = p;
	}

	/* mm cannot safely be dereferenced after task_unlock(victim) */
	mm = victim->mm;
	mark_tsk_oom_victim(victim);
	pr_err("Killed process %d (%s) total-vm:%lukB, anon-rss:%lukB, file-rss:%lukB\n",
		task_pid_nr(victim), victim->comm, K(victim->mm->total_vm),
		K(get_mm_counter(victim->mm, MM_ANONPAGES)),
		K(get_mm_counter(victim->mm, MM_FILEPAGES)));
	task_unlock(victim);

	/*
	 * Kill all user processes sharing victim->mm in other thread groups, if
	 * any.  They don't get access to memory reserves, though, to avoid
	 * depletion of all memory.  This prevents mm->mmap_sem livelock when an
	 * oom killed thread cannot exit because it requires the semaphore and
	 * its contended by another thread trying to allocate memory itself.
	 * That thread will now get access to memory reserves since it has a
	 * pending fatal signal.
	 */
	rcu_read_lock();
	for_each_process(p)
		if (p->mm == mm && !same_thread_group(p, victim) &&
		    !(p->flags & PF_KTHREAD)) {
			if (p->signal->oom_score_adj == OOM_SCORE_ADJ_MIN)
				continue;

			task_lock(p);	/* Protect ->comm from prctl() */
			pr_err("Kill process %d (%s) sharing same memory\n",
				task_pid_nr(p), p->comm);
			task_unlock(p);
			do_send_sig_info(SIGKILL, SEND_SIG_FORCED, p, true);
		}
	rcu_read_unlock();

	do_send_sig_info(SIGKILL, SEND_SIG_FORCED, victim, true);
	put_task_struct(victim);
}
#undef K

/*
 * Determines whether the kernel must panic because of the panic_on_oom sysctl.
 */
void check_panic_on_oom(enum oom_constraint constraint, gfp_t gfp_mask,
			int order, const nodemask_t *nodemask,
			struct mem_cgroup *memcg)
{
	if (likely(!sysctl_panic_on_oom))
		return;
	if (sysctl_panic_on_oom != 2) {
		/*
		 * panic_on_oom == 1 only affects CONSTRAINT_NONE, the kernel
		 * does not panic for cpuset, mempolicy, or memcg allocation
		 * failures.
		 */
		if (constraint != CONSTRAINT_NONE)
			return;
	}
	dump_header(NULL, gfp_mask, order, memcg, nodemask);
	panic("Out of memory: %s panic_on_oom is enabled\n",
		sysctl_panic_on_oom == 2 ? "compulsory" : "system-wide");
}

static BLOCKING_NOTIFIER_HEAD(oom_notify_list);

int register_oom_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&oom_notify_list, nb);
}
EXPORT_SYMBOL_GPL(register_oom_notifier);

int unregister_oom_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&oom_notify_list, nb);
}
EXPORT_SYMBOL_GPL(unregister_oom_notifier);

/*
 * Try to acquire the OOM killer lock for the zones in zonelist.  Returns zero
 * if a parallel OOM killing is already taking place that includes a zone in
 * the zonelist.  Otherwise, locks all zones in the zonelist and returns 1.
 */
bool oom_zonelist_trylock(struct zonelist *zonelist, gfp_t gfp_mask)
{
	struct zoneref *z;
	struct zone *zone;
	bool ret = true;

	spin_lock(&zone_scan_lock);
	for_each_zone_zonelist(zone, z, zonelist, gfp_zone(gfp_mask))
		if (test_bit(ZONE_OOM_LOCKED, &zone->flags)) {
			ret = false;
			goto out;
		}

	/*
	 * Lock each zone in the zonelist under zone_scan_lock so a parallel
	 * call to oom_zonelist_trylock() doesn't succeed when it shouldn't.
	 */
	for_each_zone_zonelist(zone, z, zonelist, gfp_zone(gfp_mask))
		set_bit(ZONE_OOM_LOCKED, &zone->flags);

out:
	spin_unlock(&zone_scan_lock);
	return ret;
}

/*
 * Clears the ZONE_OOM_LOCKED flag for all zones in the zonelist so that failed
 * allocation attempts with zonelists containing them may now recall the OOM
 * killer, if necessary.
 */
void oom_zonelist_unlock(struct zonelist *zonelist, gfp_t gfp_mask)
{
	struct zoneref *z;
	struct zone *zone;

	spin_lock(&zone_scan_lock);
	for_each_zone_zonelist(zone, z, zonelist, gfp_zone(gfp_mask))
		clear_bit(ZONE_OOM_LOCKED, &zone->flags);
	spin_unlock(&zone_scan_lock);
}

/**
 * __out_of_memory - kill the "best" process when we run out of memory
 * @zonelist: zonelist pointer
 * @gfp_mask: memory allocation flags
 * @order: amount of memory being requested as a power of 2
 * @nodemask: nodemask passed to page allocator
 * @force_kill: true if a task must be killed, even if others are exiting
 *
 * If we run out of memory, we have the choice between either
 * killing a random task (bad), letting the system crash (worse)
 * OR try to be smart about which process to kill. Note that we
 * don't have to be perfect here, we just have to be good.
 */
static void __out_of_memory(struct zonelist *zonelist, gfp_t gfp_mask,
		int order, nodemask_t *nodemask, bool force_kill)
{
#if defined(CONFIG_BCM_KF_OOM_REBOOT)
#define OOM_REBOOT_DELAY (5)
#define OOM_REBOOT_INTERVAL (HZ*120)
	static int oom_count=0;
	static unsigned long oom_timestamp=0;
	unsigned long freed = 0;
#else
	const nodemask_t *mpol_mask;
	struct task_struct *p;
	unsigned long totalpages;
	unsigned long freed = 0;
	unsigned int uninitialized_var(points);
	enum oom_constraint constraint = CONSTRAINT_NONE;
	int killed = 0;
#endif

	blocking_notifier_call_chain(&oom_notify_list, 0, &freed);
	if (freed > 0)
		/* Got some memory back in the last second. */
		return;

	/*
	 * If current has a pending SIGKILL or is exiting, then automatically
	 * select it.  The goal is to allow it to allocate so that it may
	 * quickly exit and free its memory.
	 *
	 * But don't select if current has already released its mm and cleared
	 * TIF_MEMDIE flag at exit_mm(), otherwise an OOM livelock may occur.
	 */
	if (current->mm &&
	    (fatal_signal_pending(current) || task_will_free_mem(current))) {
		mark_tsk_oom_victim(current);
		return;
	}

#if defined(CONFIG_BCM_KF_OOM_REBOOT)

	/* For our embedded system, most of the processes are considered essential. */
	/* Randomly killing a process is no better than a reboot so we won't kill process here*/
	
	printk(KERN_WARNING "\n\n%s triggered out of memory codition (oom killer not called): "
		"gfp_mask=0x%x, order=%d, oom_score_adj=%d\n\n",
		current->comm, gfp_mask, order, current->signal->oom_score_adj);
	dump_stack();
	show_mem(0);
	printk("\n");
	/*
	 * The process that triggered the oom is not necessarily the one that
	 * caused it.  dump_tasks shows all tasks and their memory usage.
	 */
	read_lock(&tasklist_lock);
	dump_tasks(NULL, nodemask);
	read_unlock(&tasklist_lock);

	/* Reboot if OOM, but don't do it immediately - just in case this can be too sensitive */
	if ((jiffies - oom_timestamp) > OOM_REBOOT_INTERVAL) {
		oom_timestamp = jiffies;
		oom_count = 0;		
	}
	else {
		oom_count++;
		if (oom_count >= OOM_REBOOT_DELAY) {
			panic("Reboot due to persistent out of memory codition..");
		}
	}
	schedule_timeout_interruptible(HZ*5);

#else

	/*
	 * Check if there were limitations on the allocation (only relevant for
	 * NUMA) that may require different handling.
	 */
	constraint = constrained_alloc(zonelist, gfp_mask, nodemask,
						&totalpages);
	mpol_mask = (constraint == CONSTRAINT_MEMORY_POLICY) ? nodemask : NULL;
	check_panic_on_oom(constraint, gfp_mask, order, mpol_mask, NULL);

	if (sysctl_oom_kill_allocating_task && current->mm &&
	    !oom_unkillable_task(current, NULL, nodemask) &&
	    current->signal->oom_score_adj != OOM_SCORE_ADJ_MIN) {
		get_task_struct(current);
		oom_kill_process(current, gfp_mask, order, 0, totalpages, NULL,
				 nodemask,
				 "Out of memory (oom_kill_allocating_task)");
		goto out;
	}

	p = select_bad_process(&points, totalpages, mpol_mask, force_kill);
	/* Found nothing?!?! Either we hang forever, or we panic. */
	if (!p) {
		dump_header(NULL, gfp_mask, order, NULL, mpol_mask);
		panic("Out of memory and no killable processes...\n");
	}
	if (p != (void *)-1UL) {
		oom_kill_process(p, gfp_mask, order, points, totalpages, NULL,
				 nodemask, "Out of memory");
		killed = 1;
	}
out:
	/*
	 * Give the killed threads a good chance of exiting before trying to
	 * allocate memory again.
	 */
	if (killed)
		schedule_timeout_killable(1);
#endif
}

/**
 * out_of_memory -  tries to invoke OOM killer.
 * @zonelist: zonelist pointer
 * @gfp_mask: memory allocation flags
 * @order: amount of memory being requested as a power of 2
 * @nodemask: nodemask passed to page allocator
 * @force_kill: true if a task must be killed, even if others are exiting
 *
 * invokes __out_of_memory if the OOM is not disabled by oom_killer_disable()
 * when it returns false. Otherwise returns true.
 */
bool out_of_memory(struct zonelist *zonelist, gfp_t gfp_mask,
		int order, nodemask_t *nodemask, bool force_kill)
{
	bool ret = false;

	down_read(&oom_sem);
	if (!oom_killer_disabled) {
		__out_of_memory(zonelist, gfp_mask, order, nodemask, force_kill);
		ret = true;
	}
	up_read(&oom_sem);

	return ret;
}

/*
 * The pagefault handler calls here because it is out of memory, so kill a
 * memory-hogging task.  If any populated zone has ZONE_OOM_LOCKED set, a
 * parallel oom killing is already in progress so do nothing.
 */
void pagefault_out_of_memory(void)
{
	struct zonelist *zonelist;

	down_read(&oom_sem);
	if (mem_cgroup_oom_synchronize(true))
		goto unlock;

	zonelist = node_zonelist(first_memory_node, GFP_KERNEL);
	if (oom_zonelist_trylock(zonelist, GFP_KERNEL)) {
		if (!oom_killer_disabled)
			__out_of_memory(NULL, 0, 0, NULL, false);
		else
			/*
			 * There shouldn't be any user tasks runable while the
			 * OOM killer is disabled so the current task has to
			 * be a racing OOM victim for which oom_killer_disable()
			 * is waiting for.
			 */
			WARN_ON(test_thread_flag(TIF_MEMDIE));

		oom_zonelist_unlock(zonelist, GFP_KERNEL);
	}
unlock:
	up_read(&oom_sem);
}
