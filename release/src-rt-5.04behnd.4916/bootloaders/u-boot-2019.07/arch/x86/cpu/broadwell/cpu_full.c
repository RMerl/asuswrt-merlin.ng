// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Based on code from coreboot src/soc/intel/broadwell/cpu.c
 */

#include <common.h>
#include <dm.h>
#include <cpu.h>
#include <asm/cpu.h>
#include <asm/cpu_x86.h>
#include <asm/cpu_common.h>
#include <asm/intel_regs.h>
#include <asm/msr.h>
#include <asm/post.h>
#include <asm/turbo.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pch.h>
#include <asm/arch/rcb.h>

struct cpu_broadwell_priv {
	bool ht_disabled;
};

/* Convert time in seconds to POWER_LIMIT_1_TIME MSR value */
static const u8 power_limit_time_sec_to_msr[] = {
	[0]   = 0x00,
	[1]   = 0x0a,
	[2]   = 0x0b,
	[3]   = 0x4b,
	[4]   = 0x0c,
	[5]   = 0x2c,
	[6]   = 0x4c,
	[7]   = 0x6c,
	[8]   = 0x0d,
	[10]  = 0x2d,
	[12]  = 0x4d,
	[14]  = 0x6d,
	[16]  = 0x0e,
	[20]  = 0x2e,
	[24]  = 0x4e,
	[28]  = 0x6e,
	[32]  = 0x0f,
	[40]  = 0x2f,
	[48]  = 0x4f,
	[56]  = 0x6f,
	[64]  = 0x10,
	[80]  = 0x30,
	[96]  = 0x50,
	[112] = 0x70,
	[128] = 0x11,
};

/* Convert POWER_LIMIT_1_TIME MSR value to seconds */
static const u8 power_limit_time_msr_to_sec[] = {
	[0x00] = 0,
	[0x0a] = 1,
	[0x0b] = 2,
	[0x4b] = 3,
	[0x0c] = 4,
	[0x2c] = 5,
	[0x4c] = 6,
	[0x6c] = 7,
	[0x0d] = 8,
	[0x2d] = 10,
	[0x4d] = 12,
	[0x6d] = 14,
	[0x0e] = 16,
	[0x2e] = 20,
	[0x4e] = 24,
	[0x6e] = 28,
	[0x0f] = 32,
	[0x2f] = 40,
	[0x4f] = 48,
	[0x6f] = 56,
	[0x10] = 64,
	[0x30] = 80,
	[0x50] = 96,
	[0x70] = 112,
	[0x11] = 128,
};

/*
 * The core 100MHz BLCK is disabled in deeper c-states. One needs to calibrate
 * the 100MHz BCLCK against the 24MHz BLCK to restore the clocks properly
 * when a core is woken up
 */
static int pcode_ready(void)
{
	int wait_count;
	const int delay_step = 10;

	wait_count = 0;
	do {
		if (!(readl(MCHBAR_REG(BIOS_MAILBOX_INTERFACE)) &
				MAILBOX_RUN_BUSY))
			return 0;
		wait_count += delay_step;
		udelay(delay_step);
	} while (wait_count < 1000);

	return -ETIMEDOUT;
}

static u32 pcode_mailbox_read(u32 command)
{
	int ret;

	ret = pcode_ready();
	if (ret) {
		debug("PCODE: mailbox timeout on wait ready\n");
		return ret;
	}

	/* Send command and start transaction */
	writel(command | MAILBOX_RUN_BUSY, MCHBAR_REG(BIOS_MAILBOX_INTERFACE));

	ret = pcode_ready();
	if (ret) {
		debug("PCODE: mailbox timeout on completion\n");
		return ret;
	}

	/* Read mailbox */
	return readl(MCHBAR_REG(BIOS_MAILBOX_DATA));
}

