/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2011-2013, NVIDIA Corporation.
 */

#ifndef _VIDEO_TEGRA124_SOR_H
#define _VIDEO_TEGRA124_SOR_H

#define SUPER_STATE0					0x1
#define SUPER_STATE0_UPDATE_SHIFT			0
#define SUPER_STATE0_UPDATE_DEFAULT_MASK		0x1
#define SUPER_STATE1					0x2
#define SUPER_STATE1_ATTACHED_SHIFT			3
#define SUPER_STATE1_ATTACHED_NO			(0 << 3)
#define SUPER_STATE1_ATTACHED_YES			(1 << 3)
#define SUPER_STATE1_ASY_ORMODE_SHIFT			2
#define SUPER_STATE1_ASY_ORMODE_SAFE			(0 << 2)
#define SUPER_STATE1_ASY_ORMODE_NORMAL			(1 << 2)
#define SUPER_STATE1_ASY_HEAD_OP_SHIFT			0
#define SUPER_STATE1_ASY_HEAD_OP_DEFAULT_MASK		0x3
#define SUPER_STATE1_ASY_HEAD_OP_SLEEP			0
#define SUPER_STATE1_ASY_HEAD_OP_SNOOZE			1
#define SUPER_STATE1_ASY_HEAD_OP_AWAKE			2
#define STATE0						0x3
#define STATE0_UPDATE_SHIFT				0
#define STATE0_UPDATE_DEFAULT_MASK			0x1
#define STATE1						0x4
#define STATE1_ASY_PIXELDEPTH_SHIFT			17
#define STATE1_ASY_PIXELDEPTH_DEFAULT_MASK		(0xf << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_16_422		(1 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_18_444		(2 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_20_422		(3 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_24_422		(4 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_24_444		(5 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_30_444		(6 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_32_422		(7 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_36_444		(8 << 17)
#define STATE1_ASY_PIXELDEPTH_BPP_48_444		(9 << 17)
#define STATE1_ASY_REPLICATE_SHIFT			15
#define STATE1_ASY_REPLICATE_DEFAULT_MASK		(3 << 15)
#define STATE1_ASY_REPLICATE_OFF			(0 << 15)
#define STATE1_ASY_REPLICATE_X2				(1 << 15)
#define STATE1_ASY_REPLICATE_X4				(2 << 15)
#define STATE1_ASY_DEPOL_SHIFT				14
#define STATE1_ASY_DEPOL_DEFAULT_MASK			(1 << 14)
#define STATE1_ASY_DEPOL_POSITIVE_TRUE			(0 << 14)
#define STATE1_ASY_DEPOL_NEGATIVE_TRUE			(1 << 14)
#define STATE1_ASY_VSYNCPOL_SHIFT			13
#define STATE1_ASY_VSYNCPOL_DEFAULT_MASK		(1 << 13)
#define STATE1_ASY_VSYNCPOL_POSITIVE_TRUE		(0 << 13)
#define STATE1_ASY_VSYNCPOL_NEGATIVE_TRUE		(1 << 13)
#define STATE1_ASY_HSYNCPOL_SHIFT			12
#define STATE1_ASY_HSYNCPOL_DEFAULT_MASK		(1 << 12)
#define STATE1_ASY_HSYNCPOL_POSITIVE_TRUE		(0 << 12)
#define STATE1_ASY_HSYNCPOL_NEGATIVE_TRUE		(1 << 12)
#define STATE1_ASY_PROTOCOL_SHIFT			8
#define STATE1_ASY_PROTOCOL_DEFAULT_MASK		(0xf << 8)
#define STATE1_ASY_PROTOCOL_LVDS_CUSTOM			(0 << 8)
#define STATE1_ASY_PROTOCOL_DP_A			(8 << 8)
#define STATE1_ASY_PROTOCOL_DP_B			(9 << 8)
#define STATE1_ASY_PROTOCOL_CUSTOM			(15 << 8)
#define STATE1_ASY_CRCMODE_SHIFT			6
#define STATE1_ASY_CRCMODE_DEFAULT_MASK			(3 << 6)
#define STATE1_ASY_CRCMODE_ACTIVE_RASTER		(0 << 6)
#define STATE1_ASY_CRCMODE_COMPLETE_RASTER		(1 << 6)
#define STATE1_ASY_CRCMODE_NON_ACTIVE_RASTER		(2 << 6)
#define STATE1_ASY_SUBOWNER_SHIFT			4
#define STATE1_ASY_SUBOWNER_DEFAULT_MASK		(3 << 4)
#define STATE1_ASY_SUBOWNER_NONE			(0 << 4)
#define STATE1_ASY_SUBOWNER_SUBHEAD0			(1 << 4)
#define STATE1_ASY_SUBOWNER_SUBHEAD1			(2 << 4)
#define STATE1_ASY_SUBOWNER_BOTH			(3 << 4)
#define STATE1_ASY_OWNER_SHIFT				0
#define STATE1_ASY_OWNER_DEFAULT_MASK			0xf
#define STATE1_ASY_OWNER_NONE				0
#define STATE1_ASY_OWNER_HEAD0				1
#define STATE1_ASY_OWNER_HEAD1				2
#define NV_HEAD_STATE0(i)				0x5
#define NV_HEAD_STATE0_INTERLACED_SHIFT			4
#define NV_HEAD_STATE0_INTERLACED_DEFAULT_MASK		(3 << 4)
#define NV_HEAD_STATE0_INTERLACED_PROGRESSIVE		(0 << 4)
#define NV_HEAD_STATE0_INTERLACED_INTERLACED		(1 << 4)
#define NV_HEAD_STATE0_RANGECOMPRESS_SHIFT		3
#define NV_HEAD_STATE0_RANGECOMPRESS_DEFAULT_MASK	(1 << 3)
#define NV_HEAD_STATE0_RANGECOMPRESS_DISABLE		(0 << 3)
#define NV_HEAD_STATE0_RANGECOMPRESS_ENABLE		(1 << 3)
#define NV_HEAD_STATE0_DYNRANGE_SHIFT			2
#define NV_HEAD_STATE0_DYNRANGE_DEFAULT_MASK		(1 << 2)
#define NV_HEAD_STATE0_DYNRANGE_VESA			(0 << 2)
#define NV_HEAD_STATE0_DYNRANGE_CEA			(1 << 2)
#define NV_HEAD_STATE0_COLORSPACE_SHIFT			0
#define NV_HEAD_STATE0_COLORSPACE_DEFAULT_MASK		0x3
#define NV_HEAD_STATE0_COLORSPACE_RGB			0
#define NV_HEAD_STATE0_COLORSPACE_YUV_601		1
#define NV_HEAD_STATE0_COLORSPACE_YUV_709		2
#define NV_HEAD_STATE1(i)				(7 + i)
#define NV_HEAD_STATE1_VTOTAL_SHIFT			16
#define NV_HEAD_STATE1_VTOTAL_DEFAULT_MASK		(0x7fff << 16)
#define NV_HEAD_STATE1_HTOTAL_SHIFT			0
#define NV_HEAD_STATE1_HTOTAL_DEFAULT_MASK		0x7fff
#define NV_HEAD_STATE2(i)				(9 + i)
#define NV_HEAD_STATE2_VSYNC_END_SHIFT			16
#define NV_HEAD_STATE2_VSYNC_END_DEFAULT_MASK		(0x7fff << 16)
#define NV_HEAD_STATE2_HSYNC_END_SHIFT			0
#define NV_HEAD_STATE2_HSYNC_END_DEFAULT_MASK		0x7fff
#define NV_HEAD_STATE3(i)				(0xb + i)
#define NV_HEAD_STATE3_VBLANK_END_SHIFT			16
#define NV_HEAD_STATE3_VBLANK_END_DEFAULT_MASK		(0x7fff << 16)
#define NV_HEAD_STATE3_HBLANK_END_SHIFT			0
#define NV_HEAD_STATE3_HBLANK_END_DEFAULT_MASK		0x7fff
#define NV_HEAD_STATE4(i)				(0xd + i)
#define NV_HEAD_STATE4_VBLANK_START_SHIFT		16
#define NV_HEAD_STATE4_VBLANK_START_DEFAULT_MASK	(0x7fff << 16)
#define NV_HEAD_STATE4_HBLANK_START_SHIFT		0
#define NV_HEAD_STATE4_HBLANK_START_DEFAULT_MASK	0x7fff
#define NV_HEAD_STATE5(i)				(0xf + i)
#define CRC_CNTRL					0x11
#define CRC_CNTRL_ARM_CRC_ENABLE_SHIFT			0
#define CRC_CNTRL_ARM_CRC_ENABLE_NO			0
#define CRC_CNTRL_ARM_CRC_ENABLE_YES			1
#define CRC_CNTRL_ARM_CRC_ENABLE_DIS			0
#define CRC_CNTRL_ARM_CRC_ENABLE_EN			1
#define CLK_CNTRL					0x13
#define CLK_CNTRL_DP_CLK_SEL_SHIFT			0
#define CLK_CNTRL_DP_CLK_SEL_MASK			0x3
#define CLK_CNTRL_DP_CLK_SEL_SINGLE_PCLK		0
#define CLK_CNTRL_DP_CLK_SEL_DIFF_PCLK			1
#define CLK_CNTRL_DP_CLK_SEL_SINGLE_DPCLK		2
#define CLK_CNTRL_DP_CLK_SEL_DIFF_DPCLK			3
#define CLK_CNTRL_DP_LINK_SPEED_SHIFT			2
#define CLK_CNTRL_DP_LINK_SPEED_MASK			(0x1f << 2)
#define CLK_CNTRL_DP_LINK_SPEED_G1_62			(6 << 2)
#define CLK_CNTRL_DP_LINK_SPEED_G2_7			(10 << 2)
#define CLK_CNTRL_DP_LINK_SPEED_LVDS			(7 << 2)
#define CAP						0x14
#define CAP_DP_A_SHIFT					24
#define CAP_DP_A_DEFAULT_MASK				(1 << 24)
#define CAP_DP_A_FALSE					(0 << 24)
#define CAP_DP_A_TRUE					(1 << 24)
#define CAP_DP_B_SHIFT					25
#define CAP_DP_B_DEFAULT_MASK				(1 << 24)
#define CAP_DP_B_FALSE					(0 << 24)
#define CAP_DP_B_TRUE					(1 << 24)
#define PWR						0x15
#define PWR_SETTING_NEW_SHIFT				31
#define PWR_SETTING_NEW_DEFAULT_MASK			(1 << 31)
#define PWR_SETTING_NEW_DONE				(0 << 31)
#define PWR_SETTING_NEW_PENDING				(1 << 31)
#define PWR_SETTING_NEW_TRIGGER				(1 << 31)
#define PWR_MODE_SHIFT					28
#define PWR_MODE_DEFAULT_MASK				(1 << 28)
#define PWR_MODE_NORMAL					(0 << 28)
#define PWR_MODE_SAFE					(1 << 28)
#define PWR_HALT_DELAY_SHIFT				24
#define PWR_HALT_DELAY_DEFAULT_MASK			(1 << 24)
#define PWR_HALT_DELAY_DONE				(0 << 24)
#define PWR_HALT_DELAY_ACTIVE				(1 << 24)
#define PWR_SAFE_START_SHIFT				17
#define PWR_SAFE_START_DEFAULT_MASK			(1 << 17)
#define PWR_SAFE_START_NORMAL				(0 << 17)
#define PWR_SAFE_START_ALT				(1 << 17)
#define PWR_SAFE_STATE_SHIFT				16
#define PWR_SAFE_STATE_DEFAULT_MASK			(1 << 16)
#define PWR_SAFE_STATE_PD				(0 << 16)
#define PWR_SAFE_STATE_PU				(1 << 16)
#define PWR_NORMAL_START_SHIFT				1
#define PWR_NORMAL_START_DEFAULT_MASK			(1 << 1)
#define PWR_NORMAL_START_NORMAL				(0 << 16)
#define PWR_NORMAL_START_ALT				(1 << 16)
#define PWR_NORMAL_STATE_SHIFT				0
#define PWR_NORMAL_STATE_DEFAULT_MASK			0x1
#define PWR_NORMAL_STATE_PD				0
#define PWR_NORMAL_STATE_PU				1
#define TEST						0x16
#define TEST_TESTMUX_SHIFT				24
#define TEST_TESTMUX_DEFAULT_MASK			(0xff << 24)
#define TEST_TESTMUX_AVSS				(0 << 24)
#define TEST_TESTMUX_CLOCKIN				(2 << 24)
#define TEST_TESTMUX_PLL_VOL				(4 << 24)
#define TEST_TESTMUX_SLOWCLKINT				(8 << 24)
#define TEST_TESTMUX_AVDD				(16 << 24)
#define TEST_TESTMUX_VDDREG				(32 << 24)
#define TEST_TESTMUX_REGREF_VDDREG			(64 << 24)
#define TEST_TESTMUX_REGREF_AVDD			(128 << 24)
#define TEST_CRC_SHIFT					23
#define TEST_CRC_PRE_SERIALIZE				(0 << 23)
#define TEST_CRC_POST_DESERIALIZE			(1 << 23)
#define TEST_TPAT_SHIFT					20
#define TEST_TPAT_DEFAULT_MASK				(7 << 20)
#define TEST_TPAT_LO					(0 << 20)
#define TEST_TPAT_TDAT					(1 << 20)
#define TEST_TPAT_RAMP					(2 << 20)
#define TEST_TPAT_WALK					(3 << 20)
#define TEST_TPAT_MAXSTEP				(4 << 20)
#define TEST_TPAT_MINSTEP				(5 << 20)
#define TEST_DSRC_SHIFT					16
#define TEST_DSRC_DEFAULT_MASK				(3 << 16)
#define TEST_DSRC_NORMAL				(0 << 16)
#define TEST_DSRC_DEBUG					(1 << 16)
#define TEST_DSRC_TGEN					(2 << 16)
#define TEST_HEAD_NUMBER_SHIFT				12
#define TEST_HEAD_NUMBER_DEFAULT_MASK			(3 << 12)
#define TEST_HEAD_NUMBER_NONE				(0 << 12)
#define TEST_HEAD_NUMBER_HEAD0				(1 << 12)
#define TEST_HEAD_NUMBER_HEAD1				(2 << 12)
#define TEST_ATTACHED_SHIFT				10
#define TEST_ATTACHED_DEFAULT_MASK			(1  << 10)
#define TEST_ATTACHED_FALSE				(0 << 10)
#define TEST_ATTACHED_TRUE				(1 << 10)
#define TEST_ACT_HEAD_OPMODE_SHIFT			8
#define TEST_ACT_HEAD_OPMODE_DEFAULT_MASK		(3 << 8)
#define TEST_ACT_HEAD_OPMODE_SLEEP			(0 << 8)
#define TEST_ACT_HEAD_OPMODE_SNOOZE			(1 << 8)
#define TEST_ACT_HEAD_OPMODE_AWAKE			(2 << 8)
#define TEST_INVD_SHIFT					6
#define TEST_INVD_DISABLE				(0 << 6)
#define TEST_INVD_ENABLE				(1 << 6)
#define TEST_TEST_ENABLE_SHIFT				1
#define TEST_TEST_ENABLE_DISABLE			(0 << 1)
#define TEST_TEST_ENABLE_ENABLE				(1 << 1)
#define PLL0						0x17
#define PLL0_ICHPMP_SHFIT				24
#define PLL0_ICHPMP_DEFAULT_MASK			(0xf << 24)
#define PLL0_VCOCAP_SHIFT				8
#define PLL0_VCOCAP_DEFAULT_MASK			(0xf << 8)
#define PLL0_PLLREG_LEVEL_SHIFT				6
#define PLL0_PLLREG_LEVEL_DEFAULT_MASK			(3 << 6)
#define PLL0_PLLREG_LEVEL_V25				(0 << 6)
#define PLL0_PLLREG_LEVEL_V15				(1 << 6)
#define PLL0_PLLREG_LEVEL_V35				(2 << 6)
#define PLL0_PLLREG_LEVEL_V45				(3 << 6)
#define PLL0_PULLDOWN_SHIFT				5
#define PLL0_PULLDOWN_DEFAULT_MASK			(1 << 5)
#define PLL0_PULLDOWN_DISABLE				(0 << 5)
#define PLL0_PULLDOWN_ENABLE				(1 << 5)
#define PLL0_RESISTORSEL_SHIFT				4
#define PLL0_RESISTORSEL_DEFAULT_MASK			(1 << 4)
#define PLL0_RESISTORSEL_INT				(0 << 4)
#define PLL0_RESISTORSEL_EXT				(1 << 4)
#define PLL0_VCOPD_SHIFT				2
#define PLL0_VCOPD_MASK					(1 << 2)
#define PLL0_VCOPD_RESCIND				(0 << 2)
#define PLL0_VCOPD_ASSERT				(1 << 2)
#define PLL0_PWR_SHIFT					0
#define PLL0_PWR_MASK					1
#define PLL0_PWR_ON					0
#define PLL0_PWR_OFF					1
#define PLL1_TMDS_TERM_SHIFT				8
#define PLL1_TMDS_TERM_DISABLE				(0 << 8)
#define PLL1_TMDS_TERM_ENABLE				(1 << 8)
#define PLL1						0x18
#define PLL1_TERM_COMPOUT_SHIFT				15
#define PLL1_TERM_COMPOUT_LOW				(0 << 15)
#define PLL1_TERM_COMPOUT_HIGH				(1 << 15)
#define PLL2						0x19
#define PLL2_DCIR_PLL_RESET_SHIFT			0
#define PLL2_DCIR_PLL_RESET_OVERRIDE			(0 << 0)
#define PLL2_DCIR_PLL_RESET_ALLOW			(1 << 0)
#define PLL2_AUX1_SHIFT					17
#define PLL2_AUX1_SEQ_MASK				(1 << 17)
#define PLL2_AUX1_SEQ_PLLCAPPD_ALLOW			(0 << 17)
#define PLL2_AUX1_SEQ_PLLCAPPD_OVERRIDE			(1 << 17)
#define PLL2_AUX2_SHIFT					18
#define PLL2_AUX2_MASK					(1 << 18)
#define PLL2_AUX2_OVERRIDE_POWERDOWN			(0 << 18)
#define PLL2_AUX2_ALLOW_POWERDOWN			(1 << 18)
#define PLL2_AUX6_SHIFT					22
#define PLL2_AUX6_BANDGAP_POWERDOWN_MASK		(1 << 22)
#define PLL2_AUX6_BANDGAP_POWERDOWN_DISABLE		(0 << 22)
#define PLL2_AUX6_BANDGAP_POWERDOWN_ENABLE		(1 << 22)
#define PLL2_AUX7_SHIFT					23
#define PLL2_AUX7_PORT_POWERDOWN_MASK			(1 << 23)
#define PLL2_AUX7_PORT_POWERDOWN_DISABLE		(0 << 23)
#define PLL2_AUX7_PORT_POWERDOWN_ENABLE			(1 << 23)
#define PLL2_AUX8_SHIFT					24
#define PLL2_AUX8_SEQ_PLLCAPPD_ENFORCE_MASK		(1 << 24)
#define PLL2_AUX8_SEQ_PLLCAPPD_ENFORCE_DISABLE		(0 << 24)
#define PLL2_AUX8_SEQ_PLLCAPPD_ENFORCE_ENABLE		(1 << 24)
#define PLL2_AUX9_SHIFT					25
#define PLL2_AUX9_LVDSEN_ALLOW				(0 << 25)
#define PLL2_AUX9_LVDSEN_OVERRIDE			(1 << 25)
#define PLL3						0x1a
#define PLL3_PLLVDD_MODE_SHIFT				13
#define PLL3_PLLVDD_MODE_MASK				(1 << 13)
#define PLL3_PLLVDD_MODE_V1_8				(0 << 13)
#define PLL3_PLLVDD_MODE_V3_3				(1 << 13)
#define CSTM						0x1b
#define CSTM_ROTDAT_SHIFT				28
#define CSTM_ROTDAT_DEFAULT_MASK			(7 << 28)
#define CSTM_ROTCLK_SHIFT				24
#define CSTM_ROTCLK_DEFAULT_MASK			(0xf << 24)
#define CSTM_LVDS_EN_SHIFT				16
#define CSTM_LVDS_EN_DISABLE				(0 << 16)
#define CSTM_LVDS_EN_ENABLE				(1 << 16)
#define CSTM_LINKACTB_SHIFT				15
#define CSTM_LINKACTB_DISABLE				(0 << 15)
#define CSTM_LINKACTB_ENABLE				(1 << 15)
#define CSTM_LINKACTA_SHIFT				14
#define CSTM_LINKACTA_DISABLE				(0 << 14)
#define CSTM_LINKACTA_ENABLE				(1 << 14)
#define LVDS						0x1c
#define LVDS_ROTDAT_SHIFT				28
#define LVDS_ROTDAT_DEFAULT_MASK			(7 << 28)
#define LVDS_ROTDAT_RST					(0 << 28)
#define LVDS_ROTCLK_SHIFT				24
#define LVDS_ROTCLK_DEFAULT_MASK			(0xf << 24)
#define LVDS_ROTCLK_RST					(0 << 24)
#define LVDS_PLLDIV_SHIFT				21
#define LVDS_PLLDIV_DEFAULT_MASK			(1 << 21)
#define LVDS_PLLDIV_BY_7				(0 << 21)
#define LVDS_BALANCED_SHIFT				19
#define LVDS_BALANCED_DEFAULT_MASK			(1 << 19)
#define LVDS_BALANCED_DISABLE				(0 << 19)
#define LVDS_BALANCED_ENABLE				(1 << 19)
#define LVDS_NEW_MODE_SHIFT				18
#define LVDS_NEW_MODE_DEFAULT_MASK			(1 << 18)
#define LVDS_NEW_MODE_DISABLE				(0 << 18)
#define LVDS_NEW_MODE_ENABLE				(1 << 18)
#define LVDS_DUP_SYNC_SHIFT				17
#define LVDS_DUP_SYNC_DEFAULT_MASK			(1 << 17)
#define LVDS_DUP_SYNC_DISABLE				(0 << 17)
#define LVDS_DUP_SYNC_ENABLE				(1 << 17)
#define LVDS_LVDS_EN_SHIFT				16
#define LVDS_LVDS_EN_DEFAULT_MASK			(1 << 16)
#define LVDS_LVDS_EN_ENABLE				(1 << 16)
#define LVDS_LINKACTB_SHIFT				15
#define LVDS_LINKACTB_DEFAULT_MASK			(1 << 15)
#define LVDS_LINKACTB_DISABLE				(0 << 15)
#define LVDS_LINKACTB_ENABLE				(1 << 15)
#define LVDS_LINKACTA_SHIFT				14
#define LVDS_LINKACTA_DEFAULT_MASK			(1 << 14)
#define LVDS_LINKACTA_ENABLE				(1 << 14)
#define LVDS_MODE_SHIFT					12
#define LVDS_MODE_DEFAULT_MASK				(3 << 12)
#define LVDS_MODE_LVDS					(0 << 12)
#define LVDS_UPPER_SHIFT				11
#define LVDS_UPPER_DEFAULT_MASK				(1 << 11)
#define LVDS_UPPER_FALSE				(0 << 11)
#define LVDS_UPPER_TRUE					(1 << 11)
#define LVDS_PD_TXCB_SHIFT				9
#define LVDS_PD_TXCB_DEFAULT_MASK			(1 << 9)
#define LVDS_PD_TXCB_ENABLE				(0 << 9)
#define LVDS_PD_TXCB_DISABLE				(1 << 9)
#define LVDS_PD_TXCA_SHIFT				8
#define LVDS_PD_TXCA_DEFAULT_MASK			(1 << 8)
#define LVDS_PD_TXCA_ENABLE				(0 << 8)
#define LVDS_PD_TXDB_3_SHIFT				7
#define LVDS_PD_TXDB_3_DEFAULT_MASK			(1 << 7)
#define LVDS_PD_TXDB_3_ENABLE				(0 << 7)
#define LVDS_PD_TXDB_3_DISABLE				(1 << 7)
#define LVDS_PD_TXDB_2_SHIFT				6
#define LVDS_PD_TXDB_2_DEFAULT_MASK			(1 << 6)
#define LVDS_PD_TXDB_2_ENABLE				(0 << 6)
#define LVDS_PD_TXDB_2_DISABLE				(1 << 6)
#define LVDS_PD_TXDB_1_SHIFT				5
#define LVDS_PD_TXDB_1_DEFAULT_MASK			(1 << 5)
#define LVDS_PD_TXDB_1_ENABLE				(0 << 5)
#define LVDS_PD_TXDB_1_DISABLE				(1 << 5)
#define LVDS_PD_TXDB_0_SHIFT				4
#define LVDS_PD_TXDB_0_DEFAULT_MASK			(1 << 4)
#define LVDS_PD_TXDB_0_ENABLE				(0 << 4)
#define LVDS_PD_TXDB_0_DISABLE				(1 << 4)
#define LVDS_PD_TXDA_3_SHIFT				3
#define LVDS_PD_TXDA_3_DEFAULT_MASK			(1 << 3)
#define LVDS_PD_TXDA_3_ENABLE				(0 << 3)
#define LVDS_PD_TXDA_3_DISABLE				(1 << 3)
#define LVDS_PD_TXDA_2_SHIFT				2
#define LVDS_PD_TXDA_2_DEFAULT_MASK			(1 << 2)
#define LVDS_PD_TXDA_2_ENABLE				(0 << 2)
#define LVDS_PD_TXDA_1_SHIFT				1
#define LVDS_PD_TXDA_1_DEFAULT_MASK			(1 << 1)
#define LVDS_PD_TXDA_1_ENABLE				(0 << 1)
#define LVDS_PD_TXDA_0_SHIFT				0
#define LVDS_PD_TXDA_0_DEFAULT_MASK			0x1
#define LVDS_PD_TXDA_0_ENABLE				0
#define CRCA						0x1d
#define CRCA_VALID_FALSE				0
#define CRCA_VALID_TRUE					1
#define CRCA_VALID_RST					1
#define CRCB						0x1e
#define CRCB_CRC_DEFAULT_MASK				0xffffffff
#define SEQ_CTL						0x20
#define SEQ_CTL_SWITCH_SHIFT				30
#define SEQ_CTL_SWITCH_MASK				(1 << 30)
#define SEQ_CTL_SWITCH_WAIT				(0 << 30)
#define SEQ_CTL_SWITCH_FORCE				(1 << 30)
#define SEQ_CTL_STATUS_SHIFT				28
#define SEQ_CTL_STATUS_MASK				(1 << 28)
#define SEQ_CTL_STATUS_STOPPED				(0 << 28)
#define SEQ_CTL_STATUS_RUNNING				(1 << 28)
#define SEQ_CTL_PC_SHIFT				16
#define SEQ_CTL_PC_MASK					(0xf << 16)
#define SEQ_CTL_PD_PC_ALT_SHIFT				12
#define SEQ_CTL_PD_PC_ALT_MASK				(0xf << 12)
#define SEQ_CTL_PD_PC_SHIFT				8
#define SEQ_CTL_PD_PC_MASK				(0xf << 8)
#define SEQ_CTL_PU_PC_ALT_SHIFT				4
#define SEQ_CTL_PU_PC_ALT_MASK				(0xf << 4)
#define SEQ_CTL_PU_PC_SHIFT				0
#define SEQ_CTL_PU_PC_MASK				0xf
#define LANE_SEQ_CTL					0x21
#define LANE_SEQ_CTL_SETTING_NEW_SHIFT			31
#define LANE_SEQ_CTL_SETTING_MASK			(1 << 31)
#define LANE_SEQ_CTL_SETTING_NEW_DONE			(0 << 31)
#define LANE_SEQ_CTL_SETTING_NEW_PENDING		(1 << 31)
#define LANE_SEQ_CTL_SETTING_NEW_TRIGGER		(1 << 31)
#define LANE_SEQ_CTL_SEQ_STATE_SHIFT			28
#define LANE_SEQ_CTL_SEQ_STATE_IDLE			(0 << 28)
#define LANE_SEQ_CTL_SEQ_STATE_BUSY			(1 << 28)
#define LANE_SEQ_CTL_SEQUENCE_SHIFT			20
#define LANE_SEQ_CTL_SEQUENCE_UP			(0 << 20)
#define LANE_SEQ_CTL_SEQUENCE_DOWN			(1 << 20)
#define LANE_SEQ_CTL_NEW_POWER_STATE_SHIFT		16
#define LANE_SEQ_CTL_NEW_POWER_STATE_PU			(0 << 16)
#define LANE_SEQ_CTL_NEW_POWER_STATE_PD			(1 << 16)
#define LANE_SEQ_CTL_DELAY_SHIFT			12
#define LANE_SEQ_CTL_DELAY_DEFAULT_MASK			(0xf << 12)
#define LANE_SEQ_CTL_LANE9_STATE_SHIFT			9
#define LANE_SEQ_CTL_LANE9_STATE_POWERUP		(0 << 9)
#define LANE_SEQ_CTL_LANE9_STATE_POWERDOWN		(1 << 9)
#define LANE_SEQ_CTL_LANE8_STATE_SHIFT			8
#define LANE_SEQ_CTL_LANE8_STATE_POWERUP		(0 << 8)
#define LANE_SEQ_CTL_LANE8_STATE_POWERDOWN		(1 << 8)
#define LANE_SEQ_CTL_LANE7_STATE_SHIFT			7
#define LANE_SEQ_CTL_LANE7_STATE_POWERUP		(0 << 7)
#define LANE_SEQ_CTL_LANE7_STATE_POWERDOWN		(1 << 7)
#define LANE_SEQ_CTL_LANE6_STATE_SHIFT			6
#define LANE_SEQ_CTL_LANE6_STATE_POWERUP		(0 << 6)
#define LANE_SEQ_CTL_LANE6_STATE_POWERDOWN		(1 << 6)
#define LANE_SEQ_CTL_LANE5_STATE_SHIFT			5
#define LANE_SEQ_CTL_LANE5_STATE_POWERUP		(0 << 5)
#define LANE_SEQ_CTL_LANE5_STATE_POWERDOWN		(1 << 5)
#define LANE_SEQ_CTL_LANE4_STATE_SHIFT			4
#define LANE_SEQ_CTL_LANE4_STATE_POWERUP		(0 << 4)
#define LANE_SEQ_CTL_LANE4_STATE_POWERDOWN		(1 << 4)
#define LANE_SEQ_CTL_LANE3_STATE_SHIFT			3
#define LANE_SEQ_CTL_LANE3_STATE_POWERUP		(0 << 3)
#define LANE_SEQ_CTL_LANE3_STATE_POWERDOWN		(1 << 3)
#define LANE_SEQ_CTL_LANE2_STATE_SHIFT			2
#define LANE_SEQ_CTL_LANE2_STATE_POWERUP		(0 << 2)
#define LANE_SEQ_CTL_LANE2_STATE_POWERDOWN		(1 << 2)
#define LANE_SEQ_CTL_LANE1_STATE_SHIFT			1
#define LANE_SEQ_CTL_LANE1_STATE_POWERUP		(0 << 1)
#define LANE_SEQ_CTL_LANE1_STATE_POWERDOWN		(1 << 1)
#define LANE_SEQ_CTL_LANE0_STATE_SHIFT			0
#define LANE_SEQ_CTL_LANE0_STATE_POWERUP		0
#define LANE_SEQ_CTL_LANE0_STATE_POWERDOWN		1
#define SEQ_INST(i)					(0x22 + i)
#define SEQ_INST_PLL_PULLDOWN_SHIFT			31
#define SEQ_INST_PLL_PULLDOWN_DISABLE			(0 << 31)
#define SEQ_INST_PLL_PULLDOWN_ENABLE			(1 << 31)
#define SEQ_INST_POWERDOWN_MACRO_SHIFT			30
#define SEQ_INST_POWERDOWN_MACRO_NORMAL			(0 << 30)
#define SEQ_INST_POWERDOWN_MACRO_POWERDOWN		(1 << 30)
#define SEQ_INST_ASSERT_PLL_RESET_SHIFT			29
#define SEQ_INST_ASSERT_PLL_RESET_NORMAL		(0 << 29)
#define SEQ_INST_ASSERT_PLL_RESET_RST			(1 << 29)
#define SEQ_INST_BLANK_V_SHIFT				28
#define SEQ_INST_BLANK_V_NORMAL				(0 << 28)
#define SEQ_INST_BLANK_V_INACTIVE			(1 << 28)
#define SEQ_INST_BLANK_H_SHIFT				27
#define SEQ_INST_BLANK_H_NORMAL				(0 << 27)
#define SEQ_INST_BLANK_H_INACTIVE			(1 << 27)
#define SEQ_INST_BLANK_DE_SHIFT				26
#define SEQ_INST_BLANK_DE_NORMAL			(0 << 26)
#define SEQ_INST_BLANK_DE_INACTIVE			(1 << 26)
#define SEQ_INST_BLACK_DATA_SHIFT			25
#define SEQ_INST_BLACK_DATA_NORMAL			(0 << 25)
#define SEQ_INST_BLACK_DATA_BLACK			(1 << 25)
#define SEQ_INST_TRISTATE_IOS_SHIFT			24
#define SEQ_INST_TRISTATE_IOS_ENABLE_PINS		(0 << 24)
#define SEQ_INST_TRISTATE_IOS_TRISTATE			(1 << 24)
#define SEQ_INST_DRIVE_PWM_OUT_LO_SHIFT			23
#define SEQ_INST_DRIVE_PWM_OUT_LO_FALSE			(0 << 23)
#define SEQ_INST_DRIVE_PWM_OUT_LO_TRUE			(1 << 23)
#define SEQ_INST_PIN_B_SHIFT				22
#define SEQ_INST_PIN_B_LOW				(0 << 22)
#define SEQ_INST_PIN_B_HIGH				(1 << 22)
#define SEQ_INST_PIN_A_SHIFT				21
#define SEQ_INST_PIN_A_LOW				(0 << 21)
#define SEQ_INST_PIN_A_HIGH				(1 << 21)
#define SEQ_INST_SEQUENCE_SHIFT				19
#define SEQ_INST_SEQUENCE_UP				(0 << 19)
#define SEQ_INST_SEQUENCE_DOWN				(1 << 19)
#define SEQ_INST_LANE_SEQ_SHIFT				18
#define SEQ_INST_LANE_SEQ_STOP				(0 << 18)
#define SEQ_INST_LANE_SEQ_RUN				(1 << 18)
#define SEQ_INST_PDPORT_SHIFT				17
#define SEQ_INST_PDPORT_NO				(0 << 17)
#define SEQ_INST_PDPORT_YES				(1 << 17)
#define SEQ_INST_PDPLL_SHIFT				16
#define SEQ_INST_PDPLL_NO				(0 << 16)
#define SEQ_INST_PDPLL_YES				(1 << 16)
#define SEQ_INST_HALT_SHIFT				15
#define SEQ_INST_HALT_FALSE				(0 << 15)
#define SEQ_INST_HALT_TRUE				(1 << 15)
#define SEQ_INST_WAIT_UNITS_SHIFT			12
#define SEQ_INST_WAIT_UNITS_DEFAULT_MASK		(3 << 12)
#define SEQ_INST_WAIT_UNITS_US				(0 << 12)
#define SEQ_INST_WAIT_UNITS_MS				(1 << 12)
#define SEQ_INST_WAIT_UNITS_VSYNC			(2 << 12)
#define SEQ_INST_WAIT_TIME_SHIFT			0
#define SEQ_INST_WAIT_TIME_DEFAULT_MASK			0x3ff
#define PWM_DIV						0x32
#define PWM_DIV_DIVIDE_DEFAULT_MASK			0xffffff
#define PWM_CTL						0x33
#define PWM_CTL_SETTING_NEW_SHIFT			31
#define PWM_CTL_SETTING_NEW_DONE			(0 << 31)
#define PWM_CTL_SETTING_NEW_PENDING			(1 << 31)
#define PWM_CTL_SETTING_NEW_TRIGGER			(1 << 31)
#define PWM_CTL_CLKSEL_SHIFT				30
#define PWM_CTL_CLKSEL_PCLK				(0 << 30)
#define PWM_CTL_CLKSEL_XTAL				(1 << 30)
#define PWM_CTL_DUTY_CYCLE_SHIFT			0
#define PWM_CTL_DUTY_CYCLE_MASK				0xffffff
#define MSCHECK						0x49
#define MSCHECK_CTL_SHIFT				31
#define MSCHECK_CTL_CLEAR				(0 << 31)
#define MSCHECK_CTL_RUN					(1 << 31)
#define XBAR_CTRL					0x4a
#define DP_LINKCTL(i)					(0x4c + (i))
#define DP_LINKCTL_FORCE_IDLEPTTRN_SHIFT		31
#define DP_LINKCTL_FORCE_IDLEPTTRN_NO			(0 << 31)
#define DP_LINKCTL_FORCE_IDLEPTTRN_YES			(1 << 31)
#define DP_LINKCTL_COMPLIANCEPTTRN_SHIFT		28
#define DP_LINKCTL_COMPLIANCEPTTRN_NOPATTERN		(0 << 28)
#define DP_LINKCTL_COMPLIANCEPTTRN_COLORSQARE		(1 << 28)
#define DP_LINKCTL_LANECOUNT_SHIFT			16
#define DP_LINKCTL_LANECOUNT_MASK			(0x1f << 16)
#define DP_LINKCTL_LANECOUNT_ZERO			(0 << 16)
#define DP_LINKCTL_LANECOUNT_ONE			(1 << 16)
#define DP_LINKCTL_LANECOUNT_TWO			(3 << 16)
#define DP_LINKCTL_LANECOUNT_FOUR			(15 << 16)
#define DP_LINKCTL_ENHANCEDFRAME_SHIFT			14
#define DP_LINKCTL_ENHANCEDFRAME_DISABLE		(0 << 14)
#define DP_LINKCTL_ENHANCEDFRAME_ENABLE			(1 << 14)
#define DP_LINKCTL_SYNCMODE_SHIFT			10
#define DP_LINKCTL_SYNCMODE_DISABLE			(0 << 10)
#define DP_LINKCTL_SYNCMODE_ENABLE			(1 << 10)
#define DP_LINKCTL_TUSIZE_SHIFT				2
#define DP_LINKCTL_TUSIZE_MASK				(0x7f << 2)
#define DP_LINKCTL_ENABLE_SHIFT				0
#define DP_LINKCTL_ENABLE_NO				0
#define DP_LINKCTL_ENABLE_YES				1
#define DC(i)						(0x4e + (i))
#define DC_LANE3_DP_LANE3_SHIFT				24
#define DC_LANE3_DP_LANE3_MASK				(0xff << 24)
#define DC_LANE3_DP_LANE3_P0_LEVEL0			(17 << 24)
#define DC_LANE3_DP_LANE3_P1_LEVEL0			(21 << 24)
#define DC_LANE3_DP_LANE3_P2_LEVEL0			(26 << 24)
#define DC_LANE3_DP_LANE3_P3_LEVEL0			(34 << 24)
#define DC_LANE3_DP_LANE3_P0_LEVEL1			(26 << 24)
#define DC_LANE3_DP_LANE3_P1_LEVEL1			(32 << 24)
#define DC_LANE3_DP_LANE3_P2_LEVEL1			(39 << 24)
#define DC_LANE3_DP_LANE3_P0_LEVEL2			(34 << 24)
#define DC_LANE3_DP_LANE3_P1_LEVEL2			(43 << 24)
#define DC_LANE3_DP_LANE3_P0_LEVEL3			(51 << 24)
#define DC_LANE2_DP_LANE0_SHIFT				16
#define DC_LANE2_DP_LANE0_MASK				(0xff << 16)
#define DC_LANE2_DP_LANE0_P0_LEVEL0			(17 << 16)
#define DC_LANE2_DP_LANE0_P1_LEVEL0			(21 << 16)
#define DC_LANE2_DP_LANE0_P2_LEVEL0			(26 << 16)
#define DC_LANE2_DP_LANE0_P3_LEVEL0			(34 << 16)
#define DC_LANE2_DP_LANE0_P0_LEVEL1			(26 << 16)
#define DC_LANE2_DP_LANE0_P1_LEVEL1			(32 << 16)
#define DC_LANE2_DP_LANE0_P2_LEVEL1			(39 << 16)
#define DC_LANE2_DP_LANE0_P0_LEVEL2			(34 << 16)
#define DC_LANE2_DP_LANE0_P1_LEVEL2			(43 << 16)
#define DC_LANE2_DP_LANE0_P0_LEVEL3			(51 << 16)
#define DC_LANE1_DP_LANE1_SHIFT				8
#define DC_LANE1_DP_LANE1_MASK				(0xff << 8)
#define DC_LANE1_DP_LANE1_P0_LEVEL0			(17 << 8)
#define DC_LANE1_DP_LANE1_P1_LEVEL0			(21 << 8)
#define DC_LANE1_DP_LANE1_P2_LEVEL0			(26 << 8)
#define DC_LANE1_DP_LANE1_P3_LEVEL0			(34 << 8)
#define DC_LANE1_DP_LANE1_P0_LEVEL1			(26 << 8)
#define DC_LANE1_DP_LANE1_P1_LEVEL1			(32 << 8)
#define DC_LANE1_DP_LANE1_P2_LEVEL1			(39 << 8)
#define DC_LANE1_DP_LANE1_P0_LEVEL2			(34 << 8)
#define DC_LANE1_DP_LANE1_P1_LEVEL2			(43 << 8)
#define DC_LANE1_DP_LANE1_P0_LEVEL3			(51 << 8)
#define DC_LANE0_DP_LANE2_SHIFT				0
#define DC_LANE0_DP_LANE2_MASK				0xff
#define DC_LANE0_DP_LANE2_P0_LEVEL0			17
#define DC_LANE0_DP_LANE2_P1_LEVEL0			21
#define DC_LANE0_DP_LANE2_P2_LEVEL0			26
#define DC_LANE0_DP_LANE2_P3_LEVEL0			34
#define DC_LANE0_DP_LANE2_P0_LEVEL1			26
#define DC_LANE0_DP_LANE2_P1_LEVEL1			32
#define DC_LANE0_DP_LANE2_P2_LEVEL1			39
#define DC_LANE0_DP_LANE2_P0_LEVEL2			34
#define DC_LANE0_DP_LANE2_P1_LEVEL2			43
#define DC_LANE0_DP_LANE2_P0_LEVEL3			51
#define LANE_DRIVE_CURRENT(i)				(0x4e + (i))
#define PR(i)						(0x52 + (i))
#define PR_LANE3_DP_LANE3_SHIFT				24
#define PR_LANE3_DP_LANE3_MASK				(0xff << 24)
#define PR_LANE3_DP_LANE3_D0_LEVEL0			(0 << 24)
#define PR_LANE3_DP_LANE3_D1_LEVEL0			(0 << 24)
#define PR_LANE3_DP_LANE3_D2_LEVEL0			(0 << 24)
#define PR_LANE3_DP_LANE3_D3_LEVEL0			(0 << 24)
#define PR_LANE3_DP_LANE3_D0_LEVEL1			(4 << 24)
#define PR_LANE3_DP_LANE3_D1_LEVEL1			(6 << 24)
#define PR_LANE3_DP_LANE3_D2_LEVEL1			(17 << 24)
#define PR_LANE3_DP_LANE3_D0_LEVEL2			(8 << 24)
#define PR_LANE3_DP_LANE3_D1_LEVEL2			(13 << 24)
#define PR_LANE3_DP_LANE3_D0_LEVEL3			(17 << 24)
#define PR_LANE2_DP_LANE0_SHIFT				16
#define PR_LANE2_DP_LANE0_MASK				(0xff << 16)
#define PR_LANE2_DP_LANE0_D0_LEVEL0			(0 << 16)
#define PR_LANE2_DP_LANE0_D1_LEVEL0			(0 << 16)
#define PR_LANE2_DP_LANE0_D2_LEVEL0			(0 << 16)
#define PR_LANE2_DP_LANE0_D3_LEVEL0			(0 << 16)
#define PR_LANE2_DP_LANE0_D0_LEVEL1			(4 << 16)
#define PR_LANE2_DP_LANE0_D1_LEVEL1			(6 << 16)
#define PR_LANE2_DP_LANE0_D2_LEVEL1			(17 << 16)
#define PR_LANE2_DP_LANE0_D0_LEVEL2			(8 << 16)
#define PR_LANE2_DP_LANE0_D1_LEVEL2			(13 << 16)
#define PR_LANE2_DP_LANE0_D0_LEVEL3			(17 << 16)
#define PR_LANE1_DP_LANE1_SHIFT				8
#define PR_LANE1_DP_LANE1_MASK				(0xff >> 8)
#define PR_LANE1_DP_LANE1_D0_LEVEL0			(0 >> 8)
#define PR_LANE1_DP_LANE1_D1_LEVEL0			(0 >> 8)
#define PR_LANE1_DP_LANE1_D2_LEVEL0			(0 >> 8)
#define PR_LANE1_DP_LANE1_D3_LEVEL0			(0 >> 8)
#define PR_LANE1_DP_LANE1_D0_LEVEL1			(4 >> 8)
#define PR_LANE1_DP_LANE1_D1_LEVEL1			(6 >> 8)
#define PR_LANE1_DP_LANE1_D2_LEVEL1			(17 >> 8)
#define PR_LANE1_DP_LANE1_D0_LEVEL2			(8 >> 8)
#define PR_LANE1_DP_LANE1_D1_LEVEL2			(13 >> 8)
#define PR_LANE1_DP_LANE1_D0_LEVEL3			(17 >> 8)
#define PR_LANE0_DP_LANE2_SHIFT				0
#define PR_LANE0_DP_LANE2_MASK				0xff
#define PR_LANE0_DP_LANE2_D0_LEVEL0			0
#define PR_LANE0_DP_LANE2_D1_LEVEL0			0
#define PR_LANE0_DP_LANE2_D2_LEVEL0			0
#define PR_LANE0_DP_LANE2_D3_LEVEL0			0
#define PR_LANE0_DP_LANE2_D0_LEVEL1			4
#define PR_LANE0_DP_LANE2_D1_LEVEL1			6
#define PR_LANE0_DP_LANE2_D2_LEVEL1			17
#define PR_LANE0_DP_LANE2_D0_LEVEL2			8
#define PR_LANE0_DP_LANE2_D1_LEVEL2			13
#define PR_LANE0_DP_LANE2_D0_LEVEL3			17
#define LANE4_PREEMPHASIS(i)				(0x54 + (i))
#define POSTCURSOR(i)					(0x56 + (i))
#define DP_CONFIG(i)					(0x58 + (i))
#define DP_CONFIG_RD_RESET_VAL_SHIFT			31
#define DP_CONFIG_RD_RESET_VAL_POSITIVE			(0 << 31)
#define DP_CONFIG_RD_RESET_VAL_NEGATIVE			(1 << 31)
#define DP_CONFIG_IDLE_BEFORE_ATTACH_SHIFT		28
#define DP_CONFIG_IDLE_BEFORE_ATTACH_DISABLE		(0 << 28)
#define DP_CONFIG_IDLE_BEFORE_ATTACH_ENABLE		(1 << 28)
#define DP_CONFIG_ACTIVESYM_CNTL_SHIFT			26
#define DP_CONFIG_ACTIVESYM_CNTL_DISABLE		(0 << 26)
#define DP_CONFIG_ACTIVESYM_CNTL_ENABLE			(1 << 26)
#define DP_CONFIG_ACTIVESYM_POLARITY_SHIFT		24
#define DP_CONFIG_ACTIVESYM_POLARITY_NEGATIVE		(0 << 24)
#define DP_CONFIG_ACTIVESYM_POLARITY_POSITIVE		(1 << 24)
#define DP_CONFIG_ACTIVESYM_FRAC_SHIFT			16
#define DP_CONFIG_ACTIVESYM_FRAC_MASK			(0xf << 16)
#define DP_CONFIG_ACTIVESYM_COUNT_SHIFT			8
#define DP_CONFIG_ACTIVESYM_COUNT_MASK			(0x7f << 8)
#define DP_CONFIG_WATERMARK_SHIFT			0
#define DP_CONFIG_WATERMARK_MASK			0x3f
#define DP_MN(i)					(0x5a + i)
#define DP_MN_M_MOD_SHIFT				30
#define DP_MN_M_MOD_DEFAULT_MASK			(3 << 30)
#define DP_MN_M_MOD_NONE				(0 << 30)
#define DP_MN_M_MOD_INC					(1 << 30)
#define DP_MN_M_MOD_DEC					(2 << 30)
#define DP_MN_M_DELTA_SHIFT				24
#define DP_MN_M_DELTA_DEFAULT_MASK			(0xf << 24)
#define DP_MN_N_VAL_SHIFT				0
#define DP_MN_N_VAL_DEFAULT_MASK			0xffffff
#define DP_PADCTL(i)					(0x5c + (i))
#define DP_PADCTL_SPARE_SHIFT				25
#define DP_PADCTL_SPARE_DEFAULT_MASK			(0x7f << 25)
#define DP_PADCTL_VCO_2X_SHIFT				24
#define DP_PADCTL_VCO_2X_DISABLE			(0 << 24)
#define DP_PADCTL_VCO_2X_ENABLE				(1 << 24)
#define DP_PADCTL_PAD_CAL_PD_SHIFT			23
#define DP_PADCTL_PAD_CAL_PD_POWERUP			(0 << 23)
#define DP_PADCTL_PAD_CAL_PD_POWERDOWN			(1 << 23)
#define DP_PADCTL_TX_PU_SHIFT				22
#define DP_PADCTL_TX_PU_DISABLE				(0 << 22)
#define DP_PADCTL_TX_PU_ENABLE				(1 << 22)
#define DP_PADCTL_TX_PU_MASK				(1 << 22)
#define DP_PADCTL_REG_CTRL_SHIFT			20
#define DP_PADCTL_REG_CTRL_DEFAULT_MASK			(3 << 20)
#define DP_PADCTL_VCMMODE_SHIFT				16
#define DP_PADCTL_VCMMODE_DEFAULT_MASK			(0xf << 16)
#define DP_PADCTL_VCMMODE_TRISTATE			(0 << 16)
#define DP_PADCTL_VCMMODE_TEST_MUX			(1 << 16)
#define DP_PADCTL_VCMMODE_WEAK_PULLDOWN			(2 << 16)
#define DP_PADCTL_VCMMODE_STRONG_PULLDOWN		(4 << 16)
#define DP_PADCTL_TX_PU_VALUE_SHIFT			8
#define DP_PADCTL_TX_PU_VALUE_DEFAULT_MASK		(0xff << 8)
#define DP_PADCTL_COMODE_TXD_3_DP_TXD_3_SHIFT		7
#define DP_PADCTL_COMODE_TXD_3_DP_TXD_3_DISABLE		(0 << 7)
#define DP_PADCTL_COMODE_TXD_3_DP_TXD_3_ENABLE		(1 << 7)
#define DP_PADCTL_COMODE_TXD_2_DP_TXD_0_SHIFT		6
#define DP_PADCTL_COMODE_TXD_2_DP_TXD_0_DISABLE		(0 << 6)
#define DP_PADCTL_COMODE_TXD_2_DP_TXD_0_ENABLE		(1 << 6)
#define DP_PADCTL_COMODE_TXD_1_DP_TXD_1_SHIFT		5
#define DP_PADCTL_COMODE_TXD_1_DP_TXD_1_DISABLE		(0 << 5)
#define DP_PADCTL_COMODE_TXD_1_DP_TXD_1_ENABLE		(1 << 5)
#define DP_PADCTL_COMODE_TXD_0_DP_TXD_2_SHIFT		4
#define DP_PADCTL_COMODE_TXD_0_DP_TXD_2_DISABLE		(0 << 4)
#define DP_PADCTL_COMODE_TXD_0_DP_TXD_2_ENABLE		(1 << 4)
#define DP_PADCTL_PD_TXD_3_SHIFT			3
#define DP_PADCTL_PD_TXD_3_YES				(0 << 3)
#define DP_PADCTL_PD_TXD_3_NO				(1 << 3)
#define DP_PADCTL_PD_TXD_0_SHIFT			2
#define DP_PADCTL_PD_TXD_0_YES				(0 << 2)
#define DP_PADCTL_PD_TXD_0_NO				(1 << 2)
#define DP_PADCTL_PD_TXD_1_SHIFT			1
#define DP_PADCTL_PD_TXD_1_YES				(0 << 1)
#define DP_PADCTL_PD_TXD_1_NO				(1 << 1)
#define DP_PADCTL_PD_TXD_2_SHIFT			0
#define DP_PADCTL_PD_TXD_2_YES				0
#define DP_PADCTL_PD_TXD_2_NO				1
#define DP_DEBUG(i)					(0x5e + i)
#define DP_SPARE(i)					(0x60 + (i))
#define DP_SPARE_REG_SHIFT				3
#define DP_SPARE_REG_DEFAULT_MASK			(0x1fffffff << 3)
#define DP_SPARE_SOR_CLK_SEL_SHIFT			2
#define DP_SPARE_SOR_CLK_SEL_DEFAULT_MASK		(1 << 2)
#define DP_SPARE_SOR_CLK_SEL_SAFE_SORCLK		(0 << 2)
#define DP_SPARE_SOR_CLK_SEL_MACRO_SORCLK		(1 << 2)
#define DP_SPARE_PANEL_SHIFT				1
#define DP_SPARE_PANEL_EXTERNAL				(0 << 1)
#define DP_SPARE_PANEL_INTERNAL				(1 << 1)
#define DP_SPARE_SEQ_ENABLE_SHIFT			0
#define DP_SPARE_SEQ_ENABLE_NO				0
#define DP_SPARE_SEQ_ENABLE_YES				1
#define DP_AUDIO_CTRL					0x62
#define DP_AUDIO_HBLANK_SYMBOLS				0x63
#define DP_AUDIO_HBLANK_SYMBOLS_MASK			0x1ffff
#define DP_AUDIO_HBLANK_SYMBOLS_VALUE_SHIFT		0
#define DP_AUDIO_VBLANK_SYMBOLS				0x64
#define DP_AUDIO_VBLANK_SYMBOLS_MASK			0x1ffff
#define DP_AUDIO_VBLANK_SYMBOLS_SHIFT			0
#define DP_GENERIC_INFOFRAME_HEADER			0x65
#define DP_GENERIC_INFOFRAME_SUBPACK(i)			(0x66 + (i))
#define DP_TPG						0x6d
#define DP_TPG_LANE3_CHANNELCODING_SHIFT		30
#define DP_TPG_LANE3_CHANNELCODING_DISABLE		(0 << 30)
#define DP_TPG_LANE3_CHANNELCODING_ENABLE		(1 << 30)
#define DP_TPG_LANE3_SCRAMBLEREN_SHIFT			28
#define DP_TPG_LANE3_SCRAMBLEREN_ENABLE_GALIOS		(1 << 28)
#define DP_TPG_LANE3_SCRAMBLEREN_ENABLE_FIBONACCI	(2 << 28)
#define DP_TPG_LANE3_PATTERN_SHIFT			24
#define DP_TPG_LANE3_PATTERN_DEFAULT_MASK		(0xf << 24)
#define DP_TPG_LANE3_PATTERN_NOPATTERN			(0 << 24)
#define DP_TPG_LANE3_PATTERN_TRAINING1			(1 << 24)
#define DP_TPG_LANE3_PATTERN_TRAINING2			(2 << 24)
#define DP_TPG_LANE3_PATTERN_TRAINING3			(3 << 24)
#define DP_TPG_LANE3_PATTERN_D102			(4 << 24)
#define DP_TPG_LANE3_PATTERN_SBLERRRATE			(5 << 24)
#define DP_TPG_LANE3_PATTERN_PRBS7			(6 << 24)
#define DP_TPG_LANE3_PATTERN_CSTM			(7 << 24)
#define DP_TPG_LANE3_PATTERN_HBR2_COMPLIANCE		(8 << 24)
#define DP_TPG_LANE2_CHANNELCODING_SHIFT		22
#define DP_TPG_LANE2_CHANNELCODING_DISABLE		(0 << 22)
#define DP_TPG_LANE2_CHANNELCODING_ENABLE		(1 << 22)
#define DP_TPG_LANE2_SCRAMBLEREN_SHIFT			20
#define DP_TPG_LANE2_SCRAMBLEREN_DEFAULT_MASK		(3 << 20)
#define DP_TPG_LANE2_SCRAMBLEREN_DISABLE		(0 << 20)
#define DP_TPG_LANE2_SCRAMBLEREN_ENABLE_GALIOS		(1 << 20)
#define DP_TPG_LANE2_SCRAMBLEREN_ENABLE_FIBONACCI	(2 << 20)
#define DP_TPG_LANE2_PATTERN_SHIFT			16
#define DP_TPG_LANE2_PATTERN_DEFAULT_MASK		(0xf << 16)
#define DP_TPG_LANE2_PATTERN_NOPATTERN			(0 << 16)
#define DP_TPG_LANE2_PATTERN_TRAINING1			(1 << 16)
#define DP_TPG_LANE2_PATTERN_TRAINING2			(2 << 16)
#define DP_TPG_LANE2_PATTERN_TRAINING3			(3 << 16)
#define DP_TPG_LANE2_PATTERN_D102			(4 << 16)
#define DP_TPG_LANE2_PATTERN_SBLERRRATE			(5 << 16)
#define DP_TPG_LANE2_PATTERN_PRBS7			(6 << 16)
#define DP_TPG_LANE2_PATTERN_CSTM			(7 << 16)
#define DP_TPG_LANE2_PATTERN_HBR2_COMPLIANCE		(8 << 16)
#define DP_TPG_LANE1_CHANNELCODING_SHIFT		14
#define DP_TPG_LANE1_CHANNELCODING_DISABLE		(0 << 14)
#define DP_TPG_LANE1_CHANNELCODING_ENABLE		(1 << 14)
#define DP_TPG_LANE1_SCRAMBLEREN_SHIFT			12
#define DP_TPG_LANE1_SCRAMBLEREN_DEFAULT_MASK		(3 << 12)
#define DP_TPG_LANE1_SCRAMBLEREN_DISABLE		(0 << 12)
#define DP_TPG_LANE1_SCRAMBLEREN_ENABLE_GALIOS		(1 << 12)
#define DP_TPG_LANE1_SCRAMBLEREN_ENABLE_FIBONACCI	(2 << 12)
#define DP_TPG_LANE1_PATTERN_SHIFT			8
#define DP_TPG_LANE1_PATTERN_DEFAULT_MASK		(0xf << 8)
#define DP_TPG_LANE1_PATTERN_NOPATTERN			(0 << 8)
#define DP_TPG_LANE1_PATTERN_TRAINING1			(1 << 8)
#define DP_TPG_LANE1_PATTERN_TRAINING2			(2 << 8)
#define DP_TPG_LANE1_PATTERN_TRAINING3			(3 << 8)
#define DP_TPG_LANE1_PATTERN_D102			(4 << 8)
#define DP_TPG_LANE1_PATTERN_SBLERRRATE			(5 << 8)
#define DP_TPG_LANE1_PATTERN_PRBS7			(6 << 8)
#define DP_TPG_LANE1_PATTERN_CSTM			(7 << 8)
#define DP_TPG_LANE1_PATTERN_HBR2_COMPLIANCE		(8 << 8)
#define DP_TPG_LANE0_CHANNELCODING_SHIFT		6
#define DP_TPG_LANE0_CHANNELCODING_DISABLE		(0 << 6)
#define DP_TPG_LANE0_CHANNELCODING_ENABLE		(1 << 6)
#define DP_TPG_LANE0_SCRAMBLEREN_SHIFT			4
#define DP_TPG_LANE0_SCRAMBLEREN_DEFAULT_MASK		(3 << 4)
#define DP_TPG_LANE0_SCRAMBLEREN_DISABLE		(0 << 4)
#define DP_TPG_LANE0_SCRAMBLEREN_ENABLE_GALIOS		(1 << 4)
#define DP_TPG_LANE0_SCRAMBLEREN_ENABLE_FIBONACCI	(2 << 4)
#define DP_TPG_LANE0_PATTERN_SHIFT			0
#define DP_TPG_LANE0_PATTERN_DEFAULT_MASK		0xf
#define DP_TPG_LANE0_PATTERN_NOPATTERN			0
#define DP_TPG_LANE0_PATTERN_TRAINING1			1
#define DP_TPG_LANE0_PATTERN_TRAINING2			2
#define DP_TPG_LANE0_PATTERN_TRAINING3			3
#define DP_TPG_LANE0_PATTERN_D102			4
#define DP_TPG_LANE0_PATTERN_SBLERRRATE			5
#define DP_TPG_LANE0_PATTERN_PRBS7			6
#define DP_TPG_LANE0_PATTERN_CSTM			7
#define DP_TPG_LANE0_PATTERN_HBR2_COMPLIANCE		8

