/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#ifndef _TILCDC_H
#define _TILCDC_H

/**
 * tilcdc_panel_info: Panel parameters
 *
 * @ac_bias: AC Bias Pin Frequency
 * @ac_bias_intrpt: AC Bias Pin Transitions per Interrupt
 * @dma_burst_sz: DMA burst size
 * @bpp: Bits per pixel
 * @fdd: FIFO DMA Request Delay
 * @tft_alt_mode: TFT Alternative Signal Mapping (Only for active)
 * @invert_pxl_clk: Invert pixel clock
 * @sync_edge: Horizontal and Vertical Sync Edge: 0=rising 1=falling
 * @sync_ctrl: Horizontal and Vertical Sync: Control: 0=ignore
 * @raster_order: Raster Data Order Select: 1=Most-to-least 0=Least-to-most
 * @fifo_th: DMA FIFO threshold
 */
struct tilcdc_panel_info {
	u32 ac_bias;
	u32 ac_bias_intrpt;
	u32 dma_burst_sz;
	u32 bpp;
	u32 fdd;
	bool tft_alt_mode;
	bool invert_pxl_clk;
	u32 sync_edge;
	u32 sync_ctrl;
	u32 raster_order;
	u32 fifo_th;
};

#endif /* _TILCDC_H */
