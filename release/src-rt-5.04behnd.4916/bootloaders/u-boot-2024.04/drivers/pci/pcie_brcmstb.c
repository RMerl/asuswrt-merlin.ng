// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB PCIe controller driver
 *
 * Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 * Based on upstream Linux kernel driver:
 * drivers/pci/controller/pcie-brcmstb.c
 * Copyright (C) 2009 - 2017 Broadcom
 *
 * Based driver by Nicolas Saenz Julienne
 * Copyright (C) 2020 Nicolas Saenz Julienne <nsaenzjulienne@suse.de>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <pci.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/log2.h>
#include <linux/iopoll.h>

/* Offset of the mandatory PCIe capability config registers */
#define BRCM_PCIE_CAP_REGS				0x00ac

/* The PCIe controller register offsets */
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1		0x0188
#define  VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_MASK	0xc
#define  VENDOR_SPECIFIC_REG1_LITTLE_ENDIAN		0x0

#define PCIE_RC_CFG_PRIV1_ID_VAL3			0x043c
#define  CFG_PRIV1_ID_VAL3_CLASS_CODE_MASK		0xffffff

#define PCIE_RC_CFG_PRIV1_LINK_CAPABILITY			0x04dc
#define  PCIE_RC_CFG_PRIV1_LINK_CAPABILITY_ASPM_SUPPORT_MASK	0xc00

#define PCIE_RC_DL_MDIO_ADDR				0x1100
#define PCIE_RC_DL_MDIO_WR_DATA				0x1104
#define PCIE_RC_DL_MDIO_RD_DATA				0x1108

#define PCIE_MISC_MISC_CTRL				0x4008
#define  MISC_CTRL_SCB_ACCESS_EN_MASK			0x1000
#define  MISC_CTRL_CFG_READ_UR_MODE_MASK		0x2000
#define  MISC_CTRL_MAX_BURST_SIZE_MASK			0x300000
#define  MISC_CTRL_MAX_BURST_SIZE_128			0x0
#define  MISC_CTRL_SCB0_SIZE_MASK			0xf8000000

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO		0x400c
#define PCIE_MEM_WIN0_LO(win)	\
		PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO + ((win) * 4)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI		0x4010
#define PCIE_MEM_WIN0_HI(win)	\
		PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI + ((win) * 4)

#define PCIE_MISC_RC_BAR1_CONFIG_LO			0x402c
#define  RC_BAR1_CONFIG_LO_SIZE_MASK			0x1f

#define PCIE_MISC_RC_BAR2_CONFIG_LO			0x4034
#define  RC_BAR2_CONFIG_LO_SIZE_MASK			0x1f
#define PCIE_MISC_RC_BAR2_CONFIG_HI			0x4038

#define PCIE_MISC_RC_BAR3_CONFIG_LO			0x403c
#define  RC_BAR3_CONFIG_LO_SIZE_MASK			0x1f

#define PCIE_MISC_PCIE_STATUS				0x4068
#define  STATUS_PCIE_PORT_MASK				0x80
#define  STATUS_PCIE_PORT_SHIFT				7
#define  STATUS_PCIE_DL_ACTIVE_MASK			0x20
#define  STATUS_PCIE_DL_ACTIVE_SHIFT			5
#define  STATUS_PCIE_PHYLINKUP_MASK			0x10
#define  STATUS_PCIE_PHYLINKUP_SHIFT			4

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT	0x4070
#define  MEM_WIN0_BASE_LIMIT_LIMIT_MASK			0xfff00000
#define  MEM_WIN0_BASE_LIMIT_BASE_MASK			0xfff0
#define  MEM_WIN0_BASE_LIMIT_BASE_HI_SHIFT		12
#define PCIE_MEM_WIN0_BASE_LIMIT(win)	\
	 PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT + ((win) * 4)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI		0x4080
