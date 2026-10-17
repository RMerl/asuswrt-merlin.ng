// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <div64.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/mach-imx/mu_hal.h>

#define DID_NUM 16
#define MBC_MAX_NUM 4
#define MRC_MAX_NUM 2
#define MBC_NUM(HWCFG) (((HWCFG) >> 16) & 0xF)
#define MRC_NUM(HWCFG) (((HWCFG) >> 24) & 0x1F)

struct mbc_mem_dom {
	u32 mem_glbcfg[4];
	u32 nse_blk_index;
	u32 nse_blk_set;
	u32 nse_blk_clr;
	u32 nsr_blk_clr_all;
	u32 memn_glbac[8];
	/* The upper only existed in the beginning of each MBC */
	u32 mem0_blk_cfg_w[64];
	u32 mem0_blk_nse_w[16];
	u32 mem1_blk_cfg_w[8];
	u32 mem1_blk_nse_w[2];
	u32 mem2_blk_cfg_w[8];
	u32 mem2_blk_nse_w[2];
	u32 mem3_blk_cfg_w[8];
	u32 mem3_blk_nse_w[2];/*0x1F0, 0x1F4 */
	u32 reserved[2];
};

struct mrc_rgn_dom {
	u32 mrc_glbcfg[4];
	u32 nse_rgn_indirect;
	u32 nse_rgn_set;
	u32 nse_rgn_clr;
	u32 nse_rgn_clr_all;
	u32 memn_glbac[8];
	/* The upper only existed in the beginning of each MRC */
	u32 rgn_desc_words[16][2]; /* 16  regions at max, 2 words per region */
	u32	rgn_nse;
	u32 reserved2[15];
};

struct mda_inst {
	u32 mda_w[8];
};

struct trdc_mgr {
	u32 trdc_cr;
	u32 res0[59];
	u32 trdc_hwcfg0;
	u32 trdc_hwcfg1;
	u32 res1[450];
	struct mda_inst mda[8];
	u32 res2[15808];
};

struct trdc_mbc {
	struct mbc_mem_dom mem_dom[DID_NUM];
};

struct trdc_mrc {
	struct mrc_rgn_dom mrc_dom[DID_NUM];
};

int trdc_mda_set_cpu(ulong trdc_reg, u32 mda_inst, u32 mda_reg, u8 sa, u8 dids,
		     u8 did, u8 pe, u8 pidm, u8 pid)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	u32 *mda_w = &trdc_base->mda[mda_inst].mda_w[mda_reg];
	u32 val = readl(mda_w);

	if (val & BIT(29)) /* non-cpu */
		return -EINVAL;

	val = BIT(31) | ((pid & 0x3f) << 16) | ((pidm & 0x3f) << 8) |
		((pe & 0x3) << 6) | ((sa & 0x3) << 14) | ((dids & 0x3) << 4) |
		(did & 0xf);

	writel(val, mda_w);

	return 0;
}

int trdc_mda_set_noncpu(ulong trdc_reg, u32 mda_inst, u32 mda_reg,
			bool did_bypass, u8 sa, u8 pa, u8 did)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	u32 *mda_w = &trdc_base->mda[mda_inst].mda_w[mda_reg];
	u32 val = readl(mda_w);

	if (!(val & BIT(29))) /* cpu */
		return -EINVAL;

	val = BIT(31) | ((sa & 0x3) << 6) | ((pa & 0x3) << 4) | (did & 0xf);
	if (did_bypass)
		val |= BIT(8);

	writel(val, mda_w);

	return 0;
}

static ulong trdc_get_mbc_base(ulong trdc_reg, u32 mbc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	u32 mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);

	if (mbc_x >= mbc_num)
		return 0;

	return trdc_reg + 0x10000 + 0x2000 * mbc_x;
}

static ulong trdc_get_mrc_base(ulong trdc_reg, u32 mrc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	u32 mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);
	u32 mrc_num = MRC_NUM(trdc_base->trdc_hwcfg0);

	if (mrc_x >= mrc_num)
		return 0;

	return trdc_reg + 0x10000 + 0x2000 * mbc_num + 0x1000 * mrc_x;
}

int trdc_mbc_set_control(ulong trdc_reg, u32 mbc_x, u32 glbac_id, u32 glbac_val)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mbc_dom = &mbc_base->mem_dom[0];

	debug("mbc 0x%lx\n", (ulong)mbc_dom);

	writel(glbac_val, &mbc_dom->memn_glbac[glbac_id]);

	return 0;
}

