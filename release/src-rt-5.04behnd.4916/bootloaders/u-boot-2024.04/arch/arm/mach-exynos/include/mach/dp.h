/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_DP_H_
#define __ASM_ARM_ARCH_DP_H_

#ifndef __ASSEMBLY__

struct exynos_dp {
	unsigned char	res1[0x10];
	unsigned int	tx_version;
	unsigned int	tx_sw_reset;
	unsigned int	func_en1;
	unsigned int	func_en2;
	unsigned int	video_ctl1;
	unsigned int	video_ctl2;
	unsigned int	video_ctl3;
	unsigned int	video_ctl4;
	unsigned int	color_blue_cb;
	unsigned int	color_green_y;
	unsigned int	color_red_cr;
	unsigned int	video_ctl8;
	unsigned char	res2[0x4];
	unsigned int	video_ctl10;
	unsigned int	total_ln_cfg_l;
	unsigned int	total_ln_cfg_h;
	unsigned int	active_ln_cfg_l;
	unsigned int	active_ln_cfg_h;
	unsigned int	vfp_cfg;
	unsigned int	vsw_cfg;
	unsigned int	vbp_cfg;
	unsigned int	total_pix_cfg_l;
	unsigned int	total_pix_cfg_h;
	unsigned int	active_pix_cfg_l;
	unsigned int	active_pix_cfg_h;
	unsigned int	hfp_cfg_l;
	unsigned int	hfp_cfg_h;
	unsigned int	hsw_cfg_l;
	unsigned int	hsw_cfg_h;
	unsigned int	hbp_cfg_l;
	unsigned int	hbp_cfg_h;
	unsigned int	video_status;
	unsigned int	total_ln_sta_l;
	unsigned int	total_ln_sta_h;
	unsigned int	active_ln_sta_l;
	unsigned int	active_ln_sta_h;

	unsigned int	vfp_sta;
	unsigned int	vsw_sta;
	unsigned int	vbp_sta;

	unsigned int	total_pix_sta_l;
	unsigned int	total_pix_sta_h;
	unsigned int	active_pix_sta_l;
	unsigned int	active_pix_sta_h;

	unsigned int	hfp_sta_l;
	unsigned int	hfp_sta_h;
	unsigned int	hsw_sta_l;
	unsigned int	hsw_sta_h;
	unsigned int	hbp_sta_l;
	unsigned int	hbp_sta_h;

	unsigned char	res3[0x288];

	unsigned int	lane_map;
	unsigned char	res4[0x10];
	unsigned int	analog_ctl1;
	unsigned int	analog_ctl2;
	unsigned int	analog_ctl3;

	unsigned int	pll_filter_ctl1;
	unsigned int	amp_tuning_ctl;
	unsigned char	res5[0xc];

	unsigned int	aux_hw_retry_ctl;
	unsigned char	res6[0x2c];
	unsigned int	int_state;
	unsigned int	common_int_sta1;
	unsigned int	common_int_sta2;
	unsigned int	common_int_sta3;
	unsigned int	common_int_sta4;
	unsigned char	res7[0x8];

	unsigned int	int_sta;
	unsigned char	res8[0x1c];
	unsigned int	int_ctl;
	unsigned char	res9[0x200];
	unsigned int	sys_ctl1;
	unsigned int	sys_ctl2;
	unsigned int	sys_ctl3;
	unsigned int	sys_ctl4;
	unsigned int	vid_ctl;
	unsigned char	res10[0x2c];
	unsigned int	pkt_send_ctl;
	unsigned char	res[0x4];
	unsigned int	hdcp_ctl;
	unsigned char	res11[0x34];
	unsigned int	link_bw_set;

	unsigned int	lane_count_set;
	unsigned int	training_ptn_set;
	unsigned int	ln0_link_training_ctl;
	unsigned int	ln1_link_training_ctl;
	unsigned int	ln2_link_training_ctl;
	unsigned int	ln3_link_training_ctl;
	unsigned int	dn_spread_ctl;
	unsigned int	hw_link_training_ctl;
	unsigned char	res12[0x1c];

	unsigned int	debug_ctl;
	unsigned int	hpd_deglitch_l;
	unsigned int	hpd_deglitch_h;

	unsigned char	res13[0x14];
	unsigned int	link_debug_ctl;

	unsigned char	res14[0x1c];

	unsigned int	m_vid0;
	unsigned int	m_vid1;
	unsigned int	m_vid2;
	unsigned int	n_vid0;
	unsigned int	n_vid1;
	unsigned int	n_vid2;
	unsigned int	m_vid_mon;
	unsigned int	pll_ctl;
	unsigned int	phy_pd;
	unsigned int	phy_test;
	unsigned char	res15[0x8];

	unsigned int	video_fifo_thrd;
	unsigned char	res16[0x8];
	unsigned int	audio_margin;

	unsigned int	dn_spread_ctl1;
	unsigned int	dn_spread_ctl2;
	unsigned char	res17[0x18];
	unsigned int	m_cal_ctl;
	unsigned int	m_vid_gen_filter_th;
	unsigned char	res18[0x10];
	unsigned int	m_aud_gen_filter_th;
	unsigned char	res50[0x4];

