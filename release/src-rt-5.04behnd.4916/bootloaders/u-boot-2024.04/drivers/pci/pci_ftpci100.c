// SPDX-License-Identifier: GPL-2.0-or-later

#include <common.h>
#include <pci.h>
#include <dm.h>
#include <asm/io.h>

struct ftpci100_data {
	void *reg_base;
};

/* AHB Control Registers */
struct ftpci100_ahbc {
	u32 iosize;	/* 0x00 - I/O Space Size Signal */
	u32 prot;	/* 0x04 - AHB Protection */
	u32 rsved[8];	/* 0x08-0x24 - Reserved */
	u32 conf;	/* 0x28 - PCI Configuration */
	u32 data;	/* 0x2c - PCI Configuration DATA */
};

static int ftpci100_read_config(const struct udevice *dev, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	struct ftpci100_data *priv = dev_get_priv(dev);
	struct ftpci100_ahbc *regs = priv->reg_base;
	u32 data;

	out_le32(&regs->conf, PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset));
	data = in_le32(&regs->data);
	*valuep = pci_conv_32_to_size(data, offset, size);

	return 0;
}

static int ftpci100_write_config(struct udevice *dev, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	struct ftpci100_data *priv = dev_get_priv(dev);
	struct ftpci100_ahbc *regs = priv->reg_base;
	u32 data;

	out_le32(&regs->conf, PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset));

	if (size == PCI_SIZE_32) {
		data = value;
	} else {
		u32 old = in_le32(&regs->data);

		data = pci_conv_size_to_32(old, value, offset, size);
	}

	out_le32(&regs->data, data);

	return 0;
}

static int ftpci100_probe(struct udevice *dev)
{
	struct ftpci100_data *priv = dev_get_priv(dev);
	struct pci_region *io, *mem;
	int count;

	count = pci_get_regions(dev, &io, &mem, NULL);
	if (count != 2) {
		printf("%s: wrong count of regions: %d != 2\n", dev->name, count);
		return -EINVAL;
	}

	priv->reg_base = phys_to_virt(io->phys_start);
	if (!priv->reg_base)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops ftpci100_ops = {
	.read_config	= ftpci100_read_config,
	.write_config	= ftpci100_write_config,
};

static const struct udevice_id ftpci100_ids[] = {
	{ .compatible = "faraday,ftpci100" },
	{ }
};

U_BOOT_DRIVER(ftpci100_pci) = {
	.name		= "ftpci100_pci",
	.id		= UCLASS_PCI,
	.of_match	= ftpci100_ids,
	.ops		= &ftpci100_ops,
	.probe		= ftpci100_probe,
	.priv_auto	= sizeof(struct ftpci100_data),
};
