/*
 * Copyright (C) 2011-2014 NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <soc/tegra/ahb.h>
#include <soc/tegra/mc.h>

struct tegra_smmu {
	void __iomem *regs;
	struct device *dev;

	struct tegra_mc *mc;
	const struct tegra_smmu_soc *soc;

	unsigned long pfn_mask;
	unsigned long tlb_mask;

	unsigned long *asids;
	struct mutex lock;

	struct list_head list;
};

struct tegra_smmu_as {
	struct iommu_domain domain;
	struct tegra_smmu *smmu;
	unsigned int use_count;
	struct page *count;
	struct page *pd;
	unsigned id;
	u32 attr;
};

static struct tegra_smmu_as *to_smmu_as(struct iommu_domain *dom)
{
	return container_of(dom, struct tegra_smmu_as, domain);
}

static inline void smmu_writel(struct tegra_smmu *smmu, u32 value,
			       unsigned long offset)
{
	writel(value, smmu->regs + offset);
}

static inline u32 smmu_readl(struct tegra_smmu *smmu, unsigned long offset)
{
	return readl(smmu->regs + offset);
}

#define SMMU_CONFIG 0x010
#define  SMMU_CONFIG_ENABLE (1 << 0)

#define SMMU_TLB_CONFIG 0x14
#define  SMMU_TLB_CONFIG_HIT_UNDER_MISS (1 << 29)
#define  SMMU_TLB_CONFIG_ROUND_ROBIN_ARBITRATION (1 << 28)
#define  SMMU_TLB_CONFIG_ACTIVE_LINES(smmu) \
	((smmu)->soc->num_tlb_lines & (smmu)->tlb_mask)

#define SMMU_PTC_CONFIG 0x18
#define  SMMU_PTC_CONFIG_ENABLE (1 << 29)
#define  SMMU_PTC_CONFIG_REQ_LIMIT(x) (((x) & 0x0f) << 24)
#define  SMMU_PTC_CONFIG_INDEX_MAP(x) ((x) & 0x3f)

#define SMMU_PTB_ASID 0x01c
#define  SMMU_PTB_ASID_VALUE(x) ((x) & 0x7f)

#define SMMU_PTB_DATA 0x020
#define  SMMU_PTB_DATA_VALUE(page, attr) (page_to_phys(page) >> 12 | (attr))

#define SMMU_MK_PDE(page, attr) (page_to_phys(page) >> SMMU_PTE_SHIFT | (attr))

#define SMMU_TLB_FLUSH 0x030
#define  SMMU_TLB_FLUSH_VA_MATCH_ALL     (0 << 0)
#define  SMMU_TLB_FLUSH_VA_MATCH_SECTION (2 << 0)
#define  SMMU_TLB_FLUSH_VA_MATCH_GROUP   (3 << 0)
#define  SMMU_TLB_FLUSH_ASID(x)          (((x) & 0x7f) << 24)
#define  SMMU_TLB_FLUSH_VA_SECTION(addr) ((((addr) & 0xffc00000) >> 12) | \
					  SMMU_TLB_FLUSH_VA_MATCH_SECTION)
#define  SMMU_TLB_FLUSH_VA_GROUP(addr)   ((((addr) & 0xffffc000) >> 12) | \
					  SMMU_TLB_FLUSH_VA_MATCH_GROUP)
#define  SMMU_TLB_FLUSH_ASID_MATCH       (1 << 31)

#define SMMU_PTC_FLUSH 0x034
#define  SMMU_PTC_FLUSH_TYPE_ALL (0 << 0)
#define  SMMU_PTC_FLUSH_TYPE_ADR (1 << 0)

#define SMMU_PTC_FLUSH_HI 0x9b8
#define  SMMU_PTC_FLUSH_HI_MASK 0x3

/* per-SWGROUP SMMU_*_ASID register */
#define SMMU_ASID_ENABLE (1 << 31)
#define SMMU_ASID_MASK 0x7f
#define SMMU_ASID_VALUE(x) ((x) & SMMU_ASID_MASK)

/* page table definitions */
#define SMMU_NUM_PDE 1024
#define SMMU_NUM_PTE 1024

