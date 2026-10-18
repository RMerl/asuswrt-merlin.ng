/*
 * Copyright © 2015-2019 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#define pr_fmt(fmt) "bmem: " fmt

#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/libfdt.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/brcmstb/bmem.h>
#include <linux/brcmstb/memory_api.h>

#if 0
#define DBG pr_info
#else
#define DBG(...) /* */
#endif

#define MAX_BMEM_REGIONS	8

struct bmem_region {
	phys_addr_t		addr;
	phys_addr_t		size;
	bool			valid;
};

static struct bmem_region bmem_regions[MAX_BMEM_REGIONS];
static struct platform_device *brcmstb_pdev;
static unsigned int n_bmem_regions;
static bool bmem_disabled;

/***********************************************************************
 * BMEM (reserved A/V buffer memory) support
 ***********************************************************************/

static int __init __bmem_setup(phys_addr_t addr, phys_addr_t size)
{
	int i;

	for (i = 0; i < n_bmem_regions; i++) {
		if (addr >= bmem_regions[i].addr + bmem_regions[i].size)
			continue;
		if (addr + size <= bmem_regions[i].addr)
			continue;
		pr_warn("ignoring region 0x%llx-0x%llx, overlaps existing region\n",
			(unsigned long long)addr,
			(unsigned long long)(addr + size));
		return -EINVAL;
	}

	if (n_bmem_regions == MAX_BMEM_REGIONS) {
		pr_warn_once("too many regions, ignoring extras\n");
		return -E2BIG;
	}

	bmem_regions[n_bmem_regions].addr = addr;
	bmem_regions[n_bmem_regions].size = size;
	n_bmem_regions++;
	return 0;
}

/*
 * Parses command line for bmem= options
 */
static int __init bmem_setup(char *str)
{
	phys_addr_t addr = 0, size;
	char *orig_str = str;
	int ret;

	/* allow bmem=bmem but ignore */
	if (!memcmp(str, "bmem", 5))
		return 0;

	if (!memcmp(str, "bhpa", 5)) {
		brcmstb_bmem_is_bhpa = true;
		return 0;
	}

	size = memparse(str, &str);
	if (*str == '@')
		addr = memparse(str + 1, &str);

	if ((addr & ~PAGE_MASK) || (size & ~PAGE_MASK)) {
		pr_warn("ignoring invalid range '%s' (is it missing an 'M' suffix?)\n",
				orig_str);
		return 0;
	}

	if (size == 0) {
		pr_info("disabling reserved memory\n");
		bmem_disabled = true;
		return 0;
	}

	if (addr < memblock_start_of_DRAM()) {
		pr_warn("ignoring invalid range '%s' below addressable DRAM\n",
			orig_str);
		return 0;
	}

	ret = __bmem_setup(addr, size);
	if (!ret)
		brcmstb_memory_override_defaults = true;
	return ret;
}
early_param("bmem", bmem_setup);

/*
 * Returns index if the supplied range falls entirely within a bmem region
 */
int bmem_find_region(phys_addr_t addr, phys_addr_t size)
{
	int i, idx = 0;

	for (i = 0; i < n_bmem_regions; i++) {
		if (!bmem_regions[i].valid)
			continue;
		if ((addr >= bmem_regions[i].addr) &&
		    ((addr + size) <=
			(bmem_regions[i].addr + bmem_regions[i].size))) {
			return idx;
		}
		idx++;
	}
	return -ENOENT;
}

/*
 * Finds the IDX'th valid bmem region, and fills in addr/size if it exists.
 * Returns 0 on success, <0 on failure.
 * Can pass in NULL for addr and/or size if you only care about return value.
 */
int bmem_region_info(int idx, phys_addr_t *addr, phys_addr_t *size)
{
	int i;

	for (i = 0; i < n_bmem_regions; i++) {
		if (!bmem_regions[i].valid)
			continue;
		if (!idx) {
			if (addr)
				*addr = bmem_regions[i].addr;
			if (size)
				*size = bmem_regions[i].size;
			return 0;
		}
		idx--;
	}
	return -ENOENT;
}
EXPORT_SYMBOL(bmem_region_info);