	unsigned int	aux_ch_sta;
	unsigned int	aux_err_num;
	unsigned int	aux_ch_defer_ctl;
	unsigned int	aux_rx_comm;
	unsigned int	buffer_data_ctl;

	unsigned int	aux_ch_ctl1;
	unsigned int	aux_addr_7_0;
	unsigned int	aux_addr_15_8;
	unsigned int	aux_addr_19_16;
	unsigned int	aux_ch_ctl2;
	unsigned char	res19[0x18];
	unsigned int	buf_data0;
	unsigned char	res20[0x3c];

	unsigned int	soc_general_ctl;
	unsigned char	res21[0x8c];
	unsigned int	crc_con;
	unsigned int	crc_result;
	unsigned char	res22[0x8];

	unsigned int	common_int_mask1;
	unsigned int	common_int_mask2;
	unsigned int	common_int_mask3;
	unsigned int	common_int_mask4;
	unsigned int	int_sta_mask1;
	unsigned int	int_sta_mask2;
	unsigned int	int_sta_mask3;
	unsigned int	int_sta_mask4;
	unsigned int	int_sta_mask;
	unsigned int	crc_result2;
	unsigned int	scrambler_reset_cnt;

	unsigned int	pn_inv;
	unsigned int	psr_config;
	unsigned int	psr_command0;
	unsigned int	psr_command1;
	unsigned int	psr_crc_mon0;
	unsigned int	psr_crc_mon1;

	unsigned char	res24[0x30];
	unsigned int	phy_bist_ctrl;
	unsigned char	res25[0xc];
	unsigned int	phy_ctrl;
	unsigned char	res26[0x1c];
	unsigned int	test_pattern_gen_en;
	unsigned int	test_pattern_gen_ctrl;
};

#endif	/* __ASSEMBLY__ */

/* For DP VIDEO CTL 1 */
#define VIDEO_EN_MASK				(0x01 << 7)
#define VIDEO_MUTE_MASK				(0x01 << 6)

/* For DP VIDEO CTL 4 */
#define VIDEO_BIST_MASK				(0x1 << 3)

/* EXYNOS_DP_ANALOG_CTL_1 */
#define SEL_BG_NEW_BANDGAP			(0x0 << 6)
#define SEL_BG_INTERNAL_RESISTOR		(0x1 << 6)
#define TX_TERMINAL_CTRL_73_OHM			(0x0 << 4)
#define TX_TERMINAL_CTRL_61_OHM			(0x1 << 4)
#define TX_TERMINAL_CTRL_50_OHM			(0x2 << 4)
#define TX_TERMINAL_CTRL_45_OHM			(0x3 << 4)
#define SWING_A_30PER_G_INCREASE		(0x1 << 3)
#define SWING_A_30PER_G_NORMAL			(0x0 << 3)

/* EXYNOS_DP_ANALOG_CTL_2 */
#define CPREG_BLEED				(0x1 << 4)
#define SEL_24M					(0x1 << 3)
#define TX_DVDD_BIT_1_0000V			(0x3 << 0)
#define TX_DVDD_BIT_1_0625V			(0x4 << 0)
#define TX_DVDD_BIT_1_1250V			(0x5 << 0)

/* EXYNOS_DP_ANALOG_CTL_3 */
#define DRIVE_DVDD_BIT_1_0000V			(0x3 << 5)
#define DRIVE_DVDD_BIT_1_0625V			(0x4 << 5)
#define DRIVE_DVDD_BIT_1_1250V			(0x5 << 5)
#define SEL_CURRENT_DEFAULT			(0x0 << 3)
#define VCO_BIT_000_MICRO			(0x0 << 0)
#define VCO_BIT_200_MICRO			(0x1 << 0)
#define VCO_BIT_300_MICRO			(0x2 << 0)
#define VCO_BIT_400_MICRO			(0x3 << 0)
#define VCO_BIT_500_MICRO			(0x4 << 0)
#define VCO_BIT_600_MICRO			(0x5 << 0)
#define VCO_BIT_700_MICRO			(0x6 << 0)
#define VCO_BIT_900_MICRO			(0x7 << 0)

/* EXYNOS_DP_PLL_FILTER_CTL_1 */
#define PD_RING_OSC				(0x1 << 6)
#define AUX_TERMINAL_CTRL_52_OHM		(0x3 << 4)
#define AUX_TERMINAL_CTRL_69_OHM		(0x2 << 4)
#define AUX_TERMINAL_CTRL_102_OHM		(0x1 << 4)
#define AUX_TERMINAL_CTRL_200_OHM		(0x0 << 4)
#define TX_CUR1_1X				(0x0 << 2)
#define TX_CUR1_2X				(0x1 << 2)
#define TX_CUR1_3X				(0x2 << 2)
#define TX_CUR_1_MA				(0x0 << 0)
#define TX_CUR_2_MA			        (0x1 << 0)
#define TX_CUR_3_MA				(0x2 << 0)
#define TX_CUR_4_MA				(0x3 << 0)

