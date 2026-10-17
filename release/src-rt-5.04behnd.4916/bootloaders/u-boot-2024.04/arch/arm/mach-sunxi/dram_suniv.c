// SPDX-License-Identifier: (GPL-2.0+)
/*
 * suniv DRAM initialization
 *
 * Copyright (C) 2018 Icenowy Zheng <icenowy@aosc.io>
 *
 * Based on xboot's arch/arm32/mach-f1c100s/sys-dram.c, which is:
 *
 * Copyright(c) 2007-2018 Jianjun Jiang <8192542@qq.com>
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <hang.h>
#include <sunxi_gpio.h>

#define SDR_T_CAS			(0x2)
#define SDR_T_RAS			(0x8)
#define SDR_T_RCD			(0x3)
#define SDR_T_RP			(0x3)
#define SDR_T_WR			(0x3)
#define SDR_T_RFC			(0xd)
#define SDR_T_XSR			(0xf9)
#define SDR_T_RC			(0xb)
#define SDR_T_INIT			(0x8)
#define SDR_T_INIT_REF			(0x7)
#define SDR_T_WTR			(0x2)
#define SDR_T_RRD			(0x2)
#define SDR_T_XP			(0x0)

enum dram_type {
	DRAM_TYPE_SDR	= 0,
	DRAM_TYPE_DDR	= 1,
	/* Not supported yet. */
	DRAM_TYPE_MDDR	= 2,
};

struct dram_para {
	u32 size;		/* dram size (unit: MByte) */
	u32 clk;		/* dram work clock (unit: MHz) */
	u32 access_mode;	/* 0: interleave mode 1: sequence mode */
	u32 cs_num;		/* dram chip count  1: one chip  2: two chip */
	u32 ddr8_remap;		/* for 8bits data width DDR 0: normal  1: 8bits */
	enum dram_type sdr_ddr;
	u32 bwidth;		/* dram bus width */
	u32 col_width;		/* column address width */
	u32 row_width;		/* row address width */
	u32 bank_size;		/* dram bank count */
	u32 cas;		/* dram cas */
};

struct dram_para suniv_dram_para = {
	.size = 32,
	.clk = 156,
	.access_mode = 1,
	.cs_num = 1,
	.ddr8_remap = 0,
	.sdr_ddr = DRAM_TYPE_DDR,
	.bwidth = 16,
	.col_width = 10,
	.row_width = 13,
	.bank_size = 4,
	.cas = 0x3,
};

static int dram_initial(void)
{
	unsigned int time = 0xffffff;

	setbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR, 0x1);
	while ((readl(SUNXI_DRAMC_BASE + DRAM_SCTLR) & 0x1) && time--) {
		if (time == 0)
			return 0;
	}
	return 1;
}

static int dram_delay_scan(void)
{
	unsigned int time = 0xffffff;

	setbits_le32(SUNXI_DRAMC_BASE + DRAM_DDLYR, 0x1);
	while ((readl(SUNXI_DRAMC_BASE + DRAM_DDLYR) & 0x1) && time--) {
		if (time == 0)
			return 0;
	}
	return 1;
}

static void dram_set_autofresh_cycle(u32 clk)
{
	u32 val = 0;
	u32 row = 0;
	u32 temp = 0;

	row = readl(SUNXI_DRAMC_BASE + DRAM_SCONR);
	row &= 0x1e0;
	row >>= 0x5;

	if (row == 0xc) {
		if (clk >= 1000000) {
			temp = clk + (clk >> 3) + (clk >> 4) + (clk >> 5);
			while (temp >= (10000000 >> 6)) {
				temp -= (10000000 >> 6);
				val++;
			}
		} else {
			val = (clk * 499) >> 6;
		}
	} else if (row == 0xb) {
		if (clk >= 1000000) {
			temp = clk + (clk >> 3) + (clk >> 4) + (clk >> 5);
			while (temp >= (10000000 >> 7)) {
				temp -= (10000000 >> 7);
				val++;
			}
		} else {
			val = (clk * 499) >> 5;
		}
	}
	writel(val, SUNXI_DRAMC_BASE + DRAM_SREFR);
}

static int dram_para_setup(struct dram_para *para)
{
	u32 val = 0;

	val = (para->ddr8_remap) | (0x1 << 1) |
	      ((para->bank_size >> 2) << 3) |
	      ((para->cs_num >> 1) << 4) |
	      ((para->row_width - 1) << 5) |
	      ((para->col_width - 1) << 9) |
	      ((para->sdr_ddr ? (para->bwidth >> 4) : (para->bwidth >> 5)) << 13) |
	      (para->access_mode << 15) |
	      (para->sdr_ddr << 16);

	writel(val, SUNXI_DRAMC_BASE + DRAM_SCONR);
	setbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR, 0x1 << 19);
	return dram_initial();
}

