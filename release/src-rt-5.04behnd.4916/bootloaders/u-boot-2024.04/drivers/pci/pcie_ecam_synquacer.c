// SPDX-License-Identifier: GPL-2.0
/*
 * SynQuacer PCIE host driver
 *
 * Based on drivers/pci/pcie_ecam_generic.c
 *
 * Copyright (C) 2016 Imagination Technologies
 * Copyright (C) 2021 Linaro Ltd.
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <log.h>

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/* iATU registers */
#define IATU_VIEWPORT_OFF                                   0x900
#define IATU_VIEWPORT_INBOUND                               BIT(31)
#define IATU_VIEWPORT_OUTBOUND                              0
#define IATU_VIEWPORT_REGION_INDEX(idx)                     ((idx) & 7)

#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0                   0x904
#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM          0x0
#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO           0x2
#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0         0x4
#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1         0x5
#define IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TH                BIT(12)

#define IATU_REGION_CTRL_2_OFF_OUTBOUND_0                   0x908
#define IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN         BIT(31)
#define IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE    BIT(28)
#define IATU_REGION_CTRL_2_OFF_OUTBOUND_0_MSG_CODE_32BIT    0xF
#define IATU_REGION_CTRL_2_OFF_OUTBOUND_0_MSG_CODE_64BIT    0xFF

#define IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0                   0x90C
#define IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0                 0x910
#define IATU_LIMIT_ADDR_OFF_OUTBOUND_0                      0x914
#define IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0                 0x918
#define IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0               0x91C

/* Clock and resets */
#define CORE_CONTROL                  0x000
#define APP_LTSSM_ENABLE              BIT(4)
#define DEVICE_TYPE                   (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define AXI_CLK_STOP                  0x004
#define DBI_ACLK_STOP                 BIT(8)
#define SLV_ACLK_STOP                 BIT(4)
#define MSTR_ACLK_STOP                BIT(0)
#define DBI_CSYSREQ_REG               BIT(9)
#define SLV_CSYSREQ_REG               BIT(5)
#define MSTR_CSYSREQ_REG              BIT(1)

#define RESET_CONTROL_1               0x00C
#define PERST_N_O_REG                 BIT(5)
#define PERST_N_I_REG                 BIT(4)
#define BUTTON_RST_N_REG              BIT(1)
#define PWUP_RST_N_REG                BIT(0)

#define RESET_CONTROL_2               0x010

#define RESET_SELECT_1                0x014
#define SQU_RST_SEL                   BIT(29)
#define PHY_RST_SEL                   BIT(28)
#define PWR_RST_SEL                   BIT(24)
#define STI_RST_SEL                   BIT(20)
#define N_STI_RST_SEL                 BIT(16)
#define CORE_RST_SEL                  BIT(12)
#define PERST_SEL                     BIT(4)
#define BUTTON_RST_SEL                BIT(1)
#define PWUP_RST_SEL                  BIT(0)

#define RESET_SELECT_2                0x018
#define DBI_ARST_SEL                  BIT(8)
#define SLV_ARST_SEL                  BIT(4)
#define MSTR_ARST_SEL                 BIT(0)

#define EM_CONTROL                    0x030
#define PRE_DET_STT_REG               BIT(4)

#define EM_SELECT                     0x034
#define PRE_DET_STT_SEL               BIT(4)

#define PM_CONTROL_2                  0x050
#define SYS_AUX_PWR_DET               BIT(8)

#define PHY_CONFIG_COM_6              0x114
#define PIPE_PORT_SEL                 GENMASK(1, 0)

#define LINK_MONITOR                  0x210
#define SMLH_LINK_UP                  BIT(0)

#define LINK_CAPABILITIES_REG         0x07C
#define PCIE_CAP_MAX_LINK_WIDTH       GENMASK(7, 4)
#define PCIE_CAP_MAX_LINK_SPEED       GENMASK(3, 0)