#define  MEM_WIN0_BASE_HI_BASE_MASK			0xff
#define PCIE_MEM_WIN0_BASE_HI(win)	\
	 PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI + ((win) * 8)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI		0x4084
#define  PCIE_MEM_WIN0_LIMIT_HI_LIMIT_MASK		0xff
#define PCIE_MEM_WIN0_LIMIT_HI(win)	\
	 PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI + ((win) * 8)

#define PCIE_MISC_HARD_PCIE_HARD_DEBUG			0x4204
#define  PCIE_HARD_DEBUG_SERDES_IDDQ_MASK		0x08000000

#define PCIE_MSI_INTR2_CLR				0x4508
#define PCIE_MSI_INTR2_MASK_SET				0x4510

#define PCIE_EXT_CFG_DATA				0x8000

#define PCIE_EXT_CFG_INDEX				0x9000

#define PCIE_RGR1_SW_INIT_1				0x9210
#define  RGR1_SW_INIT_1_PERST_MASK			0x1
#define  RGR1_SW_INIT_1_INIT_MASK			0x2

/* PCIe parameters */
#define BRCM_NUM_PCIE_OUT_WINS				4

/* MDIO registers */
#define MDIO_PORT0					0x0
#define MDIO_DATA_MASK					0x7fffffff
#define MDIO_DATA_SHIFT					0
#define MDIO_PORT_MASK					0xf0000
#define MDIO_PORT_SHIFT					16
#define MDIO_REGAD_MASK					0xffff
#define MDIO_REGAD_SHIFT				0
#define MDIO_CMD_MASK					0xfff00000
#define MDIO_CMD_SHIFT					20
#define MDIO_CMD_READ					0x1
#define MDIO_CMD_WRITE					0x0
#define MDIO_DATA_DONE_MASK				0x80000000
#define SSC_REGS_ADDR					0x1100
#define SET_ADDR_OFFSET					0x1f
#define SSC_CNTL_OFFSET					0x2
#define SSC_CNTL_OVRD_EN_MASK				0x8000
#define SSC_CNTL_OVRD_VAL_MASK				0x4000
#define SSC_STATUS_OFFSET				0x1
#define SSC_STATUS_SSC_MASK				0x400
#define SSC_STATUS_SSC_SHIFT				10
#define SSC_STATUS_PLL_LOCK_MASK			0x800
#define SSC_STATUS_PLL_LOCK_SHIFT			11

/**
 * struct brcm_pcie - the PCIe controller state
 * @base: Base address of memory mapped IO registers of the controller
 * @gen: Non-zero value indicates limitation of the PCIe controller operation
 *       to a specific generation (1, 2 or 3)
 * @ssc: true indicates active Spread Spectrum Clocking operation
 */
struct brcm_pcie {
	void __iomem		*base;

	int			gen;
	bool			ssc;
};

/**
 * brcm_pcie_encode_ibar_size() - Encode the inbound "BAR" region size
 * @size: The inbound region size
 *
 * This function converts size of the inbound "BAR" region to the non-linear
 * values of the PCIE_MISC_RC_BAR[123]_CONFIG_LO register SIZE field.
 *
 * Return: The encoded inbound region size
 */
static int brcm_pcie_encode_ibar_size(u64 size)
{
	int log2_in = ilog2(size);

	if (log2_in >= 12 && log2_in <= 15)
		/* Covers 4KB to 32KB (inclusive) */
		return (log2_in - 12) + 0x1c;
	else if (log2_in >= 16 && log2_in <= 37)
		/* Covers 64KB to 32GB, (inclusive) */
		return log2_in - 15;

	/* Something is awry so disable */
	return 0;
}

/**
 * brcm_pcie_rc_mode() - Check if PCIe controller is in RC mode
 * @pcie: Pointer to the PCIe controller state
 *
 * The controller is capable of serving in both RC and EP roles.
 *
 * Return: true for RC mode, false for EP mode.
 */