static int pcode_mailbox_write(u32 command, u32 data)
{
	int ret;

	ret = pcode_ready();
	if (ret) {
		debug("PCODE: mailbox timeout on wait ready\n");
		return ret;
	}

	writel(data, MCHBAR_REG(BIOS_MAILBOX_DATA));

	/* Send command and start transaction */
	writel(command | MAILBOX_RUN_BUSY, MCHBAR_REG(BIOS_MAILBOX_INTERFACE));

	ret = pcode_ready();
	if (ret) {
		debug("PCODE: mailbox timeout on completion\n");
		return ret;
	}

	return 0;
}

/* @dev is the CPU device */
static void initialize_vr_config(struct udevice *dev)
{
	int ramp, min_vid;
	msr_t msr;

	debug("Initializing VR config\n");

	/* Configure VR_CURRENT_CONFIG */
	msr = msr_read(MSR_VR_CURRENT_CONFIG);
	/*
	 * Preserve bits 63 and 62. Bit 62 is PSI4 enable, but it is only valid
	 * on ULT systems
	 */
	msr.hi &= 0xc0000000;
	msr.hi |= (0x01 << (52 - 32)); /* PSI3 threshold -  1A */
	msr.hi |= (0x05 << (42 - 32)); /* PSI2 threshold -  5A */
	msr.hi |= (0x14 << (32 - 32)); /* PSI1 threshold - 20A */
	msr.hi |= (1 <<  (62 - 32)); /* Enable PSI4 */
	/* Leave the max instantaneous current limit (12:0) to default */
	msr_write(MSR_VR_CURRENT_CONFIG, msr);

	/* Configure VR_MISC_CONFIG MSR */
	msr = msr_read(MSR_VR_MISC_CONFIG);
	/* Set the IOUT_SLOPE scalar applied to dIout in U10.1.9 format */
	msr.hi &= ~(0x3ff << (40 - 32));
	msr.hi |= (0x200 << (40 - 32)); /* 1.0 */
	/* Set IOUT_OFFSET to 0 */
	msr.hi &= ~0xff;
	/* Set entry ramp rate to slow */
	msr.hi &= ~(1 << (51 - 32));
	/* Enable decay mode on C-state entry */
	msr.hi |= (1 << (52 - 32));
	/* Set the slow ramp rate */
	msr.hi &= ~(0x3 << (53 - 32));
	/* Configure the C-state exit ramp rate */
	ramp = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
			      "intel,slow-ramp", -1);
	if (ramp != -1) {
		/* Configured slow ramp rate */
		msr.hi |= ((ramp & 0x3) << (53 - 32));
		/* Set exit ramp rate to slow */
		msr.hi &= ~(1 << (50 - 32));
	} else {
		/* Fast ramp rate / 4 */
		msr.hi |= (0x01 << (53 - 32));
		/* Set exit ramp rate to fast */
		msr.hi |= (1 << (50 - 32));
	}
	/* Set MIN_VID (31:24) to allow CPU to have full control */
	msr.lo &= ~0xff000000;
	min_vid = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				 "intel,min-vid", 0);
	msr.lo |= (min_vid & 0xff) << 24;
	msr_write(MSR_VR_MISC_CONFIG, msr);

	/*  Configure VR_MISC_CONFIG2 MSR */
	msr = msr_read(MSR_VR_MISC_CONFIG2);
	msr.lo &= ~0xffff;
	/*
	 * Allow CPU to control minimum voltage completely (15:8) and
	 * set the fast ramp voltage in 10mV steps
	 */
	if (cpu_get_family_model() == BROADWELL_FAMILY_ULT)
		msr.lo |= 0x006a; /* 1.56V */
	else
		msr.lo |= 0x006f; /* 1.60V */
	msr_write(MSR_VR_MISC_CONFIG2, msr);

	/* Set C9/C10 VCC Min */
	pcode_mailbox_write(MAILBOX_BIOS_CMD_WRITE_C9C10_VOLTAGE, 0x1f1f);
}

