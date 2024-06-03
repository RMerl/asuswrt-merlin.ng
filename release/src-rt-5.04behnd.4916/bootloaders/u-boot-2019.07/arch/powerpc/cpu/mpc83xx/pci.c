// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2007
 *
 * Author: Scott Wood <scottwood@freescale.com>,
 * with some bits from older board-specific PCI initialization.
 */

#include <common.h>
#include <pci.h>

#if defined(CONFIG_OF_LIBFDT)
#include <linux/libfdt.h>
#include <fdt_support.h>
#endif

#include <asm/mpc8349_pci.h>

#define MAX_BUSES 2

DECLARE_GLOBAL_DATA_PTR;

static struct pci_controller pci_hose[MAX_BUSES];
static int pci_num_buses;

static void pci_init_bus(int bus, struct pci_region *reg)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile pot83xx_t *pot = immr->ios.pot;
	volatile pcictrl83xx_t *pci_ctrl = &immr->pci_ctrl[bus];
	struct pci_controller *hose = &pci_hose[bus];
	u32 dev;
	u16 reg16;
	int i;

	if (bus == 1)
		pot += 3;

	/* Setup outbound translation windows */
	for (i = 0; i < 3; i++, reg++, pot++) {
		if (reg->size == 0)
			break;

		hose->regions[i] = *reg;
		hose->region_count++;

		pot->potar = reg->bus_start >> 12;
		pot->pobar = reg->phys_start >> 12;
		pot->pocmr = ~(reg->size - 1) >> 12;

		if (reg->flags & PCI_REGION_IO)
			pot->pocmr |= POCMR_IO;
#ifdef CONFIG_83XX_PCI_STREAMING
		else if (reg->flags & PCI_REGION_PREFETCH)
			pot->pocmr |= POCMR_SE;
#endif

		if (bus == 1)
			pot->pocmr |= POCMR_DST;

		pot->pocmr |= POCMR_EN;
	}

	/* Point inbound translation at RAM */
	pci_ctrl->pitar1 = 0;
	pci_ctrl->pibar1 = 0;
	pci_ctrl->piebar1 = 0;
	pci_ctrl->piwar1 = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP |
			   PIWAR_WTT_SNOOP | (__ilog2(gd->ram_size - 1));

	i = hose->region_count++;
	hose->regions[i].bus_start = 0;
	hose->regions[i].phys_start = 0;
	hose->regions[i].size = gd->ram_size;
	hose->regions[i].flags = PCI_REGION_MEM | PCI_REGION_SYS_MEMORY;

	hose->first_busno = pci_last_busno() + 1;
	hose->last_busno = 0xff;

	pci_setup_indirect(hose, CONFIG_SYS_IMMR + 0x8300 + bus * 0x80,
				 CONFIG_SYS_IMMR + 0x8304 + bus * 0x80);

	pci_register_hose(hose);

	/*
	 * Write to Command register
	 */
	reg16 = 0xff;
	dev = PCI_BDF(hose->first_busno, 0, 0);
	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(hose, dev, PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
#ifndef CONFIG_PCISLAVE
	/*
	 * Hose scan.
	 */
	hose->last_busno = pci_hose_scan(hose);
#endif
}

/*
 * The caller must have already set OCCR, and the PCI_LAW BARs
 * must have been set to cover all of the requested regions.
 *
 * If fewer than three regions are requested, then the region
 * list is terminated with a region of size 0.
 */
void mpc83xx_pci_init(int num_buses, struct pci_region **reg)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	int i;

	if (num_buses > MAX_BUSES) {
		printf("%d PCI buses requested, %d supported\n",
		       num_buses, MAX_BUSES);

		num_buses = MAX_BUSES;
	}

	pci_num_buses = num_buses;

	/*
	 * Release PCI RST Output signal.
	 * Power on to RST high must be at least 100 ms as per PCI spec.
	 * On warm boots only 1 ms is required, but we play it safe.
	 */
	udelay(100000);

	for (i = 0; i < num_buses; i++)
		immr->pci_ctrl[i].gcr = 1;

	/*
	 * RST high to first config access must be at least 2^25 cycles
	 * as per PCI spec.  This could be cut in half if we know we're
	 * running at 66MHz.  This could be insufficiently long if we're
	 * running the PCI bus at significantly less than 33MHz.
	 */
	udelay(1020000);

	for (i = 0; i < num_buses; i++)
		pci_init_bus(i, reg[i]);
}

#ifdef CONFIG_PCISLAVE

#define PCI_FUNCTION_CONFIG	0x44
#define PCI_FUNCTION_CFG_LOCK	0x20

/*
 * Unlock the configuration bit so that the host system can begin booting
 *
 * This should be used after you have:
 * 1) Called mpc83xx_pci_init()
 * 2) Set up your inbound translation windows to the appropriate size
 */
void mpc83xx_pcislave_unlock(int bus)
{
	struct pci_controller *hose = &pci_hose[bus];
	u32 dev;
	u16 reg16;

	/* Unlock configuration lock in PCI function configuration register */
	dev = PCI_BDF(hose->first_busno, 0, 0);
	pci_hose_read_config_word (hose, dev, PCI_FUNCTION_CONFIG, &reg16);
	reg16 &= ~(PCI_FUNCTION_CFG_LOCK);
	pci_hose_write_config_word (hose, dev, PCI_FUNCTION_CONFIG, reg16);

	/* The configuration bit is now unlocked, so we can scan the bus */
	hose->last_busno = pci_hose_scan(hose);
}
#endif

#if defined(CONFIG_OF_LIBFDT)
void ft_pci_setup(void *blob, bd_t *bd)
{
	int nodeoffset;
	int tmp[2];
	const char *path;

	if (pci_num_buses < 1)
		return;

	nodeoffset = fdt_path_offset(blob, "/aliases");
	if (nodeoffset >= 0) {
		path = fdt_getprop(blob, nodeoffset, "pci0", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(pci_hose[0].first_busno);
			tmp[1] = cpu_to_be32(pci_hose[0].last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);

			tmp[0] = cpu_to_be32(gd->pci_clk);
			do_fixup_by_path(blob, path, "clock-frequency",
				&tmp, sizeof(tmp[0]), 1);
		}

		if (pci_num_buses < 2)
			return;

		path = fdt_getprop(blob, nodeoffset, "pci1", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(pci_hose[1].first_busno);
			tmp[1] = cpu_to_be32(pci_hose[1].last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);

			tmp[0] = cpu_to_be32(gd->pci_clk);
			do_fixup_by_path(blob, path, "clock-frequency",
				&tmp, sizeof(tmp[0]), 1);
		}
	}
}
#endif /* CONFIG_OF_LIBFDT */
