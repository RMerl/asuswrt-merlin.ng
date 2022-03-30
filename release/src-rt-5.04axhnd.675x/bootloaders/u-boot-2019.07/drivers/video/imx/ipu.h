/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Porting to u-boot:
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Linux IPU driver for MX51:
 *
 * (C) Copyright 2005-2010 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_IPU_H__
#define __ASM_ARCH_IPU_H__

#include <linux/types.h>
#include <ipu_pixfmt.h>

#define IDMA_CHAN_INVALID	0xFF
#define HIGH_RESOLUTION_WIDTH	1024

struct clk {
	const char *name;
	int id;
	/* Source clock this clk depends on */
	struct clk *parent;
	/* Secondary clock to enable/disable with this clock */
	struct clk *secondary;
	/* Current clock rate */
	unsigned long rate;
	/* Reference count of clock enable/disable */
	__s8 usecount;
	/* Register bit position for clock's enable/disable control. */
	u8 enable_shift;
	/* Register address for clock's enable/disable control. */
	void *enable_reg;
	u32 flags;
	/*
	 * Function ptr to recalculate the clock's rate based on parent
	 * clock's rate
	 */
	void (*recalc) (struct clk *);
	/*
	 * Function ptr to set the clock to a new rate. The rate must match a
	 * supported rate returned from round_rate. Leave blank if clock is not
	* programmable
	 */
	int (*set_rate) (struct clk *, unsigned long);
	/*
	 * Function ptr to round the requested clock rate to the nearest
	 * supported rate that is less than or equal to the requested rate.
	 */
	unsigned long (*round_rate) (struct clk *, unsigned long);
	/*
	 * Function ptr to enable the clock. Leave blank if clock can not
	 * be gated.
	 */
	int (*enable) (struct clk *);
	/*
	 * Function ptr to disable the clock. Leave blank if clock can not
	 * be gated.
	 */
	void (*disable) (struct clk *);
	/* Function ptr to set the parent clock of the clock. */
	int (*set_parent) (struct clk *, struct clk *);
};

/*
 * Enumeration of Synchronous (Memory-less) panel types
 */
typedef enum {
	IPU_PANEL_SHARP_TFT,
	IPU_PANEL_TFT,
} ipu_panel_t;

/*
 * IPU Driver channels definitions.
 * Note these are different from IDMA channels
 */
#define IPU_MAX_CH	32
#define _MAKE_CHAN(num, v_in, g_in, a_in, out) \
	((num << 24) | (v_in << 18) | (g_in << 12) | (a_in << 6) | out)
#define _MAKE_ALT_CHAN(ch)		(ch | (IPU_MAX_CH << 24))
#define IPU_CHAN_ID(ch)			(ch >> 24)
#define IPU_CHAN_ALT(ch)		(ch & 0x02000000)
#define IPU_CHAN_ALPHA_IN_DMA(ch)	((uint32_t) (ch >> 6) & 0x3F)
#define IPU_CHAN_GRAPH_IN_DMA(ch)	((uint32_t) (ch >> 12) & 0x3F)
#define IPU_CHAN_VIDEO_IN_DMA(ch)	((uint32_t) (ch >> 18) & 0x3F)
#define IPU_CHAN_OUT_DMA(ch)		((uint32_t) (ch & 0x3F))
#define NO_DMA 0x3F
#define ALT	1

/*
 * Enumeration of IPU logical channels. An IPU logical channel is defined as a
 * combination of an input (memory to IPU), output (IPU to memory), and/or
 * secondary input IDMA channels and in some cases an Image Converter task.
 * Some channels consist of only an input or output.
 */
typedef enum {
	CHAN_NONE = -1,

	MEM_DC_SYNC = _MAKE_CHAN(7, 28, NO_DMA, NO_DMA, NO_DMA),
	MEM_DC_ASYNC = _MAKE_CHAN(8, 41, NO_DMA, NO_DMA, NO_DMA),
	MEM_BG_SYNC = _MAKE_CHAN(9, 23, NO_DMA, 51, NO_DMA),
	MEM_FG_SYNC = _MAKE_CHAN(10, 27, NO_DMA, 31, NO_DMA),

	MEM_BG_ASYNC0 = _MAKE_CHAN(11, 24, NO_DMA, 52, NO_DMA),
	MEM_FG_ASYNC0 = _MAKE_CHAN(12, 29, NO_DMA, 33, NO_DMA),
	MEM_BG_ASYNC1 = _MAKE_ALT_CHAN(MEM_BG_ASYNC0),
	MEM_FG_ASYNC1 = _MAKE_ALT_CHAN(MEM_FG_ASYNC0),

	DIRECT_ASYNC0 = _MAKE_CHAN(13, NO_DMA, NO_DMA, NO_DMA, NO_DMA),
	DIRECT_ASYNC1 = _MAKE_CHAN(14, NO_DMA, NO_DMA, NO_DMA, NO_DMA),

} ipu_channel_t;

/*
 * Enumeration of types of buffers for a logical channel.
 */
typedef enum {
	IPU_OUTPUT_BUFFER = 0,	/*< Buffer for output from IPU */
	IPU_ALPHA_IN_BUFFER = 1,	/*< Buffer for input to IPU */
	IPU_GRAPH_IN_BUFFER = 2,	/*< Buffer for input to IPU */
	IPU_VIDEO_IN_BUFFER = 3,	/*< Buffer for input to IPU */
	IPU_INPUT_BUFFER = IPU_VIDEO_IN_BUFFER,
	IPU_SEC_INPUT_BUFFER = IPU_GRAPH_IN_BUFFER,
} ipu_buffer_t;

