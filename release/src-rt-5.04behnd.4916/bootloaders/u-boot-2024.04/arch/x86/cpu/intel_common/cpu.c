// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Google Inc.
 * Copyright (c) 2016 Google, Inc
 * Copyright (C) 2015-2018 Intel Corporation.
 * Copyright (C) 2018 Siemens AG
 * Some code taken from coreboot cpulib.c
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <asm/cpu.h>
#include <asm/cpu_common.h>
#include <asm/global_data.h>
#include <asm/intel_regs.h>
#include <asm/lapic.h>
#include <asm/lpc_common.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/microcode.h>

DECLARE_GLOBAL_DATA_PTR;

static int report_bist_failure(void)
{
	if (gd->arch.bist != 0) {
		post_code(POST_BIST_FAILURE);
		printf("BIST failed: %08x\n", gd->arch.bist);
		return -EFAULT;
	}

	return 0;
}

int cpu_common_init(void)
{
	struct udevice *dev, *lpc;
	int ret;

	/* Halt if there was a built in self test failure */
	ret = report_bist_failure();
	if (ret)
		return ret;

	enable_lapic();

	ret = microcode_update_intel();
	if (ret && ret != -EEXIST) {
		debug("%s: Microcode update failure (err=%d)\n", __func__, ret);
		return ret;
	}

	/* Enable upper 128bytes of CMOS */
	writel(1 << 2, RCB_REG(RC));

	/* Early chipset init required before RAM init can work */
	uclass_first_device(UCLASS_NORTHBRIDGE, &dev);

	ret = uclass_first_device_err(UCLASS_LPC, &lpc);
	if (ret)
		return ret;

	/* Cause the SATA device to do its early init */
	uclass_first_device(UCLASS_AHCI, &dev);

	return 0;
}

int cpu_set_flex_ratio_to_tdp_nominal(void)
{
	msr_t flex_ratio, msr;
	u8 nominal_ratio;

	/* Check for Flex Ratio support */
	flex_ratio = msr_read(MSR_FLEX_RATIO);
	if (!(flex_ratio.lo & FLEX_RATIO_EN))
		return -EINVAL;

	/* Check for >0 configurable TDPs */
	msr = msr_read(MSR_PLATFORM_INFO);
	if (((msr.hi >> 1) & 3) == 0)
		return -EINVAL;

	/* Use nominal TDP ratio for flex ratio */
	msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
	nominal_ratio = msr.lo & 0xff;

	/* See if flex ratio is already set to nominal TDP ratio */
	if (((flex_ratio.lo >> 8) & 0xff) == nominal_ratio)
		return 0;

	/* Set flex ratio to nominal TDP ratio */
	flex_ratio.lo &= ~0xff00;
	flex_ratio.lo |= nominal_ratio << 8;
	flex_ratio.lo |= FLEX_RATIO_LOCK;
	msr_write(MSR_FLEX_RATIO, flex_ratio);

	/* Set flex ratio in soft reset data register bits 11:6 */
	clrsetbits_le32(RCB_REG(SOFT_RESET_DATA), 0x3f << 6,
			(nominal_ratio & 0x3f) << 6);

	debug("CPU: Soft reset to set up flex ratio\n");

	/* Set soft reset control to use register value */
	setbits_le32(RCB_REG(SOFT_RESET_CTRL), 1);

	/* Issue warm reset, will be "CPU only" due to soft reset data */
	outb(0x0, IO_PORT_RESET);
	outb(SYS_RST | RST_CPU, IO_PORT_RESET);
	cpu_hlt();

	/* Not reached */
	return -EINVAL;
}

int cpu_intel_get_info(struct cpu_info *info, int bclk)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_PERF_CTL);
	info->cpu_freq = ((msr.lo >> 8) & 0xff) * bclk * 1000000;
	info->features = 1 << CPU_FEAT_L1_CACHE | 1 << CPU_FEAT_MMU |
		1 << CPU_FEAT_UCODE | 1 << CPU_FEAT_DEVICE_ID;
	info->address_width = cpu_phys_address_size();

	return 0;
}

int cpu_configure_thermal_target(struct udevice *dev)
{
	u32 tcc_offset;
	msr_t msr;
	int ret;

	ret = dev_read_u32(dev, "tcc-offset", &tcc_offset);
	if (!ret)
		return -ENOENT;

	/* Set TCC activaiton offset if supported */
	msr = msr_read(MSR_PLATFORM_INFO);
	if (msr.lo & (1 << 30)) {
		msr = msr_read(MSR_TEMPERATURE_TARGET);
		msr.lo &= ~(0xf << 24); /* Bits 27:24 */
		msr.lo |= (tcc_offset & 0xf) << 24;
		msr_write(MSR_TEMPERATURE_TARGET, msr);
	}

	return 0;
}

