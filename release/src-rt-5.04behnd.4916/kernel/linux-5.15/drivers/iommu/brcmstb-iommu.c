// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022 Broadcom */

#include <linux/bitfield.h>
#include <linux/brcmstb/m3u_api.h>
#include <linux/delay.h>
#include <linux/dma-direct.h>
#include <linux/dma-iommu.h>
#include <linux/pci.h>
#include <linux/iommu.h>
#include <linux/iommu-helper.h>
#include <linux/iopoll.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/module.h>

/*
 * This IOMMU has a two level page table.  In both levels the PTE of a
 * table is a u32 but "bmmu_pte" is used here instead..  In the first
 * level, each l1 PTE may have one of two meanings
 *   (1) It points to 4MB of memory.
 *   (2) It points to an l2 table of 1024 entries, with each
 *       entry pointing to 4k of memory.
 *
 * The IOMMU supports the l2 tables having PTEs that point to pages of
 * different sizes; in the current implementation only 4K pages are used.
 *
 * The iova from the downstream device is broken down as follows:
 *
 *   iova[39..22] -- l1 table index
 *   iova[21..12] -- l2 table index (4K)
 *   iova[11..00] -- Offset
 *
 * LEGEND:
 *  V .... Valid bit
 *  W .... Writable
 *  A .... Address bit
 *  PA ... Physical address
 *  BPS .. Big page size
 *  SPS .. Super page size
 *
 * L1 ENTRY:
 *
 * 31     23                      0
 * |       |                      |  DESCRIPTION
 * |       |                      |
 * 00..............................  Invalid
 * 01WV....AAAAAAAAAAAAAAAAAAAAAAAA  Each entry points to a 4K l2 table
 * 10..............................  Invalid
 * 11WV....AAAAAAAAAAAAAAAAAAAAAAAA  Each entry is the translation for a 4MB page
 *
 * L2 ENTRY:
 *
 * 31     23                      0
 * |       |                      |   TYPE  PHYSICAL ADDRESS
 * |       |                      |
 * 00WV....AAAAAAAAAAAAAAAAAAAAAAAA     4K  iova[11:0] | (A[23:0] << 12)
 * 01WV....AAAAAAAAAAAAAAAAAAAAAAAA    BIG  iova[xx:0] | (A[23:0] << (12 + 1 + log2(BPS))
 * 10WV....AAAAAAAAAAAAAAAAAAAAAAAA  SUPER  iova[yy:0] | (A[23:0] << (12 + 1 + log2(SPS))
 * 11WV....AAAAAAAAAAAAAAAAAAAAAAAA     4M  iova[23:0] | (A[23:0] << 22)
 *
 *
 * L1 ENTRY STRUCT (aka struct brcm_entry)
 *
 *     l2_pa ........ Phys address of L2 table
 *     l2_dma ....... DMA address of L2 table
 *     l2_mmu_va .... Kernel virtual address of L2 table
 */


#define BPAGE_SIZE			SZ_4K
#define BPAGE_SHIFT			ilog2(BPAGE_SIZE)
#define BMMU_PGSIZE_BITMAP		((SZ_2M - 1) & ~(SZ_4K - 1))
#define BMMU_INVALID_PTE		0x0

/* This is an internal address that the BMMU understands */
#define BMMU_NULL_DEV_PAGE		(0x13FFFF0000ULL >> 12)

/* IOMMU register address and field mask definitions */
#define BREG_MMU_CTRL			0x00
#define	  BMASK_CAP_EXCEED		BIT(27)
#define	  BMASK_CAP_EXCEED_ABORT_EN	BIT(26)
#define	  BMASK_CAP_EXCEED_INT_EN	BIT(25)
#define	  BMASK_CAP_EXCEED_EXCP_EN	BIT(24)
#define	  BMASK_PT_INV			BIT(20)
#define	  BMASK_PT_INV_ABORT_EN		BIT(19)
#define	  BMASK_PT_INV_INT_EN		BIT(18)
#define	  BMASK_PT_INV_EXCP_EN		BIT(17)
#define	  BMASK_PT_INV_EN		BIT(16)
#define	  BMASK_WT_VIO			BIT(12)
#define	  BMASK_WT_VIO_ABORT_EN		BIT(11)
#define	  BMASK_WT_VIO_INT_EN		BIT(10)
#define	  BMASK_WT_VIO_EXCP_EN		BIT(9)
#define	  BMASK_BYPASS			BIT(8)
#define	  BMASK_TLB_CLR_BUSY		BIT(7)
#define	  BMASK_SPARE			GENMASK(5, 4)
#define	  BMASK_STATS_CLR		BIT(3)
#define	  BMASK_TLB_CLR			BIT(2)
#define	  BMASK_STATS_EN		BIT(1)
#define	  BMASK_EN			BIT(0)
#define BREG_MMU_PT_PA_BASE		0x04
#define   BMASK_PT_PA_BASE_PAGE		GENMASK(23, 0)
#define BREG_MMU_HITS			0x08
#define BREG_MMU_MISSES			0x0c
#define BREG_MMU_STALLS			0x10
#define BREG_MMU_ADDR_CAP		0x14
#define   BMASK_CAP_EN			GENMASK(31, 31)
#define   BMASK_CAP_MPAGE		GENMASK(11, 0)
#define   BMMU_CAP_IOVA_SHIFT		28
#define BREG_MMU_SHOOT_DOWN		0x18
#define   BMASK_SHOOT_BUSY		BIT(31)
#define   BMASK_SHOOT			BIT(30)
#define   BMASK_SHOOT_PAGE		GENMASK(28, 0)
#define BREG_MMU_BYPASS_START		0x1c
#define BREG_MMU_BYPASS_END		0x20
#define BREG_MMU_DEBUG_MISC		0x24
#define   BMASK_SINGLE_TABLE		BIT(31)
#define   BMASK_L2_TLB_LIMIT		GENMASK(13, 7)
#define   BMASK_L1_TLB_LIMIT		GENMASK(6, 2)
#define BREG_MMU_SECURITY		0x28
#define BREG_MMU_VIO_ID			0x2c
#define BREG_MMU_ILLEGAL_ADR		0x30
#define   BMASK_ILLEGAL_ADDR		GENMASK(30, 0)
#define   BMASK_ILLEGAL_ADDR_EN		BIT(31)
#define BREG_MMU_VIO_ADDR		0x34
#define BREG_MMU_DEBUG_INFO		0x38
#define   BMASK_VERSION			GENMASK(3, 0)
#define BREG_MMU_PT_PROT_PA_BASE	0x3c
#define BREG_MMU_VA_UNPROT_START	0x40
#define BREG_MMU_SECURE_READ		0x44
#define BREG_MMU_SECURE_WRITE		0x48

