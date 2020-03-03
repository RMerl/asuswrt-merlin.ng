#ifndef _LINUX_HUGETLB_H
#define _LINUX_HUGETLB_H

#include <linux/mm_types.h>
#include <linux/mmdebug.h>
#include <linux/fs.h>
#include <linux/hugetlb_inline.h>
#include <linux/cgroup.h>
#include <linux/list.h>
#include <linux/kref.h>

struct ctl_table;
struct user_struct;
struct mmu_gather;

#ifdef CONFIG_HUGETLB_PAGE

#include <linux/mempolicy.h>
#include <linux/shm.h>
#include <asm/tlbflush.h>

struct hugepage_subpool {
	spinlock_t lock;
	long count;
	long max_hpages;	/* Maximum huge pages or -1 if no maximum. */
	long used_hpages;	/* Used count against maximum, includes */
				/* both alloced and reserved pages. */
	struct hstate *hstate;
	long min_hpages;	/* Minimum huge pages or -1 if no minimum. */
	long rsv_hpages;	/* Pages reserved against global pool to */
				/* sasitfy minimum size. */
};

struct resv_map {
	struct kref refs;
	spinlock_t lock;
	struct list_head regions;
};
extern struct resv_map *resv_map_alloc(void);
void resv_map_release(struct kref *ref);

extern spinlock_t hugetlb_lock;
extern int hugetlb_max_hstate __read_mostly;
#define for_each_hstate(h) \
	for ((h) = hstates; (h) < &hstates[hugetlb_max_hstate]; (h)++)

struct hugepage_subpool *hugepage_new_subpool(struct hstate *h, long max_hpages,
						long min_hpages);
void hugepage_put_subpool(struct hugepage_subpool *spool);

void reset_vma_resv_huge_pages(struct vm_area_struct *vma);
int hugetlb_sysctl_handler(struct ctl_table *, int, void __user *, size_t *, loff_t *);
int hugetlb_overcommit_handler(struct ctl_table *, int, void __user *, size_t *, loff_t *);
int hugetlb_treat_movable_handler(struct ctl_table *, int, void __user *, size_t *, loff_t *);

#ifdef CONFIG_NUMA
int hugetlb_mempolicy_sysctl_handler(struct ctl_table *, int,
					void __user *, size_t *, loff_t *);
#endif

int copy_hugetlb_page_range(struct mm_struct *, struct mm_struct *, struct vm_area_struct *);
long follow_hugetlb_page(struct mm_struct *, struct vm_area_struct *,
			 struct page **, struct vm_area_struct **,
			 unsigned long *, unsigned long *, long, unsigned int);
void unmap_hugepage_range(struct vm_area_struct *,
			  unsigned long, unsigned long, struct page *);
void __unmap_hugepage_range_final(struct mmu_gather *tlb,
			  struct vm_area_struct *vma,
			  unsigned long start, unsigned long end,
			  struct page *ref_page);
void __unmap_hugepage_range(struct mmu_gather *tlb, struct vm_area_struct *vma,
				unsigned long start, unsigned long end,
				struct page *ref_page);
void hugetlb_report_meminfo(struct seq_file *);
int hugetlb_report_node_meminfo(int, char *);
void hugetlb_show_meminfo(void);
unsigned long hugetlb_total_pages(void);
int hugetlb_fault(struct mm_struct *mm, struct vm_area_struct *vma,
			unsigned long address, unsigned int flags);
int hugetlb_reserve_pages(struct inode *inode, long from, long to,
						struct vm_area_struct *vma,
						vm_flags_t vm_flags);
void hugetlb_unreserve_pages(struct inode *inode, long offset, long freed);
int dequeue_hwpoisoned_huge_page(struct page *page);
bool isolate_huge_page(struct page *page, struct list_head *list);
void putback_active_hugepage(struct page *page);
void free_huge_page(struct page *page);

#ifdef CONFIG_ARCH_WANT_HUGE_PMD_SHARE
pte_t *huge_pmd_share(struct mm_struct *mm, unsigned long addr, pud_t *pud);
#endif

extern int hugepages_treat_as_movable;
extern int sysctl_hugetlb_shm_group;
extern struct list_head huge_boot_pages;

/* arch callbacks */

pte_t *huge_pte_alloc(struct mm_struct *mm,
			unsigned long addr, unsigned long sz);
