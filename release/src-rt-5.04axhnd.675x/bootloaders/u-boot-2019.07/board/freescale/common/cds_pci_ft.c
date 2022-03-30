// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004 Freescale Semiconductor.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include "cadmus.h"

#if defined(CONFIG_OF_BOARD_SETUP)
static void cds_pci_fixup(void *blob)
{
	int node;
	const char *path;
	int len, slot, i;
	u32 *map = NULL, *piccells = NULL;
	int off, cells;

	node = fdt_path_offset(blob, "/aliases");
	if (node >= 0) {
		path = fdt_getprop(blob, node, "pci0", NULL);
		if (path) {
			node = fdt_path_offset(blob, path);
			if (node >= 0) {
				map = fdt_getprop_w(blob, node, "interrupt-map", &len);
			}
			/* Each item in "interrupt-map" property is translated with
			 * following cells:
			 * PCI #address-cells, PCI #interrupt-cells,
			 * PIC address, PIC #address-cells, PIC #interrupt-cells.
			 */
			cells = fdt_getprop_u32_default(blob, path, "#address-cells", 1);
			cells += fdt_getprop_u32_default(blob, path, "#interrupt-cells", 1);
			off = fdt_node_offset_by_phandle(blob, fdt32_to_cpu(*(map+cells)));
			if (off <= 0)
				return;
			cells += 1;
			piccells = (u32 *)fdt_getprop(blob, off, "#address-cells", NULL);
			if (piccells == NULL)
				return;
			cells += *piccells;
			piccells = (u32 *)fdt_getprop(blob, off, "#interrupt-cells", NULL);
			if (piccells == NULL)
				return;
			cells += *piccells;
		}
	}

	if (map) {
		len /= sizeof(u32);

		slot = get_pci_slot();

		for (i=0;i<len;i+=cells) {
			/* We rotate the interrupt pins so that the mapping
			 * changes depending on the slot the carrier card is in.
			 */
			map[3] = ((map[3] + slot - 2) % 4) + 1;
			map+=cells;
		}
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
	cds_pci_fixup(blob);
#endif

	return 0;
}
#endif
