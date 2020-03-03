/*
 *  linux/mm/swapfile.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *  Swap reorganised 29.12.95, Stephen Tweedie
 */

#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/kernel_stat.h>
#include <linux/swap.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/namei.h>
#include <linux/shmem_fs.h>
#include <linux/blkdev.h>
#include <linux/random.h>
#include <linux/writeback.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/ksm.h>
#include <linux/rmap.h>
#include <linux/security.h>
#include <linux/backing-dev.h>
#include <linux/mutex.h>
#include <linux/capability.h>
#include <linux/syscalls.h>
#include <linux/memcontrol.h>
#include <linux/poll.h>
#include <linux/oom.h>
#include <linux/frontswap.h>
#include <linux/swapfile.h>
#include <linux/export.h>

#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <linux/swapops.h>
#include <linux/swap_cgroup.h>

static bool swap_count_continued(struct swap_info_struct *, pgoff_t,
				 unsigned char);
static void free_swap_count_continuations(struct swap_info_struct *);
static sector_t map_swap_entry(swp_entry_t, struct block_device**);

DEFINE_SPINLOCK(swap_lock);
static unsigned int nr_swapfiles;
atomic_long_t nr_swap_pages;
/* protected with swap_lock. reading in vm_swap_full() doesn't need lock */
long total_swap_pages;
static int least_priority;

static const char Bad_file[] = "Bad swap file entry ";
static const char Unused_file[] = "Unused swap file entry ";
static const char Bad_offset[] = "Bad swap offset entry ";
static const char Unused_offset[] = "Unused swap offset entry ";

/*
 * all active swap_info_structs
 * protected with swap_lock, and ordered by priority.
 */
PLIST_HEAD(swap_active_head);

/*
 * all available (active, not full) swap_info_structs
 * protected with swap_avail_lock, ordered by priority.
 * This is used by get_swap_page() instead of swap_active_head
 * because swap_active_head includes all swap_info_structs,
 * but get_swap_page() doesn't need to look at full ones.
 * This uses its own lock instead of swap_lock because when a
 * swap_info_struct changes between not-full/full, it needs to
 * add/remove itself to/from this list, but the swap_info_struct->lock
 * is held and the locking order requires swap_lock to be taken
 * before any swap_info_struct->lock.
 */
static PLIST_HEAD(swap_avail_head);
static DEFINE_SPINLOCK(swap_avail_lock);

struct swap_info_struct *swap_info[MAX_SWAPFILES];

static DEFINE_MUTEX(swapon_mutex);

static DECLARE_WAIT_QUEUE_HEAD(proc_poll_wait);
/* Activity counter to indicate that a swapon or swapoff has occurred */
static atomic_t proc_poll_event = ATOMIC_INIT(0);

static inline unsigned char swap_count(unsigned char ent)
{
	return ent & ~SWAP_HAS_CACHE;	/* may include SWAP_HAS_CONT flag */
}

/* returns 1 if swap entry is freed */
static int
__try_to_reclaim_swap(struct swap_info_struct *si, unsigned long offset)
{
	swp_entry_t entry = swp_entry(si->type, offset);
	struct page *page;
	int ret = 0;

	page = find_get_page(swap_address_space(entry), entry.val);
	if (!page)
		return 0;
	/*
	 * This function is called from scan_swap_map() and it's called
	 * by vmscan.c at reclaiming pages. So, we hold a lock on a page, here.
	 * We have to use trylock for avoiding deadlock. This is a special
	 * case and you should use try_to_free_swap() with explicit lock_page()
	 * in usual operations.
	 */
	if (trylock_page(page)) {
		ret = try_to_free_swap(page);
		unlock_page(page);
	}
	page_cache_release(page);
	return ret;
}

/*
 * swapon tell device that all the old swap contents can be discarded,
 * to allow the swap device to optimize its wear-levelling.
 */
static int discard_swap(struct swap_info_struct *si)
{
	struct swap_extent *se;
	sector_t start_block;
	sector_t nr_blocks;
	int err = 0;

	/* Do not discard the swap header page! */
	se = &si->first_swap_extent;
	start_block = (se->start_block + 1) << (PAGE_SHIFT - 9);
	nr_blocks = ((sector_t)se->nr_pages - 1) << (PAGE_SHIFT - 9);
	if (nr_blocks) {
		err = blkdev_issue_discard(si->bdev, start_block,
				nr_blocks, GFP_KERNEL, 0);
		if (err)
			return err;
		cond_resched();
	}

	list_for_each_entry(se, &si->first_swap_extent.list, list) {
		start_block = se->start_block << (PAGE_SHIFT - 9);
		nr_blocks = (sector_t)se->nr_pages << (PAGE_SHIFT - 9);

		err = blkdev_issue_discard(si->bdev, start_block,
				nr_blocks, GFP_KERNEL, 0);
		if (err)
			break;

		cond_resched();
	}
	return err;		/* That will often be -EOPNOTSUPP */
}

/*
 * swap allocation tell device that a cluster of swap can now be discarded,
 * to allow the swap device to optimize its wear-levelling.
 */
static void discard_swap_cluster(struct swap_info_struct *si,
				 pgoff_t start_page, pgoff_t nr_pages)
{
	struct swap_extent *se = si->curr_swap_extent;
	int found_extent = 0;

	while (nr_pages) {
		struct list_head *lh;

		if (se->start_page <= start_page &&
		    start_page < se->start_page + se->nr_pages) {
			pgoff_t offset = start_page - se->start_page;
			sector_t start_block = se->start_block + offset;
			sector_t nr_blocks = se->nr_pages - offset;

			if (nr_blocks > nr_pages)
				nr_blocks = nr_pages;
			start_page += nr_blocks;
			nr_pages -= nr_blocks;

			if (!found_extent++)
				si->curr_swap_extent = se;

			start_block <<= PAGE_SHIFT - 9;
			nr_blocks <<= PAGE_SHIFT - 9;
			if (blkdev_issue_discard(si->bdev, start_block,
				    nr_blocks, GFP_NOIO, 0))
				break;
		}

		lh = se->list.next;
		se = list_entry(lh, struct swap_extent, list);
	}
}

#define SWAPFILE_CLUSTER	256
#define LATENCY_LIMIT		256

static inline void cluster_set_flag(struct swap_cluster_info *info,
	unsigned int flag)
{
	info->flags = flag;
}

static inline unsigned int cluster_count(struct swap_cluster_info *info)
{
	return info->data;
}

static inline void cluster_set_count(struct swap_cluster_info *info,
				     unsigned int c)
{
	info->data = c;
}

static inline void cluster_set_count_flag(struct swap_cluster_info *info,
					 unsigned int c, unsigned int f)
{
	info->flags = f;
	info->data = c;
}

static inline unsigned int cluster_next(struct swap_cluster_info *info)
{
	return info->data;
}

static inline void cluster_set_next(struct swap_cluster_info *info,
				    unsigned int n)
{
	info->data = n;
}

static inline void cluster_set_next_flag(struct swap_cluster_info *info,
					 unsigned int n, unsigned int f)
{
	info->flags = f;
	info->data = n;
}

static inline bool cluster_is_free(struct swap_cluster_info *info)
{
	return info->flags & CLUSTER_FLAG_FREE;
}

static inline bool cluster_is_null(struct swap_cluster_info *info)
{
	return info->flags & CLUSTER_FLAG_NEXT_NULL;
}

static inline void cluster_set_null(struct swap_cluster_info *info)
{
	info->flags = CLUSTER_FLAG_NEXT_NULL;
	info->data = 0;
}

/* Add a cluster to discard list and schedule it to do discard */
static void swap_cluster_schedule_discard(struct swap_info_struct *si,
		unsigned int idx)
{
	/*
	 * If scan_swap_map() can't find a free cluster, it will check
	 * si->swap_map directly. To make sure the discarding cluster isn't
	 * taken by scan_swap_map(), mark the swap entries bad (occupied). It
	 * will be cleared after discard
	 */
	memset(si->swap_map + idx * SWAPFILE_CLUSTER,
			SWAP_MAP_BAD, SWAPFILE_CLUSTER);

	if (cluster_is_null(&si->discard_cluster_head)) {
		cluster_set_next_flag(&si->discard_cluster_head,
						idx, 0);
		cluster_set_next_flag(&si->discard_cluster_tail,
						idx, 0);
	} else {
		unsigned int tail = cluster_next(&si->discard_cluster_tail);
		cluster_set_next(&si->cluster_info[tail], idx);
		cluster_set_next_flag(&si->discard_cluster_tail,
						idx, 0);
	}

	schedule_work(&si->discard_work);
}

/*
 * Doing discard actually. After a cluster discard is finished, the cluster
 * will be added to free cluster list. caller should hold si->lock.
*/
static void swap_do_scheduled_discard(struct swap_info_struct *si)
{
	struct swap_cluster_info *info;
	unsigned int idx;

	info = si->cluster_info;

	while (!cluster_is_null(&si->discard_cluster_head)) {
		idx = cluster_next(&si->discard_cluster_head);

		cluster_set_next_flag(&si->discard_cluster_head,
						cluster_next(&info[idx]), 0);
		if (cluster_next(&si->discard_cluster_tail) == idx) {
			cluster_set_null(&si->discard_cluster_head);
			cluster_set_null(&si->discard_cluster_tail);
		}
		spin_unlock(&si->lock);

		discard_swap_cluster(si, idx * SWAPFILE_CLUSTER,
				SWAPFILE_CLUSTER);

		spin_lock(&si->lock);
		cluster_set_flag(&info[idx], CLUSTER_FLAG_FREE);
		if (cluster_is_null(&si->free_cluster_head)) {
			cluster_set_next_flag(&si->free_cluster_head,
						idx, 0);
			cluster_set_next_flag(&si->free_cluster_tail,
						idx, 0);
		} else {
			unsigned int tail;

			tail = cluster_next(&si->free_cluster_tail);
			cluster_set_next(&info[tail], idx);
			cluster_set_next_flag(&si->free_cluster_tail,
						idx, 0);
		}
		memset(si->swap_map + idx * SWAPFILE_CLUSTER,
				0, SWAPFILE_CLUSTER);
	}
}

static void swap_discard_work(struct work_struct *work)
{
	struct swap_info_struct *si;

	si = container_of(work, struct swap_info_struct, discard_work);

	spin_lock(&si->lock);
	swap_do_scheduled_discard(si);
	spin_unlock(&si->lock);
}

/*
 * The cluster corresponding to page_nr will be used. The cluster will be
 * removed from free cluster list and its usage counter will be increased.
 */
static void inc_cluster_info_page(struct swap_info_struct *p,
	struct swap_cluster_info *cluster_info, unsigned long page_nr)
{
	unsigned long idx = page_nr / SWAPFILE_CLUSTER;

	if (!cluster_info)
		return;
	if (cluster_is_free(&cluster_info[idx])) {
		VM_BUG_ON(cluster_next(&p->free_cluster_head) != idx);
		cluster_set_next_flag(&p->free_cluster_head,
			cluster_next(&cluster_info[idx]), 0);
		if (cluster_next(&p->free_cluster_tail) == idx) {
			cluster_set_null(&p->free_cluster_tail);
			cluster_set_null(&p->free_cluster_head);
		}
		cluster_set_count_flag(&cluster_info[idx], 0, 0);
	}

	VM_BUG_ON(cluster_count(&cluster_info[idx]) >= SWAPFILE_CLUSTER);
	cluster_set_count(&cluster_info[idx],
		cluster_count(&cluster_info[idx]) + 1);
}

/*
 * The cluster corresponding to page_nr decreases one usage. If the usage
 * counter becomes 0, which means no page in the cluster is in using, we can
 * optionally discard the cluster and add it to free cluster list.
 */