/* EXYNOS_DP_PLL_FILTER_CTL_2 */
#define CH3_AMP_0_MV				(0x3 << 12)
#define CH2_AMP_0_MV				(0x3 << 8)
#define CH1_AMP_0_MV				(0x3 << 4)
#define CH0_AMP_0_MV				(0x3 << 0)

/* EXYNOS_DP_PLL_CTL */
#define DP_PLL_PD			        (0x1 << 7)
#define DP_PLL_RESET				(0x1 << 6)
#define DP_PLL_LOOP_BIT_DEFAULT		        (0x1 << 4)
#define DP_PLL_REF_BIT_1_1250V			(0x5 << 0)
#define DP_PLL_REF_BIT_1_2500V		        (0x7 << 0)

/* EXYNOS_DP_INT_CTL */
#define SOFT_INT_CTRL				(0x1 << 2)
#define INT_POL					(0x1 << 0)

/* DP TX SW RESET */
#define RESET_DP_TX				(0x01 << 0)

/* DP FUNC_EN_1 */
#define MASTER_VID_FUNC_EN_N			(0x1 << 7)
#define SLAVE_VID_FUNC_EN_N			(0x1 << 5)
#define AUD_FIFO_FUNC_EN_N			(0x1 << 4)
#define AUD_FUNC_EN_N				(0x1 << 3)
#define HDCP_FUNC_EN_N				(0x1 << 2)
#define CRC_FUNC_EN_N				(0x1 << 1)
#define SW_FUNC_EN_N				(0x1 << 0)

/* DP FUNC_EN_2 */
#define SSC_FUNC_EN_N			        (0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N		        (0x1 << 0)

/* EXYNOS_DP_PHY_PD */
#define PHY_PD					(0x1 << 5)
#define AUX_PD					(0x1 << 4)
#define CH3_PD					(0x1 << 3)
#define CH2_PD					(0x1 << 2)
#define CH1_PD					(0x1 << 1)
#define CH0_PD					(0x1 << 0)

/* EXYNOS_DP_COMMON_INT_STA_1 */
#define VSYNC_DET				(0x1 << 7)
#define PLL_LOCK_CHG				(0x1 << 6)
#define SPDIF_ERR				(0x1 << 5)
#define SPDIF_UNSTBL				(0x1 << 4)
#define VID_FORMAT_CHG				(0x1 << 3)
#define AUD_CLK_CHG				(0x1 << 2)
#define VID_CLK_CHG				(0x1 << 1)
#define SW_INT					(0x1 << 0)

/* EXYNOS_DP_DEBUG_CTL */
#define PLL_LOCK				(0x1 << 4)
#define F_PLL_LOCK				(0x1 << 3)
#define PLL_LOCK_CTRL				(0x1 << 2)

/* EXYNOS_DP_FUNC_EN_2 */
#define SSC_FUNC_EN_N				(0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N			(0x1 << 0)

/* EXYNOS_DP_COMMON_INT_STA_4 */
#define PSR_ACTIVE				(0x1 << 7)
#define PSR_INACTIVE				(0x1 << 6)
#define SPDIF_BI_PHASE_ERR			(0x1 << 5)
#define HOTPLUG_CHG				(0x1 << 2)
#define HPD_LOST				(0x1 << 1)
#define PLUG					(0x1 << 0)

/* EXYNOS_DP_INT_STA */
#define INT_HPD					(0x1 << 6)
#define HW_TRAINING_FINISH			(0x1 << 5)
#define RPLY_RECEIV				(0x1 << 1)
#define AUX_ERR					(0x1 << 0)

/* EXYNOS_DP_SYS_CTL_3 */
#define HPD_STATUS				(0x1 << 6)
#define F_HPD					(0x1 << 5)
#define HPD_CTRL				(0x1 << 4)
#define HDCP_RDY				(0x1 << 3)
#define STRM_VALID				(0x1 << 2)
#define F_VALID					(0x1 << 1)
#define VALID_CTRL				(0x1 << 0)

/* EXYNOS_DP_AUX_HW_RETRY_CTL */
#define AUX_BIT_PERIOD_EXPECTED_DELAY(x)	(((x) & 0x7) << 8)
#define AUX_HW_RETRY_INTERVAL_MASK		(0x3 << 3)
#define AUX_HW_RETRY_INTERVAL_600_MICROSECONDS	(0x0 << 3)
#define AUX_HW_RETRY_INTERVAL_800_MICROSECONDS	(0x1 << 3)
#define AUX_HW_RETRY_INTERVAL_1000_MICROSECONDS	(0x2 << 3)
#define AUX_HW_RETRY_INTERVAL_1800_MICROSECONDS	(0x3 << 3)
#define AUX_HW_RETRY_COUNT_SEL(x)		(((x) & 0x7) << 0)

/* EXYNOS_DP_AUX_CH_DEFER_CTL */
#define DEFER_CTRL_EN				(0x1 << 7)
#define DEFER_COUNT(x)				(((x) & 0x7f) << 0)

