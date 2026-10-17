// SPDX-License-Identifier: GPL-2.0+
/*
 * SH7751 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/* Register addresses and such */
#define SH7751_BCR1	(vu_long *)0xFF800000
#define SH7751_BCR2	(vu_short *)0xFF800004
#define SH7751_WCR1	(vu_long *)0xFF800008
#define SH7751_WCR2	(vu_long *)0xFF80000C
#define SH7751_WCR3	(vu_long *)0xFF800010
#define SH7751_MCR	(vu_long *)0xFF800014
#define SH7751_BCR3	(vu_short *)0xFF800050
#define SH7751_PCICONF0	(vu_long *)0xFE200000
#define SH7751_PCICONF1	(vu_long *)0xFE200004
#define SH7751_PCICONF2	(vu_long *)0xFE200008
#define SH7751_PCICONF3	(vu_long *)0xFE20000C
#define SH7751_PCICONF4	(vu_long *)0xFE200010
#define SH7751_PCICONF5	(vu_long *)0xFE200014
#define SH7751_PCICONF6	(vu_long *)0xFE200018
#define SH7751_PCICR	(vu_long *)0xFE200100
#define SH7751_PCILSR0	(vu_long *)0xFE200104
#define SH7751_PCILSR1	(vu_long *)0xFE200108
#define SH7751_PCILAR0	(vu_long *)0xFE20010C
#define SH7751_PCILAR1	(vu_long *)0xFE200110
#define SH7751_PCIMBR	(vu_long *)0xFE2001C4
#define SH7751_PCIIOBR	(vu_long *)0xFE2001C8
#define SH7751_PCIPINT	(vu_long *)0xFE2001CC
#define SH7751_PCIPINTM	(vu_long *)0xFE2001D0
#define SH7751_PCICLKR	(vu_long *)0xFE2001D4
#define SH7751_PCIBCR1	(vu_long *)0xFE2001E0
#define SH7751_PCIBCR2	(vu_long *)0xFE2001E4
#define SH7751_PCIWCR1	(vu_long *)0xFE2001E8
#define SH7751_PCIWCR2	(vu_long *)0xFE2001EC
#define SH7751_PCIWCR3	(vu_long *)0xFE2001F0
#define SH7751_PCIMCR	(vu_long *)0xFE2001F4
#define SH7751_PCIBCR3	(vu_long *)0xFE2001F8

#define BCR1_BREQEN		0x00080000
#define PCI_SH7751_ID		0x35051054
#define PCI_SH7751R_ID		0x350E1054
#define SH7751_PCICONF1_WCC	0x00000080
#define SH7751_PCICONF1_PER	0x00000040
#define SH7751_PCICONF1_BUM	0x00000004
#define SH7751_PCICONF1_MES	0x00000002
#define SH7751_PCICONF1_CMDS	0x000000C6
#define SH7751_PCI_HOST_BRIDGE	0x6
#define SH7751_PCICR_PREFIX	0xa5000000
#define SH7751_PCICR_PRST	0x00000002
#define SH7751_PCICR_CFIN	0x00000001
#define SH7751_PCIPINT_D3	0x00000002
#define SH7751_PCIPINT_D0	0x00000001
#define SH7751_PCICLKR_PREFIX	0xa5000000

#define SH7751_PCI_MEM_BASE	0xFD000000
#define SH7751_PCI_MEM_SIZE	0x01000000
#define SH7751_PCI_IO_BASE	0xFE240000
#define SH7751_PCI_IO_SIZE	0x00040000

#define SH7751_PCIPAR	(vu_long *)0xFE2001C0
#define SH7751_PCIPDR	(vu_long *)0xFE200220

#define p4_in(addr)	(*addr)
#define p4_out(data, addr) (*addr) = (data)

static int sh7751_pci_read_config(const struct udevice *dev, pci_dev_t bdf,
				  uint offset, ulong *value,
				  enum pci_size_t size)
{
	u32 addr, reg;

	addr = PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	p4_out(addr, SH7751_PCIPAR);
	reg = p4_in(SH7751_PCIPDR);
	*value = pci_conv_32_to_size(reg, offset, size);

	return 0;
}

static int sh7751_pci_write_config(struct udevice *dev, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	u32 addr, reg, old;

	addr = PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	p4_out(addr, SH7751_PCIPAR);
	old = p4_in(SH7751_PCIPDR);
	reg = pci_conv_size_to_32(old, value, offset, size);
	p4_out(reg, SH7751_PCIPDR);

	return 0;
}

