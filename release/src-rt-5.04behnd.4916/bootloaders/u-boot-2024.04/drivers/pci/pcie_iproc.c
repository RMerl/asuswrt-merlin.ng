// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020-2021 Broadcom
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <pci.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/log2.h>

#define EP_PERST_SOURCE_SELECT_SHIFT 2
#define EP_PERST_SOURCE_SELECT       BIT(EP_PERST_SOURCE_SELECT_SHIFT)
#define EP_MODE_SURVIVE_PERST_SHIFT  1
#define EP_MODE_SURVIVE_PERST        BIT(EP_MODE_SURVIVE_PERST_SHIFT)
#define RC_PCIE_RST_OUTPUT_SHIFT     0
#define RC_PCIE_RST_OUTPUT           BIT(RC_PCIE_RST_OUTPUT_SHIFT)

#define CFG_IND_ADDR_MASK            0x00001ffc

#define CFG_ADDR_CFG_ECAM_MASK       0xfffffffc
#define CFG_ADDR_CFG_TYPE_MASK       0x00000003

#define IPROC_PCI_PM_CAP             0x48
#define IPROC_PCI_PM_CAP_MASK        0xffff
#define IPROC_PCI_EXP_CAP            0xac

#define IPROC_PCIE_REG_INVALID       0xffff

#define PCI_EXP_TYPE_ROOT_PORT       0x4 /* Root Port */
#define PCI_EXP_RTCTL                28  /* Root Control */
/* CRS Software Visibility capability */
#define PCI_EXP_RTCAP_CRSVIS         0x0001

#define PCI_EXP_LNKSTA               18      /* Link Status */
#define PCI_EXP_LNKSTA_NLW           0x03f0  /* Negotiated Link Width */

#define PCIE_PHYLINKUP_SHIFT         3
#define PCIE_PHYLINKUP               BIT(PCIE_PHYLINKUP_SHIFT)
#define PCIE_DL_ACTIVE_SHIFT         2
#define PCIE_DL_ACTIVE               BIT(PCIE_DL_ACTIVE_SHIFT)

/* derive the enum index of the outbound/inbound mapping registers */
#define MAP_REG(base_reg, index)     ((base_reg) + (index) * 2)

/*
 * Maximum number of outbound mapping window sizes that can be supported by any
 * OARR/OMAP mapping pair
 */
#define MAX_NUM_OB_WINDOW_SIZES      4

#define OARR_VALID_SHIFT             0
#define OARR_VALID                   BIT(OARR_VALID_SHIFT)
#define OARR_SIZE_CFG_SHIFT          1

/*
 * Maximum number of inbound mapping region sizes that can be supported by an
 * IARR
 */
#define MAX_NUM_IB_REGION_SIZES      9

#define IMAP_VALID_SHIFT             0
#define IMAP_VALID                   BIT(IMAP_VALID_SHIFT)

#define APB_ERR_EN_SHIFT             0
#define APB_ERR_EN                   BIT(APB_ERR_EN_SHIFT)

/**
 * iProc PCIe host registers
 */
enum iproc_pcie_reg {
	/* clock/reset signal control */
	IPROC_PCIE_CLK_CTRL = 0,

	/*
	 * To allow MSI to be steered to an external MSI controller (e.g., ARM
	 * GICv3 ITS)
	 */
	IPROC_PCIE_MSI_GIC_MODE,

	/*
	 * IPROC_PCIE_MSI_BASE_ADDR and IPROC_PCIE_MSI_WINDOW_SIZE define the
	 * window where the MSI posted writes are written, for the writes to be
	 * interpreted as MSI writes.
	 */
	IPROC_PCIE_MSI_BASE_ADDR,
	IPROC_PCIE_MSI_WINDOW_SIZE,

	/*
	 * To hold the address of the register where the MSI writes are
	 * programed.  When ARM GICv3 ITS is used, this should be programmed
	 * with the address of the GITS_TRANSLATER register.
	 */
	IPROC_PCIE_MSI_ADDR_LO,
	IPROC_PCIE_MSI_ADDR_HI,

	/* enable MSI */
	IPROC_PCIE_MSI_EN_CFG,

	/* allow access to root complex configuration space */
	IPROC_PCIE_CFG_IND_ADDR,
	IPROC_PCIE_CFG_IND_DATA,

	/* allow access to device configuration space */
	IPROC_PCIE_CFG_ADDR,
	IPROC_PCIE_CFG_DATA,

	/* enable INTx */
	IPROC_PCIE_INTX_EN,
	IPROC_PCIE_INTX_CSR,

	/* outbound address mapping */
	IPROC_PCIE_OARR0,
	IPROC_PCIE_OMAP0,
	IPROC_PCIE_OARR1,
	IPROC_PCIE_OMAP1,
	IPROC_PCIE_OARR2,
	IPROC_PCIE_OMAP2,
	IPROC_PCIE_OARR3,
	IPROC_PCIE_OMAP3,

	/* inbound address mapping */
	IPROC_PCIE_IARR0,
	IPROC_PCIE_IMAP0,
	IPROC_PCIE_IARR1,
	IPROC_PCIE_IMAP1,
	IPROC_PCIE_IARR2,
	IPROC_PCIE_IMAP2,
	IPROC_PCIE_IARR3,
	IPROC_PCIE_IMAP3,
	IPROC_PCIE_IARR4,
	IPROC_PCIE_IMAP4,

	/* config read status */
	IPROC_PCIE_CFG_RD_STATUS,

	/* link status */
	IPROC_PCIE_LINK_STATUS,

	/* enable APB error for unsupported requests */
	IPROC_PCIE_APB_ERR_EN,

