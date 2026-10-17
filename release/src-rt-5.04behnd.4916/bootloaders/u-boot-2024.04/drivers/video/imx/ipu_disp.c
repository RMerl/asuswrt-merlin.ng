// SPDX-License-Identifier: GPL-2.0+
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

/* #define DEBUG */

#include <common.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include "ipu.h"
#include "ipu_regs.h"

enum csc_type_t {
	RGB2YUV = 0,
	YUV2RGB,
	RGB2RGB,
	YUV2YUV,
	CSC_NONE,
	CSC_NUM
};

struct dp_csc_param_t {
	int mode;
	const int (*coeff)[5][3];
};

#define SYNC_WAVE 0

/* DC display ID assignments */
#define DC_DISP_ID_SYNC(di)	(di)
#define DC_DISP_ID_SERIAL	2
#define DC_DISP_ID_ASYNC	3

int dmfc_type_setup;
static int dmfc_size_28, dmfc_size_29, dmfc_size_24, dmfc_size_27, dmfc_size_23;
int g_di1_tvout;

extern struct clk *g_ipu_clk;
extern struct clk *g_ldb_clk;
extern struct clk *g_di_clk[2];
extern struct clk *g_pixel_clk[2];

extern unsigned char g_ipu_clk_enabled;
extern unsigned char g_dc_di_assignment[];

void ipu_dmfc_init(int dmfc_type, int first)
{
	u32 dmfc_wr_chan, dmfc_dp_chan;

	if (first) {
		if (dmfc_type_setup > dmfc_type)
			dmfc_type = dmfc_type_setup;
		else
			dmfc_type_setup = dmfc_type;

		/* disable DMFC-IC channel*/
		__raw_writel(0x2, DMFC_IC_CTRL);
	} else if (dmfc_type_setup >= DMFC_HIGH_RESOLUTION_DC) {
		printf("DMFC high resolution has set, will not change\n");
		return;
	} else
		dmfc_type_setup = dmfc_type;

	if (dmfc_type == DMFC_HIGH_RESOLUTION_DC) {
		/* 1 - segment 0~3;
		 * 5B - segement 4, 5;
		 * 5F - segement 6, 7;
		 * 1C, 2C and 6B, 6F unused;
		 */
		debug("IPU DMFC DC HIGH RES: 1(0~3), 5B(4,5), 5F(6,7)\n");
		dmfc_wr_chan = 0x00000088;
		dmfc_dp_chan = 0x00009694;
		dmfc_size_28 = 256 * 4;
		dmfc_size_29 = 0;
		dmfc_size_24 = 0;
		dmfc_size_27 = 128 * 4;
		dmfc_size_23 = 128 * 4;
	} else if (dmfc_type == DMFC_HIGH_RESOLUTION_DP) {
		/* 1 - segment 0, 1;
		 * 5B - segement 2~5;
		 * 5F - segement 6,7;
		 * 1C, 2C and 6B, 6F unused;
		 */
		debug("IPU DMFC DP HIGH RES: 1(0,1), 5B(2~5), 5F(6,7)\n");
		dmfc_wr_chan = 0x00000090;
		dmfc_dp_chan = 0x0000968a;
		dmfc_size_28 = 128 * 4;
		dmfc_size_29 = 0;
		dmfc_size_24 = 0;
		dmfc_size_27 = 128 * 4;
		dmfc_size_23 = 256 * 4;
	} else if (dmfc_type == DMFC_HIGH_RESOLUTION_ONLY_DP) {
		/* 5B - segement 0~3;
		 * 5F - segement 4~7;
		 * 1, 1C, 2C and 6B, 6F unused;
		 */
		debug("IPU DMFC ONLY-DP HIGH RES: 5B(0~3), 5F(4~7)\n");
		dmfc_wr_chan = 0x00000000;
		dmfc_dp_chan = 0x00008c88;
		dmfc_size_28 = 0;
		dmfc_size_29 = 0;
		dmfc_size_24 = 0;
		dmfc_size_27 = 256 * 4;
		dmfc_size_23 = 256 * 4;
	} else {
		/* 1 - segment 0, 1;
		 * 5B - segement 4, 5;
		 * 5F - segement 6, 7;
		 * 1C, 2C and 6B, 6F unused;
		 */
		debug("IPU DMFC NORMAL mode: 1(0~1), 5B(4,5), 5F(6,7)\n");
		dmfc_wr_chan = 0x00000090;
		dmfc_dp_chan = 0x00009694;
		dmfc_size_28 = 128 * 4;
		dmfc_size_29 = 0;
		dmfc_size_24 = 0;
		dmfc_size_27 = 128 * 4;
		dmfc_size_23 = 128 * 4;
	}
	__raw_writel(dmfc_wr_chan, DMFC_WR_CHAN);
	__raw_writel(0x202020F6, DMFC_WR_CHAN_DEF);
	__raw_writel(dmfc_dp_chan, DMFC_DP_CHAN);
	/* Enable chan 5 watermark set at 5 bursts and clear at 7 bursts */
	__raw_writel(0x2020F6F6, DMFC_DP_CHAN_DEF);
}

void ipu_dmfc_set_wait4eot(int dma_chan, int width)
{
	u32 dmfc_gen1 = __raw_readl(DMFC_GENERAL1);

	if (width >= HIGH_RESOLUTION_WIDTH) {
		if (dma_chan == 23)
			ipu_dmfc_init(DMFC_HIGH_RESOLUTION_DP, 0);
		else if (dma_chan == 28)
			ipu_dmfc_init(DMFC_HIGH_RESOLUTION_DC, 0);
	}

	if (dma_chan == 23) { /*5B*/
		if (dmfc_size_23 / width > 3)
			dmfc_gen1 |= 1UL << 20;
		else
			dmfc_gen1 &= ~(1UL << 20);
	} else if (dma_chan == 24) { /*6B*/
		if (dmfc_size_24 / width > 1)
			dmfc_gen1 |= 1UL << 22;
		else
			dmfc_gen1 &= ~(1UL << 22);
	} else if (dma_chan == 27) { /*5F*/
		if (dmfc_size_27 / width > 2)
			dmfc_gen1 |= 1UL << 21;
		else
			dmfc_gen1 &= ~(1UL << 21);
	} else if (dma_chan == 28) { /*1*/
		if (dmfc_size_28 / width > 2)
			dmfc_gen1 |= 1UL << 16;
		else
			dmfc_gen1 &= ~(1UL << 16);
	} else if (dma_chan == 29) { /*6F*/
		if (dmfc_size_29 / width > 1)
			dmfc_gen1 |= 1UL << 23;
		else
			dmfc_gen1 &= ~(1UL << 23);
	}

	__raw_writel(dmfc_gen1, DMFC_GENERAL1);
}

