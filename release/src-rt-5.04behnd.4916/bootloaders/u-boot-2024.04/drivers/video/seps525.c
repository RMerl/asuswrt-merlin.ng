// SPDX-License-Identifier: GPL-2.0
/*
 * FB driver for the WiseChip Semiconductor Inc. (UG-6028GDEBF02) display
 * using the SEPS525 (Syncoam) LCD Controller
 *
 * Copyright (C) 2020 Xilinx Inc.
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>
#include <video.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/delay.h>

#define WIDTH		160
#define HEIGHT		128

#define SEPS525_INDEX			0x00
#define SEPS525_STATUS_RD		0x01
#define SEPS525_OSC_CTL			0x02
#define SEPS525_IREF			0x80
#define SEPS525_CLOCK_DIV		0x03
#define SEPS525_REDUCE_CURRENT		0x04
#define SEPS525_SOFT_RST		0x05
#define SEPS525_DISP_ONOFF		0x06
#define SEPS525_PRECHARGE_TIME_R	0x08
#define SEPS525_PRECHARGE_TIME_G	0x09
#define SEPS525_PRECHARGE_TIME_B	0x0A
#define SEPS525_PRECHARGE_CURRENT_R	0x0B
#define SEPS525_PRECHARGE_CURRENT_G	0x0C
#define SEPS525_PRECHARGE_CURRENT_B	0x0D
#define SEPS525_DRIVING_CURRENT_R	0x10
#define SEPS525_DRIVING_CURRENT_G	0x11
#define SEPS525_DRIVING_CURRENT_B	0x12
#define SEPS525_DISPLAYMODE_SET		0x13
#define SEPS525_RGBIF			0x14
#define SEPS525_RGB_POL			0x15
#define SEPS525_MEMORY_WRITEMODE	0x16
#define SEPS525_MX1_ADDR		0x17
#define SEPS525_MX2_ADDR		0x18
#define SEPS525_MY1_ADDR		0x19
#define SEPS525_MY2_ADDR		0x1A
#define SEPS525_MEMORY_ACCESS_POINTER_X	0x20
#define SEPS525_MEMORY_ACCESS_POINTER_Y	0x21
#define SEPS525_DDRAM_DATA_ACCESS_PORT	0x22
#define SEPS525_GRAY_SCALE_TABLE_INDEX	0x50
#define SEPS525_GRAY_SCALE_TABLE_DATA	0x51
#define SEPS525_DUTY			0x28
#define SEPS525_DSL			0x29
#define SEPS525_D1_DDRAM_FAC		0x2E
#define SEPS525_D1_DDRAM_FAR		0x2F
#define SEPS525_D2_DDRAM_SAC		0x31
#define SEPS525_D2_DDRAM_SAR		0x32
#define SEPS525_SCR1_FX1		0x33
#define SEPS525_SCR1_FX2		0x34
#define SEPS525_SCR1_FY1		0x35
#define SEPS525_SCR1_FY2		0x36
#define SEPS525_SCR2_SX1		0x37
#define SEPS525_SCR2_SX2		0x38
#define SEPS525_SCR2_SY1		0x39
#define SEPS525_SCR2_SY2		0x3A
#define SEPS525_SCREEN_SAVER_CONTEROL	0x3B
#define SEPS525_SS_SLEEP_TIMER		0x3C
#define SEPS525_SCREEN_SAVER_MODE	0x3D
#define SEPS525_SS_SCR1_FU		0x3E
#define SEPS525_SS_SCR1_MXY		0x3F
#define SEPS525_SS_SCR2_FU		0x40
#define SEPS525_SS_SCR2_MXY		0x41
#define SEPS525_MOVING_DIRECTION	0x42
#define SEPS525_SS_SCR2_SX1		0x47
#define SEPS525_SS_SCR2_SX2		0x48
#define SEPS525_SS_SCR2_SY1		0x49
#define SEPS525_SS_SCR2_SY2		0x4A

/* SEPS525_DISPLAYMODE_SET */
#define MODE_SWAP_BGR	BIT(7)
#define MODE_SM		BIT(6)
#define MODE_RD		BIT(5)
#define MODE_CD		BIT(4)