static int calibrate_24mhz_bclk(void)
{
	int err_code;
	int ret;

	ret = pcode_ready();
	if (ret)
		return ret;

	/* A non-zero value initiates the PCODE calibration */
	writel(~0, MCHBAR_REG(BIOS_MAILBOX_DATA));
	writel(MAILBOX_RUN_BUSY | MAILBOX_BIOS_CMD_FSM_MEASURE_INTVL,
	       MCHBAR_REG(BIOS_MAILBOX_INTERFACE));

	ret = pcode_ready();
	if (ret)
		return ret;

	err_code = readl(MCHBAR_REG(BIOS_MAILBOX_INTERFACE)) & 0xff;

	debug("PCODE: 24MHz BLCK calibration response: %d\n", err_code);

	/* Read the calibrated value */
	writel(MAILBOX_RUN_BUSY | MAILBOX_BIOS_CMD_READ_CALIBRATION,
	       MCHBAR_REG(BIOS_MAILBOX_INTERFACE));

	ret = pcode_ready();
	if (ret)
		return ret;

	debug("PCODE: 24MHz BLCK calibration value: 0x%08x\n",
	      readl(MCHBAR_REG(BIOS_MAILBOX_DATA)));

	return 0;
}

static void configure_pch_power_sharing(void)
{
	u32 pch_power, pch_power_ext, pmsync, pmsync2;
	int i;

	/* Read PCH Power levels from PCODE */
	pch_power = pcode_mailbox_read(MAILBOX_BIOS_CMD_READ_PCH_POWER);
	pch_power_ext = pcode_mailbox_read(MAILBOX_BIOS_CMD_READ_PCH_POWER_EXT);

	debug("PCH Power: PCODE Levels 0x%08x 0x%08x\n", pch_power,
	      pch_power_ext);

	pmsync = readl(RCB_REG(PMSYNC_CONFIG));
	pmsync2 = readl(RCB_REG(PMSYNC_CONFIG2));

	/*
	 * Program PMSYNC_TPR_CONFIG PCH power limit values
	 *  pmsync[0:4]   = mailbox[0:5]
	 *  pmsync[8:12]  = mailbox[6:11]
	 *  pmsync[16:20] = mailbox[12:17]
	 */
	for (i = 0; i < 3; i++) {
		u32 level = pch_power & 0x3f;

		pch_power >>= 6;
		pmsync &= ~(0x1f << (i * 8));
		pmsync |= (level & 0x1f) << (i * 8);
	}
	writel(pmsync, RCB_REG(PMSYNC_CONFIG));

	/*
	 * Program PMSYNC_TPR_CONFIG2 Extended PCH power limit values
	 *  pmsync2[0:4]   = mailbox[23:18]
	 *  pmsync2[8:12]  = mailbox_ext[6:11]
	 *  pmsync2[16:20] = mailbox_ext[12:17]
	 *  pmsync2[24:28] = mailbox_ext[18:22]
	 */
	pmsync2 &= ~0x1f;
	pmsync2 |= pch_power & 0x1f;

	for (i = 1; i < 4; i++) {
		u32 level = pch_power_ext & 0x3f;

		pch_power_ext >>= 6;
		pmsync2 &= ~(0x1f << (i * 8));
		pmsync2 |= (level & 0x1f) << (i * 8);
	}
	writel(pmsync2, RCB_REG(PMSYNC_CONFIG2));
}

static int bsp_init_before_ap_bringup(struct udevice *dev)
{
	int ret;

	initialize_vr_config(dev);
	ret = calibrate_24mhz_bclk();
	if (ret)
		return ret;
	configure_pch_power_sharing();

	return 0;
}

static int cpu_config_tdp_levels(void)
{
	msr_t platform_info;

	/* Bits 34:33 indicate how many levels supported */
	platform_info = msr_read(MSR_PLATFORM_INFO);
	return (platform_info.hi >> 1) & 3;
}