int trdc_mbc_blk_config(ulong trdc_reg, u32 mbc_x, u32 dom_x, u32 mem_x,
			u32 blk_x, bool sec_access, u32 glbac_id)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;
	u32 *cfg_w, *nse_w;
	u32 index, offset, val;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	mbc_dom = &mbc_base->mem_dom[dom_x];

	debug("mbc 0x%lx\n", (ulong)mbc_dom);

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem0_blk_nse_w[blk_x / 32];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem1_blk_nse_w[blk_x / 32];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem2_blk_nse_w[blk_x / 32];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem3_blk_nse_w[blk_x / 32];
		break;
	default:
		return -EINVAL;
	};

	index = blk_x % 8;
	offset = index * 4;

	val = readl((void __iomem *)cfg_w);

	val &= ~(0xFU << offset);

	/* MBC0-3
	 *  Global 0, 0x7777 secure pri/user read/write/execute, ELE has already set it.
	 *  So select MBC0_MEMN_GLBAC0
	 */
	if (sec_access) {
		val |= ((0x0 | (glbac_id & 0x7)) << offset);
		writel(val, (void __iomem *)cfg_w);
	} else {
		val |= ((0x8 | (glbac_id & 0x7)) << offset); /* nse bit set */
		writel(val, (void __iomem *)cfg_w);
	}

	return 0;
}

int trdc_mrc_set_control(ulong trdc_reg, u32 mrc_x, u32 glbac_id, u32 glbac_val)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;

	if (mrc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mrc_dom = &mrc_base->mrc_dom[0];

	debug("mrc_dom 0x%lx\n", (ulong)mrc_dom);

	writel(glbac_val, &mrc_dom->memn_glbac[glbac_id]);

	return 0;
}

int trdc_mrc_region_config(ulong trdc_reg, u32 mrc_x, u32 dom_x, u32 addr_start,
			   u32 addr_end, bool sec_access, u32 glbac_id)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;
	u32 *desc_w;
	u32 start, end;
	u32 i, free = 8;
	bool vld, hit = false;

	if (mrc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	mrc_dom = &mrc_base->mrc_dom[dom_x];

	addr_start &= ~0x3fff;
	addr_end &= ~0x3fff;

	debug("mrc_dom 0x%lx\n", (ulong)mrc_dom);

	for (i = 0; i < 8; i++) {
		desc_w = &mrc_dom->rgn_desc_words[i][0];

		debug("desc_w 0x%lx\n", (ulong)desc_w);

		start = readl((void __iomem *)desc_w) & (~0x3fff);
		end = readl((void __iomem *)(desc_w + 1));
		vld = end & 0x1;
		end = end & (~0x3fff);

		if (start == 0 && end == 0 && !vld && free >= 8)
			free = i;

		/* Check all the region descriptors, even overlap */
		if (addr_start >= end || addr_end <= start || !vld)
			continue;

		/* MRC0,1
		 *  Global 0, 0x7777 secure pri/user read/write/execute, ELE has already set it.
		 *  So select MRCx_MEMN_GLBAC0
		 */
		if (sec_access) {
			writel(start | (glbac_id & 0x7), (void __iomem *)desc_w);
			writel(end | 0x1, (void __iomem *)(desc_w + 1));
		} else {
			writel(start | (glbac_id & 0x7), (void __iomem *)desc_w);
			writel(end | 0x1 | 0x10, (void __iomem *)(desc_w + 1));
		}

		if (addr_start >= start && addr_end <= end)
			hit = true;
	}

	if (!hit) {
		if (free >= 8)
			return -EFAULT;

		desc_w = &mrc_dom->rgn_desc_words[free][0];

		debug("free desc_w 0x%lx\n", (ulong)desc_w);
		debug("[0x%x] [0x%x]\n", addr_start | (glbac_id & 0x7), addr_end | 0x1);

		if (sec_access) {
			writel(addr_start | (glbac_id & 0x7), (void __iomem *)desc_w);
			writel(addr_end | 0x1, (void __iomem *)(desc_w + 1));
		} else {
			writel(addr_start | (glbac_id & 0x7), (void __iomem *)desc_w);
			writel((addr_end | 0x1 | 0x10), (void __iomem *)(desc_w + 1));
		}
	}

	return 0;
}

bool trdc_mrc_enabled(ulong trdc_base)
{
	return (!!(readl((void __iomem *)trdc_base) & 0x8000));
}

