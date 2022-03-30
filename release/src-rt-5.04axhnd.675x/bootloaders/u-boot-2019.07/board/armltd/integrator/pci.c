// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * (C) Copyright 2011
 * Linaro
 * Linus Walleij <linus.walleij@linaro.org>
 */
#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include "integrator-sc.h"
#include "pci_v3.h"

#define INTEGRATOR_BOOT_ROM_BASE	0x20000000
#define INTEGRATOR_HDR0_SDRAM_BASE	0x80000000

/*
 * These are in the physical addresses on the CPU side, i.e.
 * where we read and write stuff - you don't want to try to
 * move these around
 */
#define PHYS_PCI_MEM_BASE	0x40000000
#define PHYS_PCI_IO_BASE	0x60000000	/* PCI I/O space base */
#define PHYS_PCI_CONFIG_BASE	0x61000000
#define PHYS_PCI_V3_BASE	0x62000000	/* V360EPC registers */
#define SZ_256M			0x10000000

/*
 * These are in the PCI BUS address space
 * Set to 0x00000000 in the Linux kernel, 0x40000000 in Boot monitor
 * we follow the example of the kernel, because that is the address
 * range that devices actually use - what would they be doing at
 * 0x40000000?
 */
#define PCI_BUS_NONMEM_START	0x00000000
#define PCI_BUS_NONMEM_SIZE	SZ_256M

#define PCI_BUS_PREMEM_START	(PCI_BUS_NONMEM_START + PCI_BUS_NONMEM_SIZE)
#define PCI_BUS_PREMEM_SIZE	SZ_256M

#if PCI_BUS_NONMEM_START & 0x000fffff
#error PCI_BUS_NONMEM_START must be megabyte aligned
#endif
#if PCI_BUS_PREMEM_START & 0x000fffff
#error PCI_BUS_PREMEM_START must be megabyte aligned
#endif

/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
#define PCI_ENET0_IOADDR	0x60000000 /* First card in PCI I/O space */
#define PCI_ENET0_MEMADDR	0x40000000 /* First card in PCI memory space */
static struct pci_config_table pci_integrator_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0f, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ }
};
#endif /* CONFIG_PCI_PNP */

/* V3 access routines */
#define v3_writeb(o, v) __raw_writeb(v, PHYS_PCI_V3_BASE + (unsigned int)(o))
#define v3_readb(o)    (__raw_readb(PHYS_PCI_V3_BASE + (unsigned int)(o)))

#define v3_writew(o, v) __raw_writew(v, PHYS_PCI_V3_BASE + (unsigned int)(o))
#define v3_readw(o)    (__raw_readw(PHYS_PCI_V3_BASE + (unsigned int)(o)))

#define v3_writel(o, v) __raw_writel(v, PHYS_PCI_V3_BASE + (unsigned int)(o))
#define v3_readl(o)    (__raw_readl(PHYS_PCI_V3_BASE + (unsigned int)(o)))