#define COMMON_INT_MASK_1			(0)
#define COMMON_INT_MASK_2			(0)
#define COMMON_INT_MASK_3			(0)
#define COMMON_INT_MASK_4			(0)
#define INT_STA_MASK				(0)

/* EXYNOS_DP_BUFFER_DATA_CTL */
#define BUF_CLR					(0x1 << 7)
#define BUF_DATA_COUNT(x)			(((x) & 0x1f) << 0)

/* EXYNOS_DP_AUX_ADDR_7_0 */
#define AUX_ADDR_7_0(x)				(((x) >> 0) & 0xff)

/* EXYNOS_DP_AUX_ADDR_15_8 */
#define AUX_ADDR_15_8(x)			(((x) >> 8) & 0xff)

/* EXYNOS_DP_AUX_ADDR_19_16 */
#define AUX_ADDR_19_16(x)			(((x) >> 16) & 0x0f)

/* EXYNOS_DP_AUX_CH_CTL_1 */
#define AUX_LENGTH(x)				(((x - 1) & 0xf) << 4)
#define AUX_TX_COMM_MASK			(0xf << 0)
#define AUX_TX_COMM_DP_TRANSACTION		(0x1 << 3)
#define AUX_TX_COMM_I2C_TRANSACTION		(0x0 << 3)
#define AUX_TX_COMM_MOT				(0x1 << 2)
#define AUX_TX_COMM_WRITE			(0x0 << 0)
#define AUX_TX_COMM_READ			(0x1 << 0)

/* EXYNOS_DP_AUX_CH_CTL_2 */
#define ADDR_ONLY				(0x1 << 1)
#define AUX_EN					(0x1 << 0)

/* EXYNOS_DP_AUX_CH_STA */
#define AUX_BUSY				(0x1 << 4)
#define AUX_STATUS_MASK				(0xf << 0)

/* EXYNOS_DP_AUX_RX_COMM */
#define AUX_RX_COMM_I2C_DEFER			(0x2 << 2)
#define AUX_RX_COMM_AUX_DEFER			(0x2 << 0)

/* EXYNOS_DP_PHY_TEST */
#define MACRO_RST				(0x1 << 5)
#define CH1_TEST				(0x1 << 1)
#define CH0_TEST				(0x1 << 0)

/* EXYNOS_DP_TRAINING_PTN_SET */
#define SCRAMBLER_TYPE				(0x1 << 9)
#define HW_LINK_TRAINING_PATTERN		(0x1 << 8)
#define SCRAMBLING_DISABLE			(0x1 << 5)
#define SCRAMBLING_ENABLE			(0x0 << 5)
#define LINK_QUAL_PATTERN_SET_MASK		(0x3 << 2)
#define LINK_QUAL_PATTERN_SET_PRBS7		(0x3 << 2)
#define LINK_QUAL_PATTERN_SET_D10_2		(0x1 << 2)
#define LINK_QUAL_PATTERN_SET_DISABLE		(0x0 << 2)
#define SW_TRAINING_PATTERN_SET_MASK		(0x3 << 0)
#define SW_TRAINING_PATTERN_SET_PTN2		(0x2 << 0)
#define SW_TRAINING_PATTERN_SET_PTN1		(0x1 << 0)
#define SW_TRAINING_PATTERN_SET_NORMAL		(0x0 << 0)

/* EXYNOS_DP_TOTAL_LINE_CFG */
#define TOTAL_LINE_CFG_L(x)			((x) & 0xff)
#define TOTAL_LINE_CFG_H(x)			((((x) >> 8)) & 0xff)
#define ACTIVE_LINE_CFG_L(x)			((x) & 0xff)
#define ACTIVE_LINE_CFG_H(x)			(((x) >> 8) & 0xff)
#define TOTAL_PIXEL_CFG_L(x)			((x) & 0xff)
#define TOTAL_PIXEL_CFG_H(x)			((((x) >> 8)) & 0xff)
#define ACTIVE_PIXEL_CFG_L(x)			((x) & 0xff)
#define ACTIVE_PIXEL_CFG_H(x)			((((x) >> 8)) & 0xff)

#define H_F_PORCH_CFG_L(x)			((x) & 0xff)
#define H_F_PORCH_CFG_H(x)			((((x) >> 8)) & 0xff)
#define H_SYNC_PORCH_CFG_L(x)			((x) & 0xff)
#define H_SYNC_PORCH_CFG_H(x)			((((x) >> 8)) & 0xff)
#define H_B_PORCH_CFG_L(x)			((x) & 0xff)
#define H_B_PORCH_CFG_H(x)			((((x) >> 8)) & 0xff)

