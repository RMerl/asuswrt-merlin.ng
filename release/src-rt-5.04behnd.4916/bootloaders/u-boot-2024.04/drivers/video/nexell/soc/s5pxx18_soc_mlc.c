// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_mlc.h"

static struct {
	struct nx_mlc_register_set *pregister;
} __g_module_variables[NUMBER_OF_MLC_MODULE] = { { NULL, },};

int nx_mlc_initialize(void)
{
	static int binit;
	u32 i;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_MLC_MODULE; i++)
			__g_module_variables[i].pregister = NULL;
		binit = 1;
	}
	return 1;
}

u32 nx_mlc_get_physical_address(u32 module_index)
{
	const u32 physical_addr[] = PHY_BASEADDR_MLC_LIST;

	return physical_addr[module_index];
}

void nx_mlc_set_base_address(u32 module_index, void *base_address)
{
	__g_module_variables[module_index].pregister =
	    (struct nx_mlc_register_set *)base_address;
}

void *nx_mlc_get_base_address(u32 module_index)
{
	return (void *)__g_module_variables[module_index].pregister;
}

void nx_mlc_set_clock_pclk_mode(u32 module_index, enum nx_pclkmode mode)
{
	const u32 pclkmode_pos = 3;
	u32 clkmode = 0;

	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	switch (mode) {
	case nx_pclkmode_dynamic:
		clkmode = 0;
		break;
	case nx_pclkmode_always:
		clkmode = 1;
		break;
	default:
		break;
	}
	regvalue = pregister->mlcclkenb;
	regvalue &= ~(1ul << pclkmode_pos);
	regvalue |= (clkmode & 0x01) << pclkmode_pos;

	writel(regvalue, &pregister->mlcclkenb);
}

enum nx_pclkmode nx_mlc_get_clock_pclk_mode(u32 module_index)
{
	const u32 pclkmode_pos = 3;

	if (__g_module_variables[module_index].pregister->mlcclkenb &
	    (1ul << pclkmode_pos)) {
		return nx_pclkmode_always;
	}
	return nx_pclkmode_dynamic;
}

void nx_mlc_set_clock_bclk_mode(u32 module_index, enum nx_bclkmode mode)
{
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;
	u32 clkmode = 0;

	pregister = __g_module_variables[module_index].pregister;
	switch (mode) {
	case nx_bclkmode_disable:
		clkmode = 0;
		break;
	case nx_bclkmode_dynamic:
		clkmode = 2;
		break;
	case nx_bclkmode_always:
		clkmode = 3;
		break;
	default:
		break;
	}
	regvalue = pregister->mlcclkenb;
	regvalue &= ~(0x3);
	regvalue |= clkmode & 0x3;

	writel(regvalue, &pregister->mlcclkenb);
}

enum nx_bclkmode nx_mlc_get_clock_bclk_mode(u32 module_index)
{
	const u32 bclkmode = 3ul << 0;

	switch (__g_module_variables[module_index].pregister->mlcclkenb &
		bclkmode) {
	case 0:
		return nx_bclkmode_disable;
	case 2:
		return nx_bclkmode_dynamic;
	case 3:
		return nx_bclkmode_always;
	}
	return nx_bclkmode_disable;
}

void nx_mlc_set_top_power_mode(u32 module_index, int bpower)
{
	const u32 pixelbuffer_pwd_pos = 11;
	const u32 pixelbuffer_pwd_mask = 1ul << pixelbuffer_pwd_pos;
	const u32 dittyflag_mask = 1ul << 3;
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue &= ~(pixelbuffer_pwd_mask | dittyflag_mask);
	regvalue |= (bpower << pixelbuffer_pwd_pos);

	writel(regvalue, &pregister->mlccontrolt);
}

int nx_mlc_get_top_power_mode(u32 module_index)
{
	const u32 pixelbuffer_pwd_pos = 11;
	const u32 pixelbuffer_pwd_mask = 1ul << pixelbuffer_pwd_pos;

	return (int)((__g_module_variables[module_index].pregister->mlccontrolt
		     & pixelbuffer_pwd_mask) >>
		     pixelbuffer_pwd_pos);
}

void nx_mlc_set_top_sleep_mode(u32 module_index, int bsleep)
{
	const u32 pixelbuffer_sld_pos = 10;
	const u32 pixelbuffer_sld_mask = 1ul << pixelbuffer_sld_pos;
	const u32 dittyflag_mask = 1ul << 3;
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	bsleep = (int)((u32)bsleep ^ 1);
	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue &= ~(pixelbuffer_sld_mask | dittyflag_mask);
	regvalue |= (bsleep << pixelbuffer_sld_pos);

	writel(regvalue, &pregister->mlccontrolt);
}

int nx_mlc_get_top_sleep_mode(u32 module_index)
{
	const u32 pixelbuffer_sld_pos = 11;
	const u32 pixelbuffer_sld_mask = 1ul << pixelbuffer_sld_pos;

	return (int)(((__g_module_variables[module_index].pregister->mlccontrolt
		     & pixelbuffer_sld_mask) >>
		     pixelbuffer_sld_pos) ^ 0x01);
}

void nx_mlc_set_top_dirty_flag(u32 module_index)
{
	const u32 dirtyflag = 1ul << 3;
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue |= dirtyflag;

	writel(regvalue, &pregister->mlccontrolt);
}

int nx_mlc_get_top_dirty_flag(u32 module_index)
{
	const u32 dirtyflag_pos = 3;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;

	return (int)((readl(&__g_module_variables[module_index]
			    .pregister->mlccontrolt) &
		      dirtyflag_mask) >> dirtyflag_pos);
}

void nx_mlc_set_mlc_enable(u32 module_index, int benb)
{
	const u32 mlcenb_pos = 1;
	const u32 mlcenb_mask = 1ul << mlcenb_pos;
	const u32 dirtyflag_pos = 3;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue &= ~(mlcenb_mask | dirtyflag_mask);
	regvalue |= (benb << mlcenb_pos);

	writel(regvalue, &pregister->mlccontrolt);
}

int nx_mlc_get_mlc_enable(u32 module_index)
{
	const u32 mlcenb_pos = 1;
	const u32 mlcenb_mask = 1ul << mlcenb_pos;

	return (int)((__g_module_variables[module_index].pregister->mlccontrolt
		     & mlcenb_mask) >> mlcenb_pos);
}

void nx_mlc_set_field_enable(u32 module_index, int benb)
{
	const u32 fieldenb_pos = 0;
	const u32 fieldenb_mask = 1ul << fieldenb_pos;
	const u32 dirtyflag_pos = 3;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue &= ~(fieldenb_mask | dirtyflag_mask);
	regvalue |= (benb << fieldenb_pos);

	writel(regvalue, &pregister->mlccontrolt);
}

int nx_mlc_get_field_enable(u32 module_index)
{
	const u32 fieldenb_pos = 0;
	const u32 fieldenb_mask = 1ul << fieldenb_pos;

	return (int)(__g_module_variables[module_index].pregister->mlccontrolt &
		     fieldenb_mask);
}

void nx_mlc_set_layer_priority(u32 module_index, enum nx_mlc_priority priority)
{
	const u32 priority_pos = 8;
	const u32 priority_mask = 0x03 << priority_pos;
	const u32 dirtyflag_pos = 3;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlccontrolt;
	regvalue &= ~(priority_mask | dirtyflag_mask);
	regvalue |= (priority << priority_pos);

	writel(regvalue, &pregister->mlccontrolt);
}

void nx_mlc_set_screen_size(u32 module_index, u32 width, u32 height)
{
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = ((height - 1) << 16) | (width - 1);

	writel(regvalue, &pregister->mlcscreensize);
}

