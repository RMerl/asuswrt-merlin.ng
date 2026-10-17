// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/arch/dsim.h>
#include <asm/arch/mipi_dsim.h>

#include "exynos_mipi_dsi_lowlevel.h"

#define MHZ			(1000 * 1000)
#define FIN_HZ			(24 * MHZ)

#define DFIN_PLL_MIN_HZ		(6 * MHZ)
#define DFIN_PLL_MAX_HZ		(12 * MHZ)

#define DFVCO_MIN_HZ		(500 * MHZ)
#define DFVCO_MAX_HZ		(1000 * MHZ)

#define TRY_GET_FIFO_TIMEOUT	(5000 * 2)

/* MIPI-DSIM status types. */
enum {
	DSIM_STATE_INIT,	/* should be initialized. */
	DSIM_STATE_STOP,	/* CPU and LCDC are LP mode. */
	DSIM_STATE_HSCLKEN,	/* HS clock was enabled. */
	DSIM_STATE_ULPS
};

/* define DSI lane types. */
enum {
	DSIM_LANE_CLOCK = (1 << 0),
	DSIM_LANE_DATA0 = (1 << 1),
	DSIM_LANE_DATA1 = (1 << 2),
	DSIM_LANE_DATA2 = (1 << 3),
	DSIM_LANE_DATA3 = (1 << 4)
};

static unsigned int dpll_table[15] = {
	100, 120, 170, 220, 270,
	320, 390, 450, 510, 560,
	640, 690, 770, 870, 950
};

static void exynos_mipi_dsi_long_data_wr(struct mipi_dsim_device *dsim,
		const unsigned char *data0, unsigned int data1)
{
	unsigned int data_cnt = 0, payload = 0;

	/* in case that data count is more then 4 */
	for (data_cnt = 0; data_cnt < data1; data_cnt += 4) {
		/*
		 * after sending 4bytes per one time,
		 * send remainder data less then 4.
		 */
		if ((data1 - data_cnt) < 4) {
			if ((data1 - data_cnt) == 3) {
				payload = data0[data_cnt] |
					data0[data_cnt + 1] << 8 |
					data0[data_cnt + 2] << 16;
			debug("count = 3 payload = %x, %x %x %x\n",
				payload, data0[data_cnt],
				data0[data_cnt + 1],
				data0[data_cnt + 2]);
			} else if ((data1 - data_cnt) == 2) {
				payload = data0[data_cnt] |
					data0[data_cnt + 1] << 8;
			debug("count = 2 payload = %x, %x %x\n", payload,
				data0[data_cnt], data0[data_cnt + 1]);
			} else if ((data1 - data_cnt) == 1) {
				payload = data0[data_cnt];
			}
		} else {
			/* send 4bytes per one time. */
			payload = data0[data_cnt] |
				data0[data_cnt + 1] << 8 |
				data0[data_cnt + 2] << 16 |
				data0[data_cnt + 3] << 24;

			debug("count = 4 payload = %x, %x %x %x %x\n",
				payload, *(u8 *)(data0 + data_cnt),
				data0[data_cnt + 1],
				data0[data_cnt + 2],
				data0[data_cnt + 3]);
		}
		exynos_mipi_dsi_wr_tx_data(dsim, payload);
	}
}

