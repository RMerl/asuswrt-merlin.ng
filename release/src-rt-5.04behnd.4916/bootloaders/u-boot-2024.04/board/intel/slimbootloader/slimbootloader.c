// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <init.h>

int board_early_init_r(void)
{
	/*
	 * Make sure PCI bus is enumerated so that peripherals on the PCI bus
	 * can be discovered by their drivers.
	 *
	 * Slim Bootloader has already done PCI bus enumeration before loading
	 * U-Boot, so U-Boot needs to preserve PCI configuration.
	 * Therefore, '# CONFIG_PCI_PNP is not set' is included in defconfig.
	 */
	pci_init();

	return 0;
}
