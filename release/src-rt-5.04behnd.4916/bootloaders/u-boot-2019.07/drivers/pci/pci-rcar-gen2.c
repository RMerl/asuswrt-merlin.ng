// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas RCar Gen2 PCIEC driver
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>

/* AHB-PCI Bridge PCI communication registers */
#define RCAR_AHBPCI_PCICOM_OFFSET	0x800

#define RCAR_PCIAHB_WIN1_CTR_REG	(RCAR_AHBPCI_PCICOM_OFFSET + 0x00)
#define RCAR_PCIAHB_WIN2_CTR_REG	(RCAR_AHBPCI_PCICOM_OFFSET + 0x04)
#define RCAR_PCIAHB_PREFETCH0		0x0
#define RCAR_PCIAHB_PREFETCH4		0x1
#define RCAR_PCIAHB_PREFETCH8		0x2
#define RCAR_PCIAHB_PREFETCH16		0x3

#define RCAR_AHBPCI_WIN1_CTR_REG	(RCAR_AHBPCI_PCICOM_OFFSET + 0x10)
#define RCAR_AHBPCI_WIN2_CTR_REG	(RCAR_AHBPCI_PCICOM_OFFSET + 0x14)
#define RCAR_AHBPCI_WIN_CTR_MEM		(3 << 1)
#define RCAR_AHBPCI_WIN_CTR_CFG		(5 << 1)
#define RCAR_AHBPCI_WIN1_HOST		BIT(30)
#define RCAR_AHBPCI_WIN1_DEVICE		BIT(31)

#define RCAR_PCI_INT_ENABLE_REG		(RCAR_AHBPCI_PCICOM_OFFSET + 0x20)
#define RCAR_PCI_INT_STATUS_REG		(RCAR_AHBPCI_PCICOM_OFFSET + 0x24)
#define RCAR_PCI_INT_SIGTABORT		BIT(0)
#define RCAR_PCI_INT_SIGRETABORT	BIT(1)
#define RCAR_PCI_INT_REMABORT		BIT(2)
#define RCAR_PCI_INT_PERR		BIT(3)
#define RCAR_PCI_INT_SIGSERR		BIT(4)
#define RCAR_PCI_INT_RESERR		BIT(5)
#define RCAR_PCI_INT_WIN1ERR		BIT(12)
#define RCAR_PCI_INT_WIN2ERR		BIT(13)
#define RCAR_PCI_INT_A			BIT(16)
#define RCAR_PCI_INT_B			BIT(17)
#define RCAR_PCI_INT_PME		BIT(19)
#define RCAR_PCI_INT_ALLERRORS (RCAR_PCI_INT_SIGTABORT		| \
				RCAR_PCI_INT_SIGRETABORT	| \
				RCAR_PCI_INT_SIGRETABORT	| \
				RCAR_PCI_INT_REMABORT		| \
				RCAR_PCI_INT_PERR		| \
				RCAR_PCI_INT_SIGSERR		| \
				RCAR_PCI_INT_RESERR		| \
				RCAR_PCI_INT_WIN1ERR		| \
				RCAR_PCI_INT_WIN2ERR)

#define RCAR_AHB_BUS_CTR_REG		(RCAR_AHBPCI_PCICOM_OFFSET + 0x30)
#define RCAR_AHB_BUS_MMODE_HTRANS	BIT(0)
#define RCAR_AHB_BUS_MMODE_BYTE_BURST	BIT(1)
#define RCAR_AHB_BUS_MMODE_WR_INCR	BIT(2)
#define RCAR_AHB_BUS_MMODE_HBUS_REQ	BIT(7)
#define RCAR_AHB_BUS_SMODE_READYCTR	BIT(17)
#define RCAR_AHB_BUS_MODE		(RCAR_AHB_BUS_MMODE_HTRANS |	\
					RCAR_AHB_BUS_MMODE_BYTE_BURST |	\
					RCAR_AHB_BUS_MMODE_WR_INCR |	\
					RCAR_AHB_BUS_MMODE_HBUS_REQ |	\
					RCAR_AHB_BUS_SMODE_READYCTR)

