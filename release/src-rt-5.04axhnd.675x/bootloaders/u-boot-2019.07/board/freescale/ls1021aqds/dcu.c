// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * FSL DCU Framebuffer driver
 */

#include <asm/io.h>
#include <common.h>
#include <fsl_dcu_fb.h>
#include <i2c.h>
#include "div64.h"
#include "../common/diu_ch7301.h"
#include "ls1021aqds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

static int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

unsigned int dcu_set_pixel_clock(unsigned int pixclock)
{
	unsigned long long div;

	div = (unsigned long long)(gd->bus_clk / 1000);
	div *= (unsigned long long)pixclock;
	do_div(div, 1000000000);

	return div;
}

int platform_dcu_init(unsigned int xres, unsigned int yres,
		      const char *port,
		      struct fb_videomode *dcu_fb_videomode)
{
	const char *name;
	unsigned int pixel_format;
	int ret;
	u8 ch;

	/* Mux I2C3+I2C4 as HSYNC+VSYNC */
	ret = i2c_read(CONFIG_SYS_I2C_QIXIS_ADDR, QIXIS_DCU_BRDCFG5,
		       1, &ch, 1);
	if (ret) {
		printf("Error: failed to read I2C @%02x\n",
		       CONFIG_SYS_I2C_QIXIS_ADDR);
		return ret;
	}
	ch &= 0x1F;
	ch |= 0xA0;
	ret = i2c_write(CONFIG_SYS_I2C_QIXIS_ADDR, QIXIS_DCU_BRDCFG5,
			1, &ch, 1);
	if (ret) {
		printf("Error: failed to write I2C @%02x\n",
		       CONFIG_SYS_I2C_QIXIS_ADDR);
		return ret;
	}

	if (strncmp(port, "hdmi", 4) == 0) {
		unsigned long pixval;

		name = "HDMI";

		pixval = 1000000000 / dcu_fb_videomode->pixclock;
		pixval *= 1000;

		i2c_set_bus_num(CONFIG_SYS_I2C_DVI_BUS_NUM);
		select_i2c_ch_pca9547(I2C_MUX_CH_CH7301);
		diu_set_dvi_encoder(pixval);
		select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	} else {
		return 0;
	}

	printf("DCU: Switching to %s monitor @ %ux%u\n", name, xres, yres);

	pixel_format = 32;
	fsl_dcu_init(xres, yres, pixel_format);

	return 0;
}
