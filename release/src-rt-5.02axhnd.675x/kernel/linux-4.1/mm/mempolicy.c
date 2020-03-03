/*
 * Simple NUMA memory policy for the Linux kernel.
 *
 * Copyright 2003,2004 Andi Kleen, SuSE Labs.
 * (C) Copyright 2005 Christoph Lameter, Silicon Graphics, Inc.
 * Subject to the GNU Public License, version 2.
 *
 * NUMA policy allows the user to give hints in which node(s) memory should
 * be allocated.
 *
 * Support four policies per VMA and per process:
 *
 * The VMA policy has priority over the process policy for a page fault.
 *
 * interleave     Allocate memory interleaved over a set of nodes,
 *                with normal fallback if it fails.
 *                For VMA based allocations this interleaves based on the
 *                offset into the backing object or offset into the mapping
 *                for anonymous memory. For process policy an process counter
 *                is used.
 *
 * bind           Only allocate memory on a specific set of nodes,
 *                no fallback.
 *                FIXME: memory is allocated starting with the first node
 *                to the last. It would be better if bind would truly restrict
 *                the allocation to memory nodes instead
 *
 * preferred       Try a specific node first before normal fallback.
 *                As a special case NUMA_NO_NODE here means do the allocation
 *                on the local CPU. This is normally identical to default,
 *                but useful to set in a VMA when you have a non default
 *                process policy.
 *
 * default        Allocate on the local node first, or when on a VMA
 *                use the process policy. This is what Linux always did
 *		  in a NUMA aware kernel and still does by, ahem, default.
 *
 * The process policy is applied for most non interrupt memory allocations
 * in that process' context. Interrupts ignore the policies and always
 * try to allocate on the local CPU. The VMA policy is only applied for memory
 * allocations for a VMA in the VM.
 *
 * Currently there are a few corner cases in swapping where the policy
 * is not applied, but the majority should be handled. When process policy
 * is used it is not remembered over swap outs/swap ins.
 *
 * Only the highest zone in the zone hierarchy gets policied. Allocations
 * requesting a lower zone just use default policy. This implies that
 * on systems with highmem kernel lowmem allocation don't get policied.
 * Same with GFP_DMA allocations.
 *
 * For shmfs/tmpfs/hugetlbfs shared memory the policy is shared between
 * all users and remembered even when nobody has memory mapped.
 */

/* Notebook:
   fix mmap readahead to honour policy and enable policy for any page cache
   object
   statistics for bigpages
   global policy for page cache? currently it uses process policy. Requires
   first item above.
   handle mremap for shared memory (currently ignored for the policy)
   grows down?
   make bind policy root only? It can trigger oom much faster and the
   kernel is not always grateful with that.
*/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/mempolicy.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/hugetlb.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/nodemask.h>
#include <linux/cpuset.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/export.h>
#include <linux/nsproxy.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/compat.h>
#include <linux/swap.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/migrate.h>
#include <linux/ksm.h>
#include <linux/rmap.h>
#include <linux/security.h>
#include <linux/syscalls.h>
#include <linux/ctype.h>
#include <linux/mm_inline.h>
#include <linux/mmu_notifier.h>
#include <linux/printk.h>

#include <asm/tlbflush.h>
#include <asm/uaccess.h>
#include <linux/random.h>

#include "internal.h"

/* Internal flags */
#define MPOL_MF_DISCONTIG_OK (MPOL_MF_INTERNAL << 0)	/* Skip checks for continuous vmas */
#define MPOL_MF_INVERT (MPOL_MF_INTERNAL << 1)		/* Invert check for nodemask */

static struct kmem_cache *policy_cache;
static struct kmem_cache *sn_cache;

/* Highest zone. An specific allocation for a zone below that is not
   policied. */
enum zone_type policy_zone = 0;

/*
 * run-time system-wide default policy => local allocation
 */
static struct mempolicy default_policy = {
	.refcnt = ATOMIC_INIT(1), /* never free it */
	.mode = MPOL_PREFERRED,
	.flags = MPOL_F_LOCAL,
};

static struct mempolicy preferred_node_policy[MAX_NUMNODES];

struct mempolicy *get_task_policy(struct task_struct *p)
{
	struct mempolicy *pol = p->mempolicy;
	int node;

	if (pol)
		return pol;

	node = numa_node_id();
	if (node != NUMA_NO_NODE) {
		pol = &preferred_node_policy[node];
		/* preferred_node_policy is not initialised early in boot */
		if (pol->mode)
			return pol;
	}

	return &default_policy;
}

static const struct mempolicy_operations {
	int (*create)(struct mempolicy *pol, const nodemask_t *nodes);
	/*
	 * If read-side task has no lock to protect task->mempolicy, write-side
	 * task will rebind the task->mempolicy by two step. The first step is
	 * setting all the newly nodes, and the second step is cleaning all the
	 * disallowed nodes. In this way, we can avoid finding no node to alloc
	 * page.
	 * If we have a lock to protect task->mempolicy in read-side, we do
	 * rebind directly.
	 *
	 * step:
	 * 	MPOL_REBIND_ONCE - do rebind work at once
	 * 	MPOL_REBIND_STEP1 - set all the newly nodes
	 * 	MPOL_REBIND_STEP2 - clean all the disallowed nodes
	 */
	void (*rebind)(struct mempolicy *pol, const nodemask_t *nodes,
			enum mpol_rebind_step step);
} mpol_ops[MPOL_MAX];

static inline int mpol_store_user_nodemask(const struct mempolicy *pol)
{
	return pol->flags & MPOL_MODE_FLAGS;
}

static void mpol_relative_nodemask(nodemask_t *ret, const nodemask_t *orig,
				   const nodemask_t *rel)
{
	nodemask_t tmp;
	nodes_fold(tmp, *orig, nodes_weight(*rel));
	nodes_onto(*ret, tmp, *rel);
}

static int mpol_new_interleave(struct mempolicy *pol, const nodemask_t *nodes)
{
	if (nodes_empty(*nodes))
		return -EINVAL;
	pol->v.nodes = *nodes;
	return 0;
}

static int mpol_new_preferred(struct mempolicy *pol, const nodemask_t *nodes)
{
	if (!nodes)
		pol->flags |= MPOL_F_LOCAL;	/* local allocation */
	else if (nodes_empty(*nodes))
		return -EINVAL;			/*  no allowed nodes */
	else
		pol->v.preferred_node = first_node(*nodes);
	return 0;
}

static int mpol_new_bind(struct mempolicy *pol, const nodemask_t *nodes)
{
	if (nodes_empty(*nodes))
		return -EINVAL;
	pol->v.nodes = *nodes;
	return 0;
}

/*
 * mpol_set_nodemask is called after mpol_new() to set up the nodemask, if
 * any, for the new policy.  mpol_new() has already validated the nodes
 * parameter with respect to the policy mode and flags.  But, we need to
 * handle an empty nodemask with MPOL_PREFERRED here.
 *
 * Must be called holding task's alloc_lock to protect task's mems_allowed
 * and mempolicy.  May also be called holding the mmap_semaphore for write.
 */
static int mpol_set_nodemask(struct mempolicy *pol,
		     const nodemask_t *nodes, struct nodemask_scratch *nsc)
{
	int ret;

	/* if mode is MPOL_DEFAULT, pol is NULL. This is right. */
	if (pol == NULL)
		return 0;
	/* Check N_MEMORY */
	nodes_and(nsc->mask1,
		  cpuset_current_mems_allowed, node_states[N_MEMORY]);

	VM_BUG_ON(!nodes);
	if (pol->mode == MPOL_PREFERRED && nodes_empty(*nodes))
		nodes = NULL;	/* explicit local allocation */
	else {
		if (pol->flags & MPOL_F_RELATIVE_NODES)
			mpol_relative_nodemask(&nsc->mask2, nodes, &nsc->mask1);
		else
			nodes_and(nsc->mask2, *nodes, nsc->mask1);

		if (mpol_store_user_nodemask(pol))
			pol->w.user_nodemask = *nodes;
		else
			pol->w.cpuset_mems_allowed =
						cpuset_current_mems_allowed;
	}

	if (nodes)
		ret = mpol_ops[pol->mode].create(pol, &nsc->mask2);
	else
		ret = mpol_ops[pol->mode].create(pol, NULL);
	return ret;
}

/*
 * This function just creates a new policy, does some check and simple
 * initialization. You must invoke mpol_set_nodemask() to set nodes.
 */
static struct mempolicy *mpol_new(unsigned short mode, unsigned short flags,
				  nodemask_t *nodes)
{
	struct mempolicy *policy;

	pr_debug("setting mode %d flags %d nodes[0] %lx\n",
		 mode, flags, nodes ? nodes_addr(*nodes)[0] : NUMA_NO_NODE);

	if (mode == MPOL_DEFAULT) {
		if (nodes && !nodes_empty(*nodes))
			return ERR_PTR(-EINVAL);
		return NULL;
	}
	VM_BUG_ON(!nodes);

	/*
	 * MPOL_PREFERRED cannot be used with MPOL_F_STATIC_NODES or
	 * MPOL_F_RELATIVE_NODES if the nodemask is empty (local allocation).
	 * All other modes require a valid pointer to a non-empty nodemask.
	 */
	if (mode == MPOL_PREFERRED) {
		if (nodes_empty(*nodes)) {
			if (((flags & MPOL_F_STATIC_NODES) ||
			     (flags & MPOL_F_RELATIVE_NODES)))
				return ERR_PTR(-EINVAL);
		}
	} else if (mode == MPOL_LOCAL) {
		if (!nodes_empty(*nodes))
			return ERR_PTR(-EINVAL);
		mode = MPOL_PREFERRED;
	} else if (nodes_empty(*nodes))
		return ERR_PTR(-EINVAL);
	policy = kmem_cache_alloc(policy_cache, GFP_KERNEL);
	if (!policy)
		return ERR_PTR(-ENOMEM);
	atomic_set(&policy->refcnt, 1);
	policy->mode = mode;
	policy->flags = flags;

	return policy;
}

/* Slow path of a mpol destructor. */
void __mpol_put(struct mempolicy *p)
{
	if (!atomic_dec_and_test(&p->refcnt))
		return;
	kmem_cache_free(policy_cache, p);
}

static void mpol_rebind_default(struct mempolicy *pol, const nodemask_t *nodes,
				enum mpol_rebind_step step)
{
}

/*
 * step:
 * 	MPOL_REBIND_ONCE  - do rebind work at once
 * 	MPOL_REBIND_STEP1 - set all the newly nodes
 * 	MPOL_REBIND_STEP2 - clean all the disallowed nodes
 */
static void mpol_rebind_nodemask(struct mempolicy *pol, const nodemask_t *nodes,
				 enum mpol_rebind_step step)
{
	nodemask_t tmp;

