/*
 * Header file for Samsung DP (Display Port) interface driver.
 *
 * Copyright (C) 2012 Samsung Electronics Co., Ltd.
 * Author: Jingoo Han <jg1.han@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef _EXYNOS_DP_CORE_H
#define _EXYNOS_DP_CORE_H

#include <drm/drm_crtc.h>
#include <drm/drm_dp_helper.h>
#include <drm/exynos_drm.h>

#include "exynos_drm_drv.h"

#define DP_TIMEOUT_LOOP_COUNT 100
#define MAX_CR_LOOP 5
#define MAX_EQ_LOOP 5

enum link_rate_type {
	LINK_RATE_1_62GBPS = 0x06,
	LINK_RATE_2_70GBPS = 0x0a
};

enum link_lane_count_type {
	LANE_COUNT1 = 1,
	LANE_COUNT2 = 2,
	LANE_COUNT4 = 4
};

enum link_training_state {
	START,
	CLOCK_RECOVERY,
	EQUALIZER_TRAINING,
	FINISHED,
	FAILED
};

enum voltage_swing_level {
	VOLTAGE_LEVEL_0,
	VOLTAGE_LEVEL_1,
	VOLTAGE_LEVEL_2,
	VOLTAGE_LEVEL_3,
};

enum pre_emphasis_level {
	PRE_EMPHASIS_LEVEL_0,
	PRE_EMPHASIS_LEVEL_1,
	PRE_EMPHASIS_LEVEL_2,
	PRE_EMPHASIS_LEVEL_3,
};

enum pattern_set {
	PRBS7,
	D10_2,
	TRAINING_PTN1,
	TRAINING_PTN2,
	DP_NONE
};

enum color_space {
	COLOR_RGB,
	COLOR_YCBCR422,
	COLOR_YCBCR444
};

enum color_depth {
	COLOR_6,
	COLOR_8,
	COLOR_10,
	COLOR_12
};

enum color_coefficient {
	COLOR_YCBCR601,
	COLOR_YCBCR709
};

enum dynamic_range {
	VESA,
	CEA
};

enum pll_status {
	PLL_UNLOCKED,
	PLL_LOCKED
};

enum clock_recovery_m_value_type {
	CALCULATED_M,
	REGISTER_M
};

enum video_timing_recognition_type {
	VIDEO_TIMING_FROM_CAPTURE,
	VIDEO_TIMING_FROM_REGISTER
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

enum dp_irq_type {
	DP_IRQ_TYPE_HP_CABLE_IN,
	DP_IRQ_TYPE_HP_CABLE_OUT,
	DP_IRQ_TYPE_HP_CHANGE,
	DP_IRQ_TYPE_UNKNOWN,
};

struct video_info {
	char *name;

	bool h_sync_polarity;
	bool v_sync_polarity;
	bool interlaced;

	enum color_space color_space;
	enum dynamic_range dynamic_range;
	enum color_coefficient ycbcr_coeff;
	enum color_depth color_depth;

	enum link_rate_type link_rate;
	enum link_lane_count_type lane_count;
};

struct link_train {
	int eq_loop;
	int cr_loop[4];

	u8 link_rate;
	u8 lane_count;
	u8 training_lane[4];

	enum link_training_state lt_state;
};

struct exynos_dp_device {
	struct exynos_drm_display display;
	struct device		*dev;
	struct drm_device	*drm_dev;
	struct drm_connector	connector;
	struct drm_encoder	*encoder;
	struct drm_panel	*panel;
	struct drm_bridge	*bridge;
	struct clk		*clock;
	unsigned int		irq;
	void __iomem		*reg_base;

	struct video_info	*video_info;
	struct link_train	link_train;
	struct work_struct	hotplug_work;
	struct phy		*phy;
	int			dpms_mode;
	int			hpd_gpio;

	struct exynos_drm_panel_info priv;
};

/* exynos_dp_reg.c */
void exynos_dp_enable_video_mute(struct exynos_dp_device *dp, bool enable);
void exynos_dp_stop_video(struct exynos_dp_device *dp);
void exynos_dp_lane_swap(struct exynos_dp_device *dp, bool enable);
void exynos_dp_init_analog_param(struct exynos_dp_device *dp);
void exynos_dp_init_interrupt(struct exynos_dp_device *dp);
void exynos_dp_reset(struct exynos_dp_device *dp);
void exynos_dp_swreset(struct exynos_dp_device *dp);
void exynos_dp_config_interrupt(struct exynos_dp_device *dp);
enum pll_status exynos_dp_get_pll_lock_status(struct exynos_dp_device *dp);
void exynos_dp_set_pll_power_down(struct exynos_dp_device *dp, bool enable);
void exynos_dp_set_analog_power_down(struct exynos_dp_device *dp,
				enum analog_power_block block,
				bool enable);
void exynos_dp_init_analog_func(struct exynos_dp_device *dp);
void exynos_dp_init_hpd(struct exynos_dp_device *dp);
enum dp_irq_type exynos_dp_get_irq_type(struct exynos_dp_device *dp);
void exynos_dp_clear_hotplug_interrupts(struct exynos_dp_device *dp);
void exynos_dp_reset_aux(struct exynos_dp_device *dp);
void exynos_dp_init_aux(struct exynos_dp_device *dp);
int exynos_dp_get_plug_in_status(struct exynos_dp_device *dp);
void exynos_dp_enable_sw_function(struct exynos_dp_device *dp);
int exynos_dp_start_aux_transaction(struct exynos_dp_device *dp);
int exynos_dp_write_byte_to_dpcd(struct exynos_dp_device *dp,
				unsigned int reg_addr,
				unsigned char data);