#define BREG_MMUC_CTRL			0x00
#define   BMASK_MMUC_VAR_SIZE		BIT(31)
#define   BMASK_MMUC_CACHE_SIZE		GENMASK(30, 29)
#define   BMASK_MMUC_CLR_STATS		BIT(11)
#define	  BMASK_MMUC_FLUSH_IDENT	GENMASK(7, 4)
#define	  BMASK_MMUC_FLUSH_IDENT_RANGE	BIT(3)
#define	  BMASK_MMUC_FLUSH_BUSY		BIT(2)
#define	  BMASK_MMUC_FLUSH		BIT(1)
#define	  BMASK_MMUC_EN			BIT(0)
#define BREG_MMUC_MISSES		0x04
#define BREG_MMUC_RWSTALLS		0x08
#define BREG_MMUC_NISTALLS		0x0c
#define BREG_MMUC_QFSTALLS		0x10
#define BREG_MMUC_HITS			0x14
#define BREG_MMUC_HCSTALL		0x18
#define BREG_MMUC_ARBCTRL		0x20
#define BREG_MMUC_SECURE		0x24

#define BREG_MIC_IDENT_CTRL		0x00
#define	  BMASK_ID_MIC_CTRL		GENMASK(1, 0)
#define BREG_MIC_IDENT0			0x04
#define BREG_MIC_IDENT1			0x08
#define BREG_MIC_IDENT(id)		(((id) > 7) ? BREG_MIC_IDENT1 : BREG_MIC_IDENT0)
#define	  BMASK_IDENT(i)		(0xf << (((i) & 0x7) * 4))
#define BREG_MIC_SECURE			0x0c
#define BREG_MIC_MMMU_FLUSH		0x10
#define	  BMASK_MIC_MMMU_IDENT		GENMASK(7, 4)
#define	  BMASK_MIC_MMMU_CTRL		BIT(2)
#define	  BMASK_MIC_MMMU_STATUS		BIT(1)
#define	  BMASK_MIC_MMMU_REQ		BIT(0)
#define BREG_MIC_MMC_FLUSH		0x14
#define	  BMASK_MIC_MMC_MMC_CTRL	BIT(10)
#define	  BMASK_MIC_MMC			GENMASK(9, 8)
#define	  BMASK_MIC_MMC_IDENT		GENMASK(7, 4)
#define	  BMASK_MIC_MMC_CTRL		BIT(2)
#define	  BMASK_MIC_MMC_STATUS		BIT(1)
#define	  BMASK_MIC_MMC_REQ		BIT(0)
#define BREG_MIC_MPU_CTRL		0x18
#define BREG_MIC_MPU_FLUSH		0x1c
#define BREG_MIC_ASG_CTRL		0x20
#define	  BMASK_MIC_INTERRUPTS		GENMASK(24, 15)

#define bmmu_pte			u32
#define   BMASK_PTE_TYPE		GENMASK(31, 30)
#define     BMMU_L1_PTE_TYPE_4K_PTR	1
#define     BMMU_L1_PTE_TYPE_4M_PAGE	3
#define     BMMU_L2_PTE_TYPE_4K_PAGE	0
#define     BMMU_L2_PTE_TYPE_BIG_PAGE	1
#define     BMMU_L2_PTE_TYPE_SUPER_PAGE	2
#define     BMMU_L2_PTE_TYPE_4M_PAGE	3
#define   BMASK_PTE_WRITEABLE		BIT(29)
#define   BMASK_PTE_VALID		BIT(28)
#define   BMASK_PTE_PAGE		GENMASK(23, 0)

#define BPTE_SIZE			sizeof(bmmu_pte)
#define BMMU_PTE_PER_PAGE		(BPAGE_SIZE / BPTE_SIZE)

#define bmmu_pte_type(pte)		FIELD_GET(BMASK_PTE_TYPE, (pte))
#define bmmu_pte_writeable(pte)		FIELD_GET(BMASK_PTE_WRITEABLE, (pte))
#define bmmu_pte_valid(pte)		FIELD_GET(BMASK_PTE_VALID, (pte))

#define bmmu_pte_4K_page(pte)		FIELD_GET(BMMU_PTE_4K_PAGE, (pte))
#define bmmu_pte_64K_page(pte)		FIELD_GET(BMMU_PTE_644K_PAGE, (pte))
#define bmmu_pte_1M_page(pte)		FIELD_GET(BMMU_PTE_1M_PAGE, (pte))
#define bmmu_pte_page(pte)		bmmu_pte_4K_page(pte)
#define bmmu_iova_to_l1_idx(iova)	FIELD_GET(GENMASK_ULL(39, 22), \
						  (unsigned long long)(iova))
#define bmmu_iova_to_l2_idx(iova)	FIELD_GET(GENMASK(21, 12), (iova))
#define bmmu_iova_4K_offset(iova)	FIELD_GET(GENMASK(11, 0), (iova))
#define bmmu_pte_to_phys(pte)		(FIELD_GET(BMASK_PTE_PAGE, (pte)) << \
					 BPAGE_SHIFT)
#define bmmu_idx_to_iova(l1, l2)	(FIELD_PREP(GENMASK_ULL(39, 22), \
						    (unsigned long long)(l1)) |	\
					 FIELD_PREP(GENMASK(21, 12), (l2)))