static void ipu_di_data_wave_config(int di,
				     int wave_gen,
				     int access_size, int component_size)
{
	u32 reg;
	reg = (access_size << DI_DW_GEN_ACCESS_SIZE_OFFSET) |
	    (component_size << DI_DW_GEN_COMPONENT_SIZE_OFFSET);
	__raw_writel(reg, DI_DW_GEN(di, wave_gen));
}

static void ipu_di_data_pin_config(int di, int wave_gen, int di_pin, int set,
				    int up, int down)
{
	u32 reg;

	reg = __raw_readl(DI_DW_GEN(di, wave_gen));
	reg &= ~(0x3 << (di_pin * 2));
	reg |= set << (di_pin * 2);
	__raw_writel(reg, DI_DW_GEN(di, wave_gen));

	__raw_writel((down << 16) | up, DI_DW_SET(di, wave_gen, set));
}

static void ipu_di_sync_config(int di, int wave_gen,
				int run_count, int run_src,
				int offset_count, int offset_src,
				int repeat_count, int cnt_clr_src,
				int cnt_polarity_gen_en,
				int cnt_polarity_clr_src,
				int cnt_polarity_trigger_src,
				int cnt_up, int cnt_down)
{
	u32 reg;

	if ((run_count >= 0x1000) || (offset_count >= 0x1000) ||
		(repeat_count >= 0x1000) ||
		(cnt_up >= 0x400) || (cnt_down >= 0x400)) {
		printf("DI%d counters out of range.\n", di);
		return;
	}

	reg = (run_count << 19) | (++run_src << 16) |
	    (offset_count << 3) | ++offset_src;
	__raw_writel(reg, DI_SW_GEN0(di, wave_gen));
	reg = (cnt_polarity_gen_en << 29) | (++cnt_clr_src << 25) |
	    (++cnt_polarity_trigger_src << 12) | (++cnt_polarity_clr_src << 9);
	reg |= (cnt_down << 16) | cnt_up;
	if (repeat_count == 0) {
		/* Enable auto reload */
		reg |= 0x10000000;
	}
	__raw_writel(reg, DI_SW_GEN1(di, wave_gen));
	reg = __raw_readl(DI_STP_REP(di, wave_gen));
	reg &= ~(0xFFFF << (16 * ((wave_gen - 1) & 0x1)));
	reg |= repeat_count << (16 * ((wave_gen - 1) & 0x1));
	__raw_writel(reg, DI_STP_REP(di, wave_gen));
}

static void ipu_dc_map_config(int map, int byte_num, int offset, int mask)
{
	int ptr = map * 3 + byte_num;
	u32 reg;

	reg = __raw_readl(DC_MAP_CONF_VAL(ptr));
	reg &= ~(0xFFFF << (16 * (ptr & 0x1)));
	reg |= ((offset << 8) | mask) << (16 * (ptr & 0x1));
	__raw_writel(reg, DC_MAP_CONF_VAL(ptr));

	reg = __raw_readl(DC_MAP_CONF_PTR(map));
	reg &= ~(0x1F << ((16 * (map & 0x1)) + (5 * byte_num)));
	reg |= ptr << ((16 * (map & 0x1)) + (5 * byte_num));
	__raw_writel(reg, DC_MAP_CONF_PTR(map));
}

static void ipu_dc_map_clear(int map)
{
	u32 reg = __raw_readl(DC_MAP_CONF_PTR(map));
	__raw_writel(reg & ~(0xFFFF << (16 * (map & 0x1))),
		     DC_MAP_CONF_PTR(map));
}

static void ipu_dc_write_tmpl(int word, u32 opcode, u32 operand, int map,
			       int wave, int glue, int sync)
{
	u32 reg;
	int stop = 1;

	reg = sync;
	reg |= (glue << 4);
	reg |= (++wave << 11);
	reg |= (++map << 15);
	reg |= (operand << 20) & 0xFFF00000;
	__raw_writel(reg, ipu_dc_tmpl_reg + word * 2);

	reg = (operand >> 12);
	reg |= opcode << 4;
	reg |= (stop << 9);
	__raw_writel(reg, ipu_dc_tmpl_reg + word * 2 + 1);
}

static void ipu_dc_link_event(int chan, int event, int addr, int priority)
{
	u32 reg;

	reg = __raw_readl(DC_RL_CH(chan, event));
	reg &= ~(0xFFFF << (16 * (event & 0x1)));
	reg |= ((addr << 8) | priority) << (16 * (event & 0x1));
	__raw_writel(reg, DC_RL_CH(chan, event));
}

/* Y = R *  1.200 + G *  2.343 + B *  .453 + 0.250;
 * U = R * -.672 + G * -1.328 + B *  2.000 + 512.250.;
 * V = R *  2.000 + G * -1.672 + B * -.328 + 512.250.;
 */
static const int rgb2ycbcr_coeff[5][3] = {
	{0x4D, 0x96, 0x1D},
	{0x3D5, 0x3AB, 0x80},
	{0x80, 0x395, 0x3EB},
	{0x0000, 0x0200, 0x0200},	/* B0, B1, B2 */
	{0x2, 0x2, 0x2},	/* S0, S1, S2 */
};

/* R = (1.164 * (Y - 16)) + (1.596 * (Cr - 128));
 * G = (1.164 * (Y - 16)) - (0.392 * (Cb - 128)) - (0.813 * (Cr - 128));
 * B = (1.164 * (Y - 16)) + (2.017 * (Cb - 128);
 */