static void set_max_ratio(void)
{
	msr_t msr, perf_ctl;

	perf_ctl.hi = 0;

	/* Check for configurable TDP option */
	if (turbo_get_state() == TURBO_ENABLED) {
		msr = msr_read(MSR_NHM_TURBO_RATIO_LIMIT);
		perf_ctl.lo = (msr.lo & 0xff) << 8;
	} else if (cpu_config_tdp_levels()) {
		/* Set to nominal TDP ratio */
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		perf_ctl.lo = (msr.lo & 0xff) << 8;
	} else {
		/* Platform Info bits 15:8 give max ratio */
		msr = msr_read(MSR_PLATFORM_INFO);
		perf_ctl.lo = msr.lo & 0xff00;
	}
	msr_write(IA32_PERF_CTL, perf_ctl);

	debug("cpu: frequency set to %d\n",
	      ((perf_ctl.lo >> 8) & 0xff) * CPU_BCLK);
}

int broadwell_init(struct udevice *dev)
{
	struct cpu_broadwell_priv *priv = dev_get_priv(dev);
	int num_threads;
	int num_cores;
	msr_t msr;
	int ret;

	msr = msr_read(CORE_THREAD_COUNT_MSR);
	num_threads = (msr.lo >> 0) & 0xffff;
	num_cores = (msr.lo >> 16) & 0xffff;
	debug("CPU has %u cores, %u threads enabled\n", num_cores,
	      num_threads);

	priv->ht_disabled = num_threads == num_cores;

	ret = bsp_init_before_ap_bringup(dev);
	if (ret)
		return ret;

	set_max_ratio();

	return ret;
}

static void configure_mca(void)
{
	msr_t msr;
	const unsigned int mcg_cap_msr = 0x179;
	int i;
	int num_banks;

	msr = msr_read(mcg_cap_msr);
	num_banks = msr.lo & 0xff;
	msr.lo = 0;
	msr.hi = 0;
	/*
	 * TODO(adurbin): This should only be done on a cold boot. Also, some
	 * of these banks are core vs package scope. For now every CPU clears
	 * every bank
	 */
	for (i = 0; i < num_banks; i++)
		msr_write(MSR_IA32_MC0_STATUS + (i * 4), msr);
}

static void enable_lapic_tpr(void)
{
	msr_t msr;

	msr = msr_read(MSR_PIC_MSG_CONTROL);
	msr.lo &= ~(1 << 10);	/* Enable APIC TPR updates */
	msr_write(MSR_PIC_MSG_CONTROL, msr);
}

static void configure_c_states(void)
{
	msr_t msr;

	msr = msr_read(MSR_PMG_CST_CONFIG_CONTROL);
	msr.lo |= (1 << 31);	/* Timed MWAIT Enable */
	msr.lo |= (1 << 30);	/* Package c-state Undemotion Enable */
	msr.lo |= (1 << 29);	/* Package c-state Demotion Enable */
	msr.lo |= (1 << 28);	/* C1 Auto Undemotion Enable */
	msr.lo |= (1 << 27);	/* C3 Auto Undemotion Enable */
	msr.lo |= (1 << 26);	/* C1 Auto Demotion Enable */
	msr.lo |= (1 << 25);	/* C3 Auto Demotion Enable */
	msr.lo &= ~(1 << 10);	/* Disable IO MWAIT redirection */
	/* The deepest package c-state defaults to factory-configured value */
	msr_write(MSR_PMG_CST_CONFIG_CONTROL, msr);

	msr = msr_read(MSR_MISC_PWR_MGMT);
	msr.lo &= ~(1 << 0);	/* Enable P-state HW_ALL coordination */
	msr_write(MSR_MISC_PWR_MGMT, msr);

	msr = msr_read(MSR_POWER_CTL);
	msr.lo |= (1 << 18);	/* Enable Energy Perf Bias MSR 0x1b0 */
	msr.lo |= (1 << 1);	/* C1E Enable */
	msr.lo |= (1 << 0);	/* Bi-directional PROCHOT# */
	msr_write(MSR_POWER_CTL, msr);

	/* C-state Interrupt Response Latency Control 0 - package C3 latency */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_0_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_0, msr);

	/* C-state Interrupt Response Latency Control 1 */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_1_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_1, msr);

	/* C-state Interrupt Response Latency Control 2 - package C6/C7 short */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_2_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_2, msr);

	/* C-state Interrupt Response Latency Control 3 - package C8 */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_3_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_3, msr);

	/* C-state Interrupt Response Latency Control 4 - package C9 */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_4_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_4, msr);

	/* C-state Interrupt Response Latency Control 5 - package C10 */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | C_STATE_LATENCY_CONTROL_5_LIMIT;
	msr_write(MSR_C_STATE_LATENCY_CONTROL_5, msr);
}