	/* Ordering Mode configuration registers */
	IPROC_PCIE_ORDERING_CFG,
	IPROC_PCIE_IMAP0_RO_CONTROL,
	IPROC_PCIE_IMAP1_RO_CONTROL,
	IPROC_PCIE_IMAP2_RO_CONTROL,
	IPROC_PCIE_IMAP3_RO_CONTROL,
	IPROC_PCIE_IMAP4_RO_CONTROL,

	/* total number of core registers */
	IPROC_PCIE_MAX_NUM_REG,
};

/* iProc PCIe PAXB v2 registers */
static const u16 iproc_pcie_reg_paxb_v2[] = {
	[IPROC_PCIE_CLK_CTRL]           = 0x000,
	[IPROC_PCIE_CFG_IND_ADDR]       = 0x120,
	[IPROC_PCIE_CFG_IND_DATA]       = 0x124,
	[IPROC_PCIE_CFG_ADDR]           = 0x1f8,
	[IPROC_PCIE_CFG_DATA]           = 0x1fc,
	[IPROC_PCIE_INTX_EN]            = 0x330,
	[IPROC_PCIE_INTX_CSR]           = 0x334,
	[IPROC_PCIE_OARR0]              = 0xd20,
	[IPROC_PCIE_OMAP0]              = 0xd40,
	[IPROC_PCIE_OARR1]              = 0xd28,
	[IPROC_PCIE_OMAP1]              = 0xd48,
	[IPROC_PCIE_OARR2]              = 0xd60,
	[IPROC_PCIE_OMAP2]              = 0xd68,
	[IPROC_PCIE_OARR3]              = 0xdf0,
	[IPROC_PCIE_OMAP3]              = 0xdf8,
	[IPROC_PCIE_IARR0]              = 0xd00,
	[IPROC_PCIE_IMAP0]              = 0xc00,
	[IPROC_PCIE_IARR2]              = 0xd10,
	[IPROC_PCIE_IMAP2]              = 0xcc0,
	[IPROC_PCIE_IARR3]              = 0xe00,
	[IPROC_PCIE_IMAP3]              = 0xe08,
	[IPROC_PCIE_IARR4]              = 0xe68,
	[IPROC_PCIE_IMAP4]              = 0xe70,
	[IPROC_PCIE_CFG_RD_STATUS]      = 0xee0,
	[IPROC_PCIE_LINK_STATUS]        = 0xf0c,
	[IPROC_PCIE_APB_ERR_EN]         = 0xf40,
	[IPROC_PCIE_ORDERING_CFG]       = 0x2000,
	[IPROC_PCIE_IMAP0_RO_CONTROL]   = 0x201c,
	[IPROC_PCIE_IMAP1_RO_CONTROL]   = 0x2020,
	[IPROC_PCIE_IMAP2_RO_CONTROL]   = 0x2024,
	[IPROC_PCIE_IMAP3_RO_CONTROL]   = 0x2028,
	[IPROC_PCIE_IMAP4_RO_CONTROL]   = 0x202c,
};

/* iProc PCIe PAXC v2 registers */
static const u16 iproc_pcie_reg_paxc_v2[] = {
	[IPROC_PCIE_MSI_GIC_MODE]	= 0x050,
	[IPROC_PCIE_MSI_BASE_ADDR]	= 0x074,
	[IPROC_PCIE_MSI_WINDOW_SIZE]	= 0x078,
	[IPROC_PCIE_MSI_ADDR_LO]	= 0x07c,
	[IPROC_PCIE_MSI_ADDR_HI]	= 0x080,
	[IPROC_PCIE_MSI_EN_CFG]		= 0x09c,
	[IPROC_PCIE_CFG_IND_ADDR]	= 0x1f0,
	[IPROC_PCIE_CFG_IND_DATA]	= 0x1f4,
	[IPROC_PCIE_CFG_ADDR]		= 0x1f8,
	[IPROC_PCIE_CFG_DATA]		= 0x1fc,
};

/**
 * List of device IDs of controllers that have corrupted
 * capability list that require SW fixup
 */
static const u16 iproc_pcie_corrupt_cap_did[] = {
	0x16cd,
	0x16f0,
	0xd802,
	0xd804
};

enum iproc_pcie_type {
	IPROC_PCIE_PAXB_V2,
	IPROC_PCIE_PAXC,
	IPROC_PCIE_PAXC_V2,
};

/**
 * struct iproc_pcie_ob - iProc PCIe outbound mapping
 *
 * @axi_offset: offset from the AXI address to the internal address used by
 * the iProc PCIe core
 * @nr_windows: total number of supported outbound mapping windows
 */
struct iproc_pcie_ob {
	resource_size_t axi_offset;
	unsigned int nr_windows;
};

/**
 * struct iproc_pcie_ib - iProc PCIe inbound mapping
 *
 * @nr_regions: total number of supported inbound mapping regions
 */
struct iproc_pcie_ib {
	unsigned int nr_regions;
};

/**
 * struct iproc_pcie_ob_map - outbound mapping controller specific parameters
 *
 * @window_sizes: list of supported outbound mapping window sizes in MB
 * @nr_sizes: number of supported outbound mapping window sizes
 */
struct iproc_pcie_ob_map {
	resource_size_t window_sizes[MAX_NUM_OB_WINDOW_SIZES];
	unsigned int nr_sizes;
};

static const struct iproc_pcie_ob_map paxb_v2_ob_map[] = {
	{
		/* OARR0/OMAP0 */
		.window_sizes = { 128, 256 },
		.nr_sizes = 2,
	},
	{
		/* OARR1/OMAP1 */
		.window_sizes = { 128, 256 },
		.nr_sizes = 2,
	},
	{
		/* OARR2/OMAP2 */
		.window_sizes = { 128, 256, 512, 1024 },
		.nr_sizes = 4,
	},
	{
		/* OARR3/OMAP3 */
		.window_sizes = { 128, 256, 512, 1024 },
		.nr_sizes = 4,
	},
};

