/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * logicore_dp_tx.h
 *
 * Driver for XILINX LogiCore DisplayPort v6.1 TX (Source)
 *
 * (C) Copyright 2016
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifndef __GDSYS_LOGICORE_DP_TX_H__
#define __GDSYS_LOGICORE_DP_TX_H__

/*
 * struct logicore_dp_tx_msa - Main Stream Attributes (MSA)
 * @pixel_clock_hz:            The pixel clock of the stream (in Hz)
 * @bits_per_color:            Number of bits per color component
 * @h_active:                  Horizontal active resolution (pixels)
 * @h_start:                   Horizontal blank start (in pixels)
 * @h_sync_polarity:           Horizontal sync polarity
 *			       (0 = negative | 1 = positive)
 * @h_sync_width:              Horizontal sync width (pixels)
 * @h_total:                   Horizontal total (pixels)
 * @v_active:                  Vertical active resolution (lines)
 * @v_start:                   Vertical blank start (in lines).
 * @v_sync_polarity:           Vertical sync polarity
 *			       (0 = negative | 1 = positive)
 * @v_sync_width:              Vertical sync width (lines)
 * @v_total:                   Vertical total (lines)
 * @override_user_pixel_width: If true, the value stored for user_pixel_width
 *			       will be used as the pixel width.
 * @user_pixel_width:          The width of the user data input port.
 *
 * This is a stripped down version of struct main_stream_attributes that
 * contains only the parameters that are not set by cfg_msa_recalculate()
 */
struct logicore_dp_tx_msa {
	u32 pixel_clock_hz;
	u32 bits_per_color;
	u16 h_active;
	u32 h_start;
	bool h_sync_polarity;
	u16 h_sync_width;
	u16 h_total;
	u16 v_active;
	u32 v_start;
	bool v_sync_polarity;
	u16 v_sync_width;
	u16 v_total;
	bool override_user_pixel_width;
	u32 user_pixel_width;
};

#endif /* __GDSYS_LOGICORE_DP_TX_H__ */
