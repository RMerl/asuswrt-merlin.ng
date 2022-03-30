/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _QEMU_DEVICE_H_
#define _QEMU_DEVICE_H_

#include <pci.h>

#define QEMU_I440FX	PCI_BDF(0, 0, 0)
#define PIIX_ISA	PCI_BDF(0, 1, 0)
#define PIIX_IDE	PCI_BDF(0, 1, 1)
#define PIIX_USB	PCI_BDF(0, 1, 2)
#define PIIX_PM	PCI_BDF(0, 1, 3)
#define ICH9_PM	PCI_BDF(0, 0x1f, 0)
#define I440FX_VGA	PCI_BDF(0, 2, 0)

#define QEMU_Q35	PCI_BDF(0, 0, 0)
#define Q35_VGA		PCI_BDF(0, 1, 0)

#endif /* _QEMU_DEVICE_H_ */
