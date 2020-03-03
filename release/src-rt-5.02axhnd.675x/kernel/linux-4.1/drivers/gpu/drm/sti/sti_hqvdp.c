/*
 * Copyright (C) STMicroelectronics SA 2014
 * Authors: Fabien Dessenne <fabien.dessenne@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/reset.h>

#include <drm/drmP.h>

#include "sti_drm_plane.h"
#include "sti_hqvdp.h"
#include "sti_hqvdp_lut.h"
#include "sti_layer.h"
#include "sti_vtg.h"

/* Firmware name */
#define HQVDP_FMW_NAME          "hqvdp-stih407.bin"

/* Regs address */
#define HQVDP_DMEM              0x00000000               /* 0x00000000 */
#define HQVDP_PMEM              0x00040000               /* 0x00040000 */
#define HQVDP_RD_PLUG           0x000E0000               /* 0x000E0000 */
#define HQVDP_RD_PLUG_CONTROL   (HQVDP_RD_PLUG + 0x1000) /* 0x000E1000 */
#define HQVDP_RD_PLUG_PAGE_SIZE (HQVDP_RD_PLUG + 0x1004) /* 0x000E1004 */
#define HQVDP_RD_PLUG_MIN_OPC   (HQVDP_RD_PLUG + 0x1008) /* 0x000E1008 */
#define HQVDP_RD_PLUG_MAX_OPC   (HQVDP_RD_PLUG + 0x100C) /* 0x000E100C */
#define HQVDP_RD_PLUG_MAX_CHK   (HQVDP_RD_PLUG + 0x1010) /* 0x000E1010 */
#define HQVDP_RD_PLUG_MAX_MSG   (HQVDP_RD_PLUG + 0x1014) /* 0x000E1014 */
#define HQVDP_RD_PLUG_MIN_SPACE (HQVDP_RD_PLUG + 0x1018) /* 0x000E1018 */
#define HQVDP_WR_PLUG           0x000E2000               /* 0x000E2000 */
#define HQVDP_WR_PLUG_CONTROL   (HQVDP_WR_PLUG + 0x1000) /* 0x000E3000 */
#define HQVDP_WR_PLUG_PAGE_SIZE (HQVDP_WR_PLUG + 0x1004) /* 0x000E3004 */
#define HQVDP_WR_PLUG_MIN_OPC   (HQVDP_WR_PLUG + 0x1008) /* 0x000E3008 */
#define HQVDP_WR_PLUG_MAX_OPC   (HQVDP_WR_PLUG + 0x100C) /* 0x000E300C */
#define HQVDP_WR_PLUG_MAX_CHK   (HQVDP_WR_PLUG + 0x1010) /* 0x000E3010 */
#define HQVDP_WR_PLUG_MAX_MSG   (HQVDP_WR_PLUG + 0x1014) /* 0x000E3014 */
#define HQVDP_WR_PLUG_MIN_SPACE (HQVDP_WR_PLUG + 0x1018) /* 0x000E3018 */
#define HQVDP_MBX               0x000E4000               /* 0x000E4000 */
#define HQVDP_MBX_IRQ_TO_XP70   (HQVDP_MBX + 0x0000)     /* 0x000E4000 */
#define HQVDP_MBX_INFO_HOST     (HQVDP_MBX + 0x0004)     /* 0x000E4004 */
#define HQVDP_MBX_IRQ_TO_HOST   (HQVDP_MBX + 0x0008)     /* 0x000E4008 */
#define HQVDP_MBX_INFO_XP70     (HQVDP_MBX + 0x000C)     /* 0x000E400C */
#define HQVDP_MBX_SW_RESET_CTRL (HQVDP_MBX + 0x0010)     /* 0x000E4010 */
#define HQVDP_MBX_STARTUP_CTRL1 (HQVDP_MBX + 0x0014)     /* 0x000E4014 */
#define HQVDP_MBX_STARTUP_CTRL2 (HQVDP_MBX + 0x0018)     /* 0x000E4018 */
#define HQVDP_MBX_GP_STATUS     (HQVDP_MBX + 0x001C)     /* 0x000E401C */
#define HQVDP_MBX_NEXT_CMD      (HQVDP_MBX + 0x0020)     /* 0x000E4020 */
#define HQVDP_MBX_CURRENT_CMD   (HQVDP_MBX + 0x0024)     /* 0x000E4024 */
#define HQVDP_MBX_SOFT_VSYNC    (HQVDP_MBX + 0x0028)     /* 0x000E4028 */

/* Plugs config */
#define PLUG_CONTROL_ENABLE     0x00000001
#define PLUG_PAGE_SIZE_256      0x00000002
#define PLUG_MIN_OPC_8          0x00000003
#define PLUG_MAX_OPC_64         0x00000006
#define PLUG_MAX_CHK_2X         0x00000001
#define PLUG_MAX_MSG_1X         0x00000000
#define PLUG_MIN_SPACE_1        0x00000000

/* SW reset CTRL */
#define SW_RESET_CTRL_FULL      BIT(0)
#define SW_RESET_CTRL_CORE      BIT(1)

/* Startup ctrl 1 */
#define STARTUP_CTRL1_RST_DONE  BIT(0)
#define STARTUP_CTRL1_AUTH_IDLE BIT(2)

/* Startup ctrl 2 */
#define STARTUP_CTRL2_FETCH_EN  BIT(1)

/* Info xP70 */
#define INFO_XP70_FW_READY      BIT(15)
#define INFO_XP70_FW_PROCESSING BIT(14)
#define INFO_XP70_FW_INITQUEUES BIT(13)

