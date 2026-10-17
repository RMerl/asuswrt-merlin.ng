/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#ifndef _ASM_ARCH_EDP_H
#define _ASM_ARCH_EDP_H

struct rk3288_edp {
	u8	res0[0x10];
	u32	dp_tx_version;
	u8	res1[0x4];
	u32	func_en_1;
	u32	func_en_2;
	u32	video_ctl_1;
	u32	video_ctl_2;
	u32	video_ctl_3;
	u32	video_ctl_4;
	u8	res2[0xc];
	u32	video_ctl_8;
	u8	res3[0x4];
	u32	video_ctl_10;
	u32	total_line_l;
	u32	total_line_h;
	u32	active_line_l;
	u32	active_line_h;
	u32	v_f_porch;
	u32	vsync;
	u32	v_b_porch;
	u32	total_pixel_l;
	u32	total_pixel_h;
	u32	active_pixel_l;
	u32	active_pixel_h;
	u32	h_f_porch_l;
	u32	h_f_porch_h;
	u32	hsync_l;
	u32	hysnc_h;
	u32	h_b_porch_l;
	u32	h_b_porch_h;
	u32	vid_status;
	u32	total_line_sta_l;
	u32	total_line_sta_h;
	u32	active_line_sta_l;
	u32	active_line_sta_h;
	u32	v_f_porch_sta;
	u32	vsync_sta;
	u32	v_b_porch_sta;
	u32	total_pixel_sta_l;
	u32	total_pixel_sta_h;
	u32	active_pixel_sta_l;
	u32	active_pixel_sta_h;
	u32	h_f_porch_sta_l;
	u32	h_f_porch_sta_h;
	u32	hsync_sta_l;
	u32	hsync_sta_h;
	u32	h_b_porch_sta_l;
	u32	h_b_porch__sta_h;
	u8      res4[0x28];
	u32	pll_reg_1;
	u8	res5[4];
	u32	ssc_reg;
	u8	res6[0xc];
	u32	tx_common;
	u32	tx_common2;
	u8	res7[0x4];
	u32	dp_aux;
	u32	dp_bias;
	u32	dp_test;
	u32	dp_pd;
	u32	dp_reserv1;
	u32	dp_reserv2;
	u8	res8[0x224];
	u32	lane_map;
	u8	res9[0x14];
	u32	analog_ctl_2;
	u8	res10[0x48];
	u32	int_state;
	u32	common_int_sta_1;
	u32	common_int_sta_2;
	u32	common_int_sta_3;
	u32	common_int_sta_4;
	u32	spdif_biphase_int_sta;
	u8	res11[0x4];
	u32	dp_int_sta;
	u32	common_int_mask_1;
	u32	common_int_mask_2;
	u32	common_int_mask_3;
	u32	common_int_mask_4;
	u8	res12[0x08];
	u32	int_sta_mask;
	u32	int_ctl;
	u8	res13[0x200];
	u32	sys_ctl_1;
	u32	sys_ctl_2;
	u32	sys_ctl_3;
	u32	sys_ctl_4;
	u32	dp_vid_ctl;
	u8	res14[0x4];
	u32	dp_aud_ctl;
	u8	res15[0x24];
	u32	pkt_send_ctl;
	u8	res16[0x4];
	u32	dp_hdcp_ctl;
	u8	res17[0x34];
	u32	link_bw_set;
	u32	lane_count_set;
	u32	dp_training_ptn_set;
	u32	ln_link_trn_ctl[4];
	u8	res18[0x4];
	u32	dp_hw_link_training;
	u8	res19[0x1c];
	u32	dp_debug_ctl;
	u32	hpd_deglitch_l;
	u32	hpd_deglitch_h;
	u8	res20[0x14];
	u32	dp_link_debug_ctl;
	u8	res21[0x1c];
	u32	m_vid_0;
	u32	m_vid_1;
	u32	m_vid_2;
	u32	n_vid_0;
	u32	n_vid_1;
	u32	n_vid_2;
	u32	m_vid_mon;
	u8	res22[0x14];
	u32	dp_video_fifo_thrd;
	u8	res23[0x8];
	u32	dp_audio_margin;
	u8	res24[0x20];
	u32	dp_m_cal_ctl;
	u32	m_vid_gen_filter_th;
	u8	res25[0x10];
	u32	m_aud_gen_filter_th;
	u8	res26[0x4];
	u32	aux_ch_sta;
	u32	aux_err_num;
	u32	aux_ch_defer_dtl;
	u32	aux_rx_comm;
	u32	buf_data_ctl;
	u32	aux_ch_ctl_1;
	u32	aux_addr_7_0;
	u32	aux_addr_15_8;
	u32	aux_addr_19_16;
	u32	aux_ch_ctl_2;
	u8	res27[0x18];
	u32	buf_data[16];
	u32	soc_general_ctl;
	u8	res29[0x1e0];
	u32	pll_reg_2;
	u32	pll_reg_3;
	u32	pll_reg_4;
	u8	res30[0x10];
	u32	pll_reg_5;
};
check_member(rk3288_edp, pll_reg_5, 0xa00);

