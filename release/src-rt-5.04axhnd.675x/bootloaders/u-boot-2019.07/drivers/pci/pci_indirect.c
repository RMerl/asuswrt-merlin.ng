// SPDX-License-Identifier: GPL-2.0+
/*
 * Support for indirect PCI bridges.
 *
 * Copyright (C) 1998 Gabriel Paubert.
 */

#include <common.h>

#if !defined(__I386__) && !defined(CONFIG_DM_PCI)

#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define cfg_read(val, addr, type, op)	*val = op((type)(addr))
#define cfg_write(val, addr, type, op)	op((type *)(addr), (val))

#if defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
#define INDIRECT_PCI_OP(rw, size, type, op, mask)                        \
static int                                                               \
indirect_##rw##_config_##size(struct pci_controller *hose,               \
			      pci_dev_t dev, int offset, type val)       \
{                                                                        \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	*(hose->cfg_addr) = dev | (offset & 0xfc) | ((offset & 0xf00) << 16) | 0x80000000; \
	sync();                                                          \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);       \
	return 0;                                                        \
}
#else
#define INDIRECT_PCI_OP(rw, size, type, op, mask)			 \
static int								 \
indirect_##rw##_config_##size(struct pci_controller *hose,		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000000);	 \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	 \
	return 0;							 \
}
#endif

INDIRECT_PCI_OP(read, byte, u8 *, in_8, 3)
INDIRECT_PCI_OP(read, word, u16 *, in_le16, 2)
INDIRECT_PCI_OP(read, dword, u32 *, in_le32, 0)
INDIRECT_PCI_OP(write, byte, u8, out_8, 3)
INDIRECT_PCI_OP(write, word, u16, out_le16, 2)
INDIRECT_PCI_OP(write, dword, u32, out_le32, 0)

void pci_setup_indirect(struct pci_controller* hose, u32 cfg_addr, u32 cfg_data)
{
	pci_set_ops(hose,
		    indirect_read_config_byte,
		    indirect_read_config_word,
		    indirect_read_config_dword,
		    indirect_write_config_byte,
		    indirect_write_config_word,
		    indirect_write_config_dword);

	hose->cfg_addr = (unsigned int *) cfg_addr;
	hose->cfg_data = (unsigned char *) cfg_data;
}

#endif	/* !__I386__ */
