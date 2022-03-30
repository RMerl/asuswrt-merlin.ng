// SPDX-License-Identifier: GPL-2.0+
/*
 * PXA LCD Controller
 *
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
 */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

#include <common.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <lcd.h>
#include <linux/types.h>
#include <stdarg.h>
#include <stdio_dev.h>

/* #define DEBUG */

#ifdef CONFIG_LCD

/*----------------------------------------------------------------------*/
/*
 * Define panel bpp, LCCR0, LCCR3 and panel_info video struct for
 * your display.
 */

#ifdef CONFIG_PXA_VGA
/* LCD outputs connected to a video DAC  */
# define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f8
# define REG_LCCR3	0x0300FF01

/* 640x480x16 @ 61 Hz */
vidinfo_t panel_info = {
	.vl_col		= 640,
	.vl_row		= 480,
	.vl_width	= 640,
	.vl_height	= 480,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_HIGH,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 0,
	.vl_clor	= 0,
	.vl_tft		= 1,
	.vl_hpw		= 40,
	.vl_blw		= 56,
	.vl_elw		= 56,
	.vl_vpw		= 20,
	.vl_bfw		= 8,
	.vl_efw		= 8,
};
#endif /* CONFIG_PXA_VIDEO */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_SHARP_LM8V31

# define LCD_BPP	LCD_COLOR8
# define LCD_INVERT_COLORS	/* Needed for colors to be correct, but why?	*/

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x0030087C
# define REG_LCCR3	0x0340FF08

vidinfo_t panel_info = {
	.vl_col		= 640,
	.vl_row		= 480,
	.vl_width	= 157,
	.vl_height	= 118,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_HIGH,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 1,
	.vl_clor	= 1,
	.vl_tft		= 0,
	.vl_hpw		= 1,
	.vl_blw		= 3,
	.vl_elw		= 3,
	.vl_vpw		= 1,
	.vl_bfw		= 0,
	.vl_efw		= 0,
};
#endif /* CONFIG_SHARP_LM8V31 */
/*----------------------------------------------------------------------*/
#ifdef CONFIG_VOIPAC_LCD

# define LCD_BPP	LCD_COLOR8
# define LCD_INVERT_COLORS

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x043008f8
# define REG_LCCR3	0x0340FF08

vidinfo_t panel_info = {
	.vl_col		= 640,
	.vl_row		= 480,
	.vl_width	= 157,
	.vl_height	= 118,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_HIGH,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 1,
	.vl_clor	= 1,
	.vl_tft		= 1,
	.vl_hpw		= 32,
	.vl_blw		= 144,
	.vl_elw		= 32,
	.vl_vpw		= 2,
	.vl_bfw		= 13,
	.vl_efw		= 30,
};
#endif /* CONFIG_VOIPAC_LCD */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_HITACHI_SX14
/* Hitachi SX14Q004-ZZA color STN LCD */
#define LCD_BPP		LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
#define REG_LCCR0	0x00301079
#define REG_LCCR3	0x0340FF20

vidinfo_t panel_info = {
	.vl_col		= 320,
	.vl_row		= 240,
	.vl_width	= 167,
	.vl_height	= 109,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_HIGH,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 1,
	.vl_splt	= 0,
	.vl_clor	= 1,
	.vl_tft		= 0,
	.vl_hpw		= 1,
	.vl_blw		= 1,
	.vl_elw		= 1,
	.vl_vpw		= 7,
	.vl_bfw		= 0,
	.vl_efw		= 0,
};
#endif /* CONFIG_HITACHI_SX14 */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_LMS283GF05

# define LCD_BPP	LCD_COLOR8
/*# define LCD_INVERT_COLORS*/

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x043008f8
# define REG_LCCR3	0x03b00009

vidinfo_t panel_info = {
	.vl_col		= 240,
	.vl_row		= 320,
	.vl_rot		= 3,
	.vl_width	= 240,
	.vl_height	= 320,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_LOW,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 1,
	.vl_clor	= 1,
	.vl_tft		= 1,
	.vl_hpw		= 4,
	.vl_blw		= 4,
	.vl_elw		= 8,
	.vl_vpw		= 4,
	.vl_bfw		= 4,
	.vl_efw		= 8,
};
#endif /* CONFIG_LMS283GF05 */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_ACX517AKN

# define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f9
# define REG_LCCR3	0x03700006

vidinfo_t panel_info = {
	.vl_col		= 320,
	.vl_row		= 320,
	.vl_width	= 320,
	.vl_height	= 320,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_LOW,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 1,
	.vl_clor	= 1,
	.vl_tft		= 1,
	.vl_hpw		= 0x04,
	.vl_blw		= 0x1c,
	.vl_elw		= 0x08,
	.vl_vpw		= 0x01,
	.vl_bfw		= 0x07,
	.vl_efw		= 0x08,
};
#endif /* CONFIG_ACX517AKN */

