/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Adapted from Linux kernel driver
 * Copyright (C) 2017 Texas Instruments
 * Author: Kishon Vijay Abraham I <kishon@ti.com>
 *
 * (C) Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#ifndef _PCI_EP_H
#define _PCI_EP_H

#include <pci.h>

/**
 * enum pci_interrupt_pin - PCI INTx interrupt values
 * @PCI_INTERRUPT_UNKNOWN: Unknown or unassigned interrupt
 * @PCI_INTERRUPT_INTA: PCI INTA pin
 * @PCI_INTERRUPT_INTB: PCI INTB pin
 * @PCI_INTERRUPT_INTC: PCI INTC pin
 * @PCI_INTERRUPT_INTD: PCI INTD pin
 *
 * Corresponds to values for legacy PCI INTx interrupts, as can be found in the
 * PCI_INTERRUPT_PIN register.
 */
enum pci_interrupt_pin {
	PCI_INTERRUPT_UNKNOWN,
	PCI_INTERRUPT_INTA,
	PCI_INTERRUPT_INTB,
	PCI_INTERRUPT_INTC,
	PCI_INTERRUPT_INTD,
};

enum pci_barno {
	BAR_0,
	BAR_1,
	BAR_2,
	BAR_3,
	BAR_4,
	BAR_5,
};

enum pci_ep_irq_type {
	PCI_EP_IRQ_UNKNOWN,
	PCI_EP_IRQ_LEGACY,
	PCI_EP_IRQ_MSI,
	PCI_EP_IRQ_MSIX,
};

/**
 * struct pci_bar - represents the BAR (Base Address Register) of EP device
 * @phys_addr: physical address that should be mapped to the BAR
 * @size: the size of the address space present in BAR
 * pci_barno: number of pci BAR to set (0..5)
 * @flags: BAR access flags
 */
struct pci_bar {
	dma_addr_t	phys_addr;
	size_t		size;
	enum pci_barno	barno;
	int		flags;
};

/**
 * struct pci_ep_header - represents standard configuration header
 * @vendorid: identifies device manufacturer
 * @deviceid: identifies a particular device
 * @revid: specifies a device-specific revision identifier
 * @progif_code: identifies a specific register-level programming interface
 * @subclass_code: identifies more specifically the function of the device
 * @baseclass_code: broadly classifies the type of function the device performs
 * @cache_line_size: specifies the system cacheline size in units of DWORDs
 * @subsys_vendor_id: vendor of the add-in card or subsystem
 * @subsys_id: id specific to vendor
 * @interrupt_pin: interrupt pin the device (or device function) uses
 */
struct pci_ep_header {
	u16	vendorid;
	u16	deviceid;
	u8	revid;
	u8	progif_code;
	u8	subclass_code;
	u8	baseclass_code;
	u8	cache_line_size;
	u16	subsys_vendor_id;
	u16	subsys_id;
	enum pci_interrupt_pin interrupt_pin;
};

