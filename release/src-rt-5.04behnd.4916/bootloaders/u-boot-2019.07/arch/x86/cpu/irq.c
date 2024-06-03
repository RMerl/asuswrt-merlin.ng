// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pci.h>
#include <asm/pirq_routing.h>
#include <asm/tables.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * pirq_reg_to_linkno() - Convert a PIRQ routing register offset to link number
 *
 * @priv:	IRQ router driver's priv data
 * @reg:	PIRQ routing register offset from the base address
 * @return:	PIRQ link number (0 for PIRQA, 1 for PIRQB, etc)
 */
static inline int pirq_reg_to_linkno(struct irq_router *priv, int reg)
{
	int linkno = 0;

	if (priv->has_regmap) {
		struct pirq_regmap *map = priv->regmap;
		int i;

		for (i = 0; i < priv->link_num; i++) {
			if (reg - priv->link_base == map->offset) {
				linkno = map->link;
				break;
			}
			map++;
		}
	} else {
		linkno = reg - priv->link_base;
	}

	return linkno;
}

/**
 * pirq_linkno_to_reg() - Convert a PIRQ link number to routing register offset
 *
 * @priv:	IRQ router driver's priv data
 * @linkno:	PIRQ link number (0 for PIRQA, 1 for PIRQB, etc)
 * @return:	PIRQ routing register offset from the base address
 */
static inline int pirq_linkno_to_reg(struct irq_router *priv, int linkno)
{
	int reg = 0;

	if (priv->has_regmap) {
		struct pirq_regmap *map = priv->regmap;
		int i;

		for (i = 0; i < priv->link_num; i++) {
			if (linkno == map->link) {
				reg = map->offset + priv->link_base;
				break;
			}
			map++;
		}
	} else {
		reg = linkno + priv->link_base;
	}

	return reg;
}

bool pirq_check_irq_routed(struct udevice *dev, int link, u8 irq)
{
	struct irq_router *priv = dev_get_priv(dev);
	u8 pirq;

	if (priv->config == PIRQ_VIA_PCI)
		dm_pci_read_config8(dev->parent,
				    pirq_linkno_to_reg(priv, link), &pirq);
	else
		pirq = readb((uintptr_t)priv->ibase +
			     pirq_linkno_to_reg(priv, link));

	pirq &= 0xf;

	/* IRQ# 0/1/2/8/13 are reserved */
	if (pirq < 3 || pirq == 8 || pirq == 13)
		return false;

	return pirq == irq ? true : false;
}

int pirq_translate_link(struct udevice *dev, int link)
{
	struct irq_router *priv = dev_get_priv(dev);

	return pirq_reg_to_linkno(priv, link);
}

void pirq_assign_irq(struct udevice *dev, int link, u8 irq)
{
	struct irq_router *priv = dev_get_priv(dev);

	/* IRQ# 0/1/2/8/13 are reserved */
	if (irq < 3 || irq == 8 || irq == 13)
		return;

	if (priv->config == PIRQ_VIA_PCI)
		dm_pci_write_config8(dev->parent,
				     pirq_linkno_to_reg(priv, link), irq);
	else
		writeb(irq, (uintptr_t)priv->ibase +
		       pirq_linkno_to_reg(priv, link));
}

static struct irq_info *check_dup_entry(struct irq_info *slot_base,
					int entry_num, int bus, int device)
{
	struct irq_info *slot = slot_base;
	int i;

	for (i = 0; i < entry_num; i++) {
		if (slot->bus == bus && slot->devfn == (device << 3))
			break;
		slot++;
	}

	return (i == entry_num) ? NULL : slot;
}

static inline void fill_irq_info(struct irq_router *priv, struct irq_info *slot,
				 int bus, int device, int pin, int pirq)
{
	slot->bus = bus;
	slot->devfn = (device << 3) | 0;
	slot->irq[pin - 1].link = pirq_linkno_to_reg(priv, pirq);
	slot->irq[pin - 1].bitmap = priv->irq_mask;
}

