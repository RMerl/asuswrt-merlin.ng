/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#ifndef _ASM_ARCH_VOP_RK3288_H
#define _ASM_ARCH_VOP_RK3288_H

struct rk3288_vop {
	u32 reg_cfg_done;
	u32 version_info;
	u32 sys_ctrl;
	u32 sys_ctrl1;
	u32 dsp_ctrl0;
	u32 dsp_ctrl1;
	u32 dsp_bg;
	u32 mcu_ctrl;
	u32 intr_ctrl0;
	u32 intr_ctrl1;
	u32 intr_reserved0;
	u32 intr_reserved1;

	u32 win0_ctrl0;
	u32 win0_ctrl1;
	u32 win0_color_key;
	u32 win0_vir;
	u32 win0_yrgb_mst;
	u32 win0_cbr_mst;
	u32 win0_act_info;
	u32 win0_dsp_info;
	u32 win0_dsp_st;
	u32 win0_scl_factor_yrgb;
	u32 win0_scl_factor_cbr;
	u32 win0_scl_offset;
	u32 win0_src_alpha_ctrl;
	u32 win0_dst_alpha_ctrl;
	u32 win0_fading_ctrl;
	u32 win0_reserved0;

	u32 win1_ctrl0;
	u32 win1_ctrl1;
	u32 win1_color_key;
	u32 win1_vir;
	u32 win1_yrgb_mst;
	u32 win1_cbr_mst;
	u32 win1_act_info;
	u32 win1_dsp_info;
	u32 win1_dsp_st;
	u32 win1_scl_factor_yrgb;
	u32 win1_scl_factor_cbr;
	u32 win1_scl_offset;
	u32 win1_src_alpha_ctrl;
	u32 win1_dst_alpha_ctrl;
	u32 win1_fading_ctrl;
	u32 win1_reservd0;
	u32 reserved2[48];
	u32 post_dsp_hact_info;
	u32 post_dsp_vact_info;
	u32 post_scl_factor_yrgb;
	u32 post_reserved;
	u32 post_scl_ctrl;
	u32 post_dsp_vact_info_f1;
	u32 dsp_htotal_hs_end;
	u32 dsp_hact_st_end;
	u32 dsp_vtotal_vs_end;
	u32 dsp_vact_st_end;
	u32 dsp_vs_st_end_f1;
	u32 dsp_vact_st_end_f1;
};
check_member(rk3288_vop, dsp_vact_st_end_f1, 0x19c);

enum rockchip_fb_data_format_t {
	ARGB8888 = 0,
	RGB888 = 1,
	RGB565 = 2,
};

enum {
	LB_YUV_3840X5 = 0x0,
	LB_YUV_2560X8 = 0x1,
	LB_RGB_3840X2 = 0x2,
	LB_RGB_2560X4 = 0x3,
	LB_RGB_1920X5 = 0x4,
	LB_RGB_1280X8 = 0x5
};

enum vop_modes {
	VOP_MODE_EDP = 0,
	VOP_MODE_MIPI,
	VOP_MODE_HDMI,
	VOP_MODE_LVDS,
	VOP_MODE_DP,
};

/* VOP_VERSION_INFO */
#define M_FPGA_VERSION (0xffff << 16)
#define M_RTL_VERSION  (0xffff)

/* VOP_SYS_CTRL */
#define M_AUTO_GATING_EN (1 << 23)
#define M_STANDBY_EN     (1 << 22)
#define M_DMA_STOP       (1 << 21)
#define M_MMU_EN         (1 << 20)
#define M_DAM_BURST_LENGTH (0x3 << 18)
#define M_MIPI_OUT_EN	   (1 << 15)
#define M_EDP_OUT_EN       (1 << 14)
#define M_HDMI_OUT_EN      (1 << 13)
#define M_RGB_OUT_EN       (1 << 12)
#define M_ALL_OUT_EN		\
		(M_MIPI_OUT_EN | M_EDP_OUT_EN | M_HDMI_OUT_EN | M_RGB_OUT_EN)
#define M_EDPI_WMS_FS      (1 << 10)
#define M_EDPI_WMS_MODE    (1 << 9)
#define M_EDPI_HALT_EN     (1 << 8)
#define M_DOUB_CH_OVERLAP_NUM (0xf << 4)
#define M_DOUB_CHANNEL_EN     (1 << 3)
#define M_DIRECT_PATH_LAYER_SEL (0x3 << 1)
#define M_DIRECT_PATH_EN       (1)

