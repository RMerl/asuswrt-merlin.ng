// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Linaro Ltd.
 * Basic ARM SMMU-500 driver, assuming a pre-initialised SMMU and only IDENTITY domains
 * this driver only implements the bare minimum to configure stream mappings for periphals
 * used by u-boot on platforms where the SMMU can't be disabled.
 */

#include <log.h>
#include <cpu_func.h>
#include <dm.h>
#include <iommu.h>
#include <linux/bitfield.h>
#include <linux/list.h>
#include <linux/err.h>
#include <lmb.h>
#include <memalign.h>
#include <asm/io.h>

#define ARM_SMMU_GR0 0
#define ARM_SMMU_GR1 1

#define ARM_SMMU_GR0_ID0 0x20
#define ARM_SMMU_ID0_NUMSMRG GENMASK(7, 0) /* Number of stream mapping groups */
#define ARM_SMMU_GR0_ID1 0x24
#define ARM_SMMU_ID1_PAGESIZE \
	BIT(31) /* Page shift is 16 bits when set, otherwise 23 */
#define ARM_SMMU_ID1_NUMPAGENDXB \
	GENMASK(30, 28) /* Number of pages before context banks */
#define ARM_SMMU_ID1_NUMCB GENMASK(7, 0) /* Number of context banks supported */

#define ARM_SMMU_GR1_CBAR(n) (0x0 + ((n) << 2))
#define ARM_SMMU_CBAR_TYPE GENMASK(17, 16)
#define ARM_SMMU_CBAR_VMID GENMASK(7, 0)
enum arm_smmu_cbar_type {
	CBAR_TYPE_S2_TRANS,
	CBAR_TYPE_S1_TRANS_S2_BYPASS,
	CBAR_TYPE_S1_TRANS_S2_FAULT,
	CBAR_TYPE_S1_TRANS_S2_TRANS,
};

#define ARM_SMMU_GR1_CBA2R(n) (0x800 + ((n) << 2))
#define ARM_SMMU_CBA2R_VA64 BIT(0)

/* Per-CB system control register */
#define ARM_SMMU_CB_SCTLR 0x0
#define ARM_SMMU_SCTLR_CFCFG BIT(7) /* Stall on context fault */
#define ARM_SMMU_SCTLR_CFIE BIT(6) /* Context fault interrupt enable */
#define ARM_SMMU_SCTLR_CFRE BIT(5) /* Abort on context fault */

/* Translation Table Base, holds address of translation table in memory to be used
 * for this context bank. Or 0 for bypass
 */
#define ARM_SMMU_CB_TTBR0 0x20
#define ARM_SMMU_CB_TTBR1 0x28
/* Translation Control Register, configured TTBR/TLB behaviour (0 for bypass) */
#define ARM_SMMU_CB_TCR 0x30
/* Memory Attribute Indirection, also 0 for bypass */
#define ARM_SMMU_CB_S1_MAIR0 0x38
#define ARM_SMMU_CB_S1_MAIR1 0x3c

#define ARM_SMMU_GR0_SMR(n) (0x800 + ((n) << 2))
#define ARM_SMMU_SMR_VALID BIT(31)
#define ARM_SMMU_SMR_MASK GENMASK(31, 16) // Always 0 for now??
#define ARM_SMMU_SMR_ID GENMASK(15, 0)

#define ARM_SMMU_GR0_S2CR(n) (0xc00 + ((n) << 2))
#define ARM_SMMU_S2CR_PRIVCFG GENMASK(25, 24)

enum arm_smmu_s2cr_privcfg {
	S2CR_PRIVCFG_DEFAULT,
	S2CR_PRIVCFG_DIPAN,
	S2CR_PRIVCFG_UNPRIV,
	S2CR_PRIVCFG_PRIV,
};

#define ARM_SMMU_S2CR_TYPE GENMASK(17, 16)

enum arm_smmu_s2cr_type {
	S2CR_TYPE_TRANS,
	S2CR_TYPE_BYPASS,
	S2CR_TYPE_FAULT,
};

#define ARM_SMMU_S2CR_EXIDVALID BIT(10)
#define ARM_SMMU_S2CR_CBNDX GENMASK(7, 0)

#define VMID_UNUSED 0xff

struct qcom_smmu_priv {
	phys_addr_t base;
	struct list_head devices;
	struct udevice *dev;

	/* Read-once config */
	int num_cb;
	int num_smr;
	u32 pgshift;
	u32 cb_pg_offset;
};

struct mmu_dev {
	struct list_head li;
	struct udevice *dev;
	u16 sid;
	u16 cbx;
	u16 smr;
};

#define page_addr(priv, page) ((priv)->base + ((page) << (priv)->pgshift))

#define smmu_readl(priv, page, offset) readl(page_addr(priv, page) + offset)
#define gr0_readl(priv, offset) smmu_readl(priv, ARM_SMMU_GR0, offset)
#define gr1_readl(priv, offset) smmu_readl(priv, ARM_SMMU_GR1, offset)
#define cbx_readl(priv, cbx, offset) \
	smmu_readl(priv, (priv->cb_pg_offset) + cbx, offset)

