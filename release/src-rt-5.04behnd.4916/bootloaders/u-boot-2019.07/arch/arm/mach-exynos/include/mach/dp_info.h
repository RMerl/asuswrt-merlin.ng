/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef _DP_INFO_H
#define _DP_INFO_H

#define msleep(a)			udelay(a * 1000)

#define DP_TIMEOUT_LOOP_COUNT		100
#define MAX_CR_LOOP			5
#define MAX_EQ_LOOP			4

#define EXYNOS_DP_SUCCESS		0

enum {
	DP_DISABLE,
	DP_ENABLE,
};

struct edp_disp_info {
	char *name;
	unsigned int h_total;
	unsigned int h_res;
	unsigned int h_sync_width;
	unsigned int h_back_porch;
	unsigned int h_front_porch;
	unsigned int v_total;
	unsigned int v_res;
	unsigned int v_sync_width;
	unsigned int v_back_porch;
	unsigned int v_front_porch;

	unsigned int v_sync_rate;
};

struct edp_link_train_info {
	unsigned int lt_status;

	unsigned int ep_loop;
	unsigned int cr_loop[4];

};

struct edp_video_info {
	unsigned int master_mode;
	unsigned int bist_mode;
	unsigned int bist_pattern;

	unsigned int h_sync_polarity;
	unsigned int v_sync_polarity;
	unsigned int interlaced;

	unsigned int color_space;
	unsigned int dynamic_range;
	unsigned int ycbcr_coeff;
	unsigned int color_depth;
};

struct exynos_dp_priv {
	struct edp_disp_info disp_info;
	struct edp_link_train_info lt_info;
	struct edp_video_info video_info;

	/*below info get from panel during training*/
	unsigned char lane_bw;
	unsigned char lane_cnt;
	unsigned char dpcd_rev;
	/*support enhanced frame cap */
	unsigned char dpcd_efc;
	struct exynos_dp *regs;
};

enum analog_power_block {
	AUX_BLOCK,
	CH0_BLOCK,
	CH1_BLOCK,
	CH2_BLOCK,
	CH3_BLOCK,
	ANALOG_TOTAL,
	POWER_ALL
};

enum pll_status {
	PLL_UNLOCKED = 0,
	PLL_LOCKED
};

enum {
	COLOR_RGB,
	COLOR_YCBCR422,
	COLOR_YCBCR444
};

enum {
	VESA,
	CEA
};

enum {
	COLOR_YCBCR601,
	COLOR_YCBCR709
};

enum {
	COLOR_6,
	COLOR_8,
	COLOR_10,
	COLOR_12
};

enum {
	DP_LANE_BW_1_62 = 0x06,
	DP_LANE_BW_2_70 = 0x0a,
};

enum {
	DP_LANE_CNT_1 = 1,
	DP_LANE_CNT_2 = 2,
	DP_LANE_CNT_4 = 4,
};

enum {
	DP_DPCD_REV_10 = 0x10,
	DP_DPCD_REV_11 = 0x11,
};

enum {
	DP_LT_NONE,
	DP_LT_START,
	DP_LT_CR,
	DP_LT_ET,
	DP_LT_FINISHED,
	DP_LT_FAIL,
};

enum  {
	PRE_EMPHASIS_LEVEL_0,
	PRE_EMPHASIS_LEVEL_1,
	PRE_EMPHASIS_LEVEL_2,
	PRE_EMPHASIS_LEVEL_3,
};

enum {
	PRBS7,
	D10_2,
	TRAINING_PTN1,
	TRAINING_PTN2,
	DP_NONE
};

enum {
	VOLTAGE_LEVEL_0,
	VOLTAGE_LEVEL_1,
	VOLTAGE_LEVEL_2,
	VOLTAGE_LEVEL_3,
};

enum pattern_type {
	NO_PATTERN,
	COLOR_RAMP,
	BALCK_WHITE_V_LINES,
	COLOR_SQUARE,
	INVALID_PATTERN,
	COLORBAR_32,
	COLORBAR_64,
	WHITE_GRAY_BALCKBAR_32,
	WHITE_GRAY_BALCKBAR_64,
	MOBILE_WHITEBAR_32,
	MOBILE_WHITEBAR_64
};

enum {
	CALCULATED_M,
	REGISTER_M
};

enum {
	VIDEO_TIMING_FROM_CAPTURE,
	VIDEO_TIMING_FROM_REGISTER
};


struct exynos_dp_platform_data {
	struct exynos_dp_priv *edp_dev_info;
};

#ifdef CONFIG_EXYNOS_DP
unsigned int exynos_init_dp(void);
#else
unsigned int exynos_init_dp(void)
{
	return 0;
}
#endif

#endif /* _DP_INFO_H */