/* SOFT_VSYNC */
#define SOFT_VSYNC_HW           0x00000000
#define SOFT_VSYNC_SW_CMD       0x00000001
#define SOFT_VSYNC_SW_CTRL_IRQ  0x00000003

/* Reset & boot poll config */
#define POLL_MAX_ATTEMPT        50
#define POLL_DELAY_MS           20

#define SCALE_FACTOR            8192
#define SCALE_MAX_FOR_LEG_LUT_F 4096
#define SCALE_MAX_FOR_LEG_LUT_E 4915
#define SCALE_MAX_FOR_LEG_LUT_D 6654
#define SCALE_MAX_FOR_LEG_LUT_C 8192

enum sti_hvsrc_orient {
	HVSRC_HORI,
	HVSRC_VERT
};

/* Command structures */
struct sti_hqvdp_top {
	u32 config;
	u32 mem_format;
	u32 current_luma;
	u32 current_enh_luma;
	u32 current_right_luma;
	u32 current_enh_right_luma;
	u32 current_chroma;
	u32 current_enh_chroma;
	u32 current_right_chroma;
	u32 current_enh_right_chroma;
	u32 output_luma;
	u32 output_chroma;
	u32 luma_src_pitch;
	u32 luma_enh_src_pitch;
	u32 luma_right_src_pitch;
	u32 luma_enh_right_src_pitch;
	u32 chroma_src_pitch;
	u32 chroma_enh_src_pitch;
	u32 chroma_right_src_pitch;
	u32 chroma_enh_right_src_pitch;
	u32 luma_processed_pitch;
	u32 chroma_processed_pitch;
	u32 input_frame_size;
	u32 input_viewport_ori;
	u32 input_viewport_ori_right;
	u32 input_viewport_size;
	u32 left_view_border_width;
	u32 right_view_border_width;
	u32 left_view_3d_offset_width;
	u32 right_view_3d_offset_width;
	u32 side_stripe_color;
	u32 crc_reset_ctrl;
};

/* Configs for interlaced : no IT, no pass thru, 3 fields */
#define TOP_CONFIG_INTER_BTM            0x00000000
#define TOP_CONFIG_INTER_TOP            0x00000002

/* Config for progressive : no IT, no pass thru, 3 fields */
#define TOP_CONFIG_PROGRESSIVE          0x00000001

/* Default MemFormat: in=420_raster_dual out=444_raster;opaque Mem2Tv mode */
#define TOP_MEM_FORMAT_DFLT             0x00018060

/* Min/Max size */
#define MAX_WIDTH                       0x1FFF
#define MAX_HEIGHT                      0x0FFF
#define MIN_WIDTH                       0x0030
#define MIN_HEIGHT                      0x0010

struct sti_hqvdp_vc1re {
	u32 ctrl_prv_csdi;
	u32 ctrl_cur_csdi;
	u32 ctrl_nxt_csdi;
	u32 ctrl_cur_fmd;
	u32 ctrl_nxt_fmd;
};

struct sti_hqvdp_fmd {
	u32 config;
	u32 viewport_ori;
	u32 viewport_size;
	u32 next_next_luma;
	u32 next_next_right_luma;
	u32 next_next_next_luma;
	u32 next_next_next_right_luma;
	u32 threshold_scd;
	u32 threshold_rfd;
	u32 threshold_move;
	u32 threshold_cfd;
};

struct sti_hqvdp_csdi {
	u32 config;
	u32 config2;
	u32 dcdi_config;
	u32 prev_luma;
	u32 prev_enh_luma;
	u32 prev_right_luma;
	u32 prev_enh_right_luma;
	u32 next_luma;
	u32 next_enh_luma;
	u32 next_right_luma;
	u32 next_enh_right_luma;
	u32 prev_chroma;
	u32 prev_enh_chroma;
	u32 prev_right_chroma;
	u32 prev_enh_right_chroma;
	u32 next_chroma;
	u32 next_enh_chroma;
	u32 next_right_chroma;
	u32 next_enh_right_chroma;
	u32 prev_motion;
	u32 prev_right_motion;
	u32 cur_motion;
	u32 cur_right_motion;
	u32 next_motion;
	u32 next_right_motion;
};

/* Config for progressive: by pass */
#define CSDI_CONFIG_PROG                0x00000000
/* Config for directional deinterlacing without motion */
#define CSDI_CONFIG_INTER_DIR           0x00000016
/* Additional configs for fader, blender, motion,... deinterlace algorithms */
#define CSDI_CONFIG2_DFLT               0x000001B3
#define CSDI_DCDI_CONFIG_DFLT           0x00203803

struct sti_hqvdp_hvsrc {
	u32 hor_panoramic_ctrl;
	u32 output_picture_size;
	u32 init_horizontal;
	u32 init_vertical;
	u32 param_ctrl;
	u32 yh_coef[NB_COEF];
	u32 ch_coef[NB_COEF];
	u32 yv_coef[NB_COEF];
	u32 cv_coef[NB_COEF];
	u32 hori_shift;
	u32 vert_shift;
};

/* Default ParamCtrl: all controls enabled */
#define HVSRC_PARAM_CTRL_DFLT           0xFFFFFFFF

struct sti_hqvdp_iqi {
	u32 config;
	u32 demo_wind_size;
	u32 pk_config;
	u32 coeff0_coeff1;
	u32 coeff2_coeff3;
	u32 coeff4;
	u32 pk_lut;
	u32 pk_gain;
	u32 pk_coring_level;
	u32 cti_config;
	u32 le_config;
	u32 le_lut[64];
	u32 con_bri;
	u32 sat_gain;
	u32 pxf_conf;
	u32 default_color;
};