static void configure_misc(void)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_MISC_ENABLE);
	msr.lo |= (1 << 0);	  /* Fast String enable */
	msr.lo |= (1 << 3);	  /* TM1/TM2/EMTTM enable */
	msr.lo |= (1 << 16);	  /* Enhanced SpeedStep Enable */
	msr_write(MSR_IA32_MISC_ENABLE, msr);

	/* Disable thermal interrupts */
	msr.lo = 0;
	msr.hi = 0;
	msr_write(MSR_IA32_THERM_INTERRUPT, msr);

	/* Enable package critical interrupt only */
	msr.lo = 1 << 4;
	msr.hi = 0;
	msr_write(MSR_IA32_PACKAGE_THERM_INTERRUPT, msr);
}

static void configure_thermal_target(struct udevice *dev)
{
	int tcc_offset;
	msr_t msr;

	tcc_offset = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				    "intel,tcc-offset", 0);

	/* Set TCC activaiton offset if supported */
	msr = msr_read(MSR_PLATFORM_INFO);
	if ((msr.lo & (1 << 30)) && tcc_offset) {
		msr = msr_read(MSR_TEMPERATURE_TARGET);
		msr.lo &= ~(0xf << 24); /* Bits 27:24 */
		msr.lo |= (tcc_offset & 0xf) << 24;
		msr_write(MSR_TEMPERATURE_TARGET, msr);
	}
}

static void configure_dca_cap(void)
{
	struct cpuid_result cpuid_regs;
	msr_t msr;

	/* Check feature flag in CPUID.(EAX=1):ECX[18]==1 */
	cpuid_regs = cpuid(1);
	if (cpuid_regs.ecx & (1 << 18)) {
		msr = msr_read(MSR_IA32_PLATFORM_DCA_CAP);
		msr.lo |= 1;
		msr_write(MSR_IA32_PLATFORM_DCA_CAP, msr);
	}
}

static void set_energy_perf_bias(u8 policy)
{
	msr_t msr;
	int ecx;

	/* Determine if energy efficient policy is supported */
	ecx = cpuid_ecx(0x6);
	if (!(ecx & (1 << 3)))
		return;

	/* Energy Policy is bits 3:0 */
	msr = msr_read(MSR_IA32_ENERGY_PERFORMANCE_BIAS);
	msr.lo &= ~0xf;
	msr.lo |= policy & 0xf;
	msr_write(MSR_IA32_ENERGY_PERFORMANCE_BIAS, msr);

	debug("cpu: energy policy set to %u\n", policy);
}

/* All CPUs including BSP will run the following function */
static void cpu_core_init(struct udevice *dev)
{
	/* Clear out pending MCEs */
	configure_mca();

	/* Enable the local cpu apics */
	enable_lapic_tpr();

	/* Configure C States */
	configure_c_states();

	/* Configure Enhanced SpeedStep and Thermal Sensors */
	configure_misc();

	/* Thermal throttle activation offset */
	configure_thermal_target(dev);

	/* Enable Direct Cache Access */
	configure_dca_cap();

	/* Set energy policy */
	set_energy_perf_bias(ENERGY_POLICY_NORMAL);

	/* Enable Turbo */
	turbo_enable();
}

/*
 * Configure processor power limits if possible
 * This must be done AFTER set of BIOS_RESET_CPL
 */
