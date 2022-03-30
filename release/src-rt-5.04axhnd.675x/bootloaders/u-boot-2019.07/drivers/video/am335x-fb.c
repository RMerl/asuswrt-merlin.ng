// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2018 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 * minimal framebuffer driver for TI's AM335x SoC to be compatible with
 * Wolfgang Denk's LCD-Framework (CONFIG_LCD, common/lcd.c)
 *
 * - supporting 16/24/32bit RGB/TFT raster Mode (not using palette)
 * - sets up LCD controller as in 'am335x_lcdpanel' struct given
 * - starts output DMA from gd->fb_base buffer
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <lcd.h>
#include "am335x-fb.h"

#if !defined(LCD_CNTL_BASE)
#error "hw-base address of LCD-Controller (LCD_CNTL_BASE) not defined!"
#endif

#define LCDC_FMAX				200000000

/* LCD Control Register */
#define LCD_CLK_DIVISOR(x)			((x) << 8)
#define LCD_RASTER_MODE				0x01
/* LCD Clock Enable Register */
#define LCD_CORECLKEN				(0x01 << 0)
#define LCD_LIDDCLKEN				(0x01 << 1)
#define LCD_DMACLKEN				(0x01 << 2)
/* LCD DMA Control Register */
#define LCD_DMA_BURST_SIZE(x)			((x) << 4)
#define LCD_DMA_BURST_1				0x0
#define LCD_DMA_BURST_2				0x1
#define LCD_DMA_BURST_4				0x2
#define LCD_DMA_BURST_8				0x3
#define LCD_DMA_BURST_16			0x4
/* LCD Timing_0 Register */
#define LCD_HBPLSB(x)				((((x)-1) & 0xFF) << 24)
#define LCD_HFPLSB(x)				((((x)-1) & 0xFF) << 16)
#define LCD_HSWLSB(x)				((((x)-1) & 0x3F) << 10)
#define LCD_HORLSB(x)				(((((x) >> 4)-1) & 0x3F) << 4)
#define LCD_HORMSB(x)				(((((x) >> 4)-1) & 0x40) >> 4)
/* LCD Timing_1 Register */
#define LCD_VBP(x)				((x) << 24)
#define LCD_VFP(x)				((x) << 16)
#define LCD_VSW(x)				(((x)-1) << 10)
#define LCD_VERLSB(x)				(((x)-1) & 0x3FF)
/* LCD Timing_2 Register */
#define LCD_HSWMSB(x)				((((x)-1) & 0x3C0) << 21)
#define LCD_VERMSB(x)				((((x)-1) & 0x400) << 16)
#define LCD_HBPMSB(x)				((((x)-1) & 0x300) >> 4)
#define LCD_HFPMSB(x)				((((x)-1) & 0x300) >> 8)
#define LCD_INVMASK(x)				((x) & 0x3F00000)
/* LCD Raster Ctrl Register */
#define LCD_TFT_24BPP_MODE			(1 << 25)
#define LCD_TFT_24BPP_UNPACK			(1 << 26)
#define LCD_PALMODE_RAWDATA			(0x02 << 20)
#define LCD_TFT_MODE				(0x01 << 7)
#define LCD_RASTER_ENABLE			(0x01 << 0)


/* Macro definitions */
#define FBSIZE(x)	((x->hactive * x->vactive * x->bpp) >> 3)

struct am335x_lcdhw {
	unsigned int		pid;			/* 0x00 */
	unsigned int		ctrl;			/* 0x04 */
	unsigned int		gap0;			/* 0x08 */
	unsigned int		lidd_ctrl;		/* 0x0C */
	unsigned int		lidd_cs0_conf;		/* 0x10 */
	unsigned int		lidd_cs0_addr;		/* 0x14 */
	unsigned int		lidd_cs0_data;		/* 0x18 */
	unsigned int		lidd_cs1_conf;		/* 0x1C */
	unsigned int		lidd_cs1_addr;		/* 0x20 */
	unsigned int		lidd_cs1_data;		/* 0x24 */
	unsigned int		raster_ctrl;		/* 0x28 */
	unsigned int		raster_timing0;		/* 0x2C */
	unsigned int		raster_timing1;		/* 0x30 */
	unsigned int		raster_timing2;		/* 0x34 */
	unsigned int		raster_subpanel;	/* 0x38 */
	unsigned int		raster_subpanel2;	/* 0x3C */
	unsigned int		lcddma_ctrl;		/* 0x40 */
	unsigned int		lcddma_fb0_base;	/* 0x44 */
	unsigned int		lcddma_fb0_ceiling;	/* 0x48 */
	unsigned int		lcddma_fb1_base;	/* 0x4C */
	unsigned int		lcddma_fb1_ceiling;	/* 0x50 */
	unsigned int		sysconfig;		/* 0x54 */
	unsigned int		irqstatus_raw;		/* 0x58 */
	unsigned int		irqstatus;		/* 0x5C */
	unsigned int		irqenable_set;		/* 0x60 */
	unsigned int		irqenable_clear;	/* 0x64 */
	unsigned int		gap1;			/* 0x68 */
	unsigned int		clkc_enable;		/* 0x6C */
	unsigned int		clkc_reset;		/* 0x70 */
};

static struct am335x_lcdhw *lcdhw = (void *)LCD_CNTL_BASE;

DECLARE_GLOBAL_DATA_PTR;