/* Default Config : IQI bypassed */
#define IQI_CONFIG_DFLT                 0x00000001
/* Default Contrast & Brightness gain = 256 */
#define IQI_CON_BRI_DFLT                0x00000100
/* Default Saturation gain = 256 */
#define IQI_SAT_GAIN_DFLT               0x00000100
/* Default PxfConf : P2I bypassed */
#define IQI_PXF_CONF_DFLT               0x00000001

struct sti_hqvdp_top_status {
	u32 processing_time;
	u32 input_y_crc;
	u32 input_uv_crc;
};

struct sti_hqvdp_fmd_status {
	u32 fmd_repeat_move_status;
	u32 fmd_scene_count_status;
	u32 cfd_sum;
	u32 field_sum;
	u32 next_y_fmd_crc;
	u32 next_next_y_fmd_crc;
	u32 next_next_next_y_fmd_crc;
};

struct sti_hqvdp_csdi_status {
	u32 prev_y_csdi_crc;
	u32 cur_y_csdi_crc;
	u32 next_y_csdi_crc;
	u32 prev_uv_csdi_crc;
	u32 cur_uv_csdi_crc;
	u32 next_uv_csdi_crc;
	u32 y_csdi_crc;
	u32 uv_csdi_crc;
	u32 uv_cup_crc;
	u32 mot_csdi_crc;
	u32 mot_cur_csdi_crc;
	u32 mot_prev_csdi_crc;
};

struct sti_hqvdp_hvsrc_status {
	u32 y_hvsrc_crc;
	u32 u_hvsrc_crc;
	u32 v_hvsrc_crc;
};

struct sti_hqvdp_iqi_status {
	u32 pxf_it_status;
	u32 y_iqi_crc;
	u32 u_iqi_crc;
	u32 v_iqi_crc;
};

/* Main commands. We use 2 commands one being processed by the firmware, one
 * ready to be fetched upon next Vsync*/
#define NB_VDP_CMD	2

struct sti_hqvdp_cmd {
	struct sti_hqvdp_top top;
	struct sti_hqvdp_vc1re vc1re;
	struct sti_hqvdp_fmd fmd;
	struct sti_hqvdp_csdi csdi;
	struct sti_hqvdp_hvsrc hvsrc;
	struct sti_hqvdp_iqi iqi;
	struct sti_hqvdp_top_status top_status;
	struct sti_hqvdp_fmd_status fmd_status;
	struct sti_hqvdp_csdi_status csdi_status;
	struct sti_hqvdp_hvsrc_status hvsrc_status;
	struct sti_hqvdp_iqi_status iqi_status;
};

/*
 * STI HQVDP structure
 *
 * @dev:               driver device
 * @drm_dev:           the drm device
 * @regs:              registers
 * @layer:             layer structure for hqvdp it self
 * @vid_plane:         VID plug used as link with compositor IP
 * @clk:               IP clock
 * @clk_pix_main:      pix main clock
 * @reset:             reset control
 * @vtg_nb:            notifier to handle VTG Vsync
 * @btm_field_pending: is there any bottom field (interlaced frame) to display
 * @curr_field_count:  number of field updates
 * @last_field_count:  number of field updates since last fps measure
 * @hqvdp_cmd:         buffer of commands
 * @hqvdp_cmd_paddr:   physical address of hqvdp_cmd
 * @vtg:               vtg for main data path
 */
struct sti_hqvdp {
	struct device *dev;
	struct drm_device *drm_dev;
	void __iomem *regs;
	struct sti_layer layer;
	struct drm_plane *vid_plane;
	struct clk *clk;
	struct clk *clk_pix_main;
	struct reset_control *reset;
	struct notifier_block vtg_nb;
	bool btm_field_pending;
	unsigned int curr_field_count;
	unsigned int last_field_count;
	void *hqvdp_cmd;
	dma_addr_t hqvdp_cmd_paddr;
	struct sti_vtg *vtg;
};

#define to_sti_hqvdp(x) container_of(x, struct sti_hqvdp, layer)

static const uint32_t hqvdp_supported_formats[] = {
	DRM_FORMAT_NV12,
};

static const uint32_t *sti_hqvdp_get_formats(struct sti_layer *layer)
{
	return hqvdp_supported_formats;
}

static unsigned int sti_hqvdp_get_nb_formats(struct sti_layer *layer)
{
	return ARRAY_SIZE(hqvdp_supported_formats);
}

/**
 * sti_hqvdp_get_free_cmd
 * @hqvdp: hqvdp structure
 *
 * Look for a hqvdp_cmd that is not being used (or about to be used) by the FW.
 *
 * RETURNS:
 * the offset of the command to be used.
 * -1 in error cases
 */
static int sti_hqvdp_get_free_cmd(struct sti_hqvdp *hqvdp)
{
	int curr_cmd, next_cmd;
	dma_addr_t cmd = hqvdp->hqvdp_cmd_paddr;
	int i;

	curr_cmd = readl(hqvdp->regs + HQVDP_MBX_CURRENT_CMD);
	next_cmd = readl(hqvdp->regs + HQVDP_MBX_NEXT_CMD);

	for (i = 0; i < NB_VDP_CMD; i++) {
		if ((cmd != curr_cmd) && (cmd != next_cmd))
			return i * sizeof(struct sti_hqvdp_cmd);
		cmd += sizeof(struct sti_hqvdp_cmd);
	}

	return -1;
}

