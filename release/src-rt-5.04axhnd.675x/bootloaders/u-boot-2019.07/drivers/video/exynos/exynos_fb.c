// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <config.h>
#include <common.h>
#include <display.h>
#include <div64.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <panel.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/fb.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/system.h>
#include <asm/gpio.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	FIMD_RGB_INTERFACE = 1,
	FIMD_CPU_INTERFACE = 2,
};

enum exynos_fb_rgb_mode_t {
	MODE_RGB_P = 0,
	MODE_BGR_P = 1,
	MODE_RGB_S = 2,
	MODE_BGR_S = 3,
};

struct exynos_fb_priv {
	ushort vl_col;		/* Number of columns (i.e. 640) */
	ushort vl_row;		/* Number of rows (i.e. 480) */
	ushort vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	ushort vl_width;	/* Width of display area in millimeters */
	ushort vl_height;	/* Height of display area in millimeters */

	/* LCD configuration register */
	u_char vl_freq;		/* Frequency */
	u_char vl_clkp;		/* Clock polarity */
	u_char vl_oep;		/* Output Enable polarity */
	u_char vl_hsp;		/* Horizontal Sync polarity */
	u_char vl_vsp;		/* Vertical Sync polarity */
	u_char vl_dp;		/* Data polarity */
	u_char vl_bpix;		/* Bits per pixel */

	/* Horizontal control register. Timing from data sheet */
	u_char vl_hspw;		/* Horz sync pulse width */
	u_char vl_hfpd;		/* Wait before of line */
	u_char vl_hbpd;		/* Wait end of line */

	/* Vertical control register. */
	u_char	vl_vspw;	/* Vertical sync pulse width */
	u_char	vl_vfpd;	/* Wait before of frame */
	u_char	vl_vbpd;	/* Wait end of frame */
	u_char  vl_cmd_allow_len; /* Wait end of frame */

	unsigned int win_id;
	unsigned int init_delay;
	unsigned int power_on_delay;
	unsigned int reset_delay;
	unsigned int interface_mode;
	unsigned int mipi_enabled;
	unsigned int dp_enabled;
	unsigned int cs_setup;
	unsigned int wr_setup;
	unsigned int wr_act;
	unsigned int wr_hold;
	unsigned int logo_on;
	unsigned int logo_width;
	unsigned int logo_height;
	int logo_x_offset;
	int logo_y_offset;
	unsigned long logo_addr;
	unsigned int rgb_mode;
	unsigned int resolution;

	/* parent clock name(MPLL, EPLL or VPLL) */
	unsigned int pclk_name;
	/* ratio value for source clock from parent clock. */
	unsigned int sclk_div;

	unsigned int dual_lcd_enabled;
	struct exynos_fb *reg;
	struct exynos_platform_mipi_dsim *dsim_platform_data_dt;
};

static void exynos_fimd_set_dualrgb(struct exynos_fb_priv *priv, bool enabled)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	if (enabled) {
		cfg = EXYNOS_DUALRGB_BYPASS_DUAL | EXYNOS_DUALRGB_LINESPLIT |
			EXYNOS_DUALRGB_VDEN_EN_ENABLE;

		/* in case of Line Split mode, MAIN_CNT doesn't neet to set. */
		cfg |= EXYNOS_DUALRGB_SUB_CNT(priv->vl_col / 2) |
			EXYNOS_DUALRGB_MAIN_CNT(0);
	}

	writel(cfg, &reg->dualrgb);
}

static void exynos_fimd_set_dp_clkcon(struct exynos_fb_priv *priv,
				      unsigned int enabled)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	if (enabled)
		cfg = EXYNOS_DP_CLK_ENABLE;

	writel(cfg, &reg->dp_mie_clkcon);
}