/* EXYNOS_DP_LN0_LINK_TRAINING_CTL */
#define MAX_PRE_EMPHASIS_REACH_0		(0x1 << 5)
#define PRE_EMPHASIS_SET_0_SET(x)		(((x) & 0x3) << 3)
#define PRE_EMPHASIS_SET_0_GET(x)		(((x) >> 3) & 0x3)
#define PRE_EMPHASIS_SET_0_MASK			(0x3 << 3)
#define PRE_EMPHASIS_SET_0_SHIFT		(3)
#define PRE_EMPHASIS_SET_0_LEVEL_3		(0x3 << 3)
#define PRE_EMPHASIS_SET_0_LEVEL_2		(0x2 << 3)
#define PRE_EMPHASIS_SET_0_LEVEL_1		(0x1 << 3)
#define PRE_EMPHASIS_SET_0_LEVEL_0		(0x0 << 3)
#define MAX_DRIVE_CURRENT_REACH_0		(0x1 << 2)
#define DRIVE_CURRENT_SET_0_MASK		(0x3 << 0)
#define DRIVE_CURRENT_SET_0_SET(x)		(((x) & 0x3) << 0)
#define DRIVE_CURRENT_SET_0_GET(x)		(((x) >> 0) & 0x3)
#define DRIVE_CURRENT_SET_0_LEVEL_3		(0x3 << 0)
#define DRIVE_CURRENT_SET_0_LEVEL_2		(0x2 << 0)
#define DRIVE_CURRENT_SET_0_LEVEL_1		(0x1 << 0)
#define DRIVE_CURRENT_SET_0_LEVEL_0		(0x0 << 0)

/* EXYNOS_DP_LN1_LINK_TRAINING_CTL */
#define MAX_PRE_EMPHASIS_REACH_1		(0x1 << 5)
#define PRE_EMPHASIS_SET_1_SET(x)		(((x) & 0x3) << 3)
#define PRE_EMPHASIS_SET_1_GET(x)		(((x) >> 3) & 0x3)
#define PRE_EMPHASIS_SET_1_MASK			(0x3 << 3)
#define PRE_EMPHASIS_SET_1_SHIFT		(3)
#define PRE_EMPHASIS_SET_1_LEVEL_3		(0x3 << 3)
#define PRE_EMPHASIS_SET_1_LEVEL_2		(0x2 << 3)
#define PRE_EMPHASIS_SET_1_LEVEL_1		(0x1 << 3)
#define PRE_EMPHASIS_SET_1_LEVEL_0		(0x0 << 3)
#define MAX_DRIVE_CURRENT_REACH_1		(0x1 << 2)
#define DRIVE_CURRENT_SET_1_MASK		(0x3 << 0)
#define DRIVE_CURRENT_SET_1_SET(x)		(((x) & 0x3) << 0)
#define DRIVE_CURRENT_SET_1_GET(x)		(((x) >> 0) & 0x3)
#define DRIVE_CURRENT_SET_1_LEVEL_3		(0x3 << 0)
#define DRIVE_CURRENT_SET_1_LEVEL_2		(0x2 << 0)
#define DRIVE_CURRENT_SET_1_LEVEL_1		(0x1 << 0)
#define DRIVE_CURRENT_SET_1_LEVEL_0		(0x0 << 0)

/* EXYNOS_DP_LN2_LINK_TRAINING_CTL */
#define MAX_PRE_EMPHASIS_REACH_2		(0x1 << 5)
#define PRE_EMPHASIS_SET_2_SET(x)		(((x) & 0x3) << 3)
#define PRE_EMPHASIS_SET_2_GET(x)		(((x) >> 3) & 0x3)
#define PRE_EMPHASIS_SET_2_MASK			(0x3 << 3)
#define PRE_EMPHASIS_SET_2_SHIFT		(3)
#define PRE_EMPHASIS_SET_2_LEVEL_3		(0x3 << 3)
#define PRE_EMPHASIS_SET_2_LEVEL_2		(0x2 << 3)
#define PRE_EMPHASIS_SET_2_LEVEL_1		(0x1 << 3)
#define PRE_EMPHASIS_SET_2_LEVEL_0		(0x0 << 3)
#define MAX_DRIVE_CURRENT_REACH_2		(0x1 << 2)
#define DRIVE_CURRENT_SET_2_MASK		(0x3 << 0)
#define DRIVE_CURRENT_SET_2_SET(x)		(((x) & 0x3) << 0)
#define DRIVE_CURRENT_SET_2_GET(x)		(((x) >> 0) & 0x3)
#define DRIVE_CURRENT_SET_2_LEVEL_3		(0x3 << 0)
#define DRIVE_CURRENT_SET_2_LEVEL_2		(0x2 << 0)
#define DRIVE_CURRENT_SET_2_LEVEL_1		(0x1 << 0)
#define DRIVE_CURRENT_SET_2_LEVEL_0		(0x0 << 0)