static unsigned long v3_open_config_window(pci_dev_t bdf, int offset)
{
	unsigned int address, mapaddress;
	unsigned int busnr = PCI_BUS(bdf);
	unsigned int devfn = PCI_FUNC(bdf);

	/*
	 * Trap out illegal values
	 */
	if (offset > 255)
		BUG();
	if (busnr > 255)
		BUG();
	if (devfn > 255)
		BUG();

	if (busnr == 0) {
		/*
		 * Linux calls the thing U-Boot calls "DEV" "SLOT"
		 * instead, but it's the same 5 bits
		 */
		int slot = PCI_DEV(bdf);

		/*
		 * local bus segment so need a type 0 config cycle
		 *
		 * build the PCI configuration "address" with one-hot in
		 * A31-A11
		 *
		 * mapaddress:
		 *  3:1 = config cycle (101)
		 *  0   = PCI A1 & A0 are 0 (0)
		 */
		address = PCI_FUNC(bdf) << 8;
		mapaddress = V3_LB_MAP_TYPE_CONFIG;

		if (slot > 12)
			/*
			 * high order bits are handled by the MAP register
			 */
			mapaddress |= 1 << (slot - 5);
		else
			/*
			 * low order bits handled directly in the address
			 */
			address |= 1 << (slot + 11);
	} else {
		/*
		 * not the local bus segment so need a type 1 config cycle
		 *
		 * address:
		 *  23:16 = bus number
		 *  15:11 = slot number (7:3 of devfn)
		 *  10:8  = func number (2:0 of devfn)
		 *
		 * mapaddress:
		 *  3:1 = config cycle (101)
		 *  0   = PCI A1 & A0 from host bus (1)
		 */
		mapaddress = V3_LB_MAP_TYPE_CONFIG | V3_LB_MAP_AD_LOW_EN;
		address = (busnr << 16) | (devfn << 8);
	}

	/*
	 * Set up base0 to see all 512Mbytes of memory space (not
	 * prefetchable), this frees up base1 for re-use by
	 * configuration memory
	 */
	v3_writel(V3_LB_BASE0, v3_addr_to_lb_base(PHYS_PCI_MEM_BASE) |
			V3_LB_BASE_ADR_SIZE_512MB | V3_LB_BASE_ENABLE);

	/*
	 * Set up base1/map1 to point into configuration space.
	 */
	v3_writel(V3_LB_BASE1, v3_addr_to_lb_base(PHYS_PCI_CONFIG_BASE) |
			V3_LB_BASE_ADR_SIZE_16MB | V3_LB_BASE_ENABLE);
	v3_writew(V3_LB_MAP1, mapaddress);

	return PHYS_PCI_CONFIG_BASE + address + offset;
}

static void v3_close_config_window(void)
{
	/*
	 * Reassign base1 for use by prefetchable PCI memory
	 */
	v3_writel(V3_LB_BASE1, v3_addr_to_lb_base(PHYS_PCI_MEM_BASE + SZ_256M) |
			V3_LB_BASE_ADR_SIZE_256MB | V3_LB_BASE_PREFETCH |
			V3_LB_BASE_ENABLE);
	v3_writew(V3_LB_MAP1, v3_addr_to_lb_map(PCI_BUS_PREMEM_START) |
			V3_LB_MAP_TYPE_MEM_MULTIPLE);

	/*
	 * And shrink base0 back to a 256M window (NOTE: MAP0 already correct)
	 */
	v3_writel(V3_LB_BASE0, v3_addr_to_lb_base(PHYS_PCI_MEM_BASE) |
			V3_LB_BASE_ADR_SIZE_256MB | V3_LB_BASE_ENABLE);
}

static int pci_integrator_read_byte(struct pci_controller *hose, pci_dev_t bdf,
				    int offset, unsigned char *val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	*val = __raw_readb(addr);
	v3_close_config_window();
	return 0;
}

static int pci_integrator_read__word(struct pci_controller *hose,
				     pci_dev_t bdf, int offset,
				     unsigned short *val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	*val = __raw_readw(addr);
	v3_close_config_window();
	return 0;
}

static int pci_integrator_read_dword(struct pci_controller *hose,
				     pci_dev_t bdf, int offset,
				     unsigned int *val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	*val = __raw_readl(addr);
	v3_close_config_window();
	return 0;
}

static int pci_integrator_write_byte(struct pci_controller *hose,
				     pci_dev_t bdf, int offset,
				     unsigned char val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	__raw_writeb((u8)val, addr);
	__raw_readb(addr);
	v3_close_config_window();
	return 0;
}

static int pci_integrator_write_word(struct pci_controller *hose,
				     pci_dev_t bdf, int offset,
				     unsigned short val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	__raw_writew((u8)val, addr);
	__raw_readw(addr);
	v3_close_config_window();
	return 0;
}

static int pci_integrator_write_dword(struct pci_controller *hose,
				      pci_dev_t bdf, int offset,
				      unsigned int val)
{
	unsigned long addr;

	addr = v3_open_config_window(bdf, offset);
	__raw_writel((u8)val, addr);
	__raw_readl(addr);
	v3_close_config_window();
	return 0;
}

struct pci_controller integrator_hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_integrator_config_table,
#endif
};

