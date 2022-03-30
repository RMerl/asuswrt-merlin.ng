/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#ifndef _DW_HDMI_H
#define _DW_HDMI_H

#include <edid.h>

#define HDMI_EDID_BLOCK_SIZE            128

/* Identification Registers */
#define HDMI_DESIGN_ID                          0x0000
#define HDMI_REVISION_ID                        0x0001
#define HDMI_PRODUCT_ID0                        0x0002
#define HDMI_PRODUCT_ID1                        0x0003
#define HDMI_CONFIG0_ID                         0x0004
#define HDMI_CONFIG1_ID                         0x0005
#define HDMI_CONFIG2_ID                         0x0006
#define HDMI_CONFIG3_ID                         0x0007

/* Interrupt Registers */
#define HDMI_IH_FC_STAT0                        0x0100
#define HDMI_IH_FC_STAT1                        0x0101
#define HDMI_IH_FC_STAT2                        0x0102
#define HDMI_IH_AS_STAT0                        0x0103
#define HDMI_IH_PHY_STAT0                       0x0104
#define HDMI_IH_I2CM_STAT0                      0x0105
#define HDMI_IH_CEC_STAT0                       0x0106
#define HDMI_IH_VP_STAT0                        0x0107
#define HDMI_IH_I2CMPHY_STAT0                   0x0108
#define HDMI_IH_AHBDMAAUD_STAT0                 0x0109

#define HDMI_IH_MUTE_FC_STAT0                   0x0180
#define HDMI_IH_MUTE_FC_STAT1                   0x0181
#define HDMI_IH_MUTE_FC_STAT2                   0x0182
#define HDMI_IH_MUTE_AS_STAT0                   0x0183
#define HDMI_IH_MUTE_PHY_STAT0                  0x0184
#define HDMI_IH_MUTE_I2CM_STAT0                 0x0185
#define HDMI_IH_MUTE_CEC_STAT0                  0x0186
#define HDMI_IH_MUTE_VP_STAT0                   0x0187
#define HDMI_IH_MUTE_I2CMPHY_STAT0              0x0188
#define HDMI_IH_MUTE_AHBDMAAUD_STAT0            0x0189
#define HDMI_IH_MUTE                            0x01FF

/* Video Sample Registers */
#define HDMI_TX_INVID0                          0x0200
#define HDMI_TX_INSTUFFING                      0x0201
#define HDMI_TX_GYDATA0                         0x0202
#define HDMI_TX_GYDATA1                         0x0203
#define HDMI_TX_RCRDATA0                        0x0204
#define HDMI_TX_RCRDATA1                        0x0205
#define HDMI_TX_BCBDATA0                        0x0206
#define HDMI_TX_BCBDATA1                        0x0207

/* Video Packetizer Registers */
#define HDMI_VP_STATUS                          0x0800
#define HDMI_VP_PR_CD                           0x0801
#define HDMI_VP_STUFF                           0x0802
#define HDMI_VP_REMAP                           0x0803
#define HDMI_VP_CONF                            0x0804
#define HDMI_VP_STAT                            0x0805
#define HDMI_VP_INT                             0x0806
#define HDMI_VP_MASK                            0x0807
#define HDMI_VP_POL                             0x0808

