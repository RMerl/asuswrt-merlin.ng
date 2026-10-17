// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Exynos PMU (Power Management Unit) driver.
 */

#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/err.h>

#define EXYNOS850_UART_IO_SHARE_CTRL	0x0760
#define SEL_RXD_AP_UART_SHIFT		16
#define SEL_RXD_AP_UART_MASK		GENMASK(17, 16)
#define SEL_TXD_GPIO_1_SHIFT		20
#define SEL_TXD_GPIO_1_MASK		GENMASK(21, 20)
#define RXD_GPIO_1			0x3
#define TXD_AP_UART			0x0

struct exynos_pmu {
	struct udevice *dev;
	const struct exynos_pmu_data *pmu_data;
	struct regmap *regmap;
};

struct exynos_pmu_data {
	int (*pmu_init)(struct exynos_pmu *priv);
};

static int exynos850_pmu_init(struct exynos_pmu *priv)
{
	ofnode node;
	bool uart_debug_1;
	unsigned int offset, mask, value;

	node = dev_ofnode(priv->dev);
	uart_debug_1 = ofnode_read_bool(node, "samsung,uart-debug-1");
	if (!uart_debug_1)
		return 0;

	/*
	 * If uart1_pins are used for serial, AP UART lines have to be muxed
	 * in PMU block to UART_DEBUG_1 path (GPIO_1). By default (reset value)
	 * UART_DEBUG_0 path (uart0_pins) is connected to AP UART lines.
	 */
	offset = EXYNOS850_UART_IO_SHARE_CTRL;
	mask = SEL_RXD_AP_UART_MASK | SEL_TXD_GPIO_1_MASK;
	value = RXD_GPIO_1 << SEL_RXD_AP_UART_SHIFT |
		TXD_AP_UART << SEL_TXD_GPIO_1_SHIFT;
	return regmap_update_bits(priv->regmap, offset, mask, value);
}

static const struct exynos_pmu_data exynos850_pmu_data = {
	.pmu_init = exynos850_pmu_init,
};

static int exynos_pmu_bind(struct udevice *dev)
{
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);
	return 0;
}

static int exynos_pmu_probe(struct udevice *dev)
{
	ofnode node;
	struct exynos_pmu *priv;

	priv = dev_get_priv(dev);
	priv->dev = dev;

	node = dev_ofnode(dev);
	priv->regmap = syscon_node_to_regmap(node);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	priv->pmu_data = (struct exynos_pmu_data *)dev_get_driver_data(dev);
	if (priv->pmu_data && priv->pmu_data->pmu_init)
		return priv->pmu_data->pmu_init(priv);

	return 0;
}

static const struct udevice_id exynos_pmu_ids[] = {
	{
		.compatible = "samsung,exynos850-pmu",
		.data = (ulong)&exynos850_pmu_data
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(exynos_pmu) = {
	.name		= "exynos-pmu",
	.id		= UCLASS_NOP,
	.of_match	= exynos_pmu_ids,
	.bind		= exynos_pmu_bind,
	.probe		= exynos_pmu_probe,
	.priv_auto	= sizeof(struct exynos_pmu),
};