int lcd_get_size(int *line_length)
{
	*line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;
	return *line_length * panel_info.vl_row + 0x20;
}

int am335xfb_init(struct am335x_lcdpanel *panel)
{
	u32 raster_ctrl = 0;

	struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;
	struct dpll_params dpll_disp = { 1, 0, 1, -1, -1, -1, -1 };
	unsigned int m, n, d, best_d = 2;
	int err = 0, err_r = 0;

	if (gd->fb_base == 0) {
		printf("ERROR: no valid fb_base stored in GLOBAL_DATA_PTR!\n");
		return -1;
	}
	if (panel == NULL) {
		printf("ERROR: missing ptr to am335x_lcdpanel!\n");
		return -1;
	}

	/* We can already set the bits for the raster_ctrl in this check */
	switch (panel->bpp) {
	case 16:
		break;
	case 32:
		raster_ctrl |= LCD_TFT_24BPP_UNPACK;
		/* fallthrough */
	case 24:
		raster_ctrl |= LCD_TFT_24BPP_MODE;
		break;
	default:
		pr_err("am335x-fb: invalid bpp value: %d\n", panel->bpp);
		return -1;
	}

	/* check given clock-frequency */
	if (panel->pxl_clk > (LCDC_FMAX / 2)) {
		pr_err("am335x-fb: requested pxl-clk: %d not supported!\n",
		       panel->pxl_clk);
		return -1;
	}

	debug("setting up LCD-Controller for %dx%dx%d (hfp=%d,hbp=%d,hsw=%d / ",
	      panel->hactive, panel->vactive, panel->bpp,
	      panel->hfp, panel->hbp, panel->hsw);
	debug("vfp=%d,vbp=%d,vsw=%d / clk=%d)\n",
	      panel->vfp, panel->vfp, panel->vsw, panel->pxl_clk);
	debug("using frambuffer at 0x%08x with size %d.\n",
	      (unsigned int)gd->fb_base, FBSIZE(panel));

	/* setup display pll for requested clock frequency */
	err = panel->pxl_clk;
	err_r = err;

	for (d = 2; d < 255; d++) {
		for (m = 2; m < 2047; m++) {
			if ((V_OSCK * m) < (panel->pxl_clk * d))
				continue;
			n = (V_OSCK * m) / (panel->pxl_clk * d);
			if (n > 127)
				break;
			if (((V_OSCK * m) / n) > LCDC_FMAX)
				break;

			err = abs((V_OSCK * m) / n / d - panel->pxl_clk);
			if (err < err_r) {
				err_r = err;
				dpll_disp.m = m;
				dpll_disp.n = n;
				best_d = d;
			}
		}
	}
	debug("%s: PLL: best error %d Hz (M %d, N %d, DISP %d)\n",
	      __func__, err_r, dpll_disp.m, dpll_disp.n, best_d);
	do_setup_dpll(&dpll_disp_regs, &dpll_disp);

	/* clock source for LCDC from dispPLL M2 */
	writel(0x0, &cmdpll->clklcdcpixelclk);

	/* palette default entry */
	memset((void *)gd->fb_base, 0, 0x20);
	*(unsigned int *)gd->fb_base = 0x4000;
	/* point fb behind palette */
	gd->fb_base += 0x20;

	/* turn ON display through powercontrol function if accessible */
	if (panel->panel_power_ctrl != NULL)
		panel->panel_power_ctrl(1);

	debug("am335x-fb: wait for stable power ...\n");
	mdelay(panel->pup_delay);
	lcdhw->clkc_enable = LCD_CORECLKEN | LCD_LIDDCLKEN | LCD_DMACLKEN;
	lcdhw->raster_ctrl = 0;
	lcdhw->ctrl = LCD_CLK_DIVISOR(best_d) | LCD_RASTER_MODE;
	lcdhw->lcddma_fb0_base = gd->fb_base;
	lcdhw->lcddma_fb0_ceiling = gd->fb_base + FBSIZE(panel);
	lcdhw->lcddma_fb1_base = gd->fb_base;
	lcdhw->lcddma_fb1_ceiling = gd->fb_base + FBSIZE(panel);
	lcdhw->lcddma_ctrl = LCD_DMA_BURST_SIZE(LCD_DMA_BURST_16);

	lcdhw->raster_timing0 = LCD_HORLSB(panel->hactive) |
				LCD_HORMSB(panel->hactive) |
				LCD_HFPLSB(panel->hfp) |
				LCD_HBPLSB(panel->hbp) |
				LCD_HSWLSB(panel->hsw);
	lcdhw->raster_timing1 = LCD_VBP(panel->vbp) |
				LCD_VFP(panel->vfp) |
				LCD_VSW(panel->vsw) |
				LCD_VERLSB(panel->vactive);
	lcdhw->raster_timing2 = LCD_HSWMSB(panel->hsw) |
				LCD_VERMSB(panel->vactive) |
				LCD_INVMASK(panel->pol) |
				LCD_HBPMSB(panel->hbp) |
				LCD_HFPMSB(panel->hfp) |
				0x0000FF00;	/* clk cycles for ac-bias */
	lcdhw->raster_ctrl =	raster_ctrl |
				LCD_PALMODE_RAWDATA |
				LCD_TFT_MODE |
				LCD_RASTER_ENABLE;

	debug("am335x-fb: waiting picture to be stable.\n.");
	mdelay(panel->pon_delay);

	return 0;
}
