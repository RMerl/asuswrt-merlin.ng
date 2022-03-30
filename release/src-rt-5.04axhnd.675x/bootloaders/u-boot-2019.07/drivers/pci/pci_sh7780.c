// SPDX-License-Identifier: GPL-2.0+
/*
 * SH7780 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 */

#include <common.h>

#include <pci.h>
#include <asm/processor.h>
#include <asm/pci.h>
#include <asm/io.h>

#define SH7780_VENDOR_ID	0x1912
#define SH7780_DEVICE_ID	0x0002
#define SH7780_PCICR_PREFIX	0xA5000000
#define SH7780_PCICR_PFCS	0x00000800
#define SH7780_PCICR_FTO	0x00000400
#define SH7780_PCICR_PFE	0x00000200
#define SH7780_PCICR_TBS	0x00000100
#define SH7780_PCICR_ARBM	0x00000040
#define SH7780_PCICR_IOCS	0x00000004
#define SH7780_PCICR_PRST	0x00000002
#define SH7780_PCICR_CFIN	0x00000001

#define p4_in(addr)			(*(vu_long *)addr)
#define p4_out(data, addr)	(*(vu_long *)addr) = (data)
#define p4_inw(addr)		(*(vu_short *)addr)
#define p4_outw(data, addr)	(*(vu_short *)addr) = (data)

int pci_sh4_read_config_dword(struct pci_controller *hose,
				    pci_dev_t dev, int offset, u32 *value)
{
	u32 par_data = 0x80000000 | dev;

	p4_out(par_data | (offset & 0xfc), SH7780_PCIPAR);
	*value = p4_in(SH7780_PCIPDR);

	return 0;
}

int pci_sh4_write_config_dword(struct pci_controller *hose,
				     pci_dev_t dev, int offset, u32 value)
{
	u32 par_data = 0x80000000 | dev;

	p4_out(par_data | (offset & 0xfc), SH7780_PCIPAR);
	p4_out(value, SH7780_PCIPDR);
	return 0;
}

int pci_sh7780_init(struct pci_controller *hose)
{
	p4_out(0x01, SH7780_PCIECR);

	if (p4_inw(SH7780_PCIVID) != SH7780_VENDOR_ID
	    && p4_inw(SH7780_PCIDID) != SH7780_DEVICE_ID) {
		printf("PCI: Unknown PCI host bridge.\n");
		return -1;
	}
	printf("PCI: SH7780 PCI host bridge found.\n");

	/* Toggle PCI reset pin */
	p4_out((SH7780_PCICR_PREFIX | SH7780_PCICR_PRST), SH7780_PCICR);
	udelay(100000);
	p4_out(SH7780_PCICR_PREFIX, SH7780_PCICR);
	p4_outw(0x0047, SH7780_PCICMD);

	p4_out(CONFIG_SH7780_PCI_LSR, SH7780_PCILSR0);
	p4_out(CONFIG_SH7780_PCI_LAR, SH7780_PCILAR0);
	p4_out(0x00000000, SH7780_PCILSR1);
	p4_out(0, SH7780_PCILAR1);
	p4_out(CONFIG_SH7780_PCI_BAR, SH7780_PCIMBAR0);
	p4_out(0x00000000, SH7780_PCIMBAR1);

	p4_out(0xFD000000, SH7780_PCIMBR0);
	p4_out(0x00FC0000, SH7780_PCIMBMR0);

	/* if use Operand Cache then enable PCICSCR Soonp bits. */
	p4_out(0x08000000, SH7780_PCICSAR0);
	p4_out(0x0000001B, SH7780_PCICSCR0);	/* Snoop bit :On */

	p4_out((SH7780_PCICR_PREFIX | SH7780_PCICR_CFIN | SH7780_PCICR_ARBM
	      | SH7780_PCICR_FTO | SH7780_PCICR_PFCS | SH7780_PCICR_PFE),
	     SH7780_PCICR);

	pci_sh4_init(hose);
	return 0;
}