/* EXYNOS_DP_LN3_LINK_TRAINING_CTL */
#define MAX_PRE_EMPHASIS_REACH_3		(0x1 << 5)
#define PRE_EMPHASIS_SET_3_SET(x)		(((x) & 0x3) << 3)
#define PRE_EMPHASIS_SET_3_GET(x)		(((x) >> 3) & 0x3)
#define PRE_EMPHASIS_SET_3_MASK			(0x3 << 3)
#define PRE_EMPHASIS_SET_3_SHIFT		(3)
#define PRE_EMPHASIS_SET_3_LEVEL_3		(0x3 << 3)
#define PRE_EMPHASIS_SET_3_LEVEL_2		(0x2 << 3)
#define PRE_EMPHASIS_SET_3_LEVEL_1		(0x1 << 3)
#define PRE_EMPHASIS_SET_3_LEVEL_0		(0x0 << 3)
#define MAX_DRIVE_CURRENT_REACH_3		(0x1 << 2)
#define DRIVE_CURRENT_SET_3_MASK		(0x3 << 0)
#define DRIVE_CURRENT_SET_3_SET(x)		(((x) & 0x3) << 0)
#define DRIVE_CURRENT_SET_3_GET(x)		(((x) >> 0) & 0x3)
#define DRIVE_CURRENT_SET_3_LEVEL_3		(0x3 << 0)
#define DRIVE_CURRENT_SET_3_LEVEL_2		(0x2 << 0)
#define DRIVE_CURRENT_SET_3_LEVEL_1		(0x1 << 0)
#define DRIVE_CURRENT_SET_3_LEVEL_0		(0x0 << 0)

/* EXYNOS_DP_VIDEO_CTL_10 */
#define FORMAT_SEL				(0x1 << 4)
#define INTERACE_SCAN_CFG			(0x1 << 2)
#define INTERACE_SCAN_CFG_SHIFT			(2)
#define VSYNC_POLARITY_CFG			(0x1 << 1)
#define V_S_POLARITY_CFG_SHIFT			(1)
#define HSYNC_POLARITY_CFG			(0x1 << 0)
#define H_S_POLARITY_CFG_SHIFT			(0)

/* EXYNOS_DP_SOC_GENERAL_CTL */
#define AUDIO_MODE_SPDIF_MODE			(0x1 << 8)
#define AUDIO_MODE_MASTER_MODE			(0x0 << 8)
#define MASTER_VIDEO_INTERLACE_EN		(0x1 << 4)
#define VIDEO_MASTER_CLK_SEL			(0x1 << 2)
#define VIDEO_MASTER_MODE_EN			(0x1 << 1)
#define VIDEO_MODE_MASK				(0x1 << 0)
#define VIDEO_MODE_SLAVE_MODE			(0x1 << 0)
#define VIDEO_MODE_MASTER_MODE			(0x0 << 0)

/* EXYNOS_DP_VIDEO_CTL_1 */
#define VIDEO_EN				(0x1 << 7)
#define HDCP_VIDEO_MUTE				(0x1 << 6)

/* EXYNOS_DP_VIDEO_CTL_2 */
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

/* EXYNOS_DP_VIDEO_CTL_3 */
#define IN_YC_COEFFI_MASK			(0x1 << 7)
#define IN_YC_COEFFI_SHIFT			(7)
#define IN_YC_COEFFI_ITU709			(0x1 << 7)
#define IN_YC_COEFFI_ITU601			(0x0 << 7)
#define VID_CHK_UPDATE_TYPE_MASK		(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_SHIFT		(4)
#define VID_CHK_UPDATE_TYPE_1			(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_0			(0x0 << 4)

/* EXYNOS_DP_TEST_PATTERN_GEN_EN */
#define TEST_PATTERN_GEN_EN			(0x1 << 0)
#define TEST_PATTERN_GEN_DIS			(0x0 << 0)

/* EXYNOS_DP_TEST_PATTERN_GEN_CTRL */
#define TEST_PATTERN_MODE_COLOR_SQUARE		(0x3 << 0)
#define TEST_PATTERN_MODE_BALCK_WHITE_V_LINES	(0x2 << 0)
#define TEST_PATTERN_MODE_COLOR_RAMP		(0x1 << 0)

/* EXYNOS_DP_VIDEO_CTL_4 */
#define BIST_EN					(0x1 << 3)
#define BIST_WIDTH_MASK				(0x1 << 2)
#define BIST_WIDTH_BAR_32_PIXEL			(0x0 << 2)
#define BIST_WIDTH_BAR_64_PIXEL			(0x1 << 2)
#define BIST_TYPE_MASK				(0x3 << 0)
#define BIST_TYPE_COLOR_BAR			(0x0 << 0)
#define BIST_TYPE_WHITE_GRAY_BLACK_BAR		(0x1 << 0)
#define BIST_TYPE_MOBILE_WHITE_BAR		(0x2 << 0)

/* EXYNOS_DP_SYS_CTL_1 */
#define DET_STA					(0x1 << 2)
#define FORCE_DET				(0x1 << 1)
#define DET_CTRL				(0x1 << 0)

/* EXYNOS_DP_SYS_CTL_2 */
#define CHA_CRI(x)				(((x) & 0xf) << 4)
#define CHA_STA					(0x1 << 2)
#define FORCE_CHA				(0x1 << 1)
#define CHA_CTRL				(0x1 << 0)

/* EXYNOS_DP_SYS_CTL_3 */
#define HPD_STATUS				(0x1 << 6)
#define F_HPD					(0x1 << 5)
#define HPD_CTRL				(0x1 << 4)
#define HDCP_RDY				(0x1 << 3)
#define STRM_VALID				(0x1 << 2)
#define F_VALID					(0x1 << 1)
#define VALID_CTRL				(0x1 << 0)

