/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2019 NXP
 *
 * PCIe Gen4 driver for NXP Layerscape SoCs
 * Author: Hou Zhiqiang <Minder.Hou@gmail.com>
 */

#ifndef _PCIE_LAYERSCAPE_GEN4_H_
#define _PCIE_LAYERSCAPE_GEN4_H_
#include <pci.h>
#include <dm.h>

#ifndef CONFIG_SYS_PCI_MEMORY_SIZE
#define CONFIG_SYS_PCI_MEMORY_SIZE		(4 * 1024 * 1024 * 1024ULL)
#endif

#ifndef CONFIG_SYS_PCI_EP_MEMORY_BASE
#define CONFIG_SYS_PCI_EP_MEMORY_BASE		CONFIG_SYS_LOAD_ADDR
#endif

#define PCIE_PF_NUM				2
#define PCIE_VF_NUM				32

#define LS_G4_PF0				0
#define LS_G4_PF1				1
#define PF_BAR_NUM				4
#define VF_BAR_NUM				4
#define PCIE_BAR_SIZE				(8 * 1024)		/* 8K */
#define PCIE_BAR0_SIZE				PCIE_BAR_SIZE
#define PCIE_BAR1_SIZE				PCIE_BAR_SIZE
#define PCIE_BAR2_SIZE				PCIE_BAR_SIZE
#define PCIE_BAR4_SIZE				PCIE_BAR_SIZE
#define SIZE_1T					(1024 * 1024 * 1024 * 1024ULL)

/* GPEX CSR */
#define GPEX_CLASSCODE				0x474
#define GPEX_CLASSCODE_SHIFT			16
#define GPEX_CLASSCODE_MASK			0xffff

#define GPEX_CFG_READY				0x4b0
#define PCIE_CONFIG_READY			BIT(0)

#define GPEX_BAR_ENABLE				0x4d4
#define GPEX_BAR_SIZE_LDW			0x4d8
#define GPEX_BAR_SIZE_UDW			0x4dC
#define GPEX_BAR_SELECT				0x4e0

#define BAR_POS(bar, pf, vf_bar)		\
	((bar) + (pf) * PF_BAR_NUM + (vf_bar) * PCIE_PF_NUM * PF_BAR_NUM)

#define GPEX_SRIOV_INIT_VFS_TOTAL_VF(pf)	(0x644 + (pf) * 4)
#define TTL_VF_MASK				0xffff
#define TTL_VF_SHIFT				16
#define INI_VF_MASK				0xffff
#define INI_VF_SHIFT				0
#define GPEX_SRIOV_VF_OFFSET_STRIDE(pf)		(0x704 + (pf) * 4)

/* PAB CSR */
#define PAB_CTRL				0x808
#define PAB_CTRL_APIO_EN			BIT(0)
#define PAB_CTRL_PPIO_EN			BIT(1)
#define PAB_CTRL_MAX_BRST_LEN_SHIFT		4
#define PAB_CTRL_MAX_BRST_LEN_MASK		0x3
#define PAB_CTRL_PAGE_SEL_SHIFT			13
#define PAB_CTRL_PAGE_SEL_MASK			0x3f
#define PAB_CTRL_FUNC_SEL_SHIFT			19
#define PAB_CTRL_FUNC_SEL_MASK			0x1ff

#define PAB_RST_CTRL				0x820
#define PAB_BR_STAT				0x80c

/* AXI PIO Engines */
#define PAB_AXI_PIO_CTRL(idx)			(0x840 + 0x10 * (idx))
#define APIO_EN					BIT(0)
#define MEM_WIN_EN				BIT(1)
#define IO_WIN_EN				BIT(2)
#define CFG_WIN_EN				BIT(3)
#define PAB_AXI_PIO_STAT(idx)			(0x844 + 0x10 * (idx))
#define PAB_AXI_PIO_SL_CMD_STAT(idx)		(0x848 + 0x10 * (idx))
#define PAB_AXI_PIO_SL_ADDR_STAT(idx)		(0x84c + 0x10 * (idx))
#define PAB_AXI_PIO_SL_EXT_ADDR_STAT(idx)	(0xb8a0 + 0x4 * (idx))

