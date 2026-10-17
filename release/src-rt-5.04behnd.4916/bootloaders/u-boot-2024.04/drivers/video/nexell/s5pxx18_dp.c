// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/arch/reset.h>
#include <asm/arch/nexell.h>
#include <asm/arch/display.h>

#include "soc/s5pxx18_soc_disptop.h"
#include "soc/s5pxx18_soc_dpc.h"
#include "soc/s5pxx18_soc_mlc.h"

#define	MLC_LAYER_RGB_0		0	/* number of RGB layer 0 */
#define	MLC_LAYER_RGB_1		1	/* number of RGB layer 1 */
#define	MLC_LAYER_VIDEO		3	/* number of Video layer: 3 = VIDEO */

#define	__io_address(a)	(void *)(uintptr_t)(a)

void dp_control_init(int module)
{
	void *base;

	/* top */
	base = __io_address(nx_disp_top_get_physical_address());
	nx_disp_top_set_base_address(base);

	/* control */
	base = __io_address(nx_dpc_get_physical_address(module));
	nx_dpc_set_base_address(module, base);

	/* top controller */
	nx_rstcon_setrst(RESET_ID_DISP_TOP, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_DISP_TOP, RSTCON_NEGATE);

	/* display controller */
	nx_rstcon_setrst(RESET_ID_DISPLAY, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_DISPLAY, RSTCON_NEGATE);

	nx_dpc_set_clock_pclk_mode(module, nx_pclkmode_always);
}

