/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __asm_arch_cpu_h
#define __asm_arch_cpu_h

/* CPU types */
#define HASWELL_FAMILY_ULT	0x40650
#define BROADWELL_FAMILY_ULT	0x306d0

/* Supported CPUIDs */
#define CPUID_HASWELL_A0	0x306c1
#define CPUID_HASWELL_B0	0x306c2
#define CPUID_HASWELL_C0	0x306c3
#define CPUID_HASWELL_ULT_B0	0x40650
#define CPUID_HASWELL_ULT	0x40651
#define CPUID_HASWELL_HALO	0x40661
#define CPUID_BROADWELL_C0	0x306d2
#define CPUID_BROADWELL_D0	0x306d3
#define CPUID_BROADWELL_E0	0x306d4

#define BROADWELL_FAMILY_ULT	0x306d0

#define CORE_THREAD_COUNT_MSR		0x35

#define MSR_VR_CURRENT_CONFIG		0x601
#define MSR_VR_MISC_CONFIG		0x603
#define MSR_DDR_RAPL_LIMIT		0x618
#define MSR_VR_MISC_CONFIG2		0x636

/* Latency times in units of 1024ns. */
#define C_STATE_LATENCY_CONTROL_0_LIMIT 0x42
#define C_STATE_LATENCY_CONTROL_1_LIMIT 0x73
#define C_STATE_LATENCY_CONTROL_2_LIMIT 0x91
#define C_STATE_LATENCY_CONTROL_3_LIMIT 0xe4
#define C_STATE_LATENCY_CONTROL_4_LIMIT 0x145
#define C_STATE_LATENCY_CONTROL_5_LIMIT 0x1ef

void cpu_set_power_limits(int power_limit_1_time);

#endif