void pci_init_board(void)
{
	struct pci_controller *hose = &integrator_hose;
	u16 val;

	/* setting this register will take the V3 out of reset */
	__raw_writel(SC_PCI_PCIEN, SC_PCI);

	/* Wait for 230 ms (from spec) before accessing any V3 registers */
	mdelay(230);

	/* Now write the Base I/O Address Word to PHYS_PCI_V3_BASE + 0x6E */
	v3_writew(V3_LB_IO_BASE, (PHYS_PCI_V3_BASE >> 16));

	/* Wait for the mailbox to settle */
	do {
		v3_writeb(V3_MAIL_DATA, 0xAA);
		v3_writeb(V3_MAIL_DATA + 4, 0x55);
	} while (v3_readb(V3_MAIL_DATA) != 0xAA ||
		 v3_readb(V3_MAIL_DATA + 4) != 0x55);

	/* Make sure that V3 register access is not locked, if it is, unlock it */
	if (v3_readw(V3_SYSTEM) & V3_SYSTEM_M_LOCK)
		v3_writew(V3_SYSTEM, 0xA05F);

	/*
	 * Ensure that the slave accesses from PCI are disabled while we
	 * setup memory windows
	 */
	val = v3_readw(V3_PCI_CMD);
	val &= ~(V3_COMMAND_M_MEM_EN | V3_COMMAND_M_IO_EN);
	v3_writew(V3_PCI_CMD, val);

	/* Clear RST_OUT to 0; keep the PCI bus in reset until we've finished */
	val = v3_readw(V3_SYSTEM);
	val &= ~V3_SYSTEM_M_RST_OUT;
	v3_writew(V3_SYSTEM, val);

	/* Make all accesses from PCI space retry until we're ready for them */
	val = v3_readw(V3_PCI_CFG);
	val |= V3_PCI_CFG_M_RETRY_EN;
	v3_writew(V3_PCI_CFG, val);

	/*
	 * Set up any V3 PCI Configuration Registers that we absolutely have to.
	 * LB_CFG controls Local Bus protocol.
	 * Enable LocalBus byte strobes for READ accesses too.
	 * set bit 7 BE_IMODE and bit 6 BE_OMODE
	 */
	val = v3_readw(V3_LB_CFG);
	val |= 0x0C0;
	v3_writew(V3_LB_CFG, val);

	/* PCI_CMD controls overall PCI operation. Enable PCI bus master. */
	val = v3_readw(V3_PCI_CMD);
	val |= V3_COMMAND_M_MASTER_EN;
	v3_writew(V3_PCI_CMD, val);

	/*
	 * PCI_MAP0 controls where the PCI to CPU memory window is on
	 * Local Bus
	 */
	v3_writel(V3_PCI_MAP0,
		  (INTEGRATOR_BOOT_ROM_BASE) | (V3_PCI_MAP_M_ADR_SIZE_512MB |
						V3_PCI_MAP_M_REG_EN |
						V3_PCI_MAP_M_ENABLE));

	/* PCI_BASE0 is the PCI address of the start of the window */
	v3_writel(V3_PCI_BASE0, INTEGRATOR_BOOT_ROM_BASE);

	/* PCI_MAP1 is LOCAL address of the start of the window */
	v3_writel(V3_PCI_MAP1,
		  (INTEGRATOR_HDR0_SDRAM_BASE) | (V3_PCI_MAP_M_ADR_SIZE_1GB |
						  V3_PCI_MAP_M_REG_EN |
						  V3_PCI_MAP_M_ENABLE));

	/* PCI_BASE1 is the PCI address of the start of the window */
	v3_writel(V3_PCI_BASE1, INTEGRATOR_HDR0_SDRAM_BASE);

	/*
	 * Set up memory the windows from local bus memory into PCI
	 * configuration, I/O and Memory regions.
	 * PCI I/O, LB_BASE2 and LB_MAP2 are used exclusively for this.
	 */
	v3_writew(V3_LB_BASE2,
		  v3_addr_to_lb_map(PHYS_PCI_IO_BASE) | V3_LB_BASE_ENABLE);
	v3_writew(V3_LB_MAP2, 0);

	/* PCI Configuration, use LB_BASE1/LB_MAP1. */

	/*
	 * PCI Memory use LB_BASE0/LB_MAP0 and LB_BASE1/LB_MAP1
	 * Map first 256Mbytes as non-prefetchable via BASE0/MAP0
	 */
	v3_writel(V3_LB_BASE0, v3_addr_to_lb_base(PHYS_PCI_MEM_BASE) |
			V3_LB_BASE_ADR_SIZE_256MB | V3_LB_BASE_ENABLE);
	v3_writew(V3_LB_MAP0,
		  v3_addr_to_lb_map(PCI_BUS_NONMEM_START) | V3_LB_MAP_TYPE_MEM);

	/* Map second 256 Mbytes as prefetchable via BASE1/MAP1 */
	v3_writel(V3_LB_BASE1, v3_addr_to_lb_base(PHYS_PCI_MEM_BASE + SZ_256M) |
			V3_LB_BASE_ADR_SIZE_256MB | V3_LB_BASE_PREFETCH |
			V3_LB_BASE_ENABLE);
	v3_writew(V3_LB_MAP1, v3_addr_to_lb_map(PCI_BUS_PREMEM_START) |
			V3_LB_MAP_TYPE_MEM_MULTIPLE);

	/* Dump PCI to local address space mappings */
	debug("LB_BASE0 = %08x\n", v3_readl(V3_LB_BASE0));
	debug("LB_MAP0 = %04x\n", v3_readw(V3_LB_MAP0));
	debug("LB_BASE1 = %08x\n", v3_readl(V3_LB_BASE1));
	debug("LB_MAP1 = %04x\n", v3_readw(V3_LB_MAP1));
	debug("LB_BASE2 = %04x\n", v3_readw(V3_LB_BASE2));
	debug("LB_MAP2 = %04x\n", v3_readw(V3_LB_MAP2));
	debug("LB_IO_BASE = %04x\n", v3_readw(V3_LB_IO_BASE));

	/*
	 * Allow accesses to PCI Configuration space and set up A1, A0 for
	 * type 1 config cycles
	 */
	val = v3_readw(V3_PCI_CFG);
	val &= ~(V3_PCI_CFG_M_RETRY_EN | V3_PCI_CFG_M_AD_LOW1);
	val |= V3_PCI_CFG_M_AD_LOW0;
	v3_writew(V3_PCI_CFG, val);

	/* now we can allow incoming PCI MEMORY accesses */
	val = v3_readw(V3_PCI_CMD);
	val |= V3_COMMAND_M_MEM_EN;
	v3_writew(V3_PCI_CMD, val);

	/*
	 * Set RST_OUT to take the PCI bus is out of reset, PCI devices can
	 * now initialise.
	 */
	val = v3_readw(V3_SYSTEM);
	val |= V3_SYSTEM_M_RST_OUT;
	v3_writew(V3_SYSTEM, val);

	/*  Lock the V3 system register so that no one else can play with it */
	val = v3_readw(V3_SYSTEM);
	val |= V3_SYSTEM_M_LOCK;
	v3_writew(V3_SYSTEM, val);

	/*
	 * Configure and register the PCI hose
	 */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space, window 0 256 MB non-prefetchable */
	pci_set_region(hose->regions + 0,
		       PCI_BUS_NONMEM_START, PHYS_PCI_MEM_BASE,
		       SZ_256M,
		       PCI_REGION_MEM);

	/* System memory space, window 1 256 MB prefetchable */
	pci_set_region(hose->regions + 1,
		       PCI_BUS_PREMEM_START, PHYS_PCI_MEM_BASE + SZ_256M,
		       SZ_256M,
		       PCI_REGION_MEM |
		       PCI_REGION_PREFETCH);

	/* PCI I/O space */
	pci_set_region(hose->regions + 2,
		       0x00000000, PHYS_PCI_IO_BASE, 0x01000000,
		       PCI_REGION_IO);

	/* PCI Memory - config space */
	pci_set_region(hose->regions + 3,
		       0x00000000, PHYS_PCI_CONFIG_BASE, 0x01000000,
		       PCI_REGION_MEM);
	/* PCI V3 regs */
	pci_set_region(hose->regions + 4,
		       0x00000000, PHYS_PCI_V3_BASE, 0x01000000,
		       PCI_REGION_MEM);

	hose->region_count = 5;

	pci_set_ops(hose,
		    pci_integrator_read_byte,
		    pci_integrator_read__word,
		    pci_integrator_read_dword,
		    pci_integrator_write_byte,
		    pci_integrator_write_word,
		    pci_integrator_write_dword);

	pci_register_hose(hose);

	pciauto_config_init(hose);
	pciauto_config_device(hose, 0);

	hose->last_busno = pci_hose_scan(hose);
}