/* PCI endpoint operations */
struct pci_ep_ops {
	/**
	 * write_header() - Write a PCI configuration space header
	 *
	 * @dev:	device to write to
	 * @func_num:	EP function to fill
	 * @hdr:	header to write
	 * @return 0 if OK, -ve on error
	 */
	int	(*write_header)(struct udevice *dev, uint func_num,
				struct pci_ep_header *hdr);
	/**
	 * read_header() - Read a PCI configuration space header
	 *
	 * @dev:	device to write to
	 * @func_num:	EP function to fill
	 * @hdr:	header to read to
	 * @return 0 if OK, -ve on error
	 */
	int	(*read_header)(struct udevice *dev, uint func_num,
			       struct pci_ep_header *hdr);
	/**
	 * set_bar() - Set BAR (Base Address Register) properties
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @bar:	bar data
	 * @return 0 if OK, -ve on error
	 */
	int	(*set_bar)(struct udevice *dev, uint func_num,
			   struct pci_bar *bar);
	/**
	 * read_bar() - Read BAR (Base Address Register) properties
	 *
	 * @dev:	device to read
	 * @func_num:	EP function to read
	 * @bar:	struct to copy data to
	 * @barno:	bar number to read
	 * @return 0 if OK, -ve on error
	 */
	int	(*read_bar)(struct udevice *dev, uint func_num,
			    struct pci_bar *bar, enum pci_barno barno);
	/**
	 * clear_bar() - clear BAR (Base Address Register)
	 *
	 * @dev:	device to clear
	 * @func_num:	EP function to clear
	 * @bar:	bar number
	 * @return 0 if OK, -ve on error
	 */
	int	(*clear_bar)(struct udevice *dev, uint func_num,
			     enum pci_barno bar);
	/**
	 * map_addr() - map CPU address to PCI address
	 *
	 * outband region is used in order to generate PCI read/write
	 * transaction from local memory/write.
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @addr:	local physical address base
	 * @pci_addr:	pci address to translate to
	 * @size:	region size
	 * @return 0 if OK, -ve on error
	 */
	int	(*map_addr)(struct udevice *dev, uint func_num,
			    phys_addr_t addr, u64 pci_addr, size_t size);
	/**
	 * unmap_addr() - unmap CPU address to PCI address
	 *
	 * unmap previously mapped region.
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @addr:	local physical address base
	 * @return 0 if OK, -ve on error
	 */
	int	(*unmap_addr)(struct udevice *dev, uint func_num,
			      phys_addr_t addr);
	/**
	 * set_msi() - set msi capability property
	 *
	 * set the number of required MSI vectors the device
	 * needs for operation.
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @interrupts:	required interrupts count
	 * @return 0 if OK, -ve on error
	 */
	int	(*set_msi)(struct udevice *dev, uint func_num, uint interrupts);

	/**
	 * get_msi() - get the number of MSI interrupts allocated by the host.
	 *
	 * Read the Multiple Message Enable bitfield from
	 * Message control register.
	 *
	 * @dev:	device to use
	 * @func_num:	EP function to use
	 * @return msi count if OK, -EINVAL if msi were not enabled at host.
	 */
	int	(*get_msi)(struct udevice *dev, uint func_num);

	/**
	 * set_msix() - set msix capability property
	 *
	 * set the number of required MSIx vectors the device
	 * needs for operation.
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @interrupts:	required interrupts count
	 * @return 0 if OK, -ve on error
	 */
	int	(*set_msix)(struct udevice *dev, uint func_num,
			    uint interrupts);

	/**
	 * get_msix() - get the number of MSIx interrupts allocated by the host.
	 *
	 * Read the Multiple Message Enable bitfield from
	 * Message control register.
	 *
	 * @dev:	device to use
	 * @func_num:	EP function to use
	 * @return msi count if OK, -EINVAL if msi were not enabled at host.
	 */
	int	(*get_msix)(struct udevice *dev, uint func_num);

	/**
	 * raise_irq() - raise a legacy, MSI or MSI-X interrupt
	 *
	 * @dev:	device to set
	 * @func_num:	EP function to set
	 * @type:	type of irq to send
	 * @interrupt_num: interrupt vector to use
	 * @return 0 if OK, -ve on error
	 */
	int	(*raise_irq)(struct udevice *dev, uint func_num,
			     enum pci_ep_irq_type type, uint interrupt_num);
	/**
	 * start() - start the PCI link
	 *
	 * @dev:	device to set
	 * @return 0 if OK, -ve on error
	 */
	int	(*start)(struct udevice *dev);

	/**
	 * stop() - stop the PCI link
	 *
	 * @dev:	device to set
	 * @return 0 if OK, -ve on error
	 */
	int	(*stop)(struct udevice *dev);
};

#define pci_ep_get_ops(dev)	((struct pci_ep_ops *)(dev)->driver->ops)

/**
 * pci_ep_write_header() - Write a PCI configuration space header
 *
 * @dev:	device to write to
 * @func_num:	EP function to fill
 * @hdr:	header to write
 * Return: 0 if OK, -ve on error
 */
int pci_ep_write_header(struct udevice *dev, uint func_num,
			struct pci_ep_header *hdr);

/**
 * dm_pci_ep_read_header() - Read a PCI configuration space header
 *
 * @dev:	device to write to
 * @func_num:	EP function to fill
 * @hdr:	header to read to
 * Return: 0 if OK, -ve on error
 */
int pci_ep_read_header(struct udevice *dev, uint func_num,
		       struct pci_ep_header *hdr);
