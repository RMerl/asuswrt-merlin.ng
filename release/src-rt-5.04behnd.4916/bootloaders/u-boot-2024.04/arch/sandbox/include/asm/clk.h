/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef __SANDBOX_CLK_H
#define __SANDBOX_CLK_H

#include <clk.h>
#include <dt-structs.h>
#include <linux/clk-provider.h>

struct udevice;

/**
 * enum sandbox_clk_id - Identity of clocks implemented by the sandbox clock
 * provider.
 *
 * These IDs are within/relative-to the clock provider.
 */
enum sandbox_clk_id {
	SANDBOX_CLK_ID_SPI,
	SANDBOX_CLK_ID_I2C,
	SANDBOX_CLK_ID_UART1,
	SANDBOX_CLK_ID_UART2,
	SANDBOX_CLK_ID_BUS,

	SANDBOX_CLK_ID_COUNT,
};

/**
 * enum sandbox_clk_test_id - Identity of the clocks consumed by the sandbox
 * clock test device.
 *
 * These are the IDs the clock consumer knows the clocks as.
 */
enum sandbox_clk_test_id {
	SANDBOX_CLK_TEST_ID_FIXED,
	SANDBOX_CLK_TEST_ID_SPI,
	SANDBOX_CLK_TEST_ID_I2C,
	SANDBOX_CLK_TEST_ID_I2C_ROOT,
	SANDBOX_CLK_TEST_ID_DEVM1,
	SANDBOX_CLK_TEST_ID_DEVM2,
	SANDBOX_CLK_TEST_ID_DEVM_NULL,

	SANDBOX_CLK_TEST_ID_COUNT,
};

#define SANDBOX_CLK_TEST_NON_DEVM_COUNT SANDBOX_CLK_TEST_ID_DEVM1

struct sandbox_clk_priv {
	bool probed;
	ulong rate[SANDBOX_CLK_ID_COUNT];
	bool enabled[SANDBOX_CLK_ID_COUNT];
	bool requested[SANDBOX_CLK_ID_COUNT];
};

struct sandbox_clk_test {
	struct clk clks[SANDBOX_CLK_TEST_NON_DEVM_COUNT];
	struct clk *clkps[SANDBOX_CLK_TEST_ID_COUNT];
	struct clk_bulk bulk;
};

/* Platform data for the sandbox fixed-rate clock driver */
struct sandbox_clk_fixed_rate_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_sandbox_fixed_clock dtplat;
#endif
	struct clk_fixed_rate fixed;
};

/**
 * sandbox_clk_query_rate - Query the current rate of a sandbox clock.
 *
 * @dev:	The sandbox clock provider device.
 * @id:		The clock to query.
 * @return:	The rate of the clock.
 */
ulong sandbox_clk_query_rate(struct udevice *dev, int id);
/**
 * sandbox_clk_query_enable - Query the enable state of a sandbox clock.
 *
 * @dev:	The sandbox clock provider device.
 * @id:		The clock to query.
 * @return:	The rate of the clock.
 */
int sandbox_clk_query_enable(struct udevice *dev, int id);
/**
 * sandbox_clk_query_requested - Query the requested state of a sandbox clock.
 *
 * @dev:	The sandbox clock provider device.
 * @id:		The clock to query.
 * @return:	The rate of the clock.
 */
int sandbox_clk_query_requested(struct udevice *dev, int id);

/**
 * sandbox_clk_test_get - Ask the sandbox clock test device to request its
 * clocks.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_get(struct udevice *dev);

/**
 * sandbox_clk_test_devm_get - Ask the sandbox clock test device to request its
 * clocks using the managed API.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_devm_get(struct udevice *dev);

/**
 * sandbox_clk_test_get_bulk - Ask the sandbox clock test device to request its
 * clocks with the bulk clk API.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_get_bulk(struct udevice *dev);
/**
 * sandbox_clk_test_get_rate - Ask the sandbox clock test device to query a
 * clock's rate.
 *
 * @dev:	The sandbox clock test (client) device.
 * @id:		The test device's clock ID to query.
 * @return:	The rate of the clock.
 */
ulong sandbox_clk_test_get_rate(struct udevice *dev, int id);
/**
 * sandbox_clk_test_round_rate - Ask the sandbox clock test device to round a
 * clock's rate.
 *
 * @dev:	The sandbox clock test (client) device.
 * @id:		The test device's clock ID to configure.
 * @return:	The rounded rate of the clock.
 */
ulong sandbox_clk_test_round_rate(struct udevice *dev, int id, ulong rate);
/**
 * sandbox_clk_test_set_rate - Ask the sandbox clock test device to set a
 * clock's rate.
 *
 * @dev:	The sandbox clock test (client) device.
 * @id:		The test device's clock ID to configure.
 * @return:	The new rate of the clock.
 */
ulong sandbox_clk_test_set_rate(struct udevice *dev, int id, ulong rate);
/**
 * sandbox_clk_test_enable - Ask the sandbox clock test device to enable a
 * clock.
 *
 * @dev:	The sandbox clock test (client) device.
 * @id:		The test device's clock ID to configure.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_enable(struct udevice *dev, int id);
/**
 * sandbox_clk_test_enable_bulk - Ask the sandbox clock test device to enable
 * all clocks in it's clock bulk struct.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_enable_bulk(struct udevice *dev);
/**
 * sandbox_clk_test_disable - Ask the sandbox clock test device to disable a
 * clock.
 *
 * @dev:	The sandbox clock test (client) device.
 * @id:		The test device's clock ID to configure.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_disable(struct udevice *dev, int id);
/**
 * sandbox_clk_test_disable_bulk - Ask the sandbox clock test device to disable
 * all clocks in it's clock bulk struct.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_disable_bulk(struct udevice *dev);
/**
 * sandbox_clk_test_release_bulk - Ask the sandbox clock test device to release
 * all clocks in it's clock bulk struct.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_release_bulk(struct udevice *dev);
/**
 * sandbox_clk_test_valid - Ask the sandbox clock test device to check its
 * clocks are valid.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
int sandbox_clk_test_valid(struct udevice *dev);
/**
 * sandbox_clk_test_valid - Ask the sandbox clock test device to check its
 * clocks are valid.
 *
 * @dev:	The sandbox clock test (client) device.
 * @return:	0 if OK, or a negative error code.
 */
struct clk *sandbox_clk_test_get_devm_clk(struct udevice *dev, int id);

#endif