#define V_AUTO_GATING_EN(x) (((x) & 1) << 23)
#define V_STANDBY_EN(x)     (((x) & 1) << 22)
#define V_DMA_STOP(x)       (((x) & 1) << 21)
#define V_MMU_EN(x)         (((x) & 1) << 20)
#define V_DMA_BURST_LENGTH(x) (((x) & 3) << 18)
#define V_MIPI_OUT_EN(x)      (((x) & 1) << 15)
#define V_EDP_OUT_EN(x)       (((x) & 1) << 14)
#define V_HDMI_OUT_EN(x)      (((x) & 1) << 13)
#define V_RGB_OUT_EN(x)       (((x) & 1) << 12)
#define V_EDPI_WMS_FS(x)      (((x) & 1) << 10)
#define V_EDPI_WMS_MODE(x)    (((x) & 1) << 9)
#define V_EDPI_HALT_EN(x)     (((x)&1)<<8)
#define V_DOUB_CH_OVERLAP_NUM(x) (((x) & 0xf) << 4)
#define V_DOUB_CHANNEL_EN(x)     (((x) & 1) << 3)
#define V_DIRECT_PATH_LAYER_SEL(x) (((x) & 3) << 1)
#define V_DIRECT_PATH_EN(x)       ((x) & 1)

/* VOP_SYS_CTRL1 */
#define M_AXI_OUTSTANDING_MAX_NUM (0x1f << 13)
#define M_AXI_MAX_OUTSTANDING_EN  (1 << 12)
#define M_NOC_WIN_QOS             (3 << 10)
#define M_NOC_QOS_EN              (1 << 9)
#define M_NOC_HURRY_THRESHOLD     (0x3f << 3)
#define M_NOC_HURRY_VALUE         (0x3 << 1)
#define M_NOC_HURRY_EN            (1)

#define V_AXI_OUTSTANDING_MAX_NUM(x) (((x) & 0x1f) << 13)
#define V_AXI_MAX_OUTSTANDING_EN(x)  (((x) & 1) << 12)
#define V_NOC_WIN_QOS(x)             (((x) & 3) << 10)
#define V_NOC_QOS_EN(x)              (((x) & 1) << 9)
#define V_NOC_HURRY_THRESHOLD(x)     (((x) & 0x3f) << 3)
#define V_NOC_HURRY_VALUE(x)         (((x) & 3) << 1)
#define V_NOC_HURRY_EN(x)            ((x) & 1)

/* VOP_DSP_CTRL0 */
#define M_DSP_Y_MIR_EN              (1 << 23)
#define M_DSP_X_MIR_EN              (1 << 22)
#define M_DSP_YUV_CLIP              (1 << 21)
#define M_DSP_CCIR656_AVG           (1 << 20)
#define M_DSP_BLACK_EN              (1 << 19)
#define M_DSP_BLANK_EN              (1 << 18)
#define M_DSP_OUT_ZERO              (1 << 17)
#define M_DSP_DUMMY_SWAP            (1 << 16)
#define M_DSP_DELTA_SWAP            (1 << 15)
#define M_DSP_RG_SWAP               (1 << 14)
#define M_DSP_RB_SWAP               (1 << 13)
#define M_DSP_BG_SWAP               (1 << 12)
#define M_DSP_FIELD_POL             (1 << 11)
#define M_DSP_INTERLACE             (1 << 10)
#define M_DSP_DDR_PHASE             (1 << 9)
#define M_DSP_DCLK_DDR              (1 << 8)
#define M_DSP_DCLK_POL              (1 << 7)
#define M_DSP_DEN_POL               (1 << 6)
#define M_DSP_VSYNC_POL             (1 << 5)
#define M_DSP_HSYNC_POL             (1 << 4)
#define M_DSP_OUT_MODE              (0xf)