static const int ycbcr2rgb_coeff[5][3] = {
	{0x095, 0x000, 0x0CC},
	{0x095, 0x3CE, 0x398},
	{0x095, 0x0FF, 0x000},
	{0x3E42, 0x010A, 0x3DD6},	/*B0,B1,B2 */
	{0x1, 0x1, 0x1},	/*S0,S1,S2 */
};

#define mask_a(a) ((u32)(a) & 0x3FF)
#define mask_b(b) ((u32)(b) & 0x3FFF)

/* Pls keep S0, S1 and S2 as 0x2 by using this convertion */
static int rgb_to_yuv(int n, int red, int green, int blue)
{
	int c;
	c = red * rgb2ycbcr_coeff[n][0];
	c += green * rgb2ycbcr_coeff[n][1];
	c += blue * rgb2ycbcr_coeff[n][2];
	c /= 16;
	c += rgb2ycbcr_coeff[3][n] * 4;
	c += 8;
	c /= 16;
	if (c < 0)
		c = 0;
	if (c > 255)
		c = 255;
	return c;
}

/*
 * Row is for BG:	RGB2YUV YUV2RGB RGB2RGB YUV2YUV CSC_NONE
 * Column is for FG:	RGB2YUV YUV2RGB RGB2RGB YUV2YUV CSC_NONE
 */
static struct dp_csc_param_t dp_csc_array[CSC_NUM][CSC_NUM] = {
	{
		{DP_COM_CONF_CSC_DEF_BOTH, &rgb2ycbcr_coeff},
		{0, 0},
		{0, 0},
		{DP_COM_CONF_CSC_DEF_BG, &rgb2ycbcr_coeff},
		{DP_COM_CONF_CSC_DEF_BG, &rgb2ycbcr_coeff}
	},
	{
		{0, 0},
		{DP_COM_CONF_CSC_DEF_BOTH, &ycbcr2rgb_coeff},
		{DP_COM_CONF_CSC_DEF_BG, &ycbcr2rgb_coeff},
		{0, 0},
		{DP_COM_CONF_CSC_DEF_BG, &ycbcr2rgb_coeff}
	},
	{
		{0, 0},
		{DP_COM_CONF_CSC_DEF_FG, &ycbcr2rgb_coeff},
		{0, 0},
		{0, 0},
		{0, 0}
	},
	{
		{DP_COM_CONF_CSC_DEF_FG, &rgb2ycbcr_coeff},
		{0, 0},
		{0, 0},
		{0, 0},
		{0, 0}
	},
	{
		{DP_COM_CONF_CSC_DEF_FG, &rgb2ycbcr_coeff},
		{DP_COM_CONF_CSC_DEF_FG, &ycbcr2rgb_coeff},
		{0, 0},
		{0, 0},
		{0, 0}
	}
};

static enum csc_type_t fg_csc_type = CSC_NONE, bg_csc_type = CSC_NONE;
static int color_key_4rgb = 1;

static void ipu_dp_csc_setup(int dp, struct dp_csc_param_t dp_csc_param,
			unsigned char srm_mode_update)
{
	u32 reg;
	const int (*coeff)[5][3];

	if (dp_csc_param.mode >= 0) {
		reg = __raw_readl(DP_COM_CONF());
		reg &= ~DP_COM_CONF_CSC_DEF_MASK;
		reg |= dp_csc_param.mode;
		__raw_writel(reg, DP_COM_CONF());
	}

	coeff = dp_csc_param.coeff;

	if (coeff) {
		__raw_writel(mask_a((*coeff)[0][0]) |
				(mask_a((*coeff)[0][1]) << 16), DP_CSC_A_0());
		__raw_writel(mask_a((*coeff)[0][2]) |
				(mask_a((*coeff)[1][0]) << 16), DP_CSC_A_1());
		__raw_writel(mask_a((*coeff)[1][1]) |
				(mask_a((*coeff)[1][2]) << 16), DP_CSC_A_2());
		__raw_writel(mask_a((*coeff)[2][0]) |
				(mask_a((*coeff)[2][1]) << 16), DP_CSC_A_3());
		__raw_writel(mask_a((*coeff)[2][2]) |
				(mask_b((*coeff)[3][0]) << 16) |
				((*coeff)[4][0] << 30), DP_CSC_0());
		__raw_writel(mask_b((*coeff)[3][1]) | ((*coeff)[4][1] << 14) |
				(mask_b((*coeff)[3][2]) << 16) |
				((*coeff)[4][2] << 30), DP_CSC_1());
	}

	if (srm_mode_update) {
		reg = __raw_readl(IPU_SRM_PRI2) | 0x8;
		__raw_writel(reg, IPU_SRM_PRI2);
	}
}