	if (pol->flags & MPOL_F_STATIC_NODES)
		nodes_and(tmp, pol->w.user_nodemask, *nodes);
	else if (pol->flags & MPOL_F_RELATIVE_NODES)
		mpol_relative_nodemask(&tmp, &pol->w.user_nodemask, nodes);
	else {
		/*
		 * if step == 1, we use ->w.cpuset_mems_allowed to cache the
		 * result
		 */
		if (step == MPOL_REBIND_ONCE || step == MPOL_REBIND_STEP1) {
			nodes_remap(tmp, pol->v.nodes,
					pol->w.cpuset_mems_allowed, *nodes);
			pol->w.cpuset_mems_allowed = step ? tmp : *nodes;
		} else if (step == MPOL_REBIND_STEP2) {
			tmp = pol->w.cpuset_mems_allowed;
			pol->w.cpuset_mems_allowed = *nodes;
		} else
			BUG();
	}

	if (nodes_empty(tmp))
		tmp = *nodes;

	if (step == MPOL_REBIND_STEP1)
		nodes_or(pol->v.nodes, pol->v.nodes, tmp);
	else if (step == MPOL_REBIND_ONCE || step == MPOL_REBIND_STEP2)
		pol->v.nodes = tmp;
	else
		BUG();

	if (!node_isset(current->il_next, tmp)) {
		current->il_next = next_node(current->il_next, tmp);
		if (current->il_next >= MAX_NUMNODES)
			current->il_next = first_node(tmp);
		if (current->il_next >= MAX_NUMNODES)
			current->il_next = numa_node_id();
	}
}

static void mpol_rebind_preferred(struct mempolicy *pol,
				  const nodemask_t *nodes,
				  enum mpol_rebind_step step)
{
	nodemask_t tmp;

	if (pol->flags & MPOL_F_STATIC_NODES) {
		int node = first_node(pol->w.user_nodemask);

		if (node_isset(node, *nodes)) {
			pol->v.preferred_node = node;
			pol->flags &= ~MPOL_F_LOCAL;
		} else
			pol->flags |= MPOL_F_LOCAL;
	} else if (pol->flags & MPOL_F_RELATIVE_NODES) {
		mpol_relative_nodemask(&tmp, &pol->w.user_nodemask, nodes);
		pol->v.preferred_node = first_node(tmp);
	} else if (!(pol->flags & MPOL_F_LOCAL)) {
		pol->v.preferred_node = node_remap(pol->v.preferred_node,
						   pol->w.cpuset_mems_allowed,
						   *nodes);
		pol->w.cpuset_mems_allowed = *nodes;
	}
}

/*
 * mpol_rebind_policy - Migrate a policy to a different set of nodes
 *
 * If read-side task has no lock to protect task->mempolicy, write-side
 * task will rebind the task->mempolicy by two step. The first step is
 * setting all the newly nodes, and the second step is cleaning all the
 * disallowed nodes. In this way, we can avoid finding no node to alloc
 * page.
 * If we have a lock to protect task->mempolicy in read-side, we do
 * rebind directly.
 *
 * step:
 * 	MPOL_REBIND_ONCE  - do rebind work at once
 * 	MPOL_REBIND_STEP1 - set all the newly nodes
 * 	MPOL_REBIND_STEP2 - clean all the disallowed nodes
 */
static void mpol_rebind_policy(struct mempolicy *pol, const nodemask_t *newmask,
				enum mpol_rebind_step step)
{
	if (!pol)
		return;
	if (!mpol_store_user_nodemask(pol) && step == MPOL_REBIND_ONCE &&
	    nodes_equal(pol->w.cpuset_mems_allowed, *newmask))
		return;

	if (step == MPOL_REBIND_STEP1 && (pol->flags & MPOL_F_REBINDING))
		return;

	if (step == MPOL_REBIND_STEP2 && !(pol->flags & MPOL_F_REBINDING))
		BUG();

	if (step == MPOL_REBIND_STEP1)
		pol->flags |= MPOL_F_REBINDING;
	else if (step == MPOL_REBIND_STEP2)
		pol->flags &= ~MPOL_F_REBINDING;
	else if (step >= MPOL_REBIND_NSTEP)
		BUG();

	mpol_ops[pol->mode].rebind(pol, newmask, step);
}

/*
 * Wrapper for mpol_rebind_policy() that just requires task
 * pointer, and updates task mempolicy.
 *
 * Called with task's alloc_lock held.
 */

void mpol_rebind_task(struct task_struct *tsk, const nodemask_t *new,
			enum mpol_rebind_step step)
{
	mpol_rebind_policy(tsk->mempolicy, new, step);
}

/*
 * Rebind each vma in mm to new nodemask.
 *
 * Call holding a reference to mm.  Takes mm->mmap_sem during call.
 */

void mpol_rebind_mm(struct mm_struct *mm, nodemask_t *new)
{
	struct vm_area_struct *vma;

	down_write(&mm->mmap_sem);
	for (vma = mm->mmap; vma; vma = vma->vm_next)
		mpol_rebind_policy(vma->vm_policy, new, MPOL_REBIND_ONCE);
	up_write(&mm->mmap_sem);
}

static const struct mempolicy_operations mpol_ops[MPOL_MAX] = {
	[MPOL_DEFAULT] = {
		.rebind = mpol_rebind_default,
	},
	[MPOL_INTERLEAVE] = {
		.create = mpol_new_interleave,
		.rebind = mpol_rebind_nodemask,
	},
	[MPOL_PREFERRED] = {
		.create = mpol_new_preferred,
		.rebind = mpol_rebind_preferred,
	},
	[MPOL_BIND] = {
		.create = mpol_new_bind,
		.rebind = mpol_rebind_nodemask,
	},
};

static void migrate_page_add(struct page *page, struct list_head *pagelist,
				unsigned long flags);

struct queue_pages {
	struct list_head *pagelist;
	unsigned long flags;
	nodemask_t *nmask;
	struct vm_area_struct *prev;
};

/*
 * Scan through pages checking if pages follow certain conditions,
 * and move them to the pagelist if they do.
 */
static int queue_pages_pte_range(pmd_t *pmd, unsigned long addr,
			unsigned long end, struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	struct page *page;
	struct queue_pages *qp = walk->private;
	unsigned long flags = qp->flags;
	int nid;
	pte_t *pte;
	spinlock_t *ptl;

	split_huge_page_pmd(vma, addr, pmd);
	if (pmd_trans_unstable(pmd))
		return 0;

	pte = pte_offset_map_lock(walk->mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		if (!pte_present(*pte))
			continue;
		page = vm_normal_page(vma, addr, *pte);
		if (!page)
			continue;
		/*
		 * vm_normal_page() filters out zero pages, but there might
		 * still be PageReserved pages to skip, perhaps in a VDSO.
		 */
		if (PageReserved(page))
			continue;
		nid = page_to_nid(page);
		if (node_isset(nid, *qp->nmask) == !!(flags & MPOL_MF_INVERT))
			continue;

		if (flags & (MPOL_MF_MOVE | MPOL_MF_MOVE_ALL))
			migrate_page_add(page, qp->pagelist, flags);
	}
	pte_unmap_unlock(pte - 1, ptl);
	cond_resched();
	return 0;
}

static int queue_pages_hugetlb(pte_t *pte, unsigned long hmask,
			       unsigned long addr, unsigned long end,
			       struct mm_walk *walk)
{
#ifdef CONFIG_HUGETLB_PAGE
	struct queue_pages *qp = walk->private;
	unsigned long flags = qp->flags;
	int nid;
	struct page *page;
	spinlock_t *ptl;
	pte_t entry;

	ptl = huge_pte_lock(hstate_vma(walk->vma), walk->mm, pte);
	entry = huge_ptep_get(pte);
	if (!pte_present(entry))
		goto unlock;
	page = pte_page(entry);
	nid = page_to_nid(page);
	if (node_isset(nid, *qp->nmask) == !!(flags & MPOL_MF_INVERT))
		goto unlock;
	/* With MPOL_MF_MOVE, we migrate only unshared hugepage. */
	if (flags & (MPOL_MF_MOVE_ALL) ||
	    (flags & MPOL_MF_MOVE && page_mapcount(page) == 1))
		isolate_huge_page(page, qp->pagelist);
unlock:
	spin_unlock(ptl);
#else
	BUG();
#endif
	return 0;
}

#ifdef CONFIG_NUMA_BALANCING
/*
 * This is used to mark a range of virtual addresses to be inaccessible.
 * These are later cleared by a NUMA hinting fault. Depending on these
 * faults, pages may be migrated for better NUMA placement.
 *
 * This is assuming that NUMA faults are handled using PROT_NONE. If
 * an architecture makes a different choice, it will need further
 * changes to the core.
 */
unsigned long change_prot_numa(struct vm_area_struct *vma,
			unsigned long addr, unsigned long end)
{
	int nr_updated;

	nr_updated = change_protection(vma, addr, end, PAGE_NONE, 0, 1);
	if (nr_updated)
		count_vm_numa_events(NUMA_PTE_UPDATES, nr_updated);

	return nr_updated;
}
#else
static unsigned long change_prot_numa(struct vm_area_struct *vma,
			unsigned long addr, unsigned long end)
{
	return 0;
}
#endif /* CONFIG_NUMA_BALANCING */

static int queue_pages_test_walk(unsigned long start, unsigned long end,
				struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	struct queue_pages *qp = walk->private;
	unsigned long endvma = vma->vm_end;
	unsigned long flags = qp->flags;

	if (vma->vm_flags & VM_PFNMAP)
		return 1;

	if (endvma > end)
		endvma = end;
	if (vma->vm_start > start)
		start = vma->vm_start;

	if (!(flags & MPOL_MF_DISCONTIG_OK)) {
		if (!vma->vm_next && vma->vm_end < end)
			return -EFAULT;
		if (qp->prev && qp->prev->vm_end < vma->vm_start)
			return -EFAULT;
	}

	qp->prev = vma;

	if (vma->vm_flags & VM_PFNMAP)
		return 1;

	if (flags & MPOL_MF_LAZY) {
		/* Similar to task_numa_work, skip inaccessible VMAs */
		if (vma->vm_flags & (VM_READ | VM_EXEC | VM_WRITE))
			change_prot_numa(vma, start, endvma);
		return 1;
	}

	if ((flags & MPOL_MF_STRICT) ||
	    ((flags & (MPOL_MF_MOVE | MPOL_MF_MOVE_ALL)) &&
	     vma_migratable(vma)))
		/* queue pages from current vma */
		return 0;
	return 1;
}

/*
 * Walk through page tables and collect pages to be migrated.
 *
 * If pages found in a given range are on a set of nodes (determined by
 * @nodes and @flags,) it's isolated and queued to the pagelist which is
 * passed via @private.)
 */
static int
queue_pages_range(struct mm_struct *mm, unsigned long start, unsigned long end,
		nodemask_t *nodes, unsigned long flags,
		struct list_head *pagelist)
{
	struct queue_pages qp = {
		.pagelist = pagelist,
		.flags = flags,
		.nmask = nodes,
		.prev = NULL,
	};
	struct mm_walk queue_pages_walk = {
		.hugetlb_entry = queue_pages_hugetlb,
		.pmd_entry = queue_pages_pte_range,
		.test_walk = queue_pages_test_walk,
		.mm = mm,
		.private = &qp,
	};

	return walk_page_range(start, end, &queue_pages_walk);
}