#define bmmu_adjust_iova(bmmu, iova)	(bmmu->type == BMMU_PCI \
					  ? iova - bmmu->iova_start : iova)



enum brcm_base_type {
	BREG_MMU,
	BREG_MMUC,
	BREG_MIC,
	/*----------*/
	BREG_TYPE_NUM,
};


enum brcm_iommu_type {
	BMMU_PCI = 0,
	BMMU_MISC,
	/*----------*/
	BMMU_TYPE_NUM,
};

static enum brcm_iommu_type brcm_iommu_str_to_type(const char *type_str)
{
	if (strcmp(type_str, "pci") == 0)
		return BMMU_PCI;
	else if (strcmp(type_str, "misc") == 0)
		return BMMU_MISC;

	return BMMU_TYPE_NUM; /* Illegal */
}

static const char *brcm_iommu_type_to_str(enum brcm_iommu_type type)
{
	static const char *type_strs[BMMU_TYPE_NUM] = {
		[BMMU_PCI] = "pci",
		[BMMU_MISC] = "misc",
	};
	return type >= BMMU_TYPE_NUM ? "unknown" : type_strs[type];
}



static DEFINE_MUTEX(bmmu_mutex);

/* This struct var (binfo) is overned by bmmu_mutex */
struct {
	struct iommu_domain *domain;
	struct iommu_group *group;
	struct iommu_device *device;
} binfo[BMMU_TYPE_NUM];

static bool pci_bus_type_is_set;

struct brcm_entry {
	phys_addr_t		l2_pa;
	dma_addr_t		l2_dma;
	bmmu_pte		*l2_mmu_va;
};

struct brcm_domain {
	struct iommu_domain	domain;
	struct iommu_group	*group;
	struct brcm_iommu	*iommu;
	u64			iova_start;
	u64			iova_size;
};

static const struct brcm_entry BMMU_INVALID_ENTRY = {
	.l2_dma		= DMA_MAPPING_ERROR,
	.l2_pa		= 0,
	.l2_mmu_va	= 0,
};


struct brcm_iommu {
	const char		*name;
	struct device		*dev;
	struct iommu_domain	*domain;
	struct iommu_device	iommu;
	struct iommu_group	*group;
	struct list_head	node;
	enum brcm_iommu_type	type;
	void __iomem		*bases[BREG_TYPE_NUM];
	bool			mmuc_enabled;
	struct brcm_entry	*l1_info;
	bmmu_pte		*l1_mmu_va;
	dma_addr_t		l1_mmu_dma;
	size_t			l1_size;
	size_t			l1_num_entries;
	spinlock_t		lock;
	bool			pcie_iova_set;

	u64			iova_start;
	u64			iova_size;

	struct {
		u64		shootdowns;
		u64		maps;
		u64		unmaps;
		u32		l1_pages;
		u32		l2_pages;
		unsigned long	iova_min;
		unsigned long	iova_max;
	} stats;
};

static const struct iommu_ops brcm_iommu_ops;

static struct brcm_domain *to_brcm_domain(struct iommu_domain *domain)
{
	return container_of(domain, struct brcm_domain, domain);
}

static int brcm_hw_tlb_clear(struct brcm_iommu *bmmu)
{
	void __iomem *base = bmmu->bases[BREG_MMU];
	u32 tmp;

	tmp = readl(base + BREG_MMU_CTRL);
	tmp |= FIELD_PREP(BMASK_TLB_CLR, 1);
	writel(tmp, base + BREG_MMU_CTRL);

	return readl_poll_timeout_atomic(base + BREG_MMU_CTRL, tmp,
					 !FIELD_GET(BMASK_TLB_CLR_BUSY, tmp),
					 1, 100);
}

static int brcm_hw_mmuc_clear(struct brcm_iommu *bmmu)
{
	void __iomem *base = bmmu->bases[BREG_MMUC];
	u32 ctrl;

	ctrl = readl(base + BREG_MMUC_CTRL);
	ctrl &= ~BMASK_MMUC_FLUSH_IDENT_RANGE;
	ctrl |= BMASK_MMUC_FLUSH;
	writel(ctrl, base + BREG_MMUC_CTRL);
	return readl_poll_timeout_atomic(base + BREG_MMUC_CTRL, ctrl,
					 !(BMASK_MMUC_FLUSH_BUSY & ctrl),
					 1, 100);
}

static void brcm_flush_mmus(struct brcm_iommu *bmmu, unsigned long iova,
			    size_t size)
{
	int err0 = 0, err1 = 0;

	err0 = brcm_hw_mmuc_clear(bmmu);
	err1 = brcm_hw_tlb_clear(bmmu);

	if (err0)
		dev_err(bmmu->dev, "Iommu error: PTE MMUC flush fail\n");
	if (err1)
		dev_err(bmmu->dev, "Iommu error: PTE TLB flush fail\n");
}

static void brcm_iommu_flush_all(struct brcm_iommu *bmmu)
{
	brcm_flush_mmus(bmmu, (unsigned long)bmmu->iova_start, bmmu->iova_size);
}

static int brcm_hw_mmuc_enable(struct brcm_iommu *bmmu, bool enable)
{
	void __iomem *base = bmmu->bases[BREG_MMUC];
	u32 ctrl;
	int ret = 0;

	if (!base)
		return 0;

	ctrl = readl(base + BREG_MMUC_CTRL);
	if (!!(ctrl & BMASK_MMUC_EN) == enable)
		return 0;
	if (!enable)
		ret = brcm_hw_mmuc_clear(bmmu);

	u32p_replace_bits(&ctrl, !!enable, BMASK_MMUC_EN);
	u32p_replace_bits(&ctrl, 0, BMASK_MMUC_FLUSH_IDENT_RANGE);
	writel(ctrl, base + BREG_MMUC_CTRL);
	bmmu->mmuc_enabled = enable;

	return ret;
}

