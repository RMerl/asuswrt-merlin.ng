// SPDX-License-Identifier: GPL-2.0+
/*
 * PCI emulation device which swaps the case of text
 *
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <asm/test.h>
#include <linux/ctype.h>

/**
 * struct swap_case_platdata - platform data for this device
 *
 * @command:	Current PCI command value
 * @bar:	Current base address values
 */
struct swap_case_platdata {
	u16 command;
	u32 bar[6];
};

#define offset_to_barnum(offset)	\
		(((offset) - PCI_BASE_ADDRESS_0) / sizeof(u32))

enum {
	MEM_TEXT_SIZE	= 0x100,
};

enum swap_case_op {
	OP_TO_LOWER,
	OP_TO_UPPER,
	OP_SWAP,
};

static struct pci_bar {
	int type;
	u32 size;
} barinfo[] = {
	{ PCI_BASE_ADDRESS_SPACE_IO, 1 },
	{ PCI_BASE_ADDRESS_MEM_TYPE_32, MEM_TEXT_SIZE },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
};

struct swap_case_priv {
	enum swap_case_op op;
	char mem_text[MEM_TEXT_SIZE];
};

static int sandbox_swap_case_get_devfn(struct udevice *dev)
{
	struct pci_child_platdata *plat = dev_get_parent_platdata(dev);

	return plat->devfn;
}

static int sandbox_swap_case_read_config(struct udevice *emul, uint offset,
					 ulong *valuep, enum pci_size_t size)
{
	struct swap_case_platdata *plat = dev_get_platdata(emul);

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
		*valuep = SANDBOX_PCI_DEVICE_ID;
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
		u32 *bar, result;

		barnum = offset_to_barnum(offset);
		bar = &plat->bar[barnum];

		result = *bar;
		if (*bar == 0xffffffff) {
			if (barinfo[barnum].type) {
				result = (~(barinfo[barnum].size - 1) &
					PCI_BASE_ADDRESS_IO_MASK) |
					PCI_BASE_ADDRESS_SPACE_IO;
			} else {
				result = (~(barinfo[barnum].size - 1) &
					PCI_BASE_ADDRESS_MEM_MASK) |
					PCI_BASE_ADDRESS_MEM_TYPE_32;
			}
		}
		debug("r bar %d=%x\n", barnum, result);
		*valuep = result;
		break;
	}
	case PCI_CAPABILITY_LIST:
		*valuep = PCI_CAP_ID_PM_OFFSET;
		break;
	case PCI_CAP_ID_PM_OFFSET:
		*valuep = (PCI_CAP_ID_EXP_OFFSET << 8) | PCI_CAP_ID_PM;
		break;
	case PCI_CAP_ID_PM_OFFSET + PCI_CAP_LIST_NEXT:
		*valuep = PCI_CAP_ID_EXP_OFFSET;
		break;
	case PCI_CAP_ID_EXP_OFFSET:
		*valuep = (PCI_CAP_ID_MSIX_OFFSET << 8) | PCI_CAP_ID_EXP;
		break;
	case PCI_CAP_ID_EXP_OFFSET + PCI_CAP_LIST_NEXT:
		*valuep = PCI_CAP_ID_MSIX_OFFSET;
		break;
	case PCI_CAP_ID_MSIX_OFFSET:
		*valuep = PCI_CAP_ID_MSIX;
		break;
	case PCI_CAP_ID_MSIX_OFFSET + PCI_CAP_LIST_NEXT:
		*valuep = 0;
		break;
	case PCI_EXT_CAP_ID_ERR_OFFSET:
		*valuep = (PCI_EXT_CAP_ID_VC_OFFSET << 20) | PCI_EXT_CAP_ID_ERR;
		break;
	case PCI_EXT_CAP_ID_VC_OFFSET:
		*valuep = (PCI_EXT_CAP_ID_DSN_OFFSET << 20) | PCI_EXT_CAP_ID_VC;
		break;
	case PCI_EXT_CAP_ID_DSN_OFFSET:
		*valuep = PCI_EXT_CAP_ID_DSN;
		break;
	}

	return 0;
}