/* func_en_1 */
#define VID_CAP_FUNC_EN_N			(0x1 << 6)
#define VID_FIFO_FUNC_EN_N			(0x1 << 5)
#define AUD_FIFO_FUNC_EN_N			(0x1 << 4)
#define AUD_FUNC_EN_N				(0x1 << 3)
#define HDCP_FUNC_EN_N				(0x1 << 2)
#define SW_FUNC_EN_N				(0x1 << 0)

/* func_en_2 */
#define SSC_FUNC_EN_N				(0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N			(0x1 << 0)

/* video_ctl_1 */
#define VIDEO_EN				(0x1 << 7)
#define VIDEO_MUTE				(0x1 << 6)

/* video_ctl_2 */
#define IN_D_RANGE_MASK				(0x1 << 7)
#define IN_D_RANGE_SHIFT			(7)
#define IN_D_RANGE_CEA				(0x1 << 7)
#define IN_D_RANGE_VESA				(0x0 << 7)
#define IN_BPC_MASK				(0x7 << 4)
#define IN_BPC_SHIFT				(4)
#define IN_BPC_12_BITS				(0x3 << 4)
#define IN_BPC_10_BITS				(0x2 << 4)
#define IN_BPC_8_BITS				(0x1 << 4)
#define IN_BPC_6_BITS				(0x0 << 4)
#define IN_COLOR_F_MASK				(0x3 << 0)
#define IN_COLOR_F_SHIFT			(0)
#define IN_COLOR_F_YCBCR444			(0x2 << 0)
#define IN_COLOR_F_YCBCR422			(0x1 << 0)
#define IN_COLOR_F_RGB				(0x0 << 0)

/* video_ctl_3 */
#define IN_YC_COEFFI_MASK			(0x1 << 7)
#define IN_YC_COEFFI_SHIFT			(7)
#define IN_YC_COEFFI_ITU709			(0x1 << 7)
#define IN_YC_COEFFI_ITU601			(0x0 << 7)
#define VID_CHK_UPDATE_TYPE_MASK		(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_SHIFT		(4)
#define VID_CHK_UPDATE_TYPE_1			(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_0			(0x0 << 4)

/* video_ctl_4 */
#define BIST_EN					(0x1 << 3)
#define BIST_WH_64				(0x1 << 2)
#define BIST_WH_32				(0x0 << 2)
#define BIST_TYPE_COLR_BAR			(0x0 << 0)
#define BIST_TYPE_GRAY_BAR			(0x1 << 0)
#define BIST_TYPE_MOBILE_BAR			(0x2 << 0)

/* video_ctl_8 */
#define VID_HRES_TH(x)				(((x) & 0xf) << 4)
#define VID_VRES_TH(x)				(((x) & 0xf) << 0)

/* video_ctl_10 */
#define F_SEL					(0x1 << 4)
#define INTERACE_SCAN_CFG			(0x1 << 2)
#define INTERACD_SCAN_CFG_OFFSET		2
#define VSYNC_POLARITY_CFG			(0x1 << 1)
#define VSYNC_POLARITY_CFG_OFFSET		1
#define HSYNC_POLARITY_CFG			(0x1 << 0)
#define HSYNC_POLARITY_CFG_OFFSET		0

/* dp_pd */
#define PD_INC_BG				(0x1 << 7)
#define PD_EXP_BG				(0x1 << 6)
#define PD_AUX					(0x1 << 5)
#define PD_PLL					(0x1 << 4)
#define PD_CH3					(0x1 << 3)
#define PD_CH2					(0x1 << 2)
#define PD_CH1					(0x1 << 1)
#define PD_CH0					(0x1 << 0)