static void exynos_fimd_set_par(struct exynos_fb_priv *priv,
				unsigned int win_id)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	/* set window control */
	cfg = readl((unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg &= ~(EXYNOS_WINCON_BITSWP_ENABLE | EXYNOS_WINCON_BYTESWP_ENABLE |
		EXYNOS_WINCON_HAWSWP_ENABLE | EXYNOS_WINCON_WSWP_ENABLE |
		EXYNOS_WINCON_BURSTLEN_MASK | EXYNOS_WINCON_BPPMODE_MASK |
		EXYNOS_WINCON_INRGB_MASK | EXYNOS_WINCON_DATAPATH_MASK);

	/* DATAPATH is DMA */
	cfg |= EXYNOS_WINCON_DATAPATH_DMA;

	cfg |= EXYNOS_WINCON_HAWSWP_ENABLE;

	/* dma burst is 16 */
	cfg |= EXYNOS_WINCON_BURSTLEN_16WORD;

	switch (priv->vl_bpix) {
	case 4:
		cfg |= EXYNOS_WINCON_BPPMODE_16BPP_565;
		break;
	default:
		cfg |= EXYNOS_WINCON_BPPMODE_24BPP_888;
		break;
	}

	writel(cfg, (unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));

	/* set window position to x=0, y=0*/
	cfg = EXYNOS_VIDOSD_LEFT_X(0) | EXYNOS_VIDOSD_TOP_Y(0);
	writel(cfg, (unsigned int)&reg->vidosd0a +
			EXYNOS_VIDOSD(win_id));

	cfg = EXYNOS_VIDOSD_RIGHT_X(priv->vl_col - 1) |
		EXYNOS_VIDOSD_BOTTOM_Y(priv->vl_row - 1) |
		EXYNOS_VIDOSD_RIGHT_X_E(1) |
		EXYNOS_VIDOSD_BOTTOM_Y_E(0);

	writel(cfg, (unsigned int)&reg->vidosd0b +
			EXYNOS_VIDOSD(win_id));

	/* set window size for window0*/
	cfg = EXYNOS_VIDOSD_SIZE(priv->vl_col * priv->vl_row);
	writel(cfg, (unsigned int)&reg->vidosd0c +
			EXYNOS_VIDOSD(win_id));
}

static void exynos_fimd_set_buffer_address(struct exynos_fb_priv *priv,
					   unsigned int win_id,
					   ulong lcd_base_addr)
{
	struct exynos_fb *reg = priv->reg;
	unsigned long start_addr, end_addr;

	start_addr = lcd_base_addr;
	end_addr = start_addr + ((priv->vl_col * (VNBITS(priv->vl_bpix) / 8)) *
				priv->vl_row);

	writel(start_addr, (unsigned int)&reg->vidw00add0b0 +
			EXYNOS_BUFFER_OFFSET(win_id));
	writel(end_addr, (unsigned int)&reg->vidw00add1b0 +
			EXYNOS_BUFFER_OFFSET(win_id));
}

static void exynos_fimd_set_clock(struct exynos_fb_priv *priv)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0, div = 0, remainder, remainder_div;
	unsigned long pixel_clock;
	unsigned long long src_clock;

	if (priv->dual_lcd_enabled) {
		pixel_clock = priv->vl_freq *
				(priv->vl_hspw + priv->vl_hfpd +
				 priv->vl_hbpd + priv->vl_col / 2) *
				(priv->vl_vspw + priv->vl_vfpd +
				 priv->vl_vbpd + priv->vl_row);
	} else if (priv->interface_mode == FIMD_CPU_INTERFACE) {
		pixel_clock = priv->vl_freq *
				priv->vl_width * priv->vl_height *
				(priv->cs_setup + priv->wr_setup +
				 priv->wr_act + priv->wr_hold + 1);
	} else {
		pixel_clock = priv->vl_freq *
				(priv->vl_hspw + priv->vl_hfpd +
				 priv->vl_hbpd + priv->vl_col) *
				(priv->vl_vspw + priv->vl_vfpd +
				 priv->vl_vbpd + priv->vl_row);
	}

	cfg = readl(&reg->vidcon0);
	cfg &= ~(EXYNOS_VIDCON0_CLKSEL_MASK | EXYNOS_VIDCON0_CLKVALUP_MASK |
		EXYNOS_VIDCON0_CLKVAL_F(0xFF) | EXYNOS_VIDCON0_VCLKEN_MASK |
		EXYNOS_VIDCON0_CLKDIR_MASK);
	cfg |= (EXYNOS_VIDCON0_CLKSEL_SCLK | EXYNOS_VIDCON0_CLKVALUP_ALWAYS |
		EXYNOS_VIDCON0_VCLKEN_NORMAL | EXYNOS_VIDCON0_CLKDIR_DIVIDED);

	src_clock = (unsigned long long) get_lcd_clk();

	/* get quotient and remainder. */
	remainder = do_div(src_clock, pixel_clock);
	div = src_clock;

	remainder *= 10;
	remainder_div = remainder / pixel_clock;

	/* round about one places of decimals. */
	if (remainder_div >= 5)
		div++;

	/* in case of dual lcd mode. */
	if (priv->dual_lcd_enabled)
		div--;

	cfg |= EXYNOS_VIDCON0_CLKVAL_F(div - 1);
	writel(cfg, &reg->vidcon0);
}

