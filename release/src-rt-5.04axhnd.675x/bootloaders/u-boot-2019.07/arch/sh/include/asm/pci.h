/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SH4 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 *
 * u-boot/include/asm-sh/pci.h
 */
#ifndef _ASM_PCI_H_
#define _ASM_PCI_H_

#include <pci.h>
#if defined(CONFIG_SH7751_PCI)
int pci_sh7751_init(struct pci_controller *hose);
#elif defined(CONFIG_SH7780_PCI)
int pci_sh7780_init(struct pci_controller *hose);
#else
#error "Not support PCI."
#endif

int pci_sh4_init(struct pci_controller *hose);
/* PCI dword read for sh4 */
int pci_sh4_read_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 *value);

/* PCI dword write for sh4 */
int pci_sh4_write_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 value);

#endif	/* _ASM_PCI_H_ */