#define smmu_writel(priv, page, offset, value) \
	writel((value), page_addr(priv, page) + offset)
#define gr0_writel(priv, offset, value) \
	smmu_writel(priv, ARM_SMMU_GR0, offset, (value))
#define gr1_writel(priv, offset, value) \
	smmu_writel(priv, ARM_SMMU_GR1, offset, (value))
#define cbx_writel(priv, cbx, offset, value) \
	smmu_writel(priv, (priv->cb_pg_offset) + cbx, offset, value)

#define gr1_setbits(priv, offset, value) \
	gr1_writel(priv, offset, gr1_readl(priv, offset) | (value))

static int get_stream_id(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	struct ofnode_phandle_args args;
	int count = ofnode_parse_phandle_with_args(node, "iommus",
						   "#iommu-cells", 0, 0, &args);

	if (count < 0 || args.args[0] == 0) {
		printf("Error: %s: iommus property not found or wrong number of cells\n",
		       __func__);
		return -EINVAL;
	}

	return args.args[0]; // Some mask from bit 16 onward?
}

static struct mmu_dev *alloc_dev(struct udevice *dev)
{
	struct qcom_smmu_priv *priv = dev_get_priv(dev->iommu);
	struct mmu_dev *mmu_dev;
	int sid;

	sid = get_stream_id(dev);
	debug("%s %s has SID %#x\n", dev->iommu->name, dev->name, sid);
	if (sid < 0 || sid > 0xffff) {
		printf("\tSMMU: Invalid stream ID for %s\n", dev->name);
		return ERR_PTR(-EINVAL);
	}

	/* We only support a single SID per device for now */
	list_for_each_entry(mmu_dev, &priv->devices, li) {
		if (mmu_dev->sid == sid)
			return ERR_PTR(-EEXIST);
	}

	mmu_dev = calloc(sizeof(*mmu_dev), 1);
	if (!mmu_dev)
		return ERR_PTR(-ENOMEM);

	mmu_dev->dev = dev;
	mmu_dev->sid = sid;

	list_add_tail(&mmu_dev->li, &priv->devices);

	return mmu_dev;
}

/* Find and init the first free context bank */
static int alloc_cb(struct qcom_smmu_priv *priv)
{
	u32 cbar, type, vmid, val;

	for (int i = 0; i < priv->num_cb; i++) {
		cbar = gr1_readl(priv, ARM_SMMU_GR1_CBAR(i));
		type = FIELD_GET(ARM_SMMU_CBAR_TYPE, cbar);
		vmid = FIELD_GET(ARM_SMMU_CBAR_VMID, cbar);

		/* Check that the context bank is available. We haven't reset the SMMU so
		 * we just make a best guess.
		 */
		if (type != CBAR_TYPE_S2_TRANS &&
		    (type != CBAR_TYPE_S1_TRANS_S2_BYPASS ||
		     vmid != VMID_UNUSED))
			continue;

		debug("%s: Found free context bank %d (cbar %#x)\n",
		      priv->dev->name, i, cbar);
		type = CBAR_TYPE_S1_TRANS_S2_BYPASS;
		vmid = 0;
		cbar &= ~ARM_SMMU_CBAR_TYPE & ~ARM_SMMU_CBAR_VMID;
		cbar |= FIELD_PREP(ARM_SMMU_CBAR_TYPE, type) |
			FIELD_PREP(ARM_SMMU_CBAR_VMID, vmid);
		gr1_writel(priv, ARM_SMMU_GR1_CBAR(i), cbar);

		val = IS_ENABLED(CONFIG_ARM64) == 1 ? ARM_SMMU_CBA2R_VA64 : 0;
		gr1_setbits(priv, ARM_SMMU_GR1_CBA2R(i), val);
		return i;
	}

	return -1;
}

/* Search for a context bank that is already configured for this stream
 * returns the context bank index or -ENOENT
 */
static int find_smr(struct qcom_smmu_priv *priv, u16 stream_id)
{
	u32 val;
	int i;

	for (i = 0; i < priv->num_smr; i++) {
		val = gr0_readl(priv, ARM_SMMU_GR0_SMR(i));
		if (!(val & ARM_SMMU_SMR_VALID) ||
		    FIELD_GET(ARM_SMMU_SMR_ID, val) != stream_id)
			continue;

		return i;
	}

	return -ENOENT;
}

