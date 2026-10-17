/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef _ASM_ARCH_CPU_H
#define _ASM_ARCH_CPU_H

/* Common Timer Copy (CTC) frequency - 19.2MHz */
#define CTC_FREQ		19200000

#define MAX_PCIE_PORTS		6
#define CLKREQ_DISABLED		0xf

#ifndef __ASSEMBLY__
/* Flush L1D to L2 */
void cpu_flush_l1d_to_l2(void);

/**
 * Enable emulation of the PM timer
 *
 * Some legacy OSes cannot tolerate the ACPI timer stoping during idle states,
 * and this results in higher power consumption. ACPI timer emulation allows
 * disabling of the ACPI Timer (PM1_TMR) to have no impact on the system, with
 * the exception that TMR_STS will not be set on an overflow condition. All
 * aligned 32-bit reads from the ACPI Timer port are valid and will behave as if
 * the ACPI timer remains enabled.
 *
 * @pmc: PMC device
 */
void enable_pm_timer_emulation(const struct udevice *pmc);
#endif

#endif /* _ASM_ARCH_CPU_H */
