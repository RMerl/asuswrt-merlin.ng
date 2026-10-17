// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Copyright (c) 2021 Rockchip, Inc.
 *
 * Copyright (C) 2018 Texas Instruments, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "pcie_dw_common.h"

int pcie_dw_get_link_speed(struct pcie_dw *pci)
{
	return (readl(pci->dbi_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_SPEED_MASK) >> PCIE_LINK_STATUS_SPEED_OFF;
}

int pcie_dw_get_link_width(struct pcie_dw *pci)
{
	return (readl(pci->dbi_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_WIDTH_MASK) >> PCIE_LINK_STATUS_WIDTH_OFF;
}

static void dw_pcie_writel_ob_unroll(struct pcie_dw *pci, u32 index, u32 reg,
				     u32 val)
{
	u32 offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
	void __iomem *base = pci->atu_base;

	writel(val, base + offset + reg);
}

static u32 dw_pcie_readl_ob_unroll(struct pcie_dw *pci, u32 index, u32 reg)
{
	u32 offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
	void __iomem *base = pci->atu_base;

	return readl(base + offset + reg);
}

/**
 * pcie_dw_prog_outbound_atu_unroll() - Configure ATU for outbound accesses
 *
 * @pcie: Pointer to the PCI controller state
 * @index: ATU region index
 * @type: ATU accsess type
 * @cpu_addr: the physical address for the translation entry
 * @pci_addr: the pcie bus address for the translation entry
 * @size: the size of the translation entry
 *
 * Return: 0 is successful and -1 is failure
 */
int pcie_dw_prog_outbound_atu_unroll(struct pcie_dw *pci, int index,
				     int type, u64 cpu_addr,
					     u64 pci_addr, u32 size)
{
	u32 retries, val;

	dev_dbg(pci->dev, "ATU programmed with: index: %d, type: %d, cpu addr: %8llx, pci addr: %8llx, size: %8x\n",
		index, type, cpu_addr, pci_addr, size);

	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LOWER_BASE,
				 lower_32_bits(cpu_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_UPPER_BASE,
				 upper_32_bits(cpu_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LIMIT,
				 lower_32_bits(cpu_addr + size - 1));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_UPPER_LIMIT,
				 upper_32_bits(cpu_addr + size - 1));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LOWER_TARGET,
				 lower_32_bits(pci_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_UPPER_TARGET,
				 upper_32_bits(pci_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_REGION_CTRL1,
				 type);
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_REGION_CTRL2,
				 PCIE_ATU_ENABLE);

	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = dw_pcie_readl_ob_unroll(pci, index,
					      PCIE_ATU_UNR_REGION_CTRL2);
		if (val & PCIE_ATU_ENABLE)
			return 0;

		udelay(LINK_WAIT_IATU);
	}
	dev_err(pci->dev, "outbound iATU is not being enabled\n");

	return -1;
}

/**
 * set_cfg_address() - Configure the PCIe controller config space access
 *
 * @pcie: Pointer to the PCI controller state
 * @d: PCI device to access
 * @where: Offset in the configuration space
 *
 * Configures the PCIe controller to access the configuration space of
 * a specific PCIe device and returns the address to use for this
 * access.
 *
 * Return: Address that can be used to access the configation space
 *         of the requested device / offset
 */
static uintptr_t set_cfg_address(struct pcie_dw *pcie,
				 pci_dev_t d, uint where)
{
	int bus = PCI_BUS(d) - pcie->first_busno;
	uintptr_t va_address;
	u32 atu_type;
	int ret;

	/* Use dbi_base for own configuration read and write */
	if (!bus) {
		va_address = (uintptr_t)pcie->dbi_base;
		goto out;
	}

	if (bus == 1)
		/*
		 * For local bus whose primary bus number is root bridge,
		 * change TLP Type field to 4.
		 */
		atu_type = PCIE_ATU_TYPE_CFG0;
	else
		/* Otherwise, change TLP Type field to 5. */
		atu_type = PCIE_ATU_TYPE_CFG1;

	/*
	 * Not accessing root port configuration space?
	 * Region #1 is used for Outbound CFG space access.
	 * Direction = Outbound
	 * Region Index = 1
	 */
	d = PCI_MASK_BUS(d);
	d = PCI_ADD_BUS(bus, d);
	ret = pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
					       atu_type, (u64)pcie->cfg_base,
						d << 8, pcie->cfg_size);
	if (ret)
		return (uintptr_t)ret;

	va_address = (uintptr_t)pcie->cfg_base;

out:
	va_address += where & ~0x3;

	return va_address;
}

/**
 * pcie_dw_addr_valid() - Check for valid bus address
 *
 * @d: The PCI device to access
 * @first_busno: Bus number of the PCIe controller root complex
 *
 * Return 1 (true) if the PCI device can be accessed by this controller.
 *
 * Return: 1 on valid, 0 on invalid
 */
static int pcie_dw_addr_valid(pci_dev_t d, int first_busno)
{
	if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
		return 0;
	if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
		return 0;

	return 1;
}

/**
 * pcie_dw_read_config() - Read from configuration space
 *
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
 * Return: 0 on success
 */
int pcie_dw_read_config(const struct udevice *bus, pci_dev_t bdf,
			uint offset, ulong *valuep,
			enum pci_size_t size)
{
	struct pcie_dw *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong value;

	dev_dbg(pcie->dev, "PCIE CFG read: bdf=%2x:%2x:%2x ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	value = readl((void __iomem *)va_address);

	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);
	*valuep = pci_conv_32_to_size(value, offset, size);

	return pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
						 PCIE_ATU_TYPE_IO, pcie->io.phys_start,
						 pcie->io.bus_start, pcie->io.size);
}

/**
 * pcie_dw_write_config() - Write to configuration space
 *
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
 * Return: 0 on success
 */