/* pll_reg_1 */
#define REF_CLK_24M				(0x1 << 0)
#define REF_CLK_27M				(0x0 << 0)
#define REF_CLK_MASK				(0x1 << 0)

/* line_map */
#define LANE3_MAP_LOGIC_LANE_0			(0x0 << 6)
#define LANE3_MAP_LOGIC_LANE_1			(0x1 << 6)
#define LANE3_MAP_LOGIC_LANE_2			(0x2 << 6)
#define LANE3_MAP_LOGIC_LANE_3			(0x3 << 6)
#define LANE2_MAP_LOGIC_LANE_0			(0x0 << 4)
#define LANE2_MAP_LOGIC_LANE_1			(0x1 << 4)
#define LANE2_MAP_LOGIC_LANE_2			(0x2 << 4)
#define LANE2_MAP_LOGIC_LANE_3			(0x3 << 4)
#define LANE1_MAP_LOGIC_LANE_0			(0x0 << 2)
#define LANE1_MAP_LOGIC_LANE_1			(0x1 << 2)
#define LANE1_MAP_LOGIC_LANE_2			(0x2 << 2)
#define LANE1_MAP_LOGIC_LANE_3			(0x3 << 2)
#define LANE0_MAP_LOGIC_LANE_0			(0x0 << 0)
#define LANE0_MAP_LOGIC_LANE_1			(0x1 << 0)
#define LANE0_MAP_LOGIC_LANE_2			(0x2 << 0)
#define LANE0_MAP_LOGIC_LANE_3			(0x3 << 0)

/* analog_ctl_2 */
#define SEL_24M					(0x1 << 3)

/* common_int_sta_1 */
#define VSYNC_DET				(0x1 << 7)
#define PLL_LOCK_CHG				(0x1 << 6)
#define SPDIF_ERR				(0x1 << 5)
#define SPDIF_UNSTBL				(0x1 << 4)
#define VID_FORMAT_CHG				(0x1 << 3)
#define AUD_CLK_CHG				(0x1 << 2)
#define VID_CLK_CHG				(0x1 << 1)
#define SW_INT					(0x1 << 0)

/* common_int_sta_2 */
#define ENC_EN_CHG				(0x1 << 6)
#define HW_BKSV_RDY				(0x1 << 3)
#define HW_SHA_DONE				(0x1 << 2)
#define HW_AUTH_STATE_CHG			(0x1 << 1)
#define HW_AUTH_DONE				(0x1 << 0)

/* common_int_sta_3 */
#define AFIFO_UNDER				(0x1 << 7)
#define AFIFO_OVER				(0x1 << 6)
#define R0_CHK_FLAG				(0x1 << 5)

/* common_int_sta_4 */
#define PSR_ACTIVE				(0x1 << 7)
#define PSR_INACTIVE				(0x1 << 6)
#define SPDIF_BI_PHASE_ERR			(0x1 << 5)
#define HOTPLUG_CHG				(0x1 << 2)
#define HPD_LOST				(0x1 << 1)
#define PLUG					(0x1 << 0)

/* dp_int_sta */
#define INT_HPD					(0x1 << 6)
#define HW_LT_DONE				(0x1 << 5)
#define SINK_LOST				(0x1 << 3)
#define LINK_LOST				(0x1 << 2)
#define RPLY_RECEIV				(0x1 << 1)
#define AUX_ERR					(0x1 << 0)

/* int_ctl */
#define SOFT_INT_CTRL				(0x1 << 2)
#define INT_POL1				(0x1 << 1)
#define INT_POL0				(0x1 << 0)
#define INT_POL					(INT_POL0 | INT_POL1)

/* sys_ctl_1 */
#define DET_STA					(0x1 << 2)
#define FORCE_DET				(0x1 << 1)
#define DET_CTRL				(0x1 << 0)

/* sys_ctl_2 */
#define CHA_CRI(x)				(((x) & 0xf) << 4)
#define CHA_STA					(0x1 << 2)
#define FORCE_CHA				(0x1 << 1)
#define CHA_CTRL				(0x1 << 0)

/* sys_ctl_3 */
#define HPD_STATUS				(0x1 << 6)
#define F_HPD					(0x1 << 5)
#define HPD_CTRL				(0x1 << 4)
#define HDCP_RDY				(0x1 << 3)
#define STRM_VALID				(0x1 << 2)
#define F_VALID					(0x1 << 1)
#define VALID_CTRL				(0x1 << 0)

