// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 DDR initialization
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 *
 * Based on spl/common/{jz4780_ddr,jz_ddr3_init}.c from X-Boot
 * Copyright (c) 2006-2013 Ingenic Semiconductor
 */

#include <common.h>
#include <asm/io.h>
#include <mach/jz4780.h>
#include <mach/jz4780_dram.h>

static const u32 get_mem_clk(void)
{
	const u32 mpll_out = ((u64)JZ4780_SYS_EXTAL * JZ4780_MPLL_M) /
			     (JZ4780_MPLL_N * JZ4780_MPLL_OD);
	return mpll_out / JZ4780_SYS_MEM_DIV;
}

u32 sdram_size(int cs)
{
	u32 dw = DDR_DW32 ? 4 : 2;
	u32 banks = DDR_BANK8 ? 8 : 4;
	u32 size = 0;

	if ((cs == 0) && DDR_CS0EN) {
		size = (1 << (DDR_ROW + DDR_COL)) * dw * banks;
		if (DDR_CS1EN && (size > 0x20000000))
			size = 0x20000000;
	} else if ((cs == 1) && DDR_CS1EN) {
		size = (1 << (DDR_ROW + DDR_COL)) * dw * banks;
	}

	return size;
}

static void ddr_cfg_init(void)
{
	void __iomem *ddr_ctl_regs = (void __iomem *)DDRC_BASE;
	u32 ddrc_cfg, tmp;

	tmp = DDR_CL;
	if (tmp)
		tmp--;
	if (tmp > 4)
		tmp = 4;

	ddrc_cfg = DDRC_CFG_TYPE_DDR3 | DDRC_CFG_IMBA |
		   DDR_DW32 | DDRC_CFG_MPRT | ((tmp | 0x8) << 2) |
		   ((DDR_ROW - 12) << 11) | ((DDR_COL - 8) << 8) |
		   (DDR_CS0EN << 6) | (DDR_BANK8 << 1) |
		   ((DDR_ROW - 12) << 27) | ((DDR_COL - 8) << 24) |
		   (DDR_CS1EN << 7) | (DDR_BANK8 << 23);

	if (DDR_BL > 4)
		ddrc_cfg |= BIT(21);

	writel(ddrc_cfg, ddr_ctl_regs + DDRC_CFG);
}

static void ddr_phy_init(const struct jz4780_ddr_config *ddr_config)
{
	void __iomem *ddr_ctl_regs = (void __iomem *)DDRC_BASE;
	void __iomem *ddr_phy_regs = ddr_ctl_regs + DDR_PHY_OFFSET;
	unsigned int count = 0, i;
	u32 reg, mask;

	writel(DDRP_DCR_TYPE_DDR3 | (DDR_BANK8 << 3), ddr_phy_regs + DDRP_DCR);

	writel(ddr_config->mr0, ddr_phy_regs + DDRP_MR0);
	writel(ddr_config->mr1, ddr_phy_regs + DDRP_MR1);
	writel(0, ddr_phy_regs + DDRP_ODTCR);
	writel(0, ddr_phy_regs + DDRP_MR2);

	writel(ddr_config->ptr0, ddr_phy_regs + DDRP_PTR0);
	writel(ddr_config->ptr1, ddr_phy_regs + DDRP_PTR1);
	writel(ddr_config->ptr2, ddr_phy_regs + DDRP_PTR2);

	writel(ddr_config->dtpr0, ddr_phy_regs + DDRP_DTPR0);
	writel(ddr_config->dtpr1, ddr_phy_regs + DDRP_DTPR1);
	writel(ddr_config->dtpr2, ddr_phy_regs + DDRP_DTPR2);

	writel(DDRP_PGCR_DQSCFG | (7 << DDRP_PGCR_CKEN_BIT) |
	       (2 << DDRP_PGCR_CKDV_BIT) |
	       (DDR_CS0EN | (DDR_CS1EN << 1)) << DDRP_PGCR_RANKEN_BIT |
	       DDRP_PGCR_ZCKSEL_32 | DDRP_PGCR_PDDISDX,
	       ddr_phy_regs + DDRP_PGCR);

	for (i = 0; i < 8; i++)
		clrbits_le32(ddr_phy_regs + DDRP_DXGCR(i), 0x3 << 9);

	count = 0;
	mask = DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE;
	for (;;) {
		reg = readl(ddr_phy_regs + DDRP_PGSR);
		if ((reg == mask) || (reg == 0x1f))
			break;
		if (count++ == 10000)
			hang();
	}

	/* DQS extension and early set to 1 */
	clrsetbits_le32(ddr_phy_regs + DDRP_DSGCR, 0x7E << 4, 0x12 << 4);

	/* 500 pull up and 500 pull down */
	clrsetbits_le32(ddr_phy_regs + DDRP_DXCCR, 0xFF << 4, 0xC4 << 4);

	/* Initialise phy */
	writel(DDRP_PIR_INIT | DDRP_PIR_DRAMINT | DDRP_PIR_DRAMRST,
	       ddr_phy_regs + DDRP_PIR);

	count = 0;
	mask |= DDRP_PGSR_DIDONE;
	for (;;) {
		reg = readl(ddr_phy_regs + DDRP_PGSR);
		if ((reg == mask) || (reg == 0x1f))
			break;
		if (count++ == 20000)
			hang();
	}

	writel(DDRP_PIR_INIT | DDRP_PIR_QSTRN, ddr_phy_regs + DDRP_PIR);

	count = 0;
	mask |= DDRP_PGSR_DTDONE;
	for (;;) {
		reg = readl(ddr_phy_regs + DDRP_PGSR);
		if (reg == mask)
			break;
		if (count++ != 50000)
			continue;
		reg &= DDRP_PGSR_DTDONE | DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR;
		if (reg)
			hang();
		count = 0;
	}

	/* Override impedance */
	clrsetbits_le32(ddr_phy_regs + DDRP_ZQXCR0(0), 0x3ff,
		((ddr_config->pullup & 0x1f) << DDRP_ZQXCR_PULLUP_IMPE_BIT) |
		((ddr_config->pulldn & 0x1f) << DDRP_ZQXCR_PULLDOWN_IMPE_BIT) |
		DDRP_ZQXCR_ZDEN);
}