#ifdef CONFIG_ACX544AKN

# define LCD_BPP	LCD_COLOR16

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f9
# define REG_LCCR3	0x04700007 /* 16bpp */

vidinfo_t panel_info = {
	.vl_col		= 320,
	.vl_row		= 320,
	.vl_width	= 320,
	.vl_height	= 320,
	.vl_clkp	= CONFIG_SYS_LOW,
	.vl_oep		= CONFIG_SYS_LOW,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_LOW,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 0,
	.vl_clor	= 1,
	.vl_tft		= 1,
	.vl_hpw		= 0x05,
	.vl_blw		= 0x13,
	.vl_elw		= 0x08,
	.vl_vpw		= 0x02,
	.vl_bfw		= 0x07,
	.vl_efw		= 0x05,
};
#endif /* CONFIG_ACX544AKN */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_LQ038J7DH53

# define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f9
# define REG_LCCR3	0x03700004

vidinfo_t panel_info = {
	.vl_col		= 320,
	.vl_row		= 480,
	.vl_width	= 320,
	.vl_height	= 480,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_LOW,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 1,
	.vl_clor	= 1,
	.vl_tft		= 1,
	.vl_hpw		= 0x04,
	.vl_blw		= 0x20,
	.vl_elw		= 0x01,
	.vl_vpw		= 0x01,
	.vl_bfw		= 0x04,
	.vl_efw		= 0x01,
};
#endif /* CONFIG_ACX517AKN */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_LITTLETON_LCD
# define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f8
# define REG_LCCR3	0x0300FF04

vidinfo_t panel_info = {
	.vl_col		= 480,
	.vl_row		= 640,
	.vl_width	= 480,
	.vl_height	= 640,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_oep		= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_HIGH,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_HIGH,
	.vl_bpix	= LCD_BPP,
	.vl_lbw		= 0,
	.vl_splt	= 0,
	.vl_clor	= 0,
	.vl_tft		= 1,
	.vl_hpw		= 9,
	.vl_blw		= 8,
	.vl_elw		= 24,
	.vl_vpw		= 2,
	.vl_bfw		= 2,
	.vl_efw		= 4,
};
#endif /* CONFIG_LITTLETON_LCD */

/*----------------------------------------------------------------------*/

static int pxafb_init_mem (void *lcdbase, vidinfo_t *vid);
static void pxafb_setup_gpio (vidinfo_t *vid);
static void pxafb_enable_controller (vidinfo_t *vid);
static int pxafb_init (vidinfo_t *vid);

/************************************************************************/
/* ---------------  PXA chipset specific functions  ------------------- */
/************************************************************************/

ushort *configuration_get_cmap(void)
{
	struct pxafb_info *fbi = &panel_info.pxa;
	return (ushort *)fbi->palette;
}

void lcd_ctrl_init (void *lcdbase)
{
	pxafb_init_mem(lcdbase, &panel_info);
	pxafb_init(&panel_info);
	pxafb_setup_gpio(&panel_info);
	pxafb_enable_controller(&panel_info);
}

/*----------------------------------------------------------------------*/
#if LCD_BPP == LCD_COLOR8
void
lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
	struct pxafb_info *fbi = &panel_info.pxa;
	unsigned short *palette = (unsigned short *)fbi->palette;
	u_int val;

	if (regno < fbi->palette_size) {
		val = ((red << 8) & 0xf800);
		val |= ((green << 4) & 0x07e0);
		val |= (blue & 0x001f);

#ifdef LCD_INVERT_COLORS
		palette[regno] = ~val;
#else
		palette[regno] = val;
#endif
	}

	debug ("setcolreg: reg %2d @ %p: R=%02X G=%02X B=%02X => %04X\n",
		regno, &palette[regno],
		red, green, blue,
		palette[regno]);
}
#endif /* LCD_COLOR8 */

/*----------------------------------------------------------------------*/
__weak void lcd_enable(void)
{
}

/************************************************************************/
/* ** PXA255 specific routines						*/
/************************************************************************/

/*
 * Calculate fb size for VIDEOLFB_ATAG. Size returned contains fb,
 * descriptors and palette areas.
 */
ulong calc_fbsize (void)
{
	ulong size;
	int line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	size = line_length * panel_info.vl_row;
	size += PAGE_SIZE;

	return size;
}