/* Frame Composer Registers */
#define HDMI_FC_INVIDCONF                       0x1000
#define HDMI_FC_INHACTV0                        0x1001
#define HDMI_FC_INHACTV1                        0x1002
#define HDMI_FC_INHBLANK0                       0x1003
#define HDMI_FC_INHBLANK1                       0x1004
#define HDMI_FC_INVACTV0                        0x1005
#define HDMI_FC_INVACTV1                        0x1006
#define HDMI_FC_INVBLANK                        0x1007
#define HDMI_FC_HSYNCINDELAY0                   0x1008
#define HDMI_FC_HSYNCINDELAY1                   0x1009
#define HDMI_FC_HSYNCINWIDTH0                   0x100A
#define HDMI_FC_HSYNCINWIDTH1                   0x100B
#define HDMI_FC_VSYNCINDELAY                    0x100C
#define HDMI_FC_VSYNCINWIDTH                    0x100D
#define HDMI_FC_INFREQ0                         0x100E
#define HDMI_FC_INFREQ1                         0x100F
#define HDMI_FC_INFREQ2                         0x1010
#define HDMI_FC_CTRLDUR                         0x1011
#define HDMI_FC_EXCTRLDUR                       0x1012
#define HDMI_FC_EXCTRLSPAC                      0x1013
#define HDMI_FC_CH0PREAM                        0x1014
#define HDMI_FC_CH1PREAM                        0x1015
#define HDMI_FC_CH2PREAM                        0x1016
#define HDMI_FC_AVICONF3                        0x1017
#define HDMI_FC_GCP                             0x1018
#define HDMI_FC_AVICONF0                        0x1019
#define HDMI_FC_AVICONF1                        0x101A
#define HDMI_FC_AVICONF2                        0x101B
#define HDMI_FC_AVIVID                          0x101C
#define HDMI_FC_AVIETB0                         0x101D
#define HDMI_FC_AVIETB1                         0x101E
#define HDMI_FC_AVISBB0                         0x101F
#define HDMI_FC_AVISBB1                         0x1020
#define HDMI_FC_AVIELB0                         0x1021
#define HDMI_FC_AVIELB1                         0x1022
#define HDMI_FC_AVISRB0                         0x1023
#define HDMI_FC_AVISRB1                         0x1024
#define HDMI_FC_AUDICONF0                       0x1025
#define HDMI_FC_AUDICONF1                       0x1026
#define HDMI_FC_AUDICONF2                       0x1027
#define HDMI_FC_AUDICONF3                       0x1028
#define HDMI_FC_VSDIEEEID0                      0x1029
#define HDMI_FC_VSDSIZE                         0x102A

/* HDMI Source PHY Registers */
#define HDMI_PHY_CONF0                          0x3000
#define HDMI_PHY_TST0                           0x3001
#define HDMI_PHY_TST1                           0x3002
#define HDMI_PHY_TST2                           0x3003
#define HDMI_PHY_STAT0                          0x3004
#define HDMI_PHY_INT0                           0x3005
#define HDMI_PHY_MASK0                          0x3006
#define HDMI_PHY_POL0                           0x3007

/* HDMI Master PHY Registers */
#define HDMI_PHY_I2CM_SLAVE_ADDR                0x3020
#define HDMI_PHY_I2CM_ADDRESS_ADDR              0x3021
#define HDMI_PHY_I2CM_DATAO_1_ADDR              0x3022
#define HDMI_PHY_I2CM_DATAO_0_ADDR              0x3023
#define HDMI_PHY_I2CM_DATAI_1_ADDR              0x3024
#define HDMI_PHY_I2CM_DATAI_0_ADDR              0x3025
#define HDMI_PHY_I2CM_OPERATION_ADDR            0x3026
#define HDMI_PHY_I2CM_INT_ADDR                  0x3027
#define HDMI_PHY_I2CM_CTLINT_ADDR               0x3028
#define HDMI_PHY_I2CM_DIV_ADDR                  0x3029
#define HDMI_PHY_I2CM_SOFTRSTZ_ADDR             0x302a
#define HDMI_PHY_I2CM_SS_SCL_HCNT_1_ADDR        0x302b
#define HDMI_PHY_I2CM_SS_SCL_HCNT_0_ADDR        0x302c
#define HDMI_PHY_I2CM_SS_SCL_LCNT_1_ADDR        0x302d
#define HDMI_PHY_I2CM_SS_SCL_LCNT_0_ADDR        0x302e
#define HDMI_PHY_I2CM_FS_SCL_HCNT_1_ADDR        0x302f
#define HDMI_PHY_I2CM_FS_SCL_HCNT_0_ADDR        0x3030
#define HDMI_PHY_I2CM_FS_SCL_LCNT_1_ADDR        0x3031
#define HDMI_PHY_I2CM_FS_SCL_LCNT_0_ADDR        0x3032