/**
 * iProc PCIe inbound mapping type
 */
enum iproc_pcie_ib_map_type {
	/* for DDR memory */
	IPROC_PCIE_IB_MAP_MEM = 0,

	/* for device I/O memory */
	IPROC_PCIE_IB_MAP_IO,

	/* invalid or unused */
	IPROC_PCIE_IB_MAP_INVALID
};

/**
 * struct iproc_pcie_ib_map - inbound mapping controller specific parameters
 *
 * @type: inbound mapping region type
 * @size_unit: inbound mapping region size unit, could be SZ_1K, SZ_1M, or SZ_1G
 * @region_sizes: list of supported inbound mapping region sizes in KB, MB, or
 * GB, depedning on the size unit
 * @nr_sizes: number of supported inbound mapping region sizes
 * @nr_windows: number of supported inbound mapping windows for the region
 * @imap_addr_offset: register offset between the upper and lower 32-bit
 * IMAP address registers
 * @imap_window_offset: register offset between each IMAP window
 */
struct iproc_pcie_ib_map {
	enum iproc_pcie_ib_map_type type;
	unsigned int size_unit;
	resource_size_t region_sizes[MAX_NUM_IB_REGION_SIZES];
	unsigned int nr_sizes;
	unsigned int nr_windows;
	u16 imap_addr_offset;
	u16 imap_window_offset;
};

static const struct iproc_pcie_ib_map paxb_v2_ib_map[] = {
	{
		/* IARR0/IMAP0 */
		.type = IPROC_PCIE_IB_MAP_IO,
		.size_unit = SZ_1K,
		.region_sizes = { 32 },
		.nr_sizes = 1,
		.nr_windows = 8,
		.imap_addr_offset = 0x40,
		.imap_window_offset = 0x4,
	},
	{
		/* IARR1/IMAP1 (currently unused) */
		.type = IPROC_PCIE_IB_MAP_INVALID,
	},
	{
		/* IARR2/IMAP2 */
		.type = IPROC_PCIE_IB_MAP_MEM,
		.size_unit = SZ_1M,
		.region_sizes = { 64, 128, 256, 512, 1024, 2048, 4096, 8192,
			16384 },
		.nr_sizes = 9,
		.nr_windows = 1,
		.imap_addr_offset = 0x4,
		.imap_window_offset = 0x8,
	},
	{
		/* IARR3/IMAP3 */
		.type = IPROC_PCIE_IB_MAP_MEM,
		.size_unit = SZ_1G,
		.region_sizes = { 1, 2, 4, 8, 16, 32 },
		.nr_sizes = 6,
		.nr_windows = 8,
		.imap_addr_offset = 0x4,
		.imap_window_offset = 0x8,
	},
	{
		/* IARR4/IMAP4 */
		.type = IPROC_PCIE_IB_MAP_MEM,
		.size_unit = SZ_1G,
		.region_sizes = { 32, 64, 128, 256, 512 },
		.nr_sizes = 5,
		.nr_windows = 8,
		.imap_addr_offset = 0x4,
		.imap_window_offset = 0x8,
	},
};

/**
 * struct iproc_pcie - iproc pcie device instance
 *
 * @dev: pointer to pcie udevice
 * @base: device I/O base address
 * @type: pci device type, PAXC or PAXB
 * @reg_offsets: pointer to pcie host register
 * @fix_paxc_cap: paxc capability
 * @need_ob_cfg: outbound mapping status
 * @ob: pcie outbound mapping
 * @ob_map: pointer to outbound mapping parameters
 * @need_ib_cfg: inbound mapping status
 * @ib: pcie inbound mapping
 * @ib_map: pointer to inbound mapping parameters
 * @ep_is_internal: ep status
 * @phy: phy device
 * @link_is_active: link up status
 * @has_apb_err_disable: apb error status
 */
struct iproc_pcie {
	struct udevice *dev;
	void __iomem *base;
	enum iproc_pcie_type type;
	u16 *reg_offsets;
	bool fix_paxc_cap;
	bool need_ob_cfg;
	struct iproc_pcie_ob ob;
	const struct iproc_pcie_ob_map *ob_map;
	bool need_ib_cfg;
	struct iproc_pcie_ib ib;
	const struct iproc_pcie_ib_map *ib_map;
	bool ep_is_internal;
	struct phy phy;
	bool link_is_active;
	bool has_apb_err_disable;
};

static inline bool iproc_pcie_reg_is_invalid(u16 reg_offset)
{
	return !!(reg_offset == IPROC_PCIE_REG_INVALID);
}

static inline u16 iproc_pcie_reg_offset(struct iproc_pcie *pcie,
					enum iproc_pcie_reg reg)
{
	return pcie->reg_offsets[reg];
}

static inline u32 iproc_pcie_read_reg(struct iproc_pcie *pcie,
				      enum iproc_pcie_reg reg)
{
	u16 offset = iproc_pcie_reg_offset(pcie, reg);

	if (iproc_pcie_reg_is_invalid(offset))
		return 0;

	return readl(pcie->base + offset);
}

static inline void iproc_pcie_write_reg(struct iproc_pcie *pcie,
					enum iproc_pcie_reg reg, u32 val)
{
	u16 offset = iproc_pcie_reg_offset(pcie, reg);

	if (iproc_pcie_reg_is_invalid(offset))
		return;

	writel(val, pcie->base + offset);
}