int ipu_dp_init(ipu_channel_t channel, uint32_t in_pixel_fmt,
		 uint32_t out_pixel_fmt)
{
	int in_fmt, out_fmt;
	int dp;
	int partial = 0;
	uint32_t reg;

	if (channel == MEM_FG_SYNC) {
		dp = DP_SYNC;
		partial = 1;
	} else if (channel == MEM_BG_SYNC) {
		dp = DP_SYNC;
		partial = 0;
	} else if (channel == MEM_BG_ASYNC0) {
		dp = DP_ASYNC0;
		partial = 0;
	} else {
		return -EINVAL;
	}

	in_fmt = format_to_colorspace(in_pixel_fmt);
	out_fmt = format_to_colorspace(out_pixel_fmt);

	if (partial) {
		if (in_fmt == RGB) {
			if (out_fmt == RGB)
				fg_csc_type = RGB2RGB;
			else
				fg_csc_type = RGB2YUV;
		} else {
			if (out_fmt == RGB)
				fg_csc_type = YUV2RGB;
			else
				fg_csc_type = YUV2YUV;
		}
	} else {
		if (in_fmt == RGB) {
			if (out_fmt == RGB)
				bg_csc_type = RGB2RGB;
			else
				bg_csc_type = RGB2YUV;
		} else {
			if (out_fmt == RGB)
				bg_csc_type = YUV2RGB;
			else
				bg_csc_type = YUV2YUV;
		}
	}

	/* Transform color key from rgb to yuv if CSC is enabled */
	reg = __raw_readl(DP_COM_CONF());
	if (color_key_4rgb && (reg & DP_COM_CONF_GWCKE) &&
		(((fg_csc_type == RGB2YUV) && (bg_csc_type == YUV2YUV)) ||
		((fg_csc_type == YUV2YUV) && (bg_csc_type == RGB2YUV)) ||
		((fg_csc_type == YUV2YUV) && (bg_csc_type == YUV2YUV)) ||
		((fg_csc_type == YUV2RGB) && (bg_csc_type == YUV2RGB)))) {
		int red, green, blue;
		int y, u, v;
		uint32_t color_key = __raw_readl(DP_GRAPH_WIND_CTRL()) &
			0xFFFFFFL;

		debug("_ipu_dp_init color key 0x%x need change to yuv fmt!\n",
			color_key);

		red = (color_key >> 16) & 0xFF;
		green = (color_key >> 8) & 0xFF;
		blue = color_key & 0xFF;

		y = rgb_to_yuv(0, red, green, blue);
		u = rgb_to_yuv(1, red, green, blue);
		v = rgb_to_yuv(2, red, green, blue);
		color_key = (y << 16) | (u << 8) | v;

		reg = __raw_readl(DP_GRAPH_WIND_CTRL()) & 0xFF000000L;
		__raw_writel(reg | color_key, DP_GRAPH_WIND_CTRL());
		color_key_4rgb = 0;

		debug("_ipu_dp_init color key change to yuv fmt 0x%x!\n",
			color_key);
	}

	ipu_dp_csc_setup(dp, dp_csc_array[bg_csc_type][fg_csc_type], 1);

	return 0;
}

void ipu_dp_uninit(ipu_channel_t channel)
{
	int dp;
	int partial = 0;

	if (channel == MEM_FG_SYNC) {
		dp = DP_SYNC;
		partial = 1;
	} else if (channel == MEM_BG_SYNC) {
		dp = DP_SYNC;
		partial = 0;
	} else if (channel == MEM_BG_ASYNC0) {
		dp = DP_ASYNC0;
		partial = 0;
	} else {
		return;
	}

	if (partial)
		fg_csc_type = CSC_NONE;
	else
		bg_csc_type = CSC_NONE;

	ipu_dp_csc_setup(dp, dp_csc_array[bg_csc_type][fg_csc_type], 0);
}

void ipu_dc_init(int dc_chan, int di, unsigned char interlaced)
{
	u32 reg = 0;

	if ((dc_chan == 1) || (dc_chan == 5)) {
		if (interlaced) {
			ipu_dc_link_event(dc_chan, DC_EVT_NL, 0, 3);
			ipu_dc_link_event(dc_chan, DC_EVT_EOL, 0, 2);
			ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA, 0, 1);
		} else {
			if (di) {
				ipu_dc_link_event(dc_chan, DC_EVT_NL, 2, 3);
				ipu_dc_link_event(dc_chan, DC_EVT_EOL, 3, 2);
				ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA,
					4, 1);
			} else {
				ipu_dc_link_event(dc_chan, DC_EVT_NL, 5, 3);
				ipu_dc_link_event(dc_chan, DC_EVT_EOL, 6, 2);
				ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA,
					7, 1);
			}
		}
		ipu_dc_link_event(dc_chan, DC_EVT_NF, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NFIELD, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_EOF, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_EOFIELD, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR, 0, 0);

		reg = 0x2;
		reg |= DC_DISP_ID_SYNC(di) << DC_WR_CH_CONF_PROG_DISP_ID_OFFSET;
		reg |= di << 2;
		if (interlaced)
			reg |= DC_WR_CH_CONF_FIELD_MODE;
	} else if ((dc_chan == 8) || (dc_chan == 9)) {
		/* async channels */
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_W_0, 0x64, 1);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_W_1, 0x64, 1);

		reg = 0x3;
		reg |= DC_DISP_ID_SERIAL << DC_WR_CH_CONF_PROG_DISP_ID_OFFSET;
	}
	__raw_writel(reg, DC_WR_CH_CONF(dc_chan));

	__raw_writel(0x00000000, DC_WR_CH_ADDR(dc_chan));

	__raw_writel(0x00000084, DC_GEN);
}

void ipu_dc_uninit(int dc_chan)
{
	if ((dc_chan == 1) || (dc_chan == 5)) {
		ipu_dc_link_event(dc_chan, DC_EVT_NL, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_EOL, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NF, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NFIELD, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_EOF, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_EOFIELD, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR, 0, 0);
	} else if ((dc_chan == 8) || (dc_chan == 9)) {
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR_W_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR_W_1, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN_W_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN_W_1, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_W_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_W_1, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR_R_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_ADDR_R_1, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN_R_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_CHAN_R_1, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_R_0, 0, 0);
		ipu_dc_link_event(dc_chan, DC_EVT_NEW_DATA_R_1, 0, 0);
	}
}

void ipu_dp_dc_enable(ipu_channel_t channel)
{
	int di;
	uint32_t reg;
	uint32_t dc_chan;

	if (channel == MEM_DC_SYNC)
		dc_chan = 1;
	else if ((channel == MEM_BG_SYNC) || (channel == MEM_FG_SYNC))
		dc_chan = 5;
	else
		return;

	if (channel == MEM_FG_SYNC) {
		/* Enable FG channel */
		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg | DP_COM_CONF_FG_EN, DP_COM_CONF());

		reg = __raw_readl(IPU_SRM_PRI2) | 0x8;
		__raw_writel(reg, IPU_SRM_PRI2);
		return;
	}

	di = g_dc_di_assignment[dc_chan];

	/* Make sure other DC sync channel is not assigned same DI */
	reg = __raw_readl(DC_WR_CH_CONF(6 - dc_chan));
	if ((di << 2) == (reg & DC_WR_CH_CONF_PROG_DI_ID)) {
		reg &= ~DC_WR_CH_CONF_PROG_DI_ID;
		reg |= di ? 0 : DC_WR_CH_CONF_PROG_DI_ID;
		__raw_writel(reg, DC_WR_CH_CONF(6 - dc_chan));
	}

	reg = __raw_readl(DC_WR_CH_CONF(dc_chan));
	reg |= 4 << DC_WR_CH_CONF_PROG_TYPE_OFFSET;
	__raw_writel(reg, DC_WR_CH_CONF(dc_chan));

	clk_enable(g_pixel_clk[di]);
}

