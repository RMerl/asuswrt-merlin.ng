/*
 * OF helpers for the MDIO (Ethernet PHY) API
 *
 * Copyright (c) 2009 Secret Lab Technologies, Ltd.
 *
 * This file is released under the GPLv2
 *
 * This file provides helper functions for extracting PHY device information
 * out of the OpenFirmware device tree and using it to populate an mii_bus.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/err.h>
#include <linux/phy.h>
#include <linux/phy_fixed.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_mdio.h>
#include <linux/module.h>

MODULE_AUTHOR("Grant Likely <grant.likely@secretlab.ca>");
MODULE_LICENSE("GPL");

/* Extract the clause 22 phy ID from the compatible string of the form
 * ethernet-phy-idAAAA.BBBB */
static int of_get_phy_id(struct device_node *device, u32 *phy_id)
{
	struct property *prop;
	const char *cp;
	unsigned int upper, lower;

	of_property_for_each_string(device, "compatible", prop, cp) {
		if (sscanf(cp, "ethernet-phy-id%4x.%4x", &upper, &lower) == 2) {
			*phy_id = ((upper & 0xFFFF) << 16) | (lower & 0xFFFF);
			return 0;
		}
	}
	return -EINVAL;
}

static int of_mdiobus_register_phy(struct mii_bus *mdio, struct device_node *child,
				   u32 addr)
{
	struct phy_device *phy;
	bool is_c45;
	int rc;
	u32 phy_id;

	is_c45 = of_device_is_compatible(child,
					 "ethernet-phy-ieee802.3-c45");

	if (!is_c45 && !of_get_phy_id(child, &phy_id))
		phy = phy_device_create(mdio, addr, phy_id, 0, NULL);
	else
		phy = get_phy_device(mdio, addr, is_c45);
	if (!phy || IS_ERR(phy))
		return 1;

	rc = irq_of_parse_and_map(child, 0);
	if (rc > 0) {
		phy->irq = rc;
		if (mdio->irq)
			mdio->irq[addr] = rc;
	} else {
		if (mdio->irq)
			phy->irq = mdio->irq[addr];
	}

	/* Associate the OF node with the device structure so it
	 * can be looked up later */
	of_node_get(child);
	phy->dev.of_node = child;

	/* All data is now stored in the phy struct;
	 * register it */
	rc = phy_device_register(phy);
	if (rc) {
		phy_device_free(phy);
		of_node_put(child);
		return 1;
	}

	dev_dbg(&mdio->dev, "registered phy %s at address %i\n",
		child->name, addr);

	return 0;
}

int of_mdio_parse_addr(struct device *dev, const struct device_node *np)
{
	u32 addr;
	int ret;

	ret = of_property_read_u32(np, "reg", &addr);
	if (ret < 0) {
		dev_err(dev, "%s has invalid PHY address\n", np->full_name);
		return ret;
	}

	/* A PHY must have a reg property in the range [0-31] */
	if (addr >= PHY_MAX_ADDR) {
		dev_err(dev, "%s PHY address %i is too large\n",
			np->full_name, addr);
		return -EINVAL;
	}

	return addr;
}
EXPORT_SYMBOL(of_mdio_parse_addr);

/**
 * of_mdiobus_register - Register mii_bus and create PHYs from the device tree
 * @mdio: pointer to mii_bus structure
 * @np: pointer to device_node of MDIO bus.
 *
 * This function registers the mii_bus structure and registers a phy_device
 * for each child node of @np.
 */
int of_mdiobus_register(struct mii_bus *mdio, struct device_node *np)
{
	struct device_node *child;
	const __be32 *paddr;
	bool scanphys = false;
	int addr, rc, i;

	/* Mask out all PHYs from auto probing.  Instead the PHYs listed in
	 * the device tree are populated after the bus has been registered */
	mdio->phy_mask = ~0;

	/* Clear all the IRQ properties */
	if (mdio->irq)
		for (i=0; i<PHY_MAX_ADDR; i++)
			mdio->irq[i] = PHY_POLL;

	mdio->dev.of_node = np;

	/* Register the MDIO bus */
	rc = mdiobus_register(mdio);
	if (rc)
		return rc;

	/* Loop over the child nodes and register a phy_device for each one */
	for_each_available_child_of_node(np, child) {
		addr = of_mdio_parse_addr(&mdio->dev, child);
		if (addr < 0) {
			scanphys = true;
			continue;
		}

		rc = of_mdiobus_register_phy(mdio, child, addr);
		if (rc)
			continue;
	}

	if (!scanphys)
		return 0;

	/* auto scan for PHYs with empty reg property */
	for_each_available_child_of_node(np, child) {
		/* Skip PHYs with reg property set */
		paddr = of_get_property(child, "reg", NULL);
		if (paddr)
			continue;

		for (addr = 0; addr < PHY_MAX_ADDR; addr++) {
			/* skip already registered PHYs */
			if (mdio->phy_map[addr])
				continue;

			/* be noisy to encourage people to set reg property */
			dev_info(&mdio->dev, "scan phy %s at address %i\n",
				 child->name, addr);

			rc = of_mdiobus_register_phy(mdio, child, addr);
			if (rc)
				continue;
		}
	}

	return 0;
}
EXPORT_SYMBOL(of_mdiobus_register);

