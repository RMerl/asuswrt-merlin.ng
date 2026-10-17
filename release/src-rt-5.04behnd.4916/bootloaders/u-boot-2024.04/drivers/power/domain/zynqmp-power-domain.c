// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021, Xilinx. Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <malloc.h>
#include <misc.h>
#include <power-domain-uclass.h>
#include <linux/bitops.h>

#include <zynqmp_firmware.h>

static int zynqmp_pm_request_node(const u32 node, const u32 capabilities,
				  const u32 qos, const enum zynqmp_pm_request_ack ack)
{
	return xilinx_pm_request(PM_REQUEST_NODE, node, capabilities,
				   qos, ack, NULL);
}

static int zynqmp_power_domain_request(struct power_domain *power_domain)
{
	int ret = 0;

	dev_dbg(power_domain->dev, "Request for id: %ld\n", power_domain->id);

	if (IS_ENABLED(CONFIG_ARCH_ZYNQMP)) {
		ret = zynqmp_pmufw_node(power_domain->id);
		if (ret == -ENODEV)
			ret = 0;
	}

	return ret;
}

static int zynqmp_power_domain_free(struct power_domain *power_domain)
{
	/* nop now */
	return 0;
}

static int zynqmp_power_domain_on(struct power_domain *power_domain)
{
	dev_dbg(power_domain->dev, "Domain ON for id: %ld\n", power_domain->id);

	return zynqmp_pm_request_node(power_domain->id,
				      ZYNQMP_PM_CAPABILITY_ACCESS,
				      ZYNQMP_PM_MAX_QOS,
				      ZYNQMP_PM_REQUEST_ACK_BLOCKING);
}

static int zynqmp_power_domain_off(struct power_domain *power_domain)
{
	/* nop now */
	return 0;
}

struct power_domain_ops zynqmp_power_domain_ops = {
	.request = zynqmp_power_domain_request,
	.rfree = zynqmp_power_domain_free,
	.on = zynqmp_power_domain_on,
	.off = zynqmp_power_domain_off,
};

static int zynqmp_power_domain_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(zynqmp_power_domain) = {
	.name = "zynqmp_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.probe = zynqmp_power_domain_probe,
	.ops = &zynqmp_power_domain_ops,
};