static int iproc_pcie_map_ep_cfg_reg(const struct udevice *udev, pci_dev_t bdf,
				     uint where, void **paddress)
{
	struct iproc_pcie *pcie = dev_get_priv(udev);
	unsigned int busno = PCI_BUS(bdf);
	unsigned int slot = PCI_DEV(bdf);
	unsigned int fn = PCI_FUNC(bdf);

	u16 offset;
	u32 val;

	/* root complex access */
	if (busno == 0) {
		if (slot > 0 || fn > 0)
			return -ENODEV;

		iproc_pcie_write_reg(pcie, IPROC_PCIE_CFG_IND_ADDR,
				     where & CFG_IND_ADDR_MASK);
		offset = iproc_pcie_reg_offset(pcie, IPROC_PCIE_CFG_IND_DATA);
		if (iproc_pcie_reg_is_invalid(offset))
			return -ENODEV;

		*paddress = (pcie->base + offset);
		return 0;
	}

	if (!pcie->link_is_active)
		return -ENODEV;

	/* EP device access */
	val = (PCIE_ECAM_OFFSET(busno, slot, fn, where) & CFG_ADDR_CFG_ECAM_MASK)
	    | (1 & CFG_ADDR_CFG_TYPE_MASK);

	iproc_pcie_write_reg(pcie, IPROC_PCIE_CFG_ADDR, val);
	offset = iproc_pcie_reg_offset(pcie, IPROC_PCIE_CFG_DATA);

	if (iproc_pcie_reg_is_invalid(offset))
		return -ENODEV;

	*paddress = (pcie->base + offset);

	return 0;
}

static void iproc_pcie_fix_cap(struct iproc_pcie *pcie, int where, ulong *val)
{
	u32 i, dev_id;

	switch (where & ~0x3) {
	case PCI_VENDOR_ID:
		dev_id = *val >> 16;

		/*
		 * Activate fixup for those controllers that have corrupted
		 * capability list registers
		 */
		for (i = 0; i < ARRAY_SIZE(iproc_pcie_corrupt_cap_did); i++)
			if (dev_id == iproc_pcie_corrupt_cap_did[i])
				pcie->fix_paxc_cap = true;
		break;

	case IPROC_PCI_PM_CAP:
		if (pcie->fix_paxc_cap) {
			/* advertise PM, force next capability to PCIe */
			*val &= ~IPROC_PCI_PM_CAP_MASK;
			*val |= IPROC_PCI_EXP_CAP << 8 | PCI_CAP_ID_PM;
		}
		break;

	case IPROC_PCI_EXP_CAP:
		if (pcie->fix_paxc_cap) {
			/* advertise root port, version 2, terminate here */
			*val = (PCI_EXP_TYPE_ROOT_PORT << 4 | 2) << 16 |
				PCI_CAP_ID_EXP;
		}
		break;

	case IPROC_PCI_EXP_CAP + PCI_EXP_RTCTL:
		/* Don't advertise CRS SV support */
		*val &= ~(PCI_EXP_RTCAP_CRSVIS << 16);
		break;

	default:
		break;
	}
}

static int iproc_pci_raw_config_read32(struct iproc_pcie *pcie,
				       unsigned int devfn, int where,
				       int size, u32 *val)
{
	void __iomem *addr;
	int ret;

	ret = iproc_pcie_map_ep_cfg_reg(pcie->dev, devfn, where & ~0x3, &addr);
	if (ret) {
		*val = ~0;
		return -EINVAL;
	}

	*val = readl(addr);

	if (size <= 2)
		*val = (*val >> (8 * (where & 3))) & ((1 << (size * 8)) - 1);

	return 0;
}

static int iproc_pci_raw_config_write32(struct iproc_pcie *pcie,
					unsigned int devfn, int where,
					int size, u32 val)
{
	void __iomem *addr;
	int ret;
	u32 mask, tmp;

	ret = iproc_pcie_map_ep_cfg_reg(pcie->dev, devfn, where & ~0x3, &addr);
	if (ret)
		return -EINVAL;

	if (size == 4) {
		writel(val, addr);
		return 0;
	}

	mask = ~(((1 << (size * 8)) - 1) << ((where & 0x3) * 8));
	tmp = readl(addr) & mask;
	tmp |= val << ((where & 0x3) * 8);
	writel(tmp, addr);
	return 0;
}

/**
 * iproc_pcie_apb_err_disable() - configure apb error
 *
 * APB error forwarding can be disabled during access of configuration
 * registers of the endpoint device, to prevent unsupported requests
 * (typically seen during enumeration with multi-function devices) from
 * triggering a system exception.
 *
 * @bus: pcie udevice
 * @bdf: pdf value
 * @disabled: flag to enable/disabled apb error
 */
static inline void iproc_pcie_apb_err_disable(const struct udevice *bus,
					      pci_dev_t bdf, bool disable)
{
	struct iproc_pcie *pcie = dev_get_priv(bus);
	u32 val;

	if (PCI_BUS(bdf) && pcie->has_apb_err_disable) {
		val = iproc_pcie_read_reg(pcie, IPROC_PCIE_APB_ERR_EN);
		if (disable)
			val &= ~APB_ERR_EN;
		else
			val |= APB_ERR_EN;
		iproc_pcie_write_reg(pcie, IPROC_PCIE_APB_ERR_EN, val);
	}
}

static int iproc_pcie_config_read32(const struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong *valuep,
				    enum pci_size_t size)
{
	struct iproc_pcie *pcie = dev_get_priv(bus);
	int ret;
	ulong data;

	iproc_pcie_apb_err_disable(bus, bdf, true);
	ret = pci_generic_mmap_read_config(bus, iproc_pcie_map_ep_cfg_reg,
					   bdf, offset, &data, PCI_SIZE_32);
	iproc_pcie_apb_err_disable(bus, bdf, false);
	if (size <= PCI_SIZE_16)
		*valuep = (data >> (8 * (offset & 3))) &
			  ((1 << (BIT(size) * 8)) - 1);
	else
		*valuep = data;

	if (!ret && PCI_BUS(bdf) == 0)
		iproc_pcie_fix_cap(pcie, offset, valuep);