static void dec_cluster_info_page(struct swap_info_struct *p,
	struct swap_cluster_info *cluster_info, unsigned long page_nr)
{
	unsigned long idx = page_nr / SWAPFILE_CLUSTER;

	if (!cluster_info)
		return;

	VM_BUG_ON(cluster_count(&cluster_info[idx]) == 0);
	cluster_set_count(&cluster_info[idx],
		cluster_count(&cluster_info[idx]) - 1);

	if (cluster_count(&cluster_info[idx]) == 0) {
		/*
		 * If the swap is discardable, prepare discard the cluster
		 * instead of free it immediately. The cluster will be freed
		 * after discard.
		 */
		if ((p->flags & (SWP_WRITEOK | SWP_PAGE_DISCARD)) ==
				 (SWP_WRITEOK | SWP_PAGE_DISCARD)) {
			swap_cluster_schedule_discard(p, idx);
			return;
		}

		cluster_set_flag(&cluster_info[idx], CLUSTER_FLAG_FREE);
		if (cluster_is_null(&p->free_cluster_head)) {
			cluster_set_next_flag(&p->free_cluster_head, idx, 0);
			cluster_set_next_flag(&p->free_cluster_tail, idx, 0);
		} else {
			unsigned int tail = cluster_next(&p->free_cluster_tail);
			cluster_set_next(&cluster_info[tail], idx);
			cluster_set_next_flag(&p->free_cluster_tail, idx, 0);
		}
	}
}

/*
 * It's possible scan_swap_map() uses a free cluster in the middle of free
 * cluster list. Avoiding such abuse to avoid list corruption.
 */
static bool
scan_swap_map_ssd_cluster_conflict(struct swap_info_struct *si,
	unsigned long offset)
{
	struct percpu_cluster *percpu_cluster;
	bool conflict;

	offset /= SWAPFILE_CLUSTER;
	conflict = !cluster_is_null(&si->free_cluster_head) &&
		offset != cluster_next(&si->free_cluster_head) &&
		cluster_is_free(&si->cluster_info[offset]);

	if (!conflict)
		return false;

	percpu_cluster = this_cpu_ptr(si->percpu_cluster);
	cluster_set_null(&percpu_cluster->index);
	return true;
}

/*
 * Try to get a swap entry from current cpu's swap entry pool (a cluster). This
 * might involve allocating a new cluster for current CPU too.
 */
static void scan_swap_map_try_ssd_cluster(struct swap_info_struct *si,
	unsigned long *offset, unsigned long *scan_base)
{
	struct percpu_cluster *cluster;
	bool found_free;
	unsigned long tmp;

new_cluster:
	cluster = this_cpu_ptr(si->percpu_cluster);
	if (cluster_is_null(&cluster->index)) {
		if (!cluster_is_null(&si->free_cluster_head)) {
			cluster->index = si->free_cluster_head;
			cluster->next = cluster_next(&cluster->index) *
					SWAPFILE_CLUSTER;
		} else if (!cluster_is_null(&si->discard_cluster_head)) {
			/*
			 * we don't have free cluster but have some clusters in
			 * discarding, do discard now and reclaim them
			 */
			swap_do_scheduled_discard(si);
			*scan_base = *offset = si->cluster_next;
			goto new_cluster;
		} else
			return;
	}

	found_free = false;

	/*
	 * Other CPUs can use our cluster if they can't find a free cluster,
	 * check if there is still free entry in the cluster
	 */
	tmp = cluster->next;
	while (tmp < si->max && tmp < (cluster_next(&cluster->index) + 1) *
	       SWAPFILE_CLUSTER) {
		if (!si->swap_map[tmp]) {
			found_free = true;
			break;
		}
		tmp++;
	}
	if (!found_free) {
		cluster_set_null(&cluster->index);
		goto new_cluster;
	}
	cluster->next = tmp + 1;
	*offset = tmp;
	*scan_base = tmp;
}

static unsigned long scan_swap_map(struct swap_info_struct *si,
				   unsigned char usage)
{
	unsigned long offset;
	unsigned long scan_base;
	unsigned long last_in_cluster = 0;
	int latency_ration = LATENCY_LIMIT;

	/*
	 * We try to cluster swap pages by allocating them sequentially
	 * in swap.  Once we've allocated SWAPFILE_CLUSTER pages this
	 * way, however, we resort to first-free allocation, starting
	 * a new cluster.  This prevents us from scattering swap pages
	 * all over the entire swap partition, so that we reduce
	 * overall disk seek times between swap pages.  -- sct
	 * But we do now try to find an empty cluster.  -Andrea
	 * And we let swap pages go all over an SSD partition.  Hugh
	 */

	si->flags += SWP_SCANNING;
	scan_base = offset = si->cluster_next;

	/* SSD algorithm */
	if (si->cluster_info) {
		scan_swap_map_try_ssd_cluster(si, &offset, &scan_base);
		goto checks;
	}

	if (unlikely(!si->cluster_nr--)) {
		if (si->pages - si->inuse_pages < SWAPFILE_CLUSTER) {
			si->cluster_nr = SWAPFILE_CLUSTER - 1;
			goto checks;
		}

		spin_unlock(&si->lock);

		/*
		 * If seek is expensive, start searching for new cluster from
		 * start of partition, to minimize the span of allocated swap.
		 * If seek is cheap, that is the SWP_SOLIDSTATE si->cluster_info
		 * case, just handled by scan_swap_map_try_ssd_cluster() above.
		 */
		scan_base = offset = si->lowest_bit;
		last_in_cluster = offset + SWAPFILE_CLUSTER - 1;

		/* Locate the first empty (unaligned) cluster */
		for (; last_in_cluster <= si->highest_bit; offset++) {
			if (si->swap_map[offset])
				last_in_cluster = offset + SWAPFILE_CLUSTER;
			else if (offset == last_in_cluster) {
				spin_lock(&si->lock);
				offset -= SWAPFILE_CLUSTER - 1;
				si->cluster_next = offset;
				si->cluster_nr = SWAPFILE_CLUSTER - 1;
				goto checks;
			}
			if (unlikely(--latency_ration < 0)) {
				cond_resched();
				latency_ration = LATENCY_LIMIT;
			}
		}

		offset = scan_base;
		spin_lock(&si->lock);
		si->cluster_nr = SWAPFILE_CLUSTER - 1;
	}

checks:
	if (si->cluster_info) {
		while (scan_swap_map_ssd_cluster_conflict(si, offset))
			scan_swap_map_try_ssd_cluster(si, &offset, &scan_base);
	}
	if (!(si->flags & SWP_WRITEOK))
		goto no_page;
	if (!si->highest_bit)
		goto no_page;
	if (offset > si->highest_bit)
		scan_base = offset = si->lowest_bit;

	/* reuse swap entry of cache-only swap if not busy. */
	if (vm_swap_full() && si->swap_map[offset] == SWAP_HAS_CACHE) {
		int swap_was_freed;
		spin_unlock(&si->lock);
		swap_was_freed = __try_to_reclaim_swap(si, offset);
		spin_lock(&si->lock);
		/* entry was freed successfully, try to use this again */
		if (swap_was_freed)
			goto checks;
		goto scan; /* check next one */
	}

	if (si->swap_map[offset])
		goto scan;

	if (offset == si->lowest_bit)
		si->lowest_bit++;
	if (offset == si->highest_bit)
		si->highest_bit--;
	si->inuse_pages++;
	if (si->inuse_pages == si->pages) {
		si->lowest_bit = si->max;
		si->highest_bit = 0;
		spin_lock(&swap_avail_lock);
		plist_del(&si->avail_list, &swap_avail_head);
		spin_unlock(&swap_avail_lock);
	}
	si->swap_map[offset] = usage;
	inc_cluster_info_page(si, si->cluster_info, offset);
	si->cluster_next = offset + 1;
	si->flags -= SWP_SCANNING;

	return offset;

scan:
	spin_unlock(&si->lock);
	while (++offset <= si->highest_bit) {
		if (!si->swap_map[offset]) {
			spin_lock(&si->lock);
			goto checks;
		}
		if (vm_swap_full() && si->swap_map[offset] == SWAP_HAS_CACHE) {
			spin_lock(&si->lock);
			goto checks;
		}
		if (unlikely(--latency_ration < 0)) {
			cond_resched();
			latency_ration = LATENCY_LIMIT;
		}
	}
	offset = si->lowest_bit;
	while (offset < scan_base) {
		if (!si->swap_map[offset]) {
			spin_lock(&si->lock);
			goto checks;
		}
		if (vm_swap_full() && si->swap_map[offset] == SWAP_HAS_CACHE) {
			spin_lock(&si->lock);
			goto checks;
		}
		if (unlikely(--latency_ration < 0)) {
			cond_resched();
			latency_ration = LATENCY_LIMIT;
		}
		offset++;
	}
	spin_lock(&si->lock);

no_page:
	si->flags -= SWP_SCANNING;
	return 0;
}

swp_entry_t get_swap_page(void)
{
	struct swap_info_struct *si, *next;
	pgoff_t offset;

	if (atomic_long_read(&nr_swap_pages) <= 0)
		goto noswap;
	atomic_long_dec(&nr_swap_pages);

	spin_lock(&swap_avail_lock);

start_over:
	plist_for_each_entry_safe(si, next, &swap_avail_head, avail_list) {
		/* requeue si to after same-priority siblings */
		plist_requeue(&si->avail_list, &swap_avail_head);
		spin_unlock(&swap_avail_lock);
		spin_lock(&si->lock);
		if (!si->highest_bit || !(si->flags & SWP_WRITEOK)) {
			spin_lock(&swap_avail_lock);
			if (plist_node_empty(&si->avail_list)) {
				spin_unlock(&si->lock);
				goto nextsi;
			}
			WARN(!si->highest_bit,
			     "swap_info %d in list but !highest_bit\n",
			     si->type);
			WARN(!(si->flags & SWP_WRITEOK),
			     "swap_info %d in list but !SWP_WRITEOK\n",
			     si->type);
			plist_del(&si->avail_list, &swap_avail_head);
			spin_unlock(&si->lock);
			goto nextsi;
		}

		/* This is called for allocating swap entry for cache */
		offset = scan_swap_map(si, SWAP_HAS_CACHE);
		spin_unlock(&si->lock);
		if (offset)
			return swp_entry(si->type, offset);
		pr_debug("scan_swap_map of si %d failed to find offset\n",
		       si->type);
		spin_lock(&swap_avail_lock);
nextsi:
		/*
		 * if we got here, it's likely that si was almost full before,
		 * and since scan_swap_map() can drop the si->lock, multiple
		 * callers probably all tried to get a page from the same si
		 * and it filled up before we could get one; or, the si filled
		 * up between us dropping swap_avail_lock and taking si->lock.
		 * Since we dropped the swap_avail_lock, the swap_avail_head
		 * list may have been modified; so if next is still in the
		 * swap_avail_head list then try it, otherwise start over.
		 */
		if (plist_node_empty(&next->avail_list))
			goto start_over;
	}

	spin_unlock(&swap_avail_lock);

	atomic_long_inc(&nr_swap_pages);
noswap:
	return (swp_entry_t) {0};
}

/* The only caller of this function is now suspend routine */
swp_entry_t get_swap_page_of_type(int type)
{
	struct swap_info_struct *si;
	pgoff_t offset;

	si = swap_info[type];
	spin_lock(&si->lock);
	if (si && (si->flags & SWP_WRITEOK)) {
		atomic_long_dec(&nr_swap_pages);
		/* This is called for allocating swap entry, not cache */
		offset = scan_swap_map(si, 1);
		if (offset) {
			spin_unlock(&si->lock);
			return swp_entry(type, offset);
		}
		atomic_long_inc(&nr_swap_pages);
	}
	spin_unlock(&si->lock);
	return (swp_entry_t) {0};
}

