// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <common.h>
#include <display.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <video_bridge.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/dp.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>

#include "exynos_dp_lowlevel.h"

DECLARE_GLOBAL_DATA_PTR;

static void exynos_dp_disp_info(struct edp_disp_info *disp_info)
{
	disp_info->h_total = disp_info->h_res + disp_info->h_sync_width +
		disp_info->h_back_porch + disp_info->h_front_porch;
	disp_info->v_total = disp_info->v_res + disp_info->v_sync_width +
		disp_info->v_back_porch + disp_info->v_front_porch;

	return;
}

static int exynos_dp_init_dp(struct exynos_dp *regs)
{
	int ret;
	exynos_dp_reset(regs);

	/* SW defined function Normal operation */
	exynos_dp_enable_sw_func(regs, DP_ENABLE);

	ret = exynos_dp_init_analog_func(regs);
	if (ret != EXYNOS_DP_SUCCESS)
		return ret;

	exynos_dp_init_hpd(regs);
	exynos_dp_init_aux(regs);

	return ret;
}

static unsigned char exynos_dp_calc_edid_check_sum(unsigned char *edid_data)
{
	int i;
	unsigned char sum = 0;

	for (i = 0; i < EDID_BLOCK_LENGTH; i++)
		sum = sum + edid_data[i];

	return sum;
}

static unsigned int exynos_dp_read_edid(struct exynos_dp *regs)
{
	unsigned char edid[EDID_BLOCK_LENGTH * 2];
	unsigned int extend_block = 0;
	unsigned char sum;
	unsigned char test_vector;
	int retval;

	/*
	 * EDID device address is 0x50.
	 * However, if necessary, you must have set upper address
	 * into E-EDID in I2C device, 0x30.
	 */

	/* Read Extension Flag, Number of 128-byte EDID extension blocks */
	exynos_dp_read_byte_from_i2c(regs, I2C_EDID_DEVICE_ADDR,
				     EDID_EXTENSION_FLAG, &extend_block);

	if (extend_block > 0) {
		printf("DP EDID data includes a single extension!\n");

		/* Read EDID data */
		retval = exynos_dp_read_bytes_from_i2c(regs,
						I2C_EDID_DEVICE_ADDR,
						EDID_HEADER_PATTERN,
						EDID_BLOCK_LENGTH,
						&edid[EDID_HEADER_PATTERN]);
		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(edid);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		/* Read additional EDID data */
		retval = exynos_dp_read_bytes_from_i2c(regs,
				I2C_EDID_DEVICE_ADDR,
				EDID_BLOCK_LENGTH,
				EDID_BLOCK_LENGTH,
				&edid[EDID_BLOCK_LENGTH]);
		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(&edid[EDID_BLOCK_LENGTH]);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		exynos_dp_read_byte_from_dpcd(regs, DPCD_TEST_REQUEST,
					      &test_vector);
		if (test_vector & DPCD_TEST_EDID_READ) {
			exynos_dp_write_byte_to_dpcd(regs,
				DPCD_TEST_EDID_CHECKSUM,
				edid[EDID_BLOCK_LENGTH + EDID_CHECKSUM]);
			exynos_dp_write_byte_to_dpcd(regs,
				DPCD_TEST_RESPONSE,
				DPCD_TEST_EDID_CHECKSUM_WRITE);
		}
	} else {
		debug("DP EDID data does not include any extensions.\n");

		/* Read EDID data */
		retval = exynos_dp_read_bytes_from_i2c(regs,
				I2C_EDID_DEVICE_ADDR,
				EDID_HEADER_PATTERN,
				EDID_BLOCK_LENGTH,
				&edid[EDID_HEADER_PATTERN]);

		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(edid);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		exynos_dp_read_byte_from_dpcd(regs, DPCD_TEST_REQUEST,
			&test_vector);
		if (test_vector & DPCD_TEST_EDID_READ) {
			exynos_dp_write_byte_to_dpcd(regs,
				DPCD_TEST_EDID_CHECKSUM, edid[EDID_CHECKSUM]);
			exynos_dp_write_byte_to_dpcd(regs,
				DPCD_TEST_RESPONSE,
				DPCD_TEST_EDID_CHECKSUM_WRITE);
		}
	}

	debug("DP EDID Read success!\n");

	return 0;
}