static bool brcm_pcie_rc_mode(struct brcm_pcie *pcie)
{
	u32 val;

	val = readl(pcie->base + PCIE_MISC_PCIE_STATUS);

	return (val & STATUS_PCIE_PORT_MASK) >> STATUS_PCIE_PORT_SHIFT;
}

/**
 * brcm_pcie_link_up() - Check whether the PCIe link is up
 * @pcie: Pointer to the PCIe controller state
 *
 * Return: true if the link is up, false otherwise.
 */
static bool brcm_pcie_link_up(struct brcm_pcie *pcie)
{
	u32 val, dla, plu;

	val = readl(pcie->base + PCIE_MISC_PCIE_STATUS);
	dla = (val & STATUS_PCIE_DL_ACTIVE_MASK) >> STATUS_PCIE_DL_ACTIVE_SHIFT;
	plu = (val & STATUS_PCIE_PHYLINKUP_MASK) >> STATUS_PCIE_PHYLINKUP_SHIFT;

	return dla && plu;
}

static int brcm_pcie_config_address(const struct udevice *dev, pci_dev_t bdf,
				    uint offset, void **paddress)
{
	struct brcm_pcie *pcie = dev_get_priv(dev);
	unsigned int pci_bus = PCI_BUS(bdf);
	unsigned int pci_dev = PCI_DEV(bdf);
	unsigned int pci_func = PCI_FUNC(bdf);
	int idx;

	/*
	 * Busses 0 (host PCIe bridge) and 1 (its immediate child)
	 * are limited to a single device each
	 */
	if (pci_bus < 2 && pci_dev > 0)
		return -EINVAL;

	/* Accesses to the RC go right to the RC registers */
	if (pci_bus == 0) {
		*paddress = pcie->base + offset;
		return 0;
	}

	/* An access to our HW w/o link-up will cause a CPU Abort */
	if (!brcm_pcie_link_up(pcie))
		return -EINVAL;

	/* For devices, write to the config space index register */
	idx = PCIE_ECAM_OFFSET(pci_bus, pci_dev, pci_func, 0);

	writel(idx, pcie->base + PCIE_EXT_CFG_INDEX);
	*paddress = pcie->base + PCIE_EXT_CFG_DATA + offset;

	return 0;
}

static int brcm_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, brcm_pcie_config_address,
					    bdf, offset, valuep, size);
}

static int brcm_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong value,
				  enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, brcm_pcie_config_address,
					     bdf, offset, value, size);
}

static const char *link_speed_to_str(unsigned int cls)
{
	switch (cls) {
	case PCI_EXP_LNKSTA_CLS_2_5GB: return "2.5";
	case PCI_EXP_LNKSTA_CLS_5_0GB: return "5.0";
	case PCI_EXP_LNKSTA_CLS_8_0GB: return "8.0";
	default:
		break;
	}

	return "??";
}

static u32 brcm_pcie_mdio_form_pkt(unsigned int port, unsigned int regad,
				   unsigned int cmd)
{
	u32 pkt;

	pkt = (port << MDIO_PORT_SHIFT) & MDIO_PORT_MASK;
	pkt |= (regad << MDIO_REGAD_SHIFT) & MDIO_REGAD_MASK;
	pkt |= (cmd << MDIO_CMD_SHIFT) & MDIO_CMD_MASK;

	return pkt;
}

/**
 * brcm_pcie_mdio_read() - Perform a register read on the internal MDIO bus
 * @base: Pointer to the PCIe controller IO registers
 * @port: The MDIO port number
 * @regad: The register address
 * @val: A pointer at which to store the read value
 *
 * Return: 0 on success and register value in @val, negative error value
 *         on failure.
 */