/* EXYNOS_DP_SYS_CTL_4 */
#define FIX_M_AUD				(0x1 << 4)
#define ENHANCED				(0x1 << 3)
#define FIX_M_VID				(0x1 << 2)
#define M_VID_UPDATE_CTRL			(0x3 << 0)

/* EXYNOS_M_VID_X */
#define M_VID0_CFG(x)				((x) & 0xff)
#define M_VID1_CFG(x)				(((x) >> 8) & 0xff)
#define M_VID2_CFG(x)				(((x) >> 16) & 0xff)

/* EXYNOS_M_VID_X */
#define N_VID0_CFG(x)				((x) & 0xff)
#define N_VID1_CFG(x)				(((x) >> 8) & 0xff)
#define N_VID2_CFG(x)				(((x) >> 16) & 0xff)

/* DPCD_TRAINING_PATTERN_SET */
#define DPCD_SCRAMBLING_DISABLED		(0x1 << 5)
#define DPCD_SCRAMBLING_ENABLED			(0x0 << 5)
#define DPCD_TRAINING_PATTERN_2			(0x2 << 0)
#define DPCD_TRAINING_PATTERN_1			(0x1 << 0)
#define DPCD_TRAINING_PATTERN_DISABLED		(0x0 << 0)

/* Definition for DPCD Register */
#define DPCD_DPCD_REV				(0x0000)
#define DPCD_MAX_LINK_RATE			(0x0001)
#define DPCD_MAX_LANE_COUNT			(0x0002)
#define DPCD_LINK_BW_SET			(0x0100)
#define DPCD_LANE_COUNT_SET			(0x0101)
#define DPCD_TRAINING_PATTERN_SET		(0x0102)
#define DPCD_TRAINING_LANE0_SET			(0x0103)
#define DPCD_LANE0_1_STATUS			(0x0202)
#define DPCD_LN_ALIGN_UPDATED			(0x0204)
#define DPCD_ADJUST_REQUEST_LANE0_1		(0x0206)
#define DPCD_ADJUST_REQUEST_LANE2_3		(0x0207)
#define DPCD_TEST_REQUEST			(0x0218)
#define DPCD_TEST_RESPONSE			(0x0260)
#define DPCD_TEST_EDID_CHECKSUM			(0x0261)
#define DPCD_SINK_POWER_STATE			(0x0600)

/* DPCD_TEST_REQUEST */
#define DPCD_TEST_EDID_READ			(0x1 << 2)

/* DPCD_TEST_RESPONSE */
#define DPCD_TEST_EDID_CHECKSUM_WRITE		(0x1 << 2)

/* DPCD_SINK_POWER_STATE */
#define DPCD_SET_POWER_STATE_D0			(0x1 << 0)
#define DPCD_SET_POWER_STATE_D4			(0x2 << 0)

/* I2C EDID Chip ID, Slave Address */
#define I2C_EDID_DEVICE_ADDR			(0x50)
#define I2C_E_EDID_DEVICE_ADDR			(0x30)
#define EDID_BLOCK_LENGTH			(0x80)
#define EDID_HEADER_PATTERN			(0x00)
#define EDID_EXTENSION_FLAG			(0x7e)
#define EDID_CHECKSUM				(0x7f)

/* DPCD_LANE0_1_STATUS */
#define DPCD_LANE1_SYMBOL_LOCKED		(0x1 << 6)
#define DPCD_LANE1_CHANNEL_EQ_DONE		(0x1 << 5)
#define DPCD_LANE1_CR_DONE			(0x1 << 4)
#define DPCD_LANE0_SYMBOL_LOCKED		(0x1 << 2)
#define DPCD_LANE0_CHANNEL_EQ_DONE		(0x1 << 1)
#define DPCD_LANE0_CR_DONE			(0x1 << 0)

/* DPCD_ADJUST_REQUEST_LANE0_1 */
#define DPCD_PRE_EMPHASIS_LANE1_MASK		(0x3 << 6)
#define DPCD_PRE_EMPHASIS_LANE1(x)		(((x) >> 6) & 0x3)
#define DPCD_PRE_EMPHASIS_LANE1_LEVEL_3		(0x3 << 6)
#define DPCD_PRE_EMPHASIS_LANE1_LEVEL_2		(0x2 << 6)
#define DPCD_PRE_EMPHASIS_LANE1_LEVEL_1		(0x1 << 6)
#define DPCD_PRE_EMPHASIS_LANE1_LEVEL_0		(0x0 << 6)
#define DPCD_VOLTAGE_SWING_LANE1_MASK		(0x3 << 4)
#define DPCD_VOLTAGE_SWING_LANE1(x)		(((x) >> 4) & 0x3)
#define DPCD_VOLTAGE_SWING_LANE1_LEVEL_3	(0x3 << 4)
#define DPCD_VOLTAGE_SWING_LANE1_LEVEL_2	(0x2 << 4)
#define DPCD_VOLTAGE_SWING_LANE1_LEVEL_1	(0x1 << 4)
#define DPCD_VOLTAGE_SWING_LANE1_LEVEL_0	(0x0 << 4)
#define DPCD_PRE_EMPHASIS_LANE0_MASK		(0x3 << 2)
#define DPCD_PRE_EMPHASIS_LANE0(x)		(((x) >> 2) & 0x3)
#define DPCD_PRE_EMPHASIS_LANE0_LEVEL_3		(0x3 << 2)
#define DPCD_PRE_EMPHASIS_LANE0_LEVEL_2		(0x2 << 2)
#define DPCD_PRE_EMPHASIS_LANE0_LEVEL_1		(0x1 << 2)
#define DPCD_PRE_EMPHASIS_LANE0_LEVEL_0		(0x0 << 2)
#define DPCD_VOLTAGE_SWING_LANE0_MASK		(0x3 << 0)
#define DPCD_VOLTAGE_SWING_LANE0(x)		(((x) >> 0) & 0x3)
#define DPCD_VOLTAGE_SWING_LANE0_LEVEL_3	(0x3 << 0)
#define DPCD_VOLTAGE_SWING_LANE0_LEVEL_2	(0x2 << 0)
#define DPCD_VOLTAGE_SWING_LANE0_LEVEL_1	(0x1 << 0)
#define DPCD_VOLTAGE_SWING_LANE0_LEVEL_0	(0x0 << 0)

