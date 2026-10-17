/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Copyright (c) 2021 Rockchip, Inc.
 *
 * Copyright (C) 2018 Texas Instruments, Inc
 */

#ifndef PCIE_DW_COMMON_H
#define PCIE_DW_COMMON_H

#define DEFAULT_DBI_ATU_OFFSET (0x3 << 20)

/* PCI DBICS registers */
#define PCIE_LINK_STATUS_REG		0x80
#define PCIE_LINK_STATUS_SPEED_OFF	16
#define PCIE_LINK_STATUS_SPEED_MASK	(0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF	20
#define PCIE_LINK_STATUS_WIDTH_MASK	(0xf << PCIE_LINK_STATUS_WIDTH_OFF)

/*
 * iATU Unroll-specific register definitions
 * From 4.80 core version the address translation will be made by unroll.
 * The registers are offset from atu_base
 */
#define PCIE_ATU_UNR_REGION_CTRL1	0x00
#define PCIE_ATU_UNR_REGION_CTRL2	0x04
#define PCIE_ATU_UNR_LOWER_BASE		0x08
#define PCIE_ATU_UNR_UPPER_BASE		0x0c
#define PCIE_ATU_UNR_LIMIT		0x10
#define PCIE_ATU_UNR_LOWER_TARGET	0x14
#define PCIE_ATU_UNR_UPPER_TARGET	0x18
#define PCIE_ATU_UNR_UPPER_LIMIT	0x20

#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)

/* Register address builder */
#define PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(region)	((region) << 9)

/* Parameters for the waiting for iATU enabled routine */
#define LINK_WAIT_MAX_IATU_RETRIES	5
#define LINK_WAIT_IATU_US		10000

/* PCI DBICS registers */
#define PCIE_LINK_STATUS_REG		0x80
#define PCIE_LINK_STATUS_SPEED_OFF	16
#define PCIE_LINK_STATUS_SPEED_MASK	(0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF	20
#define PCIE_LINK_STATUS_WIDTH_MASK	(0xf << PCIE_LINK_STATUS_WIDTH_OFF)

#define PCIE_LINK_CAPABILITY		0x7c
#define PCIE_LINK_CTL_2			0xa0
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

/* Synopsys-specific PCIe configuration registers */
#define PCIE_PORT_LINK_CONTROL		0x710
#define PORT_LINK_DLL_LINK_EN		BIT(5)
#define PORT_LINK_FAST_LINK_MODE	BIT(7)
#define PORT_LINK_MODE_MASK		GENMASK(21, 16)
#define PORT_LINK_MODE(n)		FIELD_PREP(PORT_LINK_MODE_MASK, n)
#define PORT_LINK_MODE_1_LANES		PORT_LINK_MODE(0x1)
#define PORT_LINK_MODE_2_LANES		PORT_LINK_MODE(0x3)
#define PORT_LINK_MODE_4_LANES		PORT_LINK_MODE(0x7)
#define PORT_LINK_MODE_8_LANES		PORT_LINK_MODE(0xf)

#define PCIE_LINK_WIDTH_SPEED_CONTROL	0x80C
#define PORT_LOGIC_N_FTS_MASK		GENMASK(7, 0)
#define PORT_LOGIC_SPEED_CHANGE		BIT(17)
#define PORT_LOGIC_LINK_WIDTH_MASK	GENMASK(12, 8)
#define PORT_LOGIC_LINK_WIDTH(n)	FIELD_PREP(PORT_LOGIC_LINK_WIDTH_MASK, n)
#define PORT_LOGIC_LINK_WIDTH_1_LANES	PORT_LOGIC_LINK_WIDTH(0x1)
#define PORT_LOGIC_LINK_WIDTH_2_LANES	PORT_LOGIC_LINK_WIDTH(0x2)
#define PORT_LOGIC_LINK_WIDTH_4_LANES	PORT_LOGIC_LINK_WIDTH(0x4)
#define PORT_LOGIC_LINK_WIDTH_8_LANES	PORT_LOGIC_LINK_WIDTH(0x8)

#define PCIE_MISC_CONTROL_1_OFF		0x8bc
#define PCIE_DBI_RO_WR_EN		BIT(0)

/* Parameters for the waiting for iATU enabled routine */
#define LINK_WAIT_MAX_IATU_RETRIES	5
#define LINK_WAIT_IATU			10000

/**
 * struct pcie_dw - DW PCIe controller state
 *
 * @dbi_base: The base address of dbi register space
 * @cfg_base: The base address of configuration space
 * @atu_base: The base address of ATU space
 * @cfg_size: The size of the configuration space which is needed
 *            as it gets written into the PCIE_ATU_LIMIT register
 * @first_busno: This driver supports multiple PCIe controllers.
 *               first_busno stores the bus number of the PCIe root-port
 *               number which may vary depending on the PCIe setup
 *               (PEX switches etc).
 * @io: The IO space for EP's BAR
 * @mem: The memory space for EP's BAR
 * @prefetch: The prefetch space for EP's BAR
 */
struct pcie_dw {
	struct udevice	*dev;
	void __iomem *dbi_base;
	void __iomem *cfg_base;
	void __iomem *atu_base;
	fdt_size_t cfg_size;

	int first_busno;

	/* IO, MEM & PREFETCH PCI regions */
	struct pci_region io;
	struct pci_region mem;
	struct pci_region prefetch;
};

int pcie_dw_get_link_speed(struct pcie_dw *pci);

int pcie_dw_get_link_width(struct pcie_dw *pci);

int pcie_dw_prog_outbound_atu_unroll(struct pcie_dw *pci, int index, int type, u64 cpu_addr,
				     u64 pci_addr, u32 size);

int pcie_dw_read_config(const struct udevice *bus, pci_dev_t bdf, uint offset, ulong *valuep,
			enum pci_size_t size);

int pcie_dw_write_config(struct udevice *bus, pci_dev_t bdf, uint offset, ulong value,
			 enum pci_size_t size);

static inline void dw_pcie_dbi_write_enable(struct pcie_dw *pci, bool en)
{
	u32 val;

	val = readl(pci->dbi_base + PCIE_MISC_CONTROL_1_OFF);
	if (en)
		val |= PCIE_DBI_RO_WR_EN;
	else
		val &= ~PCIE_DBI_RO_WR_EN;
	writel(val, pci->dbi_base + PCIE_MISC_CONTROL_1_OFF);
}

void pcie_dw_setup_host(struct pcie_dw *pci);

#endif
