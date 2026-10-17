// SPDX-License-Identifier: GPL-2.0
/*
 * Intel FPGA PCIe host controller driver
 *
 * Copyright (C) 2013-2018 Intel Corporation. All rights reserved
 *
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define RP_TX_REG0			0x2000
#define RP_TX_CNTRL			0x2004
#define RP_TX_SOP			BIT(0)
#define RP_TX_EOP			BIT(1)
#define RP_RXCPL_STATUS			0x200C
#define RP_RXCPL_SOP			BIT(0)
#define RP_RXCPL_EOP			BIT(1)
#define RP_RXCPL_REG			0x2008
#define P2A_INT_STATUS			0x3060
#define P2A_INT_STS_ALL			0xf
#define P2A_INT_ENABLE			0x3070
#define RP_CAP_OFFSET			0x70

/* TLP configuration type 0 and 1 */
#define TLP_FMTTYPE_CFGRD0		0x04	/* Configuration Read Type 0 */
#define TLP_FMTTYPE_CFGWR0		0x44	/* Configuration Write Type 0 */
#define TLP_FMTTYPE_CFGRD1		0x05	/* Configuration Read Type 1 */
#define TLP_FMTTYPE_CFGWR1		0x45	/* Configuration Write Type 1 */
#define TLP_PAYLOAD_SIZE		0x01
#define TLP_READ_TAG			0x1d
#define TLP_WRITE_TAG			0x10
#define RP_DEVFN			0

#define RP_CFG_ADDR(pcie, reg)						\
		((pcie->hip_base) + (reg) + (1 << 20))
#define RP_SECONDARY(pcie)						\
	readb(RP_CFG_ADDR(pcie, PCI_SECONDARY_BUS))
#define TLP_REQ_ID(bus, devfn)		(((bus) << 8) | (devfn))

#define TLP_CFGRD_DW0(pcie, bus)					\
	((((bus > RP_SECONDARY(pcie)) ? TLP_FMTTYPE_CFGRD1		\
				      : TLP_FMTTYPE_CFGRD0) << 24) |	\
					TLP_PAYLOAD_SIZE)

#define TLP_CFGWR_DW0(pcie, bus)					\
	((((bus > RP_SECONDARY(pcie)) ? TLP_FMTTYPE_CFGWR1		\
				      : TLP_FMTTYPE_CFGWR0) << 24) |	\
					TLP_PAYLOAD_SIZE)

#define TLP_CFG_DW1(pcie, tag, be)					\
	(((TLP_REQ_ID(pcie->first_busno,  RP_DEVFN)) << 16) | (tag << 8) | (be))
#define TLP_CFG_DW2(bus, dev, fn, offset)				\
	(((bus) << 24) | ((dev) << 19) | ((fn) << 16) | (offset))

#define TLP_COMP_STATUS(s)		(((s) >> 13) & 7)
#define TLP_BYTE_COUNT(s)		(((s) >> 0) & 0xfff)
#define TLP_HDR_SIZE			3
#define TLP_LOOP			20000
#define DWORD_MASK			3

#define IS_ROOT_PORT(pcie, bdf)				\
		((PCI_BUS(bdf) == pcie->first_busno) ? true : false)

/**
 * struct intel_fpga_pcie - Intel FPGA PCIe controller state
 * @bus: Pointer to the PCI bus
 * @cra_base: The base address of CRA register space
 * @hip_base: The base address of Rootport configuration space
 * @first_busno: This driver supports multiple PCIe controllers.
 *               first_busno stores the bus number of the PCIe root-port
 *               number which may vary depending on the PCIe setup.
 */
struct intel_fpga_pcie {
	struct udevice *bus;
	void __iomem *cra_base;
	void __iomem *hip_base;
	int first_busno;
};