#define LINK_CONTROL_LINK_STATUS_REG  0x080
#define PCIE_CAP_NEGO_LINK_WIDTH      GENMASK(23, 20)
#define PCIE_CAP_LINK_SPEED           GENMASK(19, 16)

#define TYPE1_CLASS_CODE_REV_ID_REG   0x008
#define BASE_CLASS_CODE               0xFF000000
#define BASE_CLASS_CODE_VALUE         0x06
#define SUBCLASS_CODE                 0x00FF0000
#define SUBCLASS_CODE_VALUE           0x04
#define PROGRAM_INTERFACE             0x0000FF00
#define PROGRAM_INTERFACE_VALUE       0x00

#define GEN2_CONTROL_OFF              0x80c
#define DIRECT_SPEED_CHANGE           BIT(17)

#define MISC_CONTROL_1_OFF            0x8BC
#define DBI_RO_WR_EN                  BIT(0)

static void or_writel(void *base, u32 offs, u32 val)
{
	writel(readl(base + offs) | val, base + offs);
}

static void masked_writel(void *base, u32 offs, u32 mask, u32 val)
{
	u32 data;
	int shift = ffs(mask);	/* Note that ffs() returns 1 for 0x1 */

	if (val && shift > 1)
		val <<= shift - 1;

	if (mask != ~0)
		data = (readl(base + offs) & ~mask) | val;
	else
		data = val;

	writel(data, base + offs);
}

static u32 masked_readl(void *base, u32 offs, u32 mask)
{
	u32 data;
	int shift = ffs(mask);	/* Note that ffs() returns 1 for 0x1 */

	data = readl(base + offs);

	if (mask != ~0)
		data &= mask;
	if (shift > 1)
		data >>= shift - 1;

	return data;
}

/*
 * Since SynQuacer's PCIe RC is expected to be initialized in the
 * firmware (including U-Boot), devicetree doesn't have control
 * blocks.
 *
 * Thus, this will initialize the PCIe RC with fixed addresses.
 */

#define SYNQUACER_PCI_SEG0_CONFIG_BASE	0x60000000
#define SYNQUACER_PCI_SEG0_CONFIG_SIZE	0x07f00000
#define SYNQUACER_PCI_SEG0_DBI_BASE	0x583d0000
#define SYNQUACER_PCI_SEG0_EXS_BASE	0x58390000

#define SYNQUACER_PCI_SEG1_CONFIG_BASE	0x70000000
#define SYNQUACER_PCI_SEG1_CONFIG_SIZE	0x07f00000
#define SYNQUACER_PCI_SEG1_DBI_BASE	0x583c0000
#define SYNQUACER_PCI_SEG1_EXS_BASE	0x58380000

#define SIZE_16KB			0x00004000
#define SIZE_64KB			0x00010000
#define SIZE_1MB			0x00100000

#define SYNQUACER_PCI_DBI_SIZE		SIZE_16KB
#define SYNQUACER_PCI_EXS_SIZE		SIZE_64KB

#define NUM_SQ_PCI_RC	2

static const struct synquacer_pcie_base {
	phys_addr_t cfg_base;
	phys_addr_t dbi_base;
	phys_addr_t exs_base;
} synquacer_pci_bases[NUM_SQ_PCI_RC] = {
	{
		.cfg_base = SYNQUACER_PCI_SEG0_CONFIG_BASE,
		.dbi_base = SYNQUACER_PCI_SEG0_DBI_BASE,
		.exs_base = SYNQUACER_PCI_SEG0_EXS_BASE,
	}, {
		.cfg_base = SYNQUACER_PCI_SEG1_CONFIG_BASE,
		.dbi_base = SYNQUACER_PCI_SEG1_DBI_BASE,
		.exs_base = SYNQUACER_PCI_SEG1_EXS_BASE,
	},
};

/**
 * struct synquacer_ecam_pcie - synquacer_ecam PCIe controller state
 * @cfg_base: The base address of memory mapped configuration space
 */