static unsigned char dc_swap;

void ipu_dp_dc_disable(ipu_channel_t channel, unsigned char swap)
{
	uint32_t reg;
	uint32_t csc;
	uint32_t dc_chan = 0;
	int timeout = 50;
	int irq = 0;

	dc_swap = swap;

	if (channel == MEM_DC_SYNC) {
		dc_chan = 1;
		irq = IPU_IRQ_DC_FC_1;
	} else if (channel == MEM_BG_SYNC) {
		dc_chan = 5;
		irq = IPU_IRQ_DP_SF_END;
	} else if (channel == MEM_FG_SYNC) {
		/* Disable FG channel */
		dc_chan = 5;

		reg = __raw_readl(DP_COM_CONF());
		csc = reg & DP_COM_CONF_CSC_DEF_MASK;
		if (csc == DP_COM_CONF_CSC_DEF_FG)
			reg &= ~DP_COM_CONF_CSC_DEF_MASK;

		reg &= ~DP_COM_CONF_FG_EN;
		__raw_writel(reg, DP_COM_CONF());

		reg = __raw_readl(IPU_SRM_PRI2) | 0x8;
		__raw_writel(reg, IPU_SRM_PRI2);

		timeout = 50;

		/*
		 * Wait for DC triple buffer to empty,
		 * this check is useful for tv overlay.
		 */
		if (g_dc_di_assignment[dc_chan] == 0)
			while ((__raw_readl(DC_STAT) & 0x00000002)
			       != 0x00000002) {
				udelay(2000);
				timeout -= 2;
				if (timeout <= 0)
					break;
			}
		else if (g_dc_di_assignment[dc_chan] == 1)
			while ((__raw_readl(DC_STAT) & 0x00000020)
			       != 0x00000020) {
				udelay(2000);
				timeout -= 2;
				if (timeout <= 0)
					break;
			}
		return;
	} else {
		return;
	}

	if (dc_swap) {
		/* Swap DC channel 1 and 5 settings, and disable old dc chan */
		reg = __raw_readl(DC_WR_CH_CONF(dc_chan));
		__raw_writel(reg, DC_WR_CH_CONF(6 - dc_chan));
		reg &= ~DC_WR_CH_CONF_PROG_TYPE_MASK;
		reg ^= DC_WR_CH_CONF_PROG_DI_ID;
		__raw_writel(reg, DC_WR_CH_CONF(dc_chan));
	} else {
		/* Make sure that we leave at the irq starting edge */
		__raw_writel(IPUIRQ_2_MASK(irq), IPUIRQ_2_STATREG(irq));
		do {
			reg = __raw_readl(IPUIRQ_2_STATREG(irq));
		} while (!(reg & IPUIRQ_2_MASK(irq)));

		reg = __raw_readl(DC_WR_CH_CONF(dc_chan));
		reg &= ~DC_WR_CH_CONF_PROG_TYPE_MASK;
		__raw_writel(reg, DC_WR_CH_CONF(dc_chan));

		reg = __raw_readl(IPU_DISP_GEN);
		if (g_dc_di_assignment[dc_chan])
			reg &= ~DI1_COUNTER_RELEASE;
		else
			reg &= ~DI0_COUNTER_RELEASE;
		__raw_writel(reg, IPU_DISP_GEN);

		/* Clock is already off because it must be done quickly, but
		   we need to fix the ref count */
		clk_disable(g_pixel_clk[g_dc_di_assignment[dc_chan]]);
	}
}

void ipu_init_dc_mappings(void)
{
	/* IPU_PIX_FMT_RGB24 */
	ipu_dc_map_clear(0);
	ipu_dc_map_config(0, 0, 7, 0xFF);
	ipu_dc_map_config(0, 1, 15, 0xFF);
	ipu_dc_map_config(0, 2, 23, 0xFF);

	/* IPU_PIX_FMT_RGB666 */
	ipu_dc_map_clear(1);
	ipu_dc_map_config(1, 0, 5, 0xFC);
	ipu_dc_map_config(1, 1, 11, 0xFC);
	ipu_dc_map_config(1, 2, 17, 0xFC);

	/* IPU_PIX_FMT_YUV444 */
	ipu_dc_map_clear(2);
	ipu_dc_map_config(2, 0, 15, 0xFF);
	ipu_dc_map_config(2, 1, 23, 0xFF);
	ipu_dc_map_config(2, 2, 7, 0xFF);

	/* IPU_PIX_FMT_RGB565 */
	ipu_dc_map_clear(3);
	ipu_dc_map_config(3, 0, 4, 0xF8);
	ipu_dc_map_config(3, 1, 10, 0xFC);
	ipu_dc_map_config(3, 2, 15, 0xF8);

	/* IPU_PIX_FMT_LVDS666 */
	ipu_dc_map_clear(4);
	ipu_dc_map_config(4, 0, 5, 0xFC);
	ipu_dc_map_config(4, 1, 13, 0xFC);
	ipu_dc_map_config(4, 2, 21, 0xFC);
}

static int ipu_pixfmt_to_map(uint32_t fmt)
{
	switch (fmt) {
	case IPU_PIX_FMT_GENERIC:
	case IPU_PIX_FMT_RGB24:
		return 0;
	case IPU_PIX_FMT_RGB666:
		return 1;
	case IPU_PIX_FMT_YUV444:
		return 2;
	case IPU_PIX_FMT_RGB565:
		return 3;
	case IPU_PIX_FMT_LVDS666:
		return 4;
	}

	return -1;
}

