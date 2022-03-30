// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <power-domain.h>
#include <asm/io.h>
#include <asm/power-domain.h>

struct sandbox_power_domain_test {
	struct power_domain pd;
};

int sandbox_power_domain_test_get(struct udevice *dev)
{
	struct sandbox_power_domain_test *sbrt = dev_get_priv(dev);

	return power_domain_get(dev, &sbrt->pd);
}

int sandbox_power_domain_test_on(struct udevice *dev)
{
	struct sandbox_power_domain_test *sbrt = dev_get_priv(dev);

	return power_domain_on(&sbrt->pd);
}

int sandbox_power_domain_test_off(struct udevice *dev)
{
	struct sandbox_power_domain_test *sbrt = dev_get_priv(dev);

	return power_domain_off(&sbrt->pd);
}

int sandbox_power_domain_test_free(struct udevice *dev)
{
	struct sandbox_power_domain_test *sbrt = dev_get_priv(dev);

	return power_domain_free(&sbrt->pd);
}

static const struct udevice_id sandbox_power_domain_test_ids[] = {
	{ .compatible = "sandbox,power-domain-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_power_domain_test) = {
	.name = "sandbox_power_domain_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_power_domain_test_ids,
	.priv_auto_alloc_size = sizeof(struct sandbox_power_domain_test),
};