static struct swap_info_struct *swap_info_get(swp_entry_t entry)
{
	struct swap_info_struct *p;
	unsigned long offset, type;

	if (!entry.val)
		goto out;
	type = swp_type(entry);
	if (type >= nr_swapfiles)
		goto bad_nofile;
	p = swap_info[type];
	if (!(p->flags & SWP_USED))
		goto bad_device;
	offset = swp_offset(entry);
	if (offset >= p->max)
		goto bad_offset;
	if (!p->swap_map[offset])
		goto bad_free;
	spin_lock(&p->lock);
	return p;

bad_free:
	pr_err("swap_free: %s%08lx\n", Unused_offset, entry.val);
	goto out;
bad_offset:
	pr_err("swap_free: %s%08lx\n", Bad_offset, entry.val);
	goto out;
bad_device:
	pr_err("swap_free: %s%08lx\n", Unused_file, entry.val);
	goto out;
bad_nofile:
	pr_err("swap_free: %s%08lx\n", Bad_file, entry.val);
out:
	return NULL;
}

static unsigned char swap_entry_free(struct swap_info_struct *p,
				     swp_entry_t entry, unsigned char usage)
{
	unsigned long offset = swp_offset(entry);
	unsigned char count;
	unsigned char has_cache;

	count = p->swap_map[offset];
	has_cache = count & SWAP_HAS_CACHE;
	count &= ~SWAP_HAS_CACHE;

	if (usage == SWAP_HAS_CACHE) {
		VM_BUG_ON(!has_cache);
		has_cache = 0;
	} else if (count == SWAP_MAP_SHMEM) {
		/*
		 * Or we could insist on shmem.c using a special
		 * swap_shmem_free() and free_shmem_swap_and_cache()...
		 */
		count = 0;
	} else if ((count & ~COUNT_CONTINUED) <= SWAP_MAP_MAX) {
		if (count == COUNT_CONTINUED) {
			if (swap_count_continued(p, offset, count))
				count = SWAP_MAP_MAX | COUNT_CONTINUED;
			else
				count = SWAP_MAP_MAX;
		} else
			count--;
	}

	if (!count)
		mem_cgroup_uncharge_swap(entry);

	usage = count | has_cache;
	p->swap_map[offset] = usage;

	/* free if no reference */
	if (!usage) {
		dec_cluster_info_page(p, p->cluster_info, offset);
		if (offset < p->lowest_bit)
			p->lowest_bit = offset;
		if (offset > p->highest_bit) {
			bool was_full = !p->highest_bit;
			p->highest_bit = offset;
			if (was_full && (p->flags & SWP_WRITEOK)) {
				spin_lock(&swap_avail_lock);
				WARN_ON(!plist_node_empty(&p->avail_list));
				if (plist_node_empty(&p->avail_list))
					plist_add(&p->avail_list,
						  &swap_avail_head);
				spin_unlock(&swap_avail_lock);
			}
		}
		atomic_long_inc(&nr_swap_pages);
		p->inuse_pages--;
		frontswap_invalidate_page(p->type, offset);
		if (p->flags & SWP_BLKDEV) {
			struct gendisk *disk = p->bdev->bd_disk;
			if (disk->fops->swap_slot_free_notify)
				disk->fops->swap_slot_free_notify(p->bdev,
								  offset);
		}
	}

	return usage;
}

/*
 * Caller has made sure that the swap device corresponding to entry
 * is still around or has not been recycled.
 */
void swap_free(swp_entry_t entry)
{
	struct swap_info_struct *p;

	p = swap_info_get(entry);
	if (p) {
		swap_entry_free(p, entry, 1);
		spin_unlock(&p->lock);
	}
}

/*
 * Called after dropping swapcache to decrease refcnt to swap entries.
 */
void swapcache_free(swp_entry_t entry)
{
	struct swap_info_struct *p;

	p = swap_info_get(entry);
	if (p) {
		swap_entry_free(p, entry, SWAP_HAS_CACHE);
		spin_unlock(&p->lock);
	}
}

/*
 * How many references to page are currently swapped out?
 * This does not give an exact answer when swap count is continued,
 * but does include the high COUNT_CONTINUED flag to allow for that.
 */
int page_swapcount(struct page *page)
{
	int count = 0;
	struct swap_info_struct *p;
	swp_entry_t entry;

	entry.val = page_private(page);
	p = swap_info_get(entry);
	if (p) {
		count = swap_count(p->swap_map[swp_offset(entry)]);
		spin_unlock(&p->lock);
	}
	return count;
}

/*
 * We can write to an anon page without COW if there are no other references
 * to it.  And as a side-effect, free up its swap: because the old content
 * on disk will never be read, and seeking back there to write new content
 * later would only waste time away from clustering.
 */
int reuse_swap_page(struct page *page)
{
	int count;

	VM_BUG_ON_PAGE(!PageLocked(page), page);
	if (unlikely(PageKsm(page)))
		return 0;
	count = page_mapcount(page);
	if (count <= 1 && PageSwapCache(page)) {
		count += page_swapcount(page);
		if (count == 1 && !PageWriteback(page)) {
			delete_from_swap_cache(page);
			SetPageDirty(page);
		}
	}
	return count <= 1;
}

/*
 * If swap is getting full, or if there are no more mappings of this page,
 * then try_to_free_swap is called to free its swap space.
 */
int try_to_free_swap(struct page *page)
{
	VM_BUG_ON_PAGE(!PageLocked(page), page);

	if (!PageSwapCache(page))
		return 0;
	if (PageWriteback(page))
		return 0;
	if (page_swapcount(page))
		return 0;

	/*
	 * Once hibernation has begun to create its image of memory,
	 * there's a danger that one of the calls to try_to_free_swap()
	 * - most probably a call from __try_to_reclaim_swap() while
	 * hibernation is allocating its own swap pages for the image,
	 * but conceivably even a call from memory reclaim - will free
	 * the swap from a page which has already been recorded in the
	 * image as a clean swapcache page, and then reuse its swap for
	 * another page of the image.  On waking from hibernation, the
	 * original page might be freed under memory pressure, then
	 * later read back in from swap, now with the wrong data.
	 *
	 * Hibernation suspends storage while it is writing the image
	 * to disk so check that here.
	 */
	if (pm_suspended_storage())
		return 0;

	delete_from_swap_cache(page);
	SetPageDirty(page);
	return 1;
}

/*
 * Free the swap entry like above, but also try to
 * free the page cache entry if it is the last user.
 */
int free_swap_and_cache(swp_entry_t entry)
{
	struct swap_info_struct *p;
	struct page *page = NULL;

	if (non_swap_entry(entry))
		return 1;

	p = swap_info_get(entry);
	if (p) {
		if (swap_entry_free(p, entry, 1) == SWAP_HAS_CACHE) {
			page = find_get_page(swap_address_space(entry),
						entry.val);
			if (page && !trylock_page(page)) {
				page_cache_release(page);
				page = NULL;
			}
		}
		spin_unlock(&p->lock);
	}
	if (page) {
		/*
		 * Not mapped elsewhere, or swap space full? Free it!
		 * Also recheck PageSwapCache now page is locked (above).
		 */
		if (PageSwapCache(page) && !PageWriteback(page) &&
				(!page_mapped(page) || vm_swap_full())) {
			delete_from_swap_cache(page);
			SetPageDirty(page);
		}
		unlock_page(page);
		page_cache_release(page);
	}
	return p != NULL;
}

#ifdef CONFIG_HIBERNATION
/*
 * Find the swap type that corresponds to given device (if any).
 *
 * @offset - number of the PAGE_SIZE-sized block of the device, starting
 * from 0, in which the swap header is expected to be located.
 *
 * This is needed for the suspend to disk (aka swsusp).
 */
int swap_type_of(dev_t device, sector_t offset, struct block_device **bdev_p)
{
	struct block_device *bdev = NULL;
	int type;

	if (device)
		bdev = bdget(device);

	spin_lock(&swap_lock);
	for (type = 0; type < nr_swapfiles; type++) {
		struct swap_info_struct *sis = swap_info[type];

		if (!(sis->flags & SWP_WRITEOK))
			continue;

		if (!bdev) {
			if (bdev_p)
				*bdev_p = bdgrab(sis->bdev);

			spin_unlock(&swap_lock);
			return type;
		}
		if (bdev == sis->bdev) {
			struct swap_extent *se = &sis->first_swap_extent;

			if (se->start_block == offset) {
				if (bdev_p)
					*bdev_p = bdgrab(sis->bdev);

				spin_unlock(&swap_lock);
				bdput(bdev);
				return type;
			}
		}
	}
	spin_unlock(&swap_lock);
	if (bdev)
		bdput(bdev);

	return -ENODEV;
}

/*
 * Get the (PAGE_SIZE) block corresponding to given offset on the swapdev
 * corresponding to given index in swap_info (swap type).
 */
sector_t swapdev_block(int type, pgoff_t offset)
{
	struct block_device *bdev;

	if ((unsigned int)type >= nr_swapfiles)
		return 0;
	if (!(swap_info[type]->flags & SWP_WRITEOK))
		return 0;
	return map_swap_entry(swp_entry(type, offset), &bdev);
}

/*
 * Return either the total number of swap pages of given type, or the number
 * of free pages of that type (depending on @free)
 *
 * This is needed for software suspend
 */
unsigned int count_swap_pages(int type, int free)
{
	unsigned int n = 0;

	spin_lock(&swap_lock);
	if ((unsigned int)type < nr_swapfiles) {
		struct swap_info_struct *sis = swap_info[type];

		spin_lock(&sis->lock);
		if (sis->flags & SWP_WRITEOK) {
			n = sis->pages;
			if (free)
				n -= sis->inuse_pages;
		}
		spin_unlock(&sis->lock);
	}
	spin_unlock(&swap_lock);
	return n;
}
#endif /* CONFIG_HIBERNATION */

static inline int maybe_same_pte(pte_t pte, pte_t swp_pte)
{
#ifdef CONFIG_MEM_SOFT_DIRTY
	/*
	 * When pte keeps soft dirty bit the pte generated
	 * from swap entry does not has it, still it's same
	 * pte from logical point of view.
	 */
	pte_t swp_pte_dirty = pte_swp_mksoft_dirty(swp_pte);
	return pte_same(pte, swp_pte) || pte_same(pte, swp_pte_dirty);
#else
	return pte_same(pte, swp_pte);
#endif
}

/*
 * No need to decide whether this PTE shares the swap entry with others,
 * just let do_wp_page work it out if a write is requested later - to
 * force COW, vm_page_prot omits write permission from any private vma.
 */
