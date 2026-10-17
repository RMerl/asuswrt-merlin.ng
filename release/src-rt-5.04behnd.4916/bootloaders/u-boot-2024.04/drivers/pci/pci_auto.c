// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI autoconfiguration library
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 * Copyright (c) 2021  Maciej W. Rozycki <macro@orcam.me.uk>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <pci.h>
#include <time.h>
#include "pci_internal.h"

/* the user can define CFG_SYS_PCI_CACHE_LINE_SIZE to avoid problems */
#ifndef CFG_SYS_PCI_CACHE_LINE_SIZE
#define CFG_SYS_PCI_CACHE_LINE_SIZE	8
#endif

static void dm_pciauto_setup_device(struct udevice *dev,
				    struct pci_region *mem,
				    struct pci_region *prefetch,
				    struct pci_region *io)
{
	u32 bar_response;
	pci_size_t bar_size;
	u16 cmdstat = 0;
	int bar, bar_nr = 0;
	int bars_num;
	u8 header_type;
	int rom_addr;
	pci_addr_t bar_value;
	struct pci_region *bar_res = NULL;
	int found_mem64 = 0;
	u16 class;

	dm_pci_read_config16(dev, PCI_COMMAND, &cmdstat);
	cmdstat = (cmdstat & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) |
			PCI_COMMAND_MASTER;

	dm_pci_read_config8(dev, PCI_HEADER_TYPE, &header_type);
	header_type &= 0x7f;

	switch (header_type) {
	case PCI_HEADER_TYPE_NORMAL:
		bars_num = 6;
		break;
	case PCI_HEADER_TYPE_BRIDGE:
		bars_num = 2;
		break;
	case PCI_HEADER_TYPE_CARDBUS:
		/* CardBus header does not have any BAR */
		bars_num = 0;
		break;
	default:
		/* Skip configuring BARs for unknown header types */
		bars_num = 0;
		break;
	}

	for (bar = PCI_BASE_ADDRESS_0;
	     bar < PCI_BASE_ADDRESS_0 + (bars_num * 4); bar += 4) {
		int ret = 0;

		/* Tickle the BAR and get the response */
		dm_pci_write_config32(dev, bar, 0xffffffff);
		dm_pci_read_config32(dev, bar, &bar_response);

		/* If BAR is not implemented (or invalid) go to the next BAR */
		if (!bar_response || bar_response == 0xffffffff)
			continue;

		found_mem64 = 0;

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = bar_response & PCI_BASE_ADDRESS_IO_MASK;
			bar_size &= ~(bar_size - 1);

			bar_res = io;

			debug("PCI Autoconfig: BAR %d, I/O, size=0x%llx, ",
			      bar_nr, (unsigned long long)bar_size);
		} else {
			if ((bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
			     PCI_BASE_ADDRESS_MEM_TYPE_64) {
				u32 bar_response_upper;
				u64 bar64;

				dm_pci_write_config32(dev, bar + 4, 0xffffffff);
				dm_pci_read_config32(dev, bar + 4,
						     &bar_response_upper);

				bar64 = ((u64)bar_response_upper << 32) |
						bar_response;

				bar_size = ~(bar64 & PCI_BASE_ADDRESS_MEM_MASK)
						+ 1;
				found_mem64 = 1;
			} else {
				bar_size = (u32)(~(bar_response &
						PCI_BASE_ADDRESS_MEM_MASK) + 1);
			}

			if (prefetch &&
			    (bar_response & PCI_BASE_ADDRESS_MEM_PREFETCH))
				bar_res = prefetch;
			else
				bar_res = mem;

			debug("PCI Autoconfig: BAR %d, %s%s, size=0x%llx, ",
			      bar_nr, bar_res == prefetch ? "Prf" : "Mem",
			      found_mem64 ? "64" : "",
			      (unsigned long long)bar_size);
		}

		ret = pciauto_region_allocate(bar_res, bar_size,
					      &bar_value, found_mem64);
		if (ret)
			printf("PCI: Failed autoconfig bar %x\n", bar);

		if (!ret) {
			/* Write it out and update our limit */
			dm_pci_write_config32(dev, bar, (u32)bar_value);

			if (found_mem64) {
				bar += 4;
#ifdef CONFIG_SYS_PCI_64BIT
				dm_pci_write_config32(dev, bar,
						      (u32)(bar_value >> 32));
#else
				/*
				 * If we are a 64-bit decoder then increment to
				 * the upper 32 bits of the bar and force it to
				 * locate in the lower 4GB of memory.
				 */
				dm_pci_write_config32(dev, bar, 0x00000000);
#endif
			}
		}

		cmdstat |= (bar_response & PCI_BASE_ADDRESS_SPACE) ?
			PCI_COMMAND_IO : PCI_COMMAND_MEMORY;

		debug("\n");

		bar_nr++;
	}

