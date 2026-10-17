/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Minda Chen <minda.chen@starfivetech.com>
 *
 */

#ifndef PCIE_PLDA_COMMON_H
#define PCIE_PLDA_COMMON_H

#define GEN_SETTINGS			0x80
#define PCIE_PCI_IDS			0x9C
#define PCIE_WINROM			0xFC
#define PMSG_SUPPORT_RX			0x3F0
#define PCI_MISC			0xB4

#define PLDA_EP_ENABLE			0
#define PLDA_RP_ENABLE			1

#define IDS_CLASS_CODE_SHIFT		8

#define PREF_MEM_WIN_64_SUPPORT		BIT(3)
#define PMSG_LTR_SUPPORT		BIT(2)
#define PLDA_FUNCTION_DIS		BIT(15)
#define PLDA_FUNC_NUM			4
#define PLDA_PHY_FUNC_SHIFT		9

#define XR3PCI_ATR_AXI4_SLV0		0x800
#define XR3PCI_ATR_SRC_ADDR_LOW		0x0
#define XR3PCI_ATR_SRC_ADDR_HIGH	0x4
#define XR3PCI_ATR_TRSL_ADDR_LOW	0x8
#define XR3PCI_ATR_TRSL_ADDR_HIGH	0xc
#define XR3PCI_ATR_TRSL_PARAM		0x10
#define XR3PCI_ATR_TABLE_OFFSET		0x20
#define XR3PCI_ATR_MAX_TABLE_NUM	8

#define XR3PCI_ATR_SRC_WIN_SIZE_SHIFT	1
#define XR3PCI_ATR_SRC_ADDR_MASK	GENMASK(31, 12)
#define XR3PCI_ATR_TRSL_ADDR_MASK	GENMASK(31, 12)
#define XR3PCI_ATR_TRSL_DIR		BIT(22)
/* IDs used in the XR3PCI_ATR_TRSL_PARAM */
#define XR3PCI_ATR_TRSLID_PCIE_MEMORY	0x0
#define XR3PCI_ATR_TRSLID_PCIE_CONFIG	0x1

/**
 * struct pcie_plda - PLDA PCIe controller state
 *
 * @reg_base: The base address of controller register space
 * @cfg_base: The base address of configuration space
 * @cfg_size: The size of configuration space
 * @sec_busno: Secondary bus number.
 * @atr_table_num: Total ATR table numbers.
 */
struct pcie_plda {
	struct udevice	*dev;
	void __iomem *reg_base;
	void __iomem *cfg_base;
	phys_size_t cfg_size;
	int sec_busno;
	int atr_table_num;
};

int plda_pcie_config_read(const struct udevice *udev, pci_dev_t bdf,
			  uint offset, ulong *valuep,
			  enum pci_size_t size);
int plda_pcie_config_write(struct udevice *udev, pci_dev_t bdf,
			   uint offset, ulong value,
			   enum pci_size_t size);
int plda_pcie_set_atr_entry(struct pcie_plda *plda, phys_addr_t src_addr,
			    phys_addr_t trsl_addr, phys_size_t window_size,
			    int trsl_param);

static inline void plda_pcie_enable_root_port(struct pcie_plda *plda)
{
	u32 value;

	value = readl(plda->reg_base + GEN_SETTINGS);
	value |= PLDA_RP_ENABLE;
	writel(value, plda->reg_base + GEN_SETTINGS);
}

static inline void plda_pcie_set_standard_class(struct pcie_plda *plda)
{
	u32 value;

	value = readl(plda->reg_base + PCIE_PCI_IDS);
	value &= 0xff;
	value |= (PCI_CLASS_BRIDGE_PCI_NORMAL << IDS_CLASS_CODE_SHIFT);
	writel(value, plda->reg_base + PCIE_PCI_IDS);
}

static inline void plda_pcie_set_pref_win_64bit(struct pcie_plda *plda)
{
	u32 value;

	value = readl(plda->reg_base + PCIE_WINROM);
	value |= PREF_MEM_WIN_64_SUPPORT;
	writel(value, plda->reg_base + PCIE_WINROM);
}

static inline void plda_pcie_disable_ltr(struct pcie_plda *plda)
{
	u32 value;

	value = readl(plda->reg_base + PMSG_SUPPORT_RX);
	value &= ~PMSG_LTR_SUPPORT;
	writel(value, plda->reg_base + PMSG_SUPPORT_RX);
}

static inline void plda_pcie_disable_func(struct pcie_plda *plda)
{
	u32 value;

	value = readl(plda->reg_base + PCI_MISC);
	value |= PLDA_FUNCTION_DIS;
	writel(value, plda->reg_base + PCI_MISC);
}
#endif