#define V_DSP_Y_MIR_EN(x)              (((x) & 1) << 23)
#define V_DSP_X_MIR_EN(x)              (((x) & 1) << 22)
#define V_DSP_YUV_CLIP(x)              (((x) & 1) << 21)
#define V_DSP_CCIR656_AVG(x)           (((x) & 1) << 20)
#define V_DSP_BLACK_EN(x)              (((x) & 1) << 19)
#define V_DSP_BLANK_EN(x)              (((x) & 1) << 18)
#define V_DSP_OUT_ZERO(x)              (((x) & 1) << 17)
#define V_DSP_DUMMY_SWAP(x)            (((x) & 1) << 16)
#define V_DSP_DELTA_SWAP(x)            (((x) & 1) << 15)
#define V_DSP_RG_SWAP(x)               (((x) & 1) << 14)
#define V_DSP_RB_SWAP(x)               (((x) & 1) << 13)
#define V_DSP_BG_SWAP(x)               (((x) & 1) << 12)
#define V_DSP_FIELD_POL(x)             (((x) & 1) << 11)
#define V_DSP_INTERLACE(x)             (((x) & 1) << 10)
#define V_DSP_DDR_PHASE(x)             (((x) & 1) << 9)
#define V_DSP_DCLK_DDR(x)              (((x) & 1) << 8)
#define V_DSP_DCLK_POL(x)              (((x) & 1) << 7)
#define V_DSP_DEN_POL(x)               (((x) & 1) << 6)
#define V_DSP_VSYNC_POL(x)             (((x) & 1) << 5)
#define V_DSP_HSYNC_POL(x)             (((x) & 1) << 4)
#define V_DSP_PIN_POL(x)               (((x) & 0xf) << 4)
#define V_DSP_OUT_MODE(x)              ((x) & 0xf)

/* VOP_DSP_CTRL1 */
#define V_RK3399_DSP_MIPI_POL(x)       ((x) << 28)
#define V_RK3399_DSP_EDP_POL(x)        ((x) << 24)
#define V_RK3399_DSP_HDMI_POL(x)       ((x) << 20)
#define V_RK3399_DSP_LVDS_POL(x)       ((x) << 16)

#define M_RK3399_DSP_MIPI_POL          (V_RK3399_DSP_MIPI_POL(0xf))
#define M_RK3399_DSP_EDP_POL           (V_RK3399_DSP_EDP_POL(0xf))
#define M_RK3399_DSP_HDMI_POL          (V_RK3399_DSP_HDMI_POL(0xf))
#define M_RK3399_DSP_LVDS_POL          (V_RK3399_DSP_LVDS_POL(0xf))

#define M_DSP_LAYER3_SEL               (3 << 14)
#define M_DSP_LAYER2_SEL               (3 << 12)
#define M_DSP_LAYER1_SEL               (3 << 10)
#define M_DSP_LAYER0_SEL               (3 << 8)
#define M_DITHER_UP_EN                 (1 << 6)
#define M_DITHER_DOWN_SEL              (1 << 4)
#define M_DITHER_DOWN_MODE             (1 << 3)
#define M_DITHER_DOWN_EN               (1 << 2)
#define M_PRE_DITHER_DOWN_EN           (1 << 1)
#define M_DSP_LUT_EN                   (1)

#define V_DSP_LAYER3_SEL(x)                (((x) & 3) << 14)
#define V_DSP_LAYER2_SEL(x)                (((x) & 3) << 12)
#define V_DSP_LAYER1_SEL(x)                (((x) & 3) << 10)
#define V_DSP_LAYER0_SEL(x)                (((x) & 3) << 8)
#define V_DITHER_UP_EN(x)                  (((x) & 1) << 6)
#define V_DITHER_DOWN_SEL(x)               (((x) & 1) << 4)
#define V_DITHER_DOWN_MODE(x)              (((x) & 1) << 3)
#define V_DITHER_DOWN_EN(x)                (((x) & 1) << 2)
#define V_PRE_DITHER_DOWN_EN(x)            (((x) & 1) << 1)
#define V_DSP_LUT_EN(x)                    ((x)&1)

/* VOP_DSP_BG */
#define M_DSP_BG_RED     (0x3f << 20)
#define M_DSP_BG_GREEN   (0x3f << 10)
#define M_DSP_BG_BLUE    (0x3f << 0)

#define V_DSP_BG_RED(x)     (((x) & 0x3f) << 20)
#define V_DSP_BG_GREEN(x)   (((x) & 0x3f) << 10)
#define V_DSP_BG_BLUE(x)    (((x) & 0x3f) << 0)