/*
 * This function is called to initialize a synchronous LCD panel.
 *
 * @param       disp            The DI the panel is attached to.
 *
 * @param       pixel_clk       Desired pixel clock frequency in Hz.
 *
 * @param       pixel_fmt       Input parameter for pixel format of buffer.
 *                              Pixel format is a FOURCC ASCII code.
 *
 * @param       width           The width of panel in pixels.
 *
 * @param       height          The height of panel in pixels.
 *
 * @param       hStartWidth     The number of pixel clocks between the HSYNC
 *                              signal pulse and the start of valid data.
 *
 * @param       hSyncWidth      The width of the HSYNC signal in units of pixel
 *                              clocks.
 *
 * @param       hEndWidth       The number of pixel clocks between the end of
 *                              valid data and the HSYNC signal for next line.
 *
 * @param       vStartWidth     The number of lines between the VSYNC
 *                              signal pulse and the start of valid data.
 *
 * @param       vSyncWidth      The width of the VSYNC signal in units of lines
 *
 * @param       vEndWidth       The number of lines between the end of valid
 *                              data and the VSYNC signal for next frame.
 *
 * @param       sig             Bitfield of signal polarities for LCD interface.
 *
 * Return:      This function returns 0 on success or negative error code on
 *              fail.
 */

int32_t ipu_init_sync_panel(int disp, uint32_t pixel_clk,
			    uint16_t width, uint16_t height,
			    uint32_t pixel_fmt,
			    uint16_t h_start_width, uint16_t h_sync_width,
			    uint16_t h_end_width, uint16_t v_start_width,
			    uint16_t v_sync_width, uint16_t v_end_width,
			    uint32_t v_to_h_sync, ipu_di_signal_cfg_t sig)
{
	uint32_t reg;
	uint32_t di_gen, vsync_cnt;
	uint32_t div, rounded_pixel_clk;
	uint32_t h_total, v_total;
	int map;
	struct clk *di_parent;

	debug("panel size = %d x %d\n", width, height);

	if ((v_sync_width == 0) || (h_sync_width == 0))
		return -EINVAL;

	/* adapt panel to ipu restricitions */
	if (v_end_width < 2) {
		v_end_width = 2;
		puts("WARNING: v_end_width (lower_margin) must be >= 2, adjusted\n");
	}

	h_total = width + h_sync_width + h_start_width + h_end_width;
	v_total = height + v_sync_width + v_start_width + v_end_width;

	/* Init clocking */
	debug("pixel clk = %dHz\n", pixel_clk);

	if (sig.ext_clk) {
		if (!(g_di1_tvout && (disp == 1))) { /*not round div for tvout*/
			/*
			 * Set the  PLL to be an even multiple
			 * of the pixel clock.
			 */
			if ((clk_get_usecount(g_pixel_clk[0]) == 0) &&
				(clk_get_usecount(g_pixel_clk[1]) == 0)) {
				di_parent = clk_get_parent(g_di_clk[disp]);
				rounded_pixel_clk =
					clk_round_rate(g_pixel_clk[disp],
						pixel_clk);
				div  = clk_get_rate(di_parent) /
					rounded_pixel_clk;
				if (div % 2)
					div++;
				if (clk_get_rate(di_parent) != div *
					rounded_pixel_clk)
					clk_set_rate(di_parent,
						div * rounded_pixel_clk);
				udelay(10000);
				clk_set_rate(g_di_clk[disp],
					2 * rounded_pixel_clk);
				udelay(10000);
			}
		}
		clk_set_parent(g_pixel_clk[disp], g_ldb_clk);
	} else {
		if (clk_get_usecount(g_pixel_clk[disp]) != 0)
			clk_set_parent(g_pixel_clk[disp], g_ipu_clk);
	}
	rounded_pixel_clk = clk_round_rate(g_pixel_clk[disp], pixel_clk);
	clk_set_rate(g_pixel_clk[disp], rounded_pixel_clk);
	udelay(5000);
	/* Get integer portion of divider */
	div = clk_get_rate(clk_get_parent(g_pixel_clk[disp])) /
		rounded_pixel_clk;

	ipu_di_data_wave_config(disp, SYNC_WAVE, div - 1, div - 1);
	ipu_di_data_pin_config(disp, SYNC_WAVE, DI_PIN15, 3, 0, div * 2);

	map = ipu_pixfmt_to_map(pixel_fmt);
	if (map < 0) {
		debug("IPU_DISP: No MAP\n");
		return -EINVAL;
	}

	di_gen = __raw_readl(DI_GENERAL(disp));

	if (sig.interlaced) {
		/* Setup internal HSYNC waveform */
		ipu_di_sync_config(
				disp,		/* display */
				1,		/* counter */
				h_total / 2 - 1,/* run count */
				DI_SYNC_CLK,	/* run_resolution */
				0,		/* offset */
				DI_SYNC_NONE,	/* offset resolution */
				0,		/* repeat count */
				DI_SYNC_NONE,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				0		/* COUNT DOWN */
				);

		/* Field 1 VSYNC waveform */
		ipu_di_sync_config(
				disp,		/* display */
				2,		/* counter */
				h_total - 1,	/* run count */
				DI_SYNC_CLK,	/* run_resolution */
				0,		/* offset */
				DI_SYNC_NONE,	/* offset resolution */
				0,		/* repeat count */
				DI_SYNC_NONE,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				4		/* COUNT DOWN */
				);

		/* Setup internal HSYNC waveform */
		ipu_di_sync_config(
				disp,		/* display */
				3,		/* counter */
				v_total * 2 - 1,/* run count */
				DI_SYNC_INT_HSYNC,	/* run_resolution */
				1,		/* offset */
				DI_SYNC_INT_HSYNC,	/* offset resolution */
				0,		/* repeat count */
				DI_SYNC_NONE,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				4		/* COUNT DOWN */
				);

		/* Active Field ? */
		ipu_di_sync_config(
				disp,		/* display */
				4,		/* counter */
				v_total / 2 - 1,/* run count */
				DI_SYNC_HSYNC,	/* run_resolution */
				v_start_width,	/*  offset */
				DI_SYNC_HSYNC,	/* offset resolution */
				2,		/* repeat count */
				DI_SYNC_VSYNC,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				0		/* COUNT DOWN */
				);

		/* Active Line */
		ipu_di_sync_config(
				disp,		/* display */
				5,		/* counter */
				0,		/* run count */
				DI_SYNC_HSYNC,	/* run_resolution */
				0,		/*  offset */
				DI_SYNC_NONE,	/* offset resolution */
				height / 2,	/* repeat count */
				4,		/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				0		/* COUNT DOWN */
				);

		/* Field 0 VSYNC waveform */
		ipu_di_sync_config(
				disp,		/* display */
				6,		/* counter */
				v_total - 1,	/* run count */
				DI_SYNC_HSYNC,	/* run_resolution */
				0,		/* offset */
				DI_SYNC_NONE,	/* offset resolution */
				0,		/* repeat count */
				DI_SYNC_NONE,	/* CNT_CLR_SEL  */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				0		/* COUNT DOWN */
				);

		/* DC VSYNC waveform */
		vsync_cnt = 7;
		ipu_di_sync_config(
				disp,		/* display */
				7,		/* counter */
				v_total / 2 - 1,/* run count */
				DI_SYNC_HSYNC,	/* run_resolution  */
				9,		/* offset  */
				DI_SYNC_HSYNC,	/* offset resolution */
				2,		/* repeat count */
				DI_SYNC_VSYNC,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				0		/* COUNT DOWN */
				);

		/* active pixel waveform */
		ipu_di_sync_config(
				disp,		/* display */
				8,		/* counter */
				0,		/* run count  */
				DI_SYNC_CLK,	/* run_resolution */
				h_start_width,	/* offset  */
				DI_SYNC_CLK,	/* offset resolution */
				width,		/* repeat count  */
				5,		/* CNT_CLR_SEL  */
				0,		/* CNT_POLARITY_GEN_EN  */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL  */
				0,		/* COUNT UP  */
				0		/* COUNT DOWN */
				);

		ipu_di_sync_config(
				disp,		/* display */
				9,		/* counter */
				v_total - 1,	/* run count */
				DI_SYNC_INT_HSYNC,/* run_resolution */
				v_total / 2,	/* offset  */
				DI_SYNC_INT_HSYNC,/* offset resolution  */
				0,		/* repeat count */
				DI_SYNC_HSYNC,	/* CNT_CLR_SEL */
				0,		/* CNT_POLARITY_GEN_EN  */
				DI_SYNC_NONE,	/* CNT_POLARITY_CLR_SEL  */
				DI_SYNC_NONE,	/* CNT_POLARITY_TRIGGER_SEL */
				0,		/* COUNT UP */
				4		/* COUNT DOWN */
				);

		/* set gentime select and tag sel */
		reg = __raw_readl(DI_SW_GEN1(disp, 9));
		reg &= 0x1FFFFFFF;
		reg |= (3 - 1)<<29 | 0x00008000;
		__raw_writel(reg, DI_SW_GEN1(disp, 9));

		__raw_writel(v_total / 2 - 1, DI_SCR_CONF(disp));

		/* set y_sel = 1 */
		di_gen |= 0x10000000;
		di_gen |= DI_GEN_POLARITY_5;
		di_gen |= DI_GEN_POLARITY_8;
	} else {
		/* Setup internal HSYNC waveform */
		ipu_di_sync_config(disp, 1, h_total - 1, DI_SYNC_CLK,
				0, DI_SYNC_NONE, 0, DI_SYNC_NONE,
				0, DI_SYNC_NONE,
				DI_SYNC_NONE, 0, 0);

		/* Setup external (delayed) HSYNC waveform */
		ipu_di_sync_config(disp, DI_SYNC_HSYNC, h_total - 1,
				DI_SYNC_CLK, div * v_to_h_sync, DI_SYNC_CLK,
				0, DI_SYNC_NONE, 1, DI_SYNC_NONE,
				DI_SYNC_CLK, 0, h_sync_width * 2);
		/* Setup VSYNC waveform */
		vsync_cnt = DI_SYNC_VSYNC;
		ipu_di_sync_config(disp, DI_SYNC_VSYNC, v_total - 1,
				DI_SYNC_INT_HSYNC, 0, DI_SYNC_NONE, 0,
				DI_SYNC_NONE, 1, DI_SYNC_NONE,
				DI_SYNC_INT_HSYNC, 0, v_sync_width * 2);
		__raw_writel(v_total - 1, DI_SCR_CONF(disp));

		/* Setup active data waveform to sync with DC */
		ipu_di_sync_config(disp, 4, 0, DI_SYNC_HSYNC,
				v_sync_width + v_start_width, DI_SYNC_HSYNC,
				height,
				DI_SYNC_VSYNC, 0, DI_SYNC_NONE,
				DI_SYNC_NONE, 0, 0);
		ipu_di_sync_config(disp, 5, 0, DI_SYNC_CLK,
				h_sync_width + h_start_width, DI_SYNC_CLK,
				width, 4, 0, DI_SYNC_NONE, DI_SYNC_NONE, 0,
				0);

		/* reset all unused counters */
		__raw_writel(0, DI_SW_GEN0(disp, 6));
		__raw_writel(0, DI_SW_GEN1(disp, 6));
		__raw_writel(0, DI_SW_GEN0(disp, 7));
		__raw_writel(0, DI_SW_GEN1(disp, 7));
		__raw_writel(0, DI_SW_GEN0(disp, 8));
		__raw_writel(0, DI_SW_GEN1(disp, 8));
		__raw_writel(0, DI_SW_GEN0(disp, 9));
		__raw_writel(0, DI_SW_GEN1(disp, 9));

		reg = __raw_readl(DI_STP_REP(disp, 6));
		reg &= 0x0000FFFF;
		__raw_writel(reg, DI_STP_REP(disp, 6));
		__raw_writel(0, DI_STP_REP(disp, 7));
		__raw_writel(0, DI_STP_REP9(disp));

		/* Init template microcode */
		if (disp) {
		   ipu_dc_write_tmpl(2, WROD(0), 0, map, SYNC_WAVE, 8, 5);
		   ipu_dc_write_tmpl(3, WROD(0), 0, map, SYNC_WAVE, 4, 5);
		   ipu_dc_write_tmpl(4, WROD(0), 0, map, SYNC_WAVE, 0, 5);
		} else {
		   ipu_dc_write_tmpl(5, WROD(0), 0, map, SYNC_WAVE, 8, 5);
		   ipu_dc_write_tmpl(6, WROD(0), 0, map, SYNC_WAVE, 4, 5);
		   ipu_dc_write_tmpl(7, WROD(0), 0, map, SYNC_WAVE, 0, 5);
		}

		if (sig.Hsync_pol)
			di_gen |= DI_GEN_POLARITY_2;
		if (sig.Vsync_pol)
			di_gen |= DI_GEN_POLARITY_3;

		if (!sig.clk_pol)
			di_gen |= DI_GEN_POL_CLK;

	}

	__raw_writel(di_gen, DI_GENERAL(disp));

	__raw_writel((--vsync_cnt << DI_VSYNC_SEL_OFFSET) |
			0x00000002, DI_SYNC_AS_GEN(disp));

	reg = __raw_readl(DI_POL(disp));
	reg &= ~(DI_POL_DRDY_DATA_POLARITY | DI_POL_DRDY_POLARITY_15);
	if (sig.enable_pol)
		reg |= DI_POL_DRDY_POLARITY_15;
	if (sig.data_pol)
		reg |= DI_POL_DRDY_DATA_POLARITY;
	__raw_writel(reg, DI_POL(disp));

	__raw_writel(width, DC_DISP_CONF2(DC_DISP_ID_SYNC(disp)));

	return 0;
}