/**
 * struct seps525_priv - Private structure
 * @reset_gpio: Reset gpio pin
 * @dc_gpio: Data/command control gpio pin
 * @dev: Device uclass for video_ops
 */
struct seps525_priv {
	struct gpio_desc reset_gpio;
	struct gpio_desc dc_gpio;
	struct udevice *dev;
};

static int seps525_spi_write_cmd(struct udevice *dev, u32 reg)
{
	struct seps525_priv *priv = dev_get_priv(dev);
	u8 buf8 = reg;
	int ret;

	ret = dm_gpio_set_value(&priv->dc_gpio, 0);
	if (ret) {
		dev_dbg(dev, "Failed to handle dc\n");
		return ret;
	}

	ret = dm_spi_xfer(dev, 8, &buf8, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret)
		dev_dbg(dev, "Failed to write command\n");

	return ret;
}

static int seps525_spi_write_data(struct udevice *dev, u32 val)
{
	struct seps525_priv *priv = dev_get_priv(dev);
	u8 buf8 = val;
	int ret;

	ret = dm_gpio_set_value(&priv->dc_gpio, 1);
	if (ret) {
		dev_dbg(dev, "Failed to handle dc\n");
		return ret;
	}

	ret = dm_spi_xfer(dev, 8, &buf8, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret)
		dev_dbg(dev, "Failed to write data\n");

	return ret;
}

static void seps525_spi_write(struct udevice *dev, u32 reg, u32 val)
{
	(void)seps525_spi_write_cmd(dev, reg);
	(void)seps525_spi_write_data(dev, val);
}

static int seps525_display_init(struct udevice *dev)
{
	/* Disable Oscillator Power Down */
	seps525_spi_write(dev, SEPS525_REDUCE_CURRENT, 0x03);
	mdelay(5);

	/* Set Normal Driving Current */
	seps525_spi_write(dev, SEPS525_REDUCE_CURRENT, 0x00);
	mdelay(5);

	seps525_spi_write(dev, SEPS525_SCREEN_SAVER_CONTEROL, 0x00);
	/* Set EXPORT1 Pin at Internal Clock */
	seps525_spi_write(dev, SEPS525_OSC_CTL, 0x01);
	/* Set Clock as 120 Frames/Sec */
	seps525_spi_write(dev, SEPS525_CLOCK_DIV, 0x90);
	/* Set Reference Voltage Controlled by External Resister */
	seps525_spi_write(dev, SEPS525_IREF, 0x01);

	/* precharge time R G B */
	seps525_spi_write(dev, SEPS525_PRECHARGE_TIME_R, 0x04);
	seps525_spi_write(dev, SEPS525_PRECHARGE_TIME_G, 0x05);
	seps525_spi_write(dev, SEPS525_PRECHARGE_TIME_B, 0x05);

	/* precharge current R G B (uA) */
	seps525_spi_write(dev, SEPS525_PRECHARGE_CURRENT_R, 0x9D);
	seps525_spi_write(dev, SEPS525_PRECHARGE_CURRENT_G, 0x8C);
	seps525_spi_write(dev, SEPS525_PRECHARGE_CURRENT_B, 0x57);

	/* driving current R G B (uA) */
	seps525_spi_write(dev, SEPS525_DRIVING_CURRENT_R, 0x56);
	seps525_spi_write(dev, SEPS525_DRIVING_CURRENT_G, 0x4D);
	seps525_spi_write(dev, SEPS525_DRIVING_CURRENT_B, 0x46);
	/* Set Color Sequence */
	seps525_spi_write(dev, SEPS525_DISPLAYMODE_SET, 0x00);
	/* Set MCU Interface Mode */
	seps525_spi_write(dev, SEPS525_RGBIF, 0x01);
	/* Set Memory Write Mode */
	seps525_spi_write(dev, SEPS525_MEMORY_WRITEMODE, 0x66);
	/* 1/128 Duty (0x0F~0x7F) */
	seps525_spi_write(dev, SEPS525_DUTY, 0x7F);
	/* Set Mapping RAM Display Start Line (0x00~0x7F) */
	seps525_spi_write(dev, SEPS525_DSL, 0x00);
	/* Display On (0x00/0x01) */
	seps525_spi_write(dev, SEPS525_DISP_ONOFF, 0x01);
	/* Set All Internal Register Value as Normal Mode */
	seps525_spi_write(dev, SEPS525_SOFT_RST, 0x00);
	/* Set RGB Interface Polarity as Active Low */
	seps525_spi_write(dev, SEPS525_RGB_POL, 0x00);

	/* Enable access for data */
	(void)seps525_spi_write_cmd(dev, SEPS525_DDRAM_DATA_ACCESS_PORT);

	return 0;
}

