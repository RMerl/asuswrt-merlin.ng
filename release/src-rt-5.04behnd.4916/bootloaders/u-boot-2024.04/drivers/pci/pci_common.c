// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <pci.h>
#include <asm/io.h>

const char *pci_class_str(u8 class)
{
	switch (class) {
	case PCI_CLASS_NOT_DEFINED:
		return "Build before PCI Rev2.0";
		break;
	case PCI_BASE_CLASS_STORAGE:
		return "Mass storage controller";
		break;
	case PCI_BASE_CLASS_NETWORK:
		return "Network controller";
		break;
	case PCI_BASE_CLASS_DISPLAY:
		return "Display controller";
		break;
	case PCI_BASE_CLASS_MULTIMEDIA:
		return "Multimedia device";
		break;
	case PCI_BASE_CLASS_MEMORY:
		return "Memory controller";
		break;
	case PCI_BASE_CLASS_BRIDGE:
		return "Bridge device";
		break;
	case PCI_BASE_CLASS_COMMUNICATION:
		return "Simple comm. controller";
		break;
	case PCI_BASE_CLASS_SYSTEM:
		return "Base system peripheral";
		break;
	case PCI_BASE_CLASS_INPUT:
		return "Input device";
		break;
	case PCI_BASE_CLASS_DOCKING:
		return "Docking station";
		break;
	case PCI_BASE_CLASS_PROCESSOR:
		return "Processor";
		break;
	case PCI_BASE_CLASS_SERIAL:
		return "Serial bus controller";
		break;
	case PCI_BASE_CLASS_INTELLIGENT:
		return "Intelligent controller";
		break;
	case PCI_BASE_CLASS_SATELLITE:
		return "Satellite controller";
		break;
	case PCI_BASE_CLASS_CRYPT:
		return "Cryptographic device";
		break;
	case PCI_BASE_CLASS_SIGNAL_PROCESSING:
		return "DSP";
		break;
	case PCI_CLASS_OTHERS:
		return "Does not fit any class";
		break;
	default:
	return  "???";
		break;
	};
}

__weak int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	/*
	 * Check if pci device should be skipped in configuration
	 */
	if (dev == PCI_BDF(hose->first_busno, 0, 0)) {
#if defined(CONFIG_PCI_CONFIG_HOST_BRIDGE) /* don't skip host bridge */
		/*
		 * Only skip configuration if "pciconfighost" is not set
		 */
		if (env_get("pciconfighost") == NULL)
			return 1;
#else
		return 1;
#endif
	}

	return 0;
}

#if defined(CONFIG_DM_PCI_COMPAT)
/* Get a virtual address associated with a BAR region */
void *pci_map_bar(pci_dev_t pdev, int bar, int flags)
{
	pci_addr_t pci_bus_addr;
	u32 bar_response;

	/* read BAR address */
	pci_read_config_dword(pdev, bar, &bar_response);
	pci_bus_addr = (pci_addr_t)(bar_response & ~0xf);

	/*
	 * Pass "0" as the length argument to pci_bus_to_virt.  The arg
	 * isn't actualy used on any platform because u-boot assumes a static
	 * linear mapping.  In the future, this could read the BAR size
	 * and pass that as the size if needed.
	 */
	return pci_bus_to_virt(pdev, pci_bus_addr, flags, 0, MAP_NOCACHE);
}

void pci_write_bar32(struct pci_controller *hose, pci_dev_t dev, int barnum,
		     u32 addr_and_ctrl)
{
	int bar;

	bar = PCI_BASE_ADDRESS_0 + barnum * 4;
	pci_hose_write_config_dword(hose, dev, bar, addr_and_ctrl);
}

u32 pci_read_bar32(struct pci_controller *hose, pci_dev_t dev, int barnum)
{
	u32 addr;
	int bar;

	bar = PCI_BASE_ADDRESS_0 + barnum * 4;
	pci_hose_read_config_dword(hose, dev, bar, &addr);
	if (addr & PCI_BASE_ADDRESS_SPACE_IO)
		return addr & PCI_BASE_ADDRESS_IO_MASK;
	else
		return addr & PCI_BASE_ADDRESS_MEM_MASK;
}

int __pci_hose_bus_to_phys(struct pci_controller *hose,
			   pci_addr_t bus_addr,
			   unsigned long flags,
			   unsigned long skip_mask,
			   phys_addr_t *pa)
{
	struct pci_region *res;
	int i;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		if (res->flags & skip_mask)
			continue;

		if (bus_addr >= res->bus_start &&
		    (bus_addr - res->bus_start) < res->size) {
			*pa = (bus_addr - res->bus_start + res->phys_start);
			return 0;
		}
	}

	return 1;
}