static int sandbox_swap_case_write_config(struct udevice *emul, uint offset,
					  ulong value, enum pci_size_t size)
{
	struct swap_case_platdata *plat = dev_get_platdata(emul);

	switch (offset) {
	case PCI_COMMAND:
		plat->command = value;
		break;
	case PCI_BASE_ADDRESS_0:
	case PCI_BASE_ADDRESS_1: {
		int barnum;
		u32 *bar;

		barnum = offset_to_barnum(offset);
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

static int sandbox_swap_case_find_bar(struct udevice *emul, unsigned int addr,
				      int *barnump, unsigned int *offsetp)
{
	struct swap_case_platdata *plat = dev_get_platdata(emul);
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

static void sandbox_swap_case_do_op(enum swap_case_op op, char *str, int len)
{
	for (; len > 0; len--, str++) {
		switch (op) {
		case OP_TO_UPPER:
			*str = toupper(*str);
			break;
		case OP_TO_LOWER:
			*str = tolower(*str);
			break;
		case OP_SWAP:
			if (isupper(*str))
				*str = tolower(*str);
			else
				*str = toupper(*str);
			break;
		}
	}
}

int sandbox_swap_case_read_io(struct udevice *dev, unsigned int addr,
			      ulong *valuep, enum pci_size_t size)
{
	struct swap_case_priv *priv = dev_get_priv(dev);
	unsigned int offset;
	int barnum;
	int ret;

	ret = sandbox_swap_case_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;

	if (barnum == 0 && offset == 0)
		*valuep = (*valuep & ~0xff) | priv->op;

	return 0;
}

int sandbox_swap_case_write_io(struct udevice *dev, unsigned int addr,
			       ulong value, enum pci_size_t size)
{
	struct swap_case_priv *priv = dev_get_priv(dev);
	unsigned int offset;
	int barnum;
	int ret;

	ret = sandbox_swap_case_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;
	if (barnum == 0 && offset == 0)
		priv->op = value;

	return 0;
}

static int sandbox_swap_case_map_physmem(struct udevice *dev,
		phys_addr_t addr, unsigned long *lenp, void **ptrp)
{
	struct swap_case_priv *priv = dev_get_priv(dev);
	unsigned int offset, avail;
	int barnum;
	int ret;

	ret = sandbox_swap_case_find_bar(dev, addr, &barnum, &offset);
	if (ret)
		return ret;
	if (barnum == 1) {
		*ptrp = priv->mem_text + offset;
		avail = barinfo[1].size - offset;
		if (avail > barinfo[1].size)
			*lenp = 0;
		else
			*lenp = min(*lenp, (ulong)avail);

		return 0;
	}

	return -ENOENT;
}

static int sandbox_swap_case_unmap_physmem(struct udevice *dev,
					   const void *vaddr, unsigned long len)
{
	struct swap_case_priv *priv = dev_get_priv(dev);

	sandbox_swap_case_do_op(priv->op, (void *)vaddr, len);

	return 0;
}

struct dm_pci_emul_ops sandbox_swap_case_emul_ops = {
	.get_devfn = sandbox_swap_case_get_devfn,
	.read_config = sandbox_swap_case_read_config,
	.write_config = sandbox_swap_case_write_config,
	.read_io = sandbox_swap_case_read_io,
	.write_io = sandbox_swap_case_write_io,
	.map_physmem = sandbox_swap_case_map_physmem,
	.unmap_physmem = sandbox_swap_case_unmap_physmem,
};

static const struct udevice_id sandbox_swap_case_ids[] = {
	{ .compatible = "sandbox,swap-case" },
	{ }
};

U_BOOT_DRIVER(sandbox_swap_case_emul) = {
	.name		= "sandbox_swap_case_emul",
	.id		= UCLASS_PCI_EMUL,
	.of_match	= sandbox_swap_case_ids,
	.ops		= &sandbox_swap_case_emul_ops,
	.priv_auto_alloc_size = sizeof(struct swap_case_priv),
	.platdata_auto_alloc_size = sizeof(struct swap_case_platdata),
};

static struct pci_device_id sandbox_swap_case_supported[] = {
	{ PCI_VDEVICE(SANDBOX, SANDBOX_PCI_DEVICE_ID), SWAP_CASE_DRV_DATA },
	{},
};

U_BOOT_PCI_DEVICE(sandbox_swap_case_emul, sandbox_swap_case_supported);
