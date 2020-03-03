/*
 *	linux/mm/mlock.c
 *
 *  (C) Copyright 1995 Linus Torvalds
 *  (C) Copyright 2002 Christoph Hellwig
 */

#include <linux/capability.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/pagemap.h>
#include <linux/pagevec.h>
#include <linux/mempolicy.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/export.h>
#include <linux/rmap.h>
#include <linux/mmzone.h>
#include <linux/hugetlb.h>
#include <linux/memcontrol.h>
#include <linux/mm_inline.h>

#include "internal.h"

int can_do_mlock(void)
{
	if (rlimit(RLIMIT_MEMLOCK) != 0)
		return 1;
	if (capable(CAP_IPC_LOCK))
		return 1;
	return 0;
}
EXPORT_SYMBOL(can_do_mlock);

/*
 * Mlocked pages are marked with PageMlocked() flag for efficient testing
 * in vmscan and, possibly, the fault path; and to support semi-accurate
 * statistics.
 *
 * An mlocked page [PageMlocked(page)] is unevictable.  As such, it will
 * be placed on the LRU "unevictable" list, rather than the [in]active lists.
 * The unevictable list is an LRU sibling list to the [in]active lists.
 * PageUnevictable is set to indicate the unevictable state.
 *
 * When lazy mlocking via vmscan, it is important to ensure that the
 * vma's VM_LOCKED status is not concurrently being modified, otherwise we
 * may have mlocked a page that is being munlocked. So lazy mlock must take
 * the mmap_sem for read, and verify that the vma really is locked
 * (see mm/rmap.c).
 */

/*
 *  LRU accounting for clear_page_mlock()
 */
void clear_page_mlock(struct page *page)
{
	if (!TestClearPageMlocked(page))
		return;

	mod_zone_page_state(page_zone(page), NR_MLOCK,
			    -hpage_nr_pages(page));
	count_vm_event(UNEVICTABLE_PGCLEARED);
	if (!isolate_lru_page(page)) {
		putback_lru_page(page);
	} else {
		/*
		 * We lost the race. the page already moved to evictable list.
		 */
		if (PageUnevictable(page))
			count_vm_event(UNEVICTABLE_PGSTRANDED);
	}
}

/*
 * Mark page as mlocked if not already.
 * If page on LRU, isolate and putback to move to unevictable list.
 */
void mlock_vma_page(struct page *page)
{
	/* Serialize with page migration */
	BUG_ON(!PageLocked(page));

	if (!TestSetPageMlocked(page)) {
		mod_zone_page_state(page_zone(page), NR_MLOCK,
				    hpage_nr_pages(page));
		count_vm_event(UNEVICTABLE_PGMLOCKED);
		if (!isolate_lru_page(page))
			putback_lru_page(page);
	}
}

/*
 * Isolate a page from LRU with optional get_page() pin.
 * Assumes lru_lock already held and page already pinned.
 */
static bool __munlock_isolate_lru_page(struct page *page, bool getpage)
{
	if (PageLRU(page)) {
		struct lruvec *lruvec;

		lruvec = mem_cgroup_page_lruvec(page, page_zone(page));
		if (getpage)
			get_page(page);
		ClearPageLRU(page);
		del_page_from_lru_list(page, lruvec, page_lru(page));
		return true;
	}

	return false;
}

/*
 * Finish munlock after successful page isolation
 *
 * Page must be locked. This is a wrapper for try_to_munlock()
 * and putback_lru_page() with munlock accounting.
 */
static void __munlock_isolated_page(struct page *page)
{
	int ret = SWAP_AGAIN;

	/*
	 * Optimization: if the page was mapped just once, that's our mapping
	 * and we don't need to check all the other vmas.
	 */
	if (page_mapcount(page) > 1)
		ret = try_to_munlock(page);

	/* Did try_to_unlock() succeed or punt? */
	if (ret != SWAP_MLOCK)
		count_vm_event(UNEVICTABLE_PGMUNLOCKED);

	putback_lru_page(page);
}

