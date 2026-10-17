// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PCI

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#if defined(CONFIG_X86) && defined(CONFIG_HAVE_FSP)
#include <asm/fsp/fsp_support.h>
#endif
#include <dt-bindings/pci/pci.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include "pci_internal.h"

DECLARE_GLOBAL_DATA_PTR;

int pci_get_bus(int busnum, struct udevice **busp)
{
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PCI, busnum, busp);

	/* Since buses may not be numbered yet try a little harder with bus 0 */
	if (ret == -ENODEV) {
		ret = uclass_first_device_err(UCLASS_PCI, busp);
		if (ret)
			return ret;
		ret = uclass_get_device_by_seq(UCLASS_PCI, busnum, busp);
	}

	return ret;
}

struct udevice *pci_get_controller(struct udevice *dev)
{
	while (device_is_on_pci_bus(dev))
		dev = dev->parent;

	return dev;
}

pci_dev_t dm_pci_get_bdf(const struct udevice *dev)
{
	struct pci_child_plat *pplat = dev_get_parent_plat(dev);
	struct udevice *bus = dev->parent;

	/*
	 * This error indicates that @dev is a device on an unprobed PCI bus.
	 * The bus likely has bus=seq == -1, so the PCI_ADD_BUS() macro below
	 * will produce a bad BDF>
	 *
	 * A common cause of this problem is that this function is called in the
	 * of_to_plat() method of @dev. Accessing the PCI bus in that
	 * method is not allowed, since it has not yet been probed. To fix this,
	 * move that access to the probe() method of @dev instead.
	 */
	if (!device_active(bus))
		log_err("PCI: Device '%s' on unprobed bus '%s'\n", dev->name,
			bus->name);
	return PCI_ADD_BUS(dev_seq(bus), pplat->devfn);
}

/**
 * pci_get_bus_max() - returns the bus number of the last active bus
 *
 * Return: last bus number, or -1 if no active buses
 */
static int pci_get_bus_max(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret = -1;

	ret = uclass_get(UCLASS_PCI, &uc);
	uclass_foreach_dev(bus, uc) {
		if (dev_seq(bus) > ret)
			ret = dev_seq(bus);
	}

	debug("%s: ret=%d\n", __func__, ret);

	return ret;
}

int pci_last_busno(void)
{
	return pci_get_bus_max();
}

int pci_get_ff(enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 0xff;
	case PCI_SIZE_16:
		return 0xffff;
	default:
		return 0xffffffff;
	}
}

static void pci_dev_find_ofnode(struct udevice *bus, phys_addr_t bdf,
				ofnode *rnode)
{
	struct fdt_pci_addr addr;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, bus) {
		ret = ofnode_read_pci_addr(node, FDT_PCI_SPACE_CONFIG, "reg",
					   &addr, NULL);
		if (ret)
			continue;

		if (PCI_MASK_BUS(addr.phys_hi) != PCI_MASK_BUS(bdf))
			continue;

		*rnode = node;
		break;
	}
};

int pci_bus_find_devfn(const struct udevice *bus, pci_dev_t find_devfn,
		       struct udevice **devp)
{
	struct udevice *dev;

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct pci_child_plat *pplat;

		pplat = dev_get_parent_plat(dev);
		if (pplat && pplat->devfn == find_devfn) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int dm_pci_bus_find_bdf(pci_dev_t bdf, struct udevice **devp)
{
	struct udevice *bus;
	int ret;

	ret = pci_get_bus(PCI_BUS(bdf), &bus);
	if (ret)
		return ret;
	return pci_bus_find_devfn(bus, PCI_MASK_BUS(bdf), devp);
}

static int pci_device_matches_ids(struct udevice *dev,
				  const struct pci_device_id *ids)
{
	struct pci_child_plat *pplat;
	int i;

	pplat = dev_get_parent_plat(dev);
	if (!pplat)
		return -EINVAL;
	for (i = 0; ids[i].vendor != 0; i++) {
		if (pplat->vendor == ids[i].vendor &&
		    pplat->device == ids[i].device)
			return i;
	}

	return -EINVAL;
}

int pci_bus_find_devices(struct udevice *bus, const struct pci_device_id *ids,
			 int *indexp, struct udevice **devp)
{
	struct udevice *dev;

	/* Scan all devices on this bus */
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		if (pci_device_matches_ids(dev, ids) >= 0) {
			if ((*indexp)-- <= 0) {
				*devp = dev;
				return 0;
			}
		}
	}

	return -ENODEV;
}

int pci_find_device_id(const struct pci_device_id *ids, int index,
		       struct udevice **devp)
{
	struct udevice *bus;

	/* Scan all known buses */
	for (uclass_first_device(UCLASS_PCI, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		if (!pci_bus_find_devices(bus, ids, &index, devp))
			return 0;
	}
	*devp = NULL;

	return -ENODEV;
}

static int dm_pci_bus_find_device(struct udevice *bus, unsigned int vendor,
				  unsigned int device, int *indexp,
				  struct udevice **devp)
{
	struct pci_child_plat *pplat;
	struct udevice *dev;

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		pplat = dev_get_parent_plat(dev);
		if (pplat->vendor == vendor && pplat->device == device) {
			if (!(*indexp)--) {
				*devp = dev;
				return 0;
			}
		}
	}

	return -ENODEV;
}

int dm_pci_find_device(unsigned int vendor, unsigned int device, int index,
		       struct udevice **devp)
{
	struct udevice *bus;

	/* Scan all known buses */
	for (uclass_first_device(UCLASS_PCI, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		if (!dm_pci_bus_find_device(bus, vendor, device, &index, devp))
			return device_probe(*devp);
	}
	*devp = NULL;

	return -ENODEV;
}

int dm_pci_find_class(uint find_class, int index, struct udevice **devp)
{
	struct udevice *dev;

	/* Scan all known buses */
	for (pci_find_first_device(&dev);
	     dev;
	     pci_find_next_device(&dev)) {
		struct pci_child_plat *pplat = dev_get_parent_plat(dev);

		if (pplat->class == find_class && !index--) {
			*devp = dev;
			return device_probe(*devp);
		}
	}
	*devp = NULL;

	return -ENODEV;
}

int pci_bus_write_config(struct udevice *bus, pci_dev_t bdf, int offset,
			 unsigned long value, enum pci_size_t size)
{
	struct dm_pci_ops *ops;

	ops = pci_get_ops(bus);
	if (!ops->write_config)
		return -ENOSYS;
	if (offset < 0 || offset >= 4096)
		return -EINVAL;
	return ops->write_config(bus, bdf, offset, value, size);
}

int pci_bus_clrset_config32(struct udevice *bus, pci_dev_t bdf, int offset,
			    u32 clr, u32 set)
{
	ulong val;
	int ret;

	ret = pci_bus_read_config(bus, bdf, offset, &val, PCI_SIZE_32);
	if (ret)
		return ret;
	val &= ~clr;
	val |= set;

	return pci_bus_write_config(bus, bdf, offset, val, PCI_SIZE_32);
}

static int pci_write_config(pci_dev_t bdf, int offset, unsigned long value,
			    enum pci_size_t size)
{
	struct udevice *bus;
	int ret;