static int unuse_pte(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long addr, swp_entry_t entry, struct page *page)
{
	struct page *swapcache;
	struct mem_cgroup *memcg;
	spinlock_t *ptl;
	pte_t *pte;
	int ret = 1;

	swapcache = page;
	page = ksm_might_need_to_copy(page, vma, addr);
	if (unlikely(!page))
		return -ENOMEM;

	if (mem_cgroup_try_charge(page, vma->vm_mm, GFP_KERNEL, &memcg)) {
		ret = -ENOMEM;
		goto out_nolock;
	}

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	if (unlikely(!maybe_same_pte(*pte, swp_entry_to_pte(entry)))) {
		mem_cgroup_cancel_charge(page, memcg);
		ret = 0;
		goto out;
	}

	dec_mm_counter(vma->vm_mm, MM_SWAPENTS);
	inc_mm_counter(vma->vm_mm, MM_ANONPAGES);
	get_page(page);
	set_pte_at(vma->vm_mm, addr, pte,
		   pte_mkold(mk_pte(page, vma->vm_page_prot)));
	if (page == swapcache) {
		page_add_anon_rmap(page, vma, addr);
		mem_cgroup_commit_charge(page, memcg, true);
	} else { /* ksm created a completely new copy */
		page_add_new_anon_rmap(page, vma, addr);
		mem_cgroup_commit_charge(page, memcg, false);
		lru_cache_add_active_or_unevictable(page, vma);
	}
	swap_free(entry);
	/*
	 * Move the page to the active list so it is not
	 * immediately swapped out again after swapon.
	 */
	activate_page(page);
out:
	pte_unmap_unlock(pte, ptl);
out_nolock:
	if (page != swapcache) {
		unlock_page(page);
		put_page(page);
	}
	return ret;
}

static int unuse_pte_range(struct vm_area_struct *vma, pmd_t *pmd,
				unsigned long addr, unsigned long end,
				swp_entry_t entry, struct page *page)
{
	pte_t swp_pte = swp_entry_to_pte(entry);
	pte_t *pte;
	int ret = 0;

	/*
	 * We don't actually need pte lock while scanning for swp_pte: since
	 * we hold page lock and mmap_sem, swp_pte cannot be inserted into the
	 * page table while we're scanning; though it could get zapped, and on
	 * some architectures (e.g. x86_32 with PAE) we might catch a glimpse
	 * of unmatched parts which look like swp_pte, so unuse_pte must
	 * recheck under pte lock.  Scanning without pte lock lets it be
	 * preemptable whenever CONFIG_PREEMPT but not CONFIG_HIGHPTE.
	 */
	pte = pte_offset_map(pmd, addr);
	do {
		/*
		 * swapoff spends a _lot_ of time in this loop!
		 * Test inline before going to call unuse_pte.
		 */
		if (unlikely(maybe_same_pte(*pte, swp_pte))) {
			pte_unmap(pte);
			ret = unuse_pte(vma, pmd, addr, entry, page);
			if (ret)
				goto out;
			pte = pte_offset_map(pmd, addr);
		}
	} while (pte++, addr += PAGE_SIZE, addr != end);
	pte_unmap(pte - 1);
out:
	return ret;
}

static inline int unuse_pmd_range(struct vm_area_struct *vma, pud_t *pud,
				unsigned long addr, unsigned long end,
				swp_entry_t entry, struct page *page)
{
	pmd_t *pmd;
	unsigned long next;
	int ret;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (pmd_none_or_trans_huge_or_clear_bad(pmd))
			continue;
		ret = unuse_pte_range(vma, pmd, addr, next, entry, page);
		if (ret)
			return ret;
	} while (pmd++, addr = next, addr != end);
	return 0;
}

static inline int unuse_pud_range(struct vm_area_struct *vma, pgd_t *pgd,
				unsigned long addr, unsigned long end,
				swp_entry_t entry, struct page *page)
{
	pud_t *pud;
	unsigned long next;
	int ret;

	pud = pud_offset(pgd, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_none_or_clear_bad(pud))
			continue;
		ret = unuse_pmd_range(vma, pud, addr, next, entry, page);
		if (ret)
			return ret;
	} while (pud++, addr = next, addr != end);
	return 0;
}

static int unuse_vma(struct vm_area_struct *vma,
				swp_entry_t entry, struct page *page)
{
	pgd_t *pgd;
	unsigned long addr, end, next;
	int ret;

	if (page_anon_vma(page)) {
		addr = page_address_in_vma(page, vma);
		if (addr == -EFAULT)
			return 0;
		else
			end = addr + PAGE_SIZE;
	} else {
		addr = vma->vm_start;
		end = vma->vm_end;
	}

	pgd = pgd_offset(vma->vm_mm, addr);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		ret = unuse_pud_range(vma, pgd, addr, next, entry, page);
		if (ret)
			return ret;
	} while (pgd++, addr = next, addr != end);
	return 0;
}

static int unuse_mm(struct mm_struct *mm,
				swp_entry_t entry, struct page *page)
{
	struct vm_area_struct *vma;
	int ret = 0;

	if (!down_read_trylock(&mm->mmap_sem)) {
		/*
		 * Activate page so shrink_inactive_list is unlikely to unmap
		 * its ptes while lock is dropped, so swapoff can make progress.
		 */
		activate_page(page);
		unlock_page(page);
		down_read(&mm->mmap_sem);
		lock_page(page);
	}
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		if (vma->anon_vma && (ret = unuse_vma(vma, entry, page)))
			break;
	}
	up_read(&mm->mmap_sem);
	return (ret < 0)? ret: 0;
}

/*
 * Scan swap_map (or frontswap_map if frontswap parameter is true)
 * from current position to next entry still in use.
 * Recycle to start on reaching the end, returning 0 when empty.
 */
static unsigned int find_next_to_unuse(struct swap_info_struct *si,
					unsigned int prev, bool frontswap)
{
	unsigned int max = si->max;
	unsigned int i = prev;
	unsigned char count;

	/*
	 * No need for swap_lock here: we're just looking
	 * for whether an entry is in use, not modifying it; false
	 * hits are okay, and sys_swapoff() has already prevented new
	 * allocations from this area (while holding swap_lock).
	 */
	for (;;) {
		if (++i >= max) {
			if (!prev) {
				i = 0;
				break;
			}
			/*
			 * No entries in use at top of swap_map,
			 * loop back to start and recheck there.
			 */
			max = prev + 1;
			prev = 0;
			i = 1;
		}
		if (frontswap) {
			if (frontswap_test(si, i))
				break;
			else
				continue;
		}
		count = READ_ONCE(si->swap_map[i]);
		if (count && swap_count(count) != SWAP_MAP_BAD)
			break;
	}
	return i;
}

/*
 * We completely avoid races by reading each swap page in advance,
 * and then search for the process using it.  All the necessary
 * page table adjustments can then be made atomically.
 *
 * if the boolean frontswap is true, only unuse pages_to_unuse pages;
 * pages_to_unuse==0 means all pages; ignored if frontswap is false
 */
int try_to_unuse(unsigned int type, bool frontswap,
		 unsigned long pages_to_unuse)
{
	struct swap_info_struct *si = swap_info[type];
	struct mm_struct *start_mm;
	volatile unsigned char *swap_map; /* swap_map is accessed without
					   * locking. Mark it as volatile
					   * to prevent compiler doing
					   * something odd.
					   */
	unsigned char swcount;
	struct page *page;
	swp_entry_t entry;
	unsigned int i = 0;
	int retval = 0;

	/*
	 * When searching mms for an entry, a good strategy is to
	 * start at the first mm we freed the previous entry from
	 * (though actually we don't notice whether we or coincidence
	 * freed the entry).  Initialize this start_mm with a hold.
	 *
	 * A simpler strategy would be to start at the last mm we
	 * freed the previous entry from; but that would take less
	 * advantage of mmlist ordering, which clusters forked mms
	 * together, child after parent.  If we race with dup_mmap(), we
	 * prefer to resolve parent before child, lest we miss entries
	 * duplicated after we scanned child: using last mm would invert
	 * that.
	 */
	start_mm = &init_mm;
	atomic_inc(&init_mm.mm_users);

	/*
	 * Keep on scanning until all entries have gone.  Usually,
	 * one pass through swap_map is enough, but not necessarily:
	 * there are races when an instance of an entry might be missed.
	 */
	while ((i = find_next_to_unuse(si, i, frontswap)) != 0) {
		if (signal_pending(current)) {
			retval = -EINTR;
			break;
		}

		/*
		 * Get a page for the entry, using the existing swap
		 * cache page if there is one.  Otherwise, get a clean
		 * page and read the swap into it.
		 */
		swap_map = &si->swap_map[i];
		entry = swp_entry(type, i);
		page = read_swap_cache_async(entry,
					GFP_HIGHUSER_MOVABLE, NULL, 0);
		if (!page) {
			/*
			 * Either swap_duplicate() failed because entry
			 * has been freed independently, and will not be
			 * reused since sys_swapoff() already disabled
			 * allocation from here, or alloc_page() failed.
			 */
			swcount = *swap_map;
			/*
			 * We don't hold lock here, so the swap entry could be
			 * SWAP_MAP_BAD (when the cluster is discarding).
			 * Instead of fail out, We can just skip the swap
			 * entry because swapoff will wait for discarding
			 * finish anyway.
			 */
			if (!swcount || swcount == SWAP_MAP_BAD)
				continue;
			retval = -ENOMEM;
			break;
		}

		/*
		 * Don't hold on to start_mm if it looks like exiting.
		 */
		if (atomic_read(&start_mm->mm_users) == 1) {
			mmput(start_mm);
			start_mm = &init_mm;
			atomic_inc(&init_mm.mm_users);
		}

		/*
		 * Wait for and lock page.  When do_swap_page races with
		 * try_to_unuse, do_swap_page can handle the fault much
		 * faster than try_to_unuse can locate the entry.  This
		 * apparently redundant "wait_on_page_locked" lets try_to_unuse
		 * defer to do_swap_page in such a case - in some tests,
		 * do_swap_page and try_to_unuse repeatedly compete.
		 */
		wait_on_page_locked(page);
		wait_on_page_writeback(page);
		lock_page(page);
		wait_on_page_writeback(page);

		/*
		 * Remove all references to entry.
		 */
		swcount = *swap_map;
		if (swap_count(swcount) == SWAP_MAP_SHMEM) {
			retval = shmem_unuse(entry, page);
			/* page has already been unlocked and released */
			if (retval < 0)
				break;
			continue;
		}
		if (swap_count(swcount) && start_mm != &init_mm)
			retval = unuse_mm(start_mm, entry, page);

		if (swap_count(*swap_map)) {
			int set_start_mm = (*swap_map >= swcount);
			struct list_head *p = &start_mm->mmlist;
			struct mm_struct *new_start_mm = start_mm;
			struct mm_struct *prev_mm = start_mm;
			struct mm_struct *mm;

			atomic_inc(&new_start_mm->mm_users);
			atomic_inc(&prev_mm->mm_users);
			spin_lock(&mmlist_lock);
			while (swap_count(*swap_map) && !retval &&
					(p = p->next) != &start_mm->mmlist) {
				mm = list_entry(p, struct mm_struct, mmlist);
				if (!atomic_inc_not_zero(&mm->mm_users))
					continue;
				spin_unlock(&mmlist_lock);
				mmput(prev_mm);
				prev_mm = mm;

				cond_resched();

				swcount = *swap_map;
				if (!swap_count(swcount)) /* any usage ? */
					;
				else if (mm == &init_mm)
					set_start_mm = 1;
				else
					retval = unuse_mm(mm, entry, page);

				if (set_start_mm && *swap_map < swcount) {
					mmput(new_start_mm);
					atomic_inc(&mm->mm_users);
					new_start_mm = mm;
					set_start_mm = 0;
				}
				spin_lock(&mmlist_lock);
			}
			spin_unlock(&mmlist_lock);
			mmput(prev_mm);
			mmput(start_mm);
			start_mm = new_start_mm;
		}
		if (retval) {
			unlock_page(page);
			page_cache_release(page);
			break;
		}

		/*
		 * If a reference remains (rare), we would like to leave
		 * the page in the swap cache; but try_to_unmap could
		 * then re-duplicate the entry once we drop page lock,
		 * so we might loop indefinitely; also, that page could
		 * not be swapped out to other storage meanwhile.  So:
		 * delete from cache even if there's another reference,
		 * after ensuring that the data has been saved to disk -
		 * since if the reference remains (rarer), it will be
		 * read from disk into another page.  Splitting into two
		 * pages would be incorrect if swap supported "shared
		 * private" pages, but they are handled by tmpfs files.
		 *
		 * Given how unuse_vma() targets one particular offset
		 * in an anon_vma, once the anon_vma has been determined,
		 * this splitting happens to be just what is needed to
		 * handle where KSM pages have been swapped out: re-reading
		 * is unnecessarily slow, but we can fix that later on.
		 */
		if (swap_count(*swap_map) &&
		     PageDirty(page) && PageSwapCache(page)) {
			struct writeback_control wbc = {
				.sync_mode = WB_SYNC_NONE,
			};

			swap_writepage(page, &wbc);
			lock_page(page);
			wait_on_page_writeback(page);
		}

		/*
		 * It is conceivable that a racing task removed this page from
		 * swap cache just before we acquired the page lock at the top,
		 * or while we dropped it in unuse_mm().  The page might even
		 * be back in swap cache on another swap area: that we must not
		 * delete, since it may not have been written out to swap yet.
		 */
		if (PageSwapCache(page) &&
		    likely(page_private(page) == entry.val))
			delete_from_swap_cache(page);

		/*
		 * So we could skip searching mms once swap count went
		 * to 1, we did not mark any present ptes as dirty: must
		 * mark page dirty so shrink_page_list will preserve it.
		 */
		SetPageDirty(page);
		unlock_page(page);
		page_cache_release(page);

		/*
		 * Make sure that we aren't completely killing
		 * interactive performance.
		 */
		cond_resched();
		if (frontswap && pages_to_unuse > 0) {
			if (!--pages_to_unuse)
				break;
		}
	}

	mmput(start_mm);
	return retval;
}