/*
 * This function sets the foreground and background plane global alpha blending
 * modes. This function also sets the DP graphic plane according to the
 * parameter of IPUv3 DP channel.
 *
 * @param	channel		IPUv3 DP channel
 *
 * @param       enable          Boolean to enable or disable global alpha
 *                              blending. If disabled, local blending is used.
 *
 * @param       alpha           Global alpha value.
 *
 * Return:      Returns 0 on success or negative error code on fail
 */
int32_t ipu_disp_set_global_alpha(ipu_channel_t channel, unsigned char enable,
				  uint8_t alpha)
{
	uint32_t reg;

	unsigned char bg_chan;

	if (!((channel == MEM_BG_SYNC || channel == MEM_FG_SYNC) ||
		(channel == MEM_BG_ASYNC0 || channel == MEM_FG_ASYNC0) ||
		(channel == MEM_BG_ASYNC1 || channel == MEM_FG_ASYNC1)))
		return -EINVAL;

	if (channel == MEM_BG_SYNC || channel == MEM_BG_ASYNC0 ||
	    channel == MEM_BG_ASYNC1)
		bg_chan = 1;
	else
		bg_chan = 0;

	if (bg_chan) {
		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg & ~DP_COM_CONF_GWSEL, DP_COM_CONF());
	} else {
		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg | DP_COM_CONF_GWSEL, DP_COM_CONF());
	}

	if (enable) {
		reg = __raw_readl(DP_GRAPH_WIND_CTRL()) & 0x00FFFFFFL;
		__raw_writel(reg | ((uint32_t) alpha << 24),
			     DP_GRAPH_WIND_CTRL());

		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg | DP_COM_CONF_GWAM, DP_COM_CONF());
	} else {
		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg & ~DP_COM_CONF_GWAM, DP_COM_CONF());
	}

	reg = __raw_readl(IPU_SRM_PRI2) | 0x8;
	__raw_writel(reg, IPU_SRM_PRI2);

	return 0;
}

