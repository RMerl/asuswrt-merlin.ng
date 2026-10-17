/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_CONFIGS_H__
#define __FSP_CONFIGS_H__

struct platform_config {
	u8 enable_ht;
	u8 enable_turbo;
	u8 enable_memory_down;
	u8 enable_fast_boot;
};

/*
 * Dummy structure for now as currently only SPD is verified in U-Boot.
 *
 * We can add the missing parameters when adding support on a board with
 * memory down configuration.
 */
struct memory_config {
	u8 dummy;
};

struct fsp_config_data {
	struct fsp_cfg_common common;
	struct platform_config plat_config;
	struct memory_config mem_config;
};

struct fspinit_rtbuf {
	u32 stack_top;
	u32 boot_mode;
	struct platform_config *plat_config;
	struct memory_config *mem_config;
};

#endif /* __FSP_CONFIGS_H__ */
