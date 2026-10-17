// SPDX-License-Identifier: GPL-2.0+
/*
 * Sandbox driver for the SOC uclass
 *
 * (C) Copyright 2020 - Texas Instruments Incorporated - https://www.ti.com/
 * Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <soc.h>

int soc_sandbox_get_family(struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "SANDBOX1xx");

	return 0;
}

int soc_sandbox_get_machine(struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "SANDBOX123");

	return 0;
}

int soc_sandbox_get_revision(struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "1.0");

	return 0;
}

static const struct soc_ops soc_sandbox_ops = {
	.get_family = soc_sandbox_get_family,
	.get_revision = soc_sandbox_get_revision,
	.get_machine = soc_sandbox_get_machine,
};

int soc_sandbox_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id soc_sandbox_ids[] = {
	{ .compatible = "sandbox,soc" },
	{ }
};

U_BOOT_DRIVER(soc_sandbox) = {
	.name           = "soc_sandbox",
	.id             = UCLASS_SOC,
	.ops		= &soc_sandbox_ops,
	.of_match       = soc_sandbox_ids,
	.probe          = soc_sandbox_probe,
};