/*
 * This function sets the transparent color key for SDC graphic plane.
 *
 * @param       channel         Input parameter for the logical channel ID.
 *
 * @param       enable          Boolean to enable or disable color key
 *
 * @param       colorKey        24-bit RGB color for transparent color key.
 *
 * Return:      Returns 0 on success or negative error code on fail
 */
int32_t ipu_disp_set_color_key(ipu_channel_t channel, unsigned char enable,
			       uint32_t color_key)
{
	uint32_t reg;
	int y, u, v;
	int red, green, blue;

	if (!((channel == MEM_BG_SYNC || channel == MEM_FG_SYNC) ||
		(channel == MEM_BG_ASYNC0 || channel == MEM_FG_ASYNC0) ||
		(channel == MEM_BG_ASYNC1 || channel == MEM_FG_ASYNC1)))
		return -EINVAL;

	color_key_4rgb = 1;
	/* Transform color key from rgb to yuv if CSC is enabled */
	if (((fg_csc_type == RGB2YUV) && (bg_csc_type == YUV2YUV)) ||
		((fg_csc_type == YUV2YUV) && (bg_csc_type == RGB2YUV)) ||
		((fg_csc_type == YUV2YUV) && (bg_csc_type == YUV2YUV)) ||
		((fg_csc_type == YUV2RGB) && (bg_csc_type == YUV2RGB))) {

		debug("color key 0x%x need change to yuv fmt\n", color_key);

		red = (color_key >> 16) & 0xFF;
		green = (color_key >> 8) & 0xFF;
		blue = color_key & 0xFF;

		y = rgb_to_yuv(0, red, green, blue);
		u = rgb_to_yuv(1, red, green, blue);
		v = rgb_to_yuv(2, red, green, blue);
		color_key = (y << 16) | (u << 8) | v;

		color_key_4rgb = 0;

		debug("color key change to yuv fmt 0x%x\n", color_key);
	}

	if (enable) {
		reg = __raw_readl(DP_GRAPH_WIND_CTRL()) & 0xFF000000L;
		__raw_writel(reg | color_key, DP_GRAPH_WIND_CTRL());

		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg | DP_COM_CONF_GWCKE, DP_COM_CONF());
	} else {
		reg = __raw_readl(DP_COM_CONF());
		__raw_writel(reg & ~DP_COM_CONF_GWCKE, DP_COM_CONF());
	}

	reg = __raw_readl(IPU_SRM_PRI2) | 0x8;
	__raw_writel(reg, IPU_SRM_PRI2);

	return 0;
}