/**
 * pci_ep_set_bar() - Set BAR (Base Address Register) properties
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @bar:	bar data
 * Return: 0 if OK, -ve on error
 */
int pci_ep_set_bar(struct udevice *dev, uint func_num, struct pci_bar *bar);

/**
 * pci_ep_read_bar() - Read BAR (Base Address Register) properties
 *
 * @dev:	device to read
 * @func_num:	EP function to read
 * @bar:	struct to copy data to
 * @barno:	bar number to read
 * Return: 0 if OK, -ve on error
 */
int pci_ep_read_bar(struct udevice *dev, uint func_no, struct pci_bar *ep_bar,
		    enum pci_barno barno);

/**
 * pci_ep_clear_bar() - Clear BAR (Base Address Register)
 *			mark the BAR as empty so host won't map it.
 * @dev:	device to clear
 * @func_num:	EP function to clear
 * @bar:	bar number
 * Return: 0 if OK, -ve on error
 */
int pci_ep_clear_bar(struct udevice *dev, uint func_num, enum pci_barno bar);
/**
 * pci_ep_map_addr() - map CPU address to PCI address
 *
 * outband region is used in order to generate PCI read/write
 * transaction from local memory/write.
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @addr:	local physical address base
 * @pci_addr:	pci address to translate to
 * @size:	region size
 * Return: 0 if OK, -ve on error
 */
int pci_ep_map_addr(struct udevice *dev, uint func_num, phys_addr_t addr,
		    u64 pci_addr, size_t size);
/**
 * pci_ep_unmap_addr() - unmap CPU address to PCI address
 *
 * unmap previously mapped region.
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @addr:	local physical address base
 * Return: 0 if OK, -ve on error
 */
int pci_ep_unmap_addr(struct udevice *dev, uint func_num, phys_addr_t addr);

/**
 * pci_ep_set_msi() - set msi capability property
 *
 * set the number of required MSI vectors the device
 * needs for operation.
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @interrupts:	required interrupts count
 * Return: 0 if OK, -ve on error
 */
int pci_ep_set_msi(struct udevice *dev, uint func_num, uint interrupts);

/**
 * pci_ep_get_msi() - get the number of MSI interrupts allocated by the host.
 *
 * Read the Multiple Message Enable bitfield from
 * Message control register.
 *
 * @dev:	device to use
 * @func_num:	EP function to use
 * Return: msi count if OK, -EINVAL if msi were not enabled at host.
 */
int pci_ep_get_msi(struct udevice *dev, uint func_num);

/**
 * pci_ep_set_msix() - set msi capability property
 *
 * set the number of required MSIx vectors the device
 * needs for operation.
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @interrupts:	required interrupts count
 * Return: 0 if OK, -ve on error
 */
int pci_ep_set_msix(struct udevice *dev, uint func_num, uint interrupts);

/**
 * pci_ep_get_msix() - get the number of MSIx interrupts allocated by the host.
 *
 * Read the Multiple Message Enable bitfield from
 * Message control register.
 *
 * @dev:	device to use
 * @func_num:	EP function to use
 * Return: msi count if OK, -EINVAL if msi were not enabled at host.
 */
int pci_ep_get_msix(struct udevice *dev, uint func_num);

/**
 * pci_ep_raise_irq() - raise a legacy, MSI or MSI-X interrupt
 *
 * @dev:	device to set
 * @func_num:	EP function to set
 * @type:	type of irq to send
 * @interrupt_num: interrupt vector to use
 * Return: 0 if OK, -ve on error
 */
int pci_ep_raise_irq(struct udevice *dev, uint func_num,
		     enum pci_ep_irq_type type, uint interrupt_num);
/**
 * pci_ep_start() - start the PCI link
 *
 * Enable PCI endpoint device and start link
 * process.
 *
 * @dev:	device to set
 * Return: 0 if OK, -ve on error
 */
int pci_ep_start(struct udevice *dev);

/**
 * pci_ep_stop() - stop the PCI link
 *
 * Disable PCI endpoint device and stop
 * link.
 *
 * @dev:	device to set
 * Return: 0 if OK, -ve on error
 */
int pci_ep_stop(struct udevice *dev);

#endif
