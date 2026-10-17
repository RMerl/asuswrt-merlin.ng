// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe driver for Marvell MVEBU SoCs
 *
 * Based on Barebox drivers/pci/pci-mvebu.c
 *
 * Ported to U-Boot by:
 * Anton Schubert <anton.schubert@gmx.de>
 * Stefan Roese <sr@denx.de>
 * Pali Rohár <pali@kernel.org>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/mbus.h>
#include <linux/printk.h>
#include <linux/sizes.h>

/* PCIe unit register offsets */
#define MVPCIE_ROOT_PORT_PCI_CFG_OFF	0x0000
#define MVPCIE_ROOT_PORT_PCI_EXP_OFF	0x0060
#define MVPCIE_BAR_LO_OFF(n)		(0x0010 + ((n) << 3))
#define MVPCIE_BAR_HI_OFF(n)		(0x0014 + ((n) << 3))
#define MVPCIE_BAR_CTRL_OFF(n)		(0x1804 + (((n) - 1) * 4))
#define MVPCIE_WIN04_CTRL_OFF(n)	(0x1820 + ((n) << 4))
#define MVPCIE_WIN04_BASE_OFF(n)	(0x1824 + ((n) << 4))
#define MVPCIE_WIN04_REMAP_OFF(n)	(0x182c + ((n) << 4))
#define MVPCIE_WIN5_CTRL_OFF		0x1880
#define MVPCIE_WIN5_BASE_OFF		0x1884
#define MVPCIE_WIN5_REMAP_OFF		0x188c
#define MVPCIE_CONF_ADDR_OFF		0x18f8
#define MVPCIE_CONF_DATA_OFF		0x18fc
#define MVPCIE_CTRL_OFF			0x1a00
#define  MVPCIE_CTRL_RC_MODE		BIT(1)
#define MVPCIE_STAT_OFF			0x1a04
#define  MVPCIE_STAT_BUS		(0xff << 8)
#define  MVPCIE_STAT_DEV		(0x1f << 16)
#define  MVPCIE_STAT_LINK_DOWN		BIT(0)

#define LINK_WAIT_RETRIES	100
#define LINK_WAIT_TIMEOUT	1000

struct mvebu_pcie {
	struct pci_controller hose;
	void __iomem *base;
	void __iomem *membase;
	struct resource mem;
	void __iomem *iobase;
	struct resource io;
	struct gpio_desc reset_gpio;
	u32 intregs;
	u32 port;
	u32 lane;
	bool is_x4;
	int devfn;
	int sec_busno;
	char name[16];
	unsigned int mem_target;
	unsigned int mem_attr;
	unsigned int io_target;
	unsigned int io_attr;
	u32 cfgcache[(0x3c - 0x10) / 4];
};

static inline bool mvebu_pcie_link_up(struct mvebu_pcie *pcie)
{
	u32 val;
	val = readl(pcie->base + MVPCIE_STAT_OFF);
	return !(val & MVPCIE_STAT_LINK_DOWN);
}

static void mvebu_pcie_wait_for_link(struct mvebu_pcie *pcie)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_WAIT_RETRIES; retries++) {
		if (mvebu_pcie_link_up(pcie)) {
			printf("%s: Link up\n", pcie->name);
			return;
		}

		udelay(LINK_WAIT_TIMEOUT);
	}

	printf("%s: Link down\n", pcie->name);
}

static void mvebu_pcie_set_local_bus_nr(struct mvebu_pcie *pcie, int busno)
{
	u32 stat;

	stat = readl(pcie->base + MVPCIE_STAT_OFF);
	stat &= ~MVPCIE_STAT_BUS;
	stat |= busno << 8;
	writel(stat, pcie->base + MVPCIE_STAT_OFF);
}

static void mvebu_pcie_set_local_dev_nr(struct mvebu_pcie *pcie, int devno)
{
	u32 stat;

	stat = readl(pcie->base + MVPCIE_STAT_OFF);
	stat &= ~MVPCIE_STAT_DEV;
	stat |= devno << 16;
	writel(stat, pcie->base + MVPCIE_STAT_OFF);
}

