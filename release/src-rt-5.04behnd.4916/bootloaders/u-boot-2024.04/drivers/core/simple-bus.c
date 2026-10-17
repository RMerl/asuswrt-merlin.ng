// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY UCLASS_SIMPLE_BUS

#include <common.h>
#include <asm/global_data.h>
#include <dm.h>
#include <dm/simple_bus.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

fdt_addr_t simple_bus_translate(struct udevice *dev, fdt_addr_t addr)
{
	struct simple_bus_plat *plat = dev_get_uclass_plat(dev);

	if (addr >= plat->base && addr < plat->base + plat->size)
		addr = (addr - plat->base) + plat->target;

	return addr;
}

static int simple_bus_post_bind(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	return 0;
#else
	struct simple_bus_plat *plat = dev_get_uclass_plat(dev);
	int ret;

	if (CONFIG_IS_ENABLED(SIMPLE_BUS_CORRECT_RANGE)) {
		uint64_t caddr, paddr, len;

		/* only read range index 0 */
		ret = fdt_read_range((void *)gd->fdt_blob, dev_of_offset(dev),
				     0, &caddr, &paddr, &len);
		if (!ret) {
			plat->base = caddr;
			plat->target = paddr;
			plat->size = len;
		}
	} else {
		u32 cell[3];

		ret = dev_read_u32_array(dev, "ranges", cell,
					 ARRAY_SIZE(cell));
		if (!ret) {
			plat->base = cell[0];
			plat->target = cell[1];
			plat->size = cell[2];
		}
	}

	return dm_scan_fdt_dev(dev);
#endif
}

UCLASS_DRIVER(simple_bus) = {
	.id		= UCLASS_SIMPLE_BUS,
	.name		= "simple_bus",
	.post_bind	= simple_bus_post_bind,
	.per_device_plat_auto	= sizeof(struct simple_bus_plat),
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id generic_simple_bus_ids[] = {
	{ .compatible = "simple-bus" },
	{ .compatible = "simple-mfd" },
	{ }
};
#endif

U_BOOT_DRIVER(simple_bus) = {
	.name	= "simple_bus",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = of_match_ptr(generic_simple_bus_ids),
	.flags	= DM_FLAG_PRE_RELOC,
};
