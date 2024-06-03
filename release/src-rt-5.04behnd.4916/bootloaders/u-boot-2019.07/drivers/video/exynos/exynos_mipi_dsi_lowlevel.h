/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef _EXYNOS_MIPI_DSI_LOWLEVEL_H
#define _EXYNOS_MIPI_DSI_LOWLEVEL_H

void exynos_mipi_dsi_register(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_func_reset(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_sw_reset(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_sw_release(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_set_interrupt_mask(struct mipi_dsim_device *dsim,
	unsigned int mode, unsigned int mask);
void exynos_mipi_dsi_set_data_lane_number(struct mipi_dsim_device *dsim,
					unsigned int count);
void exynos_mipi_dsi_init_fifo_pointer(struct mipi_dsim_device *dsim,
					unsigned int cfg);
void exynos_mipi_dsi_set_phy_tunning(struct mipi_dsim_device *dsim,
				unsigned int value);
void exynos_mipi_dsi_set_phy_tunning(struct mipi_dsim_device *dsim,
				unsigned int value);
void exynos_mipi_dsi_set_main_disp_resol(struct mipi_dsim_device *dsim,
		unsigned int width_resol, unsigned int height_resol);
void exynos_mipi_dsi_set_main_disp_vporch(struct mipi_dsim_device *dsim,
	unsigned int cmd_allow, unsigned int vfront, unsigned int vback);
void exynos_mipi_dsi_set_main_disp_hporch(struct mipi_dsim_device *dsim,
			unsigned int front, unsigned int back);
void exynos_mipi_dsi_set_main_disp_sync_area(struct mipi_dsim_device *dsim,
				unsigned int vert, unsigned int hori);
void exynos_mipi_dsi_set_sub_disp_resol(struct mipi_dsim_device *dsim,
				unsigned int vert, unsigned int hori);
void exynos_mipi_dsi_init_config(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_display_config(struct mipi_dsim_device *dsim,
				struct mipi_dsim_config *dsim_config);
void exynos_mipi_dsi_set_data_lane_number(struct mipi_dsim_device *dsim,
				unsigned int count);
void exynos_mipi_dsi_enable_lane(struct mipi_dsim_device *dsim,
			unsigned int lane, unsigned int enable);
void exynos_mipi_dsi_enable_afc(struct mipi_dsim_device *dsim,
			unsigned int enable, unsigned int afc_code);
void exynos_mipi_dsi_enable_pll_bypass(struct mipi_dsim_device *dsim,
				unsigned int enable);
void exynos_mipi_dsi_pll_freq_band(struct mipi_dsim_device *dsim,
				unsigned int freq_band);
void exynos_mipi_dsi_pll_freq(struct mipi_dsim_device *dsim,
			unsigned int pre_divider, unsigned int main_divider,
			unsigned int scaler);
void exynos_mipi_dsi_pll_stable_time(struct mipi_dsim_device *dsim,
			unsigned int lock_time);
void exynos_mipi_dsi_enable_pll(struct mipi_dsim_device *dsim,
					unsigned int enable);
void exynos_mipi_dsi_set_byte_clock_src(struct mipi_dsim_device *dsim,
					unsigned int src);
void exynos_mipi_dsi_enable_byte_clock(struct mipi_dsim_device *dsim,
					unsigned int enable);
void exynos_mipi_dsi_set_esc_clk_prs(struct mipi_dsim_device *dsim,
				unsigned int enable, unsigned int prs_val);
void exynos_mipi_dsi_enable_esc_clk_on_lane(struct mipi_dsim_device *dsim,
				unsigned int lane_sel, unsigned int enable);
void exynos_mipi_dsi_force_dphy_stop_state(struct mipi_dsim_device *dsim,
				unsigned int enable);
unsigned int exynos_mipi_dsi_is_lane_state(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_set_stop_state_counter(struct mipi_dsim_device *dsim,
				unsigned int cnt_val);
void exynos_mipi_dsi_set_bta_timeout(struct mipi_dsim_device *dsim,
				unsigned int timeout);
void exynos_mipi_dsi_set_lpdr_timeout(struct mipi_dsim_device *dsim,
				unsigned int timeout);
void exynos_mipi_dsi_set_lcdc_transfer_mode(struct mipi_dsim_device *dsim,
					unsigned int lp);
void exynos_mipi_dsi_set_cpu_transfer_mode(struct mipi_dsim_device *dsim,
					unsigned int lp);
void exynos_mipi_dsi_enable_hs_clock(struct mipi_dsim_device *dsim,
				unsigned int enable);
void exynos_mipi_dsi_dp_dn_swap(struct mipi_dsim_device *dsim,
				unsigned int swap_en);
void exynos_mipi_dsi_hs_zero_ctrl(struct mipi_dsim_device *dsim,
				unsigned int hs_zero);
void exynos_mipi_dsi_prep_ctrl(struct mipi_dsim_device *dsim,
				unsigned int prep);
void exynos_mipi_dsi_clear_interrupt(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_clear_all_interrupt(struct mipi_dsim_device *dsim);
unsigned int exynos_mipi_dsi_is_pll_stable(struct mipi_dsim_device *dsim);
unsigned int exynos_mipi_dsi_get_fifo_state(struct mipi_dsim_device *dsim);
unsigned int _exynos_mipi_dsi_get_frame_done_status(struct mipi_dsim_device
						*dsim);
void _exynos_mipi_dsi_clear_frame_done(struct mipi_dsim_device *dsim);
void exynos_mipi_dsi_wr_tx_header(struct mipi_dsim_device *dsim,
		unsigned int di, const unsigned char data0, const unsigned char data1);
void exynos_mipi_dsi_wr_tx_data(struct mipi_dsim_device *dsim,
		unsigned int tx_data);

#endif /* _EXYNOS_MIPI_DSI_LOWLEVEL_H */
