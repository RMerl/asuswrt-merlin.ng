// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe driver for Marvell MVEBU SoCs
 *
 * Based on Barebox drivers/pci/pci-mvebu.c
 *
 * Ported to U-Boot by:
 * Anton Schubert <anton.schubert@gmx.de>
 * Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/mbus.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCIe unit register offsets */
#define SELECT(x, n)			((x >> n) & 1UL)

#define PCIE_DEV_ID_OFF			0x0000
#define PCIE_CMD_OFF			0x0004
#define PCIE_DEV_REV_OFF		0x0008
#define  PCIE_BAR_LO_OFF(n)		(0x0010 + ((n) << 3))
#define  PCIE_BAR_HI_OFF(n)		(0x0014 + ((n) << 3))
#define PCIE_CAPAB_OFF			0x0060
#define PCIE_CTRL_STAT_OFF		0x0068
#define PCIE_HEADER_LOG_4_OFF		0x0128
#define  PCIE_BAR_CTRL_OFF(n)		(0x1804 + (((n) - 1) * 4))
#define  PCIE_WIN04_CTRL_OFF(n)		(0x1820 + ((n) << 4))
#define  PCIE_WIN04_BASE_OFF(n)		(0x1824 + ((n) << 4))
#define  PCIE_WIN04_REMAP_OFF(n)	(0x182c + ((n) << 4))
#define PCIE_WIN5_CTRL_OFF		0x1880
#define PCIE_WIN5_BASE_OFF		0x1884
#define PCIE_WIN5_REMAP_OFF		0x188c
#define PCIE_CONF_ADDR_OFF		0x18f8
#define  PCIE_CONF_ADDR_EN		BIT(31)
#define  PCIE_CONF_REG(r)		((((r) & 0xf00) << 16) | ((r) & 0xfc))
#define  PCIE_CONF_BUS(b)		(((b) & 0xff) << 16)
#define  PCIE_CONF_DEV(d)		(((d) & 0x1f) << 11)
#define  PCIE_CONF_FUNC(f)		(((f) & 0x7) << 8)
#define  PCIE_CONF_ADDR(dev, reg) \
	(PCIE_CONF_BUS(PCI_BUS(dev)) | PCIE_CONF_DEV(PCI_DEV(dev))    | \
	 PCIE_CONF_FUNC(PCI_FUNC(dev)) | PCIE_CONF_REG(reg) | \
	 PCIE_CONF_ADDR_EN)
#define PCIE_CONF_DATA_OFF		0x18fc
#define PCIE_MASK_OFF			0x1910
#define  PCIE_MASK_ENABLE_INTS          (0xf << 24)
#define PCIE_CTRL_OFF			0x1a00
#define  PCIE_CTRL_X1_MODE		BIT(0)
#define PCIE_STAT_OFF			0x1a04
#define  PCIE_STAT_BUS                  (0xff << 8)
#define  PCIE_STAT_DEV                  (0x1f << 16)
#define  PCIE_STAT_LINK_DOWN		BIT(0)
#define PCIE_DEBUG_CTRL			0x1a60
#define  PCIE_DEBUG_SOFT_RESET		BIT(20)

struct mvebu_pcie {
	struct pci_controller hose;
	void __iomem *base;
	void __iomem *membase;
	struct resource mem;
	void __iomem *iobase;
	u32 port;
	u32 lane;
	int devfn;
	u32 lane_mask;
	pci_dev_t dev;
	char name[16];
	unsigned int mem_target;
	unsigned int mem_attr;
};

/*
 * MVEBU PCIe controller needs MEMORY and I/O BARs to be mapped
 * into SoCs address space. Each controller will map 128M of MEM
 * and 64K of I/O space when registered.
 */
static void __iomem *mvebu_pcie_membase = (void __iomem *)MBUS_PCI_MEM_BASE;
#define PCIE_MEM_SIZE	(128 << 20)

static inline bool mvebu_pcie_link_up(struct mvebu_pcie *pcie)
{
	u32 val;
	val = readl(pcie->base + PCIE_STAT_OFF);
	return !(val & PCIE_STAT_LINK_DOWN);
}

static void mvebu_pcie_set_local_bus_nr(struct mvebu_pcie *pcie, int busno)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	stat &= ~PCIE_STAT_BUS;
	stat |= busno << 8;
	writel(stat, pcie->base + PCIE_STAT_OFF);
}

static void mvebu_pcie_set_local_dev_nr(struct mvebu_pcie *pcie, int devno)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	stat &= ~PCIE_STAT_DEV;
	stat |= devno << 16;
	writel(stat, pcie->base + PCIE_STAT_OFF);
}