	/* Configure the expansion ROM address */
	if (header_type == PCI_HEADER_TYPE_NORMAL ||
	    header_type == PCI_HEADER_TYPE_BRIDGE) {
		rom_addr = (header_type == PCI_HEADER_TYPE_NORMAL) ?
			PCI_ROM_ADDRESS : PCI_ROM_ADDRESS1;
		dm_pci_write_config32(dev, rom_addr, 0xfffffffe);
		dm_pci_read_config32(dev, rom_addr, &bar_response);
		if (bar_response) {
			bar_size = -(bar_response & ~1);
			debug("PCI Autoconfig: ROM, size=%#x, ",
			      (unsigned int)bar_size);
			if (pciauto_region_allocate(mem, bar_size, &bar_value,
						    false) == 0) {
				dm_pci_write_config32(dev, rom_addr, bar_value);
			}
			cmdstat |= PCI_COMMAND_MEMORY;
			debug("\n");
		}
	}

	/* PCI_COMMAND_IO must be set for VGA device */
	dm_pci_read_config16(dev, PCI_CLASS_DEVICE, &class);
	if (class == PCI_CLASS_DISPLAY_VGA)
		cmdstat |= PCI_COMMAND_IO;

	dm_pci_write_config16(dev, PCI_COMMAND, cmdstat);
	dm_pci_write_config8(dev, PCI_CACHE_LINE_SIZE,
			     CFG_SYS_PCI_CACHE_LINE_SIZE);
	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x80);
}

/*
 * Check if the link of a downstream PCIe port operates correctly.
 *
 * For that check if the optional Data Link Layer Link Active status gets
 * on within a 200ms period or failing that wait until the completion of
 * that period and check if link training has shown the completed status
 * continuously throughout the second half of that period.
 *
 * Observation with the ASMedia ASM2824 Gen 3 switch indicates it takes
 * 11-44ms to indicate the Data Link Layer Link Active status at 2.5GT/s,
 * though it may take a couple of link training iterations.
 */
static bool dm_pciauto_exp_link_stable(struct udevice *dev, int pcie_off)
{
	u64 loops = 0, trcount = 0, ntrcount = 0, flips = 0;
	bool dllla, lnktr, plnktr;
	u16 exp_lnksta;
	pci_dev_t bdf;
	u64 end;

	dm_pci_read_config16(dev, pcie_off + PCI_EXP_LNKSTA, &exp_lnksta);
	plnktr = !!(exp_lnksta & PCI_EXP_LNKSTA_LT);

	end = get_ticks() + usec_to_tick(200000);
	do {
		dm_pci_read_config16(dev, pcie_off + PCI_EXP_LNKSTA,
				     &exp_lnksta);
		dllla = !!(exp_lnksta & PCI_EXP_LNKSTA_DLLLA);
		lnktr = !!(exp_lnksta & PCI_EXP_LNKSTA_LT);

		flips += plnktr ^ lnktr;
		if (lnktr) {
			ntrcount = 0;
			trcount++;
		} else {
			ntrcount++;
		}
		loops++;

		plnktr = lnktr;
	} while (!dllla && get_ticks() < end);

	bdf = dm_pci_get_bdf(dev);
	debug("PCI Autoconfig: %02x.%02x.%02x: Fixup link: DL active: %u; "
	      "%3llu flips, %6llu loops of which %6llu while training, "
	      "final %6llu stable\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
	      (unsigned int)dllla,
	      (unsigned long long)flips, (unsigned long long)loops,
	      (unsigned long long)trcount, (unsigned long long)ntrcount);

	return dllla || ntrcount >= loops / 2;
}

