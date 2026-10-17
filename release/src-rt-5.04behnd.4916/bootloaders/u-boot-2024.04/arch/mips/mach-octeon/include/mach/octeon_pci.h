/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_PCI_H__
#define __OCTEON_PCI_H__

/**
 * EEPROM entry struct
 */
union octeon_pcie_eeprom {
	u64 u64;
	struct octeon_data_s {
		/**
		 * 0x9DA1 valid entry, 0x6A5D end of table, 0xffff invalid
		 * access
		 */
		u64 preamble : 16;
u64: 1; /** Reserved */
		/** Physical function number accessed by the write operation. */
		u64 pf : 2;
		/**
		 * Specifies bit<31> of the address written by hardware.
		 * 1 = configuration mask register, 0 = configuration register
		 */
		u64 cs2 : 1;
		/**
		 * Specifies bits<11:0> of the address written by hardware.
		 * Bits<30:12> of this address are all 0s.
		 */
		u64 address : 12;
		u64 data : 32;
	} s;
};

void pci_dev_post_init(void);

int octeon_pci_io_readb(unsigned int reg);
void octeon_pci_io_writeb(int value, unsigned int reg);
int octeon_pci_io_readw(unsigned int reg);
void octeon_pci_io_writew(int value, unsigned int reg);
int octeon_pci_io_readl(unsigned int reg);
void octeon_pci_io_writel(int value, unsigned int reg);
int octeon_pci_mem1_readb(unsigned int reg);
void octeon_pci_mem1_writeb(int value, unsigned int reg);
int octeon_pci_mem1_readw(unsigned int reg);
void octeon_pci_mem1_writew(int value, unsigned int reg);
int octeon_pci_mem1_readl(unsigned int reg);
void octeon_pci_mem1_writel(int value, unsigned int reg);

/* In the TLB mapped case, these also work with virtual addresses,
** and do the required virt<->phys translations as well. */
u32 octeon_pci_phys_to_bus(u32 phys);
u32 octeon_pci_bus_to_phys(u32 bus);

/**
 * Searches PCIe EEPROM for override data specified by address and pf.
 *
 * @param	address - PCIe config space address
 * @param	pf	- PCIe config space pf num
 * @param[out]	id	- override device and vendor ID
 *
 * Return:	0 if override found, 1 if not found.
 */
int octeon_find_pcie_id_override(unsigned int address, unsigned int pf, u32 *id);

#endif /* __OCTEON_PCI_H__ */
