/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014-2015 Freescale Semiconductor, Inc.
 *
 * Author:
 *	Peng Fan <Peng.Fan@freescale.com>
 */

#ifndef _ASM_ARCH_CLOCK_SLICE_H
#define _ASM_ARCH_CLOCK_SLICE_H

enum root_pre_div {
	CLK_ROOT_PRE_DIV1 = 0,
	CLK_ROOT_PRE_DIV2,
	CLK_ROOT_PRE_DIV3,
	CLK_ROOT_PRE_DIV4,
	CLK_ROOT_PRE_DIV5,
	CLK_ROOT_PRE_DIV6,
	CLK_ROOT_PRE_DIV7,
	CLK_ROOT_PRE_DIV8,
};

enum root_post_div {
	CLK_ROOT_POST_DIV1 = 0,
	CLK_ROOT_POST_DIV2,
	CLK_ROOT_POST_DIV3,
	CLK_ROOT_POST_DIV4,
	CLK_ROOT_POST_DIV5,
	CLK_ROOT_POST_DIV6,
	CLK_ROOT_POST_DIV7,
	CLK_ROOT_POST_DIV8,
	CLK_ROOT_POST_DIV9,
	CLK_ROOT_POST_DIV10,
	CLK_ROOT_POST_DIV11,
	CLK_ROOT_POST_DIV12,
	CLK_ROOT_POST_DIV13,
	CLK_ROOT_POST_DIV14,
	CLK_ROOT_POST_DIV15,
	CLK_ROOT_POST_DIV16,
	CLK_ROOT_POST_DIV17,
	CLK_ROOT_POST_DIV18,
	CLK_ROOT_POST_DIV19,
	CLK_ROOT_POST_DIV20,
	CLK_ROOT_POST_DIV21,
	CLK_ROOT_POST_DIV22,
	CLK_ROOT_POST_DIV23,
	CLK_ROOT_POST_DIV24,
	CLK_ROOT_POST_DIV25,
	CLK_ROOT_POST_DIV26,
	CLK_ROOT_POST_DIV27,
	CLK_ROOT_POST_DIV28,
	CLK_ROOT_POST_DIV29,
	CLK_ROOT_POST_DIV30,
	CLK_ROOT_POST_DIV31,
	CLK_ROOT_POST_DIV32,
	CLK_ROOT_POST_DIV33,
	CLK_ROOT_POST_DIV34,
	CLK_ROOT_POST_DIV35,
	CLK_ROOT_POST_DIV36,
	CLK_ROOT_POST_DIV37,
	CLK_ROOT_POST_DIV38,
	CLK_ROOT_POST_DIV39,
	CLK_ROOT_POST_DIV40,
	CLK_ROOT_POST_DIV41,
	CLK_ROOT_POST_DIV42,
	CLK_ROOT_POST_DIV43,
	CLK_ROOT_POST_DIV44,
	CLK_ROOT_POST_DIV45,
	CLK_ROOT_POST_DIV46,
	CLK_ROOT_POST_DIV47,
	CLK_ROOT_POST_DIV48,
	CLK_ROOT_POST_DIV49,
	CLK_ROOT_POST_DIV50,
	CLK_ROOT_POST_DIV51,
	CLK_ROOT_POST_DIV52,
	CLK_ROOT_POST_DIV53,
	CLK_ROOT_POST_DIV54,
	CLK_ROOT_POST_DIV55,
	CLK_ROOT_POST_DIV56,
	CLK_ROOT_POST_DIV57,
	CLK_ROOT_POST_DIV58,
	CLK_ROOT_POST_DIV59,
	CLK_ROOT_POST_DIV60,
	CLK_ROOT_POST_DIV61,
	CLK_ROOT_POST_DIV62,
	CLK_ROOT_POST_DIV63,
	CLK_ROOT_POST_DIV64,
};

enum root_auto_div {
	CLK_ROOT_AUTO_DIV1 = 0,
	CLK_ROOT_AUTO_DIV2,
	CLK_ROOT_AUTO_DIV4,
	CLK_ROOT_AUTO_DIV8,
	CLK_ROOT_AUTO_DIV16,
};

int clock_set_src(enum clk_root_index clock_id, enum clk_root_src clock_src);
int clock_get_src(enum clk_root_index clock_id, enum clk_root_src *p_clock_src);
int clock_set_prediv(enum clk_root_index clock_id, enum root_pre_div pre_div);
int clock_get_prediv(enum clk_root_index clock_id, enum root_pre_div *pre_div);
int clock_set_postdiv(enum clk_root_index clock_id, enum root_post_div div);
int clock_get_postdiv(enum clk_root_index clock_id, enum root_post_div *div);
int clock_set_autopostdiv(enum clk_root_index clock_id, enum root_auto_div div,
			  int auto_en);
int clock_get_autopostdiv(enum clk_root_index clock_id, enum root_auto_div *div,
			  int *auto_en);
int clock_get_target_val(enum clk_root_index clock_id, u32 *val);
int clock_set_target_val(enum clk_root_index clock_id, u32 val);
int clock_root_cfg(enum clk_root_index clock_id, enum root_pre_div pre_div,
		   enum root_post_div post_div, enum clk_root_src clock_src);
int clock_root_enabled(enum clk_root_index clock_id);

int clock_enable(enum clk_ccgr_index index, bool enable);
#endif