/* Audio Sampler Registers */
#define HDMI_AUD_CONF0                          0x3100
#define HDMI_AUD_CONF1                          0x3101
#define HDMI_AUD_INT                            0x3102
#define HDMI_AUD_CONF2                          0x3103
#define HDMI_AUD_INT1                           0x3104
#define HDMI_AUD_N1                             0x3200
#define HDMI_AUD_N2                             0x3201
#define HDMI_AUD_N3                             0x3202
#define HDMI_AUD_CTS1                           0x3203
#define HDMI_AUD_CTS2                           0x3204
#define HDMI_AUD_CTS3                           0x3205
#define HDMI_AUD_INPUTCLKFS                     0x3206
#define HDMI_AUD_SPDIFINT			0x3302
#define HDMI_AUD_CONF0_HBR                      0x3400
#define HDMI_AUD_HBR_STATUS                     0x3401
#define HDMI_AUD_HBR_INT                        0x3402
#define HDMI_AUD_HBR_POL                        0x3403
#define HDMI_AUD_HBR_MASK                       0x3404

/* Main Controller Registers */
#define HDMI_MC_SFRDIV                          0x4000
#define HDMI_MC_CLKDIS                          0x4001
#define HDMI_MC_SWRSTZ                          0x4002
#define HDMI_MC_OPCTRL                          0x4003
#define HDMI_MC_FLOWCTRL                        0x4004
#define HDMI_MC_PHYRSTZ                         0x4005
#define HDMI_MC_LOCKONCLOCK                     0x4006
#define HDMI_MC_HEACPHY_RST                     0x4007

/* Color Space  Converter Registers */
#define HDMI_CSC_CFG                            0x4100
#define HDMI_CSC_SCALE                          0x4101
#define HDMI_CSC_COEF_A1_MSB                    0x4102
#define HDMI_CSC_COEF_A1_LSB                    0x4103
#define HDMI_CSC_COEF_A2_MSB                    0x4104
#define HDMI_CSC_COEF_A2_LSB                    0x4105
#define HDMI_CSC_COEF_A3_MSB                    0x4106
#define HDMI_CSC_COEF_A3_LSB                    0x4107
#define HDMI_CSC_COEF_A4_MSB                    0x4108
#define HDMI_CSC_COEF_A4_LSB                    0x4109
#define HDMI_CSC_COEF_B1_MSB                    0x410A
#define HDMI_CSC_COEF_B1_LSB                    0x410B
#define HDMI_CSC_COEF_B2_MSB                    0x410C
#define HDMI_CSC_COEF_B2_LSB                    0x410D
#define HDMI_CSC_COEF_B3_MSB                    0x410E
#define HDMI_CSC_COEF_B3_LSB                    0x410F
#define HDMI_CSC_COEF_B4_MSB                    0x4110
#define HDMI_CSC_COEF_B4_LSB                    0x4111
#define HDMI_CSC_COEF_C1_MSB                    0x4112
#define HDMI_CSC_COEF_C1_LSB                    0x4113
#define HDMI_CSC_COEF_C2_MSB                    0x4114
#define HDMI_CSC_COEF_C2_LSB                    0x4115
#define HDMI_CSC_COEF_C3_MSB                    0x4116
#define HDMI_CSC_COEF_C3_LSB                    0x4117
#define HDMI_CSC_COEF_C4_MSB                    0x4118
#define HDMI_CSC_COEF_C4_LSB                    0x4119

