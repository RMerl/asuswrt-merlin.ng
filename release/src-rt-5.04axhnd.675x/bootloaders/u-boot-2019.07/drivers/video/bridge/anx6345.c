// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <edid.h>
#include <video_bridge.h>
#include "../anx98xx-edp.h"

#define DP_MAX_LINK_RATE		0x001
#define DP_MAX_LANE_COUNT		0x002
#define DP_MAX_LANE_COUNT_MASK		0x1f

struct anx6345_priv {
	u8 edid[EDID_SIZE];
};

static int anx6345_write(struct udevice *dev, unsigned int addr_off,
			 unsigned char reg_addr, unsigned char value)
{
	uint8_t buf[2];
	struct i2c_msg msg;
	int ret;

	msg.addr = addr_off;
	msg.flags = 0;
	buf[0] = reg_addr;
	buf[1] = value;
	msg.buf = buf;
	msg.len = 2;
	ret = dm_i2c_xfer(dev, &msg, 1);
	if (ret) {
		debug("%s: write failed, reg=%#x, value=%#x, ret=%d\n",
		      __func__, reg_addr, value, ret);
		return ret;
	}

	return 0;
}

static int anx6345_read(struct udevice *dev, unsigned int addr_off,
			unsigned char reg_addr, unsigned char *value)
{
	uint8_t addr, val;
	struct i2c_msg msg[2];
	int ret;

	msg[0].addr = addr_off;
	msg[0].flags = 0;
	addr = reg_addr;
	msg[0].buf = &addr;
	msg[0].len = 1;
	msg[1].addr = addr_off;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = &val;
	msg[1].len = 1;
	ret = dm_i2c_xfer(dev, msg, 2);
	if (ret) {
		debug("%s: read failed, reg=%.2x, value=%p, ret=%d\n",
		      __func__, (int)reg_addr, value, ret);
		return ret;
	}
	*value = val;

	return 0;
}

static int anx6345_write_r0(struct udevice *dev, unsigned char reg_addr,
			    unsigned char value)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);

	return anx6345_write(dev, chip->chip_addr, reg_addr, value);
}

static int anx6345_read_r0(struct udevice *dev, unsigned char reg_addr,
			   unsigned char *value)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);

	return anx6345_read(dev, chip->chip_addr, reg_addr, value);
}

static int anx6345_write_r1(struct udevice *dev, unsigned char reg_addr,
			    unsigned char value)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);

	return anx6345_write(dev, chip->chip_addr + 1, reg_addr, value);
}

static int anx6345_read_r1(struct udevice *dev, unsigned char reg_addr,
			   unsigned char *value)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);

	return anx6345_read(dev, chip->chip_addr + 1, reg_addr, value);
}

static int anx6345_set_backlight(struct udevice *dev, int percent)
{
	return -ENOSYS;
}

static int anx6345_aux_wait(struct udevice *dev)
{
	int ret = -ETIMEDOUT;
	u8 v;
	int retries = 1000;

	do {
		anx6345_read_r0(dev, ANX9804_DP_AUX_CH_CTL_2, &v);
		if (!(v & ANX9804_AUX_EN)) {
			ret = 0;
			break;
		}
		udelay(100);
	} while (retries--);

	if (ret) {
		debug("%s: timed out waiting for AUX_EN to clear\n", __func__);
		return ret;
	}

	ret = -ETIMEDOUT;
	retries = 1000;
	do {
		anx6345_read_r1(dev, ANX9804_DP_INT_STA, &v);
		if (v & ANX9804_RPLY_RECEIV) {
			ret = 0;
			break;
		}
		udelay(100);
	} while (retries--);

	if (ret) {
		debug("%s: timed out waiting to receive reply\n", __func__);
		return ret;
	}

	/* Clear RPLY_RECEIV bit */
	anx6345_write_r1(dev, ANX9804_DP_INT_STA, v);

	anx6345_read_r0(dev, ANX9804_AUX_CH_STA, &v);
	if ((v & ANX9804_AUX_STATUS_MASK) != 0) {
		debug("AUX status: %d\n", v & ANX9804_AUX_STATUS_MASK);
		ret = -EIO;
	}

	return ret;
}

static void anx6345_aux_addr(struct udevice *dev, u32 addr)
{
	u8 val;

	val = addr & 0xff;
	anx6345_write_r0(dev, ANX9804_DP_AUX_ADDR_7_0, val);
	val = (addr >> 8) & 0xff;
	anx6345_write_r0(dev, ANX9804_DP_AUX_ADDR_15_8, val);
	val = (addr >> 16) & 0x0f;
	anx6345_write_r0(dev, ANX9804_DP_AUX_ADDR_19_16, val);
}