/*
 * Retrain the link of a downstream PCIe port by hand if necessary.
 *
 * This is needed at least where a downstream port of the ASMedia ASM2824
 * Gen 3 switch is wired to the upstream port of the Pericom PI7C9X2G304
 * Gen 2 switch, and observed with the Delock Riser Card PCI Express x1 >
 * 2 x PCIe x1 device, P/N 41433, plugged into the SiFive HiFive Unmatched
 * board.
 *
 * In such a configuration the switches are supposed to negotiate the link
 * speed of preferably 5.0GT/s, falling back to 2.5GT/s.  However the link
 * continues switching between the two speeds indefinitely and the data
 * link layer never reaches the active state, with link training reported
 * repeatedly active ~84% of the time.  Forcing the target link speed to
 * 2.5GT/s with the upstream ASM2824 device makes the two switches talk to
 * each other correctly however.  And more interestingly retraining with a
 * higher target link speed afterwards lets the two successfully negotiate
 * 5.0GT/s.
 *
 * As this can potentially happen with any device and is cheap in the case
 * of correctly operating hardware, let's do it for all downstream ports,
 * for root complexes, PCIe switches and PCI/PCI-X to PCIe bridges.
 *
 * First check if automatic link training may have failed to complete, as
 * indicated by the optional Data Link Layer Link Active status being off
 * and the Link Bandwidth Management Status indicating that hardware has
 * changed the link speed or width in an attempt to correct unreliable
 * link operation.  If this is the case, then check if the link operates
 * correctly by seeing whether it is being trained excessively.  If it is,
 * then conclude the link is broken.
 *
 * In that case restrict the speed to 2.5GT/s, observing that the Target
 * Link Speed field is sticky and therefore the link will stay restricted
 * even after a device reset is later made by an OS that is unaware of the
 * problem.  With the speed restricted request that the link be retrained
 * and check again if the link operates correctly.  If not, then set the
 * Target Link Speed back to the original value.
 *
 * This requires the presence of the Link Control 2 register, so make sure
 * the PCI Express Capability Version is at least 2.  Also don't try, for
 * obvious reasons, to limit the speed if 2.5GT/s is the only link speed
 * supported.
 */