	ret = pci_get_bus(PCI_BUS(bdf), &bus);
	if (ret)
		return ret;

	return pci_bus_write_config(bus, bdf, offset, value, size);
}

int dm_pci_write_config(struct udevice *dev, int offset, unsigned long value,
			enum pci_size_t size)
{
	struct udevice *bus;

	for (bus = dev; device_is_on_pci_bus(bus);)
		bus = bus->parent;
	return pci_bus_write_config(bus, dm_pci_get_bdf(dev), offset, value,
				    size);
}

int pci_write_config32(pci_dev_t bdf, int offset, u32 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_32);
}

int pci_write_config16(pci_dev_t bdf, int offset, u16 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_16);
}

int pci_write_config8(pci_dev_t bdf, int offset, u8 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_8);
}

int dm_pci_write_config8(struct udevice *dev, int offset, u8 value)
{
	return dm_pci_write_config(dev, offset, value, PCI_SIZE_8);
}

int dm_pci_write_config16(struct udevice *dev, int offset, u16 value)
{
	return dm_pci_write_config(dev, offset, value, PCI_SIZE_16);
}

int dm_pci_write_config32(struct udevice *dev, int offset, u32 value)
{
	return dm_pci_write_config(dev, offset, value, PCI_SIZE_32);
}

int pci_bus_read_config(const struct udevice *bus, pci_dev_t bdf, int offset,
			unsigned long *valuep, enum pci_size_t size)
{
	struct dm_pci_ops *ops;

	ops = pci_get_ops(bus);
	if (!ops->read_config) {
		*valuep = pci_conv_32_to_size(~0, offset, size);
		return -ENOSYS;
	}
	if (offset < 0 || offset >= 4096) {
		*valuep = pci_conv_32_to_size(0, offset, size);
		return -EINVAL;
	}
	return ops->read_config(bus, bdf, offset, valuep, size);
}

static int pci_read_config(pci_dev_t bdf, int offset, unsigned long *valuep,
			   enum pci_size_t size)
{
	struct udevice *bus;
	int ret;

	ret = pci_get_bus(PCI_BUS(bdf), &bus);
	if (ret)
		return ret;

	return pci_bus_read_config(bus, bdf, offset, valuep, size);
}

int dm_pci_read_config(const struct udevice *dev, int offset,
		       unsigned long *valuep, enum pci_size_t size)
{
	const struct udevice *bus;

	for (bus = dev; device_is_on_pci_bus(bus);)
		bus = bus->parent;
	return pci_bus_read_config(bus, dm_pci_get_bdf(dev), offset, valuep,
				   size);
}

int pci_read_config32(pci_dev_t bdf, int offset, u32 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_32);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int pci_read_config16(pci_dev_t bdf, int offset, u16 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_16);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int pci_read_config8(pci_dev_t bdf, int offset, u8 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_8);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int dm_pci_read_config8(const struct udevice *dev, int offset, u8 *valuep)
{
	unsigned long value;
	int ret;

	ret = dm_pci_read_config(dev, offset, &value, PCI_SIZE_8);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int dm_pci_read_config16(const struct udevice *dev, int offset, u16 *valuep)
{
	unsigned long value;
	int ret;

	ret = dm_pci_read_config(dev, offset, &value, PCI_SIZE_16);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int dm_pci_read_config32(const struct udevice *dev, int offset, u32 *valuep)
{
	unsigned long value;
	int ret;

	ret = dm_pci_read_config(dev, offset, &value, PCI_SIZE_32);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int dm_pci_clrset_config8(struct udevice *dev, int offset, u32 clr, u32 set)
{
	u8 val;
	int ret;

	ret = dm_pci_read_config8(dev, offset, &val);
	if (ret)
		return ret;
	val &= ~clr;
	val |= set;

	return dm_pci_write_config8(dev, offset, val);
}

int dm_pci_clrset_config16(struct udevice *dev, int offset, u32 clr, u32 set)
{
	u16 val;
	int ret;

	ret = dm_pci_read_config16(dev, offset, &val);
	if (ret)
		return ret;
	val &= ~clr;
	val |= set;

	return dm_pci_write_config16(dev, offset, val);
}

int dm_pci_clrset_config32(struct udevice *dev, int offset, u32 clr, u32 set)
{
	u32 val;
	int ret;

	ret = dm_pci_read_config32(dev, offset, &val);
	if (ret)
		return ret;
	val &= ~clr;
	val |= set;

	return dm_pci_write_config32(dev, offset, val);
}

static void set_vga_bridge_bits(struct udevice *dev)
{
	struct udevice *parent = dev->parent;
	u16 bc;

	while (dev_seq(parent) != 0) {
		dm_pci_read_config16(parent, PCI_BRIDGE_CONTROL, &bc);
		bc |= PCI_BRIDGE_CTL_VGA;
		dm_pci_write_config16(parent, PCI_BRIDGE_CONTROL, bc);
		parent = parent->parent;
	}
}

int pci_auto_config_devices(struct udevice *bus)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	struct pci_child_plat *pplat;
	unsigned int sub_bus;
	struct udevice *dev;

	sub_bus = dev_seq(bus);
	debug("%s: start\n", __func__);
	pciauto_config_init(hose);
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		unsigned int max_bus;
		int ret;

		debug("%s: device %s\n", __func__, dev->name);
		if (dev_has_ofnode(dev) &&
		    dev_read_bool(dev, "pci,no-autoconfig"))
			continue;
		ret = dm_pciauto_config_device(dev);
		if (ret < 0)
			return log_msg_ret("auto", ret);
		max_bus = ret;
		sub_bus = max(sub_bus, max_bus);

		if (dev_get_parent(dev) == bus)
			continue;

		pplat = dev_get_parent_plat(dev);
		if (pplat->class == (PCI_CLASS_DISPLAY_VGA << 8))
			set_vga_bridge_bits(dev);
	}
	if (hose->last_busno < sub_bus)
		hose->last_busno = sub_bus;
	debug("%s: done\n", __func__);

	return log_msg_ret("sub", sub_bus);
}

int pci_generic_mmap_write_config(
	const struct udevice *bus,
	int (*addr_f)(const struct udevice *bus, pci_dev_t bdf, uint offset,
		      void **addrp),
	pci_dev_t bdf,
	uint offset,
	ulong value,
	enum pci_size_t size)
{
	void *address;

	if (addr_f(bus, bdf, offset, &address) < 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		writeb(value, address);
		return 0;
	case PCI_SIZE_16:
		writew(value, address);
		return 0;
	case PCI_SIZE_32:
		writel(value, address);
		return 0;
	default:
		return -EINVAL;
	}
}

int pci_generic_mmap_read_config(
	const struct udevice *bus,
	int (*addr_f)(const struct udevice *bus, pci_dev_t bdf, uint offset,
		      void **addrp),
	pci_dev_t bdf,
	uint offset,
	ulong *valuep,
	enum pci_size_t size)
{
	void *address;

	if (addr_f(bus, bdf, offset, &address) < 0) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address);
		return 0;
	case PCI_SIZE_16:
		*valuep = readw(address);
		return 0;
	case PCI_SIZE_32:
		*valuep = readl(address);
		return 0;
	default:
		return -EINVAL;
	}
}