/*
 * Accounting for page isolation fail during munlock
 *
 * Performs accounting when page isolation fails in munlock. There is nothing
 * else to do because it means some other task has already removed the page
 * from the LRU. putback_lru_page() will take care of removing the page from
 * the unevictable list, if necessary. vmscan [page_referenced()] will move
 * the page back to the unevictable list if some other vma has it mlocked.
 */
static void __munlock_isolation_failed(struct page *page)
{
	if (PageUnevictable(page))
		__count_vm_event(UNEVICTABLE_PGSTRANDED);
	else
		__count_vm_event(UNEVICTABLE_PGMUNLOCKED);
}

/**
 * munlock_vma_page - munlock a vma page
 * @page - page to be unlocked, either a normal page or THP page head
 *
 * returns the size of the page as a page mask (0 for normal page,
 *         HPAGE_PMD_NR - 1 for THP head page)
 *
 * called from munlock()/munmap() path with page supposedly on the LRU.
 * When we munlock a page, because the vma where we found the page is being
 * munlock()ed or munmap()ed, we want to check whether other vmas hold the
 * page locked so that we can leave it on the unevictable lru list and not
 * bother vmscan with it.  However, to walk the page's rmap list in
 * try_to_munlock() we must isolate the page from the LRU.  If some other
 * task has removed the page from the LRU, we won't be able to do that.
 * So we clear the PageMlocked as we might not get another chance.  If we
 * can't isolate the page, we leave it for putback_lru_page() and vmscan
 * [page_referenced()/try_to_unmap()] to deal with.
 */
unsigned int munlock_vma_page(struct page *page)
{
	int nr_pages;
	struct zone *zone = page_zone(page);

	/* For try_to_munlock() and to serialize with page migration */
	BUG_ON(!PageLocked(page));

	/*
	 * Serialize with any parallel __split_huge_page_refcount() which
	 * might otherwise copy PageMlocked to part of the tail pages before
	 * we clear it in the head page. It also stabilizes hpage_nr_pages().
	 */
	spin_lock_irq(&zone->lru_lock);

	nr_pages = hpage_nr_pages(page);
	if (!TestClearPageMlocked(page))
		goto unlock_out;

	__mod_zone_page_state(zone, NR_MLOCK, -nr_pages);

	if (__munlock_isolate_lru_page(page, true)) {
		spin_unlock_irq(&zone->lru_lock);
		__munlock_isolated_page(page);
		goto out;
	}
	__munlock_isolation_failed(page);

unlock_out:
	spin_unlock_irq(&zone->lru_lock);

out:
	return nr_pages - 1;
}

/*
 * convert get_user_pages() return value to posix mlock() error
 */
static int __mlock_posix_error_return(long retval)
{
	if (retval == -EFAULT)
		retval = -ENOMEM;
	else if (retval == -ENOMEM)
		retval = -EAGAIN;
	return retval;
}

/*
 * Prepare page for fast batched LRU putback via putback_lru_evictable_pagevec()
 *
 * The fast path is available only for evictable pages with single mapping.
 * Then we can bypass the per-cpu pvec and get better performance.
 * when mapcount > 1 we need try_to_munlock() which can fail.
 * when !page_evictable(), we need the full redo logic of putback_lru_page to
 * avoid leaving evictable page in unevictable list.
 *
 * In case of success, @page is added to @pvec and @pgrescued is incremented
 * in case that the page was previously unevictable. @page is also unlocked.
 */
static bool __putback_lru_fast_prepare(struct page *page, struct pagevec *pvec,
		int *pgrescued)
{
	VM_BUG_ON_PAGE(PageLRU(page), page);
	VM_BUG_ON_PAGE(!PageLocked(page), page);

	if (page_mapcount(page) <= 1 && page_evictable(page)) {
		pagevec_add(pvec, page);
		if (TestClearPageUnevictable(page))
			(*pgrescued)++;
		unlock_page(page);
		return true;
	}

	return false;
}

