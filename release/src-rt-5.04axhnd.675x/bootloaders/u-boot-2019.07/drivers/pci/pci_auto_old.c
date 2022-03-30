// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI autoconfiguration library (legacy version, do not change)
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 */

#include <common.h>
#include <errno.h>
#include <pci.h>

/*
 * Do not change this file. Instead, convert your board to use CONFIG_DM_PCI
 * and change pci_auto.c.
 */

/* the user can define CONFIG_SYS_PCI_CACHE_LINE_SIZE to avoid problems */
#ifndef CONFIG_SYS_PCI_CACHE_LINE_SIZE
#define CONFIG_SYS_PCI_CACHE_LINE_SIZE	8
#endif

/*
 *
 */

void pciauto_setup_device(struct pci_controller *hose,
			  pci_dev_t dev, int bars_num,
			  struct pci_region *mem,
			  struct pci_region *prefetch,
			  struct pci_region *io)
{
	u32 bar_response;
	pci_size_t bar_size;
	u16 cmdstat = 0;
	int bar, bar_nr = 0;
#ifndef CONFIG_PCI_ENUM_ONLY
	u8 header_type;
	int rom_addr;
	pci_addr_t bar_value;
	struct pci_region *bar_res;
	int found_mem64 = 0;
#endif
	u16 class;

	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &cmdstat);
	cmdstat = (cmdstat & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) | PCI_COMMAND_MASTER;

	for (bar = PCI_BASE_ADDRESS_0;
		bar < PCI_BASE_ADDRESS_0 + (bars_num * 4); bar += 4) {
		/* Tickle the BAR and get the response */
#ifndef CONFIG_PCI_ENUM_ONLY
		pci_hose_write_config_dword(hose, dev, bar, 0xffffffff);
#endif
		pci_hose_read_config_dword(hose, dev, bar, &bar_response);

		/* If BAR is not implemented go to the next BAR */
		if (!bar_response)
			continue;

#ifndef CONFIG_PCI_ENUM_ONLY
		found_mem64 = 0;
#endif

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = ((~(bar_response & PCI_BASE_ADDRESS_IO_MASK))
				   & 0xffff) + 1;
#ifndef CONFIG_PCI_ENUM_ONLY
			bar_res = io;
#endif

			debug("PCI Autoconfig: BAR %d, I/O, size=0x%llx, ",
			      bar_nr, (unsigned long long)bar_size);
		} else {
			if ((bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
			     PCI_BASE_ADDRESS_MEM_TYPE_64) {
				u32 bar_response_upper;
				u64 bar64;

#ifndef CONFIG_PCI_ENUM_ONLY
				pci_hose_write_config_dword(hose, dev, bar + 4,
					0xffffffff);
#endif
				pci_hose_read_config_dword(hose, dev, bar + 4,
					&bar_response_upper);

				bar64 = ((u64)bar_response_upper << 32) | bar_response;

				bar_size = ~(bar64 & PCI_BASE_ADDRESS_MEM_MASK) + 1;
#ifndef CONFIG_PCI_ENUM_ONLY
				found_mem64 = 1;
#endif
			} else {
				bar_size = (u32)(~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1);
			}
#ifndef CONFIG_PCI_ENUM_ONLY
			if (prefetch && (bar_response & PCI_BASE_ADDRESS_MEM_PREFETCH))
				bar_res = prefetch;
			else
				bar_res = mem;

			debug("PCI Autoconfig: BAR %d, %s, size=0x%llx, ",
			      bar_nr, bar_res == prefetch ? "Prf" : "Mem",
			      (unsigned long long)bar_size);
#endif
		}

#ifndef CONFIG_PCI_ENUM_ONLY
		if (pciauto_region_allocate(bar_res, bar_size,
					    &bar_value, found_mem64) == 0) {
			/* Write it out and update our limit */
			pci_hose_write_config_dword(hose, dev, bar, (u32)bar_value);

			if (found_mem64) {
				bar += 4;
#ifdef CONFIG_SYS_PCI_64BIT
				pci_hose_write_config_dword(hose, dev, bar, (u32)(bar_value>>32));
#else
				/*
				 * If we are a 64-bit decoder then increment to the
				 * upper 32 bits of the bar and force it to locate
				 * in the lower 4GB of memory.
				 */
				pci_hose_write_config_dword(hose, dev, bar, 0x00000000);
#endif
			}

		}
#endif
		cmdstat |= (bar_response & PCI_BASE_ADDRESS_SPACE) ?
			PCI_COMMAND_IO : PCI_COMMAND_MEMORY;

		debug("\n");

		bar_nr++;
	}