/* I2C Master Registers (E-DDC) */
#define HDMI_I2CM_SLAVE                         0x7E00
#define HDMI_I2CM_ADDRESS                       0x7E01
#define HDMI_I2CM_DATAO                         0x7E02
#define HDMI_I2CM_DATAI                         0x7E03
#define HDMI_I2CM_OPERATION                     0x7E04
#define HDMI_I2CM_INT                           0x7E05
#define HDMI_I2CM_CTLINT                        0x7E06
#define HDMI_I2CM_DIV                           0x7E07
#define HDMI_I2CM_SEGADDR                       0x7E08
#define HDMI_I2CM_SOFTRSTZ                      0x7E09
#define HDMI_I2CM_SEGPTR                        0x7E0A
#define HDMI_I2CM_SS_SCL_HCNT_1_ADDR            0x7E0B
#define HDMI_I2CM_SS_SCL_HCNT_0_ADDR            0x7E0C
#define HDMI_I2CM_SS_SCL_LCNT_1_ADDR            0x7E0D
#define HDMI_I2CM_SS_SCL_LCNT_0_ADDR            0x7E0E
#define HDMI_I2CM_FS_SCL_HCNT_1_ADDR            0x7E0F
#define HDMI_I2CM_FS_SCL_HCNT_0_ADDR            0x7E10
#define HDMI_I2CM_FS_SCL_LCNT_1_ADDR            0x7E11
#define HDMI_I2CM_FS_SCL_LCNT_0_ADDR            0x7E12
#define HDMI_I2CM_BUF0                          0x7E20

enum {
	/* HDMI PHY registers define */
	PHY_OPMODE_PLLCFG = 0x06,
	PHY_CKCALCTRL = 0x05,
	PHY_CKSYMTXCTRL = 0x09,
	PHY_VLEVCTRL = 0x0e,
	PHY_PLLCURRCTRL = 0x10,
	PHY_PLLPHBYCTRL = 0x13,
	PHY_PLLGMPCTRL = 0x15,
	PHY_PLLCLKBISTPHASE = 0x17,
	PHY_TXTERM = 0x19,

	/* ih_phy_stat0 field values */
	HDMI_IH_PHY_STAT0_HPD = 0x1,

	/* ih_mute field values */
	HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT = 0x2,
	HDMI_IH_MUTE_MUTE_ALL_INTERRUPT = 0x1,

	/* tx_invid0 field values */
	HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE = 0x00,
	HDMI_TX_INVID0_VIDEO_MAPPING_MASK = 0x1f,
	HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET = 0,

	/* tx_instuffing field values */
	HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE = 0x4,
	HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE = 0x2,
	HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE = 0x1,

	/* vp_pr_cd field values */
	HDMI_VP_PR_CD_COLOR_DEPTH_MASK = 0xf0,
	HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET = 4,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK = 0x0f,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET = 0,

	/* vp_stuff field values */
	HDMI_VP_STUFF_IDEFAULT_PHASE_MASK = 0x20,
	HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET = 5,
	HDMI_VP_STUFF_YCC422_STUFFING_MASK = 0x4,
	HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE = 0x4,
	HDMI_VP_STUFF_PP_STUFFING_MASK = 0x2,
	HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE = 0x2,
	HDMI_VP_STUFF_PR_STUFFING_MASK = 0x1,
	HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE = 0x1,

	/* vp_conf field values */
	HDMI_VP_CONF_BYPASS_EN_MASK = 0x40,
	HDMI_VP_CONF_BYPASS_EN_ENABLE = 0x40,
	HDMI_VP_CONF_PP_EN_ENMASK = 0x20,
	HDMI_VP_CONF_PP_EN_DISABLE = 0x00,
	HDMI_VP_CONF_PR_EN_MASK = 0x10,
	HDMI_VP_CONF_PR_EN_DISABLE = 0x00,
	HDMI_VP_CONF_YCC422_EN_MASK = 0x8,
	HDMI_VP_CONF_YCC422_EN_DISABLE = 0x0,
	HDMI_VP_CONF_BYPASS_SELECT_MASK = 0x4,
	HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER = 0x4,
	HDMI_VP_CONF_OUTPUT_SELECTOR_MASK = 0x3,
	HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS = 0x3,

	/* vp_remap field values */
	HDMI_VP_REMAP_YCC422_16BIT = 0x0,

	/* fc_invidconf field values */
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_MASK = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_ACTIVE = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE = 0x00,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DVI_MODEZ_MASK = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE = 0x0,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW = 0x0,
	HDMI_FC_INVIDCONF_IN_I_P_MASK = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_INTERLACED = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE = 0x0,