void exynos_set_trigger(struct exynos_fb_priv *priv)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	cfg = readl(&reg->trigcon);

	cfg |= (EXYNOS_I80SOFT_TRIG_EN | EXYNOS_I80START_TRIG);

	writel(cfg, &reg->trigcon);
}

int exynos_is_i80_frame_done(struct exynos_fb_priv *priv)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;
	int status;

	cfg = readl(&reg->trigcon);

	/* frame done func is valid only when TRIMODE[0] is set to 1. */
	status = (cfg & EXYNOS_I80STATUS_TRIG_DONE) ==
			EXYNOS_I80STATUS_TRIG_DONE;

	return status;
}

static void exynos_fimd_lcd_on(struct exynos_fb_priv *priv)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	/* display on */
	cfg = readl(&reg->vidcon0);
	cfg |= (EXYNOS_VIDCON0_ENVID_ENABLE | EXYNOS_VIDCON0_ENVID_F_ENABLE);
	writel(cfg, &reg->vidcon0);
}

static void exynos_fimd_window_on(struct exynos_fb_priv *priv,
				  unsigned int win_id)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	/* enable window */
	cfg = readl((unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));
	cfg |= EXYNOS_WINCON_ENWIN_ENABLE;
	writel(cfg, (unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg = readl(&reg->winshmap);
	cfg |= EXYNOS_WINSHMAP_CH_ENABLE(win_id);
	writel(cfg, &reg->winshmap);
}

void exynos_fimd_lcd_off(struct exynos_fb_priv *priv)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	cfg = readl(&reg->vidcon0);
	cfg &= (EXYNOS_VIDCON0_ENVID_DISABLE | EXYNOS_VIDCON0_ENVID_F_DISABLE);
	writel(cfg, &reg->vidcon0);
}

void exynos_fimd_window_off(struct exynos_fb_priv *priv, unsigned int win_id)
{
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0;

	cfg = readl((unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));
	cfg &= EXYNOS_WINCON_ENWIN_DISABLE;
	writel(cfg, (unsigned int)&reg->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg = readl(&reg->winshmap);
	cfg &= ~EXYNOS_WINSHMAP_CH_DISABLE(win_id);
	writel(cfg, &reg->winshmap);
}

/*
* The reset value for FIMD SYSMMU register MMU_CTRL is 3
* on Exynos5420 and newer versions.
* This means FIMD SYSMMU is on by default on Exynos5420
* and newer versions.
* Since in u-boot we don't use SYSMMU, we should disable
* those FIMD SYSMMU.
* Note that there are 2 SYSMMU for FIMD: m0 and m1.
* m0 handles windows 0 and 4, and m1 handles windows 1, 2 and 3.
* We disable both of them here.
*/
void exynos_fimd_disable_sysmmu(void)
{
	u32 *sysmmufimd;
	unsigned int node;
	int node_list[2];
	int count;
	int i;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "fimd",
				COMPAT_SAMSUNG_EXYNOS_SYSMMU, node_list, 2);
	for (i = 0; i < count; i++) {
		node = node_list[i];
		if (node <= 0) {
			debug("Can't get device node for fimd sysmmu\n");
			return;
		}

		sysmmufimd = (u32 *)fdtdec_get_addr(gd->fdt_blob, node, "reg");
		if (!sysmmufimd) {
			debug("Can't get base address for sysmmu fimdm0");
			return;
		}

		writel(0x0, sysmmufimd);
	}
}

