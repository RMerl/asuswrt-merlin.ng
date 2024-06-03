// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ARM Ltd 2015
 *
 * Author: Liviu Dudau <Liviu.Dudau@arm.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <pci_ids.h>
#include "pcie.h"

/* XpressRICH3 support */
#define XR3_CONFIG_BASE			0x7ff30000
#define XR3_RESET_BASE			0x7ff20000

#define XR3_PCI_ECAM_START		0x40000000
#define XR3_PCI_ECAM_SIZE		28	/* as power of 2 = 0x10000000 */
#define XR3_PCI_IOSPACE_START		0x5f800000
#define XR3_PCI_IOSPACE_SIZE		23	/* as power of 2 = 0x800000 */
#define XR3_PCI_MEMSPACE_START		0x50000000
#define XR3_PCI_MEMSPACE_SIZE		27	/* as power of 2 = 0x8000000 */
#define XR3_PCI_MEMSPACE64_START	0x4000000000
#define XR3_PCI_MEMSPACE64_SIZE		33	/* as power of 2 = 0x200000000 */

#define JUNO_V2M_MSI_START		0x2c1c0000
#define JUNO_V2M_MSI_SIZE		12	/* as power of 2 = 4096 */

#define XR3PCI_BASIC_STATUS		0x18
#define XR3PCI_BS_GEN_MASK		(0xf << 8)
#define XR3PCI_BS_LINK_MASK		0xff

#define XR3PCI_VIRTCHAN_CREDITS		0x90
#define XR3PCI_BRIDGE_PCI_IDS		0x9c
#define XR3PCI_PEX_SPC2			0xd8

#define XR3PCI_ATR_PCIE_WIN0		0x600
#define XR3PCI_ATR_PCIE_WIN1		0x700
#define XR3PCI_ATR_AXI4_SLV0		0x800

#define XR3PCI_ATR_TABLE_SIZE		0x20
#define XR3PCI_ATR_SRC_ADDR_LOW		0x0
#define XR3PCI_ATR_SRC_ADDR_HIGH	0x4
#define XR3PCI_ATR_TRSL_ADDR_LOW	0x8
#define XR3PCI_ATR_TRSL_ADDR_HIGH	0xc
#define XR3PCI_ATR_TRSL_PARAM		0x10

/* IDs used in the XR3PCI_ATR_TRSL_PARAM */
#define XR3PCI_ATR_TRSLID_AXIDEVICE	(0x420004)
#define XR3PCI_ATR_TRSLID_AXIMEMORY	(0x4e0004)  /* Write-through, read/write allocate */
#define XR3PCI_ATR_TRSLID_PCIE_CONF	(0x000001)
#define XR3PCI_ATR_TRSLID_PCIE_IO	(0x020000)
#define XR3PCI_ATR_TRSLID_PCIE_MEMORY	(0x000000)

#define XR3PCI_ECAM_OFFSET(b, d, o)	(((b) << 20) | \
					(PCI_SLOT(d) << 15) | \
					(PCI_FUNC(d) << 12) | o)

#define JUNO_RESET_CTRL			0x1004
#define JUNO_RESET_CTRL_PHY		BIT(0)
#define JUNO_RESET_CTRL_RC		BIT(1)

#define JUNO_RESET_STATUS		0x1008
#define JUNO_RESET_STATUS_PLL		BIT(0)
#define JUNO_RESET_STATUS_PHY		BIT(1)
#define JUNO_RESET_STATUS_RC		BIT(2)
#define JUNO_RESET_STATUS_MASK		(JUNO_RESET_STATUS_PLL | \
					 JUNO_RESET_STATUS_PHY | \
					 JUNO_RESET_STATUS_RC)

void xr3pci_set_atr_entry(unsigned long base, unsigned long src_addr,
			unsigned long trsl_addr, int window_size,
			int trsl_param)
{
	/* X3PCI_ATR_SRC_ADDR_LOW:
	     - bit 0: enable entry,
	     - bits 1-6: ATR window size: total size in bytes: 2^(ATR_WSIZE + 1)
	     - bits 7-11: reserved
	     - bits 12-31: start of source address
	*/
	writel((u32)(src_addr & 0xfffff000) | (window_size - 1) << 1 | 1,
	       base + XR3PCI_ATR_SRC_ADDR_LOW);
	writel((u32)(src_addr >> 32), base + XR3PCI_ATR_SRC_ADDR_HIGH);
	writel((u32)(trsl_addr & 0xfffff000), base + XR3PCI_ATR_TRSL_ADDR_LOW);
	writel((u32)(trsl_addr >> 32), base + XR3PCI_ATR_TRSL_ADDR_HIGH);
	writel(trsl_param, base + XR3PCI_ATR_TRSL_PARAM);

	debug("ATR entry: 0x%010lx %s 0x%010lx [0x%010llx] (param: 0x%06x)\n",
	       src_addr, (trsl_param & 0x400000) ? "<-" : "->", trsl_addr,
	       ((u64)1) << window_size, trsl_param);
}

