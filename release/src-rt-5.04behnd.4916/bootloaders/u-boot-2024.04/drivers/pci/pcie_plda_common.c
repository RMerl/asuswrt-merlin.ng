// SPDX-License-Identifier: GPL-2.0+
/*
 * PLDA XpressRich PCIe host controller common functions.
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 *
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include "pcie_plda_common.h"

static bool plda_pcie_addr_valid(struct pcie_plda *plda, pci_dev_t bdf)
{
	/*
	 * Single device limitation.
	 * PCIe controller contain HW issue that secondary bus of
	 * host bridge emumerate duplicate devices.
	 * Only can access device 0 in secondary bus.
	 */
	if (PCI_BUS(bdf) == plda->sec_busno && PCI_DEV(bdf) > 0)
		return false;

	return true;
}

static int plda_pcie_conf_address(const struct udevice *udev, pci_dev_t bdf,
				  uint offset, void **paddr)
{
	struct pcie_plda *priv = dev_get_priv(udev);
	int where = PCIE_ECAM_OFFSET(PCI_BUS(bdf) - dev_seq(udev),
				     PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	if (!plda_pcie_addr_valid(priv, bdf))
		return -ENODEV;

	*paddr = (void *)(priv->cfg_base + where);
	return 0;
}

int plda_pcie_config_read(const struct udevice *udev, pci_dev_t bdf,
			  uint offset, ulong *valuep,
			  enum pci_size_t size)
{
	return pci_generic_mmap_read_config(udev, plda_pcie_conf_address,
					    bdf, offset, valuep, size);
}

int plda_pcie_config_write(struct udevice *udev, pci_dev_t bdf,
			   uint offset, ulong value,
			   enum pci_size_t size)
{
	struct pcie_plda *priv = dev_get_priv(udev);
	int ret;

	ret = pci_generic_mmap_write_config(udev, plda_pcie_conf_address,
					    bdf, offset, value, size);

	/* record secondary bus number */
	if (!ret && PCI_BUS(bdf) == dev_seq(udev) &&
	    PCI_DEV(bdf) == 0 && PCI_FUNC(bdf) == 0 &&
	    (offset == PCI_SECONDARY_BUS ||
	    (offset == PCI_PRIMARY_BUS && size != PCI_SIZE_8))) {
		priv->sec_busno =
			((offset == PCI_PRIMARY_BUS) ? (value >> 8) : value) & 0xff;
		priv->sec_busno += dev_seq(udev);
		debug("Secondary bus number was changed to %d\n",
		      priv->sec_busno);
	}
	return ret;
}

int plda_pcie_set_atr_entry(struct pcie_plda *plda, phys_addr_t src_addr,
			    phys_addr_t trsl_addr, phys_size_t window_size,
			    int trsl_param)
{
	void __iomem *base =
		plda->reg_base + XR3PCI_ATR_AXI4_SLV0;

	/* Support AXI4 Slave 0 Address Translation Tables 0-7. */
	if (plda->atr_table_num >= XR3PCI_ATR_MAX_TABLE_NUM) {
		dev_err(plda->dev, "ATR table number %d exceeds max num\n",
			plda->atr_table_num);
		return -EINVAL;
	}
	base +=  XR3PCI_ATR_TABLE_OFFSET * plda->atr_table_num;
	plda->atr_table_num++;

	/*
	 * X3PCI_ATR_SRC_ADDR_LOW:
	 *   - bit 0: enable entry,
	 *   - bits 1-6: ATR window size: total size in bytes: 2^(ATR_WSIZE + 1)
	 *   - bits 7-11: reserved
	 *   - bits 12-31: start of source address
	 */
	writel((lower_32_bits(src_addr) & XR3PCI_ATR_SRC_ADDR_MASK) |
			(fls(window_size) - 1) << XR3PCI_ATR_SRC_WIN_SIZE_SHIFT | 1,
			base + XR3PCI_ATR_SRC_ADDR_LOW);
	writel(upper_32_bits(src_addr), base + XR3PCI_ATR_SRC_ADDR_HIGH);
	writel((lower_32_bits(trsl_addr) & XR3PCI_ATR_TRSL_ADDR_MASK),
	       base + XR3PCI_ATR_TRSL_ADDR_LOW);
	writel(upper_32_bits(trsl_addr), base + XR3PCI_ATR_TRSL_ADDR_HIGH);
	writel(trsl_param, base + XR3PCI_ATR_TRSL_PARAM);

	dev_dbg(plda->dev, "ATR entry: 0x%010llx %s 0x%010llx [0x%010llx] (param: 0x%06x)\n",
		src_addr, (trsl_param & XR3PCI_ATR_TRSL_DIR) ? "<-" : "->",
		trsl_addr, (u64)window_size, trsl_param);
	return 0;
}