static int pxafb_init_mem (void *lcdbase, vidinfo_t *vid)
{
	u_long palette_mem_size;
	struct pxafb_info *fbi = &vid->pxa;
	int fb_size = vid->vl_row * (vid->vl_col * NBITS (vid->vl_bpix)) / 8;

	fbi->screen = (u_long)lcdbase;

	fbi->palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;
	palette_mem_size = fbi->palette_size * sizeof(u16);

	debug("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);
	/* locate palette and descs at end of page following fb */
	fbi->palette = (u_long)lcdbase + fb_size + PAGE_SIZE - palette_mem_size;

	return 0;
}
#ifdef	CONFIG_CPU_MONAHANS
static inline void pxafb_setup_gpio (vidinfo_t *vid) {}
#else
static void pxafb_setup_gpio (vidinfo_t *vid)
{
	u_long lccr0;

	/*
	 * setup is based on type of panel supported
	 */

	lccr0 = vid->pxa.reg_lccr0;

	/* 4 bit interface */
	if ((lccr0 & LCCR0_CMS) && (lccr0 & LCCR0_SDS) && !(lccr0 & LCCR0_DPD))
	{
		debug("Setting GPIO for 4 bit data\n");
		/* bits 58-61 */
		writel(readl(GPDR1) | (0xf << 26), GPDR1);
		writel((readl(GAFR1_U) & ~(0xff << 20)) | (0xaa << 20),
			GAFR1_U);

		/* bits 74-77 */
		writel(readl(GPDR2) | (0xf << 10), GPDR2);
		writel((readl(GAFR2_L) & ~(0xff << 20)) | (0xaa << 20),
			GAFR2_L);
	}

	/* 8 bit interface */
	else if (((lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_DPD))) ||
		(!(lccr0 & LCCR0_CMS) && !(lccr0 & LCCR0_PAS) && !(lccr0 & LCCR0_SDS)))
	{
		debug("Setting GPIO for 8 bit data\n");
		/* bits 58-65 */
		writel(readl(GPDR1) | (0x3f << 26), GPDR1);
		writel(readl(GPDR2) | (0x3), GPDR2);

		writel((readl(GAFR1_U) & ~(0xfff << 20)) | (0xaaa << 20),
			GAFR1_U);
		writel((readl(GAFR2_L) & ~0xf) | (0xa), GAFR2_L);

		/* bits 74-77 */
		writel(readl(GPDR2) | (0xf << 10), GPDR2);
		writel((readl(GAFR2_L) & ~(0xff << 20)) | (0xaa << 20),
			GAFR2_L);
	}

	/* 16 bit interface */
	else if (!(lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_PAS)))
	{
		debug("Setting GPIO for 16 bit data\n");
		/* bits 58-77 */
		writel(readl(GPDR1) | (0x3f << 26), GPDR1);
		writel(readl(GPDR2) | 0x00003fff, GPDR2);

		writel((readl(GAFR1_U) & ~(0xfff << 20)) | (0xaaa << 20),
			GAFR1_U);
		writel((readl(GAFR2_L) & 0xf0000000) | 0x0aaaaaaa, GAFR2_L);
	}
	else
	{
		printf("pxafb_setup_gpio: unable to determine bits per pixel\n");
	}
}
#endif

static void pxafb_enable_controller (vidinfo_t *vid)
{
	debug("Enabling LCD controller\n");

	/* Sequence from 11.7.10 */
	writel(vid->pxa.reg_lccr3, LCCR3);
	writel(vid->pxa.reg_lccr2, LCCR2);
	writel(vid->pxa.reg_lccr1, LCCR1);
	writel(vid->pxa.reg_lccr0 & ~LCCR0_ENB, LCCR0);
	writel(vid->pxa.fdadr0, FDADR0);
	writel(vid->pxa.fdadr1, FDADR1);
	writel(readl(LCCR0) | LCCR0_ENB, LCCR0);

#ifdef	CONFIG_CPU_MONAHANS
	writel(readl(CKENA) | CKENA_1_LCD, CKENA);
#else
	writel(readl(CKEN) | CKEN16_LCD, CKEN);
#endif

	debug("FDADR0 = 0x%08x\n", readl(FDADR0));
	debug("FDADR1 = 0x%08x\n", readl(FDADR1));
	debug("LCCR0 = 0x%08x\n", readl(LCCR0));
	debug("LCCR1 = 0x%08x\n", readl(LCCR1));
	debug("LCCR2 = 0x%08x\n", readl(LCCR2));
	debug("LCCR3 = 0x%08x\n", readl(LCCR3));
}