void xr3pci_setup_atr(void)
{
	/* setup PCIe to CPU address translation tables */
	unsigned long base = XR3_CONFIG_BASE + XR3PCI_ATR_PCIE_WIN0;

	/* forward all writes from PCIe to GIC V2M (used for MSI) */
	xr3pci_set_atr_entry(base, JUNO_V2M_MSI_START, JUNO_V2M_MSI_START,
			     JUNO_V2M_MSI_SIZE, XR3PCI_ATR_TRSLID_AXIDEVICE);

	base += XR3PCI_ATR_TABLE_SIZE;

	/* PCIe devices can write anywhere in memory */
	xr3pci_set_atr_entry(base, PHYS_SDRAM_1, PHYS_SDRAM_1,
			     31 /* grant access to all RAM under 4GB */,
			     XR3PCI_ATR_TRSLID_AXIMEMORY);
	base += XR3PCI_ATR_TABLE_SIZE;
	xr3pci_set_atr_entry(base, PHYS_SDRAM_2, PHYS_SDRAM_2,
			     XR3_PCI_MEMSPACE64_SIZE,
			     XR3PCI_ATR_TRSLID_AXIMEMORY);


	/* setup CPU to PCIe address translation table */
	base = XR3_CONFIG_BASE + XR3PCI_ATR_AXI4_SLV0;

	/* setup ECAM space to bus configuration interface */
	xr3pci_set_atr_entry(base, XR3_PCI_ECAM_START, 0, XR3_PCI_ECAM_SIZE,
			     XR3PCI_ATR_TRSLID_PCIE_CONF);

	base += XR3PCI_ATR_TABLE_SIZE;

	/* setup IO space translation */
	xr3pci_set_atr_entry(base, XR3_PCI_IOSPACE_START, 0,
			     XR3_PCI_IOSPACE_SIZE, XR3PCI_ATR_TRSLID_PCIE_IO);

	base += XR3PCI_ATR_TABLE_SIZE;

	/* setup 32bit MEM space translation */
	xr3pci_set_atr_entry(base, XR3_PCI_MEMSPACE_START, XR3_PCI_MEMSPACE_START,
			     XR3_PCI_MEMSPACE_SIZE, XR3PCI_ATR_TRSLID_PCIE_MEMORY);

	base += XR3PCI_ATR_TABLE_SIZE;

	/* setup 64bit MEM space translation */
	xr3pci_set_atr_entry(base, XR3_PCI_MEMSPACE64_START, XR3_PCI_MEMSPACE64_START,
			     XR3_PCI_MEMSPACE64_SIZE, XR3PCI_ATR_TRSLID_PCIE_MEMORY);
}

void xr3pci_init(void)
{
	u32 val;
	int timeout = 200;

	/* Initialise the XpressRICH3 PCIe host bridge */

	/* add credits */
	writel(0x00f0b818, XR3_CONFIG_BASE + XR3PCI_VIRTCHAN_CREDITS);
	writel(0x1, XR3_CONFIG_BASE + XR3PCI_VIRTCHAN_CREDITS + 4);
	/* allow ECRC */
	writel(0x6006, XR3_CONFIG_BASE + XR3PCI_PEX_SPC2);
	/* setup the correct class code for the host bridge */
	writel(PCI_CLASS_BRIDGE_PCI << 16, XR3_CONFIG_BASE + XR3PCI_BRIDGE_PCI_IDS);

	/* reset phy and root complex */
	writel(JUNO_RESET_CTRL_PHY | JUNO_RESET_CTRL_RC,
	       XR3_RESET_BASE + JUNO_RESET_CTRL);

	do {
		mdelay(1);
		val = readl(XR3_RESET_BASE + JUNO_RESET_STATUS);
	} while (--timeout &&
		(val & JUNO_RESET_STATUS_MASK) != JUNO_RESET_STATUS_MASK);

	if (!timeout) {
		printf("PCI XR3 Root complex reset timed out\n");
		return;
	}

	/* Wait for the link to train */
	mdelay(20);
	timeout = 20;

	do {
		mdelay(1);
		val = readl(XR3_CONFIG_BASE + XR3PCI_BASIC_STATUS);
	} while (--timeout && !(val & XR3PCI_BS_LINK_MASK));

	if (!(val & XR3PCI_BS_LINK_MASK)) {
		printf("Failed to negotiate a link!\n");
		return;
	}

	printf("PCIe XR3 Host Bridge enabled: x%d link (Gen %d)\n",
	       val & XR3PCI_BS_LINK_MASK, (val & XR3PCI_BS_GEN_MASK) >> 8);

	xr3pci_setup_atr();
}

void vexpress64_pcie_init(void)
{
	xr3pci_init();
}