int dp_control_setup(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl)
{
	unsigned int out_format;
	unsigned int delay_mask;
	int rgb_pvd = 0, hsync_cp1 = 7, vsync_fram = 7, de_cp2 = 7;
	int v_vso = 1, v_veo = 1, e_vso = 1, e_veo = 1;

	int interlace = 0;
	int invert_field;
	int swap_rb;
	unsigned int yc_order;
	int vck_select;
	int vclk_invert;
	int emb_sync;

	enum nx_dpc_dither r_dither, g_dither, b_dither;
	int rgb_mode = 0;

	if (NULL == sync || NULL == ctrl) {
		debug("error, dp.%d not set sync or pad clock info !!!\n",
		      module);
		return -EINVAL;
	}

	out_format = ctrl->out_format;
	delay_mask = ctrl->delay_mask;
	interlace = sync->interlace;
	invert_field = ctrl->invert_field;
	swap_rb = ctrl->swap_RB;
	yc_order = ctrl->yc_order;
	vck_select = ctrl->vck_select;
	vclk_invert = ctrl->clk_inv_lv0 | ctrl->clk_inv_lv1;
	emb_sync = (out_format == DPC_FORMAT_CCIR656 ? 1 : 0);

	/* set delay mask */
	if (delay_mask & DP_SYNC_DELAY_RGB_PVD)
		rgb_pvd = ctrl->d_rgb_pvd;
	if (delay_mask & DP_SYNC_DELAY_HSYNC_CP1)
		hsync_cp1 = ctrl->d_hsync_cp1;
	if (delay_mask & DP_SYNC_DELAY_VSYNC_FRAM)
		vsync_fram = ctrl->d_vsync_fram;
	if (delay_mask & DP_SYNC_DELAY_DE_CP)
		de_cp2 = ctrl->d_de_cp2;

	if (ctrl->vs_start_offset != 0 ||
	    ctrl->vs_end_offset != 0 ||
	    ctrl->ev_start_offset != 0 || ctrl->ev_end_offset != 0) {
		v_vso = ctrl->vs_start_offset;
		v_veo = ctrl->vs_end_offset;
		e_vso = ctrl->ev_start_offset;
		e_veo = ctrl->ev_end_offset;
	}

	if (nx_dpc_format_rgb555 == out_format ||
	    nx_dpc_format_mrgb555a == out_format ||
	    nx_dpc_format_mrgb555b == out_format) {
		r_dither = nx_dpc_dither_5bit;
		g_dither = nx_dpc_dither_5bit;
		b_dither = nx_dpc_dither_5bit;
		rgb_mode = 1;
	} else if (nx_dpc_format_rgb565 == out_format ||
		       nx_dpc_format_mrgb565 == out_format) {
		r_dither = nx_dpc_dither_5bit;
		b_dither = nx_dpc_dither_5bit;
		g_dither = nx_dpc_dither_6bit, rgb_mode = 1;
	} else if ((nx_dpc_format_rgb666 == out_format) ||
		   (nx_dpc_format_mrgb666 == out_format)) {
		r_dither = nx_dpc_dither_6bit;
		g_dither = nx_dpc_dither_6bit;
		b_dither = nx_dpc_dither_6bit;
		rgb_mode = 1;
	} else {
		r_dither = nx_dpc_dither_bypass;
		g_dither = nx_dpc_dither_bypass;
		b_dither = nx_dpc_dither_bypass;
		rgb_mode = 1;
	}

	/* CLKGEN0/1 */
	nx_dpc_set_clock_source(module, 0, ctrl->clk_src_lv0 == 3 ?
				6 : ctrl->clk_src_lv0);
	nx_dpc_set_clock_divisor(module, 0, ctrl->clk_div_lv0);
	nx_dpc_set_clock_source(module, 1, ctrl->clk_src_lv1);
	nx_dpc_set_clock_divisor(module, 1, ctrl->clk_div_lv1);
	nx_dpc_set_clock_out_delay(module, 0, ctrl->clk_delay_lv0);
	nx_dpc_set_clock_out_delay(module, 1, ctrl->clk_delay_lv1);

	/* LCD out */
	nx_dpc_set_mode(module, out_format, interlace, invert_field,
			rgb_mode, swap_rb, yc_order, emb_sync, emb_sync,
			vck_select, vclk_invert, 0);
	nx_dpc_set_hsync(module, sync->h_active_len, sync->h_sync_width,
			 sync->h_front_porch, sync->h_back_porch,
			 sync->h_sync_invert);
	nx_dpc_set_vsync(module, sync->v_active_len, sync->v_sync_width,
			 sync->v_front_porch, sync->v_back_porch,
			 sync->v_sync_invert, sync->v_active_len,
			 sync->v_sync_width, sync->v_front_porch,
			 sync->v_back_porch);
	nx_dpc_set_vsync_offset(module, v_vso, v_veo, e_vso, e_veo);
	nx_dpc_set_delay(module, rgb_pvd, hsync_cp1, vsync_fram, de_cp2);
	nx_dpc_set_dither(module, r_dither, g_dither, b_dither);

	if (IS_ENABLED(CONFIG_MACH_S5P6818)) {
		/* Set TFT_CLKCTRL (offset : 1030h)
		 * Field name : DPC0_CLKCTRL, DPC1_CLKCRL
		 * Default value : clk_inv_lv0/1 = 0 : PADCLK_InvCLK
		 * Invert case   : clk_inv_lv0/1 = 1 : PADCLK_CLK
		 */
		if (module == 0 && ctrl->clk_inv_lv0)
			nx_disp_top_set_padclock(padmux_primary_mlc,
						 padclk_clk);
		if (module == 1 && ctrl->clk_inv_lv1)
			nx_disp_top_set_padclock(padmux_secondary_mlc,
						 padclk_clk);
	}

	debug("%s: dp.%d x:%4d, hf:%3d, hb:%3d, hs:%3d, hi=%d\n",
	      __func__, module, sync->h_active_len, sync->h_front_porch,
	      sync->h_back_porch, sync->h_sync_width, sync->h_sync_invert);
	debug("%s: dp.%d y:%4d, vf:%3d, vb:%3d, vs:%3d, vi=%d\n",
	      __func__, module, sync->v_active_len, sync->v_front_porch,
	      sync->v_back_porch, sync->v_sync_width, sync->h_sync_invert);
	debug("%s: dp.%d ck.0:%d:%d:%d, ck.1:%d:%d:%d\n",
	      __func__, module,
	      ctrl->clk_src_lv0, ctrl->clk_div_lv0, ctrl->clk_inv_lv0,
	      ctrl->clk_src_lv1, ctrl->clk_div_lv1, ctrl->clk_inv_lv1);
	debug("%s: dp.%d vs:%d, ve:%d, es:%d, ee:%d\n",
	      __func__, module, v_vso, v_veo, e_vso, e_veo);
	debug("%s: dp.%d delay RGB:%d, hs:%d, vs:%d, de:%d, fmt:0x%x\n",
	      __func__, module, rgb_pvd, hsync_cp1, vsync_fram, de_cp2,
	      out_format);

	return 0;
}

void dp_control_enable(int module, int on)
{
	debug("%s: dp.%d top %s\n", __func__, module, on ? "ON" : "OFF");

	nx_dpc_set_dpc_enable(module, on);
	nx_dpc_set_clock_divisor_enable(module, on);
}

void dp_plane_init(int module)
{
	void *base = __io_address(nx_mlc_get_physical_address(module));

	nx_mlc_set_base_address(module, base);
	nx_mlc_set_clock_pclk_mode(module, nx_pclkmode_always);
	nx_mlc_set_clock_bclk_mode(module, nx_bclkmode_always);
}