static unsigned int exynos_dp_handle_edid(struct exynos_dp *regs,
					  struct exynos_dp_priv *priv)
{
	unsigned char buf[12];
	unsigned int ret;
	unsigned char temp;
	unsigned char retry_cnt;
	unsigned char dpcd_rev[16];
	unsigned char lane_bw[16];
	unsigned char lane_cnt[16];

	memset(dpcd_rev, 0, 16);
	memset(lane_bw, 0, 16);
	memset(lane_cnt, 0, 16);
	memset(buf, 0, 12);

	retry_cnt = 5;
	while (retry_cnt) {
		/* Read DPCD 0x0000-0x000b */
		ret = exynos_dp_read_bytes_from_dpcd(regs, DPCD_DPCD_REV, 12,
						     buf);
		if (ret != EXYNOS_DP_SUCCESS) {
			if (retry_cnt == 0) {
				printf("DP read_byte_from_dpcd() failed\n");
				return ret;
			}
			retry_cnt--;
		} else
			break;
	}

	/* */
	temp = buf[DPCD_DPCD_REV];
	if (temp == DP_DPCD_REV_10 || temp == DP_DPCD_REV_11)
		priv->dpcd_rev = temp;
	else {
		printf("DP Wrong DPCD Rev : %x\n", temp);
		return -ENODEV;
	}

	temp = buf[DPCD_MAX_LINK_RATE];
	if (temp == DP_LANE_BW_1_62 || temp == DP_LANE_BW_2_70)
		priv->lane_bw = temp;
	else {
		printf("DP Wrong MAX LINK RATE : %x\n", temp);
		return -EINVAL;
	}

	/* Refer VESA Display Port Standard Ver1.1a Page 120 */
	if (priv->dpcd_rev == DP_DPCD_REV_11) {
		temp = buf[DPCD_MAX_LANE_COUNT] & 0x1f;
		if (buf[DPCD_MAX_LANE_COUNT] & 0x80)
			priv->dpcd_efc = 1;
		else
			priv->dpcd_efc = 0;
	} else {
		temp = buf[DPCD_MAX_LANE_COUNT];
		priv->dpcd_efc = 0;
	}

	if (temp == DP_LANE_CNT_1 || temp == DP_LANE_CNT_2 ||
			temp == DP_LANE_CNT_4) {
		priv->lane_cnt = temp;
	} else {
		printf("DP Wrong MAX LANE COUNT : %x\n", temp);
		return -EINVAL;
	}

	ret = exynos_dp_read_edid(regs);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP exynos_dp_read_edid() failed\n");
		return -EINVAL;
	}

	return ret;
}

static void exynos_dp_init_training(struct exynos_dp *regs)
{
	/*
	 * MACRO_RST must be applied after the PLL_LOCK to avoid
	 * the DP inter pair skew issue for at least 10 us
	 */
	exynos_dp_reset_macro(regs);

	/* All DP analog module power up */
	exynos_dp_set_analog_power_down(regs, POWER_ALL, 0);
}

static unsigned int exynos_dp_link_start(struct exynos_dp *regs,
					 struct exynos_dp_priv *priv)
{
	unsigned char buf[5];
	unsigned int ret = 0;

	debug("DP: %s was called\n", __func__);

	priv->lt_info.lt_status = DP_LT_CR;
	priv->lt_info.ep_loop = 0;
	priv->lt_info.cr_loop[0] = 0;
	priv->lt_info.cr_loop[1] = 0;
	priv->lt_info.cr_loop[2] = 0;
	priv->lt_info.cr_loop[3] = 0;

		/* Set sink to D0 (Sink Not Ready) mode. */
	ret = exynos_dp_write_byte_to_dpcd(regs, DPCD_SINK_POWER_STATE,
					   DPCD_SET_POWER_STATE_D0);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	/* Set link rate and count as you want to establish */
	exynos_dp_set_link_bandwidth(regs, priv->lane_bw);
	exynos_dp_set_lane_count(regs, priv->lane_cnt);

	/* Setup RX configuration */
	buf[0] = priv->lane_bw;
	buf[1] = priv->lane_cnt;

