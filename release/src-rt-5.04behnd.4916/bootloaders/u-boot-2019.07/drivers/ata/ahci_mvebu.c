// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_ahci_enable(void)
{
	return 0;
}

static int mvebu_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;
	int ret;

	ret = ahci_bind_scsi(dev, &scsi_dev);
	if (ret) {
		debug("%s: Failed to bind (err=%d\n)", __func__, ret);
		return ret;
	}

	return 0;
}

static int mvebu_ahci_probe(struct udevice *dev)
{
	/*
	 * Board specific SATA / AHCI enable code, e.g. enable the
	 * AHCI power or deassert reset
	 */
	board_ahci_enable();

	ahci_probe_scsi(dev, (ulong)devfdt_get_addr_ptr(dev));

	return 0;
}

static const struct udevice_id mvebu_ahci_ids[] = {
	{ .compatible = "marvell,armada-380-ahci" },
	{ .compatible = "marvell,armada-3700-ahci" },
	{ .compatible = "marvell,armada-8k-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_mvebu_drv) = {
	.name		= "ahci_mvebu",
	.id		= UCLASS_AHCI,
	.of_match	= mvebu_ahci_ids,
	.bind		= mvebu_ahci_bind,
	.probe		= mvebu_ahci_probe,
};