#ifndef CONFIG_PCI_ENUM_ONLY
	/* Configure the expansion ROM address */
	pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);
	header_type &= 0x7f;
	if (header_type != PCI_HEADER_TYPE_CARDBUS) {
		rom_addr = (header_type == PCI_HEADER_TYPE_NORMAL) ?
			   PCI_ROM_ADDRESS : PCI_ROM_ADDRESS1;
		pci_hose_write_config_dword(hose, dev, rom_addr, 0xfffffffe);
		pci_hose_read_config_dword(hose, dev, rom_addr, &bar_response);
		if (bar_response) {
			bar_size = -(bar_response & ~1);
			debug("PCI Autoconfig: ROM, size=%#x, ",
			      (unsigned int)bar_size);
			if (pciauto_region_allocate(mem, bar_size,
						    &bar_value, false) == 0) {
				pci_hose_write_config_dword(hose, dev, rom_addr,
							    bar_value);
			}
			cmdstat |= PCI_COMMAND_MEMORY;
			debug("\n");
		}
	}
#endif

	/* PCI_COMMAND_IO must be set for VGA device */
	pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);
	if (class == PCI_CLASS_DISPLAY_VGA)
		cmdstat |= PCI_COMMAND_IO;

	pci_hose_write_config_word(hose, dev, PCI_COMMAND, cmdstat);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE,
		CONFIG_SYS_PCI_CACHE_LINE_SIZE);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
}

void pciauto_prescan_setup_bridge(struct pci_controller *hose,
					 pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;
	u16 cmdstat, prefechable_64;

	pci_mem = hose->pci_mem;
	pci_prefetch = hose->pci_prefetch;
	pci_io = hose->pci_io;

	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &cmdstat);
	pci_hose_read_config_word(hose, dev, PCI_PREF_MEMORY_BASE,
				&prefechable_64);
	prefechable_64 &= PCI_PREF_RANGE_TYPE_MASK;

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_PRIMARY_BUS,
				   PCI_BUS(dev) - hose->first_busno);
	pci_hose_write_config_byte(hose, dev, PCI_SECONDARY_BUS,
				   sub_bus - hose->first_busno);
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS, 0xff);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		/* Set up memory and I/O filter limits, assume 32-bit I/O space */
		pci_hose_write_config_word(hose, dev, PCI_MEMORY_BASE,
					(pci_mem->bus_lower & 0xfff00000) >> 16);

		cmdstat |= PCI_COMMAND_MEMORY;
	}

	if (pci_prefetch) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		/* Set up memory and I/O filter limits, assume 32-bit I/O space */
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_BASE,
					(pci_prefetch->bus_lower & 0xfff00000) >> 16);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64)
#ifdef CONFIG_SYS_PCI_64BIT
			pci_hose_write_config_dword(hose, dev,
					PCI_PREF_BASE_UPPER32,
					pci_prefetch->bus_lower >> 32);
#else
			pci_hose_write_config_dword(hose, dev,
					PCI_PREF_BASE_UPPER32,
					0x0);
#endif

		cmdstat |= PCI_COMMAND_MEMORY;
	} else {
		/* We don't support prefetchable memory for now, so disable */
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_BASE, 0x1000);
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_LIMIT, 0x0);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64) {
			pci_hose_write_config_word(hose, dev, PCI_PREF_BASE_UPPER32, 0x0);
			pci_hose_write_config_word(hose, dev, PCI_PREF_LIMIT_UPPER32, 0x0);
		}
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_BASE,
					(pci_io->bus_lower & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_BASE_UPPER16,
					(pci_io->bus_lower & 0xffff0000) >> 16);

		cmdstat |= PCI_COMMAND_IO;
	}

	/* Enable memory and I/O accesses, enable bus master */
	pci_hose_write_config_word(hose, dev, PCI_COMMAND,
					cmdstat | PCI_COMMAND_MASTER);
}