/* PEX PIO Engines */
#define PAB_PEX_PIO_CTRL(idx)			(0x8c0 + 0x10 * (idx))
#define PPIO_EN					BIT(0)
#define PAB_PEX_PIO_STAT(idx)			(0x8c4 + 0x10 * (idx))
#define PAB_PEX_PIO_MT_STAT(idx)		(0x8c8 + 0x10 * (idx))

#define INDIRECT_ADDR_BNDRY			0xc00
#define PAGE_IDX_SHIFT				10
#define PAGE_ADDR_MASK				0x3ff

#define OFFSET_TO_PAGE_IDX(off)			\
	(((off) >> PAGE_IDX_SHIFT) & PAB_CTRL_PAGE_SEL_MASK)

#define OFFSET_TO_PAGE_ADDR(off)		\
	(((off) & PAGE_ADDR_MASK) | INDIRECT_ADDR_BNDRY)

/* APIO WINs */
#define PAB_AXI_AMAP_CTRL(idx)			(0xba0 + 0x10 * (idx))
#define PAB_EXT_AXI_AMAP_SIZE(idx)		(0xbaf0 + 0x4 * (idx))
#define PAB_AXI_AMAP_AXI_WIN(idx)		(0xba4 + 0x10 * (idx))
#define PAB_EXT_AXI_AMAP_AXI_WIN(idx)		(0x80a0 + 0x4 * (idx))
#define PAB_AXI_AMAP_PEX_WIN_L(idx)		(0xba8 + 0x10 * (idx))
#define PAB_AXI_AMAP_PEX_WIN_H(idx)		(0xbac + 0x10 * (idx))
#define PAB_AXI_AMAP_PCI_HDR_PARAM(idx)		(0x5ba0 + 0x4 * (idx))
#define FUNC_NUM_PCIE_MASK			GENMASK(7, 0)

#define AXI_AMAP_CTRL_EN			BIT(0)
#define AXI_AMAP_CTRL_TYPE_SHIFT		1
#define AXI_AMAP_CTRL_TYPE_MASK			0x3
#define AXI_AMAP_CTRL_SIZE_SHIFT		10
#define AXI_AMAP_CTRL_SIZE_MASK			0x3fffff

#define PAB_TARGET_BUS(x)			(((x) & 0xff) << 24)
#define PAB_TARGET_DEV(x)			(((x) & 0x1f) << 19)
#define PAB_TARGET_FUNC(x)			(((x) & 0x7) << 16)

#define PAB_AXI_TYPE_CFG			0x00
#define PAB_AXI_TYPE_IO				0x01
#define PAB_AXI_TYPE_MEM			0x02
#define PAB_AXI_TYPE_ATOM			0x03

#define PAB_WINS_NUM				256

/* PPIO WINs RC mode */
#define PAB_PEX_AMAP_CTRL(idx)			(0x4ba0 + 0x10 * (idx))
#define PAB_EXT_PEX_AMAP_SIZE(idx)		(0xbef0 + 0x04 * (idx))
#define PAB_PEX_AMAP_AXI_WIN(idx)		(0x4ba4 + 0x10 * (idx))
#define PAB_EXT_PEX_AMAP_AXI_WIN(idx)		(0xb4a0 + 0x04 * (idx))
#define PAB_PEX_AMAP_PEX_WIN_L(idx)		(0x4ba8 + 0x10 * (idx))
#define PAB_PEX_AMAP_PEX_WIN_H(idx)		(0x4bac + 0x10 * (idx))

#define IB_TYPE_MEM_F				0x2
#define IB_TYPE_MEM_NF				0x3

#define PEX_AMAP_CTRL_TYPE_SHIFT		0x1
#define PEX_AMAP_CTRL_EN_SHIFT			0x0
#define PEX_AMAP_CTRL_TYPE_MASK			0x3
#define PEX_AMAP_CTRL_EN_MASK			0x1

/* PPIO WINs EP mode */
#define PAB_PEX_BAR_AMAP(pf, bar)		\
	(0x1ba0 + 0x20 * (pf) + 4 * (bar))
#define BAR_AMAP_EN				BIT(0)
#define PAB_EXT_PEX_BAR_AMAP(pf, bar)		\
	(0x84a0 + 0x20 * (pf) + 4 * (bar))