/* sys_ctl_4 */
#define FIX_M_AUD				(0x1 << 4)
#define ENHANCED				(0x1 << 3)
#define FIX_M_VID				(0x1 << 2)
#define M_VID_UPDATE_CTRL			(0x3 << 0)

/* pll_reg_2 */
#define LDO_OUTPUT_V_SEL_145			(2 << 6)
#define KVCO_DEFALUT				(1 << 4)
#define CHG_PUMP_CUR_SEL_5US			(1 << 2)
#define V2L_CUR_SEL_1MA				(1 << 0)

/* pll_reg_3 */
#define LOCK_DET_CNT_SEL_256			(2 << 5)
#define LOOP_FILTER_RESET			(0 << 4)
#define PALL_SSC_RESET				(0 << 3)
#define LOCK_DET_BYPASS				(0 << 2)
#define PLL_LOCK_DET_MODE			(0 << 1)
#define PLL_LOCK_DET_FORCE			(0 << 0)

/* pll_reg_5 */
#define REGULATOR_V_SEL_950MV			(2 << 4)
#define STANDBY_CUR_SEL				(0 << 3)
#define CHG_PUMP_INOUT_CTRL_1200MV		(1 << 1)
#define CHG_PUMP_INPUT_CTRL_OP			(0 << 0)

/* ssc_reg */
#define SSC_OFFSET				(0 << 6)
#define SSC_MODE				(1 << 4)
#define SSC_DEPTH				(9 << 0)

/* tx_common */
#define TX_SWING_PRE_EMP_MODE			(1 << 7)
#define PRE_DRIVER_PW_CTRL1			(0 << 5)
#define LP_MODE_CLK_REGULATOR			(0 << 4)
#define RESISTOR_MSB_CTRL			(0 << 3)
#define RESISTOR_CTRL				(7 << 0)

/* dp_aux */
#define DP_AUX_COMMON_MODE			(0 << 4)
#define DP_AUX_EN				(0 << 3)
#define AUX_TERM_50OHM				(3 << 0)

/* dp_bias */
#define DP_BG_OUT_SEL				(4 << 4)
#define DP_DB_CUR_CTRL				(0 << 3)
#define DP_BG_SEL				(1 << 2)
#define DP_RESISTOR_TUNE_BG			(2 << 0)

/* dp_reserv2 */
#define CH1_CH3_SWING_EMP_CTRL			(5 << 4)
#define CH0_CH2_SWING_EMP_CTRL			(5 << 0)

/* dp_training_ptn_set */
#define SCRAMBLING_DISABLE			(0x1 << 5)
#define SCRAMBLING_ENABLE			(0x0 << 5)
#define LINK_QUAL_PATTERN_SET_MASK		(0x7 << 2)
#define LINK_QUAL_PATTERN_SET_HBR2		(0x5 << 2)
#define LINK_QUAL_PATTERN_SET_80BIT		(0x4 << 2)
#define LINK_QUAL_PATTERN_SET_PRBS7		(0x3 << 2)
#define LINK_QUAL_PATTERN_SET_D10_2		(0x1 << 2)
#define LINK_QUAL_PATTERN_SET_DISABLE		(0x0 << 2)
#define SW_TRAINING_PATTERN_SET_MASK		(0x3 << 0)
#define SW_TRAINING_PATTERN_SET_PTN2		(0x2 << 0)
#define SW_TRAINING_PATTERN_SET_PTN1		(0x1 << 0)
#define SW_TRAINING_PATTERN_SET_DISABLE		(0x0 << 0)

/* dp_hw_link_training_ctl */
#define HW_LT_ERR_CODE_MASK			0x70
#define HW_LT_ERR_CODE_SHIFT			4
#define HW_LT_EN				(0x1 << 0)

/* dp_debug_ctl */
#define PLL_LOCK				(0x1 << 4)
#define F_PLL_LOCK				(0x1 << 3)
#define PLL_LOCK_CTRL				(0x1 << 2)
#define POLL_EN					(0x1 << 1)
#define PN_INV					(0x1 << 0)

/* aux_ch_sta */
#define AUX_BUSY				(0x1 << 4)
#define AUX_STATUS_MASK				(0xf << 0)

/* aux_ch_defer_ctl */
#define DEFER_CTRL_EN				(0x1 << 7)
#define DEFER_COUNT(x)				(((x) & 0x7f) << 0)

/* aux_rx_comm */
#define AUX_RX_COMM_I2C_DEFER			(0x2 << 2)
#define AUX_RX_COMM_AUX_DEFER			(0x2 << 0)

