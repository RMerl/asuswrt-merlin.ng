// SPDX-License-Identifier: GPL-2.0+
/*
 * Device addresses
 *
 * Copyright (c) 2017 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

fdt_addr_t devfdt_get_addr_index(struct udevice *dev, int index)
{
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	fdt_addr_t addr;

	if (CONFIG_IS_ENABLED(OF_TRANSLATE)) {
		const fdt32_t *reg;
		int len = 0;
		int na, ns;

		na = fdt_address_cells(gd->fdt_blob,
				       dev_of_offset(dev->parent));
		if (na < 1) {
			debug("bad #address-cells\n");
			return FDT_ADDR_T_NONE;
		}

		ns = fdt_size_cells(gd->fdt_blob, dev_of_offset(dev->parent));
		if (ns < 0) {
			debug("bad #size-cells\n");
			return FDT_ADDR_T_NONE;
		}

		reg = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "reg",
				  &len);
		if (!reg || (len <= (index * sizeof(fdt32_t) * (na + ns)))) {
			debug("Req index out of range\n");
			return FDT_ADDR_T_NONE;
		}

		reg += index * (na + ns);

		if (ns) {
			/*
			 * Use the full-fledged translate function for complex
			 * bus setups.
			 */
			addr = fdt_translate_address((void *)gd->fdt_blob,
						     dev_of_offset(dev), reg);
		} else {
			/* Non translatable if #size-cells == 0 */
			addr = fdt_read_number(reg, na);
		}
	} else {
		/*
		 * Use the "simple" translate function for less complex
		 * bus setups.
		 */
		addr = fdtdec_get_addr_size_auto_parent(gd->fdt_blob,
				dev_of_offset(dev->parent), dev_of_offset(dev),
				"reg", index, NULL, false);
		if (CONFIG_IS_ENABLED(SIMPLE_BUS) && addr != FDT_ADDR_T_NONE) {
			if (device_get_uclass_id(dev->parent) ==
			    UCLASS_SIMPLE_BUS)
				addr = simple_bus_translate(dev->parent, addr);
		}
	}

#if defined(CONFIG_TRANSLATION_OFFSET)
	/*
	 * Some platforms need a special address translation. Those
	 * platforms (e.g. mvebu in SPL) can configure a translation
	 * offset by setting this value in the GD and enaling this
	 * feature via CONFIG_TRANSLATION_OFFSET. This value will
	 * get added to all addresses returned by devfdt_get_addr().
	 */
	addr += gd->translation_offset;
#endif

	return addr;
#else
	return FDT_ADDR_T_NONE;
#endif
}

fdt_addr_t devfdt_get_addr_size_index(struct udevice *dev, int index,
				   fdt_size_t *size)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	/*
	 * Only get the size in this first call. We'll get the addr in the
	 * next call to the exisiting dev_get_xxx function which handles
	 * all config options.
	 */
	fdtdec_get_addr_size_auto_noparent(gd->fdt_blob, dev_of_offset(dev),
					   "reg", index, size, false);

	/*
	 * Get the base address via the existing function which handles
	 * all Kconfig cases
	 */
	return devfdt_get_addr_index(dev, index);
#else
	return FDT_ADDR_T_NONE;
#endif
}

fdt_addr_t devfdt_get_addr_name(struct udevice *dev, const char *name)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	int index;

	index = fdt_stringlist_search(gd->fdt_blob, dev_of_offset(dev),
				      "reg-names", name);
	if (index < 0)
		return index;

	return devfdt_get_addr_index(dev, index);
#else
	return FDT_ADDR_T_NONE;
#endif
}

fdt_addr_t devfdt_get_addr(struct udevice *dev)
{
	return devfdt_get_addr_index(dev, 0);
}

void *devfdt_get_addr_ptr(struct udevice *dev)
{
	return (void *)(uintptr_t)devfdt_get_addr_index(dev, 0);
}

void *devfdt_remap_addr_index(struct udevice *dev, int index)
{
	fdt_addr_t addr = devfdt_get_addr_index(dev, index);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

void *devfdt_remap_addr_name(struct udevice *dev, const char *name)
{
	fdt_addr_t addr = devfdt_get_addr_name(dev, name);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

void *devfdt_remap_addr(struct udevice *dev)
{
	return devfdt_remap_addr_index(dev, 0);
}

void *devfdt_map_physmem(struct udevice *dev, unsigned long size)
{
	fdt_addr_t addr = devfdt_get_addr(dev);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, size, MAP_NOCACHE);
}