static int anx6345_aux_transfer(struct udevice *dev, u8 req,
				u32 addr, u8 *buf, size_t len)
{
	int i, ret;
	u8 ctrl1 = req;
	u8 ctrl2 = ANX9804_AUX_EN;

	if (len > 16)
		return -E2BIG;

	if (len)
		ctrl1 |= ANX9804_AUX_LENGTH(len);
	else
		ctrl2 |= ANX9804_ADDR_ONLY;

	if (len && !(req & ANX9804_AUX_TX_COMM_READ)) {
		for (i = 0; i < len; i++)
			anx6345_write_r0(dev, ANX9804_BUF_DATA_0 + i, buf[i]);
	}

	anx6345_aux_addr(dev, addr);
	anx6345_write_r0(dev, ANX9804_DP_AUX_CH_CTL_1, ctrl1);
	anx6345_write_r0(dev, ANX9804_DP_AUX_CH_CTL_2, ctrl2);
	ret = anx6345_aux_wait(dev);
	if (ret) {
		debug("AUX transaction timed out\n");
		return ret;
	}

	if (len && (req & ANX9804_AUX_TX_COMM_READ)) {
		for (i = 0; i < len; i++)
			anx6345_read_r0(dev, ANX9804_BUF_DATA_0 + i, &buf[i]);
	}

	return 0;
}

static int anx6345_read_aux_i2c(struct udevice *dev, u8 chip_addr,
				u8 offset, size_t count, u8 *buf)
{
	int i, ret;
	size_t cur_cnt;
	u8 cur_offset;

	for (i = 0; i < count; i += 16) {
		cur_cnt = (count - i) > 16 ? 16 : count - i;
		cur_offset = offset + i;
		ret = anx6345_aux_transfer(dev, ANX9804_AUX_TX_COMM_MOT,
					   chip_addr, &cur_offset, 1);
		if (ret) {
			debug("%s: failed to set i2c offset: %d\n",
			      __func__, ret);
			return ret;
		}
		ret = anx6345_aux_transfer(dev, ANX9804_AUX_TX_COMM_READ,
					   chip_addr, buf + i, cur_cnt);
		if (ret) {
			debug("%s: failed to read from i2c device: %d\n",
			      __func__, ret);
			return ret;
		}
	}

	return 0;
}

static int anx6345_read_dpcd(struct udevice *dev, u32 reg, u8 *val)
{
	int ret;

	ret = anx6345_aux_transfer(dev,
				   ANX9804_AUX_TX_COMM_READ |
				   ANX9804_AUX_TX_COMM_DP_TRANSACTION,
				   reg, val, 1);
	if (ret) {
		debug("Failed to read DPCD\n");
		return ret;
	}

	return 0;
}

static int anx6345_read_edid(struct udevice *dev, u8 *buf, int size)
{
	struct anx6345_priv *priv = dev_get_priv(dev);

	if (size > EDID_SIZE)
		size = EDID_SIZE;
	memcpy(buf, priv->edid, size);

	return size;
}

static int anx6345_attach(struct udevice *dev)
{
	/* No-op */
	return 0;
}