bool trdc_mbc_enabled(ulong trdc_base)
{
	return (!!(readl((void __iomem *)trdc_base) & 0x4000));
}

int release_rdc(u8 xrdc)
{
	ulong s_mu_base = 0x47520000UL;
	struct ele_msg msg;
	int ret;
	u32 rdc_id;

	switch (xrdc) {
	case 0:
		rdc_id = 0x74;
		break;
	case 1:
		rdc_id = 0x78;
		break;
	case 2:
		rdc_id = 0x82;
		break;
	case 3:
		rdc_id = 0x86;
		break;
	default:
		return -EINVAL;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_RELEASE_RDC_REQ;
	msg.data[0] = (rdc_id << 8) | 0x2; /* A55 */

	mu_hal_init(s_mu_base);
	mu_hal_sendmsg(s_mu_base, 0, *((u32 *)&msg));
	mu_hal_sendmsg(s_mu_base, 1, msg.data[0]);

	ret = mu_hal_receivemsg(s_mu_base, 0, (u32 *)&msg);
	if (!ret) {
		ret = mu_hal_receivemsg(s_mu_base, 1, &msg.data[0]);
		if (!ret) {
			if ((msg.data[0] & 0xff) == 0xd6)
				return 0;
		}

		return -EIO;
	}

	return ret;
}

void trdc_early_init(void)
{
	int ret = 0, i;

	ret |= release_rdc(0);
	ret |= release_rdc(2);
	ret |= release_rdc(1);
	ret |= release_rdc(3);

	if (!ret) {
		/* Set OCRAM to RWX for secure, when OEM_CLOSE, the image is RX only */
		trdc_mbc_set_control(0x49010000, 3, 0, 0x7700);

		for (i = 0; i < 40; i++)
			trdc_mbc_blk_config(0x49010000, 3, 3, 0, i, true, 0);

		for (i = 0; i < 40; i++)
			trdc_mbc_blk_config(0x49010000, 3, 3, 1, i, true, 0);

		for (i = 0; i < 40; i++)
			trdc_mbc_blk_config(0x49010000, 3, 0, 0, i, true, 0);

		for (i = 0; i < 40; i++)
			trdc_mbc_blk_config(0x49010000, 3, 0, 1, i, true, 0);
	}
}

void trdc_init(void)
{
	/* TRDC mega */
	if (trdc_mrc_enabled(0x49010000)) {
		/* DDR */
		trdc_mrc_set_control(0x49010000, 0, 0, 0x7777);

		/* ELE */
		trdc_mrc_region_config(0x49010000, 0, 0, 0x80000000, 0xFFFFFFFF, false, 0);

		/* MTR */
		trdc_mrc_region_config(0x49010000, 0, 1, 0x80000000, 0xFFFFFFFF, false, 0);

		/* M33 */
		trdc_mrc_region_config(0x49010000, 0, 2, 0x80000000, 0xFFFFFFFF, false, 0);

		/* A55*/
		trdc_mrc_region_config(0x49010000, 0, 3, 0x80000000, 0xFFFFFFFF, false, 0);

		/* For USDHC1 to DDR, USDHC1 is default force to non-secure */
		trdc_mrc_region_config(0x49010000, 0, 5, 0x80000000, 0xFFFFFFFF, false, 0);

		/* For USDHC2 to DDR, USDHC2 is default force to non-secure */
		trdc_mrc_region_config(0x49010000, 0, 6, 0x80000000, 0xFFFFFFFF, false, 0);

		/* eDMA */
		trdc_mrc_region_config(0x49010000, 0, 7, 0x80000000, 0xFFFFFFFF, false, 0);

		/*CoreSight, TestPort*/
		trdc_mrc_region_config(0x49010000, 0, 8, 0x80000000, 0xFFFFFFFF, false, 0);

		/* DAP */
		trdc_mrc_region_config(0x49010000, 0, 9, 0x80000000, 0xFFFFFFFF, false, 0);

		/*SoC masters */
		trdc_mrc_region_config(0x49010000, 0, 10, 0x80000000, 0xFFFFFFFF, false, 0);

		/*USB*/
		trdc_mrc_region_config(0x49010000, 0, 11, 0x80000000, 0xFFFFFFFF, false, 0);
	}
}

#if DEBUG
int trdc_mbc_control_dump(ulong trdc_reg, u32 mbc_x, u32 glbac_id)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mbc_dom = &mbc_base->mem_dom[0];

	printf("mbc_dom %u glbac %u: 0x%x\n",
	       mbc_x, glbac_id, readl(&mbc_dom->memn_glbac[glbac_id]));

	return 0;
}