static int seps525_spi_startup(struct udevice *dev)
{
	struct seps525_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return ret;

	ret = dm_spi_claim_bus(dev);
	if (ret) {
		dev_err(dev, "Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	ret = seps525_display_init(dev);
	if (ret)
		return ret;

	dm_spi_release_bus(dev);

	return 0;
}

static int seps525_sync(struct udevice *vid)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(vid);
	struct seps525_priv *priv = dev_get_priv(vid);
	struct udevice *dev = priv->dev;
	int i, ret;
	u8 data1, data2;
	u8 *start = uc_priv->fb;

	ret = dm_spi_claim_bus(dev);
	if (ret) {
		dev_err(dev, "Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	/* start position X,Y */
	seps525_spi_write(dev, SEPS525_MEMORY_ACCESS_POINTER_X, 0);
	seps525_spi_write(dev, SEPS525_MEMORY_ACCESS_POINTER_Y, 0);

	/* Enable access for data */
	(void)seps525_spi_write_cmd(dev, SEPS525_DDRAM_DATA_ACCESS_PORT);

	for (i = 0; i < (uc_priv->xsize * uc_priv->ysize); i++) {
		data2 = *start++;
		data1 = *start++;
		(void)seps525_spi_write_data(dev, data1);
		(void)seps525_spi_write_data(dev, data2);
	}

	dm_spi_release_bus(dev);

	return 0;
}

static int seps525_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct seps525_priv *priv = dev_get_priv(dev);
	u32 buswidth;
	int ret;

	buswidth = dev_read_u32_default(dev, "buswidth", 0);
	if (buswidth != 8) {
		dev_err(dev, "Only 8bit buswidth is supported now");
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "missing reset GPIO\n");
		return ret;
	}

	ret = gpio_request_by_name(dev, "dc-gpios", 0,
				   &priv->dc_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "missing dc GPIO\n");
		return ret;
	}

	uc_priv->bpix = VIDEO_BPP16;
	uc_priv->xsize = WIDTH;
	uc_priv->ysize = HEIGHT;
	uc_priv->rot = 0;

	priv->dev = dev;

	ret = seps525_spi_startup(dev);
	if (ret)
		return ret;

	return 0;
}

static int seps525_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = WIDTH * HEIGHT * 16;

	return 0;
}

static const struct video_ops seps525_ops = {
	.video_sync = seps525_sync,
};

static const struct udevice_id seps525_ids[] = {
	{ .compatible = "syncoam,seps525" },
	{ }
};

U_BOOT_DRIVER(seps525_video) = {
	.name = "seps525_video",
	.id = UCLASS_VIDEO,
	.of_match = seps525_ids,
	.ops = &seps525_ops,
	.plat_auto = sizeof(struct video_uc_plat),
	.bind = seps525_bind,
	.probe = seps525_probe,
	.priv_auto = sizeof(struct seps525_priv),
};
