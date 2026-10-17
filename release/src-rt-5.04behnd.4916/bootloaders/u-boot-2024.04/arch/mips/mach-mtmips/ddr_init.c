// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <mach/ddr.h>
#include <mach/mc.h>

#define DDR_BW_TEST_PAT			0xaa5555aa

static const u32 sdr_size_cfg1[] = {
	[DRAM_8MB] = (1 << NUMROWS_S),
	[DRAM_16MB] = (1 << NUMROWS_S) | (1 << NUMCOLS_S),
	[DRAM_32MB] = (2 << NUMROWS_S) | (1 << NUMCOLS_S),
	[DRAM_64MB] = (2 << NUMROWS_S) | (2 << NUMCOLS_S),
};

static const u32 dram_size[] = {
	[DRAM_8MB] = SZ_8M,
	[DRAM_16MB] = SZ_16M,
	[DRAM_32MB] = SZ_32M,
	[DRAM_64MB] = SZ_64M,
	[DRAM_128MB] = SZ_128M,
	[DRAM_256MB] = SZ_256M,
};

static void dram_test_write(u32 addr, u32 val)
{
	volatile ulong *target = (volatile ulong *)(KSEG1 + addr);

	sync();
	*target = val;
	sync();
}

static u32 dram_test_read(u32 addr)
{
	volatile ulong *target = (volatile ulong *)(KSEG1 + addr);
	u32 val;

	sync();
	val = *target;
	sync();

	return val;
}

static int dram_addr_test_bit(u32 bit)
{
	u32 val;

	dram_test_write(0, 0);
	dram_test_write(BIT(bit), DDR_BW_TEST_PAT);
	val = dram_test_read(0);

	if (val == DDR_BW_TEST_PAT)
		return 1;

	return 0;
}

static void mc_ddr_init(void __iomem *memc, const struct mc_ddr_cfg *cfg,
			u32 dq_dly, u32 dqs_dly, mc_reset_t mc_reset, u32 bw)
{
	u32 val;

	mc_reset(1);
	__udelay(200);
	mc_reset(0);

	clrbits_32(memc + MEMCTL_SDRAM_CFG1_REG, RBC_MAPPING);

	writel(cfg->cfg2, memc + MEMCTL_DDR_CFG2_REG);
	writel(cfg->cfg3, memc + MEMCTL_DDR_CFG3_REG);
	writel(cfg->cfg4, memc + MEMCTL_DDR_CFG4_REG);
	writel(dq_dly, memc + MEMCTL_DDR_DQ_DLY_REG);
	writel(dqs_dly, memc + MEMCTL_DDR_DQS_DLY_REG);

	writel(cfg->cfg0, memc + MEMCTL_DDR_CFG0_REG);

	val = cfg->cfg1;
	if (bw) {
		val &= ~IND_SDRAM_WIDTH_M;
		val |= (bw << IND_SDRAM_WIDTH_S) & IND_SDRAM_WIDTH_M;
	}

	writel(val, memc + MEMCTL_DDR_CFG1_REG);

	clrsetbits_32(memc + MEMCTL_PWR_SAVE_CNT_REG, SR_TAR_CNT_M,
		      1 << SR_TAR_CNT_S);

	setbits_32(memc + MEMCTL_DDR_SELF_REFRESH_REG, SR_AUTO_EN);
}

void ddr1_init(struct mc_ddr_init_param *param)
{
	enum mc_dram_size sz;
	u32 bw = 0;

	/* First initialization, determine bus width */
	mc_ddr_init(param->memc, &param->cfgs[DRAM_8MB], param->dq_dly,
		    param->dqs_dly, param->mc_reset, IND_SDRAM_WIDTH_16BIT);

	/* Test bus width */
	dram_test_write(0, DDR_BW_TEST_PAT);
	if (dram_test_read(0) == DDR_BW_TEST_PAT)
		bw = IND_SDRAM_WIDTH_16BIT;
	else
		bw = IND_SDRAM_WIDTH_8BIT;

	/* Second initialization, determine DDR capacity */
	mc_ddr_init(param->memc, &param->cfgs[DRAM_128MB], param->dq_dly,
		    param->dqs_dly, param->mc_reset, bw);

	if (dram_addr_test_bit(9)) {
		sz = DRAM_8MB;
	} else {
		if (dram_addr_test_bit(10)) {
			if (dram_addr_test_bit(23))
				sz = DRAM_16MB;
			else
				sz = DRAM_32MB;
		} else {
			if (dram_addr_test_bit(24))
				sz = DRAM_64MB;
			else
				sz = DRAM_128MB;
		}
	}

	/* Final initialization, with DDR calibration */
	mc_ddr_init(param->memc, &param->cfgs[sz], param->dq_dly,
		    param->dqs_dly, param->mc_reset, bw);

	/* Return actual DDR configuration */
	param->memsize = dram_size[sz];
	param->bus_width = bw;
}