/* buffer_data_ctl */
#define BUF_CLR					(0x1 << 7)
#define BUF_HAVE_DATA				(0x1 << 4)
#define BUF_DATA_COUNT(x)			(((x) & 0xf) << 0)

/* aux_ch_ctl_1 */
#define AUX_LENGTH(x)				(((x - 1) & 0xf) << 4)
#define AUX_TX_COMM_MASK			(0xf << 0)
#define AUX_TX_COMM_DP_TRANSACTION		(0x1 << 3)
#define AUX_TX_COMM_I2C_TRANSACTION		(0x0 << 3)
#define AUX_TX_COMM_MOT				(0x1 << 2)
#define AUX_TX_COMM_WRITE			(0x0 << 0)
#define AUX_TX_COMM_READ			(0x1 << 0)

/* aux_ch_ctl_2 */
#define PD_AUX_IDLE				(0x1 << 3)
#define ADDR_ONLY				(0x1 << 1)
#define AUX_EN					(0x1 << 0)

/* tx_sw_reset */
#define RST_DP_TX				(0x1 << 0)

/* analog_ctl_1 */
#define TX_TERMINAL_CTRL_50_OHM			(0x1 << 4)

/* analog_ctl_3 */
#define DRIVE_DVDD_BIT_1_0625V			(0x4 << 5)
#define VCO_BIT_600_MICRO			(0x5 << 0)

/* pll_filter_ctl_1 */
#define PD_RING_OSC				(0x1 << 6)
#define AUX_TERMINAL_CTRL_37_5_OHM		(0x0 << 4)
#define AUX_TERMINAL_CTRL_45_OHM		(0x1 << 4)
#define AUX_TERMINAL_CTRL_50_OHM		(0x2 << 4)
#define AUX_TERMINAL_CTRL_65_OHM		(0x3 << 4)
#define TX_CUR1_2X				(0x1 << 2)
#define TX_CUR_16_MA				(0x3 << 0)

/* Definition for DPCD Register */
#define DPCD_DPCD_REV				(0x0000)
#define DPCD_MAX_LINK_RATE			(0x0001)
#define DPCD_MAX_LANE_COUNT			(0x0002)
#define DP_MAX_LANE_COUNT_MASK			0x1f
#define DP_TPS3_SUPPORTED			(1 << 6)
#define DP_ENHANCED_FRAME_CAP			(1 << 7)

#define DPCD_LINK_BW_SET			(0x0100)
#define DPCD_LANE_COUNT_SET			(0x0101)

#define DPCD_TRAINING_PATTERN_SET		(0x0102)
#define DP_TRAINING_PATTERN_DISABLE		0
#define DP_TRAINING_PATTERN_1			1
#define DP_TRAINING_PATTERN_2			2
#define DP_TRAINING_PATTERN_3			3
#define DP_TRAINING_PATTERN_MASK		0x3

#define DPCD_TRAINING_LANE0_SET			(0x0103)
#define DP_TRAIN_VOLTAGE_SWING_MASK		0x3
#define DP_TRAIN_VOLTAGE_SWING_SHIFT		0
#define DP_TRAIN_MAX_SWING_REACHED		(1 << 2)
#define DP_TRAIN_VOLTAGE_SWING_400		(0 << 0)
#define DP_TRAIN_VOLTAGE_SWING_600		(1 << 0)
#define DP_TRAIN_VOLTAGE_SWING_800		(2 << 0)
#define DP_TRAIN_VOLTAGE_SWING_1200		(3 << 0)

#define DP_TRAIN_PRE_EMPHASIS_MASK		(3 << 3)
#define DP_TRAIN_PRE_EMPHASIS_0			(0 << 3)
#define DP_TRAIN_PRE_EMPHASIS_3_5		(1 << 3)
#define DP_TRAIN_PRE_EMPHASIS_6			(2 << 3)
#define DP_TRAIN_PRE_EMPHASIS_9_5		(3 << 3)

#define DP_TRAIN_PRE_EMPHASIS_SHIFT		3
#define DP_TRAIN_MAX_PRE_EMPHASIS_REACHED	(1 << 5)