void nx_mlc_get_screen_size(u32 module_index, u32 *pwidth, u32 *pheight)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	if (pwidth)
		*pwidth = (pregister->mlcscreensize & 0x0fff) + 1;

	if (pheight)
		*pheight = ((pregister->mlcscreensize >> 16) & 0x0fff) + 1;
}

void nx_mlc_set_background(u32 module_index, u32 color)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(color, &pregister->mlcbgcolor);
}

void nx_mlc_set_dirty_flag(u32 module_index, u32 layer)
{
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;
	const u32 dirtyflg_mask = 1ul << 4;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue |= dirtyflg_mask;

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
	} else if (layer == 3) {
		regvalue = pregister->mlcvideolayer.mlccontrol;
		regvalue |= dirtyflg_mask;

		writel(regvalue, &pregister->mlcvideolayer.mlccontrol);
	}
}

int nx_mlc_get_dirty_flag(u32 module_index, u32 layer)
{
	const u32 dirtyflg_pos = 4;
	const u32 dirtyflg_mask = 1ul << dirtyflg_pos;

	if (layer == 0 || layer == 1) {
		return (int)((__g_module_variables[module_index]
			      .pregister->mlcrgblayer[layer]
			      .mlccontrol & dirtyflg_mask) >> dirtyflg_pos);
	} else if (layer == 2) {
		return (int)((__g_module_variables[module_index]
			      .pregister->mlcrgblayer2.mlccontrol &
			      dirtyflg_mask) >> dirtyflg_pos);
	} else if (layer == 3) {
		return (int)((__g_module_variables[module_index]
			      .pregister->mlcvideolayer.mlccontrol &
			      dirtyflg_mask) >> dirtyflg_pos);
	}
	return 0;
}

void nx_mlc_set_layer_enable(u32 module_index, u32 layer, int benb)
{
	const u32 layerenb_pos = 5;
	const u32 layerenb_mask = 0x01 << layerenb_pos;
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(layerenb_mask | dirtyflag_mask);
		regvalue |= (benb << layerenb_pos);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
	} else if (layer == 3) {
		regvalue = pregister->mlcvideolayer.mlccontrol;
		regvalue &= ~(layerenb_mask | dirtyflag_mask);
		regvalue |= (benb << layerenb_pos);

		writel(regvalue, &pregister->mlcvideolayer.mlccontrol);
	}
}

int nx_mlc_get_layer_enable(u32 module_index, u32 layer)
{
	const u32 layerenb_pos = 5;
	const u32 layerenb_mask = 0x01 << layerenb_pos;

	if (layer == 0 || layer == 1) {
		return (int)((__g_module_variables[module_index]
			      .pregister->mlcrgblayer[layer]
			      .mlccontrol & layerenb_mask) >> layerenb_pos);
	} else if (layer == 3) {
		return (int)((__g_module_variables[module_index]
			      .pregister->mlcvideolayer.mlccontrol &
			      layerenb_mask) >> layerenb_pos);
	}
	return 0;
}

void nx_mlc_set_lock_size(u32 module_index, u32 layer, u32 locksize)
{
	const u32 locksize_mask = 3ul << 12;
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	register struct nx_mlc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	locksize >>= 3;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(locksize_mask | dirtyflag_mask);
		regvalue |= (locksize << 12);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
	}
}

void nx_mlc_set_alpha_blending(u32 module_index, u32 layer, int benb, u32 alpha)
{
	const u32 blendenb_pos = 2;
	const u32 blendenb_mask = 0x01 << blendenb_pos;
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	const u32 alpha_pos = 28;
	const u32 alpha_mask = 0xf << alpha_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(blendenb_mask | dirtyflag_mask);
		regvalue |= (benb << blendenb_pos);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
		regvalue = pregister->mlcrgblayer[layer].mlctpcolor;
		regvalue &= ~alpha_mask;
		regvalue |= alpha << alpha_pos;

		writel(regvalue, &pregister->mlcrgblayer[layer].mlctpcolor);
	} else if (layer == 3) {
		regvalue = pregister->mlcvideolayer.mlccontrol;
		regvalue &= ~(blendenb_mask | dirtyflag_mask);
		regvalue |= (benb << blendenb_pos);

		writel(regvalue, &pregister->mlcvideolayer.mlccontrol);

		writel(alpha << alpha_pos,
		       &pregister->mlcvideolayer.mlctpcolor);
	}
}

void nx_mlc_set_transparency(u32 module_index, u32 layer, int benb, u32 color)
{
	const u32 tpenb_pos = 0;
	const u32 tpenb_mask = 0x01 << tpenb_pos;
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	const u32 tpcolor_pos = 0;
	const u32 tpcolor_mask = ((1 << 24) - 1) << tpcolor_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(tpenb_mask | dirtyflag_mask);
		regvalue |= (benb << tpenb_pos);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
		regvalue = pregister->mlcrgblayer[layer].mlctpcolor;
		regvalue &= ~tpcolor_mask;
		regvalue |= (color & tpcolor_mask);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlctpcolor);
	}
}

void nx_mlc_set_color_inversion(u32 module_index, u32 layer, int benb,
				u32 color)
{
	const u32 invenb_pos = 1;
	const u32 invenb_mask = 0x01 << invenb_pos;
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	const u32 invcolor_pos = 0;
	const u32 invcolor_mask = ((1 << 24) - 1) << invcolor_pos;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(invenb_mask | dirtyflag_mask);
		regvalue |= (benb << invenb_pos);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
		regvalue = pregister->mlcrgblayer[layer].mlcinvcolor;
		regvalue &= ~invcolor_mask;
		regvalue |= (color & invcolor_mask);

		writel(regvalue, &pregister->mlcrgblayer[layer].mlcinvcolor);
	}
}

u32 nx_mlc_get_extended_color(u32 module_index, u32 color,
			      enum nx_mlc_rgbfmt format)
{
	u32 rgb[3] = {
		0,
	};
	u32 bw[3] = {
		0,
	};
	u32 bp[3] = {
		0,
	};
	u32 blank = 0;
	u32 fill = 0;
	u32 i = 0;

	switch (format) {
	case nx_mlc_rgbfmt_r5g6b5:
		bw[0] = 5;
		bw[1] = 6;
		bw[2] = 5;
		bp[0] = 11;
		bp[1] = 5;
		bp[2] = 0;
		break;
	case nx_mlc_rgbfmt_b5g6r5:
		bw[0] = 5;
		bw[1] = 6;
		bw[2] = 5;
		bp[0] = 0;
		bp[1] = 5;
		bp[2] = 11;
		break;
	case nx_mlc_rgbfmt_x1r5g5b5:
	case nx_mlc_rgbfmt_a1r5g5b5:
		bw[0] = 5;
		bw[1] = 5;
		bw[2] = 5;
		bp[0] = 10;
		bp[1] = 5;
		bp[2] = 0;
		break;
	case nx_mlc_rgbfmt_x1b5g5r5:
	case nx_mlc_rgbfmt_a1b5g5r5:
		bw[0] = 5;
		bw[1] = 5;
		bw[2] = 5;
		bp[0] = 0;
		bp[1] = 5;
		bp[2] = 10;
		break;
	case nx_mlc_rgbfmt_x4r4g4b4:
	case nx_mlc_rgbfmt_a4r4g4b4:
		bw[0] = 4;
		bw[1] = 4;
		bw[2] = 4;
		bp[0] = 8;
		bp[1] = 4;
		bp[2] = 0;
		break;
	case nx_mlc_rgbfmt_x4b4g4r4:
	case nx_mlc_rgbfmt_a4b4g4r4:
		bw[0] = 4;
		bw[1] = 4;
		bw[2] = 4;
		bp[0] = 0;
		bp[1] = 4;
		bp[2] = 8;
		break;
	case nx_mlc_rgbfmt_x8r3g3b2:
	case nx_mlc_rgbfmt_a8r3g3b2:
		bw[0] = 3;
		bw[1] = 3;
		bw[2] = 2;
		bp[0] = 5;
		bp[1] = 2;
		bp[2] = 0;
		break;
	case nx_mlc_rgbfmt_x8b3g3r2:
	case nx_mlc_rgbfmt_a8b3g3r2:
		bw[0] = 2;
		bw[1] = 3;
		bw[2] = 3;
		bp[0] = 0;
		bp[1] = 2;
		bp[2] = 5;
		break;
	case nx_mlc_rgbfmt_r8g8b8:
	case nx_mlc_rgbfmt_a8r8g8b8:
		bw[0] = 8;
		bw[1] = 8;
		bw[2] = 8;
		bp[0] = 16;
		bp[1] = 8;
		bp[2] = 0;
		break;
	case nx_mlc_rgbfmt_b8g8r8:
	case nx_mlc_rgbfmt_a8b8g8r8:
		bw[0] = 8;
		bw[1] = 8;
		bw[2] = 8;
		bp[0] = 0;
		bp[1] = 8;
		bp[2] = 16;
		break;
	default:
		break;
	}
	for (i = 0; i < 3; i++) {
		rgb[i] = (color >> bp[i]) & ((u32)(1 << bw[i]) - 1);
		fill = bw[i];
		blank = 8 - fill;
		rgb[i] <<= blank;
		while (blank > 0) {
			rgb[i] |= (rgb[i] >> fill);
			blank -= fill;
			fill += fill;
		}
	}