void cpu_set_perf_control(uint clk_ratio)
{
	msr_t perf_ctl;

	perf_ctl.lo = (clk_ratio & 0xff) << 8;
	perf_ctl.hi = 0;
	msr_write(MSR_IA32_PERF_CTL, perf_ctl);
	debug("CPU: frequency set to %d MHz\n", clk_ratio * INTEL_BCLK_MHZ);
}

bool cpu_config_tdp_levels(void)
{
	msr_t platform_info;

	/* Bits 34:33 indicate how many levels supported */
	platform_info = msr_read(MSR_PLATFORM_INFO);

	return ((platform_info.hi >> 1) & 3) != 0;
}

void cpu_set_p_state_to_turbo_ratio(void)
{
	msr_t msr;

	msr = msr_read(MSR_TURBO_RATIO_LIMIT);
	cpu_set_perf_control(msr.lo);
}

enum burst_mode_t cpu_get_burst_mode_state(void)
{
	enum burst_mode_t state;
	int burst_en, burst_cap;
	msr_t msr;
	uint eax;

	eax = cpuid_eax(0x6);
	burst_cap = eax & 0x2;
	msr = msr_read(MSR_IA32_MISC_ENABLE);
	burst_en = !(msr.hi & BURST_MODE_DISABLE);

	if (!burst_cap && burst_en)
		state = BURST_MODE_UNAVAILABLE;
	else if (burst_cap && !burst_en)
		state = BURST_MODE_DISABLED;
	else if (burst_cap && burst_en)
		state = BURST_MODE_ENABLED;
	else
		state = BURST_MODE_UNKNOWN;

	return state;
}

void cpu_set_burst_mode(bool burst_mode)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_MISC_ENABLE);
	if (burst_mode)
		msr.hi &= ~BURST_MODE_DISABLE;
	else
		msr.hi |= BURST_MODE_DISABLE;
	msr_write(MSR_IA32_MISC_ENABLE, msr);
}

void cpu_set_eist(bool eist_status)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_MISC_ENABLE);
	if (eist_status)
		msr.lo |= MISC_ENABLE_ENHANCED_SPEEDSTEP;
	else
		msr.lo &= ~MISC_ENABLE_ENHANCED_SPEEDSTEP;
	msr_write(MSR_IA32_MISC_ENABLE, msr);
}

int cpu_get_coord_type(void)
{
	return HW_ALL;
}

int cpu_get_min_ratio(void)
{
	msr_t msr;

	/* Get bus ratio limits and calculate clock speeds */
	msr = msr_read(MSR_PLATFORM_INFO);

	return (msr.hi >> 8) & 0xff;	/* Max Efficiency Ratio */
}

int cpu_get_max_ratio(void)
{
	u32 ratio_max;
	msr_t msr;

	if (cpu_config_tdp_levels()) {
		/* Set max ratio to nominal TDP ratio */
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		ratio_max = msr.lo & 0xff;
	} else {
		msr = msr_read(MSR_PLATFORM_INFO);
		/* Max Non-Turbo Ratio */
		ratio_max = (msr.lo >> 8) & 0xff;
	}

	return ratio_max;
}

int cpu_get_bus_clock_khz(void)
{
	/*
	 * CPU bus clock is set by default here to 100MHz. This function returns
	 * the bus clock in KHz.
	 */
	return INTEL_BCLK_MHZ * 1000;
}

int cpu_get_power_max(void)
{
	int power_unit;
	msr_t msr;

	msr = msr_read(MSR_PKG_POWER_SKU_UNIT);
	power_unit = 2 << ((msr.lo & 0xf) - 1);
	msr = msr_read(MSR_PKG_POWER_SKU);

	return (msr.lo & 0x7fff) * 1000 / power_unit;
}

int cpu_get_max_turbo_ratio(void)
{
	msr_t msr;

	msr = msr_read(MSR_TURBO_RATIO_LIMIT);

	return msr.lo & 0xff;
}

int cpu_get_cores_per_package(void)
{
	struct cpuid_result result;
	int cores = 1;

	if (gd->arch.x86_vendor != X86_VENDOR_INTEL)
		return 1;

	result = cpuid_ext(0xb, 1);
	cores = result.ebx & 0xff;

	return cores;
}

void cpu_mca_configure(void)
{
	msr_t msr;
	int i;
	int num_banks;

	msr = msr_read(MSR_IA32_MCG_CAP);
	num_banks = msr.lo & 0xff;
	msr.lo = 0;
	msr.hi = 0;
	for (i = 0; i < num_banks; i++) {
		/* Clear the machine check status */
		msr_write(MSR_IA32_MC0_STATUS + (i * 4), msr);
		/* Initialise machine checks */
		msr_write(MSR_IA32_MC0_CTL + i * 4,
			  (msr_t) {.lo = 0xffffffff, .hi = 0xffffffff});
	}
}
