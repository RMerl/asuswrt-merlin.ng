// SPDX-License-Identifier: GPL-2.0+
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm.h>
#include <linux/delay.h>
#include <wdt.h>

#define MAX6370_SET_MASK	0x7
#define MAX6370_SET_1MS		0x0
#define MAX6370_SET_10MS	0x1
#define MAX6370_SET_30MS	0x2
#define MAX6370_SET_DISABLE	0x3
#define MAX6370_SET_100MS	0x4
#define MAX6370_SET_1S		0x5
#define MAX6370_SET_10S		0x6
#define MAX6370_SET_60S		0x7

#define MAX6370_WDI		0x8

struct max6370_wdt {
	void __iomem *reg;
	struct gpio_desc gpio_wdi;
};

static int max6370_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct max6370_wdt *wdt = dev_get_priv(dev);
	u8 val;

	val = readb(wdt->reg);
	val &= ~MAX6370_SET_MASK;

	if (ms <= 1)
		val |= MAX6370_SET_1MS;
	else if (ms <= 10)
		val |= MAX6370_SET_10MS;
	else if (ms <= 30)
		val |= MAX6370_SET_30MS;
	else if (ms <= 100)
		val |= MAX6370_SET_100MS;
	else if (ms <= 1000)
		val |= MAX6370_SET_1S;
	else if (ms <= 10000)
		val |= MAX6370_SET_10S;
	else
		val |= MAX6370_SET_60S;

	writeb(val, wdt->reg);

	return 0;
}

static int max6370_wdt_stop(struct udevice *dev)
{
	struct max6370_wdt *wdt = dev_get_priv(dev);
	u8 val;

	val = readb(wdt->reg);
	val &= ~MAX6370_SET_MASK;
	val |= MAX6370_SET_DISABLE;
	writeb(val, wdt->reg);

	return 0;
}

static int max6370_wdt_reset(struct udevice *dev)
{
	struct max6370_wdt *wdt = dev_get_priv(dev);
	u8 val;

	if (dm_gpio_is_valid(&wdt->gpio_wdi)) {
		dm_gpio_set_value(&wdt->gpio_wdi, 1);
		__udelay(1);
		dm_gpio_set_value(&wdt->gpio_wdi, 0);
	} else {
		val = readb(wdt->reg);
		writeb(val | MAX6370_WDI, wdt->reg);
		writeb(val & ~MAX6370_WDI, wdt->reg);
	}

	return 0;
}

static int max6370_wdt_probe(struct udevice *dev)
{
	struct max6370_wdt *wdt = dev_get_priv(dev);

	wdt->reg = dev_read_addr_ptr(dev);
	if (!wdt->reg)
		return -EINVAL;

	/* WDI gpio is optional */
	gpio_request_by_name(dev, "gpios", 0, &wdt->gpio_wdi, GPIOD_IS_OUT);

	return 0;
}

static const struct wdt_ops max6370_wdt_ops = {
	.start = max6370_wdt_start,
	.stop = max6370_wdt_stop,
	.reset = max6370_wdt_reset,
};

static const struct udevice_id max6370_wdt_ids[] = {
	{ .compatible = "maxim,max6370" },
	{}
};

U_BOOT_DRIVER(max6370_wdt) = {
	.name = "max6370_wdt",
	.id = UCLASS_WDT,
	.of_match = max6370_wdt_ids,
	.probe = max6370_wdt_probe,
	.priv_auto = sizeof(struct max6370_wdt),
	.ops = &max6370_wdt_ops,
};