/*
 * After a successful try_to_unuse, if no swap is now in use, we know
 * we can empty the mmlist.  swap_lock must be held on entry and exit.
 * Note that mmlist_lock nests inside swap_lock, and an mm must be
 * added to the mmlist just after page_duplicate - before would be racy.
 */
static void drain_mmlist(void)
{
	struct list_head *p, *next;
	unsigned int type;

	for (type = 0; type < nr_swapfiles; type++)
		if (swap_info[type]->inuse_pages)
			return;
	spin_lock(&mmlist_lock);
	list_for_each_safe(p, next, &init_mm.mmlist)
		list_del_init(p);
	spin_unlock(&mmlist_lock);
}

/*
 * Use this swapdev's extent info to locate the (PAGE_SIZE) block which
 * corresponds to page offset for the specified swap entry.
 * Note that the type of this function is sector_t, but it returns page offset
 * into the bdev, not sector offset.
 */
static sector_t map_swap_entry(swp_entry_t entry, struct block_device **bdev)
{
	struct swap_info_struct *sis;
	struct swap_extent *start_se;
	struct swap_extent *se;
	pgoff_t offset;

	sis = swap_info[swp_type(entry)];
	*bdev = sis->bdev;

	offset = swp_offset(entry);
	start_se = sis->curr_swap_extent;
	se = start_se;

	for ( ; ; ) {
		struct list_head *lh;

		if (se->start_page <= offset &&
				offset < (se->start_page + se->nr_pages)) {
			return se->start_block + (offset - se->start_page);
		}
		lh = se->list.next;
		se = list_entry(lh, struct swap_extent, list);
		sis->curr_swap_extent = se;
		BUG_ON(se == start_se);		/* It *must* be present */
	}
}

/*
 * Returns the page offset into bdev for the specified page's swap entry.
 */
sector_t map_swap_page(struct page *page, struct block_device **bdev)
{
	swp_entry_t entry;
	entry.val = page_private(page);
	return map_swap_entry(entry, bdev);
}

/*
 * Free all of a swapdev's extent information
 */
static void destroy_swap_extents(struct swap_info_struct *sis)
{
	while (!list_empty(&sis->first_swap_extent.list)) {
		struct swap_extent *se;

		se = list_entry(sis->first_swap_extent.list.next,
				struct swap_extent, list);
		list_del(&se->list);
		kfree(se);
	}

	if (sis->flags & SWP_FILE) {
		struct file *swap_file = sis->swap_file;
		struct address_space *mapping = swap_file->f_mapping;

		sis->flags &= ~SWP_FILE;
		mapping->a_ops->swap_deactivate(swap_file);
	}
}

/*
 * Add a block range (and the corresponding page range) into this swapdev's
 * extent list.  The extent list is kept sorted in page order.
 *
 * This function rather assumes that it is called in ascending page order.
 */
int
add_swap_extent(struct swap_info_struct *sis, unsigned long start_page,
		unsigned long nr_pages, sector_t start_block)
{
	struct swap_extent *se;
	struct swap_extent *new_se;
	struct list_head *lh;

	if (start_page == 0) {
		se = &sis->first_swap_extent;
		sis->curr_swap_extent = se;
		se->start_page = 0;
		se->nr_pages = nr_pages;
		se->start_block = start_block;
		return 1;
	} else {
		lh = sis->first_swap_extent.list.prev;	/* Highest extent */
		se = list_entry(lh, struct swap_extent, list);
		BUG_ON(se->start_page + se->nr_pages != start_page);
		if (se->start_block + se->nr_pages == start_block) {
			/* Merge it */
			se->nr_pages += nr_pages;
			return 0;
		}
	}

	/*
	 * No merge.  Insert a new extent, preserving ordering.
	 */
	new_se = kmalloc(sizeof(*se), GFP_KERNEL);
	if (new_se == NULL)
		return -ENOMEM;
	new_se->start_page = start_page;
	new_se->nr_pages = nr_pages;
	new_se->start_block = start_block;

	list_add_tail(&new_se->list, &sis->first_swap_extent.list);
	return 1;
}

/*
 * A `swap extent' is a simple thing which maps a contiguous range of pages
 * onto a contiguous range of disk blocks.  An ordered list of swap extents
 * is built at swapon time and is then used at swap_writepage/swap_readpage
 * time for locating where on disk a page belongs.
 *
 * If the swapfile is an S_ISBLK block device, a single extent is installed.
 * This is done so that the main operating code can treat S_ISBLK and S_ISREG
 * swap files identically.
 *
 * Whether the swapdev is an S_ISREG file or an S_ISBLK blockdev, the swap
 * extent list operates in PAGE_SIZE disk blocks.  Both S_ISREG and S_ISBLK
 * swapfiles are handled *identically* after swapon time.
 *
 * For S_ISREG swapfiles, setup_swap_extents() will walk all the file's blocks
 * and will parse them into an ordered extent list, in PAGE_SIZE chunks.  If
 * some stray blocks are found which do not fall within the PAGE_SIZE alignment
 * requirements, they are simply tossed out - we will never use those blocks
 * for swapping.
 *
 * For S_ISREG swapfiles we set S_SWAPFILE across the life of the swapon.  This
 * prevents root from shooting her foot off by ftruncating an in-use swapfile,
 * which will scribble on the fs.
 *
 * The amount of disk space which a single swap extent represents varies.
 * Typically it is in the 1-4 megabyte range.  So we can have hundreds of
 * extents in the list.  To avoid much list walking, we cache the previous
 * search location in `curr_swap_extent', and start new searches from there.
 * This is extremely effective.  The average number of iterations in
 * map_swap_page() has been measured at about 0.3 per page.  - akpm.
 */
static int setup_swap_extents(struct swap_info_struct *sis, sector_t *span)
{
	struct file *swap_file = sis->swap_file;
	struct address_space *mapping = swap_file->f_mapping;
	struct inode *inode = mapping->host;
	int ret;

	if (S_ISBLK(inode->i_mode)) {
		ret = add_swap_extent(sis, 0, sis->max, 0);
		*span = sis->pages;
		return ret;
	}

	if (mapping->a_ops->swap_activate) {
		ret = mapping->a_ops->swap_activate(sis, swap_file, span);
		if (!ret) {
			sis->flags |= SWP_FILE;
			ret = add_swap_extent(sis, 0, sis->max, 0);
			*span = sis->pages;
		}
		return ret;
	}

	return generic_swapfile_activate(sis, swap_file, span);
}

static void _enable_swap_info(struct swap_info_struct *p, int prio,
				unsigned char *swap_map,
				struct swap_cluster_info *cluster_info)
{
	if (prio >= 0)
		p->prio = prio;
	else
		p->prio = --least_priority;
	/*
	 * the plist prio is negated because plist ordering is
	 * low-to-high, while swap ordering is high-to-low
	 */
	p->list.prio = -p->prio;
	p->avail_list.prio = -p->prio;
	p->swap_map = swap_map;
	p->cluster_info = cluster_info;
	p->flags |= SWP_WRITEOK;
	atomic_long_add(p->pages, &nr_swap_pages);
	total_swap_pages += p->pages;

	assert_spin_locked(&swap_lock);
	/*
	 * both lists are plists, and thus priority ordered.
	 * swap_active_head needs to be priority ordered for swapoff(),
	 * which on removal of any swap_info_struct with an auto-assigned
	 * (i.e. negative) priority increments the auto-assigned priority
	 * of any lower-priority swap_info_structs.
	 * swap_avail_head needs to be priority ordered for get_swap_page(),
	 * which allocates swap pages from the highest available priority
	 * swap_info_struct.
	 */
	plist_add(&p->list, &swap_active_head);
	spin_lock(&swap_avail_lock);
	plist_add(&p->avail_list, &swap_avail_head);
	spin_unlock(&swap_avail_lock);
}

static void enable_swap_info(struct swap_info_struct *p, int prio,
				unsigned char *swap_map,
				struct swap_cluster_info *cluster_info,
				unsigned long *frontswap_map)
{
	frontswap_init(p->type, frontswap_map);
	spin_lock(&swap_lock);
	spin_lock(&p->lock);
	 _enable_swap_info(p, prio, swap_map, cluster_info);
	spin_unlock(&p->lock);
	spin_unlock(&swap_lock);
}

static void reinsert_swap_info(struct swap_info_struct *p)
{
	spin_lock(&swap_lock);
	spin_lock(&p->lock);
	_enable_swap_info(p, p->prio, p->swap_map, p->cluster_info);
	spin_unlock(&p->lock);
	spin_unlock(&swap_lock);
}

