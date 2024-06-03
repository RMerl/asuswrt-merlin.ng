/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 * This file contains work-arounds for many known Broadcom PCI hardware bugs
 * or limitations
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include "../drivers/pci/pci.h"

/*
 * Resource Number for the endpoint bar#
 *
 * BAR0            BAR1            BAR2
 * res[0], res[1], res[2], res[3], res[4], res[5]
 */
#define RES_BAR0              0         /* BAR0 resource# */
#define RES_BAR1              2         /* BAR1 resource# */
#define RES_BAR2              4         /* BAR2 resource# */

/*
 * EP BP PCIE Core Registers through BAR0 space
 */
#define BAR0_PCIECOREBASE     0x2000
#define CONFIGADDR_OFFSET     0x0120
#define CONFIGDATA_OFFSET     0x0124

/*
 * EP registers accessed through BAR0 space
 */
#define REG_BAR3_CFG          0x4F4     /* EP_PRIV1_BAR3_CFG */

/*
 * BAR_CFG size defines bit[3:0]
 */
#define BAR_CFG_SZ_MASK       0xF       /* Mask */
#define BAR_CFG_SZ_8MB        0x8       /* 8 MB */
#define BAR_CFG_SZ_16MB       0x9       /* 16 MB */

/*
 * Extend BAR2 size from 8MB to 16MB on Linux
 *
 * Device [14e4:6024 14e4:6025 14e4:6026 14e4:6027]
 * The device advertise 8MB BAR#2, but switching the size to 16MB
 */
static void bcm_pci_16M_bar2_rc(struct pci_dev *dev)
{
	struct resource *res = &dev->resource[RES_BAR2];

	pci_info(dev, "Resource[%d] %pR\n", RES_BAR2, res);

	if (resource_size(res) == SZ_8M) {
	    res->end = res->start + SZ_16M - 1;
	    pci_info(dev, "Resource[%d] after fixup %pR\n", RES_BAR2, res);
	}


	return;
}

/*
 * Extend BAR2 size from 8MB to 16MB on EP
 *
 * Check and set EP BAR2 to extended size
 */
static void bcm_pci_16M_bar2_ep(struct pci_dev *dev)
{
	void __iomem *pcie_base;
	u32  reg_data;
	u8*  cfgaddr;
	u8*  cfgdata;

	/* Is currently BAR2 is set to 16MB ? */
	if (pci_resource_len(dev, RES_BAR2) != SZ_16M) {
	    /* Nothing to do */
	    return;
	}

	/* map part of bar0 space for PCIe register access */
	/* bar0 + 0x2000 = backplane address 0x28003000 (PCIe block) */
	pcie_base = ioremap(
	                pci_resource_start(dev, RES_BAR0) + BAR0_PCIECOREBASE,
	                SZ_4K);

	if (pcie_base == NULL) {
	    pci_warn(dev, "Can't map BAR0 space\n");
	    return;
	}

	cfgaddr = (u8*)pcie_base + CONFIGADDR_OFFSET;
	cfgdata = (u8*)pcie_base + CONFIGDATA_OFFSET;

	/*
	 * Read EP BAR2 configuraton
	 *
	 * EP BAR# configuration registers:
	 *  BAR0: BAR_CFG2, BAR1: BAR2_CFG, BAR2: BAR3_CFG
	 */ 
	writel(REG_BAR3_CFG, cfgaddr);
	reg_data = readl(cfgdata);
	pci_info(dev, "REG_BAR3_CFG[0x%x] = 0x%x\n", REG_BAR3_CFG, reg_data);

	/* Check if EP BAR2 is configured for 16 MB */
	if ((reg_data & BAR_CFG_SZ_MASK) == BAR_CFG_SZ_8MB) {
	    /* Update BAR2 size to 16 MB */
	    reg_data &= (~BAR_CFG_SZ_MASK);
	    reg_data |= BAR_CFG_SZ_16MB;

	    writel(REG_BAR3_CFG, cfgaddr);
	    writel(reg_data, cfgdata);

	    /* Read back BAR2 configuration */
	    writel(REG_BAR3_CFG, cfgaddr);
	    reg_data = readl(cfgdata);
	    pci_info(dev, "REG_BAR3_CFG[0x%x] after fixup 0x%x\n",
	        REG_BAR3_CFG, reg_data);
	}

	/* Clear the mapping */
	iounmap(pcie_base);

	return;
}

/*
 * WLAN 6717 11BE SoC device id's
 */ 
#define BCM6717_D11BE_ID      0x6024        /* 6717 802.11be 2G+5G device */
#define BCM6717_D11BE2G_ID    0x6025        /* 6717 802.11be 2G device */
#define BCM6717_D11BE5G_ID    0x6026        /* 6717 802.11be 5G device */
#define BCM6717_D11BE6G_ID    0x6027        /* 6717 802.11be 6G device */

#define BCM_BAR2_16M_QUIRK(devid)                    \
	DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_BROADCOM, \
	    (devid), bcm_pci_16M_bar2_rc);               \
	DECLARE_PCI_FIXUP_ENABLE(PCI_VENDOR_ID_BROADCOM, \
	    (devid), bcm_pci_16M_bar2_ep)

BCM_BAR2_16M_QUIRK(BCM6717_D11BE_ID);
BCM_BAR2_16M_QUIRK(BCM6717_D11BE2G_ID);
BCM_BAR2_16M_QUIRK(BCM6717_D11BE5G_ID);
BCM_BAR2_16M_QUIRK(BCM6717_D11BE6G_ID);
