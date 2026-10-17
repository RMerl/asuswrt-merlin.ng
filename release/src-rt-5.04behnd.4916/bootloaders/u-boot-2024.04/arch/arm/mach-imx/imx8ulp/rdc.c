// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/mu_hal.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/arch/rdc.h>
#include <div64.h>

#define XRDC_ADDR	0x292f0000
#define MRC_OFFSET	0x2000
#define MRC_STEP	0x200

#define SP(X)		((X) << 9)
#define SU(X)		((X) << 6)
#define NP(X)		((X) << 3)
#define NU(X)		((X) << 0)

#define RWX		7
#define RW		6
#define R		4
#define X		1

#define D7SEL_CODE	(SP(RW) | SU(RW) | NP(RWX) | NU(RWX))
#define D6SEL_CODE	(SP(RW) | SU(RW) | NP(RWX))
#define D5SEL_CODE	(SP(RW) | SU(RWX))
#define D4SEL_CODE	SP(RWX)
#define D3SEL_CODE	(SP(X) | SU(X) | NP(X) | NU(X))
#define D0SEL_CODE	0

#define D7SEL_DAT	(SP(RW) | SU(RW) | NP(RW) | NU(RW))
#define D6SEL_DAT	(SP(RW) | SU(RW) | NP(RW))
#define D5SEL_DAT	(SP(RW) | SU(RW) | NP(R) | NU(R))
#define D4SEL_DAT	(SP(RW) | SU(RW))
#define D3SEL_DAT	SP(RW)

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
	u32 rgn_desc_words[8][2]; /* 8 regions, 2 words per region */
	u32 reserved[16];
	u32	rgn_nse;
	u32 reserved2[15];
};

struct trdc {
	u8 res0[0x1000];
	struct mbc_mem_dom mem_dom[4][8];
	struct mrc_rgn_dom mrc_dom[2][8];
};

union dxsel_perm {
	struct {
		u8 dx;
		u8 perm;
	};

	u32 dom_perm;
};

int xrdc_config_mrc_dx_perm(u32 mrc_con, u32 region, u32 dom, u32 dxsel)
{
	ulong w2_addr;
	u32 val = 0;

	w2_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0x8;

	val = (readl(w2_addr) & (~(7 << (3 * dom)))) | (dxsel << (3 * dom));
	writel(val, w2_addr);

	return 0;
}

int xrdc_config_mrc_w0_w1(u32 mrc_con, u32 region, u32 w0, u32 size)
{
	ulong w0_addr, w1_addr;

	w0_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20;
	w1_addr = w0_addr + 4;

	if ((size % 32) != 0)
		return -EINVAL;

	writel(w0 & ~0x1f, w0_addr);
	writel(w0 + size - 1, w1_addr);

	return 0;
}

int xrdc_config_mrc_w3_w4(u32 mrc_con, u32 region, u32 w3, u32 w4)
{
	ulong w3_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0xC;
	ulong w4_addr = w3_addr + 4;

	writel(w3, w3_addr);
	writel(w4, w4_addr);

	return 0;
}

int xrdc_config_pdac_openacc(u32 bridge, u32 index)
{
	ulong w0_addr;
	u32 val;

	switch (bridge) {
	case 3:
		w0_addr = XRDC_ADDR + 0x1000 + 0x8 * index;
		break;
	case 4:
		w0_addr = XRDC_ADDR + 0x1400 + 0x8 * index;
		break;
	case 5:
		w0_addr = XRDC_ADDR + 0x1800 + 0x8 * index;
		break;
	default:
		return -EINVAL;
	}
	writel(0xffffff, w0_addr);

	val = readl(w0_addr + 4);
	writel(val | BIT(31), w0_addr + 4);

	return 0;
}

int xrdc_config_pdac(u32 bridge, u32 index, u32 dom, u32 perm)
{
	ulong w0_addr;
	u32 val;

	switch (bridge) {
	case 3:
		w0_addr = XRDC_ADDR + 0x1000 + 0x8 * index;
		break;
	case 4:
		w0_addr = XRDC_ADDR + 0x1400 + 0x8 * index;
		break;
	case 5:
		w0_addr = XRDC_ADDR + 0x1800 + 0x8 * index;
		break;
	default:
		return -EINVAL;
	}
	val = readl(w0_addr);
	writel((val & ~(0x7 << (dom * 3))) | (perm << (dom * 3)), w0_addr);

	val = readl(w0_addr + 4);
	writel(val | BIT(31), w0_addr + 4);

	return 0;
}

int xrdc_config_msc(u32 msc, u32 index, u32 dom, u32 perm)
{
	ulong w0_addr;
	u32 val;

	if (msc > 2)
		return -EINVAL;

	w0_addr = XRDC_ADDR + 0x4000 + 0x400 * msc + 0x8 * index;

	val = readl(w0_addr);
	writel((val & ~(0x7 << (dom * 3))) | (perm << (dom * 3)), w0_addr);

	val = readl(w0_addr + 4);
	writel(val | BIT(31), w0_addr + 4);

	return 0;
}