static void dm_pciauto_exp_fixup_link(struct udevice *dev, int pcie_off)
{
	u16 exp_lnksta, exp_lnkctl, exp_lnkctl2;
	u16 exp_flags, exp_type, exp_version;
	u32 exp_lnkcap;
	pci_dev_t bdf;

	dm_pci_read_config16(dev, pcie_off + PCI_EXP_FLAGS, &exp_flags);
	exp_version = exp_flags & PCI_EXP_FLAGS_VERS;
	if (exp_version < 2)
		return;

	exp_type = (exp_flags & PCI_EXP_FLAGS_TYPE) >> 4;
	switch (exp_type) {
	case PCI_EXP_TYPE_ROOT_PORT:
	case PCI_EXP_TYPE_DOWNSTREAM:
	case PCI_EXP_TYPE_PCIE_BRIDGE:
		break;
	default:
		return;
	}

	dm_pci_read_config32(dev, pcie_off + PCI_EXP_LNKCAP, &exp_lnkcap);
	if ((exp_lnkcap & PCI_EXP_LNKCAP_SLS) <= PCI_EXP_LNKCAP_SLS_2_5GB)
		return;

	dm_pci_read_config16(dev, pcie_off + PCI_EXP_LNKSTA, &exp_lnksta);
	if ((exp_lnksta & (PCI_EXP_LNKSTA_LBMS | PCI_EXP_LNKSTA_DLLLA)) !=
	    PCI_EXP_LNKSTA_LBMS)
		return;

	if (dm_pciauto_exp_link_stable(dev, pcie_off))
		return;

	bdf = dm_pci_get_bdf(dev);
	printf("PCI Autoconfig: %02x.%02x.%02x: "
	       "Downstream link non-functional\n",
	       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	printf("PCI Autoconfig: %02x.%02x.%02x: "
	       "Retrying with speed restricted to 2.5GT/s...\n",
	       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	dm_pci_read_config16(dev, pcie_off + PCI_EXP_LNKCTL, &exp_lnkctl);
	dm_pci_read_config16(dev, pcie_off + PCI_EXP_LNKCTL2, &exp_lnkctl2);

	dm_pci_write_config16(dev, pcie_off + PCI_EXP_LNKCTL2,
			      (exp_lnkctl2 & ~PCI_EXP_LNKCTL2_TLS) |
			      PCI_EXP_LNKCTL2_TLS_2_5GT);
	dm_pci_write_config16(dev, pcie_off + PCI_EXP_LNKCTL,
			      exp_lnkctl | PCI_EXP_LNKCTL_RL);

	if (dm_pciauto_exp_link_stable(dev, pcie_off)) {
		printf("PCI Autoconfig: %02x.%02x.%02x: Succeeded!\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	} else {
		printf("PCI Autoconfig: %02x.%02x.%02x: Failed!\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

		dm_pci_write_config16(dev, pcie_off + PCI_EXP_LNKCTL2,
				      exp_lnkctl2);
		dm_pci_write_config16(dev, pcie_off + PCI_EXP_LNKCTL,
				      exp_lnkctl | PCI_EXP_LNKCTL_RL);
	}
}

void dm_pciauto_prescan_setup_bridge(struct udevice *dev, int sub_bus)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;
	u16 cmdstat, prefechable_64;
	u8 io_32;
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *ctlr_hose = dev_get_uclass_priv(ctlr);
	int pcie_off;

	pci_mem = ctlr_hose->pci_mem;
	pci_prefetch = ctlr_hose->pci_prefetch;
	pci_io = ctlr_hose->pci_io;

	dm_pci_read_config16(dev, PCI_COMMAND, &cmdstat);
	dm_pci_read_config16(dev, PCI_PREF_MEMORY_BASE, &prefechable_64);
	prefechable_64 &= PCI_PREF_RANGE_TYPE_MASK;
	dm_pci_read_config8(dev, PCI_IO_BASE, &io_32);
	io_32 &= PCI_IO_RANGE_TYPE_MASK;

	/* Configure bus number registers */
	dm_pci_write_config8(dev, PCI_PRIMARY_BUS,
			     PCI_BUS(dm_pci_get_bdf(dev)) - dev_seq(ctlr));
	dm_pci_write_config8(dev, PCI_SECONDARY_BUS, sub_bus - dev_seq(ctlr));
	dm_pci_write_config8(dev, PCI_SUBORDINATE_BUS, 0xff);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		/*
		 * Set up memory and I/O filter limits, assume 32-bit
		 * I/O space
		 */
		dm_pci_write_config16(dev, PCI_MEMORY_BASE,
				      ((pci_mem->bus_lower & 0xfff00000) >> 16) &
				      PCI_MEMORY_RANGE_MASK);

		cmdstat |= PCI_COMMAND_MEMORY;
	}

	if (pci_prefetch) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		/*
		 * Set up memory and I/O filter limits, assume 32-bit
		 * I/O space
		 */
		dm_pci_write_config16(dev, PCI_PREF_MEMORY_BASE,
				(((pci_prefetch->bus_lower & 0xfff00000) >> 16) &
				PCI_PREF_RANGE_MASK) | prefechable_64);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64)
#ifdef CONFIG_SYS_PCI_64BIT
			dm_pci_write_config32(dev, PCI_PREF_BASE_UPPER32,
					      pci_prefetch->bus_lower >> 32);
#else
			dm_pci_write_config32(dev, PCI_PREF_BASE_UPPER32, 0x0);
#endif

		cmdstat |= PCI_COMMAND_MEMORY;
	} else {
		/* We don't support prefetchable memory for now, so disable */
		dm_pci_write_config16(dev, PCI_PREF_MEMORY_BASE, 0xfff0 |
								prefechable_64);
		dm_pci_write_config16(dev, PCI_PREF_MEMORY_LIMIT, 0x0 |
								prefechable_64);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64) {
			dm_pci_write_config16(dev, PCI_PREF_BASE_UPPER32, 0x0);
			dm_pci_write_config16(dev, PCI_PREF_LIMIT_UPPER32, 0x0);
		}
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		dm_pci_write_config8(dev, PCI_IO_BASE,
				     (((pci_io->bus_lower & 0x0000f000) >> 8) &
				     PCI_IO_RANGE_MASK) | io_32);
		if (io_32 == PCI_IO_RANGE_TYPE_32)
			dm_pci_write_config16(dev, PCI_IO_BASE_UPPER16,
				      (pci_io->bus_lower & 0xffff0000) >> 16);

		cmdstat |= PCI_COMMAND_IO;
	} else {
		/* Disable I/O if unsupported */
		dm_pci_write_config8(dev, PCI_IO_BASE, 0xf0 | io_32);
		dm_pci_write_config8(dev, PCI_IO_LIMIT, 0x0 | io_32);
		if (io_32 == PCI_IO_RANGE_TYPE_32) {
			dm_pci_write_config16(dev, PCI_IO_BASE_UPPER16, 0x0);
			dm_pci_write_config16(dev, PCI_IO_LIMIT_UPPER16, 0x0);
		}
	}

	/* For PCIe devices see if we need to retrain the link by hand */
	pcie_off = dm_pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (pcie_off)
		dm_pciauto_exp_fixup_link(dev, pcie_off);

	/* Enable memory and I/O accesses, enable bus master */
	dm_pci_write_config16(dev, PCI_COMMAND, cmdstat | PCI_COMMAND_MASTER);
}

