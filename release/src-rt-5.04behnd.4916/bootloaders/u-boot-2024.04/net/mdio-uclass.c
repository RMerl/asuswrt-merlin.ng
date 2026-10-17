// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/of_extra.h>
#include <dm/uclass-internal.h>
#include <linux/compat.h>

void dm_mdio_probe_devices(void)
{
	struct udevice *it;
	struct uclass *uc;

	uclass_get(UCLASS_MDIO, &uc);
	uclass_foreach_dev(it, uc) {
		device_probe(it);
	}
}

static int dm_mdio_post_bind(struct udevice *dev)
{
	const char *dt_name;

	/* set a custom name for the MDIO device, if present in DT */
	if (dev_has_ofnode(dev)) {
		dt_name = dev_read_string(dev, "device-name");
		if (dt_name) {
			debug("renaming dev %s to %s\n", dev->name, dt_name);
			device_set_name(dev, dt_name);
		}
	}

	/*
	 * MDIO command doesn't like spaces in names, don't allow them to keep
	 * it happy
	 */
	if (strchr(dev->name, ' ')) {
		debug("\nError: MDIO device name \"%s\" has a space!\n",
		      dev->name);
		return -EINVAL;
	}

#if CONFIG_IS_ENABLED(OF_REAL)
	return dm_scan_fdt_dev(dev);
#else
	return 0;
#endif
}

int dm_mdio_read(struct udevice *mdio_dev, int addr, int devad, int reg)
{
	struct mdio_ops *ops = mdio_get_ops(mdio_dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(mdio_dev, addr, devad, reg);
}

int dm_mdio_write(struct udevice *mdio_dev, int addr, int devad, int reg,
		  u16 val)
{
	struct mdio_ops *ops = mdio_get_ops(mdio_dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(mdio_dev, addr, devad, reg, val);
}

int dm_mdio_reset(struct udevice *mdio_dev)
{
	struct mdio_ops *ops = mdio_get_ops(mdio_dev);

	if (!ops->reset)
		return 0;

	return ops->reset(mdio_dev);
}

/*
 * Following read/write/reset functions are registered with legacy MII code.
 * These are called for PHY operations by upper layers and we further call the
 * DM MDIO driver functions.
 */
static int mdio_read(struct mii_dev *mii_bus, int addr, int devad, int reg)
{
	return dm_mdio_read(mii_bus->priv, addr, devad, reg);
}

static int mdio_write(struct mii_dev *mii_bus, int addr, int devad, int reg,
		      u16 val)
{
	return dm_mdio_write(mii_bus->priv, addr, devad, reg, val);
}

static int mdio_reset(struct mii_dev *mii_bus)
{
	return dm_mdio_reset(mii_bus->priv);
}

static int dm_mdio_post_probe(struct udevice *dev)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(dev);

	pdata->mii_bus = mdio_alloc();
	pdata->mii_bus->read = mdio_read;
	pdata->mii_bus->write = mdio_write;
	pdata->mii_bus->reset = mdio_reset;
	pdata->mii_bus->priv = dev;
	strlcpy(pdata->mii_bus->name, dev->name, MDIO_NAME_LEN);

	return mdio_register(pdata->mii_bus);
}

static int dm_mdio_pre_remove(struct udevice *dev)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(dev);

	dm_mdio_reset(dev);
	mdio_unregister(pdata->mii_bus);
	mdio_free(pdata->mii_bus);

	return 0;
}

struct phy_device *dm_phy_find_by_ofnode(ofnode phynode)
{
	struct mdio_perdev_priv *pdata;
	struct udevice *mdiodev;
	u32 phy_addr;

	if (ofnode_read_u32(phynode, "reg", &phy_addr))
		return NULL;

	if (uclass_get_device_by_ofnode(UCLASS_MDIO,
					ofnode_get_parent(phynode),
					&mdiodev))
		return NULL;

	if (device_probe(mdiodev))
		return NULL;

	pdata = dev_get_uclass_priv(mdiodev);

	return phy_find_by_mask(pdata->mii_bus, BIT(phy_addr));
}

struct phy_device *dm_mdio_phy_connect(struct udevice *mdiodev, int phyaddr,
				       struct udevice *ethdev,
				       phy_interface_t interface)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(mdiodev);

	if (device_probe(mdiodev))
		return NULL;

	return phy_connect(pdata->mii_bus, phyaddr, ethdev, interface);
}

static struct phy_device *dm_eth_connect_phy_handle(struct udevice *ethdev,
						    phy_interface_t interface)
{
	u32 phy_addr;
	struct udevice *mdiodev;
	struct phy_device *phy;
	ofnode phynode;

	if (IS_ENABLED(CONFIG_PHY_FIXED) &&
	    ofnode_phy_is_fixed_link(dev_ofnode(ethdev), &phynode)) {
		phy = phy_connect(NULL, 0, ethdev, interface);
		goto out;
	}

	phynode = dev_get_phy_node(ethdev);
	if (!ofnode_valid(phynode)) {
		dev_dbg(ethdev, "can't find PHY node\n");
		return NULL;
	}

	/*
	 * reading 'reg' directly should be fine.  This is a PHY node, the
	 * address is always size 1 and requires no translation
	 */
	if (ofnode_read_u32(phynode, "reg", &phy_addr)) {
		dev_dbg(ethdev, "missing reg property in phy node\n");
		return NULL;
	}

	if (uclass_get_device_by_ofnode(UCLASS_MDIO,
					ofnode_get_parent(phynode),
					&mdiodev)) {
		dev_dbg(ethdev, "can't find MDIO bus for node %s\n",
			ofnode_get_name(ofnode_get_parent(phynode)));
		return NULL;
	}

	phy = dm_mdio_phy_connect(mdiodev, phy_addr, ethdev, interface);

out:
	if (phy)
		phy->node = phynode;

	return phy;
}

/* Connect to a PHY linked in eth DT node */
struct phy_device *dm_eth_phy_connect(struct udevice *ethdev)
{
	phy_interface_t interface;
	struct phy_device *phy;

	if (!dev_has_ofnode(ethdev)) {
		debug("%s: supplied eth dev has no DT node!\n", ethdev->name);
		return NULL;
	}

	interface = dev_read_phy_mode(ethdev);
	if (interface == PHY_INTERFACE_MODE_NA)
		dev_dbg(ethdev, "can't find interface mode, default to NA\n");

	phy = dm_eth_connect_phy_handle(ethdev, interface);

	if (!phy)
		return NULL;

	phy->interface = interface;

	return phy;
}

UCLASS_DRIVER(mdio) = {
	.id = UCLASS_MDIO,
	.name = "mdio",
	.post_bind  = dm_mdio_post_bind,
	.post_probe = dm_mdio_post_probe,
	.pre_remove = dm_mdio_pre_remove,
	.per_device_auto	= sizeof(struct mdio_perdev_priv),
};