/*
 * Putback multiple evictable pages to the LRU
 *
 * Batched putback of evictable pages that bypasses the per-cpu pvec. Some of
 * the pages might have meanwhile become unevictable but that is OK.
 */
static void __putback_lru_fast(struct pagevec *pvec, int pgrescued)
{
	count_vm_events(UNEVICTABLE_PGMUNLOCKED, pagevec_count(pvec));
	/*
	 *__pagevec_lru_add() calls release_pages() so we don't call
	 * put_page() explicitly
	 */
	__pagevec_lru_add(pvec);
	count_vm_events(UNEVICTABLE_PGRESCUED, pgrescued);
}

/*
 * Munlock a batch of pages from the same zone
 *
 * The work is split to two main phases. First phase clears the Mlocked flag
 * and attempts to isolate the pages, all under a single zone lru lock.
 * The second phase finishes the munlock only for pages where isolation
 * succeeded.
 *
 * Note that the pagevec may be modified during the process.
 */
static void __munlock_pagevec(struct pagevec *pvec, struct zone *zone)
{
	int i;
	int nr = pagevec_count(pvec);
	int delta_munlocked = -nr;
	struct pagevec pvec_putback;
	int pgrescued = 0;

	pagevec_init(&pvec_putback, 0);

	/* Phase 1: page isolation */
	spin_lock_irq(&zone->lru_lock);
	for (i = 0; i < nr; i++) {
		struct page *page = pvec->pages[i];

		if (TestClearPageMlocked(page)) {
			/*
			 * We already have pin from follow_page_mask()
			 * so we can spare the get_page() here.
			 */
			if (__munlock_isolate_lru_page(page, false))
				continue;
			else
				__munlock_isolation_failed(page);
		} else {
			delta_munlocked++;
		}

		/*
		 * We won't be munlocking this page in the next phase
		 * but we still need to release the follow_page_mask()
		 * pin. We cannot do it under lru_lock however. If it's
		 * the last pin, __page_cache_release() would deadlock.
		 */
		pagevec_add(&pvec_putback, pvec->pages[i]);
		pvec->pages[i] = NULL;
	}
	__mod_zone_page_state(zone, NR_MLOCK, delta_munlocked);
	spin_unlock_irq(&zone->lru_lock);

	/* Now we can release pins of pages that we are not munlocking */
	pagevec_release(&pvec_putback);

	/* Phase 2: page munlock */
	for (i = 0; i < nr; i++) {
		struct page *page = pvec->pages[i];

		if (page) {
			lock_page(page);
			if (!__putback_lru_fast_prepare(page, &pvec_putback,
					&pgrescued)) {
				/*
				 * Slow path. We don't want to lose the last
				 * pin before unlock_page()
				 */
				get_page(page); /* for putback_lru_page() */
				__munlock_isolated_page(page);
				unlock_page(page);
				put_page(page); /* from follow_page_mask() */
			}
		}
	}

	/*
	 * Phase 3: page putback for pages that qualified for the fast path
	 * This will also call put_page() to return pin from follow_page_mask()
	 */
	if (pagevec_count(&pvec_putback))
		__putback_lru_fast(&pvec_putback, pgrescued);
}

/*
 * Fill up pagevec for __munlock_pagevec using pte walk
 *
 * The function expects that the struct page corresponding to @start address is
 * a non-TPH page already pinned and in the @pvec, and that it belongs to @zone.
 *
 * The rest of @pvec is filled by subsequent pages within the same pmd and same
 * zone, as long as the pte's are present and vm_normal_page() succeeds. These
 * pages also get pinned.
 *
 * Returns the address of the next page that should be scanned. This equals
 * @start + PAGE_SIZE when no page could be added by the pte walk.
 */
