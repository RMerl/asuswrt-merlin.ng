// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Ramon Fried <ramon.fried@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <pci_ep.h>
#include <asm/test.h>

/**
 * struct sandbox_pci_ep_priv - private data for driver
 * @hdr: Stores the EP device header
 * @msix: required MSIx count;
 * @msi: required MSI count;
 */
struct sandbox_pci_ep_priv {
	struct pci_ep_header hdr;
	struct pci_bar bars[6];
	int msix;
	int msi;
	int irq_count;
};

/* Method exported for testing purposes */
int sandbox_get_pci_ep_irq_count(struct udevice *dev)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	return priv->irq_count;
}

static const struct udevice_id sandbox_pci_ep_ids[] = {
	{ .compatible = "sandbox,pci_ep" },
	{ }
};

static int sandbox_write_header(struct udevice *dev, uint fn,
				struct pci_ep_header *hdr)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	memcpy(&priv->hdr, hdr, sizeof(*hdr));

	return 0;
}

static int sandbox_read_header(struct udevice *dev, uint fn,
			       struct pci_ep_header *hdr)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	memcpy(hdr, &priv->hdr, sizeof(*hdr));

	return 0;
}

static int sandbox_set_bar(struct udevice *dev, uint fn,
			   struct pci_bar *ep_bar)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);
	int bar_idx;

	if (fn > 0)
		return -ENODEV;

	bar_idx = ep_bar->barno;

	memcpy(&priv->bars[bar_idx], ep_bar, sizeof(*ep_bar));

	return 0;
}

static int sandbox_read_bar(struct udevice *dev, uint fn,
			    struct pci_bar *ep_bar, enum pci_barno barno)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	memcpy(ep_bar, &priv->bars[barno], sizeof(*ep_bar));

	return 0;
}

static int sandbox_set_msi(struct udevice *dev, uint fn, uint interrupts)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	priv->msi = interrupts;

	return 0;
}

static int sandbox_get_msi(struct udevice *dev, uint fn)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	return priv->msi;
}

static int sandbox_set_msix(struct udevice *dev, uint fn, uint interrupts)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	priv->msix = interrupts;

	return 0;
}

static int sandbox_get_msix(struct udevice *dev, uint fn)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	return priv->msix;
}

static int sandbox_raise_irq(struct udevice *dev, uint fn,
			     enum pci_ep_irq_type type, uint interrupt_num)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	if (fn > 0)
		return -ENODEV;

	priv->irq_count++;

	return 0;
}

static int sandbox_pci_ep_probe(struct udevice *dev)
{
	struct sandbox_pci_ep_priv *priv = dev_get_priv(dev);

	memset(priv, 0, sizeof(*priv));
	return 0;
}

static struct pci_ep_ops sandbox_pci_ep_ops = {
	.write_header = sandbox_write_header,
	.read_header = sandbox_read_header,
	.set_bar = sandbox_set_bar,
	.read_bar = sandbox_read_bar,
	.set_msi = sandbox_set_msi,
	.get_msi = sandbox_get_msi,
	.set_msix = sandbox_set_msix,
	.get_msix = sandbox_get_msix,
	.raise_irq = sandbox_raise_irq,
};

U_BOOT_DRIVER(pci_ep_sandbox) = {
	.name		= "pci_ep_sandbox",
	.id		= UCLASS_PCI_EP,
	.of_match	= sandbox_pci_ep_ids,
	.probe		= sandbox_pci_ep_probe,
	.ops		= &sandbox_pci_ep_ops,
	.priv_auto	= sizeof(struct sandbox_pci_ep_priv),
};