/*
 * Apply policy to a single VMA
 * This must be called with the mmap_sem held for writing.
 */
static int vma_replace_policy(struct vm_area_struct *vma,
						struct mempolicy *pol)
{
	int err;
	struct mempolicy *old;
	struct mempolicy *new;

	pr_debug("vma %lx-%lx/%lx vm_ops %p vm_file %p set_policy %p\n",
		 vma->vm_start, vma->vm_end, vma->vm_pgoff,
		 vma->vm_ops, vma->vm_file,
		 vma->vm_ops ? vma->vm_ops->set_policy : NULL);

	new = mpol_dup(pol);
	if (IS_ERR(new))
		return PTR_ERR(new);

	if (vma->vm_ops && vma->vm_ops->set_policy) {
		err = vma->vm_ops->set_policy(vma, new);
		if (err)
			goto err_out;
	}

	old = vma->vm_policy;
	vma->vm_policy = new; /* protected by mmap_sem */
	mpol_put(old);

	return 0;
 err_out:
	mpol_put(new);
	return err;
}

/* Step 2: apply policy to a range and do splits. */
static int mbind_range(struct mm_struct *mm, unsigned long start,
		       unsigned long end, struct mempolicy *new_pol)
{
	struct vm_area_struct *next;
	struct vm_area_struct *prev;
	struct vm_area_struct *vma;
	int err = 0;
	pgoff_t pgoff;
	unsigned long vmstart;
	unsigned long vmend;

	vma = find_vma(mm, start);
	if (!vma || vma->vm_start > start)
		return -EFAULT;

	prev = vma->vm_prev;
	if (start > vma->vm_start)
		prev = vma;

	for (; vma && vma->vm_start < end; prev = vma, vma = next) {
		next = vma->vm_next;
		vmstart = max(start, vma->vm_start);
		vmend   = min(end, vma->vm_end);

		if (mpol_equal(vma_policy(vma), new_pol))
			continue;

		pgoff = vma->vm_pgoff +
			((vmstart - vma->vm_start) >> PAGE_SHIFT);
		prev = vma_merge(mm, prev, vmstart, vmend, vma->vm_flags,
				  vma->anon_vma, vma->vm_file, pgoff,
				  new_pol);
		if (prev) {
			vma = prev;
			next = vma->vm_next;
			if (mpol_equal(vma_policy(vma), new_pol))
				continue;
			/* vma_merge() joined vma && vma->next, case 8 */
			goto replace;
		}
		if (vma->vm_start != vmstart) {
			err = split_vma(vma->vm_mm, vma, vmstart, 1);
			if (err)
				goto out;
		}
		if (vma->vm_end != vmend) {
			err = split_vma(vma->vm_mm, vma, vmend, 0);
			if (err)
				goto out;
		}
 replace:
		err = vma_replace_policy(vma, new_pol);
		if (err)
			goto out;
	}

 out:
	return err;
}

/* Set the process memory policy */
static long do_set_mempolicy(unsigned short mode, unsigned short flags,
			     nodemask_t *nodes)
{
	struct mempolicy *new, *old;
	NODEMASK_SCRATCH(scratch);
	int ret;

	if (!scratch)
		return -ENOMEM;

	new = mpol_new(mode, flags, nodes);
	if (IS_ERR(new)) {
		ret = PTR_ERR(new);
		goto out;
	}

	task_lock(current);
	ret = mpol_set_nodemask(new, nodes, scratch);
	if (ret) {
		task_unlock(current);
		mpol_put(new);
		goto out;
	}
	old = current->mempolicy;
	current->mempolicy = new;
	if (new && new->mode == MPOL_INTERLEAVE &&
	    nodes_weight(new->v.nodes))
		current->il_next = first_node(new->v.nodes);
	task_unlock(current);
	mpol_put(old);
	ret = 0;
out:
	NODEMASK_SCRATCH_FREE(scratch);
	return ret;
}

/*
 * Return nodemask for policy for get_mempolicy() query
 *
 * Called with task's alloc_lock held
 */
static void get_policy_nodemask(struct mempolicy *p, nodemask_t *nodes)
{
	nodes_clear(*nodes);
	if (p == &default_policy)
		return;

	switch (p->mode) {
	case MPOL_BIND:
		/* Fall through */
	case MPOL_INTERLEAVE:
		*nodes = p->v.nodes;
		break;
	case MPOL_PREFERRED:
		if (!(p->flags & MPOL_F_LOCAL))
			node_set(p->v.preferred_node, *nodes);
		/* else return empty node mask for local allocation */
		break;
	default:
		BUG();
	}
}

static int lookup_node(struct mm_struct *mm, unsigned long addr)
{
	struct page *p;
	int err;

	err = get_user_pages(current, mm, addr & PAGE_MASK, 1, 0, 0, &p, NULL);
	if (err >= 0) {
		err = page_to_nid(p);
		put_page(p);
	}
	return err;
}

/* Retrieve NUMA policy */
static long do_get_mempolicy(int *policy, nodemask_t *nmask,
			     unsigned long addr, unsigned long flags)
{
	int err;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = NULL;
	struct mempolicy *pol = current->mempolicy;

	if (flags &
		~(unsigned long)(MPOL_F_NODE|MPOL_F_ADDR|MPOL_F_MEMS_ALLOWED))
		return -EINVAL;

	if (flags & MPOL_F_MEMS_ALLOWED) {
		if (flags & (MPOL_F_NODE|MPOL_F_ADDR))
			return -EINVAL;
		*policy = 0;	/* just so it's initialized */
		task_lock(current);
		*nmask  = cpuset_current_mems_allowed;
		task_unlock(current);
		return 0;
	}

	if (flags & MPOL_F_ADDR) {
		/*
		 * Do NOT fall back to task policy if the
		 * vma/shared policy at addr is NULL.  We
		 * want to return MPOL_DEFAULT in this case.
		 */
		down_read(&mm->mmap_sem);
		vma = find_vma_intersection(mm, addr, addr+1);
		if (!vma) {
			up_read(&mm->mmap_sem);
			return -EFAULT;
		}
		if (vma->vm_ops && vma->vm_ops->get_policy)
			pol = vma->vm_ops->get_policy(vma, addr);
		else
			pol = vma->vm_policy;
	} else if (addr)
		return -EINVAL;

	if (!pol)
		pol = &default_policy;	/* indicates default behavior */

	if (flags & MPOL_F_NODE) {
		if (flags & MPOL_F_ADDR) {
			err = lookup_node(mm, addr);
			if (err < 0)
				goto out;
			*policy = err;
		} else if (pol == current->mempolicy &&
				pol->mode == MPOL_INTERLEAVE) {
			*policy = current->il_next;
		} else {
			err = -EINVAL;
			goto out;
		}
	} else {
		*policy = pol == &default_policy ? MPOL_DEFAULT :
						pol->mode;
		/*
		 * Internal mempolicy flags must be masked off before exposing
		 * the policy to userspace.
		 */
		*policy |= (pol->flags & MPOL_MODE_FLAGS);
	}

	err = 0;
	if (nmask) {
		if (mpol_store_user_nodemask(pol)) {
			*nmask = pol->w.user_nodemask;
		} else {
			task_lock(current);
			get_policy_nodemask(pol, nmask);
			task_unlock(current);
		}
	}

 out:
	mpol_cond_put(pol);
	if (vma)
		up_read(&current->mm->mmap_sem);
	return err;
}

#ifdef CONFIG_MIGRATION
/*
 * page migration
 */
static void migrate_page_add(struct page *page, struct list_head *pagelist,
				unsigned long flags)
{
	/*
	 * Avoid migrating a page that is shared with others.
	 */
	if ((flags & MPOL_MF_MOVE_ALL) || page_mapcount(page) == 1) {
		if (!isolate_lru_page(page)) {
			list_add_tail(&page->lru, pagelist);
			inc_zone_page_state(page, NR_ISOLATED_ANON +
					    page_is_file_cache(page));
		}
	}
}

static struct page *new_node_page(struct page *page, unsigned long node, int **x)
{
	if (PageHuge(page))
		return alloc_huge_page_node(page_hstate(compound_head(page)),
					node);
	else
		return alloc_pages_exact_node(node, GFP_HIGHUSER_MOVABLE |
						    __GFP_THISNODE, 0);
}

/*
 * Migrate pages from one node to a target node.
 * Returns error or the number of pages not migrated.
 */
static int migrate_to_node(struct mm_struct *mm, int source, int dest,
			   int flags)
{
	nodemask_t nmask;
	LIST_HEAD(pagelist);
	int err = 0;

	nodes_clear(nmask);
	node_set(source, nmask);

	/*
	 * This does not "check" the range but isolates all pages that
	 * need migration.  Between passing in the full user address
	 * space range and MPOL_MF_DISCONTIG_OK, this call can not fail.
	 */
	VM_BUG_ON(!(flags & (MPOL_MF_MOVE | MPOL_MF_MOVE_ALL)));
	queue_pages_range(mm, mm->mmap->vm_start, mm->task_size, &nmask,
			flags | MPOL_MF_DISCONTIG_OK, &pagelist);

	if (!list_empty(&pagelist)) {
		err = migrate_pages(&pagelist, new_node_page, NULL, dest,
					MIGRATE_SYNC, MR_SYSCALL);
		if (err)
			putback_movable_pages(&pagelist);
	}

	return err;
}

/*
 * Move pages between the two nodesets so as to preserve the physical
 * layout as much as possible.
 *
 * Returns the number of page that could not be moved.
 */
int do_migrate_pages(struct mm_struct *mm, const nodemask_t *from,
		     const nodemask_t *to, int flags)
{
	int busy = 0;
	int err;
	nodemask_t tmp;

	err = migrate_prep();
	if (err)
		return err;

	down_read(&mm->mmap_sem);

	/*
	 * Find a 'source' bit set in 'tmp' whose corresponding 'dest'
	 * bit in 'to' is not also set in 'tmp'.  Clear the found 'source'
	 * bit in 'tmp', and return that <source, dest> pair for migration.
	 * The pair of nodemasks 'to' and 'from' define the map.
	 *
	 * If no pair of bits is found that way, fallback to picking some
	 * pair of 'source' and 'dest' bits that are not the same.  If the
	 * 'source' and 'dest' bits are the same, this represents a node
	 * that will be migrating to itself, so no pages need move.
	 *
	 * If no bits are left in 'tmp', or if all remaining bits left
	 * in 'tmp' correspond to the same bit in 'to', return false
	 * (nothing left to migrate).
	 *
	 * This lets us pick a pair of nodes to migrate between, such that
	 * if possible the dest node is not already occupied by some other
	 * source node, minimizing the risk of overloading the memory on a
	 * node that would happen if we migrated incoming memory to a node
	 * before migrating outgoing memory source that same node.
	 *
	 * A single scan of tmp is sufficient.  As we go, we remember the
	 * most recent <s, d> pair that moved (s != d).  If we find a pair
	 * that not only moved, but what's better, moved to an empty slot
	 * (d is not set in tmp), then we break out then, with that pair.
	 * Otherwise when we finish scanning from_tmp, we at least have the
	 * most recent <s, d> pair that moved.  If we get all the way through
	 * the scan of tmp without finding any node that moved, much less
	 * moved to an empty node, then there is nothing left worth migrating.
	 */

	tmp = *from;
	while (!nodes_empty(tmp)) {
		int s,d;
		int source = NUMA_NO_NODE;
		int dest = 0;

		for_each_node_mask(s, tmp) {

			/*
			 * do_migrate_pages() tries to maintain the relative
			 * node relationship of the pages established between
			 * threads and memory areas.
                         *
			 * However if the number of source nodes is not equal to
			 * the number of destination nodes we can not preserve
			 * this node relative relationship.  In that case, skip
			 * copying memory from a node that is in the destination
			 * mask.
			 *
			 * Example: [2,3,4] -> [3,4,5] moves everything.
			 *          [0-7] - > [3,4,5] moves only 0,1,2,6,7.
			 */

			if ((nodes_weight(*from) != nodes_weight(*to)) &&
						(node_isset(s, *to)))
				continue;

			d = node_remap(s, *from, *to);
			if (s == d)
				continue;

			source = s;	/* Node moved. Memorize */
			dest = d;

			/* dest not in remaining from nodes? */
			if (!node_isset(dest, tmp))
				break;
		}
		if (source == NUMA_NO_NODE)
			break;

		node_clear(source, tmp);
		err = migrate_to_node(mm, source, dest, flags);
		if (err > 0)
			busy += err;
		if (err < 0)
			break;
	}
	up_read(&mm->mmap_sem);
	if (err < 0)
		return err;
	return busy;

}