static u32 dram_check_delay(u32 bwidth)
{
	u32 dsize;
	int i, j;
	u32 num = 0;
	u32 dflag = 0;

	dsize = ((bwidth == 16) ? 4 : 2);
	for (i = 0; i < dsize; i++) {
		if (i == 0)
			dflag = readl(SUNXI_DRAMC_BASE + DRAM_DRPTR0);
		else if (i == 1)
			dflag = readl(SUNXI_DRAMC_BASE + DRAM_DRPTR1);
		else if (i == 2)
			dflag = readl(SUNXI_DRAMC_BASE + DRAM_DRPTR2);
		else if (i == 3)
			dflag = readl(SUNXI_DRAMC_BASE + DRAM_DRPTR3);

		for (j = 0; j < 32; j++) {
			if (dflag & 0x1)
				num++;
			dflag >>= 1;
		}
	}
	return num;
}

static int sdr_readpipe_scan(void)
{
	u32 k = 0;

	for (k = 0; k < 32; k++)
		writel(k, CFG_SYS_SDRAM_BASE + 4 * k);
	for (k = 0; k < 32; k++) {
		if (readl(CFG_SYS_SDRAM_BASE + 4 * k) != k)
			return 0;
	}
	return 1;
}

static u32 sdr_readpipe_select(void)
{
	u32 value = 0;
	u32 i = 0;

	for (i = 0; i < 8; i++) {
		clrsetbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR,
				0x7 << 6, i << 6);
		if (sdr_readpipe_scan()) {
			value = i;
			return value;
		}
	}
	return value;
}

static u32 dram_check_type(struct dram_para *para)
{
	u32 times = 0;
	int i;

	for (i = 0; i < 8; i++) {
		clrsetbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR,
				0x7 << 6, i << 6);
		dram_delay_scan();
		if (readl(SUNXI_DRAMC_BASE + DRAM_DDLYR) & 0x30)
			times++;
	}

	if (times == 8) {
		para->sdr_ddr = DRAM_TYPE_SDR;
		return 0;
	}
	para->sdr_ddr = DRAM_TYPE_DDR;
	return 1;
}

static u32 dram_scan_readpipe(struct dram_para *para)
{
	u32 rp_best = 0, rp_val = 0;
	u32 readpipe[8];
	int i;

	if (para->sdr_ddr == DRAM_TYPE_DDR) {
		for (i = 0; i < 8; i++) {
			clrsetbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR,
					0x7 << 6, i << 6);
			dram_delay_scan();
			readpipe[i] = 0;
			if ((((readl(SUNXI_DRAMC_BASE + DRAM_DDLYR) >> 4) & 0x3) == 0x0) &&
			    (((readl(SUNXI_DRAMC_BASE + DRAM_DDLYR) >> 4) & 0x1) == 0x0))
				readpipe[i] = dram_check_delay(para->bwidth);
			if (rp_val < readpipe[i]) {
				rp_val = readpipe[i];
				rp_best = i;
			}
		}
		clrsetbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR,
				0x7 << 6, rp_best << 6);
		dram_delay_scan();
	} else {
		clrbits_le32(SUNXI_DRAMC_BASE + DRAM_SCONR,
			     (0x1 << 16) | (0x3 << 13));
		rp_best = sdr_readpipe_select();
		clrsetbits_le32(SUNXI_DRAMC_BASE + DRAM_SCTLR,
				0x7 << 6, rp_best << 6);
	}
	return 0;
}

static u32 dram_get_dram_size(struct dram_para *para)
{
	u32 colflag = 10, rowflag = 13;
	u32 val1 = 0;
	u32 count = 0;
	u32 addr1, addr2;
	int i;

	para->col_width = colflag;
	para->row_width = rowflag;
	dram_para_setup(para);
	dram_scan_readpipe(para);
	for (i = 0; i < 32; i++) {
		*((u8 *)(CFG_SYS_SDRAM_BASE + 0x200 + i)) = 0x11;
		*((u8 *)(CFG_SYS_SDRAM_BASE + 0x600 + i)) = 0x22;
	}
	for (i = 0; i < 32; i++) {
		val1 = *((u8 *)(CFG_SYS_SDRAM_BASE + 0x200 + i));
		if (val1 == 0x22)
			count++;
	}
	if (count == 32)
		colflag = 9;
	else
		colflag = 10;
	count = 0;
	para->col_width = colflag;
	para->row_width = rowflag;
	dram_para_setup(para);
	if (colflag == 10) {
		addr1 = CFG_SYS_SDRAM_BASE + 0x400000;
		addr2 = CFG_SYS_SDRAM_BASE + 0xc00000;
	} else {
		addr1 = CFG_SYS_SDRAM_BASE + 0x200000;
		addr2 = CFG_SYS_SDRAM_BASE + 0x600000;
	}
	for (i = 0; i < 32; i++) {
		*((u8 *)(addr1 + i)) = 0x33;
		*((u8 *)(addr2 + i)) = 0x44;
	}
	for (i = 0; i < 32; i++) {
		val1 = *((u8 *)(addr1 + i));
		if (val1 == 0x44)
			count++;
	}
	if (count == 32)
		rowflag = 12;
	else
		rowflag = 13;
	para->col_width = colflag;
	para->row_width = rowflag;
	if (para->row_width != 13)
		para->size = 16;
	else if (para->col_width == 10)
		para->size = 64;
	else
		para->size = 32;
	dram_set_autofresh_cycle(para->clk);
	para->access_mode = 0;
	dram_para_setup(para);

	return 0;
}

