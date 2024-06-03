// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@mips.com>
 */

#include <common.h>
#include <msc01.h>
#include <pci.h>
#include <pci_msc01.h>
#include <asm/io.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

struct msc01_pci_controller {
	struct pci_controller hose;
	void *base;
};

static inline struct msc01_pci_controller *
hose_to_msc01(struct pci_controller *hose)
{
	return container_of(hose, struct msc01_pci_controller, hose);
}

static int msc01_config_access(struct msc01_pci_controller *msc01,
			       unsigned char access_type, pci_dev_t bdf,
			       int where, u32 *data)
{
	const u32 aborts = MSC01_PCI_INTSTAT_MA_MSK | MSC01_PCI_INTSTAT_TA_MSK;
	void *intstat = msc01->base + MSC01_PCI_INTSTAT_OFS;
	void *cfgdata = msc01->base + MSC01_PCI_CFGDATA_OFS;
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int devfn = PCI_DEV(bdf) << 3 | PCI_FUNC(bdf);

	/* clear abort status */
	__raw_writel(aborts, intstat);

	/* setup address */
	__raw_writel((bus << MSC01_PCI_CFGADDR_BNUM_SHF) |
		     (dev << MSC01_PCI_CFGADDR_DNUM_SHF) |
		     (devfn << MSC01_PCI_CFGADDR_FNUM_SHF) |
		     ((where / 4) << MSC01_PCI_CFGADDR_RNUM_SHF),
		     msc01->base + MSC01_PCI_CFGADDR_OFS);

	/* perform access */
	if (access_type == PCI_ACCESS_WRITE)
		__raw_writel(*data, cfgdata);
	else
		*data = __raw_readl(cfgdata);

	/* check for aborts */
	if (__raw_readl(intstat) & aborts) {
		/* clear abort status */
		__raw_writel(aborts, intstat);
		return -1;
	}

	return 0;
}

static int msc01_read_config_dword(struct pci_controller *hose, pci_dev_t dev,
				   int where, u32 *value)
{
	struct msc01_pci_controller *msc01 = hose_to_msc01(hose);

	*value = 0xffffffff;
	return msc01_config_access(msc01, PCI_ACCESS_READ, dev, where, value);
}

static int msc01_write_config_dword(struct pci_controller *hose, pci_dev_t dev,
				    int where, u32 value)
{
	struct msc01_pci_controller *gt = hose_to_msc01(hose);
	u32 data = value;

	return msc01_config_access(gt, PCI_ACCESS_WRITE, dev, where, &data);
}

void msc01_pci_init(void *base, unsigned long sys_bus, unsigned long sys_phys,
		    unsigned long sys_size, unsigned long mem_bus,
		    unsigned long mem_phys, unsigned long mem_size,
		    unsigned long io_bus, unsigned long io_phys,
		    unsigned long io_size)
{
	static struct msc01_pci_controller global_msc01;
	struct msc01_pci_controller *msc01;
	struct pci_controller *hose;

	msc01 = &global_msc01;
	msc01->base = base;

	hose = &msc01->hose;

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
		    msc01_read_config_dword,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    msc01_write_config_dword);

	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
}