int dm_pci_hose_probe_bus(struct udevice *bus)
{
	u8 header_type;
	int sub_bus;
	int ret;
	int ea_pos;
	u8 reg;

	debug("%s\n", __func__);

	dm_pci_read_config8(bus, PCI_HEADER_TYPE, &header_type);
	header_type &= 0x7f;
	if (header_type != PCI_HEADER_TYPE_BRIDGE) {
		debug("%s: Skipping PCI device %d with Non-Bridge Header Type 0x%x\n",
		      __func__, PCI_DEV(dm_pci_get_bdf(bus)), header_type);
		return log_msg_ret("probe", -EINVAL);
	}

	if (IS_ENABLED(CONFIG_PCI_ENHANCED_ALLOCATION))
		ea_pos = dm_pci_find_capability(bus, PCI_CAP_ID_EA);
	else
		ea_pos = 0;

	if (ea_pos) {
		dm_pci_read_config8(bus, ea_pos + sizeof(u32) + sizeof(u8),
				    &reg);
		sub_bus = reg;
	} else {
		sub_bus = pci_get_bus_max() + 1;
	}
	debug("%s: bus = %d/%s\n", __func__, sub_bus, bus->name);
	dm_pciauto_prescan_setup_bridge(bus, sub_bus);

	ret = device_probe(bus);
	if (ret) {
		debug("%s: Cannot probe bus %s: %d\n", __func__, bus->name,
		      ret);
		return log_msg_ret("probe", ret);
	}

	if (!ea_pos)
		sub_bus = pci_get_bus_max();

	dm_pciauto_postscan_setup_bridge(bus, sub_bus);

	return sub_bus;
}

/**
 * pci_match_one_device - Tell if a PCI device structure has a matching
 *                        PCI device id structure
 * @id: single PCI device id structure to match
 * @find: the PCI device id structure to match against
 *
 * Returns true if the finding pci_device_id structure matched or false if
 * there is no match.
 */
static bool pci_match_one_id(const struct pci_device_id *id,
			     const struct pci_device_id *find)
{
	if ((id->vendor == PCI_ANY_ID || id->vendor == find->vendor) &&
	    (id->device == PCI_ANY_ID || id->device == find->device) &&
	    (id->subvendor == PCI_ANY_ID || id->subvendor == find->subvendor) &&
	    (id->subdevice == PCI_ANY_ID || id->subdevice == find->subdevice) &&
	    !((id->class ^ find->class) & id->class_mask))
		return true;

	return false;
}

/**
 * pci_need_device_pre_reloc() - Check if a device should be bound
 *
 * This checks a list of vendor/device-ID values indicating devices that should
 * be bound before relocation.
 *
 * @bus: Bus to check
 * @vendor: Vendor ID to check
 * @device: Device ID to check
 * Return: true if the vendor/device is in the list, false if not
 */
static bool pci_need_device_pre_reloc(struct udevice *bus, uint vendor,
				      uint device)
{
	u32 vendev;
	int index;

	if (spl_phase() == PHASE_SPL && CONFIG_IS_ENABLED(PCI_PNP))
		return true;

	for (index = 0;
	     !dev_read_u32_index(bus, "u-boot,pci-pre-reloc", index,
				 &vendev);
	     index++) {
		if (vendev == PCI_VENDEV(vendor, device))
			return true;
	}

	return false;
}

/**
 * pci_find_and_bind_driver() - Find and bind the right PCI driver
 *
 * This only looks at certain fields in the descriptor.
 *
 * @parent:	Parent bus
 * @find_id:	Specification of the driver to find
 * @bdf:	Bus/device/function addreess - see PCI_BDF()
 * @devp:	Returns a pointer to the device created
 * Return: 0 if OK, -EPERM if the device is not needed before relocation and
 *	   therefore was not created, other -ve value on error
 */
static int pci_find_and_bind_driver(struct udevice *parent,
				    struct pci_device_id *find_id,
				    pci_dev_t bdf, struct udevice **devp)
{
	struct pci_driver_entry *start, *entry;
	ofnode node = ofnode_null();
	const char *drv;
	int n_ents;
	int ret;
	char name[30], *str;
	bool bridge;

	*devp = NULL;

	debug("%s: Searching for driver: vendor=%x, device=%x\n", __func__,
	      find_id->vendor, find_id->device);

	/* Determine optional OF node */
	if (ofnode_valid(dev_ofnode(parent)))
		pci_dev_find_ofnode(parent, bdf, &node);

	if (ofnode_valid(node) && !ofnode_is_enabled(node)) {
		debug("%s: Ignoring disabled device\n", __func__);
		return log_msg_ret("dis", -EPERM);
	}

	start = ll_entry_start(struct pci_driver_entry, pci_driver_entry);
	n_ents = ll_entry_count(struct pci_driver_entry, pci_driver_entry);
	for (entry = start; entry != start + n_ents; entry++) {
		const struct pci_device_id *id;
		struct udevice *dev;
		const struct driver *drv;

		for (id = entry->match;
		     id->vendor || id->subvendor || id->class_mask;
		     id++) {
			if (!pci_match_one_id(id, find_id))
				continue;

			drv = entry->driver;

			/*
			 * In the pre-relocation phase, we only bind devices
			 * whose driver has the DM_FLAG_PRE_RELOC set, to save
			 * precious memory space as on some platforms as that
			 * space is pretty limited (ie: using Cache As RAM).
			 */
			if (!(gd->flags & GD_FLG_RELOC) &&
			    !(drv->flags & DM_FLAG_PRE_RELOC) &&
			    (!CONFIG_IS_ENABLED(PCI_PNP) ||
			     spl_phase() != PHASE_SPL))
				return log_msg_ret("pre", -EPERM);

			/*
			 * We could pass the descriptor to the driver as
			 * plat (instead of NULL) and allow its bind()
			 * method to return -ENOENT if it doesn't support this
			 * device. That way we could continue the search to
			 * find another driver. For now this doesn't seem
			 * necesssary, so just bind the first match.
			 */
			ret = device_bind(parent, drv, drv->name, NULL, node,
					  &dev);
			if (ret)
				goto error;
			debug("%s: Match found: %s\n", __func__, drv->name);
			dev->driver_data = id->driver_data;
			*devp = dev;
			return 0;
		}
	}

	bridge = (find_id->class >> 8) == PCI_CLASS_BRIDGE_PCI;
	/*
	 * In the pre-relocation phase, we only bind bridge devices to save
	 * precious memory space as on some platforms as that space is pretty
	 * limited (ie: using Cache As RAM).
	 */
	if (!(gd->flags & GD_FLG_RELOC) && !bridge &&
	    !pci_need_device_pre_reloc(parent, find_id->vendor,
				       find_id->device))
		return log_msg_ret("notbr", -EPERM);

	/* Bind a generic driver so that the device can be used */
	sprintf(name, "pci_%x:%x.%x", dev_seq(parent), PCI_DEV(bdf),
		PCI_FUNC(bdf));
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	drv = bridge ? "pci_bridge_drv" : "pci_generic_drv";

	ret = device_bind_driver_to_node(parent, drv, str, node, devp);
	if (ret) {
		debug("%s: Failed to bind generic driver: %d\n", __func__, ret);
		free(str);
		return ret;
	}
	debug("%s: No match found: bound generic driver instead\n", __func__);

	return 0;

error:
	debug("%s: No match found: error %d\n", __func__, ret);
	return ret;
}

