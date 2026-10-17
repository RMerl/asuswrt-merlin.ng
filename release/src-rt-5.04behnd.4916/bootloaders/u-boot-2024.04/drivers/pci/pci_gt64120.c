// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Based on the Linux implementation.
 *   Copyright (C) 1999, 2000, 2004  MIPS Technologies, Inc.
 *   Authors: Carsten Langgaard <carstenl@mips.com>
 *            Maciej W. Rozycki <macro@mips.com>
 */

#include <dm.h>
#include <gt64120.h>
#include <init.h>
#include <log.h>
#include <pci.h>
#include <pci_gt64120.h>

#include <asm/io.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

struct gt64120_regs {
	u8	unused_000[0xc18];
	u32	intrcause;
	u8	unused_c1c[0x0dc];
	u32	pci0_cfgaddr;
	u32	pci0_cfgdata;
};

struct gt64120_pci_controller {
	struct pci_controller hose;
	struct gt64120_regs *regs;
};

static inline struct gt64120_pci_controller *
hose_to_gt64120(struct pci_controller *hose)
{
	return container_of(hose, struct gt64120_pci_controller, hose);
}

#define GT_INTRCAUSE_ABORT_BITS	\
		(GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT)

static int gt_config_access(struct gt64120_pci_controller *gt,
			    unsigned char access_type, pci_dev_t bdf,
			    int where, u32 *data)
{
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int func = PCI_FUNC(bdf);
	u32 intr;
	u32 addr;
	u32 val;

	if (bus == 0 && dev >= 31) {
		/* Because of a bug in the galileo (for slot 31). */
		return -1;
	}

	if (access_type == PCI_ACCESS_WRITE)
		debug("PCI WR %02x:%02x.%x reg:%02d data:%08x\n",
		      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), where, *data);

	/* Clear cause register bits */
	writel(~GT_INTRCAUSE_ABORT_BITS, &gt->regs->intrcause);

	addr = PCI_CONF1_ADDRESS(bus, dev, func, where);

	/* Setup address */
	writel(addr, &gt->regs->pci0_cfgaddr);

	if (access_type == PCI_ACCESS_WRITE) {
		if (bus == 0 && dev == 0) {
			/*
			 * The Galileo system controller is acting
			 * differently than other devices.
			 */
			val = *data;
		} else {
			val = cpu_to_le32(*data);
		}

		writel(val, &gt->regs->pci0_cfgdata);
	} else {
		val = readl(&gt->regs->pci0_cfgdata);

		if (bus == 0 && dev == 0) {
			/*
			 * The Galileo system controller is acting
			 * differently than other devices.
			 */
			*data = val;
		} else {
			*data = le32_to_cpu(val);
		}
	}

	/* Check for master or target abort */
	intr = readl(&gt->regs->intrcause);
	if (intr & GT_INTRCAUSE_ABORT_BITS) {
		/* Error occurred, clear abort bits */
		writel(~GT_INTRCAUSE_ABORT_BITS, &gt->regs->intrcause);
		return -1;
	}

	if (access_type == PCI_ACCESS_READ)
		debug("PCI RD %02x:%02x.%x reg:%02d data:%08x\n",
		      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), where, *data);

	return 0;
}

static int gt64120_pci_read_config(const struct udevice *dev, pci_dev_t bdf,
				   uint where, ulong *val,
				   enum pci_size_t size)
{
	struct gt64120_pci_controller *gt = dev_get_priv(dev);
	u32 data = 0;

	if (gt_config_access(gt, PCI_ACCESS_READ, bdf, where, &data)) {
		*val = pci_get_ff(size);
		return 0;
	}

	*val = pci_conv_32_to_size(data, where, size);

	return 0;
}

static int gt64120_pci_write_config(struct udevice *dev, pci_dev_t bdf,
				    uint where, ulong val,
				    enum pci_size_t size)
{
	struct gt64120_pci_controller *gt = dev_get_priv(dev);
	u32 data = 0;

	if (size == PCI_SIZE_32) {
		data = val;
	} else {
		u32 old;

		if (gt_config_access(gt, PCI_ACCESS_READ, bdf, where, &old))
			return 0;

		data = pci_conv_size_to_32(old, val, where, size);
	}

	gt_config_access(gt, PCI_ACCESS_WRITE, bdf, where, &data);

	return 0;
}

static int gt64120_pci_probe(struct udevice *dev)
{
	struct gt64120_pci_controller *gt = dev_get_priv(dev);

	gt->regs = dev_remap_addr(dev);
	if (!gt->regs)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops gt64120_pci_ops = {
	.read_config	= gt64120_pci_read_config,
	.write_config	= gt64120_pci_write_config,
};

static const struct udevice_id gt64120_pci_ids[] = {
	{ .compatible = "marvell,pci-gt64120" },
	{ }
};

U_BOOT_DRIVER(gt64120_pci) = {
	.name		= "gt64120_pci",
	.id		= UCLASS_PCI,
	.of_match	= gt64120_pci_ids,
	.ops		= &gt64120_pci_ops,
	.probe		= gt64120_pci_probe,
	.priv_auto	= sizeof(struct gt64120_pci_controller),
};