/**
 * sti_hqvdp_get_curr_cmd
 * @hqvdp: hqvdp structure
 *
 * Look for the hqvdp_cmd that is being used by the FW.
 *
 * RETURNS:
 *  the offset of the command to be used.
 * -1 in error cases
 */
static int sti_hqvdp_get_curr_cmd(struct sti_hqvdp *hqvdp)
{
	int curr_cmd;
	dma_addr_t cmd = hqvdp->hqvdp_cmd_paddr;
	unsigned int i;

	curr_cmd = readl(hqvdp->regs + HQVDP_MBX_CURRENT_CMD);

	for (i = 0; i < NB_VDP_CMD; i++) {
		if (cmd == curr_cmd)
			return i * sizeof(struct sti_hqvdp_cmd);

		cmd += sizeof(struct sti_hqvdp_cmd);
	}

	return -1;
}

/**
 * sti_hqvdp_update_hvsrc
 * @orient: horizontal or vertical
 * @scale:  scaling/zoom factor
 * @hvsrc:  the structure containing the LUT coef
 *
 * Update the Y and C Lut coef, as well as the shift param
 *
 * RETURNS:
 * None.
 */
static void sti_hqvdp_update_hvsrc(enum sti_hvsrc_orient orient, int scale,
		struct sti_hqvdp_hvsrc *hvsrc)
{
	const int *coef_c, *coef_y;
	int shift_c, shift_y;

	/* Get the appropriate coef tables */
	if (scale < SCALE_MAX_FOR_LEG_LUT_F) {
		coef_y = coef_lut_f_y_legacy;
		coef_c = coef_lut_f_c_legacy;
		shift_y = SHIFT_LUT_F_Y_LEGACY;
		shift_c = SHIFT_LUT_F_C_LEGACY;
	} else if (scale < SCALE_MAX_FOR_LEG_LUT_E) {
		coef_y = coef_lut_e_y_legacy;
		coef_c = coef_lut_e_c_legacy;
		shift_y = SHIFT_LUT_E_Y_LEGACY;
		shift_c = SHIFT_LUT_E_C_LEGACY;
	} else if (scale < SCALE_MAX_FOR_LEG_LUT_D) {
		coef_y = coef_lut_d_y_legacy;
		coef_c = coef_lut_d_c_legacy;
		shift_y = SHIFT_LUT_D_Y_LEGACY;
		shift_c = SHIFT_LUT_D_C_LEGACY;
	} else if (scale < SCALE_MAX_FOR_LEG_LUT_C) {
		coef_y = coef_lut_c_y_legacy;
		coef_c = coef_lut_c_c_legacy;
		shift_y = SHIFT_LUT_C_Y_LEGACY;
		shift_c = SHIFT_LUT_C_C_LEGACY;
	} else if (scale == SCALE_MAX_FOR_LEG_LUT_C) {
		coef_y = coef_c = coef_lut_b;
		shift_y = shift_c = SHIFT_LUT_B;
	} else {
		coef_y = coef_c = coef_lut_a_legacy;
		shift_y = shift_c = SHIFT_LUT_A_LEGACY;
	}

	if (orient == HVSRC_HORI) {
		hvsrc->hori_shift = (shift_c << 16) | shift_y;
		memcpy(hvsrc->yh_coef, coef_y, sizeof(hvsrc->yh_coef));
		memcpy(hvsrc->ch_coef, coef_c, sizeof(hvsrc->ch_coef));
	} else {
		hvsrc->vert_shift = (shift_c << 16) | shift_y;
		memcpy(hvsrc->yv_coef, coef_y, sizeof(hvsrc->yv_coef));
		memcpy(hvsrc->cv_coef, coef_c, sizeof(hvsrc->cv_coef));
	}
}

/**
 * sti_hqvdp_check_hw_scaling
 * @layer: hqvdp layer
 *
 * Check if the HW is able to perform the scaling request
 * The firmware scaling limitation is "CEIL(1/Zy) <= FLOOR(LFW)" where:
 *   Zy = OutputHeight / InputHeight
 *   LFW = (Tx * IPClock) / (MaxNbCycles * Cp)
 *     Tx : Total video mode horizontal resolution
 *     IPClock : HQVDP IP clock (Mhz)
 *     MaxNbCycles: max(InputWidth, OutputWidth)
 *     Cp: Video mode pixel clock (Mhz)
 *
 * RETURNS:
 * True if the HW can scale.
 */
static bool sti_hqvdp_check_hw_scaling(struct sti_layer *layer)
{
	struct sti_hqvdp *hqvdp = to_sti_hqvdp(layer);
	unsigned long lfw;
	unsigned int inv_zy;

	lfw = layer->mode->htotal * (clk_get_rate(hqvdp->clk) / 1000000);
	lfw /= max(layer->src_w, layer->dst_w) * layer->mode->clock / 1000;

	inv_zy = DIV_ROUND_UP(layer->src_h, layer->dst_h);

	return (inv_zy <= lfw) ? true : false;
}

/**
 * sti_hqvdp_prepare_layer
 * @layer: hqvdp layer
 * @first_prepare: true if it is the first time this function is called
 *
 * Prepares a command for the firmware
 *
 * RETURNS:
 * 0 on success.
 */