static int brcm_pcie_mdio_read(void __iomem *base, unsigned int port,
			       unsigned int regad, u32 *val)
{
	u32 data, addr;
	int ret;

	addr = brcm_pcie_mdio_form_pkt(port, regad, MDIO_CMD_READ);
	writel(addr, base + PCIE_RC_DL_MDIO_ADDR);
	readl(base + PCIE_RC_DL_MDIO_ADDR);

	ret = readl_poll_timeout(base + PCIE_RC_DL_MDIO_RD_DATA, data,
				 (data & MDIO_DATA_DONE_MASK), 100);

	*val = data & MDIO_DATA_MASK;

	return ret;
}

/**
 * brcm_pcie_mdio_write() - Perform a register write on the internal MDIO bus
 * @base: Pointer to the PCIe controller IO registers
 * @port: The MDIO port number
 * @regad: Address of the register
 * @wrdata: The value to write
 *
 * Return: 0 on success, negative error value on failure.
 */
static int brcm_pcie_mdio_write(void __iomem *base, unsigned int port,
				unsigned int regad, u16 wrdata)
{
	u32 data, addr;

	addr = brcm_pcie_mdio_form_pkt(port, regad, MDIO_CMD_WRITE);
	writel(addr, base + PCIE_RC_DL_MDIO_ADDR);
	readl(base + PCIE_RC_DL_MDIO_ADDR);
	writel(MDIO_DATA_DONE_MASK | wrdata, base + PCIE_RC_DL_MDIO_WR_DATA);

	return readl_poll_timeout(base + PCIE_RC_DL_MDIO_WR_DATA, data,
				  !(data & MDIO_DATA_DONE_MASK), 100);
}

/**
 * brcm_pcie_set_ssc() - Configure the controller for Spread Spectrum Clocking
 * @base: pointer to the PCIe controller IO registers
 *
 * Return: 0 on success, negative error value on failure.
 */
static int brcm_pcie_set_ssc(void __iomem *base)
{
	int pll, ssc;
	int ret;
	u32 tmp;

	ret = brcm_pcie_mdio_write(base, MDIO_PORT0, SET_ADDR_OFFSET,
				   SSC_REGS_ADDR);
	if (ret < 0)
		return ret;

	ret = brcm_pcie_mdio_read(base, MDIO_PORT0, SSC_CNTL_OFFSET, &tmp);
	if (ret < 0)
		return ret;

	tmp |= (SSC_CNTL_OVRD_EN_MASK | SSC_CNTL_OVRD_VAL_MASK);

	ret = brcm_pcie_mdio_write(base, MDIO_PORT0, SSC_CNTL_OFFSET, tmp);
	if (ret < 0)
		return ret;

	udelay(1000);
	ret = brcm_pcie_mdio_read(base, MDIO_PORT0, SSC_STATUS_OFFSET, &tmp);
	if (ret < 0)
		return ret;

	ssc = (tmp & SSC_STATUS_SSC_MASK) >> SSC_STATUS_SSC_SHIFT;
	pll = (tmp & SSC_STATUS_PLL_LOCK_MASK) >> SSC_STATUS_PLL_LOCK_SHIFT;

	return ssc && pll ? 0 : -EIO;
}

/**
 * brcm_pcie_set_gen() - Limits operation to a specific generation (1, 2 or 3)
 * @pcie: pointer to the PCIe controller state
 * @gen: PCIe generation to limit the controller's operation to
 */
static void brcm_pcie_set_gen(struct brcm_pcie *pcie, unsigned int gen)
{
	void __iomem *cap_base = pcie->base + BRCM_PCIE_CAP_REGS;

	u16 lnkctl2 = readw(cap_base + PCI_EXP_LNKCTL2);
	u32 lnkcap = readl(cap_base + PCI_EXP_LNKCAP);

	lnkcap = (lnkcap & ~PCI_EXP_LNKCAP_SLS) | gen;
	writel(lnkcap, cap_base + PCI_EXP_LNKCAP);

	lnkctl2 = (lnkctl2 & ~0xf) | gen;
	writew(lnkctl2, cap_base + PCI_EXP_LNKCTL2);
}

