// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2007
 *
 * Author: Scott Wood <scottwood@freescale.com>,
 * with some bits from older board-specific PCI initialization.
 */

#include <common.h>
#include <init.h>
#include <pci.h>
#include <asm/bitops.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#if defined(CONFIG_OF_LIBFDT)
#include <linux/libfdt.h>
#include <fdt_support.h>
#endif

#include <asm/mpc8349_pci.h>

#define MAX_BUSES 2

DECLARE_GLOBAL_DATA_PTR;

static struct pci_controller pci_hose[MAX_BUSES];
static int pci_num_buses;

#if defined(CONFIG_OF_LIBFDT)
void ft_pci_setup(void *blob, struct bd_info *bd)
{
	int nodeoffset;
	int tmp[2];
	const char *path;

	if (pci_num_buses < 1)
		return;

	nodeoffset = fdt_path_offset(blob, "/aliases");
	if (nodeoffset >= 0) {
		path = fdt_getprop(blob, nodeoffset, "pci0", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(pci_hose[0].first_busno);
			tmp[1] = cpu_to_be32(pci_hose[0].last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);

			tmp[0] = cpu_to_be32(gd->pci_clk);
			do_fixup_by_path(blob, path, "clock-frequency",
				&tmp, sizeof(tmp[0]), 1);
		}

		if (pci_num_buses < 2)
			return;

		path = fdt_getprop(blob, nodeoffset, "pci1", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(pci_hose[1].first_busno);
			tmp[1] = cpu_to_be32(pci_hose[1].last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);

			tmp[0] = cpu_to_be32(gd->pci_clk);
			do_fixup_by_path(blob, path, "clock-frequency",
				&tmp, sizeof(tmp[0]), 1);
		}
	}
}
#endif /* CONFIG_OF_LIBFDT */