#define DPCD_LANE0_1_STATUS			(0x0202)
#define DPCD_LANE2_3_STATUS			(0x0203)
#define DP_LANE_CR_DONE				(1 << 0)
#define DP_LANE_CHANNEL_EQ_DONE			(1 << 1)
#define DP_LANE_SYMBOL_LOCKED			(1 << 2)
#define DP_CHANNEL_EQ_BITS			(DP_LANE_CR_DONE |\
						DP_LANE_CHANNEL_EQ_DONE |\
						DP_LANE_SYMBOL_LOCKED)

#define DPCD_LANE_ALIGN_STATUS_UPDATED		(0x0204)
#define DP_INTERLANE_ALIGN_DONE			(1 << 0)
#define DP_DOWNSTREAM_PORT_STATUS_CHANGED	(1 << 6)
#define DP_LINK_STATUS_UPDATED			(1 << 7)

#define DPCD_ADJUST_REQUEST_LANE0_1		(0x0206)
#define DPCD_ADJUST_REQUEST_LANE2_3		(0x0207)
#define DP_ADJUST_VOLTAGE_SWING_LANE0_MASK	0x03
#define DP_ADJUST_VOLTAGE_SWING_LANE0_SHIFT	0
#define DP_ADJUST_PRE_EMPHASIS_LANE0_MASK	0x0c
#define DP_ADJUST_PRE_EMPHASIS_LANE0_SHIFT	2
#define DP_ADJUST_VOLTAGE_SWING_LANE1_MASK	0x30
#define DP_ADJUST_VOLTAGE_SWING_LANE1_SHIFT	4
#define DP_ADJUST_PRE_EMPHASIS_LANE1_MASK	0xc0
#define DP_ADJUST_PRE_EMPHASIS_LANE1_SHIFT	6

#define DPCD_TEST_REQUEST			(0x0218)
#define DPCD_TEST_RESPONSE			(0x0260)
#define DPCD_TEST_EDID_CHECKSUM			(0x0261)
#define DPCD_LINK_POWER_STATE			(0x0600)
#define DP_SET_POWER_D0				0x1
#define DP_SET_POWER_D3				0x2
#define DP_SET_POWER_MASK			0x3

#define AUX_ADDR_7_0(x)				(((x) >> 0) & 0xff)
#define AUX_ADDR_15_8(x)			(((x) >> 8) & 0xff)
#define AUX_ADDR_19_16(x)			(((x) >> 16) & 0x0f)

#define STREAM_ON_TIMEOUT 100
#define PLL_LOCK_TIMEOUT 10
#define DP_INIT_TRIES 10

#define EDID_ADDR				0x50
#define EDID_LENGTH				0x80
#define EDID_HEADER				0x00
#define EDID_EXTENSION_FLAG			0x7e


enum dpcd_request {
	DPCD_READ,
	DPCD_WRITE,
};

enum dp_irq_type {
	DP_IRQ_TYPE_HP_CABLE_IN,
	DP_IRQ_TYPE_HP_CABLE_OUT,
	DP_IRQ_TYPE_HP_CHANGE,
	DP_IRQ_TYPE_UNKNOWN,
};

enum color_coefficient {
	COLOR_YCBCR601,
	COLOR_YCBCR709
};

enum dynamic_range {
	VESA,
	CEA
};

enum clock_recovery_m_value_type {
	CALCULATED_M,
	REGISTER_M
};

enum video_timing_recognition_type {
	VIDEO_TIMING_FROM_CAPTURE,
	VIDEO_TIMING_FROM_REGISTER
};

enum pattern_set {
	PRBS7,
	D10_2,
	TRAINING_PTN1,
	TRAINING_PTN2,
	DP_NONE
};

enum color_space {
	CS_RGB,
	CS_YCBCR422,
	CS_YCBCR444
};

enum color_depth {
	COLOR_6,
	COLOR_8,
	COLOR_10,
	COLOR_12
};

enum link_rate_type {
	LINK_RATE_1_62GBPS = 0x06,
	LINK_RATE_2_70GBPS = 0x0a
};

enum link_lane_count_type {
	LANE_CNT1 = 1,
	LANE_CNT2 = 2,
	LANE_CNT4 = 4
};

enum link_training_state {
	LT_START,
	LT_CLK_RECOVERY,
	LT_EQ_TRAINING,
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

enum analog_power_block {
	AUX_BLOCK,
	CH0_BLOCK,
	CH1_BLOCK,
	CH2_BLOCK,
	CH3_BLOCK,
	ANALOG_TOTAL,
	POWER_ALL
};

struct link_train {
	unsigned char revision;
	u8 link_rate;
	u8 lane_count;
};

#endif