/* VOP_WIN0_CTRL0 */
#define M_WIN0_YUV_CLIP     (1 << 20)
#define M_WIN0_CBR_DEFLICK  (1 << 19)
#define M_WIN0_YRGB_DEFLICK  (1 << 18)
#define M_WIN0_PPAS_ZERO_EN  (1 << 16)
#define M_WIN0_UV_SWAP       (1 << 15)
#define M_WIN0_MID_SWAP      (1 << 14)
#define M_WIN0_ALPHA_SWAP    (1 << 13)
#define M_WIN0_RB_SWAP       (1 << 12)
#define M_WIN0_CSC_MODE      (3 << 10)
#define M_WIN0_NO_OUTSTANDING (1 << 9)
#define M_WIN0_INTERLACE_READ  (1 << 8)
#define M_WIN0_LB_MODE         (7 << 5)
#define M_WIN0_FMT_10          (1 << 4)
#define M_WIN0_DATA_FMT        (7 << 1)
#define M_WIN0_EN              (1 << 0)

#define V_WIN0_YUV_CLIP(x)       (((x) & 1) << 20)
#define V_WIN0_CBR_DEFLICK(x)    (((x) & 1) << 19)
#define V_WIN0_YRGB_DEFLICK(x)   (((x) & 1) << 18)
#define V_WIN0_PPAS_ZERO_EN(x)   (((x) & 1) << 16)
#define V_WIN0_UV_SWAP(x)        (((x) & 1) << 15)
#define V_WIN0_MID_SWAP(x)       (((x) & 1) << 14)
#define V_WIN0_ALPHA_SWAP(x)     (((x) & 1) << 13)
#define V_WIN0_RB_SWAP(x)        (((x) & 1) << 12)
#define V_WIN0_CSC_MODE(x)       (((x) & 3) << 10)
#define V_WIN0_NO_OUTSTANDING(x) (((x) & 1) << 9)
#define V_WIN0_INTERLACE_READ(x)  (((x) & 1) << 8)
#define V_WIN0_LB_MODE(x)         (((x) & 7) << 5)
#define V_WIN0_FMT_10(x)          (((x) & 1) << 4)
#define V_WIN0_DATA_FMT(x)        (((x) & 7) << 1)
#define V_WIN0_EN(x)              ((x) & 1)

/* VOP_WIN0_CTRL1 */
#define M_WIN0_CBR_VSD_MODE        (1 << 31)
#define M_WIN0_CBR_VSU_MODE        (1 << 30)
#define M_WIN0_CBR_HSD_MODE        (3 << 28)
#define M_WIN0_CBR_VER_SCL_MODE    (3 << 26)
#define M_WIN0_CBR_HOR_SCL_MODE    (3 << 24)
#define M_WIN0_YRGB_VSD_MODE       (1 << 23)
#define M_WIN0_YRGB_VSU_MODE       (1 << 22)
#define M_WIN0_YRGB_HSD_MODE       (3 << 20)
#define M_WIN0_YRGB_VER_SCL_MODE   (3 << 18)
#define M_WIN0_YRGB_HOR_SCL_MODE   (3 << 16)
#define M_WIN0_LINE_LOAD_MODE      (1 << 15)
#define M_WIN0_CBR_AXI_GATHER_NUM  (7 << 12)
#define M_WIN0_YRGB_AXI_GATHER_NUM (0xf << 8)
#define M_WIN0_VSD_CBR_GT2         (1 << 7)
#define M_WIN0_VSD_CBR_GT4         (1 << 6)
#define M_WIN0_VSD_YRGB_GT2        (1 << 5)
#define M_WIN0_VSD_YRGB_GT4        (1 << 4)
#define M_WIN0_BIC_COE_SEL         (3 << 2)
#define M_WIN0_CBR_AXI_GATHER_EN   (1 << 1)
#define M_WIN0_YRGB_AXI_GATHER_EN  (1)