/* CCSR registers */
#define PCIE_LINK_CTRL_STA			0x5c
#define PCIE_LINK_SPEED_SHIFT			16
#define PCIE_LINK_SPEED_MASK			0x0f
#define PCIE_LINK_WIDTH_SHIFT			20
#define PCIE_LINK_WIDTH_MASK			0x3f
#define PCIE_SRIOV_CAPABILITY			0x2a0
#define PCIE_SRIOV_VF_OFFSET_STRIDE		0x2b4

/* LUT registers */
#define PCIE_LUT_UDR(n)				(0x800 + (n) * 8)
#define PCIE_LUT_LDR(n)				(0x804 + (n) * 8)
#define PCIE_LUT_ENABLE				BIT(31)
#define PCIE_LUT_ENTRY_COUNT			32

/* PF control registers */
#define PCIE_LTSSM_STA				0x7fc
#define LTSSM_STATE_MASK			0x7f
#define LTSSM_PCIE_L0				0x2d /* L0 state */

#define PCIE_SRDS_PRTCL(idx)			(PCIE1 + (idx))
#define PCIE_SYS_BASE_ADDR			0x3400000
#define PCIE_CCSR_SIZE				0x0100000

struct ls_pcie_g4 {
	int idx;
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource ccsr_res;
	struct fdt_resource cfg_res;
	struct fdt_resource lut_res;
	struct fdt_resource pf_ctrl_res;
	void __iomem *ccsr;
	void __iomem *cfg;
	void __iomem *lut;
	void __iomem *pf_ctrl;
	bool big_endian;
	bool enabled;
	int next_lut_index;
	struct pci_controller hose;
	int stream_id_cur;
	int mode;
	int sriov_support;
};

extern struct list_head ls_pcie_g4_list;

static inline void lut_writel(struct ls_pcie_g4 *pcie, unsigned int value,
			      unsigned int offset)
{
	if (pcie->big_endian)
		out_be32(pcie->lut + offset, value);
	else
		out_le32(pcie->lut + offset, value);
}

static inline u32 lut_readl(struct ls_pcie_g4 *pcie, unsigned int offset)
{
	if (pcie->big_endian)
		return in_be32(pcie->lut + offset);
	else
		return in_le32(pcie->lut + offset);
}

static inline void ccsr_set_page(struct ls_pcie_g4 *pcie, u8 pg_idx)
{
	u32 val;

	val = in_le32(pcie->ccsr + PAB_CTRL);
	val &= ~(PAB_CTRL_PAGE_SEL_MASK << PAB_CTRL_PAGE_SEL_SHIFT);
	val |= (pg_idx & PAB_CTRL_PAGE_SEL_MASK) << PAB_CTRL_PAGE_SEL_SHIFT;

	out_le32(pcie->ccsr + PAB_CTRL, val);
}

static inline unsigned int ccsr_readl(struct ls_pcie_g4 *pcie, u32 offset)
{
	if (offset < INDIRECT_ADDR_BNDRY) {
		ccsr_set_page(pcie, 0);
		return in_le32(pcie->ccsr + offset);
	}

	ccsr_set_page(pcie, OFFSET_TO_PAGE_IDX(offset));
	return in_le32(pcie->ccsr + OFFSET_TO_PAGE_ADDR(offset));
}

static inline void ccsr_writel(struct ls_pcie_g4 *pcie, u32 offset, u32 value)
{
	if (offset < INDIRECT_ADDR_BNDRY) {
		ccsr_set_page(pcie, 0);
		out_le32(pcie->ccsr + offset, value);
	} else {
		ccsr_set_page(pcie, OFFSET_TO_PAGE_IDX(offset));
		out_le32(pcie->ccsr + OFFSET_TO_PAGE_ADDR(offset), value);
	}
}

static inline unsigned int pf_ctrl_readl(struct ls_pcie_g4 *pcie, u32 offset)
{
	if (pcie->big_endian)
		return in_be32(pcie->pf_ctrl + offset);
	else
		return in_le32(pcie->pf_ctrl + offset);
}

static inline void pf_ctrl_writel(struct ls_pcie_g4 *pcie, u32 offset,
				  u32 value)
{
	if (pcie->big_endian)
		out_be32(pcie->pf_ctrl + offset, value);
	else
		out_le32(pcie->pf_ctrl + offset, value);
}

#endif /* _PCIE_LAYERSCAPE_GEN4_H_ */