void dm_pciauto_postscan_setup_bridge(struct udevice *dev, int sub_bus)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *ctlr_hose = dev_get_uclass_priv(ctlr);

	pci_mem = ctlr_hose->pci_mem;
	pci_prefetch = ctlr_hose->pci_prefetch;
	pci_io = ctlr_hose->pci_io;

	/* Configure bus number registers */
	dm_pci_write_config8(dev, PCI_SUBORDINATE_BUS, sub_bus - dev_seq(ctlr));

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		dm_pci_write_config16(dev, PCI_MEMORY_LIMIT,
				      ((pci_mem->bus_lower - 1) >> 16) &
				      PCI_MEMORY_RANGE_MASK);
	}

	if (pci_prefetch) {
		u16 prefechable_64;

		dm_pci_read_config16(dev, PCI_PREF_MEMORY_LIMIT,
				     &prefechable_64);
		prefechable_64 &= PCI_PREF_RANGE_TYPE_MASK;

		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		dm_pci_write_config16(dev, PCI_PREF_MEMORY_LIMIT,
				      (((pci_prefetch->bus_lower - 1) >> 16) &
				       PCI_PREF_RANGE_MASK) | prefechable_64);
		if (prefechable_64 == PCI_PREF_RANGE_TYPE_64)
#ifdef CONFIG_SYS_PCI_64BIT
			dm_pci_write_config32(dev, PCI_PREF_LIMIT_UPPER32,
					(pci_prefetch->bus_lower - 1) >> 32);
#else
			dm_pci_write_config32(dev, PCI_PREF_LIMIT_UPPER32, 0x0);
#endif
	}

	if (pci_io) {
		u8 io_32;

		dm_pci_read_config8(dev, PCI_IO_LIMIT,
				     &io_32);
		io_32 &= PCI_IO_RANGE_TYPE_MASK;

		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		dm_pci_write_config8(dev, PCI_IO_LIMIT,
				((((pci_io->bus_lower - 1) & 0x0000f000) >> 8) &
				PCI_IO_RANGE_MASK) | io_32);
		if (io_32 == PCI_IO_RANGE_TYPE_32)
			dm_pci_write_config16(dev, PCI_IO_LIMIT_UPPER16,
				((pci_io->bus_lower - 1) & 0xffff0000) >> 16);
	}
}

/*
 * HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
int dm_pciauto_config_device(struct udevice *dev)
{
	struct pci_region *pci_mem;
	struct pci_region *pci_prefetch;
	struct pci_region *pci_io;
	unsigned int sub_bus = PCI_BUS(dm_pci_get_bdf(dev));
	unsigned short class;
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *ctlr_hose = dev_get_uclass_priv(ctlr);
	int ret;

	pci_mem = ctlr_hose->pci_mem;
	pci_prefetch = ctlr_hose->pci_prefetch;
	pci_io = ctlr_hose->pci_io;

	dm_pci_read_config16(dev, PCI_CLASS_DEVICE, &class);

	switch (class) {
	case PCI_CLASS_BRIDGE_PCI:
		debug("PCI Autoconfig: Found P2P bridge, device %d\n",
		      PCI_DEV(dm_pci_get_bdf(dev)));

		dm_pciauto_setup_device(dev, pci_mem, pci_prefetch, pci_io);

		ret = dm_pci_hose_probe_bus(dev);
		if (ret < 0)
			return log_msg_ret("probe", ret);
		sub_bus = ret;
		break;

	case PCI_CLASS_BRIDGE_CARDBUS:
		/*
		 * just do a minimal setup of the bridge,
		 * let the OS take care of the rest
		 */
		dm_pciauto_setup_device(dev, pci_mem, pci_prefetch, pci_io);

		debug("PCI Autoconfig: Found P2CardBus bridge, device %d\n",
		      PCI_DEV(dm_pci_get_bdf(dev)));

		break;

#if defined(CONFIG_PCIAUTO_SKIP_HOST_BRIDGE)
	case PCI_CLASS_BRIDGE_OTHER:
		debug("PCI Autoconfig: Skipping bridge device %d\n",
		      PCI_DEV(dm_pci_get_bdf(dev)));
		break;
#endif
#if defined(CONFIG_ARCH_MPC834X)
	case PCI_CLASS_BRIDGE_OTHER:
		/*
		 * The host/PCI bridge 1 seems broken in 8349 - it presents
		 * itself as 'PCI_CLASS_BRIDGE_OTHER' and appears as an _agent_
		 * device claiming resources io/mem/irq.. we only allow for
		 * the PIMMR window to be allocated (BAR0 - 1MB size)
		 */
		debug("PCI Autoconfig: Broken bridge found, only minimal config\n");
		dm_pciauto_setup_device(dev, 0, hose->pci_mem,
					hose->pci_prefetch, hose->pci_io);
		break;
#endif

	default:
		dm_pciauto_setup_device(dev, pci_mem, pci_prefetch, pci_io);
		break;
	}

	return sub_bus;
}
