// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx AXI Bridge for PCI Express Driver
 *
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <linux/bitops.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/err.h>

/**
 * struct xilinx_pcie - Xilinx PCIe controller state
 * @cfg_base: The base address of memory mapped configuration space
 */
struct xilinx_pcie {
	void *cfg_base;
};

/* Register definitions */
#define XILINX_PCIE_REG_PSCR		0x144
#define XILINX_PCIE_REG_PSCR_LNKUP	BIT(11)
#define XILINX_PCIE_REG_RPSC		0x148
#define XILINX_PCIE_REG_RPSC_BEN	BIT(0)

/**
 * pcie_xilinx_link_up() - Check whether the PCIe link is up
 * @pcie: Pointer to the PCI controller state
 *
 * Checks whether the PCIe link for the given device is up or down.
 *
 * Return: true if the link is up, else false
 */
static bool pcie_xilinx_link_up(struct xilinx_pcie *pcie)
{
	uint32_t pscr = __raw_readl(pcie->cfg_base + XILINX_PCIE_REG_PSCR);

	return pscr & XILINX_PCIE_REG_PSCR_LNKUP;
}

/**
 * pcie_xilinx_config_address() - Calculate the address of a config access
 * @udev: Pointer to the PCI bus
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
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_xilinx_config_address(const struct udevice *udev, pci_dev_t bdf,
				      uint offset, void **paddress)
{
	struct xilinx_pcie *pcie = dev_get_priv(udev);
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int func = PCI_FUNC(bdf);
	void *addr;

	if ((bus > 0) && !pcie_xilinx_link_up(pcie))
		return -ENODEV;

	/*
	 * Busses 0 (host-PCIe bridge) & 1 (its immediate child) are
	 * limited to a single device each.
	 */
	if ((bus < 2) && (dev > 0))
		return -ENODEV;

	addr = pcie->cfg_base;
	addr += PCIE_ECAM_OFFSET(bus, dev, func, offset);
	*paddress = addr;

	return 0;
}

/**
 * pcie_xilinx_read_config() - Read from configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success, else -ENODEV or -EINVAL
 */
static int pcie_xilinx_read_config(const struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, pcie_xilinx_config_address,
					    bdf, offset, valuep, size);
}

/**
 * pcie_xilinx_write_config() - Write to configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success, else -ENODEV or -EINVAL
 */
static int pcie_xilinx_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, pcie_xilinx_config_address,
					     bdf, offset, value, size);
}

/**
 * pcie_xilinx_of_to_plat() - Translate from DT to device state
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_xilinx_of_to_plat(struct udevice *dev)
{
	struct xilinx_pcie *pcie = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;
	u32 rpsc;

	addr = dev_read_addr_size(dev, &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	pcie->cfg_base = devm_ioremap(dev, addr, size);
	if (IS_ERR(pcie->cfg_base))
		return PTR_ERR(pcie->cfg_base);

	/* Enable the Bridge enable bit */
	rpsc = __raw_readl(pcie->cfg_base + XILINX_PCIE_REG_RPSC);
	rpsc |= XILINX_PCIE_REG_RPSC_BEN;
	__raw_writel(rpsc, pcie->cfg_base + XILINX_PCIE_REG_RPSC);

	return 0;
}

static const struct dm_pci_ops pcie_xilinx_ops = {
	.read_config	= pcie_xilinx_read_config,
	.write_config	= pcie_xilinx_write_config,
};

static const struct udevice_id pcie_xilinx_ids[] = {
	{ .compatible = "xlnx,axi-pcie-host-1.00.a" },
	{ }
};

U_BOOT_DRIVER(pcie_xilinx) = {
	.name			= "pcie_xilinx",
	.id			= UCLASS_PCI,
	.of_match		= pcie_xilinx_ids,
	.ops			= &pcie_xilinx_ops,
	.of_to_plat	= pcie_xilinx_of_to_plat,
	.priv_auto	= sizeof(struct xilinx_pcie),
};
