// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 */
#include <common.h>
#include <asm/bitops.h>
#include <pci.h>
#include <dm.h>
#include <asm/fsl_law.h>

struct mpc85xx_pci_priv {
	void __iomem		*cfg_addr;
	void __iomem		*cfg_data;
};

static int mpc85xx_pci_dm_read_config(const struct udevice *dev, pci_dev_t bdf,
				      uint offset, ulong *value,
				      enum pci_size_t size)
{
	struct mpc85xx_pci_priv *priv = dev_get_priv(dev);
	u32 addr;

	if (offset > 0xff) {
		*value = pci_get_ff(size);
		return 0;
	}

	/* Skip mpc85xx PCI controller's ATMU inbound registers */
	if (PCI_BUS(bdf) == 0 && PCI_DEV(bdf) == 0 && PCI_FUNC(bdf) == 0 &&
	    (offset & ~3) >= PCI_BASE_ADDRESS_0 && (offset & ~3) <= PCI_BASE_ADDRESS_5) {
		*value = 0;
		return 0;
	}

	addr = PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	out_be32(priv->cfg_addr, addr);
	sync();

	switch (size) {
	case PCI_SIZE_8:
		*value = in_8(priv->cfg_data + (offset & 3));
		break;
	case PCI_SIZE_16:
		*value = in_le16(priv->cfg_data + (offset & 2));
		break;
	case PCI_SIZE_32:
		*value = in_le32(priv->cfg_data);
		break;
	}

	return 0;
}

static int mpc85xx_pci_dm_write_config(struct udevice *dev, pci_dev_t bdf,
				       uint offset, ulong value,
				       enum pci_size_t size)
{
	struct mpc85xx_pci_priv *priv = dev_get_priv(dev);
	u32 addr;

	if (offset > 0xff)
		return 0;

	/* Skip mpc85xx PCI controller's ATMU inbound registers */
	if (PCI_BUS(bdf) == 0 && PCI_DEV(bdf) == 0 && PCI_FUNC(bdf) == 0 &&
	    (offset & ~3) >= PCI_BASE_ADDRESS_0 && (offset & ~3) <= PCI_BASE_ADDRESS_5)
		return 0;

	addr = PCI_CONF1_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	out_be32(priv->cfg_addr, addr);
	sync();

	switch (size) {
	case PCI_SIZE_8:
		out_8(priv->cfg_data + (offset & 3), value);
		break;
	case PCI_SIZE_16:
		out_le16(priv->cfg_data + (offset & 2), value);
		break;
	case PCI_SIZE_32:
		out_le32(priv->cfg_data, value);
		break;
	}
	sync();

	return 0;
}

#ifdef CONFIG_FSL_LAW
static int
mpc85xx_pci_dm_setup_laws(struct pci_region *io, struct pci_region *mem,
			  struct pci_region *pre)
{
	/*
	 * Unfortunately we have defines for this addresse,
	 * as we have to setup the TLB, and at this stage
	 * we have no access to DT ... may we check here
	 * if the value in the define is the same ?
	 */
	if (mem)
		set_next_law(mem->phys_start, law_size_bits(mem->size),
			     LAW_TRGT_IF_PCI);
	if (io)
		set_next_law(io->phys_start, law_size_bits(io->size),
			     LAW_TRGT_IF_PCI);
	if (pre)
		set_next_law(pre->phys_start, law_size_bits(pre->size),
			     LAW_TRGT_IF_PCI);

	return 0;
}
#endif

static int mpc85xx_pci_dm_probe(struct udevice *dev)
{
	struct mpc85xx_pci_priv *priv = dev_get_priv(dev);
	struct pci_region *io;
	struct pci_region *mem;
	struct pci_region *pre;
	int count;
	ccsr_pcix_t *pcix;

	count = pci_get_regions(dev, &io, &mem, &pre);
	if (count != 2) {
		printf("%s: wrong count of regions %d only 2 allowed\n",
		       __func__, count);
		return -EINVAL;
	}

#ifdef CONFIG_FSL_LAW
	mpc85xx_pci_dm_setup_laws(io, mem, pre);
#endif

	pcix = priv->cfg_addr;
	/* BAR 1: memory */
	out_be32(&pcix->potar1, mem->bus_start >> 12);
	out_be32(&pcix->potear1, (u64)mem->bus_start >> 44);
	out_be32(&pcix->powbar1, mem->phys_start >> 12);
	out_be32(&pcix->powbear1, (u64)mem->phys_start >> 44);
	out_be32(&pcix->powar1, (POWAR_EN | POWAR_MEM_READ |
		 POWAR_MEM_WRITE | (__ilog2(mem->size) - 1)));

	/* BAR 1: IO */
	out_be32(&pcix->potar2, io->bus_start >> 12);
	out_be32(&pcix->potear2, (u64)io->bus_start >> 44);
	out_be32(&pcix->powbar2, io->phys_start >> 12);
	out_be32(&pcix->powbear2, (u64)io->phys_start >> 44);
	out_be32(&pcix->powar2, (POWAR_EN | POWAR_IO_READ |
		 POWAR_IO_WRITE | (__ilog2(io->size) - 1)));

	out_be32(&pcix->pitar1, 0);
	out_be32(&pcix->piwbar1, 0);
	out_be32(&pcix->piwar1, (PIWAR_EN | PIWAR_PF | PIWAR_LOCAL |
		 PIWAR_READ_SNOOP | PIWAR_WRITE_SNOOP | PIWAR_MEM_2G));

	out_be32(&pcix->powar3, 0);
	out_be32(&pcix->powar4, 0);
	out_be32(&pcix->piwar2, 0);
	out_be32(&pcix->piwar3, 0);

	return 0;
}

static int mpc85xx_pci_dm_remove(struct udevice *dev)
{
	return 0;
}

static int mpc85xx_pci_of_to_plat(struct udevice *dev)
{
	struct mpc85xx_pci_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->cfg_addr = (void __iomem *)map_physmem(addr, 0, MAP_NOCACHE);
	priv->cfg_data = (void __iomem *)((ulong)priv->cfg_addr + 4);

	return 0;
}

static const struct dm_pci_ops mpc85xx_pci_ops = {
	.read_config	= mpc85xx_pci_dm_read_config,
	.write_config	= mpc85xx_pci_dm_write_config,
};

static const struct udevice_id mpc85xx_pci_ids[] = {
	{ .compatible = "fsl,mpc8540-pci" },
	{ }
};

U_BOOT_DRIVER(mpc85xx_pci) = {
	.name			= "mpc85xx_pci",
	.id			= UCLASS_PCI,
	.of_match		= mpc85xx_pci_ids,
	.ops			= &mpc85xx_pci_ops,
	.probe			= mpc85xx_pci_dm_probe,
	.remove			= mpc85xx_pci_dm_remove,
	.of_to_plat	= mpc85xx_pci_of_to_plat,
	.priv_auto	= sizeof(struct mpc85xx_pci_priv),
};