struct synquacer_ecam_pcie {
	void *cfg_base;
	pci_size_t size;
	void *dbi_base;
	void *exs_base;
	int first_busno;

	struct pci_region mem;
	struct pci_region io;
	struct pci_region mem64;
};

DECLARE_GLOBAL_DATA_PTR;

/**
 * pci_synquacer_ecam_conf_address() - Calculate the address of a config access
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @paddress: Pointer to the pointer to write the calculates address to
 *
 * Calculates the address that should be accessed to perform a PCIe
 * configuration space access for a given device identified by the PCIe
 * controller device @pcie and the bus, device & function numbers in @bdf. If
 * access to the device is not valid then the function will return an error
 * code. Otherwise the address to access will be written to the pointer pointed
 * to by @paddress.
 */
static int pci_synquacer_ecam_conf_address(const struct udevice *bus,
					   pci_dev_t bdf, uint offset,
					   void **paddress)
{
	struct synquacer_ecam_pcie *pcie = dev_get_priv(bus);
	void *addr;

	addr = pcie->cfg_base;
	addr += PCIE_ECAM_OFFSET(PCI_BUS(bdf) - pcie->first_busno,
				 PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	*paddress = addr;

	return 0;
}

static bool pci_synquacer_ecam_addr_valid(const struct udevice *bus,
					  pci_dev_t bdf)
{
	struct synquacer_ecam_pcie *pcie = dev_get_priv(bus);
	int num_buses = DIV_ROUND_UP(pcie->size, 1 << 16);

	/*
	 * The Synopsys DesignWare PCIe controller in ECAM mode will not filter
	 * type 0 config TLPs sent to devices 1 and up on its downstream port,
	 * resulting in devices appearing multiple times on bus 0 unless we
	 * filter out those accesses here.
	 */
	if (PCI_BUS(bdf) == pcie->first_busno && PCI_DEV(bdf) > 0)
		return false;

	return (PCI_BUS(bdf) >= pcie->first_busno &&
		PCI_BUS(bdf) < pcie->first_busno + num_buses);
}

/**
 * pci_synquacer_ecam_read_config() - Read from configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 */
static int pci_synquacer_ecam_read_config(const struct udevice *bus,
					  pci_dev_t bdf, uint offset,
					  ulong *valuep, enum pci_size_t size)
{
	if (!pci_synquacer_ecam_addr_valid(bus, bdf)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	return pci_generic_mmap_read_config(bus, pci_synquacer_ecam_conf_address,
					    bdf, offset, valuep, size);
}

/**
 * pci_synquacer_ecam_write_config() - Write to configuration space
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 */
static int pci_synquacer_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
					   uint offset, ulong value,
					   enum pci_size_t size)
{
	if (!pci_synquacer_ecam_addr_valid(bus, bdf))
		return 0;

	return pci_generic_mmap_write_config(bus, pci_synquacer_ecam_conf_address,
					     bdf, offset, value, size);
}

/**
 * pci_synquacer_ecam_of_to_plat() - Translate from DT to device state
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pci_synquacer_ecam_of_to_plat(struct udevice *dev)
{
	struct synquacer_ecam_pcie *pcie = dev_get_priv(dev);
	struct fdt_resource reg_res;
	int i, err;

	debug("%s: called for %s\n", __func__, dev->name);

	err = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev), "reg",
			       0, &reg_res);
	if (err < 0) {
		pr_err("\"reg\" resource not found\n");
		return err;
	}

	/* Find the correct pair of the DBI/EXS base address */
	for (i = 0; i < NUM_SQ_PCI_RC; i++) {
		if (synquacer_pci_bases[i].cfg_base == reg_res.start)
			break;
	}
	if (i == NUM_SQ_PCI_RC) {
		pr_err("Unknown ECAM base address %lx.\n",
		       (unsigned long)reg_res.start);
		return -ENOENT;
	}
	pcie->dbi_base = map_physmem(synquacer_pci_bases[i].dbi_base,
				     SYNQUACER_PCI_DBI_SIZE, MAP_NOCACHE);
	if (!pcie->dbi_base) {
		pr_err("Failed to map DBI for %s\n", dev->name);
		return -ENOMEM;
	}

	pcie->exs_base = map_physmem(synquacer_pci_bases[i].exs_base,
				     SYNQUACER_PCI_EXS_SIZE, MAP_NOCACHE);
	if (!pcie->exs_base) {
		pr_err("Failed to map EXS for %s\n", dev->name);
		return -ENOMEM;
	}

	pcie->size = fdt_resource_size(&reg_res);
	pcie->cfg_base = map_physmem(reg_res.start, pcie->size, MAP_NOCACHE);
	if (!pcie->cfg_base) {
		pr_err("Failed to map config space for %s\n", dev->name);
		return -ENOMEM;
	}
	debug("mappings DBI: %p EXS: %p CFG: %p\n", pcie->dbi_base, pcie->exs_base, pcie->cfg_base);

	return 0;
}