/*
 * Allocate a new page for page migration based on vma policy.
 * Start by assuming the page is mapped by the same vma as contains @start.
 * Search forward from there, if not.  N.B., this assumes that the
 * list of pages handed to migrate_pages()--which is how we get here--
 * is in virtual address order.
 */
static struct page *new_page(struct page *page, unsigned long start, int **x)
{
	struct vm_area_struct *vma;
	unsigned long uninitialized_var(address);

	vma = find_vma(current->mm, start);
	while (vma) {
		address = page_address_in_vma(page, vma);
		if (address != -EFAULT)
			break;
		vma = vma->vm_next;
	}

	if (PageHuge(page)) {
		BUG_ON(!vma);
		return alloc_huge_page_noerr(vma, address, 1);
	}
	/*
	 * if !vma, alloc_page_vma() will use task or system default policy
	 */
	return alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
}
#else

static void migrate_page_add(struct page *page, struct list_head *pagelist,
				unsigned long flags)
{
}

int do_migrate_pages(struct mm_struct *mm, const nodemask_t *from,
		     const nodemask_t *to, int flags)
{
	return -ENOSYS;
}

static struct page *new_page(struct page *page, unsigned long start, int **x)
{
	return NULL;
}
#endif

static long do_mbind(unsigned long start, unsigned long len,
		     unsigned short mode, unsigned short mode_flags,
		     nodemask_t *nmask, unsigned long flags)
{
	struct mm_struct *mm = current->mm;
	struct mempolicy *new;
	unsigned long end;
	int err;
	LIST_HEAD(pagelist);

	if (flags & ~(unsigned long)MPOL_MF_VALID)
		return -EINVAL;
	if ((flags & MPOL_MF_MOVE_ALL) && !capable(CAP_SYS_NICE))
		return -EPERM;

	if (start & ~PAGE_MASK)
		return -EINVAL;

	if (mode == MPOL_DEFAULT)
		flags &= ~MPOL_MF_STRICT;

	len = (len + PAGE_SIZE - 1) & PAGE_MASK;
	end = start + len;

	if (end < start)
		return -EINVAL;
	if (end == start)
		return 0;

	new = mpol_new(mode, mode_flags, nmask);
	if (IS_ERR(new))
		return PTR_ERR(new);

	if (flags & MPOL_MF_LAZY)
		new->flags |= MPOL_F_MOF;

	/*
	 * If we are using the default policy then operation
	 * on discontinuous address spaces is okay after all
	 */
	if (!new)
		flags |= MPOL_MF_DISCONTIG_OK;

	pr_debug("mbind %lx-%lx mode:%d flags:%d nodes:%lx\n",
		 start, start + len, mode, mode_flags,
		 nmask ? nodes_addr(*nmask)[0] : NUMA_NO_NODE);

	if (flags & (MPOL_MF_MOVE | MPOL_MF_MOVE_ALL)) {

		err = migrate_prep();
		if (err)
			goto mpol_out;
	}
	{
		NODEMASK_SCRATCH(scratch);
		if (scratch) {
			down_write(&mm->mmap_sem);
			task_lock(current);
			err = mpol_set_nodemask(new, nmask, scratch);
			task_unlock(current);
			if (err)
				up_write(&mm->mmap_sem);
		} else
			err = -ENOMEM;
		NODEMASK_SCRATCH_FREE(scratch);
	}
	if (err)
		goto mpol_out;

	err = queue_pages_range(mm, start, end, nmask,
			  flags | MPOL_MF_INVERT, &pagelist);
	if (!err)
		err = mbind_range(mm, start, end, new);

	if (!err) {
		int nr_failed = 0;

		if (!list_empty(&pagelist)) {
			WARN_ON_ONCE(flags & MPOL_MF_LAZY);
			nr_failed = migrate_pages(&pagelist, new_page, NULL,
				start, MIGRATE_SYNC, MR_MEMPOLICY_MBIND);
			if (nr_failed)
				putback_movable_pages(&pagelist);
		}

		if (nr_failed && (flags & MPOL_MF_STRICT))
			err = -EIO;
	} else
		putback_movable_pages(&pagelist);

	up_write(&mm->mmap_sem);
 mpol_out:
	mpol_put(new);
	return err;
}

/*
 * User space interface with variable sized bitmaps for nodelists.
 */

/* Copy a node mask from user space. */
static int get_nodes(nodemask_t *nodes, const unsigned long __user *nmask,
		     unsigned long maxnode)
{
	unsigned long k;
	unsigned long nlongs;
	unsigned long endmask;

	--maxnode;
	nodes_clear(*nodes);
	if (maxnode == 0 || !nmask)
		return 0;
	if (maxnode > PAGE_SIZE*BITS_PER_BYTE)
		return -EINVAL;

	nlongs = BITS_TO_LONGS(maxnode);
	if ((maxnode % BITS_PER_LONG) == 0)
		endmask = ~0UL;
	else
		endmask = (1UL << (maxnode % BITS_PER_LONG)) - 1;

	/* When the user specified more nodes than supported just check
	   if the non supported part is all zero. */
	if (nlongs > BITS_TO_LONGS(MAX_NUMNODES)) {
		if (nlongs > PAGE_SIZE/sizeof(long))
			return -EINVAL;
		for (k = BITS_TO_LONGS(MAX_NUMNODES); k < nlongs; k++) {
			unsigned long t;
			if (get_user(t, nmask + k))
				return -EFAULT;
			if (k == nlongs - 1) {
				if (t & endmask)
					return -EINVAL;
			} else if (t)
				return -EINVAL;
		}
		nlongs = BITS_TO_LONGS(MAX_NUMNODES);
		endmask = ~0UL;
	}

	if (copy_from_user(nodes_addr(*nodes), nmask, nlongs*sizeof(unsigned long)))
		return -EFAULT;
	nodes_addr(*nodes)[nlongs-1] &= endmask;
	return 0;
}

/* Copy a kernel node mask to user space */
static int copy_nodes_to_user(unsigned long __user *mask, unsigned long maxnode,
			      nodemask_t *nodes)
{
	unsigned long copy = ALIGN(maxnode-1, 64) / 8;
	const int nbytes = BITS_TO_LONGS(MAX_NUMNODES) * sizeof(long);

	if (copy > nbytes) {
		if (copy > PAGE_SIZE)
			return -EINVAL;
		if (clear_user((char __user *)mask + nbytes, copy - nbytes))
			return -EFAULT;
		copy = nbytes;
	}
	return copy_to_user(mask, nodes_addr(*nodes), copy) ? -EFAULT : 0;
}

SYSCALL_DEFINE6(mbind, unsigned long, start, unsigned long, len,
		unsigned long, mode, const unsigned long __user *, nmask,
		unsigned long, maxnode, unsigned, flags)
{
	nodemask_t nodes;
	int err;
	unsigned short mode_flags;

	mode_flags = mode & MPOL_MODE_FLAGS;
	mode &= ~MPOL_MODE_FLAGS;
	if (mode >= MPOL_MAX)
		return -EINVAL;
	if ((mode_flags & MPOL_F_STATIC_NODES) &&
	    (mode_flags & MPOL_F_RELATIVE_NODES))
		return -EINVAL;
	err = get_nodes(&nodes, nmask, maxnode);
	if (err)
		return err;
	return do_mbind(start, len, mode, mode_flags, &nodes, flags);
}

/* Set the process memory policy */
SYSCALL_DEFINE3(set_mempolicy, int, mode, const unsigned long __user *, nmask,
		unsigned long, maxnode)
{
	int err;
	nodemask_t nodes;
	unsigned short flags;

	flags = mode & MPOL_MODE_FLAGS;
	mode &= ~MPOL_MODE_FLAGS;
	if ((unsigned int)mode >= MPOL_MAX)
		return -EINVAL;
	if ((flags & MPOL_F_STATIC_NODES) && (flags & MPOL_F_RELATIVE_NODES))
		return -EINVAL;
	err = get_nodes(&nodes, nmask, maxnode);
	if (err)
		return err;
	return do_set_mempolicy(mode, flags, &nodes);
}

SYSCALL_DEFINE4(migrate_pages, pid_t, pid, unsigned long, maxnode,
		const unsigned long __user *, old_nodes,
		const unsigned long __user *, new_nodes)
{
	const struct cred *cred = current_cred(), *tcred;
	struct mm_struct *mm = NULL;
	struct task_struct *task;
	nodemask_t task_nodes;
	int err;
	nodemask_t *old;
	nodemask_t *new;
	NODEMASK_SCRATCH(scratch);

	if (!scratch)
		return -ENOMEM;

	old = &scratch->mask1;
	new = &scratch->mask2;

	err = get_nodes(old, old_nodes, maxnode);
	if (err)
		goto out;

	err = get_nodes(new, new_nodes, maxnode);
	if (err)
		goto out;

	/* Find the mm_struct */
	rcu_read_lock();
	task = pid ? find_task_by_vpid(pid) : current;
	if (!task) {
		rcu_read_unlock();
		err = -ESRCH;
		goto out;
	}
	get_task_struct(task);

	err = -EINVAL;

	/*
	 * Check if this process has the right to modify the specified
	 * process. The right exists if the process has administrative
	 * capabilities, superuser privileges or the same
	 * userid as the target process.
	 */
	tcred = __task_cred(task);
	if (!uid_eq(cred->euid, tcred->suid) && !uid_eq(cred->euid, tcred->uid) &&
	    !uid_eq(cred->uid,  tcred->suid) && !uid_eq(cred->uid,  tcred->uid) &&
	    !capable(CAP_SYS_NICE)) {
		rcu_read_unlock();
		err = -EPERM;
		goto out_put;
	}
	rcu_read_unlock();

	task_nodes = cpuset_mems_allowed(task);
	/* Is the user allowed to access the target nodes? */
	if (!nodes_subset(*new, task_nodes) && !capable(CAP_SYS_NICE)) {
		err = -EPERM;
		goto out_put;
	}

	if (!nodes_subset(*new, node_states[N_MEMORY])) {
		err = -EINVAL;
		goto out_put;
	}

	err = security_task_movememory(task);
	if (err)
		goto out_put;

	mm = get_task_mm(task);
	put_task_struct(task);

	if (!mm) {
		err = -EINVAL;
		goto out;
	}

	err = do_migrate_pages(mm, old, new,
		capable(CAP_SYS_NICE) ? MPOL_MF_MOVE_ALL : MPOL_MF_MOVE);

	mmput(mm);
out:
	NODEMASK_SCRATCH_FREE(scratch);

	return err;

out_put:
	put_task_struct(task);
	goto out;

}


