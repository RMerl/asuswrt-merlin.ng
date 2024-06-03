/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From Coreboot file of the same name
 *
 * Copyright (C) 2011 The ChromiumOS Authors.
 */

#ifndef _ASM_ARCH_MODEL_206AX_H
#define _ASM_ARCH_MODEL_206AX_H

/* SandyBridge/IvyBridge bus clock is fixed at 100MHz */
#define SANDYBRIDGE_BCLK		100

#define  CPUID_VMX			(1 << 5)
#define  CPUID_SMX			(1 << 6)
#define MSR_FEATURE_CONFIG		0x13c
#define IA32_PLATFORM_DCA_CAP		0x1f8
#define IA32_MISC_ENABLE		0x1a0
#define MSR_TEMPERATURE_TARGET		0x1a2
#define IA32_THERM_INTERRUPT		0x19b
#define IA32_ENERGY_PERFORMANCE_BIAS	0x1b0
#define  ENERGY_POLICY_PERFORMANCE	0
#define  ENERGY_POLICY_NORMAL		6
#define  ENERGY_POLICY_POWERSAVE	15
#define IA32_PACKAGE_THERM_INTERRUPT	0x1b2
#define MSR_LT_LOCK_MEMORY		0x2e7
#define IA32_MC0_STATUS		0x401

#define MSR_MISC_PWR_MGMT		0x1aa
#define  MISC_PWR_MGMT_EIST_HW_DIS	(1 << 0)

#define MSR_PKGC3_IRTL			0x60a
#define MSR_PKGC6_IRTL			0x60b
#define MSR_PKGC7_IRTL			0x60c
#define  IRTL_VALID			(1 << 15)
#define  IRTL_1_NS			(0 << 10)
#define  IRTL_32_NS			(1 << 10)
#define  IRTL_1024_NS			(2 << 10)
#define  IRTL_32768_NS			(3 << 10)
#define  IRTL_1048576_NS		(4 << 10)
#define  IRTL_33554432_NS		(5 << 10)
#define  IRTL_RESPONSE_MASK		(0x3ff)

#define MSR_PP0_CURRENT_CONFIG		0x601
#define  PP0_CURRENT_LIMIT		(112 << 3) /* 112 A */
#define MSR_PP1_CURRENT_CONFIG		0x602
#define  PP1_CURRENT_LIMIT_SNB		(35 << 3) /* 35 A */
#define  PP1_CURRENT_LIMIT_IVB		(50 << 3) /* 50 A */
#define MSR_PKG_POWER_SKU		0x614

#define IVB_CONFIG_TDP_MIN_CPUID	0x306a2
#define MSR_CONFIG_TDP_LEVEL1		0x649
#define MSR_CONFIG_TDP_LEVEL2		0x64a
#define MSR_CONFIG_TDP_CONTROL		0x64b

/* P-state configuration */
#define PSS_MAX_ENTRIES			8
#define PSS_RATIO_STEP			2
#define PSS_LATENCY_TRANSITION		10
#define PSS_LATENCY_BUSMASTER		10

/* Configure power limits for turbo mode */
void set_power_limits(u8 power_limit_1_time);
int cpu_config_tdp_levels(void);

#endif
