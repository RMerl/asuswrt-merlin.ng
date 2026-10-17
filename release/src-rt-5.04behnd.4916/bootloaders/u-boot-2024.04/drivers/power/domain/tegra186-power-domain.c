// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <misc.h>
#include <power-domain-uclass.h>
#include <asm/arch-tegra/bpmp_abi.h>
#include <linux/bitops.h>

#define UPDATE	BIT(0)
#define ON	BIT(1)

static int tegra186_power_domain_common(struct power_domain *power_domain,
					bool on)
{
	struct mrq_pg_update_state_request req;
	int on_state = on ? ON : 0;
	int ret;

	req.partition_id = power_domain->id;
	req.logic_state = UPDATE | on_state;
	req.sram_state = UPDATE | on_state;
	/*
	 * Drivers manage their own clocks so they don't get out of sync, and
	 * since some power domains have many clocks, only a subset of which
	 * are actually needed depending on use-case.
	 */
	req.clock_state = UPDATE;

	ret = misc_call(power_domain->dev->parent, MRQ_PG_UPDATE_STATE, &req,
			sizeof(req), NULL, 0);
	if (ret < 0)
		return ret;

	return 0;
}

static int tegra186_power_domain_on(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p) (dev=%p, id=%lu)\n", __func__,
	      power_domain, power_domain->dev, power_domain->id);

	return tegra186_power_domain_common(power_domain, true);
}

static int tegra186_power_domain_off(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p) (dev=%p, id=%lu)\n", __func__,
	      power_domain, power_domain->dev, power_domain->id);

	return tegra186_power_domain_common(power_domain, false);
}

struct power_domain_ops tegra186_power_domain_ops = {
	.on = tegra186_power_domain_on,
	.off = tegra186_power_domain_off,
};

U_BOOT_DRIVER(tegra186_power_domain) = {
	.name = "tegra186_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &tegra186_power_domain_ops,
};
