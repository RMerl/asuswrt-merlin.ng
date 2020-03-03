/*
 * Macros for manipulating and testing page->flags
 */

#ifndef PAGE_FLAGS_H
#define PAGE_FLAGS_H

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/mmdebug.h>
#ifndef __GENERATING_BOUNDS_H
#include <linux/mm_types.h>
#include <generated/bounds.h>
#endif /* !__GENERATING_BOUNDS_H */

/*
 * Various page->flags bits:
 *
 * PG_reserved is set for special pages, which can never be swapped out. Some
 * of them might not even exist (eg empty_bad_page)...
 *
 * The PG_private bitflag is set on pagecache pages if they contain filesystem
 * specific data (which is normally at page->private). It can be used by
 * private allocations for its own usage.
 *
 * During initiation of disk I/O, PG_locked is set. This bit is set before I/O
 * and cleared when writeback _starts_ or when read _completes_. PG_writeback
 * is set before writeback starts and cleared when it finishes.
 *
 * PG_locked also pins a page in pagecache, and blocks truncation of the file
 * while it is held.
 *
 * page_waitqueue(page) is a wait queue of all tasks waiting for the page
 * to become unlocked.
 *
 * PG_uptodate tells whether the page's contents is valid.  When a read
 * completes, the page becomes uptodate, unless a disk I/O error happened.
 *
 * PG_referenced, PG_reclaim are used for page reclaim for anonymous and
 * file-backed pagecache (see mm/vmscan.c).
 *
 * PG_error is set to indicate that an I/O error occurred on this page.
 *
 * PG_arch_1 is an architecture specific page state bit.  The generic code
 * guarantees that this bit is cleared for a page when it first is entered into
 * the page cache.
 *
 * PG_highmem pages are not permanently mapped into the kernel virtual address
 * space, they need to be kmapped separately for doing IO on the pages.  The
 * struct page (these bits with information) are always mapped into kernel
 * address space...
 *
 * PG_hwpoison indicates that a page got corrupted in hardware and contains
 * data with incorrect ECC bits that triggered a machine check. Accessing is
 * not safe since it may cause another machine check. Don't touch!
 */

/*
 * Don't use the *_dontuse flags.  Use the macros.  Otherwise you'll break
 * locked- and dirty-page accounting.
 *
 * The page flags field is split into two parts, the main flags area
 * which extends from the low bits upwards, and the fields area which
 * extends from the high bits downwards.
 *
 *  | FIELD | ... | FLAGS |
 *  N-1           ^       0
 *               (NR_PAGEFLAGS)
 *
 * The fields area is reserved for fields mapping zone, node (for NUMA) and
 * SPARSEMEM section (for variants of SPARSEMEM that require section ids like
 * SPARSEMEM_EXTREME with !SPARSEMEM_VMEMMAP).
 */
enum pageflags {
	PG_locked,		/* Page is locked. Don't touch. */
	PG_error,
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_active,
	PG_slab,
	PG_owner_priv_1,	/* Owner use. If pagecache, fs may use*/
	PG_arch_1,
	PG_reserved,
	PG_private,		/* If pagecache, has fs-private data */
	PG_private_2,		/* If pagecache, has fs aux data */
	PG_writeback,		/* Page is under writeback */
#ifdef CONFIG_PAGEFLAGS_EXTENDED
	PG_head,		/* A head page */
	PG_tail,		/* A tail page */
#else
	PG_compound,		/* A compound page */
#endif
	PG_swapcache,		/* Swap page: swp_entry_t in private */
	PG_mappedtodisk,	/* Has blocks allocated on-disk */
	PG_reclaim,		/* To be reclaimed asap */
	PG_swapbacked,		/* Page is backed by RAM/swap */
	PG_unevictable,		/* Page is "unevictable"  */
#ifdef CONFIG_MMU
	PG_mlocked,		/* Page is vma mlocked */
#endif
#ifdef CONFIG_ARCH_USES_PG_UNCACHED
	PG_uncached,		/* Page has been mapped as uncached */
#endif
#ifdef CONFIG_MEMORY_FAILURE
	PG_hwpoison,		/* hardware poisoned page. Don't touch */
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	PG_compound_lock,
#endif
	__NR_PAGEFLAGS,