__weak extern void board_pci_fixup_dev(struct udevice *bus, struct udevice *dev)
{
}

int pci_bind_bus_devices(struct udevice *bus)
{
	ulong vendor, device;
	ulong header_type;
	pci_dev_t bdf, end;
	bool found_multi;
	int ari_off;
	int ret;

	found_multi = false;
	end = PCI_BDF(dev_seq(bus), PCI_MAX_PCI_DEVICES - 1,
		      PCI_MAX_PCI_FUNCTIONS - 1);
	for (bdf = PCI_BDF(dev_seq(bus), 0, 0); bdf <= end;
	     bdf += PCI_BDF(0, 0, 1)) {
		struct pci_child_plat *pplat;
		struct udevice *dev;
		ulong class;

		if (!PCI_FUNC(bdf))
			found_multi = false;
		if (PCI_FUNC(bdf) && !found_multi)
			continue;

		/* Check only the first access, we don't expect problems */
		ret = pci_bus_read_config(bus, bdf, PCI_VENDOR_ID, &vendor,
					  PCI_SIZE_16);
		if (ret || vendor == 0xffff || vendor == 0x0000)
			continue;

		pci_bus_read_config(bus, bdf, PCI_HEADER_TYPE,
				    &header_type, PCI_SIZE_8);

		if (!PCI_FUNC(bdf))
			found_multi = header_type & 0x80;

		debug("%s: bus %d/%s: found device %x, function %d", __func__,
		      dev_seq(bus), bus->name, PCI_DEV(bdf), PCI_FUNC(bdf));
		pci_bus_read_config(bus, bdf, PCI_DEVICE_ID, &device,
				    PCI_SIZE_16);
		pci_bus_read_config(bus, bdf, PCI_CLASS_REVISION, &class,
				    PCI_SIZE_32);
		class >>= 8;

		/* Find this device in the device tree */
		ret = pci_bus_find_devfn(bus, PCI_MASK_BUS(bdf), &dev);
		debug(": find ret=%d\n", ret);

		/* If nothing in the device tree, bind a device */
		if (ret == -ENODEV) {
			struct pci_device_id find_id;
			ulong val;

			memset(&find_id, '\0', sizeof(find_id));
			find_id.vendor = vendor;
			find_id.device = device;
			find_id.class = class;
			if ((header_type & 0x7f) == PCI_HEADER_TYPE_NORMAL) {
				pci_bus_read_config(bus, bdf,
						    PCI_SUBSYSTEM_VENDOR_ID,
						    &val, PCI_SIZE_32);
				find_id.subvendor = val & 0xffff;
				find_id.subdevice = val >> 16;
			}
			ret = pci_find_and_bind_driver(bus, &find_id, bdf,
						       &dev);
		} else {
			debug("device: %s\n", dev->name);
		}
		if (ret == -EPERM)
			continue;
		else if (ret)
			return ret;

		/* Update the platform data */
		pplat = dev_get_parent_plat(dev);
		pplat->devfn = PCI_MASK_BUS(bdf);
		pplat->vendor = vendor;
		pplat->device = device;
		pplat->class = class;

		if (IS_ENABLED(CONFIG_PCI_ARID)) {
			ari_off = dm_pci_find_ext_capability(dev,
							     PCI_EXT_CAP_ID_ARI);
			if (ari_off) {
				u16 ari_cap;

				/*
				 * Read Next Function number in ARI Cap
				 * Register
				 */
				dm_pci_read_config16(dev, ari_off + 4,
						     &ari_cap);
				/*
				 * Update next scan on this function number,
				 * subtract 1 in BDF to satisfy loop increment.
				 */
				if (ari_cap & 0xff00) {
					bdf = PCI_BDF(PCI_BUS(bdf),
						      PCI_DEV(ari_cap),
						      PCI_FUNC(ari_cap));
					bdf = bdf - 0x100;
				}
			}
		}

		board_pci_fixup_dev(bus, dev);
	}

	return 0;
}