/* Retrieve NUMA policy */
SYSCALL_DEFINE5(get_mempolicy, int __user *, policy,
		unsigned long __user *, nmask, unsigned long, maxnode,
		unsigned long, addr, unsigned long, flags)
{
	int err;
	int uninitialized_var(pval);
	nodemask_t nodes;

	if (nmask != NULL && maxnode < MAX_NUMNODES)
		return -EINVAL;

	err = do_get_mempolicy(&pval, &nodes, addr, flags);

	if (err)
		return err;

	if (policy && put_user(pval, policy))
		return -EFAULT;

	if (nmask)
		err = copy_nodes_to_user(nmask, maxnode, &nodes);

	return err;
}

#ifdef CONFIG_COMPAT

COMPAT_SYSCALL_DEFINE5(get_mempolicy, int __user *, policy,
		       compat_ulong_t __user *, nmask,
		       compat_ulong_t, maxnode,
		       compat_ulong_t, addr, compat_ulong_t, flags)
{
	long err;
	unsigned long __user *nm = NULL;
	unsigned long nr_bits, alloc_size;
	DECLARE_BITMAP(bm, MAX_NUMNODES);

	nr_bits = min_t(unsigned long, maxnode-1, MAX_NUMNODES);
	alloc_size = ALIGN(nr_bits, BITS_PER_LONG) / 8;

	if (nmask)
		nm = compat_alloc_user_space(alloc_size);

	err = sys_get_mempolicy(policy, nm, nr_bits+1, addr, flags);

	if (!err && nmask) {
		unsigned long copy_size;
		copy_size = min_t(unsigned long, sizeof(bm), alloc_size);
		err = copy_from_user(bm, nm, copy_size);
		/* ensure entire bitmap is zeroed */
		err |= clear_user(nmask, ALIGN(maxnode-1, 8) / 8);
		err |= compat_put_bitmap(nmask, bm, nr_bits);
	}

	return err;
}

COMPAT_SYSCALL_DEFINE3(set_mempolicy, int, mode, compat_ulong_t __user *, nmask,
		       compat_ulong_t, maxnode)
{
	unsigned long __user *nm = NULL;
	unsigned long nr_bits, alloc_size;
	DECLARE_BITMAP(bm, MAX_NUMNODES);

	nr_bits = min_t(unsigned long, maxnode-1, MAX_NUMNODES);
	alloc_size = ALIGN(nr_bits, BITS_PER_LONG) / 8;

	if (nmask) {
		if (compat_get_bitmap(bm, nmask, nr_bits))
			return -EFAULT;
		nm = compat_alloc_user_space(alloc_size);
		if (copy_to_user(nm, bm, alloc_size))
			return -EFAULT;
	}

	return sys_set_mempolicy(mode, nm, nr_bits+1);
}

COMPAT_SYSCALL_DEFINE6(mbind, compat_ulong_t, start, compat_ulong_t, len,
		       compat_ulong_t, mode, compat_ulong_t __user *, nmask,
		       compat_ulong_t, maxnode, compat_ulong_t, flags)
{
	unsigned long __user *nm = NULL;
	unsigned long nr_bits, alloc_size;
	nodemask_t bm;

	nr_bits = min_t(unsigned long, maxnode-1, MAX_NUMNODES);
	alloc_size = ALIGN(nr_bits, BITS_PER_LONG) / 8;

	if (nmask) {
		if (compat_get_bitmap(nodes_addr(bm), nmask, nr_bits))
			return -EFAULT;
		nm = compat_alloc_user_space(alloc_size);
		if (copy_to_user(nm, nodes_addr(bm), alloc_size))
			return -EFAULT;
	}

	return sys_mbind(start, len, mode, nm, nr_bits+1, flags);
}

#endif

struct mempolicy *__get_vma_policy(struct vm_area_struct *vma,
						unsigned long addr)
{
	struct mempolicy *pol = NULL;

	if (vma) {
		if (vma->vm_ops && vma->vm_ops->get_policy) {
			pol = vma->vm_ops->get_policy(vma, addr);
		} else if (vma->vm_policy) {
			pol = vma->vm_policy;

			/*
			 * shmem_alloc_page() passes MPOL_F_SHARED policy with
			 * a pseudo vma whose vma->vm_ops=NULL. Take a reference
			 * count on these policies which will be dropped by
			 * mpol_cond_put() later
			 */
			if (mpol_needs_cond_ref(pol))
				mpol_get(pol);
		}
	}

	return pol;
}

/*
 * get_vma_policy(@vma, @addr)
 * @vma: virtual memory area whose policy is sought
 * @addr: address in @vma for shared policy lookup
 *
 * Returns effective policy for a VMA at specified address.
 * Falls back to current->mempolicy or system default policy, as necessary.
 * Shared policies [those marked as MPOL_F_SHARED] require an extra reference
 * count--added by the get_policy() vm_op, as appropriate--to protect against
 * freeing by another task.  It is the caller's responsibility to free the
 * extra reference for shared policies.
 */
static struct mempolicy *get_vma_policy(struct vm_area_struct *vma,
						unsigned long addr)
{
	struct mempolicy *pol = __get_vma_policy(vma, addr);

	if (!pol)
		pol = get_task_policy(current);

	return pol;
}

bool vma_policy_mof(struct vm_area_struct *vma)
{
	struct mempolicy *pol;

	if (vma->vm_ops && vma->vm_ops->get_policy) {
		bool ret = false;

		pol = vma->vm_ops->get_policy(vma, vma->vm_start);
		if (pol && (pol->flags & MPOL_F_MOF))
			ret = true;
		mpol_cond_put(pol);

		return ret;
	}

	pol = vma->vm_policy;
	if (!pol)
		pol = get_task_policy(current);

	return pol->flags & MPOL_F_MOF;
}

static int apply_policy_zone(struct mempolicy *policy, enum zone_type zone)
{
	enum zone_type dynamic_policy_zone = policy_zone;

	BUG_ON(dynamic_policy_zone == ZONE_MOVABLE);

	/*
	 * if policy->v.nodes has movable memory only,
	 * we apply policy when gfp_zone(gfp) = ZONE_MOVABLE only.
	 *
	 * policy->v.nodes is intersect with node_states[N_MEMORY].
	 * so if the following test faile, it implies
	 * policy->v.nodes has movable memory only.
	 */
	if (!nodes_intersects(policy->v.nodes, node_states[N_HIGH_MEMORY]))
		dynamic_policy_zone = ZONE_MOVABLE;

	return zone >= dynamic_policy_zone;
}

/*
 * Return a nodemask representing a mempolicy for filtering nodes for
 * page allocation
 */
static nodemask_t *policy_nodemask(gfp_t gfp, struct mempolicy *policy)
{
	/* Lower zones don't get a nodemask applied for MPOL_BIND */
	if (unlikely(policy->mode == MPOL_BIND) &&
			apply_policy_zone(policy, gfp_zone(gfp)) &&
			cpuset_nodemask_valid_mems_allowed(&policy->v.nodes))
		return &policy->v.nodes;

	return NULL;
}

/* Return a zonelist indicated by gfp for node representing a mempolicy */
static struct zonelist *policy_zonelist(gfp_t gfp, struct mempolicy *policy,
	int nd)
{
	switch (policy->mode) {
	case MPOL_PREFERRED:
		if (!(policy->flags & MPOL_F_LOCAL))
			nd = policy->v.preferred_node;
		break;
	case MPOL_BIND:
		/*
		 * Normally, MPOL_BIND allocations are node-local within the
		 * allowed nodemask.  However, if __GFP_THISNODE is set and the
		 * current node isn't part of the mask, we use the zonelist for
		 * the first node in the mask instead.
		 */
		if (unlikely(gfp & __GFP_THISNODE) &&
				unlikely(!node_isset(nd, policy->v.nodes)))
			nd = first_node(policy->v.nodes);
		break;
	default:
		BUG();
	}
	return node_zonelist(nd, gfp);
}

/* Do dynamic interleaving for a process */
static unsigned interleave_nodes(struct mempolicy *policy)
{
	unsigned nid, next;
	struct task_struct *me = current;

	nid = me->il_next;
	next = next_node(nid, policy->v.nodes);
	if (next >= MAX_NUMNODES)
		next = first_node(policy->v.nodes);
	if (next < MAX_NUMNODES)
		me->il_next = next;
	return nid;
}

/*
 * Depending on the memory policy provide a node from which to allocate the
 * next slab entry.
 */
unsigned int mempolicy_slab_node(void)
{
	struct mempolicy *policy;
	int node = numa_mem_id();

	if (in_interrupt())
		return node;

	policy = current->mempolicy;
	if (!policy || policy->flags & MPOL_F_LOCAL)
		return node;

	switch (policy->mode) {
	case MPOL_PREFERRED:
		/*
		 * handled MPOL_F_LOCAL above
		 */
		return policy->v.preferred_node;

	case MPOL_INTERLEAVE:
		return interleave_nodes(policy);

	case MPOL_BIND: {
		/*
		 * Follow bind policy behavior and start allocation at the
		 * first node.
		 */
		struct zonelist *zonelist;
		struct zone *zone;
		enum zone_type highest_zoneidx = gfp_zone(GFP_KERNEL);
		zonelist = &NODE_DATA(node)->node_zonelists[0];
		(void)first_zones_zonelist(zonelist, highest_zoneidx,
							&policy->v.nodes,
							&zone);
		return zone ? zone->node : node;
	}

	default:
		BUG();
	}
}

/* Do static interleaving for a VMA with known offset. */
static unsigned offset_il_node(struct mempolicy *pol,
		struct vm_area_struct *vma, unsigned long off)
{
	unsigned nnodes = nodes_weight(pol->v.nodes);
	unsigned target;
	int c;
	int nid = NUMA_NO_NODE;

	if (!nnodes)
		return numa_node_id();
	target = (unsigned int)off % nnodes;
	c = 0;
	do {
		nid = next_node(nid, pol->v.nodes);
		c++;
	} while (c <= target);
	return nid;
}

