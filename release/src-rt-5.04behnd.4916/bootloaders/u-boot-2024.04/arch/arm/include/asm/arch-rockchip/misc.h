/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * RK3399: Architecture common definitions
 *
 * Copyright (C) 2019 Collabora Inc - https://www.collabora.com/
 *      Rohan Garg <rohan.garg@collabora.com>
 */

int rockchip_cpuid_from_efuse(const u32 cpuid_offset,
			      const u32 cpuid_length,
			      u8 *cpuid);
int rockchip_cpuid_set(const u8 *cpuid, const u32 cpuid_length);
int rockchip_setup_macaddr(void);
void rockchip_capsule_update_board_setup(void);