#define SMMU_SIZE_PD (SMMU_NUM_PDE * 4)
#define SMMU_SIZE_PT (SMMU_NUM_PTE * 4)

#define SMMU_PDE_SHIFT 22
#define SMMU_PTE_SHIFT 12

#define SMMU_PD_READABLE	(1 << 31)
#define SMMU_PD_WRITABLE	(1 << 30)
#define SMMU_PD_NONSECURE	(1 << 29)

#define SMMU_PDE_READABLE	(1 << 31)
#define SMMU_PDE_WRITABLE	(1 << 30)
#define SMMU_PDE_NONSECURE	(1 << 29)
#define SMMU_PDE_NEXT		(1 << 28)

#define SMMU_PTE_READABLE	(1 << 31)
#define SMMU_PTE_WRITABLE	(1 << 30)
#define SMMU_PTE_NONSECURE	(1 << 29)

#define SMMU_PDE_ATTR		(SMMU_PDE_READABLE | SMMU_PDE_WRITABLE | \
				 SMMU_PDE_NONSECURE)
#define SMMU_PTE_ATTR		(SMMU_PTE_READABLE | SMMU_PTE_WRITABLE | \
				 SMMU_PTE_NONSECURE)

static inline void smmu_flush_ptc(struct tegra_smmu *smmu, struct page *page,
				  unsigned long offset)
{
	phys_addr_t phys = page ? page_to_phys(page) : 0;
	u32 value;

	if (page) {
		offset &= ~(smmu->mc->soc->atom_size - 1);

		if (smmu->mc->soc->num_address_bits > 32) {
#ifdef CONFIG_PHYS_ADDR_T_64BIT
			value = (phys >> 32) & SMMU_PTC_FLUSH_HI_MASK;
#else
			value = 0;
#endif
			smmu_writel(smmu, value, SMMU_PTC_FLUSH_HI);
		}

		value = (phys + offset) | SMMU_PTC_FLUSH_TYPE_ADR;
	} else {
		value = SMMU_PTC_FLUSH_TYPE_ALL;
	}

	smmu_writel(smmu, value, SMMU_PTC_FLUSH);
}

static inline void smmu_flush_tlb(struct tegra_smmu *smmu)
{
	smmu_writel(smmu, SMMU_TLB_FLUSH_VA_MATCH_ALL, SMMU_TLB_FLUSH);
}

static inline void smmu_flush_tlb_asid(struct tegra_smmu *smmu,
				       unsigned long asid)
{
	u32 value;

	value = SMMU_TLB_FLUSH_ASID_MATCH | SMMU_TLB_FLUSH_ASID(asid) |
		SMMU_TLB_FLUSH_VA_MATCH_ALL;
	smmu_writel(smmu, value, SMMU_TLB_FLUSH);
}

static inline void smmu_flush_tlb_section(struct tegra_smmu *smmu,
					  unsigned long asid,
					  unsigned long iova)
{
	u32 value;

	value = SMMU_TLB_FLUSH_ASID_MATCH | SMMU_TLB_FLUSH_ASID(asid) |
		SMMU_TLB_FLUSH_VA_SECTION(iova);
	smmu_writel(smmu, value, SMMU_TLB_FLUSH);
}

static inline void smmu_flush_tlb_group(struct tegra_smmu *smmu,
					unsigned long asid,
					unsigned long iova)
{
	u32 value;

	value = SMMU_TLB_FLUSH_ASID_MATCH | SMMU_TLB_FLUSH_ASID(asid) |
		SMMU_TLB_FLUSH_VA_GROUP(iova);
	smmu_writel(smmu, value, SMMU_TLB_FLUSH);
}

static inline void smmu_flush(struct tegra_smmu *smmu)
{
	smmu_readl(smmu, SMMU_CONFIG);
}

static int tegra_smmu_alloc_asid(struct tegra_smmu *smmu, unsigned int *idp)
{
	unsigned long id;

	mutex_lock(&smmu->lock);

	id = find_first_zero_bit(smmu->asids, smmu->soc->num_asids);
	if (id >= smmu->soc->num_asids) {
		mutex_unlock(&smmu->lock);
		return -ENOSPC;
	}

	set_bit(id, smmu->asids);
	*idp = id;

	mutex_unlock(&smmu->lock);
	return 0;
}

