// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium PCIE host driver
 *
 * Heavily based on drivers/pci/pcie_xilinx.c
 *
 * Copyright (C) 2019
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/printk.h>

/**
 * struct phytium_pcie - phytium PCIe controller state
 * @cfg_base: The base address of memory mapped configuration space
 */
struct phytium_pcie {
	void *cfg_base;
};

/*
 * phytium_pci_skip_dev()
 * @parent: Identifies the PCIe device to access
 *
 * Checks whether the parent of the PCIe device is bridge
 *
 * Return: true if it is bridge, else false.
 */
static int phytium_pci_skip_dev(pci_dev_t parent)
{
	unsigned char pos, id;
	unsigned long addr = 0x40000000;
	unsigned short capreg;
	unsigned char port_type;

	addr += PCIE_ECAM_OFFSET(PCI_BUS(parent), PCI_DEV(parent), PCI_FUNC(parent), 0);

	pos = 0x34;
	while (1) {
		pos = readb(addr + pos);
		if (pos < 0x40)
			break;
		pos &= ~3;
		id = readb(addr + pos);
		if (id == 0xff)
			break;
		if (id == 0x10) {
			capreg = readw(addr + pos + 2);
			port_type = (capreg >> 4) & 0xf;
			if (port_type == 0x6 || port_type == 0x4)
				return 1;
			else
				return 0;
		}
		pos += 1;
	}
	return 0;
}

/**
 * pci_phytium_conf_address() - Calculate the address of a config access
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @paddress: Pointer to the pointer to write the calculates address to
 *
 * Calculates the address that should be accessed to perform a PCIe
 * configuration space access for a given device identified by the PCIe
 * controller device @pcie and the bus, device & function numbers in @bdf. If
 * access to the device is not valid then the function will return an error
 * code. Otherwise the address to access will be written to the pointer pointed
 * to by @paddress.
 */
static int pci_phytium_conf_address(const struct udevice *bus, pci_dev_t bdf,
				    uint offset, void **paddress)
{
	struct phytium_pcie *pcie = dev_get_priv(bus);
	void *addr;
	pci_dev_t bdf_parent;

	unsigned int bus_no = PCI_BUS(bdf);
	unsigned int dev_no = PCI_DEV(bdf);

	bdf_parent = PCI_BDF((bus_no - 1), 0, 0);

	addr = pcie->cfg_base;
	addr += PCIE_ECAM_OFFSET(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), 0);

	if (bus_no > 0 && dev_no > 0) {
		if ((readb(addr + PCI_HEADER_TYPE) & 0x7f) !=
				PCI_HEADER_TYPE_BRIDGE)
			return -ENODEV;
		if (phytium_pci_skip_dev(bdf_parent))
			return -ENODEV;
	}

	addr += offset;
	*paddress = addr;

	return 0;
}

/**
 * pci_phytium_read_config() - Read from configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 */
static int pci_phytium_read_config(const struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, pci_phytium_conf_address,
					    bdf, offset, valuep, size);
}

/**
 * pci_phytium_write_config() - Write to configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 */
static int pci_phytium_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, pci_phytium_conf_address,
					     bdf, offset, value, size);
}

/**
 * pci_phytium_of_to_plat() - Translate from DT to device state
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pci_phytium_of_to_plat(struct udevice *dev)
{
	struct phytium_pcie *pcie = dev_get_priv(dev);
	struct fdt_resource reg_res;

	DECLARE_GLOBAL_DATA_PTR;

	int err;

	err = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev), "reg",
			       0, &reg_res);
	if (err < 0) {
		pr_err("\"reg\" resource not found\n");
		return err;
	}

	pcie->cfg_base = map_physmem(reg_res.start,
				     fdt_resource_size(&reg_res),
				     MAP_NOCACHE);

	return 0;
}

static const struct dm_pci_ops pci_phytium_ops = {
	.read_config	= pci_phytium_read_config,
	.write_config	= pci_phytium_write_config,
};

static const struct udevice_id pci_phytium_ids[] = {
	{ .compatible = "phytium,pcie-host-1.0" },
	{ }
};

U_BOOT_DRIVER(pci_phytium) = {
	.name			= "pci_phytium",
	.id			= UCLASS_PCI,
	.of_match		= pci_phytium_ids,
	.ops			= &pci_phytium_ops,
	.of_to_plat	= pci_phytium_of_to_plat,
	.priv_auto	= sizeof(struct phytium_pcie),
};