	return (rgb[0] << 16) | (rgb[1] << 8) | (rgb[2] << 0);
}

void nx_mlc_set_format_rgb(u32 module_index, u32 layer,
			   enum nx_mlc_rgbfmt format)
{
	const u32 dirtyflag_pos = 4;
	const u32 dirtyflag_mask = 1ul << dirtyflag_pos;
	const u32 format_mask = 0xffff0000ul;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		regvalue = pregister->mlcrgblayer[layer].mlccontrol;
		regvalue &= ~(format_mask | dirtyflag_mask);
		regvalue |= (u32)format;

		writel(regvalue, &pregister->mlcrgblayer[layer].mlccontrol);
	}
}

void nx_mlc_set_format_yuv(u32 module_index, enum nx_mlc_yuvfmt format)
{
	const u32 format_mask = 0xffff0000ul;
	register u32 temp;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	temp = pregister->mlcvideolayer.mlccontrol;
	temp &= ~format_mask;
	temp |= (u32)format;

	writel(temp, &pregister->mlcvideolayer.mlccontrol);
}

void nx_mlc_set_position(u32 module_index, u32 layer, s32 sx, s32 sy,
			 s32 ex, s32 ey)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		writel((((u32)sx & 0xffful) << 16) | ((u32)ex & 0xffful),
		       &pregister->mlcrgblayer[layer].mlcleftright);

		writel((((u32)sy & 0xffful) << 16) | ((u32)ey & 0xffful),
		       &pregister->mlcrgblayer[layer].mlctopbottom);
	} else if (layer == 2) {
		writel((((u32)sx & 0xffful) << 16) | ((u32)ex & 0xffful),
		       &pregister->mlcrgblayer2.mlcleftright);

		writel((((u32)sy & 0xffful) << 16) | ((u32)ey & 0xffful),
		       &pregister->mlcrgblayer2.mlctopbottom);
	} else if (layer == 3) {
		writel((((u32)sx & 0xffful) << 16) | ((u32)ex & 0xffful),
		       &pregister->mlcvideolayer.mlcleftright);

		writel((((u32)sy & 0xffful) << 16) | ((u32)ey & 0xffful),
		       &pregister->mlcvideolayer.mlctopbottom);
	}
}

void nx_mlc_set_dither_enable_when_using_gamma(u32 module_index, int benable)
{
	const u32 ditherenb_bitpos = 0;
	const u32 ditherenb_mask = 1 << ditherenb_bitpos;
	register struct nx_mlc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~ditherenb_mask;
	read_value |= ((u32)benable << ditherenb_bitpos);

	writel(read_value, &pregister->mlcgammacont);
}

int nx_mlc_get_dither_enable_when_using_gamma(u32 module_index)
{
	const u32 ditherenb_bitpos = 0;
	const u32 ditherenb_mask = 1 << ditherenb_bitpos;

	return (int)(__g_module_variables[module_index].pregister->mlcgammacont
		     & ditherenb_mask);
}

void nx_mlc_set_gamma_priority(u32 module_index, int bvideolayer)
{
	const u32 alphaselect_bitpos = 5;
	const u32 alphaselect_mask = 1 << alphaselect_bitpos;
	register struct nx_mlc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~alphaselect_mask;
	read_value |= ((u32)bvideolayer << alphaselect_bitpos);

	writel(read_value, &pregister->mlcgammacont);
}

int nx_mlc_get_gamma_priority(u32 module_index)
{
	const u32 alphaselect_bitpos = 5;
	const u32 alphaselect_mask = 1 << alphaselect_bitpos;

	return (int)((__g_module_variables[module_index].pregister->mlcgammacont
		     & alphaselect_mask) >> alphaselect_bitpos);
}

void nx_mlc_set_rgblayer_invalid_position(u32 module_index, u32 layer,
					  u32 region, s32 sx, s32 sy,
					  s32 ex, s32 ey, int benb)
{
	const u32 invalidenb_pos = 28;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		if (region == 0) {
			writel(((benb << invalidenb_pos) |
				((sx & 0x7ff) << 16) | (ex & 0x7ff)),
			       &pregister->mlcrgblayer[layer]
			       .mlcinvalidleftright0);

			writel((((sy & 0x7ff) << 16) | (ey & 0x7ff)),
			       &pregister->mlcrgblayer[layer]
			       .mlcinvalidtopbottom0);
		} else {
			writel(((benb << invalidenb_pos) |
				((sx & 0x7ff) << 16) | (ex & 0x7ff)),
			       &pregister->mlcrgblayer[layer]
			       .mlcinvalidleftright1);

			writel((((sy & 0x7ff) << 16) | (ey & 0x7ff)),
			       &pregister->mlcrgblayer[layer]
			       .mlcinvalidtopbottom1);
		}
	}
}

void nx_mlc_set_rgblayer_stride(u32 module_index, u32 layer, s32 hstride,
				s32 vstride)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1) {
		writel(hstride, &pregister->mlcrgblayer[layer].mlchstride);
		writel(vstride, &pregister->mlcrgblayer[layer].mlcvstride);
	} else if (layer == 2) {
		writel(hstride, &pregister->mlcrgblayer2.mlchstride);
		writel(vstride, &pregister->mlcrgblayer2.mlcvstride);
	}
}

void nx_mlc_set_rgblayer_address(u32 module_index, u32 layer, u32 addr)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0 || layer == 1)
		writel(addr, &pregister->mlcrgblayer[layer].mlcaddress);
	else if (layer == 2)
		writel(addr, &pregister->mlcrgblayer2.mlcaddress);
}