static unsigned long __munlock_pagevec_fill(struct pagevec *pvec,
		struct vm_area_struct *vma, int zoneid,	unsigned long start,
		unsigned long end)
{
	pte_t *pte;
	spinlock_t *ptl;

	/*
	 * Initialize pte walk starting at the already pinned page where we
	 * are sure that there is a pte, as it was pinned under the same
	 * mmap_sem write op.
	 */
	pte = get_locked_pte(vma->vm_mm, start,	&ptl);
	/* Make sure we do not cross the page table boundary */
	end = pgd_addr_end(start, end);
	end = pud_addr_end(start, end);
	end = pmd_addr_end(start, end);

	/* The page next to the pinned page is the first we will try to get */
	start += PAGE_SIZE;
	while (start < end) {
		struct page *page = NULL;
		pte++;
		if (pte_present(*pte))
			page = vm_normal_page(vma, start, *pte);
		/*
		 * Break if page could not be obtained or the page's node+zone does not
		 * match
		 */
		if (!page || page_zone_id(page) != zoneid)
			break;

		get_page(page);
		/*
		 * Increase the address that will be returned *before* the
		 * eventual break due to pvec becoming full by adding the page
		 */
		start += PAGE_SIZE;
		if (pagevec_add(pvec, page) == 0)
			break;
	}
	pte_unmap_unlock(pte, ptl);
	return start;
}

/*
 * munlock_vma_pages_range() - munlock all pages in the vma range.'
 * @vma - vma containing range to be munlock()ed.
 * @start - start address in @vma of the range
 * @end - end of range in @vma.
 *
 *  For mremap(), munmap() and exit().
 *
 * Called with @vma VM_LOCKED.
 *
 * Returns with VM_LOCKED cleared.  Callers must be prepared to
 * deal with this.
 *
 * We don't save and restore VM_LOCKED here because pages are
 * still on lru.  In unmap path, pages might be scanned by reclaim
 * and re-mlocked by try_to_{munlock|unmap} before we unmap and
 * free them.  This will result in freeing mlocked pages.
 */
void munlock_vma_pages_range(struct vm_area_struct *vma,
			     unsigned long start, unsigned long end)
{
	vma->vm_flags &= ~VM_LOCKED;

	while (start < end) {
		struct page *page = NULL;
		unsigned int page_mask;
		unsigned long page_increm;
		struct pagevec pvec;
		struct zone *zone;
		int zoneid;

		pagevec_init(&pvec, 0);
		/*
		 * Although FOLL_DUMP is intended for get_dump_page(),
		 * it just so happens that its special treatment of the
		 * ZERO_PAGE (returning an error instead of doing get_page)
		 * suits munlock very well (and if somehow an abnormal page
		 * has sneaked into the range, we won't oops here: great).
		 */
		page = follow_page_mask(vma, start, FOLL_GET | FOLL_DUMP,
				&page_mask);

		if (page && !IS_ERR(page)) {
			if (PageTransHuge(page)) {
				lock_page(page);
				/*
				 * Any THP page found by follow_page_mask() may
				 * have gotten split before reaching
				 * munlock_vma_page(), so we need to recompute
				 * the page_mask here.
				 */
				page_mask = munlock_vma_page(page);
				unlock_page(page);
				put_page(page); /* follow_page_mask() */
			} else {
				/*
				 * Non-huge pages are handled in batches via
				 * pagevec. The pin from follow_page_mask()
				 * prevents them from collapsing by THP.
				 */
				pagevec_add(&pvec, page);
				zone = page_zone(page);
				zoneid = page_zone_id(page);

				/*
				 * Try to fill the rest of pagevec using fast
				 * pte walk. This will also update start to
				 * the next page to process. Then munlock the
				 * pagevec.
				 */
				start = __munlock_pagevec_fill(&pvec, vma,
						zoneid, start, end);
				__munlock_pagevec(&pvec, zone);
				goto next;
			}
		}
		/* It's a bug to munlock in the middle of a THP page */
		VM_BUG_ON((start >> PAGE_SHIFT) & page_mask);
		page_increm = 1 + page_mask;
		start += page_increm * PAGE_SIZE;
next:
		cond_resched();
	}
}