static void pci_synquacer_pre_init(struct synquacer_ecam_pcie *pcie)
{
	void *base = pcie->exs_base;

	masked_writel(base, EM_SELECT, PRE_DET_STT_SEL, 0);
	masked_writel(base, EM_CONTROL, PRE_DET_STT_REG, 0);
	masked_writel(base, EM_CONTROL, PRE_DET_STT_REG, 1);

	/* 1: Assert all PHY / LINK resets */
	masked_writel(base, RESET_SELECT_1, PERST_SEL, 0);
	masked_writel(base, RESET_CONTROL_1, PERST_N_I_REG, 0);
	masked_writel(base, RESET_CONTROL_1, PERST_N_O_REG, 0);

	/* Device Reset(PERST#) is effective afrer Set device_type (RC) */
	masked_writel(base, RESET_SELECT_1, PWUP_RST_SEL, 0);
	masked_writel(base, RESET_CONTROL_1, PWUP_RST_N_REG, 0);
	masked_writel(base, RESET_SELECT_1, BUTTON_RST_SEL, 0);
	masked_writel(base, RESET_CONTROL_1, BUTTON_RST_N_REG, 0);
	masked_writel(base, RESET_SELECT_1, PWR_RST_SEL, 1);
	masked_writel(base, RESET_SELECT_2, MSTR_ARST_SEL, 1);
	masked_writel(base, RESET_SELECT_2, SLV_ARST_SEL, 1);
	masked_writel(base, RESET_SELECT_2, DBI_ARST_SEL, 1);
	masked_writel(base, RESET_SELECT_1, CORE_RST_SEL, 1);
	masked_writel(base, RESET_SELECT_1, STI_RST_SEL, 1);
	masked_writel(base, RESET_SELECT_1, N_STI_RST_SEL, 1);
	masked_writel(base, RESET_SELECT_1, SQU_RST_SEL, 1);
	masked_writel(base, RESET_SELECT_1, PHY_RST_SEL, 1);

	/* 2: Set P<n>_app_ltssm_enable='0' for reprogramming before linkup. */
	masked_writel(base, CORE_CONTROL, APP_LTSSM_ENABLE, 0);

	/* 3: Set device_type (RC) */
	masked_writel(base, CORE_CONTROL, DEVICE_TYPE, 4);
}

static void pci_synquacer_dbi_init(void *dbi_base)
{
	masked_writel(dbi_base, MISC_CONTROL_1_OFF, DBI_RO_WR_EN, 1);
	/* 4 Lanes */
	masked_writel(dbi_base, LINK_CAPABILITIES_REG,
		      PCIE_CAP_MAX_LINK_WIDTH, 4);
	/* Gen 2 */
	masked_writel(dbi_base, LINK_CAPABILITIES_REG,
		      PCIE_CAP_MAX_LINK_SPEED, 2);

	masked_writel(dbi_base, TYPE1_CLASS_CODE_REV_ID_REG,
		      BASE_CLASS_CODE, BASE_CLASS_CODE_VALUE);
	masked_writel(dbi_base, TYPE1_CLASS_CODE_REV_ID_REG,
		      SUBCLASS_CODE, SUBCLASS_CODE_VALUE);
	masked_writel(dbi_base, TYPE1_CLASS_CODE_REV_ID_REG,
		      PROGRAM_INTERFACE, PROGRAM_INTERFACE_VALUE);

	masked_writel(dbi_base, MISC_CONTROL_1_OFF, DBI_RO_WR_EN, 0);
}

