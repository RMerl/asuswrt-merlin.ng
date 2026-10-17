/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 */

#ifndef _PCI_GT64120_H
#define _PCI_GT64120_H

void gt64120_pci_init(void *regs, unsigned long sys_bus, unsigned long sys_phys,
		     unsigned long sys_size, unsigned long mem_bus,
		     unsigned long mem_phys, unsigned long mem_size,
		     unsigned long io_bus, unsigned long io_phys,
		     unsigned long io_size);


#endif /* _PCI_GT64120_H */