int exynos_mipi_dsi_wr_data(struct mipi_dsim_device *dsim, unsigned int data_id,
	const unsigned char *data0, unsigned int data1)
{
	unsigned int timeout = TRY_GET_FIFO_TIMEOUT;
	unsigned long delay_val, delay;
	unsigned int check_rx_ack = 0;

	if (dsim->state == DSIM_STATE_ULPS) {
		debug("state is ULPS.\n");

		return -EINVAL;
	}

	delay_val = MHZ / dsim->dsim_config->esc_clk;
	delay = 10 * delay_val;

	mdelay(delay);

	/* only if transfer mode is LPDT, wait SFR becomes empty. */
	if (dsim->state == DSIM_STATE_STOP) {
		while (!(exynos_mipi_dsi_get_fifo_state(dsim) &
				SFR_HEADER_EMPTY)) {
			if ((timeout--) > 0)
				mdelay(1);
			else {
				debug("SRF header fifo is not empty.\n");
				return -EINVAL;
			}
		}
	}

	switch (data_id) {
	/* short packet types of packet types for command. */
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE:
		debug("data0 = %x data1 = %x\n",
				data0[0], data0[1]);
		exynos_mipi_dsi_wr_tx_header(dsim, data_id, data0[0], data0[1]);
		if (check_rx_ack) {
			/* process response func should be implemented */
			return 0;
		} else {
			return -EINVAL;
		}

	/* general command */
	case MIPI_DSI_COLOR_MODE_OFF:
	case MIPI_DSI_COLOR_MODE_ON:
	case MIPI_DSI_SHUTDOWN_PERIPHERAL:
	case MIPI_DSI_TURN_ON_PERIPHERAL:
		exynos_mipi_dsi_wr_tx_header(dsim, data_id, data0[0], data0[1]);
		if (check_rx_ack) {
			/* process response func should be implemented. */
			return 0;
		} else {
			return -EINVAL;
		}

	/* packet types for video data */
	case MIPI_DSI_V_SYNC_START:
	case MIPI_DSI_V_SYNC_END:
	case MIPI_DSI_H_SYNC_START:
	case MIPI_DSI_H_SYNC_END:
	case MIPI_DSI_END_OF_TRANSMISSION:
		return 0;

	/* short and response packet types for command */
	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
	case MIPI_DSI_DCS_READ:
		exynos_mipi_dsi_clear_all_interrupt(dsim);
		exynos_mipi_dsi_wr_tx_header(dsim, data_id, data0[0], data0[1]);
		/* process response func should be implemented. */
		return 0;

	/* long packet type and null packet */
	case MIPI_DSI_NULL_PACKET:
	case MIPI_DSI_BLANKING_PACKET:
		return 0;
	case MIPI_DSI_GENERIC_LONG_WRITE:
	case MIPI_DSI_DCS_LONG_WRITE:
	{
		unsigned int payload = 0;

		/* if data count is less then 4, then send 3bytes data.  */
		if (data1 < 4) {
			payload = data0[0] |
				data0[1] << 8 |
				data0[2] << 16;

			exynos_mipi_dsi_wr_tx_data(dsim, payload);

			debug("count = %d payload = %x,%x %x %x\n",
				data1, payload, data0[0],
				data0[1], data0[2]);
		} else {
			/* in case that data count is more then 4 */
			exynos_mipi_dsi_long_data_wr(dsim, data0, data1);
		}

		/* put data into header fifo */
		exynos_mipi_dsi_wr_tx_header(dsim, data_id, data1 & 0xff,
			(data1 & 0xff00) >> 8);

	}
	if (check_rx_ack)
		/* process response func should be implemented. */
		return 0;
	else
		return -EINVAL;

	/* packet typo for video data */
	case MIPI_DSI_PACKED_PIXEL_STREAM_16:
	case MIPI_DSI_PACKED_PIXEL_STREAM_18:
	case MIPI_DSI_PIXEL_STREAM_3BYTE_18:
	case MIPI_DSI_PACKED_PIXEL_STREAM_24:
		if (check_rx_ack) {
			/* process response func should be implemented. */
			return 0;
		} else {
			return -EINVAL;
		}
	default:
		debug("data id %x is not supported current DSI spec.\n",
			data_id);

		return -EINVAL;
	}

	return 0;
}

int exynos_mipi_dsi_pll_on(struct mipi_dsim_device *dsim, unsigned int enable)
{
	int sw_timeout;

	if (enable) {
		sw_timeout = 1000;

		exynos_mipi_dsi_clear_interrupt(dsim);
		exynos_mipi_dsi_enable_pll(dsim, 1);
		while (1) {
			sw_timeout--;
			if (exynos_mipi_dsi_is_pll_stable(dsim))
				return 0;
			if (sw_timeout == 0)
				return -EINVAL;
		}
	} else
		exynos_mipi_dsi_enable_pll(dsim, 0);

	return 0;
}

