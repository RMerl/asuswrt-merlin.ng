// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <malloc.h>
#include <asm/clk.h>
#include <dm/device_compat.h>
#include <linux/err.h>

static const char * const sandbox_clk_test_names[] = {
	[SANDBOX_CLK_TEST_ID_FIXED] = "fixed",
	[SANDBOX_CLK_TEST_ID_SPI] = "spi",
	[SANDBOX_CLK_TEST_ID_I2C] = "i2c",
	[SANDBOX_CLK_TEST_ID_I2C_ROOT] = "i2c_root",
};

int sandbox_clk_test_get(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i, ret;

	for (i = 0; i < SANDBOX_CLK_TEST_NON_DEVM_COUNT; i++) {
		ret = clk_get_by_name(dev, sandbox_clk_test_names[i],
				      &sbct->clks[i]);
		if (ret)
			return ret;
	}

	return 0;
}

int sandbox_clk_test_devm_get(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	struct clk *clk;

	clk = devm_clk_get(dev, "no-an-existing-clock");
	if (!IS_ERR(clk)) {
		dev_err(dev, "devm_clk_get() should have failed\n");
		return -EINVAL;
	}

	clk = devm_clk_get(dev, "uart2");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	sbct->clkps[SANDBOX_CLK_TEST_ID_DEVM1] = clk;

	clk = devm_clk_get_optional(dev, "uart1");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	sbct->clkps[SANDBOX_CLK_TEST_ID_DEVM2] = clk;

	sbct->clkps[SANDBOX_CLK_TEST_ID_DEVM_NULL] = NULL;
	clk = devm_clk_get_optional(dev, "not_an_existing_clock");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	if (clk)
		return -EINVAL;

	return 0;
}

int sandbox_clk_test_get_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_get_bulk(dev, &sbct->bulk);
}

ulong sandbox_clk_test_get_rate(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_get_rate(sbct->clkps[id]);
}

ulong sandbox_clk_test_round_rate(struct udevice *dev, int id, ulong rate)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_round_rate(sbct->clkps[id], rate);
}

ulong sandbox_clk_test_set_rate(struct udevice *dev, int id, ulong rate)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_set_rate(sbct->clkps[id], rate);
}

int sandbox_clk_test_enable(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_enable(sbct->clkps[id]);
}

int sandbox_clk_test_enable_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_enable_bulk(&sbct->bulk);
}

int sandbox_clk_test_disable(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_disable(sbct->clkps[id]);
}

int sandbox_clk_test_disable_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_disable_bulk(&sbct->bulk);
}

int sandbox_clk_test_release_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_release_bulk(&sbct->bulk);
}

int sandbox_clk_test_valid(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i;

	for (i = 0; i < SANDBOX_CLK_TEST_ID_COUNT; i++) {
		if (!clk_valid(sbct->clkps[i]))
			if (i != SANDBOX_CLK_TEST_ID_DEVM_NULL)
				return -EINVAL;
	}

	return 0;
}

static int sandbox_clk_test_probe(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i;

	for (i = 0; i < SANDBOX_CLK_TEST_ID_DEVM1; i++)
		sbct->clkps[i] = &sbct->clks[i];
	for (i = SANDBOX_CLK_TEST_ID_DEVM1; i < SANDBOX_CLK_TEST_ID_COUNT; i++)
		sbct->clkps[i] = NULL;

	return 0;
}

static const struct udevice_id sandbox_clk_test_ids[] = {
	{ .compatible = "sandbox,clk-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_clk_test) = {
	.name = "sandbox_clk_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_clk_test_ids,
	.probe = sandbox_clk_test_probe,
	.priv_auto	= sizeof(struct sandbox_clk_test),
};