void cpu_set_power_limits(int power_limit_1_time)
{
	msr_t msr;
	msr_t limit;
	uint power_unit;
	uint tdp, min_power, max_power, max_time;
	u8 power_limit_1_val;

	msr = msr_read(MSR_PLATFORM_INFO);
	if (power_limit_1_time > ARRAY_SIZE(power_limit_time_sec_to_msr))
		power_limit_1_time = 28;

	if (!(msr.lo & PLATFORM_INFO_SET_TDP))
		return;

	/* Get units */
	msr = msr_read(MSR_PKG_POWER_SKU_UNIT);
	power_unit = 2 << ((msr.lo & 0xf) - 1);

	/* Get power defaults for this SKU */
	msr = msr_read(MSR_PKG_POWER_SKU);
	tdp = msr.lo & 0x7fff;
	min_power = (msr.lo >> 16) & 0x7fff;
	max_power = msr.hi & 0x7fff;
	max_time = (msr.hi >> 16) & 0x7f;

	debug("CPU TDP: %u Watts\n", tdp / power_unit);

	if (power_limit_time_msr_to_sec[max_time] > power_limit_1_time)
		power_limit_1_time = power_limit_time_msr_to_sec[max_time];

	if (min_power > 0 && tdp < min_power)
		tdp = min_power;

	if (max_power > 0 && tdp > max_power)
		tdp = max_power;

	power_limit_1_val = power_limit_time_sec_to_msr[power_limit_1_time];

	/* Set long term power limit to TDP */
	limit.lo = 0;
	limit.lo |= tdp & PKG_POWER_LIMIT_MASK;
	limit.lo |= PKG_POWER_LIMIT_EN;
	limit.lo |= (power_limit_1_val & PKG_POWER_LIMIT_TIME_MASK) <<
		PKG_POWER_LIMIT_TIME_SHIFT;

	/* Set short term power limit to 1.25 * TDP */
	limit.hi = 0;
	limit.hi |= ((tdp * 125) / 100) & PKG_POWER_LIMIT_MASK;
	limit.hi |= PKG_POWER_LIMIT_EN;
	/* Power limit 2 time is only programmable on server SKU */

	msr_write(MSR_PKG_POWER_LIMIT, limit);

	/* Set power limit values in MCHBAR as well */
	writel(limit.lo, MCHBAR_REG(MCH_PKG_POWER_LIMIT_LO));
	writel(limit.hi, MCHBAR_REG(MCH_PKG_POWER_LIMIT_HI));

	/* Set DDR RAPL power limit by copying from MMIO to MSR */
	msr.lo = readl(MCHBAR_REG(MCH_DDR_POWER_LIMIT_LO));
	msr.hi = readl(MCHBAR_REG(MCH_DDR_POWER_LIMIT_HI));
	msr_write(MSR_DDR_RAPL_LIMIT, msr);

	/* Use nominal TDP values for CPUs with configurable TDP */
	if (cpu_config_tdp_levels()) {
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		limit.hi = 0;
		limit.lo = msr.lo & 0xff;
		msr_write(MSR_TURBO_ACTIVATION_RATIO, limit);
	}
}

static int broadwell_get_info(struct udevice *dev, struct cpu_info *info)
{
	msr_t msr;

	msr = msr_read(IA32_PERF_CTL);
	info->cpu_freq = ((msr.lo >> 8) & 0xff) * BROADWELL_BCLK * 1000000;
	info->features = 1 << CPU_FEAT_L1_CACHE | 1 << CPU_FEAT_MMU |
		1 << CPU_FEAT_UCODE | 1 << CPU_FEAT_DEVICE_ID;

	return 0;
}

static int broadwell_get_count(struct udevice *dev)
{
	return 4;
}

static int cpu_x86_broadwell_probe(struct udevice *dev)
{
	if (dev->seq == 0) {
		cpu_core_init(dev);
		return broadwell_init(dev);
	}

	return 0;
}

static const struct cpu_ops cpu_x86_broadwell_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= broadwell_get_info,
	.get_count	= broadwell_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_broadwell_ids[] = {
	{ .compatible = "intel,core-i3-gen5" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_broadwell_drv) = {
	.name		= "cpu_x86_broadwell",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_broadwell_ids,
	.bind		= cpu_x86_bind,
	.probe		= cpu_x86_broadwell_probe,
	.ops		= &cpu_x86_broadwell_ops,
	.priv_auto_alloc_size	= sizeof(struct cpu_broadwell_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