/*
 * mlock_fixup  - handle mlock[all]/munlock[all] requests.
 *
 * Filters out "special" vmas -- VM_LOCKED never gets set for these, and
 * munlock is a no-op.  However, for some special vmas, we go ahead and
 * populate the ptes.
 *
 * For vmas that pass the filters, merge/split as appropriate.
 */
static int mlock_fixup(struct vm_area_struct *vma, struct vm_area_struct **prev,
	unsigned long start, unsigned long end, vm_flags_t newflags)
{
	struct mm_struct *mm = vma->vm_mm;
	pgoff_t pgoff;
	int nr_pages;
	int ret = 0;
	int lock = !!(newflags & VM_LOCKED);

	if (newflags == vma->vm_flags || (vma->vm_flags & VM_SPECIAL) ||
	    is_vm_hugetlb_page(vma) || vma == get_gate_vma(current->mm))
		goto out;	/* don't set VM_LOCKED,  don't count */

	pgoff = vma->vm_pgoff + ((start - vma->vm_start) >> PAGE_SHIFT);
	*prev = vma_merge(mm, *prev, start, end, newflags, vma->anon_vma,
			  vma->vm_file, pgoff, vma_policy(vma));
	if (*prev) {
		vma = *prev;
		goto success;
	}

	if (start != vma->vm_start) {
		ret = split_vma(mm, vma, start, 1);
		if (ret)
			goto out;
	}

	if (end != vma->vm_end) {
		ret = split_vma(mm, vma, end, 0);
		if (ret)
			goto out;
	}

success:
	/*
	 * Keep track of amount of locked VM.
	 */
	nr_pages = (end - start) >> PAGE_SHIFT;
	if (!lock)
		nr_pages = -nr_pages;
	mm->locked_vm += nr_pages;

	/*
	 * vm_flags is protected by the mmap_sem held in write mode.
	 * It's okay if try_to_unmap_one unmaps a page just after we
	 * set VM_LOCKED, populate_vma_page_range will bring it back.
	 */

	if (lock)
		vma->vm_flags = newflags;
	else
		munlock_vma_pages_range(vma, start, end);

out:
	*prev = vma;
	return ret;
}

static int do_mlock(unsigned long start, size_t len, int on)
{
	unsigned long nstart, end, tmp;
	struct vm_area_struct * vma, * prev;
	int error;

	VM_BUG_ON(start & ~PAGE_MASK);
	VM_BUG_ON(len != PAGE_ALIGN(len));
	end = start + len;
	if (end < start)
		return -EINVAL;
	if (end == start)
		return 0;
	vma = find_vma(current->mm, start);
	if (!vma || vma->vm_start > start)
		return -ENOMEM;

	prev = vma->vm_prev;
	if (start > vma->vm_start)
		prev = vma;

	for (nstart = start ; ; ) {
		vm_flags_t newflags;

		/* Here we know that  vma->vm_start <= nstart < vma->vm_end. */

		newflags = vma->vm_flags & ~VM_LOCKED;
		if (on)
			newflags |= VM_LOCKED;

		tmp = vma->vm_end;
		if (tmp > end)
			tmp = end;
		error = mlock_fixup(vma, &prev, nstart, tmp, newflags);
		if (error)
			break;
		nstart = tmp;
		if (nstart < prev->vm_end)
			nstart = prev->vm_end;
		if (nstart >= end)
			break;

		vma = prev->vm_next;
		if (!vma || vma->vm_start != nstart) {
			error = -ENOMEM;
			break;
		}
	}
	return error;
}