static int brcm_hw_mmu_enable(struct brcm_iommu *bmmu, bool enable)
{
	phys_addr_t pt_base = dma_to_phys(bmmu->dev, bmmu->l1_mmu_dma);
	void __iomem *base = bmmu->bases[BREG_MMU];
	u32 ctrl, tmp;
	int ret;

	/* Older HW versions cannot meet our needs */
	tmp = FIELD_GET(BMASK_VERSION, readl(base + BREG_MMU_DEBUG_INFO));
	if (tmp < 4) {
		dev_err(bmmu->dev, "Iommu error: HW too old (v%u)\n", tmp);
		return -EINVAL;
	}

	if (!enable) {
		int ret = brcm_hw_tlb_clear(bmmu);

		/* Writing zeroes shuts everything down */
		writel(0, base + BREG_MMU_CTRL);
		writel(0, base + BREG_MMU_PT_PA_BASE);
		return ret;
	}

	/*
	 * NOTE: The MMU may already have its enable bit set for reasons
	 * unknown.  Even so, the other registers need to be properly
	 * initialized.
	 */

	/* Write the system address of the L1 page table */
	tmp = FIELD_PREP(BMASK_PT_PA_BASE_PAGE, pt_base >> BPAGE_SHIFT);
	writel(tmp, base + BREG_MMU_PT_PA_BASE);

	/* Set HW to divert illegal accesses to a device "sink" */
	tmp = FIELD_PREP(BMASK_ILLEGAL_ADDR, BMMU_NULL_DEV_PAGE) |
		FIELD_PREP(BMASK_ILLEGAL_ADDR_EN, 1);
	writel(tmp, base + BREG_MMU_ILLEGAL_ADR);

	/* We want stats and interrupts for access violations */
	ctrl = FIELD_PREP(BMASK_STATS_EN, 1) |
		FIELD_PREP(BMASK_PT_INV_EN, 1) |
		FIELD_PREP(BMASK_PT_INV_INT_EN, 1) |
		FIELD_PREP(BMASK_WT_VIO_INT_EN, 1) |
		FIELD_PREP(BMASK_CAP_EXCEED_INT_EN, 1) |
		FIELD_PREP(BMASK_PT_INV_ABORT_EN, 1) |
		FIELD_PREP(BMASK_WT_VIO_ABORT_EN, 1) |
		FIELD_PREP(BMASK_CAP_EXCEED_ABORT_EN, 1) |
		FIELD_PREP(BMASK_STATS_CLR, 1);
	writel(ctrl, base + BREG_MMU_CTRL);

	/* Set a high limit for iova addresses */
	tmp = bmmu->iova_size >> BMMU_CAP_IOVA_SHIFT;
	tmp = FIELD_PREP(BMASK_CAP_EN, 1) | FIELD_PREP(BMASK_CAP_MPAGE, tmp);
	writel(tmp, base + BREG_MMU_ADDR_CAP);

	tmp = readl(base + BREG_MMU_DEBUG_MISC);
	/* Use the 2-level page tables */
	u32p_replace_bits(&tmp, 0, BMASK_SINGLE_TABLE);
	/* Use maximum number of L1 TLB entries */
	tmp |= BMASK_L1_TLB_LIMIT;
	/* Use maximum number of L2 TLB entries */
	tmp |= BMASK_L2_TLB_LIMIT;
	writel(tmp, base + BREG_MMU_DEBUG_MISC);

	ret = brcm_hw_tlb_clear(bmmu);
	if (ret)
		return ret;

	/* Turn it on! */
	ctrl = readl(base + BREG_MMU_CTRL) | BMASK_EN;
	writel(ctrl, base + BREG_MMU_CTRL);

	return 0;
}

static int brcm_find_iova_offset(struct device *dev, u64 *iova_start,
				 u64 *iova_size)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct pci_host_bridge *bridge = pci_find_host_bridge(pdev->bus);
	struct resource_entry *entry;
	u64 offset, iova_max = 0, iova_min = ~(u64)0;
	bool first = true;

	*iova_size = 0;
	if (list_empty(&bridge->dma_ranges))
		return 0;

	resource_list_for_each_entry(entry, &bridge->dma_ranges) {
		u64 tmp = entry->res->start - entry->offset;
		u64 size = entry->res->end - entry->res->start + 1;

		if (!first && offset != entry->offset) {
			dev_err(dev, "Error: cannot handle multiple dma-range offsets\n");
			return -EINVAL;
		}
		offset = entry->offset;
		first = false;

		if (tmp < iova_min)
			iova_min = tmp;
		if (tmp + size > iova_max)
			iova_max = tmp + size;
	}
	*iova_size = iova_max - iova_min;
	*iova_start = iova_min;

	return 0;
}

static int brcm_alloc_l1_table(struct brcm_domain *bdom,
			       struct brcm_iommu *bmmu)
{
	/* Each l1 entry governs 4M of iova space */
	const u64 l1_entries = bmmu->iova_size / SZ_4M;
	const gfp_t gfp_mask = GFP_KERNEL | __GFP_ZERO | __GFP_DMA;
	unsigned long l1_npages, l1_pages_order;
	struct device *dev = bmmu->dev;
	dma_addr_t dma_addr;
	int i;

	/* Already alloc'ed, perhaps this is after an unbind/rebind */
	if (bmmu->l1_info)
		return 0;

	l1_npages = round_up(l1_entries * BPTE_SIZE, BPAGE_SIZE) / PAGE_SIZE;
	l1_pages_order = order_base_2(l1_npages);
	l1_npages = 1 << l1_pages_order;
	bmmu->l1_size = l1_npages * PAGE_SIZE;
	bmmu->l1_num_entries = bmmu->l1_size / BPTE_SIZE;
	bmmu->l1_info = devm_kcalloc(bmmu->dev, bmmu->l1_num_entries,
				     sizeof(struct brcm_entry), GFP_KERNEL);

	if (!bmmu->l1_info)
		return -ENOMEM;
	for (i = 0; i < bmmu->l1_num_entries; i++)
		bmmu->l1_info[i] = BMMU_INVALID_ENTRY;