enum {
	training_pattern_disabled	= 0,
	training_pattern_1		= 1,
	training_pattern_2		= 2,
	training_pattern_3		= 3,
	training_pattern_none		= 0xff
};

enum tegra_dc_sor_protocol {
	SOR_DP,
	SOR_LVDS,
};

#define SOR_LINK_SPEED_G1_62	6
#define SOR_LINK_SPEED_G2_7	10
#define SOR_LINK_SPEED_G5_4	20
#define SOR_LINK_SPEED_LVDS	7

struct tegra_dp_link_config {
	int	is_valid;

	/* Supported configuration */
	u8	max_link_bw;
	u8	max_lane_count;
	int	downspread;
	int	support_enhanced_framing;
	u32	bits_per_pixel;
	int	alt_scramber_reset_cap; /* true for eDP */
	int	only_enhanced_framing;	/* enhanced_frame_en ignored */
	int	frame_in_ms;

	/* Actual configuration */
	u8	link_bw;
	u8	lane_count;
	int	enhanced_framing;
	int	scramble_ena;

	u32	activepolarity;
	u32	active_count;
	u32	tu_size;
	u32	active_frac;
	u32	watermark;

	s32	hblank_sym;
	s32	vblank_sym;

	/* Training data */
	u32	drive_current;
	u32     preemphasis;
	u32	postcursor;
	u8	aux_rd_interval;
	u8	tps3_supported;
};