	/* fc_aviconf0-fc_aviconf3 field values */
	HDMI_FC_AVICONF0_PIX_FMT_MASK = 0x03,
	HDMI_FC_AVICONF0_PIX_FMT_RGB = 0x00,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR422 = 0x01,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR444 = 0x02,
	HDMI_FC_AVICONF0_ACTIVE_FMT_MASK = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_INFO_PRESENT = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_NO_INFO = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_MASK = 0x0c,
	HDMI_FC_AVICONF0_BAR_DATA_NO_DATA = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_BAR = 0x04,
	HDMI_FC_AVICONF0_BAR_DATA_HORIZ_BAR = 0x08,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_HORIZ_BAR = 0x0c,
	HDMI_FC_AVICONF0_SCAN_INFO_MASK = 0x30,
	HDMI_FC_AVICONF0_SCAN_INFO_OVERSCAN = 0x10,
	HDMI_FC_AVICONF0_SCAN_INFO_UNDERSCAN = 0x20,
	HDMI_FC_AVICONF0_SCAN_INFO_NODATA = 0x00,

	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_MASK = 0x0f,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_USE_CODED = 0x08,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_4_3 = 0x09,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_16_9 = 0x0a,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_14_9 = 0x0b,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_MASK = 0x30,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_4_3 = 0x10,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_16_9 = 0x20,
	HDMI_FC_AVICONF1_COLORIMETRY_MASK = 0xc0,
	HDMI_FC_AVICONF1_COLORIMETRY_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_COLORIMETRY_SMPTE = 0x40,
	HDMI_FC_AVICONF1_COLORIMETRY_ITUR = 0x80,
	HDMI_FC_AVICONF1_COLORIMETRY_EXTENDED_INFO = 0xc0,

	HDMI_FC_AVICONF2_SCALING_MASK = 0x03,
	HDMI_FC_AVICONF2_SCALING_NONE = 0x00,
	HDMI_FC_AVICONF2_SCALING_HORIZ = 0x01,
	HDMI_FC_AVICONF2_SCALING_VERT = 0x02,
	HDMI_FC_AVICONF2_SCALING_HORIZ_vert = 0x03,
	HDMI_FC_AVICONF2_RGB_QUANT_MASK = 0x0c,
	HDMI_FC_AVICONF2_RGB_QUANT_DEFAULT = 0x00,
	HDMI_FC_AVICONF2_RGB_QUANT_LIMITED_RANGE = 0x04,
	HDMI_FC_AVICONF2_RGB_QUANT_FULL_RANGE = 0x08,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_MASK = 0x70,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601 = 0x00,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC709 = 0x10,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_SYCC601 = 0x20,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_YCC601 = 0x30,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_RGB = 0x40,
	HDMI_FC_AVICONF2_IT_CONTENT_MASK = 0x80,
	HDMI_FC_AVICONF2_IT_CONTENT_NO_DATA = 0x00,
	HDMI_FC_AVICONF2_IT_CONTENT_VALID = 0x80,

	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_MASK = 0x03,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GRAPHICS = 0x00,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_PHOTO = 0x01,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_CINEMA = 0x02,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GAME = 0x03,
	HDMI_FC_AVICONF3_QUANT_RANGE_MASK = 0x0c,
	HDMI_FC_AVICONF3_QUANT_RANGE_LIMITED = 0x00,
	HDMI_FC_AVICONF3_QUANT_RANGE_FULL = 0x04,

	/* fc_gcp field values*/
	HDMI_FC_GCP_SET_AVMUTE = 0x02,
	HDMI_FC_GCP_CLEAR_AVMUTE = 0x01,

	/* phy_conf0 field values */
	HDMI_PHY_CONF0_PDZ_MASK = 0x80,
	HDMI_PHY_CONF0_PDZ_OFFSET = 7,
	HDMI_PHY_CONF0_ENTMDS_MASK = 0x40,
	HDMI_PHY_CONF0_ENTMDS_OFFSET = 6,
	HDMI_PHY_CONF0_SPARECTRL_MASK = 0x20,
	HDMI_PHY_CONF0_SPARECTRL_OFFSET = 5,
	HDMI_PHY_CONF0_GEN2_PDDQ_MASK = 0x10,
	HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET = 4,
	HDMI_PHY_CONF0_GEN2_TXPWRON_MASK = 0x8,
	HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET = 3,
	HDMI_PHY_CONF0_SELDATAENPOL_MASK = 0x2,
	HDMI_PHY_CONF0_SELDATAENPOL_OFFSET = 1,
	HDMI_PHY_CONF0_SELDIPIF_MASK = 0x1,
	HDMI_PHY_CONF0_SELDIPIF_OFFSET = 0,

