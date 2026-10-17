// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 */

#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <fdtdec.h>

static int atmel_ebi_probe(struct udevice *dev)
{
	int ret;
	struct udevice *ndev;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(atmel_nand_controller),
					  &ndev);
	if (ret)
		printf("Failed to probe nand driver (err = %d)\n", ret);

	return ret;
}

static const struct udevice_id atmel_ebi_match[] = {
	{.compatible = "microchip,sam9x60-ebi"},
	{.compatible = "atmel,sama5d3-ebi"},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(atmel_ebi) = {
	.name = "atmel_ebi",
	.id = UCLASS_NOP,
	.of_match = atmel_ebi_match,
	.probe = atmel_ebi_probe,
	.bind = dm_scan_fdt_dev,
};