unsigned long exynos_mipi_dsi_change_pll(struct mipi_dsim_device *dsim,
	unsigned int pre_divider, unsigned int main_divider,
	unsigned int scaler)
{
	unsigned long dfin_pll, dfvco, dpll_out;
	unsigned int i, freq_band = 0xf;

	dfin_pll = (FIN_HZ / pre_divider);

	/******************************************************
	 *	Serial Clock(=ByteClk X 8)	FreqBand[3:0] *
	 ******************************************************
	 *	~ 99.99 MHz			0000
	 *	100 ~ 119.99 MHz		0001
	 *	120 ~ 159.99 MHz		0010
	 *	160 ~ 199.99 MHz		0011
	 *	200 ~ 239.99 MHz		0100
	 *	140 ~ 319.99 MHz		0101
	 *	320 ~ 389.99 MHz		0110
	 *	390 ~ 449.99 MHz		0111
	 *	450 ~ 509.99 MHz		1000
	 *	510 ~ 559.99 MHz		1001
	 *	560 ~ 639.99 MHz		1010
	 *	640 ~ 689.99 MHz		1011
	 *	690 ~ 769.99 MHz		1100
	 *	770 ~ 869.99 MHz		1101
	 *	870 ~ 949.99 MHz		1110
	 *	950 ~ 1000 MHz			1111
	 ******************************************************/
	if (dfin_pll < DFIN_PLL_MIN_HZ || dfin_pll > DFIN_PLL_MAX_HZ) {
		debug("fin_pll range should be 6MHz ~ 12MHz\n");
		exynos_mipi_dsi_enable_afc(dsim, 0, 0);
	} else {
		if (dfin_pll < 7 * MHZ)
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x1);
		else if (dfin_pll < 8 * MHZ)
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x0);
		else if (dfin_pll < 9 * MHZ)
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x3);
		else if (dfin_pll < 10 * MHZ)
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x2);
		else if (dfin_pll < 11 * MHZ)
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x5);
		else
			exynos_mipi_dsi_enable_afc(dsim, 1, 0x4);
	}

	dfvco = dfin_pll * main_divider;
	debug("dfvco = %lu, dfin_pll = %lu, main_divider = %d\n",
				dfvco, dfin_pll, main_divider);
	if (dfvco < DFVCO_MIN_HZ || dfvco > DFVCO_MAX_HZ)
		debug("fvco range should be 500MHz ~ 1000MHz\n");

	dpll_out = dfvco / (1 << scaler);
	debug("dpll_out = %lu, dfvco = %lu, scaler = %d\n",
		dpll_out, dfvco, scaler);

	for (i = 0; i < ARRAY_SIZE(dpll_table); i++) {
		if (dpll_out < dpll_table[i] * MHZ) {
			freq_band = i;
			break;
		}
	}

	debug("freq_band = %d\n", freq_band);

	exynos_mipi_dsi_pll_freq(dsim, pre_divider, main_divider, scaler);

	exynos_mipi_dsi_hs_zero_ctrl(dsim, 0);
	exynos_mipi_dsi_prep_ctrl(dsim, 0);

	/* Freq Band */
	exynos_mipi_dsi_pll_freq_band(dsim, freq_band);

	/* Stable time */
	exynos_mipi_dsi_pll_stable_time(dsim,
				dsim->dsim_config->pll_stable_time);

	/* Enable PLL */
	debug("FOUT of mipi dphy pll is %luMHz\n",
		(dpll_out / MHZ));

	return dpll_out;
}