static int create_pirq_routing_table(struct udevice *dev)
{
	struct irq_router *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node;
	int len, count;
	const u32 *cell;
	struct pirq_regmap *map;
	struct irq_routing_table *rt;
	struct irq_info *slot, *slot_base;
	int irq_entries = 0;
	int i;
	int ret;

	node = dev_of_offset(dev);

	/* extract the bdf from fdt_pci_addr */
	priv->bdf = dm_pci_get_bdf(dev->parent);

	ret = fdt_stringlist_search(blob, node, "intel,pirq-config", "pci");
	if (!ret) {
		priv->config = PIRQ_VIA_PCI;
	} else {
		ret = fdt_stringlist_search(blob, node, "intel,pirq-config",
					    "ibase");
		if (!ret)
			priv->config = PIRQ_VIA_IBASE;
		else
			return -EINVAL;
	}

	cell = fdt_getprop(blob, node, "intel,pirq-link", &len);
	if (!cell || len != 8)
		return -EINVAL;
	priv->link_base = fdt_addr_to_cpu(cell[0]);
	priv->link_num = fdt_addr_to_cpu(cell[1]);
	if (priv->link_num > CONFIG_MAX_PIRQ_LINKS) {
		debug("Limiting supported PIRQ link number from %d to %d\n",
		      priv->link_num, CONFIG_MAX_PIRQ_LINKS);
		priv->link_num = CONFIG_MAX_PIRQ_LINKS;
	}

	cell = fdt_getprop(blob, node, "intel,pirq-regmap", &len);
	if (cell) {
		if (len % sizeof(struct pirq_regmap))
			return -EINVAL;

		count = len / sizeof(struct pirq_regmap);
		if (count < priv->link_num) {
			printf("Number of pirq-regmap entires is wrong\n");
			return -EINVAL;
		}

		count = priv->link_num;
		priv->regmap = calloc(count, sizeof(struct pirq_regmap));
		if (!priv->regmap)
			return -ENOMEM;

		priv->has_regmap = true;
		map = priv->regmap;
		for (i = 0; i < count; i++) {
			map->link = fdt_addr_to_cpu(cell[0]);
			map->offset = fdt_addr_to_cpu(cell[1]);

			cell += sizeof(struct pirq_regmap) / sizeof(u32);
			map++;
		}
	}

	priv->irq_mask = fdtdec_get_int(blob, node,
					"intel,pirq-mask", PIRQ_BITMAP);

	if (IS_ENABLED(CONFIG_GENERATE_ACPI_TABLE)) {
		/* Reserve IRQ9 for SCI */
		priv->irq_mask &= ~(1 << 9);
	}

	if (priv->config == PIRQ_VIA_IBASE) {
		int ibase_off;

		ibase_off = fdtdec_get_int(blob, node, "intel,ibase-offset", 0);
		if (!ibase_off)
			return -EINVAL;

		/*
		 * Here we assume that the IBASE register has already been
		 * properly configured by U-Boot before.
		 *
		 * By 'valid' we mean:
		 *   1) a valid memory space carved within system memory space
		 *      assigned to IBASE register block.
		 *   2) memory range decoding is enabled.
		 * Hence we don't do any santify test here.
		 */
		dm_pci_read_config32(dev->parent, ibase_off, &priv->ibase);
		priv->ibase &= ~0xf;
	}

	priv->actl_8bit = fdtdec_get_bool(blob, node, "intel,actl-8bit");
	priv->actl_addr = fdtdec_get_int(blob, node, "intel,actl-addr", 0);

	cell = fdt_getprop(blob, node, "intel,pirq-routing", &len);
	if (!cell || len % sizeof(struct pirq_routing))
		return -EINVAL;
	count = len / sizeof(struct pirq_routing);

	rt = calloc(1, sizeof(struct irq_routing_table));
	if (!rt)
		return -ENOMEM;

	/* Populate the PIRQ table fields */
	rt->signature = PIRQ_SIGNATURE;
	rt->version = PIRQ_VERSION;
	rt->rtr_bus = PCI_BUS(priv->bdf);
	rt->rtr_devfn = (PCI_DEV(priv->bdf) << 3) | PCI_FUNC(priv->bdf);
	rt->rtr_vendor = PCI_VENDOR_ID_INTEL;
	rt->rtr_device = PCI_DEVICE_ID_INTEL_ICH7_31;

	slot_base = rt->slots;

	/* Now fill in the irq_info entries in the PIRQ table */
	for (i = 0; i < count;
	     i++, cell += sizeof(struct pirq_routing) / sizeof(u32)) {
		struct pirq_routing pr;

		pr.bdf = fdt_addr_to_cpu(cell[0]);
		pr.pin = fdt_addr_to_cpu(cell[1]);
		pr.pirq = fdt_addr_to_cpu(cell[2]);

		debug("irq_info %d: b.d.f %x.%x.%x INT%c PIRQ%c\n",
		      i, PCI_BUS(pr.bdf), PCI_DEV(pr.bdf),
		      PCI_FUNC(pr.bdf), 'A' + pr.pin - 1,
		      'A' + pr.pirq);

		slot = check_dup_entry(slot_base, irq_entries,
				       PCI_BUS(pr.bdf), PCI_DEV(pr.bdf));
		if (slot) {
			debug("found entry for bus %d device %d, ",
			      PCI_BUS(pr.bdf), PCI_DEV(pr.bdf));

			if (slot->irq[pr.pin - 1].link) {
				debug("skipping\n");

				/*
				 * Sanity test on the routed PIRQ pin
				 *
				 * If they don't match, show a warning to tell
				 * there might be something wrong with the PIRQ
				 * routing information in the device tree.
				 */
				if (slot->irq[pr.pin - 1].link !=
				    pirq_linkno_to_reg(priv, pr.pirq))
					debug("WARNING: Inconsistent PIRQ routing information\n");
				continue;
			}
		} else {
			slot = slot_base + irq_entries++;
		}
		debug("writing INT%c\n", 'A' + pr.pin - 1);
		fill_irq_info(priv, slot, PCI_BUS(pr.bdf), PCI_DEV(pr.bdf),
			      pr.pin, pr.pirq);
	}

	rt->size = irq_entries * sizeof(struct irq_info) + 32;

	/* Fix up the table checksum */
	rt->checksum = table_compute_checksum(rt, rt->size);

	gd->arch.pirq_routing_table = rt;

	return 0;
}

