/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From Coreboot src/southbridge/intel/bd82x6x/me.h
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 */

#ifndef _ASM_INTEL_ME_H
#define _ASM_INTEL_ME_H

#include <asm/me_common.h>

struct __packed mbp_fw_version_name {
	u32 major_version:16;
	u32 minor_version:16;
	u32 hotfix_version:16;
	u32 build_version:16;
};

struct __packed mbp_icc_profile {
	u8 num_icc_profiles;
	u8 icc_profile_soft_strap;
	u8 icc_profile_index;
	u8 reserved;
	u32 register_lock_mask[3];
};

struct __packed platform_type_rule_data {
	u32 platform_target_usage_type:4;
	u32 platform_target_market_type:2;
	u32 super_sku:1;
	u32 reserved:1;
	u32 intel_me_fw_image_type:4;
	u32 platform_brand:4;
	u32 reserved_1:16;
};

struct __packed mbp_fw_caps {
	struct mefwcaps_sku fw_capabilities;
	u8 available;
};

struct __packed mbp_plat_type {
	struct platform_type_rule_data rule_data;
	u8 available;
};

struct __packed me_bios_payload {
	struct mbp_fw_version_name fw_version_name;
	struct mbp_fw_caps fw_caps_sku;
	struct mbp_rom_bist_data rom_bist_data;
	struct mbp_platform_key platform_key;
	struct mbp_plat_type fw_plat_type;
	struct mbp_icc_profile icc_profile;
	struct tdt_state_info at_state;
	u32 mfsintegrity;
};

#endif