static inline struct mvebu_pcie *hose_to_pcie(struct pci_controller *hose)
{
	return container_of(hose, struct mvebu_pcie, hose);
}

static bool mvebu_pcie_valid_addr(struct mvebu_pcie *pcie,
				  int busno, int dev, int func)
{
	/* On the root bus is only one PCI Bridge */
	if (busno == 0 && (dev != 0 || func != 0))
		return false;

	/* Access to other buses is possible when link is up */
	if (busno != 0 && !mvebu_pcie_link_up(pcie))
		return false;

	/* On secondary bus can be only one PCIe device */
	if (busno == pcie->sec_busno && dev != 0)
		return false;

	return true;
}

static int mvebu_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_plat(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	u32 addr, data;

	debug("PCIE CFG read: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!mvebu_pcie_valid_addr(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		debug("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	/*
	 * The configuration space of the PCI Bridge on the root bus (zero) is
	 * of Type 0 but the BAR registers (including ROM BAR) don't have the
	 * same meaning as in the PCIe specification. Therefore do not access
	 * BAR registers and non-common registers (those which have different
	 * meaning for Type 0 and Type 1 config space) of the PCI Bridge and
	 * instead read their content from driver virtual cfgcache[].
	 */
	if (busno == 0 && ((offset >= 0x10 && offset < 0x34) ||
			   (offset >= 0x38 && offset < 0x3c))) {
		data = pcie->cfgcache[(offset - 0x10) / 4];
		debug("(addr,size,val)=(0x%04x, %d, 0x%08x) from cfgcache\n",
		      offset, size, data);
		*valuep = pci_conv_32_to_size(data, offset, size);
		return 0;
	}

	/*
	 * PCI bridge is device 0 at the root bus (zero) but mvebu has it
	 * mapped on secondary bus with device number 1.
	 */
	if (busno == 0)
		addr = PCI_CONF1_EXT_ADDRESS(pcie->sec_busno, 1, 0, offset);
	else
		addr = PCI_CONF1_EXT_ADDRESS(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	/* write address */
	writel(addr, pcie->base + MVPCIE_CONF_ADDR_OFF);

	/* read data */
	switch (size) {
	case PCI_SIZE_8:
		data = readb(pcie->base + MVPCIE_CONF_DATA_OFF + (offset & 3));
		break;
	case PCI_SIZE_16:
		data = readw(pcie->base + MVPCIE_CONF_DATA_OFF + (offset & 2));
		break;
	case PCI_SIZE_32:
		data = readl(pcie->base + MVPCIE_CONF_DATA_OFF);
		break;
	default:
		return -EINVAL;
	}

	if (busno == 0 && (offset & ~3) == (PCI_HEADER_TYPE & ~3)) {
		/*
		 * Change Header Type of PCI Bridge device to Type 1
		 * (0x01, used by PCI Bridges) because mvebu reports
		 * Type 0 (0x00, used by Upstream and Endpoint devices).
		 */
		data = pci_conv_size_to_32(data, 0, offset, size);
		data &= ~0x007f0000;
		data |= PCI_HEADER_TYPE_BRIDGE << 16;
		data = pci_conv_32_to_size(data, offset, size);
	}

	debug("(addr,size,val)=(0x%04x, %d, 0x%08x)\n", offset, size, data);
	*valuep = data;

	return 0;
}

static int mvebu_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_plat(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	u32 addr, data;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,size,val)=(0x%04x, %d, 0x%08lx)\n", offset, size, value);

	if (!mvebu_pcie_valid_addr(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		debug("- out of range\n");
		return 0;
	}

	/*
	 * As explained in mvebu_pcie_read_config(), PCI Bridge Type 1 specific
	 * config registers are not available, so we write their content only
	 * into driver virtual cfgcache[].
	 * And as explained in mvebu_pcie_probe(), mvebu has its own specific
	 * way for configuring secondary bus number.
	 */
	if (busno == 0 && ((offset >= 0x10 && offset < 0x34) ||
			   (offset >= 0x38 && offset < 0x3c))) {
		debug("Writing to cfgcache only\n");
		data = pcie->cfgcache[(offset - 0x10) / 4];
		data = pci_conv_size_to_32(data, value, offset, size);
		/* mvebu PCI bridge does not have configurable bars */
		if ((offset & ~3) == PCI_BASE_ADDRESS_0 ||
		    (offset & ~3) == PCI_BASE_ADDRESS_1 ||
		    (offset & ~3) == PCI_ROM_ADDRESS1)
			data = 0x0;
		pcie->cfgcache[(offset - 0x10) / 4] = data;
		/* mvebu has its own way how to set PCI secondary bus number */
		if (offset == PCI_SECONDARY_BUS ||
		    (offset == PCI_PRIMARY_BUS && size != PCI_SIZE_8)) {
			pcie->sec_busno = (data >> 8) & 0xff;
			mvebu_pcie_set_local_bus_nr(pcie, pcie->sec_busno);
			debug("Secondary bus number was changed to %d\n",
			      pcie->sec_busno);
		}
		return 0;
	}

	/*
	 * PCI bridge is device 0 at the root bus (zero) but mvebu has it
	 * mapped on secondary bus with device number 1.
	 */
	if (busno == 0)
		addr = PCI_CONF1_EXT_ADDRESS(pcie->sec_busno, 1, 0, offset);
	else
		addr = PCI_CONF1_EXT_ADDRESS(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	/* write address */
	writel(addr, pcie->base + MVPCIE_CONF_ADDR_OFF);

	/* write data */
	switch (size) {
	case PCI_SIZE_8:
		writeb(value, pcie->base + MVPCIE_CONF_DATA_OFF + (offset & 3));
		break;
	case PCI_SIZE_16:
		writew(value, pcie->base + MVPCIE_CONF_DATA_OFF + (offset & 2));
		break;
	case PCI_SIZE_32:
		writel(value, pcie->base + MVPCIE_CONF_DATA_OFF);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * Setup PCIE BARs and Address Decode Wins:
 * BAR[0] -> internal registers
 * BAR[1] -> covers all DRAM banks
 * BAR[2] -> disabled
 * WIN[0-3] -> DRAM bank[0-3]
 */
static void mvebu_pcie_setup_wins(struct mvebu_pcie *pcie)
{
	const struct mbus_dram_target_info *dram = mvebu_mbus_dram_info();
	u32 size;
	int i;

	/* First, disable and clear BARs and windows. */
	for (i = 1; i < 3; i++) {
		writel(0, pcie->base + MVPCIE_BAR_CTRL_OFF(i));
		writel(0, pcie->base + MVPCIE_BAR_LO_OFF(i));
		writel(0, pcie->base + MVPCIE_BAR_HI_OFF(i));
	}

	for (i = 0; i < 5; i++) {
		writel(0, pcie->base + MVPCIE_WIN04_CTRL_OFF(i));
		writel(0, pcie->base + MVPCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + MVPCIE_WIN04_REMAP_OFF(i));
	}

	writel(0, pcie->base + MVPCIE_WIN5_CTRL_OFF);
	writel(0, pcie->base + MVPCIE_WIN5_BASE_OFF);
	writel(0, pcie->base + MVPCIE_WIN5_REMAP_OFF);

	/* Setup windows for DDR banks. Count total DDR size on the fly. */
	size = 0;
	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		writel(cs->base & 0xffff0000,
		       pcie->base + MVPCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + MVPCIE_WIN04_REMAP_OFF(i));
		writel(((cs->size - 1) & 0xffff0000) |
		       (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       pcie->base + MVPCIE_WIN04_CTRL_OFF(i));

		size += cs->size;
	}

	/* Round up 'size' to the nearest power of two. */
	if ((size & (size - 1)) != 0)
		size = 1 << fls(size);

	/* Setup BAR[1] to all DRAM banks. */
	writel(dram->cs[0].base | 0xc, pcie->base + MVPCIE_BAR_LO_OFF(1));
	writel(0, pcie->base + MVPCIE_BAR_HI_OFF(1));
	writel(((size - 1) & 0xffff0000) | 0x1,
	       pcie->base + MVPCIE_BAR_CTRL_OFF(1));

	/* Setup BAR[0] to internal registers. */
	writel(pcie->intregs, pcie->base + MVPCIE_BAR_LO_OFF(0));
	writel(0, pcie->base + MVPCIE_BAR_HI_OFF(0));
}

/* Only enable PCIe link, do not setup it */
static int mvebu_pcie_enable_link(struct mvebu_pcie *pcie, ofnode node)
{
	struct reset_ctl rst;
	int ret;

	ret = reset_get_by_index_nodev(node, 0, &rst);
	if (ret == -ENOENT) {
		return 0;
	} else if (ret < 0) {
		printf("%s: cannot get reset controller: %d\n", pcie->name, ret);
		return ret;
	}

	ret = reset_request(&rst);
	if (ret) {
		printf("%s: cannot request reset controller: %d\n", pcie->name, ret);
		return ret;
	}

	ret = reset_deassert(&rst);
	reset_free(&rst);
	if (ret) {
		printf("%s: cannot enable PCIe port: %d\n", pcie->name, ret);
		return ret;
	}

	return 0;
}

/* Setup PCIe link but do not enable it */
static void mvebu_pcie_setup_link(struct mvebu_pcie *pcie)
{
	u32 reg;

	/* Setup PCIe controller to Root Complex mode */
	reg = readl(pcie->base + MVPCIE_CTRL_OFF);
	reg |= MVPCIE_CTRL_RC_MODE;
	writel(reg, pcie->base + MVPCIE_CTRL_OFF);

	/*
	 * Set Maximum Link Width to X1 or X4 in Root Port's PCIe Link
	 * Capability register. This register is defined by PCIe specification
	 * as read-only but this mvebu controller has it as read-write and must
	 * be set to number of SerDes PCIe lanes (1 or 4). If this register is
	 * not set correctly then link with endpoint card is not established.
	 */
	reg = readl(pcie->base + MVPCIE_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_LNKCAP);
	reg &= ~PCI_EXP_LNKCAP_MLW;
	reg |= (pcie->is_x4 ? 4 : 1) << 4;
	writel(reg, pcie->base + MVPCIE_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_LNKCAP);
}

static int mvebu_pcie_probe(struct udevice *dev)
{
	struct mvebu_pcie *pcie = dev_get_plat(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	u32 reg;
	int ret;

	/* Request for optional PERST# GPIO */
	ret = gpio_request_by_name(dev, "reset-gpios", 0, &pcie->reset_gpio, GPIOD_IS_OUT);
	if (ret && ret != -ENOENT) {
		printf("%s: unable to request reset-gpios: %d\n", pcie->name, ret);
		return ret;
	}

	/*
	 * Change Class Code of PCI Bridge device to PCI Bridge (0x600400)
	 * because default value is Memory controller (0x508000) which
	 * U-Boot cannot recognize as P2P Bridge.
	 *
	 * Note that this mvebu PCI Bridge does not have compliant Type 1
	 * Configuration Space. Header Type is reported as Type 0 and it
	 * has format of Type 0 config space.
	 *
	 * Moreover Type 0 BAR registers (ranges 0x10 - 0x28 and 0x30 - 0x34)
	 * have the same format in Marvell's specification as in PCIe
	 * specification, but their meaning is totally different and they do
	 * different things: they are aliased into internal mvebu registers
	 * (e.g. MVPCIE_BAR_LO_OFF) and these should not be changed or
	 * reconfigured by pci device drivers.
	 *
	 * So our driver converts Type 0 config space to Type 1 and reports
	 * Header Type as Type 1. Access to BAR registers and to non-existent
	 * Type 1 registers is redirected to the virtual cfgcache[] buffer,
	 * which avoids changing unrelated registers.
	 */
	reg = readl(pcie->base + MVPCIE_ROOT_PORT_PCI_CFG_OFF + PCI_CLASS_REVISION);
	reg &= ~0xffffff00;
	reg |= PCI_CLASS_BRIDGE_PCI_NORMAL << 8;
	writel(reg, pcie->base + MVPCIE_ROOT_PORT_PCI_CFG_OFF + PCI_CLASS_REVISION);

	/*
	 * mvebu uses local bus number and local device number to determinate
	 * type of config request. Type 0 is used if target bus number equals
	 * local bus number and target device number differs from local device
	 * number. Type 1 is used if target bus number differs from local bus
	 * number. And when target bus number equals local bus number and
	 * target device equals local device number then request is routed to
	 * PCI Bridge which represent local PCIe Root Port.
	 *
	 * It means that PCI root and secondary buses shares one bus number
	 * which is configured via local bus number. Determination if config
	 * request should go to root or secondary bus is done based on local
	 * device number.
	 *
	 * PCIe is point-to-point bus, so at secondary bus is always exactly one
	 * device with number 0. So set local device number to 1, it would not
	 * conflict with any device on secondary bus number and will ensure that
	 * accessing secondary bus and all buses behind secondary would work
	 * automatically and correctly. Therefore this configuration of local
	 * device number implies that setting of local bus number configures
	 * secondary bus number. Set it to 0 as U-Boot CONFIG_PCI_PNP code will
	 * later configure it via config write requests to the correct value.
	 * mvebu_pcie_write_config() catches config write requests which tries
	 * to change secondary bus number and correctly updates local bus number
	 * based on new secondary bus number.
	 *
	 * With this configuration is PCI Bridge available at secondary bus as
	 * device number 1. But it must be available at root bus (zero) as device
	 * number 0. So in mvebu_pcie_read_config() and mvebu_pcie_write_config()
	 * functions rewrite address to the real one when accessing the root bus.
	 */
	mvebu_pcie_set_local_bus_nr(pcie, 0);
	mvebu_pcie_set_local_dev_nr(pcie, 1);

	/*
	 * Kirkwood arch code already maps mbus windows for PCIe IO and MEM.
	 * So skip calling mvebu_mbus_add_window_by_id() function as it would
	 * fail on error "conflicts with another window" which means conflict
	 * with existing PCIe window mappings.
	 */
#ifndef CONFIG_ARCH_KIRKWOOD
	if (resource_size(&pcie->mem) &&
	    mvebu_mbus_add_window_by_id(pcie->mem_target, pcie->mem_attr,
					(phys_addr_t)pcie->mem.start,
					resource_size(&pcie->mem))) {
		printf("%s: unable to add mbus window for mem at %08x+%08x\n",
		       pcie->name,
		       (u32)pcie->mem.start, (unsigned)resource_size(&pcie->mem));
		pcie->mem.start = 0;
		pcie->mem.end = -1;
	}

	if (resource_size(&pcie->io) &&
	    mvebu_mbus_add_window_by_id(pcie->io_target, pcie->io_attr,
					(phys_addr_t)pcie->io.start,
					resource_size(&pcie->io))) {
		printf("%s: unable to add mbus window for IO at %08x+%08x\n",
		       pcie->name,
		       (u32)pcie->io.start, (unsigned)resource_size(&pcie->io));
		pcie->io.start = 0;
		pcie->io.end = -1;
	}
#endif

	/* Setup windows and configure host bridge */
	mvebu_pcie_setup_wins(pcie);

	/* PCI memory space */
	pci_set_region(hose->regions + 0, pcie->mem.start,
		       pcie->mem.start, resource_size(&pcie->mem), PCI_REGION_MEM);
	hose->region_count = 1;

	if (resource_size(&pcie->mem)) {
		pci_set_region(hose->regions + hose->region_count,
			       pcie->mem.start, pcie->mem.start,
			       resource_size(&pcie->mem),
			       PCI_REGION_MEM);
		hose->region_count++;
	}

	if (resource_size(&pcie->io)) {
		pci_set_region(hose->regions + hose->region_count,
			       pcie->io.start, pcie->io.start,
			       resource_size(&pcie->io),
			       PCI_REGION_IO);
		hose->region_count++;
	}

	/* PCI Bridge support 32-bit I/O and 64-bit prefetch mem addressing */
	pcie->cfgcache[(PCI_IO_BASE - 0x10) / 4] =
		PCI_IO_RANGE_TYPE_32 | (PCI_IO_RANGE_TYPE_32 << 8);
	pcie->cfgcache[(PCI_PREF_MEMORY_BASE - 0x10) / 4] =
		PCI_PREF_RANGE_TYPE_64 | (PCI_PREF_RANGE_TYPE_64 << 16);

	/* Release PERST# via GPIO when it was defined */
	if (dm_gpio_is_valid(&pcie->reset_gpio))
		dm_gpio_set_value(&pcie->reset_gpio, 0);

	mvebu_pcie_wait_for_link(pcie);

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

static int mvebu_pcie_port_parse_dt(ofnode node, ofnode parent, struct mvebu_pcie *pcie)
{
	struct fdt_pci_addr pci_addr;
	const u32 *addr;
	u32 num_lanes;
	int ret = 0;
	int len;

	/* Get port number, lane number and memory target / attr */
	if (ofnode_read_u32(node, "marvell,pcie-port",
			    &pcie->port)) {
		ret = -ENODEV;
		goto err;
	}

	if (ofnode_read_u32(node, "marvell,pcie-lane", &pcie->lane))
		pcie->lane = 0;

	sprintf(pcie->name, "pcie%d.%d", pcie->port, pcie->lane);

	if (!ofnode_read_u32(node, "num-lanes", &num_lanes) && num_lanes == 4)
		pcie->is_x4 = true;

	/* devfn is in bits [15:8], see PCI_DEV usage */
	ret = ofnode_read_pci_addr(node, FDT_PCI_SPACE_CONFIG, "reg", &pci_addr,
				   NULL);
	if (ret < 0) {
		printf("%s: property \"reg\" is invalid\n", pcie->name);
		goto err;
	}
	pcie->devfn = pci_addr.phys_hi & 0xff00;

	ret = mvebu_get_tgt_attr(parent, pcie->devfn,
				 IORESOURCE_MEM,
				 &pcie->mem_target, &pcie->mem_attr);
	if (ret < 0) {
		printf("%s: cannot get tgt/attr for mem window\n", pcie->name);
		goto err;
	}

	ret = mvebu_get_tgt_attr(parent, pcie->devfn,
				 IORESOURCE_IO,
				 &pcie->io_target, &pcie->io_attr);
	if (ret < 0) {
		printf("%s: cannot get tgt/attr for IO window\n", pcie->name);
		goto err;
	}

	/* Parse PCIe controller register base from DT */
	addr = ofnode_get_property(node, "assigned-addresses", &len);
	if (!addr) {
		printf("%s: property \"assigned-addresses\" not found\n", pcie->name);
		ret = -FDT_ERR_NOTFOUND;
		goto err;
	}

	pcie->base = (void *)(u32)ofnode_translate_address(node, addr);
	pcie->intregs = (u32)pcie->base - fdt32_to_cpu(addr[2]);

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
	.plat_auto	= sizeof(struct mvebu_pcie),
};

/*
 * Use a MISC device to bind the n instances (child nodes) of the
 * PCIe base controller in UCLASS_PCI.
 */
static int mvebu_pcie_bind(struct udevice *parent)
{
	struct mvebu_pcie **ports_pcie;
	struct mvebu_pcie *pcie;
	struct uclass_driver *drv;
	struct udevice *dev;
	struct resource mem;
	struct resource io;
	int ports_count, i;
	ofnode *ports_nodes;
	ofnode subnode;

	/* Lookup pci driver */
	drv = lists_uclass_lookup(UCLASS_PCI);
	if (!drv) {
		puts("Cannot find PCI driver\n");
		return -ENOENT;
	}

	ports_count = ofnode_get_child_count(dev_ofnode(parent));
	ports_pcie = calloc(ports_count, sizeof(*ports_pcie));
	ports_nodes = calloc(ports_count, sizeof(*ports_nodes));
	if (!ports_pcie || !ports_nodes) {
		free(ports_pcie);
		free(ports_nodes);
		return -ENOMEM;
	}
	ports_count = 0;

#ifdef CONFIG_ARCH_KIRKWOOD
	mem.start = KW_DEFADR_PCI_MEM;
	mem.end = KW_DEFADR_PCI_MEM + KW_DEFADR_PCI_MEM_SIZE - 1;
	io.start = KW_DEFADR_PCI_IO;
	io.end = KW_DEFADR_PCI_IO + KW_DEFADR_PCI_IO_SIZE - 1;
#else
	mem.start = MBUS_PCI_MEM_BASE;
	mem.end = MBUS_PCI_MEM_BASE + MBUS_PCI_MEM_SIZE - 1;
	io.start = MBUS_PCI_IO_BASE;
	io.end = MBUS_PCI_IO_BASE + MBUS_PCI_IO_SIZE - 1;
#endif

	/* First phase: Fill mvebu_pcie struct for each port */
	ofnode_for_each_subnode(subnode, dev_ofnode(parent)) {
		if (!ofnode_is_enabled(subnode))
			continue;

		pcie = calloc(1, sizeof(*pcie));
		if (!pcie)
			continue;

		if (mvebu_pcie_port_parse_dt(subnode, dev_ofnode(parent), pcie) < 0) {
			free(pcie);
			continue;
		}

		/*
		 * MVEBU PCIe controller needs MEMORY and I/O BARs to be mapped
		 * into SoCs address space. Each controller will map 128M of MEM
		 * and 64K of I/O space when registered.
		 */

		if (resource_size(&mem) >= SZ_128M) {
			pcie->mem.start = mem.start;
			pcie->mem.end = mem.start + SZ_128M - 1;
			mem.start += SZ_128M;
		} else {
			printf("%s: unable to assign mbus window for mem\n", pcie->name);
			pcie->mem.start = 0;
			pcie->mem.end = -1;
		}

		if (resource_size(&io) >= SZ_64K) {
			pcie->io.start = io.start;
			pcie->io.end = io.start + SZ_64K - 1;
			io.start += SZ_64K;
		} else {
			printf("%s: unable to assign mbus window for io\n", pcie->name);
			pcie->io.start = 0;
			pcie->io.end = -1;
		}

		ports_pcie[ports_count] = pcie;
		ports_nodes[ports_count] = subnode;
		ports_count++;
	}

	/* Second phase: Setup all PCIe links (do not enable them yet) */
	for (i = 0; i < ports_count; i++)
		mvebu_pcie_setup_link(ports_pcie[i]);

	/* Third phase: Enable all PCIe links and create for each UCLASS_PCI device */
	for (i = 0; i < ports_count; i++) {
		pcie = ports_pcie[i];
		subnode = ports_nodes[i];

		/*
		 * PCIe link can be enabled only after all PCIe links were
		 * properly configured. This is because more PCIe links shares
		 * one enable bit and some PCIe links cannot be enabled
		 * individually.
		 */
		if (mvebu_pcie_enable_link(pcie, subnode) < 0) {
			free(pcie);
			continue;
		}

		/* Create child device UCLASS_PCI and bind it */
		device_bind(parent, &pcie_mvebu_drv, pcie->name, pcie, subnode,
			    &dev);
	}

	free(ports_pcie);
	free(ports_nodes);

	return 0;
}

static const struct udevice_id mvebu_pcie_ids[] = {
	{ .compatible = "marvell,armada-xp-pcie" },
	{ .compatible = "marvell,armada-370-pcie" },
	{ .compatible = "marvell,kirkwood-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_mvebu_base) = {
	.name			= "pcie_mvebu_base",
	.id			= UCLASS_MISC,
	.of_match		= mvebu_pcie_ids,
	.bind			= mvebu_pcie_bind,
};
