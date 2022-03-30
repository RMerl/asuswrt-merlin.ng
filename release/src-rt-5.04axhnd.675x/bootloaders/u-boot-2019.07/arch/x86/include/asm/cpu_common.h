/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_CPU_COMMON_H
#define __ASM_CPU_COMMON_H

#define IA32_PERF_CTL			0x199

/**
 * cpu_common_init() - Set up common CPU init
 *
 * This reports BIST failure, enables the LAPIC, updates microcode, enables
 * the upper 128-bytes of CROM RAM, probes the northbridge, PCH, LPC and SATA.
 *
 * @return 0 if OK, -ve on error
 */
int cpu_common_init(void);

/**
 * cpu_set_flex_ratio_to_tdp_nominal() - Set up the maximum non-turbo rate
 *
 * If a change is needed, this function will do a soft reset so it takes
 * effect.
 *
 * Some details are available here:
 * http://forum.hwbot.org/showthread.php?t=76092
 *
 * @return 0 if OK, -ve on error
 */
int cpu_set_flex_ratio_to_tdp_nominal(void);

#endif
