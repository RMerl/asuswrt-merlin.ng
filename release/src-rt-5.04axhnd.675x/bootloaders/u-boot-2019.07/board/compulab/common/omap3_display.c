// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 - 2013 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *
 * Parsing code based on linux/drivers/video/pxafb.c
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <asm/arch/dss.h>
#include <lcd.h>
#include <scf0403_lcd.h>
#include <asm/arch-omap3/dss.h>

enum display_type {
	NONE,
	DVI,
	DVI_CUSTOM,
	DATA_IMAGE, /* #define CONFIG_SCF0403_LCD to use */
};

#define CMAP_ADDR	0x80100000

/*
 * The frame buffer is allocated before we have the chance to parse user input.
 * To make sure enough memory is allocated for all resolutions, we define
 * vl_{col | row} to the maximal resolution supported by OMAP3.
 */
vidinfo_t panel_info = {
	.vl_col  = 1400,
	.vl_row  = 1050,
	.vl_bpix = LCD_BPP,
	.cmap = (ushort *)CMAP_ADDR,
};

static struct panel_config panel_cfg;
static enum display_type lcd_def;

/*
 * A note on DVI presets;
 * U-Boot can convert 8 bit BMP data to 16 bit BMP data, and OMAP DSS can
 * convert 16 bit data into 24 bit data. Thus, GFXFORMAT_RGB16 allows us to
 * support two BMP types with one setting.
 */
