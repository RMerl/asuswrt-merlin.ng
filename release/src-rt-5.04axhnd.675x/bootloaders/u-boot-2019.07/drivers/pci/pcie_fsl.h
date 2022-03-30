/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 *
 * PCIe DM U-Boot driver for Freescale PowerPC SoCs
 * Author: Hou Zhiqiang <Zhiqiang.Hou@nxp.com>
 */

#ifndef _PCIE_FSL_H_
#define _PCIE_FSL_H_

#ifdef CONFIG_SYS_FSL_PCI_VER_3_X
#define FSL_PCIE_CAP_ID			0x70
#else
#define FSL_PCIE_CAP_ID			0x4c
#endif
/* PCIe Device Control Register */
#define PCI_DCR				(FSL_PCIE_CAP_ID + 0x08)
/* PCIe Device Status Register */
#define PCI_DSR				(FSL_PCIE_CAP_ID + 0x0a)
/* PCIe Link Control Register */
#define PCI_LCR				(FSL_PCIE_CAP_ID + 0x10)
/* PCIe Link Status Register */
#define PCI_LSR				(FSL_PCIE_CAP_ID + 0x12)

#ifndef CONFIG_SYS_PCI_MEMORY_BUS
#define CONFIG_SYS_PCI_MEMORY_BUS	0
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_PHYS
#define CONFIG_SYS_PCI_MEMORY_PHYS	0
#endif

#if defined(CONFIG_SYS_PCI_64BIT) && !defined(CONFIG_SYS_PCI64_MEMORY_BUS)
#define CONFIG_SYS_PCI64_MEMORY_BUS	(64ull * 1024 * 1024 * 1024)
#endif

#define PEX_CSR0_LTSSM_MASK		0xFC
#define PEX_CSR0_LTSSM_SHIFT		2
#define LTSSM_L0_REV3			0x11
#define LTSSM_L0			0x16

struct fsl_pcie {
	int idx;
	struct udevice *bus;
	void __iomem *regs;
	u32 law_trgt_if;		/* LAW target ID */
	u32 block_rev;			/* IP block revision */
	bool mode;			/* RC&EP mode flag */
	bool enabled;			/* Enable status */
	struct list_head list;
};

extern struct list_head fsl_pcie_list;

#endif /* _PCIE_FSL_H_ */
