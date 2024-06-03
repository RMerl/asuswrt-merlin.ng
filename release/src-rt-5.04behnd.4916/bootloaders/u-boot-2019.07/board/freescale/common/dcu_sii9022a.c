// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <asm/io.h>
#include <common.h>
#include <fsl_dcu_fb.h>
#include <i2c.h>
#include <linux/fb.h>

#define PIXEL_CLK_LSB_REG		0x00
#define PIXEL_CLK_MSB_REG		0x01
#define VERT_FREQ_LSB_REG		0x02
#define VERT_FREQ_MSB_REG		0x03
#define TOTAL_PIXELS_LSB_REG		0x04
#define TOTAL_PIXELS_MSB_REG		0x05
#define TOTAL_LINES_LSB_REG		0x06
#define TOTAL_LINES_MSB_REG		0x07
#define TPI_INBUS_FMT_REG		0x08
#define TPI_INPUT_FMT_REG		0x09
#define TPI_OUTPUT_FMT_REG		0x0A
#define TPI_SYS_CTRL_REG		0x1A
#define TPI_PWR_STAT_REG		0x1E
#define TPI_AUDIO_HANDING_REG		0x25
#define TPI_AUDIO_INTF_REG		0x26
#define TPI_AUDIO_FREQ_REG		0x27
#define TPI_SET_PAGE_REG		0xBC
#define TPI_SET_OFFSET_REG		0xBD
#define TPI_RW_ACCESS_REG		0xBE
#define TPI_TRANS_MODE_REG		0xC7

#define TPI_INBUS_CLOCK_RATIO_1		(1 << 6)
#define TPI_INBUS_FULL_PIXEL_WIDE	(1 << 5)
#define TPI_INBUS_RISING_EDGE		(1 << 4)
#define TPI_INPUT_CLR_DEPTH_8BIT	(0 << 6)
#define TPI_INPUT_VRANGE_EXPAN_AUTO	(0 << 2)
#define TPI_INPUT_CLR_RGB		(0 << 0)
#define TPI_OUTPUT_CLR_DEPTH_8BIT	(0 << 6)
#define TPI_OUTPUT_VRANGE_COMPRE_AUTO	(0 << 2)
#define TPI_OUTPUT_CLR_HDMI_RGB		(0 << 0)
#define TPI_SYS_TMDS_OUTPUT		(0 << 4)
#define TPI_SYS_AV_NORAML		(0 << 3)
#define TPI_SYS_AV_MUTE			(1 << 3)
#define TPI_SYS_DVI_MODE		(0 << 0)
#define TPI_SYS_HDMI_MODE		(1 << 0)
#define TPI_PWR_STAT_MASK		(3 << 0)
#define TPI_PWR_STAT_D0			(0 << 0)
#define TPI_AUDIO_PASS_BASIC		(0 << 0)
#define TPI_AUDIO_INTF_I2S		(2 << 6)
#define TPI_AUDIO_INTF_NORMAL		(0 << 4)
#define TPI_AUDIO_TYPE_PCM		(1 << 0)
#define TPI_AUDIO_SAMP_SIZE_16BIT	(1 << 6)
#define TPI_AUDIO_SAMP_FREQ_44K		(2 << 3)
#define TPI_SET_PAGE_SII9022A		0x01
#define TPI_SET_OFFSET_SII9022A		0x82
#define TPI_RW_EN_SRC_TERMIN		(1 << 0)
#define TPI_TRANS_MODE_ENABLE		(0 << 7)