static void tegra_smmu_free_asid(struct tegra_smmu *smmu, unsigned int id)
{
	mutex_lock(&smmu->lock);
	clear_bit(id, smmu->asids);
	mutex_unlock(&smmu->lock);
}

static bool tegra_smmu_capable(enum iommu_cap cap)
{
	return false;
}

static struct iommu_domain *tegra_smmu_domain_alloc(unsigned type)
{
	struct tegra_smmu_as *as;
	unsigned int i;
	uint32_t *pd;

	if (type != IOMMU_DOMAIN_UNMANAGED)
		return NULL;

	as = kzalloc(sizeof(*as), GFP_KERNEL);
	if (!as)
		return NULL;

	as->attr = SMMU_PD_READABLE | SMMU_PD_WRITABLE | SMMU_PD_NONSECURE;

	as->pd = alloc_page(GFP_KERNEL | __GFP_DMA);
	if (!as->pd) {
		kfree(as);
		return NULL;
	}

	as->count = alloc_page(GFP_KERNEL);
	if (!as->count) {
		__free_page(as->pd);
		kfree(as);
		return NULL;
	}

	/* clear PDEs */
	pd = page_address(as->pd);
	SetPageReserved(as->pd);

	for (i = 0; i < SMMU_NUM_PDE; i++)
		pd[i] = 0;

	/* clear PDE usage counters */
	pd = page_address(as->count);
	SetPageReserved(as->count);

	for (i = 0; i < SMMU_NUM_PDE; i++)
		pd[i] = 0;

	/* setup aperture */
	as->domain.geometry.aperture_start = 0;
	as->domain.geometry.aperture_end = 0xffffffff;
	as->domain.geometry.force_aperture = true;

	return &as->domain;
}

static void tegra_smmu_domain_free(struct iommu_domain *domain)
{
	struct tegra_smmu_as *as = to_smmu_as(domain);

	/* TODO: free page directory and page tables */
	ClearPageReserved(as->pd);

	kfree(as);
}

static const struct tegra_smmu_swgroup *
tegra_smmu_find_swgroup(struct tegra_smmu *smmu, unsigned int swgroup)
{
	const struct tegra_smmu_swgroup *group = NULL;
	unsigned int i;

	for (i = 0; i < smmu->soc->num_swgroups; i++) {
		if (smmu->soc->swgroups[i].swgroup == swgroup) {
			group = &smmu->soc->swgroups[i];
			break;
		}
	}

	return group;
}

static void tegra_smmu_enable(struct tegra_smmu *smmu, unsigned int swgroup,
			      unsigned int asid)
{
	const struct tegra_smmu_swgroup *group;
	unsigned int i;
	u32 value;

	for (i = 0; i < smmu->soc->num_clients; i++) {
		const struct tegra_mc_client *client = &smmu->soc->clients[i];

		if (client->swgroup != swgroup)
			continue;

		value = smmu_readl(smmu, client->smmu.reg);
		value |= BIT(client->smmu.bit);
		smmu_writel(smmu, value, client->smmu.reg);
	}

	group = tegra_smmu_find_swgroup(smmu, swgroup);
	if (group) {
		value = smmu_readl(smmu, group->reg);
		value &= ~SMMU_ASID_MASK;
		value |= SMMU_ASID_VALUE(asid);
		value |= SMMU_ASID_ENABLE;
		smmu_writel(smmu, value, group->reg);
	}
}

static void tegra_smmu_disable(struct tegra_smmu *smmu, unsigned int swgroup,
			       unsigned int asid)
{
	const struct tegra_smmu_swgroup *group;
	unsigned int i;
	u32 value;

	group = tegra_smmu_find_swgroup(smmu, swgroup);
	if (group) {
		value = smmu_readl(smmu, group->reg);
		value &= ~SMMU_ASID_MASK;
		value |= SMMU_ASID_VALUE(asid);
		value &= ~SMMU_ASID_ENABLE;
		smmu_writel(smmu, value, group->reg);
	}

	for (i = 0; i < smmu->soc->num_clients; i++) {
		const struct tegra_mc_client *client = &smmu->soc->clients[i];

		if (client->swgroup != swgroup)
			continue;

		value = smmu_readl(smmu, client->smmu.reg);
		value &= ~BIT(client->smmu.bit);
		smmu_writel(smmu, value, client->smmu.reg);
	}
}