void exynos_fimd_lcd_init(struct udevice *dev)
{
	struct exynos_fb_priv *priv = dev_get_priv(dev);
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct exynos_fb *reg = priv->reg;
	unsigned int cfg = 0, rgb_mode;
	unsigned int offset;
	unsigned int node;

	node = dev_of_offset(dev);
	if (fdtdec_get_bool(gd->fdt_blob, node, "samsung,disable-sysmmu"))
		exynos_fimd_disable_sysmmu();

	offset = exynos_fimd_get_base_offset();

	rgb_mode = priv->rgb_mode;

	if (priv->interface_mode == FIMD_RGB_INTERFACE) {
		cfg |= EXYNOS_VIDCON0_VIDOUT_RGB;
		writel(cfg, &reg->vidcon0);

		cfg = readl(&reg->vidcon2);
		cfg &= ~(EXYNOS_VIDCON2_WB_MASK |
			EXYNOS_VIDCON2_TVFORMATSEL_MASK |
			EXYNOS_VIDCON2_TVFORMATSEL_YUV_MASK);
		cfg |= EXYNOS_VIDCON2_WB_DISABLE;
		writel(cfg, &reg->vidcon2);

		/* set polarity */
		cfg = 0;
		if (!priv->vl_clkp)
			cfg |= EXYNOS_VIDCON1_IVCLK_RISING_EDGE;
		if (!priv->vl_hsp)
			cfg |= EXYNOS_VIDCON1_IHSYNC_INVERT;
		if (!priv->vl_vsp)
			cfg |= EXYNOS_VIDCON1_IVSYNC_INVERT;
		if (!priv->vl_dp)
			cfg |= EXYNOS_VIDCON1_IVDEN_INVERT;

		writel(cfg, (unsigned int)&reg->vidcon1 + offset);

		/* set timing */
		cfg = EXYNOS_VIDTCON0_VFPD(priv->vl_vfpd - 1);
		cfg |= EXYNOS_VIDTCON0_VBPD(priv->vl_vbpd - 1);
		cfg |= EXYNOS_VIDTCON0_VSPW(priv->vl_vspw - 1);
		writel(cfg, (unsigned int)&reg->vidtcon0 + offset);

		cfg = EXYNOS_VIDTCON1_HFPD(priv->vl_hfpd - 1);
		cfg |= EXYNOS_VIDTCON1_HBPD(priv->vl_hbpd - 1);
		cfg |= EXYNOS_VIDTCON1_HSPW(priv->vl_hspw - 1);

		writel(cfg, (unsigned int)&reg->vidtcon1 + offset);

		/* set lcd size */
		cfg = EXYNOS_VIDTCON2_HOZVAL(priv->vl_col - 1) |
			EXYNOS_VIDTCON2_LINEVAL(priv->vl_row - 1) |
			EXYNOS_VIDTCON2_HOZVAL_E(priv->vl_col - 1) |
			EXYNOS_VIDTCON2_LINEVAL_E(priv->vl_row - 1);

		writel(cfg, (unsigned int)&reg->vidtcon2 + offset);
	}

	/* set display mode */
	cfg = readl(&reg->vidcon0);
	cfg &= ~EXYNOS_VIDCON0_PNRMODE_MASK;
	cfg |= (rgb_mode << EXYNOS_VIDCON0_PNRMODE_SHIFT);
	writel(cfg, &reg->vidcon0);

	/* set par */
	exynos_fimd_set_par(priv, priv->win_id);

	/* set memory address */
	exynos_fimd_set_buffer_address(priv, priv->win_id, plat->base);

	/* set buffer size */
	cfg = EXYNOS_VIDADDR_PAGEWIDTH(priv->vl_col *
			VNBITS(priv->vl_bpix) / 8) |
		EXYNOS_VIDADDR_PAGEWIDTH_E(priv->vl_col *
			VNBITS(priv->vl_bpix) / 8) |
		EXYNOS_VIDADDR_OFFSIZE(0) |
		EXYNOS_VIDADDR_OFFSIZE_E(0);

	writel(cfg, (unsigned int)&reg->vidw00add2 +
					EXYNOS_BUFFER_SIZE(priv->win_id));

	/* set clock */
	exynos_fimd_set_clock(priv);

	/* set rgb mode to dual lcd. */
	exynos_fimd_set_dualrgb(priv, priv->dual_lcd_enabled);

	/* display on */
	exynos_fimd_lcd_on(priv);

	/* window on */
	exynos_fimd_window_on(priv, priv->win_id);

	exynos_fimd_set_dp_clkcon(priv, priv->dp_enabled);
}

