// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <asm/rtc.h>
#include <dm/acpi.h>

#define REG_COUNT 0x80

static int sandbox_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	u8 buf[7];
	int ret;

	ret = dm_i2c_read(dev, REG_SEC, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	time->tm_sec  = buf[REG_SEC - REG_SEC];
	time->tm_min  = buf[REG_MIN - REG_SEC];
	time->tm_hour = buf[REG_HOUR - REG_SEC];
	time->tm_mday = buf[REG_MDAY - REG_SEC];
	time->tm_mon  = buf[REG_MON - REG_SEC];
	time->tm_year = buf[REG_YEAR - REG_SEC] + 1900;
	time->tm_wday = buf[REG_WDAY - REG_SEC];

	return 0;
}

static int sandbox_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	u8 buf[7];
	int ret;

	buf[REG_SEC - REG_SEC]  = time->tm_sec;
	buf[REG_MIN - REG_SEC]  = time->tm_min;
	buf[REG_HOUR - REG_SEC] = time->tm_hour;
	buf[REG_MDAY - REG_SEC] = time->tm_mday;
	buf[REG_MON  - REG_SEC] = time->tm_mon;
	buf[REG_YEAR - REG_SEC] = time->tm_year - 1900;
	buf[REG_WDAY - REG_SEC] = time->tm_wday;

	ret = dm_i2c_write(dev, REG_SEC, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int sandbox_rtc_reset(struct udevice *dev)
{
	return dm_i2c_reg_write(dev, REG_RESET, 0);
}

static int sandbox_rtc_read8(struct udevice *dev, unsigned int reg)
{
	return dm_i2c_reg_read(dev, reg);
}

static int sandbox_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	return dm_i2c_reg_write(dev, reg, val);
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int sandbox_rtc_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "RTCC");
}

struct acpi_ops sandbox_rtc_acpi_ops = {
	.get_name	= sandbox_rtc_get_name,
};
#endif

static int sandbox_rtc_bind(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(PLATDATA)
	struct sandbox_i2c_rtc_plat_data *plat = dev_get_plat(dev);

	/* Set up the emul_idx for i2c_emul_find() */
	i2c_emul_set_idx(dev, plat->dtplat.sandbox_emul->idx);
#endif

	return 0;
}

static const struct rtc_ops sandbox_rtc_ops = {
	.get = sandbox_rtc_get,
	.set = sandbox_rtc_set,
	.reset = sandbox_rtc_reset,
	.read8 = sandbox_rtc_read8,
	.write8 = sandbox_rtc_write8,
};

static const struct udevice_id sandbox_rtc_ids[] = {
	{ .compatible = "sandbox-rtc" },
	{ }
};

U_BOOT_DRIVER(sandbox_rtc) = {
	.name	= "sandbox_rtc",
	.id	= UCLASS_RTC,
	.of_match = sandbox_rtc_ids,
	.ops	= &sandbox_rtc_ops,
	.bind	= sandbox_rtc_bind,
	ACPI_OPS_PTR(&sandbox_rtc_acpi_ops)
};