	return ret;
}

static int iproc_pcie_config_write32(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong value,
				     enum pci_size_t size)
{
	void *addr;
	ulong mask, tmp;
	int ret;

	ret = iproc_pcie_map_ep_cfg_reg(bus, bdf, offset, &addr);
	if (ret)
		return ret;

	if (size == PCI_SIZE_32) {
		writel(value, addr);
		return ret;
	}

	iproc_pcie_apb_err_disable(bus, bdf, true);
	mask = ~(((1 << (BIT(size) * 8)) - 1) << ((offset & 0x3) * 8));
	tmp = readl(addr) & mask;
	tmp |= (value  << ((offset & 0x3) * 8));
	writel(tmp, addr);
	iproc_pcie_apb_err_disable(bus, bdf, false);

	return ret;
}

const static struct dm_pci_ops iproc_pcie_ops = {
	.read_config = iproc_pcie_config_read32,
	.write_config = iproc_pcie_config_write32,
};

static int iproc_pcie_rev_init(struct iproc_pcie *pcie)
{
	unsigned int reg_idx;
	const u16 *regs;
	u16 num_elements;

	switch (pcie->type) {
	case IPROC_PCIE_PAXC_V2:
		pcie->ep_is_internal = true;
		regs = iproc_pcie_reg_paxc_v2;
		num_elements = ARRAY_SIZE(iproc_pcie_reg_paxc_v2);
		break;
	case IPROC_PCIE_PAXB_V2:
		regs = iproc_pcie_reg_paxb_v2;
		num_elements = ARRAY_SIZE(iproc_pcie_reg_paxb_v2);
		pcie->has_apb_err_disable = true;
		if (pcie->need_ob_cfg) {
			pcie->ob.axi_offset = 0;
			pcie->ob_map = paxb_v2_ob_map;
			pcie->ob.nr_windows = ARRAY_SIZE(paxb_v2_ob_map);
		}
		pcie->need_ib_cfg = true;
		pcie->ib.nr_regions = ARRAY_SIZE(paxb_v2_ib_map);
		pcie->ib_map = paxb_v2_ib_map;
		break;
	default:
		dev_dbg(pcie->dev, "incompatible iProc PCIe interface\n");
		return -EINVAL;
	}

	pcie->reg_offsets = calloc(IPROC_PCIE_MAX_NUM_REG,
				   sizeof(*pcie->reg_offsets));
	if (!pcie->reg_offsets)
		return -ENOMEM;

	/* go through the register table and populate all valid registers */
	pcie->reg_offsets[0] = (pcie->type == IPROC_PCIE_PAXC_V2) ?
		IPROC_PCIE_REG_INVALID : regs[0];
	for (reg_idx = 1; reg_idx < num_elements; reg_idx++)
		pcie->reg_offsets[reg_idx] = regs[reg_idx] ?
			regs[reg_idx] : IPROC_PCIE_REG_INVALID;

	return 0;
}

static inline bool iproc_pcie_ob_is_valid(struct iproc_pcie *pcie,
					  int window_idx)
{
	u32 val;

	val = iproc_pcie_read_reg(pcie, MAP_REG(IPROC_PCIE_OARR0, window_idx));

	return !!(val & OARR_VALID);
}

static inline int iproc_pcie_ob_write(struct iproc_pcie *pcie, int window_idx,
				      int size_idx, u64 axi_addr, u64 pci_addr)
{
	u16 oarr_offset, omap_offset;

	/*
	 * Derive the OARR/OMAP offset from the first pair (OARR0/OMAP0) based
	 * on window index.
	 */
	oarr_offset = iproc_pcie_reg_offset(pcie, MAP_REG(IPROC_PCIE_OARR0,
							  window_idx));
	omap_offset = iproc_pcie_reg_offset(pcie, MAP_REG(IPROC_PCIE_OMAP0,
							  window_idx));
	if (iproc_pcie_reg_is_invalid(oarr_offset) ||
	    iproc_pcie_reg_is_invalid(omap_offset))
		return -EINVAL;

	/*
	 * Program the OARR registers.  The upper 32-bit OARR register is
	 * always right after the lower 32-bit OARR register.
	 */
	writel(lower_32_bits(axi_addr) | (size_idx << OARR_SIZE_CFG_SHIFT) |
	       OARR_VALID, pcie->base + oarr_offset);
	writel(upper_32_bits(axi_addr), pcie->base + oarr_offset + 4);

	/* now program the OMAP registers */
	writel(lower_32_bits(pci_addr), pcie->base + omap_offset);
	writel(upper_32_bits(pci_addr), pcie->base + omap_offset + 4);

	debug("ob window [%d]: offset 0x%x axi %pap pci %pap\n",
	      window_idx, oarr_offset, &axi_addr, &pci_addr);
	debug("oarr lo 0x%x oarr hi 0x%x\n",
	      readl(pcie->base + oarr_offset),
	      readl(pcie->base + oarr_offset + 4));
	debug("omap lo 0x%x omap hi 0x%x\n",
	      readl(pcie->base + omap_offset),
	      readl(pcie->base + omap_offset + 4));

	return 0;
}

/**
 * iproc_pcie_setup_ob() - setup outbound address mapping
 *
 * Some iProc SoCs require the SW to configure the outbound address mapping
 * Outbound address translation:
 *
 * iproc_pcie_address = axi_address - axi_offset
 * OARR = iproc_pcie_address
 * OMAP = pci_addr
 * axi_addr -> iproc_pcie_address -> OARR -> OMAP -> pci_address
 *
 * @pcie: pcie device
 * @axi_addr: axi address to be translated
 * @pci_addr: pci address
 * @size: window size
 *
 * @return: 0 on success and -ve on failure
 */