/* Programming of Silicon SIi9022a HDMI Transmitter */
int dcu_set_dvi_encoder(struct fb_videomode *videomode)
{
	u8 temp;
	u16 temp1, temp2;
	u32 temp3;

	i2c_set_bus_num(CONFIG_SYS_I2C_DVI_BUS_NUM);

	/* Enable TPI transmitter mode */
	temp = TPI_TRANS_MODE_ENABLE;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_TRANS_MODE_REG, 1, &temp, 1);

	/* Enter into D0 state, full operation */
	i2c_read(CONFIG_SYS_I2C_DVI_ADDR, TPI_PWR_STAT_REG, 1, &temp, 1);
	temp &= ~TPI_PWR_STAT_MASK;
	temp |= TPI_PWR_STAT_D0;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_PWR_STAT_REG, 1, &temp, 1);

	/* Enable source termination */
	temp = TPI_SET_PAGE_SII9022A;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_SET_PAGE_REG, 1, &temp, 1);
	temp = TPI_SET_OFFSET_SII9022A;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_SET_OFFSET_REG, 1, &temp, 1);

	i2c_read(CONFIG_SYS_I2C_DVI_ADDR, TPI_RW_ACCESS_REG, 1, &temp, 1);
	temp |= TPI_RW_EN_SRC_TERMIN;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_RW_ACCESS_REG, 1, &temp, 1);

	/* Set TPI system control */
	temp = TPI_SYS_TMDS_OUTPUT | TPI_SYS_AV_NORAML | TPI_SYS_DVI_MODE;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_SYS_CTRL_REG, 1, &temp, 1);

	/* Set pixel clock */
	temp1 = PICOS2KHZ(videomode->pixclock) / 10;
	temp = (u8)(temp1 & 0xFF);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, PIXEL_CLK_LSB_REG, 1, &temp, 1);
	temp = (u8)(temp1 >> 8);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, PIXEL_CLK_MSB_REG, 1, &temp, 1);

	/* Set total pixels per line */
	temp1 = videomode->hsync_len + videomode->left_margin +
		videomode->xres + videomode->right_margin;
	temp = (u8)(temp1 & 0xFF);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TOTAL_PIXELS_LSB_REG, 1, &temp, 1);
	temp = (u8)(temp1 >> 8);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TOTAL_PIXELS_MSB_REG, 1, &temp, 1);

	/* Set total lines */
	temp2 = videomode->vsync_len + videomode->upper_margin +
		videomode->yres + videomode->lower_margin;
	temp = (u8)(temp2 & 0xFF);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TOTAL_LINES_LSB_REG, 1, &temp, 1);
	temp = (u8)(temp2 >> 8);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TOTAL_LINES_MSB_REG, 1, &temp, 1);

	/* Set vertical frequency in Hz */
	temp3 = temp1 * temp2;
	temp3 = (PICOS2KHZ(videomode->pixclock) * 1000) / temp3;
	temp1 = (u16)temp3 * 100;
	temp = (u8)(temp1 & 0xFF);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, VERT_FREQ_LSB_REG, 1, &temp, 1);
	temp = (u8)(temp1 >> 8);
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, VERT_FREQ_MSB_REG, 1, &temp, 1);

	/* Set TPI input bus and pixel repetition data */
	temp = TPI_INBUS_CLOCK_RATIO_1 | TPI_INBUS_FULL_PIXEL_WIDE |
		TPI_INBUS_RISING_EDGE;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_INBUS_FMT_REG, 1, &temp, 1);

	/* Set TPI AVI Input format data */
	temp = TPI_INPUT_CLR_DEPTH_8BIT | TPI_INPUT_VRANGE_EXPAN_AUTO |
		TPI_INPUT_CLR_RGB;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_INPUT_FMT_REG, 1, &temp, 1);

	/* Set TPI AVI Output format data */
	temp = TPI_OUTPUT_CLR_DEPTH_8BIT | TPI_OUTPUT_VRANGE_COMPRE_AUTO |
		TPI_OUTPUT_CLR_HDMI_RGB;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_OUTPUT_FMT_REG, 1, &temp, 1);

	/* Set TPI audio configuration write data */
	temp = TPI_AUDIO_PASS_BASIC;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_AUDIO_HANDING_REG, 1, &temp, 1);

	temp = TPI_AUDIO_INTF_I2S | TPI_AUDIO_INTF_NORMAL |
		TPI_AUDIO_TYPE_PCM;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_AUDIO_INTF_REG, 1, &temp, 1);

	temp = TPI_AUDIO_SAMP_SIZE_16BIT | TPI_AUDIO_SAMP_FREQ_44K;
	i2c_write(CONFIG_SYS_I2C_DVI_ADDR, TPI_AUDIO_FREQ_REG, 1, &temp, 1);

	return 0;
}