	ret = exynos_dp_write_bytes_to_dpcd(regs, DPCD_LINK_BW_SET, 2, buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	exynos_dp_set_lane_pre_emphasis(regs, PRE_EMPHASIS_LEVEL_0,
			priv->lane_cnt);

	/* Set training pattern 1 */
	exynos_dp_set_training_pattern(regs, TRAINING_PTN1);

	/* Set RX training pattern */
	buf[0] = DPCD_SCRAMBLING_DISABLED | DPCD_TRAINING_PATTERN_1;

	buf[1] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[2] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[3] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[4] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;

	ret = exynos_dp_write_bytes_to_dpcd(regs, DPCD_TRAINING_PATTERN_SET,
					    5, buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	return ret;
}

static unsigned int exynos_dp_training_pattern_dis(struct exynos_dp *regs)
{
	unsigned int ret;

	exynos_dp_set_training_pattern(regs, DP_NONE);

	ret = exynos_dp_write_byte_to_dpcd(regs, DPCD_TRAINING_PATTERN_SET,
					   DPCD_TRAINING_PATTERN_DISABLED);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP request_link_training_req failed\n");
		return -EAGAIN;
	}

	return ret;
}

static unsigned int exynos_dp_enable_rx_to_enhanced_mode(
		struct exynos_dp *regs, unsigned char enable)
{
	unsigned char data;
	unsigned int ret;

	ret = exynos_dp_read_byte_from_dpcd(regs, DPCD_LANE_COUNT_SET,
					    &data);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read_from_dpcd failed\n");
		return -EAGAIN;
	}

	if (enable)
		data = DPCD_ENHANCED_FRAME_EN | DPCD_LN_COUNT_SET(data);
	else
		data = DPCD_LN_COUNT_SET(data);

	ret = exynos_dp_write_byte_to_dpcd(regs, DPCD_LANE_COUNT_SET, data);
	if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write_to_dpcd failed\n");
			return -EAGAIN;

	}

	return ret;
}

static unsigned int exynos_dp_set_enhanced_mode(struct exynos_dp *regs,
						unsigned char enhance_mode)
{
	unsigned int ret;

	ret = exynos_dp_enable_rx_to_enhanced_mode(regs, enhance_mode);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP rx_enhance_mode failed\n");
		return -EAGAIN;
	}

	exynos_dp_enable_enhanced_mode(regs, enhance_mode);

	return ret;
}

static int exynos_dp_read_dpcd_lane_stat(struct exynos_dp *regs,
					 struct exynos_dp_priv *priv,
					 unsigned char *status)
{
	unsigned int ret, i;
	unsigned char buf[2];
	unsigned char lane_stat[DP_LANE_CNT_4] = {0,};
	unsigned char shift_val[DP_LANE_CNT_4] = {0,};

	shift_val[0] = 0;
	shift_val[1] = 4;
	shift_val[2] = 0;
	shift_val[3] = 4;

	ret = exynos_dp_read_bytes_from_dpcd(regs, DPCD_LANE0_1_STATUS, 2,
					     buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read lane status failed\n");
		return ret;
	}

	for (i = 0; i < priv->lane_cnt; i++) {
		lane_stat[i] = (buf[(i / 2)] >> shift_val[i]) & 0x0f;
		if (lane_stat[0] != lane_stat[i]) {
			printf("Wrong lane status\n");
			return -EINVAL;
		}
	}

	*status = lane_stat[0];

	return ret;
}

static unsigned int exynos_dp_read_dpcd_adj_req(struct exynos_dp *regs,
		unsigned char lane_num, unsigned char *sw, unsigned char *em)
{
	unsigned int ret;
	unsigned char buf;
	unsigned int dpcd_addr;
	unsigned char shift_val[DP_LANE_CNT_4] = {0, 4, 0, 4};

	/* lane_num value is used as array index, so this range 0 ~ 3 */
	dpcd_addr = DPCD_ADJUST_REQUEST_LANE0_1 + (lane_num / 2);

	ret = exynos_dp_read_byte_from_dpcd(regs, dpcd_addr, &buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read adjust request failed\n");
		return -EAGAIN;
	}

	*sw = ((buf >> shift_val[lane_num]) & 0x03);
	*em = ((buf >> shift_val[lane_num]) & 0x0c) >> 2;

	return ret;
}

