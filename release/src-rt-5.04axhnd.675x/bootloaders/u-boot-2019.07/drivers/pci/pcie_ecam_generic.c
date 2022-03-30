// SPDX-License-Identifier: GPL-2.0
/*
 * Generic PCIE host provided by e.g. QEMU
 *
 * Heavily based on drivers/pci/pcie_xilinx.c
 *
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <dm.h>
#include <pci.h>

#include <asm/io.h>

/**
 * struct generic_ecam_pcie - generic_ecam PCIe controller state
 * @cfg_base: The base address of memory mapped configuration space
 */
struct generic_ecam_pcie {
	void *cfg_base;
};

/**
 * pci_generic_ecam_conf_address() - Calculate the address of a config access
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
static int pci_generic_ecam_conf_address(struct udevice *bus, pci_dev_t bdf,
					    uint offset, void **paddress)
{
	struct generic_ecam_pcie *pcie = dev_get_priv(bus);
	void *addr;

	addr = pcie->cfg_base;
	addr += PCI_BUS(bdf) << 20;
	addr += PCI_DEV(bdf) << 15;
	addr += PCI_FUNC(bdf) << 12;
	addr += offset;
	*paddress = addr;

	return 0;
}

/**
 * pci_generic_ecam_read_config() - Read from configuration space
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
static int pci_generic_ecam_read_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, pci_generic_ecam_conf_address,
					    bdf, offset, valuep, size);
}

/**
 * pci_generic_ecam_write_config() - Write to configuration space
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
static int pci_generic_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, pci_generic_ecam_conf_address,
					     bdf, offset, value, size);
}

/**
 * pci_generic_ecam_ofdata_to_platdata() - Translate from DT to device state
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pci_generic_ecam_ofdata_to_platdata(struct udevice *dev)
{
	struct generic_ecam_pcie *pcie = dev_get_priv(dev);
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

static const struct dm_pci_ops pci_generic_ecam_ops = {
	.read_config	= pci_generic_ecam_read_config,
	.write_config	= pci_generic_ecam_write_config,
};

static const struct udevice_id pci_generic_ecam_ids[] = {
	{ .compatible = "pci-host-ecam-generic" },
	{ }
};

U_BOOT_DRIVER(pci_generic_ecam) = {
	.name			= "pci_generic_ecam",
	.id			= UCLASS_PCI,
	.of_match		= pci_generic_ecam_ids,
	.ops			= &pci_generic_ecam_ops,
	.ofdata_to_platdata	= pci_generic_ecam_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct generic_ecam_pcie),
};