void pciauto_postscan_setup_bridge(struct pci_controller *hose,
					  pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;

	pci_mem = hose->pci_mem;
	pci_prefetch = hose->pci_prefetch;
	pci_io = hose->pci_io;

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS,
				   sub_bus - hose->first_busno);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		pci_hose_write_config_word(hose, dev, PCI_MEMORY_LIMIT,
				(pci_mem->bus_lower - 1) >> 16);
	}

	if (pci_prefetch) {
		u16 prefechable_64;

		pci_hose_read_config_word(hose, dev,
					PCI_PREF_MEMORY_LIMIT,
					&prefechable_64);
		prefechable_64 &= PCI_PREF_RANGE_TYPE_MASK;

		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_LIMIT,
				(pci_prefetch->bus_lower - 1) >> 16);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64)
#ifdef CONFIG_SYS_PCI_64BIT
			pci_hose_write_config_dword(hose, dev,
					PCI_PREF_LIMIT_UPPER32,
					(pci_prefetch->bus_lower - 1) >> 32);
#else
			pci_hose_write_config_dword(hose, dev,
					PCI_PREF_LIMIT_UPPER32,
					0x0);
#endif
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_LIMIT,
				((pci_io->bus_lower - 1) & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_LIMIT_UPPER16,
				((pci_io->bus_lower - 1) & 0xffff0000) >> 16);
	}
}


/*
 * HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;
	unsigned int sub_bus = PCI_BUS(dev);
	unsigned short class;
	int n;

	pci_mem = hose->pci_mem;
	pci_prefetch = hose->pci_prefetch;
	pci_io = hose->pci_io;

	pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

	switch (class) {
	case PCI_CLASS_BRIDGE_PCI:
		debug("PCI Autoconfig: Found P2P bridge, device %d\n",
		      PCI_DEV(dev));

		pciauto_setup_device(hose, dev, 2, pci_mem,
				     pci_prefetch, pci_io);

		/* Passing in current_busno allows for sibling P2P bridges */
		hose->current_busno++;
		pciauto_prescan_setup_bridge(hose, dev, hose->current_busno);
		/*
		 * need to figure out if this is a subordinate bridge on the bus
		 * to be able to properly set the pri/sec/sub bridge registers.
		 */
		n = pci_hose_scan_bus(hose, hose->current_busno);

		/* figure out the deepest we've gone for this leg */
		sub_bus = max((unsigned int)n, sub_bus);
		pciauto_postscan_setup_bridge(hose, dev, sub_bus);

		sub_bus = hose->current_busno;
		break;

	case PCI_CLASS_BRIDGE_CARDBUS:
		/*
		 * just do a minimal setup of the bridge,
		 * let the OS take care of the rest
		 */
		pciauto_setup_device(hose, dev, 0, pci_mem,
				     pci_prefetch, pci_io);

		debug("PCI Autoconfig: Found P2CardBus bridge, device %d\n",
		      PCI_DEV(dev));

		hose->current_busno++;
		break;

#if defined(CONFIG_PCIAUTO_SKIP_HOST_BRIDGE)
	case PCI_CLASS_BRIDGE_OTHER:
		debug("PCI Autoconfig: Skipping bridge device %d\n",
		      PCI_DEV(dev));
		break;
#endif
#if defined(CONFIG_ARCH_MPC834X) && !defined(CONFIG_TARGET_VME8349) && \
		!defined(CONFIG_TARGET_CADDY2)
	case PCI_CLASS_BRIDGE_OTHER:
		/*
		 * The host/PCI bridge 1 seems broken in 8349 - it presents
		 * itself as 'PCI_CLASS_BRIDGE_OTHER' and appears as an _agent_
		 * device claiming resources io/mem/irq.. we only allow for
		 * the PIMMR window to be allocated (BAR0 - 1MB size)
		 */
		debug("PCI Autoconfig: Broken bridge found, only minimal config\n");
		pciauto_setup_device(hose, dev, 0, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);
		break;
#endif

	case PCI_CLASS_PROCESSOR_POWERPC: /* an agent or end-point */
		debug("PCI AutoConfig: Found PowerPC device\n");

	default:
		pciauto_setup_device(hose, dev, 6, pci_mem,
				     pci_prefetch, pci_io);
		break;
	}

	return sub_bus;
}
