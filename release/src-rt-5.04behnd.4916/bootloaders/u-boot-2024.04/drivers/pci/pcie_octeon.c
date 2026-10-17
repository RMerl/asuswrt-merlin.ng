// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <pci.h>
#include <linux/delay.h>

#include <mach/octeon-model.h>
#include <mach/octeon_pci.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-pemx-defs.h>

struct octeon_pcie {
	void *base;
	int first_busno;
	u32 port;
	struct udevice *dev;
	int pcie_port;
};

static bool octeon_bdf_invalid(pci_dev_t bdf, int first_busno)
{
	/*
	 * In PCIe only a single device (0) can exist on the local bus.
	 * Beyound the local bus, there might be a switch and everything
	 * is possible.
	 */
	if ((PCI_BUS(bdf) == first_busno) && (PCI_DEV(bdf) > 0))
		return true;

	return false;
}

static int pcie_octeon_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct octeon_pcie *pcie = dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	int busno;
	int port;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,size,val)=(0x%04x, %d, 0x%08lx)\n", offset, size, value);

	port = pcie->pcie_port;
	busno = PCI_BUS(bdf) - hose->first_busno + 1;

	switch (size) {
	case PCI_SIZE_8:
		cvmx_pcie_config_write8(port, busno, PCI_DEV(bdf),
					PCI_FUNC(bdf), offset, value);
		break;
	case PCI_SIZE_16:
		cvmx_pcie_config_write16(port, busno, PCI_DEV(bdf),
					 PCI_FUNC(bdf), offset, value);
		break;
	case PCI_SIZE_32:
		cvmx_pcie_config_write32(port, busno, PCI_DEV(bdf),
					 PCI_FUNC(bdf), offset, value);
		break;
	default:
		printf("Invalid size\n");
	};

	return 0;
}

static int pcie_octeon_read_config(const struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	struct octeon_pcie *pcie = dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	int busno;
	int port;

	port = pcie->pcie_port;
	busno = PCI_BUS(bdf) - hose->first_busno + 1;
	if (octeon_bdf_invalid(bdf, pcie->first_busno)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	switch (size) {
	case PCI_SIZE_8:
		*valuep = cvmx_pcie_config_read8(port, busno, PCI_DEV(bdf),
						 PCI_FUNC(bdf), offset);
		break;
	case PCI_SIZE_16:
		*valuep = cvmx_pcie_config_read16(port, busno, PCI_DEV(bdf),
						  PCI_FUNC(bdf), offset);
		break;
	case PCI_SIZE_32:
		*valuep = cvmx_pcie_config_read32(port, busno, PCI_DEV(bdf),
						  PCI_FUNC(bdf), offset);
		break;
	default:
		printf("Invalid size\n");
	};

	debug("%02x.%02x.%02x: u%d %x -> %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset, *valuep);

	return 0;
}

static int pcie_octeon_probe(struct udevice *dev)
{
	struct octeon_pcie *pcie = dev_get_priv(dev);
	int node = cvmx_get_node_num();
	int pcie_port;
	int ret = 0;

	/* Get port number, lane number and memory target / attr */
	if (ofnode_read_u32(dev_ofnode(dev), "marvell,pcie-port",
			    &pcie->port)) {
		ret = -ENODEV;
		goto err;
	}

	pcie->first_busno = dev_seq(dev);
	pcie_port = ((node << 4) | pcie->port);
	ret = cvmx_pcie_rc_initialize(pcie_port);
	if (ret != 0)
		return ret;

	return 0;

err:
	return ret;
}

static const struct dm_pci_ops pcie_octeon_ops = {
	.read_config = pcie_octeon_read_config,
	.write_config = pcie_octeon_write_config,
};

static const struct udevice_id pcie_octeon_ids[] = {
	{ .compatible = "marvell,pcie-host-octeon" },
	{ }
};

U_BOOT_DRIVER(pcie_octeon) = {
	.name		= "pcie_octeon",
	.id		= UCLASS_PCI,
	.of_match	= pcie_octeon_ids,
	.ops		= &pcie_octeon_ops,
	.probe		= pcie_octeon_probe,
	.priv_auto	= sizeof(struct octeon_pcie),
	.flags		= DM_FLAG_PRE_RELOC,
};