int release_rdc(enum rdc_type type)
{
	ulong s_mu_base = 0x27020000UL;
	struct ele_msg msg;
	int ret;
	u32 rdc_id = (type == RDC_XRDC) ? 0x78 : 0x74;

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_RELEASE_RDC_REQ;
	msg.data[0] = (rdc_id << 8) | 0x2; /* A35 XRDC */

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

void xrdc_mrc_region_set_access(int mrc_index, u32 addr, u32 access)
{
	ulong xrdc_base = 0x292f0000, off;
	u32 mrgd[5];
	u8 mrcfg, j, region_num;
	u8 dsel;

	mrcfg = readb(xrdc_base + 0x140 + mrc_index);
	region_num = mrcfg & 0x1f;

	for (j = 0; j < region_num; j++) {
		off = 0x2000 + mrc_index * 0x200 + j * 0x20;

		mrgd[0] = readl(xrdc_base + off);
		mrgd[1] = readl(xrdc_base + off + 4);
		mrgd[2] = readl(xrdc_base + off + 8);
		mrgd[3] = readl(xrdc_base + off + 0xc);
		mrgd[4] = readl(xrdc_base + off + 0x10);

		debug("MRC [%u][%u]\n", mrc_index, j);
		debug("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		      mrgd[0], mrgd[1], mrgd[2], mrgd[3], mrgd[4]);

		/* hit */
		if (addr >= mrgd[0] && addr <= mrgd[1]) {
			/* find domain 7 DSEL */
			dsel = (mrgd[2] >> 21) & 0x7;
			if (dsel == 1) {
				mrgd[4] &= ~0xFFF;
				mrgd[4] |= (access & 0xFFF);
			} else if (dsel == 2) {
				mrgd[4] &= ~0xFFF0000;
				mrgd[4] |= ((access & 0xFFF) << 16);
			}

			/* not handle other cases, since ELE only set ACCESS1 and 2 */
			writel(mrgd[4], xrdc_base + off + 0x10);
			return;
		}
	}
}

void xrdc_init_mda(void)
{
	ulong xrdc_base = XRDC_ADDR, off;
	u32 i = 0;

	/* Set MDA3-5 for PXP, ENET, CAAM to DID 1*/
	for (i = 3; i <= 5; i++) {
		off = 0x800 + i * 0x20;
		writel(0x200000A1, xrdc_base + off);
		writel(0xA00000A1, xrdc_base + off);
	}

	/* Set MDA10 -15 to DID 3 for video */
	for (i = 10; i <= 15; i++) {
		off = 0x800 + i * 0x20;
		writel(0x200000A3, xrdc_base + off);
		writel(0xA00000A3, xrdc_base + off);
	}
}

void xrdc_init_mrc(void)
{
	/* Re-config MRC3 for SRAM0 in case protected by ELE */
	xrdc_config_mrc_w0_w1(3, 0, 0x22010000, 0x10000);
	xrdc_config_mrc_dx_perm(3, 0, 0, 1);
	xrdc_config_mrc_dx_perm(3, 0, 1, 1);
	xrdc_config_mrc_dx_perm(3, 0, 4, 1);
	xrdc_config_mrc_dx_perm(3, 0, 5, 1);
	xrdc_config_mrc_dx_perm(3, 0, 6, 1);
	xrdc_config_mrc_dx_perm(3, 0, 7, 1);
	xrdc_config_mrc_w3_w4(3, 0, 0x0, 0x80000FFF);

	/* Clear other 3 regions of MRC3 to invalid */
	xrdc_config_mrc_w3_w4(3, 1, 0x0, 0x0);
	xrdc_config_mrc_w3_w4(3, 2, 0x0, 0x0);
	xrdc_config_mrc_w3_w4(3, 3, 0x0, 0x0);

	/* Set MRC4 and MRC5 for DDR access from A35 and AP NIC PER masters */
	xrdc_config_mrc_w0_w1(4, 0, CFG_SYS_SDRAM_BASE, PHYS_SDRAM_SIZE);
	xrdc_config_mrc_dx_perm(4, 0, 1, 1);
	xrdc_config_mrc_dx_perm(4, 0, 7, 1);
	xrdc_config_mrc_w3_w4(4, 0, 0x0, 0x80000FFF);

	xrdc_config_mrc_w0_w1(5, 0, CFG_SYS_SDRAM_BASE, PHYS_SDRAM_SIZE);
	xrdc_config_mrc_dx_perm(5, 0, 1, 1);
	xrdc_config_mrc_w3_w4(5, 0, 0x0, 0x80000FFF);

	/* Set MRC6 for DDR access from ELE */
	xrdc_config_mrc_w0_w1(6, 0, CFG_SYS_SDRAM_BASE, PHYS_SDRAM_SIZE);
	xrdc_config_mrc_dx_perm(6, 0, 4, 1);
	xrdc_config_mrc_w3_w4(6, 0, 0x0, 0x80000FFF);

	/* The MRC8 is for SRAM1 */
	xrdc_config_mrc_w0_w1(8, 0, 0x21000000, 0x10000);
	/* Allow for all domains: So domain 2/3 (HIFI DSP/LPAV) is ok to access */
	xrdc_config_mrc_dx_perm(8, 0, 0, 1);
	xrdc_config_mrc_dx_perm(8, 0, 1, 1);
	xrdc_config_mrc_dx_perm(8, 0, 2, 1);
	xrdc_config_mrc_dx_perm(8, 0, 3, 1);
	xrdc_config_mrc_dx_perm(8, 0, 4, 1);
	xrdc_config_mrc_dx_perm(8, 0, 5, 1);
	xrdc_config_mrc_dx_perm(8, 0, 6, 1);
	xrdc_config_mrc_dx_perm(8, 0, 7, 1);
	xrdc_config_mrc_w3_w4(8, 0, 0x0, 0x80000FFF);

	/* The MRC6 is for video modules to ddr */
	xrdc_config_mrc_w0_w1(6, 0, 0x80000000, 0x80000000);
	xrdc_config_mrc_dx_perm(6, 0, 3, 1); /* allow for domain 3 video */
	xrdc_config_mrc_w3_w4(6, 0, 0x0, 0x80000FFF);
}

void xrdc_init_pdac_msc(void)
{
	/* Init LPAV PDAC and MSC for DDR init */
	xrdc_config_pdac(5, 36, 6, 0x7); /* CMC2*/
	xrdc_config_pdac(5, 36, 7, 0x7);
	xrdc_config_pdac(5, 37, 6, 0x7); /* SIM2 */
	xrdc_config_pdac(5, 37, 7, 0x7);
	xrdc_config_pdac(5, 38, 6, 0x7); /* CGC2 */
	xrdc_config_pdac(5, 38, 7, 0x7);
	xrdc_config_pdac(5, 39, 6, 0x7); /* PCC5 */
	xrdc_config_pdac(5, 39, 7, 0x7);

	xrdc_config_msc(0, 0, 6, 0x7); /* GPIOE */
	xrdc_config_msc(0, 0, 7, 0x7);
	xrdc_config_msc(0, 1, 6, 0x7); /* GPIOF */
	xrdc_config_msc(0, 1, 7, 0x7);
	xrdc_config_msc(1, 0, 6, 0x7); /* GPIOD */
	xrdc_config_msc(1, 0, 7, 0x7);
	xrdc_config_msc(2, 6, 6, 0x7); /* DDR controller */
	xrdc_config_msc(2, 6, 7, 0x7);
}

int trdc_mbc_set_access(u32 mbc_x, u32 dom_x, u32 mem_x, u32 blk_x, bool sec_access)
{
	struct trdc *trdc_base = (struct trdc *)0x28031000U;
	struct mbc_mem_dom *mbc_dom;
	u32 *cfg_w, *nse_w;
	u32 index, offset, val;

	mbc_dom = &trdc_base->mem_dom[mbc_x][dom_x];

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
		val |= (0x0 << offset);
		writel(val, (void __iomem *)cfg_w);
	} else {
		val |= (0x8 << offset); /* nse bit set */
		writel(val, (void __iomem *)cfg_w);
	}

	return 0;
}

int trdc_mrc_region_set_access(u32 mrc_x, u32 dom_x, u32 addr_start, u32 addr_end, bool sec_access)
{
	struct trdc *trdc_base = (struct trdc *)0x28031000U;
	struct mrc_rgn_dom *mrc_dom;
	u32 *desc_w;
	u32 start, end;
	u32 i, free = 8;
	bool vld, hit = false;

	mrc_dom = &trdc_base->mrc_dom[mrc_x][dom_x];

	for (i = 0; i < 8; i++) {
		desc_w = &mrc_dom->rgn_desc_words[i][0];

		start = readl((void __iomem *)desc_w) & 0xfff;
		end = readl((void __iomem *)(desc_w + 1));
		vld = end & 0x1;
		end = end & 0xfff;

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
			writel(start, (void __iomem *)desc_w);
			writel(end | 0x1, (void __iomem *)(desc_w + 1));
		} else {
			writel(start, (void __iomem *)desc_w);
			writel((end | 0x1 | 0x10), (void __iomem *)(desc_w + 1));
		}

		if (addr_start >= start && addr_end <= end)
			hit = true;
	}

	if (!hit) {
		if (free >= 8)
			return -EFAULT;

		desc_w = &mrc_dom->rgn_desc_words[free][0];

		addr_start &= ~0xfff;
		addr_end &= ~0xfff;

		if (sec_access) {
			writel(addr_start, (void __iomem *)desc_w);
			writel(addr_end | 0x1, (void __iomem *)(desc_w + 1));
		} else {
			writel(addr_start, (void __iomem *)desc_w);
			writel((addr_end | 0x1 | 0x10), (void __iomem *)(desc_w + 1));
		}
	}

	return 0;
}
