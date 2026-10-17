/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 */

#ifndef _ASM_ARCH_HOSTBRIDGE_H_
#define _ASM_ARCH_HOSTBRIDGE_H_

/**
 * struct apl_hostbridge_plat - platform data for hostbridge
 *
 * @dtplat: Platform data for of-platdata
 * @early_pads: Early pad data to set up, each (pad, cfg0, cfg1)
 * @early_pads_count: Number of pads to process
 * @pciex_region_size: BAR length in bytes
 * @bdf: Bus/device/function of hostbridge
 */
struct apl_hostbridge_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_intel_apl_hostbridge dtplat;
#endif
	u32 *early_pads;
	int early_pads_count;
	uint pciex_region_size;
	pci_dev_t bdf;
};

#endif /* _ASM_ARCH_HOSTBRIDGE_H_ */
