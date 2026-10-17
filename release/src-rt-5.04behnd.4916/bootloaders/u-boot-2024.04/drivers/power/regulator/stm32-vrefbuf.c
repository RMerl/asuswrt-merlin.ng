// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author: Fabrice Gasnier <fabrice.gasnier@st.com>
 *
 * Originally based on the Linux kernel v4.16 drivers/regulator/stm32-vrefbuf.c
 */

#define LOG_CATEGORY UCLASS_REGULATOR

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <power/regulator.h>

/* STM32 VREFBUF registers */
#define STM32_VREFBUF_CSR		0x00

/* STM32 VREFBUF CSR bitfields */
#define STM32_VRS			GENMASK(6, 4)
#define STM32_VRS_SHIFT			4
#define STM32_VRR			BIT(3)
#define STM32_HIZ			BIT(1)
#define STM32_ENVR			BIT(0)

struct stm32_vrefbuf {
	void __iomem *base;
	struct clk clk;
	struct udevice *vdda_supply;
};

static const int stm32_vrefbuf_voltages[] = {
	/* Matches resp. VRS = 000b, 001b, 010b, 011b */
	2500000, 2048000, 1800000, 1500000,
};

static int stm32_vrefbuf_set_enable(struct udevice *dev, bool enable)
{
	struct stm32_vrefbuf *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	if (enable && !(readl(priv->base + STM32_VREFBUF_CSR) & STM32_ENVR)) {
		/*
		 * There maybe an overshoot:
		 * - when disabling, then re-enabling vrefbuf too quickly
		 * - or upon platform reset as external capacitor maybe slow
		 *   discharging (VREFBUF is HiZ at reset by default).
		 * So force active discharge (HiZ=0) for 1ms before enabling.
		 */
		clrbits_le32(priv->base + STM32_VREFBUF_CSR, STM32_HIZ);
		udelay(1000);
	}

	clrsetbits_le32(priv->base + STM32_VREFBUF_CSR, STM32_ENVR,
			enable ? STM32_ENVR : 0);
	if (!enable)
		return 0;

	/*
	 * Vrefbuf startup time depends on external capacitor: wait here for
	 * VRR to be set. That means output has reached expected value.
	 * ~650us sleep should be enough for caps up to 1.5uF. Use 10ms as
	 * arbitrary timeout.
	 */
	ret = readl_poll_timeout(priv->base + STM32_VREFBUF_CSR, val,
				 val & STM32_VRR, 10000);
	if (ret < 0) {
		dev_err(dev, "stm32 vrefbuf timed out: %d\n", ret);
		clrsetbits_le32(priv->base + STM32_VREFBUF_CSR, STM32_ENVR,
				STM32_HIZ);
		return ret;
	}

	return 0;
}

static int stm32_vrefbuf_get_enable(struct udevice *dev)
{
	struct stm32_vrefbuf *priv = dev_get_priv(dev);

	return readl(priv->base + STM32_VREFBUF_CSR) & STM32_ENVR;
}

static int stm32_vrefbuf_set_value(struct udevice *dev, int uV)
{
	struct stm32_vrefbuf *priv = dev_get_priv(dev);
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(stm32_vrefbuf_voltages); i++) {
		if (uV == stm32_vrefbuf_voltages[i]) {
			clrsetbits_le32(priv->base + STM32_VREFBUF_CSR,
					STM32_VRS, i << STM32_VRS_SHIFT);
			return 0;
		}
	}

	return -EINVAL;
}

static int stm32_vrefbuf_get_value(struct udevice *dev)
{
	struct stm32_vrefbuf *priv = dev_get_priv(dev);
	u32 val;

	val = readl(priv->base + STM32_VREFBUF_CSR) & STM32_VRS;
	val >>= STM32_VRS_SHIFT;

	return stm32_vrefbuf_voltages[val];
}

static const struct dm_regulator_ops stm32_vrefbuf_ops = {
	.get_value  = stm32_vrefbuf_get_value,
	.set_value  = stm32_vrefbuf_set_value,
	.get_enable = stm32_vrefbuf_get_enable,
	.set_enable = stm32_vrefbuf_set_enable,
};

static int stm32_vrefbuf_probe(struct udevice *dev)
{
	struct stm32_vrefbuf *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		dev_err(dev, "Can't get clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "Can't enable clock: %d\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vdda-supply",
					  &priv->vdda_supply);
	if (ret) {
		dev_dbg(dev, "No vdda-supply: %d\n", ret);
		return 0;
	}

	ret = regulator_set_enable(priv->vdda_supply, true);
	if (ret) {
		dev_err(dev, "Can't enable vdda-supply: %d\n", ret);
		clk_disable(&priv->clk);
	}

	return ret;
}

static const struct udevice_id stm32_vrefbuf_ids[] = {
	{ .compatible = "st,stm32-vrefbuf" },
	{ }
};

U_BOOT_DRIVER(stm32_vrefbuf) = {
	.name  = "stm32-vrefbuf",
	.id = UCLASS_REGULATOR,
	.of_match = stm32_vrefbuf_ids,
	.probe = stm32_vrefbuf_probe,
	.ops = &stm32_vrefbuf_ops,
	.priv_auto	= sizeof(struct stm32_vrefbuf),
};