	bmmu->l1_mmu_va = (void *)devm_get_free_pages(dev, gfp_mask, l1_pages_order);
	if (!bmmu->l1_mmu_va)
		return -ENOMEM;

	bmmu->stats.l1_pages = l1_npages;
	dma_addr = dma_map_single(dev, bmmu->l1_mmu_va, bmmu->l1_size, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, dma_addr))
		return -ENOMEM;

	bmmu->l1_mmu_dma = dma_addr;
	WARN_ON_ONCE(dma_addr != virt_to_phys(bmmu->l1_mmu_va));
	dma_sync_single_for_device(bmmu->dev, dma_addr, bmmu->l1_size, DMA_TO_DEVICE);
	brcm_flush_mmus(bmmu, bmmu->iova_start, bmmu->l1_num_entries << BPAGE_SHIFT);

	return 0;
}

static int brcm_alloc_l2_table(struct brcm_iommu *bmmu, u32 l1_idx,
			       unsigned long iova)
{
	struct brcm_entry *l1_entry = bmmu->l1_info + l1_idx;
	dma_addr_t l1_dma_addr, l2_dma_addr;
	bmmu_pte *l2, *l1 = bmmu->l1_mmu_va;
	struct device *dev = bmmu->dev;
	bmmu_pte l1_pte;

	assert_spin_locked(&bmmu->lock);

	l2 = (void *)get_zeroed_page(GFP_ATOMIC | __GFP_DMA);
	if (!l2)
		return -ENOMEM;

	l2_dma_addr = dma_map_single(dev, l2, PAGE_SIZE, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, l2_dma_addr)) {
		free_page((unsigned long)l2);
		return -EIO;
	}

	WARN_ON_ONCE(l2_dma_addr != virt_to_phys(l2));
	dma_sync_single_for_device(bmmu->dev, l2_dma_addr, BPAGE_SIZE, DMA_TO_DEVICE);

	l1_entry->l2_mmu_va = l2;
	l1_entry->l2_pa = virt_to_phys(l2);
	l1_entry->l2_dma = l2_dma_addr;

	l1_pte = FIELD_PREP(BMASK_PTE_WRITEABLE, 1) |
		FIELD_PREP(BMASK_PTE_TYPE, BMMU_L1_PTE_TYPE_4K_PTR) |
		FIELD_PREP(BMASK_PTE_VALID, 1) |
		FIELD_PREP(BMASK_PTE_PAGE, virt_to_phys(l2) >>
			   BPAGE_SHIFT);
	l1_dma_addr = bmmu->l1_mmu_dma + l1_idx * BPTE_SIZE;
	l1[l1_idx] = l1_pte;
	dma_sync_single_for_device(bmmu->dev, l1_dma_addr, BPTE_SIZE, DMA_TO_DEVICE);
	bmmu->stats.l2_pages++;

	return 0;
}

static bool brcm_iommu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_INTR_REMAP:
		return true;
	default:
		break;
	}
	return false;
}

static struct iommu_domain *brcm_iommu_domain_alloc(unsigned int dom_type)
{
	struct brcm_domain *bdom;

	bdom = kzalloc(sizeof(*bdom), GFP_KERNEL);
	if (!bdom)
		return NULL;

	bdom->domain.type = IOMMU_DOMAIN_DMA;
	bdom->domain.pgsize_bitmap = BMMU_PGSIZE_BITMAP;
	iommu_get_dma_cookie(&bdom->domain);

	return &bdom->domain;
}

static void brcm_iommu_domain_free(struct iommu_domain *domain)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = bdom->iommu;
	struct brcm_entry *l1_entry;
	dma_addr_t dma;
	bmmu_pte *l2_mmu_va;
	unsigned long flags;
	unsigned int i;

	if (!bmmu)
		goto out;

	dma = bmmu->l1_mmu_dma;

	/* Free allocated l2 tables by going through l1 */
	for (i = 0; i < bmmu->l1_num_entries ; i++, dma += BPTE_SIZE) {
		unsigned long iova = bmmu_idx_to_iova(i, 0);


		l1_entry = &bmmu->l1_info[i];
		if (l1_entry->l2_dma == DMA_MAPPING_ERROR)
			continue;

		/* Invalidate all of the L2 PTEs used by the IOMMU HW */
		spin_lock_irqsave(&bmmu->lock, flags);
		l2_mmu_va = l1_entry->l2_mmu_va;
		memset(l2_mmu_va, 0, BPAGE_SIZE);
		dma_sync_single_for_device(bmmu->dev, l1_entry->l2_dma,
					   BPAGE_SIZE, DMA_TO_DEVICE);
		dma_unmap_single(bmmu->dev, l1_entry->l2_pa, BPAGE_SIZE,
				 DMA_TO_DEVICE);
		spin_unlock_irqrestore(&bmmu->lock, flags);
		free_page((unsigned long)l2_mmu_va);

		/* Take care of the L1 PTE */
		spin_lock_irqsave(&bmmu->lock, flags);
		*l1_entry = BMMU_INVALID_ENTRY;
		bmmu->l1_mmu_va[i] = BMMU_INVALID_PTE;
		dma_sync_single_for_device(bmmu->dev, dma, BPTE_SIZE,
					   DMA_TO_DEVICE);
		spin_unlock_irqrestore(&bmmu->lock, flags);

		brcm_flush_mmus(bmmu, iova, BMMU_PTE_PER_PAGE << BPAGE_SHIFT);
	}
out:
	kfree(bdom);
}

static void brcm_iommu_detach_device(struct iommu_domain *domain,
				     struct device *dev)
{
	struct brcm_iommu *bmmu = dev_iommu_priv_get(dev);

	if (bmmu)
		brcm_hw_mmu_enable(bmmu, false);
}

static int brcm_iommu_attach_device(struct iommu_domain *domain,
				    struct device *dev)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = dev_iommu_priv_get(dev);

	bdom->iommu = bmmu;
	bmmu->domain = domain;

	return 0;
}

