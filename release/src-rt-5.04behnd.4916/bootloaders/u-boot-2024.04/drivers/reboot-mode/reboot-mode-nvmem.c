// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <nvmem.h>
#include <reboot-mode/reboot-mode.h>

/**
 * struct nvmem_reboot_mode_priv - Private data for the nvmem reboot mode device
 * @cell: The nvmem cell to store the mode in
 */
struct nvmem_reboot_mode_priv {
	struct nvmem_cell cell;
};

static int reboot_mode_get(struct udevice *dev, u32 *mode)
{
	struct nvmem_reboot_mode_priv *priv = dev_get_priv(dev);

	return nvmem_cell_read(&priv->cell, mode, sizeof(*mode));
}

static int reboot_mode_set(struct udevice *dev, u32 mode)
{
	struct nvmem_reboot_mode_priv *priv = dev_get_priv(dev);

	return nvmem_cell_write(&priv->cell, &mode, sizeof(mode));
}

static const struct reboot_mode_ops nvmem_reboot_mode_ops = {
	.get = reboot_mode_get,
	.set = reboot_mode_set,
};

static int reboot_mode_probe(struct udevice *dev)
{
	struct nvmem_reboot_mode_priv *priv = dev_get_priv(dev);

	return nvmem_cell_get_by_name(dev, "reboot-mode", &priv->cell);
}

static const struct udevice_id nvmem_reboot_mode_ids[] = {
	{ .compatible = "nvmem-reboot-mode" },
	{ }
};

U_BOOT_DRIVER(nvmem_reboot_mode) = {
	.name = "nvmem-reboot-mode",
	.id = UCLASS_REBOOT_MODE,
	.of_match = nvmem_reboot_mode_ids,
	.probe = reboot_mode_probe,
	.priv_auto = sizeof(struct nvmem_reboot_mode_priv),
	.ops = &nvmem_reboot_mode_ops,
};