pte_t *huge_pte_offset(struct mm_struct *mm, unsigned long addr);
int huge_pmd_unshare(struct mm_struct *mm, unsigned long *addr, pte_t *ptep);
struct page *follow_huge_addr(struct mm_struct *mm, unsigned long address,
			      int write);
struct page *follow_huge_pmd(struct mm_struct *mm, unsigned long address,
				pmd_t *pmd, int flags);
struct page *follow_huge_pud(struct mm_struct *mm, unsigned long address,
				pud_t *pud, int flags);
int pmd_huge(pmd_t pmd);
int pud_huge(pud_t pmd);
unsigned long hugetlb_change_protection(struct vm_area_struct *vma,
		unsigned long address, unsigned long end, pgprot_t newprot);

#else /* !CONFIG_HUGETLB_PAGE */

static inline void reset_vma_resv_huge_pages(struct vm_area_struct *vma)
{
}

static inline unsigned long hugetlb_total_pages(void)
{
	return 0;
}

#define follow_hugetlb_page(m,v,p,vs,a,b,i,w)	({ BUG(); 0; })
#define follow_huge_addr(mm, addr, write)	ERR_PTR(-EINVAL)
#define copy_hugetlb_page_range(src, dst, vma)	({ BUG(); 0; })
static inline void hugetlb_report_meminfo(struct seq_file *m)
{
}
#define hugetlb_report_node_meminfo(n, buf)	0
static inline void hugetlb_show_meminfo(void)
{
}
#define follow_huge_pmd(mm, addr, pmd, flags)	NULL
#define follow_huge_pud(mm, addr, pud, flags)	NULL
#define prepare_hugepage_range(file, addr, len)	(-EINVAL)
#define pmd_huge(x)	0
#define pud_huge(x)	0
#define is_hugepage_only_range(mm, addr, len)	0
#define hugetlb_free_pgd_range(tlb, addr, end, floor, ceiling) ({BUG(); 0; })
#define hugetlb_fault(mm, vma, addr, flags)	({ BUG(); 0; })
#define huge_pte_offset(mm, address)	0
static inline int dequeue_hwpoisoned_huge_page(struct page *page)
{
	return 0;
}

static inline bool isolate_huge_page(struct page *page, struct list_head *list)
{
	return false;
}
#define putback_active_hugepage(p)	do {} while (0)

static inline unsigned long hugetlb_change_protection(struct vm_area_struct *vma,
		unsigned long address, unsigned long end, pgprot_t newprot)
{
	return 0;
}

static inline void __unmap_hugepage_range_final(struct mmu_gather *tlb,
			struct vm_area_struct *vma, unsigned long start,
			unsigned long end, struct page *ref_page)
{
	BUG();
}

static inline void __unmap_hugepage_range(struct mmu_gather *tlb,
			struct vm_area_struct *vma, unsigned long start,
			unsigned long end, struct page *ref_page)
{
	BUG();
}

#endif /* !CONFIG_HUGETLB_PAGE */
/*
 * hugepages at page global directory. If arch support
 * hugepages at pgd level, they need to define this.
 */
#ifndef pgd_huge
#define pgd_huge(x)	0
#endif

#ifndef pgd_write
static inline int pgd_write(pgd_t pgd)
{
	BUG();
	return 0;
}
#endif

#ifndef pud_write
static inline int pud_write(pud_t pud)
{
	BUG();
	return 0;
}
#endif

#ifndef is_hugepd
/*
 * Some architectures requires a hugepage directory format that is
 * required to support multiple hugepage sizes. For example
 * a4fe3ce76 "powerpc/mm: Allow more flexible layouts for hugepage pagetables"
 * introduced the same on powerpc. This allows for a more flexible hugepage
 * pagetable layout.
 */
typedef struct { unsigned long pd; } hugepd_t;
#define is_hugepd(hugepd) (0)
#define __hugepd(x) ((hugepd_t) { (x) })
static inline int gup_huge_pd(hugepd_t hugepd, unsigned long addr,
			      unsigned pdshift, unsigned long end,
			      int write, struct page **pages, int *nr)
{
	return 0;
}
#else
extern int gup_huge_pd(hugepd_t hugepd, unsigned long addr,
		       unsigned pdshift, unsigned long end,
		       int write, struct page **pages, int *nr);
#endif

#define HUGETLB_ANON_FILE "anon_hugepage"