static int brcm_check_iova(struct brcm_iommu *bmmu, unsigned long iova)
{
	if (iova >= bmmu->iova_start + bmmu->iova_size ||
	    iova < bmmu->iova_start) {
		dev_err(bmmu->dev,
			"iova addr %lx out of range [%llx..%llx]\n",
			iova, bmmu->iova_start, bmmu->iova_start +
			bmmu->iova_size - 1);
		return -EINVAL;
	}
	return 0;
}

static int brcm_iommu_map(struct iommu_domain *domain, unsigned long iova,
			  phys_addr_t paddr, size_t size, int prot, gfp_t gfp)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = bdom->iommu;
	const bmmu_pte pte_map =
		FIELD_PREP(BMASK_PTE_WRITEABLE, !!(prot & IOMMU_WRITE)) |
		FIELD_PREP(BMASK_PTE_TYPE, BMMU_L2_PTE_TYPE_4K_PAGE) |
		FIELD_PREP(BMASK_PTE_VALID, 1);
	u32 l1_idx_beg, l1_idx_end, num_pte_left;
	int ret = 0;
	u32 i;

	ret = brcm_check_iova(bmmu, iova);
	if (ret)
		return ret;

	iova = bmmu_adjust_iova(bmmu, iova);
	size = round_up(size, BPAGE_SIZE);
	num_pte_left = size >> BPAGE_SHIFT;
	iova &= ~(BPAGE_SIZE - 1);

	l1_idx_beg = bmmu_iova_to_l1_idx(iova);
	l1_idx_end = bmmu_iova_to_l1_idx(iova + size - 1);

	for (i = l1_idx_beg; i <= l1_idx_end; i++) {
		struct brcm_entry *l1_entry;
		u32 l2_idx_beg = bmmu_iova_to_l2_idx(iova);
		size_t pte_region_size;
		bmmu_pte *l2_mmu_va, l2_pte;
		bool l2_page_alloc = false;
		unsigned long flags;
		dma_addr_t l2_dma;
		u32 j, num_pte;

		spin_lock_irqsave(&bmmu->lock, flags);
		l1_entry = &bmmu->l1_info[i];
		if (l1_entry->l2_dma == DMA_MAPPING_ERROR) {
			ret = brcm_alloc_l2_table(bmmu, i, iova);
			if (ret) {
				spin_unlock_irqrestore(&bmmu->lock, flags);
				l1_entry = &bmmu->l1_info[i];
			}
		}
		spin_unlock_irqrestore(&bmmu->lock, flags);

		l2_mmu_va = l1_entry->l2_mmu_va + l2_idx_beg;
		l2_dma = l1_entry->l2_dma + l2_idx_beg * BPTE_SIZE;
		num_pte = l2_idx_beg + num_pte_left >= BMMU_PTE_PER_PAGE
			? BMMU_PTE_PER_PAGE - l2_idx_beg
			: num_pte_left;
		pte_region_size = num_pte * BPTE_SIZE;
		l2_pte = pte_map | FIELD_PREP(BMASK_PTE_PAGE,
					      paddr >> BPAGE_SHIFT);

		spin_lock_irqsave(&bmmu->lock, flags);
		/* As BMASK_PTE_PAGE starts at bit0, l2_pte++ will do */
		for (j = 0; j < num_pte; j++)
			*l2_mmu_va++ = l2_pte++;
		dma_sync_single_for_device(bmmu->dev, l2_dma,
					   pte_region_size, DMA_TO_DEVICE);
		spin_unlock_irqrestore(&bmmu->lock, flags);

		if (l2_page_alloc)
			brcm_iommu_flush_all(bmmu);

		bmmu->stats.unmaps += num_pte;
		num_pte_left -= num_pte;
		paddr += num_pte * BPAGE_SIZE;
		iova += num_pte * BPAGE_SIZE;
	}

	return 0;
}


static size_t brcm_iommu_unmap(struct iommu_domain *domain,
			       unsigned long iova_orig, size_t size,
			       struct iommu_iotlb_gather *gather)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	u32 l1_idx_beg, l1_idx_end, num_pte_left;
	struct brcm_iommu *bmmu = bdom->iommu;
	unsigned long iova = iova_orig;
	size_t unmapped = 0;
	u32 i, ret = 0;

	ret = brcm_check_iova(bmmu, iova);
	if (ret)
		return ret;

	iova = bmmu_adjust_iova(bmmu, iova);
	size = round_up(size, BPAGE_SIZE);
	num_pte_left = size >> BPAGE_SHIFT;
	iova &= ~(BPAGE_SIZE - 1);

	l1_idx_beg = bmmu_iova_to_l1_idx(iova);
	l1_idx_end = bmmu_iova_to_l1_idx(iova + size - 1);

	for (i = l1_idx_beg; i <= l1_idx_end; i++) {
		struct brcm_entry *l1_entry = &bmmu->l1_info[i];
		u32 num_pte, l2_idx_beg = bmmu_iova_to_l2_idx(iova);
		bmmu_pte *l2_mmu_va = l1_entry->l2_mmu_va + l2_idx_beg;
		dma_addr_t dma = l1_entry->l2_dma + l2_idx_beg * BPTE_SIZE;
		unsigned long flags;

		num_pte = l2_idx_beg + num_pte_left >= BMMU_PTE_PER_PAGE
			? BMMU_PTE_PER_PAGE - l2_idx_beg
			: num_pte_left;

		/* Invalidate the appropriate PTEs */
		spin_lock_irqsave(&bmmu->lock, flags);
		memset(l2_mmu_va, 0, num_pte * BPTE_SIZE);
		dma_sync_single_for_device(bmmu->dev, dma, num_pte * BPTE_SIZE,
					   DMA_TO_DEVICE);
		spin_unlock_irqrestore(&bmmu->lock, flags);
		num_pte_left -= num_pte;
		bmmu->stats.unmaps += num_pte;
		iova += BPAGE_SIZE * num_pte;
		unmapped += BPAGE_SIZE * num_pte;
	}

	iommu_iotlb_gather_add_range(gather, iova_orig, size);

	return unmapped;
}

