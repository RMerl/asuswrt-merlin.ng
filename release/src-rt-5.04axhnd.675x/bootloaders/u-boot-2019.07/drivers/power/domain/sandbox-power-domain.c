// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <power-domain-uclass.h>
#include <asm/io.h>
#include <asm/power-domain.h>

#define SANDBOX_POWER_DOMAINS 3

struct sandbox_power_domain {
	bool on[SANDBOX_POWER_DOMAINS];
};

static int sandbox_power_domain_request(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	if (power_domain->id >= SANDBOX_POWER_DOMAINS)
		return -EINVAL;

	return 0;
}

static int sandbox_power_domain_free(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return 0;
}

static int sandbox_power_domain_on(struct power_domain *power_domain)
{
	struct sandbox_power_domain *sbr = dev_get_priv(power_domain->dev);

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	sbr->on[power_domain->id] = true;

	return 0;
}

static int sandbox_power_domain_off(struct power_domain *power_domain)
{
	struct sandbox_power_domain *sbr = dev_get_priv(power_domain->dev);

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	sbr->on[power_domain->id] = false;

	return 0;
}

static int sandbox_power_domain_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int sandbox_power_domain_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static const struct udevice_id sandbox_power_domain_ids[] = {
	{ .compatible = "sandbox,power-domain" },
	{ }
};

struct power_domain_ops sandbox_power_domain_ops = {
	.request = sandbox_power_domain_request,
	.free = sandbox_power_domain_free,
	.on = sandbox_power_domain_on,
	.off = sandbox_power_domain_off,
};

U_BOOT_DRIVER(sandbox_power_domain) = {
	.name = "sandbox_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = sandbox_power_domain_ids,
	.bind = sandbox_power_domain_bind,
	.probe = sandbox_power_domain_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_power_domain),
	.ops = &sandbox_power_domain_ops,
};

int sandbox_power_domain_query(struct udevice *dev, unsigned long id)
{
	struct sandbox_power_domain *sbr = dev_get_priv(dev);

	debug("%s(dev=%p, id=%ld)\n", __func__, dev, id);

	if (id >= SANDBOX_POWER_DOMAINS)
		return -EINVAL;

	return sbr->on[id];
}