#define JZBIT(bit) ((bit % 4) * 8)
#define JZMASK(bit) (0x1f << JZBIT(bit))

static void remap_swap(int a, int b)
{
	void __iomem *ddr_ctl_regs = (void __iomem *)DDRC_BASE;
	u32 remmap[2], tmp[2];

	remmap[0] = readl(ddr_ctl_regs + DDRC_REMMAP(a / 4));
	remmap[1] = readl(ddr_ctl_regs + DDRC_REMMAP(b / 4));

	tmp[0] = (remmap[0] & JZMASK(a)) >> JZBIT(a);
	tmp[1] = (remmap[1] & JZMASK(b)) >> JZBIT(b);

	remmap[0] &= ~JZMASK(a);
	remmap[1] &= ~JZMASK(b);

	writel(remmap[0] | (tmp[1] << JZBIT(a)),
	       ddr_ctl_regs + DDRC_REMMAP(a / 4));
	writel(remmap[1] | (tmp[0] << JZBIT(b)),
	       ddr_ctl_regs + DDRC_REMMAP(b / 4));
}

static void mem_remap(void)
{
	u32 start = (DDR_ROW + DDR_COL + (DDR_DW32 ? 4 : 2) / 2) - 12;
	u32 num = DDR_BANK8 ? 3 : 2;

	if (DDR_CS0EN && DDR_CS1EN)
		num++;

	for (; num > 0; num--)
		remap_swap(0 + num - 1, start + num - 1);
}

/* Fetch DRAM config from board file */
__weak const struct jz4780_ddr_config *jz4780_get_ddr_config(void)
{
	return NULL;
}

void sdram_init(void)
{
	const struct jz4780_ddr_config *ddr_config = jz4780_get_ddr_config();
	void __iomem *ddr_ctl_regs = (void __iomem *)DDRC_BASE;
	void __iomem *ddr_phy_regs = ddr_ctl_regs + DDR_PHY_OFFSET;
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	u32 mem_clk, tmp, i;
	u32 mem_base0, mem_base1;
	u32 mem_mask0, mem_mask1;
	u32 mem_size0, mem_size1;

	if (!ddr_config)
		hang();

	/* Reset DLL in DDR PHY */
	writel(0x3, cpm_regs + 0xd0);
	mdelay(400);
	writel(0x1, cpm_regs + 0xd0);
	mdelay(400);

	/* Enter reset */
	writel(0xf << 20, ddr_ctl_regs + DDRC_CTRL);

	mem_clk = get_mem_clk();

	tmp = 1000000000 / mem_clk;
	if (1000000000 % mem_clk)
		tmp++;
	tmp = DDR_tREFI / tmp;
	tmp = tmp / (16 * (1 << DDR_CLK_DIV)) - 1;
	if (tmp > 0xff)
		tmp = 0xff;
	if (tmp < 1)
		tmp = 1;

	writel(0x0, ddr_ctl_regs + DDRC_CTRL);

	writel(0x150000, ddr_phy_regs + DDRP_DTAR);
	ddr_phy_init(ddr_config);

	writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, ddr_ctl_regs + DDRC_CTRL);
	writel(0x0, ddr_ctl_regs + DDRC_CTRL);

	ddr_cfg_init();

	for (i = 0; i < 6; i++)
		writel(ddr_config->timing[i], ddr_ctl_regs + DDRC_TIMING(i));

	mem_size0 = sdram_size(0);
	mem_size1 = sdram_size(1);

	if (!mem_size1 && mem_size0 > 0x20000000) {
		mem_base0 = 0x0;
		mem_mask0 = ~(((mem_size0 * 2) >> 24) - 1) & DDRC_MMAP_MASK_MASK;
	} else {
		mem_base0 = (DDR_MEM_PHY_BASE >> 24) & 0xff;
		mem_mask0 = ~((mem_size0 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
	}

	if (mem_size1) {
		mem_mask1 = ~((mem_size1 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
		mem_base1 = ((DDR_MEM_PHY_BASE + mem_size0) >> 24) & 0xff;
	} else {
		mem_mask1 = 0;
		mem_base1 = 0xff;
	}

	writel(mem_base0 << DDRC_MMAP_BASE_BIT | mem_mask0,
	       ddr_ctl_regs + DDRC_MMAP0);
	writel(mem_base1 << DDRC_MMAP_BASE_BIT | mem_mask1,
	       ddr_ctl_regs + DDRC_MMAP1);
	writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, ddr_ctl_regs + DDRC_CTRL);
	writel((DDR_CLK_DIV << 1) | DDRC_REFCNT_REF_EN |
	       (tmp << DDRC_REFCNT_CON_BIT),
	       ddr_ctl_regs + DDRC_REFCNT);
	writel((1 << 15) | (4 << 12) | (1 << 11) | (1 << 8) | (0 << 6) |
	       (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1),
	       ddr_ctl_regs + DDRC_CTRL);
	mem_remap();
	clrbits_le32(ddr_ctl_regs + DDRC_ST, 0x40);
}