static void irq_enable_sci(struct udevice *dev)
{
	struct irq_router *priv = dev_get_priv(dev);

	if (priv->actl_8bit) {
		/* Bit7 must be turned on to enable ACPI */
		dm_pci_write_config8(dev->parent, priv->actl_addr, 0x80);
	} else {
		/* Write 0 to enable SCI on IRQ9 */
		if (priv->config == PIRQ_VIA_PCI)
			dm_pci_write_config32(dev->parent, priv->actl_addr, 0);
		else
			writel(0, (uintptr_t)priv->ibase + priv->actl_addr);
	}
}

int irq_router_probe(struct udevice *dev)
{
	int ret;

	ret = create_pirq_routing_table(dev);
	if (ret) {
		debug("Failed to create pirq routing table\n");
		return ret;
	}
	/* Route PIRQ */
	pirq_route_irqs(dev, gd->arch.pirq_routing_table->slots,
			get_irq_slot_count(gd->arch.pirq_routing_table));

	if (IS_ENABLED(CONFIG_GENERATE_ACPI_TABLE))
		irq_enable_sci(dev);

	return 0;
}

ulong write_pirq_routing_table(ulong addr)
{
	if (!gd->arch.pirq_routing_table)
		return addr;

	return copy_pirq_routing_table(addr, gd->arch.pirq_routing_table);
}

static const struct udevice_id irq_router_ids[] = {
	{ .compatible = "intel,irq-router" },
	{ }
};

U_BOOT_DRIVER(irq_router_drv) = {
	.name		= "intel_irq",
	.id		= UCLASS_IRQ,
	.of_match	= irq_router_ids,
	.probe		= irq_router_probe,
	.priv_auto_alloc_size = sizeof(struct irq_router),
};

UCLASS_DRIVER(irq) = {
	.id		= UCLASS_IRQ,
	.name		= "irq",
};