enum {
	/*
	 * The file will be used as an shm file so shmfs accounting rules
	 * apply
	 */
	HUGETLB_SHMFS_INODE     = 1,
	/*
	 * The file is being created on the internal vfs mount and shmfs
	 * accounting rules do not apply
	 */
	HUGETLB_ANONHUGE_INODE  = 2,
};

#ifdef CONFIG_HUGETLBFS
struct hugetlbfs_sb_info {
	long	max_inodes;   /* inodes allowed */
	long	free_inodes;  /* inodes free */
	spinlock_t	stat_lock;
	struct hstate *hstate;
	struct hugepage_subpool *spool;
};

static inline struct hugetlbfs_sb_info *HUGETLBFS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

extern const struct file_operations hugetlbfs_file_operations;
extern const struct vm_operations_struct hugetlb_vm_ops;
struct file *hugetlb_file_setup(const char *name, size_t size, vm_flags_t acct,
				struct user_struct **user, int creat_flags,
				int page_size_log);

static inline int is_file_hugepages(struct file *file)
{
	if (file->f_op == &hugetlbfs_file_operations)
		return 1;
	if (is_file_shm_hugepages(file))
		return 1;

	return 0;
}


#else /* !CONFIG_HUGETLBFS */

#define is_file_hugepages(file)			0
static inline struct file *
hugetlb_file_setup(const char *name, size_t size, vm_flags_t acctflag,
		struct user_struct **user, int creat_flags,
		int page_size_log)
{
	return ERR_PTR(-ENOSYS);
}

#endif /* !CONFIG_HUGETLBFS */

#ifdef HAVE_ARCH_HUGETLB_UNMAPPED_AREA
unsigned long hugetlb_get_unmapped_area(struct file *file, unsigned long addr,
					unsigned long len, unsigned long pgoff,
					unsigned long flags);
#endif /* HAVE_ARCH_HUGETLB_UNMAPPED_AREA */

#ifdef CONFIG_HUGETLB_PAGE

#define HSTATE_NAME_LEN 32
/* Defines one hugetlb page size */
struct hstate {
	int next_nid_to_alloc;
	int next_nid_to_free;
	unsigned int order;
	unsigned long mask;
	unsigned long max_huge_pages;
	unsigned long nr_huge_pages;
	unsigned long free_huge_pages;
	unsigned long resv_huge_pages;
	unsigned long surplus_huge_pages;
	unsigned long nr_overcommit_huge_pages;
	struct list_head hugepage_activelist;
	struct list_head hugepage_freelists[MAX_NUMNODES];
	unsigned int nr_huge_pages_node[MAX_NUMNODES];
	unsigned int free_huge_pages_node[MAX_NUMNODES];
	unsigned int surplus_huge_pages_node[MAX_NUMNODES];
#ifdef CONFIG_CGROUP_HUGETLB
	/* cgroup control files */
	struct cftype cgroup_files[5];
#endif
	char name[HSTATE_NAME_LEN];
};

struct huge_bootmem_page {
	struct list_head list;
	struct hstate *hstate;
#ifdef CONFIG_HIGHMEM
	phys_addr_t phys;
#endif
};

struct page *alloc_huge_page_node(struct hstate *h, int nid);
struct page *alloc_huge_page_noerr(struct vm_area_struct *vma,
				unsigned long addr, int avoid_reserve);

/* arch callback */
int __init alloc_bootmem_huge_page(struct hstate *h);

void __init hugetlb_add_hstate(unsigned order);
struct hstate *size_to_hstate(unsigned long size);

#ifndef HUGE_MAX_HSTATE
#define HUGE_MAX_HSTATE 1
#endif

extern struct hstate hstates[HUGE_MAX_HSTATE];
extern unsigned int default_hstate_idx;

#define default_hstate (hstates[default_hstate_idx])

static inline struct hstate *hstate_inode(struct inode *i)
{
	struct hugetlbfs_sb_info *hsb;
	hsb = HUGETLBFS_SB(i->i_sb);
	return hsb->hstate;
}

static inline struct hstate *hstate_file(struct file *f)
{
	return hstate_inode(file_inode(f));
}

static inline struct hstate *hstate_sizelog(int page_size_log)
{
	if (!page_size_log)
		return &default_hstate;

	return size_to_hstate(1UL << page_size_log);
}

static inline struct hstate *hstate_vma(struct vm_area_struct *vma)
{
	return hstate_file(vma->vm_file);
}

static inline unsigned long huge_page_size(struct hstate *h)
{
	return (unsigned long)PAGE_SIZE << h->order;
}

