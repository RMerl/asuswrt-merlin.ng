// SPDX-License-Identifier: GPL-2.0+
/*
 * Simulate an I2C real time clock
 *
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

/*
 * This is a test driver. It starts off with the current time of the machine,
 * but also supports setting the time, using an offset from the current
 * clock. This driver is only intended for testing, not accurate
 * time-keeping. It does not change the system time.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <os.h>
#include <rtc.h>
#include <asm/rtc.h>
#include <asm/test.h>

#ifdef DEBUG
#define debug_buffer print_buffer
#else
#define debug_buffer(x, ...)
#endif

/**
 * struct sandbox_i2c_rtc_plat_data - platform data for the RTC
 *
 * @base_time:		Base system time when RTC device was bound
 * @offset:		RTC offset from current system time
 * @use_system_time:	true to use system time, false to use @base_time
 * @reg:		Register values
 */
struct sandbox_i2c_rtc_plat_data {
	long base_time;
	long offset;
	bool use_system_time;
	u8 reg[REG_COUNT];
};

struct sandbox_i2c_rtc {
	unsigned int offset_secs;
};

long sandbox_i2c_rtc_set_offset(struct udevice *dev, bool use_system_time,
				int offset)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(dev);
	long old_offset;

	old_offset = plat->offset;
	plat->use_system_time = use_system_time;
	if (offset != -1)
		plat->offset = offset;

	return old_offset;
}

long sandbox_i2c_rtc_get_set_base_time(struct udevice *dev, long base_time)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(dev);
	long old_base_time;

	old_base_time = plat->base_time;
	if (base_time != -1)
		plat->base_time = base_time;

	return old_base_time;
}

static void reset_time(struct udevice *dev)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(dev);
	struct rtc_time now;

	os_localtime(&now);
	plat->base_time = rtc_mktime(&now);
	plat->offset = 0;
	plat->use_system_time = true;
}

static int sandbox_i2c_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(dev);
	struct rtc_time tm_now;
	long now;

	if (plat->use_system_time) {
		os_localtime(&tm_now);
		now = rtc_mktime(&tm_now);
	} else {
		now = plat->base_time;
	}

	rtc_to_tm(now + plat->offset, time);

	return 0;
}

static int sandbox_i2c_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(dev);
	struct rtc_time tm_now;
	long now;

	if (plat->use_system_time) {
		os_localtime(&tm_now);
		now = rtc_mktime(&tm_now);
	} else {
		now = plat->base_time;
	}
	plat->offset = rtc_mktime(time) - now;

	return 0;
}

/* Update the current time in the registers */
static int sandbox_i2c_rtc_prepare_read(struct udevice *emul)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(emul);
	struct rtc_time time;
	int ret;

	ret = sandbox_i2c_rtc_get(emul, &time);
	if (ret)
		return ret;

	plat->reg[REG_SEC] = time.tm_sec;
	plat->reg[REG_MIN] = time.tm_min;
	plat->reg[REG_HOUR] = time.tm_hour;
	plat->reg[REG_MDAY] = time.tm_mday;
	plat->reg[REG_MON] = time.tm_mon;
	plat->reg[REG_YEAR] = time.tm_year - 1900;
	plat->reg[REG_WDAY] = time.tm_wday;

	return 0;
}

static int sandbox_i2c_rtc_complete_write(struct udevice *emul)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(emul);
	struct rtc_time time;
	int ret;

	time.tm_sec = plat->reg[REG_SEC];
	time.tm_min = plat->reg[REG_MIN];
	time.tm_hour = plat->reg[REG_HOUR];
	time.tm_mday = plat->reg[REG_MDAY];
	time.tm_mon = plat->reg[REG_MON];
	time.tm_year = plat->reg[REG_YEAR] + 1900;
	time.tm_wday = plat->reg[REG_WDAY];

	ret = sandbox_i2c_rtc_set(emul, &time);
	if (ret)
		return ret;

	return 0;
}

static int sandbox_i2c_rtc_xfer(struct udevice *emul, struct i2c_msg *msg,
				int nmsgs)
{
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_platdata(emul);
	uint offset = 0;
	int ret;

	debug("\n%s\n", __func__);
	ret = sandbox_i2c_rtc_prepare_read(emul);
	if (ret)
		return ret;
	for (; nmsgs > 0; nmsgs--, msg++) {
		int len;
		u8 *ptr;

		len = msg->len;
		debug("   %s: msg->len=%d",
		      msg->flags & I2C_M_RD ? "read" : "write",
		      msg->len);
		if (msg->flags & I2C_M_RD) {
			debug(", offset %x, len %x: ", offset, len);

			/* Read the register */
			memcpy(msg->buf, plat->reg + offset, len);
			memset(msg->buf + len, '\xff', msg->len - len);
			debug_buffer(0, msg->buf, 1, msg->len, 0);
		} else if (len >= 1) {
			ptr = msg->buf;
			offset = *ptr++ & (REG_COUNT - 1);
			len--;
			debug(", set offset %x: ", offset);
			debug_buffer(0, msg->buf, 1, msg->len, 0);

			/* Write the register */
			memcpy(plat->reg + offset, ptr, len);
			if (offset == REG_RESET)
				reset_time(emul);
		}
	}
	ret = sandbox_i2c_rtc_complete_write(emul);
	if (ret)
		return ret;

	return 0;
}

struct dm_i2c_ops sandbox_i2c_rtc_emul_ops = {
	.xfer = sandbox_i2c_rtc_xfer,
};

static int sandbox_i2c_rtc_bind(struct udevice *dev)
{
	reset_time(dev);

	return 0;
}

static const struct udevice_id sandbox_i2c_rtc_ids[] = {
	{ .compatible = "sandbox,i2c-rtc" },
	{ }
};

U_BOOT_DRIVER(sandbox_i2c_rtc_emul) = {
	.name		= "sandbox_i2c_rtc_emul",
	.id		= UCLASS_I2C_EMUL,
	.of_match	= sandbox_i2c_rtc_ids,
	.bind		= sandbox_i2c_rtc_bind,
	.priv_auto_alloc_size = sizeof(struct sandbox_i2c_rtc),
	.platdata_auto_alloc_size = sizeof(struct sandbox_i2c_rtc_plat_data),
	.ops		= &sandbox_i2c_rtc_emul_ops,
};
