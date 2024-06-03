// SPDX-License-Identifier: GPL-2.0+
/*
 * SH4 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 *
 * u-boot/arch/sh/cpu/sh4/pci-sh4.c
 */

#include <common.h>

#include <asm/processor.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <pci.h>

int pci_sh4_init(struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->region_count = 0;
	hose->last_busno = 0xff;

	/* PCI memory space */
	pci_set_region(hose->regions + 0,
		CONFIG_PCI_MEM_BUS,
		CONFIG_PCI_MEM_PHYS,
		CONFIG_PCI_MEM_SIZE,
		PCI_REGION_MEM);
	hose->region_count++;

	/* PCI IO space */
	pci_set_region(hose->regions + 1,
		CONFIG_PCI_IO_BUS,
		CONFIG_PCI_IO_PHYS,
		CONFIG_PCI_IO_SIZE,
		PCI_REGION_IO);
	hose->region_count++;

#if defined(CONFIG_PCI_SYS_BUS)
	/* PCI System Memory space */
	pci_set_region(hose->regions + 2,
		CONFIG_PCI_SYS_BUS,
		CONFIG_PCI_SYS_PHYS,
		CONFIG_PCI_SYS_SIZE,
		PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
	hose->region_count++;
#endif

	udelay(1000);

	pci_set_ops(hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    pci_sh4_read_config_dword,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    pci_sh4_write_config_dword);

	pci_register_hose(hose);

	udelay(1000);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
	hose->last_busno = pci_hose_scan(hose);
	return 0;
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 0;
}

#ifdef CONFIG_PCI_SCAN_SHOW
int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 1;
}
#endif /* CONFIG_PCI_SCAN_SHOW */