phys_addr_t pci_hose_bus_to_phys(struct pci_controller *hose,
				 pci_addr_t bus_addr,
				 unsigned long flags)
{
	phys_addr_t phys_addr = 0;
	int ret;

	if (!hose) {
		puts("pci_hose_bus_to_phys: invalid hose\n");
		return phys_addr;
	}

	/*
	 * if PCI_REGION_MEM is set we do a two pass search with preference
	 * on matches that don't have PCI_REGION_SYS_MEMORY set
	 */
	if ((flags & PCI_REGION_TYPE) == PCI_REGION_MEM) {
		ret = __pci_hose_bus_to_phys(hose, bus_addr,
				flags, PCI_REGION_SYS_MEMORY, &phys_addr);
		if (!ret)
			return phys_addr;
	}

	ret = __pci_hose_bus_to_phys(hose, bus_addr, flags, 0, &phys_addr);

	if (ret)
		puts("pci_hose_bus_to_phys: invalid physical address\n");

	return phys_addr;
}

int __pci_hose_phys_to_bus(struct pci_controller *hose,
			   phys_addr_t phys_addr,
			   unsigned long flags,
			   unsigned long skip_mask,
			   pci_addr_t *ba)
{
	struct pci_region *res;
	pci_addr_t bus_addr;
	int i;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		if (res->flags & skip_mask)
			continue;

		bus_addr = phys_addr - res->phys_start + res->bus_start;

		if (bus_addr >= res->bus_start &&
		    (bus_addr - res->bus_start) < res->size) {
			*ba = bus_addr;
			return 0;
		}
	}

	return 1;
}

/*
 * pci_hose_phys_to_bus(): Convert physical address to bus address
 * @hose:	PCI hose of the root PCI controller
 * @phys_addr:	physical address to convert
 * @flags:	flags of pci regions
 * Return: bus address if OK, 0 on error
 */
pci_addr_t pci_hose_phys_to_bus(struct pci_controller *hose,
				phys_addr_t phys_addr,
				unsigned long flags)
{
	pci_addr_t bus_addr = 0;
	int ret;

	if (!hose) {
		puts("pci_hose_phys_to_bus: invalid hose\n");
		return bus_addr;
	}

	/*
	 * if PCI_REGION_MEM is set we do a two pass search with preference
	 * on matches that don't have PCI_REGION_SYS_MEMORY set
	 */
	if ((flags & PCI_REGION_TYPE) == PCI_REGION_MEM) {
		ret = __pci_hose_phys_to_bus(hose, phys_addr,
				flags, PCI_REGION_SYS_MEMORY, &bus_addr);
		if (!ret)
			return bus_addr;
	}

	ret = __pci_hose_phys_to_bus(hose, phys_addr, flags, 0, &bus_addr);

	if (ret)
		puts("pci_hose_phys_to_bus: invalid physical address\n");

	return bus_addr;
}

pci_dev_t pci_find_device(unsigned int vendor, unsigned int device, int index)
{
	struct pci_device_id ids[2] = { {}, {0, 0} };

	ids[0].vendor = vendor;
	ids[0].device = device;

	return pci_find_devices(ids, index);
}

pci_dev_t pci_hose_find_devices(struct pci_controller *hose, int busnum,
				struct pci_device_id *ids, int *indexp)
{
	int found_multi = 0;
	u16 vendor, device;
	u8 header_type;
	pci_dev_t bdf;
	int i;

	for (bdf = PCI_BDF(busnum, 0, 0);
	     bdf < PCI_BDF(busnum + 1, 0, 0);
	     bdf += PCI_BDF(0, 0, 1)) {
		if (pci_skip_dev(hose, bdf))
			continue;

		if (!PCI_FUNC(bdf)) {
			pci_read_config_byte(bdf, PCI_HEADER_TYPE,
					     &header_type);
			found_multi = header_type & 0x80;
		} else {
			if (!found_multi)
				continue;
		}

		pci_read_config_word(bdf, PCI_VENDOR_ID, &vendor);
		pci_read_config_word(bdf, PCI_DEVICE_ID, &device);

		for (i = 0; ids[i].vendor != 0; i++) {
			if (vendor == ids[i].vendor &&
			    device == ids[i].device) {
				if ((*indexp) <= 0)
					return bdf;

				(*indexp)--;
			}
		}
	}

	return -1;
}

pci_dev_t pci_find_class(uint find_class, int index)
{
	int bus;
	int devnum;
	pci_dev_t bdf;
	uint32_t class;

	for (bus = 0; bus <= pci_last_busno(); bus++) {
		for (devnum = 0; devnum < PCI_MAX_PCI_DEVICES - 1; devnum++) {
			pci_read_config_dword(PCI_BDF(bus, devnum, 0),
					      PCI_CLASS_REVISION, &class);
			if (class >> 16 == 0xffff)
				continue;

			for (bdf = PCI_BDF(bus, devnum, 0);
					bdf <= PCI_BDF(bus, devnum,
						PCI_MAX_PCI_FUNCTIONS - 1);
					bdf += PCI_BDF(0, 0, 1)) {
				pci_read_config_dword(bdf, PCI_CLASS_REVISION,
						      &class);
				class >>= 8;

				if (class != find_class)
					continue;
				/*
				 * Decrement the index. We want to return the
				 * correct device, so index is 0 for the first
				 * matching device, 1 for the second, etc.
				 */
				if (index) {
					index--;
					continue;
				}
				/* Return index'th controller. */
				return bdf;
			}
		}
	}

	return -ENODEV;
}
#endif /* CONFIG_DM_PCI_COMPAT */