void ddr2_init(struct mc_ddr_init_param *param)
{
	enum mc_dram_size sz;
	u32 bw = 0;

	/* First initialization, determine bus width */
	mc_ddr_init(param->memc, &param->cfgs[DRAM_32MB], param->dq_dly,
		    param->dqs_dly, param->mc_reset, IND_SDRAM_WIDTH_16BIT);

	/* Test bus width */
	dram_test_write(0, DDR_BW_TEST_PAT);
	if (dram_test_read(0) == DDR_BW_TEST_PAT)
		bw = IND_SDRAM_WIDTH_16BIT;
	else
		bw = IND_SDRAM_WIDTH_8BIT;

	/* Second initialization, determine DDR capacity */
	mc_ddr_init(param->memc, &param->cfgs[DRAM_256MB], param->dq_dly,
		    param->dqs_dly, param->mc_reset, bw);

	if (bw == IND_SDRAM_WIDTH_16BIT) {
		if (dram_addr_test_bit(10)) {
			sz = DRAM_32MB;
		} else {
			if (dram_addr_test_bit(24)) {
				if (dram_addr_test_bit(27))
					sz = DRAM_64MB;
				else
					sz = DRAM_128MB;
			} else {
				sz = DRAM_256MB;
			}
		}
	} else {
		if (dram_addr_test_bit(23)) {
			sz = DRAM_32MB;
		} else {
			if (dram_addr_test_bit(24)) {
				if (dram_addr_test_bit(27))
					sz = DRAM_64MB;
				else
					sz = DRAM_128MB;
			} else {
				sz = DRAM_256MB;
			}
		}
	}

	/* Final initialization, with DDR calibration */
	mc_ddr_init(param->memc, &param->cfgs[sz], param->dq_dly,
		    param->dqs_dly, param->mc_reset, bw);

	/* Return actual DDR configuration */
	param->memsize = dram_size[sz];
	param->bus_width = bw;
}

static void mc_sdr_init(void __iomem *memc, mc_reset_t mc_reset, u32 cfg0,
			u32 cfg1)
{
	mc_reset(1);
	__udelay(200);
	mc_reset(0);

	writel(cfg0, memc + MEMCTL_SDRAM_CFG0_REG);
	writel(cfg1, memc + MEMCTL_SDRAM_CFG1_REG);

	while (!(readl(memc + MEMCTL_SDRAM_CFG1_REG) & SDRAM_INIT_DONE))
		;

	clrsetbits_32(memc + MEMCTL_PWR_SAVE_CNT_REG, SR_TAR_CNT_M,
		      1 << SR_TAR_CNT_S);

	setbits_32(memc + MEMCTL_DDR_SELF_REFRESH_REG, SR_AUTO_EN);
}

void sdr_init(struct mc_ddr_init_param *param)
{
	enum mc_dram_size sz;
	u32 cfg1;

	cfg1 = param->sdr_cfg1 | SDRAM_INIT_START;
	cfg1 &= ~(NUMCOLS_M | NUMROWS_M);

	/* First initialization, determine SDR capacity */
	mc_sdr_init(param->memc, param->mc_reset, param->sdr_cfg0,
		    cfg1 | sdr_size_cfg1[DRAM_64MB]);

	if (dram_addr_test_bit(9)) {
		sz = DRAM_8MB;
	} else {
		if (dram_addr_test_bit(10)) {
			if (dram_addr_test_bit(23))
				sz = DRAM_16MB;
			else
				sz = DRAM_32MB;
		} else {
			sz = DRAM_64MB;
		}
	}

	/* Final initialization */
	mc_sdr_init(param->memc, param->mc_reset, param->sdr_cfg0,
		    cfg1 | sdr_size_cfg1[sz]);

	/* Return actual DDR configuration */
	param->memsize = dram_size[sz];
}