#define RCAR_USBCTR_REG			(RCAR_AHBPCI_PCICOM_OFFSET + 0x34)
#define RCAR_USBCTR_USBH_RST		BIT(0)
#define RCAR_USBCTR_PCICLK_MASK		BIT(1)
#define RCAR_USBCTR_PLL_RST		BIT(2)
#define RCAR_USBCTR_DIRPD		BIT(8)
#define RCAR_USBCTR_PCIAHB_WIN2_EN	BIT(9)
#define RCAR_USBCTR_PCIAHB_WIN1_256M	(0 << 10)
#define RCAR_USBCTR_PCIAHB_WIN1_512M	(1 << 10)
#define RCAR_USBCTR_PCIAHB_WIN1_1G	(2 << 10)
#define RCAR_USBCTR_PCIAHB_WIN1_2G	(3 << 10)
#define RCAR_USBCTR_PCIAHB_WIN1_MASK	(3 << 10)

#define RCAR_PCI_ARBITER_CTR_REG	(RCAR_AHBPCI_PCICOM_OFFSET + 0x40)
#define RCAR_PCI_ARBITER_PCIREQ0	BIT(0)
#define RCAR_PCI_ARBITER_PCIREQ1	BIT(1)
#define RCAR_PCI_ARBITER_PCIBP_MODE	BIT(12)

#define RCAR_PCI_UNIT_REV_REG		(RCAR_AHBPCI_PCICOM_OFFSET + 0x48)

struct rcar_gen2_pci_priv {
	fdt_addr_t		cfg_base;
	fdt_addr_t		mem_base;
};

static int rcar_gen2_pci_addr_valid(pci_dev_t d, uint offset)
{
	u32 slot;

	if (PCI_FUNC(d))
		return -EINVAL;

	/* Only one EHCI/OHCI device built-in */
	slot = PCI_DEV(d);
	if (slot != 1 && slot != 2)
		return -EINVAL;

	/* bridge logic only has registers to 0x40 */
	if (slot == 0x0 && offset >= 0x40)
		return -EINVAL;

	return 0;
}

static u32 get_bus_address(struct udevice *dev, pci_dev_t bdf, u32 offset)
{
	struct rcar_gen2_pci_priv *priv = dev_get_priv(dev);

	return priv->cfg_base + (PCI_DEV(bdf) >> 1) * 0x100 + (offset & ~3);
}

static u32 setup_bus_address(struct udevice *dev, pci_dev_t bdf, u32 offset)
{
	struct rcar_gen2_pci_priv *priv = dev_get_priv(dev);
	u32 reg;

	reg = PCI_DEV(bdf) ? RCAR_AHBPCI_WIN1_DEVICE : RCAR_AHBPCI_WIN1_HOST;
	reg |= RCAR_AHBPCI_WIN_CTR_CFG;
	writel(reg, priv->cfg_base + RCAR_AHBPCI_WIN1_CTR_REG);

	return get_bus_address(dev, bdf, offset);
}

static int rcar_gen2_pci_read_config(struct udevice *dev, pci_dev_t bdf,
				     uint offset, ulong *value,
				     enum pci_size_t size)
{
	u32 addr, reg;
	int ret;

	ret = rcar_gen2_pci_addr_valid(bdf, offset);
	if (ret) {
		*value = pci_get_ff(size);
		return 0;
	}

	addr = get_bus_address(dev, bdf, offset);
	reg = readl(addr);
	*value = pci_conv_32_to_size(reg, offset, size);

	return 0;
}

static int rcar_gen2_pci_write_config(struct udevice *dev, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	u32 addr, reg, old;
	int ret;

	ret = rcar_gen2_pci_addr_valid(bdf, offset);
	if (ret)
		return ret;

	addr = get_bus_address(dev, bdf, offset);

	old = readl(addr);
	reg = pci_conv_size_to_32(old, value, offset, size);
	writel(reg, addr);

	return 0;
}