static int sti_hqvdp_prepare_layer(struct sti_layer *layer, bool first_prepare)
{
	struct sti_hqvdp *hqvdp = to_sti_hqvdp(layer);
	struct sti_hqvdp_cmd *cmd;
	int scale_h, scale_v;
	int cmd_offset;

	dev_dbg(hqvdp->dev, "%s %s\n", __func__, sti_layer_to_str(layer));

	/* prepare and commit VID plane */
	hqvdp->vid_plane->funcs->update_plane(hqvdp->vid_plane,
					layer->crtc, layer->fb,
					layer->dst_x, layer->dst_y,
					layer->dst_w, layer->dst_h,
					layer->src_x, layer->src_y,
					layer->src_w, layer->src_h);

	cmd_offset = sti_hqvdp_get_free_cmd(hqvdp);
	if (cmd_offset == -1) {
		DRM_ERROR("No available hqvdp_cmd now\n");
		return -EBUSY;
	}
	cmd = hqvdp->hqvdp_cmd + cmd_offset;

	if (!sti_hqvdp_check_hw_scaling(layer)) {
		DRM_ERROR("Scaling beyond HW capabilities\n");
		return -EINVAL;
	}

	/* Static parameters, defaulting to progressive mode */
	cmd->top.config = TOP_CONFIG_PROGRESSIVE;
	cmd->top.mem_format = TOP_MEM_FORMAT_DFLT;
	cmd->hvsrc.param_ctrl = HVSRC_PARAM_CTRL_DFLT;
	cmd->csdi.config = CSDI_CONFIG_PROG;

	/* VC1RE, FMD bypassed : keep everything set to 0
	 * IQI/P2I bypassed */
	cmd->iqi.config = IQI_CONFIG_DFLT;
	cmd->iqi.con_bri = IQI_CON_BRI_DFLT;
	cmd->iqi.sat_gain = IQI_SAT_GAIN_DFLT;
	cmd->iqi.pxf_conf = IQI_PXF_CONF_DFLT;

	/* Buffer planes address */
	cmd->top.current_luma = (u32) layer->paddr + layer->offsets[0];
	cmd->top.current_chroma = (u32) layer->paddr + layer->offsets[1];

	/* Pitches */
	cmd->top.luma_processed_pitch = cmd->top.luma_src_pitch =
			layer->pitches[0];
	cmd->top.chroma_processed_pitch = cmd->top.chroma_src_pitch =
			layer->pitches[1];

	/* Input / output size
	 * Align to upper even value */
	layer->dst_w = ALIGN(layer->dst_w, 2);
	layer->dst_h = ALIGN(layer->dst_h, 2);

	if ((layer->src_w > MAX_WIDTH) || (layer->src_w < MIN_WIDTH) ||
	    (layer->src_h > MAX_HEIGHT) || (layer->src_h < MIN_HEIGHT) ||
	    (layer->dst_w > MAX_WIDTH) || (layer->dst_w < MIN_WIDTH) ||
	    (layer->dst_h > MAX_HEIGHT) || (layer->dst_h < MIN_HEIGHT)) {
		DRM_ERROR("Invalid in/out size %dx%d -> %dx%d\n",
				layer->src_w, layer->src_h,
				layer->dst_w, layer->dst_h);
		return -EINVAL;
	}
	cmd->top.input_viewport_size = cmd->top.input_frame_size =
			layer->src_h << 16 | layer->src_w;
	cmd->hvsrc.output_picture_size = layer->dst_h << 16 | layer->dst_w;
	cmd->top.input_viewport_ori = layer->src_y << 16 | layer->src_x;

	/* Handle interlaced */
	if (layer->fb->flags & DRM_MODE_FB_INTERLACED) {
		/* Top field to display */
		cmd->top.config = TOP_CONFIG_INTER_TOP;

		/* Update pitches and vert size */
		cmd->top.input_frame_size = (layer->src_h / 2) << 16 |
					     layer->src_w;
		cmd->top.luma_processed_pitch *= 2;
		cmd->top.luma_src_pitch *= 2;
		cmd->top.chroma_processed_pitch *= 2;
		cmd->top.chroma_src_pitch *= 2;

		/* Enable directional deinterlacing processing */
		cmd->csdi.config = CSDI_CONFIG_INTER_DIR;
		cmd->csdi.config2 = CSDI_CONFIG2_DFLT;
		cmd->csdi.dcdi_config = CSDI_DCDI_CONFIG_DFLT;
	}

	/* Update hvsrc lut coef */
	scale_h = SCALE_FACTOR * layer->dst_w / layer->src_w;
	sti_hqvdp_update_hvsrc(HVSRC_HORI, scale_h, &cmd->hvsrc);

	scale_v = SCALE_FACTOR * layer->dst_h / layer->src_h;
	sti_hqvdp_update_hvsrc(HVSRC_VERT, scale_v, &cmd->hvsrc);

	if (first_prepare) {
		/* Prevent VTG shutdown */
		if (clk_prepare_enable(hqvdp->clk_pix_main)) {
			DRM_ERROR("Failed to prepare/enable pix main clk\n");
			return -ENXIO;
		}

		/* Register VTG Vsync callback to handle bottom fields */
		if ((layer->fb->flags & DRM_MODE_FB_INTERLACED) &&
				sti_vtg_register_client(hqvdp->vtg,
					&hqvdp->vtg_nb, layer->mixer_id)) {
			DRM_ERROR("Cannot register VTG notifier\n");
			return -ENXIO;
		}
	}

	return 0;
}