	/* phy_tst0 field values */
	HDMI_PHY_TST0_TSTCLR_MASK = 0x20,
	HDMI_PHY_TST0_TSTCLR_OFFSET = 5,

	/* phy_stat0 field values */
	HDMI_PHY_HPD = 0x02,
	HDMI_PHY_TX_PHY_LOCK = 0x01,

	/* phy_i2cm_slave_addr field values */
	HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2 = 0x69,

	/* phy_i2cm_operation_addr field values */
	HDMI_PHY_I2CM_OPERATION_ADDR_WRITE = 0x10,

	/* hdmi_phy_i2cm_int_addr */
	HDMI_PHY_I2CM_INT_ADDR_DONE_POL = 0x08,

	/* hdmi_phy_i2cm_ctlint_addr */
	HDMI_PHY_I2CM_CTLINT_ADDR_NAC_POL = 0x80,
	HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_POL = 0x08,

	/* aud_conf0 field values */
	HDMI_AUD_CONF0_SW_AUDIO_FIFO_RST = 0x80,
	HDMI_AUD_CONF0_I2S_SELECT = 0x20,
	HDMI_AUD_CONF0_I2S_IN_EN_0 = 0x01,
	HDMI_AUD_CONF0_I2S_IN_EN_1 = 0x02,
	HDMI_AUD_CONF0_I2S_IN_EN_2 = 0x04,
	HDMI_AUD_CONF0_I2S_IN_EN_3 = 0x08,

	/* aud_conf0 field values */
	HDMI_AUD_CONF1_I2S_MODE_STANDARD_MODE = 0x0,
	HDMI_AUD_CONF1_I2S_WIDTH_16BIT = 0x10,

	/* aud_n3 field values */
	HDMI_AUD_N3_NCTS_ATOMIC_WRITE = 0x80,
	HDMI_AUD_N3_AUDN19_16_MASK = 0x0f,

	/* aud_cts3 field values */
	HDMI_AUD_CTS3_N_SHIFT_OFFSET = 5,
	HDMI_AUD_CTS3_N_SHIFT_MASK = 0xe0,
	HDMI_AUD_CTS3_N_SHIFT_1 = 0,
	HDMI_AUD_CTS3_N_SHIFT_16 = 0x20,
	HDMI_AUD_CTS3_N_SHIFT_32 = 0x40,
	HDMI_AUD_CTS3_N_SHIFT_64 = 0x60,
	HDMI_AUD_CTS3_N_SHIFT_128 = 0x80,
	HDMI_AUD_CTS3_N_SHIFT_256 = 0xa0,
	HDMI_AUD_CTS3_CTS_MANUAL = 0x10,
	HDMI_AUD_CTS3_AUDCTS19_16_MASK = 0x0f,

	/* aud_inputclkfs filed values */
	HDMI_AUD_INPUTCLKFS_128 = 0x0,

	/* mc_clkdis field values */
	HDMI_MC_CLKDIS_HDCPCLK_DISABLE = 0x40,
	HDMI_MC_CLKDIS_CECCLK_DISABLE = 0x20,
	HDMI_MC_CLKDIS_CSCCLK_DISABLE = 0x10,
	HDMI_MC_CLKDIS_AUDCLK_DISABLE = 0x8,
	HDMI_MC_CLKDIS_PREPCLK_DISABLE = 0x4,
	HDMI_MC_CLKDIS_TMDSCLK_DISABLE = 0x2,
	HDMI_MC_CLKDIS_PIXELCLK_DISABLE = 0x1,

	/* mc_swrstz field values */
	HDMI_MC_SWRSTZ_II2SSWRST_REQ = 0x08,
	HDMI_MC_SWRSTZ_TMDSSWRST_REQ = 0x02,