static int iproc_pcie_setup_ob(struct iproc_pcie *pcie, u64 axi_addr,
			       u64 pci_addr, resource_size_t size)
{
	struct iproc_pcie_ob *ob = &pcie->ob;
	int ret = -EINVAL, window_idx, size_idx;

	if (axi_addr < ob->axi_offset) {
		pr_err("axi address %pap less than offset %pap\n",
		       &axi_addr, &ob->axi_offset);
		return -EINVAL;
	}

	/*
	 * Translate the AXI address to the internal address used by the iProc
	 * PCIe core before programming the OARR
	 */
	axi_addr -= ob->axi_offset;

	/* iterate through all OARR/OMAP mapping windows */
	for (window_idx = ob->nr_windows - 1; window_idx >= 0; window_idx--) {
		const struct iproc_pcie_ob_map *ob_map =
			&pcie->ob_map[window_idx];

		/*
		 * If current outbound window is already in use, move on to the
		 * next one.
		 */
		if (iproc_pcie_ob_is_valid(pcie, window_idx))
			continue;

		/*
		 * Iterate through all supported window sizes within the
		 * OARR/OMAP pair to find a match.  Go through the window sizes
		 * in a descending order.
		 */
		for (size_idx = ob_map->nr_sizes - 1; size_idx >= 0;
		     size_idx--) {
			resource_size_t window_size =
				ob_map->window_sizes[size_idx] * SZ_1M;

			/*
			 * Keep iterating until we reach the last window and
			 * with the minimal window size at index zero. In this
			 * case, we take a compromise by mapping it using the
			 * minimum window size that can be supported
			 */
			if (size < window_size) {
				if (size_idx > 0 || window_idx > 0)
					continue;

				/*
				 * For the corner case of reaching the minimal
				 * window size that can be supported on the
				 * last window
				 */
				axi_addr = ALIGN_DOWN(axi_addr, window_size);
				pci_addr = ALIGN_DOWN(pci_addr, window_size);
				size = window_size;
			}

			if (!IS_ALIGNED(axi_addr, window_size) ||
			    !IS_ALIGNED(pci_addr, window_size)) {
				pr_err("axi %pap or pci %pap not aligned\n",
				       &axi_addr, &pci_addr);
				return -EINVAL;
			}

			/*
			 * Match found!  Program both OARR and OMAP and mark
			 * them as a valid entry.
			 */
			ret = iproc_pcie_ob_write(pcie, window_idx, size_idx,
						  axi_addr, pci_addr);
			if (ret)
				goto err_ob;

			size -= window_size;
			if (size == 0)
				return 0;

			/*
			 * If we are here, we are done with the current window,
			 * but not yet finished all mappings.  Need to move on
			 * to the next window.
			 */
			axi_addr += window_size;
			pci_addr += window_size;
			break;
		}
	}

err_ob:
	pr_err("unable to configure outbound mapping\n");
	pr_err("axi %pap, axi offset %pap, pci %pap, res size %pap\n",
	       &axi_addr, &ob->axi_offset, &pci_addr, &size);

	return ret;
}