static int exynos_dp_equalizer_err_link(struct exynos_dp *regs,
					struct exynos_dp_priv *priv)
{
	int ret;

	ret = exynos_dp_training_pattern_dis(regs);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP training_pattern_disable() failed\n");
		priv->lt_info.lt_status = DP_LT_FAIL;
	}

	ret = exynos_dp_set_enhanced_mode(regs, priv->dpcd_efc);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP set_enhanced_mode() failed\n");
		priv->lt_info.lt_status = DP_LT_FAIL;
	}

	return ret;
}

static int exynos_dp_reduce_link_rate(struct exynos_dp *regs,
				      struct exynos_dp_priv *priv)
{
	int ret;

	if (priv->lane_bw == DP_LANE_BW_2_70) {
		priv->lane_bw = DP_LANE_BW_1_62;
		printf("DP Change lane bw to 1.62Gbps\n");
		priv->lt_info.lt_status = DP_LT_START;
		ret = EXYNOS_DP_SUCCESS;
	} else {
		ret = exynos_dp_training_pattern_dis(regs);
		if (ret != EXYNOS_DP_SUCCESS)
			printf("DP training_patter_disable() failed\n");

		ret = exynos_dp_set_enhanced_mode(regs, priv->dpcd_efc);
		if (ret != EXYNOS_DP_SUCCESS)
			printf("DP set_enhanced_mode() failed\n");

		priv->lt_info.lt_status = DP_LT_FAIL;
	}

	return ret;
}

static unsigned int exynos_dp_process_clock_recovery(struct exynos_dp *regs,
					struct exynos_dp_priv *priv)
{
	unsigned int ret;
	unsigned char lane_stat;
	unsigned char lt_ctl_val[DP_LANE_CNT_4] = {0, };
	unsigned int i;
	unsigned char adj_req_sw;
	unsigned char adj_req_em;
	unsigned char buf[5];

	debug("DP: %s was called\n", __func__);
	mdelay(1);

	ret = exynos_dp_read_dpcd_lane_stat(regs, priv, &lane_stat);
	if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP read lane status failed\n");
			priv->lt_info.lt_status = DP_LT_FAIL;
			return ret;
	}

	if (lane_stat & DP_LANE_STAT_CR_DONE) {
		debug("DP clock Recovery training succeed\n");
		exynos_dp_set_training_pattern(regs, TRAINING_PTN2);

		for (i = 0; i < priv->lane_cnt; i++) {
			ret = exynos_dp_read_dpcd_adj_req(regs, i,
						&adj_req_sw, &adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				priv->lt_info.lt_status = DP_LT_FAIL;
				return ret;
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3)
				|| (adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3 |
					MAX_PRE_EMPHASIS_REACH_3;
			}
			exynos_dp_set_lanex_pre_emphasis(regs,
							 lt_ctl_val[i], i);
		}

		buf[0] =  DPCD_SCRAMBLING_DISABLED | DPCD_TRAINING_PATTERN_2;
		buf[1] = lt_ctl_val[0];
		buf[2] = lt_ctl_val[1];
		buf[3] = lt_ctl_val[2];
		buf[4] = lt_ctl_val[3];

		ret = exynos_dp_write_bytes_to_dpcd(regs,
				DPCD_TRAINING_PATTERN_SET, 5, buf);
		if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write training pattern1 failed\n");
			priv->lt_info.lt_status = DP_LT_FAIL;
			return ret;
		} else
			priv->lt_info.lt_status = DP_LT_ET;
	} else {
		for (i = 0; i < priv->lane_cnt; i++) {
			lt_ctl_val[i] = exynos_dp_get_lanex_pre_emphasis(
						regs, i);
				ret = exynos_dp_read_dpcd_adj_req(regs, i,
						&adj_req_sw, &adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP read adj req failed\n");
				priv->lt_info.lt_status = DP_LT_FAIL;
				return ret;
			}

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
					(adj_req_em == PRE_EMPHASIS_LEVEL_3))
				ret = exynos_dp_reduce_link_rate(regs,
								 priv);

			if ((DRIVE_CURRENT_SET_0_GET(lt_ctl_val[i]) ==
						adj_req_sw) &&
				(PRE_EMPHASIS_SET_0_GET(lt_ctl_val[i]) ==
						adj_req_em)) {
				priv->lt_info.cr_loop[i]++;
				if (priv->lt_info.cr_loop[i] == MAX_CR_LOOP)
					ret = exynos_dp_reduce_link_rate(
							regs, priv);
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
					(adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3 |
					MAX_PRE_EMPHASIS_REACH_3;
			}
			exynos_dp_set_lanex_pre_emphasis(regs,
							 lt_ctl_val[i], i);
		}

		ret = exynos_dp_write_bytes_to_dpcd(regs,
				DPCD_TRAINING_LANE0_SET, 4, lt_ctl_val);
		if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write training pattern2 failed\n");
			priv->lt_info.lt_status = DP_LT_FAIL;
			return ret;
		}
	}

	return ret;
}