unsigned long exynos_fimd_calc_fbsize(struct exynos_fb_priv *priv)
{
	return priv->vl_col * priv->vl_row * (VNBITS(priv->vl_bpix) / 8);
}

int exynos_fb_ofdata_to_platdata(struct udevice *dev)
{
	struct exynos_fb_priv *priv = dev_get_priv(dev);
	unsigned int node = dev_of_offset(dev);
	const void *blob = gd->fdt_blob;
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get the FIMD base address\n");
		return -EINVAL;
	}
	priv->reg = (struct exynos_fb *)addr;

	priv->vl_col = fdtdec_get_int(blob, node, "samsung,vl-col", 0);
	if (priv->vl_col == 0) {
		debug("Can't get XRES\n");
		return -ENXIO;
	}

	priv->vl_row = fdtdec_get_int(blob, node, "samsung,vl-row", 0);
	if (priv->vl_row == 0) {
		debug("Can't get YRES\n");
		return -ENXIO;
	}

	priv->vl_width = fdtdec_get_int(blob, node,
						"samsung,vl-width", 0);

	priv->vl_height = fdtdec_get_int(blob, node,
						"samsung,vl-height", 0);

	priv->vl_freq = fdtdec_get_int(blob, node, "samsung,vl-freq", 0);
	if (priv->vl_freq == 0) {
		debug("Can't get refresh rate\n");
		return -ENXIO;
	}

	if (fdtdec_get_bool(blob, node, "samsung,vl-clkp"))
		priv->vl_clkp = VIDEO_ACTIVE_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-oep"))
		priv->vl_oep = VIDEO_ACTIVE_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-hsp"))
		priv->vl_hsp = VIDEO_ACTIVE_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-vsp"))
		priv->vl_vsp = VIDEO_ACTIVE_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-dp"))
		priv->vl_dp = VIDEO_ACTIVE_LOW;

	priv->vl_bpix = fdtdec_get_int(blob, node, "samsung,vl-bpix", 0);
	if (priv->vl_bpix == 0) {
		debug("Can't get bits per pixel\n");
		return -ENXIO;
	}

	priv->vl_hspw = fdtdec_get_int(blob, node, "samsung,vl-hspw", 0);
	if (priv->vl_hspw == 0) {
		debug("Can't get hsync width\n");
		return -ENXIO;
	}

	priv->vl_hfpd = fdtdec_get_int(blob, node, "samsung,vl-hfpd", 0);
	if (priv->vl_hfpd == 0) {
		debug("Can't get right margin\n");
		return -ENXIO;
	}

	priv->vl_hbpd = (u_char)fdtdec_get_int(blob, node,
							"samsung,vl-hbpd", 0);
	if (priv->vl_hbpd == 0) {
		debug("Can't get left margin\n");
		return -ENXIO;
	}

	priv->vl_vspw = (u_char)fdtdec_get_int(blob, node,
							"samsung,vl-vspw", 0);
	if (priv->vl_vspw == 0) {
		debug("Can't get vsync width\n");
		return -ENXIO;
	}

	priv->vl_vfpd = fdtdec_get_int(blob, node,
							"samsung,vl-vfpd", 0);
	if (priv->vl_vfpd == 0) {
		debug("Can't get lower margin\n");
		return -ENXIO;
	}

	priv->vl_vbpd = fdtdec_get_int(blob, node, "samsung,vl-vbpd", 0);
	if (priv->vl_vbpd == 0) {
		debug("Can't get upper margin\n");
		return -ENXIO;
	}

	priv->vl_cmd_allow_len = fdtdec_get_int(blob, node,
						"samsung,vl-cmd-allow-len", 0);

	priv->win_id = fdtdec_get_int(blob, node, "samsung,winid", 0);
	priv->init_delay = fdtdec_get_int(blob, node,
						"samsung,init-delay", 0);
	priv->power_on_delay = fdtdec_get_int(blob, node,
						"samsung,power-on-delay", 0);
	priv->reset_delay = fdtdec_get_int(blob, node,
						"samsung,reset-delay", 0);
	priv->interface_mode = fdtdec_get_int(blob, node,
						"samsung,interface-mode", 0);
	priv->mipi_enabled = fdtdec_get_int(blob, node,
						"samsung,mipi-enabled", 0);
	priv->dp_enabled = fdtdec_get_int(blob, node,
						"samsung,dp-enabled", 0);
	priv->cs_setup = fdtdec_get_int(blob, node,
						"samsung,cs-setup", 0);
	priv->wr_setup = fdtdec_get_int(blob, node,
						"samsung,wr-setup", 0);
	priv->wr_act = fdtdec_get_int(blob, node, "samsung,wr-act", 0);
	priv->wr_hold = fdtdec_get_int(blob, node, "samsung,wr-hold", 0);

	priv->logo_on = fdtdec_get_int(blob, node, "samsung,logo-on", 0);
	if (priv->logo_on) {
		priv->logo_width = fdtdec_get_int(blob, node,
						"samsung,logo-width", 0);
		priv->logo_height = fdtdec_get_int(blob, node,
						"samsung,logo-height", 0);
		priv->logo_addr = fdtdec_get_int(blob, node,
						"samsung,logo-addr", 0);
	}

	priv->rgb_mode = fdtdec_get_int(blob, node,
						"samsung,rgb-mode", 0);
	priv->pclk_name = fdtdec_get_int(blob, node,
						"samsung,pclk-name", 0);
	priv->sclk_div = fdtdec_get_int(blob, node,
						"samsung,sclk-div", 0);
	priv->dual_lcd_enabled = fdtdec_get_int(blob, node,
						"samsung,dual-lcd-enabled", 0);

	return 0;
}

