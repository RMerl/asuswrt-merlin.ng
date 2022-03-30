// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 *
 * Based on code from coreboot
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <pci.h>
#include <asm/cpu.h>
#include <asm/cpu_x86.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/msr.h>
#include <asm/turbo.h>

#define BYT_PRV_CLK			0x800
#define BYT_PRV_CLK_EN			(1 << 0)
#define BYT_PRV_CLK_M_VAL_SHIFT		1
#define BYT_PRV_CLK_N_VAL_SHIFT		16
#define BYT_PRV_CLK_UPDATE		(1 << 31)

static void hsuart_clock_set(void *base)
{
	u32 m, n, reg;

	/*
	 * Configure the BayTrail UART clock for the internal HS UARTs
	 * (PCI devices) to 58982400 Hz
	 */
	m = 0x2400;
	n = 0x3d09;
	reg = (m << BYT_PRV_CLK_M_VAL_SHIFT) | (n << BYT_PRV_CLK_N_VAL_SHIFT);
	writel(reg, base + BYT_PRV_CLK);
	reg |= BYT_PRV_CLK_EN | BYT_PRV_CLK_UPDATE;
	writel(reg, base + BYT_PRV_CLK);
}

/*
 * Configure the internal clock of both SIO HS-UARTs, if they are enabled
 * via FSP
 */
int arch_cpu_init_dm(void)
{
	struct udevice *dev;
	void *base;
	int ret;
	int i;

	/* Loop over the 2 HS-UARTs */
	for (i = 0; i < 2; i++) {
		ret = dm_pci_bus_find_bdf(PCI_BDF(0, 0x1e, 3 + i), &dev);
		if (!ret) {
			base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0,
					      PCI_REGION_MEM);
			hsuart_clock_set(base);
		}
	}

	return 0;
}

static void set_max_freq(void)
{
	msr_t perf_ctl;
	msr_t msr;

	/* Enable speed step */
	msr = msr_read(MSR_IA32_MISC_ENABLES);
	msr.lo |= (1 << 16);
	msr_write(MSR_IA32_MISC_ENABLES, msr);

	/*
	 * Set guaranteed ratio [21:16] from IACORE_RATIOS to bits [15:8] of
	 * the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_RATIOS);
	perf_ctl.lo = (msr.lo & 0x3f0000) >> 8;

	/*
	 * Set guaranteed vid [22:16] from IACORE_VIDS to bits [7:0] of
	 * the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_VIDS);
	perf_ctl.lo |= (msr.lo & 0x7f0000) >> 16;
	perf_ctl.hi = 0;

	msr_write(MSR_IA32_PERF_CTL, perf_ctl);
}

static int cpu_x86_baytrail_probe(struct udevice *dev)
{
	if (!ll_boot_init())
		return 0;
	debug("Init BayTrail core\n");

	/*
	 * On BayTrail the turbo disable bit is actually scoped at the
	 * building-block level, not package. For non-BSP cores that are
	 * within a building block, enable turbo. The cores within the BSP's
	 * building block will just see it already enabled and move on.
	 */
	if (lapicid())
		turbo_enable();

	/* Dynamic L2 shrink enable and threshold */
	msr_clrsetbits_64(MSR_PMG_CST_CONFIG_CONTROL, 0x3f000f, 0xe0008),

	/* Disable C1E */
	msr_clrsetbits_64(MSR_POWER_CTL, 2, 0);
	msr_setbits_64(MSR_POWER_MISC, 0x44);

	/* Set this core to max frequency ratio */
	set_max_freq();

	return 0;
}

static unsigned bus_freq(void)
{
	msr_t clk_info = msr_read(MSR_BSEL_CR_OVERCLOCK_CONTROL);
	switch (clk_info.lo & 0x3) {
	case 0:
		return 83333333;
	case 1:
		return 100000000;
	case 2:
		return 133333333;
	case 3:
		return 116666666;
	default:
		return 0;
	}
}

static unsigned long tsc_freq(void)
{
	msr_t platform_info;
	ulong bclk = bus_freq();

	if (!bclk)
		return 0;

	platform_info = msr_read(MSR_PLATFORM_INFO);

	return bclk * ((platform_info.lo >> 8) & 0xff);
}

static int baytrail_get_info(struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = tsc_freq();
	info->features = 1 << CPU_FEAT_L1_CACHE | 1 << CPU_FEAT_MMU;

	return 0;
}

static int baytrail_get_count(struct udevice *dev)
{
	int ecx = 0;

	/*
	 * Use the algorithm described in Intel 64 and IA-32 Architectures
	 * Software Developer's Manual Volume 3 (3A, 3B & 3C): System
	 * Programming Guide, Jan-2015. Section 8.9.2: Hierarchical Mapping
	 * of CPUID Extended Topology Leaf.
	 */
	while (1) {
		struct cpuid_result leaf_b;

		leaf_b = cpuid_ext(0xb, ecx);

		/*
		 * Bay Trail doesn't have hyperthreading so just determine the
		 * number of cores by from level type (ecx[15:8] == * 2)
		 */
		if ((leaf_b.ecx & 0xff00) == 0x0200)
			return leaf_b.ebx & 0xffff;

		ecx++;
	}

	return 0;
}

static const struct cpu_ops cpu_x86_baytrail_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= baytrail_get_info,
	.get_count	= baytrail_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_baytrail_ids[] = {
	{ .compatible = "intel,baytrail-cpu" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_baytrail_drv) = {
	.name		= "cpu_x86_baytrail",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_baytrail_ids,
	.bind		= cpu_x86_bind,
	.probe		= cpu_x86_baytrail_probe,
	.ops		= &cpu_x86_baytrail_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