static unsigned int exynos_dp_process_equalizer_training(
		struct exynos_dp *regs, struct exynos_dp_priv *priv)
{
	unsigned int ret;
	unsigned char lane_stat, adj_req_sw, adj_req_em, i;
	unsigned char lt_ctl_val[DP_LANE_CNT_4] = {0,};
	unsigned char interlane_aligned = 0;
	unsigned char f_bw;
	unsigned char f_lane_cnt;
	unsigned char sink_stat;

	mdelay(1);

	ret = exynos_dp_read_dpcd_lane_stat(regs, priv, &lane_stat);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read lane status failed\n");
		priv->lt_info.lt_status = DP_LT_FAIL;
		return ret;
	}

	debug("DP lane stat : %x\n", lane_stat);

	if (lane_stat & DP_LANE_STAT_CR_DONE) {
		ret = exynos_dp_read_byte_from_dpcd(regs,
						    DPCD_LN_ALIGN_UPDATED,
						    &sink_stat);
		if (ret != EXYNOS_DP_SUCCESS) {
			priv->lt_info.lt_status = DP_LT_FAIL;

			return ret;
		}

		interlane_aligned = (sink_stat & DPCD_INTERLANE_ALIGN_DONE);

		for (i = 0; i < priv->lane_cnt; i++) {
			ret = exynos_dp_read_dpcd_adj_req(regs, i,
					&adj_req_sw, &adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP read adj req 1 failed\n");
				priv->lt_info.lt_status = DP_LT_FAIL;

				return ret;
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
				(adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3;
				lt_ctl_val[i] |= MAX_PRE_EMPHASIS_REACH_3;
			}
		}

		if (((lane_stat&DP_LANE_STAT_CE_DONE) &&
			(lane_stat&DP_LANE_STAT_SYM_LOCK))
			&& (interlane_aligned == DPCD_INTERLANE_ALIGN_DONE)) {
			debug("DP Equalizer training succeed\n");

			f_bw = exynos_dp_get_link_bandwidth(regs);
			f_lane_cnt = exynos_dp_get_lane_count(regs);

			debug("DP final BandWidth : %x\n", f_bw);
			debug("DP final Lane Count : %x\n", f_lane_cnt);

			priv->lt_info.lt_status = DP_LT_FINISHED;

			exynos_dp_equalizer_err_link(regs, priv);

		} else {
			priv->lt_info.ep_loop++;

			if (priv->lt_info.ep_loop > MAX_EQ_LOOP) {
				if (priv->lane_bw == DP_LANE_BW_2_70) {
					ret = exynos_dp_reduce_link_rate(
							regs, priv);
				} else {
					priv->lt_info.lt_status =
								DP_LT_FAIL;
					exynos_dp_equalizer_err_link(regs,
								     priv);
				}
			} else {
				for (i = 0; i < priv->lane_cnt; i++)
					exynos_dp_set_lanex_pre_emphasis(
						regs, lt_ctl_val[i], i);

				ret = exynos_dp_write_bytes_to_dpcd(regs,
						DPCD_TRAINING_LANE0_SET,
						4, lt_ctl_val);
				if (ret != EXYNOS_DP_SUCCESS) {
					printf("DP set lt pattern failed\n");
					priv->lt_info.lt_status =
								DP_LT_FAIL;
					exynos_dp_equalizer_err_link(regs,
								     priv);
				}
			}
		}
	} else if (priv->lane_bw == DP_LANE_BW_2_70) {
		ret = exynos_dp_reduce_link_rate(regs, priv);
	} else {
		priv->lt_info.lt_status = DP_LT_FAIL;
		exynos_dp_equalizer_err_link(regs, priv);
	}

	return ret;
}