static int exynos_fb_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct exynos_fb_priv *priv = dev_get_priv(dev);
	struct udevice *panel, *bridge;
	struct udevice *dp;
	int ret;

	debug("%s: start\n", __func__);
	set_system_display_ctrl();
	set_lcd_clk();

#ifdef CONFIG_EXYNOS_MIPI_DSIM
	exynos_init_dsim_platform_data(&panel_info);
#endif
	exynos_fimd_lcd_init(dev);

	ret = uclass_first_device(UCLASS_PANEL, &panel);
	if (ret) {
		printf("LCD panel failed to probe\n");
		return ret;
	}
	if (!panel) {
		printf("LCD panel not found\n");
		return -ENODEV;
	}

	ret = uclass_first_device(UCLASS_DISPLAY, &dp);
	if (ret) {
		debug("%s: Display device error %d\n", __func__, ret);
		return ret;
	}
	if (!dev) {
		debug("%s: Display device missing\n", __func__);
		return -ENODEV;
	}
	ret = display_enable(dp, 18, NULL);
	if (ret) {
		debug("%s: Display enable error %d\n", __func__, ret);
		return ret;
	}

	/* backlight / pwm */
	ret = panel_enable_backlight(panel);
	if (ret) {
		debug("%s: backlight error: %d\n", __func__, ret);
		return ret;
	}

	ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &bridge);
	if (!ret)
		ret = video_bridge_set_backlight(bridge, 80);
	if (ret) {
		debug("%s: No video bridge, or no backlight on bridge\n",
		      __func__);
		exynos_pinmux_config(PERIPH_ID_PWM0, 0);
	}

	uc_priv->xsize = priv->vl_col;
	uc_priv->ysize = priv->vl_row;
	uc_priv->bpix = priv->vl_bpix;

	/* Enable flushing after LCD writes if requested */
	video_set_flush_dcache(dev, true);

	return 0;
}

static int exynos_fb_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);

	/* This is the maximum panel size we expect to see */
	plat->size = 1920 * 1080 * 2;

	return 0;
}

static const struct video_ops exynos_fb_ops = {
};

static const struct udevice_id exynos_fb_ids[] = {
	{ .compatible = "samsung,exynos-fimd" },
	{ }
};

U_BOOT_DRIVER(exynos_fb) = {
	.name	= "exynos_fb",
	.id	= UCLASS_VIDEO,
	.of_match = exynos_fb_ids,
	.ops	= &exynos_fb_ops,
	.bind	= exynos_fb_bind,
	.probe	= exynos_fb_probe,
	.ofdata_to_platdata	= exynos_fb_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct exynos_fb_priv),
};