static void brcm_pcie_set_outbound_win(struct brcm_pcie *pcie,
				       unsigned int win, u64 phys_addr,
				       u64 pcie_addr, u64 size)
{
	void __iomem *base = pcie->base;
	u32 phys_addr_mb_high, limit_addr_mb_high;
	phys_addr_t phys_addr_mb, limit_addr_mb;
	int high_addr_shift;
	u32 tmp;

	/* Set the base of the pcie_addr window */
	writel(lower_32_bits(pcie_addr), base + PCIE_MEM_WIN0_LO(win));
	writel(upper_32_bits(pcie_addr), base + PCIE_MEM_WIN0_HI(win));

	/* Write the addr base & limit lower bits (in MBs) */
	phys_addr_mb = phys_addr / SZ_1M;
	limit_addr_mb = (phys_addr + size - 1) / SZ_1M;

	tmp = readl(base + PCIE_MEM_WIN0_BASE_LIMIT(win));
	u32p_replace_bits(&tmp, phys_addr_mb,
			  MEM_WIN0_BASE_LIMIT_BASE_MASK);
	u32p_replace_bits(&tmp, limit_addr_mb,
			  MEM_WIN0_BASE_LIMIT_LIMIT_MASK);
	writel(tmp, base + PCIE_MEM_WIN0_BASE_LIMIT(win));

	/* Write the cpu & limit addr upper bits */
	high_addr_shift = MEM_WIN0_BASE_LIMIT_BASE_HI_SHIFT;
	phys_addr_mb_high = phys_addr_mb >> high_addr_shift;
	tmp = readl(base + PCIE_MEM_WIN0_BASE_HI(win));
	u32p_replace_bits(&tmp, phys_addr_mb_high,
			  MEM_WIN0_BASE_HI_BASE_MASK);
	writel(tmp, base + PCIE_MEM_WIN0_BASE_HI(win));

	limit_addr_mb_high = limit_addr_mb >> high_addr_shift;
	tmp = readl(base + PCIE_MEM_WIN0_LIMIT_HI(win));
	u32p_replace_bits(&tmp, limit_addr_mb_high,
			  PCIE_MEM_WIN0_LIMIT_HI_LIMIT_MASK);
	writel(tmp, base + PCIE_MEM_WIN0_LIMIT_HI(win));
}