static int sti_hqvdp_commit_layer(struct sti_layer *layer)
{
	struct sti_hqvdp *hqvdp = to_sti_hqvdp(layer);
	int cmd_offset;

	dev_dbg(hqvdp->dev, "%s %s\n", __func__, sti_layer_to_str(layer));

	cmd_offset = sti_hqvdp_get_free_cmd(hqvdp);
	if (cmd_offset == -1) {
		DRM_ERROR("No available hqvdp_cmd now\n");
		return -EBUSY;
	}

	writel(hqvdp->hqvdp_cmd_paddr + cmd_offset,
			hqvdp->regs + HQVDP_MBX_NEXT_CMD);

	hqvdp->curr_field_count++;

	/* Interlaced : get ready to display the bottom field at next Vsync */
	if (layer->fb->flags & DRM_MODE_FB_INTERLACED)
		hqvdp->btm_field_pending = true;

	dev_dbg(hqvdp->dev, "%s Posted command:0x%x\n",
			__func__, hqvdp->hqvdp_cmd_paddr + cmd_offset);

	return 0;
}

static int sti_hqvdp_disable_layer(struct sti_layer *layer)
{
	struct sti_hqvdp *hqvdp = to_sti_hqvdp(layer);
	int i;

	DRM_DEBUG_DRIVER("%s\n", sti_layer_to_str(layer));

	/* Unregister VTG Vsync callback */
	if ((layer->fb->flags & DRM_MODE_FB_INTERLACED) &&
		sti_vtg_unregister_client(hqvdp->vtg, &hqvdp->vtg_nb))
		DRM_DEBUG_DRIVER("Warning: cannot unregister VTG notifier\n");

	/* Set next cmd to NULL */
	writel(0, hqvdp->regs + HQVDP_MBX_NEXT_CMD);

	for (i = 0; i < POLL_MAX_ATTEMPT; i++) {
		if (readl(hqvdp->regs + HQVDP_MBX_INFO_XP70)
				& INFO_XP70_FW_READY)
			break;
		msleep(POLL_DELAY_MS);
	}

	/* VTG can stop now */
	clk_disable_unprepare(hqvdp->clk_pix_main);

	if (i == POLL_MAX_ATTEMPT) {
		DRM_ERROR("XP70 could not revert to idle\n");
		return -ENXIO;
	}

	/* disable VID plane */
	hqvdp->vid_plane->funcs->disable_plane(hqvdp->vid_plane);

	return 0;
}

/**
 * sti_vdp_vtg_cb
 * @nb: notifier block
 * @evt: event message
 * @data: private data
 *
 * Handle VTG Vsync event, display pending bottom field
 *
 * RETURNS:
 * 0 on success.
 */
int sti_hqvdp_vtg_cb(struct notifier_block *nb, unsigned long evt, void *data)
{
	struct sti_hqvdp *hqvdp = container_of(nb, struct sti_hqvdp, vtg_nb);
	int btm_cmd_offset, top_cmd_offest;
	struct sti_hqvdp_cmd *btm_cmd, *top_cmd;

	if ((evt != VTG_TOP_FIELD_EVENT) && (evt != VTG_BOTTOM_FIELD_EVENT)) {
		DRM_DEBUG_DRIVER("Unknown event\n");
		return 0;
	}

	if (hqvdp->btm_field_pending) {
		/* Create the btm field command from the current one */
		btm_cmd_offset = sti_hqvdp_get_free_cmd(hqvdp);
		top_cmd_offest = sti_hqvdp_get_curr_cmd(hqvdp);
		if ((btm_cmd_offset == -1) || (top_cmd_offest == -1)) {
			DRM_ERROR("Cannot get cmds, skip btm field\n");
			return -EBUSY;
		}

		btm_cmd = hqvdp->hqvdp_cmd + btm_cmd_offset;
		top_cmd = hqvdp->hqvdp_cmd + top_cmd_offest;

		memcpy(btm_cmd, top_cmd, sizeof(*btm_cmd));

		btm_cmd->top.config = TOP_CONFIG_INTER_BTM;
		btm_cmd->top.current_luma +=
				btm_cmd->top.luma_src_pitch / 2;
		btm_cmd->top.current_chroma +=
				btm_cmd->top.chroma_src_pitch / 2;

		/* Post the command to mailbox */
		writel(hqvdp->hqvdp_cmd_paddr + btm_cmd_offset,
				hqvdp->regs + HQVDP_MBX_NEXT_CMD);

		hqvdp->curr_field_count++;
		hqvdp->btm_field_pending = false;

		dev_dbg(hqvdp->dev, "%s Posted command:0x%x\n",
				__func__, hqvdp->hqvdp_cmd_paddr);
	}

	return 0;
}

static struct drm_plane *sti_hqvdp_find_vid(struct drm_device *dev, int id)
{
	struct drm_plane *plane;

	list_for_each_entry(plane, &dev->mode_config.plane_list, head) {
		struct sti_layer *layer = to_sti_layer(plane);

		if (layer->desc == id)
			return plane;
	}

	return NULL;
}

static void sti_hqvd_init(struct sti_layer *layer)
{
	struct sti_hqvdp *hqvdp = to_sti_hqvdp(layer);
	int size;

	/* find the plane macthing with vid 0 */
	hqvdp->vid_plane = sti_hqvdp_find_vid(hqvdp->drm_dev, STI_VID_0);
	if (!hqvdp->vid_plane) {
		DRM_ERROR("Cannot find Main video layer\n");
		return;
	}

	hqvdp->vtg_nb.notifier_call = sti_hqvdp_vtg_cb;

	/* Allocate memory for the VDP commands */
	size = NB_VDP_CMD * sizeof(struct sti_hqvdp_cmd);
	hqvdp->hqvdp_cmd = dma_alloc_writecombine(hqvdp->dev, size,
					 &hqvdp->hqvdp_cmd_paddr,
					 GFP_KERNEL | GFP_DMA);
	if (!hqvdp->hqvdp_cmd) {
		DRM_ERROR("Failed to allocate memory for VDP cmd\n");
		return;
	}

	memset(hqvdp->hqvdp_cmd, 0, size);
}