static int rcar_gen2_pci_probe(struct udevice *dev)
{
	struct rcar_gen2_pci_priv *priv = dev_get_priv(dev);
	struct clk pci_clk;
	u32 devad;
	int ret;

	ret = clk_get_by_index(dev, 0, &pci_clk);
	if (ret)
		return ret;

	ret = clk_enable(&pci_clk);
	if (ret)
		return ret;

	/* Clock & Reset & Direct Power Down */
	clrsetbits_le32(priv->cfg_base + RCAR_USBCTR_REG,
			RCAR_USBCTR_DIRPD | RCAR_USBCTR_PCICLK_MASK |
			RCAR_USBCTR_USBH_RST,
			RCAR_USBCTR_PCIAHB_WIN1_1G);
	clrbits_le32(priv->cfg_base + RCAR_USBCTR_REG, RCAR_USBCTR_PLL_RST);

	/* AHB-PCI Bridge Communication Registers */
	writel(RCAR_AHB_BUS_MODE, priv->cfg_base + RCAR_AHB_BUS_CTR_REG);
	writel((CONFIG_SYS_SDRAM_BASE & 0xf0000000) | RCAR_PCIAHB_PREFETCH16,
	       priv->cfg_base + RCAR_PCIAHB_WIN1_CTR_REG);
	writel(0xf0000000 | RCAR_PCIAHB_PREFETCH16,
	       priv->cfg_base + RCAR_PCIAHB_WIN2_CTR_REG);
	writel(priv->mem_base | RCAR_AHBPCI_WIN_CTR_MEM,
	       priv->cfg_base + RCAR_AHBPCI_WIN2_CTR_REG);
	setbits_le32(priv->cfg_base + RCAR_PCI_ARBITER_CTR_REG,
		     RCAR_PCI_ARBITER_PCIREQ0 | RCAR_PCI_ARBITER_PCIREQ1 |
		     RCAR_PCI_ARBITER_PCIBP_MODE);

	/* PCI Configuration Registers for AHBPCI */
	devad = setup_bus_address(dev, PCI_BDF(0, 0, 0), 0);
	writel(priv->cfg_base + 0x800, devad + PCI_BASE_ADDRESS_0);
	writel(CONFIG_SYS_SDRAM_BASE & 0xf0000000, devad + PCI_BASE_ADDRESS_1);
	writel(0xf0000000, devad + PCI_BASE_ADDRESS_2);
	writel(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER |
	       PCI_COMMAND_PARITY | PCI_COMMAND_SERR,
	       devad + PCI_COMMAND);

	/* PCI Configuration Registers for OHCI */
	devad = setup_bus_address(dev, PCI_BDF(0, 1, 0), 0);
	writel(priv->mem_base + 0x0, devad + PCI_BASE_ADDRESS_0);
	writel(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER |
	       PCI_COMMAND_PARITY | PCI_COMMAND_SERR,
	       devad + PCI_COMMAND);

	/* PCI Configuration Registers for EHCI */
	devad = setup_bus_address(dev, PCI_BDF(0, 2, 0), 0);
	writel(priv->mem_base + 0x1000, devad + PCI_BASE_ADDRESS_0);
	writel(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER |
	       PCI_COMMAND_PARITY | PCI_COMMAND_SERR,
	       devad + PCI_COMMAND);

	/* Enable PCI interrupt */
	setbits_le32(priv->cfg_base + RCAR_PCI_INT_ENABLE_REG,
		     RCAR_PCI_INT_A | RCAR_PCI_INT_B | RCAR_PCI_INT_PME);

	return 0;
}

static int rcar_gen2_pci_ofdata_to_platdata(struct udevice *dev)
{
	struct rcar_gen2_pci_priv *priv = dev_get_priv(dev);

	priv->cfg_base = devfdt_get_addr_index(dev, 0);
	priv->mem_base = devfdt_get_addr_index(dev, 1);
	if (!priv->cfg_base || !priv->mem_base)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops rcar_gen2_pci_ops = {
	.read_config	= rcar_gen2_pci_read_config,
	.write_config	= rcar_gen2_pci_write_config,
};

static const struct udevice_id rcar_gen2_pci_ids[] = {
	{ .compatible = "renesas,pci-rcar-gen2" },
	{ }
};

U_BOOT_DRIVER(rcar_gen2_pci) = {
	.name			= "rcar_gen2_pci",
	.id			= UCLASS_PCI,
	.of_match		= rcar_gen2_pci_ids,
	.ops			= &rcar_gen2_pci_ops,
	.probe			= rcar_gen2_pci_probe,
	.ofdata_to_platdata	= rcar_gen2_pci_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct rcar_gen2_pci_priv),
};
