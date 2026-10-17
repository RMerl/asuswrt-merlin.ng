// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <video_bridge.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Initialisation of the chip is a process of writing certain values into
 * certain registers over i2c bus. The chip in fact responds to a range of
 * addresses on the i2c bus, so for each written value three parameters are
 * required: i2c address, register address and the actual value.
 *
 * The base address is derived from the device tree, but oddly the chip
 * responds on several addresses with different register sets for each.
 */

/**
 * ps8622_write() Write a PS8622 eDP bridge i2c register
 *
 * @param dev		I2C device
 * @param addr_off	offset from the i2c base address for ps8622
 * @param reg_addr	register address to write
 * @param value		value to be written
 * Return: 0 on success, non-0 on failure
 */
static int ps8622_write(struct udevice *dev, unsigned addr_off,
			unsigned char reg_addr, unsigned char value)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	uint8_t buf[2];
	struct i2c_msg msg;
	int ret;

	msg.addr = chip->chip_addr + addr_off;
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

static int ps8622_set_backlight(struct udevice *dev, int percent)
{
	int level = percent * 255 / 100;

	debug("%s: level=%d\n", __func__, level);
	return ps8622_write(dev, 0x01, 0xa7, level);
}

static int ps8622_attach(struct udevice *dev)
{
	const uint8_t *params;
	struct udevice *reg;
	int ret, i, len;

	debug("%s: %s\n", __func__, dev->name);
	/* set the LDO providing the 1.2V rail to the Parade bridge */
	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "power-supply", &reg);
	if (!ret) {
		ret = regulator_autoset(reg);
	} else if (ret != -ENOENT) {
		debug("%s: Failed to enable power: ret=%d\n", __func__, ret);
		return ret;
	}

	ret = video_bridge_set_active(dev, true);
	if (ret)
		return ret;

	params = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "parade,regs",
			     &len);
	if (!params || len % 3) {
		debug("%s: missing/invalid params=%p, len=%x\n", __func__,
		      params, len);
		return -EINVAL;
	}

	/* need to wait 20ms after power on before doing I2C writes */
	mdelay(20);
	for (i = 0; i < len; i += 3) {
		ret = ps8622_write(dev, params[i + 0], params[i + 1],
				   params[i + 2]);
		if (ret)
			return ret;
	}

	return 0;
}

static int ps8622_probe(struct udevice *dev)
{
	debug("%s\n", __func__);
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return 0;
}

struct video_bridge_ops ps8622_ops = {
	.attach = ps8622_attach,
	.set_backlight = ps8622_set_backlight,
};

static const struct udevice_id ps8622_ids[] = {
	{ .compatible = "parade,ps8622", },
	{ .compatible = "parade,ps8625", },
	{ }
};

U_BOOT_DRIVER(parade_ps8622) = {
	.name	= "parade_ps8622",
	.id	= UCLASS_VIDEO_BRIDGE,
	.of_match = ps8622_ids,
	.probe	= ps8622_probe,
	.ops	= &ps8622_ops,
};