SYSCALL_DEFINE1(swapoff, const char __user *, specialfile)
{
	struct swap_info_struct *p = NULL;
	unsigned char *swap_map;
	struct swap_cluster_info *cluster_info;
	unsigned long *frontswap_map;
	struct file *swap_file, *victim;
	struct address_space *mapping;
	struct inode *inode;
	struct filename *pathname;
	int err, found = 0;
	unsigned int old_block_size;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	BUG_ON(!current->mm);

	pathname = getname(specialfile);
	if (IS_ERR(pathname))
		return PTR_ERR(pathname);

	victim = file_open_name(pathname, O_RDWR|O_LARGEFILE, 0);
	err = PTR_ERR(victim);
	if (IS_ERR(victim))
		goto out;

	mapping = victim->f_mapping;
	spin_lock(&swap_lock);
	plist_for_each_entry(p, &swap_active_head, list) {
		if (p->flags & SWP_WRITEOK) {
			if (p->swap_file->f_mapping == mapping) {
				found = 1;
				break;
			}
		}
	}
	if (!found) {
		err = -EINVAL;
		spin_unlock(&swap_lock);
		goto out_dput;
	}
	if (!security_vm_enough_memory_mm(current->mm, p->pages))
		vm_unacct_memory(p->pages);
	else {
		err = -ENOMEM;
		spin_unlock(&swap_lock);
		goto out_dput;
	}
	spin_lock(&swap_avail_lock);
	plist_del(&p->avail_list, &swap_avail_head);
	spin_unlock(&swap_avail_lock);
	spin_lock(&p->lock);
	if (p->prio < 0) {
		struct swap_info_struct *si = p;

		plist_for_each_entry_continue(si, &swap_active_head, list) {
			si->prio++;
			si->list.prio--;
			si->avail_list.prio--;
		}
		least_priority++;
	}
	plist_del(&p->list, &swap_active_head);
	atomic_long_sub(p->pages, &nr_swap_pages);
	total_swap_pages -= p->pages;
	p->flags &= ~SWP_WRITEOK;
	spin_unlock(&p->lock);
	spin_unlock(&swap_lock);

	set_current_oom_origin();
	err = try_to_unuse(p->type, false, 0); /* force unuse all pages */
	clear_current_oom_origin();

	if (err) {
		/* re-insert swap space back into swap_list */
		reinsert_swap_info(p);
		goto out_dput;
	}

	flush_work(&p->discard_work);

	destroy_swap_extents(p);
	if (p->flags & SWP_CONTINUED)
		free_swap_count_continuations(p);

	mutex_lock(&swapon_mutex);
	spin_lock(&swap_lock);
	spin_lock(&p->lock);
	drain_mmlist();

	/* wait for anyone still in scan_swap_map */
	p->highest_bit = 0;		/* cuts scans short */
	while (p->flags >= SWP_SCANNING) {
		spin_unlock(&p->lock);
		spin_unlock(&swap_lock);
		schedule_timeout_uninterruptible(1);
		spin_lock(&swap_lock);
		spin_lock(&p->lock);
	}

	swap_file = p->swap_file;
	old_block_size = p->old_block_size;
	p->swap_file = NULL;
	p->max = 0;
	swap_map = p->swap_map;
	p->swap_map = NULL;
	cluster_info = p->cluster_info;
	p->cluster_info = NULL;
	frontswap_map = frontswap_map_get(p);
	spin_unlock(&p->lock);
	spin_unlock(&swap_lock);
	frontswap_invalidate_area(p->type);
	frontswap_map_set(p, NULL);
	mutex_unlock(&swapon_mutex);
	free_percpu(p->percpu_cluster);
	p->percpu_cluster = NULL;
	vfree(swap_map);
	vfree(cluster_info);
	vfree(frontswap_map);
	/* Destroy swap account information */
	swap_cgroup_swapoff(p->type);

	inode = mapping->host;
	if (S_ISBLK(inode->i_mode)) {
		struct block_device *bdev = I_BDEV(inode);
		set_blocksize(bdev, old_block_size);
		blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	} else {
		mutex_lock(&inode->i_mutex);
		inode->i_flags &= ~S_SWAPFILE;
		mutex_unlock(&inode->i_mutex);
	}
	filp_close(swap_file, NULL);

	/*
	 * Clear the SWP_USED flag after all resources are freed so that swapon
	 * can reuse this swap_info in alloc_swap_info() safely.  It is ok to
	 * not hold p->lock after we cleared its SWP_WRITEOK.
	 */
	spin_lock(&swap_lock);
	p->flags = 0;
	spin_unlock(&swap_lock);

	err = 0;
	atomic_inc(&proc_poll_event);
	wake_up_interruptible(&proc_poll_wait);

out_dput:
	filp_close(victim, NULL);
out:
	putname(pathname);
	return err;
}

#ifdef CONFIG_PROC_FS
static unsigned swaps_poll(struct file *file, poll_table *wait)
{
	struct seq_file *seq = file->private_data;

	poll_wait(file, &proc_poll_wait, wait);

	if (seq->poll_event != atomic_read(&proc_poll_event)) {
		seq->poll_event = atomic_read(&proc_poll_event);
		return POLLIN | POLLRDNORM | POLLERR | POLLPRI;
	}

	return POLLIN | POLLRDNORM;
}

/* iterator */
static void *swap_start(struct seq_file *swap, loff_t *pos)
{
	struct swap_info_struct *si;
	int type;
	loff_t l = *pos;

	mutex_lock(&swapon_mutex);

	if (!l)
		return SEQ_START_TOKEN;

	for (type = 0; type < nr_swapfiles; type++) {
		smp_rmb();	/* read nr_swapfiles before swap_info[type] */
		si = swap_info[type];
		if (!(si->flags & SWP_USED) || !si->swap_map)
			continue;
		if (!--l)
			return si;
	}

	return NULL;
}

static void *swap_next(struct seq_file *swap, void *v, loff_t *pos)
{
	struct swap_info_struct *si = v;
	int type;

	if (v == SEQ_START_TOKEN)
		type = 0;
	else
		type = si->type + 1;

	for (; type < nr_swapfiles; type++) {
		smp_rmb();	/* read nr_swapfiles before swap_info[type] */
		si = swap_info[type];
		if (!(si->flags & SWP_USED) || !si->swap_map)
			continue;
		++*pos;
		return si;
	}

	return NULL;
}

static void swap_stop(struct seq_file *swap, void *v)
{
	mutex_unlock(&swapon_mutex);
}

static int swap_show(struct seq_file *swap, void *v)
{
	struct swap_info_struct *si = v;
	struct file *file;
	int len;

	if (si == SEQ_START_TOKEN) {
		seq_puts(swap,"Filename\t\t\t\tType\t\tSize\tUsed\tPriority\n");
		return 0;
	}

	file = si->swap_file;
	len = seq_path(swap, &file->f_path, " \t\n\\");
	seq_printf(swap, "%*s%s\t%u\t%u\t%d\n",
			len < 40 ? 40 - len : 1, " ",
			S_ISBLK(file_inode(file)->i_mode) ?
				"partition" : "file\t",
			si->pages << (PAGE_SHIFT - 10),
			si->inuse_pages << (PAGE_SHIFT - 10),
			si->prio);
	return 0;
}

static const struct seq_operations swaps_op = {
	.start =	swap_start,
	.next =		swap_next,
	.stop =		swap_stop,
	.show =		swap_show
};

static int swaps_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	int ret;

	ret = seq_open(file, &swaps_op);
	if (ret)
		return ret;

	seq = file->private_data;
	seq->poll_event = atomic_read(&proc_poll_event);
	return 0;
}

static const struct file_operations proc_swaps_operations = {
	.open		= swaps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
	.poll		= swaps_poll,
};

static int __init procswaps_init(void)
{
	proc_create("swaps", 0, NULL, &proc_swaps_operations);
	return 0;
}
__initcall(procswaps_init);
#endif /* CONFIG_PROC_FS */

#ifdef MAX_SWAPFILES_CHECK
static int __init max_swapfiles_check(void)
{
	MAX_SWAPFILES_CHECK();
	return 0;
}
late_initcall(max_swapfiles_check);
#endif

static struct swap_info_struct *alloc_swap_info(void)
{
	struct swap_info_struct *p;
	unsigned int type;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return ERR_PTR(-ENOMEM);

	spin_lock(&swap_lock);
	for (type = 0; type < nr_swapfiles; type++) {
		if (!(swap_info[type]->flags & SWP_USED))
			break;
	}
	if (type >= MAX_SWAPFILES) {
		spin_unlock(&swap_lock);
		kfree(p);
		return ERR_PTR(-EPERM);
	}
	if (type >= nr_swapfiles) {
		p->type = type;
		swap_info[type] = p;
		/*
		 * Write swap_info[type] before nr_swapfiles, in case a
		 * racing procfs swap_start() or swap_next() is reading them.
		 * (We never shrink nr_swapfiles, we never free this entry.)
		 */
		smp_wmb();
		nr_swapfiles++;
	} else {
		kfree(p);
		p = swap_info[type];
		/*
		 * Do not memset this entry: a racing procfs swap_next()
		 * would be relying on p->type to remain valid.
		 */
	}
	INIT_LIST_HEAD(&p->first_swap_extent.list);
	plist_node_init(&p->list, 0);
	plist_node_init(&p->avail_list, 0);
	p->flags = SWP_USED;
	spin_unlock(&swap_lock);
	spin_lock_init(&p->lock);

	return p;
}

static int claim_swapfile(struct swap_info_struct *p, struct inode *inode)
{
	int error;

	if (S_ISBLK(inode->i_mode)) {
		p->bdev = bdgrab(I_BDEV(inode));
		error = blkdev_get(p->bdev,
				   FMODE_READ | FMODE_WRITE | FMODE_EXCL,
				   sys_swapon);
		if (error < 0) {
			p->bdev = NULL;
			return -EINVAL;
		}
		p->old_block_size = block_size(p->bdev);
		error = set_blocksize(p->bdev, PAGE_SIZE);
		if (error < 0)
			return error;
		p->flags |= SWP_BLKDEV;
	} else if (S_ISREG(inode->i_mode)) {
		p->bdev = inode->i_sb->s_bdev;
		mutex_lock(&inode->i_mutex);
		if (IS_SWAPFILE(inode))
			return -EBUSY;
	} else
		return -EINVAL;

	return 0;
}

static unsigned long read_swap_header(struct swap_info_struct *p,
					union swap_header *swap_header,
					struct inode *inode)
{
	int i;
	unsigned long maxpages;
	unsigned long swapfilepages;
	unsigned long last_page;

	if (memcmp("SWAPSPACE2", swap_header->magic.magic, 10)) {
		pr_err("Unable to find swap-space signature\n");
		return 0;
	}

	/* swap partition endianess hack... */
	if (swab32(swap_header->info.version) == 1) {
		swab32s(&swap_header->info.version);
		swab32s(&swap_header->info.last_page);
		swab32s(&swap_header->info.nr_badpages);
		for (i = 0; i < swap_header->info.nr_badpages; i++)
			swab32s(&swap_header->info.badpages[i]);
	}
	/* Check the swap header's sub-version */
	if (swap_header->info.version != 1) {
		pr_warn("Unable to handle swap header version %d\n",
			swap_header->info.version);
		return 0;
	}

	p->lowest_bit  = 1;
	p->cluster_next = 1;
	p->cluster_nr = 0;

	/*
	 * Find out how many pages are allowed for a single swap
	 * device. There are two limiting factors: 1) the number
	 * of bits for the swap offset in the swp_entry_t type, and
	 * 2) the number of bits in the swap pte as defined by the
	 * different architectures. In order to find the
	 * largest possible bit mask, a swap entry with swap type 0
	 * and swap offset ~0UL is created, encoded to a swap pte,
	 * decoded to a swp_entry_t again, and finally the swap
	 * offset is extracted. This will mask all the bits from
	 * the initial ~0UL mask that can't be encoded in either
	 * the swp_entry_t or the architecture definition of a
	 * swap pte.
	 */
	maxpages = swp_offset(pte_to_swp_entry(
			swp_entry_to_pte(swp_entry(0, ~0UL)))) + 1;
	last_page = swap_header->info.last_page;
	if (last_page > maxpages) {
		pr_warn("Truncating oversized swap area, only using %luk out of %luk\n",
			maxpages << (PAGE_SHIFT - 10),
			last_page << (PAGE_SHIFT - 10));
	}
	if (maxpages > last_page) {
		maxpages = last_page + 1;
		/* p->max is an unsigned int: don't overflow it */
		if ((unsigned int)maxpages == 0)
			maxpages = UINT_MAX;
	}
	p->highest_bit = maxpages - 1;

	if (!maxpages)
		return 0;
	swapfilepages = i_size_read(inode) >> PAGE_SHIFT;
	if (swapfilepages && maxpages > swapfilepages) {
		pr_warn("Swap area shorter than signature indicates\n");
		return 0;
	}
	if (swap_header->info.nr_badpages && S_ISREG(inode->i_mode))
		return 0;
	if (swap_header->info.nr_badpages > MAX_SWAP_BADPAGES)
		return 0;