/* Determine a node number for interleave */
static inline unsigned interleave_nid(struct mempolicy *pol,
		 struct vm_area_struct *vma, unsigned long addr, int shift)
{
	if (vma) {
		unsigned long off;

		/*
		 * for small pages, there is no difference between
		 * shift and PAGE_SHIFT, so the bit-shift is safe.
		 * for huge pages, since vm_pgoff is in units of small
		 * pages, we need to shift off the always 0 bits to get
		 * a useful offset.
		 */
		BUG_ON(shift < PAGE_SHIFT);
		off = vma->vm_pgoff >> (shift - PAGE_SHIFT);
		off += (addr - vma->vm_start) >> shift;
		return offset_il_node(pol, vma, off);
	} else
		return interleave_nodes(pol);
}

/*
 * Return the bit number of a random bit set in the nodemask.
 * (returns NUMA_NO_NODE if nodemask is empty)
 */
int node_random(const nodemask_t *maskp)
{
	int w, bit = NUMA_NO_NODE;

	w = nodes_weight(*maskp);
	if (w)
		bit = bitmap_ord_to_pos(maskp->bits,
			get_random_int() % w, MAX_NUMNODES);
	return bit;
}

#ifdef CONFIG_HUGETLBFS
/*
 * huge_zonelist(@vma, @addr, @gfp_flags, @mpol)
 * @vma: virtual memory area whose policy is sought
 * @addr: address in @vma for shared policy lookup and interleave policy
 * @gfp_flags: for requested zone
 * @mpol: pointer to mempolicy pointer for reference counted mempolicy
 * @nodemask: pointer to nodemask pointer for MPOL_BIND nodemask
 *
 * Returns a zonelist suitable for a huge page allocation and a pointer
 * to the struct mempolicy for conditional unref after allocation.
 * If the effective policy is 'BIND, returns a pointer to the mempolicy's
 * @nodemask for filtering the zonelist.
 *
 * Must be protected by read_mems_allowed_begin()
 */
struct zonelist *huge_zonelist(struct vm_area_struct *vma, unsigned long addr,
				gfp_t gfp_flags, struct mempolicy **mpol,
				nodemask_t **nodemask)
{
	struct zonelist *zl;

	*mpol = get_vma_policy(vma, addr);
	*nodemask = NULL;	/* assume !MPOL_BIND */

	if (unlikely((*mpol)->mode == MPOL_INTERLEAVE)) {
		zl = node_zonelist(interleave_nid(*mpol, vma, addr,
				huge_page_shift(hstate_vma(vma))), gfp_flags);
	} else {
		zl = policy_zonelist(gfp_flags, *mpol, numa_node_id());
		if ((*mpol)->mode == MPOL_BIND)
			*nodemask = &(*mpol)->v.nodes;
	}
	return zl;
}

/*
 * init_nodemask_of_mempolicy
 *
 * If the current task's mempolicy is "default" [NULL], return 'false'
 * to indicate default policy.  Otherwise, extract the policy nodemask
 * for 'bind' or 'interleave' policy into the argument nodemask, or
 * initialize the argument nodemask to contain the single node for
 * 'preferred' or 'local' policy and return 'true' to indicate presence
 * of non-default mempolicy.
 *
 * We don't bother with reference counting the mempolicy [mpol_get/put]
 * because the current task is examining it's own mempolicy and a task's
 * mempolicy is only ever changed by the task itself.
 *
 * N.B., it is the caller's responsibility to free a returned nodemask.
 */
bool init_nodemask_of_mempolicy(nodemask_t *mask)
{
	struct mempolicy *mempolicy;
	int nid;

	if (!(mask && current->mempolicy))
		return false;

	task_lock(current);
	mempolicy = current->mempolicy;
	switch (mempolicy->mode) {
	case MPOL_PREFERRED:
		if (mempolicy->flags & MPOL_F_LOCAL)
			nid = numa_node_id();
		else
			nid = mempolicy->v.preferred_node;
		init_nodemask_of_node(mask, nid);
		break;

	case MPOL_BIND:
		/* Fall through */
	case MPOL_INTERLEAVE:
		*mask =  mempolicy->v.nodes;
		break;

	default:
		BUG();
	}
	task_unlock(current);

	return true;
}
#endif

/*
 * mempolicy_nodemask_intersects
 *
 * If tsk's mempolicy is "default" [NULL], return 'true' to indicate default
 * policy.  Otherwise, check for intersection between mask and the policy
 * nodemask for 'bind' or 'interleave' policy.  For 'perferred' or 'local'
 * policy, always return true since it may allocate elsewhere on fallback.
 *
 * Takes task_lock(tsk) to prevent freeing of its mempolicy.
 */
bool mempolicy_nodemask_intersects(struct task_struct *tsk,
					const nodemask_t *mask)
{
	struct mempolicy *mempolicy;
	bool ret = true;

	if (!mask)
		return ret;
	task_lock(tsk);
	mempolicy = tsk->mempolicy;
	if (!mempolicy)
		goto out;

	switch (mempolicy->mode) {
	case MPOL_PREFERRED:
		/*
		 * MPOL_PREFERRED and MPOL_F_LOCAL are only preferred nodes to
		 * allocate from, they may fallback to other nodes when oom.
		 * Thus, it's possible for tsk to have allocated memory from
		 * nodes in mask.
		 */
		break;
	case MPOL_BIND:
	case MPOL_INTERLEAVE:
		ret = nodes_intersects(mempolicy->v.nodes, *mask);
		break;
	default:
		BUG();
	}
out:
	task_unlock(tsk);
	return ret;
}

/* Allocate a page in interleaved policy.
   Own path because it needs to do special accounting. */
static struct page *alloc_page_interleave(gfp_t gfp, unsigned order,
					unsigned nid)
{
	struct zonelist *zl;
	struct page *page;

	zl = node_zonelist(nid, gfp);
	page = __alloc_pages(gfp, order, zl);
	if (page && page_zone(page) == zonelist_zone(&zl->_zonerefs[0]))
		inc_zone_page_state(page, NUMA_INTERLEAVE_HIT);
	return page;
}

/**
 * 	alloc_pages_vma	- Allocate a page for a VMA.
 *
 * 	@gfp:
 *      %GFP_USER    user allocation.
 *      %GFP_KERNEL  kernel allocations,
 *      %GFP_HIGHMEM highmem/user allocations,
 *      %GFP_FS      allocation should not call back into a file system.
 *      %GFP_ATOMIC  don't sleep.
 *
 *	@order:Order of the GFP allocation.
 * 	@vma:  Pointer to VMA or NULL if not available.
 *	@addr: Virtual Address of the allocation. Must be inside the VMA.
 *	@node: Which node to prefer for allocation (modulo policy).
 *	@hugepage: for hugepages try only the preferred node if possible
 *
 * 	This function allocates a page from the kernel page pool and applies
 *	a NUMA policy associated with the VMA or the current process.
 *	When VMA is not NULL caller must hold down_read on the mmap_sem of the
 *	mm_struct of the VMA to prevent it from going away. Should be used for
 *	all allocations for pages that will be mapped into user space. Returns
 *	NULL when no page can be allocated.
 */
struct page *
alloc_pages_vma(gfp_t gfp, int order, struct vm_area_struct *vma,
		unsigned long addr, int node, bool hugepage)
{
	struct mempolicy *pol;
	struct page *page;
	unsigned int cpuset_mems_cookie;
	struct zonelist *zl;
	nodemask_t *nmask;

retry_cpuset:
	pol = get_vma_policy(vma, addr);
	cpuset_mems_cookie = read_mems_allowed_begin();

	if (pol->mode == MPOL_INTERLEAVE) {
		unsigned nid;

		nid = interleave_nid(pol, vma, addr, PAGE_SHIFT + order);
		mpol_cond_put(pol);
		page = alloc_page_interleave(gfp, order, nid);
		goto out;
	}

	if (unlikely(IS_ENABLED(CONFIG_TRANSPARENT_HUGEPAGE) && hugepage)) {
		int hpage_node = node;

		/*
		 * For hugepage allocation and non-interleave policy which
		 * allows the current node (or other explicitly preferred
		 * node) we only try to allocate from the current/preferred
		 * node and don't fall back to other nodes, as the cost of
		 * remote accesses would likely offset THP benefits.
		 *
		 * If the policy is interleave, or does not allow the current
		 * node in its nodemask, we allocate the standard way.
		 */
		if (pol->mode == MPOL_PREFERRED &&
						!(pol->flags & MPOL_F_LOCAL))
			hpage_node = pol->v.preferred_node;

		nmask = policy_nodemask(gfp, pol);
		if (!nmask || node_isset(hpage_node, *nmask)) {
			mpol_cond_put(pol);
			page = alloc_pages_exact_node(hpage_node,
						gfp | __GFP_THISNODE, order);
			goto out;
		}
	}

	nmask = policy_nodemask(gfp, pol);
	zl = policy_zonelist(gfp, pol, node);
	page = __alloc_pages_nodemask(gfp, order, zl, nmask);
	mpol_cond_put(pol);
out:
	if (unlikely(!page && read_mems_allowed_retry(cpuset_mems_cookie)))
		goto retry_cpuset;
	return page;
}

/**
 * 	alloc_pages_current - Allocate pages.
 *
 *	@gfp:
 *		%GFP_USER   user allocation,
 *      	%GFP_KERNEL kernel allocation,
 *      	%GFP_HIGHMEM highmem allocation,
 *      	%GFP_FS     don't call back into a file system.
 *      	%GFP_ATOMIC don't sleep.
 *	@order: Power of two of allocation size in pages. 0 is a single page.
 *
 *	Allocate a page from the kernel page pool.  When not in
 *	interrupt context and apply the current process NUMA policy.
 *	Returns NULL when no page can be allocated.
 *
 *	Don't call cpuset_update_task_memory_state() unless
 *	1) it's ok to take cpuset_sem (can WAIT), and
 *	2) allocating for current task (not interrupt).
 */
struct page *alloc_pages_current(gfp_t gfp, unsigned order)
{
	struct mempolicy *pol = &default_policy;
	struct page *page;
	unsigned int cpuset_mems_cookie;

	if (!in_interrupt() && !(gfp & __GFP_THISNODE))
		pol = get_task_policy(current);

retry_cpuset:
	cpuset_mems_cookie = read_mems_allowed_begin();

	/*
	 * No reference counting needed for current->mempolicy
	 * nor system default_policy
	 */
	if (pol->mode == MPOL_INTERLEAVE)
		page = alloc_page_interleave(gfp, order, interleave_nodes(pol));
	else
		page = __alloc_pages_nodemask(gfp, order,
				policy_zonelist(gfp, pol, numa_node_id()),
				policy_nodemask(gfp, pol));

	if (unlikely(!page && read_mems_allowed_retry(cpuset_mems_cookie)))
		goto retry_cpuset;

	return page;
}
EXPORT_SYMBOL(alloc_pages_current);

int vma_dup_policy(struct vm_area_struct *src, struct vm_area_struct *dst)
{
	struct mempolicy *pol = mpol_dup(vma_policy(src));

	if (IS_ERR(pol))
		return PTR_ERR(pol);
	dst->vm_policy = pol;
	return 0;
}

