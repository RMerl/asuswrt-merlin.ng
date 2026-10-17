/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2021 NXP
 */

#ifndef __DSA_H__
#define __DSA_H__

#include <dm/ofnode.h>
#include <phy.h>
#include <net.h>

/**
 * DSA stands for Distributed Switch Architecture and it is infrastructure
 * intended to support drivers for Switches that rely on an intermediary
 * Ethernet device for I/O.  These switches may support cascading allowing
 * them to be arranged as a tree.
 * DSA is documented in detail in the Linux kernel documentation under
 * Documentation/networking/dsa/dsa.txt
 * The network layout of such a switch is shown below:
 *
 *                      |------|
 *                      | eth0 | <--- master eth device (regular eth driver)
 *                      |------|
 *                        ^  |
 * tag added by switch -->|  |
 *                        |  |
 *                        |  |<-- tag added by DSA driver
 *                        |  v
 *      |--------------------------------------|
 *      |             | CPU port |             | <-- DSA (switch) device
 *      |             ------------             |     (DSA driver)
 *      | _________  _________       _________ |
 *      | | port0 |  | port1 |  ...  | portn | | <-- ports as eth devices
 *      |-+-------+--+-------+-------+-------+-|     ('dsa-port' eth driver)
 *
 * In U-Boot the intent is to allow access to front panel ports (shown at the
 * bottom of the picture) through the master Ethernet dev (eth0 in the picture).
 * Front panel ports are presented as regular Ethernet devices in U-Boot and
 * they are expected to support the typical networking commands.
 * In general DSA switches require the use of tags, extra headers added both by
 * software on Tx and by the switch on Rx.  These tags carry at a minimum port
 * information and switch information for cascaded set-ups.
 * In U-Boot these tags are inserted and parsed by the DSA switch driver, the
 * class code helps with headroom/tailroom for the extra headers.
 *
 * TODO:
 * - handle switch cascading, for now U-Boot only supports stand-alone switches.
 * - Add support to probe DSA switches connected to a MDIO bus, this is needed
 * to convert switch drivers that are now under drivers/net/phy.
 */

#define DSA_PORT_NAME_LENGTH	16

/* Maximum number of ports each DSA device can have */
#define DSA_MAX_PORTS		12

/**
 * struct dsa_ops - DSA operations
 *
 * @port_probe:   Initialize a switch port.
 * @port_enable:  Enable I/O for a port.
 * @port_disable: Disable I/O for a port.
 * @xmit:         Insert the DSA tag for transmission.
 *                DSA drivers receive a copy of the packet with headroom and
 *                tailroom reserved and set to 0. 'packet' points to headroom
 *                and 'length' is updated to include both head and tailroom.
 * @rcv:          Process the DSA tag on reception and return the port index
 *                from the h/w provided tag. Return the index via 'portp'.
 *                'packet' and 'length' describe the frame as received from
 *                master including any additional headers.
 */
struct dsa_ops {
	int (*port_probe)(struct udevice *dev, int port,
			  struct phy_device *phy);
	int (*port_enable)(struct udevice *dev, int port,
			   struct phy_device *phy);
	void (*port_disable)(struct udevice *dev, int port,
			     struct phy_device *phy);
	int (*xmit)(struct udevice *dev, int port, void *packet, int length);
	int (*rcv)(struct udevice *dev, int *portp, void *packet, int length);
};

#define dsa_get_ops(dev) ((struct dsa_ops *)(dev)->driver->ops)

/**
 * struct dsa_port_pdata - DSA port platform data
 *
 * @phy:   PHY device associated with this port.
 *         The uclass code attempts to set this field for all ports except CPU
 *         port, based on DT information.  It may be NULL.
 * @index: Port index in the DSA switch, set by the uclass code.
 * @name:  Name of the port Eth device.  If a label property is present in the
 *         port DT node, it is used as name.
 */
struct dsa_port_pdata {
	struct phy_device *phy;
	u32 index;
	char name[DSA_PORT_NAME_LENGTH];
};

/**
 * struct dsa_pdata - Per-device platform data for DSA DM
 *
 * @num_ports:   Number of ports the device has, must be <= DSA_MAX_PORTS.
 *		 This number is extracted from the DT 'ports' node of this
 *		 DSA device, and it counts the CPU port and all the other
 *		 port subnodes including the disabled ones.
 * @cpu_port:    Index of the switch port linked to the master Ethernet.
 *		 The uclass code sets this based on DT information.
 * @master_node: OF node of the host Ethernet controller.
 * @cpu_port_node: DT node of the switch's CPU port.
 */
struct dsa_pdata {
	int num_ports;
	u32 cpu_port;
	ofnode master_node;
	ofnode cpu_port_node;
};

/**
 * dsa_set_tagging() - Configure the headroom and/or tailroom sizes
 *
 * The DSA class code allocates headroom and tailroom on Tx before
 * calling the DSA driver's xmit function.
 * All drivers must call this at probe time.
 *
 * @dev:	DSA device pointer
 * @headroom:	Size, in bytes, of headroom needed for the DSA tag.
 * @tailroom:	Size, in bytes, of tailroom needed for the DSA tag.
 *		Total headroom and tailroom size should not exceed
 *		DSA_MAX_OVR.
 * Return: 0 if OK, -ve on error
 */
int dsa_set_tagging(struct udevice *dev, ushort headroom, ushort tailroom);

/* DSA helpers */

/**
 * dsa_get_master() - Return a reference to the master Ethernet device
 *
 * Can be called at driver probe time or later.
 *
 * @dev:	DSA device pointer
 * Return: Master Eth 'udevice' pointer if OK, NULL on error
 */
struct udevice *dsa_get_master(struct udevice *dev);

/**
 * dsa_port_get_ofnode() - Return a reference to the given port's OF node
 *
 * Can be called at driver probe time or later.
 *
 * @dev:	DSA switch udevice pointer
 * @port:	Port index
 * Return: OF node reference if OK, NULL on error
 */
ofnode dsa_port_get_ofnode(struct udevice *dev, int port);

/**
 * dsa_port_get_pdata() - Helper that returns the platdata of an active
 *			(non-CPU) DSA port device.
 *
 * Can be called at driver probe time or later.
 *
 * @pdev:	DSA port device pointer
 * Return: 'dsa_port_pdata' pointer if OK, NULL on error
 */
static inline struct dsa_port_pdata *
	dsa_port_get_pdata(struct udevice *pdev)
{
	struct eth_pdata *eth = dev_get_plat(pdev);

	if (!eth)
		return NULL;

	return eth->priv_pdata;
}

#endif /* __DSA_H__ */