#define V_WIN0_CBR_VSD_MODE(x)        (((x) & 1) << 31)
#define V_WIN0_CBR_VSU_MODE(x)        (((x) & 1) << 30)
#define V_WIN0_CBR_HSD_MODE(x)        (((x) & 3) << 28)
#define V_WIN0_CBR_VER_SCL_MODE(x)    (((x) & 3) << 26)
#define V_WIN0_CBR_HOR_SCL_MODE(x)    (((x) & 3) << 24)
#define V_WIN0_YRGB_VSD_MODE(x)       (((x) & 1) << 23)
#define V_WIN0_YRGB_VSU_MODE(x)       (((x) & 1) << 22)
#define V_WIN0_YRGB_HSD_MODE(x)       (((x) & 3) << 20)
#define V_WIN0_YRGB_VER_SCL_MODE(x)   (((x) & 3) << 18)
#define V_WIN0_YRGB_HOR_SCL_MODE(x)   (((x) & 3) << 16)
#define V_WIN0_LINE_LOAD_MODE(x)      (((x) & 1) << 15)
#define V_WIN0_CBR_AXI_GATHER_NUM(x)  (((x) & 7) << 12)
#define V_WIN0_YRGB_AXI_GATHER_NUM(x) (((x) & 0xf) << 8)
#define V_WIN0_VSD_CBR_GT2(x)         (((x) & 1) << 7)
#define V_WIN0_VSD_CBR_GT4(x)         (((x) & 1) << 6)
#define V_WIN0_VSD_YRGB_GT2(x)        (((x) & 1) << 5)
#define V_WIN0_VSD_YRGB_GT4(x)        (((x) & 1) << 4)
#define V_WIN0_BIC_COE_SEL(x)         (((x) & 3) << 2)
#define V_WIN0_CBR_AXI_GATHER_EN(x)   (((x) & 1) << 1)
#define V_WIN0_YRGB_AXI_GATHER_EN(x)  ((x) & 1)

/*VOP_WIN0_COLOR_KEY*/
#define M_WIN0_KEY_EN                 (1 << 31)
#define M_WIN0_KEY_COLOR              (0x3fffffff)

#define V_WIN0_KEY_EN(x)              (((x) & 1) << 31)
#define V_WIN0_KEY_COLOR(x)           ((x) & 0x3fffffff)

/* VOP_WIN0_VIR */
#define V_ARGB888_VIRWIDTH(x)	(((x) & 0x3fff) << 0)
#define V_RGB888_VIRWIDTH(x)	(((((x * 3) >> 2)+((x) % 3)) & 0x3fff) << 0)
#define V_RGB565_VIRWIDTH(x)	(((x / 2) & 0x3fff) << 0)
#define YUV_VIRWIDTH(x)		(((x / 4) & 0x3fff) << 0)

/* VOP_WIN0_ACT_INFO */
#define V_ACT_HEIGHT(x)         (((x) & 0x1fff) << 16)
#define V_ACT_WIDTH(x)          ((x) & 0x1fff)

/* VOP_WIN0_DSP_INFO */
#define V_DSP_HEIGHT(x)         (((x) & 0xfff) << 16)
#define V_DSP_WIDTH(x)          ((x) & 0xfff)

/* VOP_WIN0_DSP_ST */
#define V_DSP_YST(x)            (((x) & 0x1fff) << 16)
#define V_DSP_XST(x)            ((x) & 0x1fff)

/* VOP_WIN0_SCL_OFFSET */
#define V_WIN0_VS_OFFSET_CBR(x)     (((x) & 0xff) << 24)
#define V_WIN0_VS_OFFSET_YRGB(x)    (((x) & 0xff) << 16)
#define V_WIN0_HS_OFFSET_CBR(x)     (((x) & 0xff) << 8)
#define V_WIN0_HS_OFFSET_YRGB(x)    ((x) & 0xff)

#define V_HSYNC(x)		(((x)&0x1fff)<<0)   /* hsync pulse width */
#define V_HORPRD(x)		(((x)&0x1fff)<<16)   /* horizontal period */
#define V_VSYNC(x)		(((x)&0x1fff)<<0)
#define V_VERPRD(x)		(((x)&0x1fff)<<16)

#define V_HEAP(x)		(((x)&0x1fff)<<0)/* horizontal active end */
#define V_HASP(x)		(((x)&0x1fff)<<16)/* horizontal active start */
#define V_VAEP(x)		(((x)&0x1fff)<<0)
#define V_VASP(x)		(((x)&0x1fff)<<16)

#endif
