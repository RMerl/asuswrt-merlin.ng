// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <iommu.h>
#include <lmb.h>
#include <memalign.h>
#include <asm/io.h>

#define DART_PARAMS2		0x0004
#define  DART_PARAMS2_BYPASS_SUPPORT	BIT(0)

#define DART_T8020_TLB_CMD		0x0020
#define  DART_T8020_TLB_CMD_FLUSH		BIT(20)
#define  DART_T8020_TLB_CMD_BUSY		BIT(2)
#define DART_T8020_TLB_SIDMASK		0x0034
#define DART_T8020_ERROR		0x0040
#define DART_T8020_ERROR_ADDR_LO	0x0050
#define DART_T8020_ERROR_ADDR_HI	0x0054
#define DART_T8020_CONFIG		0x0060
#define  DART_T8020_CONFIG_LOCK			BIT(15)
#define DART_T8020_SID_ENABLE		0x00fc
#define DART_T8020_TCR_BASE		0x0100
#define  DART_T8020_TCR_TRANSLATE_ENABLE	BIT(7)
#define  DART_T8020_TCR_BYPASS_DART		BIT(8)
#define  DART_T8020_TCR_BYPASS_DAPF		BIT(12)
#define DART_T8020_TTBR_BASE		0x0200
#define  DART_T8020_TTBR_VALID			BIT(31)

#define DART_T8110_PARAMS4		0x000c
#define  DART_T8110_PARAMS4_NSID_MASK		(0x1ff << 0)
#define DART_T8110_TLB_CMD		0x0080
#define  DART_T8110_TLB_CMD_BUSY		BIT(31)
#define  DART_T8110_TLB_CMD_FLUSH_ALL		BIT(8)
#define DART_T8110_ERROR		0x0100
#define DART_T8110_ERROR_MASK		0x0104
#define DART_T8110_ERROR_ADDR_LO	0x0170
#define DART_T8110_ERROR_ADDR_HI	0x0174
#define DART_T8110_PROTECT		0x0200
#define  DART_T8110_PROTECT_TTBR_TCR		BIT(0)
#define DART_T8110_SID_ENABLE_BASE	0x0c00
#define DART_T8110_TCR_BASE		0x1000
#define  DART_T8110_TCR_BYPASS_DAPF		BIT(2)
#define  DART_T8110_TCR_BYPASS_DART		BIT(1)
#define  DART_T8110_TCR_TRANSLATE_ENABLE	BIT(0)
#define DART_T8110_TTBR_BASE		0x1400
#define  DART_T8110_TTBR_VALID			BIT(0)

#define DART_SID_ENABLE(priv, idx) \
	((priv)->sid_enable_base + 4 * (idx))
#define DART_TCR(priv, sid)	((priv)->tcr_base + 4 * (sid))
#define DART_TTBR(priv, sid, idx)	\
	((priv)->ttbr_base + 4 * (priv)->nttbr * (sid) + 4 * (idx))
#define  DART_TTBR_SHIFT	12

#define DART_ALL_STREAMS(priv)	((1U << (priv)->nsid) - 1)

#define DART_PAGE_SIZE		SZ_16K
#define DART_PAGE_MASK		(DART_PAGE_SIZE - 1)

#define DART_L1_TABLE		0x3
#define DART_L2_INVAL		0
#define DART_L2_VALID		BIT(0)
#define DART_L2_FULL_PAGE	BIT(1)
#define DART_L2_START(addr)	((((addr) & DART_PAGE_MASK) >> 2) << 52)
#define DART_L2_END(addr)	((((addr) & DART_PAGE_MASK) >> 2) << 40)

struct apple_dart_priv {
	void *base;
	struct lmb lmb;
	u64 *l1, *l2;
	int bypass, shift;

	dma_addr_t dvabase;
	dma_addr_t dvaend;

	int nsid;
	int nttbr;
	int sid_enable_base;
	int tcr_base;
	u32 tcr_translate_enable;
	u32 tcr_bypass;
	int ttbr_base;
	u32 ttbr_valid;
	void (*flush_tlb)(struct apple_dart_priv *priv);
};

static void apple_dart_t8020_flush_tlb(struct apple_dart_priv *priv)
{
	dsb();

	writel(DART_ALL_STREAMS(priv), priv->base + DART_T8020_TLB_SIDMASK);
	writel(DART_T8020_TLB_CMD_FLUSH, priv->base + DART_T8020_TLB_CMD);
	while (readl(priv->base + DART_T8020_TLB_CMD) &
	       DART_T8020_TLB_CMD_BUSY)
		continue;
}