static int decode_regions(struct pci_controller *hose, ofnode parent_node,
			   ofnode node)
{
	int pci_addr_cells, addr_cells, size_cells;
	int cells_per_record;
	struct bd_info *bd;
	const u32 *prop;
	int max_regions;
	int len;
	int i;

	/* handle booting from coreboot, etc. */
	if (!ll_boot_init())
		return 0;

	prop = ofnode_get_property(node, "ranges", &len);
	if (!prop) {
		debug("%s: Cannot decode regions\n", __func__);
		return -EINVAL;
	}

	pci_addr_cells = ofnode_read_simple_addr_cells(node);
	addr_cells = ofnode_read_simple_addr_cells(parent_node);
	size_cells = ofnode_read_simple_size_cells(node);

	/* PCI addresses are always 3-cells */
	len /= sizeof(u32);
	cells_per_record = pci_addr_cells + addr_cells + size_cells;
	hose->region_count = 0;
	debug("%s: len=%d, cells_per_record=%d\n", __func__, len,
	      cells_per_record);

	/* Dynamically allocate the regions array */
	max_regions = len / cells_per_record + CONFIG_NR_DRAM_BANKS;
	hose->regions = (struct pci_region *)
		calloc(1, max_regions * sizeof(struct pci_region));
	if (!hose->regions)
		return -ENOMEM;

	for (i = 0; i < max_regions; i++, len -= cells_per_record) {
		u64 pci_addr, addr, size;
		int space_code;
		u32 flags;
		int type;
		int pos;

		if (len < cells_per_record)
			break;
		flags = fdt32_to_cpu(prop[0]);
		space_code = (flags >> 24) & 3;
		pci_addr = fdtdec_get_number(prop + 1, 2);
		prop += pci_addr_cells;
		addr = fdtdec_get_number(prop, addr_cells);
		prop += addr_cells;
		size = fdtdec_get_number(prop, size_cells);
		prop += size_cells;
		debug("%s: region %d, pci_addr=%llx, addr=%llx, size=%llx, space_code=%d\n",
		      __func__, hose->region_count, pci_addr, addr, size, space_code);
		if (space_code & 2) {
			type = flags & (1U << 30) ? PCI_REGION_PREFETCH :
					PCI_REGION_MEM;
		} else if (space_code & 1) {
			type = PCI_REGION_IO;
		} else {
			continue;
		}

		if (!IS_ENABLED(CONFIG_SYS_PCI_64BIT) &&
		    type == PCI_REGION_MEM && upper_32_bits(pci_addr)) {
			debug(" - pci_addr beyond the 32-bit boundary, ignoring\n");
			continue;
		}

		if (!IS_ENABLED(CONFIG_PHYS_64BIT) && upper_32_bits(addr)) {
			debug(" - addr beyond the 32-bit boundary, ignoring\n");
			continue;
		}

		if (~((pci_addr_t)0) - pci_addr < size) {
			debug(" - PCI range exceeds max address, ignoring\n");
			continue;
		}

		if (~((phys_addr_t)0) - addr < size) {
			debug(" - phys range exceeds max address, ignoring\n");
			continue;
		}

		pos = -1;
		if (!IS_ENABLED(CONFIG_PCI_REGION_MULTI_ENTRY)) {
			for (i = 0; i < hose->region_count; i++) {
				if (hose->regions[i].flags == type)
					pos = i;
			}
		}

		if (pos == -1)
			pos = hose->region_count++;
		debug(" - type=%d, pos=%d\n", type, pos);
		pci_set_region(hose->regions + pos, pci_addr, addr, size, type);
	}

	/* Add a region for our local memory */
	bd = gd->bd;
	if (!bd)
		return 0;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			phys_addr_t start = bd->bi_dram[i].start;

			if (IS_ENABLED(CONFIG_PCI_MAP_SYSTEM_MEMORY))
				start = virt_to_phys((void *)(uintptr_t)bd->bi_dram[i].start);

			pci_set_region(hose->regions + hose->region_count++,
				       start, start, bd->bi_dram[i].size,
				       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
		}
	}

	return 0;
}

static int pci_uclass_pre_probe(struct udevice *bus)
{
	struct pci_controller *hose;
	struct uclass *uc;
	int ret;

	debug("%s, bus=%d/%s, parent=%s\n", __func__, dev_seq(bus), bus->name,
	      bus->parent->name);
	hose = dev_get_uclass_priv(bus);

	/*
	 * Set the sequence number, if device_bind() doesn't. We want control
	 * of this so that numbers are allocated as devices are probed. That
	 * ensures that sub-bus numbered is correct (sub-buses must get numbers
	 * higher than their parents)
	 */
	if (dev_seq(bus) == -1) {
		ret = uclass_get(UCLASS_PCI, &uc);
		if (ret)
			return ret;
		bus->seq_ = uclass_find_next_free_seq(uc);
	}

	/* For bridges, use the top-level PCI controller */
	if (!device_is_on_pci_bus(bus)) {
		hose->ctlr = bus;
		ret = decode_regions(hose, dev_ofnode(bus->parent),
				     dev_ofnode(bus));
		if (ret)
			return ret;
	} else {
		struct pci_controller *parent_hose;

		parent_hose = dev_get_uclass_priv(bus->parent);
		hose->ctlr = parent_hose->bus;
	}

	hose->bus = bus;
	hose->first_busno = dev_seq(bus);
	hose->last_busno = dev_seq(bus);
	if (dev_has_ofnode(bus)) {
		hose->skip_auto_config_until_reloc =
			dev_read_bool(bus,
				      "u-boot,skip-auto-config-until-reloc");
	}

	return 0;
}

static int pci_uclass_post_probe(struct udevice *bus)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	int ret;

	debug("%s: probing bus %d\n", __func__, dev_seq(bus));
	ret = pci_bind_bus_devices(bus);
	if (ret)
		return log_msg_ret("bind", ret);

	if (CONFIG_IS_ENABLED(PCI_PNP) && ll_boot_init() &&
	    (!hose->skip_auto_config_until_reloc ||
	     (gd->flags & GD_FLG_RELOC))) {
		ret = pci_auto_config_devices(bus);
		if (ret < 0)
			return log_msg_ret("cfg", ret);
	}

#if defined(CONFIG_X86) && defined(CONFIG_HAVE_FSP)
	/*
	 * Per Intel FSP specification, we should call FSP notify API to
	 * inform FSP that PCI enumeration has been done so that FSP will
	 * do any necessary initialization as required by the chipset's
	 * BIOS Writer's Guide (BWG).
	 *
	 * Unfortunately we have to put this call here as with driver model,
	 * the enumeration is all done on a lazy basis as needed, so until
	 * something is touched on PCI it won't happen.
	 *
	 * Note we only call this 1) after U-Boot is relocated, and 2)
	 * root bus has finished probing.
	 */
	if ((gd->flags & GD_FLG_RELOC) && dev_seq(bus) == 0 && ll_boot_init()) {
		ret = fsp_init_phase_pci();
		if (ret)
			return log_msg_ret("fsp", ret);
	}
#endif

	return 0;
}

static int pci_uclass_child_post_bind(struct udevice *dev)
{
	struct pci_child_plat *pplat;

	if (!dev_has_ofnode(dev))
		return 0;

	pplat = dev_get_parent_plat(dev);

	/* Extract vendor id and device id if available */
	ofnode_read_pci_vendev(dev_ofnode(dev), &pplat->vendor, &pplat->device);

	/* Extract the devfn from fdt_pci_addr */
	pplat->devfn = pci_get_devfn(dev);

	return 0;
}

static int pci_bridge_read_config(const struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);

	return pci_bus_read_config(hose->ctlr, bdf, offset, valuep, size);
}

static int pci_bridge_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);

	return pci_bus_write_config(hose->ctlr, bdf, offset, value, size);
}

static int skip_to_next_device(struct udevice *bus, struct udevice **devp)
{
	struct udevice *dev;

	/*
	 * Scan through all the PCI controllers. On x86 there will only be one
	 * but that is not necessarily true on other hardware.
	 */
	while (bus) {
		device_find_first_child(bus, &dev);
		if (dev) {
			*devp = dev;
			return 0;
		}
		uclass_next_device(&bus);
	}

	return 0;
}

int pci_find_next_device(struct udevice **devp)
{
	struct udevice *child = *devp;
	struct udevice *bus = child->parent;

	/* First try all the siblings */
	*devp = NULL;
	while (child) {
		device_find_next_child(&child);
		if (child) {
			*devp = child;
			return 0;
		}
	}

	/* We ran out of siblings. Try the next bus */
	uclass_next_device(&bus);

	return bus ? skip_to_next_device(bus, devp) : 0;
}