static const struct sti_layer_funcs hqvdp_ops = {
	.get_formats = sti_hqvdp_get_formats,
	.get_nb_formats = sti_hqvdp_get_nb_formats,
	.init = sti_hqvd_init,
	.prepare = sti_hqvdp_prepare_layer,
	.commit = sti_hqvdp_commit_layer,
	.disable = sti_hqvdp_disable_layer,
};

struct sti_layer *sti_hqvdp_create(struct device *dev)
{
	struct sti_hqvdp *hqvdp = dev_get_drvdata(dev);

	hqvdp->layer.ops = &hqvdp_ops;

	return &hqvdp->layer;
}
EXPORT_SYMBOL(sti_hqvdp_create);

static void sti_hqvdp_init_plugs(struct sti_hqvdp *hqvdp)
{
	/* Configure Plugs (same for RD & WR) */
	writel(PLUG_PAGE_SIZE_256, hqvdp->regs + HQVDP_RD_PLUG_PAGE_SIZE);
	writel(PLUG_MIN_OPC_8, hqvdp->regs + HQVDP_RD_PLUG_MIN_OPC);
	writel(PLUG_MAX_OPC_64, hqvdp->regs + HQVDP_RD_PLUG_MAX_OPC);
	writel(PLUG_MAX_CHK_2X, hqvdp->regs + HQVDP_RD_PLUG_MAX_CHK);
	writel(PLUG_MAX_MSG_1X, hqvdp->regs + HQVDP_RD_PLUG_MAX_MSG);
	writel(PLUG_MIN_SPACE_1, hqvdp->regs + HQVDP_RD_PLUG_MIN_SPACE);
	writel(PLUG_CONTROL_ENABLE, hqvdp->regs + HQVDP_RD_PLUG_CONTROL);

	writel(PLUG_PAGE_SIZE_256, hqvdp->regs + HQVDP_WR_PLUG_PAGE_SIZE);
	writel(PLUG_MIN_OPC_8, hqvdp->regs + HQVDP_WR_PLUG_MIN_OPC);
	writel(PLUG_MAX_OPC_64, hqvdp->regs + HQVDP_WR_PLUG_MAX_OPC);
	writel(PLUG_MAX_CHK_2X, hqvdp->regs + HQVDP_WR_PLUG_MAX_CHK);
	writel(PLUG_MAX_MSG_1X, hqvdp->regs + HQVDP_WR_PLUG_MAX_MSG);
	writel(PLUG_MIN_SPACE_1, hqvdp->regs + HQVDP_WR_PLUG_MIN_SPACE);
	writel(PLUG_CONTROL_ENABLE, hqvdp->regs + HQVDP_WR_PLUG_CONTROL);
}

/**
 * sti_hqvdp_start_xp70
 * @firmware: firmware found
 * @ctxt:     hqvdp structure
 *
 * Run the xP70 initialization sequence
 */
static void sti_hqvdp_start_xp70(const struct firmware *firmware, void *ctxt)
{
	struct sti_hqvdp *hqvdp = ctxt;
	u32 *fw_rd_plug, *fw_wr_plug, *fw_pmem, *fw_dmem;
	u8 *data;
	int i;
	struct fw_header {
		int rd_size;
		int wr_size;
		int pmem_size;
		int dmem_size;
	} *header;

	DRM_DEBUG_DRIVER("\n");
	/* Check firmware parts */
	if (!firmware) {
		DRM_ERROR("Firmware not available\n");
		return;
	}

	header = (struct fw_header *) firmware->data;
	if (firmware->size < sizeof(*header)) {
		DRM_ERROR("Invalid firmware size (%d)\n", firmware->size);
		goto out;
	}
	if ((sizeof(*header) + header->rd_size + header->wr_size +
		header->pmem_size + header->dmem_size) != firmware->size) {
		DRM_ERROR("Invalid fmw structure (%d+%d+%d+%d+%d != %d)\n",
			   sizeof(*header), header->rd_size, header->wr_size,
			   header->pmem_size, header->dmem_size,
			   firmware->size);
		goto out;
	}

	data = (u8 *) firmware->data;
	data += sizeof(*header);
	fw_rd_plug = (void *) data;
	data += header->rd_size;
	fw_wr_plug = (void *) data;
	data += header->wr_size;
	fw_pmem = (void *) data;
	data += header->pmem_size;
	fw_dmem = (void *) data;

	/* Enable clock */
	if (clk_prepare_enable(hqvdp->clk))
		DRM_ERROR("Failed to prepare/enable HQVDP clk\n");

	/* Reset */
	writel(SW_RESET_CTRL_FULL, hqvdp->regs + HQVDP_MBX_SW_RESET_CTRL);

	for (i = 0; i < POLL_MAX_ATTEMPT; i++) {
		if (readl(hqvdp->regs + HQVDP_MBX_STARTUP_CTRL1)
				& STARTUP_CTRL1_RST_DONE)
			break;
		msleep(POLL_DELAY_MS);
	}
	if (i == POLL_MAX_ATTEMPT) {
		DRM_ERROR("Could not reset\n");
		goto out;
	}

	/* Init Read & Write plugs */
	for (i = 0; i < header->rd_size / 4; i++)
		writel(fw_rd_plug[i], hqvdp->regs + HQVDP_RD_PLUG + i * 4);
	for (i = 0; i < header->wr_size / 4; i++)
		writel(fw_wr_plug[i], hqvdp->regs + HQVDP_WR_PLUG + i * 4);

	sti_hqvdp_init_plugs(hqvdp);

	/* Authorize Idle Mode */
	writel(STARTUP_CTRL1_AUTH_IDLE, hqvdp->regs + HQVDP_MBX_STARTUP_CTRL1);

	/* Prevent VTG interruption during the boot */
	writel(SOFT_VSYNC_SW_CTRL_IRQ, hqvdp->regs + HQVDP_MBX_SOFT_VSYNC);
	writel(0, hqvdp->regs + HQVDP_MBX_NEXT_CMD);

	/* Download PMEM & DMEM */
	for (i = 0; i < header->pmem_size / 4; i++)
		writel(fw_pmem[i], hqvdp->regs + HQVDP_PMEM + i * 4);
	for (i = 0; i < header->dmem_size / 4; i++)
		writel(fw_dmem[i], hqvdp->regs + HQVDP_DMEM + i * 4);

	/* Enable fetch */
	writel(STARTUP_CTRL2_FETCH_EN, hqvdp->regs + HQVDP_MBX_STARTUP_CTRL2);

	/* Wait end of boot */
	for (i = 0; i < POLL_MAX_ATTEMPT; i++) {
		if (readl(hqvdp->regs + HQVDP_MBX_INFO_XP70)
				& INFO_XP70_FW_READY)
			break;
		msleep(POLL_DELAY_MS);
	}
	if (i == POLL_MAX_ATTEMPT) {
		DRM_ERROR("Could not boot\n");
		goto out;
	}

	/* Launch Vsync */
	writel(SOFT_VSYNC_HW, hqvdp->regs + HQVDP_MBX_SOFT_VSYNC);

	DRM_INFO("HQVDP XP70 started\n");
out:
	release_firmware(firmware);
}

