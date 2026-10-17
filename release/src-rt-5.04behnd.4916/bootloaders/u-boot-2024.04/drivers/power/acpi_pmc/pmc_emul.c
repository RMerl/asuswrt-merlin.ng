// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI emulation device for an x86 Power-Management Controller (PMC)
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <asm/test.h>
#include <power/acpi_pmc.h>

/**
 * struct pmc_emul_plat - platform data for this device
 *
 * @command:	Current PCI command value
 * @bar:	Current base address values
 */
struct pmc_emul_plat {
	u16 command;
	u32 bar[6];
};

enum {
	MEMMAP_SIZE	= 0x80,
};

static struct pci_bar {
	int type;
	u32 size;
} barinfo[] = {
	{ PCI_BASE_ADDRESS_MEM_TYPE_32, MEMMAP_SIZE },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ PCI_BASE_ADDRESS_SPACE_IO, 256 },
	{ 0, 0 },
};

struct pmc_emul_priv {
	u8 regs[MEMMAP_SIZE];
};

static int sandbox_pmc_emul_read_config(const struct udevice *emul, uint offset,
					ulong *valuep, enum pci_size_t size)
{
	struct pmc_emul_plat *plat = dev_get_plat(emul);

	switch (offset) {
	case PCI_COMMAND:
		*valuep = plat->command;
		break;
	case PCI_HEADER_TYPE:
		*valuep = 0;
		break;
	case PCI_VENDOR_ID:
		*valuep = SANDBOX_PCI_VENDOR_ID;
		break;
	case PCI_DEVICE_ID:
		*valuep = SANDBOX_PCI_PMC_EMUL_ID;
		break;
	case PCI_CLASS_DEVICE:
		if (size == PCI_SIZE_8) {
			*valuep = SANDBOX_PCI_CLASS_SUB_CODE;
		} else {
			*valuep = (SANDBOX_PCI_CLASS_CODE << 8) |
					SANDBOX_PCI_CLASS_SUB_CODE;
		}
		break;
	case PCI_CLASS_CODE:
		*valuep = SANDBOX_PCI_CLASS_CODE;
		break;
	case PCI_BASE_ADDRESS_0:
	case PCI_BASE_ADDRESS_1:
	case PCI_BASE_ADDRESS_2:
	case PCI_BASE_ADDRESS_3:
	case PCI_BASE_ADDRESS_4:
	case PCI_BASE_ADDRESS_5: {
		int barnum;
		u32 *bar;

		barnum = pci_offset_to_barnum(offset);
		bar = &plat->bar[barnum];

		*valuep = sandbox_pci_read_bar(*bar, barinfo[barnum].type,
					       barinfo[barnum].size);
		break;
	}
	case PCI_CAPABILITY_LIST:
		*valuep = PCI_CAP_ID_PM_OFFSET;
		break;
	}

	return 0;
}

static int sandbox_pmc_emul_write_config(struct udevice *emul, uint offset,
					 ulong value, enum pci_size_t size)
{
	struct pmc_emul_plat *plat = dev_get_plat(emul);

	switch (offset) {
	case PCI_COMMAND:
		plat->command = value;
		break;
	case PCI_BASE_ADDRESS_0:
	case PCI_BASE_ADDRESS_1: {
		int barnum;
		u32 *bar;

		barnum = pci_offset_to_barnum(offset);
		bar = &plat->bar[barnum];

		debug("w bar %d=%lx\n", barnum, value);
		*bar = value;
		/* space indicator (bit#0) is read-only */
		*bar |= barinfo[barnum].type;
		break;
	}
	}

	return 0;
}

static int sandbox_pmc_emul_find_bar(struct udevice *emul, unsigned int addr,
				     int *barnump, unsigned int *offsetp)
{
	struct pmc_emul_plat *plat = dev_get_plat(emul);
	int barnum;

	for (barnum = 0; barnum < ARRAY_SIZE(barinfo); barnum++) {
		unsigned int size = barinfo[barnum].size;
		u32 base = plat->bar[barnum] & ~PCI_BASE_ADDRESS_SPACE;

		if (addr >= base && addr < base + size) {
			*barnump = barnum;
			*offsetp = addr - base;
			return 0;
		}
	}
	*barnump = -1;

	return -ENOENT;
}

static int sandbox_pmc_emul_read_io(struct udevice *dev, unsigned int addr,
				    ulong *valuep, enum pci_size_t size)
{
	unsigned int offset;
	int barnum;
	int ret;

	ret = sandbox_pmc_emul_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;

	if (barnum == 4)
		*valuep = offset;
	else if (barnum == 0)
		*valuep = offset;

	return 0;
}

static int sandbox_pmc_emul_write_io(struct udevice *dev, unsigned int addr,
				     ulong value, enum pci_size_t size)
{
	unsigned int offset;
	int barnum;
	int ret;

	ret = sandbox_pmc_emul_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;

	return 0;
}

static int sandbox_pmc_emul_map_physmem(struct udevice *dev,
					phys_addr_t addr, unsigned long *lenp,
					void **ptrp)
{
	struct pmc_emul_priv *priv = dev_get_priv(dev);
	unsigned int offset, avail;
	int barnum;
	int ret;

	ret = sandbox_pmc_emul_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;

	if (barnum == 0) {
		*ptrp = priv->regs + offset;
		avail = barinfo[0].size - offset;
		if (avail > barinfo[0].size)
			*lenp = 0;
		else
			*lenp = min(*lenp, (ulong)avail);

		return 0;
	}

	return -ENOENT;
}

static int sandbox_pmc_probe(struct udevice *dev)
{
	struct pmc_emul_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < MEMMAP_SIZE; i++)
		priv->regs[i] = i;

	return 0;
}

static struct dm_pci_emul_ops sandbox_pmc_emul_emul_ops = {
	.read_config = sandbox_pmc_emul_read_config,
	.write_config = sandbox_pmc_emul_write_config,
	.read_io = sandbox_pmc_emul_read_io,
	.write_io = sandbox_pmc_emul_write_io,
	.map_physmem = sandbox_pmc_emul_map_physmem,
};

static const struct udevice_id sandbox_pmc_emul_ids[] = {
	{ .compatible = "sandbox,pmc-emul" },
	{ }
};

U_BOOT_DRIVER(sandbox_pmc_emul_emul) = {
	.name		= "sandbox_pmc_emul_emul",
	.id		= UCLASS_PCI_EMUL,
	.of_match	= sandbox_pmc_emul_ids,
	.ops		= &sandbox_pmc_emul_emul_ops,
	.probe		= sandbox_pmc_probe,
	.priv_auto	= sizeof(struct pmc_emul_priv),
	.plat_auto	= sizeof(struct pmc_emul_plat),
};

static struct pci_device_id sandbox_pmc_emul_supported[] = {
	{ PCI_VDEVICE(SANDBOX, SANDBOX_PCI_PMC_EMUL_ID) },
	{},
};

U_BOOT_PCI_DEVICE(sandbox_pmc_emul_emul, sandbox_pmc_emul_supported);