int pci_find_first_device(struct udevice **devp)
{
	struct udevice *bus;

	*devp = NULL;
	uclass_first_device(UCLASS_PCI, &bus);

	return skip_to_next_device(bus, devp);
}

ulong pci_conv_32_to_size(ulong value, uint offset, enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return (value >> ((offset & 3) * 8)) & 0xff;
	case PCI_SIZE_16:
		return (value >> ((offset & 2) * 8)) & 0xffff;
	default:
		return value;
	}
}

ulong pci_conv_size_to_32(ulong old, ulong value, uint offset,
			  enum pci_size_t size)
{
	uint off_mask;
	uint val_mask, shift;
	ulong ldata, mask;

	switch (size) {
	case PCI_SIZE_8:
		off_mask = 3;
		val_mask = 0xff;
		break;
	case PCI_SIZE_16:
		off_mask = 2;
		val_mask = 0xffff;
		break;
	default:
		return value;
	}
	shift = (offset & off_mask) * 8;
	ldata = (value & val_mask) << shift;
	mask = val_mask << shift;
	value = (old & ~mask) | ldata;

	return value;
}

int pci_get_dma_regions(struct udevice *dev, struct pci_region *memp, int index)
{
	int pci_addr_cells, addr_cells, size_cells;
	int cells_per_record;
	const u32 *prop;
	int len;
	int i = 0;

	prop = ofnode_get_property(dev_ofnode(dev), "dma-ranges", &len);
	if (!prop) {
		log_err("PCI: Device '%s': Cannot decode dma-ranges\n",
			dev->name);
		return -EINVAL;
	}

	pci_addr_cells = ofnode_read_simple_addr_cells(dev_ofnode(dev));
	addr_cells = ofnode_read_simple_addr_cells(dev_ofnode(dev->parent));
	size_cells = ofnode_read_simple_size_cells(dev_ofnode(dev));

	/* PCI addresses are always 3-cells */
	len /= sizeof(u32);
	cells_per_record = pci_addr_cells + addr_cells + size_cells;
	debug("%s: len=%d, cells_per_record=%d\n", __func__, len,
	      cells_per_record);

	while (len) {
		memp->bus_start = fdtdec_get_number(prop + 1, 2);
		prop += pci_addr_cells;
		memp->phys_start = fdtdec_get_number(prop, addr_cells);
		prop += addr_cells;
		memp->size = fdtdec_get_number(prop, size_cells);
		prop += size_cells;

		if (i == index)
			return 0;
		i++;
		len -= cells_per_record;
	}

	return -EINVAL;
}

int pci_get_regions(struct udevice *dev, struct pci_region **iop,
		    struct pci_region **memp, struct pci_region **prefp)
{
	struct udevice *bus = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	int i;

	*iop = NULL;
	*memp = NULL;
	*prefp = NULL;
	for (i = 0; i < hose->region_count; i++) {
		switch (hose->regions[i].flags) {
		case PCI_REGION_IO:
			if (!*iop || (*iop)->size < hose->regions[i].size)
				*iop = hose->regions + i;
			break;
		case PCI_REGION_MEM:
			if (!*memp || (*memp)->size < hose->regions[i].size)
				*memp = hose->regions + i;
			break;
		case (PCI_REGION_MEM | PCI_REGION_PREFETCH):
			if (!*prefp || (*prefp)->size < hose->regions[i].size)
				*prefp = hose->regions + i;
			break;
		}
	}

	return (*iop != NULL) + (*memp != NULL) + (*prefp != NULL);
}

u32 dm_pci_read_bar32(const struct udevice *dev, int barnum)
{
	u32 addr;
	int bar;

	bar = PCI_BASE_ADDRESS_0 + barnum * 4;
	dm_pci_read_config32(dev, bar, &addr);

	/*
	 * If we get an invalid address, return this so that comparisons with
	 * FDT_ADDR_T_NONE work correctly
	 */
	if (addr == 0xffffffff)
		return addr;
	else if (addr & PCI_BASE_ADDRESS_SPACE_IO)
		return addr & PCI_BASE_ADDRESS_IO_MASK;
	else
		return addr & PCI_BASE_ADDRESS_MEM_MASK;
}

void dm_pci_write_bar32(struct udevice *dev, int barnum, u32 addr)
{
	int bar;

	bar = PCI_BASE_ADDRESS_0 + barnum * 4;
	dm_pci_write_config32(dev, bar, addr);
}

phys_addr_t dm_pci_bus_to_phys(struct udevice *dev, pci_addr_t bus_addr,
			       size_t len, unsigned long mask,
			       unsigned long flags)
{
	struct udevice *ctlr;
	struct pci_controller *hose;
	struct pci_region *res;
	pci_addr_t offset;
	int i;

	/* The root controller has the region information */
	ctlr = pci_get_controller(dev);
	hose = dev_get_uclass_priv(ctlr);

	if (hose->region_count == 0)
		return bus_addr;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if ((res->flags & mask) != flags)
			continue;

		if (bus_addr < res->bus_start)
			continue;

		offset = bus_addr - res->bus_start;
		if (offset >= res->size)
			continue;

		if (len > res->size - offset)
			continue;

		return res->phys_start + offset;
	}

	puts("dm_pci_bus_to_phys: invalid physical address\n");
	return 0;
}

pci_addr_t dm_pci_phys_to_bus(struct udevice *dev, phys_addr_t phys_addr,
			      size_t len, unsigned long mask,
			      unsigned long flags)
{
	struct udevice *ctlr;
	struct pci_controller *hose;
	struct pci_region *res;
	phys_addr_t offset;
	int i;

	/* The root controller has the region information */
	ctlr = pci_get_controller(dev);
	hose = dev_get_uclass_priv(ctlr);

	if (hose->region_count == 0)
		return phys_addr;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if ((res->flags & mask) != flags)
			continue;

		if (phys_addr < res->phys_start)
			continue;

		offset = phys_addr - res->phys_start;
		if (offset >= res->size)
			continue;

		if (len > res->size - offset)
			continue;

		return res->bus_start + offset;
	}

	puts("dm_pci_phys_to_bus: invalid physical address\n");
	return 0;
}

static phys_addr_t dm_pci_map_ea_virt(struct udevice *dev, int ea_off,
				      struct pci_child_plat *pdata)
{
	phys_addr_t addr = 0;

	/*
	 * In the case of a Virtual Function device using BAR
	 * base and size, add offset for VFn BAR(1, 2, 3...n)
	 */
	if (pdata->is_virtfn) {
		size_t sz;
		u32 ea_entry;

		/* MaxOffset, 1st DW */
		dm_pci_read_config32(dev, ea_off + 8, &ea_entry);
		sz = ea_entry & PCI_EA_FIELD_MASK;
		/* Fill up lower 2 bits */
		sz |= (~PCI_EA_FIELD_MASK);

		if (ea_entry & PCI_EA_IS_64) {
			/* MaxOffset 2nd DW */
			dm_pci_read_config32(dev, ea_off + 16, &ea_entry);
			sz |= ((u64)ea_entry) << 32;
		}

		addr = (pdata->virtid - 1) * (sz + 1);
	}