void __init bmem_reserve(void (*setup)(phys_addr_t addr, phys_addr_t size))
{
	const void *fdt = initial_boot_params;
	phys_addr_t guard = 0;
	int i, ret, offset;

	if (bmem_disabled) {
		n_bmem_regions = 0;
		return;
	}

	if (brcmstb_default_reserve == BRCMSTB_RESERVE_BMEM &&
			!n_bmem_regions &&
			!brcmstb_memory_override_defaults)
		brcmstb_memory_default_reserve(__bmem_setup);

	/* Reassign BMEM to another memory type */
	if (setup) {
		for (i = 0; i < n_bmem_regions; ++i) {
			setup(bmem_regions[i].addr, bmem_regions[i].size);
		}
		n_bmem_regions = 0;
		return;
	}

	if (fdt) {
		/*
		 * Reserve the PAGE_SIZE memory preceeding each
		 * BMEM region so it's unusable by the kernel.
		 * This is to workaround a bug in the USB hardware
		 * that may pre-fetch beyond the end of a DMA buffer
		 * and read into BMEM and cause MRC errors.
		 * See: SWLINUX-3996.
		 */
		offset = fdt_node_offset_by_compatible(fdt, -1,
			"brcm,ehci-brcm-v2");
		if (offset >= 0)
			guard = PAGE_SIZE;
	}

	for (i = 0; i < n_bmem_regions; ++i) {
		ret = memblock_reserve(bmem_regions[i].addr - guard,
				bmem_regions[i].size + guard);
		if (ret) {
			pr_err("memblock_reserve(%pa, %pa) failed: %d\n",
					&bmem_regions[i].addr,
					&bmem_regions[i].size, ret);
#ifdef CONFIG_BRCMSTB_AUTOMAP_BMEM
		} else if (memblock_mark_automap(bmem_regions[i].addr,
						 bmem_regions[i].size)) {
			pr_err("memblock_mark_automap(%pa, %pa) failed\n",
			       &bmem_regions[i].addr,
			       &bmem_regions[i].size);
#endif
		} else {
			bmem_regions[i].valid = true;
			pr_info("Reserved %lu MiB at %pa\n",
				(unsigned long) bmem_regions[i].size / SZ_1M,
				&bmem_regions[i].addr);
		}
	}
}

/*
 * Create /proc/iomem entries for bmem
 */
static int __init bmem_region_setup(void)
{
	int i, idx = 0;

	for (i = 0; i < n_bmem_regions; i++) {
		struct resource *r;
		char *name;

		if (!bmem_regions[i].valid)
			continue;

		r = kzalloc(sizeof(*r), GFP_KERNEL);
		name = kzalloc(16, GFP_KERNEL);
		if (!r || !name)
			break;

		sprintf(name, "bmem.%d", idx);
		r->start = bmem_regions[i].addr;
		r->end = bmem_regions[i].addr + bmem_regions[i].size - 1;
		r->flags = IORESOURCE_MEM;
		r->name = name;

		insert_resource(&iomem_resource, r);
		idx++;
	}
	return 0;
}
arch_initcall(bmem_region_setup);

static ssize_t show_bmem(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long idx = 0;
	phys_addr_t addr = 0, size = 0;
	const char *name = attr->attr.name;
	int ret;

	while (*name != 0) {
		if (isdigit(*name)) {
			ret = kstrtoul(name, 10, &idx);
			if (ret)
				return ret;
			break;
		}
		name++;
	}
	bmem_region_info(idx, &addr, &size);

	return snprintf(buf, PAGE_SIZE, "%pa %pa\n", &addr, &size);
}

static int __init brcm_pdev_init(void)
{
	struct device *dev;
	int i;

	brcmstb_pdev = platform_device_alloc("brcmstb", -1);
	if (brcmstb_pdev == NULL) {
		pr_err("%s: can't allocate device\n", __func__);
		return -ENODEV;
	}
	platform_device_add(brcmstb_pdev);
	dev = &brcmstb_pdev->dev;

	/* create an attribute for each bmem region */
	for (i = 0; ; i++) {
		phys_addr_t addr, size;
		struct device_attribute *attr;
		char *name;

		if (bmem_region_info(i, &addr, &size) < 0)
			break;
		attr = kzalloc(sizeof(*attr), GFP_KERNEL);
		if (attr == NULL)
			break;

		name = kzalloc(16, GFP_KERNEL);
		if (name == NULL)
			break;
		snprintf(name, 16, "bmem.%d", i);

		sysfs_attr_init(&attr->attr);
		attr->attr.name = name;
		attr->attr.mode = 0444;
		attr->show = show_bmem;

		if (device_create_file(dev, attr) != 0)
			WARN(1, "Can't create sysfs file\n");
	}

	return 0;
}
arch_initcall(brcm_pdev_init);