/**
 * Intel FPGA PCIe port uses BAR0 of RC's configuration space as the
 * translation from PCI bus to native BUS. Entire DDR region is mapped
 * into PCIe space using these registers, so it can be reached by DMA from
 * EP devices.
 * The BAR0 of bridge should be hidden during enumeration to avoid the
 * sizing and resource allocation by PCIe core.
 */
static bool intel_fpga_pcie_hide_rc_bar(struct intel_fpga_pcie *pcie,
					pci_dev_t bdf, int offset)
{
	if (IS_ROOT_PORT(pcie, bdf) && PCI_DEV(bdf) == 0 &&
	    PCI_FUNC(bdf) == 0 && offset == PCI_BASE_ADDRESS_0)
		return true;

	return false;
}

static inline void cra_writel(struct intel_fpga_pcie *pcie, const u32 value,
			      const u32 reg)
{
	writel(value, pcie->cra_base + reg);
}

static inline u32 cra_readl(struct intel_fpga_pcie *pcie, const u32 reg)
{
	return readl(pcie->cra_base + reg);
}

static bool intel_fpga_pcie_link_up(struct intel_fpga_pcie *pcie)
{
	return !!(readw(RP_CFG_ADDR(pcie, RP_CAP_OFFSET + PCI_EXP_LNKSTA))
			& PCI_EXP_LNKSTA_DLLLA);
}

static bool intel_fpga_pcie_addr_valid(struct intel_fpga_pcie *pcie,
				       pci_dev_t bdf)
{
	/* If there is no link, then there is no device */
	if (!IS_ROOT_PORT(pcie, bdf) && !intel_fpga_pcie_link_up(pcie))
		return false;

	/* access only one slot on each root port */
	if (IS_ROOT_PORT(pcie, bdf) && PCI_DEV(bdf) > 0)
		return false;

	if ((PCI_BUS(bdf) == pcie->first_busno + 1) && PCI_DEV(bdf) > 0)
		return false;

	return true;
}

static void tlp_write_tx(struct intel_fpga_pcie *pcie, u32 reg0, u32 ctrl)
{
	cra_writel(pcie, reg0, RP_TX_REG0);
	cra_writel(pcie, ctrl, RP_TX_CNTRL);
}

static int tlp_read_packet(struct intel_fpga_pcie *pcie, u32 *value)
{
	int i;
	u32 ctrl;
	u32 comp_status;
	u32 dw[4];
	u32 count = 0;

	for (i = 0; i < TLP_LOOP; i++) {
		ctrl = cra_readl(pcie, RP_RXCPL_STATUS);
		if (!(ctrl & RP_RXCPL_SOP))
			continue;

		/* read first DW */
		dw[count++] = cra_readl(pcie, RP_RXCPL_REG);

		/* Poll for EOP */
		for (i = 0; i < TLP_LOOP; i++) {
			ctrl = cra_readl(pcie, RP_RXCPL_STATUS);
			dw[count++] = cra_readl(pcie, RP_RXCPL_REG);
			if (ctrl & RP_RXCPL_EOP) {
				comp_status = TLP_COMP_STATUS(dw[1]);
				if (comp_status) {
					*value = pci_get_ff(PCI_SIZE_32);
					return 0;
				}

				if (value &&
				    TLP_BYTE_COUNT(dw[1]) == sizeof(u32) &&
				    count >= 3)
					*value = dw[3];

				return 0;
			}
		}

		udelay(5);
	}

	dev_err(pcie->dev, "read TLP packet timed out\n");
	return -ENODEV;
}

static void tlp_write_packet(struct intel_fpga_pcie *pcie, u32 *headers,
			     u32 data)
{
	tlp_write_tx(pcie, headers[0], RP_TX_SOP);

	tlp_write_tx(pcie, headers[1], 0);

	tlp_write_tx(pcie, headers[2], 0);

	tlp_write_tx(pcie, data, RP_TX_EOP);
}