static const struct panel_config preset_dvi_640X480 = {
	.lcd_size	= PANEL_LCD_SIZE(640, 480),
	.timing_h	= DSS_HBP(48) | DSS_HFP(16) | DSS_HSW(96),
	.timing_v	= DSS_VBP(33) | DSS_VFP(10) | DSS_VSW(2),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 12 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_800X600 = {
	.lcd_size	= PANEL_LCD_SIZE(800, 600),
	.timing_h	= DSS_HBP(88) | DSS_HFP(40) | DSS_HSW(128),
	.timing_v	= DSS_VBP(23) | DSS_VFP(1) | DSS_VSW(4),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 8 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1024X768 = {
	.lcd_size	= PANEL_LCD_SIZE(1024, 768),
	.timing_h	= DSS_HBP(160) | DSS_HFP(24) | DSS_HSW(136),
	.timing_v	= DSS_VBP(29) | DSS_VFP(3) | DSS_VSW(6),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 5 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1152X864 = {
	.lcd_size	= PANEL_LCD_SIZE(1152, 864),
	.timing_h	= DSS_HBP(256) | DSS_HFP(64) | DSS_HSW(128),
	.timing_v	= DSS_VBP(32) | DSS_VFP(1) | DSS_VSW(3),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 4 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1280X960 = {
	.lcd_size	= PANEL_LCD_SIZE(1280, 960),
	.timing_h	= DSS_HBP(312) | DSS_HFP(96) | DSS_HSW(112),
	.timing_v	= DSS_VBP(36) | DSS_VFP(1) | DSS_VSW(3),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 3 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1280X1024 = {
	.lcd_size	= PANEL_LCD_SIZE(1280, 1024),
	.timing_h	= DSS_HBP(248) | DSS_HFP(48) | DSS_HSW(112),
	.timing_v	= DSS_VBP(38) | DSS_VFP(1) | DSS_VSW(3),
	.pol_freq	= DSS_IHS | DSS_IVS | DSS_IPC,
	.divisor	= 3 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dataimage_480X800 = {
	.lcd_size	= PANEL_LCD_SIZE(480, 800),
	.timing_h	= DSS_HBP(2) | DSS_HFP(2) | DSS_HSW(2),
	.timing_v	= DSS_VBP(17) | DSS_VFP(20) | DSS_VSW(3),
	.pol_freq	= DSS_IVS | DSS_IHS | DSS_IPC | DSS_ONOFF,
	.divisor	= 10 | (1 << 10),
	.data_lines	= LCD_INTERFACE_18_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

/*
 * set_resolution_params()
 *
 * Due to usage of multiple display related APIs resolution data is located in
 * more than one place. This function updates them all.
 */
static void set_resolution_params(int x, int y)
{
	panel_cfg.lcd_size = PANEL_LCD_SIZE(x, y);
	panel_info.vl_col = x;
	panel_info.vl_row = y;
	lcd_line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;
}

static void set_preset(const struct panel_config preset, int x_res, int y_res)
{
	panel_cfg = preset;
	set_resolution_params(x_res, y_res);
}

static enum display_type set_dvi_preset(const struct panel_config preset,
					int x_res, int y_res)
{
	set_preset(preset, x_res, y_res);
	return DVI;
}

static enum display_type set_dataimage_preset(const struct panel_config preset,
		int x_res, int y_res)
{
	set_preset(preset, x_res, y_res);
	return DATA_IMAGE;
}

/*
 * parse_mode() - parse the mode parameter of custom lcd settings
 *
 * @mode:	<res_x>x<res_y>
 *
 * Returns -1 on error, 0 on success.
 */
static int parse_mode(const char *mode)
{
	unsigned int modelen = strlen(mode);
	int res_specified = 0;
	unsigned int xres = 0, yres = 0;
	int yres_specified = 0;
	int i;

	for (i = modelen - 1; i >= 0; i--) {
		switch (mode[i]) {
		case 'x':
			if (!yres_specified) {
				yres = simple_strtoul(&mode[i + 1], NULL, 0);
				yres_specified = 1;
			} else {
				goto done_parsing;
			}

			break;
		case '0' ... '9':
			break;
		default:
			goto done_parsing;
		}
	}

	if (i < 0 && yres_specified) {
		xres = simple_strtoul(mode, NULL, 0);
		res_specified = 1;
	}

done_parsing:
	if (res_specified) {
		set_resolution_params(xres, yres);
	} else {
		printf("LCD: invalid mode: %s\n", mode);
		return -1;
	}

	return 0;
}

#define PIXEL_CLK_NUMERATOR (26 * 432 / 39)
/*
 * parse_pixclock() - Parse the pixclock parameter of custom lcd settings
 *
 * @pixclock:	the desired pixel clock
 *
 * Returns -1 on error, 0 on success.
 *
 * Handling the pixel_clock:
 *
 * Pixel clock is defined in the OMAP35x TRM as follows:
 * pixel_clock =
 * (SYS_CLK * 2 * PRCM.CM_CLKSEL2_PLL[18:8]) /
 * (DSS.DISPC_DIVISOR[23:16] * DSS.DISPC_DIVISOR[6:0] *
 * PRCM.CM_CLKSEL_DSS[4:0] * (PRCM.CM_CLKSEL2_PLL[6:0] + 1))
 *
 * In practice, this means that in order to set the
 * divisor for the desired pixel clock one needs to
 * solve the following equation:
 *
 * 26 * 432 / (39 * <pixel_clock>) = DSS.DISPC_DIVISOR[6:0]
 *
 * NOTE: the explicit equation above is reduced. Do not
 * try to infer anything from these numbers.
 */
static int parse_pixclock(char *pixclock)
{
	int divisor, pixclock_val;
	char *pixclk_start = pixclock;

	pixclock_val = simple_strtoul(pixclock, &pixclock, 10);
	divisor = DIV_ROUND_UP(PIXEL_CLK_NUMERATOR, pixclock_val);
	/* 0 and 1 are illegal values for PCD */
	if (divisor <= 1)
		divisor = 2;

	panel_cfg.divisor = divisor | (1 << 16);
	if (pixclock[0] != '\0') {
		printf("LCD: invalid value for pixclock:%s\n", pixclk_start);
		return -1;
	}

	return 0;
}

/*
 * parse_setting() - parse a single setting of custom lcd parameters
 *
 * @setting:	The custom lcd setting <name>:<value>
 *
 * Returns -1 on failure, 0 on success.
 */
static int parse_setting(char *setting)
{
	int num_val;
	char *setting_start = setting;

	if (!strncmp(setting, "mode:", 5)) {
		return parse_mode(setting + 5);
	} else if (!strncmp(setting, "pixclock:", 9)) {
		return parse_pixclock(setting + 9);
	} else if (!strncmp(setting, "left:", 5)) {
		num_val = simple_strtoul(setting + 5, &setting, 0);
		panel_cfg.timing_h |= DSS_HBP(num_val);
	} else if (!strncmp(setting, "right:", 6)) {
		num_val = simple_strtoul(setting + 6, &setting, 0);
		panel_cfg.timing_h |= DSS_HFP(num_val);
	} else if (!strncmp(setting, "upper:", 6)) {
		num_val = simple_strtoul(setting + 6, &setting, 0);
		panel_cfg.timing_v |= DSS_VBP(num_val);
	} else if (!strncmp(setting, "lower:", 6)) {
		num_val = simple_strtoul(setting + 6, &setting, 0);
		panel_cfg.timing_v |= DSS_VFP(num_val);
	} else if (!strncmp(setting, "hsynclen:", 9)) {
		num_val = simple_strtoul(setting + 9, &setting, 0);
		panel_cfg.timing_h |= DSS_HSW(num_val);
	} else if (!strncmp(setting, "vsynclen:", 9)) {
		num_val = simple_strtoul(setting + 9, &setting, 0);
		panel_cfg.timing_v |= DSS_VSW(num_val);
	} else if (!strncmp(setting, "hsync:", 6)) {
		if (simple_strtoul(setting + 6, &setting, 0) == 0)
			panel_cfg.pol_freq |= DSS_IHS;
		else
			panel_cfg.pol_freq &= ~DSS_IHS;
	} else if (!strncmp(setting, "vsync:", 6)) {
		if (simple_strtoul(setting + 6, &setting, 0) == 0)
			panel_cfg.pol_freq |= DSS_IVS;
		else
			panel_cfg.pol_freq &= ~DSS_IVS;
	} else if (!strncmp(setting, "outputen:", 9)) {
		if (simple_strtoul(setting + 9, &setting, 0) == 0)
			panel_cfg.pol_freq |= DSS_IEO;
		else
			panel_cfg.pol_freq &= ~DSS_IEO;
	} else if (!strncmp(setting, "pixclockpol:", 12)) {
		if (simple_strtoul(setting + 12, &setting, 0) == 0)
			panel_cfg.pol_freq |= DSS_IPC;
		else
			panel_cfg.pol_freq &= ~DSS_IPC;
	} else if (!strncmp(setting, "active", 6)) {
		panel_cfg.panel_type = ACTIVE_DISPLAY;
		return 0; /* Avoid sanity check below */
	} else if (!strncmp(setting, "passive", 7)) {
		panel_cfg.panel_type = PASSIVE_DISPLAY;
		return 0; /* Avoid sanity check below */
	} else if (!strncmp(setting, "display:", 8)) {
		if (!strncmp(setting + 8, "dvi", 3)) {
			lcd_def = DVI_CUSTOM;
			return 0; /* Avoid sanity check below */
		}
	} else {
		printf("LCD: unknown option %s\n", setting_start);
		return -1;
	}

	if (setting[0] != '\0') {
		printf("LCD: invalid value for %s\n", setting_start);
		return -1;
	}

	return 0;
}

/*
 * env_parse_customlcd() - parse custom lcd params from an environment variable.
 *
 * @custom_lcd_params:	The environment variable containing the lcd params.
 *
 * Returns -1 on failure, 0 on success.
 */
static int parse_customlcd(char *custom_lcd_params)
{
	char params_cpy[160];
	char *setting;

	strncpy(params_cpy, custom_lcd_params, 160);
	setting = strtok(params_cpy, ",");
	while (setting) {
		if (parse_setting(setting) < 0)
			return -1;

		setting = strtok(NULL, ",");
	}

	/* Currently we don't support changing this via custom lcd params */
	panel_cfg.data_lines = LCD_INTERFACE_24_BIT;
	panel_cfg.gfx_format = GFXFORMAT_RGB16; /* See dvi predefines note */

	return 0;
}

/*
 * env_parse_displaytype() - parse display type.
 *
 * Parses the environment variable "displaytype", which contains the
 * name of the display type or preset, in which case it applies its
 * configurations.
 *
 * Returns the type of display that was specified.
 */
static enum display_type env_parse_displaytype(char *displaytype)
{
	if (!strncmp(displaytype, "dvi640x480", 10))
		return set_dvi_preset(preset_dvi_640X480, 640, 480);
	else if (!strncmp(displaytype, "dvi800x600", 10))
		return set_dvi_preset(preset_dvi_800X600, 800, 600);
	else if (!strncmp(displaytype, "dvi1024x768", 11))
		return set_dvi_preset(preset_dvi_1024X768, 1024, 768);
	else if (!strncmp(displaytype, "dvi1152x864", 11))
		return set_dvi_preset(preset_dvi_1152X864, 1152, 864);
	else if (!strncmp(displaytype, "dvi1280x960", 11))
		return set_dvi_preset(preset_dvi_1280X960, 1280, 960);
	else if (!strncmp(displaytype, "dvi1280x1024", 12))
		return set_dvi_preset(preset_dvi_1280X1024, 1280, 1024);
	else if (!strncmp(displaytype, "dataimage480x800", 16))
		return set_dataimage_preset(preset_dataimage_480X800, 480, 800);

	return NONE;
}

void lcd_ctrl_init(void *lcdbase)
{
	struct prcm *prcm = (struct prcm *)PRCM_BASE;
	char *custom_lcd;
	char *displaytype = env_get("displaytype");

	if (displaytype == NULL)
		return;

	lcd_def = env_parse_displaytype(displaytype);
	/* If we did not recognize the preset, check if it's an env variable */
	if (lcd_def == NONE) {
		custom_lcd = env_get(displaytype);
		if (custom_lcd == NULL || parse_customlcd(custom_lcd) < 0)
			return;
	}

	panel_cfg.frame_buffer = lcdbase;
	omap3_dss_panel_config(&panel_cfg);
	/*
	 * Pixel clock is defined with many divisions and only few
	 * multiplications of the system clock. Since DSS FCLK divisor is set
	 * to 16 by default, we need to set it to a smaller value, like 3
	 * (chosen via trial and error).
	 */
	clrsetbits_le32(&prcm->clksel_dss, 0xF, 3);
}

#ifdef CONFIG_SCF0403_LCD
static void scf0403_enable(void)
{
	gpio_direction_output(58, 1);
	scf0403_init(157);
}
#else
static inline void scf0403_enable(void) {}
#endif

void lcd_enable(void)
{
	switch (lcd_def) {
	case NONE:
		return;
	case DVI:
	case DVI_CUSTOM:
		gpio_direction_output(54, 0); /* Turn on DVI */
		break;
	case DATA_IMAGE:
		scf0403_enable();
		break;
	}

	omap3_dss_enable();
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue) {}
