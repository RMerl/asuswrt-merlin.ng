// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI Endpoint uclass
 *
 * Based on Linux PCI-EP driver written by
 * Kishon Vijay Abraham I <kishon@ti.com>
 *
 * Copyright (c) 2019
 * Written by Ramon Fried <ramon.fried@gmail.com>
 */

#define LOG_CATEGORY UCLASS_PCI_EP

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <linux/log2.h>
#include <pci_ep.h>

DECLARE_GLOBAL_DATA_PTR;

int pci_ep_write_header(struct udevice *dev, uint fn, struct pci_ep_header *hdr)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->write_header)
		return -ENOSYS;

	return ops->write_header(dev, fn, hdr);
}

int pci_ep_read_header(struct udevice *dev, uint fn, struct pci_ep_header *hdr)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->read_header)
		return -ENOSYS;

	return ops->read_header(dev, fn, hdr);
}

int pci_ep_set_bar(struct udevice *dev, uint func_no, struct pci_bar *ep_bar)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);
	int flags = ep_bar->flags;

	/* Some basic bar validity checks */
	if (ep_bar->barno > BAR_5 || ep_bar->barno < BAR_0)
		return -EINVAL;

	if ((ep_bar->barno == BAR_5 &&
	     (flags & PCI_BASE_ADDRESS_MEM_TYPE_64)) ||
	    ((flags & PCI_BASE_ADDRESS_SPACE_IO) &&
	     (flags & PCI_BASE_ADDRESS_IO_MASK)) ||
	    (upper_32_bits(ep_bar->size) &&
	     !(flags & PCI_BASE_ADDRESS_MEM_TYPE_64)))
		return -EINVAL;

	if (!ops->set_bar)
		return -ENOSYS;

	return ops->set_bar(dev, func_no, ep_bar);
}

int pci_ep_read_bar(struct udevice *dev, uint func_no, struct pci_bar *ep_bar,
		    enum pci_barno barno)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	/* Some basic bar validity checks */
	if (barno > BAR_5 || barno < BAR_0)
		return -EINVAL;

	if (!ops->read_bar)
		return -ENOSYS;

	return ops->read_bar(dev, func_no, ep_bar, barno);
}

int pci_ep_clear_bar(struct udevice *dev, uint func_num, enum pci_barno bar)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->clear_bar)
		return -ENOSYS;

	return ops->clear_bar(dev, func_num, bar);
}

int pci_ep_map_addr(struct udevice *dev, uint func_no, phys_addr_t addr,
		    u64 pci_addr, size_t size)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->map_addr)
		return -ENOSYS;

	return ops->map_addr(dev, func_no, addr, pci_addr, size);
}

int pci_ep_unmap_addr(struct udevice *dev, uint func_no, phys_addr_t addr)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->unmap_addr)
		return -ENOSYS;

	return ops->unmap_addr(dev, func_no, addr);
}

int pci_ep_set_msi(struct udevice *dev, uint func_no, uint interrupts)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);
	uint encode_int;

	if (interrupts > 32)
		return -EINVAL;

	if (!ops->set_msi)
		return -ENOSYS;

	/* MSI spec permits allocation of
	 * only 1, 2, 4, 8, 16, 32 interrupts
	 */
	encode_int = order_base_2(interrupts);

	return ops->set_msi(dev, func_no, encode_int);
}

int pci_ep_get_msi(struct udevice *dev, uint func_no)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);
	int interrupt;

	if (!ops->get_msi)
		return -ENOSYS;

	interrupt = ops->get_msi(dev, func_no);

	if (interrupt < 0)
		return 0;

	/* Translate back from order base 2*/
	interrupt = 1 << interrupt;

	return interrupt;
}

int pci_ep_set_msix(struct udevice *dev, uint func_no, uint interrupts)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (interrupts < 1 || interrupts > 2048)
		return -EINVAL;

	if (!ops->set_msix)
		return -ENOSYS;

	return ops->set_msix(dev, func_no, interrupts - 1);
}

int pci_ep_get_msix(struct udevice *dev, uint func_no)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);
	int interrupt;

	if (!ops->get_msix)
		return -ENOSYS;

	interrupt = ops->get_msix(dev, func_no);

	if (interrupt < 0)
		return 0;

	return interrupt + 1;
}

int pci_ep_raise_irq(struct udevice *dev, uint func_no,
		     enum pci_ep_irq_type type, uint interrupt_num)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->raise_irq)
		return -ENOSYS;

	return ops->raise_irq(dev, func_no, type, interrupt_num);
}

int pci_ep_start(struct udevice *dev)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->start)
		return -ENOSYS;

	return ops->start(dev);
}

int pci_ep_stop(struct udevice *dev)
{
	struct pci_ep_ops *ops = pci_ep_get_ops(dev);

	if (!ops->stop)
		return -ENOSYS;

	return ops->stop(dev);
}

UCLASS_DRIVER(pci_ep) = {
	.id		= UCLASS_PCI_EP,
	.name		= "pci_ep",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

int pci_ep_init(void)
{
	struct udevice *dev;

	for (uclass_first_device_check(UCLASS_PCI_EP, &dev);
	     dev;
	     uclass_next_device_check(&dev)) {
		;
	}

	return 0;
}