static int mvebu_pcie_get_local_bus_nr(struct mvebu_pcie *pcie)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	return (stat & PCIE_STAT_BUS) >> 8;
}

static int mvebu_pcie_get_local_dev_nr(struct mvebu_pcie *pcie)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	return (stat & PCIE_STAT_DEV) >> 16;
}

static inline struct mvebu_pcie *hose_to_pcie(struct pci_controller *hose)
{
	return container_of(hose, struct mvebu_pcie, hose);
}

static int mvebu_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_platdata(bus);
	int local_bus = PCI_BUS(pcie->dev);
	int local_dev = PCI_DEV(pcie->dev);
	u32 reg;
	u32 data;

	debug("PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(bdf) == local_bus && PCI_DEV(bdf) != local_dev) {
		if (local_dev == 0 && PCI_DEV(bdf) != 1) {
			debug("- out of range\n");
			/*
			 * If local dev is 0, the first other dev can
			 * only be 1
			 */
			*valuep = pci_get_ff(size);
			return 0;
		} else if (local_dev != 0 && PCI_DEV(bdf) != 0) {
			debug("- out of range\n");
			/*
			 * If local dev is not 0, the first other dev can
			 * only be 0
			 */
			*valuep = pci_get_ff(size);
			return 0;
		}
	}

	/* write address */
	reg = PCIE_CONF_ADDR(bdf, offset);
	writel(reg, pcie->base + PCIE_CONF_ADDR_OFF);
	data = readl(pcie->base + PCIE_CONF_DATA_OFF);
	debug("(addr,val)=(0x%04x, 0x%08x)\n", offset, data);
	*valuep = pci_conv_32_to_size(data, offset, size);

	return 0;
}

static int mvebu_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_platdata(bus);
	int local_bus = PCI_BUS(pcie->dev);
	int local_dev = PCI_DEV(pcie->dev);
	u32 data;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(bdf) == local_bus && PCI_DEV(bdf) != local_dev) {
		if (local_dev == 0 && PCI_DEV(bdf) != 1) {
			/*
			 * If local dev is 0, the first other dev can
			 * only be 1
			 */
			return 0;
		} else if (local_dev != 0 && PCI_DEV(bdf) != 0) {
			/*
			 * If local dev is not 0, the first other dev can
			 * only be 0
			 */
			return 0;
		}
	}

	writel(PCIE_CONF_ADDR(bdf, offset), pcie->base + PCIE_CONF_ADDR_OFF);
	data = pci_conv_size_to_32(0, value, offset, size);
	writel(data, pcie->base + PCIE_CONF_DATA_OFF);

	return 0;
}

/*
 * Setup PCIE BARs and Address Decode Wins:
 * BAR[0,2] -> disabled, BAR[1] -> covers all DRAM banks
 * WIN[0-3] -> DRAM bank[0-3]
 */
static void mvebu_pcie_setup_wins(struct mvebu_pcie *pcie)
{
	const struct mbus_dram_target_info *dram = mvebu_mbus_dram_info();
	u32 size;
	int i;

	/* First, disable and clear BARs and windows. */
	for (i = 1; i < 3; i++) {
		writel(0, pcie->base + PCIE_BAR_CTRL_OFF(i));
		writel(0, pcie->base + PCIE_BAR_LO_OFF(i));
		writel(0, pcie->base + PCIE_BAR_HI_OFF(i));
	}

	for (i = 0; i < 5; i++) {
		writel(0, pcie->base + PCIE_WIN04_CTRL_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_REMAP_OFF(i));
	}

	writel(0, pcie->base + PCIE_WIN5_CTRL_OFF);
	writel(0, pcie->base + PCIE_WIN5_BASE_OFF);
	writel(0, pcie->base + PCIE_WIN5_REMAP_OFF);

	/* Setup windows for DDR banks. Count total DDR size on the fly. */
	size = 0;
	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		writel(cs->base & 0xffff0000,
		       pcie->base + PCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_REMAP_OFF(i));
		writel(((cs->size - 1) & 0xffff0000) |
		       (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       pcie->base + PCIE_WIN04_CTRL_OFF(i));

		size += cs->size;
	}

	/* Round up 'size' to the nearest power of two. */
	if ((size & (size - 1)) != 0)
		size = 1 << fls(size);

	/* Setup BAR[1] to all DRAM banks. */
	writel(dram->cs[0].base | 0xc, pcie->base + PCIE_BAR_LO_OFF(1));
	writel(0, pcie->base + PCIE_BAR_HI_OFF(1));
	writel(((size - 1) & 0xffff0000) | 0x1,
	       pcie->base + PCIE_BAR_CTRL_OFF(1));
}

