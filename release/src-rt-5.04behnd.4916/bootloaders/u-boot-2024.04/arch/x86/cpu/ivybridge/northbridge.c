// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot northbridge/intel/sandybridge/northbridge.c
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The Chromium Authors
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/msr.h>
#include <asm/cpu.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/processor.h>
#include <asm/arch/pch.h>
#include <asm/arch/model_206ax.h>
#include <asm/arch/sandybridge.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int bridge_silicon_revision(struct udevice *dev)
{
	struct cpuid_result result;
	u16 bridge_id;
	u8 stepping;

	result = cpuid(1);
	stepping = result.eax & 0xf;
	dm_pci_read_config16(dev, PCI_DEVICE_ID, &bridge_id);
	bridge_id &= 0xf0;
	return bridge_id | stepping;
}

static int get_pcie_bar(struct udevice *dev, u32 *base, u32 *len)
{
	u32 pciexbar_reg;

	*base = 0;
	*len = 0;

	dm_pci_read_config32(dev, PCIEXBAR, &pciexbar_reg);

	if (!(pciexbar_reg & (1 << 0)))
		return 0;

	switch ((pciexbar_reg >> 1) & 3) {
	case 0: /* 256MB */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28));
		*len = 256 * 1024 * 1024;
		return 1;
	case 1: /* 128M */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28) | (1 << 27));
		*len = 128 * 1024 * 1024;
		return 1;
	case 2: /* 64M */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28) | (1 << 27) | (1 << 26));
		*len = 64 * 1024 * 1024;
		return 1;
	}

	return 0;
}

static void add_fixed_resources(struct udevice *dev, int index)
{
	u32 pcie_config_base, pcie_config_size;

	if (get_pcie_bar(dev, &pcie_config_base, &pcie_config_size)) {
		debug("Adding PCIe config bar base=0x%08x size=0x%x\n",
		      pcie_config_base, pcie_config_size);
	}
}

static void northbridge_dmi_init(struct udevice *dev, int rev)
{
	/* Clear error status bits */
	writel(0xffffffff, DMIBAR_REG(0x1c4));
	writel(0xffffffff, DMIBAR_REG(0x1d0));

	/* Steps prior to DMI ASPM */
	if ((rev & BASE_REV_MASK) == BASE_REV_SNB) {
		clrsetbits_le32(DMIBAR_REG(0x250), (1 << 22) | (1 << 20),
				1 << 21);
	}

	setbits_le32(DMIBAR_REG(0x238), 1 << 29);

	if (rev >= SNB_STEP_D0) {
		setbits_le32(DMIBAR_REG(0x1f8), 1 << 16);
	} else if (rev >= SNB_STEP_D1) {
		clrsetbits_le32(DMIBAR_REG(0x1f8), 1 << 26, 1 << 16);
		setbits_le32(DMIBAR_REG(0x1fc), (1 << 12) | (1 << 23));
	}

	/* Enable ASPM on SNB link, should happen before PCH link */
	if ((rev & BASE_REV_MASK) == BASE_REV_SNB)
		setbits_le32(DMIBAR_REG(0xd04), 1 << 4);

	setbits_le32(DMIBAR_REG(0x88), (1 << 1) | (1 << 0));
}

static void northbridge_init(struct udevice *dev, int rev)
{
	u32 bridge_type;

	add_fixed_resources(dev, 6);
	northbridge_dmi_init(dev, rev);

	bridge_type = readl(MCHBAR_REG(0x5f10));
	bridge_type &= ~0xff;

	if ((rev & BASE_REV_MASK) == BASE_REV_IVB) {
		/* Enable Power Aware Interrupt Routing - fixed priority */
		clrsetbits_8(MCHBAR_REG(0x5418), 0xf, 0x4);

		/* 30h for IvyBridge */
		bridge_type |= 0x30;
	} else {
		/* 20h for Sandybridge */
		bridge_type |= 0x20;
	}
	writel(bridge_type, MCHBAR_REG(0x5f10));

	/*
	 * Set bit 0 of BIOS_RESET_CPL to indicate to the CPU
	 * that BIOS has initialized memory and power management
	 */
	setbits_8(MCHBAR_REG(BIOS_RESET_CPL), 1);
	debug("Set BIOS_RESET_CPL\n");

	/* Configure turbo power limits 1ms after reset complete bit */
	mdelay(1);
	set_power_limits(28);

	/*
	 * CPUs with configurable TDP also need power limits set
	 * in MCHBAR.  Use same values from MSR_PKG_POWER_LIMIT.
	 */
	if (cpu_ivybridge_config_tdp_levels()) {
		msr_t msr = msr_read(MSR_PKG_POWER_LIMIT);

		writel(msr.lo, MCHBAR_REG(0x59A0));
		writel(msr.hi, MCHBAR_REG(0x59A4));
	}

	/* Set here before graphics PM init */
	writel(0x00100001, MCHBAR_REG(0x5500));
}