int pcie_dw_write_config(struct udevice *bus, pci_dev_t bdf,
			 uint offset, ulong value,
			 enum pci_size_t size)
{
	struct pcie_dw *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong old;

	dev_dbg(pcie->dev, "PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	dev_dbg(pcie->dev, "(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	old = readl((void __iomem *)va_address);
	value = pci_conv_size_to_32(old, value, offset, size);
	writel(value, (void __iomem *)va_address);

	return pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
						 PCIE_ATU_TYPE_IO, pcie->io.phys_start,
						 pcie->io.bus_start, pcie->io.size);
}

/**
 * pcie_dw_setup_host() - Setup the PCIe controller for RC opertaion
 *
 * @pcie: Pointer to the PCI controller state
 *
 * Configure the host BARs of the PCIe controller root port so that
 * PCI(e) devices may access the system memory.
 */
void pcie_dw_setup_host(struct pcie_dw *pci)
{
	struct udevice *ctlr = pci_get_controller(pci->dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	u32 ret;

	if (!pci->atu_base)
		pci->atu_base = pci->dbi_base + DEFAULT_DBI_ATU_OFFSET;

	/* setup RC BARs */
	writel(PCI_BASE_ADDRESS_MEM_TYPE_64,
	       pci->dbi_base + PCI_BASE_ADDRESS_0);
	writel(0x0, pci->dbi_base + PCI_BASE_ADDRESS_1);

	/* setup interrupt pins */
	clrsetbits_le32(pci->dbi_base + PCI_INTERRUPT_LINE,
			0xff00, 0x100);

	/* setup bus numbers */
	clrsetbits_le32(pci->dbi_base + PCI_PRIMARY_BUS,
			0xffffff, 0x00ff0100);

	/* setup command register */
	clrsetbits_le32(pci->dbi_base + PCI_PRIMARY_BUS,
			0xffff,
			PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
			PCI_COMMAND_MASTER | PCI_COMMAND_SERR);

	/* Enable write permission for the DBI read-only register */
	dw_pcie_dbi_write_enable(pci, true);
	/* program correct class for RC */
	writew(PCI_CLASS_BRIDGE_PCI, pci->dbi_base + PCI_CLASS_DEVICE);
	/* Better disable write permission right after the update */
	dw_pcie_dbi_write_enable(pci, false);

	setbits_le32(pci->dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL,
		     PORT_LOGIC_SPEED_CHANGE);

	for (ret = 0; ret < hose->region_count; ret++) {
		if (hose->regions[ret].flags == PCI_REGION_IO) {
			pci->io.phys_start = hose->regions[ret].phys_start; /* IO base */
			pci->io.bus_start = hose->regions[ret].bus_start;  /* IO_bus_addr */
			pci->io.size = hose->regions[ret].size;      /* IO size */
		} else if (hose->regions[ret].flags == PCI_REGION_MEM) {
			pci->mem.phys_start = hose->regions[ret].phys_start; /* MEM base */
			pci->mem.bus_start = hose->regions[ret].bus_start;  /* MEM_bus_addr */
			pci->mem.size = hose->regions[ret].size;	    /* MEM size */
		} else if (hose->regions[ret].flags == PCI_REGION_PREFETCH) {
			pci->prefetch.phys_start = hose->regions[ret].phys_start; /* PREFETCH base */
			pci->prefetch.bus_start = hose->regions[ret].bus_start;  /* PREFETCH_bus_addr */
			pci->prefetch.size = hose->regions[ret].size;	    /* PREFETCH size */
		} else if (hose->regions[ret].flags == PCI_REGION_SYS_MEMORY) {
			if (!pci->cfg_base) {
				pci->cfg_base = (void *)(pci->io.phys_start - pci->io.size);
				pci->cfg_size = pci->io.size;
			}
		} else {
			dev_err(pci->dev, "invalid flags type!\n");
		}
	}

	dev_dbg(pci->dev, "Config space: [0x%llx - 0x%llx, size 0x%llx]\n",
		(u64)pci->cfg_base, (u64)pci->cfg_base + pci->cfg_size,
		(u64)pci->cfg_size);

	dev_dbg(pci->dev, "IO space: [0x%llx - 0x%llx, size 0x%llx]\n",
		(u64)pci->io.phys_start, (u64)pci->io.phys_start + pci->io.size,
		(u64)pci->io.size);

	dev_dbg(pci->dev, "IO bus:   [0x%llx - 0x%llx, size 0x%llx]\n",
		(u64)pci->io.bus_start,	(u64)pci->io.bus_start + pci->io.size,
		(u64)pci->io.size);

	dev_dbg(pci->dev, "MEM space: [0x%llx - 0x%llx, size 0x%llx]\n",
		(u64)pci->mem.phys_start,
		(u64)pci->mem.phys_start + pci->mem.size,
		(u64)pci->mem.size);

	dev_dbg(pci->dev, "MEM bus:   [0x%llx - 0x%llx, size 0x%llx]\n",
		(u64)pci->mem.bus_start,
		(u64)pci->mem.bus_start + pci->mem.size,
		(u64)pci->mem.size);

	if (pci->prefetch.size) {
		dev_dbg(pci->dev, "PREFETCH space: [0x%llx - 0x%llx, size 0x%llx]\n",
			(u64)pci->prefetch.phys_start,
			(u64)pci->prefetch.phys_start + pci->prefetch.size,
			(u64)pci->prefetch.size);

		dev_dbg(pci->dev, "PREFETCH bus:   [0x%llx - 0x%llx, size 0x%llx]\n",
			(u64)pci->prefetch.bus_start,
			(u64)pci->prefetch.bus_start + pci->prefetch.size,
			(u64)pci->prefetch.size);
	}
}
