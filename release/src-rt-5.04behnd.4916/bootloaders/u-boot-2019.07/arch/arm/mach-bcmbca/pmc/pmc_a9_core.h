// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Broadcom
 */

#ifndef PMC_CPU_CORE_H
#define PMC_CPU_CORE_H

int pmc_a9_core_power_up(unsigned cpu);
int pmc_a9_core_power_down(unsigned cpu);
int pmc_a9_l2cache_power_up(void);
int pmc_a9_l2cache_power_down(void);

#endif //#ifndef PMC_CPU_CORE_H
