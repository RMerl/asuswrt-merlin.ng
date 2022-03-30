/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION. All rights reserved.
 */

#include <common.h>

#include <linux/arm-smccc.h>

#include <asm/io.h>
#include <asm/arch-tegra/pmc.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_TEGRA_PMC_SECURE)
static bool tegra_pmc_detect_tz_only(void)
{
	static bool initialized = false;
	static bool is_tz_only = false;
	u32 value, saved;

	if (!initialized) {
		saved = readl(NV_PA_PMC_BASE + PMC_SCRATCH0);
		value = saved ^ 0xffffffff;

		if (value == 0xffffffff)
			value = 0xdeadbeef;

		/* write pattern and read it back */
		writel(value, NV_PA_PMC_BASE + PMC_SCRATCH0);
		value = readl(NV_PA_PMC_BASE + PMC_SCRATCH0);

		/* if we read all-zeroes, access is restricted to TZ only */
		if (value == 0) {
			debug("access to PMC is restricted to TZ\n");
			is_tz_only = true;
		} else {
			/* restore original value */
			writel(saved, NV_PA_PMC_BASE + PMC_SCRATCH0);
		}

		initialized = true;
	}

	return is_tz_only;
}
#endif

uint32_t tegra_pmc_readl(unsigned long offset)
{
#if IS_ENABLED(CONFIG_TEGRA_PMC_SECURE)
	if (tegra_pmc_detect_tz_only()) {
		struct arm_smccc_res res;

		arm_smccc_smc(TEGRA_SMC_PMC, TEGRA_SMC_PMC_READ, offset, 0, 0,
			      0, 0, 0, &res);
		if (res.a0)
			printf("%s(): SMC failed: %lu\n", __func__, res.a0);

		return res.a1;
	}
#endif

	return readl(NV_PA_PMC_BASE + offset);
}

void tegra_pmc_writel(u32 value, unsigned long offset)
{
#if IS_ENABLED(CONFIG_TEGRA_PMC_SECURE)
	if (tegra_pmc_detect_tz_only()) {
		struct arm_smccc_res res;

		arm_smccc_smc(TEGRA_SMC_PMC, TEGRA_SMC_PMC_WRITE, offset,
			      value, 0, 0, 0, 0, &res);
		if (res.a0)
			printf("%s(): SMC failed: %lu\n", __func__, res.a0);

		return;
	}
#endif

	writel(value, NV_PA_PMC_BASE + offset);
}

void reset_cpu(ulong addr)
{
	u32 value;

	value = tegra_pmc_readl(PMC_CNTRL);
	value |= PMC_CNTRL_MAIN_RST;
	tegra_pmc_writel(value, PMC_CNTRL);
}
