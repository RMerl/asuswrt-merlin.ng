// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */
#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/pmu.h>
#include <linux/errno.h>
#include <linux/io.h>

/* Registers */
struct pmu_regs {
	u32	sts;
	u32	cmd;
	u32	ics;
	u32	reserved;
	u32	wkc[4];
	u32	wks[4];
	u32	ssc[4];
	u32	sss[4];
};

/* Bits in PMU_REGS_STS */
#define PMU_REGS_STS_BUSY		(1 << 8)

struct pmu_mid {
	struct pmu_regs *regs;
};

static int pmu_read_status(struct pmu_regs *regs)
{
	int retry = 500000;
	u32 val;

	do {
		val = readl(&regs->sts);
		if (!(val & PMU_REGS_STS_BUSY))
			return 0;

		udelay(1);
	} while (--retry);

	printf("WARNING: PMU still busy\n");
	return -EBUSY;
}

static int pmu_power_lss(struct pmu_regs *regs, unsigned int lss, bool on)
{
	unsigned int offset = (lss * 2) / 32;
	unsigned int shift = (lss * 2) % 32;
	u32 ssc;
	int ret;

	/* Check PMU status */
	ret = pmu_read_status(regs);
	if (ret)
		return ret;

	/* Read PMU values */
	ssc = readl(&regs->sss[offset]);

	/* Modify PMU values */
	if (on)
		ssc &= ~(0x3 << shift);		/* D0 */
	else
		ssc |= 0x3 << shift;		/* D3hot */

	/* Write modified PMU values */
	writel(ssc, &regs->ssc[offset]);

	/* Update modified PMU values */
	writel(0x00002201, &regs->cmd);

	/* Check PMU status */
	return pmu_read_status(regs);
}

int pmu_turn_power(unsigned int lss, bool on)
{
	struct pmu_mid *pmu;
	struct udevice *dev;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_PMU, &dev);
	if (ret)
		return ret;

	pmu = dev_get_priv(dev);

	return pmu_power_lss(pmu->regs, lss, on);
}

static int pmu_mid_probe(struct udevice *dev)
{
	struct pmu_mid *pmu = dev_get_priv(dev);

	pmu->regs = syscon_get_first_range(X86_SYSCON_PMU);

	return 0;
}

static const struct udevice_id pmu_mid_match[] = {
	{ .compatible = "intel,pmu-mid", .data = X86_SYSCON_PMU },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(intel_mid_pmu) = {
	.name		= "pmu_mid",
	.id		= UCLASS_SYSCON,
	.of_match	= pmu_mid_match,
	.probe		= pmu_mid_probe,
	.priv_auto_alloc_size = sizeof(struct pmu_mid),
};
