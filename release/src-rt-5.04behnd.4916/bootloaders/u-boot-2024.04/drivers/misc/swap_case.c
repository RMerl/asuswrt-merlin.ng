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
#include <log.h>
#include <pci.h>
#include <asm/test.h>
#include <linux/ctype.h>

/**
 * struct swap_case_plat - platform data for this device
 *
 * @command:	Current PCI command value
 * @bar:	Current base address values
 */
struct swap_case_plat {
	u16 command;
	u32 bar[6];
};

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

static int sandbox_swap_case_use_ea(const struct udevice *dev)
{
	return !!ofnode_get_property(dev_ofnode(dev), "use-ea", NULL);
}

/* Please keep these macros in sync with ea_regs below */
#define PCI_CAP_ID_EA_SIZE		(sizeof(ea_regs) + 4)
#define PCI_CAP_ID_EA_ENTRY_CNT		4
/* Hardcoded EA structure, excluding 1st DW. */
static const u32 ea_regs[] = {
	/* BEI=0, ES=2, BAR0 32b Base + 32b MaxOffset, I/O space */
	(2 << 8) | 2,
	PCI_CAP_EA_BASE_LO0,
	0,
	/* BEI=1, ES=2, BAR1 32b Base + 32b MaxOffset */
	(1 << 4) | 2,
	PCI_CAP_EA_BASE_LO1,
	MEM_TEXT_SIZE - 1,
	/* BEI=2, ES=3, BAR2 64b Base + 32b MaxOffset */
	(2 << 4) | 3,
	PCI_CAP_EA_BASE_LO2 | PCI_EA_IS_64,
	PCI_CAP_EA_SIZE_LO,
	PCI_CAP_EA_BASE_HI2,
	/* BEI=4, ES=4, BAR4 64b Base + 64b MaxOffset */
	(4 << 4) | 4,
	PCI_CAP_EA_BASE_LO4 | PCI_EA_IS_64,
	PCI_CAP_EA_SIZE_LO | PCI_EA_IS_64,
	PCI_CAP_EA_BASE_HI4,
	PCI_CAP_EA_SIZE_HI,
};

static int sandbox_swap_case_read_ea(const struct udevice *emul, uint offset,
				     ulong *valuep, enum pci_size_t size)
{
	u32 reg;

	offset = offset - PCI_CAP_ID_EA_OFFSET - 4;
	reg = ea_regs[offset >> 2];
	reg >>= (offset % 4) * 8;

	*valuep = reg;
	return 0;
}

static int sandbox_swap_case_read_config(const struct udevice *emul,
					 uint offset, ulong *valuep,
					 enum pci_size_t size)
{
	struct swap_case_plat *plat = dev_get_plat(emul);

	/*
	 * The content of the EA capability structure is handled elsewhere to
	 * keep the switch/case below sane
	 */
	if (offset > PCI_CAP_ID_EA_OFFSET + PCI_CAP_LIST_NEXT &&
	    offset < PCI_CAP_ID_EA_OFFSET + PCI_CAP_ID_EA_SIZE)
		return sandbox_swap_case_read_ea(emul, offset, valuep, size);

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
		*valuep = SANDBOX_PCI_SWAP_CASE_EMUL_ID;
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
	case PCI_CAP_ID_EXP_OFFSET + PCI_EXP_DEVCAP:
		*valuep = PCI_EXP_DEVCAP_PAYLOAD_256B;
		break;
	case PCI_CAP_ID_MSIX_OFFSET:
		if (sandbox_swap_case_use_ea(emul))
			*valuep = (PCI_CAP_ID_EA_OFFSET << 8) | PCI_CAP_ID_MSIX;
		else
			*valuep = PCI_CAP_ID_MSIX;
		break;
	case PCI_CAP_ID_MSIX_OFFSET + PCI_CAP_LIST_NEXT:
		if (sandbox_swap_case_use_ea(emul))
			*valuep = PCI_CAP_ID_EA_OFFSET;
		else
			*valuep = 0;
		break;
	case PCI_CAP_ID_EA_OFFSET:
		*valuep = (PCI_CAP_ID_EA_ENTRY_CNT << 16) | PCI_CAP_ID_EA;
		break;
	case PCI_CAP_ID_EA_OFFSET + PCI_CAP_LIST_NEXT:
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
	struct swap_case_plat *plat = dev_get_plat(emul);

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

static int sandbox_swap_case_find_bar(struct udevice *emul, unsigned int addr,
				      int *barnump, unsigned int *offsetp)
{
	struct swap_case_plat *plat = dev_get_plat(emul);
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

static int sandbox_swap_case_read_io(struct udevice *dev, unsigned int addr,
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

static int sandbox_swap_case_write_io(struct udevice *dev, unsigned int addr,
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

static int pci_ea_bar2_magic = PCI_EA_BAR2_MAGIC;

static int sandbox_swap_case_map_physmem(struct udevice *dev,
		phys_addr_t addr, unsigned long *lenp, void **ptrp)
{
	struct swap_case_priv *priv = dev_get_priv(dev);
	unsigned int offset, avail;
	int barnum;
	int ret;

	if (sandbox_swap_case_use_ea(dev)) {
		/*
		 * only support mapping base address in EA test for now, we
		 * don't handle mapping an offset inside a BAR.  Seems good
		 * enough for the current test.
		 */
		switch (addr) {
		case (phys_addr_t)PCI_CAP_EA_BASE_LO0:
			*ptrp = &priv->op;
			*lenp = 4;
			break;
		case (phys_addr_t)PCI_CAP_EA_BASE_LO1:
			*ptrp = priv->mem_text;
			*lenp = barinfo[1].size - 1;
			break;
		case (phys_addr_t)((PCI_CAP_EA_BASE_HI2 << 32) |
				   PCI_CAP_EA_BASE_LO2):
			*ptrp = &pci_ea_bar2_magic;
			*lenp = PCI_CAP_EA_SIZE_LO;
			break;
#ifdef CONFIG_HOST_64BIT
		/*
		 * This cannot be work on a 32-bit machine since *lenp is ulong
		 * which is 32-bits, but it needs to have a 64-bit value
		 * assigned
		 */
		case (phys_addr_t)((PCI_CAP_EA_BASE_HI4 << 32) |
				   PCI_CAP_EA_BASE_LO4): {
			static int pci_ea_bar4_magic = PCI_EA_BAR4_MAGIC;

			*ptrp = &pci_ea_bar4_magic;
			*lenp = (PCI_CAP_EA_SIZE_HI << 32) |
				PCI_CAP_EA_SIZE_LO;
			break;
		}
#endif
		default:
			return -ENOENT;
		}
		return 0;
	}

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

static struct dm_pci_emul_ops sandbox_swap_case_emul_ops = {
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
	.priv_auto	= sizeof(struct swap_case_priv),
	.plat_auto	= sizeof(struct swap_case_plat),
};

static struct pci_device_id sandbox_swap_case_supported[] = {
	{ PCI_VDEVICE(SANDBOX, SANDBOX_PCI_SWAP_CASE_EMUL_ID),
		SWAP_CASE_DRV_DATA },
	{},
};

U_BOOT_PCI_DEVICE(sandbox_swap_case_emul, sandbox_swap_case_supported);