/* DPCD_ADJUST_REQUEST_LANE2_3 */
#define DPCD_PRE_EMPHASIS_LANE2_MASK		(0x3 << 6)
#define DPCD_PRE_EMPHASIS_LANE2(x)		(((x) >> 6) & 0x3)
#define DPCD_PRE_EMPHASIS_LANE2_LEVEL_3		(0x3 << 6)
#define DPCD_PRE_EMPHASIS_LANE2_LEVEL_2		(0x2 << 6)
#define DPCD_PRE_EMPHASIS_LANE2_LEVEL_1		(0x1 << 6)
#define DPCD_PRE_EMPHASIS_LANE2_LEVEL_0		(0x0 << 6)
#define DPCD_VOLTAGE_SWING_LANE2_MASK		(0x3 << 4)
#define DPCD_VOLTAGE_SWING_LANE2(x)		(((x) >> 4) & 0x3)
#define DPCD_VOLTAGE_SWING_LANE2_LEVEL_3	(0x3 << 4)
#define DPCD_VOLTAGE_SWING_LANE2_LEVEL_2	(0x2 << 4)
#define DPCD_VOLTAGE_SWING_LANE2_LEVEL_1	(0x1 << 4)
#define DPCD_VOLTAGE_SWING_LANE2_LEVEL_0	(0x0 << 4)
#define DPCD_PRE_EMPHASIS_LANE3_MASK		(0x3 << 2)
#define DPCD_PRE_EMPHASIS_LANE3(x)		(((x) >> 2) & 0x3)
#define DPCD_PRE_EMPHASIS_LANE3_LEVEL_3		(0x3 << 2)
#define DPCD_PRE_EMPHASIS_LANE3_LEVEL_2		(0x2 << 2)
#define DPCD_PRE_EMPHASIS_LANE3_LEVEL_1		(0x1 << 2)
#define DPCD_PRE_EMPHASIS_LANE3_LEVEL_0		(0x0 << 2)
#define DPCD_VOLTAGE_SWING_LANE3_MASK		(0x3 << 0)
#define DPCD_VOLTAGE_SWING_LANE3(x)		(((x) >> 0) & 0x3)
#define DPCD_VOLTAGE_SWING_LANE3_LEVEL_3	(0x3 << 0)
#define DPCD_VOLTAGE_SWING_LANE3_LEVEL_2	(0x2 << 0)
#define DPCD_VOLTAGE_SWING_LANE3_LEVEL_1	(0x1 << 0)
#define DPCD_VOLTAGE_SWING_LANE3_LEVEL_0	(0x0 << 0)

/* DPCD_LANE_COUNT_SET */
#define DPCD_ENHANCED_FRAME_EN			(0x1 << 7)
#define DPCD_LN_COUNT_SET(x)			((x) & 0x1f)

/* DPCD_LANE_ALIGN__STATUS_UPDATED */
#define DPCD_LINK_STATUS_UPDATED		(0x1 << 7)
#define DPCD_DOWNSTREAM_PORT_STATUS_CHANGED	(0x1 << 6)
#define DPCD_INTERLANE_ALIGN_DONE		(0x1 << 0)

/* DPCD_TRAINING_LANE0_SET */
#define DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_3		(0x3 << 3)
#define DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_2		(0x2 << 3)
#define DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_1		(0x1 << 3)
#define DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0		(0x0 << 3)
#define DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_3	(0x3 << 0)
#define DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_2	(0x2 << 0)
#define DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_1	(0x1 << 0)
#define DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0	(0x0 << 0)

#define DPCD_REQ_ADJ_SWING			(0x00)
#define DPCD_REQ_ADJ_EMPHASIS			(0x01)

#define DP_LANE_STAT_CR_DONE			(0x01 << 0)
#define DP_LANE_STAT_CE_DONE			(0x01 << 1)
#define DP_LANE_STAT_SYM_LOCK			(0x01 << 2)

#endif