SYSCALL_DEFINE2(mlock, unsigned long, start, size_t, len)
{
	unsigned long locked;
	unsigned long lock_limit;
	int error = -ENOMEM;

	if (!can_do_mlock())
		return -EPERM;

	lru_add_drain_all();	/* flush pagevec */

	len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
	start &= PAGE_MASK;

	lock_limit = rlimit(RLIMIT_MEMLOCK);
	lock_limit >>= PAGE_SHIFT;
	locked = len >> PAGE_SHIFT;

	down_write(&current->mm->mmap_sem);

	locked += current->mm->locked_vm;

	/* check against resource limits */
	if ((locked <= lock_limit) || capable(CAP_IPC_LOCK))
		error = do_mlock(start, len, 1);

	up_write(&current->mm->mmap_sem);
	if (error)
		return error;

	error = __mm_populate(start, len, 0);
	if (error)
		return __mlock_posix_error_return(error);
	return 0;
}

SYSCALL_DEFINE2(munlock, unsigned long, start, size_t, len)
{
	int ret;

	len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
	start &= PAGE_MASK;

	down_write(&current->mm->mmap_sem);
	ret = do_mlock(start, len, 0);
	up_write(&current->mm->mmap_sem);

	return ret;
}

static int do_mlockall(int flags)
{
	struct vm_area_struct * vma, * prev = NULL;

	if (flags & MCL_FUTURE)
		current->mm->def_flags |= VM_LOCKED;
	else
		current->mm->def_flags &= ~VM_LOCKED;
	if (flags == MCL_FUTURE)
		goto out;

	for (vma = current->mm->mmap; vma ; vma = prev->vm_next) {
		vm_flags_t newflags;

		newflags = vma->vm_flags & ~VM_LOCKED;
		if (flags & MCL_CURRENT)
			newflags |= VM_LOCKED;

		/* Ignore errors */
		mlock_fixup(vma, &prev, vma->vm_start, vma->vm_end, newflags);
		cond_resched_rcu_qs();
	}
out:
	return 0;
}

SYSCALL_DEFINE1(mlockall, int, flags)
{
	unsigned long lock_limit;
	int ret = -EINVAL;

	if (!flags || (flags & ~(MCL_CURRENT | MCL_FUTURE)))
		goto out;

	ret = -EPERM;
	if (!can_do_mlock())
		goto out;

	if (flags & MCL_CURRENT)
		lru_add_drain_all();	/* flush pagevec */

	lock_limit = rlimit(RLIMIT_MEMLOCK);
	lock_limit >>= PAGE_SHIFT;

	ret = -ENOMEM;
	down_write(&current->mm->mmap_sem);

	if (!(flags & MCL_CURRENT) || (current->mm->total_vm <= lock_limit) ||
	    capable(CAP_IPC_LOCK))
		ret = do_mlockall(flags);
	up_write(&current->mm->mmap_sem);
	if (!ret && (flags & MCL_CURRENT))
		mm_populate(0, TASK_SIZE);
out:
	return ret;
}

SYSCALL_DEFINE0(munlockall)
{
	int ret;

	down_write(&current->mm->mmap_sem);
	ret = do_mlockall(0);
	up_write(&current->mm->mmap_sem);
	return ret;
}

/*
 * Objects with different lifetime than processes (SHM_LOCK and SHM_HUGETLB
 * shm segments) get accounted against the user_struct instead.
 */
static DEFINE_SPINLOCK(shmlock_user_lock);

int user_shm_lock(size_t size, struct user_struct *user)
{
	unsigned long lock_limit, locked;
	int allowed = 0;

	locked = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	lock_limit = rlimit(RLIMIT_MEMLOCK);
	if (lock_limit == RLIM_INFINITY)
		allowed = 1;
	lock_limit >>= PAGE_SHIFT;
	spin_lock(&shmlock_user_lock);
	if (!allowed &&
	    locked + user->locked_shm > lock_limit && !capable(CAP_IPC_LOCK))
		goto out;
	get_uid(user);
	user->locked_shm += locked;
	allowed = 1;
out:
	spin_unlock(&shmlock_user_lock);
	return allowed;
}

void user_shm_unlock(size_t size, struct user_struct *user)
{
	spin_lock(&shmlock_user_lock);
	user->locked_shm -= (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	spin_unlock(&shmlock_user_lock);
	free_uid(user);
}