static void pcie_sq_prog_outbound_atu(void *dbi_base, int index,
				      u64 cpu_base, u64 pci_base, u64 size,
				      u32 type, u32 flags)
{
	debug("%s: %p, %d, %llx, %llx, %llx, %x, %x\n", __func__,
	      dbi_base, index, cpu_base, pci_base, size, type, flags);

	writel(IATU_VIEWPORT_OUTBOUND | IATU_VIEWPORT_REGION_INDEX(index),
	       dbi_base + IATU_VIEWPORT_OFF);

	writel((u32)(cpu_base & 0xffffffff),
	       dbi_base + IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0);
	writel((u32)(cpu_base >> 32),
	       dbi_base + IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0);
	writel((u32)(cpu_base + size - 1),
	       dbi_base + IATU_LIMIT_ADDR_OFF_OUTBOUND_0);

	writel((u32)(pci_base & 0xffffffff),
	       dbi_base + IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0);
	writel((u32)(pci_base >> 32),
	       dbi_base + IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0);

	writel(type, dbi_base + IATU_REGION_CTRL_1_OFF_OUTBOUND_0);
	writel(IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN | flags,
	       dbi_base + IATU_REGION_CTRL_2_OFF_OUTBOUND_0);
}

static void pci_synquacer_post_init(struct synquacer_ecam_pcie *pcie)
{
	void *base = pcie->exs_base;

	/*
	 * 4: Set Bifurcation  1=disable  4=able
	 * 5: Supply Reference (It has executed)
	 * 6: Wait for 10usec (Reference Clocks is stable)
	 * 7: De assert PERST#
	 */
	masked_writel(base, RESET_CONTROL_1, PERST_N_I_REG, 1);
	masked_writel(base, RESET_CONTROL_1, PERST_N_O_REG, 1);

	/* 8: Assert SYS_AUX_PWR_DET */
	masked_writel(base, PM_CONTROL_2, SYS_AUX_PWR_DET, 1);

	/* 9: Supply following clocks */
	masked_writel(base, AXI_CLK_STOP, MSTR_CSYSREQ_REG, 1);
	masked_writel(base, AXI_CLK_STOP, MSTR_ACLK_STOP, 0);
	masked_writel(base, AXI_CLK_STOP, SLV_CSYSREQ_REG, 1);
	masked_writel(base, AXI_CLK_STOP, SLV_ACLK_STOP, 0);
	masked_writel(base, AXI_CLK_STOP, DBI_CSYSREQ_REG, 1);
	masked_writel(base, AXI_CLK_STOP, DBI_ACLK_STOP, 0);

	/*
	 * 10: De assert PHY reset
	 * 11: De assert LINK's PMC reset
	 */
	masked_writel(base, RESET_CONTROL_1, PWUP_RST_N_REG, 1);
	masked_writel(base, RESET_CONTROL_1, BUTTON_RST_N_REG, 1);

	/* 12: PHY auto
	 * 13: Wrapper auto
	 * 14-17: PHY auto
	 * 18: Wrapper auto
	 * 19: Update registers through DBI AXI Slave interface
	 */
	pci_synquacer_dbi_init(pcie->dbi_base);

	or_writel(pcie->dbi_base, PCI_COMMAND,
		  PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	/* Force link speed change to Gen2 at link up */
	or_writel(pcie->dbi_base, GEN2_CONTROL_OFF, DIRECT_SPEED_CHANGE);

	/* Region 0: MMIO32 range */
	pcie_sq_prog_outbound_atu(pcie->dbi_base, 0,
				  pcie->mem.phys_start,
				  pcie->mem.bus_start,
				  pcie->mem.size,
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM |
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TH,
				  IATU_REGION_CTRL_2_OFF_OUTBOUND_0_MSG_CODE_32BIT);

	/* Region 1: Type 0 config space */
	pcie_sq_prog_outbound_atu(pcie->dbi_base, 1,
				  (u64)pcie->cfg_base,
				  0,
				  SIZE_64KB,
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
				  IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE);

	/* Region 2: Type 1 config space */
	pcie_sq_prog_outbound_atu(pcie->dbi_base, 2,
				  (u64)pcie->cfg_base + SIZE_64KB,
				  0,
				  (u64)pcie->io.phys_start  - (u64)pcie->cfg_base - SIZE_64KB,
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
				  IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE);

	/* Region 3: port I/O range */
	pcie_sq_prog_outbound_atu(pcie->dbi_base, 3,
				  pcie->io.phys_start,
				  pcie->io.bus_start,
				  pcie->io.size,
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
				  0);

	/* Region 4: MMIO64 range */
	pcie_sq_prog_outbound_atu(pcie->dbi_base, 4,
				  pcie->mem64.phys_start,
				  pcie->mem64.bus_start,
				  pcie->mem64.size,
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM |
				  IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TH,
				  IATU_REGION_CTRL_2_OFF_OUTBOUND_0_MSG_CODE_32BIT);

	/* enable link */
	if (masked_readl(base, CORE_CONTROL, APP_LTSSM_ENABLE) == 0)
		masked_writel(base, CORE_CONTROL, APP_LTSSM_ENABLE, 1);
}

static int pci_synquacer_ecam_probe(struct udevice *dev)
{
	struct synquacer_ecam_pcie *pcie = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);

	debug("Probe synquacer pcie for bus %d\n", dev_seq(dev));
	pcie->first_busno = dev_seq(dev);

	/* Store the IO and MEM windows settings for configuring ATU */
	pcie->io.phys_start = hose->regions[0].phys_start; /* IO base */
	pcie->io.bus_start  = hose->regions[0].bus_start;  /* IO_bus_addr */
	pcie->io.size	    = hose->regions[0].size;	   /* IO size */

	pcie->mem.phys_start = hose->regions[1].phys_start; /* MEM base */
	pcie->mem.bus_start  = hose->regions[1].bus_start;  /* MEM_bus_addr */
	pcie->mem.size	     = hose->regions[1].size;	    /* MEM size */

	pcie->mem64.phys_start = hose->regions[2].phys_start; /* MEM64 base */
	pcie->mem64.bus_start  = hose->regions[2].bus_start;  /* MEM64_bus_addr */
	pcie->mem64.size       = hose->regions[2].size;	    /* MEM64 size */

	pci_synquacer_pre_init(pcie);

	mdelay(150);

	pci_synquacer_post_init(pcie);

	/* It takes a while to stabilize the PCIe bus for scanning */
	mdelay(100);

	return 0;
}

static const struct dm_pci_ops pci_synquacer_ecam_ops = {
	.read_config	= pci_synquacer_ecam_read_config,
	.write_config	= pci_synquacer_ecam_write_config,
};

static const struct udevice_id pci_synquacer_ecam_ids[] = {
	{ .compatible = "socionext,synquacer-pcie-ecam" },
	{ }
};

U_BOOT_DRIVER(pci_synquacer_ecam) = {
	.name			= "pci_synquacer_ecam",
	.id			= UCLASS_PCI,
	.of_match		= pci_synquacer_ecam_ids,
	.ops			= &pci_synquacer_ecam_ops,
	.probe			= pci_synquacer_ecam_probe,
	.of_to_plat		= pci_synquacer_ecam_of_to_plat,
	.priv_auto		= sizeof(struct synquacer_ecam_pcie),
};