/* Helper function for of_phy_find_device */
static int of_phy_match(struct device *dev, void *phy_np)
{
	return dev->of_node == phy_np;
}

/**
 * of_phy_find_device - Give a PHY node, find the phy_device
 * @phy_np: Pointer to the phy's device tree node
 *
 * Returns a pointer to the phy_device.
 */
struct phy_device *of_phy_find_device(struct device_node *phy_np)
{
	struct device *d;
	if (!phy_np)
		return NULL;

	d = bus_find_device(&mdio_bus_type, NULL, phy_np, of_phy_match);
	return d ? to_phy_device(d) : NULL;
}
EXPORT_SYMBOL(of_phy_find_device);

/**
 * of_phy_connect - Connect to the phy described in the device tree
 * @dev: pointer to net_device claiming the phy
 * @phy_np: Pointer to device tree node for the PHY
 * @hndlr: Link state callback for the network device
 * @iface: PHY data interface type
 *
 * Returns a pointer to the phy_device if successful.  NULL otherwise
 */
struct phy_device *of_phy_connect(struct net_device *dev,
				  struct device_node *phy_np,
				  void (*hndlr)(struct net_device *), u32 flags,
				  phy_interface_t iface)
{
	struct phy_device *phy = of_phy_find_device(phy_np);

	if (!phy)
		return NULL;

	phy->dev_flags = flags;

	return phy_connect_direct(dev, phy, hndlr, iface) ? NULL : phy;
}
EXPORT_SYMBOL(of_phy_connect);

/**
 * of_phy_attach - Attach to a PHY without starting the state machine
 * @dev: pointer to net_device claiming the phy
 * @phy_np: Node pointer for the PHY
 * @flags: flags to pass to the PHY
 * @iface: PHY data interface type
 */
struct phy_device *of_phy_attach(struct net_device *dev,
				 struct device_node *phy_np, u32 flags,
				 phy_interface_t iface)
{
	struct phy_device *phy = of_phy_find_device(phy_np);

	if (!phy)
		return NULL;

	return phy_attach_direct(dev, phy, flags, iface) ? NULL : phy;
}
EXPORT_SYMBOL(of_phy_attach);

#if defined(CONFIG_FIXED_PHY)
/*
 * of_phy_is_fixed_link() and of_phy_register_fixed_link() must
 * support two DT bindings:
 * - the old DT binding, where 'fixed-link' was a property with 5
 *   cells encoding various informations about the fixed PHY
 * - the new DT binding, where 'fixed-link' is a sub-node of the
 *   Ethernet device.
 */
bool of_phy_is_fixed_link(struct device_node *np)
{
	struct device_node *dn;
	int len, err;
	const char *managed;

	/* New binding */
	dn = of_get_child_by_name(np, "fixed-link");
	if (dn) {
		of_node_put(dn);
		return true;
	}

	err = of_property_read_string(np, "managed", &managed);
	if (err == 0 && strcmp(managed, "auto") != 0)
		return true;

	/* Old binding */
	if (of_get_property(np, "fixed-link", &len) &&
	    len == (5 * sizeof(__be32)))
		return true;

	return false;
}
EXPORT_SYMBOL(of_phy_is_fixed_link);

int of_phy_register_fixed_link(struct device_node *np)
{
	struct fixed_phy_status status = {};
	struct device_node *fixed_link_node;
	const __be32 *fixed_link_prop;
	int len, err;
	struct phy_device *phy;
	const char *managed;

	err = of_property_read_string(np, "managed", &managed);
	if (err == 0) {
		if (strcmp(managed, "in-band-status") == 0) {
			/* status is zeroed, namely its .link member */
			phy = fixed_phy_register(PHY_POLL, &status, np);
			return IS_ERR(phy) ? PTR_ERR(phy) : 0;
		}
	}

	/* New binding */
	fixed_link_node = of_get_child_by_name(np, "fixed-link");
	if (fixed_link_node) {
		status.link = 1;
		status.duplex = of_property_read_bool(fixed_link_node,
						      "full-duplex");
		if (of_property_read_u32(fixed_link_node, "speed", &status.speed))
			return -EINVAL;
		status.pause = of_property_read_bool(fixed_link_node, "pause");
		status.asym_pause = of_property_read_bool(fixed_link_node,
							  "asym-pause");
		of_node_put(fixed_link_node);
		phy = fixed_phy_register(PHY_POLL, &status, np);
		return IS_ERR(phy) ? PTR_ERR(phy) : 0;
	}

	/* Old binding */
	fixed_link_prop = of_get_property(np, "fixed-link", &len);
	if (fixed_link_prop && len == (5 * sizeof(__be32))) {
		status.link = 1;
		status.duplex = be32_to_cpu(fixed_link_prop[1]);
		status.speed = be32_to_cpu(fixed_link_prop[2]);
		status.pause = be32_to_cpu(fixed_link_prop[3]);
		status.asym_pause = be32_to_cpu(fixed_link_prop[4]);
		phy = fixed_phy_register(PHY_POLL, &status, np);
		return IS_ERR(phy) ? PTR_ERR(phy) : 0;
	}

	return -ENODEV;
}
EXPORT_SYMBOL(of_phy_register_fixed_link);
#endif
