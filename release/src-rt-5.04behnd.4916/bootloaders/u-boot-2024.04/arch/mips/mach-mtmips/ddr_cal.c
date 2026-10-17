// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/addrspace.h>
#include <asm/cacheops.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <mach/mc.h>

DECLARE_GLOBAL_DATA_PTR;

#define COARSE_MIN_START	6
#define FINE_MIN_START		15
#define COARSE_MAX_START	7
#define FINE_MAX_START		0

#define NUM_OF_CACHELINE	128
#define TEST_PAT_SIZE		(NUM_OF_CACHELINE * CONFIG_SYS_CACHELINE_SIZE)

#define INIT_DQS_VAL		((7 << DQS1_DELAY_COARSE_TUNING_S) | \
				(4 << DQS1_DELAY_FINE_TUNING_S) | \
				(7 << DQS0_DELAY_COARSE_TUNING_S) | \
				(4 << DQS0_DELAY_FINE_TUNING_S))

static inline void pref_op(int op, const volatile void *addr)
{
	__asm__ __volatile__("pref %0, 0(%1)" : : "i" (op), "r" (addr));
}

static inline bool dqs_test_error(void __iomem *memc, u32 memsize, u32 dqsval,
				  u32 bias)
{
	u32 *nca, *ca;
	u32 off;
	int i;

	for (off = 0; off < memsize - TEST_PAT_SIZE; off += (memsize >> 6)) {
		nca = (u32 *)KSEG1ADDR(off);
		ca = (u32 *)KSEG0ADDR(off);

		writel(INIT_DQS_VAL, memc + MEMCTL_DDR_DQS_DLY_REG);
		wmb();

		for (i = 0; i < TEST_PAT_SIZE / sizeof(u32); i++)
			ca[i] = 0x1f1f1f1f;

		for (i = 0; i < TEST_PAT_SIZE / sizeof(u32); i++)
			nca[i] = (u32)nca + i + bias;

		writel(dqsval, memc + MEMCTL_DDR_DQS_DLY_REG);
		wmb();

		for (i = 0; i < TEST_PAT_SIZE; i += CONFIG_SYS_CACHELINE_SIZE)
			mips_cache(HIT_INVALIDATE_D, (u8 *)ca + i);
		wmb();

		for (i = 0; i < TEST_PAT_SIZE; i += CONFIG_SYS_CACHELINE_SIZE)
			pref_op(0, (u8 *)ca + i);

		for (i = 0; i < TEST_PAT_SIZE / sizeof(u32); i++) {
			if (ca[i] != (u32)nca + i + bias)
				return true;
		}
	}

	return false;
}

static inline int dqs_find_max(void __iomem *memc, u32 memsize, int initval,
			       int maxval, int shift, u32 regval)
{
	int fieldval;
	u32 dqsval;

	for (fieldval = initval; fieldval <= maxval; fieldval++) {
		dqsval = regval | (fieldval << shift);
		if (dqs_test_error(memc, memsize, dqsval, 3))
			return max(fieldval - 1, initval);
	}

	return maxval;
}

static inline int dqs_find_min(void __iomem *memc, u32 memsize, int initval,
			       int minval, int shift, u32 regval)
{
	int fieldval;
	u32 dqsval;

	for (fieldval = initval; fieldval >= minval; fieldval--) {
		dqsval = regval | (fieldval << shift);
		if (dqs_test_error(memc, memsize, dqsval, 1))
			return min(fieldval + 1, initval);
	}

	return minval;
}