static int sh7751_pci_probe(struct udevice *dev)
{
	/* Double-check that we're a 7751 or 7751R chip */
	if (p4_in(SH7751_PCICONF0) != PCI_SH7751_ID
	    && p4_in(SH7751_PCICONF0) != PCI_SH7751R_ID) {
		printf("PCI: Unknown PCI host bridge.\n");
		return 1;
	}
	printf("PCI: SH7751 PCI host bridge found.\n");

	/* Double-check some BSC config settings */
	/* (Area 3 non-MPX 32-bit, PCI bus pins) */
	if ((p4_in(SH7751_BCR1) & 0x20008) == 0x20000) {
		printf("SH7751_BCR1 value is wrong(0x%08X)\n",
			(unsigned int)p4_in(SH7751_BCR1));
		return 2;
	}
	if ((p4_in(SH7751_BCR2) & 0xC0) != 0xC0) {
		printf("SH7751_BCR2 value is wrong(0x%08X)\n",
			(unsigned int)p4_in(SH7751_BCR2));
		return 3;
	}
	if (p4_in(SH7751_BCR2) & 0x01) {
		printf("SH7751_BCR2 value is wrong(0x%08X)\n",
			(unsigned int)p4_in(SH7751_BCR2));
		return 4;
	}

	/* Force BREQEN in BCR1 to allow PCIC access */
	p4_out((p4_in(SH7751_BCR1) | BCR1_BREQEN), SH7751_BCR1);

	/* Toggle PCI reset pin */
	p4_out((SH7751_PCICR_PREFIX | SH7751_PCICR_PRST), SH7751_PCICR);
	udelay(32);
	p4_out(SH7751_PCICR_PREFIX, SH7751_PCICR);

	/* Set cmd bits: WCC, PER, BUM, MES */
	/* (Addr/Data stepping, Parity enabled, Bus Master, Memory enabled) */
	p4_out(0xfb900047, SH7751_PCICONF1);	/* K.Kino */

	/* Define this host as the host bridge */
	p4_out((SH7751_PCI_HOST_BRIDGE << 24), SH7751_PCICONF2);

	/* Force PCI clock(s) on */
	p4_out(0, SH7751_PCICLKR);
	p4_out(0x03, SH7751_PCICLKR);

	/* Clear powerdown IRQs, also mask them (unused) */
	p4_out((SH7751_PCIPINT_D0 | SH7751_PCIPINT_D3), SH7751_PCIPINT);
	p4_out(0, SH7751_PCIPINTM);

	p4_out(0xab000001, SH7751_PCICONF4);

	/* Set up target memory mappings (for external DMA access) */
	/* Map both P0 and P2 range to Area 3 RAM for ease of use */
	p4_out(CFG_SYS_SDRAM_SIZE - 0x100000, SH7751_PCILSR0);
	p4_out(CFG_SYS_SDRAM_BASE & 0x1FF00000, SH7751_PCILAR0);
	p4_out(CFG_SYS_SDRAM_BASE & 0xFFF00000, SH7751_PCICONF5);

	p4_out(0, SH7751_PCILSR1);
	p4_out(0, SH7751_PCILAR1);
	p4_out(0xd0000000, SH7751_PCICONF6);

	/* Map memory window to same address on PCI bus */
	p4_out(SH7751_PCI_MEM_BASE, SH7751_PCIMBR);

	/* Map IO window to same address on PCI bus */
	p4_out(SH7751_PCI_IO_BASE, SH7751_PCIIOBR);

	/* set BREQEN */
	p4_out(inl(SH7751_BCR1) | 0x00080000, SH7751_BCR1);

	/* Copy BSC registers into PCI BSC */
	p4_out(inl(SH7751_BCR1), SH7751_PCIBCR1);
	p4_out(inw(SH7751_BCR2), SH7751_PCIBCR2);
	p4_out(inw(SH7751_BCR3), SH7751_PCIBCR3);
	p4_out(inl(SH7751_WCR1), SH7751_PCIWCR1);
	p4_out(inl(SH7751_WCR2), SH7751_PCIWCR2);
	p4_out(inl(SH7751_WCR3), SH7751_PCIWCR3);
	p4_out(inl(SH7751_MCR), SH7751_PCIMCR);

	/* Finally, set central function init complete */
	p4_out((SH7751_PCICR_PREFIX | SH7751_PCICR_CFIN), SH7751_PCICR);

	return 0;
}

static const struct dm_pci_ops sh7751_pci_ops = {
	.read_config	= sh7751_pci_read_config,
	.write_config	= sh7751_pci_write_config,
};

static const struct udevice_id sh7751_pci_ids[] = {
	{ .compatible = "renesas,pci-sh7751" },
	{ }
};

U_BOOT_DRIVER(sh7751_pci) = {
	.name		= "sh7751_pci",
	.id		= UCLASS_PCI,
	.of_match	= sh7751_pci_ids,
	.ops		= &sh7751_pci_ops,
	.probe		= sh7751_pci_probe,
};