static int iproc_pcie_map_ranges(struct udevice *dev)
{
	struct iproc_pcie *pcie = dev_get_priv(dev);
	struct udevice *bus = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	int i, ret;

	for (i = 0; i < hose->region_count; i++) {
		if (hose->regions[i].flags == PCI_REGION_MEM ||
		    hose->regions[i].flags == PCI_REGION_PREFETCH) {
			debug("%d: bus_addr %p, axi_addr %p, size 0x%llx\n",
			      i, &hose->regions[i].bus_start,
			      &hose->regions[i].phys_start,
			      hose->regions[i].size);
			ret = iproc_pcie_setup_ob(pcie,
						  hose->regions[i].phys_start,
						  hose->regions[i].bus_start,
						  hose->regions[i].size);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static inline bool iproc_pcie_ib_is_in_use(struct iproc_pcie *pcie,
					   int region_idx)
{
	const struct iproc_pcie_ib_map *ib_map = &pcie->ib_map[region_idx];
	u32 val;

	val = iproc_pcie_read_reg(pcie, MAP_REG(IPROC_PCIE_IARR0, region_idx));

	return !!(val & (BIT(ib_map->nr_sizes) - 1));
}

static inline bool
iproc_pcie_ib_check_type(const struct iproc_pcie_ib_map *ib_map,
			 enum iproc_pcie_ib_map_type type)
{
	return !!(ib_map->type == type);
}

static int iproc_pcie_ib_write(struct iproc_pcie *pcie, int region_idx,
			       int size_idx, int nr_windows, u64 axi_addr,
			       u64 pci_addr, resource_size_t size)
{
	const struct iproc_pcie_ib_map *ib_map = &pcie->ib_map[region_idx];
	u16 iarr_offset, imap_offset;
	u32 val;
	int window_idx;

	iarr_offset = iproc_pcie_reg_offset(pcie, MAP_REG(IPROC_PCIE_IARR0,
							  region_idx));
	imap_offset = iproc_pcie_reg_offset(pcie, MAP_REG(IPROC_PCIE_IMAP0,
							  region_idx));
	if (iproc_pcie_reg_is_invalid(iarr_offset) ||
	    iproc_pcie_reg_is_invalid(imap_offset))
		return -EINVAL;

	debug("ib region [%d]: offset 0x%x axi %pap pci %pap\n",
	      region_idx, iarr_offset, &axi_addr, &pci_addr);

	/*
	 * Program the IARR registers.  The upper 32-bit IARR register is
	 * always right after the lower 32-bit IARR register.
	 */
	writel(lower_32_bits(pci_addr) | BIT(size_idx),
	       pcie->base + iarr_offset);
	writel(upper_32_bits(pci_addr), pcie->base + iarr_offset + 4);

	debug("iarr lo 0x%x iarr hi 0x%x\n",
	      readl(pcie->base + iarr_offset),
	      readl(pcie->base + iarr_offset + 4));

	/*
	 * Now program the IMAP registers.  Each IARR region may have one or
	 * more IMAP windows.
	 */
	size >>= ilog2(nr_windows);
	for (window_idx = 0; window_idx < nr_windows; window_idx++) {
		val = readl(pcie->base + imap_offset);
		val |= lower_32_bits(axi_addr) | IMAP_VALID;
		writel(val, pcie->base + imap_offset);
		writel(upper_32_bits(axi_addr),
		       pcie->base + imap_offset + ib_map->imap_addr_offset);

		debug("imap window [%d] lo 0x%x hi 0x%x\n",
		      window_idx, readl(pcie->base + imap_offset),
		      readl(pcie->base + imap_offset +
		      ib_map->imap_addr_offset));

		imap_offset += ib_map->imap_window_offset;
		axi_addr += size;
	}

	return 0;
}

/**
 * iproc_pcie_setup_ib() - setup inbound address mapping
 *
 * @pcie: pcie device
 * @axi_addr: axi address to be translated
 * @pci_addr: pci address
 * @size: window size
 * @type: inbound mapping type
 *
 * @return: 0 on success and -ve on failure
 */
static int iproc_pcie_setup_ib(struct iproc_pcie *pcie, u64 axi_addr,
			       u64 pci_addr, resource_size_t size,
			       enum iproc_pcie_ib_map_type type)
{
	struct iproc_pcie_ib *ib = &pcie->ib;
	int ret;
	unsigned int region_idx, size_idx;

	/* iterate through all IARR mapping regions */
	for (region_idx = 0; region_idx < ib->nr_regions; region_idx++) {
		const struct iproc_pcie_ib_map *ib_map =
			&pcie->ib_map[region_idx];

		/*
		 * If current inbound region is already in use or not a
		 * compatible type, move on to the next.
		 */
		if (iproc_pcie_ib_is_in_use(pcie, region_idx) ||
		    !iproc_pcie_ib_check_type(ib_map, type))
			continue;

		/* iterate through all supported region sizes to find a match */
		for (size_idx = 0; size_idx < ib_map->nr_sizes; size_idx++) {
			resource_size_t region_size =
			ib_map->region_sizes[size_idx] * ib_map->size_unit;

			if (size != region_size)
				continue;

			if (!IS_ALIGNED(axi_addr, region_size) ||
			    !IS_ALIGNED(pci_addr, region_size)) {
				pr_err("axi %pap or pci %pap not aligned\n",
				       &axi_addr, &pci_addr);
				return -EINVAL;
			}

			/* Match found!  Program IARR and all IMAP windows. */
			ret = iproc_pcie_ib_write(pcie, region_idx, size_idx,
						  ib_map->nr_windows, axi_addr,
						  pci_addr, size);
			if (ret)
				goto err_ib;
			else
				return 0;
		}
	}
	ret = -EINVAL;

err_ib:
	pr_err("unable to configure inbound mapping\n");
	pr_err("axi %pap, pci %pap, res size %pap\n",
	       &axi_addr, &pci_addr, &size);

	return ret;
}

static int iproc_pcie_map_dma_ranges(struct iproc_pcie *pcie)
{
	int ret;
	struct pci_region regions;
	int i = 0;

	while (!pci_get_dma_regions(pcie->dev, &regions, i)) {
		dev_dbg(pcie->dev,
			"dma %d: bus_addr %#llx, axi_addr %#llx, size %#llx\n",
			i, regions.bus_start, regions.phys_start, regions.size);

		/* Each range entry corresponds to an inbound mapping region */
		ret = iproc_pcie_setup_ib(pcie, regions.phys_start,
					  regions.bus_start,
					  regions.size,
					  IPROC_PCIE_IB_MAP_MEM);
		if (ret)
			return ret;
		i++;
	}
	return 0;
}

static void iproc_pcie_reset_map_regs(struct iproc_pcie *pcie)
{
	struct iproc_pcie_ib *ib = &pcie->ib;
	struct iproc_pcie_ob *ob = &pcie->ob;
	int window_idx, region_idx;

	if (pcie->ep_is_internal)
		return;

	/* iterate through all OARR mapping regions */
	for (window_idx = ob->nr_windows - 1; window_idx >= 0; window_idx--) {
		iproc_pcie_write_reg(pcie, MAP_REG(IPROC_PCIE_OARR0,
						   window_idx), 0);
	}

	/* iterate through all IARR mapping regions */
	for (region_idx = 0; region_idx < ib->nr_regions; region_idx++) {
		iproc_pcie_write_reg(pcie, MAP_REG(IPROC_PCIE_IARR0,
						   region_idx), 0);
	}
}

static void iproc_pcie_reset(struct iproc_pcie *pcie)
{
	u32 val;

	/*
	 * PAXC and the internal emulated endpoint device downstream should not
	 * be reset.  If firmware has been loaded on the endpoint device at an
	 * earlier boot stage, reset here causes issues.
	 */
	if (pcie->ep_is_internal)
		return;

	/*
	 * Select perst_b signal as reset source. Put the device into reset,
	 * and then bring it out of reset
	 */
	val = iproc_pcie_read_reg(pcie, IPROC_PCIE_CLK_CTRL);
	val &= ~EP_PERST_SOURCE_SELECT & ~EP_MODE_SURVIVE_PERST &
		~RC_PCIE_RST_OUTPUT;
	iproc_pcie_write_reg(pcie, IPROC_PCIE_CLK_CTRL, val);
	udelay(250);

	val |= RC_PCIE_RST_OUTPUT;
	iproc_pcie_write_reg(pcie, IPROC_PCIE_CLK_CTRL, val);
	mdelay(100);
}

static inline bool iproc_pcie_link_is_active(struct iproc_pcie *pcie)
{
	u32 val;

	val = iproc_pcie_read_reg(pcie, IPROC_PCIE_LINK_STATUS);
	return !!((val & PCIE_PHYLINKUP) && (val & PCIE_DL_ACTIVE));
}

static int iproc_pcie_check_link(struct iproc_pcie *pcie)
{
	u32 link_status, class;

	pcie->link_is_active = false;
	/* force class to PCI bridge Normal decode (0x060400) */
#define PCI_BRIDGE_CTRL_REG_OFFSET      0x43c
#define PCI_BRIDGE_CTRL_REG_CLASS_MASK  0xffffff
	iproc_pci_raw_config_read32(pcie, 0,
				    PCI_BRIDGE_CTRL_REG_OFFSET,
				    4, &class);
	class &= ~PCI_BRIDGE_CTRL_REG_CLASS_MASK;
	class |= PCI_CLASS_BRIDGE_PCI_NORMAL;
	iproc_pci_raw_config_write32(pcie, 0,
				     PCI_BRIDGE_CTRL_REG_OFFSET,
				     4, class);

	/*
	 * PAXC connects to emulated endpoint devices directly and does not
	 * have a Serdes.  Therefore skip the link detection logic here.
	 */
	if (pcie->ep_is_internal) {
		pcie->link_is_active = true;
		return 0;
	}

	if (!iproc_pcie_link_is_active(pcie)) {
		pr_err("PHY or data link is INACTIVE!\n");
		return -ENODEV;
	}

#define PCI_TARGET_LINK_SPEED_MASK      0xf
#define PCI_TARGET_LINK_WIDTH_MASK      0x3f
#define PCI_TARGET_LINK_WIDTH_OFFSET    0x4

	/* check link status to see if link is active */
	iproc_pci_raw_config_read32(pcie, 0,
				    IPROC_PCI_EXP_CAP + PCI_EXP_LNKSTA,
				    2, &link_status);
	if (link_status & PCI_EXP_LNKSTA_NLW)
		pcie->link_is_active = true;

	if (pcie->link_is_active)
		pr_info("link UP @ Speed Gen-%d and width-x%d\n",
			link_status & PCI_TARGET_LINK_SPEED_MASK,
			(link_status >> PCI_TARGET_LINK_WIDTH_OFFSET) &
			PCI_TARGET_LINK_WIDTH_MASK);
	else
		pr_info("link DOWN\n");

	return 0;
}

static int iproc_pcie_probe(struct udevice *dev)
{
	struct iproc_pcie *pcie = dev_get_priv(dev);
	int ret;

	pcie->type = (enum iproc_pcie_type)dev_get_driver_data(dev);
	debug("PAX type %d\n", pcie->type);
	pcie->base = dev_read_addr_ptr(dev);
	debug("PAX reg base %p\n", pcie->base);

	if (!pcie->base)
		return -ENODEV;

	if (dev_read_bool(dev, "brcm,pcie-ob"))
		pcie->need_ob_cfg = true;

	pcie->dev = dev;
	ret = iproc_pcie_rev_init(pcie);
	if (ret)
		return ret;

	if (!pcie->ep_is_internal) {
		ret = generic_phy_get_by_name(dev, "pcie-phy", &pcie->phy);
		if (!ret) {
			ret = generic_phy_init(&pcie->phy);
			if (ret) {
				pr_err("failed to init %s PHY\n", dev->name);
				return ret;
			}

			ret = generic_phy_power_on(&pcie->phy);
			if (ret) {
				pr_err("power on %s PHY failed\n", dev->name);
				goto err_exit_phy;
			}
		}
	}

	iproc_pcie_reset(pcie);

	if (pcie->need_ob_cfg) {
		ret = iproc_pcie_map_ranges(dev);
		if (ret) {
			pr_err("outbound map failed\n");
			goto err_power_off_phy;
		}
	}

	if (pcie->need_ib_cfg) {
		ret = iproc_pcie_map_dma_ranges(pcie);
		if (ret) {
			pr_err("inbound map failed\n");
			goto err_power_off_phy;
		}
	}

	if (iproc_pcie_check_link(pcie))
		pr_info("no PCIe EP device detected\n");

	return 0;

err_power_off_phy:
	generic_phy_power_off(&pcie->phy);
err_exit_phy:
	generic_phy_exit(&pcie->phy);
	return ret;
}

static int iproc_pcie_remove(struct udevice *dev)
{
	struct iproc_pcie *pcie = dev_get_priv(dev);
	int ret;

	iproc_pcie_reset_map_regs(pcie);

	if (generic_phy_valid(&pcie->phy)) {
		ret = generic_phy_power_off(&pcie->phy);
		if (ret) {
			pr_err("failed to power off PCIe phy\n");
			return ret;
		}

		ret = generic_phy_exit(&pcie->phy);
		if (ret) {
			pr_err("failed to power off PCIe phy\n");
			return ret;
		}
	}

	return 0;
}

static const struct udevice_id pci_iproc_ids[] = {
	{ .compatible = "brcm,iproc-pcie-paxb-v2",
	  .data = IPROC_PCIE_PAXB_V2 },
	{ .compatible = "brcm,iproc-pcie-paxc-v2",
	  .data = IPROC_PCIE_PAXC_V2 },
	{ }
};

U_BOOT_DRIVER(pci_iproc) = {
	.name = "pci_iproc",
	.id = UCLASS_PCI,
	.of_match = pci_iproc_ids,
	.ops = &iproc_pcie_ops,
	.probe = iproc_pcie_probe,
	.remove = iproc_pcie_remove,
	.priv_auto	= sizeof(struct iproc_pcie),
	.flags = DM_FLAG_OS_PREPARE,
};