static void apple_dart_t8110_flush_tlb(struct apple_dart_priv *priv)
{
	dsb();

	writel(DART_T8110_TLB_CMD_FLUSH_ALL,
	       priv->base + DART_T8110_TLB_CMD_FLUSH_ALL);
	while (readl(priv->base + DART_T8110_TLB_CMD) &
	       DART_T8110_TLB_CMD_BUSY)
		continue;
}

static dma_addr_t apple_dart_map(struct udevice *dev, void *addr, size_t size)
{
	struct apple_dart_priv *priv = dev_get_priv(dev);
	phys_addr_t paddr, dva;
	phys_size_t psize, off;
	int i, idx;

	if (priv->bypass)
		return (phys_addr_t)addr;

	paddr = ALIGN_DOWN((phys_addr_t)addr, DART_PAGE_SIZE);
	off = (phys_addr_t)addr - paddr;
	psize = ALIGN(size + off, DART_PAGE_SIZE);

	dva = lmb_alloc(&priv->lmb, psize, DART_PAGE_SIZE);

	idx = dva / DART_PAGE_SIZE;
	for (i = 0; i < psize / DART_PAGE_SIZE; i++) {
		priv->l2[idx + i] = (paddr  >> priv->shift) | DART_L2_VALID |
			DART_L2_START(0LL) | DART_L2_END(~0LL);
		paddr += DART_PAGE_SIZE;
	}
	flush_dcache_range((unsigned long)&priv->l2[idx],
			   (unsigned long)&priv->l2[idx + i]);
	priv->flush_tlb(priv);

	return dva + off;
}

static void apple_dart_unmap(struct udevice *dev, dma_addr_t addr, size_t size)
{
	struct apple_dart_priv *priv = dev_get_priv(dev);
	phys_addr_t dva;
	phys_size_t psize;
	int i, idx;

	if (priv->bypass)
		return;

	dva = ALIGN_DOWN(addr, DART_PAGE_SIZE);
	psize = size + (addr - dva);
	psize = ALIGN(psize, DART_PAGE_SIZE);

	idx = dva / DART_PAGE_SIZE;
	for (i = 0; i < psize / DART_PAGE_SIZE; i++)
		priv->l2[idx + i] = DART_L2_INVAL;
	flush_dcache_range((unsigned long)&priv->l2[idx],
			   (unsigned long)&priv->l2[idx + i]);
	priv->flush_tlb(priv);

	lmb_free(&priv->lmb, dva, psize);
}

static struct iommu_ops apple_dart_ops = {
	.map = apple_dart_map,
	.unmap = apple_dart_unmap,
};