static void simple_dram_check(void)
{
	volatile u32 *dram = (u32 *)CFG_SYS_SDRAM_BASE;
	int i;

	for (i = 0; i < 0x40; i++)
		dram[i] = i;

	for (i = 0; i < 0x40; i++) {
		if (dram[i] != i) {
			printf("DRAM initialization failed: dram[0x%x] != 0x%x.", i, dram[i]);
			hang();
		}
	}

	for (i = 0; i < 0x10000; i += 0x40)
		dram[i] = i;

	for (i = 0; i < 0x10000; i += 0x40) {
		if (dram[i] != i) {
			printf("DRAM initialization failed: dram[0x%x] != 0x%x.", i, dram[i]);
			hang();
		}
	}
}

static void do_dram_init(struct dram_para *para)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	u32 val;
	u8 m; /* PLL_DDR clock factor */

	sunxi_gpio_set_cfgpin(SUNXI_GPB(3), 0x7);
	mdelay(5);
	/* TODO: dig out what's them... some analog register? */
	if ((para->cas >> 3) & 0x1)
		setbits_le32(SUNXI_PIO_BASE + 0x2c4, (0x1 << 23) | (0x20 << 17));

	if (para->clk >= 144 && para->clk <= 180)
		writel(0xaaa, SUNXI_PIO_BASE + 0x2c0);
	if (para->clk >= 180)
		writel(0xfff, SUNXI_PIO_BASE + 0x2c0);

	if (para->cas & BIT(4))
		writel(0xd1303333, &ccm->pll5_pattern_cfg);
	else if (para->cas & BIT(5))
		writel(0xcce06666, &ccm->pll5_pattern_cfg);
	else if (para->cas & BIT(6))
		writel(0xc8909999, &ccm->pll5_pattern_cfg);
	else if (para->cas & BIT(7))
		writel(0xc440cccc, &ccm->pll5_pattern_cfg);

	if (para->clk <= 96)
		m = 2;
	else
		m = 1;

	val = CCM_PLL5_CTRL_EN | CCM_PLL5_CTRL_UPD |
	      CCM_PLL5_CTRL_N((para->clk * 2) / (24 / m)) |
	      CCM_PLL5_CTRL_K(1) | CCM_PLL5_CTRL_M(m);
	if (para->cas & GENMASK(7, 4))
		val |= CCM_PLL5_CTRL_SIGMA_DELTA_EN;
	writel(val, &ccm->pll5_cfg);
	setbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_UPD);
	mctl_await_completion(&ccm->pll5_cfg, BIT(28), BIT(28));
	mdelay(5);

	setbits_le32(&ccm->ahb_gate0, (1 << AHB_GATE_OFFSET_MCTL));
	clrbits_le32(&ccm->ahb_reset0_cfg, (1 << AHB_RESET_OFFSET_MCTL));
	udelay(50);
	setbits_le32(&ccm->ahb_reset0_cfg, (1 << AHB_RESET_OFFSET_MCTL));

	clrsetbits_le32(SUNXI_PIO_BASE + 0x2c4, (1 << 16),
			((para->sdr_ddr == DRAM_TYPE_DDR) << 16));

	val = (SDR_T_CAS << 0) | (SDR_T_RAS << 3) | (SDR_T_RCD << 7) |
	      (SDR_T_RP << 10) | (SDR_T_WR << 13) | (SDR_T_RFC << 15) |
	      (SDR_T_XSR << 19) | (SDR_T_RC << 28);
	writel(val, SUNXI_DRAMC_BASE + DRAM_STMG0R);
	val = (SDR_T_INIT << 0) | (SDR_T_INIT_REF << 16) | (SDR_T_WTR << 20) |
	      (SDR_T_RRD << 22) | (SDR_T_XP << 25);
	writel(val, SUNXI_DRAMC_BASE + DRAM_STMG1R);
	dram_para_setup(para);
	dram_check_type(para);

	clrsetbits_le32(SUNXI_PIO_BASE + 0x2c4, (1 << 16),
			((para->sdr_ddr == DRAM_TYPE_DDR) << 16));

	dram_set_autofresh_cycle(para->clk);
	dram_scan_readpipe(para);
	dram_get_dram_size(para);
	simple_dram_check();
}

unsigned long sunxi_dram_init(void)
{
	do_dram_init(&suniv_dram_para);

	return suniv_dram_para.size * 1024 * 1024;
}
