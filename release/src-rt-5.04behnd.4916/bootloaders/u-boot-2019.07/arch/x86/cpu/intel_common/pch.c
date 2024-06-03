// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <asm/pch_common.h>

u32 pch_common_sir_read(struct udevice *dev, int idx)
{
	u32 data;

	dm_pci_write_config32(dev, SATA_SIRI, idx);
	dm_pci_read_config32(dev, SATA_SIRD, &data);

	return data;
}

void pch_common_sir_write(struct udevice *dev, int idx, u32 value)
{
	dm_pci_write_config32(dev, SATA_SIRI, idx);
	dm_pci_write_config32(dev, SATA_SIRD, value);
}
