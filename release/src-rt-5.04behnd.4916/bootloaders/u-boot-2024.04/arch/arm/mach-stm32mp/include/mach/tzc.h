/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simple API for configuring TrustZone memory regions
 *
 * The premise is that the desired TZC layout is known beforehand, and it can
 * be configured in one step. tzc_configure() provides this functionality.
 */
#ifndef MACH_TZC_H
#define MACH_TZC_H

#include <linux/types.h>

enum tzc_sec_mode {
	TZC_ATTR_SEC_NONE = 0,
	TZC_ATTR_SEC_R = 1,
	TZC_ATTR_SEC_W = 2,
	TZC_ATTR_SEC_RW	 = 3
};

struct tzc_region {
	uintptr_t base;
	uintptr_t top;
	enum tzc_sec_mode sec_mode;
	uint16_t nsec_id;
	uint16_t filters_mask;
};

int tzc_configure(uintptr_t tzc, const struct tzc_region *cfg);
int tzc_disable_filters(uintptr_t tzc, uint16_t filters_mask);
int tzc_enable_filters(uintptr_t tzc, uint16_t filters_mask);
void tzc_dump_config(uintptr_t tzc);

#endif /* MACH_TZC_H */
