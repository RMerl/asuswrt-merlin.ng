// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI auto-configuration library
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 *
 * Modifications for driver model:
 * Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <pci.h>

void pciauto_region_init(struct pci_region *res)
{
	/*
	 * Avoid allocating PCI resources from address 0 -- this is illegal
	 * according to PCI 2.1 and moreover, this is known to cause Linux IDE
	 * drivers to fail. Use a reasonable starting value of 0x1000 instead
	 * if the bus start address is below 0x1000.
	 */
	res->bus_lower = res->bus_start < 0x1000 ? 0x1000 : res->bus_start;
}

void pciauto_region_align(struct pci_region *res, pci_size_t size)
{
	res->bus_lower = ((res->bus_lower - 1) | (size - 1)) + 1;
}

int pciauto_region_allocate(struct pci_region *res, pci_size_t size,
	pci_addr_t *bar, bool supports_64bit)
{
	pci_addr_t addr;

	if (!res) {
		debug("No resource\n");
		goto error;
	}

	addr = ((res->bus_lower - 1) | (size - 1)) + 1;

	if (addr - res->bus_start + size > res->size) {
		debug("No room in resource, avail start=%llx / size=%llx, "
		      "need=%llx\n", (unsigned long long)res->bus_lower,
		      (unsigned long long)res->size, (unsigned long long)size);
		goto error;
	}

	if (upper_32_bits(addr) && !supports_64bit) {
		debug("Cannot assign 64-bit address to 32-bit-only resource\n");
		goto error;
	}

	res->bus_lower = addr + size;

	debug("address=0x%llx bus_lower=0x%llx\n", (unsigned long long)addr,
	      (unsigned long long)res->bus_lower);

	*bar = addr;
	return 0;

 error:
	*bar = (pci_addr_t)-1;
	return -1;
}

static void pciauto_show_region(const char *name, struct pci_region *region)
{
	pciauto_region_init(region);
	debug("PCI Autoconfig: Bus %s region: [%llx-%llx],\n"
	      "\t\tPhysical Memory [%llx-%llx]\n", name,
	      (unsigned long long)region->bus_start,
	      (unsigned long long)(region->bus_start + region->size - 1),
	      (unsigned long long)region->phys_start,
	      (unsigned long long)(region->phys_start + region->size - 1));
}

void pciauto_config_init(struct pci_controller *hose)
{
	int i;

	hose->pci_io = NULL;
	hose->pci_mem = NULL;
	hose->pci_prefetch = NULL;

	for (i = 0; i < hose->region_count; i++) {
		switch (hose->regions[i].flags) {
		case PCI_REGION_IO:
			if (!hose->pci_io ||
			    hose->pci_io->size < hose->regions[i].size)
				hose->pci_io = hose->regions + i;
			break;
		case PCI_REGION_MEM:
			if (!hose->pci_mem ||
			    hose->pci_mem->size < hose->regions[i].size)
				hose->pci_mem = hose->regions + i;
			break;
		case (PCI_REGION_MEM | PCI_REGION_PREFETCH):
			if (!hose->pci_prefetch ||
			    hose->pci_prefetch->size < hose->regions[i].size)
				hose->pci_prefetch = hose->regions + i;
			break;
		}
	}


	if (hose->pci_mem)
		pciauto_show_region("Memory", hose->pci_mem);
	if (hose->pci_prefetch)
		pciauto_show_region("Prefetchable Mem", hose->pci_prefetch);
	if (hose->pci_io)
		pciauto_show_region("I/O", hose->pci_io);
}