int exynos_dp_read_byte_from_dpcd(struct exynos_dp_device *dp,
				unsigned int reg_addr,
				unsigned char *data);
int exynos_dp_write_bytes_to_dpcd(struct exynos_dp_device *dp,
				unsigned int reg_addr,
				unsigned int count,
				unsigned char data[]);
int exynos_dp_read_bytes_from_dpcd(struct exynos_dp_device *dp,
				unsigned int reg_addr,
				unsigned int count,
				unsigned char data[]);
int exynos_dp_select_i2c_device(struct exynos_dp_device *dp,
				unsigned int device_addr,
				unsigned int reg_addr);
int exynos_dp_read_byte_from_i2c(struct exynos_dp_device *dp,
				unsigned int device_addr,
				unsigned int reg_addr,
				unsigned int *data);
int exynos_dp_read_bytes_from_i2c(struct exynos_dp_device *dp,
				unsigned int device_addr,
				unsigned int reg_addr,
				unsigned int count,
				unsigned char edid[]);
void exynos_dp_set_link_bandwidth(struct exynos_dp_device *dp, u32 bwtype);
void exynos_dp_get_link_bandwidth(struct exynos_dp_device *dp, u32 *bwtype);
void exynos_dp_set_lane_count(struct exynos_dp_device *dp, u32 count);
void exynos_dp_get_lane_count(struct exynos_dp_device *dp, u32 *count);
void exynos_dp_enable_enhanced_mode(struct exynos_dp_device *dp, bool enable);
void exynos_dp_set_training_pattern(struct exynos_dp_device *dp,
				 enum pattern_set pattern);
void exynos_dp_set_lane0_pre_emphasis(struct exynos_dp_device *dp, u32 level);
void exynos_dp_set_lane1_pre_emphasis(struct exynos_dp_device *dp, u32 level);
void exynos_dp_set_lane2_pre_emphasis(struct exynos_dp_device *dp, u32 level);
void exynos_dp_set_lane3_pre_emphasis(struct exynos_dp_device *dp, u32 level);
void exynos_dp_set_lane0_link_training(struct exynos_dp_device *dp,
				u32 training_lane);
void exynos_dp_set_lane1_link_training(struct exynos_dp_device *dp,
				u32 training_lane);
void exynos_dp_set_lane2_link_training(struct exynos_dp_device *dp,
				u32 training_lane);
void exynos_dp_set_lane3_link_training(struct exynos_dp_device *dp,
				u32 training_lane);
u32 exynos_dp_get_lane0_link_training(struct exynos_dp_device *dp);
u32 exynos_dp_get_lane1_link_training(struct exynos_dp_device *dp);
u32 exynos_dp_get_lane2_link_training(struct exynos_dp_device *dp);
u32 exynos_dp_get_lane3_link_training(struct exynos_dp_device *dp);
void exynos_dp_reset_macro(struct exynos_dp_device *dp);
void exynos_dp_init_video(struct exynos_dp_device *dp);

void exynos_dp_set_video_color_format(struct exynos_dp_device *dp);
int exynos_dp_is_slave_video_stream_clock_on(struct exynos_dp_device *dp);
void exynos_dp_set_video_cr_mn(struct exynos_dp_device *dp,
			enum clock_recovery_m_value_type type,
			u32 m_value,
			u32 n_value);
void exynos_dp_set_video_timing_mode(struct exynos_dp_device *dp, u32 type);
void exynos_dp_enable_video_master(struct exynos_dp_device *dp, bool enable);
void exynos_dp_start_video(struct exynos_dp_device *dp);
int exynos_dp_is_video_stream_on(struct exynos_dp_device *dp);
void exynos_dp_config_video_slave_mode(struct exynos_dp_device *dp);
void exynos_dp_enable_scrambling(struct exynos_dp_device *dp);
void exynos_dp_disable_scrambling(struct exynos_dp_device *dp);

/* I2C EDID Chip ID, Slave Address */
#define I2C_EDID_DEVICE_ADDR			0x50
#define I2C_E_EDID_DEVICE_ADDR			0x30

#define EDID_BLOCK_LENGTH			0x80
#define EDID_HEADER_PATTERN			0x00
#define EDID_EXTENSION_FLAG			0x7e
#define EDID_CHECKSUM				0x7f

/* DP_MAX_LANE_COUNT */
#define DPCD_ENHANCED_FRAME_CAP(x)		(((x) >> 7) & 0x1)
#define DPCD_MAX_LANE_COUNT(x)			((x) & 0x1f)

/* DP_LANE_COUNT_SET */
#define DPCD_LANE_COUNT_SET(x)			((x) & 0x1f)

/* DP_TRAINING_LANE0_SET */
#define DPCD_PRE_EMPHASIS_SET(x)		(((x) & 0x3) << 3)
#define DPCD_PRE_EMPHASIS_GET(x)		(((x) >> 3) & 0x3)
#define DPCD_VOLTAGE_SWING_SET(x)		(((x) & 0x3) << 0)
#define DPCD_VOLTAGE_SWING_GET(x)		(((x) >> 0) & 0x3)

#endif /* _EXYNOS_DP_CORE_H */