int exynos_mipi_dsi_set_clock(struct mipi_dsim_device *dsim,
	unsigned int byte_clk_sel, unsigned int enable)
{
	unsigned int esc_div;
	unsigned long esc_clk_error_rate;
	unsigned long hs_clk = 0, byte_clk = 0, escape_clk = 0;

	if (enable) {
		dsim->e_clk_src = byte_clk_sel;

		/* Escape mode clock and byte clock source */
		exynos_mipi_dsi_set_byte_clock_src(dsim, byte_clk_sel);

		/* DPHY, DSIM Link : D-PHY clock out */
		if (byte_clk_sel == DSIM_PLL_OUT_DIV8) {
			hs_clk = exynos_mipi_dsi_change_pll(dsim,
				dsim->dsim_config->p, dsim->dsim_config->m,
				dsim->dsim_config->s);
			if (hs_clk == 0) {
				debug("failed to get hs clock.\n");
				return -EINVAL;
			}

			byte_clk = hs_clk / 8;
			exynos_mipi_dsi_enable_pll_bypass(dsim, 0);
			exynos_mipi_dsi_pll_on(dsim, 1);
		/* DPHY : D-PHY clock out, DSIM link : external clock out */
		} else if (byte_clk_sel == DSIM_EXT_CLK_DIV8)
			debug("not support EXT CLK source for MIPI DSIM\n");
		else if (byte_clk_sel == DSIM_EXT_CLK_BYPASS)
			debug("not support EXT CLK source for MIPI DSIM\n");

		/* escape clock divider */
		esc_div = byte_clk / (dsim->dsim_config->esc_clk);
		debug("esc_div = %d, byte_clk = %lu, esc_clk = %lu\n",
			esc_div, byte_clk, dsim->dsim_config->esc_clk);
		if ((byte_clk / esc_div) >= (20 * MHZ) ||
			(byte_clk / esc_div) > dsim->dsim_config->esc_clk)
			esc_div += 1;

		escape_clk = byte_clk / esc_div;
		debug("escape_clk = %lu, byte_clk = %lu, esc_div = %d\n",
			escape_clk, byte_clk, esc_div);

		/* enable escape clock. */
		exynos_mipi_dsi_enable_byte_clock(dsim, 1);

		/* enable byte clk and escape clock */
		exynos_mipi_dsi_set_esc_clk_prs(dsim, 1, esc_div);
		/* escape clock on lane */
		exynos_mipi_dsi_enable_esc_clk_on_lane(dsim,
			(DSIM_LANE_CLOCK | dsim->data_lane), 1);

		debug("byte clock is %luMHz\n",
			(byte_clk / MHZ));
		debug("escape clock that user's need is %lu\n",
			(dsim->dsim_config->esc_clk / MHZ));
		debug("escape clock divider is %x\n", esc_div);
		debug("escape clock is %luMHz\n",
			((byte_clk / esc_div) / MHZ));

		if ((byte_clk / esc_div) > escape_clk) {
			esc_clk_error_rate = escape_clk /
				(byte_clk / esc_div);
			debug("error rate is %lu over.\n",
				(esc_clk_error_rate / 100));
		} else if ((byte_clk / esc_div) < (escape_clk)) {
			esc_clk_error_rate = (byte_clk / esc_div) /
				escape_clk;
			debug("error rate is %lu under.\n",
				(esc_clk_error_rate / 100));
		}
	} else {
		exynos_mipi_dsi_enable_esc_clk_on_lane(dsim,
			(DSIM_LANE_CLOCK | dsim->data_lane), 0);
		exynos_mipi_dsi_set_esc_clk_prs(dsim, 0, 0);

		/* disable escape clock. */
		exynos_mipi_dsi_enable_byte_clock(dsim, 0);

		if (byte_clk_sel == DSIM_PLL_OUT_DIV8)
			exynos_mipi_dsi_pll_on(dsim, 0);
	}

	return 0;
}

int exynos_mipi_dsi_init_dsim(struct mipi_dsim_device *dsim)
{
	dsim->state = DSIM_STATE_INIT;

	switch (dsim->dsim_config->e_no_data_lane) {
	case DSIM_DATA_LANE_1:
		dsim->data_lane = DSIM_LANE_DATA0;
		break;
	case DSIM_DATA_LANE_2:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1;
		break;
	case DSIM_DATA_LANE_3:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2;
		break;
	case DSIM_DATA_LANE_4:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2 | DSIM_LANE_DATA3;
		break;
	default:
		debug("data lane is invalid.\n");
		return -EINVAL;
	};

	exynos_mipi_dsi_sw_reset(dsim);
	exynos_mipi_dsi_dp_dn_swap(dsim, 0);

	return 0;
}

int exynos_mipi_dsi_enable_frame_done_int(struct mipi_dsim_device *dsim,
	unsigned int enable)
{
	/* enable only frame done interrupt */
	exynos_mipi_dsi_set_interrupt_mask(dsim, INTMSK_FRAME_DONE, enable);

	return 0;
}

static void convert_to_fb_videomode(struct fb_videomode *mode1,
				    struct vidinfo *mode2)
{
	mode1->xres = mode2->vl_width;
	mode1->yres = mode2->vl_height;
	mode1->upper_margin = mode2->vl_vfpd;
	mode1->lower_margin = mode2->vl_vbpd;
	mode1->left_margin = mode2->vl_hfpd;
	mode1->right_margin = mode2->vl_hbpd;
	mode1->vsync_len = mode2->vl_vspw;
	mode1->hsync_len = mode2->vl_hspw;
}

int exynos_mipi_dsi_set_display_mode(struct mipi_dsim_device *dsim,
	struct mipi_dsim_config *dsim_config)
{
	struct exynos_platform_mipi_dsim *dsim_pd;
	struct fb_videomode lcd_video;
	struct vidinfo *vid;

	dsim_pd = (struct exynos_platform_mipi_dsim *)dsim->pd;
	vid = (struct vidinfo *)dsim_pd->lcd_panel_info;

	convert_to_fb_videomode(&lcd_video, vid);