static unsigned int exynos_dp_sw_link_training(struct exynos_dp *regs,
					       struct exynos_dp_priv *priv)
{
	unsigned int ret = 0;
	int training_finished;

	/* Turn off unnecessary lane */
	if (priv->lane_cnt == 1)
		exynos_dp_set_analog_power_down(regs, CH1_BLOCK, 1);

	training_finished = 0;

	priv->lt_info.lt_status = DP_LT_START;

	/* Process here */
	while (!training_finished) {
		switch (priv->lt_info.lt_status) {
		case DP_LT_START:
			ret = exynos_dp_link_start(regs, priv);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:link start failed\n");
				return ret;
			}
			break;
		case DP_LT_CR:
			ret = exynos_dp_process_clock_recovery(regs,
							       priv);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:clock recovery failed\n");
				return ret;
			}
			break;
		case DP_LT_ET:
			ret = exynos_dp_process_equalizer_training(regs,
								   priv);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:equalizer training failed\n");
				return ret;
			}
			break;
		case DP_LT_FINISHED:
			training_finished = 1;
			break;
		case DP_LT_FAIL:
			return -1;
		}
	}

	return ret;
}

static unsigned int exynos_dp_set_link_train(struct exynos_dp *regs,
					     struct exynos_dp_priv *priv)
{
	unsigned int ret;

	exynos_dp_init_training(regs);

	ret = exynos_dp_sw_link_training(regs, priv);
	if (ret != EXYNOS_DP_SUCCESS)
		printf("DP dp_sw_link_training() failed\n");

	return ret;
}

static void exynos_dp_enable_scramble(struct exynos_dp *regs,
				      unsigned int enable)
{
	unsigned char data;

	if (enable) {
		exynos_dp_enable_scrambling(regs, DP_ENABLE);

		exynos_dp_read_byte_from_dpcd(regs,
					      DPCD_TRAINING_PATTERN_SET, &data);
		exynos_dp_write_byte_to_dpcd(regs, DPCD_TRAINING_PATTERN_SET,
			(u8)(data & ~DPCD_SCRAMBLING_DISABLED));
	} else {
		exynos_dp_enable_scrambling(regs, DP_DISABLE);
		exynos_dp_read_byte_from_dpcd(regs,
					      DPCD_TRAINING_PATTERN_SET, &data);
		exynos_dp_write_byte_to_dpcd(regs, DPCD_TRAINING_PATTERN_SET,
			(u8)(data | DPCD_SCRAMBLING_DISABLED));
	}
}

static unsigned int exynos_dp_config_video(struct exynos_dp *regs,
					   struct exynos_dp_priv *priv)
{
	unsigned int ret = 0;
	unsigned int retry_cnt;

	mdelay(1);

	if (priv->video_info.master_mode) {
		printf("DP does not support master mode\n");
		return -ENODEV;
	} else {
		/* debug slave */
		exynos_dp_config_video_slave_mode(regs,
						  &priv->video_info);
	}

	exynos_dp_set_video_color_format(regs, &priv->video_info);

	if (priv->video_info.bist_mode) {
		if (exynos_dp_config_video_bist(regs, priv) != 0)
			return -1;
	}

	ret = exynos_dp_get_pll_lock_status(regs);
	if (ret != PLL_LOCKED) {
		printf("DP PLL is not locked yet\n");
		return -EIO;
	}

