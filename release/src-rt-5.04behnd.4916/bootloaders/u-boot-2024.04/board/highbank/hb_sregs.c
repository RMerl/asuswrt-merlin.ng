// SPDX-License-Identifier: GPL-2.0+
/*
 * Calxeda Highbank/Midway "system registers" bus driver
 *
 * There is a "clocks" subnode inside the top node, which groups all clocks,
 * both programmable PLLs as well as fixed clocks.
 * Simple allow the DT enumeration to look inside this node, so that we can
 * read the fixed clock frequencies using the DM clock framework.
 *
 * Copyright (C) 2019 Arm Ltd.
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>

static int hb_sregs_scan_fdt_dev(struct udevice *dev)
{
	ofnode clock_node, node;

	/* Search for subnode called "clocks". */
	ofnode_for_each_subnode(clock_node, dev_ofnode(dev)) {
		if (!ofnode_name_eq(clock_node, "clocks"))
			continue;

		/* Enumerate all nodes inside this "clocks" subnode. */
		ofnode_for_each_subnode(node, clock_node)
			lists_bind_fdt(dev, node, NULL, NULL, false);
		return 0;
	}

	return -ENOENT;
}

static const struct udevice_id highbank_sreg_ids[] = {
	{ .compatible = "calxeda,hb-sregs" },
	{}
};

U_BOOT_DRIVER(hb_sregs) = {
	.name = "hb-sregs",
	.id = UCLASS_SIMPLE_BUS,
	.bind = hb_sregs_scan_fdt_dev,
	.of_match = highbank_sreg_ids,
};