static int tegra_smmu_as_prepare(struct tegra_smmu *smmu,
				 struct tegra_smmu_as *as)
{
	u32 value;
	int err;

	if (as->use_count > 0) {
		as->use_count++;
		return 0;
	}

	err = tegra_smmu_alloc_asid(smmu, &as->id);
	if (err < 0)
		return err;

	smmu->soc->ops->flush_dcache(as->pd, 0, SMMU_SIZE_PD);
	smmu_flush_ptc(smmu, as->pd, 0);
	smmu_flush_tlb_asid(smmu, as->id);

	smmu_writel(smmu, as->id & 0x7f, SMMU_PTB_ASID);
	value = SMMU_PTB_DATA_VALUE(as->pd, as->attr);
	smmu_writel(smmu, value, SMMU_PTB_DATA);
	smmu_flush(smmu);

	as->smmu = smmu;
	as->use_count++;

	return 0;
}

static void tegra_smmu_as_unprepare(struct tegra_smmu *smmu,
				    struct tegra_smmu_as *as)
{
	if (--as->use_count > 0)
		return;

	tegra_smmu_free_asid(smmu, as->id);
	as->smmu = NULL;
}

static int tegra_smmu_attach_dev(struct iommu_domain *domain,
				 struct device *dev)
{
	struct tegra_smmu *smmu = dev->archdata.iommu;
	struct tegra_smmu_as *as = to_smmu_as(domain);
	struct device_node *np = dev->of_node;
	struct of_phandle_args args;
	unsigned int index = 0;
	int err = 0;

	while (!of_parse_phandle_with_args(np, "iommus", "#iommu-cells", index,
					   &args)) {
		unsigned int swgroup = args.args[0];

		if (args.np != smmu->dev->of_node) {
			of_node_put(args.np);
			continue;
		}

		of_node_put(args.np);

		err = tegra_smmu_as_prepare(smmu, as);
		if (err < 0)
			return err;

		tegra_smmu_enable(smmu, swgroup, as->id);
		index++;
	}

	if (index == 0)
		return -ENODEV;

	return 0;
}

static void tegra_smmu_detach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct tegra_smmu_as *as = to_smmu_as(domain);
	struct device_node *np = dev->of_node;
	struct tegra_smmu *smmu = as->smmu;
	struct of_phandle_args args;
	unsigned int index = 0;

	while (!of_parse_phandle_with_args(np, "iommus", "#iommu-cells", index,
					   &args)) {
		unsigned int swgroup = args.args[0];

		if (args.np != smmu->dev->of_node) {
			of_node_put(args.np);
			continue;
		}

		of_node_put(args.np);

		tegra_smmu_disable(smmu, swgroup, as->id);
		tegra_smmu_as_unprepare(smmu, as);
		index++;
	}
}

static u32 *as_get_pte(struct tegra_smmu_as *as, dma_addr_t iova,
		       struct page **pagep)
{
	u32 *pd = page_address(as->pd), *pt, *count;
	u32 pde = (iova >> SMMU_PDE_SHIFT) & 0x3ff;
	u32 pte = (iova >> SMMU_PTE_SHIFT) & 0x3ff;
	struct tegra_smmu *smmu = as->smmu;
	struct page *page;
	unsigned int i;

	if (pd[pde] == 0) {
		page = alloc_page(GFP_KERNEL | __GFP_DMA);
		if (!page)
			return NULL;

		pt = page_address(page);
		SetPageReserved(page);

		for (i = 0; i < SMMU_NUM_PTE; i++)
			pt[i] = 0;

		smmu->soc->ops->flush_dcache(page, 0, SMMU_SIZE_PT);

		pd[pde] = SMMU_MK_PDE(page, SMMU_PDE_ATTR | SMMU_PDE_NEXT);

		smmu->soc->ops->flush_dcache(as->pd, pde << 2, 4);
		smmu_flush_ptc(smmu, as->pd, pde << 2);
		smmu_flush_tlb_section(smmu, as->id, iova);
		smmu_flush(smmu);
	} else {
		page = pfn_to_page(pd[pde] & smmu->pfn_mask);
		pt = page_address(page);
	}