#define IPU_PANEL_SERIAL		1
#define IPU_PANEL_PARALLEL		2

struct ipu_channel {
	u8 video_in_dma;
	u8 alpha_in_dma;
	u8 graph_in_dma;
	u8 out_dma;
};

enum ipu_dmfc_type {
	DMFC_NORMAL = 0,
	DMFC_HIGH_RESOLUTION_DC,
	DMFC_HIGH_RESOLUTION_DP,
	DMFC_HIGH_RESOLUTION_ONLY_DP,
};


/*
 * Union of initialization parameters for a logical channel.
 */
typedef union {
	struct {
		uint32_t di;
		unsigned char interlaced;
	} mem_dc_sync;
	struct {
		uint32_t temp;
	} mem_sdc_fg;
	struct {
		uint32_t di;
		unsigned char interlaced;
		uint32_t in_pixel_fmt;
		uint32_t out_pixel_fmt;
		unsigned char alpha_chan_en;
	} mem_dp_bg_sync;
	struct {
		uint32_t temp;
	} mem_sdc_bg;
	struct {
		uint32_t di;
		unsigned char interlaced;
		uint32_t in_pixel_fmt;
		uint32_t out_pixel_fmt;
		unsigned char alpha_chan_en;
	} mem_dp_fg_sync;
} ipu_channel_params_t;

/*
 * Enumeration of IPU interrupts.
 */
enum ipu_irq_line {
	IPU_IRQ_DP_SF_END = 448 + 3,
	IPU_IRQ_DC_FC_1 = 448 + 9,
};

/*
 * Bitfield of Display Interface signal polarities.
 */
typedef struct {
	unsigned datamask_en:1;
	unsigned ext_clk:1;
	unsigned interlaced:1;
	unsigned odd_field_first:1;
	unsigned clksel_en:1;
	unsigned clkidle_en:1;
	unsigned data_pol:1;	/* true = inverted */
	unsigned clk_pol:1;	/* true = rising edge */
	unsigned enable_pol:1;
	unsigned Hsync_pol:1;	/* true = active high */
	unsigned Vsync_pol:1;
} ipu_di_signal_cfg_t;

typedef enum {
	RGB,
	YCbCr,
	YUV
} ipu_color_space_t;

/* Common IPU API */
int32_t ipu_init_channel(ipu_channel_t channel, ipu_channel_params_t *params);
void ipu_uninit_channel(ipu_channel_t channel);

int32_t ipu_init_channel_buffer(ipu_channel_t channel, ipu_buffer_t type,
				uint32_t pixel_fmt,
				uint16_t width, uint16_t height,
				uint32_t stride,
				dma_addr_t phyaddr_0, dma_addr_t phyaddr_1,
				uint32_t u_offset, uint32_t v_offset);

int32_t ipu_update_channel_buffer(ipu_channel_t channel, ipu_buffer_t type,
				  uint32_t bufNum, dma_addr_t phyaddr);

int32_t ipu_is_channel_busy(ipu_channel_t channel);
void ipu_clear_buffer_ready(ipu_channel_t channel, ipu_buffer_t type,
		uint32_t bufNum);
int32_t ipu_enable_channel(ipu_channel_t channel);
int32_t ipu_disable_channel(ipu_channel_t channel);

int32_t ipu_init_sync_panel(int disp,
			    uint32_t pixel_clk,
			    uint16_t width, uint16_t height,
			    uint32_t pixel_fmt,
			    uint16_t h_start_width, uint16_t h_sync_width,
			    uint16_t h_end_width, uint16_t v_start_width,
			    uint16_t v_sync_width, uint16_t v_end_width,
			    uint32_t v_to_h_sync, ipu_di_signal_cfg_t sig);

int32_t ipu_disp_set_global_alpha(ipu_channel_t channel, unsigned char enable,
				  uint8_t alpha);
int32_t ipu_disp_set_color_key(ipu_channel_t channel, unsigned char enable,
			       uint32_t colorKey);

uint32_t bytes_per_pixel(uint32_t fmt);

void clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);
u32 clk_get_rate(struct clk *clk);
int clk_set_rate(struct clk *clk, unsigned long rate);
long clk_round_rate(struct clk *clk, unsigned long rate);
int clk_set_parent(struct clk *clk, struct clk *parent);
int clk_get_usecount(struct clk *clk);
struct clk *clk_get_parent(struct clk *clk);

void ipu_dump_registers(void);
int ipu_probe(void);
bool ipu_clk_enabled(void);

void ipu_dmfc_init(int dmfc_type, int first);
void ipu_init_dc_mappings(void);
void ipu_dmfc_set_wait4eot(int dma_chan, int width);
void ipu_dc_init(int dc_chan, int di, unsigned char interlaced);
void ipu_dc_uninit(int dc_chan);
void ipu_dp_dc_enable(ipu_channel_t channel);
int ipu_dp_init(ipu_channel_t channel, uint32_t in_pixel_fmt,
		 uint32_t out_pixel_fmt);
void ipu_dp_uninit(ipu_channel_t channel);
void ipu_dp_dc_disable(ipu_channel_t channel, unsigned char swap);
ipu_color_space_t format_to_colorspace(uint32_t fmt);
#endif