	/* in case of VIDEO MODE (RGB INTERFACE), it sets polarities. */
	if (dsim->dsim_config->e_interface == (u32) DSIM_VIDEO) {
		if (dsim->dsim_config->auto_vertical_cnt == 0) {
			exynos_mipi_dsi_set_main_disp_vporch(dsim,
				vid->vl_cmd_allow_len,
				lcd_video.upper_margin,
				lcd_video.lower_margin);
			exynos_mipi_dsi_set_main_disp_hporch(dsim,
				lcd_video.left_margin,
				lcd_video.right_margin);
			exynos_mipi_dsi_set_main_disp_sync_area(dsim,
				lcd_video.vsync_len,
				lcd_video.hsync_len);
		}
	}

	exynos_mipi_dsi_set_main_disp_resol(dsim, lcd_video.xres,
			lcd_video.yres);

	exynos_mipi_dsi_display_config(dsim, dsim->dsim_config);

	debug("lcd panel ==> width = %d, height = %d\n",
			lcd_video.xres, lcd_video.yres);

	return 0;
}

int exynos_mipi_dsi_init_link(struct mipi_dsim_device *dsim)
{
	unsigned int time_out = 100;

	switch (dsim->state) {
	case DSIM_STATE_INIT:
		exynos_mipi_dsi_init_fifo_pointer(dsim, 0x1f);

		/* dsi configuration */
		exynos_mipi_dsi_init_config(dsim);
		exynos_mipi_dsi_enable_lane(dsim, DSIM_LANE_CLOCK, 1);
		exynos_mipi_dsi_enable_lane(dsim, dsim->data_lane, 1);

		/* set clock configuration */
		exynos_mipi_dsi_set_clock(dsim,
					dsim->dsim_config->e_byte_clk, 1);

		/* check clock and data lane state are stop state */
		while (!(exynos_mipi_dsi_is_lane_state(dsim))) {
			time_out--;
			if (time_out == 0) {
				debug("DSI Master is not stop state.\n");
				debug("Check initialization process\n");

				return -EINVAL;
			}
		}

		dsim->state = DSIM_STATE_STOP;

		/* BTA sequence counters */
		exynos_mipi_dsi_set_stop_state_counter(dsim,
			dsim->dsim_config->stop_holding_cnt);
		exynos_mipi_dsi_set_bta_timeout(dsim,
			dsim->dsim_config->bta_timeout);
		exynos_mipi_dsi_set_lpdr_timeout(dsim,
			dsim->dsim_config->rx_timeout);

		return 0;
	default:
		debug("DSI Master is already init.\n");
		return 0;
	}

	return 0;
}

int exynos_mipi_dsi_set_hs_enable(struct mipi_dsim_device *dsim)
{
	if (dsim->state == DSIM_STATE_STOP) {
		if (dsim->e_clk_src != DSIM_EXT_CLK_BYPASS) {
			dsim->state = DSIM_STATE_HSCLKEN;

			 /* set LCDC and CPU transfer mode to HS. */
			exynos_mipi_dsi_set_lcdc_transfer_mode(dsim, 0);
			exynos_mipi_dsi_set_cpu_transfer_mode(dsim, 0);

			exynos_mipi_dsi_enable_hs_clock(dsim, 1);

			return 0;
		} else
			debug("clock source is external bypass.\n");
	} else
		debug("DSIM is not stop state.\n");

	return 0;
}

int exynos_mipi_dsi_set_data_transfer_mode(struct mipi_dsim_device *dsim,
		unsigned int mode)
{
	if (mode) {
		if (dsim->state != DSIM_STATE_HSCLKEN) {
			debug("HS Clock lane is not enabled.\n");
			return -EINVAL;
		}

		exynos_mipi_dsi_set_lcdc_transfer_mode(dsim, 0);
	} else {
		if (dsim->state == DSIM_STATE_INIT || dsim->state ==
			DSIM_STATE_ULPS) {
			debug("DSI Master is not STOP or HSDT state.\n");
			return -EINVAL;
		}

		exynos_mipi_dsi_set_cpu_transfer_mode(dsim, 0);
	}

	return 0;
}

int exynos_mipi_dsi_get_frame_done_status(struct mipi_dsim_device *dsim)
{
	return _exynos_mipi_dsi_get_frame_done_status(dsim);
}

int exynos_mipi_dsi_clear_frame_done(struct mipi_dsim_device *dsim)
{
	_exynos_mipi_dsi_clear_frame_done(dsim);

	return 0;
}
