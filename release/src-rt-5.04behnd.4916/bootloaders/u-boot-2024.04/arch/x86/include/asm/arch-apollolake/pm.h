/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 * Copyright 2019 Google LLC
 */

#ifndef _ASM_ARCH_PM_H
#define _ASM_ARCH_PM_H

#include <power/acpi_pmc.h>

#define PMC_GPE_SW_31_0	0
#define PMC_GPE_SW_63_32	1
#define PMC_GPE_NW_31_0	3
#define PMC_GPE_NW_63_32	4
#define PMC_GPE_NW_95_64	5
#define PMC_GPE_N_31_0		6
#define PMC_GPE_N_63_32	7
#define PMC_GPE_W_31_0		9

#define IRQ_REG			0x106c
#define SCI_IRQ_SHIFT		24
#define SCI_IRQ_MASK		(0xff << SCI_IRQ_SHIFT)
#define SCIS_IRQ9		9
#define SCIS_IRQ10		10
#define SCIS_IRQ11		11
#define SCIS_IRQ20		20
#define SCIS_IRQ21		21
#define SCIS_IRQ22		22
#define SCIS_IRQ23		23

/* P-state configuration */
#define PSS_MAX_ENTRIES		8
#define PSS_RATIO_STEP		2
#define PSS_LATENCY_TRANSITION	10
#define PSS_LATENCY_BUSMASTER	10

#ifndef __ASSEMBLY__
/* Track power state from reset to log events */
struct __packed chipset_power_state {
	u16 pm1_sts;
	u16 pm1_en;
	u32 pm1_cnt;
	u32 gpe0_sts[GPE0_REG_MAX];
	u32 gpe0_en[GPE0_REG_MAX];
	u16 tco1_sts;
	u16 tco2_sts;
	u32 prsts;
	u32 gen_pmcon1;
	u32 gen_pmcon2;
	u32 gen_pmcon3;
	u32 prev_sleep_state;
};
#endif /* !__ASSEMBLY__ */

#endif
