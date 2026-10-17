// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_RTC

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <rtc.h>

int dm_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (!ops->get)
		return -ENOSYS;
	return ops->get(dev, time);
}

int dm_rtc_set(struct udevice *dev, struct rtc_time *time)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (!ops->set)
		return -ENOSYS;
	return ops->set(dev, time);
}

int dm_rtc_reset(struct udevice *dev)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (!ops->reset)
		return -ENOSYS;
	return ops->reset(dev);
}

int dm_rtc_read(struct udevice *dev, unsigned int reg, u8 *buf, unsigned int len)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (ops->read)
		return ops->read(dev, reg, buf, len);
	if (!ops->read8)
		return -ENOSYS;
	while (len--) {
		int ret = ops->read8(dev, reg++);

		if (ret < 0)
			return ret;
		*buf++ = ret;
	}
	return 0;
}

int dm_rtc_write(struct udevice *dev, unsigned int reg,
		 const u8 *buf, unsigned int len)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (ops->write)
		return ops->write(dev, reg, buf, len);
	if (!ops->write8)
		return -ENOSYS;
	while (len--) {
		int ret = ops->write8(dev, reg++, *buf++);

		if (ret < 0)
			return ret;
	}
	return 0;
}

int rtc_read8(struct udevice *dev, unsigned int reg)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (ops->read8)
		return ops->read8(dev, reg);
	if (ops->read) {
		u8 buf[1];
		int ret = ops->read(dev, reg, buf, 1);

		if (ret < 0)
			return ret;
		return buf[0];
	}
	return -ENOSYS;
}

int rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	struct rtc_ops *ops = rtc_get_ops(dev);

	assert(ops);
	if (ops->write8)
		return ops->write8(dev, reg, val);
	if (ops->write) {
		u8 buf[1] = { val };

		return ops->write(dev, reg, buf, 1);
	}
	return -ENOSYS;
}

int rtc_read16(struct udevice *dev, unsigned int reg, u16 *valuep)
{
	u16 value = 0;
	int ret;
	int i;

	for (i = 0; i < sizeof(value); i++) {
		ret = rtc_read8(dev, reg + i);
		if (ret < 0)
			return ret;
		value |= ret << (i << 3);
	}

	*valuep = value;
	return 0;
}

int rtc_write16(struct udevice *dev, unsigned int reg, u16 value)
{
	int i, ret;

	for (i = 0; i < sizeof(value); i++) {
		ret = rtc_write8(dev, reg + i, (value >> (i << 3)) & 0xff);
		if (ret)
			return ret;
	}

	return 0;
}

int rtc_read32(struct udevice *dev, unsigned int reg, u32 *valuep)
{
	u32 value = 0;
	int ret;
	int i;

	for (i = 0; i < sizeof(value); i++) {
		ret = rtc_read8(dev, reg + i);
		if (ret < 0)
			return ret;
		value |= ret << (i << 3);
	}

	*valuep = value;
	return 0;
}

int rtc_write32(struct udevice *dev, unsigned int reg, u32 value)
{
	int i, ret;

	for (i = 0; i < sizeof(value); i++) {
		ret = rtc_write8(dev, reg + i, (value >> (i << 3)) & 0xff);
		if (ret)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(rtc) = {
	.name		= "rtc",
	.id		= UCLASS_RTC,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind	= dm_scan_fdt_dev,
#endif
};