	/* Filesystems */
	PG_checked = PG_owner_priv_1,

	/* Two page bits are conscripted by FS-Cache to maintain local caching
	 * state.  These bits are set on pages belonging to the netfs's inodes
	 * when those inodes are being locally cached.
	 */
	PG_fscache = PG_private_2,	/* page backed by cache */

	/* XEN */
	/* Pinned in Xen as a read-only pagetable page. */
	PG_pinned = PG_owner_priv_1,
	/* Pinned as part of domain save (see xen_mm_pin_all()). */
	PG_savepinned = PG_dirty,
	/* Has a grant mapping of another (foreign) domain's page. */
	PG_foreign = PG_owner_priv_1,

	/* SLOB */
	PG_slob_free = PG_private,
};

#ifndef __GENERATING_BOUNDS_H

/*
 * Macros to create function definitions for page flags
 */
#define TESTPAGEFLAG(uname, lname)					\
static inline int Page##uname(const struct page *page)			\
			{ return test_bit(PG_##lname, &page->flags); }

#define SETPAGEFLAG(uname, lname)					\
static inline void SetPage##uname(struct page *page)			\
			{ set_bit(PG_##lname, &page->flags); }

#define CLEARPAGEFLAG(uname, lname)					\
static inline void ClearPage##uname(struct page *page)			\
			{ clear_bit(PG_##lname, &page->flags); }

#define __SETPAGEFLAG(uname, lname)					\
static inline void __SetPage##uname(struct page *page)			\
			{ __set_bit(PG_##lname, &page->flags); }

#define __CLEARPAGEFLAG(uname, lname)					\
static inline void __ClearPage##uname(struct page *page)		\
			{ __clear_bit(PG_##lname, &page->flags); }

#define TESTSETFLAG(uname, lname)					\
static inline int TestSetPage##uname(struct page *page)			\
		{ return test_and_set_bit(PG_##lname, &page->flags); }

#define TESTCLEARFLAG(uname, lname)					\
static inline int TestClearPage##uname(struct page *page)		\
		{ return test_and_clear_bit(PG_##lname, &page->flags); }

#define __TESTCLEARFLAG(uname, lname)					\
static inline int __TestClearPage##uname(struct page *page)		\
		{ return __test_and_clear_bit(PG_##lname, &page->flags); }

#define PAGEFLAG(uname, lname) TESTPAGEFLAG(uname, lname)		\
	SETPAGEFLAG(uname, lname) CLEARPAGEFLAG(uname, lname)

#define __PAGEFLAG(uname, lname) TESTPAGEFLAG(uname, lname)		\
	__SETPAGEFLAG(uname, lname)  __CLEARPAGEFLAG(uname, lname)

#define TESTSCFLAG(uname, lname)					\
	TESTSETFLAG(uname, lname) TESTCLEARFLAG(uname, lname)

#define TESTPAGEFLAG_FALSE(uname)					\
static inline int Page##uname(const struct page *page) { return 0; }

#define SETPAGEFLAG_NOOP(uname)						\
static inline void SetPage##uname(struct page *page) {  }

#define CLEARPAGEFLAG_NOOP(uname)					\
static inline void ClearPage##uname(struct page *page) {  }

#define __CLEARPAGEFLAG_NOOP(uname)					\
static inline void __ClearPage##uname(struct page *page) {  }

#define TESTSETFLAG_FALSE(uname)					\
static inline int TestSetPage##uname(struct page *page) { return 0; }

#define TESTCLEARFLAG_FALSE(uname)					\
static inline int TestClearPage##uname(struct page *page) { return 0; }

#define __TESTCLEARFLAG_FALSE(uname)					\
static inline int __TestClearPage##uname(struct page *page) { return 0; }

#define PAGEFLAG_FALSE(uname) TESTPAGEFLAG_FALSE(uname)			\
	SETPAGEFLAG_NOOP(uname) CLEARPAGEFLAG_NOOP(uname)

#define TESTSCFLAG_FALSE(uname)						\
	TESTSETFLAG_FALSE(uname) TESTCLEARFLAG_FALSE(uname)

struct page;	/* forward declaration */

TESTPAGEFLAG(Locked, locked)
PAGEFLAG(Error, error) TESTCLEARFLAG(Error, error)
PAGEFLAG(Referenced, referenced) TESTCLEARFLAG(Referenced, referenced)
	__SETPAGEFLAG(Referenced, referenced)
PAGEFLAG(Dirty, dirty) TESTSCFLAG(Dirty, dirty) __CLEARPAGEFLAG(Dirty, dirty)
PAGEFLAG(LRU, lru) __CLEARPAGEFLAG(LRU, lru)
PAGEFLAG(Active, active) __CLEARPAGEFLAG(Active, active)
	TESTCLEARFLAG(Active, active)
__PAGEFLAG(Slab, slab)
PAGEFLAG(Checked, checked)		/* Used by some filesystems */
PAGEFLAG(Pinned, pinned) TESTSCFLAG(Pinned, pinned)	/* Xen */
PAGEFLAG(SavePinned, savepinned);			/* Xen */
PAGEFLAG(Foreign, foreign);				/* Xen */
PAGEFLAG(Reserved, reserved) __CLEARPAGEFLAG(Reserved, reserved)
PAGEFLAG(SwapBacked, swapbacked) __CLEARPAGEFLAG(SwapBacked, swapbacked)
	__SETPAGEFLAG(SwapBacked, swapbacked)

__PAGEFLAG(SlobFree, slob_free)

/*
 * Private page markings that may be used by the filesystem that owns the page
 * for its own purposes.
 * - PG_private and PG_private_2 cause releasepage() and co to be invoked
 */
PAGEFLAG(Private, private) __SETPAGEFLAG(Private, private)
	__CLEARPAGEFLAG(Private, private)
PAGEFLAG(Private2, private_2) TESTSCFLAG(Private2, private_2)
PAGEFLAG(OwnerPriv1, owner_priv_1) TESTCLEARFLAG(OwnerPriv1, owner_priv_1)

/*
 * Only test-and-set exist for PG_writeback.  The unconditional operators are
 * risky: they bypass page accounting.
 */
TESTPAGEFLAG(Writeback, writeback) TESTSCFLAG(Writeback, writeback)
PAGEFLAG(MappedToDisk, mappedtodisk)

/* PG_readahead is only used for reads; PG_reclaim is only for writes */
PAGEFLAG(Reclaim, reclaim) TESTCLEARFLAG(Reclaim, reclaim)
PAGEFLAG(Readahead, reclaim) TESTCLEARFLAG(Readahead, reclaim)

#ifdef CONFIG_HIGHMEM
/*
 * Must use a macro here due to header dependency issues. page_zone() is not
 * available at this point.
 */
#define PageHighMem(__p) is_highmem(page_zone(__p))
#else
PAGEFLAG_FALSE(HighMem)
#endif

#ifdef CONFIG_SWAP
PAGEFLAG(SwapCache, swapcache)
#else
PAGEFLAG_FALSE(SwapCache)
#endif

PAGEFLAG(Unevictable, unevictable) __CLEARPAGEFLAG(Unevictable, unevictable)
	TESTCLEARFLAG(Unevictable, unevictable)

#ifdef CONFIG_MMU
PAGEFLAG(Mlocked, mlocked) __CLEARPAGEFLAG(Mlocked, mlocked)
	TESTSCFLAG(Mlocked, mlocked) __TESTCLEARFLAG(Mlocked, mlocked)
#else
PAGEFLAG_FALSE(Mlocked) __CLEARPAGEFLAG_NOOP(Mlocked)
	TESTSCFLAG_FALSE(Mlocked) __TESTCLEARFLAG_FALSE(Mlocked)
#endif

#ifdef CONFIG_ARCH_USES_PG_UNCACHED
PAGEFLAG(Uncached, uncached)
#else
PAGEFLAG_FALSE(Uncached)
#endif

#ifdef CONFIG_MEMORY_FAILURE
PAGEFLAG(HWPoison, hwpoison)
TESTSCFLAG(HWPoison, hwpoison)
#define __PG_HWPOISON (1UL << PG_hwpoison)
#else
PAGEFLAG_FALSE(HWPoison)
#define __PG_HWPOISON 0
#endif

/*
 * On an anonymous page mapped into a user virtual memory area,
 * page->mapping points to its anon_vma, not to a struct address_space;
 * with the PAGE_MAPPING_ANON bit set to distinguish it.  See rmap.h.
 *
 * On an anonymous page in a VM_MERGEABLE area, if CONFIG_KSM is enabled,
 * the PAGE_MAPPING_KSM bit may be set along with the PAGE_MAPPING_ANON bit;
 * and then page->mapping points, not to an anon_vma, but to a private
 * structure which KSM associates with that merged page.  See ksm.h.
 *
 * PAGE_MAPPING_KSM without PAGE_MAPPING_ANON is currently never used.
 *
 * Please note that, confusingly, "page_mapping" refers to the inode
 * address_space which maps the page from disk; whereas "page_mapped"
 * refers to user virtual address space into which the page is mapped.
 */
#define PAGE_MAPPING_ANON	1
#define PAGE_MAPPING_KSM	2
#define PAGE_MAPPING_FLAGS	(PAGE_MAPPING_ANON | PAGE_MAPPING_KSM)

static inline int PageAnon(struct page *page)
{
	return ((unsigned long)page->mapping & PAGE_MAPPING_ANON) != 0;
}

#ifdef CONFIG_KSM
/*
 * A KSM page is one of those write-protected "shared pages" or "merged pages"
 * which KSM maps into multiple mms, wherever identical anonymous page content
 * is found in VM_MERGEABLE vmas.  It's a PageAnon page, pointing not to any
 * anon_vma, but to that page's node of the stable tree.
 */
static inline int PageKsm(struct page *page)
{
	return ((unsigned long)page->mapping & PAGE_MAPPING_FLAGS) ==
				(PAGE_MAPPING_ANON | PAGE_MAPPING_KSM);
}
#else
TESTPAGEFLAG_FALSE(Ksm)
#endif

u64 stable_page_flags(struct page *page);

static inline int PageUptodate(struct page *page)
{
	int ret = test_bit(PG_uptodate, &(page)->flags);

	/*
	 * Must ensure that the data we read out of the page is loaded
	 * _after_ we've loaded page->flags to check for PageUptodate.
	 * We can skip the barrier if the page is not uptodate, because
	 * we wouldn't be reading anything from it.
	 *
	 * See SetPageUptodate() for the other side of the story.
	 */
	if (ret)
		smp_rmb();

	return ret;
}

static inline void __SetPageUptodate(struct page *page)
{
	smp_wmb();
	__set_bit(PG_uptodate, &(page)->flags);
}

static inline void SetPageUptodate(struct page *page)
{
	/*
	 * Memory barrier must be issued before setting the PG_uptodate bit,
	 * so that all previous stores issued in order to bring the page
	 * uptodate are actually visible before PageUptodate becomes true.
	 */
	smp_wmb();
	set_bit(PG_uptodate, &(page)->flags);
}

CLEARPAGEFLAG(Uptodate, uptodate)

int test_clear_page_writeback(struct page *page);
int __test_set_page_writeback(struct page *page, bool keep_write);

#define test_set_page_writeback(page)			\
	__test_set_page_writeback(page, false)
#define test_set_page_writeback_keepwrite(page)	\
	__test_set_page_writeback(page, true)

static inline void set_page_writeback(struct page *page)
{
	test_set_page_writeback(page);
}

static inline void set_page_writeback_keepwrite(struct page *page)
{
	test_set_page_writeback_keepwrite(page);
}

#ifdef CONFIG_PAGEFLAGS_EXTENDED
/*
 * System with lots of page flags available. This allows separate
 * flags for PageHead() and PageTail() checks of compound pages so that bit
 * tests can be used in performance sensitive paths. PageCompound is
 * generally not used in hot code paths except arch/powerpc/mm/init_64.c
 * and arch/powerpc/kvm/book3s_64_vio_hv.c which use it to detect huge pages
 * and avoid handling those in real mode.
 */
__PAGEFLAG(Head, head) CLEARPAGEFLAG(Head, head)
__PAGEFLAG(Tail, tail)

static inline int PageCompound(struct page *page)
{
	return page->flags & ((1L << PG_head) | (1L << PG_tail));

}
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static inline void ClearPageCompound(struct page *page)
{
	BUG_ON(!PageHead(page));
	ClearPageHead(page);
}
#endif

#define PG_head_mask ((1L << PG_head))

#else
/*
 * Reduce page flag use as much as possible by overlapping
 * compound page flags with the flags used for page cache pages. Possible
 * because PageCompound is always set for compound pages and not for
 * pages on the LRU and/or pagecache.
 */
TESTPAGEFLAG(Compound, compound)
__SETPAGEFLAG(Head, compound)  __CLEARPAGEFLAG(Head, compound)

/*
 * PG_reclaim is used in combination with PG_compound to mark the
 * head and tail of a compound page. This saves one page flag
 * but makes it impossible to use compound pages for the page cache.
 * The PG_reclaim bit would have to be used for reclaim or readahead
 * if compound pages enter the page cache.
 *
 * PG_compound & PG_reclaim	=> Tail page
 * PG_compound & ~PG_reclaim	=> Head page
 */
#define PG_head_mask ((1L << PG_compound))
#define PG_head_tail_mask ((1L << PG_compound) | (1L << PG_reclaim))

static inline int PageHead(struct page *page)
{
	return ((page->flags & PG_head_tail_mask) == PG_head_mask);
}

static inline int PageTail(struct page *page)
{
	return ((page->flags & PG_head_tail_mask) == PG_head_tail_mask);
}

static inline void __SetPageTail(struct page *page)
{
	page->flags |= PG_head_tail_mask;
}

static inline void __ClearPageTail(struct page *page)
{
	page->flags &= ~PG_head_tail_mask;
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static inline void ClearPageCompound(struct page *page)
{
	BUG_ON((page->flags & PG_head_tail_mask) != (1 << PG_compound));
	clear_bit(PG_compound, &page->flags);
}
#endif

#endif /* !PAGEFLAGS_EXTENDED */

#ifdef CONFIG_HUGETLB_PAGE
int PageHuge(struct page *page);
int PageHeadHuge(struct page *page);
bool page_huge_active(struct page *page);
#else
TESTPAGEFLAG_FALSE(Huge)
TESTPAGEFLAG_FALSE(HeadHuge)

static inline bool page_huge_active(struct page *page)
{
	return 0;
}
#endif


#ifdef CONFIG_TRANSPARENT_HUGEPAGE
/*
 * PageHuge() only returns true for hugetlbfs pages, but not for
 * normal or transparent huge pages.
 *
 * PageTransHuge() returns true for both transparent huge and
 * hugetlbfs pages, but not normal pages. PageTransHuge() can only be
 * called only in the core VM paths where hugetlbfs pages can't exist.
 */
static inline int PageTransHuge(struct page *page)
{
	VM_BUG_ON_PAGE(PageTail(page), page);
	return PageHead(page);
}

/*
 * PageTransCompound returns true for both transparent huge pages
 * and hugetlbfs pages, so it should only be called when it's known
 * that hugetlbfs pages aren't involved.
 */
static inline int PageTransCompound(struct page *page)
{
	return PageCompound(page);
}

/*
 * PageTransTail returns true for both transparent huge pages
 * and hugetlbfs pages, so it should only be called when it's known
 * that hugetlbfs pages aren't involved.
 */
static inline int PageTransTail(struct page *page)
{
	return PageTail(page);
}

#else

static inline int PageTransHuge(struct page *page)
{
	return 0;
}

static inline int PageTransCompound(struct page *page)
{
	return 0;
}

static inline int PageTransTail(struct page *page)
{
	return 0;
}
#endif

/*
 * PageBuddy() indicate that the page is free and in the buddy system
 * (see mm/page_alloc.c).
 *
 * PAGE_BUDDY_MAPCOUNT_VALUE must be <= -2 but better not too close to
 * -2 so that an underflow of the page_mapcount() won't be mistaken
 * for a genuine PAGE_BUDDY_MAPCOUNT_VALUE. -128 can be created very
 * efficiently by most CPU architectures.
 */
#define PAGE_BUDDY_MAPCOUNT_VALUE (-128)

static inline int PageBuddy(struct page *page)
{
	return atomic_read(&page->_mapcount) == PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void __SetPageBuddy(struct page *page)
{
	VM_BUG_ON_PAGE(atomic_read(&page->_mapcount) != -1, page);
	atomic_set(&page->_mapcount, PAGE_BUDDY_MAPCOUNT_VALUE);
}

static inline void __ClearPageBuddy(struct page *page)
{
	VM_BUG_ON_PAGE(!PageBuddy(page), page);
	atomic_set(&page->_mapcount, -1);
}

#define PAGE_BALLOON_MAPCOUNT_VALUE (-256)

static inline int PageBalloon(struct page *page)
{
	return atomic_read(&page->_mapcount) == PAGE_BALLOON_MAPCOUNT_VALUE;
}

static inline void __SetPageBalloon(struct page *page)
{
	VM_BUG_ON_PAGE(atomic_read(&page->_mapcount) != -1, page);
	atomic_set(&page->_mapcount, PAGE_BALLOON_MAPCOUNT_VALUE);
}

static inline void __ClearPageBalloon(struct page *page)
{
	VM_BUG_ON_PAGE(!PageBalloon(page), page);
	atomic_set(&page->_mapcount, -1);
}

/*
 * If network-based swap is enabled, sl*b must keep track of whether pages
 * were allocated from pfmemalloc reserves.
 */
static inline int PageSlabPfmemalloc(struct page *page)
{
	VM_BUG_ON_PAGE(!PageSlab(page), page);
	return PageActive(page);
}

static inline void SetPageSlabPfmemalloc(struct page *page)
{
	VM_BUG_ON_PAGE(!PageSlab(page), page);
	SetPageActive(page);
}

static inline void __ClearPageSlabPfmemalloc(struct page *page)
{
	VM_BUG_ON_PAGE(!PageSlab(page), page);
	__ClearPageActive(page);
}

static inline void ClearPageSlabPfmemalloc(struct page *page)
{
	VM_BUG_ON_PAGE(!PageSlab(page), page);
	ClearPageActive(page);
}

#ifdef CONFIG_MMU
#define __PG_MLOCKED		(1 << PG_mlocked)
#else
#define __PG_MLOCKED		0
#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
#define __PG_COMPOUND_LOCK		(1 << PG_compound_lock)
#else
#define __PG_COMPOUND_LOCK		0
#endif

/*
 * Flags checked when a page is freed.  Pages being freed should not have
 * these flags set.  It they are, there is a problem.
 */
#define PAGE_FLAGS_CHECK_AT_FREE \
	(1 << PG_lru	 | 1 << PG_locked    | \
	 1 << PG_private | 1 << PG_private_2 | \
	 1 << PG_writeback | 1 << PG_reserved | \
	 1 << PG_slab	 | 1 << PG_swapcache | 1 << PG_active | \
	 1 << PG_unevictable | __PG_MLOCKED | \
	 __PG_COMPOUND_LOCK)

/*
 * Flags checked when a page is prepped for return by the page allocator.
 * Pages being prepped should not have these flags set.  It they are set,
 * there has been a kernel bug or struct page corruption.
 *
 * __PG_HWPOISON is exceptional because it needs to be kept beyond page's
 * alloc-free cycle to prevent from reusing the page.
 */
#define PAGE_FLAGS_CHECK_AT_PREP	\
	(((1 << NR_PAGEFLAGS) - 1) & ~__PG_HWPOISON)

#define PAGE_FLAGS_PRIVATE				\
	(1 << PG_private | 1 << PG_private_2)
/**
 * page_has_private - Determine if page has private stuff
 * @page: The page to be checked
 *
 * Determine if a page has private stuff, indicating that release routines
 * should be invoked upon it.
 */
static inline int page_has_private(struct page *page)
{
	return !!(page->flags & PAGE_FLAGS_PRIVATE);
}

#endif /* !__GENERATING_BOUNDS_H */

#endif	/* PAGE_FLAGS_H */