	if (priv->video_info.master_mode == 0) {
		retry_cnt = 10;
		while (retry_cnt) {
			ret = exynos_dp_is_slave_video_stream_clock_on(regs);
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP stream_clock_on failed\n");
					return ret;
				}
				retry_cnt--;
				mdelay(1);
			} else
				break;
		}
	}

	/* Set to use the register calculated M/N video */
	exynos_dp_set_video_cr_mn(regs, CALCULATED_M, 0, 0);

	/* For video bist, Video timing must be generated by register */
	exynos_dp_set_video_timing_mode(regs, VIDEO_TIMING_FROM_CAPTURE);

	/* Enable video bist */
	if (priv->video_info.bist_pattern != COLOR_RAMP &&
		priv->video_info.bist_pattern != BALCK_WHITE_V_LINES &&
		priv->video_info.bist_pattern != COLOR_SQUARE)
		exynos_dp_enable_video_bist(regs,
					    priv->video_info.bist_mode);
	else
		exynos_dp_enable_video_bist(regs, DP_DISABLE);

	/* Disable video mute */
	exynos_dp_enable_video_mute(regs, DP_DISABLE);

	/* Configure video Master or Slave mode */
	exynos_dp_enable_video_master(regs,
				      priv->video_info.master_mode);

	/* Enable video */
	exynos_dp_start_video(regs);

	if (priv->video_info.master_mode == 0) {
		retry_cnt = 100;
		while (retry_cnt) {
			ret = exynos_dp_is_video_stream_on(regs);
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP Timeout of video stream\n");
					return ret;
				}
				retry_cnt--;
				mdelay(5);
			} else
				break;
		}
	}

	return ret;
}

static int exynos_dp_ofdata_to_platdata(struct udevice *dev)
{
	struct exynos_dp_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	unsigned int node = dev_of_offset(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get the DP base address\n");
		return -EINVAL;
	}
	priv->regs = (struct exynos_dp *)addr;
	priv->disp_info.h_res = fdtdec_get_int(blob, node,
							"samsung,h-res", 0);
	priv->disp_info.h_sync_width = fdtdec_get_int(blob, node,
						"samsung,h-sync-width", 0);
	priv->disp_info.h_back_porch = fdtdec_get_int(blob, node,
						"samsung,h-back-porch", 0);
	priv->disp_info.h_front_porch = fdtdec_get_int(blob, node,
						"samsung,h-front-porch", 0);
	priv->disp_info.v_res = fdtdec_get_int(blob, node,
						"samsung,v-res", 0);
	priv->disp_info.v_sync_width = fdtdec_get_int(blob, node,
						"samsung,v-sync-width", 0);
	priv->disp_info.v_back_porch = fdtdec_get_int(blob, node,
						"samsung,v-back-porch", 0);
	priv->disp_info.v_front_porch = fdtdec_get_int(blob, node,
						"samsung,v-front-porch", 0);
	priv->disp_info.v_sync_rate = fdtdec_get_int(blob, node,
						"samsung,v-sync-rate", 0);

	priv->lt_info.lt_status = fdtdec_get_int(blob, node,
						"samsung,lt-status", 0);

	priv->video_info.master_mode = fdtdec_get_int(blob, node,
						"samsung,master-mode", 0);
	priv->video_info.bist_mode = fdtdec_get_int(blob, node,
						"samsung,bist-mode", 0);
	priv->video_info.bist_pattern = fdtdec_get_int(blob, node,
						"samsung,bist-pattern", 0);
	priv->video_info.h_sync_polarity = fdtdec_get_int(blob, node,
						"samsung,h-sync-polarity", 0);
	priv->video_info.v_sync_polarity = fdtdec_get_int(blob, node,
						"samsung,v-sync-polarity", 0);
	priv->video_info.interlaced = fdtdec_get_int(blob, node,
						"samsung,interlaced", 0);
	priv->video_info.color_space = fdtdec_get_int(blob, node,
						"samsung,color-space", 0);
	priv->video_info.dynamic_range = fdtdec_get_int(blob, node,
						"samsung,dynamic-range", 0);
	priv->video_info.ycbcr_coeff = fdtdec_get_int(blob, node,
						"samsung,ycbcr-coeff", 0);
	priv->video_info.color_depth = fdtdec_get_int(blob, node,
						"samsung,color-depth", 0);
	return 0;
}

