// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 */

#define DEBUG
#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <asm/arch/power-domain.h>
#include <firmware/imx/sci/sci.h>

static int imx8_power_domain_on(struct power_domain *power_domain)
{
	u32 resource_id = power_domain->id;
	int ret;

	debug("%s: resource_id %u\n", __func__, resource_id);

	ret = sc_pm_set_resource_power_mode(-1, resource_id, SC_PM_PW_MODE_ON);
	if (ret) {
		printf("Error: %u Power up failed! (error = %d)\n",
		       resource_id, ret);
		return ret;
	}

	return 0;
}

static int imx8_power_domain_off(struct power_domain *power_domain)
{
	u32 resource_id = power_domain->id;
	int ret;

	debug("%s: resource_id %u\n", __func__, resource_id);

	ret = sc_pm_set_resource_power_mode(-1, resource_id, SC_PM_PW_MODE_OFF);
	if (ret) {
		printf("Error: %u Power off failed! (error = %d)\n",
		       resource_id, ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id imx8_power_domain_ids[] = {
	{ .compatible = "fsl,imx8qxp-scu-pd" },
	{ .compatible = "fsl,scu-pd" },
	{ }
};

struct power_domain_ops imx8_power_domain_ops_v2 = {
	.on = imx8_power_domain_on,
	.off = imx8_power_domain_off,
};

U_BOOT_DRIVER(imx8_power_domain_v2) = {
	.name = "imx8_power_domain_v2",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = imx8_power_domain_ids,
	.ops = &imx8_power_domain_ops_v2,
};