	return maxpages;
}

static int setup_swap_map_and_extents(struct swap_info_struct *p,
					union swap_header *swap_header,
					unsigned char *swap_map,
					struct swap_cluster_info *cluster_info,
					unsigned long maxpages,
					sector_t *span)
{
	int i;
	unsigned int nr_good_pages;
	int nr_extents;
	unsigned long nr_clusters = DIV_ROUND_UP(maxpages, SWAPFILE_CLUSTER);
	unsigned long idx = p->cluster_next / SWAPFILE_CLUSTER;

	nr_good_pages = maxpages - 1;	/* omit header page */

	cluster_set_null(&p->free_cluster_head);
	cluster_set_null(&p->free_cluster_tail);
	cluster_set_null(&p->discard_cluster_head);
	cluster_set_null(&p->discard_cluster_tail);

	for (i = 0; i < swap_header->info.nr_badpages; i++) {
		unsigned int page_nr = swap_header->info.badpages[i];
		if (page_nr == 0 || page_nr > swap_header->info.last_page)
			return -EINVAL;
		if (page_nr < maxpages) {
			swap_map[page_nr] = SWAP_MAP_BAD;
			nr_good_pages--;
			/*
			 * Haven't marked the cluster free yet, no list
			 * operation involved
			 */
			inc_cluster_info_page(p, cluster_info, page_nr);
		}
	}

	/* Haven't marked the cluster free yet, no list operation involved */
	for (i = maxpages; i < round_up(maxpages, SWAPFILE_CLUSTER); i++)
		inc_cluster_info_page(p, cluster_info, i);

	if (nr_good_pages) {
		swap_map[0] = SWAP_MAP_BAD;
		/*
		 * Not mark the cluster free yet, no list
		 * operation involved
		 */
		inc_cluster_info_page(p, cluster_info, 0);
		p->max = maxpages;
		p->pages = nr_good_pages;
		nr_extents = setup_swap_extents(p, span);
		if (nr_extents < 0)
			return nr_extents;
		nr_good_pages = p->pages;
	}
	if (!nr_good_pages) {
		pr_warn("Empty swap-file\n");
		return -EINVAL;
	}

	if (!cluster_info)
		return nr_extents;

	for (i = 0; i < nr_clusters; i++) {
		if (!cluster_count(&cluster_info[idx])) {
			cluster_set_flag(&cluster_info[idx], CLUSTER_FLAG_FREE);
			if (cluster_is_null(&p->free_cluster_head)) {
				cluster_set_next_flag(&p->free_cluster_head,
								idx, 0);
				cluster_set_next_flag(&p->free_cluster_tail,
								idx, 0);
			} else {
				unsigned int tail;

				tail = cluster_next(&p->free_cluster_tail);
				cluster_set_next(&cluster_info[tail], idx);
				cluster_set_next_flag(&p->free_cluster_tail,
								idx, 0);
			}
		}
		idx++;
		if (idx == nr_clusters)
			idx = 0;
	}
	return nr_extents;
}

/*
 * Helper to sys_swapon determining if a given swap
 * backing device queue supports DISCARD operations.
 */
static bool swap_discardable(struct swap_info_struct *si)
{
	struct request_queue *q = bdev_get_queue(si->bdev);

	if (!q || !blk_queue_discard(q))
		return false;

	return true;
}

