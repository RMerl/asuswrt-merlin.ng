// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/clk.h>
#include <dm/test.h>
#include <dm/uclass.h>
#include <linux/err.h>
#include <test/test.h>
#include <test/ut.h>
#include <sandbox-clk.h>

/* Tests for Common Clock Framework driver */
static int dm_test_clk_ccf(struct unit_test_state *uts)
{
	struct clk *clk, *pclk;
	struct udevice *dev, *test_dev;
	long long rate;
	int ret;
#if CONFIG_IS_ENABLED(CLK_CCF)
	struct clk clk_ccf;
	const char *clkname;
	int clkid, i;
#endif

	/* Get the device using the clk device */
	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-ccf", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test", &test_dev));

	/* Test for clk_get_by_id() */
	ret = clk_get_by_id(SANDBOX_CLK_ECSPI_ROOT, &clk);
	ut_assertok(ret);
	ut_asserteq_str("ecspi_root", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_PARENT, clk->flags);

	/* Test for clk_get_parent_rate() */
	ret = clk_get_by_id(SANDBOX_CLK_ECSPI1, &clk);
	ut_assertok(ret);
	ut_asserteq_str("ecspi1", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_PARENT, clk->flags);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 20000000);

	/* test the gate of CCF */
	ret = clk_get_by_id(SANDBOX_CLK_ECSPI0, &clk);
	ut_assertok(ret);
	ut_asserteq_str("ecspi0", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_PARENT, clk->flags);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 20000000);

	/* Test the mux of CCF */
	ret = clk_get_by_id(SANDBOX_CLK_USDHC1_SEL, &clk);
	ut_assertok(ret);
	ut_asserteq_str("usdhc1_sel", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_NO_REPARENT, clk->flags);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 60000000);

	rate = clk_set_rate(clk, 60000000);
	ut_asserteq(rate, -ENOSYS);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 60000000);

	ret = clk_get_by_id(SANDBOX_CLK_PLL3_80M, &pclk);
	ut_assertok(ret);

	ret = clk_set_parent(clk, pclk);
	ut_assertok(ret);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 80000000);

	ret = clk_get_by_id(SANDBOX_CLK_USDHC2_SEL, &clk);
	ut_assertok(ret);
	ut_asserteq_str("usdhc2_sel", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_NO_REPARENT, clk->flags);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 80000000);

	pclk = clk_get_parent(clk);
	ut_asserteq_str("pll3_80m", pclk->dev->name);
	ut_asserteq(CLK_SET_RATE_PARENT, pclk->flags);

	rate = clk_set_rate(clk, 80000000);
	ut_asserteq(rate, -ENOSYS);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 80000000);

	ret = clk_get_by_id(SANDBOX_CLK_PLL3_60M, &pclk);
	ut_assertok(ret);

	ret = clk_set_parent(clk, pclk);
	ut_assertok(ret);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 60000000);

	/* Test the composite of CCF */
	ret = clk_get_by_id(SANDBOX_CLK_I2C, &clk);
	ut_assertok(ret);
	ut_asserteq_str("i2c", clk->dev->name);
	ut_asserteq(CLK_SET_RATE_UNGATE, clk->flags);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 60000000);

	rate = clk_set_rate(clk, 60000000);
	ut_asserteq(rate, 60000000);

#if CONFIG_IS_ENABLED(CLK_CCF)
	/* Test clk tree enable/disable */

	ret = clk_get_by_index(test_dev, SANDBOX_CLK_TEST_ID_I2C_ROOT, &clk_ccf);
	ut_assertok(ret);
	ut_asserteq_str("clk-ccf", clk_ccf.dev->name);
	ut_asserteq(clk_ccf.id, SANDBOX_CLK_I2C_ROOT);

	ret = clk_get_by_id(SANDBOX_CLK_I2C_ROOT, &clk);
	ut_assertok(ret);
	ut_asserteq_str("i2c_root", clk->dev->name);
	ut_asserteq(clk->id, SANDBOX_CLK_I2C_ROOT);

	ret = clk_enable(&clk_ccf);
	ut_assertok(ret);

	ret = sandbox_clk_enable_count(clk);
	ut_asserteq(ret, 1);

	ret = clk_get_by_id(SANDBOX_CLK_I2C, &pclk);
	ut_assertok(ret);

	ret = sandbox_clk_enable_count(pclk);
	ut_asserteq(ret, 1);

	ret = clk_disable(clk);
	ut_assertok(ret);

	ret = sandbox_clk_enable_count(clk);
	ut_asserteq(ret, 0);

	ret = sandbox_clk_enable_count(pclk);
	ut_asserteq(ret, 0);

	/* Test clock re-parenting. */
	ret = clk_get_by_id(SANDBOX_CLK_USDHC1_SEL, &clk);
	ut_assertok(ret);
	ut_asserteq_str("usdhc1_sel", clk->dev->name);

	pclk = clk_get_parent(clk);
	ut_assertok_ptr(pclk);
	if (!strcmp(pclk->dev->name, "pll3_60m")) {
		clkname = "pll3_80m";
		clkid = SANDBOX_CLK_PLL3_80M;
	} else {
		clkname = "pll3_60m";
		clkid = SANDBOX_CLK_PLL3_60M;
	}

	ret = clk_get_by_id(clkid, &pclk);
	ut_assertok(ret);
	ret = clk_set_parent(clk, pclk);
	ut_assertok(ret);
	pclk = clk_get_parent(clk);
	ut_assertok_ptr(pclk);
	ut_asserteq_str(clkname, pclk->dev->name);

	/* Test disabling critical clock. */
	ret = clk_get_by_id(SANDBOX_CLK_I2C_ROOT, &clk);
	ut_assertok(ret);
	ut_asserteq_str("i2c_root", clk->dev->name);

	/* Disable it, if any. */
	ret = sandbox_clk_enable_count(clk);
	for (i = 0; i < ret; i++) {
		ret = clk_disable(clk);
		ut_assertok(ret);
	}

	ret = sandbox_clk_enable_count(clk);
	ut_asserteq(ret, 0);

	clk->flags = CLK_IS_CRITICAL;
	ret = clk_enable(clk);
	ut_assertok(ret);

	ret = clk_disable(clk);
	ut_assertok(ret);
	ret = sandbox_clk_enable_count(clk);
	ut_asserteq(ret, 1);
	clk->flags &= ~CLK_IS_CRITICAL;

	ret = clk_disable(clk);
	ut_assertok(ret);
	ret = sandbox_clk_enable_count(clk);
	ut_asserteq(ret, 0);
#endif

	return 1;
}

DM_TEST(dm_test_clk_ccf, UT_TESTF_SCAN_FDT);
