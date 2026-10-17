// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <pci.h>

#define FDT_DEV_INFO_CELLS	4
#define FDT_DEV_INFO_SIZE	(FDT_DEV_INFO_CELLS * sizeof(u32))

#define SANDBOX_PCI_DEVFN(d, f)	((d << 3) | f)

struct sandbox_pci_priv {
	struct {
		u16 vendor;
		u16 device;
	} vendev[256];
};

static int sandbox_pci_write_config(struct udevice *bus, pci_dev_t devfn,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct dm_pci_emul_ops *ops;
	struct udevice *container, *emul;
	int ret;

	ret = sandbox_pci_get_emul(bus, devfn, &container, &emul);
	if (ret)
		return ret == -ENODEV ? 0 : ret;
	ops = pci_get_emul_ops(emul);
	if (!ops || !ops->write_config)
		return -ENOSYS;

	return ops->write_config(emul, offset, value, size);
}

static int sandbox_pci_read_config(const struct udevice *bus, pci_dev_t devfn,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	struct dm_pci_emul_ops *ops;
	struct udevice *container, *emul;
	struct sandbox_pci_priv *priv = dev_get_priv(bus);
	int ret;

	/* Prepare the default response */
	*valuep = pci_get_ff(size);
	ret = sandbox_pci_get_emul(bus, devfn, &container, &emul);
	if (ret) {
		if (!container) {
			u16 vendor, device;

			devfn = SANDBOX_PCI_DEVFN(PCI_DEV(devfn),
						  PCI_FUNC(devfn));
			vendor = priv->vendev[devfn].vendor;
			device = priv->vendev[devfn].device;
			if (offset == PCI_VENDOR_ID && vendor)
				*valuep = vendor;
			else if (offset == PCI_DEVICE_ID && device)
				*valuep = device;

			return 0;
		} else {
			return ret == -ENODEV ? 0 : ret;
		}
	}
	ops = pci_get_emul_ops(emul);
	if (!ops || !ops->read_config)
		return -ENOSYS;

	return ops->read_config(emul, offset, valuep, size);
}

static int sandbox_pci_probe(struct udevice *dev)
{
	struct sandbox_pci_priv *priv = dev_get_priv(dev);
	const fdt32_t *cell;
	u8 pdev, pfn, devfn;
	int len;

	cell = ofnode_get_property(dev_ofnode(dev), "sandbox,dev-info", &len);
	if (!cell)
		return 0;

	if ((len % FDT_DEV_INFO_SIZE) == 0) {
		int num = len / FDT_DEV_INFO_SIZE;
		int i;

		for (i = 0; i < num; i++) {
			debug("dev info #%d: %02x %02x %04x %04x\n", i,
			      fdt32_to_cpu(cell[0]), fdt32_to_cpu(cell[1]),
			      fdt32_to_cpu(cell[2]), fdt32_to_cpu(cell[3]));

			pdev = fdt32_to_cpu(cell[0]);
			pfn = fdt32_to_cpu(cell[1]);
			if (pdev > 31 || pfn > 7)
				continue;
			devfn = SANDBOX_PCI_DEVFN(pdev, pfn);
			priv->vendev[devfn].vendor = fdt32_to_cpu(cell[2]);
			priv->vendev[devfn].device = fdt32_to_cpu(cell[3]);

			cell += FDT_DEV_INFO_CELLS;
		}
	}

	return 0;
}

static const struct dm_pci_ops sandbox_pci_ops = {
	.read_config = sandbox_pci_read_config,
	.write_config = sandbox_pci_write_config,
};

static const struct udevice_id sandbox_pci_ids[] = {
	{ .compatible = "sandbox,pci" },
	{ }
};

U_BOOT_DRIVER(pci_sandbox) = {
	.name	= "pci_sandbox",
	.id	= UCLASS_PCI,
	.of_match = sandbox_pci_ids,
	.ops	= &sandbox_pci_ops,
	.probe	= sandbox_pci_probe,
	.priv_auto	= sizeof(struct sandbox_pci_priv),

	/* Attach an emulator if we can */
	.child_post_bind = dm_scan_fdt_dev,
	.per_child_plat_auto	= sizeof(struct pci_child_plat),
};