void nx_mlc_set_rgblayer_gama_table_power_mode(u32 module_index, int bred,
					       int bgreen, int bblue)
{
	const u32 bgammatable_pwd_bitpos = 11;
	const u32 ggammatable_pwd_bitpos = 9;
	const u32 rgammatable_pwd_bitpos = 3;
	const u32 bgammatable_pwd_mask = (1 << bgammatable_pwd_bitpos);
	const u32 ggammatable_pwd_mask = (1 << ggammatable_pwd_bitpos);
	const u32 rgammatable_pwd_mask = (1 << rgammatable_pwd_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~(bgammatable_pwd_mask | ggammatable_pwd_mask |
			rgammatable_pwd_mask);
	read_value |= (((u32)bred << rgammatable_pwd_bitpos) |
		       ((u32)bgreen << ggammatable_pwd_bitpos) |
		       ((u32)bblue << bgammatable_pwd_bitpos));

	writel(read_value, &pregister->mlcgammacont);
}

void nx_mlc_get_rgblayer_gama_table_power_mode(u32 module_index, int *pbred,
					       int *pbgreen, int *pbblue)
{
	const u32 bgammatable_pwd_bitpos = 11;
	const u32 ggammatable_pwd_bitpos = 9;
	const u32 rgammatable_pwd_bitpos = 3;
	const u32 bgammatable_pwd_mask = (1 << bgammatable_pwd_bitpos);
	const u32 ggammatable_pwd_mask = (1 << ggammatable_pwd_bitpos);
	const u32 rgammatable_pwd_mask = (1 << rgammatable_pwd_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	if (pbred)
		*pbred = (read_value & rgammatable_pwd_mask) ? 1 : 0;

	if (pbgreen)
		*pbgreen = (read_value & ggammatable_pwd_mask) ? 1 : 0;

	if (pbblue)
		*pbblue = (read_value & bgammatable_pwd_mask) ? 1 : 0;
}

void nx_mlc_set_rgblayer_gama_table_sleep_mode(u32 module_index, int bred,
					       int bgreen, int bblue)
{
	const u32 bgammatable_sld_bitpos = 10;
	const u32 ggammatable_sld_bitpos = 8;
	const u32 rgammatable_sld_bitpos = 2;
	const u32 bgammatable_sld_mask = (1 << bgammatable_sld_bitpos);
	const u32 ggammatable_sld_mask = (1 << ggammatable_sld_bitpos);
	const u32 rgammatable_sld_mask = (1 << rgammatable_sld_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	if (bred)
		read_value &= ~rgammatable_sld_mask;
	else
		read_value |= rgammatable_sld_mask;

	if (bgreen)
		read_value &= ~ggammatable_sld_mask;
	else
		read_value |= ggammatable_sld_mask;

	if (bblue)
		read_value &= ~bgammatable_sld_mask;
	else
		read_value |= bgammatable_sld_mask;

	writel(read_value, &pregister->mlcgammacont);
}

void nx_mlc_get_rgblayer_gama_table_sleep_mode(u32 module_index, int *pbred,
					       int *pbgreen, int *pbblue)
{
	const u32 bgammatable_sld_bitpos = 10;
	const u32 ggammatable_sld_bitpos = 8;
	const u32 rgammatable_sld_bitpos = 2;
	const u32 bgammatable_sld_mask = (1 << bgammatable_sld_bitpos);
	const u32 ggammatable_sld_mask = (1 << ggammatable_sld_bitpos);
	const u32 rgammatable_sld_mask = (1 << rgammatable_sld_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;

	if (pbred)
		*pbred = (read_value & rgammatable_sld_mask) ? 0 : 1;

	if (pbgreen)
		*pbgreen = (read_value & ggammatable_sld_mask) ? 0 : 1;

	if (pbblue)
		*pbblue = (read_value & bgammatable_sld_mask) ? 0 : 1;
}

void nx_mlc_set_rgblayer_rgamma_table(u32 module_index, u32 dwaddress,
				      u32 dwdata)
{
	register struct nx_mlc_register_set *pregister;
	const u32 tableaddr_bitpos = 24;

	pregister = __g_module_variables[module_index].pregister;
	writel(((dwaddress << tableaddr_bitpos) | dwdata),
	       &pregister->mlcrgammatablewrite);
}

void nx_mlc_set_rgblayer_ggamma_table(u32 module_index, u32 dwaddress,
				      u32 dwdata)
{
	register struct nx_mlc_register_set *pregister;
	const u32 tableaddr_bitpos = 24;

	pregister = __g_module_variables[module_index].pregister;
	writel(((dwaddress << tableaddr_bitpos) | dwdata),
	       &pregister->mlcggammatablewrite);
}

void nx_mlc_set_rgblayer_bgamma_table(u32 module_index, u32 dwaddress,
				      u32 dwdata)
{
	register struct nx_mlc_register_set *pregister;
	const u32 tableaddr_bitpos = 24;

	pregister = __g_module_variables[module_index].pregister;
	writel(((dwaddress << tableaddr_bitpos) | dwdata),
	       &pregister->mlcbgammatablewrite);
}

void nx_mlc_set_rgblayer_gamma_enable(u32 module_index, int benable)
{
	const u32 rgbgammaemb_bitpos = 1;
	const u32 rgbgammaemb_mask = 1 << rgbgammaemb_bitpos;
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~rgbgammaemb_mask;
	read_value |= (u32)benable << rgbgammaemb_bitpos;

	writel(read_value, &pregister->mlcgammacont);
}

int nx_mlc_get_rgblayer_gamma_enable(u32 module_index)
{
	const u32 rgbgammaemb_bitpos = 1;
	const u32 rgbgammaemb_mask = 1 << rgbgammaemb_bitpos;

	return (int)((__g_module_variables[module_index].pregister->mlcgammacont
		     & rgbgammaemb_mask) >> rgbgammaemb_bitpos);
}

void nx_mlc_set_video_layer_stride(u32 module_index, s32 lu_stride,
				   s32 cb_stride, s32 cr_stride)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel(lu_stride, &pregister->mlcvideolayer.mlcvstride);
	writel(cb_stride, &pregister->mlcvideolayer.mlcvstridecb);
	writel(cr_stride, &pregister->mlcvideolayer.mlcvstridecr);
}

void nx_mlc_set_video_layer_address(u32 module_index, u32 lu_addr, u32 cb_addr,
				    u32 cr_addr)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(lu_addr, &pregister->mlcvideolayer.mlcaddress);
	writel(cb_addr, &pregister->mlcvideolayer.mlcaddresscb);
	writel(cr_addr, &pregister->mlcvideolayer.mlcaddresscr);
}

void nx_mlc_set_video_layer_address_yuyv(u32 module_index, u32 addr,
					 s32 stride)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(addr, &pregister->mlcvideolayer.mlcaddress);
	writel(stride, &pregister->mlcvideolayer.mlcvstride);
}

void nx_mlc_set_video_layer_scale_factor(u32 module_index, u32 hscale,
					 u32 vscale, int bhlumaenb,
					 int bhchromaenb, int bvlumaenb,
					 int bvchromaenb)
{
	const u32 filter_luma_pos = 28;
	const u32 filter_choma_pos = 29;
	const u32 scale_mask = ((1 << 23) - 1);
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel(((bhlumaenb << filter_luma_pos) |
		(bhchromaenb << filter_choma_pos) | (hscale & scale_mask)),
	       &pregister->mlcvideolayer.mlchscale);

	writel(((bvlumaenb << filter_luma_pos) |
		(bvchromaenb << filter_choma_pos) | (vscale & scale_mask)),
	       &pregister->mlcvideolayer.mlcvscale);
}

void nx_mlc_set_video_layer_scale_filter(u32 module_index, int bhlumaenb,
					 int bhchromaenb, int bvlumaenb,
					 int bvchromaenb)
{
	const u32 filter_luma_pos = 28;
	const u32 filter_choma_pos = 29;
	const u32 scale_mask = ((1 << 23) - 1);
	register struct nx_mlc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcvideolayer.mlchscale;
	read_value &= scale_mask;
	read_value |=
	    (bhlumaenb << filter_luma_pos) | (bhchromaenb << filter_choma_pos);

	writel(read_value, &pregister->mlcvideolayer.mlchscale);
	read_value = pregister->mlcvideolayer.mlcvscale;
	read_value &= scale_mask;
	read_value |=
	    (bvlumaenb << filter_luma_pos) | (bvchromaenb << filter_choma_pos);

	writel(read_value, &pregister->mlcvideolayer.mlcvscale);
}

void nx_mlc_get_video_layer_scale_filter(u32 module_index, int *bhlumaenb,
					 int *bhchromaenb, int *bvlumaenb,
					 int *bvchromaenb)
{
	const u32 filter_luma_pos = 28;
	const u32 filter_choma_pos = 29;
	const u32 filter_mask = 1ul;
	register struct nx_mlc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcvideolayer.mlchscale;
	*bhlumaenb = (read_value >> filter_luma_pos) & filter_mask;
	*bhchromaenb = (read_value >> filter_choma_pos) & filter_mask;
	read_value = pregister->mlcvideolayer.mlcvscale;
	*bvlumaenb = (read_value >> filter_luma_pos) & filter_mask;
	*bvchromaenb = (read_value >> filter_choma_pos) & filter_mask;
}

void nx_mlc_set_video_layer_scale(u32 module_index, u32 sw, u32 sh, u32 dw,
				  u32 dh, int bhlumaenb, int bhchromaenb,
				  int bvlumaenb, int bvchromaenb)
{
	const u32 filter_luma_pos = 28;
	const u32 filter_choma_pos = 29;
	const u32 scale_mask = ((1 << 23) - 1);
	register u32 hscale, vscale, cal_sh;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	if ((bhlumaenb || bhchromaenb) && dw > sw) {
		sw--;
		dw--;
	}
	hscale = (sw << 11) / dw;

	if ((bvlumaenb || bvchromaenb) && dh > sh) {
		sh--;
		dh--;
		vscale = (sh << 11) / dh;

		cal_sh = ((vscale * dh) >> 11);
		if (sh <= cal_sh)
			vscale--;

	} else {
		vscale = (sh << 11) / dh;
	}

	writel(((bhlumaenb << filter_luma_pos) |
		(bhchromaenb << filter_choma_pos) | (hscale & scale_mask)),
	       &pregister->mlcvideolayer.mlchscale);

	writel(((bvlumaenb << filter_luma_pos) |
		(bvchromaenb << filter_choma_pos) | (vscale & scale_mask)),
	       &pregister->mlcvideolayer.mlcvscale);
}

void nx_mlc_set_video_layer_luma_enhance(u32 module_index, u32 contrast,
					 s32 brightness)
{
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel((((u32)brightness & 0xfful) << 8) | contrast,
	       &pregister->mlcvideolayer.mlcluenh);
}

void nx_mlc_set_video_layer_chroma_enhance(u32 module_index, u32 quadrant,
					   s32 cb_a, s32 cb_b,
					   s32 cr_a, s32 cr_b)
{
	register struct nx_mlc_register_set *pregister;
	register u32 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = (((u32)cr_b & 0xfful) << 24) | (((u32)cr_a & 0xfful) << 16) |
	    (((u32)cb_b & 0xfful) << 8) | (((u32)cb_a & 0xfful) << 0);
	if (quadrant > 0) {
		writel(temp, &pregister->mlcvideolayer.mlcchenh[quadrant - 1]);
	} else {
		writel(temp, &pregister->mlcvideolayer.mlcchenh[0]);
		writel(temp, &pregister->mlcvideolayer.mlcchenh[1]);
		writel(temp, &pregister->mlcvideolayer.mlcchenh[2]);
		writel(temp, &pregister->mlcvideolayer.mlcchenh[3]);
	}
}

void nx_mlc_set_video_layer_line_buffer_power_mode(u32 module_index,
						   int benable)
{
	const u32 linebuff_pwd_pos = 15;
	const u32 linebuff_pwd_mask = 1ul << linebuff_pwd_pos;
	const u32 dirtyflag_mask = 1ul << 4;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlcvideolayer.mlccontrol;
	regvalue &= ~(linebuff_pwd_mask | dirtyflag_mask);
	regvalue |= ((u32)benable << linebuff_pwd_pos);

	writel(regvalue, &pregister->mlcvideolayer.mlccontrol);
}

int nx_mlc_get_video_layer_line_buffer_power_mode(u32 module_index)
{
	const u32 linebuff_pwd_pos = 15;
	const u32 linebuff_pwd_mask = 1ul << linebuff_pwd_pos;

	return (int)((__g_module_variables[module_index]
		      .pregister->mlcvideolayer.mlccontrol &
		      linebuff_pwd_mask) >> linebuff_pwd_pos);
}

void nx_mlc_set_video_layer_line_buffer_sleep_mode(u32 module_index,
						   int benable)
{
	const u32 linebuff_slmd_pos = 14;
	const u32 linebuff_slmd_mask = 1ul << linebuff_slmd_pos;
	const u32 dirtyflag_mask = 1ul << 4;
	register u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	benable = (int)((u32)benable ^ 1);
	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->mlcvideolayer.mlccontrol;
	regvalue &= ~(linebuff_slmd_mask | dirtyflag_mask);
	regvalue |= (benable << linebuff_slmd_pos);

	writel(regvalue, &pregister->mlcvideolayer.mlccontrol);
}

int nx_mlc_get_video_layer_line_buffer_sleep_mode(u32 module_index)
{
	const u32 linebuff_slmd_pos = 14;
	const u32 linebuff_slmd_mask = 1ul << linebuff_slmd_pos;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (linebuff_slmd_mask & pregister->mlcvideolayer.mlccontrol)
		return 0;
	else
		return 1;
}

void nx_mlc_set_video_layer_gama_table_power_mode(u32 module_index, int by,
						  int bu, int bv)
{
	const u32 vgammatable_pwd_bitpos = 17;
	const u32 ugammatable_pwd_bitpos = 15;
	const u32 ygammatable_pwd_bitpos = 13;
	const u32 vgammatable_pwd_mask = (1 << vgammatable_pwd_bitpos);
	const u32 ugammatable_pwd_mask = (1 << ugammatable_pwd_bitpos);
	const u32 ygammatable_pwd_mask = (1 << ygammatable_pwd_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~(ygammatable_pwd_mask | ugammatable_pwd_mask |
			vgammatable_pwd_mask);
	read_value |= (((u32)by << ygammatable_pwd_bitpos) |
		       ((u32)bu << ugammatable_pwd_bitpos) |
		       ((u32)bv << vgammatable_pwd_bitpos));

	writel(read_value, &pregister->mlcgammacont);
}

void nx_mlc_get_video_layer_gama_table_power_mode(u32 module_index, int *pby,
						  int *pbu, int *pbv)
{
	const u32 vgammatable_pwd_bitpos = 17;
	const u32 ugammatable_pwd_bitpos = 15;
	const u32 ygammatable_pwd_bitpos = 13;
	const u32 vgammatable_pwd_mask = (1 << vgammatable_pwd_bitpos);
	const u32 ugammatable_pwd_mask = (1 << ugammatable_pwd_bitpos);
	const u32 ygammatable_pwd_mask = (1 << ygammatable_pwd_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	if (pby)
		*pby = (read_value & ygammatable_pwd_mask) ? 1 : 0;

	if (pbu)
		*pbu = (read_value & ugammatable_pwd_mask) ? 1 : 0;

	if (pbv)
		*pbv = (read_value & vgammatable_pwd_mask) ? 1 : 0;
}

void nx_mlc_set_video_layer_gama_table_sleep_mode(u32 module_index, int by,
						  int bu, int bv)
{
	const u32 vgammatable_sld_bitpos = 16;
	const u32 ugammatable_sld_bitpos = 14;
	const u32 ygammatable_sld_bitpos = 12;
	const u32 vgammatable_sld_mask = (1 << vgammatable_sld_bitpos);
	const u32 ugammatable_sld_mask = (1 << ugammatable_sld_bitpos);
	const u32 ygammatable_sld_mask = (1 << ygammatable_sld_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	if (by)
		read_value &= ~ygammatable_sld_mask;
	else
		read_value |= ygammatable_sld_mask;

	if (bu)
		read_value &= ~ugammatable_sld_mask;
	else
		read_value |= ugammatable_sld_mask;

	if (bv)
		read_value &= ~vgammatable_sld_mask;
	else
		read_value |= vgammatable_sld_mask;

	writel(read_value, &pregister->mlcgammacont);
}

void nx_mlc_get_video_layer_gama_table_sleep_mode(u32 module_index, int *pby,
						  int *pbu, int *pbv)
{
	const u32 vgammatable_sld_bitpos = 16;
	const u32 ugammatable_sld_bitpos = 14;
	const u32 ygammatable_sld_bitpos = 12;
	const u32 vgammatable_sld_mask = (1 << vgammatable_sld_bitpos);
	const u32 ugammatable_sld_mask = (1 << ugammatable_sld_bitpos);
	const u32 ygammatable_sld_mask = (1 << ygammatable_sld_bitpos);
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;

	if (pby)
		*pby = (read_value & vgammatable_sld_mask) ? 0 : 1;

	if (pbu)
		*pbu = (read_value & ugammatable_sld_mask) ? 0 : 1;

	if (pbv)
		*pbv = (read_value & ygammatable_sld_mask) ? 0 : 1;
}

void nx_mlc_set_video_layer_gamma_enable(u32 module_index, int benable)
{
	const u32 yuvgammaemb_bitpos = 4;
	const u32 yuvgammaemb_mask = 1 << yuvgammaemb_bitpos;
	register u32 read_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->mlcgammacont;
	read_value &= ~yuvgammaemb_mask;
	read_value |= (u32)benable << yuvgammaemb_bitpos;

	writel(read_value, &pregister->mlcgammacont);
}

int nx_mlc_get_video_layer_gamma_enable(u32 module_index)
{
	const u32 yuvgammaemb_bitpos = 4;
	const u32 yuvgammaemb_mask = 1 << yuvgammaemb_bitpos;

	return (int)((__g_module_variables[module_index].pregister->mlcgammacont
		     & yuvgammaemb_mask) >> yuvgammaemb_bitpos);
}

void nx_mlc_set_gamma_table_poweroff(u32 module_index, int enb)
{
	register struct nx_mlc_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	if (enb == 1) {
		regvalue = pregister->mlcgammacont;
		regvalue = regvalue & 0xf3;
		writel(regvalue, &pregister->mlcgammacont);
	}
}

void nx_mlc_set_mlctop_control_parameter(u32 module_index, int field_enable,
					 int mlcenable, u8 priority,
					 enum g3daddrchangeallowed
					 g3daddr_change_allowed)
{
	register u32 mlctopcontrolreg;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	mlctopcontrolreg = (readl(&pregister->mlccontrolt)) & 0xfffffcfc;
	mlctopcontrolreg = (u32)(mlctopcontrolreg |
				  ((priority << 8) | ((mlcenable == 1) << 1) |
				   (1 ==
				    field_enable)) | (g3daddr_change_allowed <<
						      12));
	writel(mlctopcontrolreg, &pregister->mlccontrolt);
}

void nx_mlc_set_rgb0layer_control_parameter(u32 module_index, int layer_enable,
					    int grp3denable, int tp_enable,
					    u32 transparency_color,
					    int inv_enable, u32 inverse_color,
					    int blend_enable, u8 alpha_value,
					    enum mlc_rgbfmt rbgformat,
					    enum locksizesel lock_size_select)
{
	u32 layer_format;
	u32 control_enb;
	u32 alpha_argument;
	u32 lock_size = (u32)(lock_size_select & 0x3);
	u32 rgb0controlreg;
	u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	layer_format = nx_mlc_get_rgbformat(rbgformat);
	pregister = __g_module_variables[module_index].pregister;
	control_enb =
	    (u32)((grp3denable << 8) | (layer_enable << 5) |
		   (blend_enable << 2) | (inv_enable << 1) | tp_enable) & 0x127;
	alpha_argument = (u32)(alpha_value & 0xf);

	rgb0controlreg = readl(&pregister->mlcrgblayer[0].mlccontrol) & 0x10;
	regvalue =
	    (u32)(((layer_format << 16) | control_enb | (lock_size << 12)) |
		   rgb0controlreg);
	writel(regvalue, &pregister->mlcrgblayer[0].mlccontrol);

	regvalue = (u32)((alpha_argument << 28) | transparency_color);
	writel(regvalue, &pregister->mlcrgblayer[0].mlctpcolor);
	regvalue = inverse_color;
	writel(regvalue, &pregister->mlcrgblayer[0].mlcinvcolor);
}

u32 nx_mlc_get_rgbformat(enum mlc_rgbfmt rbgformat)
{
	u32 rgbformatvalue;
	const u32 format_table[] = {
		0x4432ul, 0x4342ul, 0x4211ul, 0x4120ul, 0x4003ul, 0x4554ul,
		0x3342ul, 0x2211ul, 0x1120ul, 0x1003ul, 0x4653ul, 0x4653ul,
		0x0653ul, 0x4ed3ul, 0x4f84ul, 0xc432ul, 0xc342ul, 0xc211ul,
		0xc120ul, 0xb342ul, 0xa211ul, 0x9120ul, 0xc653ul, 0xc653ul,
		0x8653ul, 0xced3ul, 0xcf84ul, 0x443aul
	};

	return rgbformatvalue = format_table[rbgformat];
}

void nx_mlc_set_rgb1layer_control_parameter(u32 module_index, int layer_enable,
					    int grp3denable, int tp_enable,
					    u32 transparency_color,
					    int inv_enable, u32 inverse_color,
					    int blend_enable, u8 alpha_value,
					    enum mlc_rgbfmt rbgformat,
					    enum locksizesel lock_size_select)
{
	u32 layer_format;
	u32 control_enb;
	u32 alpha_argument;
	u32 lock_size = (u32)(lock_size_select & 0x3);
	u32 rgb0controlreg;
	u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	layer_format = nx_mlc_get_rgbformat(rbgformat);
	pregister = __g_module_variables[module_index].pregister;

	rgb0controlreg = readl(&pregister->mlcrgblayer[1].mlccontrol) & 0x10;
	control_enb =
	    (u32)((grp3denable << 8) | (layer_enable << 5) |
		   (blend_enable << 2) | (inv_enable << 1) | tp_enable) & 0x127;
	alpha_argument = (u32)(alpha_value & 0xf);
	regvalue =
	    (u32)(((layer_format << 16) | control_enb | (lock_size << 12)) |
		   rgb0controlreg);
	writel(regvalue, &pregister->mlcrgblayer[1].mlccontrol);
	regvalue = (u32)((alpha_argument << 28) | transparency_color);
	writel(regvalue, &pregister->mlcrgblayer[1].mlctpcolor);
	regvalue = inverse_color;
	writel(regvalue, &pregister->mlcrgblayer[1].mlcinvcolor);
}

void nx_mlc_set_rgb2layer_control_parameter(u32 module_index, int layer_enable,
					    int grp3denable, int tp_enable,
					    u32 transparency_color,
					    int inv_enable, u32 inverse_color,
					    int blend_enable, u8 alpha_value,
					    enum mlc_rgbfmt rbgformat,
					    enum locksizesel lock_size_select)
{
	u32 layer_format;
	u32 control_enb;
	u32 alpha_argument;
	u32 lock_size = (u32)(lock_size_select & 0x3);
	u32 rgb0controlreg;
	u32 regvalue;
	register struct nx_mlc_register_set *pregister;

	layer_format = nx_mlc_get_rgbformat(rbgformat);
	pregister = __g_module_variables[module_index].pregister;

	rgb0controlreg = readl(&pregister->mlcrgblayer2.mlccontrol) & 0x10;
	control_enb =
	    (u32)((grp3denable << 8) | (layer_enable << 5) |
		   (blend_enable << 2) | (inv_enable << 1) | tp_enable) & 0x127;
	alpha_argument = (u32)(alpha_value & 0xf);
	regvalue =
	    (u32)(((layer_format << 16) | control_enb | (lock_size << 12)) |
		   rgb0controlreg);
	writel(regvalue, &pregister->mlcrgblayer2.mlccontrol);
	regvalue = (u32)((alpha_argument << 28) | transparency_color);
	writel(regvalue, &pregister->mlcrgblayer2.mlctpcolor);
	regvalue = inverse_color;
	writel(regvalue, &pregister->mlcrgblayer2.mlcinvcolor);
}

void nx_mlc_set_video_layer_control_parameter(u32 module_index,
					      int layer_enable, int tp_enable,
					      u32 transparency_color,
					      int inv_enable, u32 inverse_color,
					      int blend_enable, u8 alpha_value,
					      enum nx_mlc_yuvfmt yuvformat)
{
	u32 control_enb;
	u32 alpha_argument;
	u32 regvalue;
	register struct nx_mlc_register_set *pregister;
	u32 video_control_reg;

	pregister = __g_module_variables[module_index].pregister;

	video_control_reg = readl(&pregister->mlcvideolayer.mlccontrol);
	control_enb =
	    (u32)((yuvformat) | (layer_enable << 5) | (blend_enable << 2) |
		   (inv_enable << 1) | tp_enable) & 0x30027;
	alpha_argument = (u32)(alpha_value & 0xf);
	regvalue = (u32)(control_enb | video_control_reg);
	writel(regvalue, &pregister->mlcvideolayer.mlccontrol);
	regvalue = (u32)((alpha_argument << 28) | transparency_color);
	writel(regvalue, &pregister->mlcvideolayer.mlctpcolor);
	regvalue = (u32)((alpha_argument << 28) | transparency_color);
	writel(regvalue, &pregister->mlcvideolayer.mlcinvcolor);
}

void nx_mlc_set_srammode(u32 module_index, enum latyername layer_name,
			 enum srammode sram_mode)
{
	u32 control_reg_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	switch (layer_name) {
	case topmlc:
		control_reg_value = readl(&pregister->mlccontrolt);
		writel((u32)(control_reg_value | (sram_mode << 10)),
		       &pregister->mlccontrolt);
		control_reg_value = 0;
		break;
	case rgb0:
		control_reg_value =
		    readl(&pregister->mlcrgblayer[0].mlccontrol);
		writel((u32)(control_reg_value | (sram_mode << 14)),
		       &pregister->mlcrgblayer[0].mlccontrol);
		control_reg_value = 0;
		break;
	case rgb1:
		control_reg_value =
		    readl(&pregister->mlcrgblayer[1].mlccontrol);
		writel((u32)(control_reg_value | (sram_mode << 14)),
		       &pregister->mlcrgblayer[1].mlccontrol);
		control_reg_value = 0;
		break;
	case rgb2:
		control_reg_value = readl(&pregister->mlcrgblayer2.mlccontrol);
		writel((u32)(control_reg_value | (sram_mode << 14)),
		       &pregister->mlcrgblayer2.mlccontrol);
		control_reg_value = 0;
		break;
	case video:
		control_reg_value = readl(&pregister->mlcvideolayer.mlccontrol);
		writel((u32)(control_reg_value | (sram_mode << 14)),
		       &pregister->mlcvideolayer.mlccontrol);
		control_reg_value = 0;
		break;
	default:
		break;
	}
}

void nx_mlc_set_layer_reg_finish(u32 module_index, enum latyername layer_name)
{
	u32 control_reg_value;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	switch (layer_name) {
	case topmlc:
		control_reg_value = readl(&pregister->mlccontrolt);
		writel((u32)(control_reg_value | (1ul << 3)),
		       &pregister->mlccontrolt);
		control_reg_value = 0;
		break;
	case rgb0:
		control_reg_value =
		    readl(&pregister->mlcrgblayer[0].mlccontrol);
		writel((u32)(control_reg_value | (1ul << 4)),
		       &pregister->mlcrgblayer[0].mlccontrol);
		control_reg_value = 0;
		break;
	case rgb1:
		control_reg_value =
		    readl(&pregister->mlcrgblayer[1].mlccontrol);
		writel((u32)(control_reg_value | (1ul << 4)),
		       &pregister->mlcrgblayer[1].mlccontrol);
		control_reg_value = 0;
		break;
	case rgb2:
		control_reg_value = readl(&pregister->mlcrgblayer2.mlccontrol);
		writel((u32)(control_reg_value | (1ul << 4)),
		       &pregister->mlcrgblayer2.mlccontrol);
		control_reg_value = 0;
		break;
	case video:
		control_reg_value = readl(&pregister->mlcvideolayer.mlccontrol);
		writel((u32)(control_reg_value | (1ul << 4)),
		       &pregister->mlcvideolayer.mlccontrol);
		control_reg_value = 0;
		break;
	default:
		break;
	}
}

void nx_mlc_set_video_layer_coordinate(u32 module_index, int vfilterenable,
				       int hfilterenable, int vfilterenable_c,
				       int hfilterenable_c,
				       u16 video_layer_with,
				       u16 video_layer_height, s16 left,
				       s16 right, s16 top,
				       s16 bottom)
{
	s32 source_width, source_height;
	s32 destination_width;
	s32 destination_height;
	s32 hscale, vscale;
	s32 hfilterenb, vfilterenb;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((s32)(((left & 0x0fff) << 16) | (right & 0x0fff)),
	       &pregister->mlcvideolayer.mlcleftright);
	writel((s32)(((top & 0x0fff) << 16) | (bottom & 0x0fff)),
	       &pregister->mlcvideolayer.mlctopbottom);
	source_width = (s32)(video_layer_with - 1);
	source_height = (s32)(video_layer_height - 1);
	destination_width = (s32)(right - left);
	destination_height = (s32)(bottom - top);

	hscale =
	    (s32)((source_width * (1ul << 11) + (destination_width / 2)) /
		      destination_width);
	vscale =
	    (s32)((source_height * (1ul << 11) +
		      (destination_height / 2)) / destination_height);

	hfilterenb = (u32)(((hfilterenable_c << 29) | (hfilterenable) << 28)) &
	    0x30000000;
	vfilterenb = (u32)(((vfilterenable_c << 29) | (vfilterenable) << 28)) &
	    0x30000000;
	writel((u32)(hfilterenb | (hscale & 0x00ffffff)),
	       &pregister->mlcvideolayer.mlchscale);
	writel((u32)(vfilterenb | (vscale & 0x00ffffff)),
	       &pregister->mlcvideolayer.mlcvscale);
}

void nx_mlc_set_video_layer_filter_scale(u32 module_index, u32 hscale,
					 u32 vscale)
{
	register struct nx_mlc_register_set *pregister;
	u32 mlchscale = 0;
	u32 mlcvscale = 0;

	pregister = __g_module_variables[module_index].pregister;
	mlchscale = readl(&pregister->mlcvideolayer.mlchscale) & (~0x00ffffff);
	mlcvscale = readl(&pregister->mlcvideolayer.mlcvscale) & (~0x00ffffff);

	writel((u32)(mlchscale | (hscale & 0x00ffffff)),
	       &pregister->mlcvideolayer.mlchscale);
	writel((u32)(mlcvscale | (vscale & 0x00ffffff)),
	       &pregister->mlcvideolayer.mlcvscale);
}

void nx_mlc_set_gamma_control_parameter(u32 module_index, int rgbgammaenb,
					int yuvgammaenb, int yuvalphaarray,
					int dither_enb)
{
	u32 register_data;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	register_data = readl(&pregister->mlcgammacont);
	register_data = (register_data & 0xf0c) |
	    ((yuvalphaarray << 5) | (yuvgammaenb << 4) |
	     (rgbgammaenb << 1) | (dither_enb << 0));
	writel(register_data, &pregister->mlcgammacont);
}

void nx_mlc_set_layer_alpha256(u32 module_index, u32 layer, u32 alpha)
{
	u32 register_data;
	register struct nx_mlc_register_set *pregister;

	if (alpha < 0)
		alpha = 0;
	if (alpha > 255)
		alpha = 255;

	pregister = __g_module_variables[module_index].pregister;
	if (layer == 0) {
		register_data =
		    readl(&pregister->mlcrgblayer[0].mlctpcolor) & 0x00ffffff;
		register_data = register_data | (alpha << 24);
		writel(register_data, &pregister->mlcrgblayer[0].mlctpcolor);
	} else if (layer == 1) {
		register_data =
		    readl(&pregister->mlcrgblayer[1].mlctpcolor) & 0x00ffffff;
		register_data = register_data | (alpha << 24);
		writel(register_data, &pregister->mlcrgblayer[1].mlctpcolor);
	} else if (layer == 2) {
		register_data =
		    readl(&pregister->mlcrgblayer[1].mlctpcolor) & 0x00ffffff;
		register_data = register_data | (alpha << 24);
		writel(register_data, &pregister->mlcrgblayer2.mlctpcolor);
	} else {
		register_data =
		    readl(&pregister->mlcvideolayer.mlctpcolor) & 0x00ffffff;
		register_data = register_data | (alpha << 24);
		writel(register_data, &pregister->mlcvideolayer.mlctpcolor);
	}
}

int nx_mlc_is_under_flow(u32 module_index)
{
	const u32 underflow_pend_pos = 31;
	const u32 underflow_pend_mask = 1ul << underflow_pend_pos;

	return (int)((__g_module_variables[module_index].pregister->mlccontrolt
		     & underflow_pend_mask) >> underflow_pend_pos);
}

void nx_mlc_set_gamma_table(u32 module_index, int enb,
			    struct nx_mlc_gamma_table_parameter *p_gammatable)
{
	register struct nx_mlc_register_set *pregister;
	u32 i, regval = 0;

	pregister = __g_module_variables[module_index].pregister;
	if (enb == 1) {
		regval = readl(&pregister->mlcgammacont);

		regval = (1 << 11) | (1 << 9) | (1 << 3);
		writel(regval, &pregister->mlcgammacont);

		regval = regval | (1 << 10) | (1 << 8) | (1 << 2);
		writel(regval, &pregister->mlcgammacont);

		for (i = 0; i < 256; i++) {
			nx_mlc_set_rgblayer_rgamma_table(module_index, i,
						     p_gammatable->r_table[i]);
			nx_mlc_set_rgblayer_ggamma_table(module_index, i,
						     p_gammatable->g_table[i]);
			nx_mlc_set_rgblayer_bgamma_table(module_index, i,
						     p_gammatable->b_table[i]);
		}

		regval = regval | (p_gammatable->alphaselect << 5) |
		    (p_gammatable->yuvgammaenb << 4 |
		     p_gammatable->allgammaenb << 4) |
		    (p_gammatable->rgbgammaenb << 1 |
		     p_gammatable->allgammaenb << 1) |
		    (p_gammatable->ditherenb << 1);
		writel(regval, &pregister->mlcgammacont);
	} else {
		regval = regval & ~(1 << 10) & ~(1 << 8) & ~(1 << 2);
		writel(regval, &pregister->mlcgammacont);

		regval = regval & ~(1 << 11) & ~(1 << 9) & ~(1 << 3);
		writel(regval, &pregister->mlcgammacont);
	}
}

void nx_mlc_get_rgblayer_stride(u32 module_index, u32 layer, s32 *hstride,
				s32 *vstride)
{
	unsigned int hs, vs;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	hs = readl(&pregister->mlcrgblayer[layer].mlchstride);
	vs = readl(&pregister->mlcrgblayer[layer].mlcvstride);

	if (hstride)
		*(s32 *)hstride = hs;

	if (vstride)
		*(s32 *)vstride = vs;
}

void nx_mlc_get_rgblayer_address(u32 module_index, u32 layer,
				 u32 *phys_address)
{
	u32 pa;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	pa = readl(&pregister->mlcrgblayer[layer].mlcaddress);

	if (phys_address)
		*(u32 *)phys_address = pa;
}

void nx_mlc_get_position(u32 module_index, u32 layer, int *left, int *top,
			 int *right, int *bottom)
{
	int lr, tb;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	lr = readl(&pregister->mlcrgblayer[layer].mlcleftright);
	tb = readl(&pregister->mlcrgblayer[layer].mlctopbottom);

	if (left)
		*(int *)left = ((lr >> 16) & 0xFFUL);

	if (top)
		*(int *)top = ((tb >> 16) & 0xFFUL);

	if (right)
		*(int *)right = ((lr >> 0) & 0xFFUL);

	if (bottom)
		*(int *)bottom = ((tb >> 0) & 0xFFUL);
}

void nx_mlc_get_video_layer_address_yuyv(u32 module_index, u32 *address,
					 u32 *stride)
{
	u32 a, s;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	a = readl(&pregister->mlcvideolayer.mlcaddress);
	s = readl(&pregister->mlcvideolayer.mlcvstride);

	if (address)
		*(u32 *)address = a;

	if (stride)
		*(u32 *)stride = s;
}

void nx_mlc_get_video_layer_address(u32 module_index, u32 *lu_address,
				    u32 *cb_address, u32 *cr_address)
{
	u32 lua, cba, cra;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	lua = readl(&pregister->mlcvideolayer.mlcaddress);
	cba = readl(&pregister->mlcvideolayer.mlcaddresscb);
	cra = readl(&pregister->mlcvideolayer.mlcaddresscr);

	if (lu_address)
		*(u32 *)lu_address = lua;

	if (cb_address)
		*(u32 *)cb_address = cba;

	if (cr_address)
		*(u32 *)cr_address = cra;
}

void nx_mlc_get_video_layer_stride(u32 module_index, u32 *lu_stride,
				   u32 *cb_stride, u32 *cr_stride)
{
	u32 lus, cbs, crs;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	lus = readl(&pregister->mlcvideolayer.mlcvstride);
	cbs = readl(&pregister->mlcvideolayer.mlcvstridecb);
	crs = readl(&pregister->mlcvideolayer.mlcvstridecr);

	if (lu_stride)
		*(u32 *)lu_stride = lus;

	if (cb_stride)
		*(u32 *)cb_stride = cbs;

	if (cr_stride)
		*(u32 *)cr_stride = crs;
}

void nx_mlc_get_video_position(u32 module_index, int *left, int *top,
			       int *right, int *bottom)
{
	int lr, tb;
	register struct nx_mlc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	lr = readl(&pregister->mlcvideolayer.mlcleftright);
	tb = readl(&pregister->mlcvideolayer.mlctopbottom);

	if (left)
		*(int *)left = ((lr >> 16) & 0xFFUL);

	if (top)
		*(int *)top = ((tb >> 16) & 0xFFUL);

	if (right)
		*(int *)right = ((lr >> 0) & 0xFFUL);

	if (bottom)
		*(int *)bottom = ((tb >> 0) & 0xFFUL);
}
