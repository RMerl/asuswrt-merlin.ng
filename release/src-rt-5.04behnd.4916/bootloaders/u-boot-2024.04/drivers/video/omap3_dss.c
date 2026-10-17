/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * Referred to Linux Kernel DSS driver files for OMAP3 by
 * Tomi Valkeinen from drivers/video/omap2/dss/
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 and any
 * later version the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/dss.h>

/* Configure VENC for a given Mode (NTSC / PAL) */
void omap3_dss_venc_config(const struct venc_regs *venc_cfg,
				u32 height, u32 width)
{
	struct venc_regs *venc = (struct venc_regs *) OMAP3_VENC_BASE;
	struct dss_regs *dss = (struct dss_regs *) OMAP3_DSS_BASE;
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;

	writel(venc_cfg->status, &venc->status);
	writel(venc_cfg->f_control, &venc->f_control);
	writel(venc_cfg->vidout_ctrl, &venc->vidout_ctrl);
	writel(venc_cfg->sync_ctrl, &venc->sync_ctrl);
	writel(venc_cfg->llen, &venc->llen);
	writel(venc_cfg->flens, &venc->flens);
	writel(venc_cfg->hfltr_ctrl, &venc->hfltr_ctrl);
	writel(venc_cfg->cc_carr_wss_carr, &venc->cc_carr_wss_carr);
	writel(venc_cfg->c_phase, &venc->c_phase);
	writel(venc_cfg->gain_u, &venc->gain_u);
	writel(venc_cfg->gain_v, &venc->gain_v);
	writel(venc_cfg->gain_y, &venc->gain_y);
	writel(venc_cfg->black_level, &venc->black_level);
	writel(venc_cfg->blank_level, &venc->blank_level);
	writel(venc_cfg->x_color, &venc->x_color);
	writel(venc_cfg->m_control, &venc->m_control);
	writel(venc_cfg->bstamp_wss_data, &venc->bstamp_wss_data);
	writel(venc_cfg->s_carr, &venc->s_carr);
	writel(venc_cfg->line21, &venc->line21);
	writel(venc_cfg->ln_sel, &venc->ln_sel);
	writel(venc_cfg->l21__wc_ctl, &venc->l21__wc_ctl);
	writel(venc_cfg->htrigger_vtrigger, &venc->htrigger_vtrigger);
	writel(venc_cfg->savid__eavid, &venc->savid__eavid);
	writel(venc_cfg->flen__fal, &venc->flen__fal);
	writel(venc_cfg->lal__phase_reset, &venc->lal__phase_reset);
	writel(venc_cfg->hs_int_start_stop_x, &venc->hs_int_start_stop_x);
	writel(venc_cfg->hs_ext_start_stop_x, &venc->hs_ext_start_stop_x);
	writel(venc_cfg->vs_int_start_x, &venc->vs_int_start_x);
	writel(venc_cfg->vs_int_stop_x__vs_int_start_y,
			&venc->vs_int_stop_x__vs_int_start_y);
	writel(venc_cfg->vs_int_stop_y__vs_ext_start_x,
			&venc->vs_int_stop_y__vs_ext_start_x);
	writel(venc_cfg->vs_ext_stop_x__vs_ext_start_y,
			&venc->vs_ext_stop_x__vs_ext_start_y);
	writel(venc_cfg->vs_ext_stop_y, &venc->vs_ext_stop_y);
	writel(venc_cfg->avid_start_stop_x, &venc->avid_start_stop_x);
	writel(venc_cfg->avid_start_stop_y, &venc->avid_start_stop_y);
	writel(venc_cfg->fid_int_start_x__fid_int_start_y,
				&venc->fid_int_start_x__fid_int_start_y);
	writel(venc_cfg->fid_int_offset_y__fid_ext_start_x,
				&venc->fid_int_offset_y__fid_ext_start_x);
	writel(venc_cfg->fid_ext_start_y__fid_ext_offset_y,
				&venc->fid_ext_start_y__fid_ext_offset_y);
	writel(venc_cfg->tvdetgp_int_start_stop_x,
				&venc->tvdetgp_int_start_stop_x);
	writel(venc_cfg->tvdetgp_int_start_stop_y,
				&venc->tvdetgp_int_start_stop_y);
	writel(venc_cfg->gen_ctrl, &venc->gen_ctrl);
	writel(venc_cfg->output_control, &venc->output_control);
	writel(venc_cfg->dac_b__dac_c, &venc->dac_b__dac_c);

	/* Configure DSS for VENC Settings */
	writel(VENC_CLK_ENABLE | DAC_DEMEN | DAC_POWERDN | VENC_OUT_SEL,
			&dss->control);

	/* Configure height and width for Digital out */
	writel(height << DIG_LPP_SHIFT | width, &dispc->size_dig);
}

/* Configure Panel Specific Parameters */
void omap3_dss_panel_config(const struct panel_config *panel_cfg)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct dss_regs *dss = (struct dss_regs *) OMAP3_DSS_BASE;

	writel(DSS_SOFTRESET, &dss->sysconfig);
	while (!(readl(&dss->sysstatus) & DSS_RESETDONE))
		;

	writel(panel_cfg->timing_h, &dispc->timing_h);
	writel(panel_cfg->timing_v, &dispc->timing_v);
	writel(panel_cfg->pol_freq, &dispc->pol_freq);
	writel(panel_cfg->divisor, &dispc->divisor);
	writel(panel_cfg->lcd_size, &dispc->size_lcd);
	writel(panel_cfg->load_mode << LOADMODE_SHIFT, &dispc->config);
	writel(panel_cfg->panel_type << TFTSTN_SHIFT |
		panel_cfg->data_lines << DATALINES_SHIFT, &dispc->control);
	writel(panel_cfg->panel_color, &dispc->default_color0);
	writel((u32) panel_cfg->frame_buffer, &dispc->gfx_ba0);

	if (!panel_cfg->frame_buffer)
		return;

	writel(panel_cfg->gfx_format | GFX_ENABLE, &dispc->gfx_attributes);
	writel(1, &dispc->gfx_row_inc);
	writel(1, &dispc->gfx_pixel_inc);
	writel(panel_cfg->lcd_size, &dispc->gfx_size);
}

/* Enable LCD and DIGITAL OUT in DSS */
void omap3_dss_enable(void)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	u32 l;

	l = readl(&dispc->control);
	l |= LCD_ENABLE | GO_LCD | DIG_ENABLE | GO_DIG | GP_OUT0 | GP_OUT1;
	writel(l, &dispc->control);
}
