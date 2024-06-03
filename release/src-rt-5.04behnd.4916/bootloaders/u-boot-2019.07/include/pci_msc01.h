/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@mips.com>
 */

#ifndef __PCI_MSC01_H__
#define __PCI_MSC01_H__

extern void msc01_pci_init(void *base, unsigned long sys_bus,
			   unsigned long sys_phys, unsigned long sys_size,
			   unsigned long mem_bus, unsigned long mem_phys,
			   unsigned long mem_size, unsigned long io_bus,
			   unsigned long io_phys, unsigned long io_size);

#endif /* __PCI_MSC01_H__ */