static phys_addr_t brcm_iommu_iova_to_phys(struct iommu_domain *domain,
					   dma_addr_t iova)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = bdom->iommu;
	const struct brcm_entry *l1_entry;
	phys_addr_t paddr = 0;
	unsigned long flags;
	bmmu_pte l2_pte;
	u32 l1_idx, l2_idx;
	int ret = -EIO;

	if (WARN_ONCE(iova < bmmu->iova_start,
		      "iova address %pad out of bounds\n", &iova))
		return -EINVAL;

	iova = bmmu_adjust_iova(bmmu, iova);

	l1_idx = bmmu_iova_to_l1_idx(iova);
	l2_idx = bmmu_iova_to_l2_idx(iova);

	spin_lock_irqsave(&bmmu->lock, flags);
	l1_entry = &bmmu->l1_info[l1_idx];
	if (l1_entry->l2_dma != DMA_MAPPING_ERROR) {
		l2_pte = l1_entry->l2_mmu_va[l2_idx];
		if (bmmu_pte_valid(l2_pte)) {
			paddr = bmmu_pte_to_phys(l2_pte) |
				bmmu_iova_4K_offset(iova);
			ret = 0;
		}
	}
	spin_unlock_irqrestore(&bmmu->lock, flags);

	if (ret)
		dev_err(bmmu->dev, "failed to derive pa from iova\n");

	return paddr;
}

static struct iommu_group *brcm_iommu_device_group(struct device *dev)
{
	struct brcm_iommu *bmmu = dev_iommu_priv_get(dev);

	if (!bmmu)
		return NULL;

	return iommu_group_ref_get(binfo[bmmu->type].group);
}

static int brcm_iommu_of_xlate(struct device *dev,
			       struct of_phandle_args *args)
{
	struct platform_device *iommu_dev;
	int ret = -EINVAL;

	iommu_dev = of_find_device_by_node(args->np);
	if (iommu_dev) {
		dev_iommu_priv_set(dev, platform_get_drvdata(iommu_dev));
		platform_device_put(iommu_dev);
		ret = 0;
	}

	return ret;
}

static void brcm_iommu_release_device(struct device *dev)
{
	struct brcm_iommu *bmmu = dev_iommu_priv_get(dev);

	if (!bmmu)
		return;
	iommu_device_unlink(&bmmu->iommu, dev);
}

struct iommu_device *brcm_iommu_probe_device(struct device *dev)
{
	struct brcm_iommu *bmmu;
	struct platform_device *pdev;
	struct device_node *np;
	int ret;

	np = of_parse_phandle(dev->of_node, "iommus", 0);
	if (!np)
		return ERR_PTR(-EINVAL);

	pdev = of_find_device_by_node(np);
	if (!pdev)
		return ERR_PTR(-ENODEV);

	bmmu = platform_get_drvdata(pdev);
	if (!bmmu)
		return ERR_PTR(-ENODEV);

	dev_iommu_priv_set(dev, bmmu);
	of_node_put(np);

	ret = iommu_fwspec_init(dev, &dev->of_node->fwnode, &brcm_iommu_ops);
	if (ret)
		return ERR_PTR(ret);

	if (bmmu->type == BMMU_PCI && !bmmu->pcie_iova_set) {
		ret = brcm_find_iova_offset(dev, &bmmu->iova_start, &bmmu->iova_size);
		if (ret)
			return ERR_PTR(ret);
		bmmu->pcie_iova_set = true;
	}

	ret = brcm_alloc_l1_table(to_brcm_domain(bmmu->domain), bmmu);
	if (ret)
		return ERR_PTR(ret);

	ret = brcm_hw_mmu_enable(bmmu, true);
	if (ret)
		return ERR_PTR(ret);


	ret = brcm_hw_mmuc_enable(bmmu, true);
	if (ret)
		return ERR_PTR(ret);

	return &bmmu->iommu;
}

static void brcm_iommu_iotlb_sync_map(struct iommu_domain *domain,
				      unsigned long iova,
				      size_t size)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = bdom->iommu;

	iova = bmmu_adjust_iova(bmmu, iova);
	brcm_flush_mmus(bmmu, iova, size);
}

static inline void brcm_iommu_iotlb_sync(struct iommu_domain *domain,
					 struct iommu_iotlb_gather *gather)
{
	struct brcm_domain *bdom = to_brcm_domain(domain);
	struct brcm_iommu *bmmu = bdom->iommu;
	size_t size = gather->end - gather->start + 1;

	brcm_flush_mmus(bmmu, gather->start, size);
}

static const struct iommu_ops brcm_iommu_ops = {
	.attach_dev = brcm_iommu_attach_device,
	.capable = brcm_iommu_capable,
	.detach_dev = brcm_iommu_detach_device,
	.device_group = brcm_iommu_device_group,
	.domain_alloc = brcm_iommu_domain_alloc,
	.domain_free = brcm_iommu_domain_free,
	.iotlb_sync = brcm_iommu_iotlb_sync,
	.iotlb_sync_map = brcm_iommu_iotlb_sync_map,
	.iova_to_phys = brcm_iommu_iova_to_phys,
	.map = brcm_iommu_map,
	.of_xlate = brcm_iommu_of_xlate,
	.pgsize_bitmap = BMMU_PGSIZE_BITMAP,
	.probe_device = brcm_iommu_probe_device,
	.release_device	= brcm_iommu_release_device,
	.unmap = brcm_iommu_unmap,
	.owner = THIS_MODULE,
};