static int tlp_cfg_dword_read(struct intel_fpga_pcie *pcie, pci_dev_t bdf,
			      int offset, u8 byte_en, u32 *value)
{
	u32 headers[TLP_HDR_SIZE];
	u8 busno = PCI_BUS(bdf);

	headers[0] = TLP_CFGRD_DW0(pcie, busno);
	headers[1] = TLP_CFG_DW1(pcie, TLP_READ_TAG, byte_en);
	headers[2] = TLP_CFG_DW2(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	tlp_write_packet(pcie, headers, 0);

	return tlp_read_packet(pcie, value);
}

static int tlp_cfg_dword_write(struct intel_fpga_pcie *pcie, pci_dev_t bdf,
			       int offset, u8 byte_en, u32 value)
{
	u32 headers[TLP_HDR_SIZE];
	u8 busno = PCI_BUS(bdf);

	headers[0] = TLP_CFGWR_DW0(pcie, busno);
	headers[1] = TLP_CFG_DW1(pcie, TLP_WRITE_TAG, byte_en);
	headers[2] = TLP_CFG_DW2(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	tlp_write_packet(pcie, headers, value);

	return tlp_read_packet(pcie, NULL);
}

int intel_fpga_rp_conf_addr(const struct udevice *bus, pci_dev_t bdf,
			    uint offset, void **paddress)
{
	struct intel_fpga_pcie *pcie = dev_get_priv(bus);

	*paddress = RP_CFG_ADDR(pcie, offset);

	return 0;
}

static int intel_fpga_pcie_rp_rd_conf(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong *valuep,
				      enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, intel_fpga_rp_conf_addr,
					    bdf, offset, valuep, size);
}

static int intel_fpga_pcie_rp_wr_conf(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	int ret;
	struct intel_fpga_pcie *pcie = dev_get_priv(bus);

	ret = pci_generic_mmap_write_config(bus, intel_fpga_rp_conf_addr,
					    bdf, offset, value, size);
	if (!ret) {
		/* Monitor changes to PCI_PRIMARY_BUS register on root port
		 * and update local copy of root bus number accordingly.
		 */
		if (offset == PCI_PRIMARY_BUS)
			pcie->first_busno = (u8)(value);
	}

	return ret;
}

static u8 pcie_get_byte_en(uint offset, enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 1 << (offset & 3);
	case PCI_SIZE_16:
		return 3 << (offset & 3);
	default:
		return 0xf;
	}
}

static int _pcie_intel_fpga_read_config(struct intel_fpga_pcie *pcie,
					pci_dev_t bdf, uint offset,
					ulong *valuep, enum pci_size_t size)
{
	int ret;
	u32 data;
	u8 byte_en;

	/* Uses memory mapped method to read rootport config registers */
	if (IS_ROOT_PORT(pcie, bdf))
		return intel_fpga_pcie_rp_rd_conf(pcie->bus, bdf,
				       offset, valuep, size);

	byte_en = pcie_get_byte_en(offset, size);
	ret = tlp_cfg_dword_read(pcie, bdf, offset & ~DWORD_MASK,
				 byte_en, &data);
	if (ret)
		return ret;

	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08x)\n",
		offset, size, data);
	*valuep = pci_conv_32_to_size(data, offset, size);

	return 0;
}

static int _pcie_intel_fpga_write_config(struct intel_fpga_pcie *pcie,
					 pci_dev_t bdf, uint offset,
					 ulong value, enum pci_size_t size)
{
	u32 data;
	u8 byte_en;

	dev_dbg(pcie->dev, "PCIE CFG write: (b.d.f)=(%02d.%02d.%02d)\n",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08lx)\n",
		offset, size, value);

	/* Uses memory mapped method to read rootport config registers */
	if (IS_ROOT_PORT(pcie, bdf))
		return intel_fpga_pcie_rp_wr_conf(pcie->bus, bdf, offset,
						  value, size);

	byte_en = pcie_get_byte_en(offset, size);
	data = pci_conv_size_to_32(0, value, offset, size);

	return tlp_cfg_dword_write(pcie, bdf, offset & ~DWORD_MASK,
				   byte_en, data);
}