void ddr_calibrate(void __iomem *memc, u32 memsize, u32 bw)
{
	u32 dqs_coarse_min, dqs_coarse_max, dqs_coarse_val;
	u32 dqs_fine_min, dqs_fine_max, dqs_fine_val;
	u32 dqs_coarse_min_limit, dqs_fine_min_limit;
	u32 dlls, dqs_dll, ddr_cfg2_reg;
	u32 dqs_dly_tmp, dqs_dly, test_dqs, shift;
	u32 rem, mask;
	int i;

	/* Disable Self-refresh */
	clrbits_32(memc + MEMCTL_DDR_SELF_REFRESH_REG, SR_AUTO_EN);

	/* Save DDR_CFG2 and modify its DQS gating window */
	ddr_cfg2_reg = readl(memc + MEMCTL_DDR_CFG2_REG);
	mask = DQS0_GATING_WINDOW_M;
	if (bw == IND_SDRAM_WIDTH_16BIT)
		mask |= DQS1_GATING_WINDOW_M;
	clrbits_32(memc + MEMCTL_DDR_CFG2_REG, mask);

	/* Get minimum available DQS value */
	dlls = readl(memc + MEMCTL_DLL_DBG_REG);
	dlls = (dlls & MST_DLY_SEL_M) >> MST_DLY_SEL_S;

	dqs_dll = dlls >> 4;
	if (dqs_dll <= 8)
		dqs_coarse_min_limit = 8 - dqs_dll;
	else
		dqs_coarse_min_limit = 0;

	dqs_dll = dlls & 0xf;
	if (dqs_dll <= 8)
		dqs_fine_min_limit = 8 - dqs_dll;
	else
		dqs_fine_min_limit = 0;

	/* Initial DQS register value */
	dqs_dly = INIT_DQS_VAL;

	/* Calibrate DQS0 and/or DQS1 */
	for (i = 0; i < bw; i++) {
		shift = i * 8;
		dqs_dly &= ~(0xff << shift);

		/* Find maximum DQS coarse-grain */
		dqs_dly_tmp = dqs_dly | (0xf << shift);
		dqs_coarse_max = dqs_find_max(memc, memsize, COARSE_MAX_START,
					      0xf, 4 + shift, dqs_dly_tmp);

		/* Find maximum DQS fine-grain */
		dqs_dly_tmp = dqs_dly | (dqs_coarse_max << (4 + shift));
		test_dqs = dqs_find_max(memc, memsize, FINE_MAX_START, 0xf,
					shift, dqs_dly_tmp);

		if (test_dqs == FINE_MAX_START) {
			dqs_coarse_max--;
			dqs_fine_max = 0xf;
		} else {
			dqs_fine_max = test_dqs - 1;
		}

		/* Find minimum DQS coarse-grain */
		dqs_dly_tmp = dqs_dly;
		dqs_coarse_min = dqs_find_min(memc, memsize, COARSE_MIN_START,
					      dqs_coarse_min_limit, 4 + shift,
					      dqs_dly_tmp);

		/* Find minimum DQS fine-grain */
		dqs_dly_tmp = dqs_dly | (dqs_coarse_min << (4 + shift));
		test_dqs = dqs_find_min(memc, memsize, FINE_MIN_START,
					dqs_fine_min_limit, shift, dqs_dly_tmp);

		if (test_dqs == FINE_MIN_START + 1) {
			dqs_coarse_min++;
			dqs_fine_min = 0;
		} else {
			dqs_fine_min = test_dqs;
		}

		/* Calculate central DQS coarse/fine value */
		dqs_coarse_val = (dqs_coarse_max + dqs_coarse_min) >> 1;
		rem = (dqs_coarse_max + dqs_coarse_min) % 2;

		dqs_fine_val = (rem * 4) + ((dqs_fine_max + dqs_fine_min) >> 1);
		if (dqs_fine_val >= 0x10) {
			dqs_coarse_val++;
			dqs_fine_val -= 8;
		}

		/* Save current DQS value */
		dqs_dly |= ((dqs_coarse_val << 4) | dqs_fine_val) << shift;
	}

	/* Set final DQS value */
	writel(dqs_dly, memc + MEMCTL_DDR_DQS_DLY_REG);

	/* Restore DDR_CFG2 */
	writel(ddr_cfg2_reg, memc + MEMCTL_DDR_CFG2_REG);

	/* Enable Self-refresh */
	setbits_32(memc + MEMCTL_DDR_SELF_REFRESH_REG, SR_AUTO_EN);
}
