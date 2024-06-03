// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

/*
 * IO space access commands.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <asm/io.h>

int pci_map_physmem(phys_addr_t paddr, unsigned long *lenp,
		    struct udevice **devp, void **ptrp)
{
	struct udevice *dev;
	int ret;

	*ptrp = 0;
	for (uclass_first_device(UCLASS_PCI_EMUL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		struct dm_pci_emul_ops *ops = pci_get_emul_ops(dev);

		if (!ops || !ops->map_physmem)
			continue;
		ret = (ops->map_physmem)(dev, paddr, lenp, ptrp);
		if (ret)
			continue;
		*devp = dev;
		return 0;
	}

	debug("%s: failed: addr=%pap\n", __func__, &paddr);
	return -ENOSYS;
}

int pci_unmap_physmem(const void *vaddr, unsigned long len,
		      struct udevice *dev)
{
	struct dm_pci_emul_ops *ops = pci_get_emul_ops(dev);

	if (!ops || !ops->unmap_physmem)
		return -ENOSYS;
	return (ops->unmap_physmem)(dev, vaddr, len);
}

static int pci_io_read(unsigned int addr, ulong *valuep, pci_size_t size)
{
	struct udevice *dev;
	int ret;

	*valuep = pci_get_ff(size);
	for (uclass_first_device(UCLASS_PCI_EMUL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		struct dm_pci_emul_ops *ops = pci_get_emul_ops(dev);

		if (ops && ops->read_io) {
			ret = (ops->read_io)(dev, addr, valuep, size);
			if (!ret)
				return 0;
		}
	}

	debug("%s: failed: addr=%x\n", __func__, addr);
	return -ENOSYS;
}

static int pci_io_write(unsigned int addr, ulong value, pci_size_t size)
{
	struct udevice *dev;
	int ret;

	for (uclass_first_device(UCLASS_PCI_EMUL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		struct dm_pci_emul_ops *ops = pci_get_emul_ops(dev);

		if (ops && ops->write_io) {
			ret = (ops->write_io)(dev, addr, value, size);
			if (!ret)
				return 0;
		}
	}

	debug("%s: failed: addr=%x, value=%lx\n", __func__, addr, value);
	return -ENOSYS;
}

int inl(unsigned int addr)
{
	unsigned long value;
	int ret;

	ret = pci_io_read(addr, &value, PCI_SIZE_32);

	return ret ? 0 : value;
}

int inw(unsigned int addr)
{
	unsigned long value;
	int ret;

	ret = pci_io_read(addr, &value, PCI_SIZE_16);

	return ret ? 0 : value;
}

int inb(unsigned int addr)
{
	unsigned long value;
	int ret;

	ret = pci_io_read(addr, &value, PCI_SIZE_8);

	return ret ? 0 : value;
}

void outl(unsigned int value, unsigned int addr)
{
	pci_io_write(addr, value, PCI_SIZE_32);
}

void outw(unsigned int value, unsigned int addr)
{
	pci_io_write(addr, value, PCI_SIZE_16);
}

void outb(unsigned int value, unsigned int addr)
{
	pci_io_write(addr, value, PCI_SIZE_8);
}
