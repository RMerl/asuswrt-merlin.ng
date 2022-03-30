/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef _EXYNOS_EDP_LOWLEVEL_H
#define _EXYNOS_EDP_LOWLEVEL_H

void exynos_dp_enable_video_bist(struct exynos_dp *dp_regs,
				 unsigned int enable);
void exynos_dp_enable_video_mute(struct exynos_dp *dp_regs,
				 unsigned int enable);
void exynos_dp_reset(struct exynos_dp *dp_regs);
void exynos_dp_enable_sw_func(struct exynos_dp *dp_regs, unsigned int enable);
unsigned int exynos_dp_set_analog_power_down(struct exynos_dp *dp_regs,
					     unsigned int block, u32 enable);
unsigned int exynos_dp_get_pll_lock_status(struct exynos_dp *dp_regs);
int exynos_dp_init_analog_func(struct exynos_dp *dp_regs);
void exynos_dp_init_hpd(struct exynos_dp *dp_regs);
void exynos_dp_init_aux(struct exynos_dp *dp_regs);
void exynos_dp_config_interrupt(struct exynos_dp *dp_regs);
unsigned int exynos_dp_get_plug_in_status(struct exynos_dp *dp_regs);
unsigned int exynos_dp_detect_hpd(struct exynos_dp *dp_regs);
unsigned int exynos_dp_start_aux_transaction(struct exynos_dp *dp_regs);
unsigned int exynos_dp_write_byte_to_dpcd(struct exynos_dp *dp_regs,
					  unsigned int reg_addr,
					  unsigned char data);
unsigned int exynos_dp_read_byte_from_dpcd(struct exynos_dp *dp_regs,
					   unsigned int reg_addr,
					   unsigned char *data);
unsigned int exynos_dp_write_bytes_to_dpcd(struct exynos_dp *dp_regs,
					   unsigned int reg_addr,
					   unsigned int count,
					   unsigned char data[]);
unsigned int exynos_dp_read_bytes_from_dpcd(struct exynos_dp *dp_regs,
					    unsigned int reg_addr,
					    unsigned int count,
					    unsigned char data[]);
int exynos_dp_select_i2c_device(struct exynos_dp *dp_regs,
				unsigned int device_addr,
				unsigned int reg_addr);
int exynos_dp_read_byte_from_i2c(struct exynos_dp *dp_regs,
				 unsigned int device_addr,
				 unsigned int reg_addr, unsigned int *data);
int exynos_dp_read_bytes_from_i2c(struct exynos_dp *dp_regs,
				  unsigned int device_addr,
				  unsigned int reg_addr, unsigned int count,
				  unsigned char edid[]);
void exynos_dp_reset_macro(struct exynos_dp *dp_regs);
void exynos_dp_set_link_bandwidth(struct exynos_dp *dp_regs,
				  unsigned char bwtype);
unsigned char exynos_dp_get_link_bandwidth(struct exynos_dp *dp_regs);
void exynos_dp_set_lane_count(struct exynos_dp *dp_regs, unsigned char count);
unsigned int exynos_dp_get_lane_count(struct exynos_dp *dp_regs);
unsigned char exynos_dp_get_lanex_pre_emphasis(struct exynos_dp *dp_regs,
					       unsigned char lanecnt);
void exynos_dp_set_lane_pre_emphasis(struct exynos_dp *dp_regs,
				     unsigned int level, unsigned char lanecnt);
void exynos_dp_set_lanex_pre_emphasis(struct exynos_dp *dp_regs,
				      unsigned char request_val,
				      unsigned char lanecnt);
void exynos_dp_set_training_pattern(struct exynos_dp *dp_regs,
				    unsigned int pattern);
void exynos_dp_enable_enhanced_mode(struct exynos_dp *dp_regs,
				    unsigned char enable);
void exynos_dp_enable_scrambling(struct exynos_dp *dp_regs,
				 unsigned int enable);
int exynos_dp_init_video(struct exynos_dp *dp_regs);
void exynos_dp_config_video_slave_mode(struct exynos_dp *dp_regs,
				       struct edp_video_info *video_info);
void exynos_dp_set_video_color_format(struct exynos_dp *dp_regs,
				      struct edp_video_info *video_info);
int exynos_dp_config_video_bist(struct exynos_dp *dp_regs,
				struct exynos_dp_priv *priv);
unsigned int exynos_dp_is_slave_video_stream_clock_on(
					struct exynos_dp *dp_regs);
void exynos_dp_set_video_cr_mn(struct exynos_dp *dp_regs, unsigned int type,
			       unsigned int m_value, unsigned int n_value);
void exynos_dp_set_video_timing_mode(struct exynos_dp *dp_regs,
				     unsigned int type);
void exynos_dp_enable_video_master(struct exynos_dp *dp_regs,
				   unsigned int enable);
void exynos_dp_start_video(struct exynos_dp *dp_regs);
unsigned int exynos_dp_is_video_stream_on(struct exynos_dp *dp_regs);

#endif /* _EXYNOS_DP_LOWLEVEL_H */