static int apple_dart_probe(struct udevice *dev)
{
	struct apple_dart_priv *priv = dev_get_priv(dev);
	dma_addr_t addr;
	phys_addr_t l2;
	int ntte, nl1, nl2;
	int sid, i;
	u32 params2, params4;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	if (device_is_compatible(dev, "apple,t8110-dart")) {
		params4 = readl(priv->base + DART_T8110_PARAMS4);
		priv->nsid = params4 & DART_T8110_PARAMS4_NSID_MASK;
		priv->nttbr = 1;
		priv->sid_enable_base = DART_T8110_SID_ENABLE_BASE;
		priv->tcr_base = DART_T8110_TCR_BASE;
		priv->tcr_translate_enable = DART_T8110_TCR_TRANSLATE_ENABLE;
		priv->tcr_bypass =
		    DART_T8110_TCR_BYPASS_DAPF | DART_T8110_TCR_BYPASS_DART;
		priv->ttbr_base = DART_T8110_TTBR_BASE;
		priv->ttbr_valid = DART_T8110_TTBR_VALID;
		priv->flush_tlb = apple_dart_t8110_flush_tlb;
	} else {
		priv->nsid = 16;
		priv->nttbr = 4;
		priv->sid_enable_base = DART_T8020_SID_ENABLE;
		priv->tcr_base = DART_T8020_TCR_BASE;
		priv->tcr_translate_enable = DART_T8020_TCR_TRANSLATE_ENABLE;
		priv->tcr_bypass =
		    DART_T8020_TCR_BYPASS_DAPF | DART_T8020_TCR_BYPASS_DART;
		priv->ttbr_base = DART_T8020_TTBR_BASE;
		priv->ttbr_valid = DART_T8020_TTBR_VALID;
		priv->flush_tlb = apple_dart_t8020_flush_tlb;
	}

	if (device_is_compatible(dev, "apple,t6000-dart") ||
	    device_is_compatible(dev, "apple,t8110-dart"))
		priv->shift = 4;

	priv->dvabase = DART_PAGE_SIZE;
	priv->dvaend = SZ_4G - DART_PAGE_SIZE;

	lmb_init(&priv->lmb);
	lmb_add(&priv->lmb, priv->dvabase, priv->dvaend - priv->dvabase);

	/* Disable translations. */
	for (sid = 0; sid < priv->nsid; sid++)
		writel(0, priv->base + DART_TCR(priv, sid));

	/* Remove page tables. */
	for (sid = 0; sid < priv->nsid; sid++) {
		for (i = 0; i < priv->nttbr; i++)
			writel(0, priv->base + DART_TTBR(priv, sid, i));
	}
	priv->flush_tlb(priv);

	params2 = readl(priv->base + DART_PARAMS2);
	if (params2 & DART_PARAMS2_BYPASS_SUPPORT) {
		for (sid = 0; sid < priv->nsid; sid++) {
			writel(priv->tcr_bypass,
			       priv->base + DART_TCR(priv, sid));
		}
		priv->bypass = 1;
		return 0;
	}

	ntte = DIV_ROUND_UP(priv->dvaend, DART_PAGE_SIZE);
	nl2 = DIV_ROUND_UP(ntte, DART_PAGE_SIZE / sizeof(u64));
	nl1 = DIV_ROUND_UP(nl2, DART_PAGE_SIZE / sizeof(u64));

	priv->l2 = memalign(DART_PAGE_SIZE, nl2 * DART_PAGE_SIZE);
	memset(priv->l2, 0, nl2 * DART_PAGE_SIZE);
	flush_dcache_range((unsigned long)priv->l2,
			   (unsigned long)priv->l2 + nl2 * DART_PAGE_SIZE);

	priv->l1 = memalign(DART_PAGE_SIZE, nl1 * DART_PAGE_SIZE);
	memset(priv->l1, 0, nl1 * DART_PAGE_SIZE);
	l2 = (phys_addr_t)priv->l2;
	for (i = 0; i < nl2; i++) {
		priv->l1[i] = (l2 >> priv->shift) | DART_L1_TABLE;
		l2 += DART_PAGE_SIZE;
	}
	flush_dcache_range((unsigned long)priv->l1,
			   (unsigned long)priv->l1 + nl1 * DART_PAGE_SIZE);

	/* Install page tables. */
	for (sid = 0; sid < priv->nsid; sid++) {
		addr = (phys_addr_t)priv->l1;
		for (i = 0; i < nl1; i++) {
			writel(addr >> DART_TTBR_SHIFT | priv->ttbr_valid,
			       priv->base + DART_TTBR(priv, sid, i));
			addr += DART_PAGE_SIZE;
		}
	}
	priv->flush_tlb(priv);

	/* Enable all streams. */
	for (i = 0; i < priv->nsid / 32; i++)
		writel(~0, priv->base + DART_SID_ENABLE(priv, i));

	/* Enable translations. */
	for (sid = 0; sid < priv->nsid; sid++) {
		writel(priv->tcr_translate_enable,
		       priv->base + DART_TCR(priv, sid));
	}

	return 0;
}

static int apple_dart_remove(struct udevice *dev)
{
	struct apple_dart_priv *priv = dev_get_priv(dev);
	int sid, i;

	/* Disable translations. */
	for (sid = 0; sid < priv->nsid; sid++)
		writel(0, priv->base + DART_TCR(priv, sid));

	/* Remove page tables. */
	for (sid = 0; sid < priv->nsid; sid++) {
		for (i = 0; i < priv->nttbr; i++)
			writel(0, priv->base + DART_TTBR(priv, sid, i));
	}
	priv->flush_tlb(priv);

	return 0;
}

static const struct udevice_id apple_dart_ids[] = {
	{ .compatible = "apple,t8103-dart" },
	{ .compatible = "apple,t6000-dart" },
	{ .compatible = "apple,t8110-dart" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_dart) = {
	.name = "apple_dart",
	.id = UCLASS_IOMMU,
	.of_match = apple_dart_ids,
	.priv_auto = sizeof(struct apple_dart_priv),
	.ops = &apple_dart_ops,
	.probe = apple_dart_probe,
	.remove = apple_dart_remove,
	.flags	= DM_FLAG_OS_PREPARE
};