static int mvebu_pcie_probe(struct udevice *dev)
{
	struct mvebu_pcie *pcie = dev_get_platdata(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	static int bus;
	u32 reg;

	debug("%s: PCIe %d.%d - up, base %08x\n", __func__,
	      pcie->port, pcie->lane, (u32)pcie->base);

	/* Read Id info and local bus/dev */
	debug("direct conf read %08x, local bus %d, local dev %d\n",
	      readl(pcie->base), mvebu_pcie_get_local_bus_nr(pcie),
	      mvebu_pcie_get_local_dev_nr(pcie));

	mvebu_pcie_set_local_bus_nr(pcie, bus);
	mvebu_pcie_set_local_dev_nr(pcie, 0);
	pcie->dev = PCI_BDF(bus, 0, 0);

	pcie->mem.start = (u32)mvebu_pcie_membase;
	pcie->mem.end = pcie->mem.start + PCIE_MEM_SIZE - 1;
	mvebu_pcie_membase += PCIE_MEM_SIZE;

	if (mvebu_mbus_add_window_by_id(pcie->mem_target, pcie->mem_attr,
					(phys_addr_t)pcie->mem.start,
					PCIE_MEM_SIZE)) {
		printf("PCIe unable to add mbus window for mem at %08x+%08x\n",
		       (u32)pcie->mem.start, PCIE_MEM_SIZE);
	}

	/* Setup windows and configure host bridge */
	mvebu_pcie_setup_wins(pcie);

	/* Master + slave enable. */
	reg = readl(pcie->base + PCIE_CMD_OFF);
	reg |= PCI_COMMAND_MEMORY;
	reg |= PCI_COMMAND_MASTER;
	reg |= BIT(10);		/* disable interrupts */
	writel(reg, pcie->base + PCIE_CMD_OFF);

	/* Set BAR0 to internal registers */
	writel(SOC_REGS_PHY_BASE, pcie->base + PCIE_BAR_LO_OFF(0));
	writel(0, pcie->base + PCIE_BAR_HI_OFF(0));

	/* PCI memory space */
	pci_set_region(hose->regions + 0, pcie->mem.start,
		       pcie->mem.start, PCIE_MEM_SIZE, PCI_REGION_MEM);
	pci_set_region(hose->regions + 1,
		       0, 0,
		       gd->ram_size,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
	hose->region_count = 2;

	bus++;

	return 0;
}

static int mvebu_pcie_port_parse_dt(ofnode node, struct mvebu_pcie *pcie)
{
	const u32 *addr;
	int len;

	addr = ofnode_get_property(node, "assigned-addresses", &len);
	if (!addr) {
		pr_err("property \"assigned-addresses\" not found");
		return -FDT_ERR_NOTFOUND;
	}

	pcie->base = (void *)(fdt32_to_cpu(addr[2]) + SOC_REGS_PHY_BASE);

	return 0;
}

#define DT_FLAGS_TO_TYPE(flags)       (((flags) >> 24) & 0x03)
#define    DT_TYPE_IO                 0x1
#define    DT_TYPE_MEM32              0x2
#define DT_CPUADDR_TO_TARGET(cpuaddr) (((cpuaddr) >> 56) & 0xFF)
#define DT_CPUADDR_TO_ATTR(cpuaddr)   (((cpuaddr) >> 48) & 0xFF)

static int mvebu_get_tgt_attr(ofnode node, int devfn,
			      unsigned long type,
			      unsigned int *tgt,
			      unsigned int *attr)
{
	const int na = 3, ns = 2;
	const __be32 *range;
	int rlen, nranges, rangesz, pna, i;

	*tgt = -1;
	*attr = -1;

	range = ofnode_get_property(node, "ranges", &rlen);
	if (!range)
		return -EINVAL;

	/*
	 * Linux uses of_n_addr_cells() to get the number of address cells
	 * here. Currently this function is only available in U-Boot when
	 * CONFIG_OF_LIVE is enabled. Until this is enabled for MVEBU in
	 * general, lets't hardcode the "pna" value in the U-Boot code.
	 */
	pna = 2; /* hardcoded for now because of lack of of_n_addr_cells() */
	rangesz = pna + na + ns;
	nranges = rlen / sizeof(__be32) / rangesz;

	for (i = 0; i < nranges; i++, range += rangesz) {
		u32 flags = of_read_number(range, 1);
		u32 slot = of_read_number(range + 1, 1);
		u64 cpuaddr = of_read_number(range + na, pna);
		unsigned long rtype;

		if (DT_FLAGS_TO_TYPE(flags) == DT_TYPE_IO)
			rtype = IORESOURCE_IO;
		else if (DT_FLAGS_TO_TYPE(flags) == DT_TYPE_MEM32)
			rtype = IORESOURCE_MEM;
		else
			continue;

		/*
		 * The Linux code used PCI_SLOT() here, which expects devfn
		 * in bits 7..0. PCI_DEV() in U-Boot is similar to PCI_SLOT(),
		 * only expects devfn in 15..8, where its saved in this driver.
		 */
		if (slot == PCI_DEV(devfn) && type == rtype) {
			*tgt = DT_CPUADDR_TO_TARGET(cpuaddr);
			*attr = DT_CPUADDR_TO_ATTR(cpuaddr);
			return 0;
		}
	}

	return -ENOENT;
}

static int mvebu_pcie_ofdata_to_platdata(struct udevice *dev)
{
	struct mvebu_pcie *pcie = dev_get_platdata(dev);
	int ret = 0;

	/* Get port number, lane number and memory target / attr */
	if (ofnode_read_u32(dev_ofnode(dev), "marvell,pcie-port",
			    &pcie->port)) {
		ret = -ENODEV;
		goto err;
	}

	if (ofnode_read_u32(dev_ofnode(dev), "marvell,pcie-lane", &pcie->lane))
		pcie->lane = 0;

	sprintf(pcie->name, "pcie%d.%d", pcie->port, pcie->lane);

	/* pci_get_devfn() returns devfn in bits 15..8, see PCI_DEV usage */
	pcie->devfn = pci_get_devfn(dev);
	if (pcie->devfn < 0) {
		ret = -ENODEV;
		goto err;
	}

	ret = mvebu_get_tgt_attr(dev_ofnode(dev->parent), pcie->devfn,
				 IORESOURCE_MEM,
				 &pcie->mem_target, &pcie->mem_attr);
	if (ret < 0) {
		printf("%s: cannot get tgt/attr for mem window\n", pcie->name);
		goto err;
	}

	/* Parse PCIe controller register base from DT */
	ret = mvebu_pcie_port_parse_dt(dev_ofnode(dev), pcie);
	if (ret < 0)
		goto err;

	/* Check link and skip ports that have no link */
	if (!mvebu_pcie_link_up(pcie)) {
		debug("%s: %s - down\n", __func__, pcie->name);
		ret = -ENODEV;
		goto err;
	}

	return 0;

err:
	return ret;
}

static const struct dm_pci_ops mvebu_pcie_ops = {
	.read_config	= mvebu_pcie_read_config,
	.write_config	= mvebu_pcie_write_config,
};

static struct driver pcie_mvebu_drv = {
	.name			= "pcie_mvebu",
	.id			= UCLASS_PCI,
	.ops			= &mvebu_pcie_ops,
	.probe			= mvebu_pcie_probe,
	.ofdata_to_platdata	= mvebu_pcie_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mvebu_pcie),
};

/*
 * Use a MISC device to bind the n instances (child nodes) of the
 * PCIe base controller in UCLASS_PCI.
 */
static int mvebu_pcie_bind(struct udevice *parent)
{
	struct mvebu_pcie *pcie;
	struct uclass_driver *drv;
	struct udevice *dev;
	ofnode subnode;

	/* Lookup eth driver */
	drv = lists_uclass_lookup(UCLASS_PCI);
	if (!drv) {
		puts("Cannot find PCI driver\n");
		return -ENOENT;
	}

	ofnode_for_each_subnode(subnode, dev_ofnode(parent)) {
		if (!ofnode_is_available(subnode))
			continue;

		pcie = calloc(1, sizeof(*pcie));
		if (!pcie)
			return -ENOMEM;

		/* Create child device UCLASS_PCI and bind it */
		device_bind_ofnode(parent, &pcie_mvebu_drv, pcie->name, pcie,
				   subnode, &dev);
	}

	return 0;
}

static const struct udevice_id mvebu_pcie_ids[] = {
	{ .compatible = "marvell,armada-xp-pcie" },
	{ .compatible = "marvell,armada-370-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_mvebu_base) = {
	.name			= "pcie_mvebu_base",
	.id			= UCLASS_MISC,
	.of_match		= mvebu_pcie_ids,
	.bind			= mvebu_pcie_bind,
};