int trdc_mbc_mem_dump(ulong trdc_reg, u32 mbc_x, u32 dom_x, u32 mem_x, u32 word)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;
	u32 *cfg_w;

	if (mbc_base == 0)
		return -EINVAL;

	mbc_dom = &mbc_base->mem_dom[dom_x];

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[word];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[word];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[word];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[word];
		break;
	default:
		return -EINVAL;
	};

	printf("mbc_dom %u dom %u mem %u word %u: 0x%x\n",
	       mbc_x, dom_x, mem_x, word, readl((void __iomem *)cfg_w));

	return 0;
}

int trdc_mrc_control_dump(ulong trdc_reg, u32 mrc_x, u32 glbac_id)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;

	if (mrc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mrc_dom = &mrc_base->mrc_dom[0];

	printf("mrc_dom %u glbac %u: 0x%x\n",
	       mrc_x, glbac_id, readl(&mrc_dom->memn_glbac[glbac_id]));

	return 0;
}

void trdc_dump(void)
{
	u32 i;

	printf("TRDC AONMIX MBC\n");

	trdc_mbc_control_dump(0x44270000, 0, 0);
	trdc_mbc_control_dump(0x44270000, 1, 0);

	for (i = 0; i < 11; i++)
		trdc_mbc_mem_dump(0x44270000, 0, 3, 0, i);
	for (i = 0; i < 1; i++)
		trdc_mbc_mem_dump(0x44270000, 0, 3, 1, i);

	for (i = 0; i < 4; i++)
		trdc_mbc_mem_dump(0x44270000, 1, 3, 0, i);
	for (i = 0; i < 4; i++)
		trdc_mbc_mem_dump(0x44270000, 1, 3, 1, i);

	printf("TRDC WAKEUP MBC\n");

	trdc_mbc_control_dump(0x42460000, 0, 0);
	trdc_mbc_control_dump(0x42460000, 1, 0);

	for (i = 0; i < 15; i++)
		trdc_mbc_mem_dump(0x42460000, 0, 3, 0, i);

	trdc_mbc_mem_dump(0x42460000, 0, 3, 1, 0);
	trdc_mbc_mem_dump(0x42460000, 0, 3, 2, 0);

	for (i = 0; i < 2; i++)
		trdc_mbc_mem_dump(0x42460000, 1, 3, 0, i);

	trdc_mbc_mem_dump(0x42460000, 1, 3, 1, 0);
	trdc_mbc_mem_dump(0x42460000, 1, 3, 2, 0);
	trdc_mbc_mem_dump(0x42460000, 1, 3, 3, 0);

	printf("TRDC NICMIX MBC\n");

	trdc_mbc_control_dump(0x49010000, 0, 0);
	trdc_mbc_control_dump(0x49010000, 1, 0);
	trdc_mbc_control_dump(0x49010000, 2, 0);
	trdc_mbc_control_dump(0x49010000, 3, 0);

	for (i = 0; i < 7; i++)
		trdc_mbc_mem_dump(0x49010000, 0, 3, 0, i);

	for (i = 0; i < 2; i++)
		trdc_mbc_mem_dump(0x49010000, 0, 3, 1, i);

	for (i = 0; i < 5; i++)
		trdc_mbc_mem_dump(0x49010000, 0, 3, 2, i);

	for (i = 0; i < 6; i++)
		trdc_mbc_mem_dump(0x49010000, 0, 3, 3, i);

	for (i = 0; i < 1; i++)
		trdc_mbc_mem_dump(0x49010000, 1, 3, 0, i);

	for (i = 0; i < 1; i++)
		trdc_mbc_mem_dump(0x49010000, 1, 3, 1, i);

	for (i = 0; i < 3; i++)
		trdc_mbc_mem_dump(0x49010000, 1, 3, 2, i);

	for (i = 0; i < 3; i++)
		trdc_mbc_mem_dump(0x49010000, 1, 3, 3, i);

	for (i = 0; i < 2; i++)
		trdc_mbc_mem_dump(0x49010000, 2, 3, 0, i);

	for (i = 0; i < 2; i++)
		trdc_mbc_mem_dump(0x49010000, 2, 3, 1, i);

	for (i = 0; i < 5; i++)
		trdc_mbc_mem_dump(0x49010000, 3, 3, 0, i);

	for (i = 0; i < 5; i++)
		trdc_mbc_mem_dump(0x49010000, 3, 3, 1, i);
}
#endif