	/* mc_flowctrl field values */
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH = 0x1,
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS = 0x0,

	/* mc_phyrstz field values */
	HDMI_MC_PHYRSTZ_ASSERT = 0x0,
	HDMI_MC_PHYRSTZ_DEASSERT = 0x1,

	/* mc_heacphy_rst field values */
	HDMI_MC_HEACPHY_RST_ASSERT = 0x1,

	/* i2cm filed values */
	HDMI_I2CM_SLAVE_DDC_ADDR = 0x50,
	HDMI_I2CM_SEGADDR_DDC = 0x30,
	HDMI_I2CM_OP_RD8_EXT = 0x2,
	HDMI_I2CM_OP_RD8 = 0x1,
	HDMI_I2CM_DIV_FAST_STD_MODE = 0x8,
	HDMI_I2CM_DIV_FAST_MODE = 0x8,
	HDMI_I2CM_DIV_STD_MODE = 0x0,
	HDMI_I2CM_SOFTRSTZ_MASK = 0x1,

	/* CSC_CFG field values */
	HDMI_CSC_CFG_INTMODE_MASK = 0x30,
	HDMI_CSC_CFG_INTMODE_OFFSET = 4,
	HDMI_CSC_CFG_INTMODE_DISABLE = 0x00,
	HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1 = 0x10,
	HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA2 = 0x20,
	HDMI_CSC_CFG_DECMODE_MASK = 0x3,
	HDMI_CSC_CFG_DECMODE_OFFSET = 0,
	HDMI_CSC_CFG_DECMODE_DISABLE = 0x0,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA1 = 0x1,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA2 = 0x2,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3 = 0x3,

	/* CSC_SCALE field values */
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK = 0xF0,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP = 0x00,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP = 0x50,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP = 0x60,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP = 0x70,
	HDMI_CSC_SCALE_CSCSCALE_MASK = 0x03,
};

struct hdmi_mpll_config {
	u64 mpixelclock;
	/* Mode of Operation and PLL Dividers Control Register */
	u32 cpce;
	/* PLL Gmp Control Register */
	u32 gmp;
	/* PLL Current Control Register */
	u32 curr;
};

struct hdmi_phy_config {
	u64 mpixelclock;
	u32 sym_ctr;    /* clock symbol and transmitter control */
	u32 term;       /* transmission termination value */
	u32 vlev_ctr;   /* voltage level control */
};

struct hdmi_vmode {
	bool mdataenablepolarity;

	unsigned int mpixelclock;
	unsigned int mpixelrepetitioninput;
	unsigned int mpixelrepetitionoutput;
};

struct hdmi_data_info {
	unsigned int enc_in_bus_format;
	unsigned int enc_out_bus_format;
	unsigned int enc_in_encoding;
	unsigned int enc_out_encoding;
	unsigned int pix_repet_factor;
	unsigned int hdcp_enable;
	struct hdmi_vmode video_mode;
};

struct dw_hdmi {
	ulong ioaddr;
	const struct hdmi_mpll_config *mpll_cfg;
	const struct hdmi_phy_config *phy_cfg;
	u8 i2c_clk_high;
	u8 i2c_clk_low;
	u8 reg_io_width;
	struct hdmi_data_info hdmi_data;

	int (*phy_set)(struct dw_hdmi *hdmi, uint mpixelclock);
	void (*write_reg)(struct dw_hdmi *hdmi, u8 val, int offset);
	u8 (*read_reg)(struct dw_hdmi *hdmi, int offset);
};

int dw_hdmi_phy_cfg(struct dw_hdmi *hdmi, uint mpixelclock);
int dw_hdmi_phy_wait_for_hpd(struct dw_hdmi *hdmi);
void dw_hdmi_phy_init(struct dw_hdmi *hdmi);

int dw_hdmi_enable(struct dw_hdmi *hdmi, const struct display_timing *edid);
int dw_hdmi_read_edid(struct dw_hdmi *hdmi, u8 *buf, int buf_size);
void dw_hdmi_init(struct dw_hdmi *hdmi);

#endif
