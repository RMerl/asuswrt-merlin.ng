// SPDX-License-Identifier: GPL-2.0+
/*
 * TV encoder driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <common.h>

#include <asm/arch/tve.h>
#include <asm/io.h>

void tvencoder_mode_set(struct sunxi_tve_reg * const tve, enum tve_mode mode)
{
	switch (mode) {
	case tve_mode_vga:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_VGA, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_VGA, &tve->dac_cfg0);
		writel(SUNXI_TVE_UNKNOWN1_VGA, &tve->unknown1);
		break;
	case tve_mode_composite_pal_nc:
		writel(SUNXI_TVE_CHROMA_FREQ_PAL_NC, &tve->chroma_freq);
		/* Fall through */
	case tve_mode_composite_pal:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(3, 4), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_PAL, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_COMPOSITE, &tve->dac_cfg0);
		writel(SUNXI_TVE_FILTER_COMPOSITE, &tve->filter);
		writel(SUNXI_TVE_PORCH_NUM_PAL, &tve->porch_num);
		writel(SUNXI_TVE_LINE_NUM_PAL, &tve->line_num);
		writel(SUNXI_TVE_BLANK_BLACK_LEVEL_PAL,
		       &tve->blank_black_level);
		writel(SUNXI_TVE_UNKNOWN1_COMPOSITE, &tve->unknown1);
		writel(SUNXI_TVE_CBR_LEVEL_PAL, &tve->cbr_level);
		writel(SUNXI_TVE_BURST_WIDTH_COMPOSITE, &tve->burst_width);
		writel(SUNXI_TVE_UNKNOWN2_PAL, &tve->unknown2);
		writel(SUNXI_TVE_ACTIVE_NUM_COMPOSITE, &tve->active_num);
		writel(SUNXI_TVE_CHROMA_BW_GAIN_COMP, &tve->chroma_bw_gain);
		writel(SUNXI_TVE_NOTCH_WIDTH_COMPOSITE, &tve->notch_width);
		writel(SUNXI_TVE_RESYNC_NUM_PAL, &tve->resync_num);
		writel(SUNXI_TVE_SLAVE_PARA_COMPOSITE, &tve->slave_para);
		break;
	case tve_mode_composite_pal_m:
		writel(SUNXI_TVE_CHROMA_FREQ_PAL_M, &tve->chroma_freq);
		writel(SUNXI_TVE_COLOR_BURST_PAL_M, &tve->color_burst);
		/* Fall through */
	case tve_mode_composite_ntsc:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(3, 4), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_NTSC, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_COMPOSITE, &tve->dac_cfg0);
		writel(SUNXI_TVE_FILTER_COMPOSITE, &tve->filter);
		writel(SUNXI_TVE_PORCH_NUM_NTSC, &tve->porch_num);
		writel(SUNXI_TVE_LINE_NUM_NTSC, &tve->line_num);
		writel(SUNXI_TVE_BLANK_BLACK_LEVEL_NTSC,
		       &tve->blank_black_level);
		writel(SUNXI_TVE_UNKNOWN1_COMPOSITE, &tve->unknown1);
		writel(SUNXI_TVE_CBR_LEVEL_NTSC, &tve->cbr_level);
		writel(SUNXI_TVE_BURST_PHASE_NTSC, &tve->burst_phase);
		writel(SUNXI_TVE_BURST_WIDTH_COMPOSITE, &tve->burst_width);
		writel(SUNXI_TVE_UNKNOWN2_NTSC, &tve->unknown2);
		writel(SUNXI_TVE_SYNC_VBI_LEVEL_NTSC, &tve->sync_vbi_level);
		writel(SUNXI_TVE_ACTIVE_NUM_COMPOSITE, &tve->active_num);
		writel(SUNXI_TVE_CHROMA_BW_GAIN_COMP, &tve->chroma_bw_gain);
		writel(SUNXI_TVE_NOTCH_WIDTH_COMPOSITE, &tve->notch_width);
		writel(SUNXI_TVE_RESYNC_NUM_NTSC, &tve->resync_num);
		writel(SUNXI_TVE_SLAVE_PARA_COMPOSITE, &tve->slave_para);
		break;
	}
}

void tvencoder_enable(struct sunxi_tve_reg * const tve)
{
	setbits_le32(&tve->gctrl, SUNXI_TVE_GCTRL_ENABLE);
}
