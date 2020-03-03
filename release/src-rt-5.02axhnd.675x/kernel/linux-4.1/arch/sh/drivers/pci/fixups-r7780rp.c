/*
 * arch/sh/drivers/pci/fixups-r7780rp.c
 *
 * Highlander R7780RP-1 PCI fixups
 *
 * Copyright (C) 2003  Lineo uSolutions, Inc.
 * Copyright (C) 2004 - 2006  Paul Mundt
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/sh_intc.h>
#include "pci-sh4.h"

int __init pcibios_map_platform_irq(const struct pci_dev *pdev, u8 slot, u8 pin)
{
	return evt2irq(0xa20) + slot;
}