extern unsigned long vma_kernel_pagesize(struct vm_area_struct *vma);

extern unsigned long vma_mmu_pagesize(struct vm_area_struct *vma);

static inline unsigned long huge_page_mask(struct hstate *h)
{
	return h->mask;
}

static inline unsigned int huge_page_order(struct hstate *h)
{
	return h->order;
}

static inline unsigned huge_page_shift(struct hstate *h)
{
	return h->order + PAGE_SHIFT;
}

static inline bool hstate_is_gigantic(struct hstate *h)
{
	return huge_page_order(h) >= MAX_ORDER;
}

static inline unsigned int pages_per_huge_page(struct hstate *h)
{
	return 1 << h->order;
}

static inline unsigned int blocks_per_huge_page(struct hstate *h)
{
	return huge_page_size(h) / 512;
}

#include <asm/hugetlb.h>

#ifndef arch_make_huge_pte
static inline pte_t arch_make_huge_pte(pte_t entry, struct vm_area_struct *vma,
				       struct page *page, int writable)
{
	return entry;
}
#endif

static inline struct hstate *page_hstate(struct page *page)
{
	VM_BUG_ON_PAGE(!PageHuge(page), page);
	return size_to_hstate(PAGE_SIZE << compound_order(page));
}

static inline unsigned hstate_index_to_shift(unsigned index)
{
	return hstates[index].order + PAGE_SHIFT;
}

static inline int hstate_index(struct hstate *h)
{
	return h - hstates;
}

pgoff_t __basepage_index(struct page *page);

/* Return page->index in PAGE_SIZE units */
static inline pgoff_t basepage_index(struct page *page)
{
	if (!PageCompound(page))
		return page->index;

	return __basepage_index(page);
}

extern void dissolve_free_huge_pages(unsigned long start_pfn,
				     unsigned long end_pfn);
static inline int hugepage_migration_supported(struct hstate *h)
{
#ifdef CONFIG_ARCH_ENABLE_HUGEPAGE_MIGRATION
	return huge_page_shift(h) == PMD_SHIFT;
#else
	return 0;
#endif
}

static inline spinlock_t *huge_pte_lockptr(struct hstate *h,
					   struct mm_struct *mm, pte_t *pte)
{
	if (huge_page_size(h) == PMD_SIZE)
		return pmd_lockptr(mm, (pmd_t *) pte);
	VM_BUG_ON(huge_page_size(h) == PAGE_SIZE);
	return &mm->page_table_lock;
}

#ifndef hugepages_supported
/*
 * Some platform decide whether they support huge pages at boot
 * time. Some of them, such as powerpc, set HPAGE_SHIFT to 0
 * when there is no such support
 */
#define hugepages_supported() (HPAGE_SHIFT != 0)
#endif

#else	/* CONFIG_HUGETLB_PAGE */
struct hstate {};
#define alloc_huge_page_node(h, nid) NULL
#define alloc_huge_page_noerr(v, a, r) NULL
#define alloc_bootmem_huge_page(h) NULL
#define hstate_file(f) NULL
#define hstate_sizelog(s) NULL
#define hstate_vma(v) NULL
#define hstate_inode(i) NULL
#define page_hstate(page) NULL
#define huge_page_size(h) PAGE_SIZE
#define huge_page_mask(h) PAGE_MASK
#define vma_kernel_pagesize(v) PAGE_SIZE
#define vma_mmu_pagesize(v) PAGE_SIZE
#define huge_page_order(h) 0
#define huge_page_shift(h) PAGE_SHIFT
static inline unsigned int pages_per_huge_page(struct hstate *h)
{
	return 1;
}
#define hstate_index_to_shift(index) 0
#define hstate_index(h) 0

static inline pgoff_t basepage_index(struct page *page)
{
	return page->index;
}
#define dissolve_free_huge_pages(s, e)	do {} while (0)
#define hugepage_migration_supported(h)	0

static inline spinlock_t *huge_pte_lockptr(struct hstate *h,
					   struct mm_struct *mm, pte_t *pte)
{
	return &mm->page_table_lock;
}
#endif	/* CONFIG_HUGETLB_PAGE */

static inline spinlock_t *huge_pte_lock(struct hstate *h,
					struct mm_struct *mm, pte_t *pte)
{
	spinlock_t *ptl;

	ptl = huge_pte_lockptr(h, mm, pte);
	spin_lock(ptl);
	return ptl;
}

#endif /* _LINUX_HUGETLB_H */
