// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog driver for the sl28cpld
 *
 * Copyright (c) 2021 Michael Walle <michael@walle.cc>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <sl28cpld.h>
#include <div64.h>

#define SL28CPLD_WDT_CTRL		0x00
#define  WDT_CTRL_EN0			BIT(0)
#define  WDT_CTRL_EN1			BIT(1)
#define  WDT_CTRL_EN_MASK		GENMASK(1, 0)
#define  WDT_CTRL_LOCK			BIT(2)
#define  WDT_CTRL_ASSERT_SYS_RESET	BIT(6)
#define  WDT_CTRL_ASSERT_WDT_TIMEOUT	BIT(7)
#define SL28CPLD_WDT_TIMEOUT		0x01
#define SL28CPLD_WDT_KICK		0x02
#define  WDT_KICK_VALUE			0x6b

static int sl28cpld_wdt_reset(struct udevice *dev)
{
	return sl28cpld_write(dev, SL28CPLD_WDT_KICK, WDT_KICK_VALUE);
}

static int sl28cpld_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	int ret, val;

	val = sl28cpld_read(dev, SL28CPLD_WDT_CTRL);
	if (val < 0)
		return val;

	/* (1) disable watchdog */
	val &= ~WDT_CTRL_EN_MASK;
	ret = sl28cpld_write(dev, SL28CPLD_WDT_CTRL, val);
	if (ret)
		return ret;

	/* (2) set timeout */
	ret = sl28cpld_write(dev, SL28CPLD_WDT_TIMEOUT, lldiv(timeout, 1000));
	if (ret)
		return ret;

	/* (3) kick it, will reset timer to the timeout value */
	ret = sl28cpld_wdt_reset(dev);
	if (ret)
		return ret;

	/* (4) enable either recovery or normal one */
	if (flags & BIT(0))
		val |= WDT_CTRL_EN1;
	else
		val |= WDT_CTRL_EN0;

	if (flags & BIT(1))
		val |= WDT_CTRL_LOCK;

	if (flags & BIT(2))
		val &= ~WDT_CTRL_ASSERT_SYS_RESET;
	else
		val |= WDT_CTRL_ASSERT_SYS_RESET;

	if (flags & BIT(3))
		val |= WDT_CTRL_ASSERT_WDT_TIMEOUT;
	else
		val &= ~WDT_CTRL_ASSERT_WDT_TIMEOUT;

	return sl28cpld_write(dev, SL28CPLD_WDT_CTRL, val);
}

static int sl28cpld_wdt_stop(struct udevice *dev)
{
	int val;

	val = sl28cpld_read(dev, SL28CPLD_WDT_CTRL);
	if (val < 0)
		return val;

	return sl28cpld_write(dev, SL28CPLD_WDT_CTRL, val & ~WDT_CTRL_EN_MASK);
}

static int sl28cpld_wdt_expire_now(struct udevice *dev, ulong flags)
{
	return sl28cpld_wdt_start(dev, 0, flags);
}

static const struct wdt_ops sl28cpld_wdt_ops = {
	.start = sl28cpld_wdt_start,
	.reset = sl28cpld_wdt_reset,
	.stop = sl28cpld_wdt_stop,
	.expire_now = sl28cpld_wdt_expire_now,
};

static const struct udevice_id sl28cpld_wdt_ids[] = {
	{ .compatible = "kontron,sl28cpld-wdt", },
	{}
};

U_BOOT_DRIVER(sl28cpld_wdt) = {
	.name = "sl28cpld-wdt",
	.id = UCLASS_WDT,
	.of_match = sl28cpld_wdt_ids,
	.ops = &sl28cpld_wdt_ops,
};