	return addr;
}

static void *dm_pci_map_ea_bar(struct udevice *dev, int bar, size_t offset,
			       size_t len, int ea_off,
			       struct pci_child_plat *pdata)
{
	int ea_cnt, i, entry_size;
	int bar_id = (bar - PCI_BASE_ADDRESS_0) >> 2;
	u32 ea_entry;
	phys_addr_t addr;

	if (IS_ENABLED(CONFIG_PCI_SRIOV)) {
		/*
		 * In the case of a Virtual Function device, device is
		 * Physical function, so pdata will point to required VF
		 * specific data.
		 */
		if (pdata->is_virtfn)
			bar_id += PCI_EA_BEI_VF_BAR0;
	}

	/* EA capability structure header */
	dm_pci_read_config32(dev, ea_off, &ea_entry);
	ea_cnt = (ea_entry >> 16) & PCI_EA_NUM_ENT_MASK;
	ea_off += PCI_EA_FIRST_ENT;

	for (i = 0; i < ea_cnt; i++, ea_off += entry_size) {
		/* Entry header */
		dm_pci_read_config32(dev, ea_off, &ea_entry);
		entry_size = ((ea_entry & PCI_EA_ES) + 1) << 2;

		if (((ea_entry & PCI_EA_BEI) >> 4) != bar_id)
			continue;

		/* Base address, 1st DW */
		dm_pci_read_config32(dev, ea_off + 4, &ea_entry);
		addr = ea_entry & PCI_EA_FIELD_MASK;
		if (ea_entry & PCI_EA_IS_64) {
			/* Base address, 2nd DW, skip over 4B MaxOffset */
			dm_pci_read_config32(dev, ea_off + 12, &ea_entry);
			addr |= ((u64)ea_entry) << 32;
		}

		if (IS_ENABLED(CONFIG_PCI_SRIOV))
			addr += dm_pci_map_ea_virt(dev, ea_off, pdata);

		if (~((phys_addr_t)0) - addr < offset)
			return NULL;

		/* size ignored for now */
		return map_physmem(addr + offset, len, MAP_NOCACHE);
	}

	return 0;
}

void *dm_pci_map_bar(struct udevice *dev, int bar, size_t offset, size_t len,
		     unsigned long mask, unsigned long flags)
{
	struct pci_child_plat *pdata = dev_get_parent_plat(dev);
	struct udevice *udev = dev;
	pci_addr_t pci_bus_addr;
	u32 bar_response;
	int ea_off;

	if (IS_ENABLED(CONFIG_PCI_SRIOV)) {
		/*
		 * In case of Virtual Function devices, use PF udevice
		 * as EA capability is defined in Physical Function
		 */
		if (pdata->is_virtfn)
			udev = pdata->pfdev;
	}

	/*
	 * if the function supports Enhanced Allocation use that instead of
	 * BARs
	 * Incase of virtual functions, pdata will help read VF BEI
	 * and EA entry size.
	 */
	if (IS_ENABLED(CONFIG_PCI_ENHANCED_ALLOCATION))
		ea_off = dm_pci_find_capability(udev, PCI_CAP_ID_EA);
	else
		ea_off = 0;

	if (ea_off)
		return dm_pci_map_ea_bar(udev, bar, offset, len, ea_off, pdata);

	/* read BAR address */
	dm_pci_read_config32(udev, bar, &bar_response);
	pci_bus_addr = (pci_addr_t)(bar_response & ~0xf);

	/* This has a lot of baked in assumptions, but essentially tries
	 * to mirror the behavior of BAR assignment for 64 Bit enabled
	 * hosts and 64 bit placeable BARs in the auto assign code.
	 */
#if defined(CONFIG_SYS_PCI_64BIT)
	if (bar_response & PCI_BASE_ADDRESS_MEM_TYPE_64) {
		dm_pci_read_config32(udev, bar + 4, &bar_response);
		pci_bus_addr |= (pci_addr_t)bar_response << 32;
	}
#endif /* CONFIG_SYS_PCI_64BIT */

	if (~((pci_addr_t)0) - pci_bus_addr < offset)
		return NULL;

	/*
	 * Forward the length argument to dm_pci_bus_to_virt. The length will
	 * be used to check that the entire address range has been declared as
	 * a PCI range, but a better check would be to probe for the size of
	 * the bar and prevent overflow more locally.
	 */
	return dm_pci_bus_to_virt(udev, pci_bus_addr + offset, len, mask, flags,
				  MAP_NOCACHE);
}

static int _dm_pci_find_next_capability(struct udevice *dev, u8 pos, int cap)
{
	int ttl = PCI_FIND_CAP_TTL;
	u8 id;
	u16 ent;

	dm_pci_read_config8(dev, pos, &pos);

	while (ttl--) {
		if (pos < PCI_STD_HEADER_SIZEOF)
			break;
		pos &= ~3;
		dm_pci_read_config16(dev, pos, &ent);

		id = ent & 0xff;
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos = (ent >> 8);
	}

	return 0;
}

int dm_pci_find_next_capability(struct udevice *dev, u8 start, int cap)
{
	return _dm_pci_find_next_capability(dev, start + PCI_CAP_LIST_NEXT,
					    cap);
}

int dm_pci_find_capability(struct udevice *dev, int cap)
{
	u16 status;
	u8 header_type;
	u8 pos;

	dm_pci_read_config16(dev, PCI_STATUS, &status);
	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;

	dm_pci_read_config8(dev, PCI_HEADER_TYPE, &header_type);
	if ((header_type & 0x7f) == PCI_HEADER_TYPE_CARDBUS)
		pos = PCI_CB_CAPABILITY_LIST;
	else
		pos = PCI_CAPABILITY_LIST;

	return _dm_pci_find_next_capability(dev, pos, cap);
}

int dm_pci_find_next_ext_capability(struct udevice *dev, int start, int cap)
{
	u32 header;
	int ttl;
	int pos = PCI_CFG_SPACE_SIZE;

	/* minimum 8 bytes per capability */
	ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

	if (start)
		pos = start;

	dm_pci_read_config32(dev, pos, &header);
	/*
	 * If we have no capabilities, this is indicated by cap ID,
	 * cap version and next pointer all being 0.
	 */
	if (header == 0)
		return 0;

	while (ttl--) {
		if (PCI_EXT_CAP_ID(header) == cap)
			return pos;

		pos = PCI_EXT_CAP_NEXT(header);
		if (pos < PCI_CFG_SPACE_SIZE)
			break;

		dm_pci_read_config32(dev, pos, &header);
	}

	return 0;
}