static int configure_smr_s2cr(struct qcom_smmu_priv *priv, struct mmu_dev *mdev)
{
	u32 val;
	int i;

	for (i = 0; i < priv->num_smr; i++) {
		/* Configure SMR */
		val = gr0_readl(priv, ARM_SMMU_GR0_SMR(i));
		if (val & ARM_SMMU_SMR_VALID)
			continue;

		val = mdev->sid | ARM_SMMU_SMR_VALID;
		gr0_writel(priv, ARM_SMMU_GR0_SMR(i), val);

		/*
		 * WARNING: Don't change this to use S2CR_TYPE_BYPASS!
		 * Some Qualcomm boards have angry hypervisor firmware
		 * that converts S2CR type BYPASS to type FAULT on write.
		 * We don't use virtual addressing for these boards in
		 * u-boot so we can get away with using S2CR_TYPE_TRANS
		 * instead
		 */
		val = FIELD_PREP(ARM_SMMU_S2CR_TYPE, S2CR_TYPE_TRANS) |
		      FIELD_PREP(ARM_SMMU_S2CR_CBNDX, mdev->cbx);
		gr0_writel(priv, ARM_SMMU_GR0_S2CR(i), val);

		mdev->smr = i;
		break;
	}

	/* Make sure our writes went through */
	mb();

	return 0;
}

static int qcom_smmu_connect(struct udevice *dev)
{
	struct mmu_dev *mdev;
	struct qcom_smmu_priv *priv;
	int ret;

	debug("%s: %s -> %s\n", __func__, dev->name, dev->iommu->name);

	priv = dev_get_priv(dev->iommu);
	if (WARN_ON(!priv))
		return -EINVAL;

	mdev = alloc_dev(dev);
	if (IS_ERR(mdev) && PTR_ERR(mdev) != -EEXIST) {
		printf("%s: %s Couldn't create mmu context\n", __func__,
		       dev->name);
		return PTR_ERR(mdev);
	} else if (IS_ERR(mdev)) { // -EEXIST
		return 0;
	}

	if (find_smr(priv, mdev->sid) >= 0) {
		debug("Found existing context bank for %s, skipping init\n",
		      dev->name);
		return 0;
	}

	ret = alloc_cb(priv);
	if (ret < 0 || ret > 0xff) {
		printf("Error: %s: failed to allocate context bank for %s\n",
		       __func__, dev->name);
		return 0;
	}
	mdev->cbx = ret;

	/* Configure context bank registers */
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_TTBR0, 0x0);
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_TTBR1, 0x0);
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_S1_MAIR0, 0x0);
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_S1_MAIR1, 0x0);
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_SCTLR,
		   ARM_SMMU_SCTLR_CFIE | ARM_SMMU_SCTLR_CFRE |
			   ARM_SMMU_SCTLR_CFCFG);
	cbx_writel(priv, mdev->cbx, ARM_SMMU_CB_TCR, 0x0);

	/* Ensure that our writes went through */
	mb();

	configure_smr_s2cr(priv, mdev);

	return 0;
}

#ifdef DEBUG
static inline void dump_boot_mappings(struct arm_smmu_priv *priv)
{
	u32 val;
	int i;

	debug("  SMMU dump boot mappings:\n");
	for (i = 0; i < priv->num_smr; i++) {
		val = gr0_readl(priv, ARM_SMMU_GR0_SMR(i));
		if (val & ARM_SMMU_SMR_VALID)
			debug("\tSMR %3d: SID: %#lx\n", i,
			      FIELD_GET(ARM_SMMU_SMR_ID, val));
	}
}
#else
#define dump_boot_mappings(priv) \
	do {                     \
	} while (0)
#endif

static int qcom_smmu_probe(struct udevice *dev)
{
	struct qcom_smmu_priv *priv;
	u32 val;

	priv = dev_get_priv(dev);
	priv->dev = dev;
	priv->base = dev_read_addr(dev);
	INIT_LIST_HEAD(&priv->devices);

	/* Read SMMU config */
	val = gr0_readl(priv, ARM_SMMU_GR0_ID0);
	priv->num_smr = FIELD_GET(ARM_SMMU_ID0_NUMSMRG, val);

	val = gr0_readl(priv, ARM_SMMU_GR0_ID1);
	priv->num_cb = FIELD_GET(ARM_SMMU_ID1_NUMCB, val);
	priv->pgshift = FIELD_GET(ARM_SMMU_ID1_PAGESIZE, val) ? 16 : 12;
	priv->cb_pg_offset = 1
			     << (FIELD_GET(ARM_SMMU_ID1_NUMPAGENDXB, val) + 1);

	dump_boot_mappings(priv);

	return 0;
}

static int qcom_smmu_remove(struct udevice *dev)
{
	(void)dev;
	/*
	 * We should probably try and de-configure things here,
	 * however I'm yet to find a way to do it without crashing
	 * and it seems like Linux doesn't care at all anyway.
	 */

	return 0;
}

static struct iommu_ops qcom_smmu_ops = {
	.connect = qcom_smmu_connect,
};

static const struct udevice_id qcom_smmu500_ids[] = {
	{ .compatible = "qcom,sdm845-smmu-500" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(qcom_smmu500) = {
	.name = "qcom_smmu500",
	.id = UCLASS_IOMMU,
	.of_match = qcom_smmu500_ids,
	.priv_auto = sizeof(struct qcom_smmu_priv),
	.ops = &qcom_smmu_ops,
	.probe = qcom_smmu_probe,
	.remove = qcom_smmu_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