static int pxafb_init (vidinfo_t *vid)
{
	struct pxafb_info *fbi = &vid->pxa;

	debug("Configuring PXA LCD\n");

	fbi->reg_lccr0 = REG_LCCR0;
	fbi->reg_lccr3 = REG_LCCR3;

	debug("vid: vl_col=%d hslen=%d lm=%d rm=%d\n",
		vid->vl_col, vid->vl_hpw,
		vid->vl_blw, vid->vl_elw);
	debug("vid: vl_row=%d vslen=%d um=%d bm=%d\n",
		vid->vl_row, vid->vl_vpw,
		vid->vl_bfw, vid->vl_efw);

	fbi->reg_lccr1 =
		LCCR1_DisWdth(vid->vl_col) +
		LCCR1_HorSnchWdth(vid->vl_hpw) +
		LCCR1_BegLnDel(vid->vl_blw) +
		LCCR1_EndLnDel(vid->vl_elw);

	fbi->reg_lccr2 =
		LCCR2_DisHght(vid->vl_row) +
		LCCR2_VrtSnchWdth(vid->vl_vpw) +
		LCCR2_BegFrmDel(vid->vl_bfw) +
		LCCR2_EndFrmDel(vid->vl_efw);

	fbi->reg_lccr3 = REG_LCCR3 & ~(LCCR3_HSP | LCCR3_VSP);
	fbi->reg_lccr3 |= (vid->vl_hsp ? LCCR3_HorSnchL : LCCR3_HorSnchH)
			| (vid->vl_vsp ? LCCR3_VrtSnchL : LCCR3_VrtSnchH);


	/* setup dma descriptors */
	fbi->dmadesc_fblow = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 3*16);
	fbi->dmadesc_fbhigh = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 2*16);
	fbi->dmadesc_palette = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 1*16);

	#define BYTES_PER_PANEL	((fbi->reg_lccr0 & LCCR0_SDS) ? \
		(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8 / 2) : \
		(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8))

	/* populate descriptors */
	fbi->dmadesc_fblow->fdadr = (u_long)fbi->dmadesc_fblow;
	fbi->dmadesc_fblow->fsadr = fbi->screen + BYTES_PER_PANEL;
	fbi->dmadesc_fblow->fidr  = 0;
	fbi->dmadesc_fblow->ldcmd = BYTES_PER_PANEL;

	fbi->fdadr1 = (u_long)fbi->dmadesc_fblow; /* only used in dual-panel mode */

	fbi->dmadesc_fbhigh->fsadr = fbi->screen;
	fbi->dmadesc_fbhigh->fidr = 0;
	fbi->dmadesc_fbhigh->ldcmd = BYTES_PER_PANEL;

	fbi->dmadesc_palette->fsadr = fbi->palette;
	fbi->dmadesc_palette->fidr  = 0;
	fbi->dmadesc_palette->ldcmd = (fbi->palette_size * 2) | LDCMD_PAL;

	if( NBITS(vid->vl_bpix) < 12)
	{
		/* assume any mode with <12 bpp is palette driven */
		fbi->dmadesc_palette->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_palette;
		/* flips back and forth between pal and fbhigh */
		fbi->fdadr0 = (u_long)fbi->dmadesc_palette;
	}
	else
	{
		/* palette shouldn't be loaded in true-color mode */
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->fdadr0 = (u_long)fbi->dmadesc_fbhigh; /* no pal just fbhigh */
	}

	debug("fbi->dmadesc_fblow = 0x%lx\n", (u_long)fbi->dmadesc_fblow);
	debug("fbi->dmadesc_fbhigh = 0x%lx\n", (u_long)fbi->dmadesc_fbhigh);
	debug("fbi->dmadesc_palette = 0x%lx\n", (u_long)fbi->dmadesc_palette);

	debug("fbi->dmadesc_fblow->fdadr = 0x%lx\n", fbi->dmadesc_fblow->fdadr);
	debug("fbi->dmadesc_fbhigh->fdadr = 0x%lx\n", fbi->dmadesc_fbhigh->fdadr);
	debug("fbi->dmadesc_palette->fdadr = 0x%lx\n", fbi->dmadesc_palette->fdadr);

	debug("fbi->dmadesc_fblow->fsadr = 0x%lx\n", fbi->dmadesc_fblow->fsadr);
	debug("fbi->dmadesc_fbhigh->fsadr = 0x%lx\n", fbi->dmadesc_fbhigh->fsadr);
	debug("fbi->dmadesc_palette->fsadr = 0x%lx\n", fbi->dmadesc_palette->fsadr);

	debug("fbi->dmadesc_fblow->ldcmd = 0x%lx\n", fbi->dmadesc_fblow->ldcmd);
	debug("fbi->dmadesc_fbhigh->ldcmd = 0x%lx\n", fbi->dmadesc_fbhigh->ldcmd);
	debug("fbi->dmadesc_palette->ldcmd = 0x%lx\n", fbi->dmadesc_palette->ldcmd);

	return 0;
}

/************************************************************************/
/************************************************************************/

#endif /* CONFIG_LCD */