int dp_plane_screen_setup(int module, struct dp_plane_top *top)
{
	int width = top->screen_width;
	int height = top->screen_height;
	int interlace = top->interlace;
	int video_prior = top->video_prior;
	unsigned int bg_color = top->back_color;

	/* MLC TOP layer */
	nx_mlc_set_screen_size(module, width, height);
	nx_mlc_set_layer_priority(module, video_prior);
	nx_mlc_set_background(module, bg_color);
	nx_mlc_set_field_enable(module, interlace);
	nx_mlc_set_rgblayer_gama_table_power_mode(module, 0, 0, 0);
	nx_mlc_set_rgblayer_gama_table_sleep_mode(module, 1, 1, 1);
	nx_mlc_set_rgblayer_gamma_enable(module, 0);
	nx_mlc_set_dither_enable_when_using_gamma(module, 0);
	nx_mlc_set_gamma_priority(module, 0);
	nx_mlc_set_top_power_mode(module, 1);
	nx_mlc_set_top_sleep_mode(module, 0);

	debug("%s: dp.%d screen %dx%d, %s, priority:%d, bg:0x%x\n",
	      __func__, module, width, height,
	      interlace ? "Interlace" : "Progressive",
	      video_prior, bg_color);

	return 0;
}

void dp_plane_screen_enable(int module, int on)
{
	/* enable top screen */
	nx_mlc_set_mlc_enable(module, on);
	nx_mlc_set_top_dirty_flag(module);
	debug("%s: dp.%d top %s\n", __func__, module, on ? "ON" : "OFF");
}

int dp_plane_layer_setup(int module, struct dp_plane_info *plane)
{
	int sx = plane->left;
	int sy = plane->top;
	int ex = sx + plane->width - 1;
	int ey = sy + plane->height - 1;
	int pixel_byte = plane->pixel_byte;
	int mem_lock_size = 16;	/* fix mem lock size */
	int layer = plane->layer;
	unsigned int format = plane->format;

	if (!plane->enable)
		return -EINVAL;

	/* MLC layer */
	nx_mlc_set_lock_size(module, layer, mem_lock_size);
	nx_mlc_set_alpha_blending(module, layer, 0, 15);
	nx_mlc_set_transparency(module, layer, 0, 0);
	nx_mlc_set_color_inversion(module, layer, 0, 0);
	nx_mlc_set_rgblayer_invalid_position(module, layer, 0, 0, 0, 0, 0, 0);
	nx_mlc_set_rgblayer_invalid_position(module, layer, 1, 0, 0, 0, 0, 0);
	nx_mlc_set_format_rgb(module, layer, format);
	nx_mlc_set_position(module, layer, sx, sy, ex, ey);
	nx_mlc_set_rgblayer_stride(module, layer, pixel_byte,
				   plane->width * pixel_byte);
	nx_mlc_set_rgblayer_address(module, layer, plane->fb_base);

	debug("%s: dp.%d.%d %d * %d, %dbpp, fmt:0x%x\n",
	      __func__, module, layer, plane->width, plane->height,
	      pixel_byte * 8, format);
	debug("%s: b:0x%x, l:%d, t:%d, r:%d, b:%d, hs:%d, vs:%d\n",
	      __func__, plane->fb_base, sx, sy, ex, ey,
	      plane->width * pixel_byte, pixel_byte);

	return 0;
}

int dp_plane_set_enable(int module, int layer, int on)
{
	int hl, hc;
	int vl, vc;

	debug("%s: dp.%d.%d %s:%s\n",
	      __func__, module, layer,
	      layer == MLC_LAYER_VIDEO ? "Video" : "RGB",
	      on ? "ON" : "OFF");

	if (layer != MLC_LAYER_VIDEO) {
		nx_mlc_set_layer_enable(module, layer, on);
		nx_mlc_set_dirty_flag(module, layer);
		return 0;
	}

	/* video layer */
	if (on) {
		nx_mlc_set_video_layer_line_buffer_power_mode(module, 1);
		nx_mlc_set_video_layer_line_buffer_sleep_mode(module, 0);
		nx_mlc_set_layer_enable(module, layer, 1);
		nx_mlc_set_dirty_flag(module, layer);
	} else {
		nx_mlc_set_layer_enable(module, layer, 0);
		nx_mlc_set_dirty_flag(module, layer);
		nx_mlc_get_video_layer_scale_filter(module,
						    &hl, &hc, &vl, &vc);
		if (hl || hc || vl || vc)
			nx_mlc_set_video_layer_scale_filter(module, 0, 0, 0, 0);
		nx_mlc_set_video_layer_line_buffer_power_mode(module, 0);
		nx_mlc_set_video_layer_line_buffer_sleep_mode(module, 1);
		nx_mlc_set_dirty_flag(module, layer);
	}

	return 0;
}

void dp_plane_layer_enable(int module,
			   struct dp_plane_info *plane, int on)
{
	dp_plane_set_enable(module, plane->layer, on);
}

int dp_plane_set_address(int module, int layer, unsigned int address)
{
	nx_mlc_set_rgblayer_address(module, layer, address);
	nx_mlc_set_dirty_flag(module, layer);

	return 0;
}

int dp_plane_wait_vsync(int module, int layer, int fps)
{
	int cnt = 0;

	if (fps == 0)
		return (int)nx_mlc_get_dirty_flag(module, layer);

	while (fps > cnt++) {
		while (nx_mlc_get_dirty_flag(module, layer))
			;
		nx_mlc_set_dirty_flag(module, layer);
	}
	return 0;
}
