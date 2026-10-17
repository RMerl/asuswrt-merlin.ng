// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2021, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/scmi_test.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

/*
 * Simulate to some extent a SCMI exchange.
 * This drivers gets SCMI resources and offers API function to the
 * SCMI test sequence manipulate the resources, currently clock
 * and reset controllers.
 */

#define SCMI_TEST_DEVICES_CLK_COUNT		2
#define SCMI_TEST_DEVICES_RD_COUNT		1
#define SCMI_TEST_DEVICES_VOLTD_COUNT		2

/*
 * struct sandbox_scmi_device_priv - Storage for device handles used by test
 * @pwdom:		Power domain device
 * @clk:		Array of clock instances used by tests
 * @reset_clt:		Array of the reset controller instances used by tests
 * @regulators:		Array of regulator device references used by the tests
 * @devices:		Resources exposed by sandbox_scmi_devices_ctx()
 */
struct sandbox_scmi_device_priv {
	struct power_domain pwdom;
	struct clk clk[SCMI_TEST_DEVICES_CLK_COUNT];
	struct reset_ctl reset_ctl[SCMI_TEST_DEVICES_RD_COUNT];
	struct udevice *regulators[SCMI_TEST_DEVICES_VOLTD_COUNT];
	struct sandbox_scmi_devices devices;
};

struct sandbox_scmi_devices *sandbox_scmi_devices_ctx(struct udevice *dev)
{
	struct sandbox_scmi_device_priv *priv = dev_get_priv(dev);

	if (priv)
		return &priv->devices;

	return NULL;
}

static int sandbox_scmi_devices_remove(struct udevice *dev)
{
	struct sandbox_scmi_devices *devices = sandbox_scmi_devices_ctx(dev);
	int ret = 0;
	size_t n;

	if (!devices)
		return 0;

	if (CONFIG_IS_ENABLED(RESET_SCMI))
		for (n = 0; n < SCMI_TEST_DEVICES_RD_COUNT; n++) {
			int ret2 = reset_free(devices->reset + n);

			if (ret2 && !ret)
				ret = ret2;
		}

	return ret;
}

static int sandbox_scmi_devices_probe(struct udevice *dev)
{
	struct sandbox_scmi_device_priv *priv = dev_get_priv(dev);
	int ret;
	size_t n;

	priv->devices = (struct sandbox_scmi_devices){
		.pwdom = &priv->pwdom,
		.pwdom_count = 1,
		.clk = priv->clk,
		.clk_count = SCMI_TEST_DEVICES_CLK_COUNT,
		.reset = priv->reset_ctl,
		.reset_count = SCMI_TEST_DEVICES_RD_COUNT,
		.regul = priv->regulators,
		.regul_count = SCMI_TEST_DEVICES_VOLTD_COUNT,
	};

	if (CONFIG_IS_ENABLED(SCMI_POWER_DOMAIN)) {
		ret = power_domain_get_by_index(dev, priv->devices.pwdom, 0);
		if (ret) {
			dev_err(dev, "%s: Failed on power domain\n", __func__);
			return ret;
		}
	}

	if (CONFIG_IS_ENABLED(CLK_SCMI)) {
		for (n = 0; n < SCMI_TEST_DEVICES_CLK_COUNT; n++) {
			ret = clk_get_by_index(dev, n, priv->devices.clk + n);
			if (ret) {
				dev_err(dev, "%s: Failed on clk %zu\n",
					__func__, n);
				return ret;
			}
		}
	}

	if (CONFIG_IS_ENABLED(RESET_SCMI)) {
		for (n = 0; n < SCMI_TEST_DEVICES_RD_COUNT; n++) {
			ret = reset_get_by_index(dev, n,
						 priv->devices.reset + n);
			if (ret) {
				dev_err(dev, "%s: Failed on reset %zu\n",
					__func__, n);
				goto err_reset;
			}
		}
	}

	if (CONFIG_IS_ENABLED(DM_REGULATOR_SCMI)) {
		for (n = 0; n < SCMI_TEST_DEVICES_VOLTD_COUNT; n++) {
			char name[32];

			ret = snprintf(name, sizeof(name), "regul%zu-supply",
				       n);
			assert(ret >= 0 && ret < sizeof(name));

			ret = device_get_supply_regulator(dev, name,
							  priv->devices.regul
								+ n);
			if (ret) {
				dev_err(dev, "%s: Failed on voltd %zu\n",
					__func__, n);
				goto err_regul;
			}
		}
	}

	return 0;

err_regul:
	n = SCMI_TEST_DEVICES_RD_COUNT;
err_reset:
	if (CONFIG_IS_ENABLED(RESET_SCMI))
		for (; n > 0; n--)
			reset_free(priv->devices.reset + n - 1);

	return ret;
}

static const struct udevice_id sandbox_scmi_devices_ids[] = {
	{ .compatible = "sandbox,scmi-devices" },
	{ }
};

U_BOOT_DRIVER(sandbox_scmi_devices) = {
	.name = "sandbox-scmi_devices",
	.id = UCLASS_MISC,
	.of_match = sandbox_scmi_devices_ids,
	.priv_auto = sizeof(struct sandbox_scmi_device_priv),
	.remove = sandbox_scmi_devices_remove,
	.probe = sandbox_scmi_devices_probe,
};