static int brcm_pcie_probe(struct udevice *dev)
{
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	struct brcm_pcie *pcie = dev_get_priv(dev);
	void __iomem *base = pcie->base;
	struct pci_region region;
	bool ssc_good = false;
	int num_out_wins = 0;
	u64 rc_bar2_offset, rc_bar2_size;
	unsigned int scb_size_val;
	int i, ret;
	u16 nlw, cls, lnksta;
	u32 tmp;

	/*
	 * Reset the bridge, assert the fundamental reset. Note for some SoCs,
	 * e.g. BCM7278, the fundamental reset should not be asserted here.
	 * This will need to be changed when support for other SoCs is added.
	 */
	setbits_le32(base + PCIE_RGR1_SW_INIT_1,
		     RGR1_SW_INIT_1_INIT_MASK | RGR1_SW_INIT_1_PERST_MASK);
	/*
	 * The delay is a safety precaution to preclude the reset signal
	 * from looking like a glitch.
	 */
	udelay(100);

	/* Take the bridge out of reset */
	clrbits_le32(base + PCIE_RGR1_SW_INIT_1, RGR1_SW_INIT_1_INIT_MASK);

	clrbits_le32(base + PCIE_MISC_HARD_PCIE_HARD_DEBUG,
		     PCIE_HARD_DEBUG_SERDES_IDDQ_MASK);

	/* Wait for SerDes to be stable */
	udelay(100);

	/* Set SCB_MAX_BURST_SIZE, CFG_READ_UR_MODE, SCB_ACCESS_EN */
	clrsetbits_le32(base + PCIE_MISC_MISC_CTRL,
			MISC_CTRL_MAX_BURST_SIZE_MASK,
			MISC_CTRL_SCB_ACCESS_EN_MASK |
			MISC_CTRL_CFG_READ_UR_MODE_MASK |
			MISC_CTRL_MAX_BURST_SIZE_128);

	pci_get_dma_regions(dev, &region, 0);
	rc_bar2_offset = region.bus_start - region.phys_start;
	rc_bar2_size = 1ULL << fls64(region.size - 1);

	tmp = lower_32_bits(rc_bar2_offset);
	u32p_replace_bits(&tmp, brcm_pcie_encode_ibar_size(rc_bar2_size),
			  RC_BAR2_CONFIG_LO_SIZE_MASK);
	writel(tmp, base + PCIE_MISC_RC_BAR2_CONFIG_LO);
	writel(upper_32_bits(rc_bar2_offset),
	       base + PCIE_MISC_RC_BAR2_CONFIG_HI);

	scb_size_val = rc_bar2_size ?
		       ilog2(rc_bar2_size) - 15 : 0xf; /* 0xf is 1GB */

	tmp = readl(base + PCIE_MISC_MISC_CTRL);
	u32p_replace_bits(&tmp, scb_size_val,
			  MISC_CTRL_SCB0_SIZE_MASK);
	writel(tmp, base + PCIE_MISC_MISC_CTRL);

	/* Disable the PCIe->GISB memory window (RC_BAR1) */
	clrbits_le32(base + PCIE_MISC_RC_BAR1_CONFIG_LO,
		     RC_BAR1_CONFIG_LO_SIZE_MASK);

	/* Disable the PCIe->SCB memory window (RC_BAR3) */
	clrbits_le32(base + PCIE_MISC_RC_BAR3_CONFIG_LO,
		     RC_BAR3_CONFIG_LO_SIZE_MASK);

	/* Mask all interrupts since we are not handling any yet */
	writel(0xffffffff, base + PCIE_MSI_INTR2_MASK_SET);

	/* Clear any interrupts we find on boot */
	writel(0xffffffff, base + PCIE_MSI_INTR2_CLR);

	if (pcie->gen)
		brcm_pcie_set_gen(pcie, pcie->gen);

	/* Unassert the fundamental reset */
	clrbits_le32(pcie->base + PCIE_RGR1_SW_INIT_1,
		     RGR1_SW_INIT_1_PERST_MASK);

	/*
	 * Wait for 100ms after PERST# deassertion; see PCIe CEM specification
	 * sections 2.2, PCIe r5.0, 6.6.1.
	 */
	mdelay(100);

	/* Give the RC/EP time to wake up, before trying to configure RC.
	 * Intermittently check status for link-up, up to a total of 100ms.
	 */
	for (i = 0; i < 100 && !brcm_pcie_link_up(pcie); i += 5)
		mdelay(5);

	if (!brcm_pcie_link_up(pcie)) {
		printf("PCIe BRCM: link down\n");
		return -EINVAL;
	}

	if (!brcm_pcie_rc_mode(pcie)) {
		printf("PCIe misconfigured; is in EP mode\n");
		return -EINVAL;
	}

	for (i = 0; i < hose->region_count; i++) {
		struct pci_region *reg = &hose->regions[i];

		if (reg->flags != PCI_REGION_MEM)
			continue;

		if (num_out_wins >= BRCM_NUM_PCIE_OUT_WINS)
			return -EINVAL;

		brcm_pcie_set_outbound_win(pcie, num_out_wins, reg->phys_start,
					   reg->bus_start, reg->size);

		num_out_wins++;
	}

	/*
	 * For config space accesses on the RC, show the right class for
	 * a PCIe-PCIe bridge (the default setting is to be EP mode).
	 */
	clrsetbits_le32(base + PCIE_RC_CFG_PRIV1_ID_VAL3,
			CFG_PRIV1_ID_VAL3_CLASS_CODE_MASK, 0x060400);

	if (pcie->ssc) {
		ret = brcm_pcie_set_ssc(pcie->base);
		if (!ret)
			ssc_good = true;
		else
			printf("PCIe BRCM: failed attempt to enter SSC mode\n");
	}

	lnksta = readw(base + BRCM_PCIE_CAP_REGS + PCI_EXP_LNKSTA);
	cls = lnksta & PCI_EXP_LNKSTA_CLS;
	nlw = (lnksta & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;

	printf("PCIe BRCM: link up, %s Gbps x%u %s\n", link_speed_to_str(cls),
	       nlw, ssc_good ? "(SSC)" : "(!SSC)");

	/* PCIe->SCB endian mode for BAR */
	clrsetbits_le32(base + PCIE_RC_CFG_VENDOR_SPECIFIC_REG1,
			VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_MASK,
			VENDOR_SPECIFIC_REG1_LITTLE_ENDIAN);

	/*
	 * We used to enable the CLKREQ# input here, but a few PCIe cards don't
	 * attach anything to the CLKREQ# line, so we shouldn't assume that
	 * it's connected and working. The controller does allow detecting
	 * whether the port on the other side of our link is/was driving this
	 * signal, so we could check before we assume. But because this signal
	 * is for power management, which doesn't make sense in a bootloader,
	 * let's instead just unadvertise ASPM support.
	 */
	clrbits_le32(base + PCIE_RC_CFG_PRIV1_LINK_CAPABILITY,
		     PCIE_RC_CFG_PRIV1_LINK_CAPABILITY_ASPM_SUPPORT_MASK);

	return 0;
}

static int brcm_pcie_remove(struct udevice *dev)
{
	struct brcm_pcie *pcie = dev_get_priv(dev);
	void __iomem *base = pcie->base;

	/* Assert fundamental reset */
	setbits_le32(base + PCIE_RGR1_SW_INIT_1, RGR1_SW_INIT_1_PERST_MASK);

	/* Turn off SerDes */
	setbits_le32(base + PCIE_MISC_HARD_PCIE_HARD_DEBUG,
		     PCIE_HARD_DEBUG_SERDES_IDDQ_MASK);

	/* Shutdown bridge */
	setbits_le32(base + PCIE_RGR1_SW_INIT_1, RGR1_SW_INIT_1_INIT_MASK);

	return 0;
}

static int brcm_pcie_of_to_plat(struct udevice *dev)
{
	struct brcm_pcie *pcie = dev_get_priv(dev);
	ofnode dn = dev_ofnode(dev);
	u32 max_link_speed;
	int ret;

	/* Get the controller base address */
	pcie->base = dev_read_addr_ptr(dev);
	if (!pcie->base)
		return -EINVAL;

	pcie->ssc = ofnode_read_bool(dn, "brcm,enable-ssc");

	ret = ofnode_read_u32(dn, "max-link-speed", &max_link_speed);
	if (ret < 0 || max_link_speed > 4)
		pcie->gen = 0;
	else
		pcie->gen = max_link_speed;

	return 0;
}

static const struct dm_pci_ops brcm_pcie_ops = {
	.read_config	= brcm_pcie_read_config,
	.write_config	= brcm_pcie_write_config,
};

static const struct udevice_id brcm_pcie_ids[] = {
	{ .compatible = "brcm,bcm2711-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_brcm_base) = {
	.name			= "pcie_brcm",
	.id			= UCLASS_PCI,
	.ops			= &brcm_pcie_ops,
	.of_match		= brcm_pcie_ids,
	.probe			= brcm_pcie_probe,
	.remove			= brcm_pcie_remove,
	.of_to_plat	= brcm_pcie_of_to_plat,
	.priv_auto	= sizeof(struct brcm_pcie),
	.flags		= DM_FLAG_OS_PREPARE,
};