	*pagep = page;

	/* Keep track of entries in this page table. */
	count = page_address(as->count);
	if (pt[pte] == 0)
		count[pde]++;

	return &pt[pte];
}

static void as_put_pte(struct tegra_smmu_as *as, dma_addr_t iova)
{
	u32 pde = (iova >> SMMU_PDE_SHIFT) & 0x3ff;
	u32 pte = (iova >> SMMU_PTE_SHIFT) & 0x3ff;
	u32 *count = page_address(as->count);
	u32 *pd = page_address(as->pd), *pt;
	struct page *page;

	page = pfn_to_page(pd[pde] & as->smmu->pfn_mask);
	pt = page_address(page);

	/*
	 * When no entries in this page table are used anymore, return the
	 * memory page to the system.
	 */
	if (pt[pte] != 0) {
		if (--count[pde] == 0) {
			ClearPageReserved(page);
			__free_page(page);
			pd[pde] = 0;
		}

		pt[pte] = 0;
	}
}

static int tegra_smmu_map(struct iommu_domain *domain, unsigned long iova,
			  phys_addr_t paddr, size_t size, int prot)
{
	struct tegra_smmu_as *as = to_smmu_as(domain);
	struct tegra_smmu *smmu = as->smmu;
	unsigned long offset;
	struct page *page;
	u32 *pte;

	pte = as_get_pte(as, iova, &page);
	if (!pte)
		return -ENOMEM;

	*pte = __phys_to_pfn(paddr) | SMMU_PTE_ATTR;
	offset = offset_in_page(pte);

	smmu->soc->ops->flush_dcache(page, offset, 4);
	smmu_flush_ptc(smmu, page, offset);
	smmu_flush_tlb_group(smmu, as->id, iova);
	smmu_flush(smmu);

	return 0;
}

static size_t tegra_smmu_unmap(struct iommu_domain *domain, unsigned long iova,
			       size_t size)
{
	struct tegra_smmu_as *as = to_smmu_as(domain);
	struct tegra_smmu *smmu = as->smmu;
	unsigned long offset;
	struct page *page;
	u32 *pte;

	pte = as_get_pte(as, iova, &page);
	if (!pte)
		return 0;

	offset = offset_in_page(pte);
	as_put_pte(as, iova);

	smmu->soc->ops->flush_dcache(page, offset, 4);
	smmu_flush_ptc(smmu, page, offset);
	smmu_flush_tlb_group(smmu, as->id, iova);
	smmu_flush(smmu);

	return size;
}

static phys_addr_t tegra_smmu_iova_to_phys(struct iommu_domain *domain,
					   dma_addr_t iova)
{
	struct tegra_smmu_as *as = to_smmu_as(domain);
	struct page *page;
	unsigned long pfn;
	u32 *pte;

	pte = as_get_pte(as, iova, &page);
	pfn = *pte & as->smmu->pfn_mask;

	return PFN_PHYS(pfn);
}

static struct tegra_smmu *tegra_smmu_find(struct device_node *np)
{
	struct platform_device *pdev;
	struct tegra_mc *mc;

	pdev = of_find_device_by_node(np);
	if (!pdev)
		return NULL;

	mc = platform_get_drvdata(pdev);
	if (!mc)
		return NULL;

	return mc->smmu;
}

static int tegra_smmu_add_device(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct of_phandle_args args;
	unsigned int index = 0;

	while (of_parse_phandle_with_args(np, "iommus", "#iommu-cells", index,
					  &args) == 0) {
		struct tegra_smmu *smmu;

		smmu = tegra_smmu_find(args.np);
		if (smmu) {
			/*
			 * Only a single IOMMU master interface is currently
			 * supported by the Linux kernel, so abort after the
			 * first match.
			 */
			dev->archdata.iommu = smmu;
			break;
		}

		index++;
	}

	return 0;
}

