/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 */

#ifndef _PCIE_LAYERSCAPE_H_
#define _PCIE_LAYERSCAPE_H_
#include <pci.h>
#include <dm.h>

#ifndef CONFIG_SYS_PCI_MEMORY_BUS
#define CONFIG_SYS_PCI_MEMORY_BUS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_PHYS
#define CONFIG_SYS_PCI_MEMORY_PHYS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_SIZE
#define CONFIG_SYS_PCI_MEMORY_SIZE (2 * 1024 * 1024 * 1024UL) /* 2G */
#endif

#ifndef CONFIG_SYS_PCI_EP_MEMORY_BASE
#define CONFIG_SYS_PCI_EP_MEMORY_BASE CONFIG_SYS_LOAD_ADDR
#endif

#define PCIE_PHYS_SIZE			0x200000000
#define LS2088A_PCIE_PHYS_SIZE		0x800000000
#define LS2088A_PCIE1_PHYS_ADDR		0x2000000000

/* iATU registers */
#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_REGION_INBOUND		(0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND	(0x0 << 31)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX2		(0x2 << 0)
#define PCIE_ATU_REGION_INDEX3		(0x3 << 0)
#define PCIE_ATU_REGION_NUM		6
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_BAR_NUM(bar)		((bar) << 8)
#define PCIE_ATU_LOWER_BASE		0x90C
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET		0x91C

/* DBI registers */
#define PCIE_SRIOV		0x178
#define PCIE_STRFMR1		0x71c /* Symbol Timer & Filter Mask Register1 */
#define PCIE_DBI_RO_WR_EN	0x8bc

#define PCIE_LINK_CAP		0x7c
#define PCIE_LINK_SPEED_MASK	0xf
#define PCIE_LINK_WIDTH_MASK	0x3f0
#define PCIE_LINK_STA		0x82

#define LTSSM_STATE_MASK	0x3f
#define LTSSM_PCIE_L0		0x11 /* L0 state */

#define PCIE_DBI_SIZE		0x100000 /* 1M */

#define PCIE_LCTRL0_CFG2_ENABLE	(1 << 31)
#define PCIE_LCTRL0_VF(vf)	((vf) << 22)
#define PCIE_LCTRL0_PF(pf)	((pf) << 16)
#define PCIE_LCTRL0_VF_ACTIVE	(1 << 21)
#define PCIE_LCTRL0_VAL(pf, vf)	(PCIE_LCTRL0_PF(pf) |			   \
				 PCIE_LCTRL0_VF(vf) |			   \
				 ((vf) == 0 ? 0 : PCIE_LCTRL0_VF_ACTIVE) | \
				 PCIE_LCTRL0_CFG2_ENABLE)

#define PCIE_NO_SRIOV_BAR_BASE	0x1000

#define PCIE_PF_NUM		2
#define PCIE_VF_NUM		64

#define PCIE_BAR0_SIZE		(4 * 1024) /* 4K */
#define PCIE_BAR1_SIZE		(8 * 1024) /* 8K for MSIX */
#define PCIE_BAR2_SIZE		(4 * 1024) /* 4K */
#define PCIE_BAR4_SIZE		(1 * 1024 * 1024) /* 1M */

/* LUT registers */
#define PCIE_LUT_UDR(n)		(0x800 + (n) * 8)
#define PCIE_LUT_LDR(n)		(0x804 + (n) * 8)
#define PCIE_LUT_ENABLE		(1 << 31)
#define PCIE_LUT_ENTRY_COUNT	32

/* PF Controll registers */
#define PCIE_PF_CONFIG		0x14
#define PCIE_PF_VF_CTRL		0x7F8
#define PCIE_PF_DBG		0x7FC
#define PCIE_CONFIG_READY	(1 << 0)

#define PCIE_SRDS_PRTCL(idx)	(PCIE1 + (idx))
#define PCIE_SYS_BASE_ADDR	0x3400000
#define PCIE_CCSR_SIZE		0x0100000

/* CS2 */
#define PCIE_CS2_OFFSET		0x1000 /* For PCIe without SR-IOV */

#define SVR_LS102XA		0
#define SVR_VAR_PER_SHIFT	8
#define SVR_LS102XA_MASK	0x700
#define SVR_LS2088A		0x870900
#define SVR_LS2084A		0x870910
#define SVR_LS2048A		0x870920
#define SVR_LS2044A		0x870930
#define SVR_LS2081A		0x870918
#define SVR_LS2041A		0x870914

/* LS1021a PCIE space */
#define LS1021_PCIE_SPACE_OFFSET	0x4000000000ULL
#define LS1021_PCIE_SPACE_SIZE		0x0800000000ULL

/* LS1021a PEX1/2 Misc Ports Status Register */
#define LS1021_PEXMSCPORTSR(pex_idx)	(0x94 + (pex_idx) * 4)
#define LS1021_LTSSM_STATE_SHIFT	20

struct ls_pcie {
	int idx;
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	struct fdt_resource lut_res;
	struct fdt_resource ctrl_res;
	struct fdt_resource cfg_res;
	void __iomem *dbi;
	void __iomem *lut;
	void __iomem *ctrl;
	void __iomem *cfg0;
	void __iomem *cfg1;
	bool big_endian;
	bool enabled;
	int next_lut_index;
	int mode;
};

extern struct list_head ls_pcie_list;

#endif /* _PCIE_LAYERSCAPE_H_ */