static int pcie_intel_fpga_read_config(const struct udevice *bus, pci_dev_t bdf,
				       uint offset, ulong *valuep,
				       enum pci_size_t size)
{
	struct intel_fpga_pcie *pcie = dev_get_priv(bus);

	dev_dbg(pcie->dev, "PCIE CFG read:  (b.d.f)=(%02d.%02d.%02d)\n",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (intel_fpga_pcie_hide_rc_bar(pcie, bdf, offset)) {
		*valuep = (u32)pci_get_ff(size);
		return 0;
	}

	if (!intel_fpga_pcie_addr_valid(pcie, bdf)) {
		*valuep = (u32)pci_get_ff(size);
		return 0;
	}

	return _pcie_intel_fpga_read_config(pcie, bdf, offset, valuep, size);
}

static int pcie_intel_fpga_write_config(struct udevice *bus, pci_dev_t bdf,
					uint offset, ulong value,
					enum pci_size_t size)
{
	struct intel_fpga_pcie *pcie = dev_get_priv(bus);

	if (intel_fpga_pcie_hide_rc_bar(pcie, bdf, offset))
		return 0;

	if (!intel_fpga_pcie_addr_valid(pcie, bdf))
		return 0;

	return _pcie_intel_fpga_write_config(pcie, bdf, offset, value,
					  size);
}

static int pcie_intel_fpga_probe(struct udevice *dev)
{
	struct intel_fpga_pcie *pcie = dev_get_priv(dev);

	pcie->bus = pci_get_controller(dev);
	pcie->first_busno = dev_seq(dev);

	/* clear all interrupts */
	cra_writel(pcie, P2A_INT_STS_ALL, P2A_INT_STATUS);
	/* disable all interrupts */
	cra_writel(pcie, 0, P2A_INT_ENABLE);

	return 0;
}

static int pcie_intel_fpga_of_to_plat(struct udevice *dev)
{
	struct intel_fpga_pcie *pcie = dev_get_priv(dev);
	struct fdt_resource reg_res;
	int node = dev_of_offset(dev);
	int ret;

	DECLARE_GLOBAL_DATA_PTR;

	ret = fdt_get_named_resource(gd->fdt_blob, node, "reg", "reg-names",
				     "Cra", &reg_res);
	if (ret) {
		dev_err(dev, "resource \"Cra\" not found\n");
		return ret;
	}

	pcie->cra_base = map_physmem(reg_res.start,
				     fdt_resource_size(&reg_res),
				     MAP_NOCACHE);

	ret = fdt_get_named_resource(gd->fdt_blob, node, "reg", "reg-names",
				     "Hip", &reg_res);
	if (ret) {
		dev_err(dev, "resource \"Hip\" not found\n");
		return ret;
	}

	pcie->hip_base = map_physmem(reg_res.start,
				     fdt_resource_size(&reg_res),
				     MAP_NOCACHE);

	return 0;
}

static const struct dm_pci_ops pcie_intel_fpga_ops = {
	.read_config	= pcie_intel_fpga_read_config,
	.write_config	= pcie_intel_fpga_write_config,
};

static const struct udevice_id pcie_intel_fpga_ids[] = {
	{ .compatible = "altr,pcie-root-port-2.0" },
	{},
};

U_BOOT_DRIVER(pcie_intel_fpga) = {
	.name			= "pcie_intel_fpga",
	.id			= UCLASS_PCI,
	.of_match		= pcie_intel_fpga_ids,
	.ops			= &pcie_intel_fpga_ops,
	.of_to_plat	= pcie_intel_fpga_of_to_plat,
	.probe			= pcie_intel_fpga_probe,
	.priv_auto	= sizeof(struct intel_fpga_pcie),
};