int dm_pci_find_ext_capability(struct udevice *dev, int cap)
{
	return dm_pci_find_next_ext_capability(dev, 0, cap);
}

int dm_pci_flr(struct udevice *dev)
{
	int pcie_off;
	u32 cap;

	/* look for PCI Express Capability */
	pcie_off = dm_pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pcie_off)
		return -ENOENT;

	/* check FLR capability */
	dm_pci_read_config32(dev, pcie_off + PCI_EXP_DEVCAP, &cap);
	if (!(cap & PCI_EXP_DEVCAP_FLR))
		return -ENOENT;

	dm_pci_clrset_config16(dev, pcie_off + PCI_EXP_DEVCTL, 0,
			       PCI_EXP_DEVCTL_BCR_FLR);

	/* wait 100ms, per PCI spec */
	mdelay(100);

	return 0;
}

#if defined(CONFIG_PCI_SRIOV)
int pci_sriov_init(struct udevice *pdev, int vf_en)
{
	u16 vendor, device;
	struct udevice *bus;
	struct udevice *dev;
	pci_dev_t bdf;
	u16 ctrl;
	u16 num_vfs;
	u16 total_vf;
	u16 vf_offset;
	u16 vf_stride;
	int vf, ret;
	int pos;

	pos = dm_pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_SRIOV);
	if (!pos) {
		debug("Error: SRIOV capability not found\n");
		return -ENOENT;
	}

	dm_pci_read_config16(pdev, pos + PCI_SRIOV_CTRL, &ctrl);

	dm_pci_read_config16(pdev, pos + PCI_SRIOV_TOTAL_VF, &total_vf);
	if (vf_en > total_vf)
		vf_en = total_vf;
	dm_pci_write_config16(pdev, pos + PCI_SRIOV_NUM_VF, vf_en);

	ctrl |= PCI_SRIOV_CTRL_VFE | PCI_SRIOV_CTRL_MSE;
	dm_pci_write_config16(pdev, pos + PCI_SRIOV_CTRL, ctrl);

	dm_pci_read_config16(pdev, pos + PCI_SRIOV_NUM_VF, &num_vfs);
	if (num_vfs > vf_en)
		num_vfs = vf_en;

	dm_pci_read_config16(pdev, pos + PCI_SRIOV_VF_OFFSET, &vf_offset);
	dm_pci_read_config16(pdev, pos + PCI_SRIOV_VF_STRIDE, &vf_stride);

	dm_pci_read_config16(pdev, PCI_VENDOR_ID, &vendor);
	dm_pci_read_config16(pdev, pos + PCI_SRIOV_VF_DID, &device);

	bdf = dm_pci_get_bdf(pdev);

	ret = pci_get_bus(PCI_BUS(bdf), &bus);
	if (ret)
		return ret;

	bdf += PCI_BDF(0, 0, vf_offset);

	for (vf = 0; vf < num_vfs; vf++) {
		struct pci_child_plat *pplat;
		ulong class;

		pci_bus_read_config(bus, bdf, PCI_CLASS_DEVICE,
				    &class, PCI_SIZE_16);

		debug("%s: bus %d/%s: found VF %x:%x\n", __func__,
		      dev_seq(bus), bus->name, PCI_DEV(bdf), PCI_FUNC(bdf));

		/* Find this device in the device tree */
		ret = pci_bus_find_devfn(bus, PCI_MASK_BUS(bdf), &dev);

		if (ret == -ENODEV) {
			struct pci_device_id find_id;

			memset(&find_id, '\0', sizeof(find_id));
			find_id.vendor = vendor;
			find_id.device = device;
			find_id.class = class;

			ret = pci_find_and_bind_driver(bus, &find_id,
						       bdf, &dev);

			if (ret)
				return ret;
		}

		/* Update the platform data */
		pplat = dev_get_parent_plat(dev);
		pplat->devfn = PCI_MASK_BUS(bdf);
		pplat->vendor = vendor;
		pplat->device = device;
		pplat->class = class;
		pplat->is_virtfn = true;
		pplat->pfdev = pdev;
		pplat->virtid = vf * vf_stride + vf_offset;

		debug("%s: bus %d/%s: found VF %x:%x %x:%x class %lx id %x\n",
		      __func__, dev_seq(dev), dev->name, PCI_DEV(bdf),
		      PCI_FUNC(bdf), vendor, device, class, pplat->virtid);
		bdf += PCI_BDF(0, 0, vf_stride);
	}

	return 0;
}

int pci_sriov_get_totalvfs(struct udevice *pdev)
{
	u16 total_vf;
	int pos;

	pos = dm_pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_SRIOV);
	if (!pos) {
		debug("Error: SRIOV capability not found\n");
		return -ENOENT;
	}

	dm_pci_read_config16(pdev, pos + PCI_SRIOV_TOTAL_VF, &total_vf);

	return total_vf;
}
#endif /* SRIOV */

UCLASS_DRIVER(pci) = {
	.id		= UCLASS_PCI,
	.name		= "pci",
	.flags		= DM_UC_FLAG_SEQ_ALIAS | DM_UC_FLAG_NO_AUTO_SEQ,
	.post_bind	= dm_scan_fdt_dev,
	.pre_probe	= pci_uclass_pre_probe,
	.post_probe	= pci_uclass_post_probe,
	.child_post_bind = pci_uclass_child_post_bind,
	.per_device_auto	= sizeof(struct pci_controller),
	.per_child_plat_auto	= sizeof(struct pci_child_plat),
};

static const struct dm_pci_ops pci_bridge_ops = {
	.read_config	= pci_bridge_read_config,
	.write_config	= pci_bridge_write_config,
};

static const struct udevice_id pci_bridge_ids[] = {
	{ .compatible = "pci-bridge" },
	{ }
};

U_BOOT_DRIVER(pci_bridge_drv) = {
	.name		= "pci_bridge_drv",
	.id		= UCLASS_PCI,
	.of_match	= pci_bridge_ids,
	.ops		= &pci_bridge_ops,
};

UCLASS_DRIVER(pci_generic) = {
	.id		= UCLASS_PCI_GENERIC,
	.name		= "pci_generic",
};

static const struct udevice_id pci_generic_ids[] = {
	{ .compatible = "pci-generic" },
	{ }
};

U_BOOT_DRIVER(pci_generic_drv) = {
	.name		= "pci_generic_drv",
	.id		= UCLASS_PCI_GENERIC,
	.of_match	= pci_generic_ids,
};

int pci_init(void)
{
	struct udevice *bus;

	/*
	 * Enumerate all known controller devices. Enumeration has the side-
	 * effect of probing them, so PCIe devices will be enumerated too.
	 */
	for (uclass_first_device_check(UCLASS_PCI, &bus);
	     bus;
	     uclass_next_device_check(&bus)) {
		;
	}

	return 0;
}