SYSCALL_DEFINE2(swapon, const char __user *, specialfile, int, swap_flags)
{
	struct swap_info_struct *p;
	struct filename *name;
	struct file *swap_file = NULL;
	struct address_space *mapping;
	int i;
	int prio;
	int error;
	union swap_header *swap_header;
	int nr_extents;
	sector_t span;
	unsigned long maxpages;
	unsigned char *swap_map = NULL;
	struct swap_cluster_info *cluster_info = NULL;
	unsigned long *frontswap_map = NULL;
	struct page *page = NULL;
	struct inode *inode = NULL;

	if (swap_flags & ~SWAP_FLAGS_VALID)
		return -EINVAL;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	p = alloc_swap_info();
	if (IS_ERR(p))
		return PTR_ERR(p);

	INIT_WORK(&p->discard_work, swap_discard_work);

	name = getname(specialfile);
	if (IS_ERR(name)) {
		error = PTR_ERR(name);
		name = NULL;
		goto bad_swap;
	}
	swap_file = file_open_name(name, O_RDWR|O_LARGEFILE, 0);
	if (IS_ERR(swap_file)) {
		error = PTR_ERR(swap_file);
		swap_file = NULL;
		goto bad_swap;
	}

	p->swap_file = swap_file;
	mapping = swap_file->f_mapping;

	for (i = 0; i < nr_swapfiles; i++) {
		struct swap_info_struct *q = swap_info[i];

		if (q == p || !q->swap_file)
			continue;
		if (mapping == q->swap_file->f_mapping) {
			error = -EBUSY;
			goto bad_swap;
		}
	}

	inode = mapping->host;
	/* If S_ISREG(inode->i_mode) will do mutex_lock(&inode->i_mutex); */
	error = claim_swapfile(p, inode);
	if (unlikely(error))
		goto bad_swap;

	/*
	 * Read the swap header.
	 */
	if (!mapping->a_ops->readpage) {
		error = -EINVAL;
		goto bad_swap;
	}
	page = read_mapping_page(mapping, 0, swap_file);
	if (IS_ERR(page)) {
		error = PTR_ERR(page);
		goto bad_swap;
	}
	swap_header = kmap(page);

	maxpages = read_swap_header(p, swap_header, inode);
	if (unlikely(!maxpages)) {
		error = -EINVAL;
		goto bad_swap;
	}

	/* OK, set up the swap map and apply the bad block list */
	swap_map = vzalloc(maxpages);
	if (!swap_map) {
		error = -ENOMEM;
		goto bad_swap;
	}
	if (p->bdev && blk_queue_nonrot(bdev_get_queue(p->bdev))) {
		p->flags |= SWP_SOLIDSTATE;
		/*
		 * select a random position to start with to help wear leveling
		 * SSD
		 */
		p->cluster_next = 1 + (prandom_u32() % p->highest_bit);

		cluster_info = vzalloc(DIV_ROUND_UP(maxpages,
			SWAPFILE_CLUSTER) * sizeof(*cluster_info));
		if (!cluster_info) {
			error = -ENOMEM;
			goto bad_swap;
		}
		p->percpu_cluster = alloc_percpu(struct percpu_cluster);
		if (!p->percpu_cluster) {
			error = -ENOMEM;
			goto bad_swap;
		}
		for_each_possible_cpu(i) {
			struct percpu_cluster *cluster;
			cluster = per_cpu_ptr(p->percpu_cluster, i);
			cluster_set_null(&cluster->index);
		}
	}

	error = swap_cgroup_swapon(p->type, maxpages);
	if (error)
		goto bad_swap;

	nr_extents = setup_swap_map_and_extents(p, swap_header, swap_map,
		cluster_info, maxpages, &span);
	if (unlikely(nr_extents < 0)) {
		error = nr_extents;
		goto bad_swap;
	}
	/* frontswap enabled? set up bit-per-page map for frontswap */
	if (frontswap_enabled)
		frontswap_map = vzalloc(BITS_TO_LONGS(maxpages) * sizeof(long));

	if (p->bdev &&(swap_flags & SWAP_FLAG_DISCARD) && swap_discardable(p)) {
		/*
		 * When discard is enabled for swap with no particular
		 * policy flagged, we set all swap discard flags here in
		 * order to sustain backward compatibility with older
		 * swapon(8) releases.
		 */
		p->flags |= (SWP_DISCARDABLE | SWP_AREA_DISCARD |
			     SWP_PAGE_DISCARD);

		/*
		 * By flagging sys_swapon, a sysadmin can tell us to
		 * either do single-time area discards only, or to just
		 * perform discards for released swap page-clusters.
		 * Now it's time to adjust the p->flags accordingly.
		 */
		if (swap_flags & SWAP_FLAG_DISCARD_ONCE)
			p->flags &= ~SWP_PAGE_DISCARD;
		else if (swap_flags & SWAP_FLAG_DISCARD_PAGES)
			p->flags &= ~SWP_AREA_DISCARD;

		/* issue a swapon-time discard if it's still required */
		if (p->flags & SWP_AREA_DISCARD) {
			int err = discard_swap(p);
			if (unlikely(err))
				pr_err("swapon: discard_swap(%p): %d\n",
					p, err);
		}
	}

	mutex_lock(&swapon_mutex);
	prio = -1;
	if (swap_flags & SWAP_FLAG_PREFER)
		prio =
		  (swap_flags & SWAP_FLAG_PRIO_MASK) >> SWAP_FLAG_PRIO_SHIFT;
	enable_swap_info(p, prio, swap_map, cluster_info, frontswap_map);

	pr_info("Adding %uk swap on %s.  "
			"Priority:%d extents:%d across:%lluk %s%s%s%s%s\n",
		p->pages<<(PAGE_SHIFT-10), name->name, p->prio,
		nr_extents, (unsigned long long)span<<(PAGE_SHIFT-10),
		(p->flags & SWP_SOLIDSTATE) ? "SS" : "",
		(p->flags & SWP_DISCARDABLE) ? "D" : "",
		(p->flags & SWP_AREA_DISCARD) ? "s" : "",
		(p->flags & SWP_PAGE_DISCARD) ? "c" : "",
		(frontswap_map) ? "FS" : "");

	mutex_unlock(&swapon_mutex);
	atomic_inc(&proc_poll_event);
	wake_up_interruptible(&proc_poll_wait);

	if (S_ISREG(inode->i_mode))
		inode->i_flags |= S_SWAPFILE;
	error = 0;
	goto out;
bad_swap:
	free_percpu(p->percpu_cluster);
	p->percpu_cluster = NULL;
	if (inode && S_ISBLK(inode->i_mode) && p->bdev) {
		set_blocksize(p->bdev, p->old_block_size);
		blkdev_put(p->bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	}
	destroy_swap_extents(p);
	swap_cgroup_swapoff(p->type);
	spin_lock(&swap_lock);
	p->swap_file = NULL;
	p->flags = 0;
	spin_unlock(&swap_lock);
	vfree(swap_map);
	vfree(cluster_info);
	if (swap_file) {
		if (inode && S_ISREG(inode->i_mode)) {
			mutex_unlock(&inode->i_mutex);
			inode = NULL;
		}
		filp_close(swap_file, NULL);
	}
out:
	if (page && !IS_ERR(page)) {
		kunmap(page);
		page_cache_release(page);
	}
	if (name)
		putname(name);
	if (inode && S_ISREG(inode->i_mode))
		mutex_unlock(&inode->i_mutex);
	return error;
}

void si_swapinfo(struct sysinfo *val)
{
	unsigned int type;
	unsigned long nr_to_be_unused = 0;

	spin_lock(&swap_lock);
	for (type = 0; type < nr_swapfiles; type++) {
		struct swap_info_struct *si = swap_info[type];

		if ((si->flags & SWP_USED) && !(si->flags & SWP_WRITEOK))
			nr_to_be_unused += si->inuse_pages;
	}
	val->freeswap = atomic_long_read(&nr_swap_pages) + nr_to_be_unused;
	val->totalswap = total_swap_pages + nr_to_be_unused;
	spin_unlock(&swap_lock);
}

/*
 * Verify that a swap entry is valid and increment its swap map count.
 *
 * Returns error code in following case.
 * - success -> 0
 * - swp_entry is invalid -> EINVAL
 * - swp_entry is migration entry -> EINVAL
 * - swap-cache reference is requested but there is already one. -> EEXIST
 * - swap-cache reference is requested but the entry is not used. -> ENOENT
 * - swap-mapped reference requested but needs continued swap count. -> ENOMEM
 */
static int __swap_duplicate(swp_entry_t entry, unsigned char usage)
{
	struct swap_info_struct *p;
	unsigned long offset, type;
	unsigned char count;
	unsigned char has_cache;
	int err = -EINVAL;

	if (non_swap_entry(entry))
		goto out;

	type = swp_type(entry);
	if (type >= nr_swapfiles)
		goto bad_file;
	p = swap_info[type];
	offset = swp_offset(entry);

	spin_lock(&p->lock);
	if (unlikely(offset >= p->max))
		goto unlock_out;

	count = p->swap_map[offset];

	/*
	 * swapin_readahead() doesn't check if a swap entry is valid, so the
	 * swap entry could be SWAP_MAP_BAD. Check here with lock held.
	 */
	if (unlikely(swap_count(count) == SWAP_MAP_BAD)) {
		err = -ENOENT;
		goto unlock_out;
	}

	has_cache = count & SWAP_HAS_CACHE;
	count &= ~SWAP_HAS_CACHE;
	err = 0;

	if (usage == SWAP_HAS_CACHE) {

		/* set SWAP_HAS_CACHE if there is no cache and entry is used */
		if (!has_cache && count)
			has_cache = SWAP_HAS_CACHE;
		else if (has_cache)		/* someone else added cache */
			err = -EEXIST;
		else				/* no users remaining */
			err = -ENOENT;

	} else if (count || has_cache) {

		if ((count & ~COUNT_CONTINUED) < SWAP_MAP_MAX)
			count += usage;
		else if ((count & ~COUNT_CONTINUED) > SWAP_MAP_MAX)
			err = -EINVAL;
		else if (swap_count_continued(p, offset, count))
			count = COUNT_CONTINUED;
		else
			err = -ENOMEM;
	} else
		err = -ENOENT;			/* unused swap entry */

	p->swap_map[offset] = count | has_cache;

unlock_out:
	spin_unlock(&p->lock);
out:
	return err;

bad_file:
	pr_err("swap_dup: %s%08lx\n", Bad_file, entry.val);
	goto out;
}

/*
 * Help swapoff by noting that swap entry belongs to shmem/tmpfs
 * (in which case its reference count is never incremented).
 */
void swap_shmem_alloc(swp_entry_t entry)
{
	__swap_duplicate(entry, SWAP_MAP_SHMEM);
}

/*
 * Increase reference count of swap entry by 1.
 * Returns 0 for success, or -ENOMEM if a swap_count_continuation is required
 * but could not be atomically allocated.  Returns 0, just as if it succeeded,
 * if __swap_duplicate() fails for another reason (-EINVAL or -ENOENT), which
 * might occur if a page table entry has got corrupted.
 */
int swap_duplicate(swp_entry_t entry)
{
	int err = 0;

	while (!err && __swap_duplicate(entry, 1) == -ENOMEM)
		err = add_swap_count_continuation(entry, GFP_ATOMIC);
	return err;
}

/*
 * @entry: swap entry for which we allocate swap cache.
 *
 * Called when allocating swap cache for existing swap entry,
 * This can return error codes. Returns 0 at success.
 * -EBUSY means there is a swap cache.
 * Note: return code is different from swap_duplicate().
 */
int swapcache_prepare(swp_entry_t entry)
{
	return __swap_duplicate(entry, SWAP_HAS_CACHE);
}

struct swap_info_struct *page_swap_info(struct page *page)
{
	swp_entry_t swap = { .val = page_private(page) };
	BUG_ON(!PageSwapCache(page));
	return swap_info[swp_type(swap)];
}

/*
 * out-of-line __page_file_ methods to avoid include hell.
 */
struct address_space *__page_file_mapping(struct page *page)
{
	VM_BUG_ON_PAGE(!PageSwapCache(page), page);
	return page_swap_info(page)->swap_file->f_mapping;
}
EXPORT_SYMBOL_GPL(__page_file_mapping);

pgoff_t __page_file_index(struct page *page)
{
	swp_entry_t swap = { .val = page_private(page) };
	VM_BUG_ON_PAGE(!PageSwapCache(page), page);
	return swp_offset(swap);
}
EXPORT_SYMBOL_GPL(__page_file_index);

/*
 * add_swap_count_continuation - called when a swap count is duplicated
 * beyond SWAP_MAP_MAX, it allocates a new page and links that to the entry's
 * page of the original vmalloc'ed swap_map, to hold the continuation count
 * (for that entry and for its neighbouring PAGE_SIZE swap entries).  Called
 * again when count is duplicated beyond SWAP_MAP_MAX * SWAP_CONT_MAX, etc.
 *
 * These continuation pages are seldom referenced: the common paths all work
 * on the original swap_map, only referring to a continuation page when the
 * low "digit" of a count is incremented or decremented through SWAP_MAP_MAX.
 *
 * add_swap_count_continuation(, GFP_ATOMIC) can be called while holding
 * page table locks; if it fails, add_swap_count_continuation(, GFP_KERNEL)
 * can be called after dropping locks.
 */
int add_swap_count_continuation(swp_entry_t entry, gfp_t gfp_mask)
{
	struct swap_info_struct *si;
	struct page *head;
	struct page *page;
	struct page *list_page;
	pgoff_t offset;
	unsigned char count;

	/*
	 * When debugging, it's easier to use __GFP_ZERO here; but it's better
	 * for latency not to zero a page while GFP_ATOMIC and holding locks.
	 */
	page = alloc_page(gfp_mask | __GFP_HIGHMEM);

	si = swap_info_get(entry);
	if (!si) {
		/*
		 * An acceptable race has occurred since the failing
		 * __swap_duplicate(): the swap entry has been freed,
		 * perhaps even the whole swap_map cleared for swapoff.
		 */
		goto outer;
	}

	offset = swp_offset(entry);
	count = si->swap_map[offset] & ~SWAP_HAS_CACHE;

	if ((count & ~COUNT_CONTINUED) != SWAP_MAP_MAX) {
		/*
		 * The higher the swap count, the more likely it is that tasks
		 * will race to add swap count continuation: we need to avoid
		 * over-provisioning.
		 */
		goto out;
	}

	if (!page) {
		spin_unlock(&si->lock);
		return -ENOMEM;
	}

	/*
	 * We are fortunate that although vmalloc_to_page uses pte_offset_map,
	 * no architecture is using highmem pages for kernel page tables: so it
	 * will not corrupt the GFP_ATOMIC caller's atomic page table kmaps.
	 */
	head = vmalloc_to_page(si->swap_map + offset);
	offset &= ~PAGE_MASK;

	/*
	 * Page allocation does not initialize the page's lru field,
	 * but it does always reset its private field.
	 */
	if (!page_private(head)) {
		BUG_ON(count & COUNT_CONTINUED);
		INIT_LIST_HEAD(&head->lru);
		set_page_private(head, SWP_CONTINUED);
		si->flags |= SWP_CONTINUED;
	}

	list_for_each_entry(list_page, &head->lru, lru) {
		unsigned char *map;

		/*
		 * If the previous map said no continuation, but we've found
		 * a continuation page, free our allocation and use this one.
		 */
		if (!(count & COUNT_CONTINUED))
			goto out;

		map = kmap_atomic(list_page) + offset;
		count = *map;
		kunmap_atomic(map);

		/*
		 * If this continuation count now has some space in it,
		 * free our allocation and use this one.
		 */
		if ((count & ~COUNT_CONTINUED) != SWAP_CONT_MAX)
			goto out;
	}

	list_add_tail(&page->lru, &head->lru);
	page = NULL;			/* now it's attached, don't free it */
out:
	spin_unlock(&si->lock);
outer:
	if (page)
		__free_page(page);
	return 0;
}

/*
 * swap_count_continued - when the original swap_map count is incremented
 * from SWAP_MAP_MAX, check if there is already a continuation page to carry
 * into, carry if so, or else fail until a new continuation page is allocated;
 * when the original swap_map count is decremented from 0 with continuation,
 * borrow from the continuation and report whether it still holds more.
 * Called while __swap_duplicate() or swap_entry_free() holds swap_lock.
 */
static bool swap_count_continued(struct swap_info_struct *si,
				 pgoff_t offset, unsigned char count)
{
	struct page *head;
	struct page *page;
	unsigned char *map;

	head = vmalloc_to_page(si->swap_map + offset);
	if (page_private(head) != SWP_CONTINUED) {
		BUG_ON(count & COUNT_CONTINUED);
		return false;		/* need to add count continuation */
	}

	offset &= ~PAGE_MASK;
	page = list_entry(head->lru.next, struct page, lru);
	map = kmap_atomic(page) + offset;

	if (count == SWAP_MAP_MAX)	/* initial increment from swap_map */
		goto init_map;		/* jump over SWAP_CONT_MAX checks */

	if (count == (SWAP_MAP_MAX | COUNT_CONTINUED)) { /* incrementing */
		/*
		 * Think of how you add 1 to 999
		 */
		while (*map == (SWAP_CONT_MAX | COUNT_CONTINUED)) {
			kunmap_atomic(map);
			page = list_entry(page->lru.next, struct page, lru);
			BUG_ON(page == head);
			map = kmap_atomic(page) + offset;
		}
		if (*map == SWAP_CONT_MAX) {
			kunmap_atomic(map);
			page = list_entry(page->lru.next, struct page, lru);
			if (page == head)
				return false;	/* add count continuation */
			map = kmap_atomic(page) + offset;
init_map:		*map = 0;		/* we didn't zero the page */
		}
		*map += 1;
		kunmap_atomic(map);
		page = list_entry(page->lru.prev, struct page, lru);
		while (page != head) {
			map = kmap_atomic(page) + offset;
			*map = COUNT_CONTINUED;
			kunmap_atomic(map);
			page = list_entry(page->lru.prev, struct page, lru);
		}
		return true;			/* incremented */

	} else {				/* decrementing */
		/*
		 * Think of how you subtract 1 from 1000
		 */
		BUG_ON(count != COUNT_CONTINUED);
		while (*map == COUNT_CONTINUED) {
			kunmap_atomic(map);
			page = list_entry(page->lru.next, struct page, lru);
			BUG_ON(page == head);
			map = kmap_atomic(page) + offset;
		}
		BUG_ON(*map == 0);
		*map -= 1;
		if (*map == 0)
			count = 0;
		kunmap_atomic(map);
		page = list_entry(page->lru.prev, struct page, lru);
		while (page != head) {
			map = kmap_atomic(page) + offset;
			*map = SWAP_CONT_MAX | count;
			count = COUNT_CONTINUED;
			kunmap_atomic(map);
			page = list_entry(page->lru.prev, struct page, lru);
		}
		return count == COUNT_CONTINUED;
	}
}

/*
 * free_swap_count_continuations - swapoff free all the continuation pages
 * appended to the swap_map, after swap_map is quiesced, before vfree'ing it.
 */
static void free_swap_count_continuations(struct swap_info_struct *si)
{
	pgoff_t offset;

	for (offset = 0; offset < si->max; offset += PAGE_SIZE) {
		struct page *head;
		head = vmalloc_to_page(si->swap_map + offset);
		if (page_private(head)) {
			struct list_head *this, *next;
			list_for_each_safe(this, next, &head->lru) {
				struct page *page;
				page = list_entry(this, struct page, lru);
				list_del(this);
				__free_page(page);
			}
		}
	}
}