static irqreturn_t brcm_iommu_isr(int irq, void *data)
{
	static const char fmt[] = "Error: %s, AXI=0x%x\n";
	struct brcm_iommu *bmmu = data;
	void __iomem *base = bmmu->bases[BREG_MMU];
	const u32 ctrl = readl(base + BREG_MMU_CTRL);
	u32 axi;

	if (!(ctrl & (BMASK_PT_INV | BMASK_CAP_EXCEED | BMASK_WT_VIO)))
		return IRQ_NONE;
	axi = readl(base + BREG_MMU_VIO_ADDR);
	writel(0, base + BREG_MMU_VIO_ADDR);

	if (FIELD_GET(BMASK_PT_INV, ctrl))
		dev_err(bmmu->dev, fmt, "Failed page table walk", axi);
	if (FIELD_GET(BMASK_CAP_EXCEED, ctrl))
		dev_err(bmmu->dev, fmt, "Iova exceeds allowed region", axi);
	if (FIELD_GET(BMASK_WT_VIO, ctrl))
		dev_err(bmmu->dev, fmt, "Write to read-only memory", axi);

	/* This will clear any set violation flag bits */
	writel(ctrl, base + BREG_MMU_CTRL);

	return IRQ_HANDLED;
}

static int brcm_iommu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iommu_group *new_group = NULL;
	struct brcm_iommu *bmmu;
	struct resource res;
	const char *type_str;
	int irq, i, err = 0;

	bmmu = devm_kzalloc(dev, sizeof(struct brcm_iommu), GFP_KERNEL);
	if (!bmmu)
		return -ENOMEM;

	err = of_property_read_string(dev->of_node, "brcm,type", &type_str);
	if (err)
		return -EINVAL;
	bmmu->type = brcm_iommu_str_to_type(type_str);
	if (bmmu->type >= BMMU_TYPE_NUM)
		return -EINVAL;

	bmmu->name		= dev_name(dev);
	bmmu->dev		= dev;

	/* Defaults, will most likely be updated */
	bmmu->iova_size		= SZ_4G;
	bmmu->iova_start	= 0;
	bmmu->stats.iova_min	= ULONG_MAX;
	bmmu->stats.iova_max	= 0;

	spin_lock_init(&bmmu->lock);

	platform_set_drvdata(pdev, bmmu);
	if (!pci_bus_type_is_set) {
		if (bus_set_iommu(&pci_bus_type, &brcm_iommu_ops))
			dev_warn(dev, "could not set iommu pci bus_type\n");
		else
			pci_bus_type_is_set = true;
	}

	mutex_lock(&bmmu_mutex);
	if (binfo[bmmu->type].group) {
		bmmu->group = binfo[bmmu->type].group;
		bmmu->domain = binfo[bmmu->type].domain;
	} else {
		/* Create group */
		new_group = iommu_group_alloc();
		if (IS_ERR(new_group)) {
			mutex_unlock(&bmmu_mutex);
			return PTR_ERR(new_group);
		}
		bmmu->group = new_group;
		binfo[bmmu->type].group = new_group;
		err = iommu_group_set_name(new_group, brcm_iommu_type_to_str(bmmu->type));
	}
	mutex_unlock(&bmmu_mutex);
	if (err)
		goto err_put_group;

	err = iommu_device_sysfs_add(&bmmu->iommu, dev, NULL, dev_name(dev));
	if (err)
		goto err_put_group;

	err = iommu_device_register(&bmmu->iommu, &brcm_iommu_ops, dev);
	if (err)
		goto err_remove_sysfs;

	if (bmmu->type == BMMU_PCI) {
		irq = platform_get_irq_byname(pdev, "mmu");
		if (irq < 0)
			return -ENODEV;
		err = devm_request_irq(bmmu->dev, irq, brcm_iommu_isr, IRQF_SHARED,
				       dev_name(bmmu->dev), bmmu);
		if (err < 0)
			goto err_unregister;
	}

	for (i = 0; i < BREG_TYPE_NUM; i++) {
		void __iomem *base;

		if (bmmu->type == BMMU_PCI) {
			/* For PCI type, these nodes will always exist */
			base = devm_platform_ioremap_resource(pdev, i);
		} else {
			/* Other types provide nodes on need-to-know basis */
			if (of_address_to_resource(dev->of_node, i, &res))
				return -EINVAL;
			base = devm_ioremap(dev, res.start, resource_size(&res));
		}
		if (IS_ERR(base)) {
			err = PTR_ERR(base);
			goto err_unregister;
		}
		bmmu->bases[i] = base;
	}
	binfo[bmmu->type].device = &bmmu->iommu;

	return 0;

err_unregister:
	iommu_device_unregister(&bmmu->iommu);
err_remove_sysfs:
	iommu_device_sysfs_remove(&bmmu->iommu);
err_put_group:
	iommu_group_put(new_group);

	return err;
}

static int brcm_iommu_remove(struct platform_device *pdev)
{
	struct brcm_iommu *bmmu = platform_get_drvdata(pdev);

	if (bmmu->group) {
		iommu_group_put(bmmu->group);
		bmmu->group = NULL;
		iommu_device_sysfs_remove(&bmmu->iommu);
		iommu_device_unregister(&bmmu->iommu);
	}

	return 0;
}

static int brcm_iommu_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct brcm_iommu *bmmu = platform_get_drvdata(pdev);

	brcm_hw_mmu_enable(bmmu, false);

	return 0;
}

static int brcm_iommu_resume_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct brcm_iommu *bmmu = platform_get_drvdata(pdev);

	brcm_hw_mmuc_enable(bmmu, true);
	brcm_hw_mmu_enable(bmmu, true);

	return 0;
}

static const struct dev_pm_ops brcm_iommu_pm_ops = {
	.suspend_noirq = brcm_iommu_suspend_noirq,
	.resume_noirq = brcm_iommu_resume_noirq,
};

static const struct of_device_id brcm_iommu_of_match[] = {
	{ .compatible = "brcm,bcm7712-m3u", },
	{},
};

static struct platform_driver brcm_iommu_driver = {
	.probe	= brcm_iommu_probe,
	.remove	= brcm_iommu_remove,
	.driver	= {
		.name	= "brcm-iommu",
		.pm	= &brcm_iommu_pm_ops,
		.of_match_table = of_match_ptr(brcm_iommu_of_match),
	},
};

static int __init brcm_iommu_init(void)
{
	return platform_driver_register(&brcm_iommu_driver);
}
subsys_initcall(brcm_iommu_init);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB IOMMU driver");
MODULE_LICENSE("GPL v2");