/*
 * If mpol_dup() sees current->cpuset == cpuset_being_rebound, then it
 * rebinds the mempolicy its copying by calling mpol_rebind_policy()
 * with the mems_allowed returned by cpuset_mems_allowed().  This
 * keeps mempolicies cpuset relative after its cpuset moves.  See
 * further kernel/cpuset.c update_nodemask().
 *
 * current's mempolicy may be rebinded by the other task(the task that changes
 * cpuset's mems), so we needn't do rebind work for current task.
 */

/* Slow path of a mempolicy duplicate */
struct mempolicy *__mpol_dup(struct mempolicy *old)
{
	struct mempolicy *new = kmem_cache_alloc(policy_cache, GFP_KERNEL);

	if (!new)
		return ERR_PTR(-ENOMEM);

	/* task's mempolicy is protected by alloc_lock */
	if (old == current->mempolicy) {
		task_lock(current);
		*new = *old;
		task_unlock(current);
	} else
		*new = *old;

	if (current_cpuset_is_being_rebound()) {
		nodemask_t mems = cpuset_mems_allowed(current);
		if (new->flags & MPOL_F_REBINDING)
			mpol_rebind_policy(new, &mems, MPOL_REBIND_STEP2);
		else
			mpol_rebind_policy(new, &mems, MPOL_REBIND_ONCE);
	}
	atomic_set(&new->refcnt, 1);
	return new;
}

/* Slow path of a mempolicy comparison */
bool __mpol_equal(struct mempolicy *a, struct mempolicy *b)
{
	if (!a || !b)
		return false;
	if (a->mode != b->mode)
		return false;
	if (a->flags != b->flags)
		return false;
	if (mpol_store_user_nodemask(a))
		if (!nodes_equal(a->w.user_nodemask, b->w.user_nodemask))
			return false;

	switch (a->mode) {
	case MPOL_BIND:
		/* Fall through */
	case MPOL_INTERLEAVE:
		return !!nodes_equal(a->v.nodes, b->v.nodes);
	case MPOL_PREFERRED:
		return a->v.preferred_node == b->v.preferred_node;
	default:
		BUG();
		return false;
	}
}

/*
 * Shared memory backing store policy support.
 *
 * Remember policies even when nobody has shared memory mapped.
 * The policies are kept in Red-Black tree linked from the inode.
 * They are protected by the sp->lock spinlock, which should be held
 * for any accesses to the tree.
 */

/* lookup first element intersecting start-end */
/* Caller holds sp->lock */
static struct sp_node *
sp_lookup(struct shared_policy *sp, unsigned long start, unsigned long end)
{
	struct rb_node *n = sp->root.rb_node;

	while (n) {
		struct sp_node *p = rb_entry(n, struct sp_node, nd);

		if (start >= p->end)
			n = n->rb_right;
		else if (end <= p->start)
			n = n->rb_left;
		else
			break;
	}
	if (!n)
		return NULL;
	for (;;) {
		struct sp_node *w = NULL;
		struct rb_node *prev = rb_prev(n);
		if (!prev)
			break;
		w = rb_entry(prev, struct sp_node, nd);
		if (w->end <= start)
			break;
		n = prev;
	}
	return rb_entry(n, struct sp_node, nd);
}

/* Insert a new shared policy into the list. */
/* Caller holds sp->lock */
static void sp_insert(struct shared_policy *sp, struct sp_node *new)
{
	struct rb_node **p = &sp->root.rb_node;
	struct rb_node *parent = NULL;
	struct sp_node *nd;

	while (*p) {
		parent = *p;
		nd = rb_entry(parent, struct sp_node, nd);
		if (new->start < nd->start)
			p = &(*p)->rb_left;
		else if (new->end > nd->end)
			p = &(*p)->rb_right;
		else
			BUG();
	}
	rb_link_node(&new->nd, parent, p);
	rb_insert_color(&new->nd, &sp->root);
	pr_debug("inserting %lx-%lx: %d\n", new->start, new->end,
		 new->policy ? new->policy->mode : 0);
}

/* Find shared policy intersecting idx */
struct mempolicy *
mpol_shared_policy_lookup(struct shared_policy *sp, unsigned long idx)
{
	struct mempolicy *pol = NULL;
	struct sp_node *sn;

	if (!sp->root.rb_node)
		return NULL;
	spin_lock(&sp->lock);
	sn = sp_lookup(sp, idx, idx+1);
	if (sn) {
		mpol_get(sn->policy);
		pol = sn->policy;
	}
	spin_unlock(&sp->lock);
	return pol;
}

static void sp_free(struct sp_node *n)
{
	mpol_put(n->policy);
	kmem_cache_free(sn_cache, n);
}

/**
 * mpol_misplaced - check whether current page node is valid in policy
 *
 * @page: page to be checked
 * @vma: vm area where page mapped
 * @addr: virtual address where page mapped
 *
 * Lookup current policy node id for vma,addr and "compare to" page's
 * node id.
 *
 * Returns:
 *	-1	- not misplaced, page is in the right node
 *	node	- node id where the page should be
 *
 * Policy determination "mimics" alloc_page_vma().
 * Called from fault path where we know the vma and faulting address.
 */
int mpol_misplaced(struct page *page, struct vm_area_struct *vma, unsigned long addr)
{
	struct mempolicy *pol;
	struct zone *zone;
	int curnid = page_to_nid(page);
	unsigned long pgoff;
	int thiscpu = raw_smp_processor_id();
	int thisnid = cpu_to_node(thiscpu);
	int polnid = -1;
	int ret = -1;

	BUG_ON(!vma);

	pol = get_vma_policy(vma, addr);
	if (!(pol->flags & MPOL_F_MOF))
		goto out;

	switch (pol->mode) {
	case MPOL_INTERLEAVE:
		BUG_ON(addr >= vma->vm_end);
		BUG_ON(addr < vma->vm_start);

		pgoff = vma->vm_pgoff;
		pgoff += (addr - vma->vm_start) >> PAGE_SHIFT;
		polnid = offset_il_node(pol, vma, pgoff);
		break;

	case MPOL_PREFERRED:
		if (pol->flags & MPOL_F_LOCAL)
			polnid = numa_node_id();
		else
			polnid = pol->v.preferred_node;
		break;

	case MPOL_BIND:
		/*
		 * allows binding to multiple nodes.
		 * use current page if in policy nodemask,
		 * else select nearest allowed node, if any.
		 * If no allowed nodes, use current [!misplaced].
		 */
		if (node_isset(curnid, pol->v.nodes))
			goto out;
		(void)first_zones_zonelist(
				node_zonelist(numa_node_id(), GFP_HIGHUSER),
				gfp_zone(GFP_HIGHUSER),
				&pol->v.nodes, &zone);
		polnid = zone->node;
		break;

	default:
		BUG();
	}

	/* Migrate the page towards the node whose CPU is referencing it */
	if (pol->flags & MPOL_F_MORON) {
		polnid = thisnid;

		if (!should_numa_migrate_memory(current, page, curnid, thiscpu))
			goto out;
	}

	if (curnid != polnid)
		ret = polnid;
out:
	mpol_cond_put(pol);

	return ret;
}

static void sp_delete(struct shared_policy *sp, struct sp_node *n)
{
	pr_debug("deleting %lx-l%lx\n", n->start, n->end);
	rb_erase(&n->nd, &sp->root);
	sp_free(n);
}

static void sp_node_init(struct sp_node *node, unsigned long start,
			unsigned long end, struct mempolicy *pol)
{
	node->start = start;
	node->end = end;
	node->policy = pol;
}

static struct sp_node *sp_alloc(unsigned long start, unsigned long end,
				struct mempolicy *pol)
{
	struct sp_node *n;
	struct mempolicy *newpol;

	n = kmem_cache_alloc(sn_cache, GFP_KERNEL);
	if (!n)
		return NULL;

	newpol = mpol_dup(pol);
	if (IS_ERR(newpol)) {
		kmem_cache_free(sn_cache, n);
		return NULL;
	}
	newpol->flags |= MPOL_F_SHARED;
	sp_node_init(n, start, end, newpol);

	return n;
}

/* Replace a policy range. */
static int shared_policy_replace(struct shared_policy *sp, unsigned long start,
				 unsigned long end, struct sp_node *new)
{
	struct sp_node *n;
	struct sp_node *n_new = NULL;
	struct mempolicy *mpol_new = NULL;
	int ret = 0;

restart:
	spin_lock(&sp->lock);
	n = sp_lookup(sp, start, end);
	/* Take care of old policies in the same range. */
	while (n && n->start < end) {
		struct rb_node *next = rb_next(&n->nd);
		if (n->start >= start) {
			if (n->end <= end)
				sp_delete(sp, n);
			else
				n->start = end;
		} else {
			/* Old policy spanning whole new range. */
			if (n->end > end) {
				if (!n_new)
					goto alloc_new;

				*mpol_new = *n->policy;
				atomic_set(&mpol_new->refcnt, 1);
				sp_node_init(n_new, end, n->end, mpol_new);
				n->end = start;
				sp_insert(sp, n_new);
				n_new = NULL;
				mpol_new = NULL;
				break;
			} else
				n->end = start;
		}
		if (!next)
			break;
		n = rb_entry(next, struct sp_node, nd);
	}
	if (new)
		sp_insert(sp, new);
	spin_unlock(&sp->lock);
	ret = 0;

err_out:
	if (mpol_new)
		mpol_put(mpol_new);
	if (n_new)
		kmem_cache_free(sn_cache, n_new);

	return ret;

alloc_new:
	spin_unlock(&sp->lock);
	ret = -ENOMEM;
	n_new = kmem_cache_alloc(sn_cache, GFP_KERNEL);
	if (!n_new)
		goto err_out;
	mpol_new = kmem_cache_alloc(policy_cache, GFP_KERNEL);
	if (!mpol_new)
		goto err_out;
	goto restart;
}

/**
 * mpol_shared_policy_init - initialize shared policy for inode
 * @sp: pointer to inode shared policy
 * @mpol:  struct mempolicy to install
 *
 * Install non-NULL @mpol in inode's shared policy rb-tree.
 * On entry, the current task has a reference on a non-NULL @mpol.
 * This must be released on exit.
 * This is called at get_inode() calls and we can use GFP_KERNEL.
 */
void mpol_shared_policy_init(struct shared_policy *sp, struct mempolicy *mpol)
{
	int ret;

	sp->root = RB_ROOT;		/* empty tree == default mempolicy */
	spin_lock_init(&sp->lock);

	if (mpol) {
		struct vm_area_struct pvma;
		struct mempolicy *new;
		NODEMASK_SCRATCH(scratch);

		if (!scratch)
			goto put_mpol;
		/* contextualize the tmpfs mount point mempolicy */
		new = mpol_new(mpol->mode, mpol->flags, &mpol->w.user_nodemask);
		if (IS_ERR(new))
			goto free_scratch; /* no valid nodemask intersection */

		task_lock(current);
		ret = mpol_set_nodemask(new, &mpol->w.user_nodemask, scratch);
		task_unlock(current);
		if (ret)
			goto put_new;

		/* Create pseudo-vma that contains just the policy */
		memset(&pvma, 0, sizeof(struct vm_area_struct));
		pvma.vm_end = TASK_SIZE;	/* policy covers entire file */
		mpol_set_shared_policy(sp, &pvma, new); /* adds ref */

put_new:
		mpol_put(new);			/* drop initial ref */
free_scratch:
		NODEMASK_SCRATCH_FREE(scratch);
put_mpol:
		mpol_put(mpol);	/* drop our incoming ref on sb mpol */
	}
}

