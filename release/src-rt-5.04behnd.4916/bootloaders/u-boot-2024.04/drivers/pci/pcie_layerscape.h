/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2020 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 */

#ifndef _PCIE_LAYERSCAPE_H_
#define _PCIE_LAYERSCAPE_H_
#include <pci.h>

#include <linux/sizes.h>
#include <asm/arch-fsl-layerscape/svr.h>
#include <asm/arch-ls102xa/svr.h>

#ifndef CFG_SYS_PCI_MEMORY_BUS
#define CFG_SYS_PCI_MEMORY_BUS CFG_SYS_SDRAM_BASE
#endif

#ifndef CFG_SYS_PCI_MEMORY_PHYS
#define CFG_SYS_PCI_MEMORY_PHYS CFG_SYS_SDRAM_BASE
#endif

#ifndef CFG_SYS_PCI_MEMORY_SIZE
#define CFG_SYS_PCI_MEMORY_SIZE SZ_4G
#endif

#ifndef CFG_SYS_PCI_EP_MEMORY_BASE
#define CFG_SYS_PCI_EP_MEMORY_BASE CONFIG_SYS_LOAD_ADDR
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
#define PCIE_ATU_REGION_NUM_SRIOV	24
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_FUNC_NUM(pf)		((pf) << 20)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_FUNC_NUM_MATCH_EN	BIT(19)
#define PCIE_ATU_VFBAR_MATCH_MODE_EN	BIT(26)
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
#define PCIE_DBI_RO_WR_EN		BIT(0)
#define PCIE_MISC_CONTROL_1_OFF         0x8BC

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
#define FSL_PCIE_EP_MIN_APERTURE        4096     /* 4 Kbytes */
#define PCIE_PF_NUM		2
#define PCIE_VF_NUM		64
#define BAR_NUM			8

#define PCIE_BAR0_SIZE		SZ_4K
#define PCIE_BAR1_SIZE		SZ_8K
#define PCIE_BAR2_SIZE		SZ_4K
#define PCIE_BAR4_SIZE		SZ_1M

#define PCIE_SRIOV_VFBAR0	0x19C

#define PCIE_MASK_OFFSET(flag, pf, off) ((flag) ? 0 : (0x1000 + (off) * (pf)))

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

/* LS1021a PCIE space */
#define LS1021_PCIE_SPACE_OFFSET	0x4000000000ULL
#define LS1021_PCIE_SPACE_SIZE		0x0800000000ULL

/* LS1021a PEX1/2 Misc Ports Status Register */
#define LS1021_PEXMSCPORTSR(pex_idx)	(0x94 + (pex_idx) * 4)
#define LS1021_LTSSM_STATE_SHIFT	20

/* LX2160a PF1 offset */
#define LX2160_PCIE_PF1_OFFSET	0x8000

/* layerscape PF1 offset */
#define LS_PCIE_PF1_OFFSET	0x20000

struct ls_pcie {
	void __iomem *dbi;
	void __iomem *lut;
	void __iomem *ctrl;
	int idx;
	bool big_endian;
	int mode;
};

struct ls_pcie_rc {
	struct ls_pcie *pcie;
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	struct fdt_resource lut_res;
	struct fdt_resource ctrl_res;
	struct fdt_resource cfg_res;
	void __iomem *cfg0;
	void __iomem *cfg1;
	bool enabled;
	int next_lut_index;
	int stream_id_cur;
};

struct ls_pcie_ep {
	struct fdt_resource addr_res;
	struct ls_pcie *pcie;
	struct udevice *bus;
	void __iomem *addr;
	u32 cfg2_flag;
	u32 sriov_flag;
	u32 pf1_offset;
	u32 num_ib_wins;
	u32 num_ob_wins;
	u8 max_functions;
};

extern struct list_head ls_pcie_list;

unsigned int dbi_readl(struct ls_pcie *pcie, unsigned int offset);
void dbi_writel(struct ls_pcie *pcie, unsigned int value, unsigned int offset);
unsigned int ctrl_readl(struct ls_pcie *pcie, unsigned int offset);
void ctrl_writel(struct ls_pcie *pcie, unsigned int value, unsigned int offset);
void ls_pcie_atu_outbound_set(struct ls_pcie *pcie, int idx, int type,
			      u64 phys, u64 bus_addr, u64 size);
void ls_pcie_atu_inbound_set(struct ls_pcie *pcie, u32 pf, u32 vf_flag,
			     int type, int idx, int bar, u64 phys);
void ls_pcie_dump_atu(struct ls_pcie *pcie, u32 win_num, u32 type);
int ls_pcie_link_up(struct ls_pcie *pcie);
void ls_pcie_dbi_ro_wr_en(struct ls_pcie *pcie);
void ls_pcie_dbi_ro_wr_dis(struct ls_pcie *pcie);

#endif /* _PCIE_LAYERSCAPE_H_ */