static void tegra_smmu_remove_device(struct device *dev)
{
	dev->archdata.iommu = NULL;
}

static const struct iommu_ops tegra_smmu_ops = {
	.capable = tegra_smmu_capable,
	.domain_alloc = tegra_smmu_domain_alloc,
	.domain_free = tegra_smmu_domain_free,
	.attach_dev = tegra_smmu_attach_dev,
	.detach_dev = tegra_smmu_detach_dev,
	.add_device = tegra_smmu_add_device,
	.remove_device = tegra_smmu_remove_device,
	.map = tegra_smmu_map,
	.unmap = tegra_smmu_unmap,
	.map_sg = default_iommu_map_sg,
	.iova_to_phys = tegra_smmu_iova_to_phys,

	.pgsize_bitmap = SZ_4K,
};

static void tegra_smmu_ahb_enable(void)
{
	static const struct of_device_id ahb_match[] = {
		{ .compatible = "nvidia,tegra30-ahb", },
		{ }
	};
	struct device_node *ahb;

	ahb = of_find_matching_node(NULL, ahb_match);
	if (ahb) {
		tegra_ahb_enable_smmu(ahb);
		of_node_put(ahb);
	}
}

struct tegra_smmu *tegra_smmu_probe(struct device *dev,
				    const struct tegra_smmu_soc *soc,
				    struct tegra_mc *mc)
{
	struct tegra_smmu *smmu;
	size_t size;
	u32 value;
	int err;

	/* This can happen on Tegra20 which doesn't have an SMMU */
	if (!soc)
		return NULL;

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu)
		return ERR_PTR(-ENOMEM);

	/*
	 * This is a bit of a hack. Ideally we'd want to simply return this
	 * value. However the IOMMU registration process will attempt to add
	 * all devices to the IOMMU when bus_set_iommu() is called. In order
	 * not to rely on global variables to track the IOMMU instance, we
	 * set it here so that it can be looked up from the .add_device()
	 * callback via the IOMMU device's .drvdata field.
	 */
	mc->smmu = smmu;

	size = BITS_TO_LONGS(soc->num_asids) * sizeof(long);

	smmu->asids = devm_kzalloc(dev, size, GFP_KERNEL);
	if (!smmu->asids)
		return ERR_PTR(-ENOMEM);

	mutex_init(&smmu->lock);

	smmu->regs = mc->regs;
	smmu->soc = soc;
	smmu->dev = dev;
	smmu->mc = mc;

	smmu->pfn_mask = BIT_MASK(mc->soc->num_address_bits - PAGE_SHIFT) - 1;
	dev_dbg(dev, "address bits: %u, PFN mask: %#lx\n",
		mc->soc->num_address_bits, smmu->pfn_mask);
	smmu->tlb_mask = (smmu->soc->num_tlb_lines << 1) - 1;
	dev_dbg(dev, "TLB lines: %u, mask: %#lx\n", smmu->soc->num_tlb_lines,
		smmu->tlb_mask);

	value = SMMU_PTC_CONFIG_ENABLE | SMMU_PTC_CONFIG_INDEX_MAP(0x3f);

	if (soc->supports_request_limit)
		value |= SMMU_PTC_CONFIG_REQ_LIMIT(8);

	smmu_writel(smmu, value, SMMU_PTC_CONFIG);

	value = SMMU_TLB_CONFIG_HIT_UNDER_MISS |
		SMMU_TLB_CONFIG_ACTIVE_LINES(smmu);

	if (soc->supports_round_robin_arbitration)
		value |= SMMU_TLB_CONFIG_ROUND_ROBIN_ARBITRATION;

	smmu_writel(smmu, value, SMMU_TLB_CONFIG);

	smmu_flush_ptc(smmu, NULL, 0);
	smmu_flush_tlb(smmu);
	smmu_writel(smmu, SMMU_CONFIG_ENABLE, SMMU_CONFIG);
	smmu_flush(smmu);

	tegra_smmu_ahb_enable();

	err = bus_set_iommu(&platform_bus_type, &tegra_smmu_ops);
	if (err < 0)
		return ERR_PTR(err);

	return smmu;
}
