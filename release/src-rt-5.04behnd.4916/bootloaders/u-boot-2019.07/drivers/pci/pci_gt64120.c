// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Based on the Linux implementation.
 *   Copyright (C) 1999, 2000, 2004  MIPS Technologies, Inc.
 *   Authors: Carsten Langgaard <carstenl@mips.com>
 *            Maciej W. Rozycki <macro@mips.com>
 */

#include <common.h>
#include <gt64120.h>
#include <pci.h>
#include <pci_gt64120.h>

#include <asm/io.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

struct gt64120_regs {
	u8	unused_000[0xc18];
	u32	intrcause;
	u8	unused_c1c[0x0dc];
	u32	pci0_cfgaddr;
	u32	pci0_cfgdata;
};

struct gt64120_pci_controller {
	struct pci_controller hose;
	struct gt64120_regs *regs;
};

static inline struct gt64120_pci_controller *
hose_to_gt64120(struct pci_controller *hose)
{
	return container_of(hose, struct gt64120_pci_controller, hose);
}

#define GT_INTRCAUSE_ABORT_BITS	\
		(GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT)

static int gt_config_access(struct gt64120_pci_controller *gt,
			    unsigned char access_type, pci_dev_t bdf,
			    int where, u32 *data)
{
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int devfn = PCI_DEV(bdf) << 3 | PCI_FUNC(bdf);
	u32 intr;
	u32 addr;
	u32 val;

	if (bus == 0 && dev >= 31) {
		/* Because of a bug in the galileo (for slot 31). */
		return -1;
	}

	if (access_type == PCI_ACCESS_WRITE)
		debug("PCI WR %02x:%02x.%x reg:%02d data:%08x\n",
		      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), where, *data);

	/* Clear cause register bits */
	writel(~GT_INTRCAUSE_ABORT_BITS, &gt->regs->intrcause);

	addr = GT_PCI0_CFGADDR_CONFIGEN_BIT;
	addr |=	bus << GT_PCI0_CFGADDR_BUSNUM_SHF;
	addr |=	devfn << GT_PCI0_CFGADDR_FUNCTNUM_SHF;
	addr |= (where / 4) << GT_PCI0_CFGADDR_REGNUM_SHF;

	/* Setup address */
	writel(addr, &gt->regs->pci0_cfgaddr);

	if (access_type == PCI_ACCESS_WRITE) {
		if (bus == 0 && dev == 0) {
			/*
			 * The Galileo system controller is acting
			 * differently than other devices.
			 */
			val = *data;
		} else {
			val = cpu_to_le32(*data);
		}

		writel(val, &gt->regs->pci0_cfgdata);
	} else {
		val = readl(&gt->regs->pci0_cfgdata);

		if (bus == 0 && dev == 0) {
			/*
			 * The Galileo system controller is acting
			 * differently than other devices.
			 */
			*data = val;
		} else {
			*data = le32_to_cpu(val);
		}
	}

	/* Check for master or target abort */
	intr = readl(&gt->regs->intrcause);
	if (intr & GT_INTRCAUSE_ABORT_BITS) {
		/* Error occurred, clear abort bits */
		writel(~GT_INTRCAUSE_ABORT_BITS, &gt->regs->intrcause);
		return -1;
	}

	if (access_type == PCI_ACCESS_READ)
		debug("PCI RD %02x:%02x.%x reg:%02d data:%08x\n",
		      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), where, *data);

	return 0;
}

static int gt_read_config_dword(struct pci_controller *hose, pci_dev_t dev,
				int where, u32 *value)
{
	struct gt64120_pci_controller *gt = hose_to_gt64120(hose);

	*value = 0xffffffff;
	return gt_config_access(gt, PCI_ACCESS_READ, dev, where, value);
}

static int gt_write_config_dword(struct pci_controller *hose, pci_dev_t dev,
				 int where, u32 value)
{
	struct gt64120_pci_controller *gt = hose_to_gt64120(hose);
	u32 data = value;

	return gt_config_access(gt, PCI_ACCESS_WRITE, dev, where, &data);
}

void gt64120_pci_init(void *regs, unsigned long sys_bus, unsigned long sys_phys,
		     unsigned long sys_size, unsigned long mem_bus,
		     unsigned long mem_phys, unsigned long mem_size,
		     unsigned long io_bus, unsigned long io_phys,
		     unsigned long io_size)
{
	static struct gt64120_pci_controller global_gt;
	struct gt64120_pci_controller *gt;
	struct pci_controller *hose;

	gt = &global_gt;
	gt->regs = regs;

	hose = &gt->hose;

	hose->first_busno = 0;
	hose->last_busno = 0;

	/* System memory space */
	pci_set_region(&hose->regions[0], sys_bus, sys_phys, sys_size,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(&hose->regions[1], mem_bus, mem_phys, mem_size,
		       PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region(&hose->regions[2], io_bus, io_phys, io_size,
		       PCI_REGION_IO);

	hose->region_count = 3;

	pci_set_ops(hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    gt_read_config_dword,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    gt_write_config_dword);

	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
}