static void sandybridge_setup_northbridge_bars(struct udevice *dev)
{
	/* Set up all hardcoded northbridge BARs */
	debug("Setting up static registers\n");
	dm_pci_write_config32(dev, EPBAR, DEFAULT_EPBAR | 1);
	dm_pci_write_config32(dev, EPBAR + 4, (0LL + DEFAULT_EPBAR) >> 32);
	dm_pci_write_config32(dev, MCHBAR, MCH_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev, MCHBAR + 4, (0LL + MCH_BASE_ADDRESS) >> 32);
	/* 64MB - busses 0-63 */
	dm_pci_write_config32(dev, PCIEXBAR, DEFAULT_PCIEXBAR | 5);
	dm_pci_write_config32(dev, PCIEXBAR + 4,
			      (0LL + DEFAULT_PCIEXBAR) >> 32);
	dm_pci_write_config32(dev, DMIBAR, DEFAULT_DMIBAR | 1);
	dm_pci_write_config32(dev, DMIBAR + 4, (0LL + DEFAULT_DMIBAR) >> 32);

	/* Set C0000-FFFFF to access RAM on both reads and writes */
	dm_pci_write_config8(dev, PAM0, 0x30);
	dm_pci_write_config8(dev, PAM1, 0x33);
	dm_pci_write_config8(dev, PAM2, 0x33);
	dm_pci_write_config8(dev, PAM3, 0x33);
	dm_pci_write_config8(dev, PAM4, 0x33);
	dm_pci_write_config8(dev, PAM5, 0x33);
	dm_pci_write_config8(dev, PAM6, 0x33);
}

/**
 * sandybridge_init_iommu() - Set up IOMMU so that azalia can be used
 *
 * It is not obvious where these values come from. They may be undocumented.
 */
static void sandybridge_init_iommu(struct udevice *dev)
{
	u32 capid0_a;

	dm_pci_read_config32(dev, 0xe4, &capid0_a);
	if (capid0_a & (1 << 23)) {
		log_debug("capid0_a not needed\n");
		return;
	}

	/* setup BARs */
	writel(IOMMU_BASE1 >> 32, MCHBAR_REG(0x5404));
	writel(IOMMU_BASE1 | 1, MCHBAR_REG(0x5400));
	writel(IOMMU_BASE2 >> 32, MCHBAR_REG(0x5414));
	writel(IOMMU_BASE2 | 1, MCHBAR_REG(0x5410));

	/* lock policies */
	writel(0x80000000, IOMMU_BASE1 + 0xff0);

	/* Enable azalia sound */
	writel(0x20000000, IOMMU_BASE2 + 0xff0);
	writel(0xa0000000, IOMMU_BASE2 + 0xff0);
}

static int bd82x6x_northbridge_early_init(struct udevice *dev)
{
	const int chipset_type = SANDYBRIDGE_MOBILE;
	u32 capid0_a;
	u8 reg8;

	/* Device ID Override Enable should be done very early */
	dm_pci_read_config32(dev, 0xe4, &capid0_a);
	if (capid0_a & (1 << 10)) {
		dm_pci_read_config8(dev, 0xf3, &reg8);
		reg8 &= ~7; /* Clear 2:0 */

		if (chipset_type == SANDYBRIDGE_MOBILE)
			reg8 |= 1; /* Set bit 0 */

		dm_pci_write_config8(dev, 0xf3, reg8);
	}

	sandybridge_setup_northbridge_bars(dev);

	/* Setup IOMMU BARs */
	sandybridge_init_iommu(dev);

	/* Device Enable */
	dm_pci_write_config32(dev, DEVEN, DEVEN_HOST | DEVEN_IGD);

	return 0;
}

static int bd82x6x_northbridge_probe(struct udevice *dev)
{
	int rev;

	if (!(gd->flags & GD_FLG_RELOC))
		return bd82x6x_northbridge_early_init(dev);

	rev = bridge_silicon_revision(dev);
	northbridge_init(dev, rev);

	return 0;
}

static const struct udevice_id bd82x6x_northbridge_ids[] = {
	{ .compatible = "intel,bd82x6x-northbridge" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_northbridge_drv) = {
	.name		= "bd82x6x_northbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= bd82x6x_northbridge_ids,
	.probe		= bd82x6x_northbridge_probe,
};