int sti_hqvdp_bind(struct device *dev, struct device *master, void *data)
{
	struct sti_hqvdp *hqvdp = dev_get_drvdata(dev);
	struct drm_device *drm_dev = data;
	struct sti_layer *layer;
	int err;

	DRM_DEBUG_DRIVER("\n");

	hqvdp->drm_dev = drm_dev;

	/* Request for firmware */
	err = request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				HQVDP_FMW_NAME,	hqvdp->dev,
				GFP_KERNEL, hqvdp, sti_hqvdp_start_xp70);
	if (err) {
		DRM_ERROR("Can't get HQVDP firmware\n");
		return err;
	}

	layer = sti_layer_create(hqvdp->dev, STI_HQVDP_0, hqvdp->regs);
	if (!layer) {
		DRM_ERROR("Can't create HQVDP plane\n");
		return -ENOMEM;
	}

	sti_drm_plane_init(drm_dev, layer, 1, DRM_PLANE_TYPE_OVERLAY);

	return 0;
}

static void sti_hqvdp_unbind(struct device *dev,
		struct device *master, void *data)
{
	/* do nothing */
}

static const struct component_ops sti_hqvdp_ops = {
	.bind = sti_hqvdp_bind,
	.unbind = sti_hqvdp_unbind,
};

static int sti_hqvdp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *vtg_np;
	struct sti_hqvdp *hqvdp;
	struct resource *res;

	DRM_DEBUG_DRIVER("\n");

	hqvdp = devm_kzalloc(dev, sizeof(*hqvdp), GFP_KERNEL);
	if (!hqvdp) {
		DRM_ERROR("Failed to allocate HQVDP context\n");
		return -ENOMEM;
	}

	hqvdp->dev = dev;

	/* Get Memory resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		DRM_ERROR("Get memory resource failed\n");
		return -ENXIO;
	}
	hqvdp->regs = devm_ioremap(dev, res->start, resource_size(res));
	if (hqvdp->regs == NULL) {
		DRM_ERROR("Register mapping failed\n");
		return -ENXIO;
	}

	/* Get clock resources */
	hqvdp->clk = devm_clk_get(dev, "hqvdp");
	hqvdp->clk_pix_main = devm_clk_get(dev, "pix_main");
	if (IS_ERR(hqvdp->clk) || IS_ERR(hqvdp->clk_pix_main)) {
		DRM_ERROR("Cannot get clocks\n");
		return -ENXIO;
	}

	/* Get reset resources */
	hqvdp->reset = devm_reset_control_get(dev, "hqvdp");
	if (!IS_ERR(hqvdp->reset))
		reset_control_deassert(hqvdp->reset);

	vtg_np = of_parse_phandle(pdev->dev.of_node, "st,vtg", 0);
	if (vtg_np)
		hqvdp->vtg = of_vtg_find(vtg_np);

	platform_set_drvdata(pdev, hqvdp);

	return component_add(&pdev->dev, &sti_hqvdp_ops);
}

static int sti_hqvdp_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &sti_hqvdp_ops);
	return 0;
}

static struct of_device_id hqvdp_of_match[] = {
	{ .compatible = "st,stih407-hqvdp", },
	{ /* end node */ }
};
MODULE_DEVICE_TABLE(of, hqvdp_of_match);

struct platform_driver sti_hqvdp_driver = {
	.driver = {
		.name = "sti-hqvdp",
		.owner = THIS_MODULE,
		.of_match_table = hqvdp_of_match,
	},
	.probe = sti_hqvdp_probe,
	.remove = sti_hqvdp_remove,
};

module_platform_driver(sti_hqvdp_driver);

MODULE_AUTHOR("Benjamin Gaignard <benjamin.gaignard@st.com>");
MODULE_DESCRIPTION("STMicroelectronics SoC DRM driver");
MODULE_LICENSE("GPL");