int mpol_set_shared_policy(struct shared_policy *info,
			struct vm_area_struct *vma, struct mempolicy *npol)
{
	int err;
	struct sp_node *new = NULL;
	unsigned long sz = vma_pages(vma);

	pr_debug("set_shared_policy %lx sz %lu %d %d %lx\n",
		 vma->vm_pgoff,
		 sz, npol ? npol->mode : -1,
		 npol ? npol->flags : -1,
		 npol ? nodes_addr(npol->v.nodes)[0] : NUMA_NO_NODE);

	if (npol) {
		new = sp_alloc(vma->vm_pgoff, vma->vm_pgoff + sz, npol);
		if (!new)
			return -ENOMEM;
	}
	err = shared_policy_replace(info, vma->vm_pgoff, vma->vm_pgoff+sz, new);
	if (err && new)
		sp_free(new);
	return err;
}

/* Free a backing policy store on inode delete. */
void mpol_free_shared_policy(struct shared_policy *p)
{
	struct sp_node *n;
	struct rb_node *next;

	if (!p->root.rb_node)
		return;
	spin_lock(&p->lock);
	next = rb_first(&p->root);
	while (next) {
		n = rb_entry(next, struct sp_node, nd);
		next = rb_next(&n->nd);
		sp_delete(p, n);
	}
	spin_unlock(&p->lock);
}

#ifdef CONFIG_NUMA_BALANCING
static int __initdata numabalancing_override;

static void __init check_numabalancing_enable(void)
{
	bool numabalancing_default = false;

	if (IS_ENABLED(CONFIG_NUMA_BALANCING_DEFAULT_ENABLED))
		numabalancing_default = true;

	/* Parsed by setup_numabalancing. override == 1 enables, -1 disables */
	if (numabalancing_override)
		set_numabalancing_state(numabalancing_override == 1);

	if (num_online_nodes() > 1 && !numabalancing_override) {
		pr_info("%s automatic NUMA balancing. "
			"Configure with numa_balancing= or the "
			"kernel.numa_balancing sysctl",
			numabalancing_default ? "Enabling" : "Disabling");
		set_numabalancing_state(numabalancing_default);
	}
}

static int __init setup_numabalancing(char *str)
{
	int ret = 0;
	if (!str)
		goto out;

	if (!strcmp(str, "enable")) {
		numabalancing_override = 1;
		ret = 1;
	} else if (!strcmp(str, "disable")) {
		numabalancing_override = -1;
		ret = 1;
	}
out:
	if (!ret)
		pr_warn("Unable to parse numa_balancing=\n");

	return ret;
}
__setup("numa_balancing=", setup_numabalancing);
#else
static inline void __init check_numabalancing_enable(void)
{
}
#endif /* CONFIG_NUMA_BALANCING */

/* assumes fs == KERNEL_DS */
void __init numa_policy_init(void)
{
	nodemask_t interleave_nodes;
	unsigned long largest = 0;
	int nid, prefer = 0;

	policy_cache = kmem_cache_create("numa_policy",
					 sizeof(struct mempolicy),
					 0, SLAB_PANIC, NULL);

	sn_cache = kmem_cache_create("shared_policy_node",
				     sizeof(struct sp_node),
				     0, SLAB_PANIC, NULL);

	for_each_node(nid) {
		preferred_node_policy[nid] = (struct mempolicy) {
			.refcnt = ATOMIC_INIT(1),
			.mode = MPOL_PREFERRED,
			.flags = MPOL_F_MOF | MPOL_F_MORON,
			.v = { .preferred_node = nid, },
		};
	}

	/*
	 * Set interleaving policy for system init. Interleaving is only
	 * enabled across suitably sized nodes (default is >= 16MB), or
	 * fall back to the largest node if they're all smaller.
	 */
	nodes_clear(interleave_nodes);
	for_each_node_state(nid, N_MEMORY) {
		unsigned long total_pages = node_present_pages(nid);

		/* Preserve the largest node */
		if (largest < total_pages) {
			largest = total_pages;
			prefer = nid;
		}

		/* Interleave this node? */
		if ((total_pages << PAGE_SHIFT) >= (16 << 20))
			node_set(nid, interleave_nodes);
	}

	/* All too small, use the largest */
	if (unlikely(nodes_empty(interleave_nodes)))
		node_set(prefer, interleave_nodes);

	if (do_set_mempolicy(MPOL_INTERLEAVE, 0, &interleave_nodes))
		pr_err("%s: interleaving failed\n", __func__);

	check_numabalancing_enable();
}

/* Reset policy of current process to default */
void numa_default_policy(void)
{
	do_set_mempolicy(MPOL_DEFAULT, 0, NULL);
}

/*
 * Parse and format mempolicy from/to strings
 */

/*
 * "local" is implemented internally by MPOL_PREFERRED with MPOL_F_LOCAL flag.
 */
static const char * const policy_modes[] =
{
	[MPOL_DEFAULT]    = "default",
	[MPOL_PREFERRED]  = "prefer",
	[MPOL_BIND]       = "bind",
	[MPOL_INTERLEAVE] = "interleave",
	[MPOL_LOCAL]      = "local",
};


#ifdef CONFIG_TMPFS
/**
 * mpol_parse_str - parse string to mempolicy, for tmpfs mpol mount option.
 * @str:  string containing mempolicy to parse
 * @mpol:  pointer to struct mempolicy pointer, returned on success.
 *
 * Format of input:
 *	<mode>[=<flags>][:<nodelist>]
 *
 * On success, returns 0, else 1
 */
int mpol_parse_str(char *str, struct mempolicy **mpol)
{
	struct mempolicy *new = NULL;
	unsigned short mode;
	unsigned short mode_flags;
	nodemask_t nodes;
	char *nodelist = strchr(str, ':');
	char *flags = strchr(str, '=');
	int err = 1;

	if (nodelist) {
		/* NUL-terminate mode or flags string */
		*nodelist++ = '\0';
		if (nodelist_parse(nodelist, nodes))
			goto out;
		if (!nodes_subset(nodes, node_states[N_MEMORY]))
			goto out;
	} else
		nodes_clear(nodes);

	if (flags)
		*flags++ = '\0';	/* terminate mode string */

	for (mode = 0; mode < MPOL_MAX; mode++) {
		if (!strcmp(str, policy_modes[mode])) {
			break;
		}
	}
	if (mode >= MPOL_MAX)
		goto out;

	switch (mode) {
	case MPOL_PREFERRED:
		/*
		 * Insist on a nodelist of one node only
		 */
		if (nodelist) {
			char *rest = nodelist;
			while (isdigit(*rest))
				rest++;
			if (*rest)
				goto out;
		}
		break;
	case MPOL_INTERLEAVE:
		/*
		 * Default to online nodes with memory if no nodelist
		 */
		if (!nodelist)
			nodes = node_states[N_MEMORY];
		break;
	case MPOL_LOCAL:
		/*
		 * Don't allow a nodelist;  mpol_new() checks flags
		 */
		if (nodelist)
			goto out;
		mode = MPOL_PREFERRED;
		break;
	case MPOL_DEFAULT:
		/*
		 * Insist on a empty nodelist
		 */
		if (!nodelist)
			err = 0;
		goto out;
	case MPOL_BIND:
		/*
		 * Insist on a nodelist
		 */
		if (!nodelist)
			goto out;
	}

	mode_flags = 0;
	if (flags) {
		/*
		 * Currently, we only support two mutually exclusive
		 * mode flags.
		 */
		if (!strcmp(flags, "static"))
			mode_flags |= MPOL_F_STATIC_NODES;
		else if (!strcmp(flags, "relative"))
			mode_flags |= MPOL_F_RELATIVE_NODES;
		else
			goto out;
	}

	new = mpol_new(mode, mode_flags, &nodes);
	if (IS_ERR(new))
		goto out;

	/*
	 * Save nodes for mpol_to_str() to show the tmpfs mount options
	 * for /proc/mounts, /proc/pid/mounts and /proc/pid/mountinfo.
	 */
	if (mode != MPOL_PREFERRED)
		new->v.nodes = nodes;
	else if (nodelist)
		new->v.preferred_node = first_node(nodes);
	else
		new->flags |= MPOL_F_LOCAL;

	/*
	 * Save nodes for contextualization: this will be used to "clone"
	 * the mempolicy in a specific context [cpuset] at a later time.
	 */
	new->w.user_nodemask = nodes;

	err = 0;

out:
	/* Restore string for error message */
	if (nodelist)
		*--nodelist = ':';
	if (flags)
		*--flags = '=';
	if (!err)
		*mpol = new;
	return err;
}
#endif /* CONFIG_TMPFS */

/**
 * mpol_to_str - format a mempolicy structure for printing
 * @buffer:  to contain formatted mempolicy string
 * @maxlen:  length of @buffer
 * @pol:  pointer to mempolicy to be formatted
 *
 * Convert @pol into a string.  If @buffer is too short, truncate the string.
 * Recommend a @maxlen of at least 32 for the longest mode, "interleave", the
 * longest flag, "relative", and to display at least a few node ids.
 */
void mpol_to_str(char *buffer, int maxlen, struct mempolicy *pol)
{
	char *p = buffer;
	nodemask_t nodes = NODE_MASK_NONE;
	unsigned short mode = MPOL_DEFAULT;
	unsigned short flags = 0;

	if (pol && pol != &default_policy && !(pol->flags & MPOL_F_MORON)) {
		mode = pol->mode;
		flags = pol->flags;
	}

	switch (mode) {
	case MPOL_DEFAULT:
		break;
	case MPOL_PREFERRED:
		if (flags & MPOL_F_LOCAL)
			mode = MPOL_LOCAL;
		else
			node_set(pol->v.preferred_node, nodes);
		break;
	case MPOL_BIND:
	case MPOL_INTERLEAVE:
		nodes = pol->v.nodes;
		break;
	default:
		WARN_ON_ONCE(1);
		snprintf(p, maxlen, "unknown");
		return;
	}

	p += snprintf(p, maxlen, "%s", policy_modes[mode]);

	if (flags & MPOL_MODE_FLAGS) {
		p += snprintf(p, buffer + maxlen - p, "=");

		/*
		 * Currently, the only defined flags are mutually exclusive
		 */
		if (flags & MPOL_F_STATIC_NODES)
			p += snprintf(p, buffer + maxlen - p, "static");
		else if (flags & MPOL_F_RELATIVE_NODES)
			p += snprintf(p, buffer + maxlen - p, "relative");
	}

	if (!nodes_empty(nodes))
		p += scnprintf(p, buffer + maxlen - p, ":%*pbl",
			       nodemask_pr_args(&nodes));
}