static int exynos_dp_bridge_init(struct udevice *dev)
{
	const int max_tries = 10;
	int num_tries;
	int ret;

	debug("%s\n", __func__);
	ret = video_bridge_attach(dev);
	if (ret) {
		debug("video bridge init failed: %d\n", ret);
		return ret;
	}

	/*
	 * We need to wait for 90ms after bringing up the bridge since there
	 * is a phantom "high" on the HPD chip during its bootup.  The phantom
	 * high comes within 7ms of de-asserting PD and persists for at least
	 * 15ms.  The real high comes roughly 50ms after PD is de-asserted. The
	 * phantom high makes it hard for us to know when the NXP chip is up.
	 */
	mdelay(90);

	for (num_tries = 0; num_tries < max_tries; num_tries++) {
		/* Check HPD. If it's high, or we don't have it, all is well */
		ret = video_bridge_check_attached(dev);
		if (!ret || ret == -ENOENT)
			return 0;

		debug("%s: eDP bridge failed to come up; try %d of %d\n",
		      __func__, num_tries, max_tries);
	}

	/* Immediately go into bridge reset if the hp line is not high */
	return -EIO;
}

static int exynos_dp_bridge_setup(const void *blob)
{
	const int max_tries = 2;
	int num_tries;
	struct udevice *dev;
	int ret;

	/* Configure I2C registers for Parade bridge */
	ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &dev);
	if (ret) {
		debug("video bridge init failed: %d\n", ret);
		return ret;
	}

	if (strncmp(dev->driver->name, "parade", 6)) {
		/* Mux HPHPD to the special hotplug detect mode */
		exynos_pinmux_config(PERIPH_ID_DPHPD, 0);
	}

	for (num_tries = 0; num_tries < max_tries; num_tries++) {
		ret = exynos_dp_bridge_init(dev);
		if (!ret)
			return 0;
		if (num_tries == max_tries - 1)
			break;

		/*
		* If we're here, the bridge chip failed to initialise.
		* Power down the bridge in an attempt to reset.
		*/
		video_bridge_set_active(dev, false);

		/*
		* Arbitrarily wait 300ms here with DP_N low.  Don't know for
		* sure how long we should wait, but we're being paranoid.
		*/
		mdelay(300);
	}

	return ret;
}
int exynos_dp_enable(struct udevice *dev, int panel_bpp,
		     const struct display_timing *timing)
{
	struct exynos_dp_priv *priv = dev_get_priv(dev);
	struct exynos_dp *regs = priv->regs;
	unsigned int ret;

	debug("%s: start\n", __func__);
	exynos_dp_disp_info(&priv->disp_info);

	ret = exynos_dp_bridge_setup(gd->fdt_blob);
	if (ret && ret != -ENODEV)
		printf("LCD bridge failed to enable: %d\n", ret);

	exynos_dp_phy_ctrl(1);

	ret = exynos_dp_init_dp(regs);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP exynos_dp_init_dp() failed\n");
		return ret;
	}

	ret = exynos_dp_handle_edid(regs, priv);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("EDP handle_edid fail\n");
		return ret;
	}

	ret = exynos_dp_set_link_train(regs, priv);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP link training fail\n");
		return ret;
	}

	exynos_dp_enable_scramble(regs, DP_ENABLE);
	exynos_dp_enable_rx_to_enhanced_mode(regs, DP_ENABLE);
	exynos_dp_enable_enhanced_mode(regs, DP_ENABLE);

	exynos_dp_set_link_bandwidth(regs, priv->lane_bw);
	exynos_dp_set_lane_count(regs, priv->lane_cnt);

	exynos_dp_init_video(regs);
	ret = exynos_dp_config_video(regs, priv);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("Exynos DP init failed\n");
		return ret;
	}

	debug("Exynos DP init done\n");

	return ret;
}


static const struct dm_display_ops exynos_dp_ops = {
	.enable = exynos_dp_enable,
};

static const struct udevice_id exynos_dp_ids[] = {
	{ .compatible = "samsung,exynos5-dp" },
	{ }
};

U_BOOT_DRIVER(exynos_dp) = {
	.name	= "exynos_dp",
	.id	= UCLASS_DISPLAY,
	.of_match = exynos_dp_ids,
	.ops	= &exynos_dp_ops,
	.ofdata_to_platdata	= exynos_dp_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct exynos_dp_priv),
};