#define TEGRA_SOR_TIMEOUT_MS		1000
#define TEGRA_SOR_ATTACH_TIMEOUT_MS	1000

int tegra_dc_sor_enable_dp(struct udevice *sor,
			   const struct tegra_dp_link_config *link_cfg);
int tegra_dc_sor_set_power_state(struct udevice *sor, int pu_pd);
void tegra_dc_sor_set_dp_linkctl(struct udevice *dev, int ena,
	u8 training_pattern, const struct tegra_dp_link_config *link_cfg);
void tegra_dc_sor_set_link_bandwidth(struct udevice *dev, u8 link_bw);
void tegra_dc_sor_set_lane_count(struct udevice *dev, u8 lane_count);
void tegra_dc_sor_set_panel_power(struct udevice *sor,
				  int power_up);
void tegra_dc_sor_set_internal_panel(struct udevice *dev, int is_int);
void tegra_dc_sor_read_link_config(struct udevice *dev, u8 *link_bw,
				   u8 *lane_count);
void tegra_dc_sor_set_lane_parm(struct udevice *dev,
		const struct tegra_dp_link_config *link_cfg);
void tegra_dc_sor_power_down_unused_lanes(struct udevice *sor,
			const struct tegra_dp_link_config *link_cfg);
int tegra_dc_sor_set_voltage_swing(struct udevice *sor,
				const struct tegra_dp_link_config *link_cfg);
int tegra_sor_precharge_lanes(struct udevice *dev,
			      const struct tegra_dp_link_config *cfg);
void tegra_dp_disable_tx_pu(struct udevice *sor);
void tegra_dp_set_pe_vs_pc(struct udevice *dev, u32 mask, u32 pe_reg,
			   u32 vs_reg, u32 pc_reg, u8 pc_supported);

int tegra_dc_sor_attach(struct udevice *dc_dev, struct udevice *sor,
			const struct tegra_dp_link_config *link_cfg,
			const struct display_timing *timing);
int tegra_dc_sor_detach(struct udevice *dc_dev, struct udevice *sor);

void tegra_dc_sor_disable_win_short_raster(struct dc_ctlr *disp_ctrl,
					   int *dc_reg_ctx);
int tegra_dc_sor_general_act(struct dc_ctlr *disp_ctrl);
void tegra_dc_sor_restore_win_and_raster(struct dc_ctlr *disp_ctrl,
					 int *dc_reg_ctx);

int tegra_dc_sor_init(struct udevice **sorp);
#endif
