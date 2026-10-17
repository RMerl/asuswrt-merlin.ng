/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 */

#ifndef ASM_ARCH_PMC_H
#define ASM_ARCH_PMC_H

struct apl_pmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_intel_apl_pmc dtplat;
#endif
	pci_dev_t bdf;
};

#endif	/* ASM_ARCH_PMC_H */
