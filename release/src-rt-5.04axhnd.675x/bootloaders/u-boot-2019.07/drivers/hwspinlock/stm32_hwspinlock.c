// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <hwspinlock.h>
#include <asm/io.h>

#define STM32_MUTEX_COREID	BIT(8)
#define STM32_MUTEX_LOCK_BIT	BIT(31)
#define STM32_MUTEX_NUM_LOCKS	32

struct stm32mp1_hws_priv {
	fdt_addr_t base;
};

static int stm32mp1_lock(struct udevice *dev, int index)
{
	struct stm32mp1_hws_priv *priv = dev_get_priv(dev);
	u32 status;

	if (index >= STM32_MUTEX_NUM_LOCKS)
		return -EINVAL;

	status = readl(priv->base + index * sizeof(u32));
	if (status == (STM32_MUTEX_LOCK_BIT | STM32_MUTEX_COREID))
		return -EBUSY;

	writel(STM32_MUTEX_LOCK_BIT | STM32_MUTEX_COREID,
	       priv->base + index * sizeof(u32));

	status = readl(priv->base + index * sizeof(u32));
	if (status != (STM32_MUTEX_LOCK_BIT | STM32_MUTEX_COREID))
		return -EINVAL;

	return 0;
}

static int stm32mp1_unlock(struct udevice *dev, int index)
{
	struct stm32mp1_hws_priv *priv = dev_get_priv(dev);

	if (index >= STM32_MUTEX_NUM_LOCKS)
		return -EINVAL;

	writel(STM32_MUTEX_COREID, priv->base + index * sizeof(u32));

	return 0;
}

static int stm32mp1_hwspinlock_probe(struct udevice *dev)
{
	struct stm32mp1_hws_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		clk_free(&clk);

	return ret;
}

static const struct hwspinlock_ops stm32mp1_hwspinlock_ops = {
	.lock = stm32mp1_lock,
	.unlock = stm32mp1_unlock,
};

static const struct udevice_id stm32mp1_hwspinlock_ids[] = {
	{ .compatible = "st,stm32-hwspinlock" },
	{}
};

U_BOOT_DRIVER(hwspinlock_stm32mp1) = {
	.name = "hwspinlock_stm32mp1",
	.id = UCLASS_HWSPINLOCK,
	.of_match = stm32mp1_hwspinlock_ids,
	.ops = &stm32mp1_hwspinlock_ops,
	.probe = stm32mp1_hwspinlock_probe,
	.priv_auto_alloc_size = sizeof(struct stm32mp1_hws_priv),
};