static int anx6345_enable(struct udevice *dev)
{
	u8 chipid, colordepth, lanes, data_rate, c;
	int ret, i, bpp;
	struct display_timing timing;
	struct anx6345_priv *priv = dev_get_priv(dev);

	/* Deassert reset and enable power */
	ret = video_bridge_set_active(dev, true);
	if (ret)
		return ret;

	/* Reset */
	anx6345_write_r1(dev, ANX9804_RST_CTRL_REG, 1);
	mdelay(100);
	anx6345_write_r1(dev, ANX9804_RST_CTRL_REG, 0);

	/* Write 0 to the powerdown reg (powerup everything) */
	anx6345_write_r1(dev, ANX9804_POWERD_CTRL_REG, 0);

	ret = anx6345_read_r1(dev, ANX9804_DEV_IDH_REG, &chipid);
	if (ret)
		debug("%s: read id failed: %d\n", __func__, ret);

	switch (chipid) {
	case 0x63:
		debug("ANX63xx detected.\n");
		break;
	default:
		debug("Error anx6345 chipid mismatch: %.2x\n", (int)chipid);
		return -ENODEV;
	}

	for (i = 0; i < 100; i++) {
		anx6345_read_r0(dev, ANX9804_SYS_CTRL2_REG, &c);
		anx6345_write_r0(dev, ANX9804_SYS_CTRL2_REG, c);
		anx6345_read_r0(dev, ANX9804_SYS_CTRL2_REG, &c);
		if ((c & ANX9804_SYS_CTRL2_CHA_STA) == 0)
			break;

		mdelay(5);
	}
	if (i == 100)
		debug("Error anx6345 clock is not stable\n");

	/* Set a bunch of analog related register values */
	anx6345_write_r0(dev, ANX9804_PLL_CTRL_REG, 0x00);
	anx6345_write_r1(dev, ANX9804_ANALOG_DEBUG_REG1, 0x70);
	anx6345_write_r0(dev, ANX9804_LINK_DEBUG_REG, 0x30);

	/* Force HPD */
	anx6345_write_r0(dev, ANX9804_SYS_CTRL3_REG,
			 ANX9804_SYS_CTRL3_F_HPD | ANX9804_SYS_CTRL3_HPD_CTRL);

	/* Power up and configure lanes */
	anx6345_write_r0(dev, ANX9804_ANALOG_POWER_DOWN_REG, 0x00);
	anx6345_write_r0(dev, ANX9804_TRAINING_LANE0_SET_REG, 0x00);
	anx6345_write_r0(dev, ANX9804_TRAINING_LANE1_SET_REG, 0x00);
	anx6345_write_r0(dev, ANX9804_TRAINING_LANE2_SET_REG, 0x00);
	anx6345_write_r0(dev, ANX9804_TRAINING_LANE3_SET_REG, 0x00);

	/* Reset AUX CH */
	anx6345_write_r1(dev, ANX9804_RST_CTRL2_REG,
			 ANX9804_RST_CTRL2_AUX);
	anx6345_write_r1(dev, ANX9804_RST_CTRL2_REG, 0);

	/* Powerdown audio and some other unused bits */
	anx6345_write_r1(dev, ANX9804_POWERD_CTRL_REG, ANX9804_POWERD_AUDIO);
	anx6345_write_r0(dev, ANX9804_HDCP_CONTROL_0_REG, 0x00);
	anx6345_write_r0(dev, 0xa7, 0x00);

	anx6345_read_aux_i2c(dev, 0x50, 0x0, EDID_SIZE, priv->edid);
	if (edid_get_timing(priv->edid, EDID_SIZE, &timing, &bpp) != 0) {
		debug("Failed to parse EDID\n");
		return -EIO;
	}
	debug("%s: panel found: %dx%d, bpp %d\n", __func__,
	      timing.hactive.typ, timing.vactive.typ, bpp);
	if (bpp == 6)
		colordepth = 0x00; /* 6 bit */
	else
		colordepth = 0x10; /* 8 bit */
	anx6345_write_r1(dev, ANX9804_VID_CTRL2_REG, colordepth);

	if (anx6345_read_dpcd(dev, DP_MAX_LINK_RATE, &data_rate)) {
		debug("%s: Failed to DP_MAX_LINK_RATE\n", __func__);
		return -EIO;
	}
	debug("%s: data_rate: %d\n", __func__, (int)data_rate);
	if (anx6345_read_dpcd(dev, DP_MAX_LANE_COUNT, &lanes)) {
		debug("%s: Failed to read DP_MAX_LANE_COUNT\n", __func__);
		return -EIO;
	}
	lanes &= DP_MAX_LANE_COUNT_MASK;
	debug("%s: lanes: %d\n", __func__, (int)lanes);

	/* Set data-rate / lanes */
	anx6345_write_r0(dev, ANX9804_LINK_BW_SET_REG, data_rate);
	anx6345_write_r0(dev, ANX9804_LANE_COUNT_SET_REG, lanes);

	/* Link training */
	anx6345_write_r0(dev, ANX9804_LINK_TRAINING_CTRL_REG,
			 ANX9804_LINK_TRAINING_CTRL_EN);
	mdelay(5);
	for (i = 0; i < 100; i++) {
		anx6345_read_r0(dev, ANX9804_LINK_TRAINING_CTRL_REG, &c);
		if ((chipid == 0x63) && (c & 0x80) == 0)
			break;

		mdelay(5);
	}
	if (i == 100) {
		debug("Error anx6345 link training timeout\n");
		return -ENODEV;
	}

	/* Enable */
	anx6345_write_r1(dev, ANX9804_VID_CTRL1_REG,
			 ANX9804_VID_CTRL1_VID_EN | ANX9804_VID_CTRL1_EDGE);
	/* Force stream valid */
	anx6345_write_r0(dev, ANX9804_SYS_CTRL3_REG,
			 ANX9804_SYS_CTRL3_F_HPD |
			 ANX9804_SYS_CTRL3_HPD_CTRL |
			 ANX9804_SYS_CTRL3_F_VALID |
			 ANX9804_SYS_CTRL3_VALID_CTRL);

	return 0;
}

static int anx6345_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return anx6345_enable(dev);
}

struct video_bridge_ops anx6345_ops = {
	.attach = anx6345_attach,
	.set_backlight = anx6345_set_backlight,
	.read_edid = anx6345_read_edid,
};

static const struct udevice_id anx6345_ids[] = {
	{ .compatible = "analogix,anx6345", },
	{ }
};

U_BOOT_DRIVER(analogix_anx6345) = {
	.name	= "analogix_anx6345",
	.id	= UCLASS_VIDEO_BRIDGE,
	.of_match = anx6345_ids,
	.probe	= anx6345_probe,
	.ops	= &anx6345_ops,
	.priv_auto_alloc_size = sizeof(struct anx6345_priv),
};
